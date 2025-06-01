// Project BrewDev - FOSS Gaggia Controller
// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include <SPI.h>                 // Standard SPI library, often used by MAX31855
#include <Adafruit_MAX31855.h>   // For the MAX31855 TC sensor
#include <Adafruit_MAX31865.h>   // For PT100 RTD Sensor (Brew)       
#include <PID_v1.h>              // Brett Beauregard's PID library
#include <U8g2lib.h>             // For the OLED display by olikraus
#include <AiEsp32RotaryEncoder.h> // For the ESP32 specific Rotary Encoder
#include <Preferences.h>         // For saving settings on ESP32
#include <Wire.h>                // For I2C communication (used by U8g2)

///////////////////////////////////////////////////////////////////////
// Pin Definitions (Using placeholders replace with actual ESP32 pins)
///////////////////////////////////////////////////////////////////////

// MAX31855 Thermocouple (Hardware SPI)
// Note: On ESP32, default HSPI pins are: CS=15, MISO=12, MOSI=13, SCK=14
// You can often redefine CS, but MISO, MOSI, SCK are best left to defaults if using hardware SPI.
// Let's assume you'll use a software-definable CS pin and default HSPI for others if needed,
// or you can define all if using software SPI (slower).

//Thermocouples

// SPI Bus Pins (Software SPI on VSPI pins)
const int SPI_SCK_PIN  = 18; // ESP32's VSPI SCK
const int SPI_MISO_PIN = 19; // ESP32's VSPI MISO
const int SPI_MOSI_PIN = 23; // ESP32's VSPI MOSI (Needed for MAX31865)

// MAX31855 for steam uses default Hardware SPI (HSPI) pins.
// Default HSPI for ESP32 are usually: MISO=12, MOSI=13, SCK=14. CS is defined below.

// Chip Select (CS) Pins
const int MAX31855_CS_PIN_STEAM = 4;  // CS for K-type Steam Thermocouple (MAX31855) - using your previous example pin
const int MAX31865_CS_PIN_BREW  = 5;  // CS for PT100 Brew Sensor (MAX31865) - using your previous example pin
// // OLED Display (I2C)
const int OLED_SDA_PIN = 21;
const int OLED_SCL_PIN = 22;

// Rotary Encoder (KY-040) for AiEsp32RotaryEncoder
const int ROTARY_ENCODER_A_PIN       = 32; // CLK pin on KY-040
const int ROTARY_ENCODER_B_PIN       = 33; // DT pin on KY-040
const int ROTARY_ENCODER_BUTTON_PIN  = 25; // SW pin on KY-040

// SSR (Solid State Relay) Control Pin
const int SSR_CONTROL_PIN = 26;

// Gaggia Machine Switch Inputs (these will need safe interfacing, e.g., optocouplers)
const int GAGGIA_BREW_SWITCH_PIN  = 16; 
const int GAGGIA_STEAM_SWITCH_PIN = 17; 

// "Ready" LED Indicator
const int READY_LED_PIN = 27;

// -----------------------------------------------------------------------------
// Global Objects & Variables
// -----------------------------------------------------------------------------

// --- Sensors ---
// K-Type Thermocouple for Steam (MAX31855)
// Uses hardware SPI with specified pins. Note: Adafruit_MAX31855 constructor can take all 3 SPI pins for software SPI
// or just CS for hardware SPI (using default SPI). Let's be explicit for hardware SPI.
// Adafruit_MAX31855 thermocouple_steam(SPI_SCK_PIN, MAX31855_CS_PIN_STEAM, SPI_MISO_PIN); // If using this constructor style for explicitness
// OR, if the library defaults to using the global SPI object when only CS is passed:

Adafruit_MAX31855 thermocouple_steam(MAX31855_CS_PIN_STEAM);


// PT100 RTD Sensor for Brew (MAX31865)
// Constructor for hardware SPI: Adafruit_MAX31865(cs_pin, mosi_pin, miso_pin, sck_pin)
// Note: some Adafruit libraries allow passing just CS and assume default hardware SPI pins.
// Let's use the explicit hardware SPI constructor to be clear.

Adafruit_MAX31865 sensor_pt100_brew(MAX31865_CS_PIN_BREW, SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN);

