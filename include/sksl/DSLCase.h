/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_CASE
#define SKSL_DSL_CASE

#include "include/private/SkSLDefines.h"
#include "include/sksl/DSLExpression.h"
#include "include/sksl/DSLStatement.h"

#include <memory>

namespace SkSL {

class Statement;

namespace dsl {

class DSLCase {
public:
    class Value {
    public:
        Value(SKSL_INT value)
            : fValue(value)
            , fIsDefault(false) {}

    private:
        Value()
            : fValue(-1)
            , fIsDefault(true) {}

        SKSL_INT fValue;
        bool fIsDefault;

        friend class DSLCase;
    };

    static Value Default() {
        return Value();
    }

    template<class... Statements>
    DSLCase(Value value, Statements... statements)
        : fValue(std::move(value)) {
        fStatements.reserve_back(sizeof...(statements));
        // in C++17, we could just do:
        // (fStatements.push_back(DSLStatement(std::move(statements)).release()), ...);
        int unused[] =
          {0,
           (static_cast<void>(fStatements.push_back(DSLStatement(std::move(statements)).release())),
            0)...};
        static_cast<void>(unused);
    }

    DSLCase(DSLCase&&);

    DSLCase(Value value, SkSL::StatementArray statements);

    ~DSLCase();

    void append(DSLStatement stmt);

private:
    std::unique_ptr<Expression> value();

    Value fValue;
    SkSL::StatementArray fStatements;

    template<class... Cases>
    friend DSLPossibleStatement Switch(DSLExpression value, Cases... cases);
};

} // namespace dsl

} // namespace SkSL

#endif
