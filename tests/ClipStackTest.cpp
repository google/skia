/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#if SK_SUPPORT_GPU
    #include "GrReducedClip.h"
#endif
#include "SkClipStack.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRect.h"
#include "SkRegion.h"

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
    s.clipDevPath(p, SkRegion::kIntersect_Op, doAA);

    s.save();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());

    SkRect r = SkRect::MakeLTRB(1, 2, 3, 4);
    s.clipDevRect(r, SkRegion::kIntersect_Op, doAA);
    r = SkRect::MakeLTRB(10, 11, 12, 13);
    s.clipDevRect(r, SkRegion::kIntersect_Op, doAA);

    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());

    r = SkRect::MakeLTRB(14, 15, 16, 17);
    s.clipDevRect(r, SkRegion::kUnion_Op, doAA);

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
    s.clipDevRect(r, SkRegion::kUnion_Op, doAA);
    REPORTER_ASSERT(reporter, s == copy);

    // Test that a different op on one level triggers not equal.
    s.restore();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());
    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());
    r = SkRect::MakeLTRB(14, 15, 16, 17);
    s.clipDevRect(r, SkRegion::kIntersect_Op, doAA);
    REPORTER_ASSERT(reporter, s != copy);

    // Test that version constructed with rect-path rather than a rect is still considered equal.
    s.restore();
    s.save();
    SkPath rp;
    rp.addRect(r);
    s.clipDevPath(rp, SkRegion::kUnion_Op, doAA);
    REPORTER_ASSERT(reporter, s == copy);

    // Test that different rects triggers not equal.
    s.restore();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());
    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());

    r = SkRect::MakeLTRB(24, 25, 26, 27);
    s.clipDevRect(r, SkRegion::kUnion_Op, doAA);
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
    s.clipDevPath(p, SkRegion::kIntersect_Op, doAA);
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
        stack.clipDevRect(gRects[i], SkRegion::kUnion_Op, false);
    }

    assert_count(reporter, stack, 4);

    // bottom to top iteration
    {
        const SkClipStack::Element* element = NULL;

        SkClipStack::B2TIter iter(stack);
        int i;

        for (i = 0, element = iter.next(); element; ++i, element = iter.next()) {
            REPORTER_ASSERT(reporter, SkClipStack::Element::kRect_Type == element->getType());
            REPORTER_ASSERT(reporter, element->getRect() == gRects[i]);
        }

        SkASSERT(i == 4);
    }

    // top to bottom iteration
    {
        const SkClipStack::Element* element = NULL;

        SkClipStack::Iter iter(stack, SkClipStack::Iter::kTop_IterStart);
        int i;

        for (i = 3, element = iter.prev(); element; --i, element = iter.prev()) {
            REPORTER_ASSERT(reporter, SkClipStack::Element::kRect_Type == element->getType());
            REPORTER_ASSERT(reporter, element->getRect() == gRects[i]);
        }

        SkASSERT(i == -1);
    }

    // skipToTopmost
    {
        const SkClipStack::Element* element = NULL;

        SkClipStack::Iter iter(stack, SkClipStack::Iter::kBottom_IterStart);

        element = iter.skipToTopmost(SkRegion::kUnion_Op);
        REPORTER_ASSERT(reporter, SkClipStack::Element::kRect_Type == element->getType());
        REPORTER_ASSERT(reporter, element->getRect() == gRects[3]);
    }
}

