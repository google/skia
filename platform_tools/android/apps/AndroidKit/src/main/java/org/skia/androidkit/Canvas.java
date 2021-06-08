/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import org.skia.androidkit.Color;
import org.skia.androidkit.Image;
import org.skia.androidkit.Matrix;
import org.skia.androidkit.Paint;
import org.skia.androidkit.SamplingOptions;
import org.skia.androidkit.Surface;

public class Canvas {
    private long mNativeInstance;
    private Surface mSurface;

    public int getWidth() {
        return nGetWidth(mNativeInstance);
    }

    public int getHeight() {
        return nGetHeight(mNativeInstance);
    }

    public int save() {
        return nSave(mNativeInstance);
    }

    public void restore() {
        nRestore(mNativeInstance);
    }

    public int saveLayer() {
        return nSaveLayer(mNativeInstance);
    }

    public Matrix getLocalToDevice() {
        long native_matrix = nGetLocalToDevice(mNativeInstance);
        return new Matrix(native_matrix);
    }

    public void concat(Matrix m) {
        nConcat(mNativeInstance, m.getNativeInstance());
    }

    public void concat(float[] rowMajorMatrix) {
        if (rowMajorMatrix.length != 16) {
            throw new java.lang.IllegalArgumentException("Expecting a 16 float array.");
        }

        nConcat16f(mNativeInstance, rowMajorMatrix);
    }

    public void translate(float tx, float ty, float tz) {
        nTranslate(mNativeInstance, tx ,ty, tz);
    }

    public void translate(float tx, float ty) {
        nTranslate(mNativeInstance, tx ,ty, 0);
    }

    public void scale(float sx, float sy, float sz) {
        nScale(mNativeInstance, sx ,sy, sz);
    }

    public void scale(float sx, float sy) {
        nScale(mNativeInstance, sx ,sy, 1);
    }

    public void drawRect(float left, float top, float right, float bottom, Paint paint) {
        nDrawRect(mNativeInstance, left, top, right, bottom, paint.getNativeInstance());
    }

    public void drawColor(Color c) {
        nDrawColor(mNativeInstance, c.r(), c.g(), c.b(), c.a());
    }

    public void drawColor(float r, float g, float b, float a) {
        nDrawColor(mNativeInstance, r, g, b, a);
    }

    public void drawColor(int icolor) {
        nDrawColor(mNativeInstance,
            (float)((icolor >> 16) & 0xff) / 255,
            (float)((icolor >>  8) & 0xff) / 255,
            (float)((icolor >>  0) & 0xff) / 255,
            (float)((icolor >> 24) & 0xff) / 255
        );
    }

    public void drawImage(Image image, float x, float y) {
        drawImage(image, x, y, new SamplingOptions());
    }

    public void drawImage(Image image, float x, float y, SamplingOptions sampling) {
        nDrawImage(mNativeInstance, image.getNativeInstance(), x, y,
                   sampling.getNativeDesc(), sampling.getCubicCoeffB(), sampling.getCubicCoeffC());
    }

    // package private
    Canvas(Surface surface, long native_instance) {
        mNativeInstance = native_instance;
        mSurface = surface;
    }

    // package private
    long getNativeInstance() { return mNativeInstance; }

    private static native int  nGetWidth(long nativeInstance);
    private static native int  nGetHeight(long nativeInstance);
    private static native int  nSave(long nativeInstance);
    private static native void nRestore(long nativeInstance);
    private static native int  nSaveLayer(long nativeInstance);
    private static native long nGetLocalToDevice(long mNativeInstance);
    private static native void nConcat(long nativeInstance, long nativeMatrix);
    private static native void nConcat16f(long nativeInstance, float[] floatMatrix);
    private static native void nTranslate(long nativeInstance, float tx, float ty, float tz);
    private static native void nScale(long nativeInstance, float sx, float sy, float sz);

    private static native void nDrawColor(long nativeInstance, float r, float g, float b, float a);

    private static native void nDrawRect(long nativeInstance,
                                         float left, float top, float right, float bottom,
                                         long nativePaint);
    private static native void nDrawImage(long nativeInstance, long nativeImage, float x, float y,
                                          int samplingDesc,
                                          float samplingCoeffB, float samplingCoeffC);
}
