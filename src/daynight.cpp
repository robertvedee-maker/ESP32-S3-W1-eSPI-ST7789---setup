/*
 * (c)2026 R van Dorland
 */

#include "daynight.h"
#include "helpers.h"
#include "network_logic.h" // Zodat we bij de tijd-functies kunnen komen
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <time.h>
#include <SolarCalculator.h>

// Tijd variabelen (gebruikt in manageBrightness)
uint8_t currentHour = 0; // Extern gemaakt in main.cpp
uint8_t currentMinute = 0; // Extern gemaakt in main.cpp
uint8_t currentSecond = 0; // Extern gemaakt in main.cpp

//  Backlight timing variabelen
unsigned long lastBrightnessCheck = 0; // Extern gemaakt in helpers.h
const unsigned long brightnessInterval = 10 * 60 * 1000; // Elke 10 minuten controleren (extern gemaakt in helpers.h)

String sunriseStr; // Extern gemaakt in helpers.h
String sunsetStr; // Extern gemaakt in helpers.h
String currentTimeStr; // Extern gemaakt in helpers.h
String currentDateStr; // Extern gemaakt in helpers.h

double sunrise_local = 0; // Extern gemaakt in helpers.h
double sunset_local = 0; // Extern gemaakt in helpers.h

double latitude = SECRET_LAT; // Extern gemaakt in helpers.h
double longitude = SECRET_LON; // Extern gemaakt in helpers.h

// Forward declaration of setBacklight function
extern void setBacklight(int brightness); // Defined in helpers.cpp
bool isNightMode = false; // Houdt bij of we in nachtmodus zitten

// Beheer de helderheid van de backlight op basis van zonsopgang en zonsondergang
void manageBrightness()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
        return;

    time_t now = time(nullptr);
    double transit, sunrise, sunset;

    // 1. Bereken de tijden (deze komen altijd terug in UTC)
    calcSunriseSunset(now, latitude, longitude, transit, sunrise, sunset);

    // 2. Bepaal de lokale offset (Winter = 1.0, Zomer = 2.0)
    double utcOffset = (timeinfo.tm_isdst > 0) ? 2.0 : 1.0;

    // 3. Tel de offset handmatig op bij de resultaten
    double sunrise_local = sunrise + utcOffset;
    double sunset_local = sunset + utcOffset;

    // 4. Formatteer de LOKALE tijden voor het display
    sunriseStr = formatTime(sunrise_local);
    sunsetStr = formatTime(sunset_local);

    // 5. Gebruik de lokale tijden voor de contrast-regeling
    double currentHour = timeinfo.tm_hour + (timeinfo.tm_min / 60.0);

    if (currentHour >= sunrise && currentHour < sunset) {
        if (isNightMode) { // Was het net nacht? Dan nu naar dag-stand
            setBacklight(2);
            isNightMode = false;
        }
    } else {
        if (!isNightMode) { // Was het net dag? Dan nu naar nacht-stand
            setBacklight(100);
            isNightMode = true;
        }
    }

}

// Update de globale tijd variabelen met de huidige lokale tijd
void updateLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        // Als het ophalen mislukt (bijv. nog geen WiFi sync)
        return;
    }
    currentHour = timeinfo.tm_hour;
    currentMinute = timeinfo.tm_min;
    currentSecond = timeinfo.tm_sec;

}