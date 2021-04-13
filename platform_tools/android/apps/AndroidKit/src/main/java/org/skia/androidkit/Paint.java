/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import org.skia.androidkit.Color;

public class Paint {
    private long mNativeInstance;

    public Paint() {
        mNativeInstance = nCreate();
    }

    public void setColor(Color c) {
        nSetColor(mNativeInstance, c.r(), c.g(), c.b(), c.a());
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
}
