/*
 * Copyright 2018 Google LLC All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.skar;

import android.graphics.Matrix;

/**
 * Provides static methods for matrix manipulation needed to draw in ARCore with Canvas.
 * Input matrices are assumed to be 4x4 android.opengl.Matrix types (16-float arrays in column-major
 * order).
 * Output matrices are 3x3 android.graphics.Matrix types.
 */

public class CanvasMatrixUtil {

    /******************* PUBLIC FUNCTIONS ***********************/

    /**
     * Returns an android.graphics.Matrix that can be used on a Canvas to draw a 2D object in
     * perspective. Object will be rotated towards the XZ plane and will appear to stick on Planes.
     * Undefined behavior when any of the matrices are not of size 16, or are null.
     *
     * @param model          4x4 model matrix of the object to be drawn (global/world)
     * @param view           4x4 camera view matrix (brings objects to camera origin and
     *                       orientation)
     * @param projection     4x4 projection matrix
     * @param viewPortWidth  width of viewport of GLSurfaceView
     * @param viewPortHeight height of viewport of GLSurfaceView
     * @return 3x3 matrix that puts a 2D objects in perspective on a Canvas
     */

    public static Matrix createPerspectiveMatrix(float[] model, float[] view, float[] projection,
                                                 float viewPortWidth, float viewPortHeight) {
        float[] viewPort = createViewportMatrix(viewPortWidth, viewPortHeight);
        float[] planeStickRotation = createXYtoXZRotationMatrix();
        float[][] matrices = {planeStickRotation, model, view, projection, viewPort};
        return createMatrixFrom4x4Array(matrices);
    }

    /**
     * Returns a 16-float matrix in column-major order that represents a viewport matrix given
     * the width and height of the viewport.
     *
     * @param width  width of viewport
     * @param height height of viewport
     */

    public static float[] createViewportMatrix(float width, float height) {
        float[] viewPort = new float[16];
        android.opengl.Matrix.setIdentityM(viewPort, 0);
        android.opengl.Matrix.translateM(viewPort, 0, width / 2, height / 2, 0);
        android.opengl.Matrix.scaleM(viewPort, 0, width / 2, -height / 2, 0);
        return viewPort;
    }

    /**
     * Returns a 16-float matrix in column-major order that is used to rotate objects from the
     * XY plane to the XZ plane. This is useful given that objects drawn on the Canvas are on the
     * XY plane.
     * In order to get objects to appear as if they are sticking on planes/ceilings/walls, we need
     * to rotate them from the XY plane to the XZ plane.
     */

    public static float[] createXYtoXZRotationMatrix() {
        float[] rotation = new float[16];
        android.opengl.Matrix.setIdentityM(rotation, 0);
        android.opengl.Matrix.rotateM(rotation, 0, 90, 1, 0, 0);
        return rotation;
    }


    /**
     * Returns an android.graphics.Matrix resulting from a 16-float matrix array in column-major
     * order.
     * Undefined behavior when the array is not of size 16 or is null.
     *
     * @param m4 16-float matrix in column-major order
     */

    public static Matrix createMatrixFrom4x4(float[] m4) {
        float[] m3 = matrix4x4ToMatrix3x3(m4);
        return createMatrixFrom3x3(m3);
    }

    /**
     * Returns an android.graphics.Matrix resulting from the concatenation of 16-float matrices
     * in column-major order from left to right.
     * e.g: m4Array = {m1, m2, m3} --> returns m = m3 * m2 * m1
     * Undefined behavior when the array is empty, null, or contains arrays not of size 9 (or null)
     *
     * @param m4Array array of 16-float matrices in column-major order
     */

    public static Matrix createMatrixFrom4x4Array(float[][] m4Array) {
        float[] result = multiplyMatrices4x4(m4Array);
        return createMatrixFrom4x4(result);
    }

    /**
     * Returns 4-float array resulting from the multiplication of a Vector of 4 floats
     * with a 4x4 float Matrix. The return is essentially m4 * v4, with perspective-divide applied
     * if perspectiveDivide is true
     * @param m4                16-float matrix in column-major order
     * @param v4                4-float vector
     * @param perspectiveDivide if true, divide return value by the w-coordinate
     * @return                  4-float array resulting from the multiplication
     */

    public static float[] multiplyMatrixVector(float[] m4, float[] v4, boolean perspectiveDivide) {
        float[] result = new float[4];
        android.opengl.Matrix.multiplyMV(result, 0, m4, 0, v4, 0);
        if (perspectiveDivide) {
            return new float[] {result[0] / result[3], result[1] / result[3],
                                result[2] / result[3], 1};
        }

        return new float[] {result[0], result[1], result[2], result[3]};
    }

    /**
     * Returns a 16-float matrix in column-major order resulting from the multiplication of matrices
     * e.g: m4Array = {m1, m2, m3} --> returns m = m3 * m2 * m1
     * Undefined behavior when the array is empty, null, or contains arrays not of size 9 (or null)
     *
     * @param m4Array array of 16-float matrices in column-major order
     */

    public static float[] multiplyMatrices4x4(float[][] m4Array) {
        float[] result = new float[16];
        android.opengl.Matrix.setIdentityM(result, 0);
        float[] rhs = result;
        for (int i = 0; i < m4Array.length; i++) {
            float[] lhs = m4Array[i];
            android.opengl.Matrix.multiplyMM(result, 0, lhs, 0, rhs, 0);
            rhs = result;
        }
        return result;
    }

    /******************* PRIVATE FUNCTIONS ***********************/

    /**
     * Returns an android.graphics.Matrix resulting from a 9-float matrix array in row-major order.
     * Undefined behavior when the array is not of size 9 or is null.
     *
     * @param m3 9-float matrix array in row-major order
     */

    private static Matrix createMatrixFrom3x3(float[] m3) {
        Matrix m = new Matrix();
        m.setValues(m3);
        return m;
    }

    /**
     * Returns a 9-float matrix in row-major order given a 16-float matrix in column-major order.
     * This will drop the Z column and row.
     * Undefined behavior when the array is not of size 9 or is null.
     *
     * @param m4 16-float matrix in column-major order
     */

    private static float[] matrix4x4ToMatrix3x3(float[] m4) {
        float[] m3 = new float[9];

        int j = 0;
        for (int i = 0; i < 7; i = i + 3) {
            if (j == 2) {
                j++; //skip row #3
            }
            m3[i] = m4[j];
            m3[i + 1] = m4[j + 4];
            m3[i + 2] = m4[j + 12];
            j++;
        }
        return m3;
    }
}
