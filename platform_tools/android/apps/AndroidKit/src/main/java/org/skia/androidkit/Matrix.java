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
    public Matrix(float m0, float m4, float m8,  float m12,
                  float m1, float m5, float m9,  float m13,
                  float m2, float m6, float m10, float m14,
                  float m3, float m7, float m11, float m15) {
        mNativeInstance = nCreate(m0, m4, m8,  m12,
                                  m1, m5, m9,  m13,
                                  m2, m6, m10, m14,
                                  m3, m7, m11, m15);
    }

    private Matrix(long nativeInstance) {
        mNativeInstance = nativeInstance;
    }

    /*
     * A: this Matrix
     * B: Matrix passed in
     * Concat A * B, return new Matrix C as result
     */
    public static Matrix Concat(Matrix a, Matrix b) {
        long nativeA = a.mNativeInstance;
        long nativeB = b.mNativeInstance;
        long nativeC = nConcat(nativeA, nativeB);
        return new Matrix(nativeC);
    }

    /*
     * A: this Matrix
     * B: Matrix passed in
     * Concat B * A, store result in Matrix A
     */
    public void preConcat(Matrix b) {
        long nativeA = this.mNativeInstance;
        long nativeB = b.mNativeInstance;
        nPreConcat(nativeA, nativeB);
    }

    /*
     * Translates this Matrix by x, y, z
     * Store result in caller Matrix
     * returns reference to this Matrix for operation chaining
     *
     * TODO: optimize calls to JNI
     */
    public Matrix translate(float x, float y, float z) {
        Matrix t = new Matrix(1, 0, 0, x,
                              0, 1, 0, y,
                              0, 0, 1, z,
                              0, 0, 0, 1);
        this.preConcat(t);
        return this;
    }

    /*
     * Scales this Matrix by x, y, z
     * Store result in caller Matrix
     * returns reference to this Matrix for operation chaining
     */
    public Matrix scale(float x, float y, float z) {
        Matrix s = new Matrix(x, 0, 0, 0,
                              0, y, 0, 0,
                              0, 0, z, 0,
                              0, 0, 0, 1);
        this.preConcat(s);
        return this;
    }

    /*
     * Rotates this Matrix along the x-axis by rad radians
     * Store result in caller Matrix
     * returns reference to this Matrix for operation chaining
     */
    public Matrix rotateX(float rad) {
        Matrix r = new Matrix(1, 0,                     0,                      0,
                              0, (float) Math.cos(rad), (float) -Math.sin(rad), 0,
                              0, (float) Math.sin(rad), (float) Math.cos(rad),  0,
                              0, 0,                     0,                      1);
        this.preConcat(r);
        return this;
    }

    /*
     * Rotates this Matrix along the y-axis by rad radians
     * Store result in caller Matrix
     * returns reference to this Matrix for operation chaining
     */
    public Matrix rotateY(float rad) {
        Matrix r = new Matrix((float) Math.cos(rad),  0, (float) Math.sin(rad), 0,
                              0,                      1, 0,                     0,
                              (float) -Math.sin(rad), 0, (float) Math.cos(rad), 0,
                              0,                      0, 0,                     1);
        this.preConcat(r);
        return this;
    }

    /*
     * Rotates this Matrix along the z-axis by rad radians
     * Store result in caller Matrix
     * returns reference to this Matrix for operation chaining
     */
    public Matrix rotateZ(float rad) {
        Matrix r = new Matrix((float) Math.cos(rad), (float) -Math.sin(rad), 0, 0,
                (float) Math.sin(rad), (float) Math.cos(rad),  0, 0,
                0,                     0,                      1, 0,
                0,                     0,                      0, 1);
        this.preConcat(r);
        return this;
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

    // package private
    long getNativeInstance() { return mNativeInstance; }

    private static native long nCreate(float m0, float m4, float m8,  float m12,
                                       float m1, float m5, float m9,  float m13,
                                       float m2, float m6, float m10, float m14,
                                       float m3, float m7, float m11, float m15);
    private static native void nRelease(long nativeInstance);

    private static native void nPreConcat(long mNativeInstanceA, long mNativeInstanceB);
    private static native long nConcat(long mNativeInstanceA, long mNativeInstanceB);
}
