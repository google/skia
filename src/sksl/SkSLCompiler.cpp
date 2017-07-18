/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCompiler.h"

#include "SkSLCFGGenerator.h"
#include "SkSLCPPCodeGenerator.h"
#include "SkSLGLSLCodeGenerator.h"
#include "SkSLHCodeGenerator.h"
#include "SkSLIRGenerator.h"
#include "SkSLSPIRVCodeGenerator.h"
#include "ir/SkSLExpression.h"
#include "ir/SkSLExpressionStatement.h"
#include "ir/SkSLIntLiteral.h"
#include "ir/SkSLModifiersDeclaration.h"
#include "ir/SkSLNop.h"
#include "ir/SkSLSymbolTable.h"
#include "ir/SkSLTernaryExpression.h"
#include "ir/SkSLUnresolvedFunction.h"
#include "ir/SkSLVarDeclarations.h"

#ifdef SK_ENABLE_SPIRV_VALIDATION
#include "spirv-tools/libspirv.hpp"
#endif

#define STRINGIFY(x) #x

// include the built-in shader symbols as static strings

static const char* SKSL_INCLUDE =
#include "sksl.include"
;

static const char* SKSL_VERT_INCLUDE =
#include "sksl_vert.include"
;

static const char* SKSL_FRAG_INCLUDE =
#include "sksl_frag.include"
;

static const char* SKSL_GEOM_INCLUDE =
#include "sksl_geom.include"
;

static const char* SKSL_FP_INCLUDE =
#include "sksl_fp.include"
;


