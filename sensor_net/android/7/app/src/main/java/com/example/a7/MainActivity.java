package com.example.a6_1;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import androidx.core.app.ActivityCompat;
// エラーが出る場合，
// import androidx.core.app.ActivityCompat;
// に変更する．

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.hardware.SensorManager;
import android.util.Log;
import android.view.WindowManager;

import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.AxisBase;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.formatter.IAxisValueFormatter;
import com.github.mikephil.charting.utils.Transformer;
import com.github.mikephil.charting.utils.ViewPortHandler;



import java.util.UUID;

public class MainActivity extends AppCompatActivity {
    private LineChart mLineChart;

    // 接続対象となるペリフェラルの名前(XXXXは学籍番号の下4桁)
    private static final String PERIPHERAL_NAME = "SecNet2_2600";

    // 接続対象となるサービスのUUID.
    private static final String SERVICE_UUID = "19B10010-E8F2-537E-4F6C-D104768A1214";

    // 接続対象となるキャラクタリスティックのUUID.
    private static final String CHAR_WRITE_UUID = "19B10011-E8F2-537E-4F6C-D104768A1214";
    private static final String CHAR_NOTIFY_UUID = "19B10012-E8F2-537E-4F6C-D104768A1214";

    // キャラクタリスティック設定UUID(固定値).
    private static final String CHARACTERISTIC_CONFIG_UUID = "00002902-0000-1000-8000-00805f9b34fb";

    // ペリフェラルと接続しない: 0, ペリフェラルと接続する: 1
    private int flag_connect = 0;

    // グラフ表示用の変数
    private int num;
    private float[] values;
    private String[] labels; // データのラベルを格納する配列
    private int[] colors; // グラフにプロットする点の色を格納する配列
    private float max, min;

    // 値をプロットするx座標
    private float count = 0;

    // BLEで利用するクラス群
    private BluetoothManager mBleManager;
    private BluetoothAdapter mBleAdapter;
    private BluetoothLeScanner mBleScanner;
    private BluetoothGatt mBleGatt;
    private BluetoothGattCharacteristic notifyCharacteristic, writeCharacteristic;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // アプリ実行中はスリープしない
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        num = 2;
        values = new float[num];
        labels = new String[num];
        colors = new int[num];

        for(int i=0; i<num; i++){
            values[i] = 0;
        }

        labels[0] = "Sensor data";
        labels[1] = "RSSI";
//        labels[2] = "加速度 Z軸";


        colors[0] = Color.rgb(0xFF, 0x00, 0x00); // 赤
        colors[1] = Color.rgb(0x00, 0xFF, 0x00); // 緑
        //       colors[2] = Color.rgb(0x00, 0x00, 0xFF); // 青

        max = 0;
        min = -100;

        // グラフViewを初期化する
        initChart();

        // Bluetoothの使用準備.
        mBleManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBleAdapter = mBleManager.getAdapter();

        // 一定間隔でグラフをアップデートする
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    updateGraph();
                    try {
                        Thread.sleep(500);
                    } catch (Exception e) {
                        Log.e("Test", "例外出力", e);
                    }
                }
            }
        }).start();
    }

    @Override
    protected void onResume() {
        super.onResume();

        // *************************************************** //
        // Bluetooth関連の処理                                  //
        // *************************************************** //

        // 位置情報の利用許可を利用者に求める (BLEも位置情報として利用できるため，許可が必要)
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, 1);

            return;
        }

        // BLEが使用可能なら、通信相手をスキャン
        if ((mBleAdapter != null) || (mBleAdapter.isEnabled())) {
            mBleScanner = mBleAdapter.getBluetoothLeScanner();
            mBleScanner.startScan(scanCallback);
        }
    }
    //    ペリフェラルへ送り返す値を返り値とする
    private int DecideControlParameter(int input){
        int output;

//        output = input + 1;
        output = 1023;
        return output;
    }

    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
        @Override
