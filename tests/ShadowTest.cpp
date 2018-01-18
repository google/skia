/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkDrawShadowInfo.h"
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

void check_xformed_bounds(skiatest::Reporter* reporter, const SkPath& path, const SkMatrix& ctm) {
    const SkDrawShadowRec rec = {
        SkPoint3::Make(0, 0, 4),
        SkPoint3::Make(100, 0, 600),
        800.f,
        0x08000000,
        0x40000000,
        0
    };
    SkRect bounds;
    SkDrawShadowMetrics::GetLocalBounds(path, rec, ctm, &bounds);
    ctm.mapRect(&bounds);

    auto verts = SkShadowTessellator::MakeAmbient(path, ctm, rec.fZPlaneParams, true);
    if (verts) {
        REPORTER_ASSERT(reporter, bounds.contains(verts->bounds()));
    }

    SkPoint mapXY = ctm.mapXY(rec.fLightPos.fX, rec.fLightPos.fY);
    SkPoint3 devLightPos = SkPoint3::Make(mapXY.fX, mapXY.fY, rec.fLightPos.fZ);
    verts = SkShadowTessellator::MakeSpot(path, ctm, rec.fZPlaneParams, devLightPos,
                                          rec.fLightRadius, false);
    if (verts) {
        REPORTER_ASSERT(reporter, bounds.contains(verts->bounds()));
    }
}

void check_bounds(skiatest::Reporter* reporter, const SkPath& path) {
    SkMatrix ctm;
    ctm.setTranslate(100, 100);
    check_xformed_bounds(reporter, path, ctm);
    ctm.postScale(2, 2);
    check_xformed_bounds(reporter, path, ctm);
    ctm.preRotate(45);
    check_xformed_bounds(reporter, path, ctm);
    ctm.preSkew(40, -20);
    check_xformed_bounds(reporter, path, ctm);
    ctm[SkMatrix::kMPersp0] = 0.0001f;
    ctm[SkMatrix::kMPersp1] = 12.f;
    check_xformed_bounds(reporter, path, ctm);
}

DEF_TEST(ShadowBounds, reporter) {
    SkPath path;
    path.addRRect(SkRRect::MakeRectXY(SkRect::MakeLTRB(-50, -20, 40, 30), 4, 4));
    check_bounds(reporter, path);

    path.reset();
    path.addOval(SkRect::MakeLTRB(300, 300, 900, 900));
    check_bounds(reporter, path);

    path.reset();
    path.cubicTo(100, 50, 20, 100, 0, 0);
    check_bounds(reporter, path);
}
