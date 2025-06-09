// Project BrewDev - FOSS Gaggia Controller
// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include <SPI.h>                 // Standard SPI library, often used by MAX31855
#include <Adafruit_MAX31855.h>   // For the MAX31855 TC sensor
#include <Adafruit_MAX31865.h>   // For PT100 RTD Sensor (Brew)       
#include <PID_v1.h>              // Brett Beauregard's PID library
#include <U8g2lib.h>             // For the OLED display by olikraus
#include <ESP32Encoder.h>        // For the ESP32 specific Rotary Encoder
#include <Preferences.h>         // For saving settings on ESP32
#include <Wire.h>                // For I2C communication (used by U8g2)


// ===================================
// --- Master Project Settings ---
#define DEBUG_MODE false // Set to 'true' to disable heater output, 'false' for normal operation

// --- Temp Boundaries for encoder ---
#define BREW_TEMP_MIN 85.0
#define BREW_TEMP_MAX 105.0
#define STEAM_TEMP_MIN 120.0
#define STEAM_TEMP_MAX 160.0
// ===================================



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
// 1. Create a pointer for our shared SPI bus object (VSPI)
SPIClass * vspi = new SPIClass(VSPI);

// 2. Declare the sensor objects, telling each one to use our new 'vspi' bus
Adafruit_MAX31855 thermocouple_steam(MAX31855_CS_PIN_STEAM, vspi);
Adafruit_MAX31865 sensor_pt100_brew(MAX31865_CS_PIN_BREW, vspi);

// --- OLED Display (U8g2) ---
// Constructor for 1.3" SH1106 I2C OLED. U8G2_R0 = No rotation.

U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// --- Rotary Encoder ---
ESP32Encoder encoder;
long oldEncoderValue = 0; // We'll use this to track changes

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

enum TimerState { STOPPED, RUNNING, FINISHED };
TimerState shotTimerState = STOPPED;
unsigned long shotStartTime = 0;
unsigned long currentShotDuration = 0;
unsigned long shotFinishTime = 0;
const unsigned long FLASH_DURATION = 5000; // 5 seconds

// --- Ready LED ---

bool systemReady = false;
int lastButtonState = HIGH;

