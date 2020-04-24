/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkScalar.h"
#include "src/gpu/geometry/GrQuad.h"
#include "src/gpu/geometry/GrQuadUtils.h"
#include "tests/Test.h"

#define ASSERT(cond) REPORTER_ASSERT(r, cond)
#define ASSERTF(cond, ...) REPORTER_ASSERT(r, cond, __VA_ARGS__)
#define TEST(name) DEF_TEST(GrQuadCrop##name, r)
#define ASSERT_NEARLY_EQUAL(expected, actual) \
    ASSERTF(SkScalarNearlyEqual(expected, actual), "expected: %f, actual: %f", \
            expected, actual)

// Make the base rect contain the origin and have unique edge values so that each transform
// produces a different axis-aligned rectangle.
static const SkRect kDrawRect = SkRect::MakeLTRB(-5.f, -6.f, 10.f, 11.f);

static void run_crop_axis_aligned_test(skiatest::Reporter* r, const SkRect& clipRect, GrAA clipAA,
                                       const SkMatrix& viewMatrix, const SkMatrix* localMatrix) {
    // Should use run_crop_fully_covers_test for non-rect matrices
    SkASSERT(viewMatrix.rectStaysRect());

    GrQuad drawQuad = GrQuad::MakeFromRect(kDrawRect, viewMatrix);
    GrQuad localQuad = GrQuad::MakeFromRect(kDrawRect, localMatrix ? *localMatrix : SkMatrix::I());
    GrQuad* localQuadPtr = localMatrix ? &localQuad : nullptr;
    GrQuadAAFlags edgeFlags = clipAA == GrAA::kYes ? GrQuadAAFlags::kNone : GrQuadAAFlags::kAll;

    bool exact = GrQuadUtils::CropToRect(clipRect, clipAA, &edgeFlags, &drawQuad, localQuadPtr);
    ASSERTF(exact, "Expected exact crop");
    ASSERTF(drawQuad.quadType() == GrQuad::Type::kAxisAligned,
            "Expected quad to remain axis-aligned");

    // Since we remained a rectangle, the bounds will exactly match the coordinates
    SkRect expectedBounds = viewMatrix.mapRect(kDrawRect);
    SkAssertResult(expectedBounds.intersect(clipRect));

    SkRect actualBounds = drawQuad.bounds();
    ASSERT_NEARLY_EQUAL(expectedBounds.fLeft, actualBounds.fLeft);
    ASSERT_NEARLY_EQUAL(expectedBounds.fTop, actualBounds.fTop);
    ASSERT_NEARLY_EQUAL(expectedBounds.fRight, actualBounds.fRight);
    ASSERT_NEARLY_EQUAL(expectedBounds.fBottom, actualBounds.fBottom);

    // Confirm that local coordinates match up with clipped edges and the transform
    SkMatrix invViewMatrix;
    SkAssertResult(viewMatrix.invert(&invViewMatrix));

    if (localMatrix) {
        SkMatrix toLocal = SkMatrix::Concat(*localMatrix, invViewMatrix);

        for (int p = 0; p < 4; ++p) {
            SkPoint expectedPoint = drawQuad.point(p);
            toLocal.mapPoints(&expectedPoint, 1);
            SkPoint actualPoint = localQuad.point(p);

            ASSERT_NEARLY_EQUAL(expectedPoint.fX, actualPoint.fX);
            ASSERT_NEARLY_EQUAL(expectedPoint.fY, actualPoint.fY);
        }
    }

    // Confirm that the edge flags match, by mapping clip rect to drawRect space and
    // comparing to the original draw rect edges
    SkRect drawClip = invViewMatrix.mapRect(clipRect);
    if (drawClip.fLeft > kDrawRect.fLeft) {
        if (clipAA == GrAA::kYes) {
            ASSERTF(edgeFlags & GrQuadAAFlags::kLeft, "Expected left edge AA set");
        } else {
            ASSERTF(!(edgeFlags & GrQuadAAFlags::kLeft), "Expected left edge AA unset");
        }
    }
    if (drawClip.fRight < kDrawRect.fRight) {
        if (clipAA == GrAA::kYes) {
            ASSERTF(edgeFlags & GrQuadAAFlags::kRight, "Expected right edge AA set");
        } else {
            ASSERTF(!(edgeFlags & GrQuadAAFlags::kRight),  "Expected right edge AA unset");
        }
    }
    if (drawClip.fTop > kDrawRect.fTop) {
        if (clipAA == GrAA::kYes) {
            ASSERTF(edgeFlags & GrQuadAAFlags::kTop, "Expected top edge AA set");
        } else {
            ASSERTF(!(edgeFlags & GrQuadAAFlags::kTop), "Expected top edge AA unset");
        }
    }
    if (drawClip.fBottom < kDrawRect.fBottom) {
        if (clipAA == GrAA::kYes) {
            ASSERTF(edgeFlags & GrQuadAAFlags::kBottom, "Expected bottom edge AA set");
        } else {
            ASSERTF(!(edgeFlags & GrQuadAAFlags::kBottom), "Expected bottom edge AA unset");
        }
    }
}

