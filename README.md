# Hardware-program-sources

## 概要

AndroidアプリとBluetooth Low Energy規格（以後"BLE"）で通信をし、センサの情報やリポバッテリーの残量を送信するプログラムです。
あくまでコントローラー的な役割なので、Android側からBLEを通して指示を受信しそれを受けて動作します。 

## 対象デバイス

BLE規格で通信することができるかつ、ArduinoIDEで書き込みのできるもの。  
ESP32を使って開発したので、Pinなどはそれに基づいた設定になっています。

## 使い方

#### デバイス別 README URL

[new BLE Device ver](https://github.com/Hanjuku-Kobo/Hardware-program-sources/blob/main/BLE_Controller/README.md)  
[old BLE Device ver](https://github.com/Hanjuku-Kobo/Hardware-program-sources/blob/main/Old_BLE_Controller/README.md)  
[Bluetooth Serial ver](https://github.com/Hanjuku-Kobo/Hardware-program-sources/blob/main/Serial_BT_Controller/README.md)

#### 違い
最新デバイスには、電圧監視システムが追加されているが旧デバイスの回路にはそれがない  
Bluetoothの規格が異なる
