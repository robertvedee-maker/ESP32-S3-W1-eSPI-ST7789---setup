/*
 * (c)2026 R van Dorland
 */

// Hardware-specific library

// Sketch to draw an analogue clock on the screen
// This uses anti-aliased drawing functions that are built into TFT_eSPI

// Anti-aliased lines can be drawn with sub-pixel resolution and permit lines to be
// drawn with less jaggedness.

// Based on a sketch by DavyLandman:
// https://github.com/Bodmer/TFT_eSPI/issues/905

#include "daynight.h"
#include "environment_sensors.h"
#include "network_logic.h" // Volgorde is hier erg belangrijk. niet aanpassen!
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <helpers.h>
#include <secret.h>

bool eersteStart = true; // Zorgt ervoor dat info éénmalig getoond wordt
extern bool isNightMode; // Track day/night mode for clock display

// const int xxPin = ##; //  --- gedefinieerd in pio.ini ---
const int freq = 5000; // 5 kHz is ideaal voor backlights
const int resolution = 8; // 8-bit resolutie (0 - 255)

#include "NotoSansBold15.h"

extern TFT_eSPI tft; // TFT instance from helpers.h
TFT_eSprite clockFace = TFT_eSprite(&tft); // Sprite for clock face

#define CLOCK_X_POS 10
#define CLOCK_Y_POS 10

#define CLOCK_FG TFT_SKYBLUE
#define CLOCK_BG TFT_NAVY
#define SECOND_FG TFT_RED
#define LABEL_FG TFT_GOLD
#define BORDER TFT_LIGHTGREY

#define CLOCK_R 229.0f / 2.0f // Clock face radius (float type)
#define H_HAND_LENGTH CLOCK_R / 2.0f
#define M_HAND_LENGTH CLOCK_R / 1.4f
#define S_HAND_LENGTH CLOCK_R / 1.3f

#define FACE_W CLOCK_R * 2 + 1
#define FACE_H CLOCK_R * 2 + 1

// Calculate 1 second increment angles. Hours and minute hand angles
// change every second so we see smooth sub-pixel movement
#define SECOND_ANGLE 360.0 / 60.0
#define MINUTE_ANGLE SECOND_ANGLE / 60.0
#define HOUR_ANGLE MINUTE_ANGLE / 12.0

float time_secs = 0;

// Time for next tick
uint32_t targetTime = 0;

// Forward declarations
static void renderFace(float t);
void getCoord(int16_t x, int16_t y, float* xp, float* yp, int16_t r, float a);
void updateLocalTime(); // Add forward declaration

/* =========================================================================
 *                                   Setup
 * =========================================================================
 */
void setup()
{
    Wire.begin(4, 5); // SDA = GPIO4, SCL = GPIO5
    Serial.begin(115200);
    Serial.flush(); // Forceer de USB-stack om de verbinding te verversen
    // Serial.setDebugOutput(true);     // Forceer de USB poort om te herstarten

    uint32_t start = millis();
    while (!Serial && (millis() - start) < 2000) {
        delay(10);
    }
    // Print een paar extra lege regels om de buffer "schoon" te maken
    Serial.println("\n\n");
    // Serial.println("--- DEBUG START ---");
    Serial.println("Booting...");

    setupSensors(); // Initialiseer de sensoren

    Serial.println(F("Setup started"));
    delay(2000);

    // Setup complete the backlight PWM
    // setupBacklight();
    // 1. Backlight PWM setup for ESP32 Arduino Core v2.x
    ledcSetup(0, freq, resolution);
    ledcAttachPin(TFT_BL, 0);

    // 2. Netwerk (nu lekker kort!)
    setupWiFi(SECRET_SSID, SECRET_PASSWORD);

    // fetchWeather(); // Haal direct het eerste weerbericht op

    if (WiFi.status() == WL_CONNECTED) {
        toonNetwerkInfo(); // Deze functie bevat de 'rtc_info->reason' check
        setupOTA(); // Start OTA service nadat WiFi verbonden is
    }

    // 3. Tijd en Regeling
    configTzTime(SECRET_TZ_INFO, SECRET_NTP_SERVER);

    // Initialiseer eerste waarden
    manageBrightness();
    setupBacklight();
    // setBacklight(50); // Zet backlight op volle helderheid tijdens setup

    // Initialise the screen
    tft.init();

    // Ideally set orientation for good viewing angle range because
    // the anti-aliasing effectiveness varies with screen viewing angle
    // Usually this is when screen ribbon connector is at the bottom
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    // Create the clock face sprite
    // clockFace.setColorDepth(8); // 8-bit will work, but reduces effectiveness of anti-aliasing
    clockFace.createSprite(FACE_W, FACE_H);

    // Only 1 font used in the sprite, so can remain loaded
    clockFace.loadFont(NotoSansBold15);

    // Draw the whole clock - NTP time not available yet
    renderFace(time_secs);

    targetTime = millis() + 100;

    // testje voor het weergeven van wat netwerk informatie
    // ... je code voor het informatiescherm ...

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.drawRoundRect(1, 1, tft.width() - 2, tft.height() - 2, 5, BORDER);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("SYSTEEM START", tft.width() / 2, tft.height() / 2 - 20);
    tft.setTextDatum(CC_DATUM);
    tft.setCursor(tft.width() / 2, tft.height() / 2);
    tft.print("IP:   " + WiFi.localIP().toString());
    tft.setTextDatum(CC_DATUM);
    tft.setCursor(tft.width() / 2, tft.height() / 2 + 20);
    tft.print("mDNS: " + String(DEVICE_MDNS_NAME) /* + ".local"*/);
    delay(5000);
    tft.fillScreen(TFT_BLACK);
}

