
#include <SPI.h>
#include <EPD1in54.h>
#include <EPDPaint.h>

#include "RREFont.h"
#include "rre_fixed_8x16.h"

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

#define SCR_WD 200
#define SCR_HT 200

void customRect(int x, int y, int w, int h, int c)
{
  paint.setWidth(w);
  paint.setHeight(h);

  paint.clear(UNCOLORED);

  paint.drawFilledRectangle(0, 0, w - 1, h - 1, c);
  epd.setFrameMemory(paint.getImage(), x, y, paint.getWidth(), paint.getHeight());
  return;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);

  if (epd.init(lutFullUpdate) != 0)
  {
    Serial.print("e-Paper init failed");
    return;
  }

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

  font.init(customRect, SCR_WD, SCR_HT); // custom fillRect function and screen width and height values
  font.setFont(&rre_fixed_8x16);
  font.setColor(COLORED);
  font.setScale(4);
  font.setBold(1);
  // font.setSpacing(1);

  font.printStr(ALIGN_CENTER, 10, "1234"); // center
  epd.displayFrame();
  font.printStr(ALIGN_CENTER, 50, "5678"); // center
  epd.displayFrame();
}

void loop()
{

  // paint.setWidth(15);
  // paint.setHeight(15);

  // paint.clear(UNCOLORED);
  // // paint.drawStringAt(0, 4, time_string, &Font24, COLORED);

  // paint.drawFilledRectangle(2, 2, 10, 10, COLORED);
  // paint.drawPixel(1, 1, UNCOLORED);
  // paint.drawPixel(2, 2, UNCOLORED);
  // paint.drawPixel(3, 3, UNCOLORED);
  // paint.drawPixel(4, 4, UNCOLORED);
  // paint.drawPixel(5, 5, UNCOLORED);

  // // set first memory area
  // epd.setFrameMemory(paint.getImage(), 0, 0, paint.getWidth(), paint.getHeight());
  // epd.displayFrame();

  // // set second memory area
  // epd.setFrameMemory(paint.getImage(), 5, 50, paint.getWidth(), paint.getHeight());
  // epd.displayFrame();

  delay(2000);
}
