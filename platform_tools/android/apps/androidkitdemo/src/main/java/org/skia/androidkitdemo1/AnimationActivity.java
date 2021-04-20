/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkitdemo1;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import org.skia.androidkit.*;

class RenderThread extends Thread {
    private android.view.Surface mAndroidSurface;
    private Surface              mSurface;
    private boolean              mRunning;

    private static final String TAG = "*** AK RenderThread";

    public RenderThread(android.view.Surface surface) {
        mAndroidSurface = surface;
    }

    public void finish() {
        mRunning = false;
    }

    @Override
    public void run() {
        mRunning = true;

        Log.d(TAG, "start");

        long time_base = java.lang.System.currentTimeMillis();

        // TODO: convert to native AK surface.
        while (mRunning) {
            android.graphics.Canvas android_canvas = mAndroidSurface.lockHardwareCanvas();

            int w = android_canvas.getWidth(),
                h = android_canvas.getHeight();

            android.graphics.Bitmap bm =
                    android.graphics.Bitmap.createBitmap(w, h,
                                                         android.graphics.Bitmap.Config.ARGB_8888,
                                                         true);
            Surface surface = new Surface(bm);
            renderFrame(surface.getCanvas(),
                        (double)(java.lang.System.currentTimeMillis() - time_base) / 1000,
                        w, h);
            surface.release();

            android_canvas.drawBitmap(bm, 0, 0, new android.graphics.Paint());

            mAndroidSurface.unlockCanvasAndPost(android_canvas);
        }

        Log.d(TAG, "finish");
    }

    private void renderFrame(Canvas canvas, double t, int canvas_width, int canvas_height) {
        final float kWidth  = 400,
                    kHeight = 200,
                    kSpeed  = 4;

        canvas.drawColor(0xffffffe0);

        Paint p = new Paint();
        p.setColor(new Color(0, 1, 0, 1));

        float x = (float)(java.lang.Math.cos(t * kSpeed) + 1) * canvas_width/2;
        canvas.drawRect(x - kWidth/2, (canvas_height - kHeight)/2,
                        x + kWidth/2, (canvas_height + kHeight)/2, p);
    }
}

public class AnimationActivity extends Activity implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("androidkit");
    }

    private RenderThread mRenderThread;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_animation);

        SurfaceView sv = findViewById(R.id.surfaceView);
        sv.getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
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
        if (mRenderThread != null) {
            mRenderThread.finish();
            try {
                mRenderThread.join();
            } catch (InterruptedException e) {}
        }
    }
}
