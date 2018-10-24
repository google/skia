/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STANDALONE

#include "SkSLInterpreter.h"
#include "ir/SkSLBinaryExpression.h"
#include "ir/SkSLExpressionStatement.h"
#include "ir/SkSLForStatement.h"
#include "ir/SkSLFunctionCall.h"
#include "ir/SkSLFunctionReference.h"
#include "ir/SkSLIfStatement.h"
#include "ir/SkSLIndexExpression.h"
#include "ir/SkSLPostfixExpression.h"
#include "ir/SkSLPrefixExpression.h"
#include "ir/SkSLProgram.h"
#include "ir/SkSLStatement.h"
#include "ir/SkSLTernaryExpression.h"
#include "ir/SkSLVarDeclarations.h"
#include "ir/SkSLVarDeclarationsStatement.h"
#include "ir/SkSLVariableReference.h"
#include "SkRasterPipeline.h"

namespace SkSL {

void Interpreter::run() {
    for (const auto& e : *fProgram) {
        if (ProgramElement::kFunction_Kind == e.fKind) {
            const FunctionDefinition& f = (const FunctionDefinition&) e;
            if ("appendStages" == f.fDeclaration.fName) {
                this->run(f);
                return;
            }
        }
    }
    SkASSERT(false);
}

static int SizeOf(const Type& type) {
    return 1;
}

void Interpreter::run(const FunctionDefinition& f) {
    fVars.emplace_back();
    StackIndex current = (StackIndex) fStack.size();
    for (int i = f.fDeclaration.fParameters.size() - 1; i >= 0; --i) {
        current -= SizeOf(f.fDeclaration.fParameters[i]->fType);
        fVars.back()[f.fDeclaration.fParameters[i]] = current;
    }
    fCurrentIndex.push_back({ f.fBody.get(), 0 });
    while (fCurrentIndex.size()) {
        this->runStatement();
    }
}

void Interpreter::push(Value value) {
    fStack.push_back(value);
}

Interpreter::Value Interpreter::pop() {
    auto iter = fStack.end() - 1;
    Value result = *iter;
    fStack.erase(iter);
    return result;
}

