/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSPath.h"
#include "tests/Test.h"


static void test_edger(skiatest::Reporter* r,
                       const std::initializer_list<SkPath::Verb>& in,
                       const std::initializer_list<SkPath::Verb>& expected) {
    SkPath path;
    SkScalar x = 0, y = 0;
    for (auto v : in) {
        switch (v) {
            case SkPath::kMove_Verb: path.moveTo(x++, y++); break;
            case SkPath::kLine_Verb: path.lineTo(x++, y++); break;
            case SkPath::kClose_Verb: path.close(); break;
            default: SkASSERT(false);
        }
    }

    SkPathEdgeIter iter(path);
    for (auto v : expected) {
        auto e = iter.next();
        REPORTER_ASSERT(r, e);
        REPORTER_ASSERT(r, SkPathEdgeIter::EdgeToVerb(e.fEdge) == v);
    }
    auto e = iter.next();
    REPORTER_ASSERT(r, !e);
}

DEF_TEST(pathedger, r) {
    auto M = SkPath::kMove_Verb;
    auto L = SkPath::kLine_Verb;
    auto C = SkPath::kClose_Verb;

    test_edger(r, { M }, {});
    test_edger(r, { M, M }, {});
    test_edger(r, { M, C }, {});
    test_edger(r, { M, M, C }, {});
    test_edger(r, { M, L }, { L, L });
    test_edger(r, { M, L, C }, { L, L });
    test_edger(r, { M, L, L }, { L, L, L });
    test_edger(r, { M, L, L, C }, { L, L, L });

    test_edger(r, { M, L, L, M, L, L }, { L, L, L,   L, L, L });
}
