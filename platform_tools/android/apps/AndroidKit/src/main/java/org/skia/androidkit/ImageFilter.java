/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

public class ImageFilter {
    private long mNativeInstance;

    private ImageFilter(long native_instance) {
        mNativeInstance = native_instance;
    }

    /**
     *  Create a filter that calculates the diffuse illumination from a distant light source,
     *  interpreting the alpha channel of the input as the height profile of the surface (to
     *  approximate normal vectors).
     *  @param x, y, z      The direction to the distance light.
     *  @param c            The color of the diffuse light source.
     *  @param surfaceScale Scale factor to transform from alpha values to physical height.
     *  @param kd           Diffuse reflectance coefficient.
     *  @param input        The input filter that defines surface normals (as alpha), or uses the
     *                      source bitmap when null.
     */
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
     *  Create a filter that calculates the diffuse illumination from a distant light source,
     *  interpreting the alpha channel of the input as the height profile of the surface (to
     *  approximate normal vectors).
     *  @param sigmaX   The Gaussian sigma value for blurring along the X axis.
     *  @param sigmaY   The Gaussian sigma value for blurring along the Y axis.
     *  @param tileMode The tile mode applied at edges
     *  @param input    The input filter that is blurred, uses source bitmap if this is null.
     */
    public static ImageFilter blur(float sigmaX, float sigmaY, TileMode tileMode, @Nullable ImageFilter input) {
        long nativeInput = 0;
        if (input != null) {
            nativeInput = input.getNativeInstance();
        }
        return new ImageFilter(nBlur(sigmaX, sigmaY, tileMode.nativeInt, nativeInput));
    }

    /**
     *  Create a filter that draws a drop shadow under the input content. This filter produces an
     *  image that includes the inputs' content.
     *  @param dx       The X offset of the shadow.
     *  @param dy       The Y offset of the shadow.
     *  @param sigmaX   The blur radius for the shadow, along the X axis.
     *  @param sigmaY   The blur radius for the shadow, along the Y axis.
     *  @param c    The color of the drop shadow.
     *  @param input    The input filter, or will use the source bitmap if this is null.
     */
    public static ImageFilter dropShadow(float dx, float dy,
                                         float sigmaX, float sigmaY,
                                         Color c, @Nullable ImageFilter input) {
        long nativeInput = 0;
        if (input != null) {
            nativeInput = input.getNativeInstance();
        }
        return new ImageFilter(nDropShadow(dx, dy, sigmaX, sigmaY, c.r(), c.g(), c.b(), nativeInput));
    }

    public static ImageFilter blend(BlendMode blendMode, @Nullable ImageFilter background,
                                                         @Nullable ImageFilter foreground) {
        long nativeBackground = 0;
        long nativeForeground = 0;
        if (background != null) {
            nativeBackground = background.getNativeInstance();
        }
        if (foreground != null) {
            nativeForeground = foreground.getNativeInstance();
        }
        return new ImageFilter(nBlend(blendMode.nativeInt, nativeBackground, nativeForeground));
    }

    public static ImageFilter image(@NonNull Image image) {
        return new ImageFilter(nImage(image.getNativeInstance()));
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
    private static native long nBlur(float sigmaX, float sigmaY, int tileMode, long native_input);
    private static native long nDropShadow(float dx, float dy, float sigmaX, float sigmaY,
                                           float r, float g, float b, long native_input);
    private static native long nBlend(int blendMode, long native_background, long native_foreground);
    private static native long nImage(long native_image);
}
