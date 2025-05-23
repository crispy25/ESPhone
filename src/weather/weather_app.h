#pragma once

#include <Arduino.h>

struct WeatherInfo
{
    String temperature;
    bool clouds;
};

WeatherInfo parseJSON(const String &json);

void update_weather();
