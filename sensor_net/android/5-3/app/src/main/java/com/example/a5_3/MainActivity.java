package com.example.a5_3;

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

import android.graphics.Color;
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

public class MainActivity extends AppCompatActivity implements SensorEventListener {

    private LineChart mLineChart;

    // センサーマネージャを定義
    private SensorManager manager;

    // グラフに表示するデータに関する値を定義
    private int num; // グラフにプロットするデータの数

    private String[] labels; // データのラベルを格納する配列
    private int[] colors; // グラフにプロットする点の色を格納する配列
    private float max, min; // グラフのY軸の最大値と最小値

    private float[] values; // データを格納する配列

    // 値をプロットするx座標
    private float count = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // センサーを制御するための変数の初期化
        manager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);

        // アプリ実行中はスリープしない
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        // グラフに表示するデータに関連する値を初期化
        num = 3;
        values = new float[num];
        labels = new String[num];
        colors = new int[num];

        labels[0] = "加速度 X軸";
        labels[1] = "加速度 Y軸";
        labels[2] = "加速度 Z軸";

        colors[0] = Color.rgb(0xFF, 0x00, 0x00); // 赤
        colors[1] = Color.rgb(0x00, 0xFF, 0x00); // 緑
        colors[2] = Color.rgb(0x00, 0x00, 0xFF); // 青

        max = 50;
        min = -50;

        // グラフViewを初期化する
        initChart();

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

        // アプリケーション開始時に呼ばれるコールバック関数
    @Override
    protected void onResume()
    {
        super.onResume();
        // 情報を取得するセンサーの設定（今回は加速度センサを取得）
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

    public void onSensorChanged(SensorEvent event)
    {
        // 取得した情報が加速度センサーからのものか確認
        if(event.sensor.getType() == Sensor.TYPE_ACCELEROMETER){
            // 受け取った情報を格納用の配列にコピー
            values = event.values.clone();
        }
    }

    // センサーの精度の変更時に呼ばれるコールバック関数(今回は何もしない)
    public void onAccuracyChanged(Sensor sensor, int accuracy){}

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