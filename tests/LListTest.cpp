/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkRandom.h"
#include "SkTInternalLList.h"
#include "SkTLList.h"

class ListElement {
public:
    ListElement(int id) : fID(id) {
    }
    bool operator== (const ListElement& other) { return fID == other.fID; }

#ifdef SK_ENABLE_INST_COUNT
    // Make the instance count available publicly.
    static int InstanceCount() { return GetInstanceCount(); }
#endif

    int fID;

private:
    SK_DECLARE_INST_COUNT_ROOT(ListElement);
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(ListElement);
};

SK_DEFINE_INST_COUNT(ListElement);

static void check_list(const SkTInternalLList<ListElement>& list,
                       skiatest::Reporter* reporter,
                       bool empty,
                       int numElements,
                       bool in0, bool in1, bool in2, bool in3,
                       ListElement elements[4]) {

    REPORTER_ASSERT(reporter, empty == list.isEmpty());
#if SK_DEBUG
    REPORTER_ASSERT(reporter, numElements == list.countEntries());
    REPORTER_ASSERT(reporter, in0 == list.isInList(&elements[0]));
    REPORTER_ASSERT(reporter, in1 == list.isInList(&elements[1]));
    REPORTER_ASSERT(reporter, in2 == list.isInList(&elements[2]));
    REPORTER_ASSERT(reporter, in3 == list.isInList(&elements[3]));
#endif
}

static void TestTInternalLList(skiatest::Reporter* reporter) {
    SkTInternalLList<ListElement> list;
    ListElement elements[4] = {
        ListElement(0),
        ListElement(1),
        ListElement(2),
        ListElement(3),
    };

    // list should be empty to start with
    check_list(list, reporter, true, 0, false, false, false, false, elements);

    list.addToHead(&elements[0]);

    check_list(list, reporter, false, 1, true, false, false, false, elements);

    list.addToHead(&elements[1]);
    list.addToHead(&elements[2]);
    list.addToHead(&elements[3]);

    check_list(list, reporter, false, 4, true, true, true, true, elements);

    // test out iterators
    typedef SkTInternalLList<ListElement>::Iter Iter;
    Iter iter;

    ListElement* cur = iter.init(list, Iter::kHead_IterStart);
    for (int i = 0; NULL != cur; ++i, cur = iter.next()) {
        REPORTER_ASSERT(reporter, cur->fID == 3-i);
    }

    cur = iter.init(list, Iter::kTail_IterStart);
    for (int i = 0; NULL != cur; ++i, cur = iter.prev()) {
        REPORTER_ASSERT(reporter, cur->fID == i);
    }

    // remove middle, frontmost then backmost
    list.remove(&elements[1]);
    list.remove(&elements[3]);
    list.remove(&elements[0]);

    check_list(list, reporter, false, 1, false, false, true, false, elements);

    // remove last element
    list.remove(&elements[2]);

    // list should be empty again
    check_list(list, reporter, true, 0, false, false, false, false, elements);
}

