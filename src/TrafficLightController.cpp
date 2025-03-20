#include "TrafficLightController.h"
#include "PinDefinitions.h"
#include "TestLamps.h"

// --- State Variables and Timing Constants ---
TrafficLightState currentState = MAIN_GREEN; // Make currentState accessible globally
static unsigned long stateStartTime = 0;

const unsigned long MAIN_GREEN_DURATION_DEFAULT = 10000; // 10 sec.
static unsigned long mainGreenDuration = MAIN_GREEN_DURATION_DEFAULT;
const unsigned long MAIN_YELLOW_DURATION = 3000;       // 3 sec.
const unsigned long ALL_RED_DURATION = 2000;           // 2 sec.
const unsigned long SIDE_GREEN_DURATION = 5000;        // 5 sec.
const unsigned long SIDE_YELLOW_DURATION = 3000;       // 3 sec.
const unsigned long PEDESTRIAN_GREEN_DURATION = 10000; // 10 sec

// Flags to debounce button presses
static bool pedestrianFlag = false;
static bool vehicleFlag = false;

// Variable to direct the traffic flow
// true = main road, false = side road
static bool directionIsMain = true;

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
    case ALL_RED:
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
    case PEDESTRIAN_GREEN:
        digitalWrite(LAMP1_GREEN_PED, LOW);
        digitalWrite(LAMP1_RED_PED, HIGH);
        digitalWrite(LAMP2_GREEN_PED, LOW);
        digitalWrite(LAMP2_RED_PED, HIGH);
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
    case ALL_RED:
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
    case PEDESTRIAN_GREEN:
        digitalWrite(LAMP1_GREEN_PED, HIGH);
        digitalWrite(LAMP1_RED_PED, LOW);
        digitalWrite(LAMP2_GREEN_PED, HIGH);
        digitalWrite(LAMP2_RED_PED, LOW);

        digitalWrite(LAMP1_RED, HIGH);
        digitalWrite(LAMP2_RED, HIGH);
        digitalWrite(LAMP3_RED, HIGH);
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
    case ALL_RED:
        Serial.println("ALL_RED");
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
    case MAIN_RED_YELLOW:
        Serial.println("MAIN_RED_YELLOW");
        break;
    case PEDESTRIAN_GREEN:
        Serial.println("PEDESTRIAN_GREEN");
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
            changeState(ALL_RED);
        break;

    case ALL_RED:
        directionIsMain = !directionIsMain; // Toggle direction
        if (elapsedTime >= ALL_RED_DURATION)
        {
            if (pedestrianFlag)
                changeState(PEDESTRIAN_GREEN);
            else if (directionIsMain)
                changeState(MAIN_RED_YELLOW);
            else
                changeState(SIDE_RED_YELLOW);
        }
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
            changeState(ALL_RED);
        break;

    case MAIN_RED_YELLOW:
        if (elapsedTime >= ALL_RED_DURATION)
            changeState(MAIN_GREEN);
        break;

    case PEDESTRIAN_GREEN:
        if (elapsedTime >= PEDESTRIAN_GREEN_DURATION)
        {
            pedestrianFlag = false; // Reset pedestrian flag
            changeState(ALL_RED);
        }
        else if (PEDESTRIAN_GREEN_DURATION - elapsedTime <= 3000)
        {
            // Blink pedestrian green light
            if ((elapsedTime / 250) % 2 == 0) // Toggle every 250ms
            {
                digitalWrite(LAMP1_GREEN_PED, HIGH);
                digitalWrite(LAMP2_GREEN_PED, HIGH);
            }
            else
            {
                digitalWrite(LAMP1_GREEN_PED, LOW);
                digitalWrite(LAMP2_GREEN_PED, LOW);
            }
        }
        break;
    }
}

void handlePedestrianButton()
{
    if (!pedestrianFlag)
    {
        Serial.println("Pedestrian button pressed");
        pedestrianFlag = true;

        // blink pedestrian green light
        bool currentGreen = digitalRead(LAMP1_GREEN_PED);
        digitalWrite(LAMP1_GREEN_PED, !currentGreen);
        digitalWrite(LAMP2_GREEN_PED, !currentGreen);
        delay(100);
        digitalWrite(LAMP1_GREEN_PED, currentGreen);
        digitalWrite(LAMP2_GREEN_PED, currentGreen);
    }
}

void handleVehicleButton()
{
    if (!vehicleFlag)
    {
        Serial.println("Vehicle detection button pressed");
        vehicleFlag = true;
        mainGreenDuration = MAIN_GREEN_DURATION_DEFAULT / 2; // Reduce main green duration

        // blink the yellow light of the side road
        bool currentYellow = digitalRead(LAMP1_YELLOW);
        digitalWrite(LAMP2_YELLOW, !currentYellow);
        delay(100);
        digitalWrite(LAMP2_YELLOW, currentYellow);
    }
}

void setTrafficLightState(const String &state)
{
    if (state == "MAIN_GREEN")
        changeState(MAIN_GREEN);
    else if (state == "MAIN_YELLOW")
        changeState(MAIN_YELLOW);
    else if (state == "ALL_RED")
        changeState(ALL_RED);
    else if (state == "SIDE_RED_YELLOW")
        changeState(SIDE_RED_YELLOW);
    else if (state == "SIDE_GREEN")
        changeState(SIDE_GREEN);
    else if (state == "SIDE_YELLOW")
        changeState(SIDE_YELLOW);
    else if (state == "MAIN_RED_YELLOW")
        changeState(MAIN_RED_YELLOW);
    else if (state == "PEDESTRIAN_GREEN")
        changeState(PEDESTRIAN_GREEN);
}
