/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPath.h"
#include "SkShadowTessellator.h"
#include "SkShadowUtils.h"
#include "SkVertices.h"
#include "Test.h"

void tessellate_shadow(skiatest::Reporter* reporter, const SkPath& path, const SkMatrix& ctm,
                       bool expectSuccess) {

    auto heightParams = SkPoint3::Make(0, 0, 4);

    auto verts = SkShadowTessellator::MakeAmbient(path, ctm, heightParams, true);
    if (expectSuccess != SkToBool(verts)) {
        ERRORF(reporter, "Expected shadow tessellation to %s but it did not.",
               expectSuccess ? "succeed" : "fail");
    }
    verts = SkShadowTessellator::MakeAmbient(path, ctm, heightParams, false);
    if (expectSuccess != SkToBool(verts)) {
        ERRORF(reporter, "Expected shadow tessellation to %s but it did not.",
               expectSuccess ? "succeed" : "fail");
    }
    verts = SkShadowTessellator::MakeSpot(path, ctm, heightParams, {0, 0, 128}, 128.f, false);
    if (expectSuccess != SkToBool(verts)) {
        ERRORF(reporter, "Expected shadow tessellation to %s but it did not.",
               expectSuccess ? "succeed" : "fail");
    }
    verts = SkShadowTessellator::MakeSpot(path, ctm, heightParams, {0, 0, 128}, 128.f, false);
    if (expectSuccess != SkToBool(verts)) {
        ERRORF(reporter, "Expected shadow tessellation to %s but it did not.",
               expectSuccess ? "succeed" : "fail");
    }
}

DEF_TEST(ShadowUtils, reporter) {
    SkCanvas canvas(100, 100);

    SkPath path;
    path.cubicTo(100, 50, 20, 100, 0, 0);
    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), true);

    // This line segment has no area and no shadow.
    path.reset();
    path.lineTo(10.f, 10.f);
    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), false);

    // A series of colinear line segments
    path.reset();
    for (int i = 0; i < 10; ++i) {
        path.lineTo((SkScalar)i, (SkScalar)i);
    }
    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), false);
}
