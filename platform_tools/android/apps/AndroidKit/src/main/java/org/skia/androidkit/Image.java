/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import android.support.annotation.Nullable;
import java.io.InputStream;
import org.skia.androidkit.Matrix;
import org.skia.androidkit.SamplingOptions;
import org.skia.androidkit.Shader;
import org.skia.androidkit.TileMode;

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

    /**
     * Construct an Image from an encoded data stream.
     *
     * Returns null for unsupported formats or invalid stream.
     */
    public static Image fromStream(InputStream encodedStream) throws java.io.IOException {
        byte[] encodedData = new byte[encodedStream.available()];
        encodedStream.read(encodedData);

        return fromEncoded(encodedData);
    }

    public int getWidth() {
        return nGetWidth(mNativeInstance);
    }

    public int getHeight() {
        return nGetHeight(mNativeInstance);
    }

    public Shader makeShader(TileMode tmx, TileMode tmy, SamplingOptions sampling) {
        return makeShader(tmx, tmy, sampling, null);
    }

    public Shader makeShader(TileMode tmx, TileMode tmy, SamplingOptions sampling,
                             @Nullable Matrix localMatrix) {
        long nativeMatrix = localMatrix != null ? localMatrix.getNativeInstance() : 0;
        return new Shader(nMakeShader(mNativeInstance, tmx.nativeInt, tmy.nativeInt,
                                      sampling.getNativeDesc(),
                                      sampling.getCubicCoeffB(), sampling.getCubicCoeffC(),
                                      nativeMatrix));
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

    private static native long nMakeShader(long nativeInstance, int tmx, int tmy, int samplingDesc,
                                           float samplingCoeffB, float samplingCoeffC,
                                           long nativeMatrix);
}
