/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsQuad.h"
#include "SkPathOpsRect.h"

void SkDRect::setBounds(const SkDLine& line) {
    set(line[0]);
    add(line[1]);
}

void SkDRect::setBounds(const SkDQuad& quad) {
    set(quad[0]);
    add(quad[2]);
    double tValues[2];
    int roots = 0;
    if (!between(quad[0].fX, quad[1].fX, quad[2].fX)) {
        roots = SkDQuad::FindExtrema(quad[0].fX, quad[1].fX, quad[2].fX, tValues);
    }
    if (!between(quad[0].fY, quad[1].fY, quad[2].fY)) {
        roots += SkDQuad::FindExtrema(quad[0].fY, quad[1].fY, quad[2].fY, &tValues[roots]);
    }
    for (int x = 0; x < roots; ++x) {
        add(quad.ptAtT(tValues[x]));
    }
}

void SkDRect::setRawBounds(const SkDQuad& quad) {
    set(quad[0]);
    for (int x = 1; x < 3; ++x) {
        add(quad[x]);
    }
}

static bool is_bounded_by_end_points(double a, double b, double c, double d) {
    return between(a, b, d) && between(a, c, d);
}

void SkDRect::setBounds(const SkDCubic& c) {
    set(c[0]);
    add(c[3]);
    double tValues[4];
    int roots = 0;
    if (!is_bounded_by_end_points(c[0].fX, c[1].fX, c[2].fX, c[3].fX)) {
        roots = SkDCubic::FindExtrema(c[0].fX, c[1].fX, c[2].fX, c[3].fX, tValues);
    }
    if (!is_bounded_by_end_points(c[0].fY, c[1].fY, c[2].fY, c[3].fY)) {
        roots += SkDCubic::FindExtrema(c[0].fY, c[1].fY, c[2].fY, c[3].fY, &tValues[roots]);
    }
    for (int x = 0; x < roots; ++x) {
        add(c.ptAtT(tValues[x]));
    }
}

void SkDRect::setRawBounds(const SkDCubic& cubic) {
    set(cubic[0]);
    for (int x = 1; x < 4; ++x) {
        add(cubic[x]);
    }
}
