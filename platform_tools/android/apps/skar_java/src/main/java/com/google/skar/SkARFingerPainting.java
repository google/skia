package com.google.skar;

import android.graphics.Matrix;
import android.graphics.Path;
import android.graphics.PointF;

public class SkARFingerPainting { ;
    public Path path = new Path();

    private int numberOfPoints = 0;

    // Holds the model matrix of the first point added to such that the path can be drawn at the
    // model location (i.e on the Plane)
    private float[] modelMatrix;

    // Holds the inverse model matrix of the first point that was added such that the path is drawn
    // first at (0, 0)
    private float[] inverseModelMatrix;

    public SkARFingerPainting() {}

    // Adds another point to the path in Local space (i.e apply InverseModelMatrix to points located
    // in Global space (e.g hit positions acquired through hit tests)
    public void addPoint(PointF p) {
        if (numberOfPoints == 0) {
            path.moveTo(p.x, p.y);
        } else {
            path.lineTo(p.x, p.y);
        }
        numberOfPoints++;
    }

    public boolean isEmpty() {
        return numberOfPoints == 0;
    }

    public float[] getModelMatrix() {
        return modelMatrix;
    }

    public float[] getRawInverseModelMatrix() {
        return inverseModelMatrix;
    }

    public Matrix getInverseModelMatrix() {
        return SkARMatrix.createMatrixFrom4x4(inverseModelMatrix);
    }

    public void setModelMatrix(float[] m) {
        modelMatrix = m;
    }

    public void setInverseModelMatrix(float[] m) {
        inverseModelMatrix = m;
    }
}