// --- OLED Display (U8g2) ---
// Constructor for 1.3" SH1106 I2C OLED. U8G2_R0 = No rotation.

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// --- Rotary Encoder (AiEsp32RotaryEncoder) ---
// Last parameter is for VCC, -1 means not used/needed for basic KY-040

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, -1);

// --- PID Controller ---

double currentBrewTemperature;
double currentSteamTemperature;
double currentTemperature;
double pidOutput;
double brewSetTemperature;
double steamSetTemperature;
double activeSetTemperature;

// Brew Mode Tunings (Placeholders)

double Kp_brew = 30.0; double Ki_brew = 1.0; double Kd_brew = 15.0;

// Steam Mode Tunings (Placeholders)

double Kp_steam = 40.0; double Ki_steam = 2.0; double Kd_steam = 8.0;

PID brewPid(&currentTemperature, &pidOutput, &activeSetTemperature, Kp_brew, Ki_brew, Kd_brew, DIRECT);

unsigned long pidWindowSize = 5000; // 5 seconds for time-proportioned PID output
unsigned long pidWindowStartTime;

// --- PT100 Specific Constants ---

const float PT100_RNOMINAL = 100.0; // Nominal resistance of PT100 at 0°C
const float PT100_RREF     = 430.0; // Reference resistor value on your MAX31865 board 

// --- Ready LED Temperature Tolerance ---

const double READY_TEMP_TOLERANCE = 0.5; // Degrees Celsius +/- for "Ready" state

// --- Preferences (for saving settings) ---

Preferences preferences;
const char* prefs_brew_temp_key = "brew_temp";
const char* prefs_steam_temp_key = "steam_temp";

// --- Operational State / Mode ---

enum OperatingMode { BREW_MODE, STEAM_MODE };
OperatingMode currentMode = BREW_MODE;
OperatingMode previousMode = BREW_MODE;
bool brewSwitchState = false;  // Current state of the Gaggia brew switch (LOW if active)
bool steamSwitchState = false; // Current state of the Gaggia steam switch (LOW if active)

// --- Shot Timer ---

unsigned long shotStartTime = 0;
unsigned long currentShotDuration = 0;
bool shotTimerRunning = false;

// --- Ready LED ---

bool systemReady = false;

/////////////////
///// setup /////
/////////////////

