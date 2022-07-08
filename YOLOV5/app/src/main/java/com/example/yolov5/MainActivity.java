package com.example.yolov5;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Color;


import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.InputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;


import android.database.Cursor;
import android.provider.MediaStore;
import android.util.Log;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;
public class MainActivity extends AppCompatActivity implements View.OnClickListener{

    static {
        System.loadLibrary("native-lib"); // Load native library at runtime, native-lib.dll (Windows) or native-lib.so (Unixes)
    }

    native  float[][] resultJNI(Object bitmap, String str);
    private ImageView imageView;
    private TextView textView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

//        findViewById(R.id.show).setOnClickListener((View.OnClickListener) this);
        findViewById(R.id.demo).setOnClickListener((View.OnClickListener) this);
        findViewById(R.id.process).setOnClickListener((View.OnClickListener) this);

        // 对两个控件进行实例化
        imageView = findViewById(R.id.imageView);
        textView = findViewById(R.id.textView);

        findViewById(R.id.textView).setOnClickListener((View.OnClickListener) this);
    }

    @Override
    public void onClick(View v){
        if (v.getId() == R.id.demo)
        {
            Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.test1);
            //显示图像
            imageView.setImageBitmap(bitmap);
            String demo = "demo";
            textView.setText(demo);
        }
//        else if(v.getId() == R.id.show) {
//            String demo2 = "demo222";
//            textView.setText(demo2);
//        }
        else {
            BitmapFactory.Options bfoOptions = new BitmapFactory.Options();
            bfoOptions.inScaled = false;
            Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.test1, bfoOptions);

            try{
                boolean res = copyAssetAndWrite("yolov5n.mnn");
            } catch (Exception e){
                // 获取异常基本信息
                System.out.println(e.getMessage());
            }
            File dataFile = new File(getCacheDir(),"yolov5n.mnn");
            System.out.println("模型路径:" + dataFile.getAbsolutePath());
            System.out.println(bitmap.getWidth());
            System.out.println( bitmap.getHeight());

            float[][] result = resultJNI(bitmap, dataFile.getAbsolutePath());
            System.out.println( "result values :\n");
            for (int i = 0; i < result.length; i++)
            {
                for (int j = 0; j < result[i].length; j++)
                {
                     System.out.println(result[i][j]  + " ");
                }
                System.out.println("\n");
            }

            Bitmap rgba = bitmap.copy(Bitmap.Config.ARGB_8888, true);

            // 设置随机颜色
            final int[] colors = new int[] {
                    Color.rgb( 54,  67, 244),
                    Color.rgb( 99,  30, 233),
                    Color.rgb(176,  39, 156),
                    Color.rgb(183,  58, 103),
                    Color.rgb(181,  81,  63),
                    Color.rgb(243, 150,  33),
                    Color.rgb(244, 169,   3),
                    Color.rgb(212, 188,   0),
                    Color.rgb(136, 150,   0),
                    Color.rgb( 80, 175,  76),
                    Color.rgb( 74, 195, 139),
                    Color.rgb( 57, 220, 205),
                    Color.rgb( 59, 235, 255),
                    Color.rgb(  7, 193, 255),
                    Color.rgb(  0, 152, 255),
                    Color.rgb( 34,  87, 255),
                    Color.rgb( 72,  85, 121),
                    Color.rgb(158, 158, 158),
                    Color.rgb(139, 125,  96)
            };
            String[] category_Name = new String[]
                    {"person", "bicycle", "car", "motorcycle", "airplane",
                    "bus", "train", "truck", "boat", "traffic light","fire hydrant",
                    "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse",
                    "sheep", "cow",	"elephant", "bear", "zebra", "giraffe", "backpack",
                    "umbrella", "handbag", "tie", "suitcase", "frisbee","skis", "snowboard",
                    "sports ball", "kite", "baseball bat", "baseball glove", "skateboard",
                    "surfboard","tennis racket", "bottle", "wine glass", "cup", "fork", "knife",
                    "spoon", "bowl", "banana", "apple","sandwich", "orange", "broccoli","carrot",
                    "hot dog", "pizza", "donut", "cake", "chair", "couch","potted plant", "bed",
                    "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
                    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase",
                    "scissors", "teddy bear","hair drier", "toothbrush" };

            Canvas canvas = new Canvas(rgba);
            Paint paint = new Paint();
            paint.setStyle(Paint.Style.STROKE);
            paint.setStrokeWidth(4);

            Paint textPaint=new Paint();
            textPaint.setStyle(Paint.Style.FILL);
            textPaint.setStrokeWidth(8);
            textPaint.setTextSize(50);
            textPaint.setTextAlign(Paint.Align.CENTER);

            for (int i = 0; i < result.length; i++)
             {
                 float[] bbox = result[i];
                 int x = (int)bbox[0];
                 int y = (int)bbox[1];
                 int x2 = (int)bbox[2];
                 int y2 = (int)bbox[3];
                 float score = bbox[4] ;
                 int category_index = (int)bbox[5];
                 System.out.println("模型路径:"+ score);
                 paint.setColor(colors[category_index % 19]);

                 String text= category_Name[category_index] + ":" +Float.toString(score);
                 canvas.drawText(text,x, y,textPaint);
                 canvas.drawRect(x, y, x2, y2, paint);
             }

            imageView.setImageBitmap(rgba);
//            textView.setText(category_index.);
        }
    }

    private boolean copyAssetAndWrite(String fileName) throws Exception{
        try {
            File cacheDir=getCacheDir();
            if (!cacheDir.exists()){
                cacheDir.mkdirs();
            }
            File outFile =new File(cacheDir,fileName);
            if (!outFile.exists()){
                boolean res=outFile.createNewFile();
                if (!res){
                    return false;
                }
            }else {
                if (outFile.length()>10)
                {
                    //表示已经写入一次
                    return true;
                }
            }
            InputStream is=getAssets().open(fileName);
            FileOutputStream fos = new FileOutputStream(outFile);
            byte[] buffer = new byte[1024];
            int byteCount;
            while ((byteCount = is.read(buffer)) != -1) {
                fos.write(buffer, 0, byteCount);
            }
            fos.flush();
            is.close();
            fos.close();
            return true;
        } catch (IOException e) {
            e.printStackTrace();
        }

        return false;
    }

}
