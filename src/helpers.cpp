#include "helpers.h"
#include <Arduino.h>

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI(); // Hier wordt hij echt gemaakt

void setupBacklight() {
    pinMode(TFT_BL, OUTPUT); // Forceer de pin eerst als output
    ledcSetup(7, 5000, 8); 
    // Gebruik de variabele uit je platformio.ini
    ledcAttachPin(TFT_BL, 7); 
}

// Laat dit staan! Dit is de "motor" die het dimmen uitvoert.
void setBacklight(int brightness) {
    // We sturen de waarde naar kanaal 7, dat we in setupBacklight hebben aangemaakt
    ledcWrite(7, brightness); 
}