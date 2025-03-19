#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Arduino_LSM6DS3.h>
#include "PinDefinitions.h"
#include "TestLamps.h"

WiFiServer server(80);
// T-Junction Traffic Light Controller
// Pinout for 3 traffic lights and pedestrian lights:
//
// Traffic Lights:
// 1. Left-facing traffic light:
//    - Green: D3
//    - Yellow: D4
//    - Red: D5
// 2. Downward-facing traffic light (opposite to 1):
//    - Green: D8
//    - Yellow: D9
//    - Red: D10
// 3. Right-facing traffic light:
//    - Green: A7
//    - Yellow: A6
//    - Red: A2
//
// Pedestrian Lights:
// 1. Right-facing pedestrian light (on the same pole as traffic light 1):
//    - Green: D6
//    - Red: D7
// 2. Left-facing pedestrian light (on the same pole as traffic light 2):
//    - Green: D11
//    - Red: D12
//
// Buttons:
// - Pedestrian button: A1
// - Vehicle detection button: D2
//
// Traffic Flow Logic:
// - The main road runs from left to right (traffic light 1 to traffic light 3).
// - Default state prioritizes the main road (traffic light 1 and 3 green).
// - When the pedestrian button is pressed, the lights will change to allow pedestrians to cross. The green pedestrian light will blink before returning to the red state.
// - If traffic is detected on the lower road (via the vehicle detection button), the lights will prioritize the lower road (traffic light 2 green, traffic light 1 and 3 red) otherwise the main road will be prioritized (eg. it will be green for longer than the lower road).
// the code should not use delay() as other functions be happening in the meantime.

// Neue Zustände: Hauptstraße grün, Hauptstraße gelb, alle rot (Übergang zu Side),
// Nebenstraße grün, Nebenstraße gelb, alle rot (Übergang zu Main)
enum TrafficLightState
{
  MAIN_GREEN,
  MAIN_YELLOW,
  ALL_RED_TO_SIDE,
  SIDE_RED_YELLOW,
  SIDE_GREEN,
  SIDE_YELLOW,
  ALL_RED_TO_MAIN,
  MAIN_RED_YELLOW
};

TrafficLightState currentState = MAIN_GREEN;
unsigned long stateStartTime = 0;

// Zeitdauern in Millisekunden
const unsigned long MAIN_GREEN_DURATION_DEFAULT = 10000; // 10 Sek.
unsigned long mainGreenDuration = MAIN_GREEN_DURATION_DEFAULT;
const unsigned long MAIN_YELLOW_DURATION = 3000; // 3 Sek.
const unsigned long ALL_RED_DURATION = 2000;     // 2 Sek.
const unsigned long SIDE_GREEN_DURATION = 5000;  // 5 Sek.
const unsigned long SIDE_YELLOW_DURATION = 3000; // 3 Sek.

bool pedestrianButtonPressed = false;
bool vehicleButtonPressed = false;