static void run_crop_fully_covered_test(skiatest::Reporter* r, GrAA clipAA,
                                        const SkMatrix& viewMatrix, const SkMatrix* localMatrix) {
    // Should use run_crop_axis_aligned for rect transforms since that verifies more behavior
    SkASSERT(!viewMatrix.rectStaysRect());

    // Test what happens when the geometry fully covers the crop rect. Given a fixed crop,
    // use the provided view matrix to derive the "input" geometry that we know covers the crop.
    SkMatrix invViewMatrix;
    SkAssertResult(viewMatrix.invert(&invViewMatrix));

    SkRect containsCrop = kDrawRect; // Use kDrawRect as the crop rect for this test
    containsCrop.outset(10.f, 10.f);
    SkRect drawRect = invViewMatrix.mapRect(containsCrop);

    GrQuad drawQuad = GrQuad::MakeFromRect(drawRect, viewMatrix);
    GrQuadAAFlags edgeFlags = clipAA == GrAA::kYes ? GrQuadAAFlags::kNone : GrQuadAAFlags::kAll;

    if (localMatrix) {
        GrQuad localQuad = GrQuad::MakeFromRect(drawRect, *localMatrix);

        GrQuad originalDrawQuad = drawQuad;
        GrQuad originalLocalQuad = localQuad;
        GrQuadAAFlags originalEdgeFlags = edgeFlags;

        bool exact = GrQuadUtils::CropToRect(kDrawRect, clipAA, &edgeFlags, &drawQuad, &localQuad);
        // Currently non-rect matrices don't know how to update local coordinates, so the crop
        // doesn't know how to restrict itself and should leave the inputs unmodified
        ASSERTF(!exact, "Expected crop to be not exact");
        ASSERTF(edgeFlags == originalEdgeFlags, "Expected edge flags not to be modified");

        for (int i = 0; i < 4; ++i) {
            ASSERT_NEARLY_EQUAL(originalDrawQuad.x(i), drawQuad.x(i));
            ASSERT_NEARLY_EQUAL(originalDrawQuad.y(i), drawQuad.y(i));
            ASSERT_NEARLY_EQUAL(originalDrawQuad.w(i), drawQuad.w(i));

            ASSERT_NEARLY_EQUAL(originalLocalQuad.x(i), localQuad.x(i));
            ASSERT_NEARLY_EQUAL(originalLocalQuad.y(i), localQuad.y(i));
            ASSERT_NEARLY_EQUAL(originalLocalQuad.w(i), localQuad.w(i));
        }
    } else {
        // Since no local coordinates were provided, and the input draw geometry is known to
        // fully cover the crop rect, the quad should be updated to match cropRect exactly
        bool exact = GrQuadUtils::CropToRect(kDrawRect, clipAA, &edgeFlags, &drawQuad, nullptr);
        ASSERTF(exact, "Expected crop to be exact");

        GrQuadAAFlags expectedFlags = clipAA == GrAA::kYes ? GrQuadAAFlags::kAll
                                                           : GrQuadAAFlags::kNone;
        ASSERTF(expectedFlags == edgeFlags, "Expected edge flags do not match clip AA setting");
        ASSERTF(drawQuad.quadType() == GrQuad::Type::kAxisAligned, "Unexpected quad type");

        ASSERT_NEARLY_EQUAL(kDrawRect.fLeft, drawQuad.x(0));
        ASSERT_NEARLY_EQUAL(kDrawRect.fTop, drawQuad.y(0));
        ASSERT_NEARLY_EQUAL(1.f, drawQuad.w(0));

        ASSERT_NEARLY_EQUAL(kDrawRect.fLeft, drawQuad.x(1));
        ASSERT_NEARLY_EQUAL(kDrawRect.fBottom, drawQuad.y(1));
        ASSERT_NEARLY_EQUAL(1.f, drawQuad.w(1));

        ASSERT_NEARLY_EQUAL(kDrawRect.fRight, drawQuad.x(2));
        ASSERT_NEARLY_EQUAL(kDrawRect.fTop, drawQuad.y(2));
        ASSERT_NEARLY_EQUAL(1.f, drawQuad.w(2));

        ASSERT_NEARLY_EQUAL(kDrawRect.fRight, drawQuad.x(3));
        ASSERT_NEARLY_EQUAL(kDrawRect.fBottom, drawQuad.y(3));
        ASSERT_NEARLY_EQUAL(1.f, drawQuad.w(3));
    }
}

