/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/core/SkVertices.h"
#include "include/private/base/SkTo.h"
#include "include/utils/SkShadowUtils.h"
#include "src/core/SkDrawShadowInfo.h"
#include "src/core/SkVerticesPriv.h"
#include "src/utils/SkShadowTessellator.h"
#include "tests/Test.h"

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

enum ExpectVerts {
    kDont_ExpectVerts,
    kDo_ExpectVerts
};

void check_result(skiatest::Reporter* reporter, sk_sp<SkVertices> verts, ExpectVerts expectVerts) {
    const bool expectSuccess = expectVerts == kDo_ExpectVerts;
    if (expectSuccess != SkToBool(verts)) {
        ERRORF(reporter, "Expected shadow tessellation to %s but it did not.",
               expectSuccess ? "succeed" : "fail");
    }
    if (SkToBool(verts)) {
        if (kDont_ExpectVerts == expectVerts && verts->priv().vertexCount()) {
            ERRORF(reporter, "Expected shadow tessellation to generate no vertices but it did.");
        } else if (kDo_ExpectVerts == expectVerts && !verts->priv().vertexCount()) {
            ERRORF(reporter, "Expected shadow tessellation to generate vertices but it didn't.");
        }
    }
}

void tessellate_shadow(skiatest::Reporter* reporter, const SkPath& path, const SkMatrix& ctm,
                       const SkPoint3& heightParams, ExpectVerts expectVerts) {

    auto verts = SkShadowTessellator::MakeAmbient(path, ctm, heightParams, true);
    check_result(reporter, verts, expectVerts);

    verts = SkShadowTessellator::MakeAmbient(path, ctm, heightParams, false);
    check_result(reporter, verts, expectVerts);

    verts = SkShadowTessellator::MakeSpot(path, ctm, heightParams, {0, 0, 128}, 128.f, true, false);
    check_result(reporter, verts, expectVerts);

    verts = SkShadowTessellator::MakeSpot(path, ctm, heightParams, {0, 0, 128}, 128.f, false,
                                          false);
    check_result(reporter, verts, expectVerts);

    verts = SkShadowTessellator::MakeSpot(path, ctm, heightParams, {0, 0, 128}, 128.f, true, true);
    check_result(reporter, verts, expectVerts);

    verts = SkShadowTessellator::MakeSpot(path, ctm, heightParams, {0, 0, 128}, 128.f, false, true);
    check_result(reporter, verts, expectVerts);
}

DEF_TEST(ShadowUtils, reporter) {
    SkCanvas canvas(100, 100);

    SkPath path;
    path.cubicTo(100, 50, 20, 100, 0, 0);
    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), {0, 0, 4}, kDo_ExpectVerts);
    // super high path
    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), {0, 0, 4.0e+37f}, kDo_ExpectVerts);

    // This line segment has no area and no shadow.
    path.reset();
    path.lineTo(10.f, 10.f);
    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), {0, 0, 4}, kDont_ExpectVerts);

    // A series of collinear line segments
    path.reset();
    for (int i = 0; i < 10; ++i) {
        path.lineTo((SkScalar)i, (SkScalar)i);
    }
    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), {0, 0, 4}, kDont_ExpectVerts);

    // ugly degenerate path
    path.reset();
    path.moveTo(-134217728, 2.22265153e+21f);
    path.cubicTo(-2.33326106e+21f, 7.36298265e-41f, 3.72237738e-22f, 5.99502692e-36f,
                 1.13631943e+22f, 2.0890786e+33f);
    path.cubicTo(1.03397626e-25f, 5.99502692e-36f, 9.18354962e-41f, 0, 4.6142745e-37f, -213558848);
    path.lineTo(-134217728, 2.2226515e+21f);
    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), {0, 0, 9}, kDont_ExpectVerts);

    // simple concave path (star of David)
    path.reset();
    path.moveTo(0.0f, -50.0f);
    path.lineTo(14.43f, -25.0f);
    path.lineTo(43.30f, -25.0f);
    path.lineTo(28.86f, 0.0f);
    path.lineTo(43.30f, 25.0f);
    path.lineTo(14.43f, 25.0f);
    path.lineTo(0.0f, 50.0f);
    path.lineTo(-14.43f, 25.0f);
    path.lineTo(-43.30f, 25.0f);
    path.lineTo(-28.86f, 0.0f);
    path.lineTo(-43.30f, -25.0f);
    path.lineTo(-14.43f, -25.0f);
