package com.google.skar;

import android.graphics.Matrix;

/**
 * Provides static methods for matrix manipulation. Input matrices are assumed to be 4x4
 * android.opengl.Matrix types. Output matrices are 3x3 android.graphics.Matrix types.
 * The main use of this class is to be able to get a Matrix for a Canvas that applies perspective
 * to 2D objects
 */

public class SkARMatrix {
    /**
     * Returns an android.graphics.Matrix that can be used on a Canvas to draw a 2D object in
     * perspective. Objects will be rotated towards the XZ plane. Undefined behavior when any of
     * the matrices are not of size 16, or are null.
     *
     * @param model      4x4 model matrix of the object to be drawn (global/world)
     * @param view       4x4 camera view matrix (brings objects to camera origin and orientation)
     * @param projection 4x4 projection matrix
     * @param viewport   4x4 viewport matrix
     * @return 3x3 matrix that puts a 2D objects in perspective on a Canvas
     */

    public static Matrix createPerspectiveMatrix(float[] model, float[] view, float[] projection,
                                                 float[] viewport) {
        float[] skiaRotation = createXYtoXZRotationMatrix();
        float[][] matrices = {skiaRotation, model, view, projection, viewport};
        return createMatrixFrom4x4Array(matrices);
    }

    /**
     * Returns an android.graphics.Matrix that can be used on a Canvas to draw a 2D object in
     * perspective. Undefined behavior when any of the matrices are not of size 16, or are null.
     *
     * @param model       4x4 model matrix of the object to be drawn (global/world)
     * @param view        4x4 camera view matrix (brings objects to camera origin and orientation)
     * @param projection  4x4 projection matrix
     * @param viewport    4x4 viewport matrix
     * @param rotatePlane specifies if object should be from the XY plane to the XZ plane
     * @return 3x3 matrix that puts a 2D objects in perspective on a Canvas
     */

    public static Matrix createPerspectiveMatrix(float[] model, float[] view, float[] projection,
                                                 float[] viewport, boolean rotatePlane) {
        if (rotatePlane) {
            return createPerspectiveMatrix(model, view, projection, viewport);
        } else {
            float[][] matrices = {model, view, projection, viewport};
            return createMatrixFrom4x4Array(matrices);
        }
    }

    /**
     * Returns an android.graphics.Matrix that can be used on a Canvas to draw a 2D object in
     * perspective. Undefined behavior when any of the matrices are not of size 16, or are null.
     *
     * @param model          4x4 model matrix of the object to be drawn (global/world)
     * @param view           4x4 camera view matrix (brings objects to camera origin and orientation)
     * @param projection     4x4 projection matrix
     * @param viewPortWidth  width of viewport of GLSurfaceView
     * @param viewPortHeight height of viewport of GLSurfaceView
     * @param rotatePlane    specifies if object should be from the XY plane to the XZ plane
     * @return 3x3 matrix that puts a 2D objects in perspective on a Canvas
     */

    public static Matrix createPerspectiveMatrix(float[] model, float[] view, float[] projection,
                                                 float viewPortWidth, float viewPortHeight, boolean rotatePlane) {
        if (rotatePlane) {
            return createPerspectiveMatrix(model, view, projection, viewPortWidth, viewPortHeight);
        } else {
            float[] viewPort = createViewportMatrix(viewPortWidth, viewPortHeight);
            float[][] matrices = {model, view, projection, viewPort};
            return createMatrixFrom4x4Array(matrices);
        }
    }

    /**
     * Returns an android.graphics.Matrix that can be used on a Canvas to draw a 2D object in
     * perspective. Object will be rotated towards the XZ plane. Undefined behavior when any of
     * the matrices are not of size 16, or are null.
     *
     * @param model          4x4 model matrix of the object to be drawn (global/world)
     * @param view           4x4 camera view matrix (brings objects to camera origin and orientation)
     * @param projection     4x4 projection matrix
     * @param viewPortWidth  width of viewport of GLSurfaceView
     * @param viewPortHeight height of viewport of GLSurfaceView
     * @return 3x3 matrix that puts a 2D objects in perspective on a Canvas
     */

    public static Matrix createPerspectiveMatrix(float[] model, float[] view, float[] projection,
                                                 float viewPortWidth, float viewPortHeight) {
        float[] viewPort = createViewportMatrix(viewPortWidth, viewPortHeight);
        float[] skiaRotation = createXYtoXZRotationMatrix();
        float[][] matrices = {skiaRotation, model, view, projection, viewPort};
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
     * Returns a 16-float matrix in column-major order that is used to rotate objects from the XY plane
     * to the XZ plane. This is useful given that objects drawn on the Canvas are on the XY plane.
     * In order to get objects to appear as if they are sticking on planes/ceilings/walls, we need
     * to rotate them from the XY plane to the XZ plane.
     */

    private static float[] createXYtoXZRotationMatrix() {
        float[] rotation = new float[16];
        android.opengl.Matrix.setIdentityM(rotation, 0);
        android.opengl.Matrix.rotateM(rotation, 0, 90, 1, 0, 0);
        return rotation;
    }

    /**
     * Returns an android.graphics.Matrix resulting from a 9-float matrix array in row-major order.
     * Undefined behavior when the array is not of size 9 or is null.
     *
     * @param m3 9-float matrix array in row-major order
     */

    public static Matrix createMatrixFrom3x3(float[] m3) {
        Matrix m = new Matrix();
        m.setValues(m3);
        return m;
    }

    /**
     * Returns an android.graphics.Matrix resulting from a 16-float matrix array in column-major order
     * Undefined behavior when the array is not of size 16 or is null.
     *
     * @param m4
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

    /**
     * Returns a 16-float matrix in column-major order resulting from the multiplication of matrices.
     * e.g: m4Array = {m1, m2, m3} --> returns m = m3 * m2 * m1
     * Undefined behavior when the array is empty, null, or contains arrays not of size 9 (or null)
     *
     * @param m4Array array of 16-float matrices in column-major order
     */

    private static float[] multiplyMatrices4x4(float[][] m4Array) {
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
}
