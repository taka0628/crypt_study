#include <CurieBLE.h>

BLEService bleService("19B10010-E8F2-537E-4F6C-D104768A1214"); // Service�̍쐬

// write(Android -> Arduino�ւ̒ʐM)�pCharacteristic�̍쐬
BLECharacteristic writeCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, 2);
// notify(Arduino -> Android�ւ̒ʐM)�pCharacteristic�̍쐬
BLECharacteristic notifyCharacteristic("19B10012-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 2);

int send_value = 0; // �Z���g�����֑��M���鐔�l���i�[����ϐ�
int recv_value = 0;
byte *value = new byte[2]; // �f�[�^����M�p�̔z��̕ҏW

void setup() {
  Serial.begin(9600);

  // BLE�̏��������J�n
  BLE.begin();

  // �y���t�F�����̖��O��ݒ� (XXXX�͊w�Дԍ��̉�4��)
  BLE.setLocalName("SecNet2_XXXX");
  // �y���t�F��������L������Service��UUID��ݒ�
  BLE.setAdvertisedService(bleService);

  // Service��Characteristic��ǉ�
  bleService.addCharacteristic(writeCharacteristic);
  bleService.addCharacteristic(notifyCharacteristic);

  // Service��ǉ�
  BLE.addService(bleService);

  // Characteristic�ɏ����l(0)��ݒ�
//  writeCharacteristic.setValue(value, 2);
//  notifyCharacteristic.setValue(value, 2);

  // �L�����J�n
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  // BLE�̃C�x���g���|�[�����O
  BLE.poll();

  /** �Z���g�����֑��M����l�̌��� **/
  send_value += 1;
  if(send_value > 1000){
    send_value = 0;
  }
  /******************************/
  
  // �Z���T�[����擾�����l��Android�ɑ��M
  memcpy(value, (byte *)&send_value, 2);
  notifyCharacteristic.setValue(value, 2);

  // Android����A�N�`���G�[�^����p�̒l����M
  memcpy(value, writeCharacteristic.value(), 2);
  memcpy((byte *)&recv_value, value, 2);

  /** �Z���g���������M�����l�ɑ΂��鏈�� **/
  Serial.print("Received: ");
  Serial.println(recv_value);
  /*************************************/

  delay(500);
}

