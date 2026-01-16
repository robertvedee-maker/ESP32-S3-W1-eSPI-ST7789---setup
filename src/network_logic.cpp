/*
 * (c)2026 R van Dorland
 */

#include "network_logic.h"

#include "helpers.h" // Nodig voor u8g2 en uitlijning
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <esp_system.h>
#include <esp_wifi.h>

extern String sunriseStr; // Extern gemaakt in helpers.h
extern String sunsetStr; // Extern gemaakt in helpers.h
extern String currentTimeStr; // Extern gemaakt in helpers.h
extern String currentDateStr; // Extern gemaakt in helpers.h
extern bool eersteStart; // Extern gemaakt in main.cpp
extern unsigned long lastBrightnessCheck; // Extern gemaakt in helpers.h
extern const unsigned long brightnessInterval; // Extern gemaakt in helpers.h
extern const char* DEVICE_MDNS_NAME; // Extern gemaakt in secret.h
extern TFT_eSPI tft; // Belofte dat de variabele tft ergens bestaat

// Kleur definities
#define BORDER TFT_LIGHTGREY // Randkleur voor de schermen

/**
 * Toont alleen netwerk informatie bij een "koude" start (stekker erin)
 * Bij een software herstart (na OTA) wordt dit overgeslagen.
 */

void toonNetwerkInfo()
{
    // Controleer: Is dit een koude start (Power On) of handmatige Reset knop?
    esp_reset_reason_t reset_reason = esp_reset_reason();

    // We gebruiken de namen die de compiler zojuist zelf voorstelde:
    if (reset_reason == ESP_RST_POWERON || reset_reason == ESP_RST_SW) {
        // ... je code voor het informatiescherm ...
        tft.setTextDatum(MC_DATUM);
        tft.fillScreen(TFT_BLACK);
        tft.drawRoundRect(1, 1, tft.width() - 2, tft.height() - 2, 5, BORDER);
        tft.drawString("SYSTEEM START", tft.width() / 2, 15);
        tft.setCursor(12, 35);
        tft.print("IP:   " + WiFi.localIP().toString());
        tft.setCursor(12, 48);
        tft.print("mDNS: " + String(DEVICE_MDNS_NAME) /* + ".local"*/);
        delay(4000);
    }

    // Zet de vlag op false zodat de loop() weet dat we klaar zijn
    eersteStart = false;
}

/**
 * WiFi SETUP
 */
void setupWiFi(const char* ssid, const char* password)
{
    WiFi.setSleep(false); // Voorkom dat WiFi in slaap valt

    WiFi.begin(ssid, password);
    esp_wifi_set_max_tx_power(34); // Maximaal zendvermogen (in dBm)

    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
        tft.setTextDatum(MC_DATUM);
        tft.fillScreen(TFT_BLACK);
        tft.drawRoundRect(1, 1, tft.width() - 2, tft.height() - 2, 5, BORDER);
        tft.drawString("WiFi Verbinden...", tft.width() / 2, tft.height() / 2);
        delay(500);
    }
}

/**
 * OTA SETUP
 */
void setupOTA(const char* hostname) // Gebruik DEVICE_MDNS_NAME uit secret.h
{
    ArduinoOTA.setHostname(hostname);

    ArduinoOTA.onStart([]() {
        // Veiligheidshalve alle interrupts uitzetten om conflicten tijdens OTA te voorkomen bij het updaten.
        detachInterrupt(digitalPinToInterrupt());

        tft.setTextDatum(MC_DATUM);
        tft.fillScreen(TFT_BLACK);
        tft.drawRoundRect(1, 1, tft.width() - 2, tft.height() - 2, 5, BORDER);
        tft.drawString("OTA Update Start...", tft.width() / 2, tft.height() / 2);
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        tft.setTextDatum(MC_DATUM);
        tft.fillScreen(TFT_BLACK);
        tft.drawRoundRect(1, 1, tft.width() - 2, tft.height() - 2, 5, BORDER);
        tft.drawString("Bezig met UPDATEN", tft.width() / 2, tft.height() / 2 - 10);
        // Voortgangsbalkje
        unsigned int width = (progress / (total / 100));
        tft.drawRect(14, tft.height() / 2 + 5, 100, 5, TFT_WHITE); // Buitenste kader
        tft.fillRect(15, tft.height() / 2 + 6, width - 2, 3, TFT_GREEN); // Binnenste vulling
    });

    ArduinoOTA.begin();
    MDNS.begin(hostname);
    MDNS.addService("arduino", "tcp", 8266);
}