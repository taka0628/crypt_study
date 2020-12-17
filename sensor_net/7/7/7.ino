#include <CurieBLE.h>

BLEService bleService("19B10010-E8F2-537E-4F6C-D104768A1214"); // Serviceの作成

// write(Android -> Arduinoへの通信)用Characteristicの作成
BLECharacteristic writeCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, 2);
// notify(Arduino -> Androidへの通信)用Characteristicの作成
BLECharacteristic notifyCharacteristic("19B10012-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 2);

//送受信関連
int send_value = 0;        // セントラルへ送信する数値を格納する変数
int recv_value = 0;        //セントラルから受信した値を格納する変数
byte *value = new byte[2]; // データ送受信用の配列の編集
int DATA_SIZE_MAX = 65535; //送信可能なデータの最大値、16bit

//センサ変数
const int pinrotary = A0; //センサポート
const int pinled = 3;     //出力ポート

void setup()
{
  Serial.begin(9600);

  //  照度センサとLEDのセットアップ
  pinMode(pinrotary, INPUT); // Rotary Angle Sensor を接続したポートを入力に設定
  pinMode(pinled, 3);        // LED を接続したポートを出力に設定

  // BLEの初期化を開始
  BLE.begin();

  // ペリフェラルの名前を設定 (XXXXは学籍番号の下4桁)
  BLE.setLocalName("SecNet2_2600");
  // ペリフェラルから広告するServiceのUUIDを設定
  BLE.setAdvertisedService(bleService);

  // ServiceへCharacteristicを追加
  bleService.addCharacteristic(writeCharacteristic);
  bleService.addCharacteristic(notifyCharacteristic);

  // Serviceを追加
  BLE.addService(bleService);

  // Characteristicに初期値(0)を設定
  //  writeCharacteristic.setValue(value, 2);
  //  notifyCharacteristic.setValue(value, 2);

  // 広告を開始
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop()
{
  // BLEのイベントをポーリング
  BLE.poll();

  /** セントラルへ送信する値の決定 **/
  int send_value = analogRead(pinrotary); // Rotary Angle Sensor から値を読み取り
  //   送信可能なデータサイズであることを確認
  if (send_value >= DATA_SIZE_MAX)
  {
    send_value = DATA_SIZE_MAX;
  }
  else if (send_value < 0)
  {
    // 送信不可のデータであればゼロクリアを行う。
    send_value = 0;
  }

  /******************************/

  // センサーから取得した値をAndroidに送信
  memcpy(value, (byte *)&send_value, 2);
  notifyCharacteristic.setValue(value, 2);

  // Androidからアクチュエータ制御用の値を受信
  memcpy(value, writeCharacteristic.value(), 2);
  memcpy((byte *)&recv_value, value, 2);

  /** セントラルから受信した値に対する処理 **/
  Serial.print("Received: ");
  Serial.println(recv_value);
  // LEDへ受信した値を送り、発光させる。
  analogWrite(3, recv_value);
  /*************************************/

  delay(500);
}
