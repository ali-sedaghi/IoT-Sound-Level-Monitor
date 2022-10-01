#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display size in px
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_RESET -1

// OLED display instant
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Sound sensor pin
int sound_pin = A0;

// Sound variables
int sampling_rate = 64;
long sound_signal = 0;
long sum_sound_signal = 0;
long avg_sound_signal = 0;
long sound_level = 0;

// Sound Threshold
int sound_low = 50;
int sound_medium = 200;
int error = 100;

// Thingspeak server
String api_key = "OLQD4LWGMIVM6H88";
const char* server = "api.thingspeak.com";

// WiFi
const char *ssid = "ESP_Modem";
const char *password = "123456789asd";
// const char *ssid = "D-Link";
// const char *password = "66215474asd";

// WiFi client instant
WiFiClient client;

void setup () {
  // Init pins
  pinMode(sound_pin, INPUT);
  
  // Init serial and baud rate
  Serial.begin(115200);
  
  // Init OLDED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  delay(10);
  
  // Print WiFi connecting status on terminal
  Serial.println("Connecting to ");
  Serial.println(ssid);
  
  // Display WiFi connecting status on OLED
  display.setCursor(0, 0);
  display.println("Connecting to: ");
  display.setCursor(0, 20);
  display.setTextSize(2);
  display.print(ssid);
  display.display();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  
  // Print dots when trying to connecting WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Print WiFi connected status on terminal
  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Display WiFi connected status on OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("WiFi connected.");
  display.display();
  delay(4000);
}

void loop () {
  // Reset everything
  sound_signal = 0;
  sum_sound_signal = 0;
  avg_sound_signal = 0;
  sound_level = 0;
  display.clearDisplay();
  
  // Read sound signal value 'sampling_rate' times
  // Sum up the reading values
  for (int i = 0; i < sampling_rate; i++) {
    sound_signal = analogRead(sound_pin);
    sum_sound_signal += sound_signal;
  }
  
  // Average sound values in 'sampling_rate' times
  avg_sound_signal = sum_sound_signal / sampling_rate;
  
  // Remove the negative value for sound level
  sound_level = avg_sound_signal - error;
  
  // Print sound level
  Serial.print("Sound Level: ");
  Serial.println(sound_level);
  
  // Display sound level
  display.setCursor(0, 0);
  display.println("--- Decibelmeter ---");
  
  display.setCursor(0, 15);
  display.print("Sound Level: ");
  display.println(sound_level);
  
  // Display message based on sound level
  if(sound_level < sound_low) {
    Serial.print("Intensity: LOW");
    display.setCursor(0, 30);
    display.print("Intensity: LOW");
    display.display();
  } 
  if((sound_level > sound_low) && (sound_level < sound_medium)) {
    Serial.print("Intensity: MEDIUM");
    display.setCursor(0, 30);
    display.print("Intensity: MEDIUM");
    display.setCursor(0, 45);
    display.print("Please be quiet.");
    display.display();
  }
  if(sound_level > sound_medium) {
    Serial.print("Intensity: HIGH");    
    display.setCursor(0, 30);
    display.print("Intensity: HIGH");
    display.setCursor(0, 45);
    display.print("BE QUIET.");
    display.display();
  }

  delay(200);
  
  // HTTP POST request
  if (client.connect(server, 80)) {
    String post_str = api_key;
    post_str += "&field1=";
    post_str += String(sound_level);
    post_str += "r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + api_key + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(post_str.length());
    client.print("\n\n");
    client.print(post_str);
  }
  client.stop();
}
