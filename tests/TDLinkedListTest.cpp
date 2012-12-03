/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkTInternalLList.h"

class ListElement {
public:
    ListElement(int id) : fID(id) {
    }

    int fID;

private:
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(ListElement);
};

static void CheckList(const SkTInternalLList<ListElement>& list,
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

static void TestTDLinkedList(skiatest::Reporter* reporter) {
    SkTInternalLList<ListElement> list;
    ListElement elements[4] = {
        ListElement(0),
        ListElement(1),
        ListElement(2),
        ListElement(3),
    };

    // list should be empty to start with
    CheckList(list, reporter, true, 0, false, false, false, false, elements);

    list.addToHead(&elements[0]);

    CheckList(list, reporter, false, 1, true, false, false, false, elements);

    list.addToHead(&elements[1]);
    list.addToHead(&elements[2]);
    list.addToHead(&elements[3]);

    CheckList(list, reporter, false, 4, true, true, true, true, elements);

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

    CheckList(list, reporter, false, 1, false, false, true, false, elements);

    // remove last element
    list.remove(&elements[2]);

    // list should be empty again
    CheckList(list, reporter, true, 0, false, false, false, false, elements);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("TDLinkedList", TestTDLinkedListClass, TestTDLinkedList)
