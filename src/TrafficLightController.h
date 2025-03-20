#ifndef TRAFFIC_LIGHT_CONTROLLER_H
#define TRAFFIC_LIGHT_CONTROLLER_H

#include <Arduino.h>

// Define the possible states for the traffic light system.
enum TrafficLightState
{
    MAIN_GREEN,
    MAIN_YELLOW,
    ALL_RED,
    SIDE_RED_YELLOW,
    SIDE_GREEN,
    SIDE_YELLOW,
    MAIN_RED_YELLOW,
    PEDESTRIAN_GREEN
};

// Initialize the controller state machine.
void initTrafficController();

// Call this function in loop() to update the state based on elapsed time.
void updateTrafficController();

// Functions to handle external events.
void handlePedestrianButton();
void handleVehicleButton();

#endif // TRAFFIC_LIGHT_CONTROLLER_H
