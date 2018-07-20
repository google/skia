package com.google.skar;

import android.graphics.Path;
import android.graphics.PointF;

import java.util.ArrayList;

public class SkARFingerPainting {
    // Points obtained by touching the screen. The first point is always brough to (0,0).
    // All subsequent points are translated by the same amount.
    public ArrayList<Float> points = new ArrayList<>();

    public Path path = new Path();

    // Previous point added to the path. This points belongs to the path in local space.
    public PointF previousPoint;

    // Holds the model matrix of the first point added to such that the path can be drawn at the
    // model location (i.e on the Plane)
    private float[] modelMatrix;

    private boolean isSmooth;

    public SkARFingerPainting(boolean smooth) {
        this.isSmooth = smooth;
    }

    public boolean getSmoothness() {
        return isSmooth;
    }

    public void setSmoothness(boolean smooth) {
        isSmooth = smooth;
    }

    // Adds another point to the path in Local space
    public void addPoint(PointF p) {
        points.add(p.x);
        points.add(p.y);
        previousPoint = p;
    }

    // Used to build a path before rendering it
    public void buildPath() {
        int nbPts = points.size() / 2;
        if (nbPts <= 1) {
            return;
        }

        path = new Path();
        if (isSmooth) {
            // If less than 3 points, than draw a line between the two points
            if (nbPts <= 2) {
                path.moveTo(points.get(0), points.get(1));
                path.lineTo(points.get(2), points.get(3));
            } else {
                // Else, essentially run deCasteljau
                path.moveTo(points.get(0), points.get(1));
                PointF mid = new PointF((points.get(0) + points.get(2)) / 2,
                                        (points.get(1) + points.get(3)) / 2);
                path.lineTo(mid.x, mid.y);

                for (int i = 1; i < nbPts -1; i++) {
                    float x1 = points.get(i * 2);
                    float x2 = points.get(i * 2 + 2);

                    float y1 = points.get(i * 2 + 1);
                    float y2 = points.get(i * 2 + 3);

                    float midX = (x1 + x2) / 2.0f;
                    float midY = (y1 + y2) / 2.0f;
                    path.quadTo(x1, y1, midX, midY);
                }

                path.lineTo(points.get(points.size() - 2), points.get(points.size() - 1));
            }
        } else {
            path.moveTo(points.get(0), points.get(1));
            for (int i = 1; i < nbPts; i++) {
                path.lineTo(points.get(i * 2), points.get(i * 2 + 1));
            }
        }
    }

    public boolean isEmpty() {
        return path.isEmpty();
    }

    public float[] getModelMatrix() {
        return modelMatrix;
    }

    public void setModelMatrix(float[] m) {
        modelMatrix = m;
    }

    public void reset() {
        points = new ArrayList<>();
        path = new Path();
    }
}
