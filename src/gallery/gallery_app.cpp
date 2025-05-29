/*
 * The following code is derived from:
 * https://github.com/Bodmer/TJpg_Decoder
 * 
 * Original License:
 * 
 * This library incorporate the Tiny JPEG Decompressor code files:
 * "tjpgd.h" and "tjpgd.c". The licence for these files is:
 * 
 * /*----------------------------------------------------------------------------/
 * / TJpgDec - Tiny JPEG Decompressor R0.01c                     (C)ChaN, 2019
 * /-----------------------------------------------------------------------------/
 * / The TJpgDec is a generic JPEG decompressor module for tiny embedded systems.
 * / This is a free software that opened for education, research and commercial
 * /  developments under license policy of following terms.
 * /
 * /  Copyright (C) 2019, ChaN, all right reserved.
 * /
 * / * The TJpgDec module is a free software and there is NO WARRANTY.
 * / * No restriction on use. You can use, modify and redistribute it for
 * /   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
 * / * Redistributions of source code must retain the above copyright notice.
 * /
 * /
 * /-----------------------------------------------------------------------------/
 * 
 * This Arduino library "TJpd_Decoder" has been created by Bodmer, for all the
 * additional code the FreeBSD licence applies and is compatible with the GNU GPL:
 * 
 * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvStartvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 * Software License Agreement (FreeBSD License)
 * 
 * Copyright (c) 2019 Bodmer (https://github.com/Bodmer)
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^End^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*/

#include "galery_app.h"

#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include <FS.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "../utils/macros.h"
#include "../utils/utils.h"

#define MAX_IMAGES 10

extern TFT_eSPI tft;
extern volatile bool button_pressed;
static uint8_t images_count = 0;

// Fetch a file from the URL given and save it in LittleFS
// Return 1 if a web fetch was needed or 0 if file already exists
bool getFile(String url, String filename) {

  Serial.println("Downloading "  + filename + " from " + url);

  // Check WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    Serial.print("[HTTP] begin...\n");

    HTTPClient http;
    // Configure server and url
    http.begin(url);

    Serial.print("[HTTP] GET...\n");
    // Start connection and send HTTP header
    int httpCode = http.GET();
    if (httpCode == 200) {
      fs::File f = LittleFS.open(filename, "w+");
      if (!f) {
        Serial.println("file open failed");
        return 0;
      }
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // File found at server
      if (httpCode == HTTP_CODE_OK) {

        // Get length of document (is -1 when Server sends no Content-Length header)
        int total = http.getSize();
        int len = total;

        // Create buffer for read
        uint8_t buff[128] = { 0 };

        // Get tcp stream
        WiFiClient * stream = http.getStreamPtr();

        // Read all data from server
        while (http.connected() && (len > 0 || len == -1)) {
          // Get available data size
          size_t size = stream->available();

          if (size) {
            // Read up to 128 bytes
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

            // Write it to file
            f.write(buff, c);

            // Calculate remaining bytes
            if (len > 0) {
              len -= c;
            }
          }
          yield();
        }
        Serial.println();
        Serial.print("[HTTP] connection closed or file end.\n");
      }
      f.close();
    }
    else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  return 1; // File was fetched from web
}

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // Return 1 to decode next block
  return 1;
}


void show_local_images()
{
	File root = LittleFS.open("/", "r");

	if (!root) {
		Serial.println(F("Failed to open directory"));
		return;
	}
	if (!root.isDirectory()) {
		Serial.println(F("Not a directory"));
		return;
	}

	fs::File file = root.openNextFile();

	if (!file)
		return;

	digitalWrite(LED_PIN, HIGH);

	// Now draw the LittleFS file
	TJpgDec.drawFsJpg(0, 0, file.path(), LittleFS);

	Serial.println(file.path());

	digitalWrite(LED_PIN, LOW);

	file = root.openNextFile();

	uint16_t touch_x = 0, touch_y = 0;

    while (!button_pressed && file) {

        bool touch = tft.getTouch(&touch_x, &touch_y);

		if (!touch)
			continue;
        
        digitalWrite(LED_PIN, HIGH);

        // Now draw the LittleFS file
        TJpgDec.drawFsJpg(0, 0, file.path(), LittleFS);

        digitalWrite(LED_PIN, LOW);

		Serial.println(file.path());

		file = root.openNextFile();
    }

    button_pressed = false;

	tft.fillScreen(TFT_BLACK);
	delay(2000);
}



void show_online_images()
{
	uint16_t touch_x = 0, touch_y = 0;
    int file_index = 0;

	digitalWrite(LED_PIN, HIGH);

	String file_name("/cat0");  // Note name preceded with "/"

	bool loaded_ok = false;
	
	while (!loaded_ok && !button_pressed)
		loaded_ok = getFile(GALLERY_URL, file_name);

	Serial.println("Image downloaded");

	// Now draw the LittleFS file
	TJpgDec.drawFsJpg(0, 0, file_name, LittleFS);

	digitalWrite(LED_PIN, LOW);

	file_index++;
	file_index %= MAX_IMAGES;

    while (!button_pressed) {

        bool touch = tft.getTouch(&touch_x, &touch_y);

		if (!touch)
			continue;
        
        digitalWrite(LED_PIN, HIGH);

		String file_name("/cat");  // Note name preceded with "/"
		file_name += file_index;

        bool loaded_ok = false;
		
		while (!loaded_ok && !button_pressed)
			loaded_ok = getFile(GALLERY_URL, file_name);

        Serial.println("Image downloaded");

        // Now draw the LittleFS file
        TJpgDec.drawFsJpg(0, 0, file_name, LittleFS);

        digitalWrite(LED_PIN, LOW);

		file_index++;
		file_index %= MAX_IMAGES;
    }

	size_t total = LittleFS.totalBytes();
	size_t used = LittleFS.usedBytes();

	Serial.println("LittleFS Storage Info:");
	Serial.printf("Free space: %u bytes\n", total - used);

  button_pressed = false;
}


void galery_app(bool connected_to_wifi)
{   
    tft.fillScreen(TFT_BLACK);

    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS initialisation failed!");
        return;
    }

    // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
    TJpgDec.setJpgScale(1);

    // The byte order can be swapped (set true for TFT_eSPI)
    TJpgDec.setSwapBytes(true);

    // The decoder must be given the exact name of the rendering function above
    TJpgDec.setCallback(tft_output);

    if (connected_to_wifi) {
        show_online_images();
    } else {
		show_local_images();
    }
}
