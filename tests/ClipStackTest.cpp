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
#include "include/gpu/GrConfig.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrTexture.h"
#include "include/private/GrResourceKey.h"
#include "include/private/SkTemplates.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkClipOpPriv.h"
#include "src/core/SkClipStack.h"
#include "src/core/SkTLList.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrClipStackClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrReducedClip.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrTextureProxy.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

#include <cstring>
#include <initializer_list>
#include <new>

class GrCaps;

typedef GrReducedClip::ElementList ElementList;
typedef GrReducedClip::InitialState InitialState;

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
    s.clipPath(p, SkMatrix::I(), kIntersect_SkClipOp, doAA);

    s.save();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());

    SkRect r = SkRect::MakeLTRB(1, 2, 3, 4);
    s.clipRect(r, SkMatrix::I(), kIntersect_SkClipOp, doAA);
    r = SkRect::MakeLTRB(10, 11, 12, 13);
    s.clipRect(r, SkMatrix::I(), kIntersect_SkClipOp, doAA);

    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());

    r = SkRect::MakeLTRB(14, 15, 16, 17);
    s.clipRect(r, SkMatrix::I(), kUnion_SkClipOp, doAA);

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
    s.clipRect(r, SkMatrix::I(), kUnion_SkClipOp, doAA);
    REPORTER_ASSERT(reporter, s == copy);

    // Test that a different op on one level triggers not equal.
    s.restore();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());
    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());
    r = SkRect::MakeLTRB(14, 15, 16, 17);
    s.clipRect(r, SkMatrix::I(), kIntersect_SkClipOp, doAA);
    REPORTER_ASSERT(reporter, s != copy);

    // Test that version constructed with rect-path rather than a rect is still considered equal.
    s.restore();
    s.save();
    SkPath rp;
    rp.addRect(r);
    s.clipPath(rp, SkMatrix::I(), kUnion_SkClipOp, doAA);
    REPORTER_ASSERT(reporter, s == copy);

    // Test that different rects triggers not equal.
    s.restore();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());
    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());

    r = SkRect::MakeLTRB(24, 25, 26, 27);
    s.clipRect(r, SkMatrix::I(), kUnion_SkClipOp, doAA);
    REPORTER_ASSERT(reporter, s != copy);

    // Sanity check
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
    s.clipPath(p, SkMatrix::I(), kIntersect_SkClipOp, doAA);
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
        // the union op will prevent these from being fused together
        stack.clipRect(gRects[i], SkMatrix::I(), kUnion_SkClipOp, false);
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

        element = iter.skipToTopmost(kUnion_SkClipOp);
        REPORTER_ASSERT(reporter, SkClipStack::Element::DeviceSpaceType::kRect ==
                                          element->getDeviceSpaceType());
        REPORTER_ASSERT(reporter, element->getDeviceSpaceRect() == gRects[3]);
    }
}