static void test_axis_aligned_all_clips(skiatest::Reporter* r, const SkMatrix& viewMatrix,
                                        const SkMatrix* localMatrix) {
    static const float kInsideEdge = SkScalarAbs(kDrawRect.fLeft) - 1.f;
    static const float kOutsideEdge = SkScalarAbs(kDrawRect.fBottom) + 1.f;
    static const float kIntersectEdge = SkScalarAbs(kDrawRect.fTop) + 1.f;

    static const SkRect kInsideClipRect = SkRect::MakeLTRB(-kInsideEdge, -kInsideEdge,
                                                           kInsideEdge, kInsideEdge);
    static const SkRect kContainsClipRect = SkRect::MakeLTRB(-kOutsideEdge, -kOutsideEdge,
                                                             kOutsideEdge, kOutsideEdge);
    static const SkRect kXYAxesClipRect = SkRect::MakeLTRB(-kIntersectEdge, -kIntersectEdge,
                                                           kIntersectEdge, kIntersectEdge);
    static const SkRect kXAxisClipRect = SkRect::MakeLTRB(-kIntersectEdge, -kOutsideEdge,
                                                          kIntersectEdge, kOutsideEdge);
    static const SkRect kYAxisClipRect = SkRect::MakeLTRB(-kOutsideEdge, -kIntersectEdge,
                                                          kOutsideEdge, kIntersectEdge);

    run_crop_axis_aligned_test(r, kInsideClipRect, GrAA::kNo, viewMatrix, localMatrix);
    run_crop_axis_aligned_test(r, kContainsClipRect, GrAA::kNo, viewMatrix, localMatrix);
    run_crop_axis_aligned_test(r, kXYAxesClipRect, GrAA::kNo, viewMatrix, localMatrix);
    run_crop_axis_aligned_test(r, kXAxisClipRect, GrAA::kNo, viewMatrix, localMatrix);
    run_crop_axis_aligned_test(r, kYAxisClipRect, GrAA::kNo, viewMatrix, localMatrix);

    run_crop_axis_aligned_test(r, kInsideClipRect, GrAA::kYes, viewMatrix, localMatrix);
    run_crop_axis_aligned_test(r, kContainsClipRect, GrAA::kYes, viewMatrix, localMatrix);
    run_crop_axis_aligned_test(r, kXYAxesClipRect, GrAA::kYes, viewMatrix, localMatrix);
    run_crop_axis_aligned_test(r, kXAxisClipRect, GrAA::kYes, viewMatrix, localMatrix);
    run_crop_axis_aligned_test(r, kYAxisClipRect, GrAA::kYes, viewMatrix, localMatrix);
}

static void test_axis_aligned(skiatest::Reporter* r, const SkMatrix& viewMatrix) {
    test_axis_aligned_all_clips(r, viewMatrix, nullptr);

    SkMatrix normalized = SkMatrix::MakeRectToRect(kDrawRect, SkRect::MakeWH(1.f, 1.f),
                                                   SkMatrix::kFill_ScaleToFit);
    test_axis_aligned_all_clips(r, viewMatrix, &normalized);

    SkMatrix rotated;
    rotated.setRotate(45.f);
    test_axis_aligned_all_clips(r, viewMatrix, &rotated);

    SkMatrix perspective;
    perspective.setPerspY(0.001f);
    perspective.setSkewX(8.f / 25.f);
    test_axis_aligned_all_clips(r, viewMatrix, &perspective);
}

static void test_crop_fully_covered(skiatest::Reporter* r, const SkMatrix& viewMatrix) {
    run_crop_fully_covered_test(r, GrAA::kNo, viewMatrix, nullptr);
    run_crop_fully_covered_test(r, GrAA::kYes, viewMatrix, nullptr);

    SkMatrix normalized = SkMatrix::MakeRectToRect(kDrawRect, SkRect::MakeWH(1.f, 1.f),
                                                   SkMatrix::kFill_ScaleToFit);
    run_crop_fully_covered_test(r, GrAA::kNo, viewMatrix, &normalized);
    run_crop_fully_covered_test(r, GrAA::kYes, viewMatrix, &normalized);

    SkMatrix rotated;
    rotated.setRotate(45.f);
    run_crop_fully_covered_test(r, GrAA::kNo, viewMatrix, &rotated);
    run_crop_fully_covered_test(r, GrAA::kYes, viewMatrix, &rotated);

    SkMatrix perspective;
    perspective.setPerspY(0.001f);
    perspective.setSkewX(8.f / 25.f);
    run_crop_fully_covered_test(r, GrAA::kNo, viewMatrix, &perspective);
    run_crop_fully_covered_test(r, GrAA::kYes, viewMatrix, &perspective);
}

TEST(AxisAligned) {
    test_axis_aligned(r, SkMatrix::I());
    test_axis_aligned(r, SkMatrix::MakeScale(-1.f, 1.f));
    test_axis_aligned(r, SkMatrix::MakeScale(1.f, -1.f));

    SkMatrix rotation;
    rotation.setRotate(90.f);
    test_axis_aligned(r, rotation);
    rotation.setRotate(180.f);
    test_axis_aligned(r, rotation);
    rotation.setRotate(270.f);
    test_axis_aligned(r, rotation);
}

TEST(FullyCovered) {
    SkMatrix rotation;
    rotation.setRotate(34.f);
    test_crop_fully_covered(r, rotation);

    SkMatrix skew;
    skew.setSkewX(0.3f);
    skew.setSkewY(0.04f);
    test_crop_fully_covered(r, skew);

    SkMatrix perspective;
    perspective.setPerspX(0.001f);
    perspective.setSkewY(8.f / 25.f);
    test_crop_fully_covered(r, perspective);
}
