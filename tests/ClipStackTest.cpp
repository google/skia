/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkTemplates.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkClipStack.h"
#include "tests/Test.h"

#include <cstring>
#include <initializer_list>
#include <new>

static void test_assign_and_comparison(skiatest::Reporter* reporter) {
    SkClipStack s;
    bool doAA = false;

    REPORTER_ASSERT(reporter, 0 == s.getSaveCount());

    // Build up a clip stack with a path, an empty clip, and a rect.
    s.save();
    REPORTER_ASSERT(reporter, 1 == s.getSaveCount());

    SkPath p;
    p.moveTo(5, 6);
    p.lineTo(7, 8);
    p.lineTo(5, 9);
    p.close();
    s.clipPath(p, SkMatrix::I(), SkClipOp::kIntersect, doAA);

    s.save();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());

    SkRect r = SkRect::MakeLTRB(1, 2, 103, 104);
    s.clipRect(r, SkMatrix::I(), SkClipOp::kIntersect, doAA);
    r = SkRect::MakeLTRB(4, 5, 56, 57);
    s.clipRect(r, SkMatrix::I(), SkClipOp::kIntersect, doAA);

    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());

    r = SkRect::MakeLTRB(14, 15, 16, 17);
    s.clipRect(r, SkMatrix::I(), SkClipOp::kDifference, doAA);

    // Test that assignment works.
    SkClipStack copy = s;
    REPORTER_ASSERT(reporter, s == copy);

    // Test that different save levels triggers not equal.
    s.restore();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());
    REPORTER_ASSERT(reporter, s != copy);

    // Test that an equal, but not copied version is equal.
    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());
    r = SkRect::MakeLTRB(14, 15, 16, 17);
    s.clipRect(r, SkMatrix::I(), SkClipOp::kDifference, doAA);
    REPORTER_ASSERT(reporter, s == copy);

    // Test that a different op on one level triggers not equal.
    s.restore();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());
    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());
    r = SkRect::MakeLTRB(14, 15, 16, 17);
    s.clipRect(r, SkMatrix::I(), SkClipOp::kIntersect, doAA);
    REPORTER_ASSERT(reporter, s != copy);

    // Test that version constructed with rect-path rather than a rect is still considered equal.
    s.restore();
    s.save();
    SkPath rp;
    rp.addRect(r);
    s.clipPath(rp, SkMatrix::I(), SkClipOp::kDifference, doAA);
    REPORTER_ASSERT(reporter, s == copy);

    // Test that different rects triggers not equal.
    s.restore();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());
    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());

    r = SkRect::MakeLTRB(24, 25, 26, 27);
    s.clipRect(r, SkMatrix::I(), SkClipOp::kDifference, doAA);
    REPORTER_ASSERT(reporter, s != copy);

    s.restore();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());

    copy.restore();
    REPORTER_ASSERT(reporter, 2 == copy.getSaveCount());
    REPORTER_ASSERT(reporter, s == copy);
    s.restore();
    REPORTER_ASSERT(reporter, 1 == s.getSaveCount());
    copy.restore();
    REPORTER_ASSERT(reporter, 1 == copy.getSaveCount());
    REPORTER_ASSERT(reporter, s == copy);

    // Test that different paths triggers not equal.
    s.restore();
    REPORTER_ASSERT(reporter, 0 == s.getSaveCount());
    s.save();
    REPORTER_ASSERT(reporter, 1 == s.getSaveCount());

    p.addRect(r);
    s.clipPath(p, SkMatrix::I(), SkClipOp::kIntersect, doAA);
    REPORTER_ASSERT(reporter, s != copy);
}

static void assert_count(skiatest::Reporter* reporter, const SkClipStack& stack,
                         int count) {
    SkClipStack::B2TIter iter(stack);
    int counter = 0;
    while (iter.next()) {
        counter += 1;
    }
    REPORTER_ASSERT(reporter, count == counter);
}