void setup() {
  // Start Serial communication for debugging (optional, but very helpful)
  Serial.begin(115200);
  unsigned long setupStartTime = millis();
  while (!Serial && (millis() - setupStartTime < 2000)) { // Wait a bit for serial to connect, but don't hang forever
    delay(100);
  }
  Serial.println("\n\nProject BrewDev Starting Up...");

  // --- Initialize Preferences (Non-Volatile Storage) ---
  preferences.begin("brewdev", false); // "brewdev" namespace, false for R/W mode
  // Load saved setpoint temperatures (or use defaults if not found)
  brewSetTemperature = preferences.getDouble(prefs_brew_temp_key, 92.0); // Default 92.0°C
  steamSetTemperature = preferences.getDouble(prefs_steam_temp_key, 140.0); // Default 140.0°C
  Serial.print("Loaded Brew Set Temperature: "); Serial.println(brewSetTemperature);
  Serial.print("Loaded Steam Set Temperature: "); Serial.println(steamSetTemperature);
  
  // Set the initial active setpoint (e.g., to brew temperature, assuming it starts in a non-steam mode)
  //
  activeSetTemperature = brewSetTemperature; 
  currentMode = BREW_MODE; // Start in BREW, can switch based on Gaggia switches later

  // --- Initialize Sensors ---
  // PT100 for Brew Temperature

  Serial.println("Initializing PT100 Brew Sensor (MAX31865)...");
  if (!sensor_pt100_brew.begin()) { // Specify PT100 type
    Serial.println("ERROR: Could not initialize PT100/MAX31865 for brew! Check wiring.");
    // Consider how to handle this error more robustly
  } else {
    // Configure for your PT100 wire type (2, 3, or 4 wire)
    // Example for a 3-wire PT100:
    sensor_pt100_brew.setWires(MAX31865_3WIRE); 
    // You'll need to check your PT100 probe and MAX31865 module for how to set this.
    // Other options: MAX31865_2WIRE, MAX31865_4WIRE
    Serial.println("PT100 Brew Sensor (MAX31865) initialized.");
  }
  delay(250); // Short delay for sensor
  currentBrewTemperature = sensor_pt100_brew.temperature(PT100_RNOMINAL, PT100_RREF);

  // Check for faults with PT100 (more specific than just isnan)

  uint8_t fault = sensor_pt100_brew.readFault();
  if (fault) {
    Serial.print("Fault detected on PT100 Brew Sensor: 0x"); Serial.println(fault, HEX);
    sensor_pt100_brew.clearFault(); // Clear the fault
    currentBrewTemperature = -1; // Error value
  } else if (isnan(currentBrewTemperature)) { // Fallback isnan check
     Serial.println("ERROR: Brew temperature is NaN after PT100 init! Check wiring/RREF.");
     currentBrewTemperature = -1; // Error value
  }else {
    Serial.print("Initial Brew Temperature (PT100): "); Serial.print(currentBrewTemperature); Serial.println(" *C");
  }

  // K-Type for Steam Temperature
  Serial.println("Initializing K-Type Steam Thermocouple (MAX31855)...");
  if (!thermocouple_steam.begin()) { // Assuming hardware SPI, this might not return a useful bool
    Serial.println("Warning: MAX31855 thermocouple_steam.begin() called.");
  }

  delay(250); // Short delay for sensors

  currentSteamTemperature = thermocouple_steam.readCelsius();
  if (isnan(currentSteamTemperature)) {
    Serial.println("ERROR: Could not read temperature from Steam Thermocouple! Check wiring.");
    currentSteamTemperature = -1; // Error value
  } else {
    Serial.print("Initial Steam Temperature (K-Type): "); Serial.print(currentSteamTemperature); Serial.println(" *C");
  }

  // Set the generic currentTemperature for initial PID setup (defaults to brew)

  currentTemperature = currentBrewTemperature; 

  // --- Initialize OLED Display (U8g2) ---

Serial.println("Initializing I2C Bus for OLED...");
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN); // Explicitly initialize I2C with defined pins

  Serial.println("Initializing OLED Display...");
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr); // Choose a font (ncenB08 is a nice readable one)
  u8g2.clearBuffer();
  u8g2.drawStr(0, 12, "Project BrewDev"); // Adjusted y-pos for typical 8px font height
  u8g2.drawStr(0, 26, "Initializing...");
  u8g2.sendBuffer();
  delay(2000); // Show init message for 2 seconds

  // --- Initialize Rotary Encoder ---

  Serial.println("Initializing Rotary Encoder...");
  rotaryEncoder.begin();
  // Set boundaries for temperature adjustment (e.g., 70°C to 150°C for brew/steam range)
  // We can adjust these min/max values later. False means no wrap-around.
  rotaryEncoder.setBoundaries(70, 150, false); 
  rotaryEncoder.setEncoderValue(activeSetTemperature); // Set initial encoder value to the current active setpoint
  rotaryEncoder.setAcceleration(250); // Lower is faster acceleration. 0 means no acceleration. 250 is moderate.
  // rotaryEncoder.setKnobNotPressedValue(HIGH); // If your encoder button is active LOW (goes to GND when pressed), it's HIGH when not pressed. This is often default.

  // --- Initialize PID Controller ---

  Serial.println("Initializing PID Controller...");
  // Set PID sample time (how often it calculates). 1000ms = 1 second.
  brewPid.SetSampleTime(1000);
  brewPid.SetOutputLimits(0, pidWindowSize);
  brewPid.SetTunings(Kp_brew, Ki_brew, Kd_brew);
  brewPid.SetMode(AUTOMATIC); // Turn the PID on
  pidWindowStartTime = millis(); // Initialize PID window start time

  // --- Set Pin Modes ---

  Serial.println("Setting Pin Modes...");
  pinMode(SSR_CONTROL_PIN, OUTPUT);
  digitalWrite(SSR_CONTROL_PIN, LOW); // Ensure SSR is off initially

  // Using INPUT_PULLUP assumes the switches connect the pin to GND when active (active LOW)

  pinMode(GAGGIA_BREW_SWITCH_PIN, INPUT_PULLUP);  
  pinMode(GAGGIA_STEAM_SWITCH_PIN, INPUT_PULLUP); 

  pinMode(READY_LED_PIN, OUTPUT);
  digitalWrite(READY_LED_PIN, LOW); // Ensure LED is off initially

  Serial.println("Setup Complete. Project BrewDev is Ready!");
  u8g2.clearBuffer(); 
  u8g2.setCursor(0,12);
  u8g2.print("Ready!");
  u8g2.setCursor(0,26);
  u8g2.print("Temp: "); u8g2.print(currentTemperature, 1); u8g2.print("C"); 

  // Display initial temp
  u8g2.sendBuffer();
}
//////////////////////////////
// loop() - Runs repeatedly //
//////////////////////////////

 void loop() {
  // --- 1- Read Gaggia Switch Inputs & Determine Mode ---
  updateOperationMode();
  // --- 2- Read Current Temperatures from Both Sensors ---
  readSensorTemperatures();
  // --- 3- Helper for all PID related updates
  processPidLogic();
  // --- 4- SSR control
  controlSSR();
  // --- 5 - Encoder logic
  handleEncoderInput();
  // --- 6 - Manage Shot Timer
  manageShotTimer();
  // --- 7 - Update "Ready" LED
  updateReadyLedState();
  // --- 8 - Update OLED Display
  refreshOledDisplay();
 }


  // --------------------------------------------------------------------// Helper Functions // ------------------------------------------------//-------------------------------------------------------------------