// uncomment when transparent concave shadows are working
//    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), {0, 0, 9}, kDo_ExpectVerts, true);

    // complex concave path (bowtie)
    path.reset();
    path.moveTo(-50, -50);
    path.lineTo(-50, 50);
    path.lineTo(50, -50);
    path.lineTo(50, 50);
    path.lineTo(-50, -50);
    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), {0, 0, 9}, kDont_ExpectVerts);

    // multiple contour path
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), {0, 0, 9}, kDont_ExpectVerts);
}

void check_xformed_bounds(skiatest::Reporter* reporter, const SkPath& path, const SkMatrix& ctm) {
    SkDrawShadowRec rec = {
        SkPoint3::Make(0, 0, 4),
        SkPoint3::Make(100, 0, 600),
        800.f,
        0x08000000,
        0x40000000,
        0
    };
    // point light
    SkRect bounds;
    SkDrawShadowMetrics::GetLocalBounds(path, rec, ctm, &bounds);
    ctm.mapRect(&bounds);

    auto verts = SkShadowTessellator::MakeAmbient(path, ctm, rec.fZPlaneParams, true);
    if (verts) {
        REPORTER_ASSERT(reporter, bounds.contains(verts->bounds()));
    }

    SkPoint mapXY = ctm.mapPoint({rec.fLightPos.fX, rec.fLightPos.fY});
    SkPoint3 devLightPos = SkPoint3::Make(mapXY.fX, mapXY.fY, rec.fLightPos.fZ);
    verts = SkShadowTessellator::MakeSpot(path, ctm, rec.fZPlaneParams, devLightPos,
                                          rec.fLightRadius, false, false);
    if (verts) {
        REPORTER_ASSERT(reporter, bounds.contains(verts->bounds()));
    }

    // directional light
    rec.fFlags |= SkShadowFlags::kDirectionalLight_ShadowFlag;
    rec.fLightRadius = 2.0f;
    SkDrawShadowMetrics::GetLocalBounds(path, rec, ctm, &bounds);
    ctm.mapRect(&bounds);

    verts = SkShadowTessellator::MakeAmbient(path, ctm, rec.fZPlaneParams, true);
    if (verts) {
        REPORTER_ASSERT(reporter, bounds.contains(verts->bounds()));
    }

    devLightPos = rec.fLightPos;
    devLightPos.normalize();
    verts = SkShadowTessellator::MakeSpot(path, ctm, rec.fZPlaneParams, devLightPos,
                                          rec.fLightRadius, false, true);
    if (verts) {
        REPORTER_ASSERT(reporter, bounds.contains(verts->bounds()));
    }
}

void check_bounds(skiatest::Reporter* reporter, const SkPath& path) {
    const bool fixed_shadows_in_perspective = false;    // skbug.com/40041026

    SkMatrix ctm;
    ctm.setTranslate(100, 100);
    check_xformed_bounds(reporter, path, ctm);
    ctm.postScale(2, 2);
    check_xformed_bounds(reporter, path, ctm);
    ctm.preRotate(45);
    check_xformed_bounds(reporter, path, ctm);
    ctm.preSkew(40, -20);
    check_xformed_bounds(reporter, path, ctm);
    if (fixed_shadows_in_perspective) {
        ctm[SkMatrix::kMPersp0] = 0.0001f;
        ctm[SkMatrix::kMPersp1] = 12.f;
        check_xformed_bounds(reporter, path, ctm);
        ctm[SkMatrix::kMPersp0] = 0.0001f;
        ctm[SkMatrix::kMPersp1] = -12.f;
        check_xformed_bounds(reporter, path, ctm);
        ctm[SkMatrix::kMPersp0] = 12.f;
        ctm[SkMatrix::kMPersp1] = 0.0001f;
        check_xformed_bounds(reporter, path, ctm);
    }
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

#endif // !defined(SK_ENABLE_OPTIMIZE_SIZE)