// Exercise the SkClipStack's getConservativeBounds computation
static void test_bounds(skiatest::Reporter* reporter, SkClipStack::Element::Type primType) {
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

    static const SkRegion::Op gOps[] = {
        SkRegion::kIntersect_Op,
        SkRegion::kDifference_Op,
        SkRegion::kUnion_Op,
        SkRegion::kXOR_Op,
        SkRegion::kReverseDifference_Op
    };

    SkRect rectA, rectB;

    rectA.iset(10, 10, 50, 50);
    rectB.iset(40, 40, 80, 80);

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
    int numBitTests = SkClipStack::Element::kPath_Type == primType ? 4 : 1;
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
                case SkClipStack::Element::kEmpty_Type:
                    SkDEBUGFAIL("Don't call this with kEmpty.");
                    break;
                case SkClipStack::Element::kRect_Type:
                    stack.clipDevRect(rectA, SkRegion::kIntersect_Op, false);
                    stack.clipDevRect(rectB, gOps[op], false);
                    break;
                case SkClipStack::Element::kRRect_Type:
                    stack.clipDevRRect(rrectA, SkRegion::kIntersect_Op, false);
                    stack.clipDevRRect(rrectB, gOps[op], false);
                    break;
                case SkClipStack::Element::kPath_Type:
                    stack.clipDevPath(pathA, SkRegion::kIntersect_Op, false);
                    stack.clipDevPath(pathB, gOps[op], false);
                    break;
            }

            REPORTER_ASSERT(reporter, !stack.isWideOpen());
            REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID != stack.getTopmostGenID());

            stack.getConservativeBounds(0, 0, 100, 100, &devClipBound,
                                        &isIntersectionOfRects);

            if (SkClipStack::Element::kRect_Type == primType) {
                REPORTER_ASSERT(reporter, isIntersectionOfRects ==
                        (gOps[op] == SkRegion::kIntersect_Op));
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

    rectA.iset(10, 10, 40, 40);
    rectB.iset(50, 50, 80, 80);

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

        stack.clipDevPath(clipA, SkRegion::kReplace_Op, false);
        stack.clipDevPath(clipB, SkRegion::kUnion_Op, false);

        REPORTER_ASSERT(reporter, stack.isWideOpen());
        REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID == stack.getTopmostGenID());
    }

    // Test out union w/ a wide open clip
    {
        SkClipStack stack;

        stack.clipDevRect(rectA, SkRegion::kUnion_Op, false);

        REPORTER_ASSERT(reporter, stack.isWideOpen());
        REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID == stack.getTopmostGenID());
    }

    // Test out empty difference from a wide open clip
    {
        SkClipStack stack;

        SkRect emptyRect;
        emptyRect.setEmpty();

        stack.clipDevRect(emptyRect, SkRegion::kDifference_Op, false);

        REPORTER_ASSERT(reporter, stack.isWideOpen());
        REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID == stack.getTopmostGenID());
    }

    // Test out return to wide open
    {
        SkClipStack stack;

        stack.save();

        stack.clipDevRect(rectA, SkRegion::kReplace_Op, false);

        REPORTER_ASSERT(reporter, !stack.isWideOpen());
        REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID != stack.getTopmostGenID());

        stack.restore();

        REPORTER_ASSERT(reporter, stack.isWideOpen());
        REPORTER_ASSERT(reporter, SkClipStack::kWideOpenGenID == stack.getTopmostGenID());
    }
}

