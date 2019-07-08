/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include "src/sksl/SkSLByteCodeGenerator.h"
#include "src/sksl/SkSLCFGGenerator.h"
#include "src/sksl/SkSLCPPCodeGenerator.h"
#include "src/sksl/SkSLGLSLCodeGenerator.h"
#include "src/sksl/SkSLHCodeGenerator.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLMetalCodeGenerator.h"
#include "src/sksl/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/SkSLSPIRVCodeGenerator.h"
#include "src/sksl/ir/SkSLEnum.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#ifdef SK_ENABLE_SPIRV_VALIDATION
#include "spirv-tools/libspirv.hpp"
#endif

// include the built-in shader symbols as static strings

#define STRINGIFY(x) #x

static const char* SKSL_GPU_INCLUDE =
#include "sksl_gpu.inc"
;

static const char* SKSL_INTERP_INCLUDE =
#include "sksl_interp.inc"
;

static const char* SKSL_VERT_INCLUDE =
#include "sksl_vert.inc"
;

static const char* SKSL_FRAG_INCLUDE =
#include "sksl_frag.inc"
;

static const char* SKSL_GEOM_INCLUDE =
#include "sksl_geom.inc"
;

static const char* SKSL_FP_INCLUDE =
#include "sksl_enums.inc"
#include "sksl_fp.inc"
;

static const char* SKSL_PIPELINE_INCLUDE =
#include "sksl_pipeline.inc"
;

