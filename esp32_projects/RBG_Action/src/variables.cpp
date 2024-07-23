#include "header.h"

Adafruit_NeoPixel pixels;
volatile float R = 0, G = 0, B = 0;

Action actions[16];
uint8_t current_com = 0, com_size = 0;

bool isOTAInProgress = false;