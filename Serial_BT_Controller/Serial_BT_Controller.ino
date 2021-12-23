#include <BluetoothSerial.h>
#include <driver/rtc_io.h>

BluetoothSerial SerialBT;

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

String useExtensions;

// リポバッテリーの電圧を計算
float getBattery(){
  int d = analogRead(33);
  
  const int R1 = 10000; // 10kΩ
  const int R2 = 10000; // 10kΩ
  
  return d * (R1+R2) / R2 * (3.7/4096);
}

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
  // 小数点を切り捨て
  int accelResult = round(accel * 1000);
  
  return (String)accelResult;
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
  detachInterrupt(digitalPinToInterrupt(BUTTON));

  //deep_sleep復帰ボタン設定
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, HIGH);

  Serial.println("Going to sleep now");
  esp_deep_sleep_start();
}

void setup() {
  // 一応起動しておくらしい
  Serial.begin(115200); 

  // 引数 = デバイス名
  /*
   * 書き込みは1バイトまで(8桁)
   * 読み込みも同じ
   */
  SerialBT.begin("ESP32");

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
    ledcWrite(1, 96);
    ledcWrite(2, 32);
  }

  // 拡張機能名をリセット
  useExtensions = "";
}

void loop() {
  // BluetoothSerialに書き込まれたか
  if (SerialBT.available()) {
    String value = (String) SerialBT.readString();
    Serial.println(value);

    if (value.equals("battery")) {
      // 電圧を取得して小数点以下一桁に変更
      String battery = (String) ("%.1f", getBattery());

      // string -> char* -> uint8_t
      SerialBT.write((uint8_t)atoi(battery.c_str()));
    } else {
      // 拡張機能用の変数に代入
      useExtensions = value;
    }
  }
  
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

      // 指定のSerialPortへ送信
      SerialBT.write((uint8_t)atoi(strVal.c_str()));
    } 
    else if (useExtensions.equals("pressure")) {
      // 小数点以下一桁に変更
      float volt = ("%.1f", getVoltage(analogRead(pinY)));

      /*
       * float battery = getBattery();
       * 
       * Serial.println(" バッテリーの電圧 : " + (String) battery);
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

      SerialBT.write((uint8_t)atoi(result.c_str()));
    } else {
      delay(1000);
    }
  
  delay(50);
}