// Exercise the SkClipStack's getConservativeBounds computation
static void test_bounds(skiatest::Reporter* reporter,
                        SkClipStack::Element::DeviceSpaceType primType) {
    static const int gNumCases = 20;
    static const SkRect gAnswerRectsBW[gNumCases] = {
        // A op B
        { 40, 40, 50, 50 },
        { 10, 10, 50, 50 },
        { 10, 10, 80, 80 },
        { 10, 10, 80, 80 },
        { 40, 40, 80, 80 },

        // invA op B
        { 40, 40, 80, 80 },
        { 0, 0, 100, 100 },
        { 0, 0, 100, 100 },
        { 0, 0, 100, 100 },
        { 40, 40, 50, 50 },

        // A op invB
        { 10, 10, 50, 50 },
        { 40, 40, 50, 50 },
        { 0, 0, 100, 100 },
        { 0, 0, 100, 100 },
        { 0, 0, 100, 100 },

        // invA op invB
        { 0, 0, 100, 100 },
        { 40, 40, 80, 80 },
        { 0, 0, 100, 100 },
        { 10, 10, 80, 80 },
        { 10, 10, 50, 50 },
    };

    static const SkClipOp gOps[] = {
        kIntersect_SkClipOp,
        kDifference_SkClipOp,
        kUnion_SkClipOp,
        kXOR_SkClipOp,
        kReverseDifference_SkClipOp
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

            pathA.setFillType(doInvA ? SkPath::kInverseEvenOdd_FillType :
                                       SkPath::kEvenOdd_FillType);
            pathB.setFillType(doInvB ? SkPath::kInverseEvenOdd_FillType :
                                       SkPath::kEvenOdd_FillType);

            switch (primType) {
                case SkClipStack::Element::DeviceSpaceType::kEmpty:
                    SkDEBUGFAIL("Don't call this with kEmpty.");
                    break;
                case SkClipStack::Element::DeviceSpaceType::kRect:
                    stack.clipRect(rectA, SkMatrix::I(), kIntersect_SkClipOp, false);
                    stack.clipRect(rectB, SkMatrix::I(), gOps[op], false);
                    break;
                case SkClipStack::Element::DeviceSpaceType::kRRect:
                    stack.clipRRect(rrectA, SkMatrix::I(), kIntersect_SkClipOp, false);
                    stack.clipRRect(rrectB, SkMatrix::I(), gOps[op], false);
                    break;
                case SkClipStack::Element::DeviceSpaceType::kPath:
                    stack.clipPath(pathA, SkMatrix::I(), kIntersect_SkClipOp, false);
                    stack.clipPath(pathB, SkMatrix::I(), gOps[op], false);
                    break;
            }

            REPORTER_ASSERT(reporter, !stack.isWideOpen());
            REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID != stack.getTopmostGenID());

            stack.getConservativeBounds(0, 0, 100, 100, &devClipBound,
                                        &isIntersectionOfRects);

            if (SkClipStack::Element::DeviceSpaceType::kRect == primType) {
                REPORTER_ASSERT(reporter, isIntersectionOfRects ==
                        (gOps[op] == kIntersect_SkClipOp));
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

    // Test out case where the user specifies a union that includes everything
    {
        SkClipStack stack;

        SkPath clipA, clipB;

        clipA.addRoundRect(rectA, SkIntToScalar(5), SkIntToScalar(5));
        clipA.setFillType(SkPath::kInverseEvenOdd_FillType);

        clipB.addRoundRect(rectB, SkIntToScalar(5), SkIntToScalar(5));
        clipB.setFillType(SkPath::kInverseEvenOdd_FillType);

        stack.clipPath(clipA, SkMatrix::I(), kReplace_SkClipOp, false);
        stack.clipPath(clipB, SkMatrix::I(), kUnion_SkClipOp, false);

        REPORTER_ASSERT(reporter, stack.isWideOpen());
        REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID == stack.getTopmostGenID());
    }

    // Test out union w/ a wide open clip
    {
        SkClipStack stack;

        stack.clipRect(rectA, SkMatrix::I(), kUnion_SkClipOp, false);

        REPORTER_ASSERT(reporter, stack.isWideOpen());
        REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID == stack.getTopmostGenID());
    }

    // Test out empty difference from a wide open clip
    {
        SkClipStack stack;

        SkRect emptyRect;
        emptyRect.setEmpty();

        stack.clipRect(emptyRect, SkMatrix::I(), kDifference_SkClipOp, false);

        REPORTER_ASSERT(reporter, stack.isWideOpen());
        REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID == stack.getTopmostGenID());
    }

    // Test out return to wide open
    {
        SkClipStack stack;

        stack.save();

        stack.clipRect(rectA, SkMatrix::I(), kReplace_SkClipOp, false);

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
    stack.clipPath(path, SkMatrix::I(), kIntersect_SkClipOp, false);

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
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
    }

    // Adding a new rect with the replace operator should not increase
    // the stack depth. AA replacing AA.
    {
        SkClipStack stack;
        REPORTER_ASSERT(reporter, 0 == count(stack));
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, true);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, true);
        REPORTER_ASSERT(reporter, 1 == count(stack));
    }

    // Adding a new rect with the replace operator should not increase
    // the stack depth. BW replacing AA replacing BW.
    {
        SkClipStack stack;
        REPORTER_ASSERT(reporter, 0 == count(stack));
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, true);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
    }

    // Make sure replace clip rects don't collapse too much.
    {
        SkClipStack stack;
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, false);
        stack.clipRect(rect2, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.save();
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, false);
        REPORTER_ASSERT(reporter, 2 == count(stack));
        stack.getBounds(&bound, &type, &isIntersectionOfRects);
        REPORTER_ASSERT(reporter, bound == rect);
        stack.restore();
        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.save();
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, false);
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, false);
        REPORTER_ASSERT(reporter, 2 == count(stack));
        stack.restore();
        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.save();
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, false);
        stack.clipRect(rect2, SkMatrix::I(), kIntersect_SkClipOp, false);
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, false);
        REPORTER_ASSERT(reporter, 2 == count(stack));
        stack.restore();
        REPORTER_ASSERT(reporter, 1 == count(stack));
    }
}

