#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Arduino_LSM6DS3.h>
#include "PinDefinitions.h"
#include "TestLamps.h"
#include "TrafficLightController.h"

WiFiServer server(80);

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
