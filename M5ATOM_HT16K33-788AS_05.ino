// NORA Badge M5Atom Version
// NORA-T001 / HT16K33-788AS Board
// Adafruit LED Matrix backpacks
// NimBLE library version
// WiFi enabled
// MQTT Subscriber

#include <M5Atom.h>

// Import libraries (NimBLE for Arduino)
#include <NimBLEDevice.h>

#include <string.h>
#include "NoraPeri.h"

// Import WiFiClient and PubSubClient
#include <WiFiClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

char *ssid = "";  // Wi-Fi SSID
char *password = "";  // Wi-Fi Password
const char *endpoint = "";  // MQTT Broker
const char *mqtt_username = "";  // MQTT ID
const char *mqtt_password = "";  // MQTT Password
const int port = 1883;  // MQTT Port
char *deviceID = "M5Atom" __DATE__ __TIME__;
char *pubTopic = "M5Atom";
char *subTopic = "message";

WiFiClient httpsClient;
PubSubClient mqttClient(httpsClient);

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

static NimBLEUUID ledService(BLE_SERVICE_UUID);
static NimBLEUUID ledCharService(BLE_CHARACTERISTIC_SERVICE_UUID);
static NimBLEUUID ledCharData(BLE_CHARACTERISTIC_DATA_UUID);

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
uint8_t       display_speed_mode            = 0x80;   //表示スピード(1-8)＆モード

uint8_t       save_Flash                    = 0;      //フラッシュ(点滅)設定
uint8_t       save_Marqee                   = 1;      //マーキー設定
uint8_t       save_speed_mode               = 0x71;   //表示スピード＆モード

int           blanklength     = 32;


class MyServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
        Serial.print("connected from: ");
        Serial.print(NimBLEAddress(desc->peer_id_addr).toString().c_str());
        Serial.println("");
    };
    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("disconnected");
    }
};


class switchCharacteristicWritten: public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic *characteristic) {
    Serial.println(F("Characteristic read:"));
  }
  void onWrite(NimBLECharacteristic *characteristic) {
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
  M5.begin(true, true, true); // default Serial: true, I2C: true, display: false
  Wire.begin(25, 21, 1000);  // Backside G25: SDA, G21: SCL
  // Wire.begin(26, 32, 1000);  // Grove G26: SDA, G32: SCL
  Serial.begin(115200);  // BLE通信に必要
//  M5.dis.setBrightness(3);  // >= 3

//  M5.dis.setBrightness(3);  // >= 3
//  M5.dis.drawpix(0, 0xf00000); 
  M5.dis.setBrightness(10);  // >= 3
  M5.dis.drawpix(0, 0, 0xf00000);
  matrix.begin(0x71);  // pass in the address
  matrix1.begin(0x70);  // pass in the address
  matrix.clear();
  matrix1.clear();
  matrix.setBrightness(0);
  matrix1.setBrightness(0);
  matrix.setTextWrap(false);
  matrix1.setTextWrap(false);
  matrix.setTextColor(LED_ON);
  matrix1.setTextColor(LED_ON);
  matrix.setRotation(1);
  matrix1.setRotation(1);
  
//  NimBLEDevice::init("LSLED");
  NimBLEDevice::init(BLE_DEVICE_NAME);
  NimBLEServer *pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService *pLedService = pServer->createService(ledService);
  pLedService->start();

  NimBLEService *pService = pServer->createService(ledCharService);
  NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                            ledCharData,
                                            NIMBLE_PROPERTY::READ |
                                            NIMBLE_PROPERTY::WRITE |
                                            NIMBLE_PROPERTY::NOTIFY |
                                            NIMBLE_PROPERTY::INDICATE
                                          );
  pCharacteristic->setCallbacks(new switchCharacteristicWritten());
  pService->start();

//  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
//  // pAdvertising->addServiceUUID(BLE_SERVICE_UUID);  //  LEDバッジアプリから認識されないのでコメントアウト
//  // pAdvertising->addServiceUUID(BLE_CHARACTERISTIC_SERVICE_UUID);
//  pAdvertising->addServiceUUID("fee7");  // BLE_SERVICE_UUID, BLE_CHARACTERISTIC_SERVICE_UUIDを指定するとUUIDが128bit形式で広報される(2個広報しても1個しか広報されない)ので、LEDバッジアプリから認識されない
//  pAdvertising->addServiceUUID("fee0");  // UUIDが16bit形式であれば2個広報されて、LEDバッジアプリから認識される

  NimBLEAdvertising *pAdvertising = pServer->getAdvertising();
  NimBLEAdvertisementData advertisementData;
  advertisementData.setManufacturerData("NORA");
  advertisementData.setCompleteServices(NimBLEUUID("fee7"));
  advertisementData.setCompleteServices(NimBLEUUID("fee0"));
  pAdvertising->setAdvertisementData(advertisementData);

  pAdvertising->setScanResponse(true);
  pAdvertising->start();
  
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

  // Start WiFi
  Serial.println("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  int lpcnt=0 ;
  int lpcnt2=0 ;
      
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lpcnt += 1 ;                           //
    if (lpcnt > 6) {                       // 6回目(3秒) で切断/再接続
      WiFi.disconnect(true,true);          //
      WiFi.begin(ssid, password);    //
      lpcnt = 0 ;                          //
      lpcnt2 += 1 ;                        // 再接続の回数をカウント
    }                                      //
    if (lpcnt2 > 3) {                      // 3回 接続できなければ、
      ESP.restart() ;                      // ソフトウェアリセット
    }             
  }
 
  // WiFi Connected
  Serial.println("\nWiFi Connected.");
