/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkRandom.h"

enum Op {
    kPlus,
    kMinus,
    kMul,
    kDiv,
    kNoop
};

static Op next_op(Op op) {
    SkASSERT(kNoop != op);

    return Op(op + 1);
}

static int apply(Op op, int left, int right) {
    switch (op) {
    case kPlus:
        return left + right;  // symmetric
    case kMinus:
        return left - right;  // !!
    case kMul:
        return left * right;  // symmetric
    case kDiv:
        if (!right) {
            return 10000000;
        }
        return left / right;  // !!
    }

    SkASSERT(0);
    return -100000;
}

static char op_name(Op op) {
    switch (op) {
    case kPlus:
        return '+';
    case kMinus:
        return '-';
    case kMul:
        return '*';
    case kDiv:
        return '/';
    }

    SkASSERT(0);
    return '$';
}

static bool try_all(Op leftOp, Op midOp, Op rightOp, int foo[4]) {

    int left = apply(leftOp, foo[0], foo[1]);
    int right = apply(rightOp, foo[2], foo[3]);
    int ans = apply(midOp, left, right);
    SkDebugf("(%d %c %d) %c (%d %c %d) == %d\n",
            foo[0], op_name(leftOp), foo[1], op_name(midOp),
            foo[2], op_name(rightOp), foo[3], ans);

    return ans == 24;
}

static bool try_rec(Op leftOp, Op midOp, Op rightOp, int foo[4]) {

    if (kNoop == leftOp || kNoop == midOp || kNoop == rightOp) {
        return false;
    }

    if (try_all(midOp, leftOp, rightOp, foo)) {
        return true;
    }

    if (try_rec(next_op(leftOp), midOp, rightOp, foo)) {
        return true;
    }

    if (try_rec(leftOp, next_op(midOp), rightOp, foo)) {
        return true;
    }

    if (try_rec(leftOp, midOp, next_op(rightOp), foo)) {
        return true;
    }

    return false;
}

static void swap1(int foo[4], int i, int j) {
    int temp = foo[i];
    foo[i] = foo[j];
    foo[j] = temp;
}

#include <algorithm>

static bool next_perm(int numbers[4]) {
    for (int i = 2; i >= 0; --i) {
        if (numbers[i] < numbers[i+1]) {
            for (int j = 3; j >= 0; --j) {
                if (numbers[i] < numbers[j]) {
                    swap1(numbers, i, j);
                    std::reverse(&numbers[i+1], &numbers[4]);
                    return true;
                }
            }
        }
    }

    return false;
}

bool can_compute_24(int numbers[4]) {
    std::sort(numbers, numbers+4);

    bool shouldContinue = true;
    while (shouldContinue) {
//        SkDebugf("%d %d %d %d\n", numbers[0], numbers[1], numbers[2], numbers[3]);
        if (try_rec(kPlus, kPlus, kPlus, numbers)) {
            return true;
        }

        shouldContinue = next_perm(numbers);
    }

    return false;
}

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(1000, 1000);
    }

    void onDraw(SkCanvas* canvas) override {
        SkRandom rand;

        int foo[4];
        bool can;
#if 0
        foo[0] = rand.nextRangeU(1, 9);
        foo[1] = rand.nextRangeU(1, 9);
        foo[2] = rand.nextRangeU(1, 9);
        foo[3] = rand.nextRangeU(1, 9);

        can = can_compute(foo);
#endif

        // 1 8 9 8 -> true (8+8+9-1)
        // 7 3 2 4 -> true ((2+4) * (7-3))
        // 1 1 1 1 -> failure

        foo[0] = 2;
        foo[1] = 4;
        foo[2] = 7;
        foo[3] = 3;

        can = can_compute_24(foo);
        SkASSERT(can);

        foo[0] = 8;
        foo[1] = 8;
        foo[2] = 9;
        foo[3] = 1;

        can = can_compute_24(foo);
        SkASSERT(can);

        foo[0] = 1;
        foo[1] = 1;
        foo[2] = 1;
        foo[3] = 1;

        can = can_compute_24(foo);
        SkASSERT(!can);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}
