/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.viewer;

import android.app.Activity;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class ViewerActivity extends Activity implements SurfaceHolder.Callback {
    private SurfaceView mView;
    private ViewerApplication mApplication;

    private native void onSurfaceCreated(long handle, Surface surface);
    private native void onSurfaceChanged(long handle, Surface surface);
    private native void onSurfaceDestroyed(long handle);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mApplication = (ViewerApplication) getApplication();
        mView = (SurfaceView) findViewById(R.id.surfaceView);
        mView.getHolder().addCallback(this);
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
}
