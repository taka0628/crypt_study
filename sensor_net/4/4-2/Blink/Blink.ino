const int pinrotary = A0;
const int pinled = 3;
void setup() {
 Serial.begin(9600); // シリアルモニタと 9600bps で通信するように設定
pinMode(pinrotary, INPUT); // Rotary Angle Sensor を接続したポートを入力に設定
pinMode(pinled, 3); // LED を接続したポートを出力に設定
}
void loop() {
 int value = analogRead(pinrotary); // Rotary Angle Sensor から値を読み取り
 Serial.println(value); // センサーから読み取った値をシリアルモニタにログ表示
 analogWrite(pinled, value/4); // センサーから読み取った値の 1/4 を LED の明るさに設定
 delay(100); // 100 ミリ秒待機
}
