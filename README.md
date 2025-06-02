# ProjectBrewDev: FOSS Gaggia Classic Pro PID Controller
**⚠️ IMPORTANT DISCLAIMER: WORK IN PROGRESS - DO NOT USE! ⚠️**  

**This project is currently under active development and is NOT yet stable, complete, or tested for safety and reliability. The code and hardware designs shared here are experimental and intended for development and learning purposes only at this stage.**  

**DO NOT attempt to install or use this system on your Gaggia Classic Pro or any other espresso machine in its current state. Doing so could result in damage to your machine, create an electrical hazard, or lead to unpredictable behavior.**  

Project BrewDev is a DIY, open-source PID temperature controller and enhancement system specifically designed for the Gaggia Classic Pro espresso machine. It utilizes an ESP32 microcontroller to provide precise temperature control and additional features for an improved brewing experience.

## Project Goal
This project aims to design and build a custom, open-source (FOSS) PID controller and enhancement system specifically for the Gaggia Classic Pro espresso machine. [cite: 1] The primary motivations are:

* **Enhanced Control & Consistency:** To achieve precise, stable, and user-configurable PID temperature control for both brewing and steam functions, leading to more consistent and repeatable espresso shots.
* **Learning & Customization:** To foster a deeper understanding of the machine's operation and provide a platform for custom modifications and feature additions beyond what off-the-shelf solutions might offer. This project is built with a DIY spirit, valuing hands-on learning and tailored solutions.
* **Integrated User Experience:** To move beyond basic PID functionality by incorporating a user-friendly interface with an OLED display and rotary encoder for real-time feedback and easy adjustments, along with practical features like a shot timer.
* **Open Source Contribution:** To develop and share a well-documented, FOSS solution (hardware designs, software, and documentation) on GitHub, making it accessible for other Gaggia enthusiasts to build, adapt, and contribute to.

## Current Status

Project BrewDev is currently in the V1 development phase. The initial software for V1 has been drafted and compiles, component selection is complete, and the wiring/hardware assembly phase is commencing. This README and other documentation are being actively developed alongside the build.


## Key Features (V1 Scope)

The initial V1 release of Project BrewDev aims to implement the following core features:

