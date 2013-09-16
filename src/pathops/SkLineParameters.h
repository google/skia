/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsQuad.h"

// Sources
// computer-aided design - volume 22 number 9 november 1990 pp 538 - 549
// online at http://cagd.cs.byu.edu/~tom/papers/bezclip.pdf

// This turns a line segment into a parameterized line, of the form
// ax + by + c = 0
// When a^2 + b^2 == 1, the line is normalized.
// The distance to the line for (x, y) is d(x,y) = ax + by + c
//
// Note that the distances below are not necessarily normalized. To get the true
// distance, it's necessary to either call normalize() after xxxEndPoints(), or
// divide the result of xxxDistance() by sqrt(normalSquared())

class SkLineParameters {
public:
    void cubicEndPoints(const SkDCubic& pts) {
        cubicEndPoints(pts, 0, 1);
        if (dx() == 0 && dy() == 0) {
            cubicEndPoints(pts, 0, 2);
            if (dx() == 0 && dy() == 0) {
                cubicEndPoints(pts, 0, 3);
            }
        }
    }

    void cubicEndPoints(const SkDCubic& pts, int s, int e) {
        a = pts[s].fY - pts[e].fY;
        b = pts[e].fX - pts[s].fX;
        c = pts[s].fX * pts[e].fY - pts[e].fX * pts[s].fY;
    }

    double cubicPart(const SkDCubic& part) {
        cubicEndPoints(part);
        if (part[0] == part[1] || ((const SkDLine& ) part[0]).nearRay(part[2])) {
            return pointDistance(part[3]);
        }
        return pointDistance(part[2]);
    }

    void lineEndPoints(const SkDLine& pts) {
        a = pts[0].fY - pts[1].fY;
        b = pts[1].fX - pts[0].fX;
        c = pts[0].fX * pts[1].fY - pts[1].fX * pts[0].fY;
    }

    void quadEndPoints(const SkDQuad& pts) {
        quadEndPoints(pts, 0, 1);
        if (dx() == 0 && dy() == 0) {
            quadEndPoints(pts, 0, 2);
        }
    }

    void quadEndPoints(const SkDQuad& pts, int s, int e) {
        a = pts[s].fY - pts[e].fY;
        b = pts[e].fX - pts[s].fX;
        c = pts[s].fX * pts[e].fY - pts[e].fX * pts[s].fY;
    }

    double quadPart(const SkDQuad& part) {
        quadEndPoints(part);
        return pointDistance(part[2]);
    }

    double normalSquared() const {
        return a * a + b * b;
    }

    bool normalize() {
        double normal = sqrt(normalSquared());
        if (approximately_zero(normal)) {
            a = b = c = 0;
            return false;
        }
        double reciprocal = 1 / normal;
        a *= reciprocal;
        b *= reciprocal;
        c *= reciprocal;
        return true;
    }

    void cubicDistanceY(const SkDCubic& pts, SkDCubic& distance) const {
        double oneThird = 1 / 3.0;
        for (int index = 0; index < 4; ++index) {
            distance[index].fX = index * oneThird;
            distance[index].fY = a * pts[index].fX + b * pts[index].fY + c;
        }
    }

    void quadDistanceY(const SkDQuad& pts, SkDQuad& distance) const {
        double oneHalf = 1 / 2.0;
        for (int index = 0; index < 3; ++index) {
            distance[index].fX = index * oneHalf;
            distance[index].fY = a * pts[index].fX + b * pts[index].fY + c;
        }
    }

    double controlPtDistance(const SkDCubic& pts, int index) const {
        SkASSERT(index == 1 || index == 2);
        return a * pts[index].fX + b * pts[index].fY + c;
    }

    double controlPtDistance(const SkDQuad& pts) const {
        return a * pts[1].fX + b * pts[1].fY + c;
    }

    double pointDistance(const SkDPoint& pt) const {
        return a * pt.fX + b * pt.fY + c;
    }

    double dx() const {
        return b;
    }

    double dy() const {
        return -a;
    }

private:
    double a;
    double b;
    double c;
};
