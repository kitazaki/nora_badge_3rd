// Sample for MINI LED BADGE
// アナログ入力 → ランダム表示

// #include <M5Stack.h>  // M5Stackを使用する場合
// #include <M5StickC.h>  // M5StickCを使用する場合
#include <M5Atom.h>

#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
//#include "misakiUTF16.h"  // 日本語フォント

Adafruit_8x16matrix matrix = Adafruit_8x16matrix();
Adafruit_8x16matrix matrix1 = Adafruit_8x16matrix();

int analogIn = 32;

void setup() {
  M5.begin();
  Serial.begin(115200);

  // I2C初期化
  // Wire.begin(21, 22, 1000);  // M5Stack Grove G21: SDA, G22: SCL
  // Wire.begin(32, 33, 1000);  // M5StickC Grove G32: SDA, G33: SCL
  // Wire.begin(26, 32, 1000);  // M5Atom Grove G26: SDA, G32: SCL
  Wire.begin(25, 21, 1000);  // M5Atom Backside G25: SDA, G21: SCL

  // MINI LED BADGE初期化
  matrix.begin(0x71);
  matrix1.begin(0x70);
//  matrix.clear();
//  matrix1.clear();
  matrix.setBrightness(0);
  matrix1.setBrightness(0);
//  matrix.setTextWrap(false);
//  matrix1.setTextWrap(false);
//  matrix.setTextColor(LED_ON);
//  matrix1.setTextColor(LED_ON);
  matrix.setRotation(1);
  matrix1.setRotation(1);
}

void loop() {
  TestLEDMatrix();
}

void TestLEDMatrix(){
  matrix.clear();
  matrix1.clear();

  int n = analogRead(analogIn);
  //Serial.printf("R=%d\n",n);

  for (int x=0; x<16; x++) {
    for (int y=0; y <8; y++) {
      if (rand()%4095 < n) {
        matrix.drawPixel(x, y, LED_ON);
      } else {
        matrix.drawPixel(x, y, LED_OFF);
      }
      if (rand()%4095 < n) {
        matrix1.drawPixel(x, y, LED_ON);
      } else {
        matrix1.drawPixel(x, y, LED_OFF);
      }
    }
  }

  matrix.writeDisplay();
  matrix1.writeDisplay();
  //delay(20);
}
