/*
 * (c)2026 R van Dorland
 */

#include "daynight.h"
#include "helpers.h"
#include "network_logic.h" // Zodat we bij de tijd-functies kunnen

// Forward declaration of setBacklight function
extern void setBacklight(int brightness);

// Het u8g2 object wordt in main.cpp gedefinieerd, we vertellen de compiler dat het bestaat
// extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2; // Pas het type aan naar jouw specifieke display type!

// const char* TZ_INFO = SECRET_TZ_INFO;
// const char* ntpServer = SECRET_NTP_SERVER;

uint8_t currentHour = 0;
uint8_t currentMinute = 0;
uint8_t currentSecond = 0;

String sunriseStr;
String sunsetStr;

double sunrise_local = 0;
double sunset_local = 0;

double latitude = SECRET_LAT;
double longitude = SECRET_LON;

bool isNightMode = false;

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
            setBacklight(255);
            isNightMode = false;
        }
    } else {
        if (!isNightMode) { // Was het net dag? Dan nu naar nacht-stand
            setBacklight(50);
            isNightMode = true;
        }
    }
}

void updateLocalTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        // Als het ophalen mislukt (bijv. nog geen WiFi sync)
        return;
    }
    currentHour   = timeinfo.tm_hour;
    currentMinute = timeinfo.tm_min;
    currentSecond = timeinfo.tm_sec;
}