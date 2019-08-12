// MyProjecThing
// by Hyun & TOM

//#include <SPI.h>                // the SPI bus
//#include <Adafruit_GFX.h>       // core graphics library
//#include "Adafruit_HX8357.h"    // tft display local hacked version
//#include "Adafruit_STMPE610.h"  // touch screen local hacked version
//#include <Wire.h>               // I²C comms on the Arduino
//#include "IOExpander.h"         // unPhone's IOExpander (controlled via I²C)

#include "unphone.h"

//The library for sendin IR signal
#include <IRremote.h>
IRsend irsend;

// macros for debug calls to Serial.printf, with (D) and without (DD)
// new line, and to Serial.println (DDD)
bool DBG = false; // debug switch
bool switchColor = false; // changing the color of the switch button

// parent for device element classes
class Chunk {
  public:
    int begin();  // initialisation
    void test();  // validation
};

// esp32 feather pin definitions
#define TFT_DC   33

// calibration data for the raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750
#define MINPRESSURE 20
#define MAXPRESSURE 1000

// size of the color selection boxes and the paintbrush size
#define BOXSIZE 70
// #define PENRADIUS 3
#define PENRADIUS 9
// #define PENRADIUS 15
//The location of the button on the screen
#define HEIGHT 150
#define WIDTH 130


int buttoncount = 0;

// setup ////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  Serial.println("TV-B_GONE");
  Wire.setClock(100000);
  Wire.begin();
  IOExpander::begin();
  checkPowerSwitch(); // check if the power switch is now off & if so shutdown
  delay(100);

  tft.begin(HX8357D);
  TestScreen::init();
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, HIGH);
 

  if(! ts.begin()) { // init the touchscreen
  D("failed to start touchscreen controller");
   TestScreen::fail("TOUCH");
   delay(3000);
  } else {
   D("touchscreen started");
  }

   //Make the black screen
   tft.fillScreen(HX8357_BLACK);  

  //Message for indicating button (ON/OFF)
  tft.setCursor(120, 120);
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(3);
  tft.println("ON/OFF");
  //Making default green button for turning on the tv
  tft.fillRect(130, 150, BOXSIZE, BOXSIZE, HX8357_GREEN);

}


// loop /////////////////////////////////////////////////////////////////////
int loopiter = 0;
void loop(void) {
  checkPowerSwitch(); // check if the power switch is now off & if so shutdown

  // retrieve a point
  TS_Point p = ts.getPoint();
  
//  DBG = true;
//  if(loopiter++ % 1000 == 0) {
//    DD("p(%d/%d/%d); ", p.x, p.y, p.z)
//    m.test();
//  }

  //This is for refreshing button count. By writing number 0-9 in black color, we overwrite the previous button count written in yellow to clear the screen for displaying the next count
  for (int i = 0; i < 11; i++) {
    tft.setCursor(85, 85);
    tft.setTextColor(HX8357_BLACK);
    tft.setTextSize(2);
    tft.print("Button Count: "); tft.print(i); tft.println(i);
  }


  // scale from ~0->4000 to tft.width using the calibration #'s
  if (ts.touched()) { // Only activate the button when it is touched
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) { // Certain pressure
      //Get the x,y coordinate
      p.x = map(p.x, TS_MAXX, TS_MINX, tft.width(), 0);
      p.y = map(p.y, TS_MAXY, TS_MINY, 0, tft.height());
//      Serial.print("("); Serial.print(p.x);
//      Serial.print(", "); Serial.print(p.y);
//      Serial.println("END OF THE POINT");
      if (p.x < BOXSIZE + WIDTH  && p.x > BOXSIZE && p.y < HEIGHT + BOXSIZE  && p.y > HEIGHT) { // When the button is clicked
        if (switchColor) { // Change the color
	  //Serial.println("Green");
          tft.fillRect(130, 150, BOXSIZE, BOXSIZE, HX8357_GREEN); // Make a green button for TV on
          switchColor = !switchColor; // Change the color whenever the button is clicked

          //Vibrateion whenever click the button
          IOExpander::digitalWrite(IOExpander::VIBRATION, HIGH);
          delay(200);
          IOExpander::digitalWrite(IOExpander::VIBRATION, LOW);
          delay(200);
		
          //If button is clickd then button count should be increased
          buttoncount++;

          //For displaying updated button count
          tft.setCursor(85, 85);
          tft.setTextColor(HX8357_YELLOW);
          tft.setTextSize(2);
          tft.print("Button Count: "); tft.println(buttoncount);
        }
        else {
          //Serial.println("Red");
          tft.fillRect(130, 150, BOXSIZE, BOXSIZE, HX8357_RED); // Make a red button for TV off
          switchColor = !switchColor;

          IOExpander::digitalWrite(IOExpander::VIBRATION, HIGH);
          delay(200);
          IOExpander::digitalWrite(IOExpander::VIBRATION, LOW);
          delay(200);

          buttoncount++;

          tft.setCursor(85, 85);
          tft.setTextColor(HX8357_YELLOW);
          tft.setTextSize(2);
          tft.print("Button Count: "); tft.println(buttoncount);
        }
          // Sony protocol requires three repeats of each command
	  // But panasonic remote control don't need to repeat
  
          irsend.sendPanasonic(0x4004, 0xD00BCB1);  // Hex decmial has been used
          Serial.println("Signal is sent");

        delay(900); // Take some time for the next button activation
      } 
    }
  }
}
