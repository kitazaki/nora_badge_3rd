// #include <BLEPeripheral.h>

/* BLEデバイス定義 */
#define BLE_DEVICE_NAME "LSLED"
#define BLE_SERVICE_UUID "0000fee7-0000-1000-8000-00805f9b34fb"
#define BLE_CHARACTERISTIC_SERVICE_UUID "0000fee0-0000-1000-8000-00805f9b34fb" 
#define BLE_CHARACTERISTIC_DATA_UUID "0000fee1-0000-1000-8000-00805f9b34fb"

/* b1144フォーマット定義 */
#define B1144_HEADER_KEY  "wang"
#define B1144_FLASH_POS 6
#define B1144_MARQEE_POS 7
#define B1144_SPEED_MODE_POS 8
#define B1144_LENGTH_POS 16
#define B1144_DATE_POS 38
#define B1144_DATA_POS 64
#define B1144_SEPARATE_SIZE 16
#define B1144_1CHAR_SIZE 11
#define B1144_TIMEOUT 100000
#define MAX7219_1CHAR_SIZE 8
#define DEVICENUM_MAX  16