namespace SkSL {

Compiler::Compiler(Flags flags)
: fFlags(flags)
, fErrorCount(0) {
    auto types = std::shared_ptr<SymbolTable>(new SymbolTable(this));
    auto symbols = std::shared_ptr<SymbolTable>(new SymbolTable(types, this));
    fIRGenerator = new IRGenerator(&fContext, symbols, *this);
    fTypes = types;
    #define ADD_TYPE(t) types->addWithoutOwnership(fContext.f ## t ## _Type->fName, \
                                                   fContext.f ## t ## _Type.get())
    ADD_TYPE(Void);
    ADD_TYPE(Float);
    ADD_TYPE(Vec2);
    ADD_TYPE(Vec3);
    ADD_TYPE(Vec4);
    ADD_TYPE(Double);
    ADD_TYPE(DVec2);
    ADD_TYPE(DVec3);
    ADD_TYPE(DVec4);
    ADD_TYPE(Int);
    ADD_TYPE(IVec2);
    ADD_TYPE(IVec3);
    ADD_TYPE(IVec4);
    ADD_TYPE(UInt);
    ADD_TYPE(UVec2);
    ADD_TYPE(UVec3);
    ADD_TYPE(UVec4);
    ADD_TYPE(Bool);
    ADD_TYPE(BVec2);
    ADD_TYPE(BVec3);
    ADD_TYPE(BVec4);
    ADD_TYPE(Mat2x2);
    types->addWithoutOwnership(String("mat2x2"), fContext.fMat2x2_Type.get());
    ADD_TYPE(Mat2x3);
    ADD_TYPE(Mat2x4);
    ADD_TYPE(Mat3x2);
    ADD_TYPE(Mat3x3);
    types->addWithoutOwnership(String("mat3x3"), fContext.fMat3x3_Type.get());
    ADD_TYPE(Mat3x4);
    ADD_TYPE(Mat4x2);
    ADD_TYPE(Mat4x3);
    ADD_TYPE(Mat4x4);
    types->addWithoutOwnership(String("mat4x4"), fContext.fMat4x4_Type.get());
    ADD_TYPE(GenType);
    ADD_TYPE(GenDType);
    ADD_TYPE(GenIType);
    ADD_TYPE(GenUType);
    ADD_TYPE(GenBType);
    ADD_TYPE(Mat);
    ADD_TYPE(Vec);
    ADD_TYPE(GVec);
    ADD_TYPE(GVec2);
    ADD_TYPE(GVec3);
    ADD_TYPE(GVec4);
    ADD_TYPE(DVec);
    ADD_TYPE(IVec);
    ADD_TYPE(UVec);
    ADD_TYPE(BVec);

    ADD_TYPE(Sampler1D);
    ADD_TYPE(Sampler2D);
    ADD_TYPE(Sampler3D);
    ADD_TYPE(SamplerExternalOES);
    ADD_TYPE(SamplerCube);
    ADD_TYPE(Sampler2DRect);
    ADD_TYPE(Sampler1DArray);
    ADD_TYPE(Sampler2DArray);
    ADD_TYPE(SamplerCubeArray);
    ADD_TYPE(SamplerBuffer);
    ADD_TYPE(Sampler2DMS);
    ADD_TYPE(Sampler2DMSArray);

    ADD_TYPE(ISampler2D);

    ADD_TYPE(Image2D);
    ADD_TYPE(IImage2D);

    ADD_TYPE(SubpassInput);
    ADD_TYPE(SubpassInputMS);

    ADD_TYPE(GSampler1D);
    ADD_TYPE(GSampler2D);
    ADD_TYPE(GSampler3D);
    ADD_TYPE(GSamplerCube);
    ADD_TYPE(GSampler2DRect);
    ADD_TYPE(GSampler1DArray);
    ADD_TYPE(GSampler2DArray);
    ADD_TYPE(GSamplerCubeArray);
    ADD_TYPE(GSamplerBuffer);
    ADD_TYPE(GSampler2DMS);
    ADD_TYPE(GSampler2DMSArray);

    ADD_TYPE(Sampler1DShadow);
    ADD_TYPE(Sampler2DShadow);
    ADD_TYPE(SamplerCubeShadow);
    ADD_TYPE(Sampler2DRectShadow);
    ADD_TYPE(Sampler1DArrayShadow);
    ADD_TYPE(Sampler2DArrayShadow);
    ADD_TYPE(SamplerCubeArrayShadow);
    ADD_TYPE(GSampler2DArrayShadow);
    ADD_TYPE(GSamplerCubeArrayShadow);
    ADD_TYPE(ColorSpaceXform);

    String skCapsName("sk_Caps");
    Variable* skCaps = new Variable(Position(), Modifiers(), skCapsName,
                                    *fContext.fSkCaps_Type, Variable::kGlobal_Storage);
    fIRGenerator->fSymbolTable->add(skCapsName, std::unique_ptr<Symbol>(skCaps));

    String skArgsName("sk_Args");
    Variable* skArgs = new Variable(Position(), Modifiers(), skArgsName,
                                    *fContext.fSkArgs_Type, Variable::kGlobal_Storage);
    fIRGenerator->fSymbolTable->add(skArgsName, std::unique_ptr<Symbol>(skArgs));

    Modifiers::Flag ignored1;
    std::vector<std::unique_ptr<ProgramElement>> ignored2;
    fIRGenerator->convertProgram(String(SKSL_INCLUDE), *fTypes, &ignored1, &ignored2);
    fIRGenerator->fSymbolTable->markAllFunctionsBuiltin();
    ASSERT(!fErrorCount);
}

Compiler::~Compiler() {
    delete fIRGenerator;
}

// add the definition created by assigning to the lvalue to the definition set
void Compiler::addDefinition(const Expression* lvalue, std::unique_ptr<Expression>* expr,
                             DefinitionMap* definitions) {
    switch (lvalue->fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((VariableReference*) lvalue)->fVariable;
            if (var.fStorage == Variable::kLocal_Storage) {
                (*definitions)[&var] = expr;
            }
            break;
        }
        case Expression::kSwizzle_Kind:
            // We consider the variable written to as long as at least some of its components have
            // been written to. This will lead to some false negatives (we won't catch it if you
            // write to foo.x and then read foo.y), but being stricter could lead to false positives
            // (we write to foo.x, and then pass foo to a function which happens to only read foo.x,
            // but since we pass foo as a whole it is flagged as an error) unless we perform a much
            // more complicated whole-program analysis. This is probably good enough.
            this->addDefinition(((Swizzle*) lvalue)->fBase.get(),
                                (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                definitions);
            break;
        case Expression::kIndex_Kind:
            // see comments in Swizzle
            this->addDefinition(((IndexExpression*) lvalue)->fBase.get(),
                                (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                definitions);
            break;
        case Expression::kFieldAccess_Kind:
            // see comments in Swizzle
            this->addDefinition(((FieldAccess*) lvalue)->fBase.get(),
                                (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                definitions);
            break;
        default:
            // not an lvalue, can't happen
            ASSERT(false);
    }
}

// add local variables defined by this node to the set
void Compiler::addDefinitions(const BasicBlock::Node& node,
                              DefinitionMap* definitions) {
    switch (node.fKind) {
        case BasicBlock::Node::kExpression_Kind: {
            ASSERT(node.expression());
            const Expression* expr = (Expression*) node.expression()->get();
            switch (expr->fKind) {
                case Expression::kBinary_Kind: {
                    BinaryExpression* b = (BinaryExpression*) expr;
                    if (b->fOperator == Token::EQ) {
                        this->addDefinition(b->fLeft.get(), &b->fRight, definitions);
                    } else if (Token::IsAssignment(b->fOperator)) {
                        this->addDefinition(
                                       b->fLeft.get(),
                                       (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                       definitions);

                    }
                    break;
                }
                case Expression::kPrefix_Kind: {
                    const PrefixExpression* p = (PrefixExpression*) expr;
                    if (p->fOperator == Token::MINUSMINUS || p->fOperator == Token::PLUSPLUS) {
                        this->addDefinition(
                                       p->fOperand.get(),
                                       (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                       definitions);
                    }
                    break;
                }
                case Expression::kPostfix_Kind: {
                    const PostfixExpression* p = (PostfixExpression*) expr;
                    if (p->fOperator == Token::MINUSMINUS || p->fOperator == Token::PLUSPLUS) {
                        this->addDefinition(
                                       p->fOperand.get(),
                                       (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                       definitions);
                    }
                    break;
                }
                case Expression::kVariableReference_Kind: {
                    const VariableReference* v = (VariableReference*) expr;
                    if (v->fRefKind != VariableReference::kRead_RefKind) {
                        this->addDefinition(
                                       v,
                                       (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                       definitions);
                    }
                }
                default:
                    break;
            }
            break;
        }
        case BasicBlock::Node::kStatement_Kind: {
            const Statement* stmt = (Statement*) node.statement()->get();
            if (stmt->fKind == Statement::kVarDeclaration_Kind) {
                VarDeclaration& vd = (VarDeclaration&) *stmt;
                if (vd.fValue) {
                    (*definitions)[vd.fVar] = &vd.fValue;
                }
            }
            break;
        }
    }
}

void Compiler::scanCFG(CFG* cfg, BlockId blockId, std::set<BlockId>* workList) {
    BasicBlock& block = cfg->fBlocks[blockId];

    // compute definitions after this block
    DefinitionMap after = block.fBefore;
    for (const BasicBlock::Node& n : block.fNodes) {
        this->addDefinitions(n, &after);
    }

    // propagate definitions to exits
    for (BlockId exitId : block.fExits) {
        BasicBlock& exit = cfg->fBlocks[exitId];
        for (const auto& pair : after) {
            std::unique_ptr<Expression>* e1 = pair.second;
            auto found = exit.fBefore.find(pair.first);
            if (found == exit.fBefore.end()) {
                // exit has no definition for it, just copy it
                workList->insert(exitId);
                exit.fBefore[pair.first] = e1;
            } else {
                // exit has a (possibly different) value already defined
                std::unique_ptr<Expression>* e2 = exit.fBefore[pair.first];
                if (e1 != e2) {
                    // definition has changed, merge and add exit block to worklist
                    workList->insert(exitId);
                    if (e1 && e2) {
                        exit.fBefore[pair.first] =
                                       (std::unique_ptr<Expression>*) &fContext.fDefined_Expression;
                    } else {
                        exit.fBefore[pair.first] = nullptr;
                    }
                }
            }
        }
    }
}

// returns a map which maps all local variables in the function to null, indicating that their value
// is initially unknown
static DefinitionMap compute_start_state(const CFG& cfg) {
    DefinitionMap result;
    for (const auto& block : cfg.fBlocks) {
        for (const auto& node : block.fNodes) {
            if (node.fKind == BasicBlock::Node::kStatement_Kind) {
                ASSERT(node.statement());
                const Statement* s = node.statement()->get();
                if (s->fKind == Statement::kVarDeclarations_Kind) {
                    const VarDeclarationsStatement* vd = (const VarDeclarationsStatement*) s;
                    for (const auto& decl : vd->fDeclaration->fVars) {
                        if (decl->fKind == Statement::kVarDeclaration_Kind) {
                            result[((VarDeclaration&) *decl).fVar] = nullptr;
                        }
                    }
                }
            }
        }
    }
    return result;
}

/**
 * Returns true if assigning to this lvalue has no effect.
 */
static bool is_dead(const Expression& lvalue) {
    switch (lvalue.fKind) {
        case Expression::kVariableReference_Kind:
            return ((VariableReference&) lvalue).fVariable.dead();
        case Expression::kSwizzle_Kind:
            return is_dead(*((Swizzle&) lvalue).fBase);
        case Expression::kFieldAccess_Kind:
            return is_dead(*((FieldAccess&) lvalue).fBase);
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = (IndexExpression&) lvalue;
            return is_dead(*idx.fBase) && !idx.fIndex->hasSideEffects();
        }
        default:
            ABORT("invalid lvalue: %s\n", lvalue.description().c_str());
    }
}

/**
 * Returns true if this is an assignment which can be collapsed down to just the right hand side due
 * to a dead target and lack of side effects on the left hand side.
 */
static bool dead_assignment(const BinaryExpression& b) {
    if (!Token::IsAssignment(b.fOperator)) {
        return false;
    }
    return is_dead(*b.fLeft);
}

void Compiler::computeDataFlow(CFG* cfg) {
    cfg->fBlocks[cfg->fStart].fBefore = compute_start_state(*cfg);
    std::set<BlockId> workList;
    for (BlockId i = 0; i < cfg->fBlocks.size(); i++) {
        workList.insert(i);
    }
    while (workList.size()) {
        BlockId next = *workList.begin();
        workList.erase(workList.begin());
        this->scanCFG(cfg, next, &workList);
    }
}

/**
 * Attempts to replace the expression pointed to by iter with a new one (in both the CFG and the
 * IR). If the expression can be cleanly removed, returns true and updates the iterator to point to
 * the newly-inserted element. Otherwise updates only the IR and returns false (and the CFG will
 * need to be regenerated).
 */
bool try_replace_expression(BasicBlock* b,
                            std::vector<BasicBlock::Node>::iterator* iter,
                            std::unique_ptr<Expression>* newExpression) {
    std::unique_ptr<Expression>* target = (*iter)->expression();
    if (!b->tryRemoveExpression(iter)) {
        *target = std::move(*newExpression);
        return false;
    }
    *target = std::move(*newExpression);
    return b->tryInsertExpression(iter, target);
}

/**
 * Returns true if the expression is a constant numeric literal with the specified value, or a
 * constant vector with all elements equal to the specified value.
 */
bool is_constant(const Expression& expr, double value) {
    switch (expr.fKind) {
        case Expression::kIntLiteral_Kind:
            return ((IntLiteral&) expr).fValue == value;
        case Expression::kFloatLiteral_Kind:
            return ((FloatLiteral&) expr).fValue == value;
        case Expression::kConstructor_Kind: {
            Constructor& c = (Constructor&) expr;
            if (c.fType.kind() == Type::kVector_Kind && c.isConstant()) {
                for (int i = 0; i < c.fType.columns(); ++i) {
                    if (!is_constant(c.getVecComponent(i), value)) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }
        default:
            return false;
    }
}

/**
 * Collapses the binary expression pointed to by iter down to just the right side (in both the IR
 * and CFG structures).
 */
void delete_left(BasicBlock* b,
                 std::vector<BasicBlock::Node>::iterator* iter,
                 bool* outUpdated,
                 bool* outNeedsRescan) {
    *outUpdated = true;
    std::unique_ptr<Expression>* target = (*iter)->expression();
    ASSERT((*target)->fKind == Expression::kBinary_Kind);
    BinaryExpression& bin = (BinaryExpression&) **target;
    bool result;
    if (bin.fOperator == Token::EQ) {
        result = b->tryRemoveLValueBefore(iter, bin.fLeft.get());
    } else {
        result = b->tryRemoveExpressionBefore(iter, bin.fLeft.get());
    }
    *target = std::move(bin.fRight);
    if (!result) {
        *outNeedsRescan = true;
        return;
    }
    if (*iter == b->fNodes.begin()) {
        *outNeedsRescan = true;
        return;
    }
    --(*iter);
    if ((*iter)->fKind != BasicBlock::Node::kExpression_Kind ||
        (*iter)->expression() != &bin.fRight) {
        *outNeedsRescan = true;
        return;
    }
    *iter = b->fNodes.erase(*iter);
    ASSERT((*iter)->expression() == target);
}

/**
 * Collapses the binary expression pointed to by iter down to just the left side (in both the IR and
 * CFG structures).
 */
void delete_right(BasicBlock* b,
                  std::vector<BasicBlock::Node>::iterator* iter,
                  bool* outUpdated,
                  bool* outNeedsRescan) {
    *outUpdated = true;
    std::unique_ptr<Expression>* target = (*iter)->expression();
    ASSERT((*target)->fKind == Expression::kBinary_Kind);
    BinaryExpression& bin = (BinaryExpression&) **target;
    if (!b->tryRemoveExpressionBefore(iter, bin.fRight.get())) {
        *target = std::move(bin.fLeft);
        *outNeedsRescan = true;
        return;
    }
    *target = std::move(bin.fLeft);
    if (*iter == b->fNodes.begin()) {
        *outNeedsRescan = true;
        return;
    }
    --(*iter);
    if (((*iter)->fKind != BasicBlock::Node::kExpression_Kind ||
        (*iter)->expression() != &bin.fLeft)) {
        *outNeedsRescan = true;
        return;
    }
    *iter = b->fNodes.erase(*iter);
    ASSERT((*iter)->expression() == target);
}

/**
 * Constructs the specified type using a single argument.
 */
static std::unique_ptr<Expression> construct(const Type& type, std::unique_ptr<Expression> v) {
    std::vector<std::unique_ptr<Expression>> args;
    args.push_back(std::move(v));
    auto result = std::unique_ptr<Expression>(new Constructor(Position(), type, std::move(args)));
    return result;
}

/**
 * Used in the implementations of vectorize_left and vectorize_right. Given a vector type and an
 * expression x, deletes the expression pointed to by iter and replaces it with <type>(x).
 */
static void vectorize(BasicBlock* b,
                      std::vector<BasicBlock::Node>::iterator* iter,
                      const Type& type,
                      std::unique_ptr<Expression>* otherExpression,
                      bool* outUpdated,
                      bool* outNeedsRescan) {
    ASSERT((*(*iter)->expression())->fKind == Expression::kBinary_Kind);
    ASSERT(type.kind() == Type::kVector_Kind);
    ASSERT((*otherExpression)->fType.kind() == Type::kScalar_Kind);
    *outUpdated = true;
    std::unique_ptr<Expression>* target = (*iter)->expression();
    if (!b->tryRemoveExpression(iter)) {
        *target = construct(type, std::move(*otherExpression));
        *outNeedsRescan = true;
    } else {
        *target = construct(type, std::move(*otherExpression));
        if (!b->tryInsertExpression(iter, target)) {
            *outNeedsRescan = true;
        }
    }
}

/**
 * Given a binary expression of the form x <op> vec<n>(y), deletes the right side and vectorizes the
 * left to yield vec<n>(x).
 */
static void vectorize_left(BasicBlock* b,
                           std::vector<BasicBlock::Node>::iterator* iter,
                           bool* outUpdated,
                           bool* outNeedsRescan) {
    BinaryExpression& bin = (BinaryExpression&) **(*iter)->expression();
    vectorize(b, iter, bin.fRight->fType, &bin.fLeft, outUpdated, outNeedsRescan);
}

/**
 * Given a binary expression of the form vec<n>(x) <op> y, deletes the left side and vectorizes the
 * right to yield vec<n>(y).
 */
static void vectorize_right(BasicBlock* b,
                            std::vector<BasicBlock::Node>::iterator* iter,
                            bool* outUpdated,
                            bool* outNeedsRescan) {
    BinaryExpression& bin = (BinaryExpression&) **(*iter)->expression();
    vectorize(b, iter, bin.fLeft->fType, &bin.fRight, outUpdated, outNeedsRescan);
}

// Mark that an expression which we were writing to is no longer being written to
void clear_write(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind: {
            ((VariableReference&) expr).setRefKind(VariableReference::kRead_RefKind);
            break;
        }
        case Expression::kFieldAccess_Kind:
            clear_write(*((FieldAccess&) expr).fBase);
            break;
        case Expression::kSwizzle_Kind:
            clear_write(*((Swizzle&) expr).fBase);
            break;
        case Expression::kIndex_Kind:
            clear_write(*((IndexExpression&) expr).fBase);
            break;
        default:
            ABORT("shouldn't be writing to this kind of expression\n");
            break;
    }
}

void Compiler::simplifyExpression(DefinitionMap& definitions,
                                  BasicBlock& b,
                                  std::vector<BasicBlock::Node>::iterator* iter,
                                  std::unordered_set<const Variable*>* undefinedVariables,
                                  bool* outUpdated,
                                  bool* outNeedsRescan) {
    Expression* expr = (*iter)->expression()->get();
    ASSERT(expr);
    if ((*iter)->fConstantPropagation) {
        std::unique_ptr<Expression> optimized = expr->constantPropagate(*fIRGenerator, definitions);
        if (optimized) {
            *outUpdated = true;
            if (!try_replace_expression(&b, iter, &optimized)) {
                *outNeedsRescan = true;
                return;
            }
            ASSERT((*iter)->fKind == BasicBlock::Node::kExpression_Kind);
            expr = (*iter)->expression()->get();
        }
    }
    switch (expr->fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((VariableReference*) expr)->fVariable;
            if (var.fStorage == Variable::kLocal_Storage && !definitions[&var] &&
                (*undefinedVariables).find(&var) == (*undefinedVariables).end()) {
                (*undefinedVariables).insert(&var);
                this->error(expr->fPosition,
                            "'" + var.fName + "' has not been assigned");
            }
            break;
        }
        case Expression::kTernary_Kind: {
            TernaryExpression* t = (TernaryExpression*) expr;
            if (t->fTest->fKind == Expression::kBoolLiteral_Kind) {
                // ternary has a constant test, replace it with either the true or
                // false branch
                if (((BoolLiteral&) *t->fTest).fValue) {
                    (*iter)->setExpression(std::move(t->fIfTrue));
                } else {
                    (*iter)->setExpression(std::move(t->fIfFalse));
                }
                *outUpdated = true;
                *outNeedsRescan = true;
            }
            break;
        }
        case Expression::kBinary_Kind: {
            BinaryExpression* bin = (BinaryExpression*) expr;
            if (dead_assignment(*bin)) {
                delete_left(&b, iter, outUpdated, outNeedsRescan);
                break;
            }
            // collapse useless expressions like x * 1 or x + 0
            if (((bin->fLeft->fType.kind()  != Type::kScalar_Kind) &&
                 (bin->fLeft->fType.kind()  != Type::kVector_Kind)) ||
                ((bin->fRight->fType.kind() != Type::kScalar_Kind) &&
                 (bin->fRight->fType.kind() != Type::kVector_Kind))) {
                break;
            }
            switch (bin->fOperator) {
                case Token::STAR:
                    if (is_constant(*bin->fLeft, 1)) {
                        if (bin->fLeft->fType.kind() == Type::kVector_Kind &&
                            bin->fRight->fType.kind() == Type::kScalar_Kind) {
                            // vec4(1) * x -> vec4(x)
                            vectorize_right(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // 1 * x -> x
                            // 1 * vec4(x) -> vec4(x)
                            // vec4(1) * vec4(x) -> vec4(x)
                            delete_left(&b, iter, outUpdated, outNeedsRescan);
                        }
                    }
                    else if (is_constant(*bin->fLeft, 0)) {
                        if (bin->fLeft->fType.kind() == Type::kScalar_Kind &&
                            bin->fRight->fType.kind() == Type::kVector_Kind) {
                            // 0 * vec4(x) -> vec4(0)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // 0 * x -> 0
                            // vec4(0) * x -> vec4(0)
                            // vec4(0) * vec4(x) -> vec4(0)
                            delete_right(&b, iter, outUpdated, outNeedsRescan);
                        }
                    }
                    else if (is_constant(*bin->fRight, 1)) {
                        if (bin->fLeft->fType.kind() == Type::kScalar_Kind &&
                            bin->fRight->fType.kind() == Type::kVector_Kind) {
                            // x * vec4(1) -> vec4(x)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x * 1 -> x
                            // vec4(x) * 1 -> vec4(x)
                            // vec4(x) * vec4(1) -> vec4(x)
                            delete_right(&b, iter, outUpdated, outNeedsRescan);
                        }
                    }
                    else if (is_constant(*bin->fRight, 0)) {
                        if (bin->fLeft->fType.kind() == Type::kVector_Kind &&
                            bin->fRight->fType.kind() == Type::kScalar_Kind) {
                            // vec4(x) * 0 -> vec4(0)
                            vectorize_right(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x * 0 -> 0
                            // x * vec4(0) -> vec4(0)
                            // vec4(x) * vec4(0) -> vec4(0)
                            delete_left(&b, iter, outUpdated, outNeedsRescan);
                        }
                    }
                    break;
                case Token::PLUS:
                    if (is_constant(*bin->fLeft, 0)) {
                        if (bin->fLeft->fType.kind() == Type::kVector_Kind &&
                            bin->fRight->fType.kind() == Type::kScalar_Kind) {
                            // vec4(0) + x -> vec4(x)
                            vectorize_right(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // 0 + x -> x
                            // 0 + vec4(x) -> vec4(x)
                            // vec4(0) + vec4(x) -> vec4(x)
                            delete_left(&b, iter, outUpdated, outNeedsRescan);
                        }
                    } else if (is_constant(*bin->fRight, 0)) {
                        if (bin->fLeft->fType.kind() == Type::kScalar_Kind &&
                            bin->fRight->fType.kind() == Type::kVector_Kind) {
                            // x + vec4(0) -> vec4(x)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x + 0 -> x
                            // vec4(x) + 0 -> vec4(x)
                            // vec4(x) + vec4(0) -> vec4(x)
                            delete_right(&b, iter, outUpdated, outNeedsRescan);
                        }
                    }
                    break;
                case Token::MINUS:
                    if (is_constant(*bin->fRight, 0)) {
                        if (bin->fLeft->fType.kind() == Type::kScalar_Kind &&
                            bin->fRight->fType.kind() == Type::kVector_Kind) {
                            // x - vec4(0) -> vec4(x)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x - 0 -> x
                            // vec4(x) - 0 -> vec4(x)
                            // vec4(x) - vec4(0) -> vec4(x)
                            delete_right(&b, iter, outUpdated, outNeedsRescan);
                        }
                    }
                    break;
                case Token::SLASH:
                    if (is_constant(*bin->fRight, 1)) {
                        if (bin->fLeft->fType.kind() == Type::kScalar_Kind &&
                            bin->fRight->fType.kind() == Type::kVector_Kind) {
                            // x / vec4(1) -> vec4(x)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x / 1 -> x
                            // vec4(x) / 1 -> vec4(x)
                            // vec4(x) / vec4(1) -> vec4(x)
                            delete_right(&b, iter, outUpdated, outNeedsRescan);
                        }
                    } else if (is_constant(*bin->fLeft, 0)) {
                        if (bin->fLeft->fType.kind() == Type::kScalar_Kind &&
                            bin->fRight->fType.kind() == Type::kVector_Kind) {
                            // 0 / vec4(x) -> vec4(0)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // 0 / x -> 0
                            // vec4(0) / x -> vec4(0)
                            // vec4(0) / vec4(x) -> vec4(0)
                            delete_right(&b, iter, outUpdated, outNeedsRescan);
                        }
                    }
                    break;
                case Token::PLUSEQ:
                    if (is_constant(*bin->fRight, 0)) {
                        clear_write(*bin->fLeft);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                case Token::MINUSEQ:
                    if (is_constant(*bin->fRight, 0)) {
                        clear_write(*bin->fLeft);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                case Token::STAREQ:
                    if (is_constant(*bin->fRight, 1)) {
                        clear_write(*bin->fLeft);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                case Token::SLASHEQ:
                    if (is_constant(*bin->fRight, 1)) {
                        clear_write(*bin->fLeft);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                default:
                    break;
            }
        }
        default:
            break;
    }
}

// returns true if this statement could potentially execute a break at the current level (we ignore
// nested loops and switches, since any breaks inside of them will merely break the loop / switch)
static bool contains_break(Statement& s) {
    switch (s.fKind) {
        case Statement::kBlock_Kind:
            for (const auto& sub : ((Block&) s).fStatements) {
                if (contains_break(*sub)) {
                    return true;
                }
            }
            return false;
        case Statement::kBreak_Kind:
            return true;
        case Statement::kIf_Kind: {
            const IfStatement& i = (IfStatement&) s;
            return contains_break(*i.fIfTrue) || (i.fIfFalse && contains_break(*i.fIfFalse));
        }
        default:
            return false;
    }
}

// Returns a block containing all of the statements that will be run if the given case matches
// (which, owing to the statements being owned by unique_ptrs, means the switch itself will be
// broken by this call and must then be discarded).
// Returns null (and leaves the switch unmodified) if no such simple reduction is possible, such as
// when break statements appear inside conditionals.
static std::unique_ptr<Statement> block_for_case(SwitchStatement* s, SwitchCase* c) {
    bool capturing = false;
    std::vector<std::unique_ptr<Statement>*> statementPtrs;
    for (const auto& current : s->fCases) {
        if (current.get() == c) {
            capturing = true;
        }
        if (capturing) {
            for (auto& stmt : current->fStatements) {
                if (stmt->fKind == Statement::kBreak_Kind) {
                    capturing = false;
                    break;
                }
                if (contains_break(*stmt)) {
                    return nullptr;
                }
                statementPtrs.push_back(&stmt);
            }
            if (!capturing) {
                break;
            }
        }
    }
    std::vector<std::unique_ptr<Statement>> statements;
    for (const auto& s : statementPtrs) {
        statements.push_back(std::move(*s));
    }
    return std::unique_ptr<Statement>(new Block(Position(), std::move(statements), s->fSymbols));
}

void Compiler::simplifyStatement(DefinitionMap& definitions,
                                 BasicBlock& b,
                                 std::vector<BasicBlock::Node>::iterator* iter,
                                 std::unordered_set<const Variable*>* undefinedVariables,
                                 bool* outUpdated,
                                 bool* outNeedsRescan) {
    Statement* stmt = (*iter)->statement()->get();
    switch (stmt->fKind) {
        case Statement::kVarDeclaration_Kind: {
            const auto& varDecl = (VarDeclaration&) *stmt;
            if (varDecl.fVar->dead() &&
                (!varDecl.fValue ||
                 !varDecl.fValue->hasSideEffects())) {
                if (varDecl.fValue) {
                    ASSERT((*iter)->statement()->get() == stmt);
                    if (!b.tryRemoveExpressionBefore(iter, varDecl.fValue.get())) {
                        *outNeedsRescan = true;
                    }
                }
                (*iter)->setStatement(std::unique_ptr<Statement>(new Nop()));
                *outUpdated = true;
            }
            break;
        }
        case Statement::kIf_Kind: {
            IfStatement& i = (IfStatement&) *stmt;
            if (i.fTest->fKind == Expression::kBoolLiteral_Kind) {
                // constant if, collapse down to a single branch
                if (((BoolLiteral&) *i.fTest).fValue) {
                    ASSERT(i.fIfTrue);
                    (*iter)->setStatement(std::move(i.fIfTrue));
                } else {
                    if (i.fIfFalse) {
                        (*iter)->setStatement(std::move(i.fIfFalse));
                    } else {
                        (*iter)->setStatement(std::unique_ptr<Statement>(new Nop()));
                    }
                }
                *outUpdated = true;
                *outNeedsRescan = true;
                break;
            }
            if (i.fIfFalse && i.fIfFalse->isEmpty()) {
                // else block doesn't do anything, remove it
                i.fIfFalse.reset();
                *outUpdated = true;
                *outNeedsRescan = true;
            }
            if (!i.fIfFalse && i.fIfTrue->isEmpty()) {
                // if block doesn't do anything, no else block
                if (i.fTest->hasSideEffects()) {
                    // test has side effects, keep it
                    (*iter)->setStatement(std::unique_ptr<Statement>(
                                                      new ExpressionStatement(std::move(i.fTest))));
                } else {
                    // no if, no else, no test side effects, kill the whole if
                    // statement
                    (*iter)->setStatement(std::unique_ptr<Statement>(new Nop()));
                }
                *outUpdated = true;
                *outNeedsRescan = true;
            }
            break;
        }
        case Statement::kSwitch_Kind: {
            SwitchStatement& s = (SwitchStatement&) *stmt;
            if (s.fValue->isConstant()) {
                // switch is constant, replace it with the case that matches
                bool found = false;
                SwitchCase* defaultCase = nullptr;
                for (const auto& c : s.fCases) {
                    if (!c->fValue) {
                        defaultCase = c.get();
                        continue;
                    }
                    ASSERT(c->fValue->fKind == s.fValue->fKind);
                    found = c->fValue->compareConstant(fContext, *s.fValue);
                    if (found) {
                        std::unique_ptr<Statement> newBlock = block_for_case(&s, c.get());
                        if (newBlock) {
                            (*iter)->setStatement(std::move(newBlock));
                            break;
                        } else {
                            if (s.fIsStatic && !(fFlags & kPermitInvalidStaticTests_Flag)) {
                                this->error(s.fPosition,
                                            "static switch contains non-static conditional break");
                                s.fIsStatic = false;
                            }
                            return; // can't simplify
                        }
                    }
                }
                if (!found) {
                    // no matching case. use default if it exists, or kill the whole thing
                    if (defaultCase) {
                        std::unique_ptr<Statement> newBlock = block_for_case(&s, defaultCase);
                        if (newBlock) {
                            (*iter)->setStatement(std::move(newBlock));
                        } else {
                            if (s.fIsStatic && !(fFlags & kPermitInvalidStaticTests_Flag)) {
                                this->error(s.fPosition,
                                            "static switch contains non-static conditional break");
                                s.fIsStatic = false;
                            }
                            return; // can't simplify
                        }
                    } else {
                        (*iter)->setStatement(std::unique_ptr<Statement>(new Nop()));
                    }
                }
                *outUpdated = true;
                *outNeedsRescan = true;
            }
            break;
        }
        case Statement::kExpression_Kind: {
            ExpressionStatement& e = (ExpressionStatement&) *stmt;
            ASSERT((*iter)->statement()->get() == &e);
            if (!e.fExpression->hasSideEffects()) {
                // Expression statement with no side effects, kill it
                if (!b.tryRemoveExpressionBefore(iter, e.fExpression.get())) {
                    *outNeedsRescan = true;
                }
                ASSERT((*iter)->statement()->get() == stmt);
                (*iter)->setStatement(std::unique_ptr<Statement>(new Nop()));
                *outUpdated = true;
            }
            break;
        }
        default:
            break;
    }
}

void Compiler::scanCFG(FunctionDefinition& f) {
    CFG cfg = CFGGenerator().getCFG(f);
    this->computeDataFlow(&cfg);

    // check for unreachable code
    for (size_t i = 0; i < cfg.fBlocks.size(); i++) {
        if (i != cfg.fStart && !cfg.fBlocks[i].fEntrances.size() &&
            cfg.fBlocks[i].fNodes.size()) {
            Position p;
            switch (cfg.fBlocks[i].fNodes[0].fKind) {
                case BasicBlock::Node::kStatement_Kind:
                    p = (*cfg.fBlocks[i].fNodes[0].statement())->fPosition;
                    break;
                case BasicBlock::Node::kExpression_Kind:
                    p = (*cfg.fBlocks[i].fNodes[0].expression())->fPosition;
                    break;
            }
            this->error(p, String("unreachable"));
        }
    }
    if (fErrorCount) {
        return;
    }

    // check for dead code & undefined variables, perform constant propagation
    std::unordered_set<const Variable*> undefinedVariables;
    bool updated;
    bool needsRescan = false;
    do {
        if (needsRescan) {
            cfg = CFGGenerator().getCFG(f);
            this->computeDataFlow(&cfg);
            needsRescan = false;
        }

        updated = false;
        for (BasicBlock& b : cfg.fBlocks) {
            DefinitionMap definitions = b.fBefore;

            for (auto iter = b.fNodes.begin(); iter != b.fNodes.end() && !needsRescan; ++iter) {
                if (iter->fKind == BasicBlock::Node::kExpression_Kind) {
                    this->simplifyExpression(definitions, b, &iter, &undefinedVariables, &updated,
                                             &needsRescan);
                } else {
                    this->simplifyStatement(definitions, b, &iter, &undefinedVariables, &updated,
                                             &needsRescan);
                }
                if (needsRescan) {
                    break;
                }
                this->addDefinitions(*iter, &definitions);
            }
        }
    } while (updated);
    ASSERT(!needsRescan);

    // verify static ifs & switches, clean up dead variable decls
    for (BasicBlock& b : cfg.fBlocks) {
        DefinitionMap definitions = b.fBefore;

        for (auto iter = b.fNodes.begin(); iter != b.fNodes.end() && !needsRescan;) {
            if (iter->fKind == BasicBlock::Node::kStatement_Kind) {
                const Statement& s = **iter->statement();
                switch (s.fKind) {
                    case Statement::kIf_Kind:
                        if (((const IfStatement&) s).fIsStatic &&
                            !(fFlags & kPermitInvalidStaticTests_Flag)) {
                            this->error(s.fPosition, "static if has non-static test");
                        }
                        ++iter;
                        break;
                    case Statement::kSwitch_Kind:
                        if (((const SwitchStatement&) s).fIsStatic &&
                             !(fFlags & kPermitInvalidStaticTests_Flag)) {
                            this->error(s.fPosition, "static switch has non-static test");
                        }
                        ++iter;
                        break;
                    case Statement::kVarDeclarations_Kind: {
                        VarDeclarations& decls = *((VarDeclarationsStatement&) s).fDeclaration;
                        for (auto varIter = decls.fVars.begin(); varIter != decls.fVars.end();) {
                            if ((*varIter)->fKind == Statement::kNop_Kind) {
                                varIter = decls.fVars.erase(varIter);
                            } else {
                                ++varIter;
                            }
                        }
                        if (!decls.fVars.size()) {
                            iter = b.fNodes.erase(iter);
                        } else {
                            ++iter;
                        }
                        break;
                    }
                    default:
                        ++iter;
                        break;
                }
            } else {
                ++iter;
            }
        }
    }

    // check for missing return
    if (f.fDeclaration.fReturnType != *fContext.fVoid_Type) {
        if (cfg.fBlocks[cfg.fExit].fEntrances.size()) {
            this->error(f.fPosition, String("function can exit without returning a value"));
        }
    }
}

std::unique_ptr<Program> Compiler::convertProgram(Program::Kind kind, String text,
                                                  const Program::Settings& settings) {
    fErrorText = "";
    fErrorCount = 0;
    fIRGenerator->start(&settings);
    std::vector<std::unique_ptr<ProgramElement>> elements;
    Modifiers::Flag ignored;
    switch (kind) {
        case Program::kVertex_Kind:
            fIRGenerator->convertProgram(String(SKSL_VERT_INCLUDE), *fTypes, &ignored, &elements);
            break;
        case Program::kFragment_Kind:
            fIRGenerator->convertProgram(String(SKSL_FRAG_INCLUDE), *fTypes, &ignored, &elements);
            break;
        case Program::kGeometry_Kind:
            fIRGenerator->convertProgram(String(SKSL_GEOM_INCLUDE), *fTypes, &ignored, &elements);
            break;
        case Program::kFragmentProcessor_Kind:
            fIRGenerator->convertProgram(String(SKSL_FP_INCLUDE), *fTypes, &ignored, &elements);
            break;
    }
    fIRGenerator->fSymbolTable->markAllFunctionsBuiltin();
    Modifiers::Flag defaultPrecision;
    fIRGenerator->convertProgram(text, *fTypes, &defaultPrecision, &elements);
    if (!fErrorCount) {
        for (auto& element : elements) {
            if (element->fKind == ProgramElement::kFunction_Kind) {
                this->scanCFG((FunctionDefinition&) *element);
            }
        }
    }
    auto result = std::unique_ptr<Program>(new Program(kind, settings, defaultPrecision, &fContext,
                                                       std::move(elements),
                                                       fIRGenerator->fSymbolTable,
                                                       fIRGenerator->fInputs));
    fIRGenerator->finish();
    this->writeErrorCount();
    if (fErrorCount) {
        return nullptr;
    }
    return result;
}

bool Compiler::toSPIRV(const Program& program, OutputStream& out) {
#ifdef SK_ENABLE_SPIRV_VALIDATION
    StringStream buffer;
    SPIRVCodeGenerator cg(&fContext, &program, this, &buffer);
    bool result = cg.generateCode();
    if (result) {
        spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_0);
        const String& data = buffer.str();
        ASSERT(0 == data.size() % 4);
        auto dumpmsg = [](spv_message_level_t, const char*, const spv_position_t&, const char* m) {
            SkDebugf("SPIR-V validation error: %s\n", m);
        };
        tools.SetMessageConsumer(dumpmsg);
        // Verify that the SPIR-V we produced is valid. If this assert fails, check the logs prior
        // to the failure to see the validation errors.
        ASSERT_RESULT(tools.Validate((const uint32_t*) data.c_str(), data.size() / 4));
        out.write(data.c_str(), data.size());
    }
#else
    SPIRVCodeGenerator cg(&fContext, &program, this, &out);
    bool result = cg.generateCode();
#endif
    this->writeErrorCount();
    return result;
}

bool Compiler::toSPIRV(const Program& program, String* out) {
    StringStream buffer;
    bool result = this->toSPIRV(program, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

bool Compiler::toGLSL(const Program& program, OutputStream& out) {
    GLSLCodeGenerator cg(&fContext, &program, this, &out);
    bool result = cg.generateCode();
    this->writeErrorCount();
    return result;
}

bool Compiler::toGLSL(const Program& program, String* out) {
    StringStream buffer;
    bool result = this->toGLSL(program, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

bool Compiler::toCPP(const Program& program, String name, OutputStream& out) {
    CPPCodeGenerator cg(&fContext, &program, this, name, &out);
    bool result = cg.generateCode();
    this->writeErrorCount();
    return result;
}

bool Compiler::toH(const Program& program, String name, OutputStream& out) {
    HCodeGenerator cg(&program, this, name, &out);
    bool result = cg.generateCode();
    this->writeErrorCount();
    return result;
}

void Compiler::error(Position position, String msg) {
    fErrorCount++;
    fErrorText += "error: " + position.description() + ": " + msg.c_str() + "\n";
}

String Compiler::errorText() {
    String result = fErrorText;
    return result;
}

void Compiler::writeErrorCount() {
    if (fErrorCount) {
        fErrorText += to_string(fErrorCount) + " error";
        if (fErrorCount > 1) {
            fErrorText += "s";
        }
        fErrorText += "\n";
    }
}

} // namespace
