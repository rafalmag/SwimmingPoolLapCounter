
#include <SPI.h>
#include <EPD1in54.h>
#include <EPDPaint.h>

#include "RREFont.h"
#include "rre_term_10x16.h"

#include <CapacitiveSensor.h>
#include <Button.h>
#include <ButtonEventCallback.h>
#include <CapacitiveButton.h>
#include "AnalogPin.h"

RREFont font;

#define COLORED 0
#define UNCOLORED 1

/**
 * Due to RAM not enough in Arduino UNO, a frame buffer is not allowed.
 * In this case, a smaller image buffer is allocated and you have to
 * update a partial display several times.
 * 1 byte = 8 pixels, therefore you have to set 8*N pixels at a time.
 */
unsigned char image[1024];
EPDPaint paint(image, 0, 0); // width should be the multiple of 8
EPD1in54 epd;                // default reset: 8, dc: 9, cs: 10, busy: 7

// D4 and D5
#define BUZZER_PIN PIN4
#define LED_PIN PIN5

#define THRESHOLD_PIN A4
AnalogPin thresholdPin(THRESHOLD_PIN);

#define TWO_LAP_MODE_PIN A0
#define UNUSED_TOGGLE_PIN A1

CapacitiveButton capacitiveButton = CapacitiveButton(A2, A3);

// screen size
#define SCR_WD 200
#define SCR_HT 200

int lapCount = 0;

void customRect(int x, int y, int w, int h, int c)
{
  int extra_x = x % 8;
  int new_x = x - extra_x;

  paint.setWidth(w + extra_x);
  paint.setHeight(h);

  paint.clear(UNCOLORED);

  paint.drawFilledRectangle(0, 0, w - 1 + extra_x, h - 1, c);
  epd.setFrameMemory(paint.getImage(), new_x, y, paint.getWidth(), paint.getHeight());
  return;
}

void rmInit()
{
  // init-RM

  /**
   *  there are 2 memory areas embedded in the e-paper display
   *  and once the display is refreshed, the memory area will be auto-toggled,
   *  i.e. the next action of SetFrameMemory will set the other memory area
   *  therefore you have to clear the frame memory twice.
   */
  epd.clearFrameMemory(0xFF); // bit set = white, bit reset = black
  epd.displayFrame();
  epd.clearFrameMemory(0xFF); // bit set = white, bit reset = black
  epd.displayFrame();

  paint.setRotate(ROTATE_0);
  paint.setWidth(200);
  paint.setHeight(24);

  /* For simplicity, the arguments are explicit numerical coordinates */
  paint.clear(COLORED);
  paint.drawStringAt(30, 4, "SwimmingPoolLapCounter", &Font16, UNCOLORED);
  epd.setFrameMemory(paint.getImage(), 0, 10, paint.getWidth(), paint.getHeight());

  paint.clear(UNCOLORED);
  paint.drawStringAt(30, 4, "by Rafal Magda", &Font16, COLORED);
  epd.setFrameMemory(paint.getImage(), 0, 30, paint.getWidth(), paint.getHeight());

  epd.displayFrame();
  delay(2000);

  // init-RM end

  /**
   *  there are 2 memory areas embedded in the e-paper display
   *  and once the display is refreshed, the memory area will be auto-toggled,
   *  i.e. the next action of SetFrameMemory will set the other memory area
   *  therefore you have to clear the frame memory twice.
   */
  epd.clearFrameMemory(0xFF); // bit set = white, bit reset = black
  epd.displayFrame();
  epd.clearFrameMemory(0xFF); // bit set = white, bit reset = black
  epd.displayFrame();

  font.setScale(4);
  font.setSpacing(4);
  font.printStr(ALIGN_CENTER, 10, "1234");
  epd.displayFrame();
  font.printStr(ALIGN_CENTER, 50, "5678");
  epd.displayFrame();
}

void incrementLapCount()
{
  if (digitalRead(TWO_LAP_MODE_PIN) == HIGH)
  {
    lapCount += 2;
  }
  else
  {
    lapCount++;
  }
}

// Use this function to configure the internal CapSense object to suit you. See the documentation at: http://playground.arduino.cc/Main/CapacitiveSensor
// This function can be left out if the defaults are acceptable - just don't call configureButton
void configureCapacitiveButton(CapacitiveSensor &capSense)
{
  // Set the capacitive sensor to timeout after 300ms
  capSense.set_CS_Timeout_Millis(300);
  // Set the capacitive sensor to auto-calibrate every 6secs
  capSense.set_CS_AutocaL_Millis(6000);
}
void dispNumber(int number);