// Exercise the SkClipStack's bottom to top and bidirectional iterators
// (including the skipToTopmost functionality)
static void test_iterators(skiatest::Reporter* reporter) {
    SkClipStack stack;

    static const SkRect gRects[] = {
        { 0,   0,  40,  40 },
        { 60,  0, 100,  40 },
        { 0,  60,  40, 100 },
        { 60, 60, 100, 100 }
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRects); i++) {
        // the difference op will prevent these from being fused together
        stack.clipRect(gRects[i], SkMatrix::I(), SkClipOp::kDifference, false);
    }

    assert_count(reporter, stack, 4);

    // bottom to top iteration
    {
        const SkClipStack::Element* element = nullptr;

        SkClipStack::B2TIter iter(stack);
        int i;

        for (i = 0, element = iter.next(); element; ++i, element = iter.next()) {
            REPORTER_ASSERT(reporter, SkClipStack::Element::DeviceSpaceType::kRect ==
                                              element->getDeviceSpaceType());
            REPORTER_ASSERT(reporter, element->getDeviceSpaceRect() == gRects[i]);
        }

        SkASSERT(i == 4);
    }

    // top to bottom iteration
    {
        const SkClipStack::Element* element = nullptr;

        SkClipStack::Iter iter(stack, SkClipStack::Iter::kTop_IterStart);
        int i;

        for (i = 3, element = iter.prev(); element; --i, element = iter.prev()) {
            REPORTER_ASSERT(reporter, SkClipStack::Element::DeviceSpaceType::kRect ==
                                              element->getDeviceSpaceType());
            REPORTER_ASSERT(reporter, element->getDeviceSpaceRect() == gRects[i]);
        }

        SkASSERT(i == -1);
    }

    // skipToTopmost
    {
        const SkClipStack::Element* element = nullptr;

        SkClipStack::Iter iter(stack, SkClipStack::Iter::kBottom_IterStart);

        element = iter.skipToTopmost(SkClipOp::kDifference);
        REPORTER_ASSERT(reporter, SkClipStack::Element::DeviceSpaceType::kRect ==
                                          element->getDeviceSpaceType());
        REPORTER_ASSERT(reporter, element->getDeviceSpaceRect() == gRects[3]);
    }
}