 Interpreter::StackIndex Interpreter::stackAlloc(int count) {
    int result = fStack.size();
    for (int i = 0; i < count; ++i) {
        fStack.push_back(Value((int) 0xDEADBEEF));
    }
    return result;
}

void Interpreter::runStatement() {
    const Statement& stmt = *fCurrentIndex.back().fStatement;
    const size_t index = fCurrentIndex.back().fIndex;
    fCurrentIndex.pop_back();
    switch (stmt.fKind) {
        case Statement::kBlock_Kind: {
            const Block& b = (const Block&) stmt;
            if (!b.fStatements.size()) {
                break;
            }
            SkASSERT(index < b.fStatements.size());
            if (index < b.fStatements.size() - 1) {
                fCurrentIndex.push_back({ &b, index + 1 });
            }
            fCurrentIndex.push_back({ b.fStatements[index].get(), 0 });
            break;
        }
        case Statement::kBreak_Kind:
            SkASSERT(index == 0);
            abort();
        case Statement::kContinue_Kind:
            SkASSERT(index == 0);
            abort();
        case Statement::kDiscard_Kind:
            SkASSERT(index == 0);
            abort();
        case Statement::kDo_Kind:
            abort();
        case Statement::kExpression_Kind:
            SkASSERT(index == 0);
            this->evaluate(*((const ExpressionStatement&) stmt).fExpression);
            break;
        case Statement::kFor_Kind: {
            ForStatement& f = (ForStatement&) stmt;
            switch (index) {
                case 0:
                    // initializer
                    fCurrentIndex.push_back({ &f, 1 });
                    if (f.fInitializer) {
                        fCurrentIndex.push_back({ f.fInitializer.get(), 0 });
                    }
                    break;
                case 1:
                    // test & body
                    if (f.fTest && !evaluate(*f.fTest).fBool) {
                        break;
                    } else {
                        fCurrentIndex.push_back({ &f, 2 });
                        fCurrentIndex.push_back({ f.fStatement.get(), 0 });
                    }
                    break;
                case 2:
                    // next
                    if (f.fNext) {
                        this->evaluate(*f.fNext);
                    }
                    fCurrentIndex.push_back({ &f, 1 });
                    break;
                default:
                    SkASSERT(false);
            }
            break;
        }
        case Statement::kGroup_Kind:
            abort();
        case Statement::kIf_Kind: {
            IfStatement& i = (IfStatement&) stmt;
            if (evaluate(*i.fTest).fBool) {
                fCurrentIndex.push_back({ i.fIfTrue.get(), 0 });
            } else if (i.fIfFalse) {
                fCurrentIndex.push_back({ i.fIfFalse.get(), 0 });
            }
            break;
        }
        case Statement::kNop_Kind:
            SkASSERT(index == 0);
            break;
        case Statement::kReturn_Kind:
            SkASSERT(index == 0);
            abort();
        case Statement::kSwitch_Kind:
            abort();
        case Statement::kVarDeclarations_Kind:
            SkASSERT(index == 0);
            for (const auto& decl :((const VarDeclarationsStatement&) stmt).fDeclaration->fVars) {
                const Variable* var = ((VarDeclaration&) *decl).fVar;
                StackIndex pos = this->stackAlloc(SizeOf(var->fType));
                fVars.back()[var] = pos;
                if (var->fInitialValue) {
                    fStack[pos] = this->evaluate(*var->fInitialValue);
                }
            }
            break;
        case Statement::kWhile_Kind:
            abort();
        default:
            abort();
    }
}

static Interpreter::TypeKind type_kind(const Type& type) {
    if (type.fName == "int") {
        return Interpreter::kInt_TypeKind;
    } else if (type.fName == "float") {
        return Interpreter::kFloat_TypeKind;
    }
    ABORT("unsupported type: %s\n", type.description().c_str());
}

Interpreter::StackIndex Interpreter::getLValue(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kFieldAccess_Kind:
            break;
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = (const IndexExpression&) expr;
            return this->evaluate(*idx.fBase).fInt + this->evaluate(*idx.fIndex).fInt;
        }
        case Expression::kSwizzle_Kind:
            break;
        case Expression::kVariableReference_Kind:
            SkASSERT(fVars.size());
            SkASSERT(fVars.back().find(&((VariableReference&) expr).fVariable) !=
                   fVars.back().end());
            return fVars.back()[&((VariableReference&) expr).fVariable];
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = (const TernaryExpression&) expr;
            return this->getLValue(this->evaluate(*t.fTest).fBool ? *t.fIfTrue : *t.fIfFalse);
        }
        case Expression::kTypeReference_Kind:
            break;
        default:
            break;
    }
    ABORT("unsupported lvalue");
}

struct CallbackCtx : public SkRasterPipeline_CallbackCtx {
    Interpreter* fInterpreter;
    const FunctionDefinition* fFunction;
};

static void do_callback(SkRasterPipeline_CallbackCtx* raw, int activePixels) {
    CallbackCtx& ctx = (CallbackCtx&) *raw;
    for (int i = 0; i < activePixels; ++i) {
        ctx.fInterpreter->push(Interpreter::Value(ctx.rgba[i * 4 + 0]));
        ctx.fInterpreter->push(Interpreter::Value(ctx.rgba[i * 4 + 1]));
        ctx.fInterpreter->push(Interpreter::Value(ctx.rgba[i * 4 + 2]));
        ctx.fInterpreter->run(*ctx.fFunction);
        ctx.read_from[i * 4 + 2] = ctx.fInterpreter->pop().fFloat;
        ctx.read_from[i * 4 + 1] = ctx.fInterpreter->pop().fFloat;
        ctx.read_from[i * 4 + 0] = ctx.fInterpreter->pop().fFloat;
    }
}

void Interpreter::appendStage(const AppendStage& a) {
    switch (a.fStage) {
        case SkRasterPipeline::matrix_4x5: {
            SkASSERT(a.fArguments.size() == 1);
            StackIndex transpose = evaluate(*a.fArguments[0]).fInt;
            fPipeline.append(SkRasterPipeline::matrix_4x5, &fStack[transpose]);
            break;
        }
        case SkRasterPipeline::callback: {
            SkASSERT(a.fArguments.size() == 1);
            CallbackCtx* ctx = new CallbackCtx();
            ctx->fInterpreter = this;
            ctx->fn = do_callback;
            for (const auto& e : *fProgram) {
                if (ProgramElement::kFunction_Kind == e.fKind) {
                    const FunctionDefinition& f = (const FunctionDefinition&) e;
                    if (&f.fDeclaration ==
                                      ((const FunctionReference&) *a.fArguments[0]).fFunctions[0]) {
                        ctx->fFunction = &f;
                    }
                }
            }
            fPipeline.append(SkRasterPipeline::callback, ctx);
            break;
        }
        default:
            fPipeline.append(a.fStage);
    }
}

