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
    private Surface mSurface;


    public RenderThread(android.view.Surface surface) {
        mAndroidSurface = surface;
    }

    @Override
    public void run() {
        Log.e("*** AK", "RenderThread start");

        Surface surface = new Surface(mAndroidSurface);

        Paint p = new Paint();

        surface.getCanvas().drawRect(100, 100, 500, 300, p);
        // android.graphics.Canvas canvas = mAndroidSurface.lockHardwareCanvas();

        // canvas.drawColor(0xff00ff00);

        // mAndroidSurface.unlockCanvasAndPost(canvas);

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
            try {
                mRenderThread.join();
            } catch (InterruptedException e) {}
        }
    }
}
