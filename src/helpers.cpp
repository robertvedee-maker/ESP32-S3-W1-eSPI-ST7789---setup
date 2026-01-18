#include "helpers.h"
#include <Arduino.h>

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI(); // Hier wordt hij echt gemaakt
// int brightness = 255; // Standaard helderheid (0-255)

void setupBacklight() 
{
    ledcSetup(4, 12000, 8); // ledc: 4  => Group: 0, Channel: 2, Timer: 1, led frequency, resolution  bits
    ledcAttachPin(TFT_BL, 4); // gpio number and channel
    ledcWrite(4, 0); // write to channel number 4}


    // pinMode(TFT_BL, OUTPUT); // Forceer de pin eerst als output
    // ledcSetup(4, 12000, 8); 
    // // Gebruik de variabele uit je platformio.ini
    // ledcAttachPin(TFT_BL, 4); 
}

// Laat dit staan! Dit is de "motor" die het dimmen uitvoert.
// void setBacklight(int brightness) {
//     // We sturen de waarde naar kanaal 7, dat we in setupBacklight hebben aangemaakt
//     ledcWrite(4, brightness); 
// }

/*
void manageBrightness() {
    // ... jouw logica met sunrise/sunset ...
    
    if (currentHour >= sunrise_local && currentHour < sunset_local) {
        if (isNightMode) { 
            isNightMode = false;
            setBrightness = SECRET_BL_Sunrise; // Waarde voor overdag (bijv. 0)
        }
    } else {
        if (!isNightMode) {
            isNightMode = true;
            setBrightness = SECRET_BL_Sunset; // Waarde voor nacht (bijv. 200)
        }
    }

    // STUUR NAAR HARDWARE:
    // Roep de helper functie aan met de nieuw berekende waarde
    setBacklight(setBrightness);
}

*/