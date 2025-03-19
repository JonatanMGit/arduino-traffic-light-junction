#include "PinDefinitions.h"

// Hauptstraße ist von links nach rechts, die Ampel 1 zeigt nach links
int LAMP1_RED = 5;    // Rot (Auto)
int LAMP1_YELLOW = 4; // Gelb (Auto)
int LAMP1_GREEN = 3;  // Grün (Auto)

// Nebenstraße ampel 2 zeigt nach unten
int LAMP2_RED = 10;   // Rot (Auto)
int LAMP2_YELLOW = 9; // Gelb (Auto)
int LAMP2_GREEN = 8;  // Grün (Auto)

// Ampel 3 Hauptstraße zeigt nach rechts
int LAMP3_RED = A2;    // Rot (Auto)
int LAMP3_YELLOW = A6; // Gelb (Auto)
int LAMP3_GREEN = A7;  // Grün (Auto)

// Fußgängerampel
int LAMP1_GREEN_PED = 6;  // Grün Fußgänger
int LAMP1_RED_PED = 7;    // Rot Fußgänger
int LAMP2_GREEN_PED = 11; // Grün Fußgänger
int LAMP2_RED_PED = 12;   // Rot Fußgänger

int PED_BUTTON = A1;    // Fußgänger Knopf
int VEHICLE_BUTTON = 2; // Fahrzeugerkennung