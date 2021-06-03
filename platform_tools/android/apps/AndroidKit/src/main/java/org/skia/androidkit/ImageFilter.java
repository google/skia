/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import android.support.annotation.Nullable;

public class ImageFilter {
    private long mNativeInstance;

    private ImageFilter(long native_instance) {
        mNativeInstance = native_instance;
    }

    public static ImageFilter distantLitDiffuse(float x, float y, float z, Color c,
                                                float surfaceScale, float kd, @Nullable ImageFilter input) {
        long nativeInput = 0;
        if (input != null) {
            nativeInput = input.getNativeInstance();
        }
        return new ImageFilter(nDistantLitDiffuse(x, y, z, c.r(), c.g(), c.b(),
                                                  surfaceScale, kd, nativeInput));
    }
    /**
     * Releases any resources associated with this Shader.
     */
    public void release() {
        nRelease(mNativeInstance);
        mNativeInstance = 0;
    }

    @Override
    protected void finalize() throws Throwable {
        release();
    }

    // package private
    long getNativeInstance() { return mNativeInstance; }

    private static native void nRelease(long nativeInstance);
    private static native long nDistantLitDiffuse(float x, float y, float z,
                                                  float r, float g, float b,
                                                  float surfaceScale, float kd,
                                                  long native_input);
}
