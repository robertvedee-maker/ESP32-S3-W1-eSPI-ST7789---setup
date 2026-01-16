/*
 * (c)2026 R van Dorland
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>
// #include <U8g2lib.h> // Nodig voor het u8g2 object
#include "daynight.h"
#include <TFT_eSPI.h>
#include <time.h>

void setupBacklight(); // Voeg deze toe
void setBacklight(int brightness);

extern unsigned long lastBrightnessCheck;
extern const unsigned long brightnessInterval;

extern double sunrise_local;
extern double sunset_local;

// extern int blPin;

extern String sunriseStr;
extern String sunsetStr;
extern String currentTimeStr;
extern String currentDateStr;

// Nederlandse namen voor weekdagen en maanden
static const char* const wd_nl[] PROGMEM = { "Zo", "Ma", "Di", "Wo", "Do", "Vr", "Za" };
static const char* const mo_nl[] PROGMEM = { "Jan", "Feb", "Mrt", "Apr", "Mei", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dec" };

// Functie die van een double (18.5) een mooie String maakt ("18:30")
[[maybe_unused]] static String formatTime(double decimalTime)
{
    int h = (int)decimalTime;
    int m = (int)((decimalTime - h) * 60 + 0.5); // +0.5 voor correct afronden

    char buffer[6]; // Genoeg voor "HH:MM\0"
    sprintf(buffer, "%02d:%02d", h, m);
    return String(buffer);
}

// NIEUW: Functie voor de huidige TIJD (UU:MM)
[[maybe_unused]] static String formatCurrentTime(struct tm* timeinfo)
{
    char buffer[9];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo); // %H:%M geeft bijv. 09:45
    return String(buffer);
}

[[maybe_unused]] inline void updateDateTimeStrings(struct tm* timeInfo)
{
    char buff[32];

    // 1. Tijd opmaak: HH:MM:SS
    sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
    currentTimeStr = String(buff);

    // 2. Datum opmaak: Zo.11 Jan 2026
    const char* wday = (const char*)pgm_read_ptr(&wd_nl[timeInfo->tm_wday]);
    const char* month = (const char*)pgm_read_ptr(&mo_nl[timeInfo->tm_mon]);

    sprintf_P(buff, PSTR("%s.%02d %s %04d"), wday, timeInfo->tm_mday, month, timeInfo->tm_year + 1900);
    currentDateStr = String(buff);
}

void setBacklight(int brightness); 

[[maybe_unused]] inline void setBacklightBrightness(int brightness)
{
    // brightness waarde tussen 0 (uit) en 255 (max helderheid)
    ledcWrite(TFT_BL, brightness);
}

#endif