//  TestLEDMatrix("WiFi Connected.");
      
  //mqttClient.setBufferSize(2048);
  mqttClient.setServer(endpoint, port);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(2048);
    
  connectMQTT();
//  TestLEDMatrix("MQTT Connected.");
}


long messageSentAt = 0;
int count = 0;
char pubMessage[128];
int led,red,green,blue;

//////////////////////////////////////////////////////

void loop() {
  // poll peripheral
  // blePeripheral.poll();

  mqttLoop();

  long now = millis();
  if (now - messageSentAt > 5000) {
      messageSentAt = now;
      sprintf(pubMessage, "{\"count\": %d}", count++);
      Serial.print("Publishing message to topic ");
      Serial.println(pubTopic);
      Serial.println(pubMessage);
      mqttClient.publish(pubTopic, pubMessage);
      Serial.println("Published.");
  }
  
  if(isWritting == false){
    //Serial.print("display_speed_mode: ");
    //Serial.println(display_speed_mode);
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
  // digitalWrite(LED_PIN, HIGH);
  // delay(100);
  // digitalWrite(LED_PIN, LOW);   
  // delay(100);
  M5.dis.drawpix(0, 0xf00000); 
  delay(100);
  M5.dis.drawpix(0, 0x000000); 
}

void DebugLEDOnOff_G(){
  // digitalWrite(LED_PIN, HIGH);
  // delay(100);
  // digitalWrite(LED_PIN, LOW);   
  // delay(100);
  M5.dis.drawpix(0, 0xf00000); 
  delay(100);
  M5.dis.drawpix(0, 0x000000); 
}

void DebugLEDOnOff_R(){
  // digitalWrite(LED_PIN, HIGH);
  // delay(100);
  // digitalWrite(LED_PIN, LOW);   
  // delay(100);
  M5.dis.drawpix(0, 0x00f000); 
  delay(100);
  M5.dis.drawpix(0, 0x000000); 
}


void connectMQTT() {
    while (!mqttClient.connected()) {
        if (mqttClient.connect(deviceID, mqtt_username, mqtt_password)) {
            Serial.println("Connected.");
            int qos = 0;
            mqttClient.subscribe(subTopic, qos);
            Serial.println("Subscribed.");
        } else {
            Serial.print("Failed. Error state=");
            Serial.println(mqttClient.state());
            delay(5000);
        }
    }
}

   
void mqttCallback (char* topic, byte* payload, unsigned int length) {
 
    String str = "";
    Serial.print("Received. topic=");
    Serial.println(topic);
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        str += (char)payload[i];
    }
    Serial.print("\n");
 
    StaticJsonBuffer<200> jsonBuffer;
     
    JsonObject& root = jsonBuffer.parseObject(str);
   
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }
    const char* message = root["message"];
    led = root["led"];
    red = root["r"];
    green = root["g"];
    blue = root["b"];
 
    Serial.print("red = ");
    Serial.print(red);
    Serial.print(" green = ");
    Serial.print(green);
    Serial.print(" blue = ");
    Serial.println(blue);
 
    if( led == 1 ){
      DebugLEDOnOff_G();
      uint16_t RGB = ((red>>3)<<11) | ((green>>2)<<5) | (blue>>3);
      // TestLEDMatrix(message);
      TestLEDMatrix2(message);
    } else {
      DebugLEDOnOff_R();
    }
 
    delay(300);
     
}
  
void mqttLoop() {
    if (!mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop();
}
 

void TestLEDMatrix(const char* message){
  uint8_t buf[8];
  //char *str="日本語のテストです";
  char *str = const_cast <char*> (message);
  int len = strlen(str);
  Serial.print("message length: ");
  Serial.println(len);

  char *ptr = str;
  int char_count = 0;
  while(*ptr){
    char_count++;
    ptr = getFontData(buf,ptr,true);
    if(!ptr)
      break;
  }
  Serial.print("char count: ");
  Serial.println(char_count);
  
  for (int16_t x=17; x>=-(char_count+2)*8; x--) {
    //Serial.println(x);
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

void TestLEDMatrix2(const char* message){
  uint8_t buf[8];
  //char *str="日本語のテストです";
  char *str = const_cast <char*> (message);
  int len = strlen(str);
  Serial.print("message length: ");
  Serial.println(len);

  memset(MAX7219__payload, '¥0', sizeof(MAX7219__payload));

  char *ptr = str;
  uint8_t char_count = 0;
  while(*ptr){
    ptr = getFontData(buf,ptr,true);
    for (int i=0; i<8; i++) {
      // Serial.println(buf[i]);
      MAX7219__payload[char_count*8+i] = buf[i];
    }
    char_count++;
    if(!ptr)
      break;
  }
  MAX7219__CharLength = char_count;
  Serial.print("char count: ");
  Serial.println(char_count);  
}
