#include "Test.h"
#include "SkDeque.h"

static void assert_count(skiatest::Reporter* reporter, const SkDeque& deq, int count) {
    if (0 == count) {
        REPORTER_ASSERT(reporter, deq.empty());
        REPORTER_ASSERT(reporter, 0 == deq.count());
        REPORTER_ASSERT(reporter, sizeof(int) == deq.elemSize());
        REPORTER_ASSERT(reporter, NULL == deq.front());
        REPORTER_ASSERT(reporter, NULL == deq.back());
    } else {
        REPORTER_ASSERT(reporter, !deq.empty());
        REPORTER_ASSERT(reporter, count == deq.count());
        REPORTER_ASSERT(reporter, sizeof(int) == deq.elemSize());
        REPORTER_ASSERT(reporter, NULL != deq.front());
        REPORTER_ASSERT(reporter, NULL != deq.back());
        if (1 == count) {
            REPORTER_ASSERT(reporter, deq.back() == deq.front());
        } else {
            REPORTER_ASSERT(reporter, deq.back() != deq.front());
        }
    }
}

static void assert_f2biter(skiatest::Reporter* reporter, const SkDeque& deq,
                           int max, int min) {
    SkDeque::F2BIter iter(deq);
    void* ptr;

    int value = max;
    while ((ptr = iter.next()) != NULL) {
        REPORTER_ASSERT(reporter, value == *(int*)ptr);
        value -= 1;
    }
    REPORTER_ASSERT(reporter, value+1 == min);
}

static void TestDeque(skiatest::Reporter* reporter) {
    SkDeque deq(sizeof(int));
    int i;

    assert_count(reporter, deq, 0);
    for (i = 1; i <= 10; i++) {
        *(int*)deq.push_front() = i;
    }
    assert_count(reporter, deq, 10);
    assert_f2biter(reporter, deq, 10, 1);

    for (i = 0; i < 5; i++) {
        deq.pop_front();
    }
    assert_count(reporter, deq, 5);
    assert_f2biter(reporter, deq, 5, 1);

    for (i = 0; i < 5; i++) {
        deq.pop_front();
    }
    assert_count(reporter, deq, 0);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Deque", TestDequeClass, TestDeque)
