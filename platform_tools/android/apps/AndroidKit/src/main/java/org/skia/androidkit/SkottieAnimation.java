/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import org.skia.androidkit.Canvas;

public class SkottieAnimation {
    private long mNativeInstance;

    /**
      * Create an animation from the provided JSON string.
      */
    public SkottieAnimation(String animation_json) {
        mNativeInstance = nCreate(animation_json);
    }

    /**
      * Returns the animation duration in seconds.
      */
    public double getDuration() {
        return nGetDuration(mNativeInstance);
    }

    /**
     * Returns the animation frame count.  This is normally an integral value,
     * but both the JSON encoding and Skottie's frame-based APIs support fractional frames.
     */
    public double getFrameCount() {
        return nGetFrameCount(mNativeInstance);
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
      * Update the animation state to match |t|, specifed in seconds.
      * The input is clamped to [0..duration).
      */
    public void seekTime(double t) {
        nSeekTime(mNativeInstance, t);
    }

    /**
     * Update the animation state to match |f|, specified as a frame index
     * in the [0..frameCount) domain.
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
      * Draw the current frame to the Canvas.
      */
    public void render(Canvas canvas) {
        nRender(mNativeInstance, canvas.getNativeInstance());
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

    private static native long nCreate(String json);
    private static native void nRelease(long nativeInstance);

    private static native double nGetDuration(long nativeInstance);
    private static native double nGetFrameCount(long nativeInstance);
    private static native float  nGetWidth(long nativeInstance);
    private static native float  nGetHeight(long nativeInstance);

    private static native void nSeekTime(long nativeInstance, double t);
    private static native void nSeekFrame(long nativeInstance, double frame);
    private static native void nRender(long nativeInstance, long nativeCanvas);
}
