/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import android.support.annotation.Nullable;
import android.util.Log;

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

    public void restoreToCount(int count) {
        nRestoreToCount(mNativeInstance, count);
    }

    public int saveLayer(@Nullable Paint paint) {
        long nativePaint = 0;
        if (paint != null) {
            nativePaint = paint.getNativeInstance();
        }
        return nSaveLayer(mNativeInstance, nativePaint);
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

    public void clipPath(Path path, ClipOp op, boolean antiAliasing) {
        nClipPath(mNativeInstance, path.getNativeInstance(), op.mNativeInt, antiAliasing);
    }

    public void clipRect(float left, float top, float right, float bottom,
                         ClipOp op, boolean antiAliasing) {
        nClipRect(mNativeInstance, left, top, right, bottom, op.mNativeInt, antiAliasing);
    }

    public void clipRRect(float left, float top, float right, float bottom, float xRad, float yRad,
                         ClipOp op, boolean antiAliasing) {
        nClipRRect(mNativeInstance, left, top, right, bottom, xRad, yRad, op.mNativeInt, antiAliasing);
    }

    public void clipShader(Shader shader, ClipOp op) {
        nClipShader(mNativeInstance, shader.getNativeInstance(), op.mNativeInt);
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

    public void drawPath(Path path, Paint paint) {
        nDrawPath(mNativeInstance, path.getNativeInstance(), paint.getNativeInstance());
    }

    /**
     *
     * @param glyphs: glyph IDs, passed into native code as unsigned 16 bits
     * @param positions: array of floats in the form of [x1, y1, x2, y2, ...]
     *                   for where to draw each glyph relative to origin, should be double the length
     *                   of glyph ID array
     * @param xOrigin: x-value of origin
     * @param yOrigin: y-value of origin
     * @param font: typeface, text size and so, used to describe the text
     * @param paint blend, color, and so on, used to draw
     */
    public void drawGlyphs(char[] glyphs, float[] positions, float xOrigin, float yOrigin,
                           Font font, Paint paint) {
        if (glyphs.length * 2 == positions.length) {
            nDrawGlyphs(mNativeInstance, glyphs, positions, xOrigin, yOrigin,
                        font.getNativeInstance(), paint.getNativeInstance());
        } else {
            throw new IllegalArgumentException("Positions array must be double the length of " +
                    "glyphIDs, one x and y per id");
        }
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
    private static native void nRestoreToCount(long nativeInstance, int count);
    private static native int  nSaveLayer(long nativeInstance, long nativePaint);
    private static native long nGetLocalToDevice(long mNativeInstance);
    private static native void nConcat(long nativeInstance, long nativeMatrix);
    private static native void nConcat16f(long nativeInstance, float[] floatMatrix);
    private static native void nTranslate(long nativeInstance, float tx, float ty, float tz);
    private static native void nScale(long nativeInstance, float sx, float sy, float sz);

    private static native void nClipPath(long nativeInstance, long nativePath, int clipOp,
                                         boolean doAA);
    private static native void nClipRect(long nativeInstance, float left, float top, float right,
                                         float bottom, int clipOp, boolean doAA);
    private static native void nClipRRect(long nativeInstance, float left, float top, float right,
                                          float bottom, float xRad, float yRad,
                                          int clipOp, boolean doAA);
    private static native void nClipShader(long nativeInstance, long nativeShader, int clipOp);


    private static native void nDrawColor(long nativeInstance, float r, float g, float b, float a);

    private static native void nDrawRect(long nativeInstance,
                                         float left, float top, float right, float bottom,
                                         long nativePaint);
    private static native void nDrawImage(long nativeInstance, long nativeImage, float x, float y,
                                          int samplingDesc,
                                          float samplingCoeffB, float samplingCoeffC);
    private static native void nDrawPath(long nativeInstance, long nativePath, long nativePaint);
    private static native void nDrawGlyphs(long nativeInstance, char[] glyphs, float[] positions,
                                           float originX, float originY, long nativeFont, long nativePaint);
}