// btn is a reference to the button that fired the event. That means you can use the same event handler for many buttons
void onButtonPressed(Button &btn)
{
  digitalWrite(LED_PIN, HIGH);    // LED on
  digitalWrite(BUZZER_PIN, HIGH); // buzzer on
}

void onReleaseCallbackFunction(Button &btn, uint_least16_t duration)
{
  incrementLapCount();
  Serial.print("button pressed ");
  Serial.println(lapCount);
  digitalWrite(BUZZER_PIN, LOW); // buzzer off
  dispNumber(lapCount);
  digitalWrite(LED_PIN, LOW); // LED off
  delay(250);
}

void onHoldCallbackFunction(Button &btn, uint_least16_t duration)
{
  digitalWrite(BUZZER_PIN, LOW); // buzzer off
  // btn is a reference to the button that was held
  // duration is how long the button was held for
  Serial.println("button LONG pressed ");
  lapCount = 0;
  dispNumber(lapCount);
  digitalWrite(LED_PIN, LOW); // LED off
  delay(250);
}

long threshold;
void setButtonThreshold()
{
  threshold = map(thresholdPin.readSmoothed(), 0, 1023, 1, 10000L);
  // Serial.print("threshold ");
  // Serial.println(threshold);
  capacitiveButton.setThreshold(threshold);
}

void setup()
{
  pinMode(THRESHOLD_PIN, INPUT);
  pinMode(UNUSED_TOGGLE_PIN, INPUT_PULLUP);
  pinMode(TWO_LAP_MODE_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  // put your setup code here, to run once:
  Serial.begin(9600);
  capacitiveButton.configureButton(configureCapacitiveButton);

  thresholdPin.setSmoothWeight(8);
  setButtonThreshold();

  capacitiveButton.setNumberOfSamples(25);
  capacitiveButton.onPress(onButtonPressed);
  if (capacitiveButton.onRelease(10, 3000 - 100, onReleaseCallbackFunction) != attSuccessful)
  {
    Serial.print("Button attach onRelease callback failed");
    return;
  }
  if (capacitiveButton.onHold(3000, onHoldCallbackFunction) != attSuccessful)
  {
    Serial.print("Button attach onHold callback failed");
    return;
  }

  if (epd.init(lutFullUpdate) != 0)
  {
    Serial.print("e-Paper init failed");
    return;
  }

  font.init(customRect, SCR_WD, SCR_HT); // custom fillRect function and screen width and height values
  // font.setColor(COLORED, UNCOLORED); //this clears the background, but is 2x slower
  font.setColor(COLORED); // this requires explicit cleanup
  font.setFont(&rre_term_10x16);

  // rmInit();

  dispNumber(0);
}

char buf[10];
void dispNumber(int number)
{
  // Serial.print("Disp number ");
  // Serial.print(number);
  // consider use of this every %10 laps, but then consider font.setColor(COLORED, UNCOLORED) near line 106
  epd.clearFrameMemory(0xFF); // bit set = white, bit reset = black

  itoa(number, buf, 10);

  if (0 <= number && number <= 9)
  {
    font.setFont(&rre_term_10x16); // set font resets spacing
    font.setScale(10, 10);
    font.printStr(ALIGN_CENTER, 20, buf);
  }
  else if (10 <= number && number <= 99)
  {
    font.setFont(&rre_term_10x16);
    font.setScale(8);
    font.setSpacing(4);
    font.printStr(ALIGN_CENTER, 40, buf);
  }
  else if (100 <= number && number <= 999)
  {
    font.setFont(&rre_term_10x16);
    font.setScale(5, 7);
    font.setSpacing(4);
    font.printStr(ALIGN_CENTER, 52, buf);
  }
  else if (1000 <= number && number <= 9999)
  {
    font.setFont(&rre_term_10x16);
    font.setScale(4, 6);
    font.setSpacing(4);
    font.printStr(ALIGN_CENTER, 48, buf);
  }
  else
  {
    font.setFont(&rre_term_10x16); // set font resets spacing
    font.setScale(2, 2);
    font.printStr(ALIGN_CENTER, 20, "out of range");
  }
  // Serial.println(" OK");
  epd.displayFrame();
}

void loop()
{
  setButtonThreshold();
  capacitiveButton.update();
}