/* =========================================================================
 *                                   Loop
 * =========================================================================
 */
void loop()
{
    // Regel de backlight (optioneel, kan ook in updateLocalTime)
    manageBrightness();


    // Haal de nieuwste tijd op in de variabelen
    updateLocalTime();

    // Bereken de seconden voor de klok
    float time_secs = (currentHour * 3600ULL) + (currentMinute * 60ULL) + currentSecond;

    // Teken de klok
    renderFace(time_secs);

    delay(100); // Korte pauze voor stabiliteit
}
/* =========================================================================
 * Render the clock face in the sprite and push to TFT
 */
static void renderFace(float t)
{
    static unsigned long lastSensorRead = 0;
    static float stablePressure = 0;
    static float stableTemperature = 0;
    static float stableAltitude = 0;
    static float stableSeaLevelPressure = 0;
    static float stableHumidity = 0;
    float h_angle = t * HOUR_ANGLE;
    float m_angle = t * MINUTE_ANGLE;
    float s_angle = t * SECOND_ANGLE;

    // Update de sensor maar één keer per 2 minuten (120000 ms)
    if (millis() - lastSensorRead > 120000 || lastSensorRead == 0) {
        readSensors();
        lastSensorRead = millis();

        // Optioneel: Print naar serial voor controle
        Serial.println("--- Dag / Nacht modus ---");
        Serial.print(isNightMode ? "Nachtmodus\n" : "Dagmodus\n");
    }

    uint16_t CLK_BG = isNightMode ? TFT_BLUE : TFT_WHITE;
    // uint16_t bgColor = TFT_BLACK;

    // The face is completely redrawn - this can be done quickly
    clockFace.fillSprite(TFT_BLACK);

    // Draw the face circle
    clockFace.fillSmoothCircle(CLOCK_R, CLOCK_R, CLOCK_R, CLK_BG);

    // Set text datum to middle centre and the colour
    clockFace.setTextDatum(MC_DATUM);

    // The background colour will be read during the character rendering
    clockFace.setTextColor(CLOCK_FG, CLK_BG);

    // Text offset adjustment
    constexpr uint32_t dialOffset = CLOCK_R - 10;

    float xp = 0.0, yp = 0.0; // Use float pixel position for smooth AA motion

    // Draw digits around clock perimeter
    for (uint32_t h = 1; h <= 12; h++) {
        getCoord(CLOCK_R, CLOCK_R, &xp, &yp, dialOffset, h * 360.0 / 12);
        clockFace.drawNumber(h, xp, 2 + yp);
    }

    // Add text (could be digital time...)
    clockFace.setTextColor(LABEL_FG, CLK_BG);
    clockFace.setTextDatum(MC_DATUM);
    clockFace.drawString("RD-web", CLOCK_R, CLOCK_R * 1.25);

    clockFace.setTextDatum(MR_DATUM);
    clockFace.drawFloat(stableTemperature, 1, CLOCK_R * 0.75, CLOCK_R * 0.5);
    clockFace.setTextDatum(ML_DATUM);
    clockFace.drawString("°C", CLOCK_R * 0.75 + 5, CLOCK_R * 0.5);

    clockFace.setTextDatum(MR_DATUM);
    clockFace.drawFloat(stableHumidity, 1, CLOCK_R * 1.25, CLOCK_R * 0.5);
    clockFace.setTextDatum(ML_DATUM);
    clockFace.drawString("%", CLOCK_R * 1.25 + 5, CLOCK_R * 0.5);

    clockFace.setTextDatum(MR_DATUM); // Middle Right (voor het getal)
    clockFace.drawFloat(stablePressure, 0, CLOCK_R, CLOCK_R * 0.75);
    clockFace.setTextDatum(ML_DATUM); // Middle Left (voor de eenheid)
    clockFace.drawString("hPa", CLOCK_R + 5, CLOCK_R * 0.75);

    // Draw minute hand
    getCoord(CLOCK_R, CLOCK_R, &xp, &yp, M_HAND_LENGTH, m_angle);
    clockFace.drawWideLine(CLOCK_R, CLOCK_R, xp, yp, 6.0f, CLOCK_FG);
    clockFace.drawWideLine(CLOCK_R, CLOCK_R, xp, yp, 2.0f, CLK_BG);

    // Draw hour hand
    getCoord(CLOCK_R, CLOCK_R, &xp, &yp, H_HAND_LENGTH, h_angle);
    clockFace.drawWideLine(CLOCK_R, CLOCK_R, xp, yp, 6.0f, CLOCK_FG);
    clockFace.drawWideLine(CLOCK_R, CLOCK_R, xp, yp, 2.0f, CLK_BG);

    // Draw the central pivot circle
    clockFace.fillSmoothCircle(CLOCK_R, CLOCK_R, 4, CLOCK_FG);

    // Draw second hand
    getCoord(CLOCK_R, CLOCK_R, &xp, &yp, S_HAND_LENGTH, s_angle);
    clockFace.drawWedgeLine(CLOCK_R, CLOCK_R, xp, yp, 2.5, 1.0, SECOND_FG);
    clockFace.pushSprite(5, 5, TFT_TRANSPARENT);
}

// =========================================================================
// Get coordinates of end of a line, pivot at x,y, length r, angle a
// =========================================================================
// Coordinates are returned to caller via the xp and yp pointers
#define DEG2RAD 0.0174532925
void getCoord(int16_t x, int16_t y, float* xp, float* yp, int16_t r, float a)
{
    float sx1 = cos((a - 90) * DEG2RAD);
    float sy1 = sin((a - 90) * DEG2RAD);
    *xp = sx1 * r + x;
    *yp = sy1 * r + y;
}
