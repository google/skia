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
import android.view.KeyEvent;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class SampleView extends GLSurfaceView {

    private final SampleApp mApp;

    public SampleView(SampleApp app) {
        super(app);
        mApp = app;
        setEGLContextClientVersion(2);
        setEGLConfigChooser(8,8,8,8,0,8);
        setRenderer(new SampleView.Renderer());
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public void postInval() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mApp.postInval();
            }
        });
    }

    // Called by JNI
    @SuppressWarnings("unused")
    private void queueSkEvent() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mApp.processSkEvent();
            }
        });
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int count = event.getPointerCount();
        for (int i = 0; i < count; i++) {
            final float x = event.getX(i);
            final float y = event.getY(i);
            final int owner = event.getPointerId(i);
            int action = event.getAction() & MotionEvent.ACTION_MASK;
            switch (action) {
                case MotionEvent.ACTION_POINTER_UP:
                    action = MotionEvent.ACTION_UP;
                    break;
                case MotionEvent.ACTION_POINTER_DOWN:
                    action = MotionEvent.ACTION_DOWN;
                    break;
                default:
                    break;
            }
            final int finalAction = action;
            queueEvent(new Runnable() {
               @Override
               public void run() {
                   mApp.handleClick(owner, x, y, finalAction);
               }
            });
        }
        return true;
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