//        コネクションが正しく確立できた場合
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState)
        {
            // 接続状況が変化したら実行.
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                // 接続に成功したらサービスを検索する.
                gatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                // 接続が切れたらGATTを空にする.
                if (mBleGatt != null)
                {
                    mBleGatt.close();
                    mBleGatt = null;
                }
            }
        }

        @Override
//        サービスが発見できたとき
        public void onServicesDiscovered(BluetoothGatt gatt, int status)
        {
            // Serviceが見つかったら実行.
            if (status == BluetoothGatt.GATT_SUCCESS) {
                // UUIDが同じかどうかを確認する.
                BluetoothGattService service = gatt.getService(UUID.fromString(SERVICE_UUID));
                if (service != null)
                {
                    // 指定したUUIDを持つCharacteristicを確認する.
                    notifyCharacteristic = service.getCharacteristic(UUID.fromString(CHAR_NOTIFY_UUID));
                    writeCharacteristic = service.getCharacteristic(UUID.fromString(CHAR_WRITE_UUID));

                    if (notifyCharacteristic != null) {

                        // Service, CharacteristicのUUIDが同じならBluetoothGattを更新する.
                        mBleGatt = gatt;

                        // キャラクタリスティックが見つかったら、Notificationをリクエスト.
                        boolean registered = mBleGatt.setCharacteristicNotification(notifyCharacteristic, true);

                        // Characteristic の Notificationを有効化する.
                        BluetoothGattDescriptor descriptor = notifyCharacteristic.getDescriptor(
                                UUID.fromString(CHARACTERISTIC_CONFIG_UUID));

                        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                        mBleGatt.writeDescriptor(descriptor);
                    }
                }
            }
        }

        @Override
