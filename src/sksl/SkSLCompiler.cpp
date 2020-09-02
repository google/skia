/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include <memory>
#include <unordered_set>

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLByteCodeGenerator.h"
#include "src/sksl/SkSLCFGGenerator.h"
#include "src/sksl/SkSLCPPCodeGenerator.h"
#include "src/sksl/SkSLGLSLCodeGenerator.h"
#include "src/sksl/SkSLHCodeGenerator.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLMetalCodeGenerator.h"
#include "src/sksl/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/SkSLRehydrator.h"
#include "src/sksl/SkSLSPIRVCodeGenerator.h"
#include "src/sksl/SkSLSPIRVtoHLSL.h"
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

#include <fstream>

#if !defined(SKSL_STANDALONE) & SK_SUPPORT_GPU
#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrShaderCaps.h"
#endif

#ifdef SK_ENABLE_SPIRV_VALIDATION
#include "spirv-tools/libspirv.hpp"
#endif

#if !SKSL_STANDALONE

#include "src/sksl/generated/sksl_fp.dehydrated.sksl"
#include "src/sksl/generated/sksl_frag.dehydrated.sksl"
#include "src/sksl/generated/sksl_geom.dehydrated.sksl"
#include "src/sksl/generated/sksl_gpu.dehydrated.sksl"
#include "src/sksl/generated/sksl_interp.dehydrated.sksl"
#include "src/sksl/generated/sksl_pipeline.dehydrated.sksl"
#include "src/sksl/generated/sksl_vert.dehydrated.sksl"

#else

// GN generates or copies all of these files to the skslc executable directory
static const char SKSL_GPU_INCLUDE[]      = "sksl_gpu.sksl";
static const char SKSL_INTERP_INCLUDE[]   = "sksl_interp.sksl";
static const char SKSL_VERT_INCLUDE[]     = "sksl_vert.sksl";
static const char SKSL_FRAG_INCLUDE[]     = "sksl_frag.sksl";
static const char SKSL_GEOM_INCLUDE[]     = "sksl_geom.sksl";
static const char SKSL_FP_INCLUDE[]       = "sksl_fp.sksl";
static const char SKSL_PIPELINE_INCLUDE[] = "sksl_pipeline.sksl";

#endif

