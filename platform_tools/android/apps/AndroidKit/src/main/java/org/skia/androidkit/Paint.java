/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import org.skia.androidkit.Color;
import org.skia.androidkit.Shader;

public class Paint {
    private long mNativeInstance;

    public Paint() {
        mNativeInstance = nCreate();
    }

    public Paint setColor(Color c) {
        nSetColor(mNativeInstance, c.r(), c.g(), c.b(), c.a());
        return this;
    }

    public Paint setShader(Shader shader) {
        nSetShader(mNativeInstance, shader != null ? shader.getNativeInstance() : 0);
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
    private static native void nSetShader(long nativeInstance, long nativeShader);
}
