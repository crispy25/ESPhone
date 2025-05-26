#pragma once

#include <Arduino.h>

struct WeatherInfo
{
    String temperature;
    bool clouds;
    bool raining;
};

WeatherInfo parseJSON(const String &json);

void update_weather(bool connected_to_wifi, bool ask_location);
