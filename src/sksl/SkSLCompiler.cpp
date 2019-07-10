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
, fErrorCount(0) {
    fIRGenerator = new IRGenerator(*this);
    auto types = std::shared_ptr<SymbolTable>(new SymbolTable(fIRGenerator));
    auto symbols = std::shared_ptr<SymbolTable>(new SymbolTable(types, fIRGenerator));
    fTypes = types;
    #define ADD_TYPE(t) types->add(this->context().f ## t ## _Type.typeNode().fName, \
                                   this->context().f ## t ## _Type)
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

    std::vector<IRNode::ID> ignored;
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

    StringFragment skCapsName("sk_Caps");
    IRNode::ID skCaps = fIRGenerator->createNode(new Variable(fIRGenerator, -1, Modifiers(),
                                                              skCapsName,
                                                              this->context().fSkCaps_Type,
                                                              Variable::kGlobal_Storage,
                                                              IRNode::ID()));
    fGpuSymbolTable->add(skCapsName, skCaps);

    StringFragment skArgsName("sk_Args");
    IRNode::ID skArgs = fIRGenerator->createNode(new Variable(fIRGenerator, -1, Modifiers(),
                                                              skArgsName,
                                                              this->context().fSkArgs_Type,
                                                              Variable::kGlobal_Storage,
                                                              IRNode::ID()));
    fGpuSymbolTable->add(skArgsName, skArgs);
}

Compiler::~Compiler() {
    delete fIRGenerator;
}

const Context& Compiler::context() {
    return fIRGenerator->fContext;
}

