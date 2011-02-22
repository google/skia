#include "Test.h"
#include "SkClipStack.h"

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
    const SkRect answer = { 25, 25, 75, 75 };

    REPORTER_ASSERT(reporter, clip);
    REPORTER_ASSERT(reporter, clip->fRect);
    REPORTER_ASSERT(reporter, !clip->fPath);
    REPORTER_ASSERT(reporter, SkRegion::kIntersect_Op == clip->fOp);
    REPORTER_ASSERT(reporter, *clip->fRect == answer);
    // now check that we only had one in our iterator
    REPORTER_ASSERT(reporter, !iter.next());

    stack.reset();
    assert_count(reporter, stack, 0);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("ClipStack", TestClipStackClass, TestClipStack)
