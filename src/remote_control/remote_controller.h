#pragma once

#include <Arduino.h>


void IRAM_ATTR gesture_sensor_isr();

bool get_sensor_flag();

void reset_sensor_flag();

void init_gesture_sensor();

uint8_t get_gesture();
