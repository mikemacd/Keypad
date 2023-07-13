#include <HardwareSerial.h>  
#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#include "Secrets.h"

#include "SinricPro.h"
#include "SinricProGarageDoor.h"
 
#include <ESP8266WiFi.h> 

#include <Keypad.h>
 
void openDoor();
void resetInput();
void keypadEventHandler(KeypadEvent);

void setupKeypad();
void setupWiFi();
void setupSinricPro();

void keypadLoop();
void sinricLoop();

bool onDoorStateHandler(const String& deviceId, bool &doorState);

const int SERIAL_SPEED = 9600;
const int MAX_INPUT = 1024;
char input[MAX_INPUT] = "";
int inputIndex = 0; 
const int DEBOUNCE_TIME = 50;

const byte n_rows = 4;
const byte n_cols = 3;

char keys[n_rows][n_cols] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}}; 

byte rowPins[n_rows] = {D2, D5, D6, D0};
byte colPins[n_cols] = {D1, D3, D4};

Keypad myKeypad = Keypad(makeKeymap(keys), rowPins, colPins, n_rows, n_cols);
 
void openDoor()
{
  Serial.print("\nOpening Door...\n");
}

void setup()
{
  Serial.begin(SERIAL_SPEED);
  setupKeypad();
  setupWiFi();
  setupSinricPro();

  Serial.println("Setup Complete.");
}

void loop()
{
  keypadLoop();
  sinricLoop();
}



void setupKeypad() {
  myKeypad.addEventListener(keypadEventHandler);
  myKeypad.setDebounceTime(DEBOUNCE_TIME);
  resetInput();
}

void keypadEventHandler(KeypadEvent ekey) {
  if (myKeypad.getState() == PRESSED)
  {

    switch (ekey)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      input[inputIndex] = ekey;
      inputIndex++;
      Serial.print(ekey);

      break;
    // Clear input
    case '*':
      Serial.printf("\nInput: %s==%s? %d", input, keypadPassword, strcmp(input, keypadPassword) == 0);
      break;
    case '#':
      resetInput();
      Serial.print("\nCLEARED!\n");
      break;
    default:
      Serial.print("\nmDefault!\n");

      break;
    }
  }
  if (strcmp(input, keypadPassword) == 0)
  {
    Serial.print("\nPassword Correct!\n");
    resetInput();

    openDoor();
  }
}

void resetInput()
{
  for (int i = 0; i < inputIndex; i++)
  {
    input[i] = '\0';
  }
  inputIndex = 0;
}

void keypadLoop(){
      myKeypad.getKey();
} 
 
void setupSinricPro() {
  SinricProGarageDoor& myGarageDoor = SinricPro[sinricDeviceID];
  myGarageDoor.onDoorState(onDoorStateHandler);

  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });

  SinricPro.begin(sinricAppKey, sinricAppSecret);
}

bool onDoorStateHandler(const String& deviceId, bool &doorState) {
  Serial.printf("Garagedoor is %s now.\r\n", doorState?"closed":"open");
  return true;
}
void sinricLoop() {
    SinricPro.handle();
}

void setupWiFi() {
  WiFi.begin(ssid, wifiPassword); // Connect to the network
  Serial.printf("Connecting to %s ...", ssid);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  { // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print(++i);
    Serial.print('.');
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer

}