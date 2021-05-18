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
import java.io.InputStream;
import org.skia.androidkit.*;

// TODO: refactor to share w/ other activities
class SkottieAnimationRenderThread extends Thread {
    private android.view.Surface mAndroidSurface;
    private SkottieAnimation     mAnimation;
    private Matrix               mAnimationMatrix;
    private Surface              mSurface;
    private boolean              mRunning;

    private static final String TAG = "*** AK RenderThread";

    public SkottieAnimationRenderThread(android.view.Surface surface, SkottieAnimation animation) {
        mAndroidSurface = surface;
        mAnimation = animation;
    }

    public void finish() {
        mRunning = false;
    }

    @Override
    public void run() {
        mRunning = true;

        Log.d(TAG, "start");

        long time_base = java.lang.System.currentTimeMillis();

        Surface surface = Surface.CreateGL(mAndroidSurface);

        // Scale to fit/center.
        float sx = surface.getWidth()  / mAnimation.getWidth();
        float sy = surface.getHeight() / mAnimation.getHeight();
        float s = Math.min(sx, sy);
        mAnimationMatrix = new Matrix()
            .translate((surface.getWidth()  - s * mAnimation.getWidth())  / 2,
                       (surface.getHeight() - s * mAnimation.getHeight()) / 2)
            .scale(s, s);

        while (mRunning) {
            renderFrame(surface.getCanvas(),
                        (double)(java.lang.System.currentTimeMillis() - time_base) / 1000,
                        surface.getWidth(), surface.getHeight());
            surface.flushAndSubmit();
        }

        surface.release();
        Log.d(TAG, "finish");
    }

    private void renderFrame(Canvas canvas, double t, int canvas_width, int canvas_height) {
        t = t % mAnimation.getDuration();
        mAnimation.seekTime(t);

        canvas.save();
        canvas.concat(mAnimationMatrix);

        canvas.drawColor(1, 1, 1, 1);
        mAnimation.render(canvas);

        canvas.restore();
    }
}

public class SkottieAnimationActivity extends Activity implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("androidkit");
    }

    private SkottieAnimationRenderThread mRenderThread;
    private SkottieAnimation             mAnimation;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_animation);

        SurfaceView sv = findViewById(R.id.surfaceView);
        sv.getHolder().addCallback(this);

        try {
            InputStream is = getResources().openRawResource(R.raw.im_thirsty);
            byte[] data = new byte[is.available()];
            is.read(data);
            mAnimation = new SkottieAnimation(new String(data));
        } catch (Exception e) {
            Log.e("AndroidKit", "Could not load resource: " + R.raw.im_thirsty);
        }
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

        mRenderThread = new SkottieAnimationRenderThread(holder.getSurface(), mAnimation);;
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
