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

    public void setColor(Color c) {
        nSetColor(mNativeInstance, c.r(), c.g(), c.b(), c.a());
    }

    public void setShader(Shader shader) {
        nSetShader(mNativeInstance, shader != null ? shader.getNativeInstance() : 0);
    }

    public enum Style {
        // Draws are filled, ignoring stroke settings.
        FILL            (0),
        // Draws are stroked.
        STROKE          (1),
        /**
         * Draws are filled and stroked at the same time.
         * Use this style to avoid hitting the same pixels twice with a stroke draw and
         * a fill draw.
         */
        FILL_AND_STROKE (2);

        Style(int nativeInt) {
            this.nativeInt = nativeInt;
        }
        final int nativeInt;
    }

    public void setStyle(Style style) {
        nSetStyle(mNativeInstance, style.nativeInt);
    }

    public void setStrokeWidth(float w) {
        nSetStrokeWidth(mNativeInstance, w);
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
    private static native void nSetStyle(long nativeInstance, int style);
    private static native void nSetStrokeWidth(long nativeInstance, float w);
    private static native void nSetShader(long nativeInstance, long nativeShader);
}