* PID control for brew temperature
* PID control for steam temperature (triggered by Gaggia's physical switch)
* An I2C OLED Display (1.3") to show current mode, current temp, desired temp, and a shot timer.
* A Rotary Encoder (KY-040 module) for user input
    * Adjusting brew and steam setpoint temperatures.
* A Shot Timer function, triggered by the Gaggia's physical brew switch, which resets when the brew switch is turned off.
* A "Ready" LED indicator to visually signal when the machine has reached the desired temperature.



Project BrewDev: V1 Component Checklist
---------------------------------------
#### I. Core Platform & Development:

{ } [ESP32-WROOM-32D Development Board](https://www.aliexpress.com/item/1005006456519790.html?spm=a2g0o.productlist.main.3.15bcAfdYAfdYRM&algo_pvid=ffcd6df9-9ebe-4001-b6d5-fae6cf6b98cf&algo_exp_id=ffcd6df9-9ebe-4001-b6d5-fae6cf6b98cf-2&pdp_ext_f=%7B%22order%22%3A%2210915%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%2119.78%216.41%21%21%2189.97%2129.13%21%40210330dd17487764894447939ecfaa%2112000037265317361%21sea%21AU%213080408886%21X&curPageLogUid=XJKHBT6ymrRs&utparam-url=scene%3Asearch%7Cquery_from%3A)  

{ } [ESP32 38Pin Screw Terminal Board (for easier wiring)](https://www.aliexpress.com/item/1005006026098254.html?spm=a2g0o.productlist.main.1.4eaa7afarTEsGo&algo_pvid=c3cbc9bc-7ff6-4c7d-86fa-5924999aca85&algo_exp_id=c3cbc9bc-7ff6-4c7d-86fa-5924999aca85-0&pdp_ext_f=%7B%22order%22%3A%22218%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%215.98%215.70%21%21%213.79%213.61%21%40210308a417487812027447139ec55e%2112000035378878500%21sea%21AU%213080408886%21X&curPageLogUid=6cTqvdge5t59&utparam-url=scene%3Asearch%7Cquery_from%3A)  


#### II. Sensors & Amplifiers (for V1 PID Control):

##### For Brew Temperature:  

{ } [M4 Screw PT100 Thermocouple Sensor Probe](https://www.aliexpress.com/item/1005008644082234.html?spm=a2g0o.productlist.main.4.27e86a08jmZQVz&algo_pvid=5cb3f48c-9396-4061-a75e-d4601745c35e&algo_exp_id=5cb3f48c-9396-4061-a75e-d4601745c35e-3&pdp_ext_f=%7B%22order%22%3A%2210%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%2110.17%217.63%21%21%216.44%214.83%21%40210308a417487820902015341ec55e%2112000046072476035%21sea%21AU%213080408886%21X&curPageLogUid=nRPo32LtRwza&utparam-url=scene%3Asearch%7Cquery_from%3A)(Note:ensure you Select PT100 M4 sensor)  

{ } [MAX31865 Platinum Resistance Temperature Sensor Module](https://www.aliexpress.com/item/1005006996558022.html?spm=a2g0o.productlist.main.3.47517ef76AR2bT&algo_pvid=e23983b9-4432-45e0-8389-facf78d8f1b5&algo_exp_id=e23983b9-4432-45e0-8389-facf78d8f1b5-2&pdp_ext_f=%7B%22order%22%3A%22211%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%214.47%214.34%21%21%2120.35%2119.75%21%402101ead817487771176568460ef032%2112000038988167857%21sea%21AU%213080408886%21X&curPageLogUid=QaGzaiM77Khy&utparam-url=scene%3Asearch%7Cquery_from%3A)  


##### For Steam Temperature:  


{ } [K-Type Thermocouple Probe (ensure you have one compatible with the MAX31855)](https://www.aliexpress.com/item/1005005496786289.html?spm=a2g0o.productlist.main.1.7dd9236fusXsxI&algo_pvid=d2b3cf6a-636d-4870-9a61-269e426183fa&algo_exp_id=d2b3cf6a-636d-4870-9a61-269e426183fa-0&pdp_ext_f=%7B%22order%22%3A%22906%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%213.27%212.91%21%21%212.07%211.84%21%402103245417487774616223073e1561%2112000033327476707%21sea%21AU%213080408886%21X&curPageLogUid=iCr16ZWSsp3i&utparam-url=scene%3Asearch%7Cquery_from%3A)(Note:Ensure you pick k-type m4)

{ } [MAX31855 Thermocouple Sensor Module](https://www.aliexpress.com/item/1902975189.html?spm=a2g0o.productlist.main.1.428e287cNdxcZy&algo_pvid=3b100b3c-6b06-4296-a7df-7b496b6c2fbe&algo_exp_id=3b100b3c-6b06-4296-a7df-7b496b6c2fbe-0&pdp_ext_f=%7B%22order%22%3A%2270%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%212.86%212.86%21%21%211.81%211.81%21%402103244b17487820073372535e03d5%2112000018875273580%21sea%21AU%213080408886%21X&curPageLogUid=gbY9soG79bIv&utparam-url=scene%3Asearch%7Cquery_from%3A)  


#### III. User Interface:  


{ } [DIYUSER 1.3" IIC OLED Display Module (White/BLUE, SH1106 based)](https://www.aliexpress.com/item/1005007451015054.html?spm=a2g0o.productlist.main.3.29d477e2vhVh6h&algo_pvid=a85804ac-08e5-427a-aa2c-28ba35e142ab&algo_exp_id=a85804ac-08e5-427a-aa2c-28ba35e142ab-2&pdp_ext_f=%7B%22order%22%3A%221277%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%215.72%215.40%21%21%213.62%213.42%21%402103244b17487776655308597e044a%2112000040806152742%21sea%21AU%213080408886%21X&curPageLogUid=4yPN9mkLGFx2&utparam-url=scene%3Asearch%7Cquery_from%3A)  


{ } [KY-040 Rotary Encoder Module](https://www.aliexpress.com/item/1005006551162496.html?spm=a2g0o.productlist.main.1.64372834Vi9sS7&algo_pvid=134749c8-49cf-431b-a412-205056a24d29&algo_exp_id=134749c8-49cf-431b-a412-205056a24d29-0&pdp_ext_f=%7B%22order%22%3A%222249%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%218.90%212.81%21%21%2140.51%2112.82%21%402151e6dc17487779261336681eee87%2112000037644212083%21sea%21AU%213080408886%21X&curPageLogUid=EkH5oimkA3ZJ&utparam-url=scene%3Asearch%7Cquery_from%3A)  

{ } [Standard LED (e.g., 3mm or 5mm, any color for "Ready" indicator, can be mounted on faceplate with panel mount adaptor)](https://www.aliexpress.com/item/1005007591932915.html?spm=a2g0o.productlist.main.7.224e5a83uSY5uC&algo_pvid=f579ac9b-b04b-4d63-8645-c1dc84de87be&algo_exp_id=f579ac9b-b04b-4d63-8645-c1dc84de87be-6&pdp_ext_f=%7B%22order%22%3A%2239%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%2116.32%215.39%21%21%2174.24%2124.50%21%40212a6e3217487781761867293e459c%2112000041422307129%21sea%21AU%213080408886%21X&curPageLogUid=vtBytDIiIAth&utparam-url=scene%3Asearch%7Cquery_from%3A)  

{ } [Current Limiting Resistor for LED (e.g., 220Ω - 330Ω for 3.3V operation multi pack is cheap on aliexpress)](https://www.aliexpress.com/item/1005006209050774.html?src=google&albch=shopping&acnt=272-267-0231&slnk=&plac=&mtctp=&albbt=Google_7_shopping&gclsrc=aw.ds&albagn=888888&ds_e_adid=738012934484&ds_e_matchtype=search&ds_e_device=c&ds_e_network=g&ds_e_product_group_id=2403836566736&ds_e_product_id=en1005006209050774&ds_e_product_merchant_id=5086892413&ds_e_product_country=AU&ds_e_product_language=en&ds_e_product_channel=online&ds_e_product_store_id=&ds_url_v=2&albcp=22318332228&albag=177676186793&isSmbAutoCall=false&needSmbHouyi=false&gad_source=1&gad_campaignid=22318332228&gbraid=0AAAAAoukdWM6zllmMoPaQTHTTIoOrFtQm&gclid=Cj0KCQjw9O_BBhCUARIsAHQMjS4QOitYwIrYsQKYPh3EqaiO0prRmBSIkzV7G7uUrvxxCCEjVo4S0bIaAkZGEALw_wcB&aff_fcid=bdcb064c5439481fa9a21fb7bcd8d287-1748778614867-07419-UneMJZVf&aff_fsk=UneMJZVf&aff_platform=aaf&sk=UneMJZVf&aff_trace_key=bdcb064c5439481fa9a21fb7bcd8d287-1748778614867-07419-UneMJZVf&terminal_id=735104564fec4889aded04102bdc1e8b&afSmartRedirect=n)  


#### IV. Power Supply & Control:  


{ } [HLK-PM01 (5V 3W AC-DC Power Supply Module for ESP32 system)](https://www.aliexpress.com/item/1005006072424191.html?spm=a2g0o.productlist.main.4.5d461adc3hd6eq&algo_pvid=dc2a3b39-0f5c-4065-b766-0b1d8509b58a&algo_exp_id=dc2a3b39-0f5c-4065-b766-0b1d8509b58a-3&pdp_ext_f=%7B%22order%22%3A%2269%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%215.45%215.45%21%21%2124.81%2124.81%21%402103241117487787808572909ee1cc%2112000035600877318%21sea%21AU%213080408886%21X&curPageLogUid=CG40YkbqM723&utparam-url=scene%3Asearch%7Cquery_from%3A)
{ } [Solid State Relay (MAKE SURE TO GET DC-AC SSR-40A)](https://www.aliexpress.com/item/1005005837105164.html?spm=a2g0o.productlist.main.1.457f714bCzx2Ei&algo_pvid=ca326593-964b-4b6e-9b52-219a5dc736d2&algo_exp_id=ca326593-964b-4b6e-9b52-219a5dc736d2-0&pdp_ext_f=%7B%22order%22%3A%221473%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%215.19%215.19%21%21%213.29%213.29%21%40210312d517487790839296128ee120%2112000034526092498%21sea%21AU%213080408886%21X&curPageLogUid=wvfErnjh160s&utparam-url=scene%3Asearch%7Cquery_from%3A)

#### V. Interfacing Modules (for Gaggia Switches):

{ } [AC 220V Optocoupler Isolation Module (1 Channel, TTL Output) x 2](https://www.aliexpress.com/item/1005007458865867.html?spm=a2g0o.productlist.main.1.7cd8162fC5uAG2&algo_pvid=63d2b8c8-ce46-42ac-857e-32f904086e11&algo_exp_id=63d2b8c8-ce46-42ac-857e-32f904086e11-0&pdp_ext_f=%7B%22order%22%3A%22167%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%213.59%213.59%21%21%2116.33%2116.33%21%402103277f17487794090943784eb984%2112000040841794135%21sea%21AU%213080408886%21X&curPageLogUid=jF7iY50Ske8i&utparam-url=scene%3Asearch%7Cquery_from%3A#nav-specification)  


#### VI. Essential Prototyping & Connection:  


{ } [Dupont Jumper Wires (Mix kit)](https://www.aliexpress.com/item/1005003252824475.html?spm=a2g0o.productlist.main.5.63ddk4SHk4SHTM&algo_pvid=48048bc2-0faa-4727-8d68-ca8d407a8dd9&algo_exp_id=48048bc2-0faa-4727-8d68-ca8d407a8dd9-4&pdp_ext_f=%7B%22order%22%3A%221502%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%217.14%216.89%21%21%214.52%214.36%21%40210123bc17487826590682400e9326%2112000024867532534%21sea%21AU%213080408886%21X&curPageLogUid=raedyEZAP8Dg&utparam-url=scene%3Asearch%7Cquery_from%3A)  

{ } [Wires for mains connections (18AWG RED/BLACK 5M of each is plenty)](https://www.aliexpress.com/item/1005006566120439.html?spm=a2g0o.productlist.main.9.c5296f0fEFh9GV&algo_pvid=9f873115-046d-46ce-b744-743b7adb9b8e&algo_exp_id=9f873115-046d-46ce-b744-743b7adb9b8e-8&pdp_ext_f=%7B%22order%22%3A%224247%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%2112.87%2112.85%21%21%2158.57%2158.48%21%40210337bc17487798074353806ecb65%2112000037691464131%21sea%21AU%213080408886%21X&curPageLogUid=zzFj7TSVBcwy&utparam-url=scene%3Asearch%7Cquery_from%3A)  

{ } [Wires for low voltage DC connections (22AWG MULTIPLE COLORS they come in packs, get a 5M one to be safe)](https://www.aliexpress.com/item/1005008683131221.html?spm=a2g0o.productlist.main.25.3537d5e2txHsOF&algo_pvid=2f4e5808-74cc-43aa-b811-544b14ca8732&algo_exp_id=2f4e5808-74cc-43aa-b811-544b14ca8732-22&pdp_ext_f=%7B%22order%22%3A%22294%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%214.54%212.23%21%21%2120.64%2110.11%21%402103010b17487799899288909e16d3%2112000046261040423%21sea%21AU%213080408886%21X&curPageLogUid=g25GAs3gdbwT&utparam-url=scene%3Asearch%7Cquery_from%3A)  

{ } [6.3mm Spade connector (Female, Male)](https://www.aliexpress.com/item/1005002765359666.html?spm=a2g0o.productlist.main.8.50f73225R0rOpZ&algo_pvid=d8656ba3-762d-48c3-b9d9-fb7f6dadfae5&algo_exp_id=d8656ba3-762d-48c3-b9d9-fb7f6dadfae5-7&pdp_ext_f=%7B%22order%22%3A%221570%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%213.39%213.14%21%21%212.15%211.99%21%402103247917487797088492619ee480%2112000022078614610%21sea%21AU%213080408886%21X&curPageLogUid=78MVaeSWbwPC&utparam-url=scene%3Asearch%7Cquery_from%3A)  




#### VII. Recommended for V1 Completion (but not core electronics):  


{ } [Aluminum Project Box (approx but not smaller than 55x95x150mm)](https://www.aliexpress.com/item/1005007115490142.html?spm=a2g0o.productlist.main.4.163554beE1yCYm&algo_pvid=b009735f-8ff7-445f-8a34-52fb15dfbad4&algo_exp_id=b009735f-8ff7-445f-8a34-52fb15dfbad4-3&pdp_ext_f=%7B%22order%22%3A%22186%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%216.05%216.05%21%21%213.83%213.83%21%40212a70c117487815001093581e6b16%2112000039451210900%21sea%21AU%213080408886%21X&curPageLogUid=W5JS6SXYNcEz&utparam-url=scene%3Asearch%7Cquery_from%3A)  

{ } [Mounting hardware (M2/M2.5/M3 bolts, nuts, and standoffs for modules and ESP32 within enclosure)](https://www.aliexpress.com/item/1005007123615498.html?spm=a2g0o.productlist.main.41.59bd2d4d08cUx2&algo_pvid=369a9011-3c99-4da9-b83a-15d65f51f7d2&algo_exp_id=369a9011-3c99-4da9-b83a-15d65f51f7d2-38&pdp_ext_f=%7B%22order%22%3A%22180%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21AUD%2110.36%215.70%21%21%2147.15%2125.93%21%40210313e917487821581835152ed8d8%2112000039501725238%21sea%21AU%213080408886%21X&curPageLogUid=2gnKW9aMVtaJ&utparam-url=scene%3Asearch%7Cquery_from%3A)  

{ } [Plastic Enclosure box for internal Gaggia chassis (Aim for 80x70x30mm)](https://www.aliexpress.com/item/1005005448610944.html?src=google&albch=shopping&acnt=742-864-1166&slnk=&plac=&mtctp=&albbt=Google_7_shopping&gclsrc=aw.ds&albagn=888888&ds_e_adid=&ds_e_matchtype=&ds_e_device=c&ds_e_network=x&ds_e_product_group_id=&ds_e_product_id=en1005005448610944&ds_e_product_merchant_id=723481895&ds_e_product_country=AU&ds_e_product_language=en&ds_e_product_channel=online&ds_e_product_store_id=&ds_url_v=2&albcp=21819463808&albag=&isSmbAutoCall=false&needSmbHouyi=false&gad_source=1&gad_campaignid=21819486122&gbraid=0AAAAA99aYpc3XuyMWTCZ0Rmhx_6aHQLgD&gclid=Cj0KCQjw9O_BBhCUARIsAHQMjS5HM00lwcmN3n7eleJXT4_7lFQl2JaOf2czvwjFeyi5BPZcmoASr00aApMREALw_wcB&aff_fcid=0465bbc3554149b080f80e4a820be986-1748825094267-08274-UneMJZVf&aff_fsk=UneMJZVf&aff_platform=aaf&sk=UneMJZVf&aff_trace_key=0465bbc3554149b080f80e4a820be986-1748825094267-08274-UneMJZVf&terminal_id=735104564fec4889aded04102bdc1e8b&afSmartRedirect=n)  


