/*
 * (c)2026 R van Dorland
 */

#ifndef DAY_NIGHT_H
#define DAY_NIGHT_H

#include "secret.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SolarCalculator.h>
#include <time.h>

extern uint8_t currentHour;
extern uint8_t currentMinute;
extern uint8_t currentSecond;

// Functie declaratie
void manageBrightness();
void updateDateTimeStrings(struct tm* timeInfo);
void setBacklight(int brightness); 

// Extern variabelen (zodat ze ook elders gebruikt kunnen worden indien nodig)
[[maybe_unused]] extern const char* ntpServer;
[[maybe_unused]] extern const long gmtOffset_sec;
[[maybe_unused]] extern const int daylightOffset_sec;

void updateLocalTime();

#endif
