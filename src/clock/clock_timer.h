#pragma once

#include <Ticker.h>

void init_clock_timer();

void sync_clock_timer(char *time);

void update_clock_time();

void draw_clock_time();

int str_to_time(char *time);

void time_to_str(int time);