// Simplified path-based version of test_rect_replace.
static void test_path_replace(skiatest::Reporter* reporter) {
    SkRect rect = SkRect::MakeWH(100, 100);
    SkPath path;
    path.addCircle(50, 50, 50);

    // Replace operation doesn't grow the stack.
    {
        SkClipStack stack;
        REPORTER_ASSERT(reporter, 0 == count(stack));
        stack.clipPath(path, SkMatrix::I(), kReplace_SkClipOp, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.clipPath(path, SkMatrix::I(), kReplace_SkClipOp, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
    }

    // Replacing rect with path.
    {
        SkClipStack stack;
        stack.clipRect(rect, SkMatrix::I(), kReplace_SkClipOp, true);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.clipPath(path, SkMatrix::I(), kReplace_SkClipOp, true);
        REPORTER_ASSERT(reporter, 1 == count(stack));
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

        stack.clipRect(overlapLeft, SkMatrix::I(), kReplace_SkClipOp, false);

        stack.clipRect(overlapRight, SkMatrix::I(), kIntersect_SkClipOp, false);

        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, isIntersectionOfRects);
    }

    // all aa overlapping - should merge
    {
        SkClipStack stack;

        stack.clipRect(overlapLeft, SkMatrix::I(), kReplace_SkClipOp, true);

        stack.clipRect(overlapRight, SkMatrix::I(), kIntersect_SkClipOp, true);

        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, isIntersectionOfRects);
    }

    // mixed overlapping - should _not_ merge
    {
        SkClipStack stack;

        stack.clipRect(overlapLeft, SkMatrix::I(), kReplace_SkClipOp, true);

        stack.clipRect(overlapRight, SkMatrix::I(), kIntersect_SkClipOp, false);

        REPORTER_ASSERT(reporter, 2 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, !isIntersectionOfRects);
    }

    // mixed nested (bw inside aa) - should merge
    {
        SkClipStack stack;

        stack.clipRect(nestedParent, SkMatrix::I(), kReplace_SkClipOp, true);

        stack.clipRect(nestedChild, SkMatrix::I(), kIntersect_SkClipOp, false);

        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, isIntersectionOfRects);
    }

    // mixed nested (aa inside bw) - should merge
    {
        SkClipStack stack;

        stack.clipRect(nestedParent, SkMatrix::I(), kReplace_SkClipOp, false);

        stack.clipRect(nestedChild, SkMatrix::I(), kIntersect_SkClipOp, true);

        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, isIntersectionOfRects);
    }

    // reverse nested (aa inside bw) - should _not_ merge
    {
        SkClipStack stack;

        stack.clipRect(nestedChild, SkMatrix::I(), kReplace_SkClipOp, false);

        stack.clipRect(nestedParent, SkMatrix::I(), kIntersect_SkClipOp, true);

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
        stack.clipRect(outsideRect, SkMatrix::I(), kDifference_SkClipOp, false);
        // return false because quickContains currently does not care for kDifference_SkClipOp
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    // Replace Op tests
    {
        SkClipStack stack;
        stack.clipRect(outsideRect, SkMatrix::I(), kReplace_SkClipOp, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipRect(insideRect, SkMatrix::I(), kIntersect_SkClipOp, false);
        stack.save(); // To prevent in-place substitution by replace OP
        stack.clipRect(outsideRect, SkMatrix::I(), kReplace_SkClipOp, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
        stack.restore();
    }

    {
        SkClipStack stack;
        stack.clipRect(outsideRect, SkMatrix::I(), kIntersect_SkClipOp, false);
        stack.save(); // To prevent in-place substitution by replace OP
        stack.clipRect(insideRect, SkMatrix::I(), kReplace_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
        stack.restore();
    }

    // Verify proper traversal of multi-element clip
    {
        SkClipStack stack;
        stack.clipRect(insideRect, SkMatrix::I(), kIntersect_SkClipOp, false);
        // Use a path for second clip to prevent in-place intersection
        stack.clipPath(outsideCircle, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    // Intersect Op tests with rectangles
    {
        SkClipStack stack;
        stack.clipRect(outsideRect, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipRect(insideRect, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipRect(intersectingRect, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipRect(nonIntersectingRect, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    // Intersect Op tests with circle paths
    {
        SkClipStack stack;
        stack.clipPath(outsideCircle, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipPath(insideCircle, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipPath(intersectingCircle, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipPath(nonIntersectingCircle, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    // Intersect Op tests with inverse filled rectangles
    {
        SkClipStack stack;
        SkPath path;
        path.addRect(outsideRect);
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path;
        path.addRect(insideRect);
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path;
        path.addRect(intersectingRect);
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path;
        path.addRect(nonIntersectingRect);
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }

    // Intersect Op tests with inverse filled circles
    {
        SkClipStack stack;
        SkPath path = outsideCircle;
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path = insideCircle;
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path = intersectingCircle;
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), kIntersect_SkClipOp, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path = nonIntersectingCircle;
        path.toggleInverseFillType();
        stack.clipPath(path, SkMatrix::I(), kIntersect_SkClipOp, false);
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
        region->op(elemRegion, (SkRegion::Op)element->getOp());
    }
}

static void test_invfill_diff_bug(skiatest::Reporter* reporter) {
    SkClipStack stack;
    stack.clipRect({10, 10, 20, 20}, SkMatrix::I(), kIntersect_SkClipOp, false);

    SkPath path;
    path.addRect({30, 10, 40, 20});
    path.setFillType(SkPath::kInverseWinding_FillType);
    stack.clipPath(path, SkMatrix::I(), kDifference_SkClipOp, false);

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

// Functions that add a shape to the clip stack. The shape is computed from a rectangle.
// AA is always disabled since the clip stack reducer can cause changes in aa rasterization of the
// stack. A fractional edge repeated in different elements may be rasterized fewer times using the
// reduced stack.
typedef void (*AddElementFunc) (const SkRect& rect,
                                bool invert,
                                SkClipOp op,
                                SkClipStack* stack,
                                bool doAA);

static void add_round_rect(const SkRect& rect, bool invert, SkClipOp op, SkClipStack* stack,
                           bool doAA) {
    SkScalar rx = rect.width() / 10;
    SkScalar ry = rect.height() / 20;
    if (invert) {
        SkPath path;
        path.addRoundRect(rect, rx, ry);
        path.setFillType(SkPath::kInverseWinding_FillType);
        stack->clipPath(path, SkMatrix::I(), op, doAA);
    } else {
        SkRRect rrect;
        rrect.setRectXY(rect, rx, ry);
        stack->clipRRect(rrect, SkMatrix::I(), op, doAA);
    }
};

static void add_rect(const SkRect& rect, bool invert, SkClipOp op, SkClipStack* stack,
                     bool doAA) {
    if (invert) {
        SkPath path;
        path.addRect(rect);
        path.setFillType(SkPath::kInverseWinding_FillType);
        stack->clipPath(path, SkMatrix::I(), op, doAA);
    } else {
        stack->clipRect(rect, SkMatrix::I(), op, doAA);
    }
};

static void add_oval(const SkRect& rect, bool invert, SkClipOp op, SkClipStack* stack,
                     bool doAA) {
    SkPath path;
    path.addOval(rect);
    if (invert) {
        path.setFillType(SkPath::kInverseWinding_FillType);
    }
    stack->clipPath(path, SkMatrix::I(), op, doAA);
};

static void add_elem_to_stack(const SkClipStack::Element& element, SkClipStack* stack) {
    switch (element.getDeviceSpaceType()) {
        case SkClipStack::Element::DeviceSpaceType::kRect:
            stack->clipRect(element.getDeviceSpaceRect(), SkMatrix::I(), element.getOp(),
                            element.isAA());
            break;
        case SkClipStack::Element::DeviceSpaceType::kRRect:
            stack->clipRRect(element.getDeviceSpaceRRect(), SkMatrix::I(), element.getOp(),
                             element.isAA());
            break;
        case SkClipStack::Element::DeviceSpaceType::kPath:
            stack->clipPath(element.getDeviceSpacePath(), SkMatrix::I(), element.getOp(),
                            element.isAA());
            break;
        case SkClipStack::Element::DeviceSpaceType::kEmpty:
            SkDEBUGFAIL("Why did the reducer produce an explicit empty.");
            stack->clipEmpty();
            break;
    }
}

static void test_reduced_clip_stack(skiatest::Reporter* reporter) {
    // We construct random clip stacks, reduce them, and then rasterize both versions to verify that
    // they are equal.

    // All the clip elements will be contained within these bounds.
    static const SkIRect kIBounds = SkIRect::MakeWH(100, 100);
    static const SkRect kBounds = SkRect::Make(kIBounds);

    enum {
        kNumTests = 250,
        kMinElemsPerTest = 1,
        kMaxElemsPerTest = 50,
    };

    // min/max size of a clip element as a fraction of kBounds.
    static const SkScalar kMinElemSizeFrac = SK_Scalar1 / 5;
    static const SkScalar kMaxElemSizeFrac = SK_Scalar1;

    static const SkClipOp kOps[] = {
        kDifference_SkClipOp,
        kIntersect_SkClipOp,
        kUnion_SkClipOp,
        kXOR_SkClipOp,
        kReverseDifference_SkClipOp,
        kReplace_SkClipOp,
    };

    // Replace operations short-circuit the optimizer. We want to make sure that we test this code
    // path a little bit but we don't want it to prevent us from testing many longer traversals in
    // the optimizer.
    static const int kReplaceDiv = 4 * kMaxElemsPerTest;

    // We want to test inverse fills. However, they are quite rare in practice so don't over do it.
    static const SkScalar kFractionInverted = SK_Scalar1 / kMaxElemsPerTest;

    static const SkScalar kFractionAntialiased = 0.25;

    static const AddElementFunc kElementFuncs[] = {
        add_rect,
        add_round_rect,
        add_oval,
    };

    SkRandom r;

    for (int i = 0; i < kNumTests; ++i) {
        SkString testCase;
        testCase.printf("Iteration %d", i);

        // Randomly generate a clip stack.
        SkClipStack stack;
        int numElems = r.nextRangeU(kMinElemsPerTest, kMaxElemsPerTest);
        bool doAA = r.nextBiasedBool(kFractionAntialiased);
        for (int e = 0; e < numElems; ++e) {
            SkClipOp op = kOps[r.nextULessThan(SK_ARRAY_COUNT(kOps))];
            if (op == kReplace_SkClipOp) {
                if (r.nextU() % kReplaceDiv) {
                    --e;
                    continue;
                }
            }

            // saves can change the clip stack behavior when an element is added.
            bool doSave = r.nextBool();

            SkSize size = SkSize::Make(
                kBounds.width()  * r.nextRangeScalar(kMinElemSizeFrac, kMaxElemSizeFrac),
                kBounds.height() * r.nextRangeScalar(kMinElemSizeFrac, kMaxElemSizeFrac));

            SkPoint xy = {r.nextRangeScalar(kBounds.fLeft, kBounds.fRight - size.fWidth),
                          r.nextRangeScalar(kBounds.fTop, kBounds.fBottom - size.fHeight)};

            SkRect rect;
            if (doAA) {
                rect.setXYWH(xy.fX, xy.fY, size.fWidth, size.fHeight);
                if (GrClip::IsPixelAligned(rect)) {
                    // Don't create an element that may accidentally become not antialiased.
                    rect.outset(0.5f, 0.5f);
                }
                SkASSERT(!GrClip::IsPixelAligned(rect));
            } else {
                rect.setXYWH(SkScalarFloorToScalar(xy.fX),
                             SkScalarFloorToScalar(xy.fY),
                             SkScalarCeilToScalar(size.fWidth),
                             SkScalarCeilToScalar(size.fHeight));
            }

            bool invert = r.nextBiasedBool(kFractionInverted);

            kElementFuncs[r.nextULessThan(SK_ARRAY_COUNT(kElementFuncs))](rect, invert, op, &stack,
                                                                          doAA);
            if (doSave) {
                stack.save();
            }
        }

        auto context = GrContext::MakeMock(nullptr);
        const GrCaps* caps = context->priv().caps();

        // Zero the memory we will new the GrReducedClip into. This ensures the elements gen ID
        // will be kInvalidGenID if left uninitialized.
        SkAlignedSTStorage<1, GrReducedClip> storage;
        memset(storage.get(), 0, sizeof(GrReducedClip));
        GR_STATIC_ASSERT(0 == SkClipStack::kInvalidGenID);

        // Get the reduced version of the stack.
        SkRect queryBounds = kBounds;
        queryBounds.outset(kBounds.width() / 2, kBounds.height() / 2);
        const GrReducedClip* reduced = new (storage.get()) GrReducedClip(stack, queryBounds, caps);

        REPORTER_ASSERT(reporter,
                        reduced->maskElements().isEmpty() ||
                                SkClipStack::kInvalidGenID != reduced->maskGenID(),
                        testCase.c_str());

        if (!reduced->maskElements().isEmpty()) {
            REPORTER_ASSERT(reporter, reduced->hasScissor(), testCase.c_str());
            SkRect stackBounds;
            SkClipStack::BoundsType stackBoundsType;
            stack.getBounds(&stackBounds, &stackBoundsType);
            REPORTER_ASSERT(reporter, reduced->maskRequiresAA() == doAA, testCase.c_str());
        }

        // Build a new clip stack based on the reduced clip elements
        SkClipStack reducedStack;
        if (GrReducedClip::InitialState::kAllOut == reduced->initialState()) {
            // whether the result is bounded or not, the whole plane should start outside the clip.
            reducedStack.clipEmpty();
        }
        for (ElementList::Iter iter(reduced->maskElements()); iter.get(); iter.next()) {
            add_elem_to_stack(*iter.get(), &reducedStack);
        }

        SkIRect scissor = reduced->hasScissor() ? reduced->scissor() : kIBounds;

        // GrReducedClipStack assumes that the final result is clipped to the returned bounds
        reducedStack.clipDevRect(scissor, kIntersect_SkClipOp);
        stack.clipDevRect(scissor, kIntersect_SkClipOp);

        // convert both the original stack and reduced stack to SkRegions and see if they're equal
        SkRegion region;
        set_region_to_stack(stack, scissor, &region);

        SkRegion reducedRegion;
        set_region_to_stack(reducedStack, scissor, &reducedRegion);

        REPORTER_ASSERT(reporter, region == reducedRegion, testCase.c_str());

        reduced->~GrReducedClip();
    }
}

#ifdef SK_BUILD_FOR_WIN
    #define SUPPRESS_VISIBILITY_WARNING
#else
    #define SUPPRESS_VISIBILITY_WARNING __attribute__((visibility("hidden")))
#endif

static void test_reduced_clip_stack_genid(skiatest::Reporter* reporter) {
    {
        SkClipStack stack;
        stack.clipRect(SkRect::MakeXYWH(0, 0, 100, 100), SkMatrix::I(), kReplace_SkClipOp,
                       true);
        stack.clipRect(SkRect::MakeXYWH(0, 0, SkScalar(50.3), SkScalar(50.3)), SkMatrix::I(),
                       kReplace_SkClipOp, true);
        SkRect bounds = SkRect::MakeXYWH(0, 0, 100, 100);

        auto context = GrContext::MakeMock(nullptr);
        const GrCaps* caps = context->priv().caps();

        SkAlignedSTStorage<1, GrReducedClip> storage;
        memset(storage.get(), 0, sizeof(GrReducedClip));
        GR_STATIC_ASSERT(0 == SkClipStack::kInvalidGenID);
        const GrReducedClip* reduced = new (storage.get()) GrReducedClip(stack, bounds, caps);

        REPORTER_ASSERT(reporter, reduced->maskElements().count() == 1);
        // Clips will be cached based on the generation id. Make sure the gen id is valid.
        REPORTER_ASSERT(reporter, SkClipStack::kInvalidGenID != reduced->maskGenID());

        reduced->~GrReducedClip();
    }
    {
        SkClipStack stack;

        // Create a clip with following 25.3, 25.3 boxes which are 25 apart:
        //  A  B
        //  C  D

        stack.clipRect(SkRect::MakeXYWH(0, 0, SkScalar(25.3), SkScalar(25.3)), SkMatrix::I(),
                       kReplace_SkClipOp, true);
        uint32_t genIDA = stack.getTopmostGenID();
        stack.clipRect(SkRect::MakeXYWH(50, 0, SkScalar(25.3), SkScalar(25.3)), SkMatrix::I(),
                       kUnion_SkClipOp, true);
        uint32_t genIDB = stack.getTopmostGenID();
        stack.clipRect(SkRect::MakeXYWH(0, 50, SkScalar(25.3), SkScalar(25.3)), SkMatrix::I(),
                       kUnion_SkClipOp, true);
        uint32_t genIDC = stack.getTopmostGenID();
        stack.clipRect(SkRect::MakeXYWH(50, 50, SkScalar(25.3), SkScalar(25.3)), SkMatrix::I(),
                       kUnion_SkClipOp, true);
        uint32_t genIDD = stack.getTopmostGenID();


#define IXYWH SkIRect::MakeXYWH
#define XYWH SkRect::MakeXYWH

        SkIRect stackBounds = IXYWH(0, 0, 76, 76);

        // The base test is to test each rect in two ways:
        // 1) The box dimensions. (Should reduce to "all in", no elements).
        // 2) A bit over the box dimensions.
        // In the case 2, test that the generation id is what is expected.
        // The rects are of fractional size so that case 2 never gets optimized to an empty element
        // list.

        // Not passing in tighter bounds is tested for consistency.
        static const struct SUPPRESS_VISIBILITY_WARNING {
            SkRect testBounds;
            int reducedClipCount;
            uint32_t reducedGenID;
            InitialState initialState;
            SkIRect clipIRect;
            // parameter.
        } testCases[] = {

            // Rect A.
            { XYWH(0, 0, 25, 25), 0, SkClipStack::kInvalidGenID, GrReducedClip::InitialState::kAllIn, IXYWH(0, 0, 25, 25) },
            { XYWH(0.1f, 0.1f, 25.1f, 25.1f), 0, SkClipStack::kInvalidGenID, GrReducedClip::InitialState::kAllIn, IXYWH(0, 0, 26, 26) },
            { XYWH(0, 0, 27, 27), 1, genIDA, GrReducedClip::InitialState::kAllOut, IXYWH(0, 0, 26, 26)},

            // Rect B.
            { XYWH(50, 0, 25, 25), 0, SkClipStack::kInvalidGenID, GrReducedClip::InitialState::kAllIn, IXYWH(50, 0, 25, 25) },
            { XYWH(50, 0, 25.3f, 25.3f), 0, SkClipStack::kInvalidGenID, GrReducedClip::InitialState::kAllIn, IXYWH(50, 0, 26, 26) },
            { XYWH(50, 0, 27, 27), 1, genIDB, GrReducedClip::InitialState::kAllOut, IXYWH(50, 0, 26, 27) },

            // Rect C.
            { XYWH(0, 50, 25, 25), 0, SkClipStack::kInvalidGenID, GrReducedClip::InitialState::kAllIn, IXYWH(0, 50, 25, 25) },
            { XYWH(0.2f, 50.1f, 25.1f, 25.2f), 0, SkClipStack::kInvalidGenID, GrReducedClip::InitialState::kAllIn, IXYWH(0, 50, 26, 26) },
            { XYWH(0, 50, 27, 27), 1, genIDC, GrReducedClip::InitialState::kAllOut, IXYWH(0, 50, 27, 26) },

            // Rect D.
            { XYWH(50, 50, 25, 25), 0, SkClipStack::kInvalidGenID, GrReducedClip::InitialState::kAllIn, IXYWH(50, 50, 25, 25)},
            { XYWH(50.3f, 50.3f, 25, 25), 0, SkClipStack::kInvalidGenID, GrReducedClip::InitialState::kAllIn, IXYWH(50, 50, 26, 26)},
            { XYWH(50, 50, 27, 27), 1, genIDD, GrReducedClip::InitialState::kAllOut,  IXYWH(50, 50, 26, 26)},

            // Other tests:
            { XYWH(0, 0, 100, 100), 4, genIDD, GrReducedClip::InitialState::kAllOut, stackBounds },

            // Rect in the middle, touches none.
            { XYWH(26, 26, 24, 24), 0, SkClipStack::kInvalidGenID, GrReducedClip::InitialState::kAllOut, IXYWH(26, 26, 24, 24) },

            // Rect in the middle, touches all the rects. GenID is the last rect.
            { XYWH(24, 24, 27, 27), 4, genIDD, GrReducedClip::InitialState::kAllOut, IXYWH(24, 24, 27, 27) },
        };

#undef XYWH
#undef IXYWH
        auto context = GrContext::MakeMock(nullptr);
        const GrCaps* caps = context->priv().caps();

        for (size_t i = 0; i < SK_ARRAY_COUNT(testCases); ++i) {
            const GrReducedClip reduced(stack, testCases[i].testBounds, caps);
            REPORTER_ASSERT(reporter, reduced.maskElements().count() ==
                            testCases[i].reducedClipCount);
            SkASSERT(reduced.maskElements().count() == testCases[i].reducedClipCount);
            if (reduced.maskElements().count()) {
                REPORTER_ASSERT(reporter, reduced.maskGenID() == testCases[i].reducedGenID);
                SkASSERT(reduced.maskGenID() == testCases[i].reducedGenID);
            }
            REPORTER_ASSERT(reporter, reduced.initialState() == testCases[i].initialState);
            SkASSERT(reduced.initialState() == testCases[i].initialState);
            REPORTER_ASSERT(reporter, reduced.hasScissor());
            SkASSERT(reduced.hasScissor());
            REPORTER_ASSERT(reporter, reduced.scissor() == testCases[i].clipIRect);
            SkASSERT(reduced.scissor() == testCases[i].clipIRect);
        }
    }
}

static void test_reduced_clip_stack_no_aa_crash(skiatest::Reporter* reporter) {
    SkClipStack stack;
    stack.clipDevRect(SkIRect::MakeXYWH(0, 0, 100, 100), kReplace_SkClipOp);
    stack.clipDevRect(SkIRect::MakeXYWH(0, 0, 50, 50), kReplace_SkClipOp);
    SkRect bounds = SkRect::MakeXYWH(0, 0, 100, 100);

    auto context = GrContext::MakeMock(nullptr);
    const GrCaps* caps = context->priv().caps();

    // At the time, this would crash.
    const GrReducedClip reduced(stack, bounds, caps);
    REPORTER_ASSERT(reporter, reduced.maskElements().isEmpty());
}

enum class ClipMethod {
    kSkipDraw,
    kIgnoreClip,
    kScissor,
    kAAElements
};

static void test_aa_query(skiatest::Reporter* reporter, const SkString& testName,
                          const SkClipStack& stack, const SkMatrix& queryXform,
                          const SkRect& preXformQuery, ClipMethod expectedMethod,
                          int numExpectedElems = 0) {
    auto context = GrContext::MakeMock(nullptr);
    const GrCaps* caps = context->priv().caps();

    SkRect queryBounds;
    queryXform.mapRect(&queryBounds, preXformQuery);
    const GrReducedClip reduced(stack, queryBounds, caps);

    SkClipStack::BoundsType stackBoundsType;
    SkRect stackBounds;
    stack.getBounds(&stackBounds, &stackBoundsType);

    switch (expectedMethod) {
        case ClipMethod::kSkipDraw:
            SkASSERT(0 == numExpectedElems);
            REPORTER_ASSERT(reporter, reduced.maskElements().isEmpty(), testName.c_str());
            REPORTER_ASSERT(reporter,
                            GrReducedClip::InitialState::kAllOut == reduced.initialState(),
                            testName.c_str());
            return;
        case ClipMethod::kIgnoreClip:
            SkASSERT(0 == numExpectedElems);
            REPORTER_ASSERT(
                    reporter,
                    !reduced.hasScissor() || GrClip::IsInsideClip(reduced.scissor(), queryBounds),
                    testName.c_str());
            REPORTER_ASSERT(reporter, reduced.maskElements().isEmpty(), testName.c_str());
            REPORTER_ASSERT(reporter,
                            GrReducedClip::InitialState::kAllIn == reduced.initialState(),
                            testName.c_str());
            return;
        case ClipMethod::kScissor: {
            SkASSERT(SkClipStack::kNormal_BoundsType == stackBoundsType);
            SkASSERT(0 == numExpectedElems);
            SkIRect expectedScissor;
            stackBounds.round(&expectedScissor);
            REPORTER_ASSERT(reporter, reduced.maskElements().isEmpty(), testName.c_str());
            REPORTER_ASSERT(reporter, reduced.hasScissor(), testName.c_str());
            REPORTER_ASSERT(reporter, expectedScissor == reduced.scissor(), testName.c_str());
            REPORTER_ASSERT(reporter,
                            GrReducedClip::InitialState::kAllIn == reduced.initialState(),
                            testName.c_str());
            return;
        }
        case ClipMethod::kAAElements: {
            SkIRect expectedClipIBounds = GrClip::GetPixelIBounds(queryBounds);
            if (SkClipStack::kNormal_BoundsType == stackBoundsType) {
                SkAssertResult(expectedClipIBounds.intersect(GrClip::GetPixelIBounds(stackBounds)));
            }
            REPORTER_ASSERT(reporter, numExpectedElems == reduced.maskElements().count(),
                            testName.c_str());
            REPORTER_ASSERT(reporter, reduced.hasScissor(), testName.c_str());
            REPORTER_ASSERT(reporter, expectedClipIBounds == reduced.scissor(), testName.c_str());
            REPORTER_ASSERT(reporter,
                            reduced.maskElements().isEmpty() || reduced.maskRequiresAA(),
                            testName.c_str());
            break;
        }
    }
}

static void test_reduced_clip_stack_aa(skiatest::Reporter* reporter) {
    constexpr SkScalar IL = 2, IT = 1, IR = 6, IB = 7;         // Pixel aligned rect.
    constexpr SkScalar L = 2.2f, T = 1.7f, R = 5.8f, B = 7.3f; // Generic rect.
    constexpr SkScalar l = 3.3f, t = 2.8f, r = 4.7f, b = 6.2f; // Small rect contained in R.

    SkRect alignedRect = {IL, IT, IR, IB};
    SkRect rect = {L, T, R, B};
    SkRect innerRect = {l, t, r, b};

    SkMatrix m;
    m.setIdentity();

    constexpr SkScalar kMinScale = 2.0001f;
    constexpr SkScalar kMaxScale = 3;
    constexpr int kNumIters = 8;

    SkString name;
    SkRandom rand;

    for (int i = 0; i < kNumIters; ++i) {
        // Pixel-aligned rect (iior=true).
        name.printf("Pixel-aligned rect test, iter %i", i);
        SkClipStack stack;
        stack.clipRect(alignedRect, SkMatrix::I(), kIntersect_SkClipOp, true);
        test_aa_query(reporter, name, stack, m, {IL, IT, IR, IB}, ClipMethod::kIgnoreClip);
        test_aa_query(reporter, name, stack, m, {IL, IT-1, IR, IT}, ClipMethod::kSkipDraw);
        test_aa_query(reporter, name, stack, m, {IL, IT-2, IR, IB}, ClipMethod::kScissor);

        // Rect (iior=true).
        name.printf("Rect test, iter %i", i);
        stack.reset();
        stack.clipRect(rect, SkMatrix::I(), kIntersect_SkClipOp, true);
        test_aa_query(reporter, name, stack, m, {L, T,  R, B}, ClipMethod::kIgnoreClip);
        test_aa_query(reporter, name, stack, m, {L-.1f, T, L, B}, ClipMethod::kSkipDraw);
        test_aa_query(reporter, name, stack, m, {L-.1f, T, L+.1f, B}, ClipMethod::kAAElements, 1);

        // Difference rect (iior=false, inside-out bounds).
        name.printf("Difference rect test, iter %i", i);
        stack.reset();
        stack.clipRect(rect, SkMatrix::I(), kDifference_SkClipOp, true);
        test_aa_query(reporter, name, stack, m, {L, T, R, B}, ClipMethod::kSkipDraw);
        test_aa_query(reporter, name, stack, m, {L, T-.1f, R, T}, ClipMethod::kIgnoreClip);
        test_aa_query(reporter, name, stack, m, {L, T-.1f, R, T+.1f}, ClipMethod::kAAElements, 1);

        // Complex clip (iior=false, normal bounds).
        name.printf("Complex clip test, iter %i", i);
        stack.reset();
        stack.clipRect(rect, SkMatrix::I(), kIntersect_SkClipOp, true);
        stack.clipRect(innerRect, SkMatrix::I(), kXOR_SkClipOp, true);
        test_aa_query(reporter, name, stack, m, {l, t, r, b}, ClipMethod::kSkipDraw);
        test_aa_query(reporter, name, stack, m, {r-.1f, t, R, b}, ClipMethod::kAAElements, 1);
        test_aa_query(reporter, name, stack, m, {r-.1f, t, R+.1f, b}, ClipMethod::kAAElements, 2);
        test_aa_query(reporter, name, stack, m, {r, t, R+.1f, b}, ClipMethod::kAAElements, 1);
        test_aa_query(reporter, name, stack, m, {r, t, R, b}, ClipMethod::kIgnoreClip);
        test_aa_query(reporter, name, stack, m, {R, T, R+.1f, B}, ClipMethod::kSkipDraw);

        // Complex clip where outer rect is pixel aligned (iior=false, normal bounds).
        name.printf("Aligned Complex clip test, iter %i", i);
        stack.reset();
        stack.clipRect(alignedRect, SkMatrix::I(), kIntersect_SkClipOp, true);
        stack.clipRect(innerRect, SkMatrix::I(), kXOR_SkClipOp, true);
        test_aa_query(reporter, name, stack, m, {l, t, r, b}, ClipMethod::kSkipDraw);
        test_aa_query(reporter, name, stack, m, {l, b-.1f, r, IB}, ClipMethod::kAAElements, 1);
        test_aa_query(reporter, name, stack, m, {l, b-.1f, r, IB+.1f}, ClipMethod::kAAElements, 1);
        test_aa_query(reporter, name, stack, m, {l, b, r, IB+.1f}, ClipMethod::kAAElements, 0);
        test_aa_query(reporter, name, stack, m, {l, b, r, IB}, ClipMethod::kIgnoreClip);
        test_aa_query(reporter, name, stack, m, {IL, IB, IR, IB+.1f}, ClipMethod::kSkipDraw);

        // Apply random transforms and try again. This ensures the clip stack reduction is hardened
        // against FP rounding error.
        SkScalar sx = rand.nextRangeScalar(kMinScale, kMaxScale);
        sx = SkScalarFloorToScalar(sx * alignedRect.width()) / alignedRect.width();
        SkScalar sy = rand.nextRangeScalar(kMinScale, kMaxScale);
        sy = SkScalarFloorToScalar(sy * alignedRect.height()) / alignedRect.height();
        SkScalar tx = SkScalarRoundToScalar(sx * alignedRect.x()) - sx * alignedRect.x();
        SkScalar ty = SkScalarRoundToScalar(sy * alignedRect.y()) - sy * alignedRect.y();

        SkMatrix xform = SkMatrix::MakeScale(sx, sy);
        xform.postTranslate(tx, ty);
        xform.mapRect(&alignedRect);
        xform.mapRect(&rect);
        xform.mapRect(&innerRect);
        m.postConcat(xform);
    }
}

static void test_tiny_query_bounds_assertion_bug(skiatest::Reporter* reporter) {
    // https://bugs.chromium.org/p/skia/issues/detail?id=5990
    const SkRect clipBounds = SkRect::MakeXYWH(1.5f, 100, 1000, 1000);

    SkClipStack rectStack;
    rectStack.clipRect(clipBounds, SkMatrix::I(), kIntersect_SkClipOp, true);

    SkPath clipPath;
    clipPath.moveTo(clipBounds.left(), clipBounds.top());
    clipPath.quadTo(clipBounds.right(), clipBounds.top(),
                    clipBounds.right(), clipBounds.bottom());
    clipPath.quadTo(clipBounds.left(), clipBounds.bottom(),
                    clipBounds.left(), clipBounds.top());
    SkClipStack pathStack;
    pathStack.clipPath(clipPath, SkMatrix::I(), kIntersect_SkClipOp, true);

    auto context = GrContext::MakeMock(nullptr);
    const GrCaps* caps = context->priv().caps();

    for (const SkClipStack& stack : {rectStack, pathStack}) {
        for (SkRect queryBounds : {SkRect::MakeXYWH(53, 60, GrClip::kBoundsTolerance, 1000),
                                   SkRect::MakeXYWH(53, 60, GrClip::kBoundsTolerance/2, 1000),
                                   SkRect::MakeXYWH(53, 160, 1000, GrClip::kBoundsTolerance),
                                   SkRect::MakeXYWH(53, 160, 1000, GrClip::kBoundsTolerance/2)}) {
            const GrReducedClip reduced(stack, queryBounds, caps);
            REPORTER_ASSERT(reporter, !reduced.hasScissor());
            REPORTER_ASSERT(reporter, reduced.maskElements().isEmpty());
            REPORTER_ASSERT(reporter,
                            GrReducedClip::InitialState::kAllOut == reduced.initialState());
        }
    }
}

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
        stack.clipDevRect(gRects[i], kIntersect_SkClipOp);
    }

    // all of the above rects should have been intersected, leaving only 1 rect
    SkClipStack::B2TIter iter(stack);
    const SkClipStack::Element* element = iter.next();
    SkRect answer;
    answer.setLTRB(25, 25, 75, 75);

    REPORTER_ASSERT(reporter, element);
    REPORTER_ASSERT(reporter,
                    SkClipStack::Element::DeviceSpaceType::kRect == element->getDeviceSpaceType());
    REPORTER_ASSERT(reporter, kIntersect_SkClipOp == element->getOp());
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

    test_reduced_clip_stack(reporter);
    test_reduced_clip_stack_genid(reporter);
    test_reduced_clip_stack_no_aa_crash(reporter);
    test_reduced_clip_stack_aa(reporter);
    test_tiny_query_bounds_assertion_bug(reporter);
    test_is_rrect_deep_rect_stack(reporter);
}

//////////////////////////////////////////////////////////////////////////////

sk_sp<GrTextureProxy> GrClipStackClip::testingOnly_createClipMask(GrContext* context) const {
    const GrReducedClip reducedClip(*fStack, SkRect::MakeWH(512, 512), 0);
    return this->createSoftwareClipMask(context, reducedClip, nullptr);
}

// Verify that clip masks are freed up when the clip state that generated them goes away.
DEF_GPUTEST_FOR_ALL_CONTEXTS(ClipMaskCache, reporter, ctxInfo) {
    // This test uses resource key tags which only function in debug builds.
#ifdef SK_DEBUG
    GrContext* context = ctxInfo.grContext();
    SkClipStack stack;

    SkPath path;
    path.addCircle(10, 10, 8);
    path.addCircle(15, 15, 8);
    path.setFillType(SkPath::kEvenOdd_FillType);

    static const char* kTag = GrClipStackClip::kMaskTestTag;
    GrResourceCache* cache = context->priv().getResourceCache();

    static constexpr int kN = 5;

    for (int i = 0; i < kN; ++i) {
        SkMatrix m;
        m.setTranslate(0.5, 0.5);
        stack.save();
        stack.clipPath(path, m, SkClipOp::kIntersect, true);
        sk_sp<GrTextureProxy> mask = GrClipStackClip(&stack).testingOnly_createClipMask(context);
        mask->instantiate(context->priv().resourceProvider());
        GrTexture* tex = mask->peekTexture();
        REPORTER_ASSERT(reporter, 0 == strcmp(tex->getUniqueKey().tag(), kTag));
        // Make sure mask isn't pinned in cache.
        mask.reset(nullptr);
        context->flush();
        REPORTER_ASSERT(reporter, i + 1 == cache->countUniqueKeysWithTag(kTag));
    }

    for (int i = 0; i < kN; ++i) {
        stack.restore();
        cache->purgeAsNeeded();
        REPORTER_ASSERT(reporter, kN - (i + 1) == cache->countUniqueKeysWithTag(kTag));
    }
#endif
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(canvas_private_clipRgn, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    const int w = 10;
    const int h = 10;
    SkImageInfo info = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info);
    SkCanvas* canvas = surf->getCanvas();
    SkRegion rgn;

    canvas->temporary_internal_getRgnClip(&rgn);
    REPORTER_ASSERT(reporter, rgn.isRect());
    REPORTER_ASSERT(reporter, rgn.getBounds() == SkIRect::MakeWH(w, h));

    canvas->save();
    canvas->clipRect(SkRect::MakeWH(5, 5), kDifference_SkClipOp);
    canvas->temporary_internal_getRgnClip(&rgn);
    REPORTER_ASSERT(reporter, rgn.isComplex());
    REPORTER_ASSERT(reporter, rgn.getBounds() == SkIRect::MakeWH(w, h));
    canvas->restore();

    canvas->save();
    canvas->clipRRect(SkRRect::MakeOval(SkRect::MakeLTRB(3, 3, 7, 7)));
    canvas->temporary_internal_getRgnClip(&rgn);
    REPORTER_ASSERT(reporter, rgn.isComplex());
    REPORTER_ASSERT(reporter, rgn.getBounds() == SkIRect::MakeLTRB(3, 3, 7, 7));
    canvas->restore();
}
