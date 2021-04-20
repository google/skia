/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import org.skia.androidkit.Color;
import org.skia.androidkit.Paint;
import org.skia.androidkit.Surface;

public class Canvas {
    private long mNativeInstance;
    private Surface mSurface;

    public void drawRect(float left, float right, float top, float bottom, Paint paint) {
        nDrawRect(mNativeInstance, left, right, top, bottom, paint.getNativeInstance());
    }

    public void drawColor(Color c) {
        nDrawColor(mNativeInstance, c.r(), c.g(), c.b(), c.a());
    }

    public void drawColor(float r, float g, float b, float a) {
        nDrawColor(mNativeInstance, r, g, b, a);
    }

    public void drawColor(int icolor) {
        nDrawColor(mNativeInstance,
            (float)((icolor >> 16) & 0xff) / 255,
            (float)((icolor >>  8) & 0xff) / 255,
            (float)((icolor >>  0) & 0xff) / 255,
            (float)((icolor >> 24) & 0xff) / 255
        );
    }

    // package private
    Canvas(Surface surface, long native_instance) {
        mNativeInstance = native_instance;
        mSurface = surface;
    }

    private static native void nDrawColor(long nativeInstance, float r, float g, float b, float a);

    private static native void nDrawRect(long nativeInstance,
                                         float left, float right, float top, float bottom,
                                         long nativePaint);
}
