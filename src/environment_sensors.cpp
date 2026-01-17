/*
 * (c)2026 R van Dorland
 */

#include "environment_sensors.h"
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Wire.h>

// Define color constants if not defined elsewhere
#ifndef LABEL_FG
#define LABEL_FG TFT_WHITE
#endif
#ifndef CLK_BG
#define CLK_BG TFT_BLACK
#endif

Adafruit_BMP280 bmp; // use I2C interface
Adafruit_Sensor* bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor* bmp_pressure = bmp.getPressureSensor();
String bmp_pressure_str = "----"; // Variable to store pressure as string
String bmp_temp_str = "----"; // Variable to store temperature as string
String bmp_alt_str = "----"; // Variable to store altitude as string
String bmp_hum_str = "----"; // Variable to store humidity as string
String bmp_seaLevel_str = "----"; // Variable to store sea level pressure as string

Adafruit_AHTX0 aht;
Adafruit_Sensor* aht_temp;
Adafruit_Sensor* aht_humidity;

// Variables to store stable sensor readings
float temperature = 0.0F;
float humidity = 0.0F;
float pressure = 0.0F;
float stablePressure = 0.0F;
float stableTemperature = 0.0F;
float stableHumidity = 0.0F;
float stableAltitude = 0.0F;
float stableSeaLevelPressure = 0.0F;

extern TFT_eSPI tft; // TFT instance from helpers.h
extern TFT_eSprite clockFace; // Sprite for clock face

extern int CLOCK_R; // Clock radius from helpers.h or main

// extern FACE_H;
// extern FACE_W;

void setupSensors()
{
    unsigned status;

    Serial.println(F("BMP280 Sensor event test"));
    // status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
    status = bmp.begin(0x77);
    if (!status) {
        Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                         "try a different address!"));
        Serial.print("SensorID was: 0x");
        Serial.println(bmp.sensorID(), 16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1)
            delay(10);
    }
    Serial.println(F("BMP280 Found!"));

    /* Default settings from datasheet. */
    bmp.setSampling(
        Adafruit_BMP280::MODE_NORMAL, /* Operating Mode. */
        Adafruit_BMP280::SAMPLING_X2, /* Temp. oversampling */
        Adafruit_BMP280::SAMPLING_X16, /* Pressure oversampling */
        Adafruit_BMP280::FILTER_X16, /* Filtering. */
        Adafruit_BMP280::STANDBY_MS_500 /* Standby time. */
    );

    bmp_temp->printSensorDetails();
    bmp_pressure->printSensorDetails();

    Serial.println("Adafruit AHT10/AHT20 test!");

    if (!aht.begin()) {
        Serial.println("Failed to find AHT10/AHT20 chip");
        while (1) {
            delay(10);
        }
    }
    Serial.println("AHT10/AHT20 Found!");

    aht_temp = aht.getTemperatureSensor();
    aht_temp->printSensorDetails();
    aht_humidity = aht.getHumiditySensor();
    aht_humidity->printSensorDetails();

    delay(500);
}

void getSensorReadings(float* temperature, float* humidity, float* pressure)
{
    // float rawPressure = bmp.readPressure() / 100.0F;
    // float rawTemperature = bmp.readTemperature();
    // float rawAltitude = bmp.readAltitude(1013.25); // Standaard zeeniveau druk
    // float rawSeaLevelPressure = bmp.seaLevelForAltitude(rawAltitude, rawPressure);

    // Optioneel: Rond af op 1 decimaal om het geflikker in de cijfers te stoppen
    // stablePressure = round(rawPressure * 10.0F) / 10.0F;
    // stableTemperature = round(rawTemperature * 10.0F) / 10.0F;
    // stableAltitude = round(rawAltitude * 10.0F) / 10.0F;
    // stableSeaLevelPressure = round(rawSeaLevelPressure * 10.0F) / 10.0F;

    // 1. Lees BMP280 (Luchtdruk en evt. Temp)
    stablePressure = round(bmp.readPressure() / 100.0F); // Geen decimalen

    // 2. Lees AHT20 (Luchtvochtigheid en Temp)
    sensors_event_t humidityEvent, tempEvent;
    aht.getEvent(&humidityEvent, &tempEvent);
    stableHumidity = round(humidityEvent.relative_humidity); // Geen decimalen
    stableTemperature = round(tempEvent.temperature * 10.0F) / 10.0F; // 1 decimaal voor temp

    // Optionally print to serial for verification
    Serial.println("--- Sensor Readings ---");
    Serial.printf("BMP280 P: %.2f hPa\n", stablePressure);
    Serial.printf("AHT20 T: %.2f C | H: %.2f %%\n", stableTemperature, stableHumidity);
}

void renderClockFace()
{
    static unsigned long lastSensorRead = 0;
    static float stablePressure = 0;
    static float stableTemperature = 0;
    static float stableAltitude = 0;
    static float stableSeaLevelPressure = 0;
    static float stableHumidity = 0;
    // float h_angle = t * HOUR_ANGLE;
    // float m_angle = t * MINUTE_ANGLE;
    // float s_angle = t * SECOND_ANGLE;

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

}