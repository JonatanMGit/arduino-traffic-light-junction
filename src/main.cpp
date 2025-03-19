#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Arduino_LSM6DS3.h>
#include "PinDefinitions.h"
#include "TestLamps.h"
#include "TrafficLightController.h"

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

void setup()
{
  Serial.begin(9600);

  // Initialize buttons and built-in LED
  pinMode(PED_BUTTON, INPUT_PULLUP);
  pinMode(VEHICLE_BUTTON, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize all lamp pins
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

  digitalWrite(LAMP1_RED_PED, HIGH);
  digitalWrite(LAMP2_RED_PED, HIGH);

  // Initialize WiFi
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ; // halt
  }
  WiFi.beginAP("Ampel");
  Serial.println("Access Point started");
  server.begin();
  Serial.println("Server started");

  // Initialize IMU sensor
  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ; // halt
  }
  Serial.println("IMU initialized");
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");
  Serial.println("Gyroscope in degrees/second");
  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println("Hz");

  // Initialize the traffic light controller module
  initTrafficController();
}

void loop()
{
  // Check for button presses (using INPUT_PULLUP: LOW means pressed)
  if (digitalRead(PED_BUTTON) == LOW)
  {
    handlePedestrianButton();
  }
  if (digitalRead(VEHICLE_BUTTON) == LOW)
  {
    handleVehicleButton();
  }

  // Update the traffic light state machine
  updateTrafficController();
}
