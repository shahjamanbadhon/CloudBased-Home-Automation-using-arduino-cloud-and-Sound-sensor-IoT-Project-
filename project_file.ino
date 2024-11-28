
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <ESP8266Wifi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Arduino_JSON.h>

//Clap----------------------------------------------

int sensor = 15; //D8 Connected to digital output of KY-038 sound sensor module
//int led = 02; //D4 Connected to postive of led
boolean is_on = false; //To determine/track if led is on or off
//---------------------------------------------------------




const char DEVICE_LOGIN_NAME[]  = "***************"; //Enter DEVICE ID

const char SSID[]               = "**********";    //Enter WiFi SSID (name)
const char PASS[]               = "************";    //Enter WiFi password
const char DEVICE_KEY[]         = "*********************";    //Enter Secret device password (Secret Key)


// define the GPIO connected with Relays and switches
#define RelayPin1 5  //D1
#define RelayPin2 4  //D2
#define RelayPin3 14 //D5
#define RelayPin4 12 //D6

#define SwitchPin1 10  //SD3
#define SwitchPin2 0   //D3 
#define SwitchPin3 13  //D7
#define SwitchPin4 3   //RX

#define wifiLed   16   //D0


int toggleState_1 = 0; //Define integer to remember the toggle state for relay 1
int toggleState_2 = 0; //Define integer to remember the toggle state for relay 2
int toggleState_3 = 0; //Define integer to remember the toggle state for relay 3
int toggleState_4 = 0; //Define integer to remember the toggle state for relay 4

void onSwitch1Change();
void onSwitch2Change();
void onSwitch3Change();
void onSwitch4Change();

CloudSwitch switch1;
CloudSwitch switch2;
CloudSwitch switch3;
CloudSwitch switch4;

void initProperties(){

  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);

  ArduinoCloud.addProperty(switch1, READWRITE, ON_CHANGE, onSwitch1Change);
  ArduinoCloud.addProperty(switch2, READWRITE, ON_CHANGE, onSwitch2Change);
  ArduinoCloud.addProperty(switch3, READWRITE, ON_CHANGE, onSwitch3Change);
  ArduinoCloud.addProperty(switch4, READWRITE, ON_CHANGE, onSwitch4Change);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

const int debounceDelay = 200; // Debounce delay in milliseconds
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long lastDebounceTime3 = 0;
unsigned long lastDebounceTime4 = 0;

// Track the previous state of each switch to handle edge detection
bool previousSwitchState1 = HIGH;
bool previousSwitchState2 = HIGH;
bool previousSwitchState3 = HIGH;
bool previousSwitchState4 = HIGH;

void manual_control() {
  unsigned long currentMillis = millis();

  // Manual Switch Control
  // Check each switch pin and toggle the corresponding relay if the switch is pressed (LOW state)
  
  // Check if Switch 1 is pressed
  if (digitalRead(SwitchPin1) == LOW && previousSwitchState1 == HIGH) {
    if (currentMillis - lastDebounceTime1 >= debounceDelay) {
      toggleState_1 = !toggleState_1;
      digitalWrite(RelayPin1, !toggleState_1);
      switch1 = toggleState_1; // Toggle Relay 1
      lastDebounceTime1 = currentMillis; // Update the last debounce time
    }
  }
  previousSwitchState1 = digitalRead(SwitchPin1);

  // Check if Switch 2 is pressed
  if (digitalRead(SwitchPin2) == LOW && previousSwitchState2 == HIGH) {
    if (currentMillis - lastDebounceTime2 >= debounceDelay) {
      toggleState_2 = !toggleState_2;
      digitalWrite(RelayPin2, !toggleState_2);
      switch2 = toggleState_2; // Toggle Relay 2
      lastDebounceTime2 = currentMillis; // Update the last debounce time
    }
  }
  previousSwitchState2 = digitalRead(SwitchPin2);

  // Check if Switch 3 is pressed
  if (digitalRead(SwitchPin3) == LOW && previousSwitchState3 == HIGH) {
    if (currentMillis - lastDebounceTime3 >= debounceDelay) {
      toggleState_3 = !toggleState_3;
      digitalWrite(RelayPin3, !toggleState_3);
      switch3 = toggleState_3; // Toggle Relay 3
      lastDebounceTime3 = currentMillis; // Update the last debounce time
    }
  }
  previousSwitchState3 = digitalRead(SwitchPin3);

  // Check if Switch 4 is pressed
  if (digitalRead(SwitchPin4) == LOW && previousSwitchState4 == HIGH) {
    if (currentMillis - lastDebounceTime4 >= debounceDelay) {
      toggleState_4 = !toggleState_4;
      digitalWrite(RelayPin4, !toggleState_4);
      switch4 = toggleState_4; // Toggle Relay 4
      lastDebounceTime4 = currentMillis; // Update the last debounce time
    }
  }
  previousSwitchState4 = digitalRead(SwitchPin4);
}


