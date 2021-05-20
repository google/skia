/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

public class Image {
    private long mNativeInstance;

    /**
     * Construct an Image from encoded (PNG, GIF, etc) data.
     *
     * Returns null for unsupported formats or invalid data.
     */
    public static Image fromEncoded(byte[] encodedData) {
        long nativeImage = nCreate(encodedData);
        return nativeImage != 0
            ? new Image(nativeImage)
            : null;
    }

    public int getWidth() {
        return nGetWidth(mNativeInstance);
    }

    public int getHeight() {
        return nGetHeight(mNativeInstance);
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
    Image(long nativeInstance) {
        mNativeInstance = nativeInstance;
    }

    // package private
    long getNativeInstance() { return mNativeInstance; }

    private static native long nCreate(byte[] encodedData);
    private static native void nRelease(long nativeInstance);

    private static native int nGetWidth(long nativeInstance);
    private static native int nGetHeight(long nativeInstance);
}
