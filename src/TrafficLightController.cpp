#include "TrafficLightController.h"
#include "PinDefinitions.h"
#include "TestLamps.h"

// --- State Variables and Timing Constants ---
static TrafficLightState currentState = MAIN_GREEN;
static unsigned long stateStartTime = 0;

const unsigned long MAIN_GREEN_DURATION_DEFAULT = 10000; // 10 sec.
static unsigned long mainGreenDuration = MAIN_GREEN_DURATION_DEFAULT;
const unsigned long MAIN_YELLOW_DURATION = 3000; // 3 sec.
const unsigned long ALL_RED_DURATION = 2000;     // 2 sec.
const unsigned long SIDE_GREEN_DURATION = 5000;  // 5 sec.
const unsigned long SIDE_YELLOW_DURATION = 3000; // 3 sec.

// Flags to debounce button presses
static bool pedestrianFlag = false;
static bool vehicleFlag = false;

// --- Internal Functions ---

// Update the lamps based on the current state.
// Uses a static variable to track the previous state and only update necessary pins.
static void setLights(TrafficLightState state)
{
    static TrafficLightState previousState = MAIN_GREEN;

    // Reset lights that were set in the previous state
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

    // Set lights for the new state
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

    previousState = state;

    // Print the current state for debugging
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

// Change the current state, update the timer, and set the lights.
static void changeState(TrafficLightState newState)
{
    currentState = newState;
    stateStartTime = millis();
    setLights(newState);
}

// --- Public Functions ---

void initTrafficController()
{
    currentState = MAIN_GREEN;
    stateStartTime = millis();
    setLights(currentState);
}

void updateTrafficController()
{
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - stateStartTime;

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
            vehicleFlag = false;                             // Reset vehicle detection
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

void handlePedestrianButton()
{
    if (!pedestrianFlag)
    {
        Serial.println("Pedestrian button pressed");
        pedestrianFlag = true;
    }
}

void handleVehicleButton()
{
    if (!vehicleFlag)
    {
        Serial.println("Vehicle detection button pressed");
        vehicleFlag = true;
        mainGreenDuration = MAIN_GREEN_DURATION_DEFAULT / 2; // Reduce main green duration
    }
}
