#pragma once

#include <Arduino.h>



void IRAM_ATTR gesture_isr();
bool get_gesture_flag();
bool set_gesture_flag(bool value);

void IRAM_ATTR proximity_isr();
bool get_proximity_flag();
bool set_proximity_flag(bool value);


void init_gesture_sensor();

uint8_t get_gesture();