//////////////////////////
// 1. READ SWITCH INPUT //
//////////////////////////

  void updateOperationMode() {
  // Read Gaggia Switch Inputs
  // Assuming brewSwitchState and steamSwitchState are global, or pass them if local
  brewSwitchState = (digitalRead(GAGGIA_BREW_SWITCH_PIN) == LOW);
  steamSwitchState = (digitalRead(GAGGIA_STEAM_SWITCH_PIN) == LOW);

  previousMode = currentMode; // Store last mode

  if (steamSwitchState) {
    currentMode = STEAM_MODE;
  } else {
    currentMode = BREW_MODE;
  }

  if (currentMode != previousMode) {
    Serial.print("Mode changed to: ");
    if (currentMode == BREW_MODE) Serial.println("BREW");
    else if (currentMode == STEAM_MODE) Serial.println("STEAM");
  }
}
/////////////////////////////////
// 2. Read Sensor Temperatures //
/////////////////////////////////

void readSensorTemperatures() {

  // Brew Temperature (PT100 via MAX31865)

  double brewTempReading = sensor_pt100_brew.temperature(PT100_RNOMINAL, PT100_RREF);
  uint8_t brewFault = sensor_pt100_brew.readFault();
  if (brewFault) {
    Serial.print("Fault detected on PT100 Brew Sensor in loop: 0x"); Serial.println(brewFault, HEX);
    sensor_pt100_brew.clearFault();
    // Optionally set currentBrewTemperature to an error value or handle more robustly
  } else if (!isnan(brewTempReading)) {
    currentBrewTemperature = brewTempReading;
  } else {
    Serial.println("Warning: Failed to read valid temperature from Brew PT100 in loop!");
  }

  // Steam Temperature (K-Type via MAX31855)

  double steamTempReading = thermocouple_steam.readCelsius();
  if (!isnan(steamTempReading)) {
    currentSteamTemperature = steamTempReading;
  } else {
    Serial.println("Warning: Failed to read valid temperature from Steam K-Type in loop!");
  }
}

/////////////////
// 3.PID logic //
/////////////////
//
void processPidLogic() {
  // Update PID Input (currentTemperature) based on Current Mode
  // This ensures the PID object uses the correct sensor data for its calculations.
  if (currentMode == BREW_MODE) {
    currentTemperature = currentBrewTemperature; 
  } else if (currentMode == STEAM_MODE) {
    currentTemperature = currentSteamTemperature;
  }
  // Now 'currentTemperature' holds the reading from the sensor relevant to the current mode.

  // Update PID Controller based on Mode (if mode has changed)

  if (currentMode != previousMode) { 
    if (currentMode == BREW_MODE) {
      activeSetTemperature = brewSetTemperature;
      brewPid.SetTunings(Kp_brew, Ki_brew, Kd_brew); // Apply brew tunings
      Serial.println("PID configured for BREW Mode.");
      brewPid.SetMode(AUTOMATIC);    // Ensure PID is on
    } else if (currentMode == STEAM_MODE) { 
      activeSetTemperature = steamSetTemperature;
      brewPid.SetTunings(Kp_steam, Ki_steam, Kd_steam); 
      Serial.println("PID configured for STEAM Mode.");
      brewPid.SetMode(AUTOMATIC);    // Ensure PID is on
    }
    rotaryEncoder.setEncoderValue(activeSetTemperature);
    pidWindowStartTime = millis(); // Reset PID window
  }

  // PID computation should happen every loop now that we are always in an active mode

  bool computed = brewPid.Compute();
  if (computed) {
    // A good place for debugging PID output if needed later:
    // Serial.print("PID Output: "); Serial.println(pidOutput); 
  }
}

