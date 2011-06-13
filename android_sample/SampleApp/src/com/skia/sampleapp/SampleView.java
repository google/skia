/*
 * Copyright (C) 2011 Skia
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.skia.sampleapp;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.ScaleGestureDetector.OnScaleGestureListener;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class SampleView extends GLSurfaceView implements OnScaleGestureListener {
    
    private final SampleApp mApp;
    private ScaleGestureDetector mDetector;
    public SampleView(SampleApp app) {
        super(app);
        mApp = app;
        setEGLContextClientVersion(2);
        setEGLConfigChooser(8,8,8,8,0,8);
        setRenderer(new SampleView.Renderer());
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        mDetector = new ScaleGestureDetector(app, this);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        mDetector.onTouchEvent(event);
        if (mDetector.isInProgress()) {
            return true;
        }

        final int x = (int) event.getX();
        final int y = (int) event.getY();
        final int action = event.getAction();
        queueEvent(new Runnable() {
           @Override
           public void run() {
               mApp.handleClick(x, y, action);
           }
        });
        
        return true;
    }
    // ScaleGestureDetector.OnScaleGestureListener implementation
    @Override
    public boolean onScaleBegin(ScaleGestureDetector detector) {
        return true;
    }

    @Override
    public boolean onScale(ScaleGestureDetector detector) {
        if (detector.getScaleFactor() != 1) {
            final float difference = detector.getCurrentSpan() - detector.getPreviousSpan();
            queueEvent(new Runnable() {
                @Override
                public void run() {
                    mApp.zoom(difference * .03f);
                }
            });

            return true;
        }
        return false;
    }

    @Override
    public void onScaleEnd(ScaleGestureDetector detector) {

    }

    private class Renderer implements GLSurfaceView.Renderer {
        public void onDrawFrame(GL10 gl) {
            mApp.draw();
        }
        
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            mApp.updateSize(width, height);
        }
        
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            gl.glClearStencil(0);
            gl.glClear(gl.GL_STENCIL_BUFFER_BIT);
            mApp.init();
            mApp.createOSWindow(SampleView.this);
        }
    }
}