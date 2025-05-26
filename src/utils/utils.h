#pragma once

#include <Arduino.h>
#include <Wire.h>


bool is_button_pressed(int bx, int by, int bw, int bh, int px, int py);

// TODO: Remove after done testing

void test_touch();

void list_devices();