static int count(const SkClipStack& stack) {

    SkClipStack::Iter iter(stack, SkClipStack::Iter::kTop_IterStart);

    const SkClipStack::Element* element = NULL;
    int count = 0;

    for (element = iter.prev(); element; element = iter.prev(), ++count) {
        ;
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
    stack.clipDevPath(path, SkRegion::kIntersect_Op, false);

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
        stack.clipDevRect(rect, SkRegion::kReplace_Op, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.clipDevRect(rect, SkRegion::kReplace_Op, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
    }

    // Adding a new rect with the replace operator should not increase
    // the stack depth. AA replacing AA.
    {
        SkClipStack stack;
        REPORTER_ASSERT(reporter, 0 == count(stack));
        stack.clipDevRect(rect, SkRegion::kReplace_Op, true);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.clipDevRect(rect, SkRegion::kReplace_Op, true);
        REPORTER_ASSERT(reporter, 1 == count(stack));
    }

    // Adding a new rect with the replace operator should not increase
    // the stack depth. BW replacing AA replacing BW.
    {
        SkClipStack stack;
        REPORTER_ASSERT(reporter, 0 == count(stack));
        stack.clipDevRect(rect, SkRegion::kReplace_Op, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.clipDevRect(rect, SkRegion::kReplace_Op, true);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.clipDevRect(rect, SkRegion::kReplace_Op, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
    }

    // Make sure replace clip rects don't collapse too much.
    {
        SkClipStack stack;
        stack.clipDevRect(rect, SkRegion::kReplace_Op, false);
        stack.clipDevRect(rect2, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.save();
        stack.clipDevRect(rect, SkRegion::kReplace_Op, false);
        REPORTER_ASSERT(reporter, 2 == count(stack));
        stack.getBounds(&bound, &type, &isIntersectionOfRects);
        REPORTER_ASSERT(reporter, bound == rect);
        stack.restore();
        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.save();
        stack.clipDevRect(rect, SkRegion::kReplace_Op, false);
        stack.clipDevRect(rect, SkRegion::kReplace_Op, false);
        REPORTER_ASSERT(reporter, 2 == count(stack));
        stack.restore();
        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.save();
        stack.clipDevRect(rect, SkRegion::kReplace_Op, false);
        stack.clipDevRect(rect2, SkRegion::kIntersect_Op, false);
        stack.clipDevRect(rect, SkRegion::kReplace_Op, false);
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
        stack.clipDevPath(path, SkRegion::kReplace_Op, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.clipDevPath(path, SkRegion::kReplace_Op, false);
        REPORTER_ASSERT(reporter, 1 == count(stack));
    }

    // Replacing rect with path.
    {
        SkClipStack stack;
        stack.clipDevRect(rect, SkRegion::kReplace_Op, true);
        REPORTER_ASSERT(reporter, 1 == count(stack));
        stack.clipDevPath(path, SkRegion::kReplace_Op, true);
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

        stack.clipDevRect(overlapLeft, SkRegion::kReplace_Op, false);

        stack.clipDevRect(overlapRight, SkRegion::kIntersect_Op, false);

        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, isIntersectionOfRects);
    }

    // all aa overlapping - should merge
    {
        SkClipStack stack;

        stack.clipDevRect(overlapLeft, SkRegion::kReplace_Op, true);

        stack.clipDevRect(overlapRight, SkRegion::kIntersect_Op, true);

        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, isIntersectionOfRects);
    }

    // mixed overlapping - should _not_ merge
    {
        SkClipStack stack;

        stack.clipDevRect(overlapLeft, SkRegion::kReplace_Op, true);

        stack.clipDevRect(overlapRight, SkRegion::kIntersect_Op, false);

        REPORTER_ASSERT(reporter, 2 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, !isIntersectionOfRects);
    }

    // mixed nested (bw inside aa) - should merge
    {
        SkClipStack stack;

        stack.clipDevRect(nestedParent, SkRegion::kReplace_Op, true);

        stack.clipDevRect(nestedChild, SkRegion::kIntersect_Op, false);

        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, isIntersectionOfRects);
    }

    // mixed nested (aa inside bw) - should merge
    {
        SkClipStack stack;

        stack.clipDevRect(nestedParent, SkRegion::kReplace_Op, false);

        stack.clipDevRect(nestedChild, SkRegion::kIntersect_Op, true);

        REPORTER_ASSERT(reporter, 1 == count(stack));

        stack.getBounds(&bound, &type, &isIntersectionOfRects);

        REPORTER_ASSERT(reporter, isIntersectionOfRects);
    }

    // reverse nested (aa inside bw) - should _not_ merge
    {
        SkClipStack stack;

        stack.clipDevRect(nestedChild, SkRegion::kReplace_Op, false);

        stack.clipDevRect(nestedParent, SkRegion::kIntersect_Op, true);

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
        stack.clipDevRect(outsideRect, SkRegion::kDifference_Op, false);
        // return false because quickContains currently does not care for kDifference_Op
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    // Replace Op tests
    {
        SkClipStack stack;
        stack.clipDevRect(outsideRect, SkRegion::kReplace_Op, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipDevRect(insideRect, SkRegion::kIntersect_Op, false);
        stack.save(); // To prevent in-place substitution by replace OP
        stack.clipDevRect(outsideRect, SkRegion::kReplace_Op, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
        stack.restore();
    }

    {
        SkClipStack stack;
        stack.clipDevRect(outsideRect, SkRegion::kIntersect_Op, false);
        stack.save(); // To prevent in-place substitution by replace OP
        stack.clipDevRect(insideRect, SkRegion::kReplace_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
        stack.restore();
    }

    // Verify proper traversal of multi-element clip
    {
        SkClipStack stack;
        stack.clipDevRect(insideRect, SkRegion::kIntersect_Op, false);
        // Use a path for second clip to prevent in-place intersection
        stack.clipDevPath(outsideCircle, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    // Intersect Op tests with rectangles
    {
        SkClipStack stack;
        stack.clipDevRect(outsideRect, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipDevRect(insideRect, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipDevRect(intersectingRect, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipDevRect(nonIntersectingRect, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    // Intersect Op tests with circle paths
    {
        SkClipStack stack;
        stack.clipDevPath(outsideCircle, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipDevPath(insideCircle, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipDevPath(intersectingCircle, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        stack.clipDevPath(nonIntersectingCircle, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    // Intersect Op tests with inverse filled rectangles
    {
        SkClipStack stack;
        SkPath path;
        path.addRect(outsideRect);
        path.toggleInverseFillType();
        stack.clipDevPath(path, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path;
        path.addRect(insideRect);
        path.toggleInverseFillType();
        stack.clipDevPath(path, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path;
        path.addRect(intersectingRect);
        path.toggleInverseFillType();
        stack.clipDevPath(path, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path;
        path.addRect(nonIntersectingRect);
        path.toggleInverseFillType();
        stack.clipDevPath(path, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }

    // Intersect Op tests with inverse filled circles
    {
        SkClipStack stack;
        SkPath path = outsideCircle;
        path.toggleInverseFillType();
        stack.clipDevPath(path, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path = insideCircle;
        path.toggleInverseFillType();
        stack.clipDevPath(path, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path = intersectingCircle;
        path.toggleInverseFillType();
        stack.clipDevPath(path, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, false == stack.quickContains(testRect));
    }

    {
        SkClipStack stack;
        SkPath path = nonIntersectingCircle;
        path.toggleInverseFillType();
        stack.clipDevPath(path, SkRegion::kIntersect_Op, false);
        REPORTER_ASSERT(reporter, true == stack.quickContains(testRect));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
// Functions that add a shape to the clip stack. The shape is computed from a rectangle.
// AA is always disabled since the clip stack reducer can cause changes in aa rasterization of the
// stack. A fractional edge repeated in different elements may be rasterized fewer times using the
// reduced stack.
typedef void (*AddElementFunc) (const SkRect& rect,
                                bool invert,
                                SkRegion::Op op,
                                SkClipStack* stack);

static void add_round_rect(const SkRect& rect, bool invert, SkRegion::Op op, SkClipStack* stack) {
    SkScalar rx = rect.width() / 10;
    SkScalar ry = rect.height() / 20;
    if (invert) {
        SkPath path;
        path.addRoundRect(rect, rx, ry);
        path.setFillType(SkPath::kInverseWinding_FillType);
        stack->clipDevPath(path, op, false);
    } else {
        SkRRect rrect;
        rrect.setRectXY(rect, rx, ry);
        stack->clipDevRRect(rrect, op, false);
    }
};

static void add_rect(const SkRect& rect, bool invert, SkRegion::Op op, SkClipStack* stack) {
    if (invert) {
        SkPath path;
        path.addRect(rect);
        path.setFillType(SkPath::kInverseWinding_FillType);
        stack->clipDevPath(path, op, false);
    } else {
        stack->clipDevRect(rect, op, false);
    }
};

static void add_oval(const SkRect& rect, bool invert, SkRegion::Op op, SkClipStack* stack) {
    SkPath path;
    path.addOval(rect);
    if (invert) {
        path.setFillType(SkPath::kInverseWinding_FillType);
    }
    stack->clipDevPath(path, op, false);
};

static void add_elem_to_stack(const SkClipStack::Element& element, SkClipStack* stack) {
    switch (element.getType()) {
        case SkClipStack::Element::kRect_Type:
            stack->clipDevRect(element.getRect(), element.getOp(), element.isAA());
            break;
        case SkClipStack::Element::kRRect_Type:
            stack->clipDevRRect(element.getRRect(), element.getOp(), element.isAA());
            break;
        case SkClipStack::Element::kPath_Type:
            stack->clipDevPath(element.getPath(), element.getOp(), element.isAA());
            break;
        case SkClipStack::Element::kEmpty_Type:
            SkDEBUGFAIL("Why did the reducer produce an explicit empty.");
            stack->clipEmpty();
            break;
    }
}

static void add_elem_to_region(const SkClipStack::Element& element,
                               const SkIRect& bounds,
                               SkRegion* region) {
    SkRegion elemRegion;
    SkRegion boundsRgn(bounds);
    SkPath path;

    switch (element.getType()) {
        case SkClipStack::Element::kEmpty_Type:
            elemRegion.setEmpty();
            break;
        default:
            element.asPath(&path);
            elemRegion.setPath(path, boundsRgn);
            break;
    }
    region->op(elemRegion, element.getOp());
}

static void test_reduced_clip_stack(skiatest::Reporter* reporter) {
    // We construct random clip stacks, reduce them, and then rasterize both versions to verify that
    // they are equal.

    // All the clip elements will be contained within these bounds.
    static const SkRect kBounds = SkRect::MakeWH(100, 100);

    enum {
        kNumTests = 200,
        kMinElemsPerTest = 1,
        kMaxElemsPerTest = 50,
    };

    // min/max size of a clip element as a fraction of kBounds.
    static const SkScalar kMinElemSizeFrac = SK_Scalar1 / 5;
    static const SkScalar kMaxElemSizeFrac = SK_Scalar1;

    static const SkRegion::Op kOps[] = {
        SkRegion::kDifference_Op,
        SkRegion::kIntersect_Op,
        SkRegion::kUnion_Op,
        SkRegion::kXOR_Op,
        SkRegion::kReverseDifference_Op,
        SkRegion::kReplace_Op,
    };

    // Replace operations short-circuit the optimizer. We want to make sure that we test this code
    // path a little bit but we don't want it to prevent us from testing many longer traversals in
    // the optimizer.
    static const int kReplaceDiv = 4 * kMaxElemsPerTest;

    // We want to test inverse fills. However, they are quite rare in practice so don't over do it.
    static const SkScalar kFractionInverted = SK_Scalar1 / kMaxElemsPerTest;

    static const AddElementFunc kElementFuncs[] = {
        add_rect,
        add_round_rect,
        add_oval,
    };

    SkRandom r;

    for (int i = 0; i < kNumTests; ++i) {
        // Randomly generate a clip stack.
        SkClipStack stack;
        int numElems = r.nextRangeU(kMinElemsPerTest, kMaxElemsPerTest);
        for (int e = 0; e < numElems; ++e) {
            SkRegion::Op op = kOps[r.nextULessThan(SK_ARRAY_COUNT(kOps))];
            if (op == SkRegion::kReplace_Op) {
                if (r.nextU() % kReplaceDiv) {
                    --e;
                    continue;
                }
            }

            // saves can change the clip stack behavior when an element is added.
            bool doSave = r.nextBool();

            SkSize size = SkSize::Make(
                SkScalarFloorToScalar(SkScalarMul(kBounds.width(), r.nextRangeScalar(kMinElemSizeFrac, kMaxElemSizeFrac))),
                SkScalarFloorToScalar(SkScalarMul(kBounds.height(), r.nextRangeScalar(kMinElemSizeFrac, kMaxElemSizeFrac))));

            SkPoint xy = {SkScalarFloorToScalar(r.nextRangeScalar(kBounds.fLeft, kBounds.fRight - size.fWidth)),
                          SkScalarFloorToScalar(r.nextRangeScalar(kBounds.fTop, kBounds.fBottom - size.fHeight))};

            SkRect rect = SkRect::MakeXYWH(xy.fX, xy.fY, size.fWidth, size.fHeight);

            bool invert = r.nextBiasedBool(kFractionInverted);

            kElementFuncs[r.nextULessThan(SK_ARRAY_COUNT(kElementFuncs))](rect, invert, op, &stack);
            if (doSave) {
                stack.save();
            }
        }

        SkRect inflatedBounds = kBounds;
        inflatedBounds.outset(kBounds.width() / 2, kBounds.height() / 2);
        SkIRect inflatedIBounds;
        inflatedBounds.roundOut(&inflatedIBounds);

        typedef GrReducedClip::ElementList ElementList;
        // Get the reduced version of the stack.
        ElementList reducedClips;
        int32_t reducedGenID;
        GrReducedClip::InitialState initial;
        SkIRect tBounds(inflatedIBounds);
        SkIRect* tightBounds = r.nextBool() ? &tBounds : NULL;
        GrReducedClip::ReduceClipStack(stack,
                                       inflatedIBounds,
                                       &reducedClips,
                                       &reducedGenID,
                                       &initial,
                                       tightBounds);

        REPORTER_ASSERT(reporter, SkClipStack::kInvalidGenID != reducedGenID);

        // Build a new clip stack based on the reduced clip elements
        SkClipStack reducedStack;
        if (GrReducedClip::kAllOut_InitialState == initial) {
            // whether the result is bounded or not, the whole plane should start outside the clip.
            reducedStack.clipEmpty();
        }
        for (ElementList::Iter iter = reducedClips.headIter(); iter.get(); iter.next()) {
            add_elem_to_stack(*iter.get(), &reducedStack);
        }

        // GrReducedClipStack assumes that the final result is clipped to the returned bounds
        if (tightBounds) {
            reducedStack.clipDevRect(*tightBounds, SkRegion::kIntersect_Op);
        }

        // convert both the original stack and reduced stack to SkRegions and see if they're equal
        SkRegion region;
        SkRegion reducedRegion;

        region.setRect(inflatedIBounds);
        const SkClipStack::Element* element;
        SkClipStack::Iter iter(stack, SkClipStack::Iter::kBottom_IterStart);
        while ((element = iter.next())) {
            add_elem_to_region(*element, inflatedIBounds, &region);
        }

        reducedRegion.setRect(inflatedIBounds);
        iter.reset(reducedStack, SkClipStack::Iter::kBottom_IterStart);
        while ((element = iter.next())) {
            add_elem_to_region(*element, inflatedIBounds, &reducedRegion);
        }
        SkString testCase;
        testCase.printf("Iteration %d", i);
        REPORTER_ASSERT_MESSAGE(reporter, region == reducedRegion, testCase.c_str());
    }
}

#if defined(WIN32)
    #define SUPPRESS_VISIBILITY_WARNING
#else
    #define SUPPRESS_VISIBILITY_WARNING __attribute__((visibility("hidden")))
#endif

static void test_reduced_clip_stack_genid(skiatest::Reporter* reporter) {
    {
        SkClipStack stack;
        stack.clipDevRect(SkRect::MakeXYWH(0, 0, 100, 100), SkRegion::kReplace_Op, true);
        stack.clipDevRect(SkRect::MakeXYWH(0, 0, SkScalar(50.3), SkScalar(50.3)), SkRegion::kReplace_Op, true);
        SkIRect inflatedIBounds = SkIRect::MakeXYWH(0, 0, 100, 100);

        GrReducedClip::ElementList reducedClips;
        int32_t reducedGenID;
        GrReducedClip::InitialState initial;
        SkIRect tightBounds;

        GrReducedClip::ReduceClipStack(stack,
                                       inflatedIBounds,
                                       &reducedClips,
                                       &reducedGenID,
                                       &initial,
                                       &tightBounds);

        REPORTER_ASSERT(reporter, reducedClips.count() == 1);
        // Clips will be cached based on the generation id. Make sure the gen id is valid.
        REPORTER_ASSERT(reporter, SkClipStack::kInvalidGenID != reducedGenID);
    }
    {
        SkClipStack stack;

        // Create a clip with following 25.3, 25.3 boxes which are 25 apart:
        //  A  B
        //  C  D

        stack.clipDevRect(SkRect::MakeXYWH(0, 0, SkScalar(25.3), SkScalar(25.3)), SkRegion::kReplace_Op, true);
        int32_t genIDA = stack.getTopmostGenID();
        stack.clipDevRect(SkRect::MakeXYWH(50, 0, SkScalar(25.3), SkScalar(25.3)), SkRegion::kUnion_Op, true);
        int32_t genIDB = stack.getTopmostGenID();
        stack.clipDevRect(SkRect::MakeXYWH(0, 50, SkScalar(25.3), SkScalar(25.3)), SkRegion::kUnion_Op, true);
        int32_t genIDC = stack.getTopmostGenID();
        stack.clipDevRect(SkRect::MakeXYWH(50, 50, SkScalar(25.3), SkScalar(25.3)), SkRegion::kUnion_Op, true);
        int32_t genIDD = stack.getTopmostGenID();


#define XYWH SkIRect::MakeXYWH

        SkIRect unused;
        unused.setEmpty();
        SkIRect stackBounds = XYWH(0, 0, 76, 76);

        // The base test is to test each rect in two ways:
        // 1) The box dimensions. (Should reduce to "all in", no elements).
        // 2) A bit over the box dimensions.
        // In the case 2, test that the generation id is what is expected.
        // The rects are of fractional size so that case 2 never gets optimized to an empty element
        // list.

        // Not passing in tighter bounds is tested for consistency.
        static const struct SUPPRESS_VISIBILITY_WARNING {
            SkIRect testBounds;
            int reducedClipCount;
            int32_t reducedGenID;
            GrReducedClip::InitialState initialState;
            SkIRect tighterBounds; // If this is empty, the query will not pass tighter bounds
            // parameter.
        } testCases[] = {
            // Rect A.
            { XYWH(0, 0, 25, 25), 0, SkClipStack::kWideOpenGenID, GrReducedClip::kAllIn_InitialState, XYWH(0, 0, 25, 25) },
            { XYWH(0, 0, 25, 25), 0, SkClipStack::kWideOpenGenID, GrReducedClip::kAllIn_InitialState, unused },
            { XYWH(0, 0, 27, 27), 1, genIDA, GrReducedClip::kAllOut_InitialState, XYWH(0, 0, 27, 27)},
            { XYWH(0, 0, 27, 27), 1, genIDA, GrReducedClip::kAllOut_InitialState, unused },

            // Rect B.
            { XYWH(50, 0, 25, 25), 0, SkClipStack::kWideOpenGenID, GrReducedClip::kAllIn_InitialState, XYWH(50, 0, 25, 25) },
            { XYWH(50, 0, 25, 25), 0, SkClipStack::kWideOpenGenID, GrReducedClip::kAllIn_InitialState, unused },
            { XYWH(50, 0, 27, 27), 1, genIDB, GrReducedClip::kAllOut_InitialState, XYWH(50, 0, 26, 27) },
            { XYWH(50, 0, 27, 27), 1, genIDB, GrReducedClip::kAllOut_InitialState, unused },

            // Rect C.
            { XYWH(0, 50, 25, 25), 0, SkClipStack::kWideOpenGenID, GrReducedClip::kAllIn_InitialState, XYWH(0, 50, 25, 25) },
            { XYWH(0, 50, 25, 25), 0, SkClipStack::kWideOpenGenID, GrReducedClip::kAllIn_InitialState, unused },
            { XYWH(0, 50, 27, 27), 1, genIDC, GrReducedClip::kAllOut_InitialState, XYWH(0, 50, 27, 26) },
            { XYWH(0, 50, 27, 27), 1, genIDC, GrReducedClip::kAllOut_InitialState, unused },

            // Rect D.
            { XYWH(50, 50, 25, 25), 0, SkClipStack::kWideOpenGenID, GrReducedClip::kAllIn_InitialState, unused },
            { XYWH(50, 50, 25, 25), 0, SkClipStack::kWideOpenGenID, GrReducedClip::kAllIn_InitialState, XYWH(50, 50, 25, 25)},
            { XYWH(50, 50, 27, 27), 1, genIDD, GrReducedClip::kAllOut_InitialState, unused },
            { XYWH(50, 50, 27, 27), 1, genIDD, GrReducedClip::kAllOut_InitialState,  XYWH(50, 50, 26, 26)},

            // Other tests:
            { XYWH(0, 0, 100, 100), 4, genIDD, GrReducedClip::kAllOut_InitialState, unused },
            { XYWH(0, 0, 100, 100), 4, genIDD, GrReducedClip::kAllOut_InitialState, stackBounds },

            // Rect in the middle, touches none.
            { XYWH(26, 26, 24, 24), 0, SkClipStack::kEmptyGenID, GrReducedClip::kAllOut_InitialState, unused },
            { XYWH(26, 26, 24, 24), 0, SkClipStack::kEmptyGenID, GrReducedClip::kAllOut_InitialState, XYWH(26, 26, 24, 24) },

            // Rect in the middle, touches all the rects. GenID is the last rect.
            { XYWH(24, 24, 27, 27), 4, genIDD, GrReducedClip::kAllOut_InitialState, unused },
            { XYWH(24, 24, 27, 27), 4, genIDD, GrReducedClip::kAllOut_InitialState, XYWH(24, 24, 27, 27) },
        };

#undef XYWH

        for (size_t i = 0; i < SK_ARRAY_COUNT(testCases); ++i) {
            GrReducedClip::ElementList reducedClips;
            int32_t reducedGenID;
            GrReducedClip::InitialState initial;
            SkIRect tightBounds;

            GrReducedClip::ReduceClipStack(stack,
                                           testCases[i].testBounds,
                                           &reducedClips,
                                           &reducedGenID,
                                           &initial,
                                           testCases[i].tighterBounds.isEmpty() ? NULL : &tightBounds);

            REPORTER_ASSERT(reporter, reducedClips.count() == testCases[i].reducedClipCount);
            SkASSERT(reducedClips.count() == testCases[i].reducedClipCount);
            REPORTER_ASSERT(reporter, reducedGenID == testCases[i].reducedGenID);
            SkASSERT(reducedGenID == testCases[i].reducedGenID);
            REPORTER_ASSERT(reporter, initial == testCases[i].initialState);
            SkASSERT(initial == testCases[i].initialState);
            if (!testCases[i].tighterBounds.isEmpty()) {
                REPORTER_ASSERT(reporter, tightBounds == testCases[i].tighterBounds);
                SkASSERT(tightBounds == testCases[i].tighterBounds);
            }
        }
    }
}

static void test_reduced_clip_stack_no_aa_crash(skiatest::Reporter* reporter) {
    SkClipStack stack;
    stack.clipDevRect(SkIRect::MakeXYWH(0, 0, 100, 100), SkRegion::kReplace_Op);
    stack.clipDevRect(SkIRect::MakeXYWH(0, 0, 50, 50), SkRegion::kReplace_Op);
    SkIRect inflatedIBounds = SkIRect::MakeXYWH(0, 0, 100, 100);

    GrReducedClip::ElementList reducedClips;
    int32_t reducedGenID;
    GrReducedClip::InitialState initial;
    SkIRect tightBounds;

    // At the time, this would crash.
    GrReducedClip::ReduceClipStack(stack,
                                   inflatedIBounds,
                                   &reducedClips,
                                   &reducedGenID,
                                   &initial,
                                   &tightBounds);

    REPORTER_ASSERT(reporter, 0 == reducedClips.count());
}

#endif

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
        stack.clipDevRect(gRects[i], SkRegion::kIntersect_Op);
    }

    // all of the above rects should have been intersected, leaving only 1 rect
    SkClipStack::B2TIter iter(stack);
    const SkClipStack::Element* element = iter.next();
    SkRect answer;
    answer.iset(25, 25, 75, 75);

    REPORTER_ASSERT(reporter, element);
    REPORTER_ASSERT(reporter, SkClipStack::Element::kRect_Type == element->getType());
    REPORTER_ASSERT(reporter, SkRegion::kIntersect_Op == element->getOp());
    REPORTER_ASSERT(reporter, element->getRect() == answer);
    // now check that we only had one in our iterator
    REPORTER_ASSERT(reporter, !iter.next());

    stack.reset();
    REPORTER_ASSERT(reporter, 0 == stack.getSaveCount());
    assert_count(reporter, stack, 0);

    test_assign_and_comparison(reporter);
    test_iterators(reporter);
    test_bounds(reporter, SkClipStack::Element::kRect_Type);
    test_bounds(reporter, SkClipStack::Element::kRRect_Type);
    test_bounds(reporter, SkClipStack::Element::kPath_Type);
    test_isWideOpen(reporter);
    test_rect_merging(reporter);
    test_rect_replace(reporter);
    test_rect_inverse_fill(reporter);
    test_path_replace(reporter);
    test_quickContains(reporter);
#if SK_SUPPORT_GPU
    test_reduced_clip_stack(reporter);
    test_reduced_clip_stack_genid(reporter);
    test_reduced_clip_stack_no_aa_crash(reporter);
#endif
}