//        ペリフェラルからデータを受信したとき
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic)
        {
            byte[] recvValue;
            byte[] sendValue = new byte[2];
            final String text;
            int input, output;

            // キャラクタリスティックのUUIDをチェック(getUuidの結果が全て小文字で帰ってくるのでUpperCaseに変換)
            if (CHAR_NOTIFY_UUID.equals(characteristic.getUuid().toString().toUpperCase()))
            {
                recvValue = characteristic.getValue();
                input = (recvValue[0]&0xFF) + (recvValue[1]&0xFF)*0x100;

                text = "BLE Received Value: " + values[0];
                values[0] = input;

                output = DecideControlParameter(input);
                sendValue[0] = (byte)(output&0xFF);
                sendValue[1] = (byte)((output/0x100)&0xFF);

                Log.d("blelog", text);

                writeCharacteristic.setValue(sendValue);
                mBleGatt.writeCharacteristic(writeCharacteristic);

            }
        }
    };

    private ScanCallback scanCallback = new ScanCallback(){
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);

            // 発見したペリフェラルが接続対象と一致する場合には、Rssiを取得
            if((result.getDevice().getName() != null) && (result.getDevice().getName().equals(PERIPHERAL_NAME))){
                values[1] = result.getRssi();

                String text = "Found: " + result.getDevice().getName() + ", " + values[1];
                Log.d("blelog", text);

                if(flag_connect == 1){
                    result.getDevice().connectGatt(getApplicationContext(), false, mGattCallback);
                }
            }
        }

        @Override
        public void onScanFailed(int intErrorCode) {
            super.onScanFailed(intErrorCode);
        }
    };

    /** グラフViewの初期化 **/

    private void initChart() {
        // 線グラフView
        mLineChart = (LineChart) findViewById(R.id.chart_DynamicMultiLineGraph);

        // グラフ説明テキストを表示するか
        mLineChart.getDescription().setEnabled(true);
        // グラフ説明テキストのテキスト設定
        mLineChart.getDescription().setText("Line Chart of Sensor Data");
        // グラフ説明テキストの文字色設定
        mLineChart.getDescription().setTextColor(Color.BLACK);
        // グラフ説明テキストの文字サイズ設定
        mLineChart.getDescription().setTextSize(10f);
        // グラフ説明テキストの表示位置設定
        mLineChart.getDescription().setPosition(0, 0);

        // グラフへのタッチジェスチャーを有効にするか
        mLineChart.setTouchEnabled(true);

        // グラフのスケーリングを有効にするか
        mLineChart.setScaleEnabled(true);

        // グラフのドラッギングを有効にするか
        mLineChart.setDragEnabled(true);

        // グラフのピンチ/ズームを有効にするか
        mLineChart.setPinchZoom(true);

        // グラフの背景色設定
        mLineChart.setBackgroundColor(Color.WHITE);

        // 空のデータをセットする
        mLineChart.setData(new LineData());

        // Y軸(左)の設定
        // Y軸(左)の取得
        YAxis leftYAxis = mLineChart.getAxisLeft();
        // Y軸(左)の最大値設定
        leftYAxis.setAxisMaximum(max);
        // Y軸(左)の最小値設定
        leftYAxis.setAxisMinimum(min);

        // Y軸(右)の設定
        // Y軸(右)は表示しない
        mLineChart.getAxisRight().setEnabled(false);

        // X軸の設定
        XAxis xAxis = mLineChart.getXAxis();
        // X軸の値表示設定
        xAxis.setValueFormatter(new IAxisValueFormatter() {
            @Override
            public String getFormattedValue(float value, AxisBase axis) {
                if(value >= 10) {
                    // データ20個ごとに目盛りに文字を表示
                    if (((int) value % 20) == 0) {
                        return Float.toString(value);
                    }
                }
                // nullを返すと落ちるので、値を書かない場合は空文字を返す
                return "";
            }
        });
    }

    private void updateGraph() {
        // 線の情報を取得
        LineData lineData = mLineChart.getData();
        if(lineData == null) {
            return;
        }

        LineDataSet[] lineDataSet = new LineDataSet[num];

        for(int i = 0; i<num; i++){
            // i番目の線を取得
            lineDataSet[i] = (LineDataSet) lineData.getDataSetByIndex(i);
            // i番目の線が初期化されていない場合は初期化する
            if( lineDataSet[i] == null) {
                // LineDataSetオブジェクト生成
                lineDataSet[i] = new LineDataSet(null, labels[i]);
                // 線の色設定
                lineDataSet[i].setColor(colors[i]);
                // 線にプロット値の点を描画しない
                lineDataSet[i].setDrawCircles(false);
                // 線にプロット値の値テキストを描画しない
                lineDataSet[i].setDrawValues(false);
                // 線を追加
                lineData.addDataSet(lineDataSet[i]);
            }
            // i番目の線に値を追加
            lineData.addEntry(new Entry(count, values[i]), i);
        }

        // 値更新通知
        mLineChart.notifyDataSetChanged();

        // X軸に表示する最大のEntryの数を指定
        mLineChart.setVisibleXRangeMaximum(100);

        // オシロスコープのように古いデータを左に寄せていくように表示位置をずらす
        mLineChart.moveViewTo(count, getVisibleYCenterValue(mLineChart), YAxis.AxisDependency.LEFT);

        count++;
    }

    /**
     * 表示しているY座標の中心値を返す<br>
     *     基準のY座標は左
     * @param lineChart 対象のLineChart
     * @return 表示しているY座標の中心値
     */
    private float getVisibleYCenterValue(LineChart lineChart) {
        Transformer transformer = lineChart.getTransformer(YAxis.AxisDependency.LEFT);
        ViewPortHandler viewPortHandler = lineChart.getViewPortHandler();

        float highestVisibleY = (float) transformer.getValuesByTouchPoint(viewPortHandler.contentLeft(),
                viewPortHandler.contentTop()).y;
        float highestY = Math.min(lineChart.getAxisLeft().mAxisMaximum, highestVisibleY);

        float lowestVisibleY = (float) transformer.getValuesByTouchPoint(viewPortHandler.contentLeft(),
                viewPortHandler.contentBottom()).y;
        float lowestY = Math.max(lineChart.getAxisLeft().mAxisMinimum, lowestVisibleY);

        return highestY - (Math.abs(highestY - lowestY) / 2);
    }


}
