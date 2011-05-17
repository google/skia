#include "Test.h"
#include "SkClipStack.h"
#include "SkPath.h"
#include "SkRect.h"

static void test_assign_and_comparison(skiatest::Reporter* reporter) {
    SkClipStack s;

    // Build up a clip stack with a path, an empty clip, and a rect.
    s.save();
    SkPath p;
    p.moveTo(5, 6);
    p.lineTo(7, 8);
    p.lineTo(5, 9);
    p.close();
    s.clipDevPath(p);

    s.save();
    SkRect r = SkRect::MakeLTRB(1, 2, 3, 4);
    s.clipDevRect(r);
    r = SkRect::MakeLTRB(10, 11, 12, 13);
    s.clipDevRect(r);

    s.save();
    r = SkRect::MakeLTRB(14, 15, 16, 17);
    s.clipDevRect(r, SkRegion::kUnion_Op);

    // Test that assignment works.
    SkClipStack copy = s;
    REPORTER_ASSERT(reporter, s == copy);

    // Test that different save levels triggers not equal.
    s.restore();
    REPORTER_ASSERT(reporter, s != copy);

    // Test that an equal, but not copied version is equal.
    s.save();
    r = SkRect::MakeLTRB(14, 15, 16, 17);
    s.clipDevRect(r, SkRegion::kUnion_Op);
    REPORTER_ASSERT(reporter, s == copy);

    // Test that a different op on one level triggers not equal.
    s.restore();
    s.save();
    r = SkRect::MakeLTRB(14, 15, 16, 17);
    s.clipDevRect(r);
    REPORTER_ASSERT(reporter, s != copy);

    // Test that different state (clip type) triggers not equal.
    s.restore();
    s.save();
    SkPath rp;
    rp.addRect(r);
    s.clipDevPath(rp, SkRegion::kUnion_Op);
    REPORTER_ASSERT(reporter, s != copy);

    // Test that different rects triggers not equal.
    s.restore();
    s.save();
    r = SkRect::MakeLTRB(24, 25, 26, 27);
    s.clipDevRect(r, SkRegion::kUnion_Op);
    REPORTER_ASSERT(reporter, s != copy);

    // Sanity check
    s.restore();
    copy.restore();
    REPORTER_ASSERT(reporter, s == copy);
    s.restore();
    copy.restore();
    REPORTER_ASSERT(reporter, s == copy);

    // Test that different paths triggers not equal.
    s.restore();
    s.save();
    p.addRect(r);
    s.clipDevPath(p);
    REPORTER_ASSERT(reporter, s != copy);
}

static void assert_count(skiatest::Reporter* reporter, const SkClipStack& stack,
                         int count) {
    REPORTER_ASSERT(reporter, count == stack.getSaveCount());

    SkClipStack::B2FIter iter(stack);
    int counter = 0;
    while (iter.next()) {
        counter += 1;
    }
    REPORTER_ASSERT(reporter, count == counter);
}

static void TestClipStack(skiatest::Reporter* reporter) {
    SkClipStack stack;

    assert_count(reporter, stack, 0);

    static const SkIRect gRects[] = {
        { 0, 0, 100, 100 },
        { 25, 25, 125, 125 },
        { 0, 0, 1000, 1000 },
        { 0, 0, 75, 75 }
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRects); i++) {
        stack.clipDevRect(gRects[i]);
    }

    // all of the above rects should have been intersected, leaving only 1 rect
    SkClipStack::B2FIter iter(stack);
    const SkClipStack::B2FIter::Clip* clip = iter.next();
    SkRect answer;
    answer.iset(25, 25, 75, 75);

    REPORTER_ASSERT(reporter, clip);
    REPORTER_ASSERT(reporter, clip->fRect);
    REPORTER_ASSERT(reporter, !clip->fPath);
    REPORTER_ASSERT(reporter, SkRegion::kIntersect_Op == clip->fOp);
    REPORTER_ASSERT(reporter, *clip->fRect == answer);
    // now check that we only had one in our iterator
    REPORTER_ASSERT(reporter, !iter.next());

    stack.reset();
    assert_count(reporter, stack, 0);

    test_assign_and_comparison(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("ClipStack", TestClipStackClass, TestClipStack)
