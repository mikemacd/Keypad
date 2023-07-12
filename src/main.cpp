#include "Secrets.h"

#include <ESP8266WiFi.h> // Include the Wi-Fi library

#include <Keypad.h>
 
const int SERIAL_SPEED = 9600;
const int DEBOUNCE_TIME = 50;
const int MAX_INPUT = 1024;


char input[MAX_INPUT] = "";
int inputIndex = 0; // Current index of the input string

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

void resetInput()
{
  for (int i = 0; i < inputIndex; i++)
  {
    input[i] = '\0';
  }
  inputIndex = 0;
}

void keypadEvent(KeypadEvent ekey)
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

void setup()
{

  Serial.begin(SERIAL_SPEED); // Start the Serial communication to send messages to the computer
  myKeypad.addEventListener(keypadEvent);
  myKeypad.setDebounceTime(DEBOUNCE_TIME);
  resetInput();

  delay(10);
  Serial.println('\n');

  WiFi.begin(ssid, wifiPassword); // Connect to the network
  Serial.printf("Connecting to %s ...", ssid);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  { // Wait for the Wi-Fi to connect
    delay(500);
    Serial.print(++i);
    Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer
}

void loop()
{
  myKeypad.getKey();
  delay(5);
}
