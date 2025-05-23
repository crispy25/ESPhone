#include "clock_timer.h"

#include <TFT_eSPI.h>
#include <Arduino.h>
#include <time.h>

#define TICK 60
#define MAX_STR_TIME_LEN 16
#define COPY_TIME(T) strncpy(str_time, T, MAX_STR_TIME_LEN), str_time[MAX_STR_TIME_LEN - 1] = 0

extern TFT_eSPI tft;

static Ticker timer;
static int current_time = 0;
static char str_time[MAX_STR_TIME_LEN] = {0};
static uint8_t offset = 0;


volatile bool update_time = false;


void init_clock_timer()
{
	timer.attach(TICK, update_clock_time);

	COPY_TIME(__TIME__);
	current_time = str_to_time(str_time);
	update_clock_time();
}


void sync_clock_timer(char *time)
{
	timer.attach(TICK, update_clock_time);

	COPY_TIME(time);
	current_time = str_to_time(str_time);
	update_time = true;
}


void update_clock_time()
{
	current_time += TICK;
	update_time = true;
}


void draw_clock_time()
{
	// Clear old time
	tft.fillRect(140, 288, 98, 30, TFT_BLACK);

	tft.setTextColor(TFT_WHITE);
	tft.setTextSize(2);
	tft.setCursor(164, 292);

	time_to_str(current_time);

	tft.print(str_time);
}


int str_to_time(char *str_time)
{
	int time = 0;

	// Split HH:MM::SS formatted string into hours, minutes and seconds
	str_time[2] = 0;
	str_time[5] = 0;
	time += atoi(str_time) * 3600 + atoi(str_time + 3) * 60 + atoi(str_time + 6);

	return time;
}


void time_to_str(int time)
{
	int hours = time / 3600 % 24;
	int minutes = (time - hours * 3600) / 60 % 60;
	// int seconds = (time - hours * 3600 - minutes * 60) % 60;

	// sprintf(str_time, "%02d:%02d:%02d", hours, minutes, seconds);
	sprintf(str_time, "%02d:%02d", hours, minutes);
}
