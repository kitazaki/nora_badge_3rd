// NORA Badge XIAO ESP32C3 Version
// NORA-T001 / HT16K33-788AS Board
// Adafruit LED Matrix backpacks

// Import libraries (ESP32 BLE for Arduino)
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <string.h>
#include "NoraPeri.h"

// LED indicator
int led = D10;  // XIAO ESP32C3はオンボードLEDが付いていないため、D10を使用

// Import libraries (I2C Adafruit LED Matrix backpacks)
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <misakiUTF16.h>  // for example

Adafruit_8x16matrix matrix = Adafruit_8x16matrix();
Adafruit_8x16matrix matrix1 = Adafruit_8x16matrix();

#define  SCREENS          4   // LED Matrixカスケード接続数
// B1144format Header
#define  ONE_CHAR_SIZE    11  //1文字分の文字データサイズ
#define  MAX_WRITE_LENGTH 16  //BLE Writeデータ最大サイズ

/* 左、右、固定スクロール対応 Y.Iida */
#define MODE_LEFT         0
#define MODE_RIGHT        1

#define MAXSCREENS 4  // 最大接続数
static uint8_t buf[8*MAXSCREENS];
uint8_t* MAX7219_getBuffer() {
  return buf;  
}

static BLEUUID ledService(BLE_SERVICE_UUID);
static BLEUUID ledCharService(BLE_CHARACTERISTIC_SERVICE_UUID);
static BLEUUID ledCharData(BLE_CHARACTERISTIC_DATA_UUID);

// create switch characteristic
unsigned char num                           = 16;     //
int MaxWriteLen                             = 16;     //Write受信サイズ
unsigned char LED_ReceiveValue[16]          = "";     //Write受信データ
int           LED_ReceiveLength             = 0;      //Write受信データサイズ
int           ConcateCount                  = 0;      //Write連結数(ヘッダ含む)
uint8_t       LED_payload[1408]             = {};     //ペイロード配列
unsigned int  B1122_CharLength              = 0;      //ペイロードの文字数
unsigned int  B1122_PayloadLines            = 0;      //ペーロード自体の数
unsigned int  MAX7219__CharLength           = 0;      //ペイロードの文字数(LEDボード入力用)
bool          isWritting                    = false;  //Writeイベント発生フラグ
int           margecount                    = 0;      //マージ用ループカウンター
int           PayloadLineCounter            = 0;      //ペイロードカウント
int           SurplusLength                 = 0;      //文字格納余り
int           WritePayloadSize              = 0;      //ペーロードのサイズ ( 11 * 文字数 )
uint8_t       MAX7219__payload[1024]        = {};     //文字配列(LED出力用)

uint8_t       display_Flash                 = 1;      //フラッシュ(点滅)設定
uint8_t       display_Marqee                = 0;      //マーキー設定
uint8_t       display_speed_mode            = 0x60;   //表示スピード(1-8)＆モード
                                                      // XIAO ESP32C3はスクロールスピードが速すぎるので値を80→60へ変更
uint8_t       save_Flash                    = 0;      //フラッシュ(点滅)設定
uint8_t       save_Marqee                   = 1;      //マーキー設定
uint8_t       save_speed_mode               = 0x71;   //表示スピード＆モード

int           blanklength     = 32;

//BLECharacteristic switchCharacteristic = BLECharacteristic(ledCharService,
//                                         BLECharacteristic::PROPERTY_READ |
//                                         BLECharacteristic::PROPERTY_WRITE |
//                                         BLECharacteristic::PROPERTY_NOTIFY |
//                                         BLECharacteristic::PROPERTY_INDICATE
//                                         );

//BLEDescriptor switchDescriptor = BLEDescriptor(ledCharData, num);

void bdaDump(esp_bd_addr_t bd) {
    for (int i = 0; i < ESP_BD_ADDR_LEN; i++) {
        Serial.printf("%02x", bd[i]);
        if (i < ESP_BD_ADDR_LEN - 1) {
            Serial.print(":");
        }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t* param) {
        Serial.print("connected from: ");
        bdaDump(param->connect.remote_bda);
        Serial.println("");
    };

    void onDisconnect(BLEServer* pServer) {
        Serial.println("disconnected");
    }
};

//class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
//    void onRead(BLECharacteristic* pCharacteristic) {
//        Serial.println("Characteristic read:");
//    };
//    void onWrite(BLECharacteristic* pCharacteristic) {
//        Serial.println("Characteristic write:");
//    };
//};


class switchCharacteristicWritten: public BLECharacteristicCallbacks {
  
  void onRead(BLECharacteristic *characteristic) {
      Serial.println(F("Characteristic read:"));
  }
  
