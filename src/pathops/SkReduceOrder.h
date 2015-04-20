/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkReduceOrder_DEFINED
#define SkReduceOrder_DEFINED

#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsQuad.h"

union SkReduceOrder {
    enum Quadratics {
        kNo_Quadratics,
        kAllow_Quadratics
    };

    int reduce(const SkDCubic& cubic, Quadratics);
    int reduce(const SkDLine& line);
    int reduce(const SkDQuad& quad);

    static SkPath::Verb Conic(const SkPoint pts[3], SkScalar weight, SkPoint* reducePts);
    static SkPath::Verb Cubic(const SkPoint pts[4], SkPoint* reducePts);
    static SkPath::Verb Quad(const SkPoint pts[3], SkPoint* reducePts);

    SkDLine fLine;
    SkDQuad fQuad;
    SkDCubic fCubic;
};

#endif
