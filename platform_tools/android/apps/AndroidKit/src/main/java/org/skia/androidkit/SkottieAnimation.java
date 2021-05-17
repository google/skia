/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

public class SkottieAnimation {
    private long mNativeInstance;

    public SkottieAnimation(String animation_json) {
        mNativeInstance = nCreate(animation_json);
    }

    public double getFPS() {
        return nGetFPS(mNativeInstance);
    }

    public double getDuration() {
        return nGetDuration(mNativeInstance);
    }

    public float getWidth() {
        return nGetWidth(mNativeInstance);
    }

    public float getHeight() {
        return nGetHeight(mNativeInstance);
    }

    public void seekFrame(double frame) {
        nSeekFrame(mNativeInstance, frame);
    }

    public void seekTime(double t) {
        nSeekTime(mNativeInstance, t);
    }

    /**
     * Releases any resources associated with this Animation.
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

    private static native long nCreate(String json);
    private static native void nRelease(long nativeInstance);

    private static native double nGetFPS(long nativeInstance);
    private static native double nGetDuration(long nativeInstance);
    private static native float  nGetWidth(long nativeInstance);
    private static native float  nGetHeight(long nativeInstance);

    private static native void nSeekFrame(long nativeInstance, double frame);
    private static native void nSeekTime(long nativeInstance, double t);
}
