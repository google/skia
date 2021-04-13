/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import org.skia.androidkit.Paint;
import org.skia.androidkit.Surface;

public class Canvas {
    private long mNativeInstance;
    private Surface mSurface;

    public void drawRect(float left, float right, float top, float bottom, Paint paint) {
        nDrawRect(mNativeInstance, left, right, top, bottom, paint.getNativeInstance());
    }

    // package private
    Canvas(Surface surface, long native_instance) {
        mNativeInstance = native_instance;
        mSurface = surface;
    }

    private static native void nDrawRect(long nativeInstance,
                                         float left, float right, float top, float bottom,
                                         long nativePaint);
}
