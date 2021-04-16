package org.skia.akdemo2;

import android.app.Activity;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import org.skia.androidkit.Canvas;
import org.skia.androidkit.Surface;

class RenderThread extends Thread {
    private android.view.Surface mAndroidSurface;
    private Surface              mSurface;
    private boolean              mRunning;

    public RenderThread(android.view.Surface surface) {
        mAndroidSurface = surface;
    }

    public void finish() {
        mRunning = false;
    }

    @Override
    public void run() {
        mRunning = true;

        Log.e("*** AK", "RenderThread start");

        Surface surface = new Surface(mAndroidSurface);
        Canvas canvas = surface.getCanvas();

        Paint p = new Paint();

        long time_base = java.lang.System.currentTimeMillis();

        // TODO: get from canvas
        final float kWidth = 1000;
    
        while (mRunning) {
            canvas.clear(0xff2200aa);
            double t = (double)(java.lang.System.currentTimeMillis() - time_base) / 300;
            float x =  (float)(java.lang.Math.cos(t) + 1) * kWidth / 2;
            canvas.drawRect(x - 200, 100, x + 200, 300, p);
            surface.swapBuffers();
        }

        surface.release();

        Log.e("*** AK", "RenderThread done");
    }
}

public class MainActivity extends Activity implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("androidkit");
    }

    private RenderThread mRenderThread;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        SurfaceView sv = findViewById(R.id.surfaceView);
        sv.getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.e("*** AK", "surfaceCreated");
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.e("*** AK", "surfaceChanged");

        if (mRenderThread != null) {
            mRenderThread.finish();
            try {
                mRenderThread.join();
            } catch (InterruptedException e) {}
        }

        mRenderThread = new RenderThread(holder.getSurface());;
        mRenderThread.start();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.e("*** AK", "surfaceDestroyed");

        if (mRenderThread != null) {
            mRenderThread.finish();
            try {
                mRenderThread.join();
            } catch (InterruptedException e) {}
        }
    }
}
