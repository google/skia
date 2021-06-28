/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

public class ColorFilter {
    private long mNativeInstance;

    /**
     * Releases any resources associated with this ColorFilter.
     */
    public void release() {
        nRelease(mNativeInstance);
        mNativeInstance = 0;
    }

    @Override
    protected void finalize() throws Throwable {
        release();
    }

    protected ColorFilter(long native_instance) {
        mNativeInstance = native_instance;
    }

    long getNativeInstance() { return mNativeInstance; }

    private static native void nRelease(long nativeInstance);
}