static unsigned char logo_bitmap[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x1f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xfe, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe6,
   0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x61, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x06, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
   0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0xc6, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x0c, 0xcc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
   0xcc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xd8, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x30, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
   0xf0, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf0, 0x7c, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xc0, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x03, 0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x03, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x02, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x18, 0x0c, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x0c, 0x00,
   0x00, 0x00, 0x80, 0x7f, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0xf0, 0x01,
   0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x80, 0x03, 0x00,
   0x00, 0x00, 0x0e, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00,
   0x00, 0x60, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x20, 0x00, 0x00,
   0x00, 0xc0, 0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00,
   0x00, 0x20, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,
   0x00, 0x30, 0x00, 0x03, 0x00, 0x30, 0x00, 0x00, 0x00, 0x30, 0x00, 0x0f,
   0x00, 0x30, 0x00, 0x00, 0x00, 0x10, 0x00, 0x38, 0x00, 0x30, 0x00, 0x00,
   0x00, 0x18, 0x00, 0x70, 0x00, 0x10, 0x00, 0x00, 0x00, 0x18, 0x00, 0xe0,
   0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0xc0, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x08, 0x00, 0x80, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x80,
   0x01, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x80, 0x01, 0x07, 0x00, 0x00,
   0x00, 0x0c, 0x00, 0x80, 0x01, 0x03, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x80,
   0x00, 0x03, 0x00, 0x00, 0x80, 0x0f, 0x00, 0xc0, 0x08, 0x03, 0x00, 0x00,
   0xc0, 0x0c, 0x00, 0xc0, 0x18, 0x02, 0x00, 0x00, 0xc0, 0x08, 0x00, 0x60,
   0x10, 0x06, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x30, 0x30, 0x06, 0x00, 0x00,
   0xc0, 0x00, 0x00, 0x1c, 0x30, 0x0c, 0x00, 0x00, 0x80, 0x01, 0x00, 0xfe,
   0x21, 0x3c, 0x00, 0x00, 0x00, 0x03, 0x00, 0x80, 0x63, 0x70, 0x00, 0x00,
   0x00, 0x7e, 0x00, 0x00, 0x63, 0xc0, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00,
   0xc6, 0xc0, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0x83, 0x7f, 0x00, 0x00,
   0x00, 0x00, 0xfe, 0xff, 0x01, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/////////////////
///// setup /////
/////////////////

void setup() {
  // Start Serial communication for debugging (optional, but very helpful)
  Serial.begin(115200);
  Serial.println("\n--- Setup Checkpoint Test ---");
  pinMode(ROTARY_ENCODER_BUTTON_PIN, INPUT_PULLUP);
  unsigned long setupStartTime = millis();
  while (!Serial && (millis() - setupStartTime < 2000)) { // Wait a bit for serial to connect, but don't hang forever
    delay(100);
  }
  Serial.println("\n\nProject BrewDev Starting Up...");

  // --- Initialize Preferences (Non-Volatile Storage) ---
  preferences.begin("brewdev", false); // "brewdev" namespace, false for R/W mode
//preferences.clear();
  // Load saved setpoint temperatures (or use defaults if not found)
  brewSetTemperature = preferences.getDouble(prefs_brew_temp_key, 92.0); // Default 92.0°C
  steamSetTemperature = preferences.getDouble(prefs_steam_temp_key, 140.0); // Default 140.0°C
  Serial.print("Loaded Brew Set Temperature: "); Serial.println(brewSetTemperature);
  Serial.print("Loaded Steam Set Temperature: "); Serial.println(steamSetTemperature);
  
  // Set the initial active setpoint (e.g., to brew temperature, assuming it starts in a non-steam mode)
  //
  activeSetTemperature = brewSetTemperature; 
  currentMode = BREW_MODE; // Start in BREW, can switch based on Gaggia switches later
  Serial.println("Checkpoint 1: Preferences OK");
  // --- Initialize Sensors ---
  // PT100 for Brew Temperature
  // 1. Initialize the shared VSPI bus with your defined pins
  vspi->begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, -1); // <<< ADD THIS LINE
  Serial.println("Checkpoint 2: SPI Bus OK");
 
  // 2. PT100 for Brew Temperature
  Serial.println("Initializing PT100 Brew Sensor (MAX31865)...");
  // This line is now modified to be cleaner, passing the wire type directly.
  if (!sensor_pt100_brew.begin(MAX31865_3WIRE)) { // <<< MODIFIED LINE
    Serial.println("ERROR: Could not initialize PT100/MAX31865 for brew! Check wiring.");
  } else {
    Serial.println("PT100 Brew Sensor (MAX31865) initialized.");
  }
  delay(250);
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
  Serial.println("Checkpoint 3: PT100 Sensor OK");

  // K-Type for Steam Temperature
 Serial.println("Initializing K-Type Steam Thermocouple (MAX31855)...");
  // This is now simplified, as the if() check was unreliable.
  thermocouple_steam.begin(); // <<< MODIFIED LINE

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
  Serial.println("Checkpoint 4: K-Type Sensor OK");

  // --- Initialize OLED Display (U8g2) ---

  Serial.println("Initializing I2C Bus for OLED...");
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN); // Explicitly initialize I2C with defined pins
  Serial.println("Checkpoint 5: I2C Bus OK");
  Serial.println("Initializing OLED Display...");
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  Serial.println("Checkpoint 6: U8g2 Library OK");
// Call our new function to draw the logo
  drawSplashScreen(); 
// Keep the splash screen visible for 1.5 seconds
  delay(1500);
  Serial.println("Checkpoint 7: Splash Screen OK");

  // --- Initialize Rotary Encoder ---
Serial.println("Initializing PCNT Encoder...");
// Attach the new safe pins for rotation. Note the order: DT pin, CLK pin
encoder.attachHalfQuad(ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_A_PIN);

