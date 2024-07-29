/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IRHELPERS
#define SKSL_IRHELPERS

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFieldSymbol.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <memory>
#include <utility>

namespace SkSL {

class Statement;
class Variable;

struct IRHelpers {
    IRHelpers(const Context& c) : fContext(c) {}

    // Note that this class doesn't adhere to the typical Skia style rules; function names are
    // capitalized, and we don't use `this->` prefixes. This helps nested expressions flow more
    // naturally.

    std::unique_ptr<Expression> Ref(const Variable* var) const {
        return VariableReference::Make(Position(), var);
    }

    std::unique_ptr<Expression> Field(const Variable* var, int idx) const {
        return FieldAccess::Make(fContext, Position(), Ref(var), idx,
                                 FieldAccess::OwnerKind::kAnonymousInterfaceBlock);
    }

    std::unique_ptr<Expression> Swizzle(std::unique_ptr<Expression> base, ComponentArray c) const {
        Position pos = base->fPosition;
        return Swizzle::Make(fContext, pos, std::move(base), std::move(c));
    }

    std::unique_ptr<Expression> Index(std::unique_ptr<Expression> base,
                                      std::unique_ptr<Expression> idx) const {
        Position pos = base->fPosition.rangeThrough(idx->fPosition);
        return IndexExpression::Make(fContext, pos, std::move(base), std::move(idx));
    }

    std::unique_ptr<Expression> Binary(std::unique_ptr<Expression> l,
                                       Operator op,
                                       std::unique_ptr<Expression> r) const {
        Position pos = l->fPosition.rangeThrough(r->fPosition);
        return BinaryExpression::Make(fContext, pos, std::move(l), op, std::move(r));
    }

    std::unique_ptr<Expression> Mul(std::unique_ptr<Expression> l,
                                    std::unique_ptr<Expression> r) const {
        return Binary(std::move(l), OperatorKind::STAR, std::move(r));
    }

    std::unique_ptr<Expression> Add(std::unique_ptr<Expression> l,
                                    std::unique_ptr<Expression> r) const {
        return Binary(std::move(l), OperatorKind::PLUS, std::move(r));
    }

    std::unique_ptr<Expression> Float(float value) const {
        return Literal::MakeFloat(Position(), value, fContext.fTypes.fFloat.get());
    }

    std::unique_ptr<Expression> Int(int value) const {
        return Literal::MakeInt(Position(), value, fContext.fTypes.fInt.get());
    }

    std::unique_ptr<Expression> CtorXYZW(std::unique_ptr<Expression> xy,
                                         std::unique_ptr<Expression> z,
                                         std::unique_ptr<Expression> w) const {
        ExpressionArray args;
        args.push_back(std::move(xy));
        args.push_back(std::move(z));
        args.push_back(std::move(w));
        return ConstructorCompound::Make(fContext, Position(), *fContext.fTypes.fFloat4,
                                         std::move(args));
    }

    std::unique_ptr<Statement> Assign(std::unique_ptr<Expression> l,
                                      std::unique_ptr<Expression> r) const {
        SkAssertResult(Analysis::UpdateVariableRefKind(l.get(), VariableRefKind::kWrite));
        return ExpressionStatement::Make(fContext,
                                         Binary(std::move(l), OperatorKind::EQ, std::move(r)));
    }

    const Context& fContext;
};

}  // namespace SkSL

#endif
