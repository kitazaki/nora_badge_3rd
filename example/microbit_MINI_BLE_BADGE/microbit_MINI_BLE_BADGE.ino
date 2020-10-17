// MINI LED BADGE for micro:bit
// NORA-T002 / HT16K33-788AS Board

// LED pin デバッグ用LED　Pin
#define COL1      3
#define LED_PIN   26

// Import libraries (I2C 16x8 LED Matrix)
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include "misakiUTF16.h"  // for example

// init matrix (I2C address: 0x71) / matrix1 (I2C address: 0x70)
Adafruit_8x16matrix matrix = Adafruit_8x16matrix();
Adafruit_8x16matrix matrix1 = Adafruit_8x16matrix();

void setup() {
  Serial.begin(115200);  // BLE通信に必要
  Serial.println("MINI LED BADGE");
//  #if defined (__AVR_ATmega32U4__)
//    delay(5000);  //5 seconds delay for enabling to see the start up comments on the serial board
//  #endif

  // for debug: set LED pin to output mode
  pinMode(COL1, OUTPUT);
  digitalWrite(COL1, LOW);
  pinMode(LED_PIN, OUTPUT);

  Wire.begin();
  matrix.begin(0x71);
  matrix1.begin(0x70);
  // matrix.setTextSize(1);
  // matrix1.setTextSize(1);
  matrix.setBrightness(0);
  matrix1.setBrightness(0);
  matrix.setTextWrap(false);
  matrix1.setTextWrap(false);
  matrix.setTextColor(LED_ON);
  matrix1.setTextColor(LED_ON);
  matrix.setRotation(1);
  matrix1.setRotation(1);
}


void loop() {
  // poll peripheral
  // blePeripheral.poll();

  TestLEDMatrix();
  DebugLEDOnOff();
//  delay(100);
}


void DebugLEDOnOff(){
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);   
  delay(100);
}


// for example
void TestLEDMatrix(){
  uint8_t buf[8];
  char *str="日本語のテストです";
  for (int8_t x=17; x>=-88; x--) {
    char *ptr = str;
    uint16_t n = 0;
    matrix.clear();
    matrix1.clear();
    matrix.setCursor(x,0);
    matrix1.setCursor(x+16,0);
    while(*ptr){
      ptr = getFontData(buf,ptr,true);
      if(!ptr)
        break;
      matrix.drawBitmap(x+n,0,buf,8,8,1);
      matrix1.drawBitmap(x+16+n,0,buf,8,8,1);
      n+=8;
    }
    matrix.writeDisplay();
    matrix1.writeDisplay();
    delay(20);
  }
}
