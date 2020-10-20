/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLNodeArrayWrapper.h"

#include "tests/Test.h"

DEF_TEST(SkSLNodeArrayWrapper, r) {
    SkSL::ExpressionArray base;
    SkSL::NodeArrayWrapper<SkSL::IntLiteral, SkSL::Expression> wrapper(&base);
    REPORTER_ASSERT(r, wrapper.empty());
    base.emplace_back(new SkSL::IntLiteral(-1, 0));
    REPORTER_ASSERT(r, !wrapper.empty());
    base.emplace_back(new SkSL::IntLiteral(-1, 1));
    base.emplace_back(new SkSL::IntLiteral(-1, 2));
    REPORTER_ASSERT(r, wrapper.count() == 3);
    REPORTER_ASSERT(r, wrapper[0].value() == 0);
    REPORTER_ASSERT(r, wrapper[1].value() == 1);
    REPORTER_ASSERT(r, wrapper[2].value() == 2);
    wrapper.push_back(new SkSL::IntLiteral(-1, 3));
    REPORTER_ASSERT(r, base.count() == 4);
    REPORTER_ASSERT(r, wrapper.count() == 4);
    REPORTER_ASSERT(r, wrapper[3].value() == 3);
    auto iter = wrapper.begin();
    int i = 0;
    while (iter != wrapper.end()) {
        REPORTER_ASSERT(r, wrapper[i].value() == iter->value());
        ++i;
        ++iter;
    }

    const SkSL::NodeArrayWrapper<SkSL::IntLiteral, SkSL::Expression> copy(wrapper);
    SkSL::NodeArrayWrapper<SkSL::IntLiteral, SkSL::Expression>::const_iterator constIter =
                                                                                       copy.begin();
    i =  0;
    while (constIter != copy.end()) {
        REPORTER_ASSERT(r, copy[i].value() == constIter->value());
        ++i;
        ++constIter;
    }

    REPORTER_ASSERT(r, wrapper.front().value() == 0);
    REPORTER_ASSERT(r, wrapper.back().value() == 3);
    wrapper.pop_back();
    REPORTER_ASSERT(r, wrapper.back().value() == 2);
    wrapper.reset();
    REPORTER_ASSERT(r, wrapper.empty());
}

DEF_TEST(SkSLConstNodeArrayWrapper, r) {
    SkSL::ExpressionArray base;
    SkSL::ConstNodeArrayWrapper<SkSL::IntLiteral, SkSL::Expression> wrapper(&base);
    REPORTER_ASSERT(r, wrapper.empty());
    base.emplace_back(new SkSL::IntLiteral(-1, 0));
    REPORTER_ASSERT(r, !wrapper.empty());
    base.emplace_back(new SkSL::IntLiteral(-1, 1));
    base.emplace_back(new SkSL::IntLiteral(-1, 2));
    REPORTER_ASSERT(r, wrapper.count() == 3);
    REPORTER_ASSERT(r, wrapper[0].value() == 0);
    REPORTER_ASSERT(r, wrapper[1].value() == 1);
    REPORTER_ASSERT(r, wrapper[2].value() == 2);
    auto iter = wrapper.begin();
    int i = 0;
    while (iter != wrapper.end()) {
        REPORTER_ASSERT(r, wrapper[i].value() == iter->value());
        ++i;
        ++iter;
    }
    REPORTER_ASSERT(r, wrapper.front().value() == 0);
    REPORTER_ASSERT(r, wrapper.back().value() == 2);
}