////////////////////
// - 4. SSR logic //
////////////////////

void controlSSR() {
  // Control the SSR (Heating Element) using Time-Proportioned Window
  unsigned long now = millis();
  
  if (currentMode == BREW_MODE || currentMode == STEAM_MODE) {
    // Time-proportioned PID output control
    if (now - pidWindowStartTime >= pidWindowSize) { // Use >= to be precise
      // Time to start a new window
      // pidWindowStartTime += pidWindowSize; // Catches up if a loop was too long - can cause drift if loop is very slow
      // More robust against very slow loops, though += is fine if loops are fast:
      pidWindowStartTime = now; 
    }

    // pidOutput is the ON-time in ms for the current window (due to SetOutputLimits in setup)
    if (pidOutput > (now - pidWindowStartTime)) {
      digitalWrite(SSR_CONTROL_PIN, HIGH); // Turn SSR ON
      // Serial.println("SSR ON"); // Debug
    } else {
      digitalWrite(SSR_CONTROL_PIN, LOW);  // Turn SSR OFF
      // Serial.println("SSR OFF"); // Debug
    }
  } else { 
    // This 'else' block should ideally not be reached if currentMode is always BREW or STEAM.
    // However, as a failsafe:
    digitalWrite(SSR_CONTROL_PIN, LOW); // Ensure SSR is OFF
    // Serial.println("SSR OFF - Failsafe (Unexpected Mode)"); // Debug
  }
}

/////////////////////
// 5.Encoder logic //
/////////////////////

void handleEncoderInput() {
  if (rotaryEncoder.encoderChanged()) {
    long newEncoderValue = rotaryEncoder.readEncoder();
    Serial.print("Encoder Value: "); Serial.println(newEncoderValue);

    if (currentMode == BREW_MODE) {
      brewSetTemperature = (double)newEncoderValue;
      activeSetTemperature = brewSetTemperature; // Ensure PID's setpoint is also updated live
      preferences.putDouble(prefs_brew_temp_key, brewSetTemperature); // Save it
      Serial.print("New Brew Set Temperature: "); Serial.println(brewSetTemperature);
    } else if (currentMode == STEAM_MODE) {
      steamSetTemperature = (double)newEncoderValue;
      activeSetTemperature = steamSetTemperature; // Ensure PID's setpoint is also updated live
      preferences.putDouble(prefs_steam_temp_key, steamSetTemperature); // Save it
      Serial.print("New Steam Set Temperature: "); Serial.println(steamSetTemperature);
    }
    // Note: The PID's actual setpoint variable 'activeSetTemperature' is updated directly here.
    // The PID object itself uses a *pointer* to 'activeSetTemperature', so it sees the change automatically.
  }

  if (rotaryEncoder.isEncoderButtonClicked()) {
    // Encoder button clicked, but no specific action defined for V1 yet (Option B from our discussion).
    // Serial.println("Encoder Button Clicked! (No V1 action)"); // Optional debug
  }

  // Optional: Check for long press or other button events if needed from AiEsp32RotaryEncoder
  // if (rotaryEncoder.isEncoderButtonLongPressed()) {
  //   Serial.println("Encoder Button Long Pressed!");
  // }
}

//////////////////////////
// 6. Manage Shot Timer //
//////////////////////////