// Setzt die Ampellampen je nach Zustand. Pedestrian-Lampen bleiben hier immer rot.
void setLights(TrafficLightState state)
{
  static TrafficLightState previousState = MAIN_GREEN; // Track the previous state to optimize updates

  // Reset only the lights that were active in the previous state
  switch (previousState)
  {
  case MAIN_GREEN:
    digitalWrite(LAMP1_GREEN, LOW);
    digitalWrite(LAMP3_GREEN, LOW);
    digitalWrite(LAMP2_RED, LOW);
    break;

  case MAIN_YELLOW:
    digitalWrite(LAMP1_YELLOW, LOW);
    digitalWrite(LAMP3_YELLOW, LOW);
    digitalWrite(LAMP2_RED, LOW);
    break;

  case ALL_RED_TO_SIDE:
  case ALL_RED_TO_MAIN:
    digitalWrite(LAMP1_RED, LOW);
    digitalWrite(LAMP2_RED, LOW);
    digitalWrite(LAMP3_RED, LOW);
    break;

  case SIDE_RED_YELLOW:
    digitalWrite(LAMP2_RED, LOW);
    digitalWrite(LAMP2_YELLOW, LOW);
    digitalWrite(LAMP1_RED, LOW);
    digitalWrite(LAMP3_RED, LOW);
    break;

  case SIDE_GREEN:
    digitalWrite(LAMP2_GREEN, LOW);
    digitalWrite(LAMP1_RED, LOW);
    digitalWrite(LAMP3_RED, LOW);
    break;

  case SIDE_YELLOW:
    digitalWrite(LAMP2_YELLOW, LOW);
    digitalWrite(LAMP1_RED, LOW);
    digitalWrite(LAMP3_RED, LOW);
    break;

  case MAIN_RED_YELLOW:
    digitalWrite(LAMP1_RED, LOW);
    digitalWrite(LAMP1_YELLOW, LOW);
    digitalWrite(LAMP3_RED, LOW);
    digitalWrite(LAMP3_YELLOW, LOW);
    digitalWrite(LAMP2_RED, LOW);
    break;
  }

  // Set the lights for the new state
  switch (state)
  {
  case MAIN_GREEN:
    digitalWrite(LAMP1_GREEN, HIGH);
    digitalWrite(LAMP3_GREEN, HIGH);
    digitalWrite(LAMP2_RED, HIGH);
    break;

  case MAIN_YELLOW:
    digitalWrite(LAMP1_YELLOW, HIGH);
    digitalWrite(LAMP3_YELLOW, HIGH);
    digitalWrite(LAMP2_RED, HIGH);
    break;

  case ALL_RED_TO_SIDE:
  case ALL_RED_TO_MAIN:
    digitalWrite(LAMP1_RED, HIGH);
    digitalWrite(LAMP2_RED, HIGH);
    digitalWrite(LAMP3_RED, HIGH);
    break;

  case SIDE_RED_YELLOW:
    digitalWrite(LAMP2_RED, HIGH);
    digitalWrite(LAMP2_YELLOW, HIGH);
    digitalWrite(LAMP1_RED, HIGH);
    digitalWrite(LAMP3_RED, HIGH);
    break;

  case SIDE_GREEN:
    digitalWrite(LAMP2_GREEN, HIGH);
    digitalWrite(LAMP1_RED, HIGH);
    digitalWrite(LAMP3_RED, HIGH);
    break;

  case SIDE_YELLOW:
    digitalWrite(LAMP2_YELLOW, HIGH);
    digitalWrite(LAMP1_RED, HIGH);
    digitalWrite(LAMP3_RED, HIGH);
    break;

  case MAIN_RED_YELLOW:
    digitalWrite(LAMP1_RED, HIGH);
    digitalWrite(LAMP1_YELLOW, HIGH);
    digitalWrite(LAMP3_RED, HIGH);
    digitalWrite(LAMP3_YELLOW, HIGH);
    digitalWrite(LAMP2_RED, HIGH);
    break;
  }

  // Update the previous state
  previousState = state;

  // Print state name instead of number
  Serial.print("Current state: ");
  switch (state)
  {
  case MAIN_GREEN:
    Serial.println("MAIN_GREEN");
    break;
  case MAIN_YELLOW:
    Serial.println("MAIN_YELLOW");
    break;
  case ALL_RED_TO_SIDE:
    Serial.println("ALL_RED_TO_SIDE");
    break;
  case SIDE_RED_YELLOW:
    Serial.println("SIDE_RED_YELLOW");
    break;
  case SIDE_GREEN:
    Serial.println("SIDE_GREEN");
    break;
  case SIDE_YELLOW:
    Serial.println("SIDE_YELLOW");
    break;
  case ALL_RED_TO_MAIN:
    Serial.println("ALL_RED_TO_MAIN");
    break;
  case MAIN_RED_YELLOW:
    Serial.println("MAIN_RED_YELLOW");
    break;
  }
}

