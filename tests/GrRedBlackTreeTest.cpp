/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test
#if SK_SUPPORT_GPU

#include "GrRedBlackTree.h"
#include "SkRandom.h"
#include "Test.h"

typedef GrRedBlackTree<int> Tree;

DEF_TEST(GrRedBlackTree, reporter) {
    Tree tree;

    SkRandom r;

    int count[100] = {0};
    // add 10K ints
    for (int i = 0; i < 10000; ++i) {
        int x = r.nextU() % 100;
        Tree::Iter xi = tree.insert(x);
        REPORTER_ASSERT(reporter, *xi == x);
        ++count[x];
    }

    tree.insert(0);
    ++count[0];
    tree.insert(99);
    ++count[99];
    REPORTER_ASSERT(reporter, *tree.begin() == 0);
    REPORTER_ASSERT(reporter, *tree.last() == 99);
    REPORTER_ASSERT(reporter, --(++tree.begin()) == tree.begin());
    REPORTER_ASSERT(reporter, --tree.end() == tree.last());
    REPORTER_ASSERT(reporter, tree.count() == 10002);

    int c = 0;
    // check that we iterate through the correct number of
    // elements and they are properly sorted.
    for (Tree::Iter a = tree.begin(); tree.end() != a; ++a) {
        Tree::Iter b = a;
        ++b;
        ++c;
        REPORTER_ASSERT(reporter, b == tree.end() || *a <= *b);
    }
    REPORTER_ASSERT(reporter, c == tree.count());

    // check that the tree reports the correct number of each int
    // and that we can iterate through them correctly both forward
    // and backward.
    for (int i = 0; i < 100; ++i) {
        int c;
        c = tree.countOf(i);
        REPORTER_ASSERT(reporter, c == count[i]);
        c = 0;
        Tree::Iter iter = tree.findFirst(i);
        while (iter != tree.end() && *iter == i) {
            ++c;
            ++iter;
        }
        REPORTER_ASSERT(reporter, count[i] == c);
        c = 0;
        iter = tree.findLast(i);
        if (iter != tree.end()) {
            do {
                if (*iter == i) {
                    ++c;
                } else {
                    break;
                }
                if (iter != tree.begin()) {
                    --iter;
                } else {
                    break;
                }
            } while (true);
        }
        REPORTER_ASSERT(reporter, c == count[i]);
    }
    // remove all the ints between 25 and 74. Randomly chose to remove
    // the first, last, or any entry for each.
    for (int i = 25; i < 75; ++i) {
        while (0 != tree.countOf(i)) {
            --count[i];
            int x = r.nextU() % 3;
            Tree::Iter iter;
            switch (x) {
            case 0:
                iter = tree.findFirst(i);
                break;
            case 1:
                iter = tree.findLast(i);
                break;
            case 2:
            default:
                iter = tree.find(i);
                break;
            }
            tree.remove(iter);
        }
        REPORTER_ASSERT(reporter, 0 == count[i]);
        REPORTER_ASSERT(reporter, tree.findFirst(i) == tree.end());
        REPORTER_ASSERT(reporter, tree.findLast(i) == tree.end());
        REPORTER_ASSERT(reporter, tree.find(i) == tree.end());
    }
    // remove all of the 0 entries. (tests removing begin())
    REPORTER_ASSERT(reporter, *tree.begin() == 0);
    REPORTER_ASSERT(reporter, *(--tree.end()) == 99);
    while (0 != tree.countOf(0)) {
        --count[0];
        tree.remove(tree.find(0));
    }
    REPORTER_ASSERT(reporter, 0 == count[0]);
    REPORTER_ASSERT(reporter, tree.findFirst(0) == tree.end());
    REPORTER_ASSERT(reporter, tree.findLast(0) == tree.end());
    REPORTER_ASSERT(reporter, tree.find(0) == tree.end());
    REPORTER_ASSERT(reporter, 0 < *tree.begin());

    // remove all the 99 entries (tests removing last()).
    while (0 != tree.countOf(99)) {
        --count[99];
        tree.remove(tree.find(99));
    }
    REPORTER_ASSERT(reporter, 0 == count[99]);
    REPORTER_ASSERT(reporter, tree.findFirst(99) == tree.end());
    REPORTER_ASSERT(reporter, tree.findLast(99) == tree.end());
    REPORTER_ASSERT(reporter, tree.find(99) == tree.end());
    REPORTER_ASSERT(reporter, 99 > *(--tree.end()));
    REPORTER_ASSERT(reporter, tree.last() == --tree.end());

    // Make sure iteration still goes through correct number of entries
    // and is still sorted correctly.
    c = 0;
    for (Tree::Iter a = tree.begin(); tree.end() != a; ++a) {
        Tree::Iter b = a;
        ++b;
        ++c;
        REPORTER_ASSERT(reporter, b == tree.end() || *a <= *b);
    }
    REPORTER_ASSERT(reporter, c == tree.count());

    // repeat check that correct number of each entry is in the tree
    // and iterates correctly both forward and backward.
    for (int i = 0; i < 100; ++i) {
        REPORTER_ASSERT(reporter, tree.countOf(i) == count[i]);
        int c = 0;
        Tree::Iter iter = tree.findFirst(i);
        while (iter != tree.end() && *iter == i) {
            ++c;
            ++iter;
        }
        REPORTER_ASSERT(reporter, count[i] == c);
        c = 0;
        iter = tree.findLast(i);
        if (iter != tree.end()) {
            do {
                if (*iter == i) {
                    ++c;
                } else {
                    break;
                }
                if (iter != tree.begin()) {
                    --iter;
                } else {
                    break;
                }
            } while (true);
        }
        REPORTER_ASSERT(reporter, count[i] == c);
    }

    // remove all entries
    while (!tree.empty()) {
        tree.remove(tree.begin());
    }

    // test reset on empty tree.
    tree.reset();
}

#endif
