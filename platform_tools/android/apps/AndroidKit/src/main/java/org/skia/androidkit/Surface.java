/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import android.graphics.Bitmap;
import android.os.Build;
import android.support.annotation.RequiresApi;

import org.skia.androidkit.Canvas;
import org.skia.androidkit.Image;

public class Surface {
    private long mNativeInstance;

    /**
     * Create a Surface backed by the provided Bitmap.
     *
     * The Bitmap must be mutable and its pixels are locked for the lifetime of the Surface.
     */
    public Surface(Bitmap bitmap) {
        this(CreateBitmapInstance(bitmap));
    }

    @RequiresApi(Build.VERSION_CODES.N)
    static public Surface CreateVulkan(android.view.Surface surface) {
        return new Surface(nCreateVKSurface(surface));
    }

    static public Surface CreateGL(android.view.Surface surface) {
        return new Surface(nCreateGLSurface(surface));
    }

    /**
     * Create a Surface backed by the provided Android Surface (android.view.Surface).
     * AndroidKit handles thread management. Assumes OpenGL backend.
     */
    static public Surface createThreadedSurface(android.view.Surface surface) {
        return new Surface(nCreateThreadedSurface(surface));
    }

    /**
     * The Canvas associated with this Surface.
     */
    public Canvas getCanvas() {
        // TODO: given that canvases are now ephemeral, it would make sense to be more explicit
        // e.g. lockCanvas/unlockCanvasAndPost?
        return new Canvas(this, nGetNativeCanvas(mNativeInstance));
    }


    /**
     * Returns an Image capturing the Surface contents.
     * Subsequent drawing to Surface contents are not captured.
     */
    public Image makeImageSnapshot() {
        return new Image(nMakeImageSnapshot(mNativeInstance));
    }

    /***
     * Triggers the immediate execution of all pending draw operations.
     *
     * Additionaly, if the backing device is multi-buffered, submits the current
     * buffer to be displayed.
     */
    public void flushAndSubmit() {
        nFlushAndSubmit(mNativeInstance);
    }

    public int getWidth() {
        return nGetWidth(mNativeInstance);
    }

    public int getHeight() {
        return nGetHeight(mNativeInstance);
    }

    /**
     * Releases any resources associated with this Surface.
     */
    public void release() {
        nRelease(mNativeInstance);
        mNativeInstance = 0;
    }

    @Override
    protected void finalize() throws Throwable
    {
        release();
    }

    private Surface(long native_instance) {
        mNativeInstance = native_instance;
    }

    private static long CreateBitmapInstance(Bitmap bitmap) {
        if (!bitmap.isMutable()) {
            throw new IllegalStateException("Immutable bitmap passed to Surface constructor");
        }
        return nCreateBitmap(bitmap);
    }

    private static native long nCreateBitmap(Bitmap bitmap);
    private static native long nCreateThreadedSurface(android.view.Surface surface);
    private static native long nCreateVKSurface(android.view.Surface surface);
    private static native long nCreateGLSurface(android.view.Surface surface);

    private static native void nRelease(long nativeInstance);
    private static native long nGetNativeCanvas(long nativeInstance);
    private static native void nFlushAndSubmit(long nativeInstance);
    private static native int  nGetWidth(long nativeInstance);
    private static native int  nGetHeight(long nativeInstance);
    private static native long nMakeImageSnapshot(long nativeInstance);
}
