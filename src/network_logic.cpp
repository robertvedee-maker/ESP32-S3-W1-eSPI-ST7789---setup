/*
 * (c)2026 R van Dorland
 */
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <TFT_eSPI.h>

#include "network_logic.h"
#include <WiFi.h>
#include <esp_system.h>
#include <esp_wifi.h>

#include "BootImages.h"
#include "NotoSansBold15.h"
#include "helpers.h" // Nodig voor u8g2 en uitlijning
#include "secret.h" // Voor DEVICE_MDNS_NAME

// Globale variabele voor eerste start detectie
extern bool eersteStart;
extern TFT_eSPI tft;

// Calculatie ten behoeve van positionering van de elementen

int centerX = 120; // tft.width() / 2;
int centerY = 120; // tft.height() / 2;
int leftTab = centerX - 60;
// int fHeight = tft.fontHeight(); // Hoogte van het font
int fHeight = 15; // Hoogte van het font
int row1 = centerY + fHeight; // Eerste regel
int row2 = row1 + fHeight + 4; // Tweede regel
int row3 = row2 + fHeight + 4; // Derde regel

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

        tft.drawRoundRect(1, 1, tft.width() - 2, tft.height() - 2, 10, 0xFFFF);
        tft.drawBitmap(leftTab, centerY - 57, image_RFIDDolphinSuccess_bits, 108, 57, 0xFFFF);

        tft.setTextColor(0xFFFF);
        tft.setTextSize(1);
        tft.setTextDatum(TL_DATUM);

        tft.drawString("SYSTEEM START", leftTab, row1);

        tft.setCursor(leftTab, row2);
        tft.print("IP:   " + WiFi.localIP().toString());

        tft.setCursor(leftTab, row3);
        tft.print("mDNS: " + String(DEVICE_MDNS_NAME) /* + ".local"*/);

        delay(4000);
        tft.fillScreen(TFT_BLACK);
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

    ledcSetup(4, 12000, 8);
    ledcAttachPin(TFT_BL, 4);
    ledcWrite(4, 20); // Zet de backlight op een laag niveau tijdens het verbinden

    WiFi.begin(ssid, password);
    // esp_wifi_set_max_tx_power(34); // Maximaal zendvermogen (in dBm)

    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
        tft.drawRoundRect(1, 1, tft.width() - 2, tft.height() - 2, 10, 0xFFFF);
        tft.drawBitmap(leftTab, centerY - 52, image_Scanning_bits, 123, 52, 0xFFFF);

        tft.setTextColor(0xFFFF);
        tft.setTextSize(1);
        tft.setTextDatum(TL_DATUM);

        tft.drawString("WiFi Verbinden...", leftTab, row1);

        delay(1000);
        tft.fillScreen(TFT_BLACK);
    }
}

/**
 * OTA SETUP
 */
void setupOTA() // Gebruik DEVICE_MDNS_NAME uit secret.h
{
    ArduinoOTA.setHostname(DEVICE_MDNS_NAME);

    ArduinoOTA.onStart([]() {
        // Veiligheidshalve alle interrupts uitzetten om conflicten tijdens OTA te voorkomen bij het updaten.
        // detachInterrupt(digitalPinToInterrupt());

        tft.fillScreen(TFT_BLACK);
        tft.drawRoundRect(1, 1, tft.width() - 2, tft.height() - 2, 10, 0xFFFF);
        tft.drawBitmap(59, 92, image_DolphinWait_bits, 59, 54, 0xFFFF);

        tft.setTextColor(0xFFFF);
        tft.setTextSize(1);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("OTA Update Start...", leftTab, row1);
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        tft.fillScreen(TFT_BLACK);
        tft.drawRoundRect(1, 1, tft.width() - 2, tft.height() - 2, 5, BORDER);
        tft.drawBitmap(59, 92, image_DolphinWait_bits, 59, 54, 0xFFFF);

        tft.setTextColor(0xFFFF);
        tft.setTextSize(1);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("Bezig met UPDATEN", leftTab, row1);
        // Voortgangsbalkje
        unsigned int width = (progress / (total / 100));
        tft.drawRect(14, row3 + 5, 100, 5, TFT_WHITE); // Buitenste kader
        tft.fillRect(15, row3 + 6, width - 2, 3, TFT_GREEN); // Binnenste vulling
    });

    ArduinoOTA.onEnd([]() {
        tft.fillScreen(TFT_BLACK);
    });

    MDNS.begin(DEVICE_MDNS_NAME);
    ArduinoOTA.begin();
}