void setup() {


//------------------------CLAP----------------------------------

  pinMode(sensor, INPUT); //Setting the pin to input for reading data
  //pinMode(led, OUTPUT); //Setting the pin to output for turning the led on/off

  //-------------------------------------------------------------


  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500);

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(RelayPin3, OUTPUT);
  pinMode(RelayPin4, OUTPUT);

  pinMode(wifiLed, OUTPUT);

  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);
  pinMode(SwitchPin3, INPUT_PULLUP);
  pinMode(SwitchPin4, INPUT_PULLUP);

  //During Starting all Relays should TURN OFF
  digitalWrite(RelayPin1, HIGH);
  digitalWrite(RelayPin2, HIGH);
  digitalWrite(RelayPin3, HIGH);
  digitalWrite(RelayPin4, HIGH);

  digitalWrite(wifiLed, HIGH);  //Turn OFF WiFi LED
}

void loop() {
  ArduinoCloud.update();
  
  manual_control(); //Control relays manually

  if (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(wifiLed, HIGH); //Turn OFF WiFi LED
    //Serial.println("NO WIFI");
  }
  else{
    digitalWrite(wifiLed, LOW); //Turn ON WiFi LED
    //Serial.println("YES WIFI");
  }



//------------------Clap----------------------------------


int data = digitalRead(sensor); //Reading data from sensor and storing in variable

  if (data == 1) { // 1 is sent from sensor when loud noise is detected
    if (is_on == true) { // If led is on then turn it off
      
      digitalWrite(RelayPin1, HIGH);
      digitalWrite(RelayPin2, HIGH);
      is_on = false;
      switch1 = 0;
      switch2 = 0;
    }
    else { // else if led is off then turn it on
      digitalWrite(RelayPin1, LOW);
      digitalWrite(RelayPin2, LOW);
      is_on = true;
      switch1 = 1;
      switch2 = 1;
    }
  }










//-------------------------


}

void onSwitch1Change() {
  if (switch1 == 1)
  {
    digitalWrite(RelayPin1, LOW);
    Serial.println("Device1 ON");
    toggleState_1 = 1;
    is_on = true;
  }
  else
  {
    digitalWrite(RelayPin1, HIGH);
    Serial.println("Device1 OFF");
    toggleState_1 = 0;
    is_on = false;
  }
}

void onSwitch2Change() {
  if (switch2 == 1)
  {
    digitalWrite(RelayPin2, LOW);
    Serial.println("Device2 ON");
    toggleState_2 = 1;
    is_on = true;
  }
  else
  {
    digitalWrite(RelayPin2, HIGH);
    Serial.println("Device2 OFF");
    toggleState_2 = 0;
    is_on = false;
  }
}

void onSwitch3Change() {
  if (switch3 == 1)
  {
    digitalWrite(RelayPin3, LOW);
    Serial.println("Device2 ON");
    toggleState_3 = 1;
  }
  else
  {
    digitalWrite(RelayPin3, HIGH);
    Serial.println("Device3 OFF");
    toggleState_3 = 0;
  }
}

void onSwitch4Change() {
  if (switch4 == 1)
  {
    digitalWrite(RelayPin4, LOW);
    Serial.println("Device4 ON");
    toggleState_4 = 1;
  }
  else
  {
    digitalWrite(RelayPin4, HIGH);
    Serial.println("Device4 OFF");
    toggleState_4 = 0;
  }
}
