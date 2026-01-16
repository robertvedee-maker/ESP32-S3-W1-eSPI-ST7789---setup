/*
 * (c)2026 R van Dorland
 */

#ifndef NETWORK_LOGIC_H
#define NETWORK_LOGIC_H

#include <TFT_eSPI.h>



// Belofte dat de functies bestaan (alleen de naam en de ;)
void toonNetwerkInfo();
void setupWiFi(const char* ssid, const char* password);
void setupOTA();

#endif