#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <driver/rtc_io.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_WRITE_UUID "b62c1ffa-bdd8-46ea-a378-d539cf405d93"
#define CHARACTERISTIC_READ_UUID "c8f8d86f-f03a-428f-8917-39384ad98e4b"

//保持される変数
RTC_DATA_ATTR int bootCounter = 0;
RTC_DATA_ATTR bool state = true;

//ピンを設定
const int RED = A18;  // pin -> 25
const int GREEN = A19;// pin -> 26
const int BLUE = A17; // pin -> 27

const int BUTTON = 13;

// 10/8 xとzのピンを逆転
// 新基盤で変更するため
const int pinX = 34;
const int pinY = 35;
const int pinZ = 32;

// 地球の重力である1Gの加速度(m/s^2)
float ms2 = 9.8;

// 電源電圧5V時のオフセット電圧(0G = 2.5V = 2500mV)
float offset_voltage = 2290.0; 

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristicWrite = NULL;
BLECharacteristic* pCharacteristicRead = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

String useExtensions;

// 接続状態が変更されたら呼ばれる
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      useExtensions = "";
    }
};

// リポバッテリーの電圧を計算
float getBattery(){
  int d = analogRead(33);
  
  const int R1 = 10000; // 10kΩ
  const int R2 = 10000; // 10kΩ
  
  return d * (R1+R2) / R2 * (3.7/4096);
}

// タブレットの設定からBluetoothを接続しなおしたらつながった?
// write or read の callback
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
      
      std::string stdValue = pCharacteristic->getValue();
      String value = stdValue.c_str();
      
      Serial.println(value);

      if (value.equals("battery")) {
        String battery = (String) getBattery();
        
        pCharacteristicWrite->setValue(battery.c_str());
        pCharacteristicWrite->notify();
      } else {
        // 拡張機能用の変数
      useExtensions = value;
      }
    }
};

String getCalcuAccel(float d) {
  // 電圧を計算
  float volt = (d / 4096.0) * 5.0 * 1000;

  // オフセット電圧を引いた電圧を求める
  volt = volt - offset_voltage;

  // 重力を求める
  float gravity = volt / 1000.0;

  // 重力から加速度を算出
  float accel = gravity * ms2;

  // 0.12345の形なので1000倍する
  int accelResult = round(accel * 1000);

  // 小数点を切り捨てて型変換
  String result = (String)accelResult;

  return result;
}

// 圧力から電圧を計算
double getVoltage(int d) {
  double R = 10; // 10k Ω

  double volt = d * 3.3 / 4096;

  return volt;
}

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case 2  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 3  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 4  : Serial.println("Wakeup caused by timer"); break;
    case 5  : Serial.println("Wakeup caused by touchpad"); break;
    case 6  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}

void startDeepSleep() {
  useExtensions = "";
  
  //割り込み停止
  detachInterrupt(13);

  //deep_sleep復帰ボタン設定
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, HIGH);

  Serial.println("Going to sleep now");
  esp_deep_sleep_start();
}

void setup() {
  // シリアルモニターの初期化をする
  Serial.begin(115200);

  while ( !Serial ) {}

  // (channel, frequency, ?bit)
  ledcSetup(0, 12800, 8);
  ledcSetup(1, 12800, 8);
  ledcSetup(2, 12800, 8);

  // (pin, channel)
  ledcAttachPin(RED, 0);
  ledcAttachPin(GREEN, 1);
  ledcAttachPin(BLUE, 2);

  pinMode(BUTTON, INPUT_PULLUP);

  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);
  pinMode(pinZ, INPUT);

  // BLEセットアップ
  BLEDevice::init("ESP32");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristicWrite = pService->createCharacteristic(
                           CHARACTERISTIC_WRITE_UUID,
                           BLECharacteristic::PROPERTY_READ   |
                           BLECharacteristic::PROPERTY_WRITE  |
                           BLECharacteristic::PROPERTY_NOTIFY
                         );

  pCharacteristicRead = pService->createCharacteristic(
                          CHARACTERISTIC_READ_UUID,
                          BLECharacteristic::PROPERTY_READ   |
                          BLECharacteristic::PROPERTY_WRITE  |
                          BLECharacteristic::PROPERTY_NOTIFY
                        );

  pCharacteristicRead->setCallbacks(new MyCharacteristicCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();

  bootCounter ++;
  Serial.println("Boot counter : " + String(bootCounter));

  if (state) {
    state = false;

    startDeepSleep();
  } else {
    // 起動処理
    print_wakeup_reason();

    state = true;

    // 復帰ボタン解除
    rtc_gpio_deinit(GPIO_NUM_13);

    // 割り込み処理
    attachInterrupt(BUTTON, startDeepSleep, RISING);

    // (channel, size)
    ledcWrite(0, 32);
    ledcWrite(1, 32);
    ledcWrite(2, 32);
  }

  // 拡張機能名をリセット
  useExtensions = "";
}

void loop() {
  // notify changed value
  if (deviceConnected) {
    if (useExtensions.equals("acceleration")) {
      //","これの位置で区切るため間に仕込む
      String strVal(
        "a,"
        + getCalcuAccel(analogRead(pinX))
        + ","
        + getCalcuAccel(analogRead(pinY))
        + ","
        + getCalcuAccel(analogRead(pinZ))
      );

      Serial.println(strVal);

      // 指定のUUIDへString型をchar型に変換してnotify形式で送信
      pCharacteristicWrite->setValue(strVal.c_str());
      pCharacteristicWrite->notify();

      // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    delay(3);
    
    } 
    else if (useExtensions.equals("pressure")) {
      double volt = getVoltage(analogRead(pinY));

      /*
       * float battery = getBattery();
       * 
       * Serial.println(" バッテリーの電圧 : " + (String) battery);
       * 
       * 
       */
      
      
      Serial.print("出力電圧 : " + (String) volt);
      

      String result(
        useExtensions 
        + ","
        + (String) volt 
        + ","
        + "0"
        + ","
        + "0" );

      pCharacteristicWrite->setValue(result.c_str());
      pCharacteristicWrite->notify();

      delay(3);
      
    } else {
      delay(2000);
    }
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }

  delay(50);
}
