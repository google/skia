package com.google.skar;

import android.graphics.Color;
import android.graphics.Path;
import android.graphics.PointF;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import java.util.ArrayList;

public class SkARFingerPainting {
    // Points obtained by touching the screen. The first point is always brough to (0,0).
    // All subsequent points are translated by the same amount.
    private ArrayList<PointF> points = new ArrayList<>();
    private ArrayList<Integer> jumpPoints = new ArrayList<>();
    private Map<Integer, Integer> indexColors = new HashMap<>();
    private Map<Path, Integer> pathColors = new HashMap<>();
    private ArrayList<Path> paths = new ArrayList<>();
    private int color = Color.RED;

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
    public void addPoint(PointF p, boolean jumpPoint) {
        points.add(p);
        if (jumpPoint) {
            Log.i("Jumped!", Integer.toString(points.size() - 1));
            jumpPoints.add(points.size() - 1);
            indexColors.put(points.size() - 1, color);
        }
        previousPoint = p;
    }

    // Used to build a path before rendering it
    public void buildPath() {
        paths = new ArrayList<>();
        if (points.size() <= 1) {
            return;
        }


        if (isSmooth) {
            int start = 0;
            for (int j = 1; j < jumpPoints.size(); j++) {

                int finish = jumpPoints.get(j);
                buildSmoothFromTo(start, finish);
                start = finish;
            }

            buildSmoothFromTo(start, points.size());
        } else {

            int start = 0;
            for (int j = 1; j < jumpPoints.size(); j++) {
                int finish = jumpPoints.get(j);
                buildRoughFromTo(start, finish);
                start = finish;
            }

            buildRoughFromTo(start, points.size());
        }
    }

    private void buildRoughFromTo(int start, int finish) {
        Path p = new Path();
        int c = indexColors.get(start);
        p.moveTo(points.get(start).x, points.get(start).y);
        for (int i = start + 1; i < finish; i++) {
            p.lineTo(points.get(i).x, points.get(i).y);
        }
        paths.add(p);
        pathColors.put(p, c);
    }

    private void buildSmoothFromTo(int start, int finish) {
        Path p = new Path();
        int c = indexColors.get(start);
        int nbPts = finish - start;
        // If less than 3 points, than draw a line between the two points
        if (nbPts <= 2 && nbPts > 1) {
            p.moveTo(points.get(start).x, points.get(start).y);
            p.lineTo(points.get(start + 1).x, points.get(start + 1).y);
        } else if (nbPts >= 3){
            // Else, essentially run deCasteljau
            p.moveTo(points.get(start).x, points.get(start).y);
            PointF mid = new PointF((points.get(start).x + points.get(start + 1).x) / 2,
                                    (points.get(start).y + points.get(start + 1).y) / 2);
            p.lineTo(mid.x, mid.y);

            for (int i = start + 1; i < finish - 1; i++) {
                PointF p1 = points.get(i);
                PointF p2 = points.get(i + 1);
                p.quadTo(p1.x, p1.y, (p1.x + p2.x) / 2, (p1.y + p2.y) / 2);
            }

            p.lineTo(points.get(finish - 1).x, points.get(finish - 1).y);
        }
        paths.add(p);
        pathColors.put(p, c);
    }

    public boolean isEmpty() {
        return points.isEmpty();
    }

    public float[] getModelMatrix() {
        return modelMatrix;
    }

    public void setModelMatrix(float[] m) {
        modelMatrix = m;
    }

    public void setColor(int color) {
        this.color = color;
    }

    public int getPathColor(Path p) {
        return pathColors.get(p);
    }

    public ArrayList<Path> getPaths() {
        return paths;
    }

    public void reset() {
        points.clear();
        jumpPoints.clear();
        paths.clear();
        pathColors.clear();
        indexColors.clear();
    }
}