static void TestTLList(skiatest::Reporter* reporter) {
    typedef SkTLList<ListElement> ElList;
    typedef ElList::Iter Iter;
    SkRandom random;

    for (int i = 1; i <= 16; i *= 2) {

        ElList list1(i);
        ElList list2(i);
        Iter iter1;
        Iter iter2;
        Iter iter3;
        Iter iter4;

#ifdef SK_ENABLE_INST_COUNT
        SkASSERT(0 == ListElement::InstanceCount());
#endif

        REPORTER_ASSERT(reporter, list1.isEmpty());
        REPORTER_ASSERT(reporter, NULL == iter1.init(list1, Iter::kHead_IterStart));
        REPORTER_ASSERT(reporter, NULL == iter1.init(list1, Iter::kTail_IterStart));
        // Try popping an empty list
        list1.popHead();
        list1.popTail();
        REPORTER_ASSERT(reporter, list1.isEmpty());
        REPORTER_ASSERT(reporter, list1 == list2);

        // Create two identical lists, one by appending to head and the other to the tail.
        list1.addToHead(ListElement(1));
        list2.addToTail(ListElement(1));
#ifdef SK_ENABLE_INST_COUNT
        SkASSERT(2 == ListElement::InstanceCount());
#endif
        iter1.init(list1, Iter::kHead_IterStart);
        iter2.init(list1, Iter::kTail_IterStart);
        REPORTER_ASSERT(reporter, iter1.get()->fID == iter2.get()->fID);
        iter3.init(list2, Iter::kHead_IterStart);
        iter4.init(list2, Iter::kTail_IterStart);
        REPORTER_ASSERT(reporter, iter3.get()->fID == iter1.get()->fID);
        REPORTER_ASSERT(reporter, iter4.get()->fID == iter1.get()->fID);
        REPORTER_ASSERT(reporter, list1 == list2);
    
        // add an element to the second list, check that iters are still valid
        list2.addToHead(ListElement(2));
#ifdef SK_ENABLE_INST_COUNT
        SkASSERT(3 == ListElement::InstanceCount());
#endif
        REPORTER_ASSERT(reporter, iter3.get()->fID == iter1.get()->fID);
        REPORTER_ASSERT(reporter, iter4.get()->fID == iter1.get()->fID);
        REPORTER_ASSERT(reporter, 1 == Iter(list2, Iter::kTail_IterStart).get()->fID);
        REPORTER_ASSERT(reporter, 2 == Iter(list2, Iter::kHead_IterStart).get()->fID);
        REPORTER_ASSERT(reporter, list1 != list2);
        list1.addToHead(ListElement(2));
        REPORTER_ASSERT(reporter, list1 == list2);
#ifdef SK_ENABLE_INST_COUNT
        SkASSERT(4 == ListElement::InstanceCount());
#endif
        REPORTER_ASSERT(reporter, !list1.isEmpty());

        list1.reset();
        list2.reset();
#ifdef SK_ENABLE_INST_COUNT
        SkASSERT(0 == ListElement::InstanceCount());
#endif
        REPORTER_ASSERT(reporter, list1.isEmpty() && list2.isEmpty());

        int count = 0;
        for (int j = 0; j < 100; ++j) {
            if (list1.isEmpty() || random.nextBiasedBool(3  * SK_Scalar1 / 4)) {
                int id = static_cast<int>(random.nextU());
                if (random.nextBool()) {
                    list1.addToHead(ListElement(id));
                } else {
                    list1.addToTail(ListElement(id));
                }
                ++count;
            } else {
                // walk to a random place either forward or backwards and remove.
                int n = random.nextULessThan(list1.count());
                Iter::IterStart start;
                ListElement* (Iter::*incrFunc)();
                
                if (random.nextBool()) {
                    start = Iter::kHead_IterStart;
                    incrFunc = &Iter::next;
                } else {
                    start = Iter::kTail_IterStart;
                    incrFunc = &Iter::prev;
                }

                // find the element
                Iter iter(list1, start);
                while (n--) {
                    REPORTER_ASSERT(reporter, NULL != iter.get());
                    (iter.*incrFunc)();
                }
                REPORTER_ASSERT(reporter, NULL != iter.get());

                // remember the prev and next elements from the element to be removed
                Iter prev = iter;
                Iter next = iter;
                prev.prev();
                next.next();
                list1.remove(iter.get());

                // make sure the remembered next/prev iters still work
                Iter pn = prev; pn.next();
                Iter np = next; np.prev();
                // pn should match next unless the target node was the head, in which case prev
                // walked off the list.
                REPORTER_ASSERT(reporter, pn.get() == next.get() || NULL == prev.get());
                // Similarly, np should match prev unless next originally walked off the tail.
                REPORTER_ASSERT(reporter, np.get() == prev.get() || NULL == next.get());
                --count;
            }
            REPORTER_ASSERT(reporter, count == list1.count());
#ifdef SK_ENABLE_INST_COUNT
            SkASSERT(count == ListElement::InstanceCount());
#endif
        }
        list1.reset();
#ifdef SK_ENABLE_INST_COUNT
        SkASSERT(0 == ListElement::InstanceCount());
#endif
    }
}

static void test_llists(skiatest::Reporter* reporter) {
    TestTInternalLList(reporter);
    TestTLList(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("LList", TestLListClass, test_llists)
