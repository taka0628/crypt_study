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

public class MainActivity extends AppCompatActivity {

    private LineChart mLineChart;

    // �O���t�ɕ\������f�[�^�Ɋւ���l���`
    private int num; // �O���t�Ƀv���b�g����f�[�^�̐�

    private String[] labels; // �f�[�^�̃��x�����i�[����z��
    private int[] colors; // �O���t�Ƀv���b�g����_�̐F���i�[����z��
    private float max, min; // �O���t��Y���̍ő�l�ƍŏ��l

    private float[] values; // �f�[�^���i�[����z��

    // �l���v���b�g����x���W
    private float count = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // �A�v�����s���̓X���[�v���Ȃ�
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        // �O���t�ɕ\������f�[�^�Ɋ֘A����l��������
        num = 3;
        values = new float[num];
        labels = new String[num];
        colors = new int[num];

        labels[0] = "�����x X��";
        labels[1] = "�����x Y��";
        labels[2] = "�����x Z��";

        colors[0] = Color.rgb(0xFF, 0x00, 0x00); // ��
        colors[1] = Color.rgb(0x00, 0xFF, 0x00); // ��
        colors[2] = Color.rgb(0x00, 0x00, 0xFF); // ��
        
        max = 50;
        min = -50;

        // �O���tView������������
        initChart();

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
