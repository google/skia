package com.google.skar;

import android.graphics.Path;
import android.graphics.PointF;

import java.util.ArrayList;

public class SkARFingerPainting {
    // Points obtained by touching the screen. The first point is always brough to (0,0).
    // All subsequent points are translated by the same amount.
    public ArrayList<PointF> points = new ArrayList<>();

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
        points.add(p);
        previousPoint = p;
    }

    // Used to build a path before rendering it
    public void buildPath() {
        if (points.size() < 1) {
            return;
        }

        path = new Path();
        if (isSmooth) {
            // If less than 3 points, than draw a line between the two points
            if (points.size() <= 2 && points.size() > 0) {
                path.moveTo(points.get(0).x, points.get(0).y);
                path.lineTo(points.get(1).x, points.get(1).y);
            } else if (points.size() >= 3){
                // Else, essentially run deCasteljau
                path.moveTo(points.get(0).x, points.get(0).y);
                PointF mid = new PointF((points.get(0).x + points.get(1).x) / 2,
                                        (points.get(0).y + points.get(1).y) / 2);
                path.lineTo(mid.x, mid.y);

                for (int i = 1; i < points.size() - 1; i++) {
                    PointF p1 = points.get(i);
                    PointF p2 = points.get(i + 1);
                    PointF midP = new PointF((p1.x + p2.x) / 2,(p1.y + p2.y) / 2);
                    path.quadTo(p1.x, p1.y, midP.x, midP.y);
                }

                path.lineTo(points.get(points.size() - 1).x, points.get(points.size() - 1).y);
            }
        } else {
            path.moveTo(points.get(0).x, points.get(0).y);
            for (int i = 1; i < points.size(); i++) {
                path.lineTo(points.get(i).x, points.get(i).y);
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
