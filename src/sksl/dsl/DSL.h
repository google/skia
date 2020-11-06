/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL
#define SKSL_DSL

#include "src/sksl/SkSLContext.h"
#include "src/sksl/dsl/Function.h"
#include "src/sksl/dsl/Type.h"
#include "src/sksl/dsl/Var.h"
#include "src/sksl/dsl/priv/BlockCreator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/dsl/priv/Statement.h"

namespace skslcode {

extern Var sk_FragColor;

enum SwizzleComponent {
    R,
    G,
    B,
    A,
    X,
    Y,
    Z,
    W,
    ZERO,
    ONE
};

template<class... Stmts>
std::unique_ptr<SkSL::Statement> Block(Stmts... stmts) {
    return BlockCreator<Stmts...>(std::move(stmts)...).block();
}

std::unique_ptr<SkSL::Statement> Block(SkSL::StatementArray stmts);

std::unique_ptr<SkSL::Statement> Do(Statement stmt, Expression test);

std::unique_ptr<SkSL::Statement> If(Expression test, Statement ifTrue);

std::unique_ptr<SkSL::Statement> If(Expression test, Statement ifTrue, Statement ifFalse);

Expression Swizzle(Expression base, SwizzleComponent a);

Expression Swizzle(Expression base, SwizzleComponent a, SwizzleComponent b);

Expression Swizzle(Expression base, SwizzleComponent a, SwizzleComponent b, SwizzleComponent c);

Expression Swizzle(Expression base, SwizzleComponent a, SwizzleComponent b, SwizzleComponent c,
                   SwizzleComponent d);

std::unique_ptr<SkSL::Statement> While(Expression test, Statement stmt);

} // namespace skslcode

#endif