Interpreter::Value Interpreter::call(const FunctionCall& c) {
    abort();
}

Interpreter::Value Interpreter::evaluate(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kAppendStage_Kind:
            this->appendStage((const AppendStage&) expr);
            return Value((int) 0xDEADBEEF);
        case Expression::kBinary_Kind: {
            #define ARITHMETIC(op) {                               \
                Value left = this->evaluate(*b.fLeft);             \
                Value right = this->evaluate(*b.fRight);           \
                switch (type_kind(b.fLeft->fType)) {               \
                    case kFloat_TypeKind:                          \
                        return Value(left.fFloat op right.fFloat); \
                    case kInt_TypeKind:                            \
                        return Value(left.fInt op right.fInt);     \
                    default:                                       \
                        abort();                                   \
                }                                                  \
            }
            #define BITWISE(op) {                                  \
                Value left = this->evaluate(*b.fLeft);             \
                Value right = this->evaluate(*b.fRight);           \
                switch (type_kind(b.fLeft->fType)) {               \
                    case kInt_TypeKind:                            \
                        return Value(left.fInt op right.fInt);     \
                    default:                                       \
                        abort();                                   \
                }                                                  \
            }
            #define LOGIC(op) {                                    \
                Value left = this->evaluate(*b.fLeft);             \
                Value right = this->evaluate(*b.fRight);           \
                switch (type_kind(b.fLeft->fType)) {               \
                    case kFloat_TypeKind:                          \
                        return Value(left.fFloat op right.fFloat); \
                    case kInt_TypeKind:                            \
                        return Value(left.fInt op right.fInt);     \
                    default:                                       \
                        abort();                                   \
                }                                                  \
            }
            #define COMPOUND_ARITHMETIC(op) {                      \
                StackIndex left = this->getLValue(*b.fLeft);       \
                Value right = this->evaluate(*b.fRight);           \
                Value result = fStack[left];                       \
                switch (type_kind(b.fLeft->fType)) {               \
                    case kFloat_TypeKind:                          \
                        result.fFloat op right.fFloat;             \
                        break;                                     \
                    case kInt_TypeKind:                            \
                        result.fInt op right.fInt;                 \
                        break;                                     \
                    default:                                       \
                        abort();                                   \
                }                                                  \
                fStack[left] = result;                             \
                return result;                                     \
            }
            #define COMPOUND_BITWISE(op) {                         \
                StackIndex left = this->getLValue(*b.fLeft);       \
                Value right = this->evaluate(*b.fRight);           \
                Value result = fStack[left];                       \
                switch (type_kind(b.fLeft->fType)) {               \
                    case kInt_TypeKind:                            \
                        result.fInt op right.fInt;                 \
                        break;                                     \
                    default:                                       \
                        abort();                                   \
                }                                                  \
                fStack[left] = result;                             \
                return result;                                     \
            }
            const BinaryExpression& b = (const BinaryExpression&) expr;
            switch (b.fOperator) {
                case Token::PLUS:       ARITHMETIC(+)
                case Token::MINUS:      ARITHMETIC(-)
                case Token::STAR:       ARITHMETIC(*)
                case Token::SLASH:      ARITHMETIC(/)
                case Token::BITWISEAND: BITWISE(&)
                case Token::BITWISEOR:  BITWISE(|)
                case Token::BITWISEXOR: BITWISE(^)
                case Token::LT:         LOGIC(<)
                case Token::GT:         LOGIC(>)
                case Token::LTEQ:       LOGIC(<=)
                case Token::GTEQ:       LOGIC(>=)
                case Token::LOGICALAND: {
                    Value result = this->evaluate(*b.fLeft);
                    if (result.fBool) {
                        result = this->evaluate(*b.fRight);
                    }
                    return result;
                }
                case Token::LOGICALOR: {
                    Value result = this->evaluate(*b.fLeft);
                    if (!result.fBool) {
                        result = this->evaluate(*b.fRight);
                    }
                    return result;
                }
                case Token::EQ: {
                    StackIndex left = this->getLValue(*b.fLeft);
                    Value right = this->evaluate(*b.fRight);
                    fStack[left] = right;
                    return right;
                }
                case Token::PLUSEQ:       COMPOUND_ARITHMETIC(+=)
                case Token::MINUSEQ:      COMPOUND_ARITHMETIC(-=)
                case Token::STAREQ:       COMPOUND_ARITHMETIC(*=)
                case Token::SLASHEQ:      COMPOUND_ARITHMETIC(/=)
                case Token::BITWISEANDEQ: COMPOUND_BITWISE(&=)
                case Token::BITWISEOREQ:  COMPOUND_BITWISE(|=)
                case Token::BITWISEXOREQ: COMPOUND_BITWISE(^=)
                default:
                    ABORT("unsupported operator: %s\n", expr.description().c_str());
            }
            break;
        }
        case Expression::kBoolLiteral_Kind:
            return Value(((const BoolLiteral&) expr).fValue);
        case Expression::kConstructor_Kind:
            break;
        case Expression::kIntLiteral_Kind:
            return Value((int) ((const IntLiteral&) expr).fValue);
        case Expression::kFieldAccess_Kind:
            break;
        case Expression::kFloatLiteral_Kind:
            return Value((float) ((const FloatLiteral&) expr).fValue);
        case Expression::kFunctionCall_Kind:
            return this->call((const FunctionCall&) expr);
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = (const IndexExpression&) expr;
            StackIndex pos = this->evaluate(*idx.fBase).fInt +
                             this->evaluate(*idx.fIndex).fInt;
            return fStack[pos];
        }
        case Expression::kPrefix_Kind: {
            const PrefixExpression& p = (const PrefixExpression&) expr;
            switch (p.fOperator) {
                case Token::MINUS: {
                    Value base = this->evaluate(*p.fOperand);
                    switch (type_kind(p.fType)) {
                        case kFloat_TypeKind:
                            return Value(-base.fFloat);
                        case kInt_TypeKind:
                            return Value(-base.fInt);
                        default:
                            abort();
                    }
                }
                case Token::LOGICALNOT: {
                    Value base = this->evaluate(*p.fOperand);
                    return Value(!base.fBool);
                }
                default:
                    abort();
            }
        }
        case Expression::kPostfix_Kind: {
            const PostfixExpression& p = (const PostfixExpression&) expr;
            StackIndex lvalue = this->getLValue(*p.fOperand);
            Value result = fStack[lvalue];
            switch (type_kind(p.fType)) {
                case kFloat_TypeKind:
                    if (Token::PLUSPLUS == p.fOperator) {
                        ++fStack[lvalue].fFloat;
                    } else {
                        SkASSERT(Token::MINUSMINUS == p.fOperator);
                        --fStack[lvalue].fFloat;
                    }
                    break;
                case kInt_TypeKind:
                    if (Token::PLUSPLUS == p.fOperator) {
                        ++fStack[lvalue].fInt;
                    } else {
                        SkASSERT(Token::MINUSMINUS == p.fOperator);
                        --fStack[lvalue].fInt;
                    }
                    break;
                default:
                    abort();
            }
            return result;
        }
        case Expression::kSetting_Kind:
            break;
        case Expression::kSwizzle_Kind:
            break;
        case Expression::kVariableReference_Kind:
            SkASSERT(fVars.size());
            SkASSERT(fVars.back().find(&((VariableReference&) expr).fVariable) !=
                   fVars.back().end());
            return fStack[fVars.back()[&((VariableReference&) expr).fVariable]];
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = (const TernaryExpression&) expr;
            return this->evaluate(this->evaluate(*t.fTest).fBool ? *t.fIfTrue : *t.fIfFalse);
        }
        case Expression::kTypeReference_Kind:
            break;
        default:
            break;
    }
    ABORT("unsupported expression: %s\n", expr.description().c_str());
}

} // namespace

#endif