namespace SkSL {

static void grab_intrinsics(std::vector<std::unique_ptr<ProgramElement>>* src,
                            IRIntrinsicMap* target) {
    for (auto iter = src->begin(); iter != src->end(); ) {
        std::unique_ptr<ProgramElement>& element = *iter;
        switch (element->fKind) {
            case ProgramElement::kFunction_Kind: {
                FunctionDefinition& f = element->as<FunctionDefinition>();
                SkASSERT(f.fDeclaration.fBuiltin);
                String key = f.fDeclaration.description();
                SkASSERT(target->find(key) == target->end());
                (*target)[key] = IRIntrinsic{std::move(element), /*fAlreadyIncluded=*/false};
                iter = src->erase(iter);
                break;
            }
            case ProgramElement::kEnum_Kind: {
                Enum& e = element->as<Enum>();
                StringFragment name = e.fTypeName;
                SkASSERT(target->find(name) == target->end());
                (*target)[name] = IRIntrinsic{std::move(element), /*fAlreadyIncluded=*/false};
                iter = src->erase(iter);
                break;
            }
            default:
                printf("unsupported include file element\n");
                SkASSERT(false);
        }
    }
}

Compiler::Compiler(Flags flags)
: fGPUIntrinsics(std::make_unique<IRIntrinsicMap>())
, fInterpreterIntrinsics(std::make_unique<IRIntrinsicMap>())
, fFlags(flags)
, fContext(std::make_shared<Context>())
, fErrorCount(0) {
    auto symbols = std::make_shared<SymbolTable>(this);
    fIRGenerator = std::make_unique<IRGenerator>(fContext.get(), &fInliner, symbols, *this);
    #define ADD_TYPE(t) symbols->addWithoutOwnership(fContext->f ## t ## _Type->fName, \
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
    ADD_TYPE(GenType);
    ADD_TYPE(GenHType);
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
    ADD_TYPE(Sampler);
    ADD_TYPE(Texture2D);

    StringFragment fpAliasName("shader");
    symbols->addWithoutOwnership(fpAliasName, fContext->fFragmentProcessor_Type.get());

    StringFragment skCapsName("sk_Caps");
    fIRGenerator->fSymbolTable->add(
            skCapsName,
            std::make_unique<Variable>(/*offset=*/-1, Modifiers(), skCapsName,
                                       *fContext->fSkCaps_Type, Variable::kGlobal_Storage));

    fIRGenerator->fIntrinsics = fGPUIntrinsics.get();
    std::vector<std::unique_ptr<ProgramElement>> gpuIntrinsics;
    std::vector<std::unique_ptr<ProgramElement>> interpIntrinsics;
#if SKSL_STANDALONE
    this->processIncludeFile(Program::kFragment_Kind, SKSL_GPU_INCLUDE, symbols, &gpuIntrinsics,
                             &fGpuSymbolTable);
    this->processIncludeFile(Program::kVertex_Kind, SKSL_VERT_INCLUDE, fGpuSymbolTable,
                             &fVertexInclude, &fVertexSymbolTable);
    this->processIncludeFile(Program::kFragment_Kind, SKSL_FRAG_INCLUDE, fGpuSymbolTable,
                             &fFragmentInclude, &fFragmentSymbolTable);
#else
    {
        Rehydrator rehydrator(fContext.get(), symbols, this, SKSL_INCLUDE_sksl_gpu,
                          SKSL_INCLUDE_sksl_gpu_LENGTH);
        fGpuSymbolTable = rehydrator.symbolTable();
        gpuIntrinsics = rehydrator.elements();
    }
    {
        Rehydrator rehydrator(fContext.get(), fGpuSymbolTable, this, SKSL_INCLUDE_sksl_vert,
                          SKSL_INCLUDE_sksl_vert_LENGTH);
        fVertexSymbolTable = rehydrator.symbolTable();
        fVertexInclude = rehydrator.elements();
    }
    {
        Rehydrator rehydrator(fContext.get(), fGpuSymbolTable, this, SKSL_INCLUDE_sksl_frag,
                          SKSL_INCLUDE_sksl_frag_LENGTH);
        fFragmentSymbolTable = rehydrator.symbolTable();
        fFragmentInclude = rehydrator.elements();
    }
#endif
    grab_intrinsics(&gpuIntrinsics, fGPUIntrinsics.get());
    grab_intrinsics(&interpIntrinsics, fInterpreterIntrinsics.get());
}

Compiler::~Compiler() {}

void Compiler::loadGeometryIntrinsics() {
    if (fGeometrySymbolTable) {
        return;
    }
    #if !SKSL_STANDALONE
        {
            Rehydrator rehydrator(fContext.get(), fGpuSymbolTable, this, SKSL_INCLUDE_sksl_geom,
                              SKSL_INCLUDE_sksl_geom_LENGTH);
            fGeometrySymbolTable = rehydrator.symbolTable();
            fGeometryInclude = rehydrator.elements();
        }
    #else
        this->processIncludeFile(Program::kGeometry_Kind, SKSL_GEOM_INCLUDE, fGpuSymbolTable,
                                 &fGeometryInclude, &fGeometrySymbolTable);
    #endif
}

void Compiler::loadPipelineIntrinsics() {
    if (fPipelineSymbolTable) {
        return;
    }
    #if !SKSL_STANDALONE
        {
            Rehydrator rehydrator(fContext.get(), fGpuSymbolTable, this,
                                  SKSL_INCLUDE_sksl_pipeline,
                                  SKSL_INCLUDE_sksl_pipeline_LENGTH);
            fPipelineSymbolTable = rehydrator.symbolTable();
            fPipelineInclude = rehydrator.elements();
        }
    #else
        this->processIncludeFile(Program::kPipelineStage_Kind, SKSL_PIPELINE_INCLUDE,
                                 fGpuSymbolTable, &fPipelineInclude, &fPipelineSymbolTable);
    #endif
}

void Compiler::loadInterpreterIntrinsics() {
    if (fInterpreterSymbolTable) {
        return;
    }
    this->loadPipelineIntrinsics();
    #if !SKSL_STANDALONE
        {
            Rehydrator rehydrator(fContext.get(), fPipelineSymbolTable, this,
                                  SKSL_INCLUDE_sksl_interp,
                                  SKSL_INCLUDE_sksl_interp_LENGTH);
            fInterpreterSymbolTable = rehydrator.symbolTable();
            fInterpreterInclude = rehydrator.elements();
        }
    #else
        this->processIncludeFile(Program::kGeneric_Kind, SKSL_INTERP_INCLUDE,
                                 fIRGenerator->fSymbolTable, &fInterpreterInclude,
                                 &fInterpreterSymbolTable);
    #endif
}

void Compiler::processIncludeFile(Program::Kind kind, const char* path,
                                  std::shared_ptr<SymbolTable> base,
                                  std::vector<std::unique_ptr<ProgramElement>>* outElements,
                                  std::shared_ptr<SymbolTable>* outSymbolTable) {
    std::ifstream in(path);
    std::string stdText{std::istreambuf_iterator<char>(in),
                        std::istreambuf_iterator<char>()};
    if (in.rdstate()) {
        printf("error reading %s\n", path);
        abort();
    }
    if (!base) {
        base = fIRGenerator->fSymbolTable;
    }
    SkASSERT(base);
    const String* source = base->takeOwnershipOfString(std::make_unique<String>(stdText.c_str()));
    fSource = source;
    std::shared_ptr<SymbolTable> old = fIRGenerator->fSymbolTable;
    if (base) {
        fIRGenerator->fSymbolTable = std::move(base);
    }
    Program::Settings settings;
#if !defined(SKSL_STANDALONE) & SK_SUPPORT_GPU
    GrContextOptions opts;
    GrShaderCaps caps(opts);
    settings.fCaps = &caps;
#endif
    SkASSERT(fIRGenerator->fCanInline);
    fIRGenerator->fCanInline = false;
    fIRGenerator->start(&settings, nullptr, true);
    fIRGenerator->convertProgram(kind, source->c_str(), source->length(), outElements);
    fIRGenerator->fCanInline = true;
    if (this->fErrorCount) {
        printf("Unexpected errors: %s\n", this->fErrorText.c_str());
    }
    SkASSERT(!fErrorCount);
    *outSymbolTable = fIRGenerator->fSymbolTable;
#ifdef SK_DEBUG
    fSource = nullptr;
#endif
    fIRGenerator->fSymbolTable = std::move(old);
}

// add the definition created by assigning to the lvalue to the definition set
void Compiler::addDefinition(const Expression* lvalue, std::unique_ptr<Expression>* expr,
                             DefinitionMap* definitions) {
    switch (lvalue->fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = lvalue->as<VariableReference>().fVariable;
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
            this->addDefinition(lvalue->as<Swizzle>().fBase.get(),
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            break;
        case Expression::kIndex_Kind:
            // see comments in Swizzle
            this->addDefinition(lvalue->as<IndexExpression>().fBase.get(),
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            break;
        case Expression::kFieldAccess_Kind:
            // see comments in Swizzle
            this->addDefinition(lvalue->as<FieldAccess>().fBase.get(),
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            break;
        case Expression::kTernary_Kind:
            // To simplify analysis, we just pretend that we write to both sides of the ternary.
            // This allows for false positives (meaning we fail to detect that a variable might not
            // have been assigned), but is preferable to false negatives.
            this->addDefinition(lvalue->as<TernaryExpression>().fIfTrue.get(),
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            this->addDefinition(lvalue->as<TernaryExpression>().fIfFalse.get(),
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
            Expression* expr = node.expression()->get();
            switch (expr->fKind) {
                case Expression::kBinary_Kind: {
                    BinaryExpression* b = &expr->as<BinaryExpression>();
                    if (b->fOperator == Token::Kind::TK_EQ) {
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
                    const FunctionCall& c = expr->as<FunctionCall>();
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
                    const PrefixExpression* p = &expr->as<PrefixExpression>();
                    if (p->fOperator == Token::Kind::TK_MINUSMINUS ||
                        p->fOperator == Token::Kind::TK_PLUSPLUS) {
                        this->addDefinition(
                                      p->fOperand.get(),
                                      (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                      definitions);
                    }
                    break;
                }
                case Expression::kPostfix_Kind: {
                    const PostfixExpression* p = &expr->as<PostfixExpression>();
                    if (p->fOperator == Token::Kind::TK_MINUSMINUS ||
                        p->fOperator == Token::Kind::TK_PLUSPLUS) {
                        this->addDefinition(
                                      p->fOperand.get(),
                                      (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                      definitions);
                    }
                    break;
                }
                case Expression::kVariableReference_Kind: {
                    const VariableReference* v = &expr->as<VariableReference>();
                    if (v->fRefKind != VariableReference::kRead_RefKind) {
                        this->addDefinition(
                                      v,
                                      (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                      definitions);
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case BasicBlock::Node::kStatement_Kind: {
            Statement* stmt = node.statement()->get();
            if (stmt->fKind == Statement::kVarDeclaration_Kind) {
                VarDeclaration& vd = stmt->as<VarDeclaration>();
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
                    const VarDeclarationsStatement* vd = &s->as<VarDeclarationsStatement>();
                    for (const auto& decl : vd->fDeclaration->fVars) {
                        if (decl->fKind == Statement::kVarDeclaration_Kind) {
                            result[decl->as<VarDeclaration>().fVar] = nullptr;
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
            return lvalue.as<VariableReference>().fVariable.dead();
        case Expression::kSwizzle_Kind:
            return is_dead(*lvalue.as<Swizzle>().fBase);
        case Expression::kFieldAccess_Kind:
            return is_dead(*lvalue.as<FieldAccess>().fBase);
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = lvalue.as<IndexExpression>();
            return is_dead(*idx.fBase) &&
                   !idx.fIndex->hasProperty(Expression::Property::kSideEffects);
        }
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = lvalue.as<TernaryExpression>();
            return !t.fTest->hasSideEffects() && is_dead(*t.fIfTrue) && is_dead(*t.fIfFalse);
        }
        case Expression::kExternalValue_Kind:
            return false;
        default:
#ifdef SK_DEBUG
            ABORT("invalid lvalue: %s\n", lvalue.description().c_str());
#endif
            return false;
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
static bool try_replace_expression(BasicBlock* b,
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
template <typename T = double>
static bool is_constant(const Expression& expr, T value) {
    switch (expr.fKind) {
        case Expression::kIntLiteral_Kind:
            return expr.as<IntLiteral>().fValue == value;

        case Expression::kFloatLiteral_Kind:
            return expr.as<FloatLiteral>().fValue == value;

        case Expression::kConstructor_Kind: {
            const Constructor& constructor = expr.as<Constructor>();
            if (constructor.isCompileTimeConstant()) {
                bool isFloat = constructor.fType.columns() > 1
                                       ? constructor.fType.componentType().isFloat()
                                       : constructor.fType.isFloat();
                switch (constructor.fType.kind()) {
                    case Type::kVector_Kind:
                        for (int i = 0; i < constructor.fType.columns(); ++i) {
                            if (isFloat) {
                                if (constructor.getFVecComponent(i) != value) {
                                    return false;
                                }
                            } else {
                                if (constructor.getIVecComponent(i) != value) {
                                    return false;
                                }
                            }
                        }
                        return true;

                    case Type::kScalar_Kind:
                        SkASSERT(constructor.fArguments.size() == 1);
                        return is_constant<T>(*constructor.fArguments[0], value);

                    default:
                        return false;
                }
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
static void delete_left(BasicBlock* b,
                        std::vector<BasicBlock::Node>::iterator* iter,
                        bool* outUpdated,
                        bool* outNeedsRescan) {
    *outUpdated = true;
    std::unique_ptr<Expression>* target = (*iter)->expression();
    BinaryExpression& bin = (*target)->as<BinaryExpression>();
    SkASSERT(!bin.fLeft->hasSideEffects());
    bool result;
    if (bin.fOperator == Token::Kind::TK_EQ) {
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
static void delete_right(BasicBlock* b,
                         std::vector<BasicBlock::Node>::iterator* iter,
                         bool* outUpdated,
                         bool* outNeedsRescan) {
    *outUpdated = true;
    std::unique_ptr<Expression>* target = (*iter)->expression();
    BinaryExpression& bin = (*target)->as<BinaryExpression>();
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
    BinaryExpression& bin = (*(*iter)->expression())->as<BinaryExpression>();
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
    BinaryExpression& bin = (*(*iter)->expression())->as<BinaryExpression>();
    vectorize(b, iter, bin.fLeft->fType, &bin.fRight, outUpdated, outNeedsRescan);
}

// Mark that an expression which we were writing to is no longer being written to
static void clear_write(Expression& expr) {
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind: {
            expr.as<VariableReference>().setRefKind(VariableReference::kRead_RefKind);
            break;
        }
        case Expression::kFieldAccess_Kind:
            clear_write(*expr.as<FieldAccess>().fBase);
            break;
        case Expression::kSwizzle_Kind:
            clear_write(*expr.as<Swizzle>().fBase);
            break;
        case Expression::kIndex_Kind:
            clear_write(*expr.as<IndexExpression>().fBase);
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
            const VariableReference& ref = expr->as<VariableReference>();
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
            TernaryExpression* t = &expr->as<TernaryExpression>();
            if (t->fTest->fKind == Expression::kBoolLiteral_Kind) {
                // ternary has a constant test, replace it with either the true or
                // false branch
                if (t->fTest->as<BoolLiteral>().fValue) {
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
            BinaryExpression* bin = &expr->as<BinaryExpression>();
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
                case Token::Kind::TK_STAR:
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
                case Token::Kind::TK_PLUS:
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
                case Token::Kind::TK_MINUS:
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
                case Token::Kind::TK_SLASH:
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
                case Token::Kind::TK_PLUSEQ:
                    if (is_constant(*bin->fRight, 0)) {
                        clear_write(*bin->fLeft);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                case Token::Kind::TK_MINUSEQ:
                    if (is_constant(*bin->fRight, 0)) {
                        clear_write(*bin->fLeft);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                case Token::Kind::TK_STAREQ:
                    if (is_constant(*bin->fRight, 1)) {
                        clear_write(*bin->fLeft);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                case Token::Kind::TK_SLASHEQ:
                    if (is_constant(*bin->fRight, 1)) {
                        clear_write(*bin->fLeft);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                default:
                    break;
            }
            break;
        }
        case Expression::kSwizzle_Kind: {
            Swizzle& s = expr->as<Swizzle>();
            // detect identity swizzles like foo.rgba
            if ((int) s.fComponents.size() == s.fBase->fType.columns()) {
                bool identity = true;
                for (int i = 0; i < (int) s.fComponents.size(); ++i) {
                    if (s.fComponents[i] != i) {
                        identity = false;
                        break;
                    }
                }
                if (identity) {
                    *outUpdated = true;
                    if (!try_replace_expression(&b, iter, &s.fBase)) {
                        *outNeedsRescan = true;
                        return;
                    }
                    SkASSERT((*iter)->fKind == BasicBlock::Node::kExpression_Kind);
                    break;
                }
            }
            // detect swizzles of swizzles, e.g. replace foo.argb.r000 with foo.a000
            if (s.fBase->fKind == Expression::kSwizzle_Kind) {
                Swizzle& base = s.fBase->as<Swizzle>();
                std::vector<int> final;
                for (int c : s.fComponents) {
                    if (c == SKSL_SWIZZLE_0 || c == SKSL_SWIZZLE_1) {
                        final.push_back(c);
                    } else {
                        final.push_back(base.fComponents[c]);
                    }
                }
                *outUpdated = true;
                std::unique_ptr<Expression> replacement(new Swizzle(*fContext, base.fBase->clone(),
                                                                    std::move(final)));
                if (!try_replace_expression(&b, iter, &replacement)) {
                    *outNeedsRescan = true;
                    return;
                }
                SkASSERT((*iter)->fKind == BasicBlock::Node::kExpression_Kind);
            }
            break;
        }
        default:
            break;
    }
}

// Returns true if this statement could potentially execute a break at the current level. We ignore
// nested loops and switches, since any breaks inside of them will merely break the loop / switch.
static bool contains_conditional_break(Statement& stmt) {
    class ContainsConditionalBreak : public ProgramVisitor {
    public:
        bool visitStatement(const Statement& stmt) override {
            switch (stmt.fKind) {
                case Statement::kBlock_Kind:
                    return this->INHERITED::visitStatement(stmt);

                case Statement::kBreak_Kind:
                    return fInConditional > 0;

                case Statement::kIf_Kind: {
                    ++fInConditional;
                    bool result = this->INHERITED::visitStatement(stmt);
                    --fInConditional;
                    return result;
                }

                default:
                    return false;
            }
        }

        int fInConditional = 0;
        using INHERITED = ProgramVisitor;
    };

    return ContainsConditionalBreak{}.visitStatement(stmt);
}

// returns true if this statement definitely executes a break at the current level (we ignore
// nested loops and switches, since any breaks inside of them will merely break the loop / switch)
static bool contains_unconditional_break(Statement& stmt) {
    class ContainsUnconditionalBreak : public ProgramVisitor {
    public:
        bool visitStatement(const Statement& stmt) override {
            switch (stmt.fKind) {
                case Statement::kBlock_Kind:
                    return this->INHERITED::visitStatement(stmt);

                case Statement::kBreak_Kind:
                    return true;

                default:
                    return false;
            }
        }

        using INHERITED = ProgramVisitor;
    };

    return ContainsUnconditionalBreak{}.visitStatement(stmt);
}

static void move_all_but_break(std::unique_ptr<Statement>& stmt,
                               std::vector<std::unique_ptr<Statement>>* target) {
    switch (stmt->fKind) {
        case Statement::kBlock_Kind: {
            // Recurse into the block.
            Block& block = static_cast<Block&>(*stmt);

            std::vector<std::unique_ptr<Statement>> blockStmts;
            blockStmts.reserve(block.fStatements.size());
            for (std::unique_ptr<Statement>& statementInBlock : block.fStatements) {
                move_all_but_break(statementInBlock, &blockStmts);
            }

            target->push_back(std::make_unique<Block>(block.fOffset, std::move(blockStmts),
                                                      block.fSymbols, block.fIsScope));
            break;
        }

        case Statement::kBreak_Kind:
            // Do not append a break to the target.
            break;

        default:
            // Append normal statements to the target.
            target->push_back(std::move(stmt));
            break;
    }
}

// Returns a block containing all of the statements that will be run if the given case matches
// (which, owing to the statements being owned by unique_ptrs, means the switch itself will be
// broken by this call and must then be discarded).
// Returns null (and leaves the switch unmodified) if no such simple reduction is possible, such as
// when break statements appear inside conditionals.
static std::unique_ptr<Statement> block_for_case(SwitchStatement* switchStatement,
                                                 SwitchCase* caseToCapture) {
    // We have to be careful to not move any of the pointers until after we're sure we're going to
    // succeed, so before we make any changes at all, we check the switch-cases to decide on a plan
    // of action. First, find the switch-case we are interested in.
    auto iter = switchStatement->fCases.begin();
    for (; iter != switchStatement->fCases.end(); ++iter) {
        if (iter->get() == caseToCapture) {
            break;
        }
    }

    // Next, walk forward through the rest of the switch. If we find a conditional break, we're
    // stuck and can't simplify at all. If we find an unconditional break, we have a range of
    // statements that we can use for simplification.
    auto startIter = iter;
    Statement* unconditionalBreakStmt = nullptr;
    for (; iter != switchStatement->fCases.end(); ++iter) {
        for (std::unique_ptr<Statement>& stmt : (*iter)->fStatements) {
            if (contains_conditional_break(*stmt)) {
                // We can't reduce switch-cases to a block when they have conditional breaks.
                return nullptr;
            }

            if (contains_unconditional_break(*stmt)) {
                // We found an unconditional break. We can use this block, but we need to strip
                // out the break statement.
                unconditionalBreakStmt = stmt.get();
                break;
            }
        }

        if (unconditionalBreakStmt != nullptr) {
            break;
        }
    }

    // We fell off the bottom of the switch or encountered a break. We know the range of statements
    // that we need to move over, and we know it's safe to do so.
    std::vector<std::unique_ptr<Statement>> caseStmts;

    // We can move over most of the statements as-is.
    while (startIter != iter) {
        for (std::unique_ptr<Statement>& stmt : (*startIter)->fStatements) {
            caseStmts.push_back(std::move(stmt));
        }
        ++startIter;
    }

    // If we found an unconditional break at the end, we need to move what we can while avoiding
    // that break.
    if (unconditionalBreakStmt != nullptr) {
        for (std::unique_ptr<Statement>& stmt : (*startIter)->fStatements) {
            if (stmt.get() == unconditionalBreakStmt) {
                move_all_but_break(stmt, &caseStmts);
                unconditionalBreakStmt = nullptr;
                break;
            }

            caseStmts.push_back(std::move(stmt));
        }
    }

    SkASSERT(unconditionalBreakStmt == nullptr);  // Verify that we fixed the unconditional break.

    // Return our newly-synthesized block.
    return std::make_unique<Block>(/*offset=*/-1, std::move(caseStmts), switchStatement->fSymbols);
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
            const auto& varDecl = stmt->as<VarDeclaration>();
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
            IfStatement& i = stmt->as<IfStatement>();
            if (i.fTest->fKind == Expression::kBoolLiteral_Kind) {
                // constant if, collapse down to a single branch
                if (i.fTest->as<BoolLiteral>().fValue) {
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
            SwitchStatement& s = stmt->as<SwitchStatement>();
            if (s.fValue->isCompileTimeConstant()) {
                // switch is constant, replace it with the case that matches
                bool found = false;
                SwitchCase* defaultCase = nullptr;
                for (const std::unique_ptr<SwitchCase>& c : s.fCases) {
                    if (!c->fValue) {
                        defaultCase = c.get();
                        continue;
                    }
                    if (is_constant<int64_t>(*s.fValue, c->fValue->getConstantInt())) {
                        std::unique_ptr<Statement> newBlock = block_for_case(&s, c.get());
                        if (newBlock) {
                            (*iter)->setStatement(std::move(newBlock));
                            found = true;
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
            ExpressionStatement& e = stmt->as<ExpressionStatement>();
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
                    if ((*cfg.fBlocks[i].fNodes[0].expression())->fKind ==
                        Expression::kBoolLiteral_Kind) {
                        // Function inlining can generate do { ... } while(false) loops which always
                        // break, so the boolean condition is considered unreachable. Since not
                        // being able to reach a literal is a non-issue in the first place, we
                        // don't report an error in this case.
                        continue;
                    }
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
        bool first = true;
        for (BasicBlock& b : cfg.fBlocks) {
            if (!first && b.fEntrances.empty()) {
                // Block was reachable before optimization, but has since become unreachable. In
                // addition to being dead code, it's broken - since control flow can't reach it, no
                // prior variable definitions can reach it, and therefore variables might look to
                // have not been properly assigned. Kill it.
                for (BasicBlock::Node& node : b.fNodes) {
                    if (node.fKind == BasicBlock::Node::kStatement_Kind &&
                        (*node.statement())->fKind != Statement::kNop_Kind) {
                        node.setStatement(std::unique_ptr<Statement>(new Nop()));
                    }
                }
                continue;
            }
            first = false;
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
                        if (s.as<IfStatement>().fIsStatic &&
                            !(fFlags & kPermitInvalidStaticTests_Flag)) {
                            this->error(s.fOffset, "static if has non-static test");
                        }
                        ++iter;
                        break;
                    case Statement::kSwitch_Kind:
                        if (s.as<SwitchStatement>().fIsStatic &&
                             !(fFlags & kPermitInvalidStaticTests_Flag)) {
                            this->error(s.fOffset, "static switch has non-static test");
                        }
                        ++iter;
                        break;
                    case Statement::kVarDeclarations_Kind: {
                        VarDeclarations& decls = *s.as<VarDeclarationsStatement>().fDeclaration;
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
            this->error(f.fOffset, String("function '" + String(f.fDeclaration.fName) +
                                          "' can exit without returning a value"));
        }
    }
}

void Compiler::registerExternalValue(ExternalValue* value) {
    fIRGenerator->fRootSymbolTable->addWithoutOwnership(value->fName, value);
}

const Symbol* Compiler::takeOwnership(std::unique_ptr<const Symbol> symbol) {
    return fIRGenerator->fRootSymbolTable->takeOwnershipOfSymbol(std::move(symbol));
}

std::unique_ptr<Program> Compiler::convertProgram(Program::Kind kind, String text,
                                                  const Program::Settings& settings) {
    fErrorText = "";
    fErrorCount = 0;
    fInliner.reset(context(), settings);
    std::vector<std::unique_ptr<ProgramElement>>* inherited;
    std::vector<std::unique_ptr<ProgramElement>> elements;
    switch (kind) {
        case Program::kVertex_Kind:
            inherited = &fVertexInclude;
            fIRGenerator->fSymbolTable = fVertexSymbolTable;
            fIRGenerator->fIntrinsics = fGPUIntrinsics.get();
            fIRGenerator->start(&settings, inherited);
            break;
        case Program::kFragment_Kind:
            inherited = &fFragmentInclude;
            fIRGenerator->fSymbolTable = fFragmentSymbolTable;
            fIRGenerator->fIntrinsics = fGPUIntrinsics.get();
            fIRGenerator->start(&settings, inherited);
            break;
        case Program::kGeometry_Kind:
            this->loadGeometryIntrinsics();
            inherited = &fGeometryInclude;
            fIRGenerator->fSymbolTable = fGeometrySymbolTable;
            fIRGenerator->fIntrinsics = fGPUIntrinsics.get();
            fIRGenerator->start(&settings, inherited);
            break;
        case Program::kFragmentProcessor_Kind: {
#if !SKSL_STANDALONE
            {
                Rehydrator rehydrator(fContext.get(), fGpuSymbolTable, this,
                                      SKSL_INCLUDE_sksl_fp,
                                      SKSL_INCLUDE_sksl_fp_LENGTH);
                fFPSymbolTable = rehydrator.symbolTable();
                fFPInclude = rehydrator.elements();
            }
            inherited = &fFPInclude;
            fIRGenerator->fSymbolTable = fFPSymbolTable;
            fIRGenerator->fIntrinsics = fGPUIntrinsics.get();
            fIRGenerator->start(&settings, inherited);
            break;
#else
            inherited = nullptr;
            fIRGenerator->fSymbolTable = fGpuSymbolTable;
            fIRGenerator->start(&settings, /*inherited=*/nullptr, /*builtin=*/true);
            fIRGenerator->fIntrinsics = fGPUIntrinsics.get();
            std::ifstream in(SKSL_FP_INCLUDE);
            std::string stdText{std::istreambuf_iterator<char>(in),
                                std::istreambuf_iterator<char>()};
            if (in.rdstate()) {
                printf("error reading %s\n", SKSL_FP_INCLUDE);
                abort();
            }
            const String* source = fGpuSymbolTable->takeOwnershipOfString(
                                                         std::make_unique<String>(stdText.c_str()));
            fIRGenerator->convertProgram(kind, source->c_str(), source->length(), &elements);
            fIRGenerator->fIsBuiltinCode = false;
            break;
#endif
        }
        case Program::kPipelineStage_Kind:
            this->loadPipelineIntrinsics();
            inherited = &fPipelineInclude;
            fIRGenerator->fSymbolTable = fPipelineSymbolTable;
            fIRGenerator->fIntrinsics = fGPUIntrinsics.get();
            fIRGenerator->start(&settings, inherited);
            break;
        case Program::kGeneric_Kind:
            this->loadInterpreterIntrinsics();
            inherited = &fInterpreterInclude;
            fIRGenerator->fSymbolTable = fInterpreterSymbolTable;
            fIRGenerator->fIntrinsics = fInterpreterIntrinsics.get();
            fIRGenerator->start(&settings, inherited);
            break;
    }
    std::unique_ptr<String> textPtr(new String(std::move(text)));
    fSource = textPtr.get();
    fIRGenerator->convertProgram(kind, textPtr->c_str(), textPtr->size(), &elements);
    auto result = std::make_unique<Program>(kind,
                                            std::move(textPtr),
                                            settings,
                                            fContext,
                                            inherited,
                                            std::move(elements),
                                            fIRGenerator->fSymbolTable,
                                            fIRGenerator->fInputs);
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

        // Scan and optimize based on the control-flow graph for each function.
        for (ProgramElement& element : program) {
            if (element.fKind == ProgramElement::kFunction_Kind) {
                this->scanCFG(element.as<FunctionDefinition>());
            }
        }

        // Remove dead functions. We wait until after analysis so that we still report errors, even
        // in unused code.
        if (program.fSettings.fRemoveDeadFunctions) {
            program.fElements.erase(
                    std::remove_if(program.fElements.begin(),
                                   program.fElements.end(),
                                   [](const std::unique_ptr<ProgramElement>& pe) {
                                       if (pe->fKind != ProgramElement::kFunction_Kind) {
                                           return false;
                                       }
                                       const FunctionDefinition& fn = pe->as<FunctionDefinition>();
                                       return fn.fDeclaration.fCallCount == 0 &&
                                              fn.fDeclaration.fName != "main";
                                   }),
                    program.fElements.end());
        }

        if (program.fKind != Program::kFragmentProcessor_Kind) {
            // Remove dead variables.
            for (ProgramElement& element : program) {
                if (element.fKind == ProgramElement::kVar_Kind) {
                    VarDeclarations& vars = element.as<VarDeclarations>();
                    vars.fVars.erase(
                            std::remove_if(vars.fVars.begin(), vars.fVars.end(),
                                           [](const std::unique_ptr<Statement>& stmt) {
                                               return stmt->as<VarDeclaration>().fVar->dead();
                                           }),
                            vars.fVars.end());
                }
            }

            // Remove empty variable declarations with no variables left inside of them.
            program.fElements.erase(
                    std::remove_if(program.fElements.begin(), program.fElements.end(),
                                   [](const std::unique_ptr<ProgramElement>& element) {
                                       return element->fKind == ProgramElement::kVar_Kind &&
                                              element->as<VarDeclarations>().fVars.empty();
                                   }),
                    program.fElements.end());
        }
    }
    return fErrorCount == 0;
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

bool Compiler::toHLSL(Program& program, String* out) {
    String spirv;
    if (!this->toSPIRV(program, &spirv)) {
        return false;
    }

    return SPIRVtoHLSL(spirv, out);
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

#if defined(SKSL_STANDALONE) || defined(GR_TEST_UTILS)
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
#endif // defined(SKSL_STANDALONE) || defined(GR_TEST_UTILS)

#endif // defined(SKSL_STANDALONE) || SK_SUPPORT_GPU

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
bool Compiler::toPipelineStage(Program& program, PipelineStageArgs* outArgs) {
    if (!this->optimize(program)) {
        return false;
    }
    fSource = program.fSource.get();
    StringStream buffer;
    PipelineStageCodeGenerator cg(fContext.get(), &program, this, &buffer, outArgs);
    bool result = cg.generateCode();
    fSource = nullptr;
    if (result) {
        outArgs->fCode = buffer.str();
    }
    return result;
}
#endif

std::unique_ptr<ByteCode> Compiler::toByteCode(Program& program) {
#if defined(SK_ENABLE_SKSL_INTERPRETER)
    if (!this->optimize(program)) {
        return nullptr;
    }
    fSource = program.fSource.get();
    std::unique_ptr<ByteCode> result(new ByteCode());
    ByteCodeGenerator cg(fContext.get(), &program, this, result.get());
    bool success = cg.generateCode();
    fSource = nullptr;
    if (success) {
        return result;
    }
#else
    ABORT("ByteCode interpreter not enabled");
#endif
    return nullptr;
}

const char* Compiler::OperatorName(Token::Kind kind) {
    switch (kind) {
        case Token::Kind::TK_PLUS:         return "+";
        case Token::Kind::TK_MINUS:        return "-";
        case Token::Kind::TK_STAR:         return "*";
        case Token::Kind::TK_SLASH:        return "/";
        case Token::Kind::TK_PERCENT:      return "%";
        case Token::Kind::TK_SHL:          return "<<";
        case Token::Kind::TK_SHR:          return ">>";
        case Token::Kind::TK_LOGICALNOT:   return "!";
        case Token::Kind::TK_LOGICALAND:   return "&&";
        case Token::Kind::TK_LOGICALOR:    return "||";
        case Token::Kind::TK_LOGICALXOR:   return "^^";
        case Token::Kind::TK_BITWISENOT:   return "~";
        case Token::Kind::TK_BITWISEAND:   return "&";
        case Token::Kind::TK_BITWISEOR:    return "|";
        case Token::Kind::TK_BITWISEXOR:   return "^";
        case Token::Kind::TK_EQ:           return "=";
        case Token::Kind::TK_EQEQ:         return "==";
        case Token::Kind::TK_NEQ:          return "!=";
        case Token::Kind::TK_LT:           return "<";
        case Token::Kind::TK_GT:           return ">";
        case Token::Kind::TK_LTEQ:         return "<=";
        case Token::Kind::TK_GTEQ:         return ">=";
        case Token::Kind::TK_PLUSEQ:       return "+=";
        case Token::Kind::TK_MINUSEQ:      return "-=";
        case Token::Kind::TK_STAREQ:       return "*=";
        case Token::Kind::TK_SLASHEQ:      return "/=";
        case Token::Kind::TK_PERCENTEQ:    return "%=";
        case Token::Kind::TK_SHLEQ:        return "<<=";
        case Token::Kind::TK_SHREQ:        return ">>=";
        case Token::Kind::TK_LOGICALANDEQ: return "&&=";
        case Token::Kind::TK_LOGICALOREQ:  return "||=";
        case Token::Kind::TK_LOGICALXOREQ: return "^^=";
        case Token::Kind::TK_BITWISEANDEQ: return "&=";
        case Token::Kind::TK_BITWISEOREQ:  return "|=";
        case Token::Kind::TK_BITWISEXOREQ: return "^=";
        case Token::Kind::TK_PLUSPLUS:     return "++";
        case Token::Kind::TK_MINUSMINUS:   return "--";
        case Token::Kind::TK_COMMA:        return ",";
        default:
            ABORT("unsupported operator: %d\n", (int) kind);
    }
}


bool Compiler::IsAssignment(Token::Kind op) {
    switch (op) {
        case Token::Kind::TK_EQ:           // fall through
        case Token::Kind::TK_PLUSEQ:       // fall through
        case Token::Kind::TK_MINUSEQ:      // fall through
        case Token::Kind::TK_STAREQ:       // fall through
        case Token::Kind::TK_SLASHEQ:      // fall through
        case Token::Kind::TK_PERCENTEQ:    // fall through
        case Token::Kind::TK_SHLEQ:        // fall through
        case Token::Kind::TK_SHREQ:        // fall through
        case Token::Kind::TK_BITWISEOREQ:  // fall through
        case Token::Kind::TK_BITWISEXOREQ: // fall through
        case Token::Kind::TK_BITWISEANDEQ: // fall through
        case Token::Kind::TK_LOGICALOREQ:  // fall through
        case Token::Kind::TK_LOGICALXOREQ: // fall through
        case Token::Kind::TK_LOGICALANDEQ:
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

}  // namespace SkSL
