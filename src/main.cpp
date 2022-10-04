#include <Arduino.h>

#include <SPI.h>
#include <EPD1in54.h>
#include <EPDPaint.h>
#include "imagedata.h"

// The Nano (like all Arduinos) has an LED attached to digital pin 13, which is also 
//  used for SPI: 10 (SS), 11 (MOSI), 12 (MISO), 13 (SCK).
EPD1in54 epd; // default reset: 8, dc: 9, cs: 10, busy: 7

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello World!");
  // pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW

  if (epd.init(lutFullUpdate) != 0) {
    Serial.print("e-Paper init failed");
    return;
  }

  epd.setFrameMemory(IMAGE_DATA);
  epd.displayFrame();
  epd.setFrameMemory(IMAGE_DATA);
  epd.displayFrame();

}

void loop() {
  // put your main code here, to run repeatedly:

}