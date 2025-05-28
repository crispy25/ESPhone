#pragma once

#include <Arduino.h>
#include "../utils/macros.h"

void init_wifi();

bool connect_to_wifi(String wifi, String password);

void info_app(bool &connected_to_wifi);
