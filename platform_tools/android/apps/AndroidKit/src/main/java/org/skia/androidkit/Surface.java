/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import android.graphics.Bitmap;
import org.skia.androidkit.Canvas;

public class Surface {
    private long mNativeInstance;
    private Canvas mCanvas;

    /**
     * Create a Surface backed by the provided Bitmap.
     *
     * The Bitmap must be mutable and its pixels are locked for the lifetime of the Surface.
     */
    public Surface(Bitmap bitmap) {
        this(CreateBitmapInstance(bitmap));
    }

    /**
     * The Canvas associated with this Surface.
     */
    public Canvas getCanvas() {
        return mCanvas;
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
        mCanvas = new Canvas(this, nGetNativeCanvas(native_instance));
    }

    private static long CreateBitmapInstance(Bitmap bitmap) {
        if (!bitmap.isMutable()) {
            throw new IllegalStateException("Immutable bitmap passed to Surface constructor");
        }
        return nCreateBitmap(bitmap);
    }

    private static native long nCreateBitmap(Bitmap bitmap);
    private static native void nRelease(long nativeInstance);
    private static native long nGetNativeCanvas(long nativeInstance);
}