void manageShotTimer() {
  // brewSwitchState is global and updated by updateOperationMode()
  // It's true if the Gaggia's brew switch is ON (active LOW)

  if (brewSwitchState) {
    // Brew switch is ON
    if (!shotTimerRunning) {
      // Timer was not running, so let's start it
      shotTimerRunning = true;
      shotStartTime = millis();
      currentShotDuration = 0; // Reset duration at the start of a new shot
      Serial.println("Shot Timer Started.");
    } else {
      // Timer is already running, update the duration
      currentShotDuration = millis() - shotStartTime;
    }
  } else {
    // Brew switch is OFF
    if (shotTimerRunning) {
      // Timer was running, so let's stop it
      shotTimerRunning = false;
      Serial.print("Shot Timer Stopped. Final Duration: ");
      Serial.print(currentShotDuration / 1000.0, 1); // Display in seconds with one decimal
      Serial.println("s");
      // The requirement is "resets when brew switch is off".
      currentShotDuration = 0; 
    }
    // Ensure it's always 0 when the switch is off as per "resets when brew switch is off"
    if (!shotTimerRunning && currentShotDuration != 0) { 
        currentShotDuration = 0;
    }
  }

  // Optional: For debugging, print currentShotDuration every second if running
  // static unsigned long lastTimerPrintTime = 0;
  // if (shotTimerRunning && (millis() - lastTimerPrintTime > 1000)) {
  //   Serial.print("Shot Duration: "); Serial.print(currentShotDuration / 1000.0, 1); Serial.println("s");
  //   lastTimerPrintTime = millis();
  // }
}

/////////////////////////
// 7. Update Ready LED //
/////////////////////////

void updateReadyLedState() {
  // The system is considered "ready" if the current temperature is within tolerance of the active setpoint,
  // and we are in an active heating mode.
  if (currentMode == BREW_MODE || currentMode == STEAM_MODE) {
    if (abs(currentTemperature - activeSetTemperature) <= READY_TEMP_TOLERANCE) {
      if (!systemReady) { // Only print if state changes
        Serial.println("System is now READY (temperature at setpoint).");
      }
      systemReady = true;
      digitalWrite(READY_LED_PIN, HIGH); // Turn Ready LED ON
    } else {
      if (systemReady) { // Only print if state changes
        Serial.println("System is NOT ready (temperature drifting).");
      }
      systemReady = false;
      digitalWrite(READY_LED_PIN, LOW);  // Turn Ready LED OFF
    }
  } else {
    // This 'else' block should ideally not be reached if currentMode is always BREW or STEAM.
    // However, as a failsafe:
    if (systemReady) { // Only print if state changes
        Serial.println("System is NOT ready (not in an active heating mode - failsafe).");
    }
    systemReady = false;
    digitalWrite(READY_LED_PIN, LOW); // Turn Ready LED OFF
  }
}

/////////////////////////////
// 8. Refresh OLED Display //
/////////////////////////////

void refreshOledDisplay() {
  // Periodically updates the display (e.g., every 100ms)
  static unsigned long lastDisplayUpdateTime = 0; // Static to retain value across calls
  
  if (millis() - lastDisplayUpdateTime > 100) { // Update display every 100ms
    lastDisplayUpdateTime = millis(); // Reset the timer for the next update

    u8g2.clearBuffer(); // Clear the internal memory

    // Line 1: Display Mode
    u8g2.setCursor(0, 12); 
    u8g2.print("Mode: ");
    if (currentMode == BREW_MODE) {
      u8g2.print("BREW");
    } else if (currentMode == STEAM_MODE) {
      u8g2.print("STEAM");
    }

    // Line 2: Display Current Actual Temperature
    u8g2.setCursor(0, 26);
    u8g2.print("Actual: ");
    u8g2.print(currentTemperature, 1); 
    u8g2.print((char)176); 
    u8g2.print("C");

    // Line 3: Display Setpoint Temperature
    u8g2.setCursor(0, 40);
    u8g2.print("Set:    "); 
    u8g2.print(activeSetTemperature, 1); 
    u8g2.print((char)176);
    u8g2.print("C");

    // Line 4: Display Shot Timer and Ready Status
    u8g2.setCursor(0, 54);
    u8g2.print("Timer: ");
    float durationInSeconds = currentShotDuration / 1000.0;
    u8g2.print(durationInSeconds, 1);
    u8g2.print("s");

    if (systemReady) {
      u8g2.setCursor(80, 54); 
      u8g2.print("READY");
    }
    
    u8g2.sendBuffer(); // Transfer internal memory to the display
  } 
}
