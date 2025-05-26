#include "galery_app.h"

#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include <FS.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "../utils/macros.h"
#include "../utils/utils.h"

extern TFT_eSPI tft;
extern bool button_pressed;

// Fetch a file from the URL given and save it in LittleFS
// Return 1 if a web fetch was needed or 0 if file already exists
bool getFile(String url, String filename) {

  // If it exists then no need to fetch it
//   if (LittleFS.exists(filename) == true) {
//     Serial.println("Found " + filename);
//     return 0;
//   }

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


bool show_local_image()
{
    if (LittleFS.exists("/cat.jpg") == true) {
        Serial.println("Cat image present.");
        TJpgDec.drawFsJpg(0, 0, "/cat.jpg", LittleFS);
        return true;
    }

    return false;
}


bool download_image()
{
    digitalWrite(LED_PIN, HIGH);
    bool loaded_ok = getFile(GALLERY_URL, "/cat.jpg"); // Note name preceded with "/"

    if (loaded_ok) { Serial.println("Image downloaded"); }

    // Now draw the LittleFS file
    TJpgDec.drawFsJpg(0, 0, "/cat.jpg", LittleFS);
    digitalWrite(LED_PIN, LOW);

    return loaded_ok;
}


void show_online_images()
{
    uint16_t touch_x, touch_y;
    while (!button_pressed) {

        bool touch = tft.getTouch(&touch_x, &touch_y);

		if (!touch)
			continue;
        
        digitalWrite(LED_PIN, HIGH);
        bool loaded_ok = getFile(GALLERY_URL, "/cat.jpg"); // Note name preceded with "/"

        if (loaded_ok) { Serial.println("Image downloaded"); }
        digitalWrite(LED_PIN, LOW);

        // Now draw the LittleFS file
        Serial.println(TJpgDec.drawFsJpg(0, 0, "/cat.jpg", LittleFS));
    }

    button_pressed = false;
}


void galery_app(bool connected_to_wifi)
{   
    tft.fillScreen(TFT_BLACK);

    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS initialisation failed!");
    } 

    // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
    TJpgDec.setJpgScale(1);

    // The byte order can be swapped (set true for TFT_eSPI)
    TJpgDec.setSwapBytes(true);

    // The decoder must be given the exact name of the rendering function above
    TJpgDec.setCallback(tft_output);

    bool image_present = show_local_image();

    if (connected_to_wifi) {
        if (!image_present)
            download_image();

        show_online_images();
    } else {
        delay(4000);
    }
}

