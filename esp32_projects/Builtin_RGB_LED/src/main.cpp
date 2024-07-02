#include <Adafruit_NeoPixel.h>

#define PIN 38

Adafruit_NeoPixel pixels(1, PIN, NEO_GRB + NEO_KHZ800);

void setup()
{

  pixels.begin();
  Serial.begin(115200);
}

void loop()
{
  Serial.println("hello");
  pixels.clear();

  // Set the color of the LED (e.g., red)
  pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  pixels.show();

  delay(1000); // Wait for a second

  // Set the color of the LED (e.g., green)
  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show();

  delay(1000); // Wait for a second

  // Set the color of the LED (e.g., blue)
  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  pixels.show();

  delay(1000); // Wait for a second
}