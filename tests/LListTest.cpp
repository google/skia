/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkRandom.h"
#include "src/core/SkTInternalLList.h"
#include "tests/Test.h"

class ListElement {
public:
    ListElement(int id) : fID(id) {
    }
    bool operator== (const ListElement& other) { return fID == other.fID; }

    int fID;

private:

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(ListElement);
};

static void check_list(const SkTInternalLList<ListElement>& list,
                       skiatest::Reporter* reporter,
                       bool empty,
                       int numElements,
                       bool in0, bool in1, bool in2, bool in3,
                       ListElement elements[4]) {

    REPORTER_ASSERT(reporter, empty == list.isEmpty());
#ifdef SK_DEBUG
    list.validate();
    REPORTER_ASSERT(reporter, numElements == list.countEntries());
    REPORTER_ASSERT(reporter, in0 == list.isInList(&elements[0]));
    REPORTER_ASSERT(reporter, in1 == list.isInList(&elements[1]));
    REPORTER_ASSERT(reporter, in2 == list.isInList(&elements[2]));
    REPORTER_ASSERT(reporter, in3 == list.isInList(&elements[3]));
#endif
}

DEF_TEST(InternalLList, reporter) {
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
    for (int i = 0; cur; ++i, cur = iter.next()) {
        REPORTER_ASSERT(reporter, cur->fID == 3-i);
    }

    cur = iter.init(list, Iter::kTail_IterStart);
    for (int i = 0; cur; ++i, cur = iter.prev()) {
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

    // test out methods that add to the middle of the list.
    list.addAfter(&elements[1], nullptr);
    check_list(list, reporter, false, 1, false, true, false, false, elements);

    list.remove(&elements[1]);

    list.addBefore(&elements[1], nullptr);
    check_list(list, reporter, false, 1, false, true, false, false, elements);

    list.addBefore(&elements[0], &elements[1]);
    check_list(list, reporter, false, 2, true, true, false, false, elements);

    list.addAfter(&elements[3], &elements[1]);
    check_list(list, reporter, false, 3, true, true, false, true, elements);

    list.addBefore(&elements[2], &elements[3]);
    check_list(list, reporter, false, 4, true, true, true, true, elements);

    cur = iter.init(list, Iter::kHead_IterStart);
    for (int i = 0; cur; ++i, cur = iter.next()) {
        REPORTER_ASSERT(reporter, cur->fID == i);
    }
    while (!list.isEmpty()) {
        list.remove(list.tail());
    }

    // test concat.
    SkTInternalLList<ListElement> listA, listB;
    listA.concat(std::move(listB));
    check_list(listA, reporter, true, 0, false, false, false, false, elements);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    check_list(listB, reporter, true, 0, false, false, false, false, elements);

    listB.addToTail(&elements[0]);
    listA.concat(std::move(listB));
    check_list(listA, reporter, false, 1, true, false, false, false, elements);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    check_list(listB, reporter, true, 0, false, false, false, false, elements);

    listB.addToTail(&elements[1]);
    listA.concat(std::move(listB));
    check_list(listA, reporter, false, 2, true, true, false, false, elements);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    check_list(listB, reporter, true, 0, false, false, false, false, elements);

    listA.concat(std::move(listB));
    check_list(listA, reporter, false, 2, true, true, false, false, elements);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    check_list(listB, reporter, true, 0, false, false, false, false, elements);

    listB.addToTail(&elements[2]);
    listB.addToTail(&elements[3]);
    listA.concat(std::move(listB));
    check_list(listA, reporter, false, 4, true, true, true, true, elements);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    check_list(listB, reporter, true, 0, false, false, false, false, elements);

    cur = iter.init(listA, Iter::kHead_IterStart);
    for (int i = 0; cur; ++i, cur = iter.next()) {
        REPORTER_ASSERT(reporter, cur->fID == i);
    }
}
