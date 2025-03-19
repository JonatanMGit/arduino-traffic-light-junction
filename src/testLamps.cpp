#include "TestLamps.h"

void testLamps()
{
    // teste die Ampel, indem nacheinander jede LED eingeschaltet wird
    digitalWrite(LAMP1_RED, HIGH);
    delay(500);
    digitalWrite(LAMP1_RED, LOW);
    digitalWrite(LAMP1_YELLOW, HIGH);
    delay(500);
    digitalWrite(LAMP1_YELLOW, LOW);
    digitalWrite(LAMP1_GREEN, HIGH);
    delay(500);
    digitalWrite(LAMP1_GREEN, LOW);
    digitalWrite(LAMP1_RED_PED, HIGH);
    delay(500);
    digitalWrite(LAMP1_RED_PED, LOW);
    digitalWrite(LAMP1_GREEN_PED, HIGH);
    delay(500);
    digitalWrite(LAMP1_GREEN_PED, LOW);
    digitalWrite(LAMP2_RED, HIGH);
    delay(500);
    digitalWrite(LAMP2_RED, LOW);
    digitalWrite(LAMP2_YELLOW, HIGH);
    delay(500);
    digitalWrite(LAMP2_YELLOW, LOW);
    digitalWrite(LAMP2_GREEN, HIGH);
    delay(500);
    digitalWrite(LAMP2_GREEN, LOW);
    digitalWrite(LAMP2_RED_PED, HIGH);
    delay(500);
    digitalWrite(LAMP2_RED_PED, LOW);
    digitalWrite(LAMP2_GREEN_PED, HIGH);
    delay(500);
    digitalWrite(LAMP2_GREEN_PED, LOW);
    digitalWrite(LAMP3_RED, HIGH);
    delay(500);
    digitalWrite(LAMP3_RED, LOW);
    digitalWrite(LAMP3_YELLOW, HIGH);
    delay(500);
    digitalWrite(LAMP3_YELLOW, LOW);
    digitalWrite(LAMP3_GREEN, HIGH);
    delay(500);
    digitalWrite(LAMP3_GREEN, LOW);
}