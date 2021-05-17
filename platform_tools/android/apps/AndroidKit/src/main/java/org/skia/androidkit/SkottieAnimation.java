/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

public class SkottieAnimation {
    private long mNativeInstance;

    /**
      * Create an animation from the provided JSON string.
      */
    public SkottieAnimation(String animation_json) {
        mNativeInstance = nCreate(animation_json);
    }

    /**
      * Returns the animation frame rate (frames / second).
      */
    public double getFPS() {
        return nGetFPS(mNativeInstance);
    }

    /**
      * Returns the animation duration in seconds.
      */
    public double getDuration() {
        return nGetDuration(mNativeInstance);
    }

    /**
      * Returns the intrinsic animation width.
      */
    public float getWidth() {
        return nGetWidth(mNativeInstance);
    }

    /**
      * Returns the intrinsic animation height.
      */
    public float getHeight() {
        return nGetHeight(mNativeInstance);
    }

    /**
     * Update the animation state to match |f|, specified as a frame index
     * i.e. relative to getDuration() * getFPS().
     *
     * Fractional values are allowed and meaningful - e.g.
     *
     *   0.0 -> first frame
     *   1.0 -> second frame
     *   0.5 -> halfway between first and second frame
     */
    public void seekFrame(double f) {
        nSeekFrame(mNativeInstance, f);
    }

    /**
      * Update the animation state to match t, specifed in seconds.
      */
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
