// Sample for MINI LED BADGE

// #include <M5Stack.h>  // M5Stackを使用する場合
// #include <M5StickC.h>  // M5StickCを使用する場合
#include <M5Atom.h>

#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include "misakiUTF16.h"  // 日本語フォント

Adafruit_8x16matrix matrix = Adafruit_8x16matrix();
Adafruit_8x16matrix matrix1 = Adafruit_8x16matrix();

void setup() {
  M5.begin();
  Serial.begin(115200);

  // I2C初期化
  // Wire.begin(21, 22, 1000);  // M5Stack Grove G21: SDA, G22: SCL
  // Wire.begin(32, 33, 1000);  // M5StickC Grove G32: SDA, G33: SCL
  Wire.begin(26, 32, 1000);  // M5Atom Grove G26: SDA, G32: SCL
  // Wire.begin(25, 21, 1000);  // M5Atom Backside G25: SDA, G21: SCL

  // MINI LED BADGE初期化
  matrix.begin(0x71);
  matrix1.begin(0x70);
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
  TestLEDMatrix();
}

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
    //delay(20);
  }
}
