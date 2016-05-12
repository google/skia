/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.viewer;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

public class ViewerActivity
        extends Activity implements SurfaceHolder.Callback, View.OnTouchListener {
    private static final float FLING_VELOCITY_THRESHOLD = 1000;

    private SurfaceView mView;
    private ViewerApplication mApplication;
    private GestureDetector mGestureDetector;

    private native void onSurfaceCreated(long handle, Surface surface);
    private native void onSurfaceChanged(long handle, Surface surface);
    private native void onSurfaceDestroyed(long handle);
    private native void onKeyPressed(long handle, int keycode);

    private class GestureListener extends GestureDetector.SimpleOnGestureListener {
        @Override
        public boolean onDown(MotionEvent e) {
            return true;
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
            if (Math.abs(velocityX) > Math.abs(velocityY)
                && Math.abs(velocityX) > FLING_VELOCITY_THRESHOLD) {
                if (velocityX > 0) {
                    // Fling right
                    onKeyPressed(mApplication.getNativeHandle(), KeyEvent.KEYCODE_SOFT_RIGHT);
                } else {
                    // Fling left
                    onKeyPressed(mApplication.getNativeHandle(), KeyEvent.KEYCODE_SOFT_LEFT);
                }
                return true;
            }
            return false;
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mApplication = (ViewerApplication) getApplication();
        mView = (SurfaceView) findViewById(R.id.surfaceView);
        mView.getHolder().addCallback(this);

        mGestureDetector = new GestureDetector(getApplicationContext(), new GestureListener());
        mView.setOnTouchListener(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if (mApplication.getNativeHandle() != 0) {
            onSurfaceCreated(mApplication.getNativeHandle(), holder.getSurface());
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (mApplication.getNativeHandle() != 0) {
            onSurfaceChanged(mApplication.getNativeHandle(), holder.getSurface());
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if (mApplication.getNativeHandle() != 0) {
            onSurfaceDestroyed(mApplication.getNativeHandle());
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        return mGestureDetector.onTouchEvent(event);
    }
}
