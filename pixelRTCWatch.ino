W// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

#include <TinyWireM.h>
#include "TinyRTClib.h"
#include <Adafruit_NeoPixel.h>

const short NEOPIXEL_PIN = 3;
const short  DELAY = 100;
const short  MAIN_BUTTON_PIN = 1;
const short  SECONDARY_BUTTON_PIN = 4;

const short  TIME_MIN = 0;
const short  TIME_HOUR = 1;

const short  MODE_TIME = 0;
const short  MODE_TIME_2 = 1;
const short  MODE_FLASHLIGHT =2;
const short  MODE_STROBELIGHT =3;
const short  MODE_RAINBOW =4;
const short  MODE_BRIGHTNESS=5;
const short  MODE_SETTIME_HOUR = 6;
const short  MODE_SETTIME_MIN = 7;

const short  LONG_PRESS_TIME = 13;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

RTC_DS1307 RTC;
short  watchMode = MODE_TIME;
short  lastWatchMode;
short  timeInBrightnessMode;
short  timeAfterLongPress;



void setup () {
    pinMode(MAIN_BUTTON_PIN, INPUT);
    pinMode(SECONDARY_BUTTON_PIN, INPUT);
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    strip.setBrightness(20);
    TinyWireM.begin();
    RTC.begin();

  //if (RTC.isrunning() == flase) {
    //successful = false;
    // following line sets the RTC to the date & time this sketch was compiled
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  //}
}

void loop () {
  switch (watchMode){
    case MODE_TIME:
      showTime(TIME_HOUR,true,MODE_TIME);
      if (watchMode == MODE_TIME)
        clearPixels(true);
      if (watchMode == MODE_TIME)
          showTime(TIME_MIN,true,MODE_TIME);
      if (watchMode == MODE_TIME)
          clearPixels(true);
    break;
    case MODE_TIME_2:
      showAllTime(MODE_TIME_2);
    break;
    case MODE_FLASHLIGHT:
        flashLight(false);
    break;
    case MODE_STROBELIGHT:
        flashLight(true);
    break;
    case MODE_BRIGHTNESS:
      showBrightness();
    break;

    case MODE_SETTIME_HOUR:
        showTime(TIME_HOUR,false,MODE_SETTIME_HOUR);
        specialDelay(5, MODE_SETTIME_HOUR);
        clearPixels(false);
        specialDelay(5, MODE_SETTIME_HOUR);
    break;
    case MODE_SETTIME_MIN:
        showTime(TIME_MIN,false,MODE_SETTIME_MIN);
        specialDelay(5, MODE_SETTIME_MIN);
        clearPixels(false);
        specialDelay(5, MODE_SETTIME_MIN);
    break;
    case MODE_RAINBOW:
        rainbow();
        if (okToContinue(MODE_RAINBOW))
            rainbowCycle();
    break;
    
  }
  
}

void specialDelay(short  numDelay, short  mode){
  for (short  i = 0; i<numDelay; i++){
      if (okToContinue(mode))
          delay(DELAY);
  } 
}

void showBrightness(){
    for(short  i=0; i<12; i++) {
      if (i<((strip.getBrightness()*12)/255)+1)
          strip.setPixelColor(i, 255, 255, 255);
      else
          strip.setPixelColor(i, 0, 0, 0);
      strip.show();
    }
    if (timeInBrightnessMode == 50){
        watchMode = lastWatchMode;
        timeInBrightnessMode = 0;
    }else
      timeInBrightnessMode ++;
    if (!buttonInterupted())
       delay(DELAY);
}

void flashLight(boolean strobe){
  for(short  i=0; i<12; i++)
      strip.setPixelColor(i, 255, 255, 255);
  strip.show();

  if (buttonInterupted()==false){
      if (strobe)
          delay(DELAY/3);
  }
  
  if (strobe)
      clearPixels(false);
  strip.show();
  if (buttonInterupted()==false){
      if (strobe)
          delay(DELAY/3);
      else
          delay(DELAY);
  }
}


boolean okToContinue(short  mode){
    if (!buttonInterupted() && watchMode ==mode)
        return true;
    return false;
}