// Zustand wechseln und Startzeit aktualisieren
void changeState(TrafficLightState newState)
{
  currentState = newState;
  stateStartTime = millis();
  setLights(newState);
}

void setup()
{
  Serial.begin(9600);

  pinMode(PED_BUTTON, INPUT_PULLUP);     // Fußgänger Knopf
  pinMode(VEHICLE_BUTTON, INPUT_PULLUP); // Fahrzeugerkennung

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LAMP1_RED, OUTPUT);
  pinMode(LAMP1_YELLOW, OUTPUT);
  pinMode(LAMP1_GREEN, OUTPUT);
  pinMode(LAMP1_GREEN_PED, OUTPUT);
  pinMode(LAMP1_RED_PED, OUTPUT);

  pinMode(LAMP2_RED, OUTPUT);
  pinMode(LAMP2_YELLOW, OUTPUT);
  pinMode(LAMP2_GREEN, OUTPUT);
  pinMode(LAMP2_GREEN_PED, OUTPUT);
  pinMode(LAMP2_RED_PED, OUTPUT);

  pinMode(LAMP3_RED, OUTPUT);
  pinMode(LAMP3_YELLOW, OUTPUT);
  pinMode(LAMP3_GREEN, OUTPUT);

  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  // create AP
  WiFi.beginAP("Ampel");
  Serial.println("Access Point started");

  server.begin();

  Serial.println("Server started");

  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }
  Serial.println("IMU initialized");

  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Gyroscope in degrees/second");

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println("Hz");

  setLights(currentState);
  stateStartTime = millis(); // Setze die Startzeit für den ersten Zustand
}

void loop()
{
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - stateStartTime;

  // Check if the pedestrian button is pressed
  if (digitalRead(PED_BUTTON) == LOW)
  {
    // Debounce the button press
    if (!pedestrianButtonPressed)
    {
      Serial.println("Pedestrian button pressed");
      pedestrianButtonPressed = true;
    }
  }

  // Check if the vehicle detection button is pressed
  if (digitalRead(VEHICLE_BUTTON) == LOW)
  {
    // Debounce the button press
    if (!vehicleButtonPressed)
    {
      Serial.println("Vehicle detection button pressed");
      vehicleButtonPressed = true;
      mainGreenDuration = MAIN_GREEN_DURATION_DEFAULT / 2; // Halve the green time
    }
  }

  switch (currentState)
  {
  case MAIN_GREEN:
    if (elapsedTime >= mainGreenDuration)
      changeState(MAIN_YELLOW);
    break;

  case MAIN_YELLOW:
    if (elapsedTime >= MAIN_YELLOW_DURATION)
      changeState(ALL_RED_TO_SIDE);
    break;

  case ALL_RED_TO_SIDE:
    if (elapsedTime >= ALL_RED_DURATION)
      changeState(SIDE_RED_YELLOW);
    break;

  case SIDE_RED_YELLOW:
    if (elapsedTime >= ALL_RED_DURATION)
      changeState(SIDE_GREEN);
    break;

  case SIDE_GREEN:
    if (elapsedTime >= SIDE_GREEN_DURATION)
    {
      changeState(SIDE_YELLOW);
      vehicleButtonPressed = false;                    // Reset vehicle detection
      mainGreenDuration = MAIN_GREEN_DURATION_DEFAULT; // Reset green time
    }
    break;

  case SIDE_YELLOW:
    if (elapsedTime >= SIDE_YELLOW_DURATION)
      changeState(ALL_RED_TO_MAIN);
    break;

  case ALL_RED_TO_MAIN:
    if (elapsedTime >= ALL_RED_DURATION)
      changeState(MAIN_RED_YELLOW);
    break;

  case MAIN_RED_YELLOW:
    if (elapsedTime >= ALL_RED_DURATION)
      changeState(MAIN_GREEN);
    break;
  }
}