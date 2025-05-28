#include "chat_app.h"

#include <TFT_eSPI.h>
#include <WiFi.h>
#include <string.h>

#include "../utils/macros.h"
#include "../utils/utils.h"
#include "../keyboard/keyboard.h"
#include "../wifi/wifi.h"

#define PORT 8881
#define MAX_MESSAGES 14
#define MAX_BUFFER_SIZE 64
#define CHAT_BUTTON_AREA 168, 284, 64, 28
#define MESSAGE_MIN_LEN_FOR_MULTILINE 20

extern TFT_eSPI tft;


void show_no_connection()
{
    tft.setCursor(20, 140);
    tft.setTextSize(2);

    tft.print("No connection!");
}


void draw_chat_button()
{
    tft.setCursor(177, 292);
    tft.print("Chat");

    tft.drawRect(168, 285, 64, 28, TFT_WHITE);
}


void draw_ui()
{
    tft.fillRect(BACKGROUND_RECT, TFT_BLACK);
    tft.drawLine(0, 28, 240, 28, TFT_WHITE);

    // Clear time
    tft.fillRect(140, 288, 98, 30, TFT_BLACK);

    tft.setTextColor(TFT_WHITE);
	tft.setTextSize(2);

    tft.setCursor(72, 8);
    tft.print("ByteChat");

    draw_chat_button();
}


void register_user(String &username, String &destination)
{
    String user_input = keyboard_mode("Enter username:");
    if (!user_input.isEmpty())
        username = user_input;

    delay(500);

    user_input = keyboard_mode("Enter chat IP:");
    if (user_input.isEmpty()) {
        destination = WiFi.localIP().toString();
        
        int last_dot = destination.lastIndexOf(".");
        destination = destination.substring(0, last_dot + 1) + "255";
    }
}


String get_displayed_message(String &message) {
    if (message.length() > MESSAGE_MIN_LEN_FOR_MULTILINE)
        message = message.substring(0, MESSAGE_MIN_LEN_FOR_MULTILINE);
    
    return message;
}


void show_new_message(String messages[MAX_MESSAGES], uint8_t &messages_count, String message, uint16_t &user_messages, uint16_t user_color)
{
    if (!message.isEmpty()) {
        messages[messages_count++] = message;
    }

    if (messages_count >= MAX_MESSAGES) {
        messages_count = MAX_MESSAGES - 1;
        user_messages >>= 1;
        for (int i = 0; i < MAX_MESSAGES - 1; i++)
            messages[i] = messages[i + 1];
    }

    tft.fillRect(BACKGROUND_RECT, TFT_BLACK);
    tft.drawLine(0, 28, 240, 28, TFT_WHITE);
    
    tft.setTextSize(2);
    tft.setCursor(72, 8);
    tft.print("ByteChat");

    tft.setCursor(4, 35);

    for (int i = 0; i < messages_count; i++) {
        tft.setCursor(4, tft.getCursorY());
        tft.setTextColor(bitRead(user_messages, i) ? user_color : TFT_WHITE);
        tft.println(get_displayed_message(messages[i]));
    }

    // Reset text color
    tft.setTextColor(TFT_WHITE);
}


void chat_app(bool connected_to_wifi, uint16_t user_color)
{
    String messages[MAX_MESSAGES];
    char recv_buffer[MAX_BUFFER_SIZE];
    uint8_t messages_count = 0;
    uint16_t user_messages = 0;
    String username = "ESPhone";
    String chat_ip(WiFi.localIP().toString());
    WiFiUDP client;

    uint16_t touch_x = 0, touch_y = 0;
    bool touch = false;

    if (!connected_to_wifi) {
        show_no_connection();
        return;
    }

    if (user_color == TFT_BLACK)
        user_color = TFT_GREEN;

    register_user(username, chat_ip);
    Serial.println(chat_ip);
    
    String send_buffer(username + ": ");

    draw_ui();

    client.begin(PORT);

    while (!is_button_pressed(HOME_BUTTON, touch_x, touch_y)) {
		touch = tft.getTouch(&touch_x, &touch_y, 100);

        int packet_size = client.parsePacket();
        if (packet_size > 0) {
            // Avoid out of bounds access
            packet_size = min(packet_size, MAX_BUFFER_SIZE);

            String new_message = "";
            int bytes_read = 0;
            while (bytes_read < packet_size) {
                int len = client.read(recv_buffer, 8);
                recv_buffer[len] = 0;

                new_message.concat(recv_buffer);
                bytes_read += len;
            }
            show_new_message(messages, messages_count, new_message, user_messages, user_color);
        }

        if (!touch)
			continue;

        if (is_button_pressed(CHAT_BUTTON_AREA, touch_x, touch_y)) {
            String user_input = keyboard_mode("Enter message:");
            
            // Draw chat button as it gets cleared by keyboard
            draw_chat_button();

            String message = send_buffer + user_input;
            client.beginPacket(chat_ip.c_str(), PORT);
            client.print(message);
            client.endPacket();

            show_new_message(messages, messages_count, message, bitSet(user_messages, messages_count), user_color);
        }
    }
}