// Exercise the SkClipStack's getConservativeBounds computation
static void test_bounds(skiatest::Reporter* reporter,
                        SkClipStack::Element::DeviceSpaceType primType) {
    static const int gNumCases = 8;
    static const SkRect gAnswerRectsBW[gNumCases] = {
        // A op B
        { 40, 40, 50, 50 },
        { 10, 10, 50, 50 },

        // invA op B
        { 40, 40, 80, 80 },
        { 0, 0, 100, 100 },

        // A op invB
        { 10, 10, 50, 50 },
        { 40, 40, 50, 50 },

        // invA op invB
        { 0, 0, 100, 100 },
        { 40, 40, 80, 80 },
    };

    static const SkClipOp gOps[] = {
        SkClipOp::kIntersect,
        SkClipOp::kDifference
    };

    SkRect rectA, rectB;

    rectA.setLTRB(10, 10, 50, 50);
    rectB.setLTRB(40, 40, 80, 80);

    SkRRect rrectA, rrectB;
    rrectA.setOval(rectA);
    rrectB.setRectXY(rectB, SkIntToScalar(1), SkIntToScalar(2));

    SkPath pathA, pathB;

    pathA.addRoundRect(rectA, SkIntToScalar(5), SkIntToScalar(5));
    pathB.addRoundRect(rectB, SkIntToScalar(5), SkIntToScalar(5));

    SkClipStack stack;
    SkRect devClipBound;
    bool isIntersectionOfRects = false;

    int testCase = 0;
    int numBitTests = SkClipStack::Element::DeviceSpaceType::kPath == primType ? 4 : 1;
    for (int invBits = 0; invBits < numBitTests; ++invBits) {
        for (size_t op = 0; op < SK_ARRAY_COUNT(gOps); ++op) {

            stack.save();
            bool doInvA = SkToBool(invBits & 1);
            bool doInvB = SkToBool(invBits & 2);

            pathA.setFillType(doInvA ? SkPathFillType::kInverseEvenOdd :
                                       SkPathFillType::kEvenOdd);
            pathB.setFillType(doInvB ? SkPathFillType::kInverseEvenOdd :
                                       SkPathFillType::kEvenOdd);

            switch (primType) {
                case SkClipStack::Element::DeviceSpaceType::kShader:
                case SkClipStack::Element::DeviceSpaceType::kEmpty:
                    SkDEBUGFAIL("Don't call this with kEmpty or kShader.");
                    break;
                case SkClipStack::Element::DeviceSpaceType::kRect:
                    stack.clipRect(rectA, SkMatrix::I(), SkClipOp::kIntersect, false);
                    stack.clipRect(rectB, SkMatrix::I(), gOps[op], false);
                    break;
                case SkClipStack::Element::DeviceSpaceType::kRRect:
                    stack.clipRRect(rrectA, SkMatrix::I(), SkClipOp::kIntersect, false);
                    stack.clipRRect(rrectB, SkMatrix::I(), gOps[op], false);
                    break;
                case SkClipStack::Element::DeviceSpaceType::kPath:
                    stack.clipPath(pathA, SkMatrix::I(), SkClipOp::kIntersect, false);
                    stack.clipPath(pathB, SkMatrix::I(), gOps[op], false);
                    break;
            }

            REPORTER_ASSERT(reporter, !stack.isWideOpen());
            REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID != stack.getTopmostGenID());

            stack.getConservativeBounds(0, 0, 100, 100, &devClipBound,
                                        &isIntersectionOfRects);

            if (SkClipStack::Element::DeviceSpaceType::kRect == primType) {
                REPORTER_ASSERT(reporter, isIntersectionOfRects ==
                        (gOps[op] == SkClipOp::kIntersect));
            } else {
                REPORTER_ASSERT(reporter, !isIntersectionOfRects);
            }

            SkASSERT(testCase < gNumCases);
            REPORTER_ASSERT(reporter, devClipBound == gAnswerRectsBW[testCase]);
            ++testCase;

            stack.restore();
        }
    }
}

// Test out 'isWideOpen' entry point
static void test_isWideOpen(skiatest::Reporter* reporter) {
    {
        // Empty stack is wide open. Wide open stack means that gen id is wide open.
        SkClipStack stack;
        REPORTER_ASSERT(reporter, stack.isWideOpen());
        REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID == stack.getTopmostGenID());
    }

    SkRect rectA, rectB;

    rectA.setLTRB(10, 10, 40, 40);
    rectB.setLTRB(50, 50, 80, 80);

    // Stack should initially be wide open
    {
        SkClipStack stack;

        REPORTER_ASSERT(reporter, stack.isWideOpen());
        REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID == stack.getTopmostGenID());
    }

    // Test out empty difference from a wide open clip
    {
        SkClipStack stack;

        SkRect emptyRect;
        emptyRect.setEmpty();

        stack.clipRect(emptyRect, SkMatrix::I(), SkClipOp::kDifference, false);

        REPORTER_ASSERT(reporter, stack.isWideOpen());
        REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID == stack.getTopmostGenID());
    }

    // Test out return to wide open
    {
        SkClipStack stack;

        stack.save();

        stack.clipRect(rectA, SkMatrix::I(), SkClipOp::kIntersect, false);

        REPORTER_ASSERT(reporter, !stack.isWideOpen());
        REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID != stack.getTopmostGenID());

        stack.restore();

        REPORTER_ASSERT(reporter, stack.isWideOpen());
        REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID == stack.getTopmostGenID());
    }
}

static int count(const SkClipStack& stack) {

    SkClipStack::Iter iter(stack, SkClipStack::Iter::kTop_IterStart);

    const SkClipStack::Element* element = nullptr;
    int count = 0;

    for (element = iter.prev(); element; element = iter.prev(), ++count) {
    }

    return count;
}

