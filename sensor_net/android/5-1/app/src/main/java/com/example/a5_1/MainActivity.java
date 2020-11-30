package com.example.a5_1;

import androidx.appcompat.app.AppCompatActivity;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.view.WindowManager;
import android.widget.TextView;
import java.util.List;
import android.os.Bundle;

public class MainActivity extends AppCompatActivity implements SensorEventListener {

    // センサーマネージャを定義
    private SensorManager manager;
    // 画面の各表示欄を制御するための変数（今回は 3 個、必要に応じて増やす）
    private TextView sensor1, sensor2, sensor3;
    // センサーから届いた値を格納する配列を定義
    private float[] values = new float[3];

    // アプリケーション起動時に呼ばれるコールバック関数
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // 画面の各表示欄を制御するための変数の初期化
        sensor1 = (TextView)findViewById(R.id.sensor1);
        sensor2 = (TextView)findViewById(R.id.sensor2);
        sensor3 = (TextView)findViewById(R.id.sensor3);
        // センサーを制御するための変数の初期化
        manager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);

    }

    // アプリケーション開始時に呼ばれるコールバック関数
    @Override
    protected void onResume()
    {
        super.onResume();
        // 情報を取得するセンサーの設定（今回は加速度センサを取得）
//        List<Sensor> sensors = manager.getSensorList(Sensor.TYPE_ACCELEROMETER);
//        Sensor sensor = sensors.get(0);
        List<Sensor> sensors = manager.getSensorList(Sensor.TYPE_ACCELEROMETER);
        Sensor sensor = sensors.get(0);
        // センサーからの情報の取得を開始
        manager.registerListener(this, sensor, SensorManager.SENSOR_DELAY_UI);
    }

    // アプリケーション一時停止時に呼ばれるコールバック関数
    @Override
    protected void onPause()
    {
        super.onPause();
        // センサのリスナー登録解除
        manager.unregisterListener(this);
    }

    // センサーイベント受信時に呼ばれるコールバック関数
    public void onSensorChanged(SensorEvent event)
    {
        // 取得した情報が加速度センサーからのものか確認
        if(event.sensor.getType() == Sensor.TYPE_ACCELEROMETER){
            // 受け取った情報を格納用の配列にコピー
            values = event.values.clone();
            // 受け取った情報を表示欄に表示
            sensor1.setText("Acc X-axis: " + values[0]);
            sensor2.setText("Acc Y-axis: " + values[1]);
            sensor3.setText("Acc Z-axis: " + values[2]);
        }
    }
    // センサーの精度の変更時に呼ばれるコールバック関数(今回は何もしない)
    public void onAccuracyChanged(Sensor sensor, int accuracy){}

}