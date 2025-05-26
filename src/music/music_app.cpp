#include "music_app.h"

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "../utils/macros.h"
#include "../utils/utils.h"
#include "songs.h"


extern TFT_eSPI tft;

static String songs_name[] = {"Pink Panther", "Star Wars", "Merry Christmas", "Pacman"};
static int* songs_notes[] = {pink_panther_melody, star_wars_melody, christmas_melody, pacman_melody};
static int* songs_durations[] = {pink_panther_durations, star_wars_durations, christmas_durations, pacman_durations};
static uint8_t notes_count[] = {88, 88, 26, 31};
static uint8_t songs_count = 4;

extern bool button_pressed;


void play_song(uint8_t song_id)
{
	uint8_t notes = notes_count[song_id];
	int* melody = songs_notes[song_id];
	int* durations = songs_durations[song_id];

	for (int note = 0; note < notes; note++) { 
		//to calculate the note duration, take one second divided by the note type. 
		//e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc. 
		int duration = 1000 / durations[note]; 
		tone(BUZZER_PIN, melody[note], duration); 
		//to distinguish the notes, set a minimum time between them. 
		//the note's duration + 30% seems to work well: 
		int pauseBetweenNotes = duration * 1.30; 
		delay(pauseBetweenNotes); 
		//stop the tone playing: 
		noTone(BUZZER_PIN);

		// Check if user wants to play next song
		if (button_pressed) {
			button_pressed = false;
			break;
		}

		// Draw notes progress
		tft.fillRect(0, 300, (1.0 * note / notes) * 240, 40, TFT_WHITE);
	} 
}


void piano()
{	
	tft.fillRect(BACKGROUND_RECT, TFT_BLACK);

	tft.fillCircle(120, 160, 40, TFT_WHITE);
	tft.fillCircle(120, 160, 8, TFT_BLACK);


	uint16_t touch_x = 0, touch_y = 0;
	bool touch;

	while (!is_button_pressed(HOME_BUTTON, touch_x, touch_y)) {
		touch = tft.getTouch(&touch_x, &touch_y, 100);

		if (!touch) {
			// noTone(BUZZER_PIN);
			delay(100);
			ledcWriteTone(0, 0); 
			continue;
		}

		uint8_t f = map(touch_x + touch_y * 320, 0, 800, 30, 4000);

		ledcWriteTone(0, f);  // Play tone on channel 0
		delay(100);

		// ledcWriteTone(0, 0); 
	}
}


void draw_song_info(uint8_t song_id)
{	
	// Draw background color
	tft.fillScreen(TFT_BLACK);
	tft.setTextSize(2);
	tft.setTextColor(TFT_WHITE);

	tft.setCursor(40, 140);
	tft.println("Now playing:");
	tft.setCursor(40, 160);
	tft.print(songs_name[song_id]);
}


void init_music_app()
{
	pinMode(BUZZER_PIN, OUTPUT);

	// ledcSetup(0, 2000, 8);       // channel 0, 2 KHz, 8-bit resolution
  	ledcAttachPin(BUZZER_PIN, 0);
}


void music_app()
{
	uint8_t selected_song = 0;

	while (selected_song < songs_count) {

		draw_song_info(selected_song);
		play_song(selected_song);

		selected_song++;
	}
}
