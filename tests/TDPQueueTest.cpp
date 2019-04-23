/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkRandom.h"
#include "src/core/SkTDPQueue.h"
#include "tests/Test.h"

namespace { bool intless(const int& a, const int& b) { return a < b; } }

static void simple_test(skiatest::Reporter* reporter) {
    SkTDPQueue<int, intless> heap;
    REPORTER_ASSERT(reporter, 0 == heap.count());

    heap.insert(0);
    REPORTER_ASSERT(reporter, 1 == heap.count());
    REPORTER_ASSERT(reporter, 0 == heap.peek());
    heap.pop();
    REPORTER_ASSERT(reporter, 0 == heap.count());

    heap.insert(0);
    heap.insert(1);
    REPORTER_ASSERT(reporter, 2 == heap.count());
    REPORTER_ASSERT(reporter, 0 == heap.peek());
    heap.pop();
    REPORTER_ASSERT(reporter, 1 == heap.count());
    REPORTER_ASSERT(reporter, 1 == heap.peek());
    heap.pop();
    REPORTER_ASSERT(reporter, 0 == heap.count());

    heap.insert(2);
    heap.insert(1);
    heap.insert(0);
    REPORTER_ASSERT(reporter, 3 == heap.count());
    REPORTER_ASSERT(reporter, 0 == heap.peek());
    heap.pop();
    REPORTER_ASSERT(reporter, 2 == heap.count());
    REPORTER_ASSERT(reporter, 1 == heap.peek());
    heap.pop();
    REPORTER_ASSERT(reporter, 1 == heap.count());
    REPORTER_ASSERT(reporter, 2 == heap.peek());
    heap.pop();
    REPORTER_ASSERT(reporter, 0 == heap.count());

    heap.insert(2);
    heap.insert(3);
    heap.insert(0);
    heap.insert(1);
    REPORTER_ASSERT(reporter, 4 == heap.count());
    REPORTER_ASSERT(reporter, 0 == heap.peek());
    heap.pop();
    REPORTER_ASSERT(reporter, 3 == heap.count());
    REPORTER_ASSERT(reporter, 1 == heap.peek());
    heap.pop();
    REPORTER_ASSERT(reporter, 2 == heap.count());
    REPORTER_ASSERT(reporter, 2 == heap.peek());
    heap.pop();
    REPORTER_ASSERT(reporter, 1 == heap.count());
    REPORTER_ASSERT(reporter, 3 == heap.peek());
    heap.pop();
    REPORTER_ASSERT(reporter, 0 == heap.count());
}

struct Dummy {
    int fValue;
    int fPriority;
    mutable int fIndex;

    static bool LessP(Dummy* const& a, Dummy* const& b) { return a->fPriority < b->fPriority; }
    static int* PQIndex(Dummy* const& dummy) { return &dummy->fIndex; }

    bool operator== (const Dummy& that) const {
        return fValue == that.fValue && fPriority == that.fPriority;
    }
    bool operator!= (const Dummy& that) const { return !(*this == that); }
};

void random_test(skiatest::Reporter* reporter) {
    SkRandom random;
    static const Dummy kSentinel = {-1, -1, -1};

    for (int i = 0; i < 100; ++i) {
        // Create a random set of Dummy objects.
        int count = random.nextULessThan(100);
        SkTDArray<Dummy> array;
        array.setReserve(count);
        for (int j = 0; j < count; ++j) {
            Dummy* dummy = array.append();
            dummy->fPriority = random.nextS();
            dummy->fValue = random.nextS();
            dummy->fIndex = -1;
            if (*dummy == kSentinel) {
                array.pop();
                --j;
            }
        }

        // Stick the dummy objects in the pqueue.
        SkTDPQueue<Dummy*, Dummy::LessP, Dummy::PQIndex> pq;
        for (int j = 0; j < count; ++j) {
            pq.insert(&array[j]);
        }
        REPORTER_ASSERT(reporter, pq.count() == array.count());
        for (int j = 0; j < count; ++j) {
            // every item should have an entry in the queue.
            REPORTER_ASSERT(reporter, -1 != array[j].fIndex);
        }

        // Begin the test.
        while (pq.count()) {
            // Make sure the top of the queue is really the highest priority.
            Dummy* top = pq.peek();
            for (int k = 0; k < count; ++k) {
                REPORTER_ASSERT(reporter, kSentinel == array[k] ||
                                            array[k].fPriority >= top->fPriority);
            }
            // Do one of three random actions:
            unsigned action = random.nextULessThan(3);
            switch (action) {
                case 0: { // pop the top,
                    Dummy* top = pq.peek();
                    REPORTER_ASSERT(reporter, array.begin() <= top && top < array.end());
                    pq.pop();
                    *top = kSentinel;
                    break;
                }
                case 1: { // remove a random element,
                    int item;
                    do {
                        item = random.nextULessThan(count);
                    } while (array[item] == kSentinel);
                    pq.remove(&array[item]);
                    array[item] = kSentinel;
                    break;
                }
                case 2: { // or change an element's priority.
                    int item;
                    do {
                        item = random.nextULessThan(count);
                    } while (array[item] == kSentinel);
                    array[item].fPriority = random.nextS();
                    pq.priorityDidChange(&array[item]);
                    break;
                }
            }
        }
   }
}

void sort_test(skiatest::Reporter* reporter) {
    SkRandom random;

    SkTDPQueue<Dummy *, Dummy::LessP, Dummy::PQIndex> pqTest;
    SkTDPQueue<Dummy *, Dummy::LessP, Dummy::PQIndex> pqControl;

    // Create a random set of Dummy objects and populate the test queue.
    int count = random.nextULessThan(100);
    SkTDArray<Dummy> testArray;
    testArray.setReserve(count);
    for (int i = 0; i < count; i++) {
        Dummy *dummy = testArray.append();
        dummy->fPriority = random.nextS();
        dummy->fValue = random.nextS();
        dummy->fIndex = -1;
        pqTest.insert(&testArray[i]);
    }

    // Stick equivalent dummy objects into the control queue.
    SkTDArray<Dummy> controlArray;
    controlArray.setReserve(count);
    for (int i = 0; i < count; i++) {
        Dummy *dummy = controlArray.append();
        dummy->fPriority = testArray[i].fPriority;
        dummy->fValue = testArray[i].fValue;
        dummy->fIndex = -1;
        pqControl.insert(&controlArray[i]);
    }

    // Sort the queue
    pqTest.sort();

    // Compare elements in the queue to ensure they are in sorted order
    int prevPriority = pqTest.peek()->fPriority;
    for (int i = 0; i < count; i++) {
        REPORTER_ASSERT(reporter, i <= pqTest.at(i)->fIndex);
        REPORTER_ASSERT(reporter, prevPriority <= pqTest.at(i)->fPriority);
        prevPriority = pqTest.at(i)->fPriority;
    }

    // Verify that after sorting the queue still produces the same result as the control queue
    for (int i = 0; i < count; i++) {
        REPORTER_ASSERT(reporter, *pqControl.peek() == *pqTest.peek());
        pqControl.pop();
        pqTest.pop();
    }
}

DEF_TEST(TDPQueueTest, reporter) {
    simple_test(reporter);
    random_test(reporter);
    sort_test(reporter);
}
