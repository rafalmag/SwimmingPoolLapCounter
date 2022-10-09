
#include <SPI.h>
#include <EPD1in54.h>
#include <EPDPaint.h>

#include "RREFont.h"
#include "rre_term_10x16.h"

#include <CapacitiveSensor.h>
#include <Button.h>
#include <ButtonEventCallback.h>
#include <CapacitiveButton.h>

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
#define BUZZER_PIN 4
#define LED_PIN 5

// A4 and A5
CapacitiveSensor   cs_4_2 = CapacitiveSensor(A4,A2);

// CapacitiveButton button1 = CapacitiveButton(A4, A2);

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

// Use this function to configure the internal CapSense object to suit you. See the documentation at: http://playground.arduino.cc/Main/CapacitiveSensor
// This function can be left out if the defaults are acceptable - just don't call configureButton
void configureCapacitiveButton(CapacitiveSensor& capSense){

        // Set the capacitive sensor to timeout after 300ms
        capSense.set_CS_Timeout_Millis(300);
        // Set the capacitive sensor to auto-calibrate every 10secs
        capSense.set_CS_AutocaL_Millis(10000);
}

// btn is a reference to the button that fired the event. That means you can use the same event handler for many buttons
void onButtonPressed(Button& btn){
lapCount++;
        Serial.print("button pressed ");
        Serial.println(lapCount);

}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);

  cs_4_2.set_CS_AutocaL_Millis(6000);

  // cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);
  // cs_4_2.set_CS_AutocaL_Millis(300);
  // button1.configureButton(configureCapacitiveButton);
  // button1.setThreshold(4000);      // More sensitive
  // button1.setNumberOfSamples(30);
  // button1.onPress(onButtonPressed);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

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

  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
}

char buf[10];
int randNumber;

void loop()
{
long start = millis();
    long total1 =  cs_4_2.capacitiveSensor(25);

    Serial.print(millis() - start);        // check on performance in milliseconds
    Serial.print("ms \t");                    // tab character for debug windown spacing

    Serial.println(total1);
    if(total1>3000){
      digitalWrite(LED_PIN, HIGH); // LED on
     delay(250);   
      digitalWrite(LED_PIN, LOW); // LED off
      delay(250);   
    }

     delay(20);  


  // button1.update();
                           // arbitrary delay to limit data to serial port 

/*  
  Serial.print("\nNew value:");
digitalWrite(LED_PIN, HIGH); // LED on
digitalWrite(BUZZER_PIN, HIGH); // buzzer on


// consider use of this every %10 laps, but then consider font.setColor(COLORED, UNCOLORED) near line 106
  epd.clearFrameMemory(0xFF); // bit set = white, bit reset = black

  randNumber = random(1, 6); // inclusive, exclusive
  memset(buf, 0x00, sizeof(buf));
  if (randNumber == 1)
  {
    itoa(random(1, 4), buf, 10);
    Serial.print(buf);
    font.setFont(&rre_term_10x16); // set font resets spacing
    font.setScale(10, 10);
    font.printStr(ALIGN_CENTER, 20, buf);
  }
  else if (randNumber == 2 || randNumber == 5)
  {
    itoa(random(10, 99), buf, 10);
    Serial.print(buf);
    font.setFont(&rre_term_10x16);
    font.setScale(8);
    font.setSpacing(4); 
    font.printStr(ALIGN_CENTER, 40, buf);
  }
  else if (randNumber == 3)
  {
    itoa(random(100, 999), buf, 10);
    Serial.print(buf);
    font.setFont(&rre_term_10x16);
    font.setScale(5, 7);
    font.setSpacing(4);
    font.printStr(ALIGN_CENTER, 52, buf);
  }
  else if (randNumber == 4)
  {
    itoa(random(1000, 9999), buf, 10);
    Serial.print(buf);
    font.setFont(&rre_term_10x16);
    font.setScale(4, 6);
    font.setSpacing(4);
    font.printStr(ALIGN_CENTER, 48, buf);
  }
  Serial.print(" OK");
  epd.displayFrame();
  digitalWrite(LED_PIN, LOW); // LED off
  digitalWrite(BUZZER_PIN, LOW); // buzzer on
  delay(3000);
  */
}
