/*
 * (c)2026 R van Dorland
 */

#ifndef ENVIRONMENT_SENSORS_H
#define ENVIRONMENT_SENSORS_H

#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>

extern Adafruit_BMP280 bmp;
extern Adafruit_AHTX0 aht;

// Functie declaratie
void setupSensors();
void readSensors();


#endif