boolean buttonInterupted(){
  if (timeAfterLongPress > 0)
          timeAfterLongPress--;
  if (digitalRead(MAIN_BUTTON_PIN) == HIGH){
      if (timeAfterLongPress == 0){
          short  tempPressTimeData =0;
          do{
            if (tempPressTimeData > LONG_PRESS_TIME){
              //longpress
              break;
            }
            strip.setPixelColor(tempPressTimeData, 100, 255, 255);
            strip.show();
            tempPressTimeData++;
            delay(DELAY);
          }while (digitalRead(MAIN_BUTTON_PIN) == HIGH);

         if (watchMode == MODE_SETTIME_HOUR || watchMode == MODE_SETTIME_MIN){
              if (watchMode == MODE_SETTIME_HOUR)
                  watchMode = MODE_SETTIME_MIN;
              else
                  watchMode = lastWatchMode;
         }else{
             if (tempPressTimeData > LONG_PRESS_TIME){
                 timeAfterLongPress = 30;
                 lastWatchMode = watchMode;
                 watchMode = MODE_SETTIME_HOUR;
             }else{
               if (watchMode<4)
                   watchMode++;
               else
                   watchMode = 0;
             }
         }
         return true;
      }
      return false;
  }else if (digitalRead(SECONDARY_BUTTON_PIN) == HIGH){
    if (watchMode == MODE_SETTIME_HOUR || watchMode == MODE_SETTIME_MIN){
            short  tempPressTimeData =0;
            do{
            if (tempPressTimeData > LONG_PRESS_TIME/2)//longpress
                break;
            tempPressTimeData++;
            delay(DELAY);
          }while (digitalRead(SECONDARY_BUTTON_PIN) == HIGH);
      
        DateTime now = RTC.now();
        if (watchMode == MODE_SETTIME_HOUR){
          short  h = now.hour();
          if (h>=24)
              h=0;
          RTC.adjust(DateTime(now.year(), now.month(), now.day(), h+1, now.minute(), now.second()));
        }else{
          short  m = now.minute();
          if (m>=59)
              m=0;
          RTC.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), m+2, now.second()));
        }
    }else{
        timeInBrightnessMode = 0;
        if (watchMode!=MODE_BRIGHTNESS){
            lastWatchMode = watchMode;
            watchMode = MODE_BRIGHTNESS;
            timeAfterLongPress = 20;
        }
        if (timeAfterLongPress==0){
          if (strip.getBrightness()>=270)
              strip.setBrightness(1);
          else
              strip.setBrightness(strip.getBrightness()+10);
          strip.show();
        }
    }
    return false;
  }
  else
      return false;
}

void showTime(short  what,boolean animate,short  mode){
  if (watchMode == mode){
      short  numPixelsToLight;
      DateTime now = RTC.now();
        
      switch(what){
        case TIME_MIN:
          numPixelsToLight = ((now.minute())*12)/59;
          break;
        case TIME_HOUR:
          numPixelsToLight = now.hour();
          if (numPixelsToLight == 0)
              numPixelsToLight =12;
          if (numPixelsToLight>12)
              numPixelsToLight -= 12;
          break;
      }
      
      for(short  i=0; i<numPixelsToLight; i++) {
          strip.setPixelColor(i, Wheel(i*256/12-what*100));
          strip.show();
          if (okToContinue(mode)){
              if (animate){
                  delay(DELAY);
              }
          }else
              break;
      }

      if (animate)
          specialDelay(30-numPixelsToLight, MODE_TIME);
  }
}

void showAllTime(short  mode){
    DateTime now = RTC.now();
    short  numHourPixels = now.hour();
    short  numMinPixels = ((now.minute())*12)/59;
    short  numSecondPixels = ((now.second())*12)/60;
    if (numSecondPixels==0)
        numSecondPixels =12;
    if (numHourPixels == 0)
        numHourPixels =12;
    if (numHourPixels>12)
        numHourPixels -= 12;

    short r,g,b;
    for(short  i=0; i<12; i++) {
      r = 0;
      g = 0;
      b = 0;
      if (i == numMinPixels-1)
          r = 255;
      if (i == numSecondPixels-1)
          g = 255;
      if (i==numHourPixels-1)
          b = 255;
          
      strip.setPixelColor(i, r,g,b);
  }
  if (okToContinue(mode))
      delay(DELAY);
  strip.show();
}



void rainbow() {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      if (watchMode != MODE_RAINBOW)
          break;
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    if (okToContinue(MODE_RAINBOW))
        delay(DELAY);
    else
        break;
  }
}

void rainbowCycle() {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
        if (watchMode != MODE_RAINBOW)
            break;
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    if (okToContinue(MODE_RAINBOW))
        delay(DELAY/2);
    else
        break;
  }
}


void clearPixels(boolean animate){
    for(short  i=0; i<12; i++) {
      strip.setPixelColor(i, 0,0,0);
      strip.show();
      if (!buttonInterupted()){
          if (animate){
              delay(DELAY);
          }
      }else
          break;
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
