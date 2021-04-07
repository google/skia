package org.skia.androidkitdemo1;

import org.skia.androidkit.Canvas;
//import android.graphics.Canvas;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Paint;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "ANDROIDKIT DEMO";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Bitmap.Config conf = Bitmap.Config.ARGB_8888;
        Bitmap bmp = Bitmap.createBitmap(200, 200, conf);
        Canvas canvas = new Canvas(bmp);
        Log.v(TAG, "java canvas initialized");
        Paint p = new Paint();
        p.setColor(Color.RED);
        //canvas.drawRect(0, 0, 100, 100, p);
        ImageView image = findViewById(R.id.image);
        image.setImageBitmap(bmp);
    }
}