// Set the starting count to match the loaded temperature
  encoder.setCount(activeSetTemperature * 2);
  oldEncoderValue = encoder.getCount();
  Serial.println("Checkpoint 8: Encoder OK");
  // --- Initialize PID Controller ---

  Serial.println("Initializing PID Controller...");
  // Set PID sample time (how often it calculates). 1000ms = 1 second.
  brewPid.SetSampleTime(1000);
  brewPid.SetOutputLimits(0, pidWindowSize);
  brewPid.SetTunings(Kp_brew, Ki_brew, Kd_brew);
  brewPid.SetMode(AUTOMATIC); // Turn the PID on
  pidWindowStartTime = millis(); // Initialize PID window start time
  Serial.println("Checkpoint 9: PID Controller OK");

  // --- Set Pin Modes ---

  Serial.println("Setting Pin Modes...");
  pinMode(SSR_CONTROL_PIN, OUTPUT);
  digitalWrite(SSR_CONTROL_PIN, LOW); // Ensure SSR is off initially

  // Using INPUT_PULLUP assumes the switches connect the pin to GND when active (active LOW)

  pinMode(GAGGIA_BREW_SWITCH_PIN, INPUT_PULLUP);  
  pinMode(GAGGIA_STEAM_SWITCH_PIN, INPUT_PULLUP); 

  pinMode(READY_LED_PIN, OUTPUT);
  
  digitalWrite(READY_LED_PIN, LOW); // Ensure LED is off initially
  Serial.println("Checkpoint 10: Pin Modes OK");

  Serial.println("Setup Complete. Project BrewDev is Ready!");
  u8g2.clearBuffer(); 
  u8g2.setCursor(0,12);
  u8g2.print("Ready!");
  u8g2.setCursor(0,26);
  u8g2.print("Temp: "); u8g2.print(currentTemperature, 1); u8g2.print("C"); 

  // Display initial temp
  u8g2.sendBuffer();

  // CHECKPOINT pass
  Serial.println("--- Setup Function Complete ---");
}

//////////////////////////////
// loop() - Runs repeatedly //
//////////////////////////////

 void loop() {
  // --- 1- Read Gaggia Switch Inputs & Determine Mode ---
  updateOperationMode();
  Serial.println("Loop CP1: updateOperationMode OK");
  // --- 2- Read Current Temperatures from Both Sensors ---
  readSensorTemperatures();
  Serial.println("Loop CP2: readSensorTemperatures OK");
  // --- 3- Helper for all PID related updates
  processPidLogic();
  Serial.println("Loop CP3: processPidLogic OK");
  // --- 4- SSR control
  controlSSR();
  Serial.println("Loop CP4: controlSSR OK");
  // --- 5 - Encoder logic
  handleEncoderInput();
  Serial.println("Loop CP5: handleEncoderInput OK");
  // --- 6 - Manage Shot Timer
  manageShotTimer();
  Serial.println("Loop CP6: manageShotTimer OK");
  // --- 7 - Update "Ready" LED
  updateReadyLedState();
  Serial.println("Loop CP7: updateReadyLedState OK");
  // --- 8 - Update OLED Display
  refreshOledDisplay();
  Serial.println("Loop CP8: refreshOledDisplay OK");
  // --- 9 - Log PID data for Tuning
  logPidData();
  Serial.println("Loop CP9: PIDLogACTIVE");
 
  delay(150);
 }

//----------------------------
  // --- Helper Functions ---
//----------------------------

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

    // pidOutput is the ON-time in ms for the current window...
    if (pidOutput > (now - pidWindowStartTime)) {
      // Only turn the SSR ON if DEBUG_MODE is set to false
      if (!DEBUG_MODE) {
        digitalWrite(SSR_CONTROL_PIN, HIGH); // Turn SSR ON
      }
    } else {
      // This 'else' block should ideally not be reached if currentMode is always BREW or STEAM.
    // However, as a failsafe:
    digitalWrite(SSR_CONTROL_PIN, LOW); // Ensure SSR is OFF
    // Serial.println("SSR OFF - Failsafe (Unexpected Mode)"); // Debug
    }
  }
}

