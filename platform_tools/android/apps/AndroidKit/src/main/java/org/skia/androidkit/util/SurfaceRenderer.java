/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit.util;

import android.util.Log;
import android.view.SurfaceHolder;
import org.skia.androidkit.Canvas;
import org.skia.androidkit.Surface;

/**
  * Utility base class facilitating the implementation of Surface-bound animations.
  *
  * Provides a dedicated render thread and user content callbacks.
  */
public abstract class SurfaceRenderer implements SurfaceHolder.Callback, Runnable {
    private android.view.Surface mAndroidSurface;
    private Thread               mRenderThread;
    private boolean              mThreadRunning;
    private long                 mTimeBase;

    /**
      * Initialization callback.
      *
      * This can be invoked multiple times if the underlying surface changes.
      */
    protected abstract void onSurfaceInitialized(Surface surface);

    /**
      * Callback for frame content.
      *
      * Invoked once per (vsync'ed) frame.
      */
    protected abstract void onRenderFrame(Canvas canvas, long ms);

    @Override
    public void run() {
        Log.d("SurfaceRenderer", "Render thread started.");

        // TODO: Vulkan support?
        Surface surface = Surface.CreateGL(mAndroidSurface);
        onSurfaceInitialized(surface);

        mTimeBase = java.lang.System.currentTimeMillis();

        while (mThreadRunning) {
            long timestamp = java.lang.System.currentTimeMillis() - mTimeBase;
            onRenderFrame(surface.getCanvas(), timestamp);
            surface.flushAndSubmit();
        }

        // Ensure that the backing surface is released on the same thread.
        surface.release();

        Log.d("SurfaceRenderer", "Render thread finished.");
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // Initialization handled in surfaceChanged().
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        mAndroidSurface = holder.getSurface();

        stopRenderThread();
        startRenderThread();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        stopRenderThread();
    }

    private void startRenderThread() {
        if (!mThreadRunning) {
            mThreadRunning = true;
            mRenderThread = new Thread(this);
            mRenderThread.start();
        }
    }

    private void stopRenderThread() {
        if (mThreadRunning) {
            mThreadRunning = false;
            try {
                mRenderThread.join();
            } catch (InterruptedException e) {}
        }
    }

    public void setBaseTime(long millis) {
        mTimeBase = millis;
    }

    public void release() {
        stopRenderThread();
    }
}
