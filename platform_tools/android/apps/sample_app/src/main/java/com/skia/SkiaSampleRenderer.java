/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package com.skia;

import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.util.Log;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.opengles.GL11;

public class SkiaSampleRenderer implements GLSurfaceView.Renderer {

    private final SkiaSampleView mSampleView;
    private Handler mHandler = new Handler();
    private int mMSAASampleCount;
    private String mCmdLineFlags;

    SkiaSampleRenderer(SkiaSampleView view, String cmdLineFlags) {
        mSampleView = view;
        mCmdLineFlags = cmdLineFlags;
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        draw();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        updateSize(width, height);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        if (gl instanceof GL11) {
            int value[] = new int[1];
            ((GL11) gl).glGetIntegerv(GL11.GL_SAMPLES, value, 0);
            if (value[0] == 1) {
                mMSAASampleCount = 0;
            } else {
                mMSAASampleCount = value[0];
            }
        }

        gl.glClearStencil(0);
        gl.glClear(GL10.GL_STENCIL_BUFFER_BIT);

        init((SkiaSampleActivity)mSampleView.getContext(), mCmdLineFlags, mMSAASampleCount);
    }

    // Called by JNI and the view.
    synchronized public int getMSAASampleCount() {
        return mMSAASampleCount;
    }

    // Called by JNI
    private void startTimer(int ms) {
        // After the delay, queue an event to the Renderer's thread
        // to handle the event on the timer queue
        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                mSampleView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        serviceQueueTimer();
                    }
                });
            }
        }, ms);
    }

    // Called by JNI
    private void queueSkEvent() {
        mSampleView.queueEvent(new Runnable() {
            @Override
            public void run() {
                processSkEvent();
            }
        });
    }

    // Called by JNI
    private void requestRender() {
        mSampleView.requestRender();
    }

    native void init(SkiaSampleActivity activity, String flags, int msaaSampleCount);
    native void term();
    native void draw();
    native void updateSize(int w, int h);
    native void handleClick(int owner, float x, float y, int state);
    native void showOverview();
    native void nextSample();
    native void previousSample();
    native void goToSample(int position);
    native void toggleRenderingMode();
    native void toggleSlideshow();
    native void toggleFPS();
    native void toggleTiling();
    native void toggleBBox();
    native void processSkEvent();
    native void serviceQueueTimer();
    native void saveToPDF();
    native void postInval();
}