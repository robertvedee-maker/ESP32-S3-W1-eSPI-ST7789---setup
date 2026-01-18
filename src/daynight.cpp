/*
 * (c)2026 R van Dorland
 */

#include "daynight.h"
#include "helpers.h"
#include "network_logic.h"
#include "secret.h"
#include <Arduino.h>
#include <SolarCalculator.h>
#include <TFT_eSPI.h>
#include <time.h>

// Tijd variabelen (gebruikt in manageTimeFunctions)
uint8_t currentHour = 0; 
uint8_t currentMinute = 0;
uint8_t currentSecond = 0; 

//  Backlight timing variabelen
unsigned long lastBrightnessCheck = 0;
const unsigned long brightnessInterval = 10 * 60 * 1000; // Elke 10 minuten controleren (extern gemaakt in helpers.h)

String sunriseStr;
String sunsetStr;
String currentTimeStr;
String currentDateStr;

double sunrise_local = 0;
double sunset_local = 0;

// const char* TZ_INFO = SECRET_TZ_INFO;
// const char* ntpServer = SECRET_NTP_SERVER;
double latitude = SECRET_LAT;
double longitude = SECRET_LON;

bool isNightMode = false; // Houdt bij of we in nachtmodus zitten
int setBrightness = 0; // Huidige helderheid van de backlight

// Beheer de helderheid van de backlight op basis van zonsopgang en zonsondergang
void manageTimeFunctions()
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

    if (currentHour >= sunrise_local && currentHour < sunset_local) {
        if (isNightMode) { // Was het net nacht? Dan nu naar dag-stand
            // setBacklight(100);
            isNightMode = false;
            setBrightness = SECRET_BL_Sunrise;
        }
    } else {
        if (!isNightMode) { // Was het net dag? Dan nu naar nacht-stand
            // setBacklight(250);
            isNightMode = true;
            setBrightness = SECRET_BL_Sunset;
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


/*

void manageBrightness() {
    // Definieer hoe lang de overgang duurt in minuten (bijv. 60 minuten)
    const int fadeDuration = 60; 
    int targetBrightness = SECRET_BL_Sunrise; // Standaard dag-stand

    // 1. Bereken de huidige tijd in minuten vanaf middernacht
    int currentTotalMinutes = (currentHour * 60) + currentMinute;
    int sunsetMinutes = (sunset_local * 60);
    int sunriseMinutes = (sunrise_local * 60);

    // 2. Logica voor de overgang
    if (currentTotalMinutes >= sunsetMinutes && currentTotalMinutes < (sunsetMinutes + fadeDuration)) {
        // We zitten in de AVOND-FADE
        float progress = (float)(currentTotalMinutes - sunsetMinutes) / fadeDuration;
        // Bereken waarde tussen Dag (255) en Nacht (50)
        targetBrightness = SECRET_BL_Sunrise - (progress * (SECRET_BL_Sunrise - SECRET_BL_Sunset));
    } 
    else if (currentTotalMinutes >= sunriseMinutes && currentTotalMinutes < (sunriseMinutes + fadeDuration)) {
        // We zitten in de OCHTEND-FADE (optioneel)
        float progress = (float)(currentTotalMinutes - sunriseMinutes) / fadeDuration;
        targetBrightness = SECRET_BL_Sunset + (progress * (SECRET_BL_Sunrise - SECRET_BL_Sunset));
    }
    else if (currentTotalMinutes >= (sunsetMinutes + fadeDuration) || currentTotalMinutes < sunriseMinutes) {
        // Het is echt nacht
        targetBrightness = SECRET_BL_Sunset;
    } else {
        // Het is echt dag
        targetBrightness = SECRET_BL_Sunrise;
    }

    // 3. Update de globale variabele
    setBrightness = (int)targetBrightness;

    // 4. Stuur alleen naar hardware bij verandering (zoals we net bespraken)
    static int lastSentBrightness = -1;
    if (setBrightness != lastSentBrightness) {
        ledcWrite(4, setBrightness);
        lastSentBrightness = setBrightness;
    }
}


// Voeg dit toe net voordat je de waarde naar de hardware stuurt
setBrightness = constrain((int)targetBrightness, SECRET_BL_Sunset, SECRET_BL_Sunrise);


// De diesel-methode: rustig en veilig
int finalValue = (int)targetBrightness;
setBrightness = constrain(finalValue, 0, 255); 

*/