/////////////////////
// 5.Encoder logic //
/////////////////////

void handleEncoderInput() {
  // Read the raw hardware count from the encoder (which represents half-degrees)
  long newEncoderCount = encoder.getCount();

  if (newEncoderCount != oldEncoderValue) {
    // Convert the raw count into our temperature value by dividing by 2.0
    double newTemperature = newEncoderCount / 2.0;

    if (currentMode == BREW_MODE) {
      // Constrain the new value within our defined brew boundaries
      brewSetTemperature = constrain(newTemperature, BREW_TEMP_MIN, BREW_TEMP_MAX);
      activeSetTemperature = brewSetTemperature;
    } else { // STEAM_MODE
      // Constrain the new value within our defined steam boundaries
      steamSetTemperature = constrain(newTemperature, STEAM_TEMP_MIN, STEAM_TEMP_MAX);
      activeSetTemperature = steamSetTemperature;
    }

    // IMPORTANT: Re-sync the raw encoder count with the (potentially constrained) value
    // This stops the counter from running away if you hit a boundary.
    encoder.setCount(activeSetTemperature * 2);
    oldEncoderValue = encoder.getCount(); 
  }

  // --- Logic for button click (with robust debouncing) ---
  int currentButtonState = digitalRead(ROTARY_ENCODER_BUTTON_PIN);
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    Serial.println("Encoder button pressed! Saving settings...");
    preferences.putDouble(prefs_brew_temp_key, brewSetTemperature);
    preferences.putDouble(prefs_steam_temp_key, steamSetTemperature);

    u8g2.setCursor(80, 54); 
    u8g2.print("Saved!");
    u8g2.sendBuffer();
    delay(1000); 
    delay(50); 
  }
  lastButtonState = currentButtonState;
}


//////////////////////////
// 6. Manage Shot Timer //
//////////////////////////