void Compiler::processIncludeFile(Program::Kind kind, const char* src, size_t length,
                                  std::shared_ptr<SymbolTable> base,
                                  std::vector<IRNode::ID>* outElements,
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
void Compiler::addDefinition(IRNode::ID lvalueID, IRNode::ID expr,
                             DefinitionMap* definitions) {
    Expression& lvalue = lvalueID.expressionNode();
    const Context& context = fIRGenerator->fContext;
    switch (lvalue.fKind) {
        case Expression::kVariableReference_Kind: {
            IRNode::ID varID = ((VariableReference&) lvalue).fVariable;
            if (((Variable&) varID.node()).fStorage == Variable::kLocal_Storage) {
                (*definitions)[varID] = expr;
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
            this->addDefinition(((Swizzle&) lvalue).fBase, context.fDefined_Expression,
                                definitions);
            break;
        case Expression::kIndex_Kind:
            // see comments in Swizzle
            this->addDefinition(((IndexExpression&) lvalue).fBase, context.fDefined_Expression,
                                definitions);
            break;
        case Expression::kFieldAccess_Kind:
            // see comments in Swizzle
            this->addDefinition(((FieldAccess&) lvalue).fBase, context.fDefined_Expression,
                                definitions);
            break;
        case Expression::kTernary_Kind:
            // To simplify analysis, we just pretend that we write to both sides of the ternary.
            // This allows for false positives (meaning we fail to detect that a variable might not
            // have been assigned), but is preferable to false negatives.
            this->addDefinition(((TernaryExpression&) lvalue).fIfTrue,
                                context.fDefined_Expression,
                                definitions);
            this->addDefinition(((TernaryExpression&) lvalue).fIfFalse,
                                context.fDefined_Expression,
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
    const Context& context = fIRGenerator->fContext;
    switch (node.fKind) {
        case BasicBlock::Node::kExpression_Kind: {
            SkASSERT(node.expression());
            const Expression& expr = node.expression();
            switch (expr.fKind) {
                case Expression::kBinary_Kind: {
                    const BinaryExpression& b = (BinaryExpression&) expr;
                    if (b.fOperator == Token::EQ) {
                        this->addDefinition(b.fLeft, b.fRight, definitions);
                    } else if (Compiler::IsAssignment(b.fOperator)) {
                        this->addDefinition(b.fLeft, context.fDefined_Expression, definitions);

                    }
                    break;
                }
                case Expression::kFunctionCall_Kind: {
                    const FunctionCall& c = (FunctionCall&) expr;
                    const FunctionDeclaration& f = (FunctionDeclaration&) c.fFunction.node();
                    for (size_t i = 0; i < f.fParameters.size(); ++i) {
                        Variable& param = (Variable&) f.fParameters[i].node();
                        if (param.fModifiers.fFlags & Modifiers::kOut_Flag) {
                            this->addDefinition(c.fArguments[i], context.fDefined_Expression,
                                                definitions);
                        }
                    }
                    break;
                }
                case Expression::kPrefix_Kind: {
                    const PrefixExpression& p = (PrefixExpression&) expr;
                    if (p.fOperator == Token::MINUSMINUS || p.fOperator == Token::PLUSPLUS) {
                        this->addDefinition(p.fOperand, context.fDefined_Expression, definitions);
                    }
                    break;
                }
                case Expression::kPostfix_Kind: {
                    const PostfixExpression& p = (PostfixExpression&) expr;
                    if (p.fOperator == Token::MINUSMINUS || p.fOperator == Token::PLUSPLUS) {
                        this->addDefinition(p.fOperand, context.fDefined_Expression, definitions);
                    }
                    break;
                }
                case Expression::kVariableReference_Kind: {
                    const VariableReference& v = (VariableReference&) expr;
                    if (v.fRefKind != kRead_RefKind) {
                        this->addDefinition(node.id(), context.fDefined_Expression, definitions);
                    }
                }
                default:
                    break;
            }
            break;
        }
        case BasicBlock::Node::kStatement_Kind: {
            const Statement& stmt = node.statement();
            if (stmt.fKind == Statement::kVarDeclaration_Kind) {
                VarDeclaration& vd = (VarDeclaration&) stmt;
                if (vd.fValue) {
                    (*definitions)[vd.fVar] = vd.fValue;
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
            IRNode::ID e1 = pair.second;
            auto found = exit.fBefore.find(pair.first);
            if (found == exit.fBefore.end()) {
                // exit has no definition for it, just copy it
                workList->insert(exitId);
                exit.fBefore[pair.first] = e1;
            } else {
                // exit has a (possibly different) value already defined
                IRNode::ID e2 = exit.fBefore[pair.first];
                if (e1 != e2) {
                    // definition has changed, merge and add exit block to worklist
                    workList->insert(exitId);
                    if (e1 && e2) {
                        exit.fBefore[pair.first] = fIRGenerator->fContext.fDefined_Expression;
                    } else {
                        exit.fBefore[pair.first] = IRNode::ID();
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
                const Statement& s = node.statement();
                if (s.fKind == Statement::kVarDeclarations_Kind) {
                    const VarDeclarationsStatement& vd = (VarDeclarationsStatement&) s;
                    for (IRNode::ID decl : ((VarDeclarations&) vd.fDeclaration.node()).fVars) {
                        if (decl.statementNode().fKind == Statement::kVarDeclaration_Kind) {
                            result[((VarDeclaration&) decl.node()).fVar] = IRNode::ID();
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
            return ((Variable&) ((VariableReference&) lvalue).fVariable.node()).dead();
        case Expression::kSwizzle_Kind:
            return is_dead(((Swizzle&) lvalue).fBase.expressionNode());
        case Expression::kFieldAccess_Kind:
            return is_dead(((FieldAccess&) lvalue).fBase.expressionNode());
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = (IndexExpression&) lvalue;
            return is_dead(idx.fBase.expressionNode()) &&
                   !idx.fIndex.expressionNode().hasSideEffects();
        }
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = (TernaryExpression&) lvalue;
            return !t.fTest.expressionNode().hasSideEffects() &&
                   is_dead(t.fIfTrue.expressionNode()) &&
                   is_dead(t.fIfFalse.expressionNode());
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
    return is_dead(b.fLeft.expressionNode());
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
                            IRNode::ID newExpression) {
    IRNodeIDPtr target = (*iter)->idPtr();
    if (!b->tryRemoveExpression(iter)) {
        *target.ptr() = newExpression;
        return false;
    }
    *target.ptr() = newExpression;
    return b->tryInsertExpression(iter, (*iter)->idPtr());
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
            Type& cType = c.fType.typeNode();
            bool isFloat = cType.columns() > 1 ? cType.componentType().typeNode().isFloat()
                                               : cType.isFloat();
            if (cType.kind() == Type::kVector_Kind && c.isConstant()) {
                for (int i = 0; i < cType.columns(); ++i) {
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
    SkASSERT((*iter)->expression().fKind == Expression::kBinary_Kind);
    IRNodeIDPtr target = (*iter)->idPtr();
    BinaryExpression& bin = (BinaryExpression&) (*iter)->expression();
    SkASSERT(!bin.fLeft.expressionNode().hasSideEffects());
    bool result;
    if (bin.fOperator == Token::EQ) {
        result = b->tryRemoveLValueBefore(iter, bin.fLeft);
    } else {
        result = b->tryRemoveExpressionBefore(iter, bin.fLeft);
    }
    *target.ptr() = bin.fRight;
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
        (*iter)->id() != bin.fRight) {
        *outNeedsRescan = true;
        return;
    }
    *iter = b->fNodes.erase(*iter);
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
    SkASSERT((*iter)->expression().fKind == Expression::kBinary_Kind);
    IRNodeIDPtr target = (*iter)->idPtr();
    BinaryExpression& bin = (BinaryExpression&) (*iter)->expression();
    SkASSERT(!bin.fRight.expressionNode().hasSideEffects());
    if (!b->tryRemoveExpressionBefore(iter, bin.fRight)) {
        *target.ptr() = bin.fLeft;
        *outNeedsRescan = true;
        return;
    }
    *target.ptr() = bin.fLeft;
    if (*iter == b->fNodes.begin()) {
        *outNeedsRescan = true;
        return;
    }
    --(*iter);
    if (((*iter)->fKind != BasicBlock::Node::kExpression_Kind || (*iter)->id() != bin.fLeft)) {
        *outNeedsRescan = true;
        return;
    }
    *iter = b->fNodes.erase(*iter);
}

/**
 * Constructs the specified type using a single argument.
 */
static IRNode::ID construct(IRNode::ID type, IRNode::ID v) {
    return v.fIRGenerator->createNode(new Constructor(v.fIRGenerator, -1, type, { v }));
}

/**
 * Used in the implementations of vectorize_left and vectorize_right. Given a vector type and an
 * expression x, deletes the expression pointed to by iter and replaces it with <type>(x).
 */
static void vectorize(BasicBlock* b,
                      std::vector<BasicBlock::Node>::iterator* iter,
                      IRNode::ID type,
                      IRNode::ID otherExpression,
                      bool* outUpdated,
                      bool* outNeedsRescan) {
    SkASSERT((*iter)->expression().fKind == Expression::kBinary_Kind);
    SkASSERT(type.typeNode().kind() == Type::kVector_Kind);
    SkASSERT(otherExpression.expressionNode().fType.typeNode().kind() == Type::kScalar_Kind);
    *outUpdated = true;
    IRNodeIDPtr target = (*iter)->idPtr();
    if (!b->tryRemoveExpression(iter)) {
        *target.ptr() = construct(type, otherExpression);
        *outNeedsRescan = true;
    } else {
        *target.ptr() = construct(type, otherExpression);
        if (!b->tryInsertExpression(iter, (*iter)->idPtr())) {
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
    BinaryExpression& bin = (BinaryExpression&) (*iter)->expression();
    vectorize(b, iter, bin.fRight.expressionNode().fType, bin.fLeft, outUpdated, outNeedsRescan);
}

/**
 * Given a binary expression of the form vec<n>(x) <op> y, deletes the left side and vectorizes the
 * right to yield vec<n>(y).
 */
static void vectorize_right(BasicBlock* b,
                            std::vector<BasicBlock::Node>::iterator* iter,
                            bool* outUpdated,
                            bool* outNeedsRescan) {
    BinaryExpression& bin = (BinaryExpression&) (*iter)->expression();
    vectorize(b, iter, bin.fLeft.expressionNode().fType, bin.fRight, outUpdated, outNeedsRescan);
}

// Mark that an expression which we were writing to is no longer being written to
void clear_write(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind: {
            ((VariableReference&) expr).setRefKind(kRead_RefKind);
            break;
        }
        case Expression::kFieldAccess_Kind:
            clear_write(((FieldAccess&) expr).fBase.expressionNode());
            break;
        case Expression::kSwizzle_Kind:
            clear_write(((Swizzle&) expr).fBase.expressionNode());
            break;
        case Expression::kIndex_Kind:
            clear_write(((IndexExpression&) expr).fBase.expressionNode());
            break;
        default:
            ABORT("shouldn't be writing to this kind of expression\n");
            break;
    }
}

void Compiler::simplifyExpression(DefinitionMap& definitions,
                                  BasicBlock& b,
                                  std::vector<BasicBlock::Node>::iterator* iter,
                                  std::unordered_set<IRNode::ID>* undefinedVariables,
                                  bool* outUpdated,
                                  bool* outNeedsRescan) {
    Expression* expr = &(*iter)->expression();
    if ((*iter)->fConstantPropagation) {
        IRNode::ID optimized = expr->constantPropagate(definitions);
        if (optimized) {
            *outUpdated = true;
            if (!try_replace_expression(&b, iter, optimized)) {
                *outNeedsRescan = true;
                return;
            }
            SkASSERT((*iter)->fKind == BasicBlock::Node::kExpression_Kind);
            expr = &(*iter)->expression();
        }
    }
    switch (expr->fKind) {
        case Expression::kVariableReference_Kind: {
            const VariableReference* ref = (VariableReference*) expr;
            IRNode::ID varID = ref->fVariable;
            const Variable& var = (Variable&) varID.node();
            if (ref->refKind() != kWrite_RefKind && ref->refKind() != kPointer_RefKind &&
                var.fStorage == Variable::kLocal_Storage && !definitions[varID] &&
                (*undefinedVariables).find(varID) == (*undefinedVariables).end()) {
                (*undefinedVariables).insert(varID);
                this->error(expr->fOffset, "'" + var.fName + "' has not been assigned");
            }
            break;
        }
        case Expression::kTernary_Kind: {
            TernaryExpression* t = (TernaryExpression*) expr;
            if (t->fTest.expressionNode().fKind == Expression::kBoolLiteral_Kind) {
                // ternary has a constant test, replace it with either the true or
                // false branch
                if (((BoolLiteral&) t->fTest.expressionNode()).fValue) {
                    (*iter)->setID(t->fIfTrue);
                } else {
                    (*iter)->setID(t->fIfFalse);
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
            Expression& left = bin->fLeft.expressionNode();
            Expression& right = bin->fRight.expressionNode();
            // collapse useless expressions like x * 1 or x + 0
            if (((left.fType.typeNode().kind()  != Type::kScalar_Kind) &&
                 (left.fType.typeNode().kind()  != Type::kVector_Kind)) ||
                ((right.fType.typeNode().kind() != Type::kScalar_Kind) &&
                 (right.fType.typeNode().kind() != Type::kVector_Kind))) {
                break;
            }
            switch (bin->fOperator) {
                case Token::STAR:
                    if (is_constant(left, 1)) {
                        if (left.fType.typeNode().kind() == Type::kVector_Kind &&
                            right.fType.typeNode().kind() == Type::kScalar_Kind) {
                            // float4(1) * x -> float4(x)
                            vectorize_right(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // 1 * x -> x
                            // 1 * float4(x) -> float4(x)
                            // float4(1) * float4(x) -> float4(x)
                            delete_left(&b, iter, outUpdated, outNeedsRescan);
                        }
                    }
                    else if (is_constant(left, 0)) {
                        if (left.fType.typeNode().kind() == Type::kScalar_Kind &&
                            right.fType.typeNode().kind() == Type::kVector_Kind &&
                            !right.hasSideEffects()) {
                            // 0 * float4(x) -> float4(0)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // 0 * x -> 0
                            // float4(0) * x -> float4(0)
                            // float4(0) * float4(x) -> float4(0)
                            if (!right.hasSideEffects()) {
                                delete_right(&b, iter, outUpdated, outNeedsRescan);
                            }
                        }
                    }
                    else if (is_constant(right, 1)) {
                        if (left.fType.typeNode().kind() == Type::kScalar_Kind &&
                            right.fType.typeNode().kind() == Type::kVector_Kind) {
                            // x * float4(1) -> float4(x)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x * 1 -> x
                            // float4(x) * 1 -> float4(x)
                            // float4(x) * float4(1) -> float4(x)
                            delete_right(&b, iter, outUpdated, outNeedsRescan);
                        }
                    }
                    else if (is_constant(right, 0)) {
                        if (left.fType.typeNode().kind() == Type::kVector_Kind &&
                            right.fType.typeNode().kind() == Type::kScalar_Kind &&
                            !left.hasSideEffects()) {
                            // float4(x) * 0 -> float4(0)
                            vectorize_right(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x * 0 -> 0
                            // x * float4(0) -> float4(0)
                            // float4(x) * float4(0) -> float4(0)
                            if (!left.hasSideEffects()) {
                                delete_left(&b, iter, outUpdated, outNeedsRescan);
                            }
                        }
                    }
                    break;
                case Token::PLUS:
                    if (is_constant(left, 0)) {
                        if (left.fType.typeNode().kind() == Type::kVector_Kind &&
                            right.fType.typeNode().kind() == Type::kScalar_Kind) {
                            // float4(0) + x -> float4(x)
                            vectorize_right(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // 0 + x -> x
                            // 0 + float4(x) -> float4(x)
                            // float4(0) + float4(x) -> float4(x)
                            delete_left(&b, iter, outUpdated, outNeedsRescan);
                        }
                    } else if (is_constant(right, 0)) {
                        if (left.fType.typeNode().kind() == Type::kScalar_Kind &&
                            right.fType.typeNode().kind() == Type::kVector_Kind) {
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
                    if (is_constant(right, 0)) {
                        if (left.fType.typeNode().kind() == Type::kScalar_Kind &&
                            right.fType.typeNode().kind() == Type::kVector_Kind) {
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
                    if (is_constant(right, 1)) {
                        if (left.fType.typeNode().kind() == Type::kScalar_Kind &&
                            right.fType.typeNode().kind() == Type::kVector_Kind) {
                            // x / float4(1) -> float4(x)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x / 1 -> x
                            // float4(x) / 1 -> float4(x)
                            // float4(x) / float4(1) -> float4(x)
                            delete_right(&b, iter, outUpdated, outNeedsRescan);
                        }
                    } else if (is_constant(left, 0)) {
                        if (left.fType.typeNode().kind() == Type::kScalar_Kind &&
                            right.fType.typeNode().kind() == Type::kVector_Kind &&
                            !right.hasSideEffects()) {
                            // 0 / float4(x) -> float4(0)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // 0 / x -> 0
                            // float4(0) / x -> float4(0)
                            // float4(0) / float4(x) -> float4(0)
                            if (!right.hasSideEffects()) {
                                delete_right(&b, iter, outUpdated, outNeedsRescan);
                            }
                        }
                    }
                    break;
                case Token::PLUSEQ:
                    if (is_constant(right, 0)) {
                        clear_write(left);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                case Token::MINUSEQ:
                    if (is_constant(right, 0)) {
                        clear_write(left);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                case Token::STAREQ:
                    if (is_constant(right, 1)) {
                        clear_write(left);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                case Token::SLASHEQ:
                    if (is_constant(right, 1)) {
                        clear_write(left);
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
        case Statement::kBlock_Kind: {
            Block& b = (Block&) s;
            for (IRNode::ID sub : b.fStatements) {
                if (contains_conditional_break(sub.statementNode(), inConditional)) {
                    return true;
                }
            }
            return false;
        }
        case Statement::kBreak_Kind:
            return inConditional;
        case Statement::kIf_Kind: {
            const IfStatement& i = (IfStatement&) s;
            return contains_conditional_break(i.fIfTrue.statementNode(), true) ||
                   (i.fIfFalse && contains_conditional_break(i.fIfFalse.statementNode(), true));
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
            for (IRNode::ID sub : ((Block&) s).fStatements) {
                if (contains_unconditional_break(sub.statementNode())) {
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

// Returns a block containing all of the statements that will be run if the given case matches.
// Returns null if no such simple reduction is possible, such as when break statements appear inside
// conditionals.
static IRNode::ID block_for_case(IRNode::ID switchStatementID,
                                 IRNode::ID switchCaseID) {
    bool capturing = false;
    SwitchStatement& switchStatement = (SwitchStatement&) switchStatementID.node();
    std::vector<IRNode::ID> statementIDs;
    for (IRNode::ID current : switchStatement.fCases) {
        if (current == switchCaseID) {
            capturing = true;
        }
        if (capturing) {
            for (IRNode::ID stmt : ((SwitchCase&) current.node()).fStatements) {
                if (contains_conditional_break(stmt.statementNode(),
                                               stmt.statementNode().fKind == Statement::kIf_Kind)) {
                    return IRNode::ID();
                }
                if (contains_unconditional_break(stmt.statementNode())) {
                    capturing = false;
                    break;
                }
                statementIDs.push_back(stmt);
            }
            if (!capturing) {
                break;
            }
        }
    }
    std::vector<IRNode::ID> statements;
    for (IRNode::ID s : statementIDs) {
        statements.push_back(s);
    }
    return switchStatementID.fIRGenerator->createNode(new Block(switchStatementID.fIRGenerator,
                                                                -1,
                                                                std::move(statements),
                                                                switchStatement.fSymbols));
}

void Compiler::simplifyStatement(DefinitionMap& definitions,
                                 BasicBlock& b,
                                 std::vector<BasicBlock::Node>::iterator* iter,
                                 std::unordered_set<IRNode::ID>* undefinedVariables,
                                 bool* outUpdated,
                                 bool* outNeedsRescan) {
    Statement& stmt = (*iter)->statement();
    switch (stmt.fKind) {
        case Statement::kVarDeclaration_Kind: {
            const auto& varDecl = (VarDeclaration&) stmt;
            if (((Variable&) varDecl.fVar.node()).dead() &&
                (!varDecl.fValue || !varDecl.fValue.expressionNode().hasSideEffects())) {
                if (varDecl.fValue) {
                    SkASSERT((*iter)->id() == (*iter)->id());
                    if (!b.tryRemoveExpressionBefore(iter, varDecl.fValue)) {
                        *outNeedsRescan = true;
                    }
                }
                (*iter)->setID(fIRGenerator->createNode(new Nop(fIRGenerator)));
                *outUpdated = true;
            }
            break;
        }
        case Statement::kIf_Kind: {
            IfStatement& i = (IfStatement&) stmt;
            if (i.fTest.expressionNode().fKind == Expression::kBoolLiteral_Kind) {
                // constant if, collapse down to a single branch
                if (((BoolLiteral&) i.fTest.expressionNode()).fValue) {
                    SkASSERT(i.fIfTrue);
                    (*iter)->setID(i.fIfTrue);
                } else {
                    if (i.fIfFalse) {
                        (*iter)->setID(i.fIfFalse);
                    } else {
                        (*iter)->setID(fIRGenerator->createNode(new Nop(fIRGenerator)));
                    }
                }
                *outUpdated = true;
                *outNeedsRescan = true;
                break;
            }
            if (i.fIfFalse && i.fIfFalse.statementNode().isEmpty()) {
                // else block doesn't do anything, remove it
                i.fIfFalse = IRNode::ID();
                *outUpdated = true;
                *outNeedsRescan = true;
            }
            if (!i.fIfFalse && i.fIfTrue.statementNode().isEmpty()) {
                // if block doesn't do anything, no else block
                if (i.fTest.expressionNode().hasSideEffects()) {
                    // test has side effects, keep it
                    (*iter)->setID(fIRGenerator->createNode(new ExpressionStatement(fIRGenerator,
                                                                                    i.fTest)));
                } else {
                    // no if, no else, no test side effects, kill the whole if
                    // statement
                    (*iter)->setID(fIRGenerator->createNode(new Nop(fIRGenerator)));
                }
                *outUpdated = true;
                *outNeedsRescan = true;
            }
            break;
        }
        case Statement::kSwitch_Kind: {
            SwitchStatement& s = (SwitchStatement&) stmt;
            if (s.fValue.expressionNode().isConstant()) {
                // switch is constant, replace it with the case that matches
                bool found = false;
                IRNode::ID defaultCase;
                for (IRNode::ID cID : s.fCases) {
                    SwitchCase& c = (SwitchCase&) cID.node();
                    if (!c.fValue) {
                        defaultCase = cID;
                        continue;
                    }
                    SkASSERT(c.fValue.expressionNode().fKind == s.fValue.expressionNode().fKind);
                    found = c.fValue.expressionNode().compareConstant(s.fValue.expressionNode());
                    if (found) {
                        IRNode::ID newBlock = block_for_case((*iter)->id(), cID);
                        if (newBlock) {
                            (*iter)->setID(newBlock);
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
                        IRNode::ID newBlock = block_for_case((*iter)->id(), defaultCase);
                        if (newBlock) {
                            (*iter)->setID(newBlock);
                        } else {
                            if (s.fIsStatic && !(fFlags & kPermitInvalidStaticTests_Flag)) {
                                this->error(s.fOffset,
                                            "static switch contains non-static conditional break");
                                s.fIsStatic = false;
                            }
                            return; // can't simplify
                        }
                    } else {
                        (*iter)->setID(fIRGenerator->createNode(new Nop(fIRGenerator)));
                    }
                }
                *outUpdated = true;
                *outNeedsRescan = true;
            }
            break;
        }
        case Statement::kExpression_Kind: {
            ExpressionStatement& e = (ExpressionStatement&) stmt;
            if (!e.fExpression.expressionNode().hasSideEffects()) {
                // Expression statement with no side effects, kill it
                if (!b.tryRemoveExpressionBefore(iter, e.fExpression)) {
                    *outNeedsRescan = true;
                }
                (*iter)->setID(fIRGenerator->createNode(new Nop(fIRGenerator)));
                *outUpdated = true;
            }
            break;
        }
        default:
            break;
    }
}

void Compiler::scanCFG(IRNode::ID functionDefinition) {
    CFG cfg = CFGGenerator().getCFG(functionDefinition);
    this->computeDataFlow(&cfg);

    // check for unreachable code
    for (size_t i = 0; i < cfg.fBlocks.size(); i++) {
        if (i != cfg.fStart && !cfg.fBlocks[i].fEntrances.size() &&
            cfg.fBlocks[i].fNodes.size()) {
            int offset;
            switch (cfg.fBlocks[i].fNodes[0].fKind) {
                case BasicBlock::Node::kStatement_Kind:
                    offset = cfg.fBlocks[i].fNodes[0].statement().fOffset;
                    break;
                case BasicBlock::Node::kExpression_Kind:
                    offset = cfg.fBlocks[i].fNodes[0].expression().fOffset;
                    break;
            }
            this->error(offset, String("unreachable"));
        }
    }
    if (fErrorCount) {
        return;
    }

    // check for dead code & undefined variables, perform constant propagation
    std::unordered_set<IRNode::ID> undefinedVariables;
    bool updated;
    bool needsRescan = false;
    do {
        if (needsRescan) {
            cfg = CFGGenerator().getCFG(functionDefinition);
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
                const Statement& s = iter->statement();
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
                        VarDeclarationsStatement& vds = (VarDeclarationsStatement&) s;
                        VarDeclarations& decls = (VarDeclarations&) vds.fDeclaration.node();
                        for (auto varIter = decls.fVars.begin(); varIter != decls.fVars.end();) {
                            if (varIter->statementNode().fKind == Statement::kNop_Kind) {
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
    FunctionDefinition& f = (FunctionDefinition&) functionDefinition.node();
    FunctionDeclaration& fd = (FunctionDeclaration&) f.fDeclaration.node();
    if (fd.fReturnType != fIRGenerator->fContext.fVoid_Type) {
        if (cfg.fBlocks[cfg.fExit].fEntrances.size()) {
            this->error(f.fOffset, String("function can exit without returning a value"));
        }
    }
}

void Compiler::registerExternalValue(ExternalValue* value) {
    //fIRGenerator->fRootSymbolTable->add(value->fName, value);
}

std::unique_ptr<Program> Compiler::convertProgram(Program::Kind kind, String text,
                                                  const Program::Settings& settings) {
    fErrorText = "";
    fErrorCount = 0;
    std::vector<IRNode::ID>* inherited;
    std::vector<IRNode::ID> elements;
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
    for (IRNode::ID e : elements) {
        if (((ProgramElement&) e.node()).fKind == ProgramElement::kEnum_Kind) {
            ((Enum&) e.node()).fBuiltin = true;
        }
    }
    std::unique_ptr<String> textPtr(new String(std::move(text)));
    fSource = textPtr.get();
    fIRGenerator->convertProgram(kind, textPtr->c_str(), textPtr->size(), *fTypes, &elements);
    auto result = std::unique_ptr<Program>(new Program(kind,
                                                       std::move(textPtr),
                                                       settings,
                                                       &fIRGenerator->fContext,
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
        for (IRNode::ID e : program) {
            if (((ProgramElement&) e.node()).fKind == ProgramElement::kFunction_Kind) {
                this->scanCFG(e);
            }
        }
        if (program.fKind != Program::kFragmentProcessor_Kind) {
            for (auto iter = program.fElements.begin(); iter != program.fElements.end();) {
                if (((ProgramElement&) iter->node()).fKind == ProgramElement::kVar_Kind) {
                    VarDeclarations& vars = (VarDeclarations&) iter->node();
                    for (auto varIter = vars.fVars.begin(); varIter != vars.fVars.end();) {
                        IRNode::ID var = ((VarDeclaration&) varIter->node()).fVar;
                        if (((Variable&) var.node()).dead()) {
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
    std::vector<IRNode::ID> elements;
    for (IRNode::ID e : program) {
        elements.push_back(((ProgramElement&) e.node()).clone());
    }
    Program::Settings settings;
    settings.fCaps = program.fSettings.fCaps;
    for (auto iter = inputs.begin(); iter != inputs.end(); ++iter) {
        settings.fArgs.insert(*iter);
    }
    std::unique_ptr<Program> result(new Program(program.fKind,
                                                nullptr,
                                                settings,
                                                &program.fContext,
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
    SPIRVCodeGenerator cg(fIRGenerator, &program, &buffer);
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
    SPIRVCodeGenerator cg(fIRGenerator, &program, &out);
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
    GLSLCodeGenerator cg(fIRGenerator, &program, &out);
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
/*    if (!this->optimize(program)) {
        return false;
    }
    MetalCodeGenerator cg(fIRGenerator, &program, &out);
    bool result = cg.generateCode();
    return result;*/
    abort();
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
    CPPCodeGenerator cg(fIRGenerator, &program, name, &out);
    bool result = cg.generateCode();
    fSource = nullptr;
    return result;
}

bool Compiler::toH(Program& program, String name, OutputStream& out) {
    if (!this->optimize(program)) {
        return false;
    }
    fSource = program.fSource.get();
    HCodeGenerator cg(fIRGenerator, &program, name, &out);
    bool result = cg.generateCode();
    fSource = nullptr;
    return result;
}

bool Compiler::toPipelineStage(const Program& program, String* out,
                               std::vector<FormatArg>* outFormatArgs) {
    SkASSERT(program.fIsOptimized);
    fSource = program.fSource.get();
    StringStream buffer;
    PipelineStageCodeGenerator cg(fIRGenerator, &program, &buffer, outFormatArgs);
    bool result = cg.generateCode();
    fSource = nullptr;
    if (result) {
        *out = buffer.str();
    }
    return result;
}

#endif

std::unique_ptr<ByteCode> Compiler::toByteCode(Program& program) {
#if defined(SK_ENABLE_SKSL_INTERPRETER)
    if (!this->optimize(program)) {
        return nullptr;
    }
    std::unique_ptr<ByteCode> result(new ByteCode());
    ByteCodeGenerator cg(&context(), &program, this, result.get());
    if (cg.generateCode()) {
        return result;
    }
#else
    ABORT("ByteCode interpreter not enabled");
#endif
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
    if (!fSource) {
        // FIXME remove this test
        return Position(-1, -1);
    }
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
