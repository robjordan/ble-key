/**
 * Mediation device between Lekato Wireless Page Turner and Tablets that 
 * lack the hid-multitouch input device.
 */
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <BleKeyboard.h>

const unsigned DEBOUNCE_MS = 5;
const unsigned MODESWITCH_MS = 1550;
const unsigned LKEY = 17;
const unsigned RKEY = 16;
const unsigned DUTY_CYCLE = 8;
// setting PWM properties
const int freq = 5000;
const int resolution = 8;

BleKeyboard bleKeyboard("ESP32_P_Turner");

typedef enum {landscape = 0, portrait = 1} lp_mode_t;
lp_mode_t lpMode = portrait;
const int led[] = {
  25, // landscape
  33  // portrait
};
const char *name[] = {
  "landscape",
  "portrait"
};
volatile bool leftState = HIGH, rightState = HIGH;
volatile unsigned long leftDebounceTime = ULONG_MAX; 
volatile unsigned long rightDebounceTime = ULONG_MAX; 
unsigned long modeSwitchTime = ULONG_MAX;

void connect_lekato(void);

// Interrupt handlers
void IRAM_ATTR left(){
  // Serial.println("L");
  leftDebounceTime = millis() + DEBOUNCE_MS;
}

void IRAM_ATTR right(){
  // Serial.println("Rf");
  rightDebounceTime = millis() + DEBOUNCE_MS;
}

void switch_mode(lp_mode_t mode) {
    // digitalWrite(led[mode], HIGH);
    // digitalWrite(led[!mode], LOW);
    ledcWrite(mode, DUTY_CYCLE);
    ledcWrite(!mode, 0);
    lpMode = mode;
    Serial.printf("Going to %s mode.\n", name[mode]);
}


void setup() {
  setCpuFrequencyMhz(160); // save 20mA
  Serial.begin(115200);
  Serial.println("Lekato mediation device.");
  pinMode(LKEY, INPUT);
  pinMode(RKEY, INPUT);
  // pinMode(led[portrait], OUTPUT);
  // pinMode(led[landscape], OUTPUT);

  // configure LED PWM functionalitites
  ledcSetup(portrait, freq, resolution);
  ledcSetup(landscape, freq, resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(led[landscape], landscape);
  ledcAttachPin(led[portrait], portrait);

  switch_mode(portrait);
  attachInterrupt(digitalPinToInterrupt(LKEY), left, FALLING);
  attachInterrupt(digitalPinToInterrupt(RKEY), right, FALLING);
  // connect_lekato();
  bleKeyboard.begin();
}

void loop() {

  if (millis() > leftDebounceTime) {
    if (leftState == HIGH) {
      // The left button has been depressed for more than debounce time
      leftState = LOW;
      leftDebounceTime = ULONG_MAX;
      modeSwitchTime = millis() + MODESWITCH_MS;
      attachInterrupt(digitalPinToInterrupt(LKEY), left, RISING); 
      if (lpMode == portrait) {
        bleKeyboard.write(KEY_LEFT_ARROW);
        Serial.println("Sending Left Arrow.");
      } else {
        bleKeyboard.write(KEY_UP_ARROW);
        Serial.println("Sending Up Arrow.");   
      }
   
    } else {
      // The left button has been released for more than debounce time
      leftState = HIGH;
      leftDebounceTime = ULONG_MAX;
      modeSwitchTime = ULONG_MAX;
      attachInterrupt(digitalPinToInterrupt(LKEY), left, FALLING);    
    }
  }

  if (millis() > rightDebounceTime) {
    if (rightState == HIGH) {
      // The right button has been depressed for more than debounce time
      rightState = LOW;
      rightDebounceTime = ULONG_MAX;
      modeSwitchTime = millis() + MODESWITCH_MS;
      attachInterrupt(digitalPinToInterrupt(RKEY), right, RISING);    
      if (lpMode == portrait) {
        bleKeyboard.write(KEY_RIGHT_ARROW);
        Serial.println("Sending Right Arrow.");
      } else {
        bleKeyboard.write(KEY_DOWN_ARROW);
        Serial.println("Sending Down Arrow.");   
      }   
    } else {
      // The right button has been released for more than debounce time
      rightState = HIGH;
      rightDebounceTime = ULONG_MAX;
      modeSwitchTime = ULONG_MAX;
      attachInterrupt(digitalPinToInterrupt(RKEY), right, FALLING);         
    }
  } 

  if (millis() > modeSwitchTime) {
    modeSwitchTime = ULONG_MAX;
    if (lpMode == portrait && rightState == LOW) {
      switch_mode(landscape);
    } else if (lpMode == landscape && leftState == LOW) {
      switch_mode(portrait);
    } else {
      Serial.println("Mode switch error.");
    }
  }

  // TESTING: For checking cycle time on oscilloscope
  // digitalWrite(LED, HIGH);
  // digitalWrite(LED, LOW);
  
  delay(1);
}


// void connect_lekato () {
//     // Connect with it just to stop it pairing with other devices and stop the flashing
//     NimBLEDevice::init("");
//     // Only Caroline's Lekato
//     NimBLEDevice::whiteListAdd(NimBLEAddress("01:5b:76:3b:46:a3")); 
//     NimBLEScan *pScan = NimBLEDevice::getScan();
//     Serial.println("Scanning, wait 5 seconds.");
//     NimBLEScanResults results = pScan->start(5);
//     NimBLEUUID serviceUuid("1812");
//     Serial.printf("Scan found %d devices.\n", results.getCount());
    
  
//     for(int i = 0; i < results.getCount(); i++) {
//         Serial.println("Device:");
//         NimBLEAdvertisedDevice device = results.getDevice(i);
//         for (int sd = 0; sd < device.getServiceUUIDCount(); sd++) {
//           Serial.println(device.getServiceUUID(sd).toString().c_str());
//         }
//         if (device.isAdvertisingService(serviceUuid)) {        
//             NimBLEClient *pClient = NimBLEDevice::createClient();
//             Serial.println("Client created.");
            
//             if (pClient->connect(&device)) {
//                 Serial.println("Connected device.");
//                 NimBLERemoteService *pService = pClient->getService(serviceUuid);
                
//                 if (pService != nullptr) {
//                     Serial.println("Got service.");
//                     NimBLERemoteCharacteristic *pCharacteristic = pService->getCharacteristic("2a4c");
                    
//                     if (pCharacteristic != nullptr) {
//                         Serial.println("Got characteristic.");
//                         std::string value = pCharacteristic->readValue();
//                         Serial.println("Connected to Lekato.");
//                     }
//                 }

//             } else {
//                 Serial.println("Failed to connect to Lekato.");
//             }
            
//             NimBLEDevice::deleteClient(pClient);
//         }
//     }
// }