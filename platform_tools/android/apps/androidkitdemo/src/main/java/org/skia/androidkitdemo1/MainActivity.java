/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkitdemo1;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.ImageView;
import java.io.InputStream;
import org.skia.androidkit.*;

public class MainActivity extends Activity implements SurfaceHolder.Callback {
    public Surface surfaceSurface;

    static {
        System.loadLibrary("androidkit");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Paint p = new Paint();
        p.setColor(new Color(0, 1, 0, 1));

        // Bitmap
        Bitmap.Config conf = Bitmap.Config.ARGB_8888;
        Bitmap bmp = Bitmap.createBitmap(400, 400, conf);
        Surface bitmapSurface = new Surface(bmp);
        Canvas canvas = bitmapSurface.getCanvas();

        canvas.drawRect(0, 0, 100, 100, p);

        float[] m = {1, 0, 0, 100,
                     0, 1, 0, 100,
                     0, 0, 1,   0,
                     0, 0, 0,   1};
        p.setColor(new Color(0, 0, 1, 1));
        canvas.save();
        canvas.concat(m);
        canvas.drawRect(0, 0, 100, 100, p);
        canvas.restore();

        Image snapshot = bitmapSurface.makeImageSnapshot();
        canvas.drawImage(snapshot, 0, 200);

        try {
            InputStream is = getResources().openRawResource(R.raw.brickwork_texture);
            byte[] data = new byte[is.available()];
            is.read(data);

            Image image = Image.fromEncoded(data);
            // TODO: Canvas.scale
            canvas.concat(new Matrix().scale(10, 10));
            canvas.drawImage(image, 20, 0, SamplingOptions.CATMULLROM());
        } catch (Exception e) {
            Log.e("AndroidKit Demo", "Could not load Image resource: " + R.raw.brickwork_texture);
        }

        ImageView image = findViewById(R.id.image);
        image.setImageBitmap(bmp);

        //Surface
        SurfaceView surfaceView = findViewById(R.id.surface);
        surfaceView.getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        surfaceSurface = Surface.createThreadedSurface(holder.getSurface());
        surfaceSurface.getCanvas().drawColor(0xffffffe0);
        surfaceSurface.flushAndSubmit();
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }
}
