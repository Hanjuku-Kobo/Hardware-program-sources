## 使い方
1. デバイスをペアリングする。
2. 各ソフトで、ポート番号を指定して数値を115200にする
3. 欲しいデータの定数を送る（例 : "battery")
4. Bluetooth Serialで送られてくるデータを取得する  

デバイス名 :  
```
SerialBT.begin("ESP32");
```
データ識別名 :
```
// バッテリー電圧
"battery"

// 加速度データ
"acceleration"

// 圧力データ
"pressure"
```

バッテリー電圧の電圧のデータ形式 :
```
float getBattery(){
  int d = analogRead(33);
  
  const int R1 = 10000; // 10kΩ
  const int R2 = 10000; // 10kΩ
  
  return d * (R1+R2) / R2 * (3.7/4096);
}

// 電圧を取得して小数点以下一桁に変更
      String battery = (String) ("%.1f", getBattery());
```

加速度データ形式 :
```
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

String strVal(
        "a,"
        + getCalcuAccel(analogRead(pinX))
        + ","
        + getCalcuAccel(analogRead(pinY))
        + ","
        + getCalcuAccel(analogRead(pinZ))
      );
```

圧力データ形式 :
```
double getVoltage(int d) {
  double R = 10; // 10k Ω

  double volt = d * 3.3 / 4096;

  return volt;
}

// 小数点以下一桁に変更
      float volt = ("%.1f", getVoltage(analogRead(pinY)));

      String result(
        useExtensions 
        + ","
        + (String) volt
        + ","
        + "0"
        + ","
        + "0" );
```
