#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

#include <LBLE.h>
#include <LBLEPeriphral.h>

#include <Grove_LED_Bar.h>

//pins:
const int HX711_dout = 17; //mcu > HX711 dout pin
const int HX711_sck = 16; //mcu > HX711 sck pin

// HX711 define
//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;

// BLE define
// Define a simple GATT service with only 1 characteristic
LBLEService counterService("4e38e0c3-ab04-4c5d-b54a-852900379bb3");
LBLECharacteristicInt counterCharacteristic("4e38e0c4-ab04-4c5d-b54a-852900379bb3", LBLE_READ | LBLE_WRITE);

// LED define
int blink_status = 0;

// BTN define
int buttonState = 0;
int p_buttonState = 0;
const int buttonPin = 6;

// LED_BAR define
Grove_LED_Bar bar(5,4,0,LED_BAR_10);

// my para
float offset = 0.0;
int weight = 0;
int strength = 0;
int pineapple_id = 0;



void setup() {
  Serial.begin(9600); delay(1000);
  Serial.println();
  Serial.println("Starting...");

  LoadCell.begin();
  //LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  float calibrationValue; // calibration value (see example file "Calibration.ino")
  calibrationValue = 696.0; // uncomment this if you want to set the calibration value in the sketch
#if defined(ESP8266)|| defined(ESP32)
  //EEPROM.begin(512); // uncomment this if you use ESP8266/ESP32 and want to fetch the calibration value from eeprom
#endif
  //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom

  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }

  float tmp = 0;
  int count = 0;
  while(count < 10)
  {
      static boolean newDataReady = 0;
      const int serialPrintInterval = 0; //increase value to slow down serial print activity
    
      // check for new data/start next conversion:
      if (LoadCell.update()){
        newDataReady = true;
      }
    
      // get smoothed value from the dataset:
      if (newDataReady) {
        if (millis() > t + serialPrintInterval) {
          float i = LoadCell.getData();
          Serial.print("Load_cell output val: ");
          Serial.println(i);
          tmp += i;count++;
          Serial.println(count);
          newDataReady = 0;
          t = millis();
        }
      }
      // receive command from serial terminal, send 't' to initiate tare operation:
      if (Serial.available() > 0) {
        char inByte = Serial.read();
        if (inByte == 't') LoadCell.tareNoDelay();
      }
    
      // check if last tare operation is complete:
      if (LoadCell.getTareStatus() == true) {
        Serial.println("Tare complete");
      }
  }
  Serial.print("count: ");
  Serial.println(count);
  offset = tmp/10;
  Serial.print("offset: ");
  Serial.println(offset);

  // BLE setup
  // ================================================================
  // Initialize LED pin
  pinMode(LED_BUILTIN, OUTPUT);
  blink_status = 0;
  digitalWrite(LED_BUILTIN, blink_status);

  //Initialize serial and wait for port to open:
  // Serial.begin(9600);

  // Initialize BLE subsystem
  LBLE.begin();
  while (!LBLE.ready()) {
    delay(100);
  }
  Serial.println("BLE ready");

  Serial.print("Device Address = [");
  Serial.print(LBLE.getDeviceAddress());
  Serial.println("]");

  // configure our advertisement data.
  // In this case, we simply create an advertisement that represents an
  // connectable device with a device name
  LBLEAdvertisementData advertisement;
  advertisement.configAsConnectableDevice("GATT");

  // Configure our device's Generic Access Profile's device name
  // Ususally this is the same as the name in the advertisement data.
  LBLEPeripheral.setName("GATT Test");

  // Add characteristics into counterService
  counterService.addAttribute(counterCharacteristic);

  // Add service to GATT server (peripheral)
  LBLEPeripheral.addService(counterService);

  // start the GATT server - it is now 
  // available to connect
  LBLEPeripheral.begin();

  // start advertisment
  LBLEPeripheral.advertise(advertisement);

  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  bar.begin();
}


void loop() {
  // delay(1000);
  // get BTN state
  buttonState = digitalRead(buttonPin);

  if(buttonState == HIGH && p_buttonState == LOW)
  {
    digitalWrite(LED_BUILTIN, HIGH);

    static boolean newDataReady = 0;
    const int serialPrintInterval = 0; //increase value to slow down serial print activity

    // init para
    float total = 0;
    int count = 50;

    // count 10 times
    while(count--)
    {
      delay(150);
      // check for new data/start next conversion:
      if (LoadCell.update()) newDataReady = true;
      
      // get smoothed value from the dataset:
      if (newDataReady) {
        if (millis() > t + serialPrintInterval) {
          float i = LoadCell.getData();
          
          Serial.print("Load_cell output val: ");
          Serial.println(i);
          newDataReady = 0;
          t = millis();
          if(count<=10){
            total += i;
          }
          bar.setLevel(11-count/5);
        }
      }
    }

    total /= 10;
    total -= offset;
    Serial.print("Total: ");
    Serial.println(total);

    // get weight
    if(total<1000) weight = 0;
    else if(total<2000) weight = 1;
    else if(total<3000) weight = 2;
    else if(total<4000) weight = 3;
    else if(total<5000) weight = 4;
    else weight = 5;
    // get weight end
    Serial.print("Weight: ");
    Serial.println(weight);
    
    // receive command from serial terminal, send 't' to initiate tare operation:
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 't') LoadCell.tareNoDelay();
    }
  
    // check if last tare operation is complete:
    if (LoadCell.getTareStatus() == true) {
      Serial.println("Tare complete");
    }
  
    // send data
    Serial.print("conected=");
    Serial.println(LBLEPeripheral.connected());
  
    if(LBLEPeripheral.connected())
    {
      // increment the value
      const int newValue = weight;
      counterCharacteristic.setValue(newValue);
      
      // broadcasting value changes to all connected central devices
      LBLEPeripheral.notifyAll(counterCharacteristic);
    }
    bar.setLevel(0);
  }
  else
  {
    delay(150);
    digitalWrite(LED_BUILTIN, LOW);
    static boolean newDataReady = 0;
    const int serialPrintInterval = 0; //increase value to slow down serial print activity
    float total = 0;
    
    if (LoadCell.update()) newDataReady = true;
    
    // get smoothed value from the dataset:
    if (newDataReady) {
      if (millis() > t + serialPrintInterval) {
        float i = LoadCell.getData();
        total = i;
        Serial.print("Load_cell output val: ");
        Serial.println(i);
        newDataReady = 0;
        t = millis();
      }
    }

    // get strength
    if(total<0) strength = 0;
    else if(total<500) strength = 1;
    else if(total<1000) strength = 2;
    else if(total<1500) strength = 3;
    else if(total<2000) strength = 4;
    else if(total<2500) strength = 5;
    else if(total<3000) strength = 6;
    else if(total<3500) strength = 7;
    else if(total<4000) strength = 8;
    else if(total<4500) strength = 9;
    else if(total<5000) strength = 10;
    else strength = 10;
    // get strength end
    Serial.print("Strength: ");
    Serial.println(strength);
    bar.setLevel(strength);
  }
  p_buttonState = buttonState;

}
