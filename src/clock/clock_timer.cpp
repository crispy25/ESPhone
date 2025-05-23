#include "clock_timer.h"

#include <Ticker.h>
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <time.h>

#define TICK 1

extern TFT_eSPI tft;

static Ticker timer;
static const long gmtOffset_sec = 2 * 3600;
static const int daylightOffset_sec = 3600;

volatile bool update_time = false;


int monthToInt(const char *month)
{
	const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
							"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	for (int i = 0; i < 12; i++) {
		if (!strcmp(month, months[i]))
			return i;
	}

	return 0;
}


void get_compile_time_info(struct tm &time_info)
{
	char buffer[20] = {0};
	strcpy(buffer, __DATE__);

	char *p = strtok(buffer, " ");
	int i = 0;

	while (p) {
		if (i == 0)
			time_info.tm_mon = monthToInt(p);
		else if (i == 1)
			time_info.tm_mday = atoi(p);
		else
			time_info.tm_year = atoi(p) - 1900;		
		
		i++;
		p = strtok(NULL, " ");
	}

	strcpy(buffer, __TIME__);

	buffer[2] = 0;
	buffer[5] = 0;

	time_info.tm_hour = atoi(buffer);
	time_info.tm_min = atoi(buffer + 3);
	time_info.tm_sec = atoi(buffer + 6);
}


void init_clock_timer()
{
	struct tm time_info;
	get_compile_time_info(time_info);

	time_t total = mktime(&time_info);
	
	struct timeval tv = {.tv_sec = total};
	settimeofday(&tv, NULL);
	
	timer.attach(TICK, update_clock_time);
}


void sync_clock_timer(const char *server)
{
	timer.detach();

	configTime(gmtOffset_sec, daylightOffset_sec, server);

	timer.attach(TICK, update_clock_time);
}


void update_clock_time()
{
	update_time = true;
}


void draw_clock_time()
{
	tft.setTextColor(TFT_WHITE);
	tft.setTextSize(2);
	tft.setCursor(140, 292);

	char buffer[10] = {0};
	struct tm time_info = {0};

	if(getLocalTime(&time_info))
		strftime(buffer, sizeof(buffer), "%H:%M:%S", &time_info);

	// Clear old time
	tft.fillRect(140, 288, 98, 30, TFT_BLACK);

	tft.print(buffer);
}
