/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// AJAR=$ANDROID_SDK_ROOT/platforms/android-19/android.jar
// SRC=platform_tools/android/apps/canvasproof/src/main
// javac -classpath $AJAR $SRC/java/org/skia/canvasproof/GaneshPictureRenderer.java
// javah -classpath $AJAR:$SRC/java -d $SRC/jni org.skia.canvasproof.GaneshPictureRenderer

package org.skia.canvasproof;

import android.app.Activity;
import android.graphics.Rect;
import android.opengl.GLSurfaceView;
import android.util.Log;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class GaneshPictureRenderer implements GLSurfaceView.Renderer {
    private static final String TAG = "GaneshPictureRenderer";
    private long picturePtr;
    private long contextPtr;
    private float scale;
    private int width;
    private int height;
    private GLSurfaceView view;

    GaneshPictureRenderer() {
        try {
            System.loadLibrary("skia_android");
            System.loadLibrary("canvasproof");
        } catch (java.lang.Error e) {
            Log.e(TAG, "System.loadLibrary error", e);
            return;
        }
        this.scale = 1;
    }
    public GLSurfaceView makeView(Activity activity) {
        this.view = new GLSurfaceView(activity);
        this.view.setEGLConfigChooser(8, 8, 8, 8, 0, 8);
        this.view.setEGLContextClientVersion(2);
        this.view.setRenderer(this);
        this.view.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        return this.view;
    }
    static public Rect cullRect(long picturePtr) {
        Rect rect = new Rect();
        try {
            GaneshPictureRenderer.GetCullRect(rect, picturePtr);
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "GetCullRect failed", e);
        }
        return rect;
    }
    public void setPicture(long picturePtr) {
        this.picturePtr = picturePtr;
        this.view.requestRender();
    }
    public void setScale(float s) { this.scale = s; }

    public void releaseResources() {
        if (this.contextPtr != 0) {
            try {
                GaneshPictureRenderer.CleanUp(this.contextPtr);
            } catch (UnsatisfiedLinkError e) {
                Log.e(TAG, "CleanUp failed", e);
            }
        }
        this.contextPtr = 0;
    }

    private void createContext() {
        try {
            this.contextPtr = GaneshPictureRenderer.Ctor();
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Ctor failed", e);
        }
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig c) {
        this.releaseResources();
        this.createContext();
    }
    @Override
    public void onDrawFrame(GL10 gl) {
        if (this.contextPtr == 0) {
            this.createContext();
        }
        if (this.width > 0 && this.height > 0 &&
            this.contextPtr != 0 && this.picturePtr != 0) {
            try {
                GaneshPictureRenderer.DrawThisFrame(
                        this.width, this.height, this.scale,
                        this.contextPtr, this.picturePtr);
            } catch (UnsatisfiedLinkError e) {
                Log.e(TAG, "DrawThisFrame failed", e);
            }
        }
    }
    @Override
    public void onSurfaceChanged(GL10 gl, int w, int h) {
        this.width = w;
        this.height = h;
    }
    @Override
    public void finalize() throws Throwable {
        super.finalize();
        this.releaseResources();
    }

    // Make the native functions static to simplify JNI interaction.
    private static native void DrawThisFrame(int w, int h, float s, long pr, long pc);
    private static native long Ctor();
    private static native void CleanUp(long p);
    private static native void GetCullRect(Rect r, long picture);
}