  void onWrite(BLECharacteristic *characteristic) {

    // central wrote new value to characteristic, update LED
    Serial.println(F("Characteristic write:"));
  
    //Write受信データサイズ
    LED_ReceiveLength = characteristic->getValue().length();

    //Writeデータ最大サイズ(16)回ループ
    LED_ReceiveValue[16] = {};
    memset(LED_ReceiveValue, '¥0', sizeof(LED_ReceiveValue));
    for(int loopcount = 0; loopcount < 16; loopcount++){
      LED_ReceiveValue[loopcount] = characteristic->getValue().data()[loopcount];    
    }

    //ヘッダとペイロード切り分け、ヘッダは必要部分だけ抽出。ペイロードは配列に格納。
    if (LED_ReceiveValue[0] == 0x77) {
      isWritting          = true;
      ConcateCount        = 0;
      PayloadLineCounter  = 0;
      save_Flash          = LED_ReceiveValue[6];
      save_Marqee         = LED_ReceiveValue[7];
      save_speed_mode     = LED_ReceiveValue[8];
      //DebugLEDOnOff();
      //ヘッダ抽出 ※後で配列でなく変数にする。 
      
    } else {
      //Debug
      //DebugLEDOnOff();
    
      //ヘッダ抽出続き
      if(ConcateCount == 1){
        //ヘッダから文字数を取得
        B1122_CharLength = LED_ReceiveValue[1];
        //ペーロードのサイズを算出 ( 文字数 * 11 )
        WritePayloadSize   = B1122_CharLength * ONE_CHAR_SIZE;
      
        //ペイロードの数 16byteのペイロードの数を算出
        //文字データ数(文字数x11Byte) / 16 + 1(あまりが出た時)
        B1122_PayloadLines = WritePayloadSize / MAX_WRITE_LENGTH;
        SurplusLength = WritePayloadSize % MAX_WRITE_LENGTH;
        if( SurplusLength != 0){
          B1122_PayloadLines = B1122_PayloadLines + 1;
        }

        //配列初期化(文字数 x 11)
        //LED_payload[WritePayloadSize] = {};
        memset(LED_payload, '¥0', sizeof(LED_payload));
      //ペイロード格納
      } else if (ConcateCount >= 4) {
      
        //最終行のあまりがでるまでPayloadをマージ格納
        for(margecount = 0; margecount < MAX_WRITE_LENGTH; margecount++){
          //ペイロードカウントがMAXペイロード数未満
          if((PayloadLineCounter < B1122_PayloadLines)){
            LED_payload[(PayloadLineCounter * MAX_WRITE_LENGTH) + margecount] = LED_ReceiveValue[margecount];
          } else {
            //最後のPayload
            //ループカウンタがデータが埋まってるところまでデータ移行
            if(margecount < (MAX_WRITE_LENGTH - SurplusLength)){
              LED_payload[(PayloadLineCounter * MAX_WRITE_LENGTH) + margecount] = LED_ReceiveValue[margecount];
            }       
          }
        }
        PayloadLineCounter++;
      }
      //
      /**/
      //連結カウンタが最大値(全データ受信)
      if(PayloadLineCounter == B1122_PayloadLines){  
        int charcount        = 0;  //文字byteデータ格納ループカウンタ
        int B1144dotcout     = 0;  //1文字(11byte)カウンタ
        int MAX7219dotcout   = 0;  //1文字(8byte)カウンタ
        //文字列(文字数x8)分の配列初期化
        //MAX7219__payload[B1122_CharLength * 8] = {};
        memset(MAX7219__payload, '¥0', sizeof(MAX7219__payload));
        //文字数分ループ
        //
        //Debug ダミーデータ(NORA)を格納。
        /**
        MAX7219__payload[5*8] = {};
        LED_payload[55] = {};
        uint8_t deb[55] = {0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 
                           0x00, 0x00, 0xfe, 0xc0, 0xc0, 0xfe, 0xc0, 0xc0, 0xc0, 0xfe, 0x00, 
                           0x00, 0x00, 0xfe, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0xc6, 0x00, 
                           0x00, 0x00, 0xc6, 0xee, 0xfe, 0xd6, 0xc6, 0xc6, 0xc6, 0xc6, 0x00,
                           0x00, 0x00, 0x7C, 0x00, 0xFE, 0x10, 0x10, 0x10, 0x20, 0x40, 0x00};
        for(int t=0;t<55;t++){
          LED_payload[t] = deb[t];
        }
        B1122_CharLength = 5;
        /**/
        //Debug
        //

        int display_mode = save_speed_mode & 0x0F;
        if(display_mode == MODE_RIGHT){
          for(int blankcount = 0; blankcount < blanklength; blankcount++){
            MAX7219__payload[ MAX7219dotcout] = 0x00;
            MAX7219dotcout++;
          }        
        }
        for(charcount = 0; charcount < B1122_CharLength; charcount++){
          //11byte分ループ
          for(B1144dotcout = 0; B1144dotcout < ONE_CHAR_SIZE; B1144dotcout++){
            //上2列、下1列は格納しない。
            if((B1144dotcout != 0) && (B1144dotcout != 1) && (B1144dotcout != 10) ){
              MAX7219__payload[MAX7219dotcout] = LED_payload[(charcount*11) + B1144dotcout];
              MAX7219dotcout++;
            }
          }
        }
        if(display_mode != MODE_RIGHT){
          for(int blankcount = 0; blankcount < blanklength; blankcount++){
            MAX7219__payload[ MAX7219dotcout ] = 0x00;
            MAX7219dotcout++;
          }
        }
        //文字列長差し替え
        MAX7219__CharLength = B1122_CharLength + 4;
        display_Flash       = save_Flash;
        display_Marqee      = save_Marqee;
        display_speed_mode  = save_speed_mode;
        //書き込み中フラグをオフ
        isWritting = false;
      }
    }
    ConcateCount++;
    //delay(50);
  }
};


