# Hardware-program-sources
## 概要
AndroidアプリとBluetooth Low Energy規格（以後"BLE"）で通信をし、センサの情報やリポバッテリーの残量を送信するプログラムです。
あくまでコントローラー的な役割なので、Android側からBLEを通して指示を受信しそれを受けて動作します。  
## 対象デバイス
BLE規格で通信することができるかつ、ArduinoIDEで書き込みのできるもの。  
ESP32を使って開発したので、Pinなどはそれに基づいた設定になっています。
## 使い方
Bluetooth UUID :
```  
#define SERVICE_UUID              "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_WRITE_UUID "b62c1ffa-bdd8-46ea-a378-d539cf405d93"
#define CHARACTERISTIC_READ_UUID  "c8f8d86f-f03a-428f-8917-39384ad98e4b"
```  

必要データ識別名  
 : BLECharacteristicCallbacks{}
```
// 電圧が欲しい場合        -> battery
if (value.equals("battery")) {}
```
 : loop()
```
// 加速度データが欲しい場合 -> acceleration
if (useExtensions.equals("acceleration")) {}

// 圧力データが欲しい場合   -> pressure
if (useExtensions.equals("pressure")) {}
```

加速度センサのデータ形式 :
```
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

String strVal(
        "a,"
        + getCalcuAccel(analogRead(pinX))
        + ","
        + getCalcuAccel(analogRead(pinY))
        + ","
        + getCalcuAccel(analogRead(pinZ))
      );
```  

圧センサのデータ形式 :
```
double getVoltage(int d) {
  double R = 10; // 10k Ω

  double volt = d * 3.3 / 4096;

  return volt;
}

double volt = getVoltage(analogRead(pinY));

String result(
        useExtensions 
        + ","
        + (String) volt 
        + ","
        + "0"
        + ","
        + "0" );
```  

バッテリー電圧のデータ形式 :
```
float getBattery(){
  int d = analogRead(33);
  
  const int R1 = 10000; // 10kΩ
  const int R2 = 10000; // 10kΩ
  
  // 例) 3.0
  return d * (R1+R2) / R2 * (3.7/4096);
}
```
