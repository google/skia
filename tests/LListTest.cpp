/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkRandom.h"
#include "src/core/SkTInternalLList.h"
#include "src/core/SkTLList.h"
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

static void test_tinternallist(skiatest::Reporter* reporter) {
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

template <unsigned int N> static void test_tllist(skiatest::Reporter* reporter) {
    typedef SkTLList<ListElement, N> ElList;
    typedef typename ElList::Iter Iter;
    SkRandom random;

    ElList list1;
    ElList list2;
    Iter iter1;
    Iter iter2;
    Iter iter3;
    Iter iter4;

    REPORTER_ASSERT(reporter, list1.isEmpty());
    REPORTER_ASSERT(reporter, nullptr == iter1.init(list1, Iter::kHead_IterStart));
    REPORTER_ASSERT(reporter, nullptr == iter1.init(list1, Iter::kTail_IterStart));
    // Try popping an empty list
    list1.popHead();
    list1.popTail();
    REPORTER_ASSERT(reporter, list1.isEmpty());
    REPORTER_ASSERT(reporter, list1 == list2);

    // Create two identical lists, one by appending to head and the other to the tail.
    list1.addToHead(ListElement(1));
    list2.addToTail(ListElement(1));
    iter1.init(list1, Iter::kHead_IterStart);
    iter2.init(list1, Iter::kTail_IterStart);
    REPORTER_ASSERT(reporter, iter1.get()->fID == iter2.get()->fID);
    iter3.init(list2, Iter::kHead_IterStart);
    iter4.init(list2, Iter::kTail_IterStart);
    REPORTER_ASSERT(reporter, iter3.get()->fID == iter1.get()->fID);
    REPORTER_ASSERT(reporter, iter4.get()->fID == iter1.get()->fID);
    REPORTER_ASSERT(reporter, list1 == list2);

    list2.reset();

    // use both before/after in-place construction on an empty list
    list2.addBefore(list2.headIter(), 1);
    REPORTER_ASSERT(reporter, list2 == list1);
    list2.reset();

    list2.addAfter(list2.tailIter(), 1);
    REPORTER_ASSERT(reporter, list2 == list1);

    // add an element to the second list, check that iters are still valid
    iter3.init(list2, Iter::kHead_IterStart);
    iter4.init(list2, Iter::kTail_IterStart);
    list2.addToHead(ListElement(2));

    REPORTER_ASSERT(reporter, iter3.get()->fID == iter1.get()->fID);
    REPORTER_ASSERT(reporter, iter4.get()->fID == iter1.get()->fID);
    REPORTER_ASSERT(reporter, 1 == Iter(list2, Iter::kTail_IterStart).get()->fID);
    REPORTER_ASSERT(reporter, 2 == Iter(list2, Iter::kHead_IterStart).get()->fID);
    REPORTER_ASSERT(reporter, list1 != list2);
    list1.addToHead(ListElement(2));
    REPORTER_ASSERT(reporter, list1 == list2);
    REPORTER_ASSERT(reporter, !list1.isEmpty());

    list1.reset();
    list2.reset();
    REPORTER_ASSERT(reporter, list1.isEmpty() && list2.isEmpty());

    // randomly perform insertions and deletions on a list and perform tests
    int count = 0;
    for (int j = 0; j < 100; ++j) {
        if (list1.isEmpty() || random.nextBiasedBool(3  * SK_Scalar1 / 4)) {
            int id = j;
            // Choose one of three ways to insert a new element: at the head, at the tail,
            // before a random element, after a random element
            int numValidMethods = 0 == count ? 2 : 4;
            int insertionMethod = random.nextULessThan(numValidMethods);
            switch (insertionMethod) {
                case 0:
                    list1.addToHead(ListElement(id));
                    break;
                case 1:
                    list1.addToTail(ListElement(id));
                    break;
                case 2: // fallthru to share code that picks random element.
                case 3: {
                    int n = random.nextULessThan(list1.count());
                    Iter iter = list1.headIter();
                    // remember the elements before/after the insertion point.
                    while (n--) {
                        iter.next();
                    }
                    Iter prev(iter);
                    Iter next(iter);
                    next.next();
                    prev.prev();

                    SkASSERT(iter.get());
                    // insert either before or after the iterator, then check that the
                    // surrounding sequence is correct.
                    if (2 == insertionMethod) {
                        list1.addBefore(iter, id);
                        Iter newItem(iter);
                        newItem.prev();
                        REPORTER_ASSERT(reporter, newItem.get()->fID == id);

                        if (next.get()) {
                            REPORTER_ASSERT(reporter, next.prev()->fID == iter.get()->fID);
                        }
                        if (prev.get()) {
                            REPORTER_ASSERT(reporter, prev.next()->fID == id);
                        }
                    } else {
                        list1.addAfter(iter, id);
                        Iter newItem(iter);
                        newItem.next();
                        REPORTER_ASSERT(reporter, newItem.get()->fID == id);

                        if (next.get()) {
                            REPORTER_ASSERT(reporter, next.prev()->fID == id);
                        }
                        if (prev.get()) {
                            REPORTER_ASSERT(reporter, prev.next()->fID == iter.get()->fID);
                        }
                    }
                }
            }
            ++count;
        } else {
            // walk to a random place either forward or backwards and remove.
            int n = random.nextULessThan(list1.count());
            typename Iter::IterStart start;
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
                REPORTER_ASSERT(reporter, iter.get());
                (iter.*incrFunc)();
            }
            REPORTER_ASSERT(reporter, iter.get());

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
            REPORTER_ASSERT(reporter, pn.get() == next.get() || nullptr == prev.get());
            // Similarly, np should match prev unless next originally walked off the tail.
            REPORTER_ASSERT(reporter, np.get() == prev.get() || nullptr == next.get());
            --count;
        }
        REPORTER_ASSERT(reporter, count == list1.count());
    }
}

DEF_TEST(LList, reporter) {
    test_tinternallist(reporter);
    test_tllist<1>(reporter);
    test_tllist<3>(reporter);
    test_tllist<8>(reporter);
    test_tllist<10>(reporter);
    test_tllist<16>(reporter);
}