static void test_rect_inverse_fill(skiatest::Reporter* reporter) {
    // non-intersecting rectangles
    SkRect rect  = SkRect::MakeLTRB(0, 0, 10, 10);

    SkPath path;
    path.addRect(rect);
    path.toggleInverseFillType();
    SkClipStack stack;
    stack.clipPath(path, SkMatrix::I(), SkClipOp::kIntersect, false);

    SkRect bounds;
    SkClipStack::BoundsType boundsType;
    stack.getBounds(&bounds, &boundsType);
    REPORTER_ASSERT(reporter, SkClipStack::kInsideOut_BoundsType == boundsType);
    REPORTER_ASSERT(reporter, bounds == rect);
}

static void test_rect_replace(skiatest::Reporter* reporter) {
    SkRect rect = SkRect::MakeWH(100, 100);
    SkRect rect2 = SkRect::MakeXYWH(50, 50, 100, 100);

    SkRect bound;
    SkClipStack::BoundsType type;
    bool isIntersectionOfRects;

    // Adding a new rect with the replace operator should not increase
    // the stack depth. BW replacing BW.
    {
        SkClipStack stack;
        REPORTER_ASSERT(reporter, 0 == count(stack));
        stack.replaceClip(rect, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.replaceClip(rect, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
    }

    // Adding a new rect with the replace operator should not increase
    // the stack depth. AA replacing AA.
    {
        SkClipStack stack;
        REPORTER_ASSERT(reporter, 0 == count(stack));
        stack.replaceClip(rect, true);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.replaceClip(rect, true);
        REPORTER_ASSERT(reporter, 1 == count(stack));
    }

    // Adding a new rect with the replace operator should not increase
    // the stack depth. BW replacing AA replacing BW.
    {
        SkClipStack stack;
        REPORTER_ASSERT(reporter, 0 == count(stack));
        stack.replaceClip(rect, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.replaceClip(rect, true);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.replaceClip(rect, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
    }

    // Make sure replace clip rects don't collapse too much.
    {
        SkClipStack stack;
        stack.replaceClip(rect, false);
        stack.clipRect(rect2, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.save();
        stack.replaceClip(rect, false);
        REPORTER_ASSERT(reporter, 2 == count(stack));
        stack.getBounds(&bound, &type, &isIntersectionOfRects);
        REPORTER_ASSERT(reporter, bound == rect);
        stack.restore();
        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.save();
        stack.replaceClip(rect, false);
        stack.replaceClip(rect, false);
        REPORTER_ASSERT(reporter, 2 == count(stack));
        stack.restore();
        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.save();
        stack.replaceClip(rect, false);
        stack.clipRect(rect2, SkMatrix::I(), SkClipOp::kIntersect, false);
        stack.replaceClip(rect, false);
        REPORTER_ASSERT(reporter, 2 == count(stack));
        stack.restore();
        REPORTER_ASSERT(reporter, 1 == count(stack));
    }
}

// Simplified path-based version of test_rect_replace.
static void test_path_replace(skiatest::Reporter* reporter) {
    auto replacePath = [](SkClipStack* stack, const SkPath& path, bool doAA) {
        const SkRect wideOpen = SkRect::MakeLTRB(-1000, -1000, 1000, 1000);
        stack->replaceClip(wideOpen, false);
        stack->clipPath(path, SkMatrix::I(), SkClipOp::kIntersect, doAA);
    };
    SkRect rect = SkRect::MakeWH(100, 100);
    SkPath path;
    path.addCircle(50, 50, 50);

    // Emulating replace operations with more complex geometry is not atomic, it's a replace
    // with a wide-open rect and then an intersection with the complex geometry. The replace can
    // combine with prior elements, but the subsequent intersect cannot be combined so the stack
    // continues to grow.
    {
        SkClipStack stack;
        REPORTER_ASSERT(reporter, 0 == count(stack));
        replacePath(&stack, path, false);
        REPORTER_ASSERT(reporter, 2 == count(stack));
        replacePath(&stack, path, false);
        REPORTER_ASSERT(reporter, 2 == count(stack));
    }

    // Replacing rect with path.
    {
        SkClipStack stack;
        stack.replaceClip(rect, true);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        replacePath(&stack, path, true);
        REPORTER_ASSERT(reporter, 2 == count(stack));
    }
}

// Test out SkClipStack's merging of rect clips. In particular exercise
// merging of aa vs. bw rects.
static void test_rect_merging(skiatest::Reporter* reporter) {

    SkRect overlapLeft  = SkRect::MakeLTRB(10, 10, 50, 50);
    SkRect overlapRight = SkRect::MakeLTRB(40, 40, 80, 80);

    SkRect nestedParent = SkRect::MakeLTRB(10, 10, 90, 90);
    SkRect nestedChild  = SkRect::MakeLTRB(40, 40, 60, 60);

    SkRect bound;
    SkClipStack::BoundsType type;
    bool isIntersectionOfRects;

    // all bw overlapping - should merge
    {
        SkClipStack stack;
        stack.clipRect(overlapLeft, SkMatrix::I(), SkClipOp::kIntersect, false);
        stack.clipRect(overlapRight, SkMatrix::I(), SkClipOp::kIntersect, false);

        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, isIntersectionOfRects);
    }

    // all aa overlapping - should merge
    {
        SkClipStack stack;
        stack.clipRect(overlapLeft, SkMatrix::I(), SkClipOp::kIntersect, true);
        stack.clipRect(overlapRight, SkMatrix::I(), SkClipOp::kIntersect, true);

        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, isIntersectionOfRects);
    }

    // mixed overlapping - should _not_ merge
    {
        SkClipStack stack;
        stack.clipRect(overlapLeft, SkMatrix::I(), SkClipOp::kIntersect, true);
        stack.clipRect(overlapRight, SkMatrix::I(), SkClipOp::kIntersect, false);

        REPORTER_ASSERT(reporter, 2 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, !isIntersectionOfRects);
    }

    // mixed nested (bw inside aa) - should merge
    {
        SkClipStack stack;
        stack.clipRect(nestedParent, SkMatrix::I(), SkClipOp::kIntersect, true);
        stack.clipRect(nestedChild, SkMatrix::I(), SkClipOp::kIntersect, false);

        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, isIntersectionOfRects);
    }

    // mixed nested (aa inside bw) - should merge
    {
        SkClipStack stack;
        stack.clipRect(nestedParent, SkMatrix::I(), SkClipOp::kIntersect, false);
        stack.clipRect(nestedChild, SkMatrix::I(), SkClipOp::kIntersect, true);

        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, isIntersectionOfRects);
    }

    // reverse nested (aa inside bw) - should _not_ merge
    {
        SkClipStack stack;
        stack.clipRect(nestedChild, SkMatrix::I(), SkClipOp::kIntersect, false);
        stack.clipRect(nestedParent, SkMatrix::I(), SkClipOp::kIntersect, true);

        REPORTER_ASSERT(reporter, 2 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, !isIntersectionOfRects);
    }
}

static void test_quickContains(skiatest::Reporter* reporter) {
    SkRect testRect = SkRect::MakeLTRB(10, 10, 40, 40);
    SkRect insideRect = SkRect::MakeLTRB(20, 20, 30, 30);
    SkRect intersectingRect = SkRect::MakeLTRB(25, 25, 50, 50);
    SkRect outsideRect = SkRect::MakeLTRB(0, 0, 50, 50);
    SkRect nonIntersectingRect = SkRect::MakeLTRB(100, 100, 110, 110);

    SkPath insideCircle;
    insideCircle.addCircle(25, 25, 5);
    SkPath intersectingCircle;
    intersectingCircle.addCircle(25, 40, 10);
    SkPath outsideCircle;
    outsideCircle.addCircle(25, 25, 50);
    SkPath nonIntersectingCircle;
    nonIntersectingCircle.addCircle(100, 100, 5);

    {
        SkClipStack stack;
        stack.clipRect(outsideRect, SkMatrix::I(), SkClipOp::kDifference, false);
        // return false because quickContains currently does not care for kDifference
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    // Replace Op tests
    {
        SkClipStack stack;
        stack.replaceClip(outsideRect, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipRect(insideRect, SkMatrix::I(), SkClipOp::kIntersect, false);
        stack.save(); // To prevent in-place substitution by replace OP
        stack.replaceClip(outsideRect, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
        stack.restore();
    }

    {
        SkClipStack stack;
        stack.clipRect(outsideRect, SkMatrix::I(), SkClipOp::kIntersect, false);
        stack.save(); // To prevent in-place substitution by replace OP
        stack.replaceClip(insideRect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
        stack.restore();
    }

    // Verify proper traversal of multi-element clip
    {
        SkClipStack stack;
        stack.clipRect(insideRect, SkMatrix::I(), SkClipOp::kIntersect, false);
        // Use a path for second clip to prevent in-place intersection
        stack.clipPath(outsideCircle, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    // Intersect Op tests with rectangles
    {
        SkClipStack stack;
        stack.clipRect(outsideRect, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipRect(insideRect, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipRect(intersectingRect, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipRect(nonIntersectingRect, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    // Intersect Op tests with circle paths
    {
        SkClipStack stack;
        stack.clipPath(outsideCircle, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipPath(insideCircle, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipPath(intersectingCircle, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipPath(nonIntersectingCircle, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    // Intersect Op tests with inverse filled rectangles
    {
        SkClipStack stack;
        SkPath path;
        path.addRect(outsideRect);
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path;
        path.addRect(insideRect);
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path;
        path.addRect(intersectingRect);
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path;
        path.addRect(nonIntersectingRect);
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }

    // Intersect Op tests with inverse filled circles
    {
        SkClipStack stack;
        SkPath path = outsideCircle;
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path = insideCircle;
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path = intersectingCircle;
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path = nonIntersectingCircle;
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), SkClipOp::kIntersect, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }
}

static void set_region_to_stack(const SkClipStack& stack, const SkIRect& bounds, SkRegion* region) {
    region->setRect(bounds);
    SkClipStack::Iter iter(stack, SkClipStack::Iter::kBottom_IterStart);
    while (const SkClipStack::Element *element = iter.next()) {
        SkRegion elemRegion;
        SkRegion boundsRgn(bounds);
        SkPath path;

        switch (element->getDeviceSpaceType()) {
            case SkClipStack::Element::DeviceSpaceType::kEmpty:
                elemRegion.setEmpty();
                break;
            default:
                element->asDeviceSpacePath(&path);
                elemRegion.setPath(path, boundsRgn);
                break;
        }

        region->op(elemRegion, element->isReplaceOp() ? SkRegion::kReplace_Op
                                                      : (SkRegion::Op) element->getOp());
    }
}

static void test_invfill_diff_bug(skiatest::Reporter* reporter) {
    SkClipStack stack;
    stack.clipRect({10, 10, 20, 20}, SkMatrix::I(), SkClipOp::kIntersect, false);

    SkPath path;
    path.addRect({30, 10, 40, 20});
    path.setFillType(SkPathFillType::kInverseWinding);
    stack.clipPath(path, SkMatrix::I(), SkClipOp::kDifference, false);

    REPORTER_ASSERT(reporter, SkClipStack::kEmptyGenID == stack.getTopmostGenID());

    SkRect stackBounds;
    SkClipStack::BoundsType stackBoundsType;
    stack.getBounds(&stackBounds, &stackBoundsType);

    REPORTER_ASSERT(reporter, stackBounds.isEmpty());
    REPORTER_ASSERT(reporter, SkClipStack::kNormal_BoundsType == stackBoundsType);

    SkRegion region;
    set_region_to_stack(stack, {0, 0, 50, 30}, &region);

    REPORTER_ASSERT(reporter, region.isEmpty());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void test_is_rrect_deep_rect_stack(skiatest::Reporter* reporter) {
    static constexpr SkRect kTargetBounds = SkRect::MakeWH(1000, 500);
    // All antialiased or all not antialiased.
    for (bool aa : {false, true}) {
        SkClipStack stack;
        for (int i = 0; i <= 100; ++i) {
            stack.save();
            stack.clipRect(SkRect::MakeLTRB(i, 0.5, kTargetBounds.width(), kTargetBounds.height()),
                           SkMatrix::I(), SkClipOp::kIntersect, aa);
        }
        SkRRect rrect;
        bool isAA;
        SkRRect expected = SkRRect::MakeRect(
                SkRect::MakeLTRB(100, 0.5, kTargetBounds.width(), kTargetBounds.height()));
        if (stack.isRRect(kTargetBounds, &rrect, &isAA)) {
            REPORTER_ASSERT(reporter, rrect == expected);
            REPORTER_ASSERT(reporter, aa == isAA);
        } else {
            ERRORF(reporter, "Expected to be an rrect.");
        }
    }
    // Mixed AA and non-AA without simple containment.
    SkClipStack stack;
    for (int i = 0; i <= 100; ++i) {
        bool aa = i & 0b1;
        int j = 100 - i;
        stack.save();
        stack.clipRect(SkRect::MakeLTRB(i, j + 0.5, kTargetBounds.width(), kTargetBounds.height()),
                       SkMatrix::I(), SkClipOp::kIntersect, aa);
    }
    SkRRect rrect;
    bool isAA;
    REPORTER_ASSERT(reporter, !stack.isRRect(kTargetBounds, &rrect, &isAA));
}

DEF_TEST(ClipStack, reporter) {
    SkClipStack stack;

    REPORTER_ASSERT(reporter, 0 == stack.getSaveCount());
    assert_count(reporter, stack, 0);

    static const SkIRect gRects[] = {
        { 0, 0, 100, 100 },
        { 25, 25, 125, 125 },
        { 0, 0, 1000, 1000 },
        { 0, 0, 75, 75 }
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRects); i++) {
        stack.clipDevRect(gRects[i], SkClipOp::kIntersect);
    }

    // all of the above rects should have been intersected, leaving only 1 rect
    SkClipStack::B2TIter iter(stack);
    const SkClipStack::Element* element = iter.next();
    SkRect answer;
    answer.setLTRB(25, 25, 75, 75);

    REPORTER_ASSERT(reporter, element);
    REPORTER_ASSERT(reporter,
                    SkClipStack::Element::DeviceSpaceType::kRect == element->getDeviceSpaceType());
    REPORTER_ASSERT(reporter, SkClipOp::kIntersect == element->getOp());
    REPORTER_ASSERT(reporter, element->getDeviceSpaceRect() == answer);
    // now check that we only had one in our iterator
    REPORTER_ASSERT(reporter, !iter.next());

    stack.reset();
    REPORTER_ASSERT(reporter, 0 == stack.getSaveCount());
    assert_count(reporter, stack, 0);

    test_assign_and_comparison(reporter);
    test_iterators(reporter);
    test_bounds(reporter, SkClipStack::Element::DeviceSpaceType::kRect);
    test_bounds(reporter, SkClipStack::Element::DeviceSpaceType::kRRect);
    test_bounds(reporter, SkClipStack::Element::DeviceSpaceType::kPath);
    test_isWideOpen(reporter);
    test_rect_merging(reporter);
    test_rect_replace(reporter);
    test_rect_inverse_fill(reporter);
    test_path_replace(reporter);
    test_quickContains(reporter);
    test_invfill_diff_bug(reporter);
    test_is_rrect_deep_rect_stack(reporter);
}