void manageShotTimer() {
  // Check if the brew switch is ON
  if (brewSwitchState) {
    // If the timer was stopped, start it now
    if (shotTimerState == STOPPED) {
      shotTimerState = RUNNING;
      shotStartTime = millis();
      currentShotDuration = 0;
      Serial.println("Shot Timer Started.");
    }
    // If it's running, update the duration
    if (shotTimerState == RUNNING) {
      currentShotDuration = millis() - shotStartTime;
    }
  } else {
    // If the brew switch is OFF...
    // ...and the timer was running, switch it to the FINISHED state
    if (shotTimerState == RUNNING) {
      shotTimerState = FINISHED;
      shotFinishTime = millis(); // Record the time we finished
      Serial.print("Shot Timer Finished. Final Duration: ");
      Serial.println(currentShotDuration / 1000.0, 1);
    }
  }

  // If the timer is in the FINISHED state, check if 5 seconds have passed
  if (shotTimerState == FINISHED && millis() - shotFinishTime > FLASH_DURATION) {
    // After 5 seconds, stop it completely and reset
    shotTimerState = STOPPED;
    currentShotDuration = 0;
    Serial.println("Finished display timeout. Timer reset.");
  }
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
  static unsigned long lastDisplayUpdateTime = 0;
  if (millis() - lastDisplayUpdateTime > 50) {
    lastDisplayUpdateTime = millis();

    char buffer[20]; 

    u8g2.firstPage();
    do {
      // --- STATUS BAR (TOP ROW) ---
      u8g2.setFont(u8g2_font_unifont_t_symbols);
      if (DEBUG_MODE) { u8g2.drawGlyph(0, 12, 0x2699); } 
      else {
        if (currentMode == BREW_MODE) { u8g2.drawGlyph(0, 12, 0x2615); } 
        else { u8g2.drawGlyph(0, 12, 0x2601); }
      }
      u8g2.setFont(u8g2_font_prospero_bold_nbp_tr); 
      const char *title = "BrewDev";
      int textWidth = u8g2.getStrWidth(title);
      int textX = (128 - textWidth) / 2;
      u8g2.drawBox(textX - 4, 1, textWidth + 8, 12);
      u8g2.setDrawColor(0);
      u8g2.drawStr(textX, 11, title);
      u8g2.setDrawColor(1);
      u8g2.setFont(u8g2_font_ncenB08_tr); 
      u8g2.drawHLine(0, 14, 128);

      // --- MAIN TEMPERATURE DISPLAY (MIDDLE) ---
      // Switched to a slightly smaller, 24-pixel font to make more room
      u8g2.setFont(u8g2_font_logisoso24_tr); 
      // Use a wider buffer for safety with negative numbers
      dtostrf(currentTemperature, 6, 1, buffer);
      textWidth = u8g2.getStrWidth(buffer);
      int numberX = (128 - textWidth) / 2;
      // Adjusted y-position to re-center the smaller font
      u8g2.drawStr(numberX, 48, buffer);

      // Draw the Celsius symbol
      u8g2.setFont(u8g2_font_ncenB08_tr);
      // Adjusted y-position to align with the new font size
      u8g2.setCursor(numberX + textWidth, 38); 
      u8g2.print((char)176); // Degree symbol °
      u8g2.print("C");

      // --- BOTTOM ROW INFO ---
      u8g2.setCursor(0, 64);
      u8g2.print("Set: ");
      u8g2.print(activeSetTemperature, 1);
      
      // --- CONTEXT-SENSITIVE STATUS ON THE RIGHT ---
      if (shotTimerState == RUNNING) {
        sprintf(buffer, "T: %.1fs", (float)(currentShotDuration / 1000.0));
        u8g2.drawStr(74, 64, buffer);
      } else if (shotTimerState == FINISHED) {
        if ((millis() / 500) % 2 == 0) {
          sprintf(buffer, "T: %.1fs", (float)(currentShotDuration / 1000.0));
          u8g2.drawStr(74, 64, buffer);
        }
      } else if (systemReady) {
        u8g2.drawStr(74, 64, "Ready!");
      } else {
        // Segmented Power Meter
        u8g2.setFont(u8g2_font_unifont_t_symbols);
        u8g2.drawGlyph(74, 64, 0x1F525); // Flame icon 🔥
        u8g2.setFont(u8g2_font_ncenB08_tr);
        int segments = map(pidOutput, 0, pidWindowSize, 0, 5);
        int segment_width = 6;
        int segment_height = 8;
        int segment_spacing = 1;
        int start_x = 90;
        int start_y = 56;
        for (int i = 0; i < 5; i++) {
          u8g2.drawFrame(start_x + (i * (segment_width + segment_spacing)), start_y, segment_width, segment_height);
        }
        if (segments > 0) {
          for (int i = 0; i < segments; i++) {
            u8g2.drawBox(start_x + (i * (segment_width + segment_spacing)), start_y, segment_width, segment_height);
          }
        }
      }
      
    } while (u8g2.nextPage());
  }
}


//////////////////////
// 9. Splash Screen //
//////////////////////

void drawSplashScreen() {
  u8g2.firstPage();
  do {
    // All drawing commands MUST go inside this do...while loop for Full Frame Buffer mode.
    u8g2.drawXBM( (128 - 60) / 2, (64 - 60) / 2, 60, 60, logo_bitmap);
  } while (u8g2.nextPage());
}



//////////////////////
// 10. Log PID data for tuning //
//////////////////////

void logPidData() {
  // This function prints the key variables in a comma-separated format
  // that can be easily graphed by the Arduino Serial Plotter.
  
  Serial.print(activeSetTemperature);
  Serial.print(",");
  Serial.print(currentTemperature);
  Serial.print(",");
  
  // We scale the pidOutput (0-5000) to be on the same rough scale as the temperature
  // for easier graphing. This is for display only.
  double scaledOutput = map(pidOutput, 0, pidWindowSize, 0, 150);
  Serial.println(scaledOutput);
}
