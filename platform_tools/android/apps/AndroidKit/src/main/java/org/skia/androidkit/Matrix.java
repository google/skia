/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

/*
 * 4x4 matrix backed by SkM44
 */
public class Matrix {
    private long mNativeInstance;

    /*
     * Returns identity Matrix
     */
    public Matrix() {
        mNativeInstance = nCreate(1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1);
    }

    /*
     * Returns Matrix populated with values passed in (row-major order).
     */
    public Matrix(float m11, float m12, float m13, float m14,
                  float m21, float m22, float m23, float m24,
                  float m31, float m32, float m33, float m34,
                  float m41, float m42, float m43, float m44) {
        mNativeInstance = nCreate(m11, m12, m13, m14,
                                  m21, m22, m23, m24,
                                  m31, m32, m33, m34,
                                  m41, m42, m43, m44);
    }

    /*
     * Add Matrix A (this) and B (parameter) together, store result in Matrix A
     */
    public void add(Matrix b) {
        long nativeA = this.mNativeInstance;
        long nativeB = b.mNativeInstance;
        nAdd(nativeA, nativeB);
    }

    /*
     * Multiply Matrix A (this) and B (parameter) together, store result in Matrix A
     */
    public void multiply(Matrix m) {
        long nativeA = this.mNativeInstance;
        long nativeB = m.mNativeInstance;
        nMultiply(nativeA, nativeB);
    }

    /**
     * Releases any resources associated with this Matrix.
     */
    public void release() {
        nRelease(mNativeInstance);
        mNativeInstance = 0;
    }

    @Override
    protected void finalize() throws Throwable {
        release();
    }

    private static native long nCreate(float m11, float m12, float m13, float m14,
                                       float m21, float m22, float m23, float m24,
                                       float m31, float m32, float m33, float m34,
                                       float m41, float m42, float m43, float m44);
    private static native void nRelease(long nativeInstance);

    private static native void nAdd(long mNativeInstanceA, long mNativeInstanceB);
    private static native void nMultiply(long mNativeInstanceA, long mNativeInstanceB);
}