void setup() {
  Wire.begin();  // 引数はあとで調べる
  Serial.begin(115200);  // BLE通信に必要
  pinMode(led, OUTPUT);  // LED indicator

//  BLEDevice::init("LSLED");
  BLEDevice::init(BLE_DEVICE_NAME);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pLedService = pServer->createService(ledService);
  pLedService->start();

  BLEService *pService = pServer->createService(ledCharService);
  
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         ledCharData,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );

//  pCharacteristic->addDescriptor(&switchDescriptor);

  pCharacteristic->setCallbacks(new switchCharacteristicWritten());

//  pService->addCharacteristic(&switchCharacteristic);

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  BLEAdvertisementData advertisementData;
  advertisementData.setManufacturerData("NORA");
  pAdvertising->setAdvertisementData(advertisementData);
  pAdvertising->addServiceUUID(ledService);
  pAdvertising->addServiceUUID(ledCharService);
  pAdvertising->setScanResponse(true);  // false にするとServiceUUIDが表示されない
//  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

  int DefaultLength = 32;
  memset(MAX7219__payload, '¥0', sizeof(MAX7219__payload));
  uint8_t base[DefaultLength] = {0xc6, 0xe6, 0xe6, 0xf6, 0xde, 0xce, 0xce, 0xc6, 
                                 0xfe, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xfe, 
                                 0xfe, 0xc6, 0xc6, 0xfe, 0xd8, 0xcc, 0xc6, 0xc6, 
                                 0xfe, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0xc6
                                 };
  for(int basecount = 0; basecount < DefaultLength; basecount++){
    MAX7219__payload[basecount] = base[basecount];
  }
  MAX7219__CharLength = 4;
  
  matrix.begin(0x71);  // pass in the address
  matrix1.begin(0x70);  // pass in the address
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

  if(isWritting == false){
    MAX7219Display(MAX7219__payload, MAX7219__CharLength, display_Flash, display_Marqee, display_speed_mode);
    // TestLEDMatrix();
    DebugLEDOnOff();
//    delay(100);
  }
}


void MAX7219Display(uint8_t *dotArray,int charlen, uint8_t Flash, uint8_t Marquee, uint8_t SpMode){
  uint8_t  font[8];
  uint8_t* ptr = MAX7219_getBuffer();
  uint8_t  Mode = 0;
  uint8_t  Speed = 0;
  uint8_t* WkdotArray;
  uint8_t  wkbuff[8*4];

  memset(wkbuff, '¥0', sizeof(wkbuff));

  // モードの取得
  Mode = SpMode & 0x0F;
  // スピードの取得
  Speed = SpMode >> 4;
  Speed = Speed & 0x0F;

  /* LEDの表示を初期化する */
  // MAX7219_clear();
  
  // 4文字以下なら4文字にする
  if(charlen < 4){
    memcpy(wkbuff, dotArray, charlen*8);
    charlen = 4;
    WkdotArray = wkbuff;
  }
  else{
    WkdotArray = dotArray;    
  }
 
  /* スクロール表示 */
  // 右スクロール
  if(Mode == MODE_RIGHT){
    for (int8_t x=-8*charlen-16; x<=17; x++) {
      uint16_t n = 0;
      matrix.clear();
      matrix1.clear();
      matrix.setCursor(x,0);
      matrix1.setCursor(x+16,0);
      for(int Mcount = 0; Mcount < charlen; Mcount++){
        memcpy(font, WkdotArray+(Mcount*8), 8);
        matrix.drawBitmap(x+n,0,font,8,8,1);
        matrix1.drawBitmap(x+16+n,0,font,8,8,1);
        n+=8;
      }
      matrix.writeDisplay();
      matrix1.writeDisplay();
      delay(100+((3-Speed)*20));
    }
  }
  // 左スクロール
  else{
    for (int8_t x=17; x>=-8*charlen-16; x--) {
      uint16_t n = 0;
      matrix.clear();
      matrix1.clear();
      matrix.setCursor(x,0);
      matrix1.setCursor(x+16,0);
      for(int Mcount = 0; Mcount < charlen; Mcount++){
        memcpy(font, WkdotArray+(Mcount*8), 8);
        matrix.drawBitmap(x+n,0,font,8,8,1);
        matrix1.drawBitmap(x+16+n,0,font,8,8,1);
        n+=8;
      }
      matrix.writeDisplay();
      matrix1.writeDisplay();
      delay(100+((3-Speed)*20));
    }
  }

  return;
}


void DebugLEDOnOff(){
  digitalWrite(led, HIGH);
  delay(100);
  digitalWrite(led, LOW);   
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
    //delay(20);
  }
}
