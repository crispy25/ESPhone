#pragma once

#include <Arduino.h>
#include "../macros.h"

int show_networks(String wifi_names[MAX_WIFIS], uint8_t wifi_signals[MAX_WIFIS], uint8_t wifi_security[MAX_WIFIS], int &wifis, bool update);

bool connect_to_wifi(String wifi, String password);

void show_wifi_status(String wifi, bool connected);
