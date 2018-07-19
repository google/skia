package com.google.skar;

import android.graphics.Path;
import android.graphics.PointF;

public class SkARFingerPainting {
    public Path path = new Path();

    // Previous point added to the path. This points belongs to the path in local space.
    public PointF previousPoint;

    private int numberOfPoints = 0;

    // Holds the model matrix of the first point added to such that the path can be drawn at the
    // model location (i.e on the Plane)
    private float[] modelMatrix;

    public SkARFingerPainting() {}

    // Adds another point to the path in Local space
    public void addPoint(PointF p) {
        if (numberOfPoints == 0) {
            path.moveTo(p.x, p.y);
        } else {
            path.lineTo(p.x, p.y);
        }
        previousPoint = p;
        numberOfPoints++;
    }

    public boolean isEmpty() {
        return numberOfPoints == 0;
    }

    public float[] getModelMatrix() {
        return modelMatrix;
    }

    public void setModelMatrix(float[] m) {
        modelMatrix = m;
    }

    public void reset() {
        path = new Path();
        numberOfPoints = 0;
    }
}
