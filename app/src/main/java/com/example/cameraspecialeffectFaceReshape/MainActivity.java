package com.example.cameraspecialeffectFaceReshape;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.hardware.camera2.CameraAccessException;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;


public class MainActivity extends AppCompatActivity {
    public static final int REQUEST_CAMERA = 100;
    private TextView textView,textViewfillter;
    int numberA = 10;
    int numberB = 10;
    //* seekbar 结束******************************

    // *日志标签*
    private final String logTag = "MainActivity";
    // *展示视图*
    GLSurfaceView show_view;
    // *相机管理类*
    CameraHelper cameraHelper = null;
    // *弹出框*
    private AlertDialog.Builder m_alertDialog = null;
    // *特效的id*
    private int effect_id = 0;
    // *渲染器*
    GLVideoRender m_render = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // *seekbar******************************
        //文本框
        textView= findViewById(R.id.textView);
        textViewfillter= findViewById(R.id.fillter);
        //滑动条1（sigmaA）
        //* seekbar 声明开始******************************
        SeekBar seekBar = findViewById(R.id.seekBar);
        seekBar.setMax(100);//设置最大值(设置不了最小值)
        seekBar.setProgress(10);//设置初始值
        //滑动条2（sigmaB）
        SeekBar seekBar2 = findViewById(R.id.seekBar2);
        seekBar2.setMax(30);//设置最大值(设置不了最小值)
        seekBar2.setProgress(10);//设置初始值
        // *seekbar 结束******************************

        // *获取button和视图的*
        //2.初始化4个按钮
        Button open_close_btn = findViewById(R.id.open_close_btn);
        Button change_btn = findViewById(R.id.change_btn);
        Button choose_btn = findViewById(R.id.choose_btn);
        Button save_btn = findViewById(R.id.save_btn);

        //3.OpenGL显示视图
        show_view = findViewById(R.id.show_surface);
        m_render = new GLVideoRender(this, show_view);
        //4.打开摄像头
        cameraHelper = new CameraHelper(this);
        cameraHelper.setVideoRender(m_render);
        //5.设置弹出框
        m_alertDialog = new AlertDialog.Builder(this);
        final String[] items = {"原始","V型瘦脸+V下巴","整体瘦脸","脸部MASK(index太多,确定模型后再弄)","亮牙","大眼"};
        m_alertDialog.setTitle("选择特效").setSingleChoiceItems(items, 0, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                effect_id = i;
            }
        });
        m_alertDialog.setPositiveButton("确认", new DialogInterface.OnClickListener() {
            @SuppressLint("SetTextI18n")
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                GLRenderJNI.setRenderType(effect_id);
                Log.i(logTag, "当前是特效是: " + items[effect_id]);
                textViewfillter.setText("【"+items[effect_id]+"】");
            }
        });

//        //6.打开相机按钮 回调函数
//        open_close_btn.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if(cameraHelper.isOpend()) {
//                    try {
//                        cameraHelper.closeCamera();
//                        Toast.makeText(MainActivity.this, "已关闭相机", Toast.LENGTH_SHORT).show();
//                    } catch (InterruptedException e) {
//                        e.printStackTrace();
//                    }
//                } else {
//                    try {
//                        boolean have_camera_permision  = cameraHelper.openCamera();
//                        if(!have_camera_permision){Toast.makeText(MainActivity.this, "请手动打开相机权限！", Toast.LENGTH_SHORT).show();}
//                        else{Toast.makeText(MainActivity.this, "已打开相机", Toast.LENGTH_SHORT).show();}
//                    } catch (CameraAccessException e) {
//                        e.printStackTrace();
//                    }
//                }
//            }
//        });

        //7.切换摄像头按钮 回调函数
        change_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try {
                    cameraHelper.changeCameraOrientation();
                } catch (InterruptedException | CameraAccessException e) {
                    e.printStackTrace();
                }
            }
        });

        //8.选择滤镜按钮 回调函数
        choose_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                m_alertDialog.setSingleChoiceItems(items, effect_id, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        effect_id = i;
                    }
                });
                m_alertDialog.show();
//                Toast.makeText(MainActivity.this, "请选择滤镜！", Toast.LENGTH_SHORT).show();
            }
        });

        //9.保存照片按钮 回调函数
        save_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GLRenderJNI.savePng();
                Toast.makeText(MainActivity.this, "保存照片功能已关闭！", Toast.LENGTH_SHORT).show();
            }
        });

        //*seekbar*****************************
        //滑动条1（sigmaA）
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            //改变数值
            @SuppressLint("SetTextI18n")
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                numberA = progress;
                textView.setText(numberA + ", " + numberB/100.0);
                GLRenderJNI.setSigmaA(numberA);
//                CLRenderJNI.renderOpenCL();
            }
            //开始拖动
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
//                Toast.makeText(MainActivity.this,"从"+number+"开始滑动",Toast.LENGTH_SHORT).show();
            }
            //停止拖动
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
//                Toast.makeText(MainActivity.this,"滑动到："+number,Toast.LENGTH_SHORT).show();

            }
        });
        //滑动条2（sigmaB）
        seekBar2.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            //改变数值
            @SuppressLint("SetTextI18n")
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                numberB = progress;
                textView.setText(numberA + ", " + numberB/100.0);
                GLRenderJNI.setSigmaB(numberB);
            }
            //开始拖动
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
//                Toast.makeText(MainActivity.this,"从"+number+"开始滑动",Toast.LENGTH_SHORT).show();
            }
            //停止拖动
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
//                Toast.makeText(MainActivity.this,"滑动到："+number,Toast.LENGTH_SHORT).show();

            }
        });
        //*seekbar 结束*****************************
    }

    //在OnResume动态申请权限
    @Override
    public void onResume()
    {
        super.onResume();
        if (ContextCompat.checkSelfPermission(getApplicationContext(), Manifest.permission.CAMERA) == PackageManager.PERMISSION_DENIED)
        {
            ActivityCompat.requestPermissions(this, new String[] {Manifest.permission.CAMERA}, REQUEST_CAMERA);
        }

        //每次onResume从后台切回来时，打开相机
        try {
            boolean have_camera_permision = cameraHelper.openCamera();
            if (!have_camera_permision) {
                Toast.makeText(MainActivity.this, "请手动打开相机权限！", Toast.LENGTH_SHORT).show();
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }
}
