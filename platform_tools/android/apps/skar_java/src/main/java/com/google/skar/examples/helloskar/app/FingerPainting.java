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

package com.google.skar.examples.helloskar.app;

import android.graphics.Color;
import android.graphics.Path;
import android.graphics.PointF;

import com.google.skar.examples.helloskar.helpers.GestureHelper;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class FingerPainting {
    public static class BuiltPath {
        public Path path;
        public int color;

        BuiltPath(Path p, int c) {
            this.path = p;
            this.color = c;
        }
    }

    // Points obtained by touching the screen. The first point is always brought to (0,0).
    // All subsequent points are translated by the same amount.
    private ArrayList<PointF> points = new ArrayList<>();

    // Indices in points array that indicate the start of a new path. E.g: if 5 is added to
    // jumpPoints, then index 5 of the points array is the start of a new path (use moveTo())
    private ArrayList<Integer> jumpPoints = new ArrayList<>();

    // Map from index (start of path) to color of path
    private Map<Integer, Integer> indexColors = new HashMap<>();

    // List of built paths (reset each frame)
    private ArrayList<BuiltPath> paths = new ArrayList<>();

    // Previous point added to the path. This points belongs to the path in local space.
    private float[] previousLocalPoint = new float[2];

    // Previous point added to the path. This points belongs to the path in global space (i.e Pose)
    private float[] previousGlobalPoint = new float[2];

    // Holds the model matrix of the first point added to such that the path can be drawn at the
    // model location (i.e on the Plane)
    private float[] modelMatrix;

    // Currently selected color in the UI
    private int color = Color.RED;

    // True if path should be drawn using buildSmoothFromTo()
    private boolean isSmooth;

    public FingerPainting(boolean smooth) {
        this.isSmooth = smooth;
    }

    public void setSmoothness(boolean smooth) {
        isSmooth = smooth;
    }

    /**
     * Given a hit location in Global space (e.g on a Plane), and the associated ScrollEvent,
     * construct the next point in the path in Local space. The first point of the Finger Painting
     * must be at (0,0)
     * @param hitLocation   (x, y) coordinates of the hit position in Global space
     * @param holdTap       ScrollEvent associated with the hit test that calls this function
     * @return              true if point was computed and added. False otherwise.
     */
    public boolean computeNextPoint(float[] hitLocation, float[] modelMat, GestureHelper.ScrollEvent holdTap) {
        if (isEmpty()) {
            // If finger painting is empty, then first point is origin. Model matrix
            // of the finger painting is the model matrix of the first point
            addPoint(new PointF(0, 0), true);

            // Get model matrix of first point
            setModelMatrix(modelMat);
        } else {
            // Else, construct next point given its distance from previous point
            float localDistanceScale = 1000;
            PointF distance = new PointF(hitLocation[0] - previousGlobalPoint[0],
                                         hitLocation[2] - previousGlobalPoint[1]);

            if (distance.length() < 0.01f) {
                // If distance between previous stored point and current point is too
                // small, skip it
                return false;
            }

            // New point is distance + old point
            PointF p = new PointF(distance.x * localDistanceScale + previousLocalPoint[0],
                                  distance.y * localDistanceScale + previousLocalPoint[1]);

            addPoint(p, holdTap.isStartOfScroll);
        }
        previousGlobalPoint[0] = hitLocation[0];
        previousGlobalPoint[1] = hitLocation[2];
        return true;
    }

    /**
     * Constructs the Paths to be drawn (populates the paths List). Call this before drawing this
     * Finger Painting (every frame).
     */
    public void buildPath() {
        if (points.size() <= 1) {
            // Don't build anything if the path only contains one point
            return;
        }

        paths = new ArrayList<>();

        if (isSmooth) {
            buildSmooth();
        } else {
            buildRough();
        }
    }

    /**
     * @return 16-float matrix that takes a point from Local space to Global space (onto the Plane)
     */
    public float[] getModelMatrix() {
        return modelMatrix;
    }

    /**
     * Change currently selected color. Preferably called through a UI element (a menu)
     * @param color   color to be selected this frame
     */
    public void setColor(int color) {
        this.color = color;
    }

    /**
     * @return List of built paths contained within this Finger Painting
     */
    public List<BuiltPath> getPaths() {
        return paths;
    }

    /**
     * Clears data contained within this Finger Painting
     */
    public void reset() {
        points.clear();
        jumpPoints.clear();
        paths.clear();
        indexColors.clear();
    }

    /********************** PRIVATE HELPERS **************************************/

    // Adds another point to the path in Local space
    private void addPoint(PointF p, boolean jumpPoint) {
        points.add(p);
        if (jumpPoint) {
            jumpPoints.add(points.size() - 1);
            indexColors.put(points.size() - 1, color);
        }
        previousLocalPoint[0] = p.x;
        previousLocalPoint[1] = p.y;
    }

    // Builds paths of this Finger Painting using the rough algorithm
    private void buildRough() {
        int start = 0; // starting index of each path. 1st path starts at index 0 points list
        for (int j = 1; j < jumpPoints.size(); j++) {
            int finish = jumpPoints.get(j); // finishing index of current path
            buildRoughFromTo(start, finish);
            start = finish;
        }

        buildRoughFromTo(start, points.size());
    }

    // Builds paths of this Finger Painting using the smooth algorithm
    private void buildSmooth() {
        int start = 0;
        for (int j = 1; j < jumpPoints.size(); j++) {
            int finish = jumpPoints.get(j);
            buildSmoothFromTo(start, finish);
            start = finish;
        }

        buildSmoothFromTo(start, points.size());
    }

    // Builds a rough path that starts at index (start) of the points List, and ends at (finish - 1)
    // of the points List
    private void buildRoughFromTo(int start, int finish) {
        Path p = new Path();
        int c = indexColors.get(start);
        p.moveTo(points.get(start).x, points.get(start).y);

        for (int i = start + 1; i < finish; i++) {
            p.lineTo(points.get(i).x, points.get(i).y);
        }

        BuiltPath bp = new BuiltPath(p, c);
        paths.add(bp);
    }

    // Builds a smooth path that starts at index (start) of the points List, and ends at (finish - 1)
    // of the points List
    private void buildSmoothFromTo(int start, int finish) {
        Path p = new Path();
        int c = indexColors.get(start);

        int nbPts = finish - start; // # of points within this path (not including the finish index)

        // If only 2 points in path, draw a line between them
        if (nbPts == 2) {
            p.moveTo(points.get(start).x, points.get(start).y);
            p.lineTo(points.get(start + 1).x, points.get(start + 1).y);
            BuiltPath bp = new BuiltPath(p, c);
            paths.add(bp);
        } else if (nbPts >= 3) {
            // Else (3 pts +), essentially run deCasteljau
            p.moveTo(points.get(start).x, points.get(start).y);
            p.lineTo((points.get(start).x + points.get(start + 1).x) / 2,
                     (points.get(start).y + points.get(start + 1).y) / 2);

            for (int i = start + 1; i < finish - 1; i++) {
                PointF p1 = points.get(i);
                PointF p2 = points.get(i + 1);
                p.quadTo(p1.x, p1.y, (p1.x + p2.x) / 2, (p1.y + p2.y) / 2);
            }

            p.lineTo(points.get(finish - 1).x, points.get(finish - 1).y);
            BuiltPath bp = new BuiltPath(p, c);
            paths.add(bp);
        }
    }

    private boolean isEmpty() { return points.isEmpty(); }

    private void setModelMatrix(float[] m) {
        modelMatrix = m;
    }
}
