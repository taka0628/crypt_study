import android.support.v4.app.ActivityCompat;
// �G���[���o��ꍇ�C
// import androidx.core.app.ActivityCompat;
// �ɕύX����D

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

    // �ڑ��ΏۂƂȂ�y���t�F�����̖��O(XXXX�͊w�Дԍ��̉�4��)
    private static final String PERIPHERAL_NAME = "SecNet2_XXXX";

    // �ڑ��ΏۂƂȂ�T�[�r�X��UUID.
    private static final String SERVICE_UUID = "19B10010-E8F2-537E-4F6C-D104768A1214";

    // �ڑ��ΏۂƂȂ�L�����N�^���X�e�B�b�N��UUID.
    private static final String CHAR_WRITE_UUID = "19B10011-E8F2-537E-4F6C-D104768A1214";
    private static final String CHAR_NOTIFY_UUID = "19B10012-E8F2-537E-4F6C-D104768A1214";

    // �L�����N�^���X�e�B�b�N�ݒ�UUID(�Œ�l).
    private static final String CHARACTERISTIC_CONFIG_UUID = "00002902-0000-1000-8000-00805f9b34fb";

    // �y���t�F�����Ɛڑ����Ȃ�: 0, �y���t�F�����Ɛڑ�����: 1
    private int flag_connect = 1;

    // �O���t�\���p�̕ϐ�
    private int num;
    private float[] values;
    private String[] labels; // �f�[�^�̃��x�����i�[����z��
    private int[] colors; // �O���t�Ƀv���b�g����_�̐F���i�[����z��
    private float max, min;

    // �l���v���b�g����x���W
    private float count = 0;

    // BLE�ŗ��p����N���X�Q
    private BluetoothManager mBleManager;
    private BluetoothAdapter mBleAdapter;
    private BluetoothLeScanner mBleScanner;
    private BluetoothGatt mBleGatt;
    private BluetoothGattCharacteristic notifyCharacteristic, writeCharacteristic;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // �A�v�����s���̓X���[�v���Ȃ�
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
//        labels[2] = "�����x Z��";


        colors[0] = Color.rgb(0xFF, 0x00, 0x00); // ��
        colors[1] = Color.rgb(0x00, 0xFF, 0x00); // ��
        //       colors[2] = Color.rgb(0x00, 0x00, 0xFF); // ��

        max = 1000;
        min = -1000;

        // �O���tView������������
        initChart();

        // Bluetooth�̎g�p����.
        mBleManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBleAdapter = mBleManager.getAdapter();

        // ���Ԋu�ŃO���t���A�b�v�f�[�g����
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    updateGraph();
                    try {
                        Thread.sleep(500);
                    } catch (Exception e) {
                        Log.e("Test", "��O�o��", e);
                    }
                }
            }
        }).start();
    }

    @Override
    protected void onResume() {
        super.onResume();

        // *************************************************** //
        // Bluetooth�֘A�̏���                                  //
        // *************************************************** //

        // �ʒu���̗��p���𗘗p�҂ɋ��߂� (BLE���ʒu���Ƃ��ė��p�ł��邽�߁C�����K�v)
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, 1);

            return;
        }

        // BLE���g�p�\�Ȃ�A�ʐM������X�L����
        if ((mBleAdapter != null) || (mBleAdapter.isEnabled())) {
            mBleScanner = mBleAdapter.getBluetoothLeScanner();
            mBleScanner.startScan(scanCallback);
        }
    }

    private int DecideControlParameter(int input){
        int output;

        output = input + 1;

        return output;
    }

    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState)
        {
            // �ڑ��󋵂��ω���������s.
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                // �ڑ��ɐ���������T�[�r�X����������.
                gatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                // �ڑ����؂ꂽ��GATT����ɂ���.
                if (mBleGatt != null)
                {
                    mBleGatt.close();
                    mBleGatt = null;
                }
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status)
        {
            // Service��������������s.
            if (status == BluetoothGatt.GATT_SUCCESS) {
                // UUID���������ǂ������m�F����.
                BluetoothGattService service = gatt.getService(UUID.fromString(SERVICE_UUID));
                if (service != null)
                {
                    // �w�肵��UUID������Characteristic���m�F����.
                    notifyCharacteristic = service.getCharacteristic(UUID.fromString(CHAR_NOTIFY_UUID));
                    writeCharacteristic = service.getCharacteristic(UUID.fromString(CHAR_WRITE_UUID));

                    if (notifyCharacteristic != null) {

                        // Service, Characteristic��UUID�������Ȃ�BluetoothGatt���X�V����.
                        mBleGatt = gatt;

                        // �L�����N�^���X�e�B�b�N������������ANotification�����N�G�X�g.
                        boolean registered = mBleGatt.setCharacteristicNotification(notifyCharacteristic, true);

                        // Characteristic �� Notification��L��������.
                        BluetoothGattDescriptor descriptor = notifyCharacteristic.getDescriptor(
                                UUID.fromString(CHARACTERISTIC_CONFIG_UUID));

                        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                        mBleGatt.writeDescriptor(descriptor);
                    }
                }
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic)
        {
            byte[] recvValue;
            byte[] sendValue = new byte[2];
            final String text;
            int input, output;

            // �L�����N�^���X�e�B�b�N��UUID���`�F�b�N(getUuid�̌��ʂ��S�ď������ŋA���Ă���̂�UpperCase�ɕϊ�)
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

            // ���������y���t�F�������ڑ��Ώۂƈ�v����ꍇ�ɂ́ARssi���擾
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

    /** �O���tView�̏����� **/

    private void initChart() {
        // ���O���tView
        mLineChart = (LineChart) findViewById(R.id.chart_DynamicMultiLineGraph);

        // �O���t�����e�L�X�g��\�����邩
        mLineChart.getDescription().setEnabled(true);
        // �O���t�����e�L�X�g�̃e�L�X�g�ݒ�
        mLineChart.getDescription().setText("Line Chart of Sensor Data");
        // �O���t�����e�L�X�g�̕����F�ݒ�
        mLineChart.getDescription().setTextColor(Color.BLACK);
        // �O���t�����e�L�X�g�̕����T�C�Y�ݒ�
        mLineChart.getDescription().setTextSize(10f);
        // �O���t�����e�L�X�g�̕\���ʒu�ݒ�
        mLineChart.getDescription().setPosition(0, 0);

        // �O���t�ւ̃^�b�`�W�F�X�`���[��L���ɂ��邩
        mLineChart.setTouchEnabled(true);

        // �O���t�̃X�P�[�����O��L���ɂ��邩
        mLineChart.setScaleEnabled(true);

        // �O���t�̃h���b�M���O��L���ɂ��邩
        mLineChart.setDragEnabled(true);

        // �O���t�̃s���`/�Y�[����L���ɂ��邩
        mLineChart.setPinchZoom(true);

        // �O���t�̔w�i�F�ݒ�
        mLineChart.setBackgroundColor(Color.WHITE);

        // ��̃f�[�^���Z�b�g����
        mLineChart.setData(new LineData());

        // Y��(��)�̐ݒ�
        // Y��(��)�̎擾
        YAxis leftYAxis = mLineChart.getAxisLeft();
        // Y��(��)�̍ő�l�ݒ�
        leftYAxis.setAxisMaximum(max);
        // Y��(��)�̍ŏ��l�ݒ�
        leftYAxis.setAxisMinimum(min);

        // Y��(�E)�̐ݒ�
        // Y��(�E)�͕\�����Ȃ�
        mLineChart.getAxisRight().setEnabled(false);

        // X���̐ݒ�
        XAxis xAxis = mLineChart.getXAxis();
        // X���̒l�\���ݒ�
        xAxis.setValueFormatter(new IAxisValueFormatter() {
            @Override
            public String getFormattedValue(float value, AxisBase axis) {
                if(value >= 10) {
                    // �f�[�^20���Ƃɖڐ���ɕ�����\��
                    if (((int) value % 20) == 0) {
                        return Float.toString(value);
                    }
                }
                // null��Ԃ��Ɨ�����̂ŁA�l�������Ȃ��ꍇ�͋󕶎���Ԃ�
                return "";
            }
        });
    }

    private void updateGraph() {
        // ���̏����擾
        LineData lineData = mLineChart.getData();
        if(lineData == null) {
            return;
        }

        LineDataSet[] lineDataSet = new LineDataSet[num];

        for(int i = 0; i<num; i++){
            // i�Ԗڂ̐����擾
            lineDataSet[i] = (LineDataSet) lineData.getDataSetByIndex(i);
            // i�Ԗڂ̐�������������Ă��Ȃ��ꍇ�͏���������
            if( lineDataSet[i] == null) {
                // LineDataSet�I�u�W�F�N�g����
                lineDataSet[i] = new LineDataSet(null, labels[i]);
                // ���̐F�ݒ�
                lineDataSet[i].setColor(colors[i]);
                // ���Ƀv���b�g�l�̓_��`�悵�Ȃ�
                lineDataSet[i].setDrawCircles(false);
                // ���Ƀv���b�g�l�̒l�e�L�X�g��`�悵�Ȃ�
                lineDataSet[i].setDrawValues(false);
                // ����ǉ�
                lineData.addDataSet(lineDataSet[i]);
            }
            // i�Ԗڂ̐��ɒl��ǉ�
            lineData.addEntry(new Entry(count, values[i]), i);
        }

        // �l�X�V�ʒm
        mLineChart.notifyDataSetChanged();

        // X���ɕ\������ő��Entry�̐����w��
        mLineChart.setVisibleXRangeMaximum(100);

        // �I�V���X�R�[�v�̂悤�ɌÂ��f�[�^�����Ɋ񂹂Ă����悤�ɕ\���ʒu�����炷
        mLineChart.moveViewTo(count, getVisibleYCenterValue(mLineChart), YAxis.AxisDependency.LEFT);

        count++;
    }

    /**
     * �\�����Ă���Y���W�̒��S�l��Ԃ�<br>
     *     ���Y���W�͍�
     * @param lineChart �Ώۂ�LineChart
     * @return �\�����Ă���Y���W�̒��S�l
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