namespace SkSL {

Compiler::Compiler(Flags flags)
: fFlags(flags)
, fContext(new Context())
, fErrorCount(0) {
    auto types = std::shared_ptr<SymbolTable>(new SymbolTable(this));
    auto symbols = std::shared_ptr<SymbolTable>(new SymbolTable(types, this));
    fIRGenerator = new IRGenerator(fContext.get(), symbols, *this);
    fTypes = types;
    #define ADD_TYPE(t) types->addWithoutOwnership(fContext->f ## t ## _Type->fName, \
                                                   fContext->f ## t ## _Type.get())
    ADD_TYPE(Void);
    ADD_TYPE(Float);
    ADD_TYPE(Float2);
    ADD_TYPE(Float3);
    ADD_TYPE(Float4);
    ADD_TYPE(Half);
    ADD_TYPE(Half2);
    ADD_TYPE(Half3);
    ADD_TYPE(Half4);
    ADD_TYPE(Double);
    ADD_TYPE(Double2);
    ADD_TYPE(Double3);
    ADD_TYPE(Double4);
    ADD_TYPE(Int);
    ADD_TYPE(Int2);
    ADD_TYPE(Int3);
    ADD_TYPE(Int4);
    ADD_TYPE(UInt);
    ADD_TYPE(UInt2);
    ADD_TYPE(UInt3);
    ADD_TYPE(UInt4);
    ADD_TYPE(Short);
    ADD_TYPE(Short2);
    ADD_TYPE(Short3);
    ADD_TYPE(Short4);
    ADD_TYPE(UShort);
    ADD_TYPE(UShort2);
    ADD_TYPE(UShort3);
    ADD_TYPE(UShort4);
    ADD_TYPE(Byte);
    ADD_TYPE(Byte2);
    ADD_TYPE(Byte3);
    ADD_TYPE(Byte4);
    ADD_TYPE(UByte);
    ADD_TYPE(UByte2);
    ADD_TYPE(UByte3);
    ADD_TYPE(UByte4);
    ADD_TYPE(Bool);
    ADD_TYPE(Bool2);
    ADD_TYPE(Bool3);
    ADD_TYPE(Bool4);
    ADD_TYPE(Float2x2);
    ADD_TYPE(Float2x3);
    ADD_TYPE(Float2x4);
    ADD_TYPE(Float3x2);
    ADD_TYPE(Float3x3);
    ADD_TYPE(Float3x4);
    ADD_TYPE(Float4x2);
    ADD_TYPE(Float4x3);
    ADD_TYPE(Float4x4);
    ADD_TYPE(Half2x2);
    ADD_TYPE(Half2x3);
    ADD_TYPE(Half2x4);
    ADD_TYPE(Half3x2);
    ADD_TYPE(Half3x3);
    ADD_TYPE(Half3x4);
    ADD_TYPE(Half4x2);
    ADD_TYPE(Half4x3);
    ADD_TYPE(Half4x4);
    ADD_TYPE(Double2x2);
    ADD_TYPE(Double2x3);
    ADD_TYPE(Double2x4);
    ADD_TYPE(Double3x2);
    ADD_TYPE(Double3x3);
    ADD_TYPE(Double3x4);
    ADD_TYPE(Double4x2);
    ADD_TYPE(Double4x3);
    ADD_TYPE(Double4x4);
    ADD_TYPE(GenType);
    ADD_TYPE(GenHType);
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
    ADD_TYPE(HVec);
    ADD_TYPE(DVec);
    ADD_TYPE(IVec);
    ADD_TYPE(UVec);
    ADD_TYPE(SVec);
    ADD_TYPE(USVec);
    ADD_TYPE(ByteVec);
    ADD_TYPE(UByteVec);
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
    ADD_TYPE(FragmentProcessor);
    ADD_TYPE(SkRasterPipeline);

    StringFragment skCapsName("sk_Caps");
    Variable* skCaps = new Variable(-1, Modifiers(), skCapsName,
                                    *fContext->fSkCaps_Type, Variable::kGlobal_Storage);
    fIRGenerator->fSymbolTable->add(skCapsName, std::unique_ptr<Symbol>(skCaps));

    StringFragment skArgsName("sk_Args");
    Variable* skArgs = new Variable(-1, Modifiers(), skArgsName,
                                    *fContext->fSkArgs_Type, Variable::kGlobal_Storage);
    fIRGenerator->fSymbolTable->add(skArgsName, std::unique_ptr<Symbol>(skArgs));

    std::vector<std::unique_ptr<ProgramElement>> ignored;
    this->processIncludeFile(Program::kFragment_Kind, SKSL_GPU_INCLUDE, strlen(SKSL_GPU_INCLUDE),
                             symbols, &ignored, &fGpuSymbolTable);
    this->processIncludeFile(Program::kVertex_Kind, SKSL_VERT_INCLUDE, strlen(SKSL_VERT_INCLUDE),
                             fGpuSymbolTable, &fVertexInclude, &fVertexSymbolTable);
    this->processIncludeFile(Program::kFragment_Kind, SKSL_FRAG_INCLUDE, strlen(SKSL_FRAG_INCLUDE),
                             fGpuSymbolTable, &fFragmentInclude, &fFragmentSymbolTable);
    this->processIncludeFile(Program::kGeometry_Kind, SKSL_GEOM_INCLUDE, strlen(SKSL_GEOM_INCLUDE),
                             fGpuSymbolTable, &fGeometryInclude, &fGeometrySymbolTable);
    this->processIncludeFile(Program::kPipelineStage_Kind, SKSL_PIPELINE_INCLUDE,
                             strlen(SKSL_PIPELINE_INCLUDE), fGpuSymbolTable, &fPipelineInclude,
                             &fPipelineSymbolTable);
    this->processIncludeFile(Program::kGeneric_Kind, SKSL_INTERP_INCLUDE,
                             strlen(SKSL_INTERP_INCLUDE), symbols, &fInterpreterInclude,
                             &fInterpreterSymbolTable);
}

Compiler::~Compiler() {
    delete fIRGenerator;
}

void Compiler::processIncludeFile(Program::Kind kind, const char* src, size_t length,
                                  std::shared_ptr<SymbolTable> base,
                                  std::vector<std::unique_ptr<ProgramElement>>* outElements,
                                  std::shared_ptr<SymbolTable>* outSymbolTable) {
    fIRGenerator->fSymbolTable = std::move(base);
    Program::Settings settings;
    fIRGenerator->start(&settings, nullptr);
    fIRGenerator->convertProgram(kind, src, length, *fTypes, outElements);
    if (this->fErrorCount) {
        printf("Unexpected errors: %s\n", this->fErrorText.c_str());
    }
    SkASSERT(!fErrorCount);
    fIRGenerator->fSymbolTable->markAllFunctionsBuiltin();
    *outSymbolTable = fIRGenerator->fSymbolTable;
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
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            break;
        case Expression::kIndex_Kind:
            // see comments in Swizzle
            this->addDefinition(((IndexExpression*) lvalue)->fBase.get(),
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            break;
        case Expression::kFieldAccess_Kind:
            // see comments in Swizzle
            this->addDefinition(((FieldAccess*) lvalue)->fBase.get(),
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            break;
        case Expression::kTernary_Kind:
            // To simplify analysis, we just pretend that we write to both sides of the ternary.
            // This allows for false positives (meaning we fail to detect that a variable might not
            // have been assigned), but is preferable to false negatives.
            this->addDefinition(((TernaryExpression*) lvalue)->fIfTrue.get(),
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            this->addDefinition(((TernaryExpression*) lvalue)->fIfFalse.get(),
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            break;
        case Expression::kExternalValue_Kind:
            break;
        default:
            // not an lvalue, can't happen
            SkASSERT(false);
    }
}

// add local variables defined by this node to the set
void Compiler::addDefinitions(const BasicBlock::Node& node,
                              DefinitionMap* definitions) {
    switch (node.fKind) {
        case BasicBlock::Node::kExpression_Kind: {
            SkASSERT(node.expression());
            const Expression* expr = (Expression*) node.expression()->get();
            switch (expr->fKind) {
                case Expression::kBinary_Kind: {
                    BinaryExpression* b = (BinaryExpression*) expr;
                    if (b->fOperator == Token::EQ) {
                        this->addDefinition(b->fLeft.get(), &b->fRight, definitions);
                    } else if (Compiler::IsAssignment(b->fOperator)) {
                        this->addDefinition(
                                      b->fLeft.get(),
                                      (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                      definitions);

                    }
                    break;
                }
                case Expression::kFunctionCall_Kind: {
                    const FunctionCall& c = (const FunctionCall&) *expr;
                    for (size_t i = 0; i < c.fFunction.fParameters.size(); ++i) {
                        if (c.fFunction.fParameters[i]->fModifiers.fFlags & Modifiers::kOut_Flag) {
                            this->addDefinition(
                                      c.fArguments[i].get(),
                                      (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                      definitions);
                        }
                    }
                    break;
                }
                case Expression::kPrefix_Kind: {
                    const PrefixExpression* p = (PrefixExpression*) expr;
                    if (p->fOperator == Token::MINUSMINUS || p->fOperator == Token::PLUSPLUS) {
                        this->addDefinition(
                                      p->fOperand.get(),
                                      (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                      definitions);
                    }
                    break;
                }
                case Expression::kPostfix_Kind: {
                    const PostfixExpression* p = (PostfixExpression*) expr;
                    if (p->fOperator == Token::MINUSMINUS || p->fOperator == Token::PLUSPLUS) {
                        this->addDefinition(
                                      p->fOperand.get(),
                                      (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                      definitions);
                    }
                    break;
                }
                case Expression::kVariableReference_Kind: {
                    const VariableReference* v = (VariableReference*) expr;
                    if (v->fRefKind != VariableReference::kRead_RefKind) {
                        this->addDefinition(
                                      v,
                                      (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
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
        if (exitId == blockId) {
            continue;
        }
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
                                      (std::unique_ptr<Expression>*) &fContext->fDefined_Expression;
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
                SkASSERT(node.statement());
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
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = (TernaryExpression&) lvalue;
            return !t.fTest->hasSideEffects() && is_dead(*t.fIfTrue) && is_dead(*t.fIfFalse);
        }
        case Expression::kExternalValue_Kind:
            return false;
        default:
            ABORT("invalid lvalue: %s\n", lvalue.description().c_str());
    }
}

/**
 * Returns true if this is an assignment which can be collapsed down to just the right hand side due
 * to a dead target and lack of side effects on the left hand side.
 */
static bool dead_assignment(const BinaryExpression& b) {
    if (!Compiler::IsAssignment(b.fOperator)) {
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
            bool isFloat = c.fType.columns() > 1 ? c.fType.componentType().isFloat()
                                                 : c.fType.isFloat();
            if (c.fType.kind() == Type::kVector_Kind && c.isConstant()) {
                for (int i = 0; i < c.fType.columns(); ++i) {
                    if (isFloat) {
                        if (c.getFVecComponent(i) != value) {
                            return false;
                        }
                    } else if (c.getIVecComponent(i) != value) {
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
    SkASSERT((*target)->fKind == Expression::kBinary_Kind);
    BinaryExpression& bin = (BinaryExpression&) **target;
    SkASSERT(!bin.fLeft->hasSideEffects());
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
    SkASSERT((*iter)->expression() == target);
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
    SkASSERT((*target)->fKind == Expression::kBinary_Kind);
    BinaryExpression& bin = (BinaryExpression&) **target;
    SkASSERT(!bin.fRight->hasSideEffects());
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
    SkASSERT((*iter)->expression() == target);
}

/**
 * Constructs the specified type using a single argument.
 */
static std::unique_ptr<Expression> construct(const Type& type, std::unique_ptr<Expression> v) {
    std::vector<std::unique_ptr<Expression>> args;
    args.push_back(std::move(v));
    auto result = std::unique_ptr<Expression>(new Constructor(-1, type, std::move(args)));
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
    SkASSERT((*(*iter)->expression())->fKind == Expression::kBinary_Kind);
    SkASSERT(type.kind() == Type::kVector_Kind);
    SkASSERT((*otherExpression)->fType.kind() == Type::kScalar_Kind);
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
    SkASSERT(expr);
    if ((*iter)->fConstantPropagation) {
        std::unique_ptr<Expression> optimized = expr->constantPropagate(*fIRGenerator, definitions);
        if (optimized) {
            *outUpdated = true;
            if (!try_replace_expression(&b, iter, &optimized)) {
                *outNeedsRescan = true;
                return;
            }
            SkASSERT((*iter)->fKind == BasicBlock::Node::kExpression_Kind);
            expr = (*iter)->expression()->get();
        }
    }
    switch (expr->fKind) {
        case Expression::kVariableReference_Kind: {
            const VariableReference& ref = (VariableReference&) *expr;
            const Variable& var = ref.fVariable;
            if (ref.refKind() != VariableReference::kWrite_RefKind &&
                ref.refKind() != VariableReference::kPointer_RefKind &&
                var.fStorage == Variable::kLocal_Storage && !definitions[&var] &&
                (*undefinedVariables).find(&var) == (*undefinedVariables).end()) {
                (*undefinedVariables).insert(&var);
                this->error(expr->fOffset,
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
                            // float4(1) * x -> float4(x)
                            vectorize_right(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // 1 * x -> x
                            // 1 * float4(x) -> float4(x)
                            // float4(1) * float4(x) -> float4(x)
                            delete_left(&b, iter, outUpdated, outNeedsRescan);
                        }
                    }
                    else if (is_constant(*bin->fLeft, 0)) {
                        if (bin->fLeft->fType.kind() == Type::kScalar_Kind &&
                            bin->fRight->fType.kind() == Type::kVector_Kind &&
                            !bin->fRight->hasSideEffects()) {
                            // 0 * float4(x) -> float4(0)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // 0 * x -> 0
                            // float4(0) * x -> float4(0)
                            // float4(0) * float4(x) -> float4(0)
                            if (!bin->fRight->hasSideEffects()) {
                                delete_right(&b, iter, outUpdated, outNeedsRescan);
                            }
                        }
                    }
                    else if (is_constant(*bin->fRight, 1)) {
                        if (bin->fLeft->fType.kind() == Type::kScalar_Kind &&
                            bin->fRight->fType.kind() == Type::kVector_Kind) {
                            // x * float4(1) -> float4(x)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x * 1 -> x
                            // float4(x) * 1 -> float4(x)
                            // float4(x) * float4(1) -> float4(x)
                            delete_right(&b, iter, outUpdated, outNeedsRescan);
                        }
                    }
                    else if (is_constant(*bin->fRight, 0)) {
                        if (bin->fLeft->fType.kind() == Type::kVector_Kind &&
                            bin->fRight->fType.kind() == Type::kScalar_Kind &&
                            !bin->fLeft->hasSideEffects()) {
                            // float4(x) * 0 -> float4(0)
                            vectorize_right(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x * 0 -> 0
                            // x * float4(0) -> float4(0)
                            // float4(x) * float4(0) -> float4(0)
                            if (!bin->fLeft->hasSideEffects()) {
                                delete_left(&b, iter, outUpdated, outNeedsRescan);
                            }
                        }
                    }
                    break;
                case Token::PLUS:
                    if (is_constant(*bin->fLeft, 0)) {
                        if (bin->fLeft->fType.kind() == Type::kVector_Kind &&
                            bin->fRight->fType.kind() == Type::kScalar_Kind) {
                            // float4(0) + x -> float4(x)
                            vectorize_right(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // 0 + x -> x
                            // 0 + float4(x) -> float4(x)
                            // float4(0) + float4(x) -> float4(x)
                            delete_left(&b, iter, outUpdated, outNeedsRescan);
                        }
                    } else if (is_constant(*bin->fRight, 0)) {
                        if (bin->fLeft->fType.kind() == Type::kScalar_Kind &&
                            bin->fRight->fType.kind() == Type::kVector_Kind) {
                            // x + float4(0) -> float4(x)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x + 0 -> x
                            // float4(x) + 0 -> float4(x)
                            // float4(x) + float4(0) -> float4(x)
                            delete_right(&b, iter, outUpdated, outNeedsRescan);
                        }
                    }
                    break;
                case Token::MINUS:
                    if (is_constant(*bin->fRight, 0)) {
                        if (bin->fLeft->fType.kind() == Type::kScalar_Kind &&
                            bin->fRight->fType.kind() == Type::kVector_Kind) {
                            // x - float4(0) -> float4(x)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x - 0 -> x
                            // float4(x) - 0 -> float4(x)
                            // float4(x) - float4(0) -> float4(x)
                            delete_right(&b, iter, outUpdated, outNeedsRescan);
                        }
                    }
                    break;
                case Token::SLASH:
                    if (is_constant(*bin->fRight, 1)) {
                        if (bin->fLeft->fType.kind() == Type::kScalar_Kind &&
                            bin->fRight->fType.kind() == Type::kVector_Kind) {
                            // x / float4(1) -> float4(x)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x / 1 -> x
                            // float4(x) / 1 -> float4(x)
                            // float4(x) / float4(1) -> float4(x)
                            delete_right(&b, iter, outUpdated, outNeedsRescan);
                        }
                    } else if (is_constant(*bin->fLeft, 0)) {
                        if (bin->fLeft->fType.kind() == Type::kScalar_Kind &&
                            bin->fRight->fType.kind() == Type::kVector_Kind &&
                            !bin->fRight->hasSideEffects()) {
                            // 0 / float4(x) -> float4(0)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // 0 / x -> 0
                            // float4(0) / x -> float4(0)
                            // float4(0) / float4(x) -> float4(0)
                            if (!bin->fRight->hasSideEffects()) {
                                delete_right(&b, iter, outUpdated, outNeedsRescan);
                            }
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
static bool contains_conditional_break(Statement& s, bool inConditional) {
    switch (s.fKind) {
        case Statement::kBlock_Kind:
            for (const auto& sub : ((Block&) s).fStatements) {
                if (contains_conditional_break(*sub, inConditional)) {
                    return true;
                }
            }
            return false;
        case Statement::kBreak_Kind:
            return inConditional;
        case Statement::kIf_Kind: {
            const IfStatement& i = (IfStatement&) s;
            return contains_conditional_break(*i.fIfTrue, true) ||
                   (i.fIfFalse && contains_conditional_break(*i.fIfFalse, true));
        }
        default:
            return false;
    }
}

// returns true if this statement definitely executes a break at the current level (we ignore
// nested loops and switches, since any breaks inside of them will merely break the loop / switch)
static bool contains_unconditional_break(Statement& s) {
    switch (s.fKind) {
        case Statement::kBlock_Kind:
            for (const auto& sub : ((Block&) s).fStatements) {
                if (contains_unconditional_break(*sub)) {
                    return true;
                }
            }
            return false;
        case Statement::kBreak_Kind:
            return true;
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
                if (contains_conditional_break(*stmt, s->fKind == Statement::kIf_Kind)) {
                    return nullptr;
                }
                if (contains_unconditional_break(*stmt)) {
                    capturing = false;
                    break;
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
    return std::unique_ptr<Statement>(new Block(-1, std::move(statements), s->fSymbols));
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
                    SkASSERT((*iter)->statement()->get() == stmt);
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
                    SkASSERT(i.fIfTrue);
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
                    SkASSERT(c->fValue->fKind == s.fValue->fKind);
                    found = c->fValue->compareConstant(*fContext, *s.fValue);
                    if (found) {
                        std::unique_ptr<Statement> newBlock = block_for_case(&s, c.get());
                        if (newBlock) {
                            (*iter)->setStatement(std::move(newBlock));
                            break;
                        } else {
                            if (s.fIsStatic && !(fFlags & kPermitInvalidStaticTests_Flag)) {
                                this->error(s.fOffset,
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
                                this->error(s.fOffset,
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
            SkASSERT((*iter)->statement()->get() == &e);
            if (!e.fExpression->hasSideEffects()) {
                // Expression statement with no side effects, kill it
                if (!b.tryRemoveExpressionBefore(iter, e.fExpression.get())) {
                    *outNeedsRescan = true;
                }
                SkASSERT((*iter)->statement()->get() == stmt);
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
            int offset;
            switch (cfg.fBlocks[i].fNodes[0].fKind) {
                case BasicBlock::Node::kStatement_Kind:
                    offset = (*cfg.fBlocks[i].fNodes[0].statement())->fOffset;
                    break;
                case BasicBlock::Node::kExpression_Kind:
                    offset = (*cfg.fBlocks[i].fNodes[0].expression())->fOffset;
                    break;
            }
            this->error(offset, String("unreachable"));
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
    SkASSERT(!needsRescan);

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
                            this->error(s.fOffset, "static if has non-static test");
                        }
                        ++iter;
                        break;
                    case Statement::kSwitch_Kind:
                        if (((const SwitchStatement&) s).fIsStatic &&
                             !(fFlags & kPermitInvalidStaticTests_Flag)) {
                            this->error(s.fOffset, "static switch has non-static test");
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
    if (f.fDeclaration.fReturnType != *fContext->fVoid_Type) {
        if (cfg.fBlocks[cfg.fExit].fEntrances.size()) {
            this->error(f.fOffset, String("function can exit without returning a value"));
        }
    }
}

void Compiler::registerExternalValue(ExternalValue* value) {
    fIRGenerator->fRootSymbolTable->addWithoutOwnership(value->fName, value);
}

Symbol* Compiler::takeOwnership(std::unique_ptr<Symbol> symbol) {
    return fIRGenerator->fRootSymbolTable->takeOwnership(std::move(symbol));
}

std::unique_ptr<Program> Compiler::convertProgram(Program::Kind kind, String text,
                                                  const Program::Settings& settings) {
    fErrorText = "";
    fErrorCount = 0;
    std::vector<std::unique_ptr<ProgramElement>>* inherited;
    std::vector<std::unique_ptr<ProgramElement>> elements;
    switch (kind) {
        case Program::kVertex_Kind:
            inherited = &fVertexInclude;
            fIRGenerator->fSymbolTable = fVertexSymbolTable;
            fIRGenerator->start(&settings, inherited);
            break;
        case Program::kFragment_Kind:
            inherited = &fFragmentInclude;
            fIRGenerator->fSymbolTable = fFragmentSymbolTable;
            fIRGenerator->start(&settings, inherited);
            break;
        case Program::kGeometry_Kind:
            inherited = &fGeometryInclude;
            fIRGenerator->fSymbolTable = fGeometrySymbolTable;
            fIRGenerator->start(&settings, inherited);
            break;
        case Program::kFragmentProcessor_Kind:
            inherited = nullptr;
            fIRGenerator->fSymbolTable = fGpuSymbolTable;
            fIRGenerator->start(&settings, nullptr);
            fIRGenerator->convertProgram(kind, SKSL_FP_INCLUDE, strlen(SKSL_FP_INCLUDE), *fTypes,
                                         &elements);
            fIRGenerator->fSymbolTable->markAllFunctionsBuiltin();
            break;
        case Program::kPipelineStage_Kind:
            inherited = &fPipelineInclude;
            fIRGenerator->fSymbolTable = fPipelineSymbolTable;
            fIRGenerator->start(&settings, inherited);
            break;
        case Program::kGeneric_Kind:
            inherited = &fInterpreterInclude;
            fIRGenerator->fSymbolTable = fInterpreterSymbolTable;
            fIRGenerator->start(&settings, inherited);
            break;
    }
    for (auto& element : elements) {
        if (element->fKind == ProgramElement::kEnum_Kind) {
            ((Enum&) *element).fBuiltin = true;
        }
    }
    std::unique_ptr<String> textPtr(new String(std::move(text)));
    fSource = textPtr.get();
    fIRGenerator->convertProgram(kind, textPtr->c_str(), textPtr->size(), *fTypes, &elements);
    auto result = std::unique_ptr<Program>(new Program(kind,
                                                       std::move(textPtr),
                                                       settings,
                                                       fContext,
                                                       inherited,
                                                       std::move(elements),
                                                       fIRGenerator->fSymbolTable,
                                                       fIRGenerator->fInputs));
    if (fErrorCount) {
        return nullptr;
    }
    return result;
}

bool Compiler::optimize(Program& program) {
    SkASSERT(!fErrorCount);
    if (!program.fIsOptimized) {
        program.fIsOptimized = true;
        fIRGenerator->fKind = program.fKind;
        fIRGenerator->fSettings = &program.fSettings;
        for (auto& element : program) {
            if (element.fKind == ProgramElement::kFunction_Kind) {
                this->scanCFG((FunctionDefinition&) element);
            }
        }
        if (program.fKind != Program::kFragmentProcessor_Kind) {
            for (auto iter = program.fElements.begin(); iter != program.fElements.end();) {
                if ((*iter)->fKind == ProgramElement::kVar_Kind) {
                    VarDeclarations& vars = (VarDeclarations&) **iter;
                    for (auto varIter = vars.fVars.begin(); varIter != vars.fVars.end();) {
                        const Variable& var = *((VarDeclaration&) **varIter).fVar;
                        if (var.dead()) {
                            varIter = vars.fVars.erase(varIter);
                        } else {
                            ++varIter;
                        }
                    }
                    if (vars.fVars.size() == 0) {
                        iter = program.fElements.erase(iter);
                        continue;
                    }
                }
                ++iter;
            }
        }
    }
    return fErrorCount == 0;
}

std::unique_ptr<Program> Compiler::specialize(
                   Program& program,
                   const std::unordered_map<SkSL::String, SkSL::Program::Settings::Value>& inputs) {
    std::vector<std::unique_ptr<ProgramElement>> elements;
    for (const auto& e : program) {
        elements.push_back(e.clone());
    }
    Program::Settings settings;
    settings.fCaps = program.fSettings.fCaps;
    for (auto iter = inputs.begin(); iter != inputs.end(); ++iter) {
        settings.fArgs.insert(*iter);
    }
    std::unique_ptr<Program> result(new Program(program.fKind,
                                                nullptr,
                                                settings,
                                                program.fContext,
                                                program.fInheritedElements,
                                                std::move(elements),
                                                program.fSymbols,
                                                program.fInputs));
    return result;
}

#if defined(SKSL_STANDALONE) || SK_SUPPORT_GPU

bool Compiler::toSPIRV(Program& program, OutputStream& out) {
    if (!this->optimize(program)) {
        return false;
    }
#ifdef SK_ENABLE_SPIRV_VALIDATION
    StringStream buffer;
    fSource = program.fSource.get();
    SPIRVCodeGenerator cg(fContext.get(), &program, this, &buffer);
    bool result = cg.generateCode();
    fSource = nullptr;
    if (result) {
        spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_0);
        const String& data = buffer.str();
        SkASSERT(0 == data.size() % 4);
        auto dumpmsg = [](spv_message_level_t, const char*, const spv_position_t&, const char* m) {
            SkDebugf("SPIR-V validation error: %s\n", m);
        };
        tools.SetMessageConsumer(dumpmsg);
        // Verify that the SPIR-V we produced is valid. If this SkASSERT fails, check the logs prior
        // to the failure to see the validation errors.
        SkAssertResult(tools.Validate((const uint32_t*) data.c_str(), data.size() / 4));
        out.write(data.c_str(), data.size());
    }
#else
    fSource = program.fSource.get();
    SPIRVCodeGenerator cg(fContext.get(), &program, this, &out);
    bool result = cg.generateCode();
    fSource = nullptr;
#endif
    return result;
}

bool Compiler::toSPIRV(Program& program, String* out) {
    StringStream buffer;
    bool result = this->toSPIRV(program, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

bool Compiler::toGLSL(Program& program, OutputStream& out) {
    if (!this->optimize(program)) {
        return false;
    }
    fSource = program.fSource.get();
    GLSLCodeGenerator cg(fContext.get(), &program, this, &out);
    bool result = cg.generateCode();
    fSource = nullptr;
    return result;
}

bool Compiler::toGLSL(Program& program, String* out) {
    StringStream buffer;
    bool result = this->toGLSL(program, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

bool Compiler::toMetal(Program& program, OutputStream& out) {
    if (!this->optimize(program)) {
        return false;
    }
    MetalCodeGenerator cg(fContext.get(), &program, this, &out);
    bool result = cg.generateCode();
    return result;
}

bool Compiler::toMetal(Program& program, String* out) {
    if (!this->optimize(program)) {
        return false;
    }
    StringStream buffer;
    bool result = this->toMetal(program, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

bool Compiler::toCPP(Program& program, String name, OutputStream& out) {
    if (!this->optimize(program)) {
        return false;
    }
    fSource = program.fSource.get();
    CPPCodeGenerator cg(fContext.get(), &program, this, name, &out);
    bool result = cg.generateCode();
    fSource = nullptr;
    return result;
}

bool Compiler::toH(Program& program, String name, OutputStream& out) {
    if (!this->optimize(program)) {
        return false;
    }
    fSource = program.fSource.get();
    HCodeGenerator cg(fContext.get(), &program, this, name, &out);
    bool result = cg.generateCode();
    fSource = nullptr;
    return result;
}

bool Compiler::toPipelineStage(const Program& program, String* out,
                               std::vector<FormatArg>* outFormatArgs) {
    SkASSERT(program.fIsOptimized);
    fSource = program.fSource.get();
    StringStream buffer;
    PipelineStageCodeGenerator cg(fContext.get(), &program, this, &buffer, outFormatArgs);
    bool result = cg.generateCode();
    fSource = nullptr;
    if (result) {
        *out = buffer.str();
    }
    return result;
}

#endif

std::unique_ptr<ByteCode> Compiler::toByteCode(Program& program) {
    if (!this->optimize(program)) {
        return nullptr;
    }
    std::unique_ptr<ByteCode> result(new ByteCode());
    ByteCodeGenerator cg(fContext.get(), &program, this, result.get());
    if (cg.generateCode()) {
        return result;
    }
    return nullptr;
}

const char* Compiler::OperatorName(Token::Kind kind) {
    switch (kind) {
        case Token::PLUS:         return "+";
        case Token::MINUS:        return "-";
        case Token::STAR:         return "*";
        case Token::SLASH:        return "/";
        case Token::PERCENT:      return "%";
        case Token::SHL:          return "<<";
        case Token::SHR:          return ">>";
        case Token::LOGICALNOT:   return "!";
        case Token::LOGICALAND:   return "&&";
        case Token::LOGICALOR:    return "||";
        case Token::LOGICALXOR:   return "^^";
        case Token::BITWISENOT:   return "~";
        case Token::BITWISEAND:   return "&";
        case Token::BITWISEOR:    return "|";
        case Token::BITWISEXOR:   return "^";
        case Token::EQ:           return "=";
        case Token::EQEQ:         return "==";
        case Token::NEQ:          return "!=";
        case Token::LT:           return "<";
        case Token::GT:           return ">";
        case Token::LTEQ:         return "<=";
        case Token::GTEQ:         return ">=";
        case Token::PLUSEQ:       return "+=";
        case Token::MINUSEQ:      return "-=";
        case Token::STAREQ:       return "*=";
        case Token::SLASHEQ:      return "/=";
        case Token::PERCENTEQ:    return "%=";
        case Token::SHLEQ:        return "<<=";
        case Token::SHREQ:        return ">>=";
        case Token::LOGICALANDEQ: return "&&=";
        case Token::LOGICALOREQ:  return "||=";
        case Token::LOGICALXOREQ: return "^^=";
        case Token::BITWISEANDEQ: return "&=";
        case Token::BITWISEOREQ:  return "|=";
        case Token::BITWISEXOREQ: return "^=";
        case Token::PLUSPLUS:     return "++";
        case Token::MINUSMINUS:   return "--";
        case Token::COMMA:        return ",";
        default:
            ABORT("unsupported operator: %d\n", kind);
    }
}


bool Compiler::IsAssignment(Token::Kind op) {
    switch (op) {
        case Token::EQ:           // fall through
        case Token::PLUSEQ:       // fall through
        case Token::MINUSEQ:      // fall through
        case Token::STAREQ:       // fall through
        case Token::SLASHEQ:      // fall through
        case Token::PERCENTEQ:    // fall through
        case Token::SHLEQ:        // fall through
        case Token::SHREQ:        // fall through
        case Token::BITWISEOREQ:  // fall through
        case Token::BITWISEXOREQ: // fall through
        case Token::BITWISEANDEQ: // fall through
        case Token::LOGICALOREQ:  // fall through
        case Token::LOGICALXOREQ: // fall through
        case Token::LOGICALANDEQ:
            return true;
        default:
            return false;
    }
}

Position Compiler::position(int offset) {
    SkASSERT(fSource);
    int line = 1;
    int column = 1;
    for (int i = 0; i < offset; i++) {
        if ((*fSource)[i] == '\n') {
            ++line;
            column = 1;
        }
        else {
            ++column;
        }
    }
    return Position(line, column);
}

void Compiler::error(int offset, String msg) {
    fErrorCount++;
    Position pos = this->position(offset);
    fErrorText += "error: " + to_string(pos.fLine) + ": " + msg.c_str() + "\n";
}

String Compiler::errorText() {
    this->writeErrorCount();
    fErrorCount = 0;
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
