#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define DEBUG_ESP_PORT Serial
#define NODEBUG_WEBSOCKETS
#define NDEBUG
#define NODEBUG_SINRIC
#endif

#include "Secrets.h"

#include <WiFiUdp.h>
#include <Syslog.h>

#define SYSLOG_PORT 514

// This device info
#define DEVICE_HOSTNAME "makerspace"
#define APP_NAME "door-controller"

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udpClient;

// Create a new syslog instance with LOG_KERN facility
Syslog syslog(udpClient, REMOTE_SYSLOG, SYSLOG_PORT, DEVICE_HOSTNAME, APP_NAME, LOG_KERN);
int syslogIiteration = 1;

#include "SinricPro.h"
#include "SinricProGarageDoor.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <Keypad.h>

void openDoor();
void resetInput();
void keypadEventHandler(KeypadEvent);

void setupKeypad();
void setupWiFi();
void setupSinricPro();
void setupWebserver();

void keypadLoop();
void sinricLoop();
void webserverLoop();

void handleRoot();
void handleApiRequest();
void handleLogRequest();

bool onDoorStateHandler(const String &deviceId, bool &doorState);

const int SERIAL_SPEED = 9600;
const int MAX_INPUT = 1024;
const int RELAY_PIN = D7;

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

ESP8266WebServer server(80); // Create a web server on port 80

void openDoor()
{
  Serial.print("\nOpening Door...\n");
  digitalWrite(RELAY_PIN, LOW);
  delay(250);
  digitalWrite(RELAY_PIN, HIGH);
  Serial.print("\nToggle Door Command Sent!\n");
  syslog.logf(LOG_DEBUG, "Door opened.");
}

void setup()
{

  pinMode(D0, INPUT_PULLUP);
  pinMode(D1, INPUT_PULLUP);
  pinMode(D2, INPUT_PULLUP);
  pinMode(D3, INPUT_PULLUP);
  pinMode(D4, INPUT_PULLUP);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
  digitalWrite(D0, HIGH);
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
  digitalWrite(D5, HIGH);
  digitalWrite(D6, HIGH);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  Serial.begin(SERIAL_SPEED);
  while (!Serial && !Serial.available())
    ;

  Serial.setDebugOutput(true);

  setupKeypad();
  setupWiFi();
  setupWebserver();
  setupSinricPro();

  // initialize our logger

  Serial.printf("Remote syslog server: %s\n", REMOTE_SYSLOG);
  syslog.logf(LOG_INFO, "Remote syslog server: %s", REMOTE_SYSLOG);

  syslog.log(LOG_INFO, "Setup Complete!");
  Serial.println("Setup Complete.");
}

void loop()
{
  keypadLoop();
  sinricLoop();
  server.handleClient(); // Handle incoming client requests
  delay(10);
}

void setupKeypad()
{
  myKeypad.addEventListener(keypadEventHandler);
  myKeypad.setDebounceTime(DEBOUNCE_TIME);
  resetInput();
}

void keypadEventHandler(KeypadEvent ekey)
{
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
      syslog.logf(LOG_DEBUG, "Keypad button pushed: %c", ekey);
      break;
    // Clear input
    case '*':
    case '#':
      resetInput();
      syslog.log(LOG_INFO, "Input cleared!");

      Serial.print("\nCLEARED!\n");
      break;
    default:
      Serial.print(".");

      break;
    }
  }
  if (strcmp(input, keypadPassword) == 0)
  {
    syslog.log(LOG_INFO, "Keypad Password correct!");
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

void keypadLoop()
{
  char key = myKeypad.getKey();
  if (key)
  {
    Serial.print("\nKeyPress:");
    Serial.print(key);
    Serial.print("\n");
    syslog.logf(LOG_DEBUG, "\nKeyPress: %c\n", key);
  }
}

void setupSinricPro()
{

  SinricPro.onConnected([]()
                        {
                          Serial.printf("Connected to SinricPro\r\n");
                          syslog.log(LOG_INFO, "Connected to SinricPro."); });
  SinricPro.onDisconnected([]()
                           { 
    Serial.printf("Disconnected from SinricPro\r\n"); 
    syslog.log(LOG_INFO, "Disconnected from SinricPro."); });

  SinricProGarageDoor &myGarageDoor = SinricPro[sinricDeviceID];
  myGarageDoor.onDoorState(onDoorStateHandler);

  SinricPro.begin(sinricAppKey, sinricAppSecret);
}

bool onDoorStateHandler(const String &deviceId, bool &doorState)
{
  syslog.log(LOG_INFO, "Received Sinric message to toggle door.");
  openDoor();

  return true;
}

void sinricLoop()
{
  SinricPro.handle();
}

void setupWiFi()
{
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
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  syslog.logf(LOG_INFO, "Connected to wifi. IP: %s", WiFi.localIP().toString().c_str());
}

void loopWebserver()
{
  server.handleClient(); // Handle incoming client requests
}

void setupWebserver()
{
  server.on("/", handleRoot);           // Handle root endpoint
  server.on("/open", handleApiRequest); // Handle API request

  server.begin();
}

void handleRoot()
{
  syslog.logf(LOG_INFO, "Received API request to get health");

  server.send(200, "text/plain", "ESP8266 is running!"); // Default response
}

void handleApiRequest()
{
  syslog.logf(LOG_INFO, "Received API request to open door");

  openDoor(); // Call the function to open the door
  server.send(200, "text/plain", "Door opened!");
}