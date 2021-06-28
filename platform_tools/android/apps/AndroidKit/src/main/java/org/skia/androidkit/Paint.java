/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import android.support.annotation.Nullable;

public class Paint {
    private long mNativeInstance;

    public Paint() {
        mNativeInstance = nCreate();
    }

    public Paint setColor(Color c) {
        nSetColor(mNativeInstance, c.r(), c.g(), c.b(), c.a());
        return this;
    }

    public Paint setColor(float r, float g, float b, float a) {
        nSetColor(mNativeInstance, r, g, b, a);
        return this;
    }

    public Paint setColorFilter(@Nullable ColorFilter filter) {
        nSetColorFilter(mNativeInstance, filter != null ? filter.getNativeInstance() : 0);
        return this;
    }

    public Paint setShader(@Nullable Shader shader) {
        nSetShader(mNativeInstance, shader != null ? shader.getNativeInstance() : 0);
        return this;
    }

    public Paint setImageFilter(@Nullable ImageFilter filter) {
        nSetImageFilter(mNativeInstance, filter != null ? filter.getNativeInstance() : 0);
        return this;
    }

    public Paint setStroke(boolean stroke) {
        nSetStroke(mNativeInstance, stroke);
        return this;
    }

    public Paint setStrokeWidth(float w) {
        nSetStrokeWidth(mNativeInstance, w);
        return this;
    }

    public enum Cap {
        BUTT    (0),
        ROUND   (1),
        SQUARE  (2);

        private Cap(int nativeInt) {
            this.nativeInt = nativeInt;
        }
        final int nativeInt;
    }
    public Paint setStrokeCap(Cap cap) {
        nSetStrokeCap(mNativeInstance, cap.nativeInt);
        return this;
    }


    public enum Join {
        MITER   (0),
        ROUND   (1),
        BEVEL   (2);

        private Join(int nativeInt) {
            this.nativeInt = nativeInt;
        }
        final int nativeInt;
    }
    public Paint setStrokeJoin(Join join) {
        nSetStrokeJoin(mNativeInstance, join.nativeInt);
        return this;
    }

    public Paint setStrokeMiter(float limit) {
        nSetStrokeMiter(mNativeInstance, limit);
        return this;
    }

    /**
     * Releases any resources associated with this Paint.
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

    private static native long nCreate();
    private static native void nRelease(long nativeInstance);

    private static native void nSetColor(long nativeInstance, float r, float g, float b, float a);
    private static native void nSetStroke(long nativeInstance, boolean stroke);
    private static native void nSetStrokeWidth(long nativeInstance, float w);
    private static native void nSetStrokeCap(long nativeInstance, int native_cap);
    private static native void nSetStrokeJoin(long nativeInstance, int native_join);
    private static native void nSetStrokeMiter(long nativeInstance, float limit);
    private static native void nSetColorFilter(long nativeInstance, long nativeCF);
    private static native void nSetShader(long nativeInstance, long nativeShader);
    private static native void nSetImageFilter(long nativeInstance, long nativeFilter);
}
