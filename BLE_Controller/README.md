## 使い方

1. このデバイスにBLE規格で接続する
2. 欲しいデータの文字列をCharacteristicsを通して送信する
3. Characteristics.notify()で送られてくるデータを受け取る

Bluetooth UUID :
```  
SERVICE_UUID              "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
CHARACTERISTIC_WRITE_UUID "b62c1ffa-bdd8-46ea-a378-d539cf405d93"
CHARACTERISTIC_READ_UUID  "c8f8d86f-f03a-428f-8917-39384ad98e4b"
```  

必要データ識別名  
 : BLECharacteristicCallbacks{}
```
"battery"
```
 : loop()
```
//加速度データが欲しい場合
"acceleration"

// 圧力データが欲しい場合
"pressure"
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
