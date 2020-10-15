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
#include "src/utils/SkBitSet.h"

#include <fstream>

#if !defined(SKSL_STANDALONE) & SK_SUPPORT_GPU
#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrShaderCaps.h"
#endif

#ifdef SK_ENABLE_SPIRV_VALIDATION
#include "spirv-tools/libspirv.hpp"
#endif

#if defined(SKSL_STANDALONE)

// In standalone mode, we load the textual sksl source files. GN generates or copies these files
// to the skslc executable directory. The "data" in this mode is just the filename.
#define MODULE_DATA(name) MakeModulePath("sksl_" #name ".sksl")

#else

// At runtime, we load the dehydrated sksl data files. The data is a (pointer, size) pair.
#include "src/sksl/generated/sksl_fp.dehydrated.sksl"
#include "src/sksl/generated/sksl_frag.dehydrated.sksl"
#include "src/sksl/generated/sksl_geom.dehydrated.sksl"
#include "src/sksl/generated/sksl_gpu.dehydrated.sksl"
#include "src/sksl/generated/sksl_interp.dehydrated.sksl"
#include "src/sksl/generated/sksl_pipeline.dehydrated.sksl"
#include "src/sksl/generated/sksl_vert.dehydrated.sksl"

#define MODULE_DATA(name) MakeModuleData(SKSL_INCLUDE_sksl_##name,\
                                         SKSL_INCLUDE_sksl_##name##_LENGTH)

#endif

namespace SkSL {

class AutoSource {
public:
    AutoSource(Compiler* compiler, const String* source)
            : fCompiler(compiler), fOldSource(fCompiler->fSource) {
        fCompiler->fSource = source;
    }

    ~AutoSource() { fCompiler->fSource = fOldSource; }

    Compiler* fCompiler;
    const String* fOldSource;
};

Compiler::Compiler(Flags flags)
: fFlags(flags)
, fContext(std::make_shared<Context>())
, fErrorCount(0) {
    fRootSymbolTable = std::make_shared<SymbolTable>(this);
    fIRGenerator = std::make_unique<IRGenerator>(fContext.get(), &fInliner, *this);
#define ADD_TYPE(t) fRootSymbolTable->addWithoutOwnership(fContext->f##t##_Type.get())
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

    // sk_Caps is "builtin", but all references to it are resolved to Settings, so we don't need to
    // treat it as builtin (ie, no need to clone it into the Program).
    StringFragment skCapsName("sk_Caps");
    fRootSymbolTable->add(std::make_unique<Variable>(/*offset=*/-1,
                                                     fIRGenerator->fModifiers->handle(Modifiers()),
                                                     skCapsName,
                                                     fContext->fSkCaps_Type.get(),
                                                     /*builtin=*/false,
                                                     Variable::Storage::kGlobal));

    fRootModule = {fRootSymbolTable, /*fIntrinsics=*/nullptr};

    fGPUModule = this->parseModule(Program::kFragment_Kind, MODULE_DATA(gpu), fRootModule);
    fVertexModule = this->parseModule(Program::kVertex_Kind, MODULE_DATA(vert), fGPUModule);
    fFragmentModule = this->parseModule(Program::kFragment_Kind, MODULE_DATA(frag), fGPUModule);
}

Compiler::~Compiler() {}

const ParsedModule& Compiler::loadGeometryModule() {
    if (!fGeometryModule.fSymbols) {
        fGeometryModule = this->parseModule(Program::kGeometry_Kind, MODULE_DATA(geom), fGPUModule);
    }
    return fGeometryModule;
}

const ParsedModule& Compiler::loadFPModule() {
    if (!fFPModule.fSymbols) {
        fFPModule =
                this->parseModule(Program::kFragmentProcessor_Kind, MODULE_DATA(fp), fGPUModule);
    }
    return fFPModule;
}

const ParsedModule& Compiler::loadPipelineModule() {
    if (!fPipelineModule.fSymbols) {
        fPipelineModule =
                this->parseModule(Program::kPipelineStage_Kind, MODULE_DATA(pipeline), fGPUModule);

        // Add some aliases to the pipeline module so that it's friendlier, and more like GLSL
        fPipelineModule.fSymbols->addAlias("shader", fContext->fFragmentProcessor_Type.get());

        fPipelineModule.fSymbols->addAlias("vec2", fContext->fFloat2_Type.get());
        fPipelineModule.fSymbols->addAlias("vec3", fContext->fFloat3_Type.get());
        fPipelineModule.fSymbols->addAlias("vec4", fContext->fFloat4_Type.get());

        fPipelineModule.fSymbols->addAlias("bvec2", fContext->fBool2_Type.get());
        fPipelineModule.fSymbols->addAlias("bvec3", fContext->fBool3_Type.get());
        fPipelineModule.fSymbols->addAlias("bvec4", fContext->fBool4_Type.get());

        fPipelineModule.fSymbols->addAlias("mat2", fContext->fFloat2x2_Type.get());
        fPipelineModule.fSymbols->addAlias("mat3", fContext->fFloat3x3_Type.get());
        fPipelineModule.fSymbols->addAlias("mat4", fContext->fFloat4x4_Type.get());

        fPipelineModule.fSymbols->addAlias("mat2x2", fContext->fFloat2x2_Type.get());
        fPipelineModule.fSymbols->addAlias("mat2x3", fContext->fFloat2x3_Type.get());
        fPipelineModule.fSymbols->addAlias("mat2x4", fContext->fFloat2x4_Type.get());

        fPipelineModule.fSymbols->addAlias("mat3x2", fContext->fFloat3x2_Type.get());
        fPipelineModule.fSymbols->addAlias("mat3x3", fContext->fFloat3x3_Type.get());
        fPipelineModule.fSymbols->addAlias("mat3x4", fContext->fFloat3x4_Type.get());

        fPipelineModule.fSymbols->addAlias("mat4x2", fContext->fFloat4x2_Type.get());
        fPipelineModule.fSymbols->addAlias("mat4x3", fContext->fFloat4x3_Type.get());
        fPipelineModule.fSymbols->addAlias("mat4x4", fContext->fFloat4x4_Type.get());
    }
    return fPipelineModule;
}

const ParsedModule& Compiler::loadInterpreterModule() {
    if (!fInterpreterModule.fSymbols) {
        fInterpreterModule =
                this->parseModule(Program::kGeneric_Kind, MODULE_DATA(interp), fRootModule);
    }
    return fInterpreterModule;
}

const ParsedModule& Compiler::moduleForProgramKind(Program::Kind kind) {
    switch (kind) {
        case Program::kVertex_Kind:            return fVertexModule;                 break;
        case Program::kFragment_Kind:          return fFragmentModule;               break;
        case Program::kGeometry_Kind:          return this->loadGeometryModule();    break;
        case Program::kFragmentProcessor_Kind: return this->loadFPModule();          break;
        case Program::kPipelineStage_Kind:     return this->loadPipelineModule();    break;
        case Program::kGeneric_Kind:           return this->loadInterpreterModule(); break;
    }
    SkUNREACHABLE;
}

LoadedModule Compiler::loadModule(Program::Kind kind,
                                  ModuleData data,
                                  std::shared_ptr<SymbolTable> base) {
    if (!base) {
        base = fRootSymbolTable;
    }

#if defined(SKSL_STANDALONE)
    SkASSERT(data.fPath);
    std::ifstream in(data.fPath);
    std::unique_ptr<String> text = std::make_unique<String>(std::istreambuf_iterator<char>(in),
                                                            std::istreambuf_iterator<char>());
    if (in.rdstate()) {
        printf("error reading %s\n", data.fPath);
        abort();
    }
    const String* source = fRootSymbolTable->takeOwnershipOfString(std::move(text));
    AutoSource as(this, source);
    Program::Settings settings;
    SkASSERT(fIRGenerator->fCanInline);
    fIRGenerator->fCanInline = false;
    ParsedModule baseModule = {base, /*fIntrinsics=*/nullptr};
    IRGenerator::IRBundle ir = fIRGenerator->convertProgram(
            kind, &settings, baseModule, /*isBuiltinCode=*/true, source->c_str(), source->length(),
            /*externalValues=*/nullptr);
    LoadedModule module = { std::move(ir.fSymbolTable), std::move(ir.fElements) };
    fIRGenerator->fCanInline = true;
    if (this->fErrorCount) {
        printf("Unexpected errors: %s\n", this->fErrorText.c_str());
        SkDEBUGFAILF("%s %s\n", data.fPath, this->fErrorText.c_str());
    }
    fModifiers.push_back(std::move(ir.fModifiers));
#else
    SkASSERT(data.fData && (data.fSize != 0));
    Rehydrator rehydrator(fContext.get(), fIRGenerator->fModifiers.get(), base, this,
                          data.fData, data.fSize);
    LoadedModule module = { rehydrator.symbolTable(), rehydrator.elements() };
    fModifiers.push_back(fIRGenerator->releaseModifiers());
#endif

    return module;
}

ParsedModule Compiler::parseModule(Program::Kind kind, ModuleData data, const ParsedModule& base) {
    auto [symbols, elements] = this->loadModule(kind, data, base.fSymbols);

    // For modules that just declare (but don't define) intrinsic functions, there will be no new
    // program elements. In that case, we can share our parent's intrinsic map:
    if (elements.empty()) {
        return {symbols, base.fIntrinsics};
    }

    auto intrinsics = std::make_shared<IRIntrinsicMap>(base.fIntrinsics.get());

    // Now, transfer all of the program elements to an intrinsic map. This maps certain types of
    // global objects to the declaring ProgramElement.
    for (std::unique_ptr<ProgramElement>& element : elements) {
        switch (element->kind()) {
            case ProgramElement::Kind::kFunction: {
                const FunctionDefinition& f = element->as<FunctionDefinition>();
                SkASSERT(f.declaration().isBuiltin());
                intrinsics->insertOrDie(f.declaration().description(), std::move(element));
                break;
            }
            case ProgramElement::Kind::kEnum: {
                const Enum& e = element->as<Enum>();
                SkASSERT(e.isBuiltin());
                intrinsics->insertOrDie(e.typeName(), std::move(element));
                break;
            }
            case ProgramElement::Kind::kGlobalVar: {
                const GlobalVarDeclaration& global = element->as<GlobalVarDeclaration>();
                const Variable& var = global.declaration()->as<VarDeclaration>().var();
                SkASSERT(var.isBuiltin());
                intrinsics->insertOrDie(var.name(), std::move(element));
                break;
            }
            case ProgramElement::Kind::kInterfaceBlock: {
                const Variable& var = element->as<InterfaceBlock>().variable();
                SkASSERT(var.isBuiltin());
                intrinsics->insertOrDie(var.name(), std::move(element));
                break;
            }
            default:
                printf("Unsupported element: %s\n", element->description().c_str());
                SkASSERT(false);
                break;
        }
    }

    return {symbols, std::move(intrinsics)};
}

// add the definition created by assigning to the lvalue to the definition set
void Compiler::addDefinition(const Expression* lvalue, std::unique_ptr<Expression>* expr,
                             DefinitionMap* definitions) {
    switch (lvalue->kind()) {
        case Expression::Kind::kVariableReference: {
            const Variable& var = *lvalue->as<VariableReference>().variable();
            if (var.storage() == Variable::Storage::kLocal) {
                definitions->set(&var, expr);
            }
            break;
        }
        case Expression::Kind::kSwizzle:
            // We consider the variable written to as long as at least some of its components have
            // been written to. This will lead to some false negatives (we won't catch it if you
            // write to foo.x and then read foo.y), but being stricter could lead to false positives
            // (we write to foo.x, and then pass foo to a function which happens to only read foo.x,
            // but since we pass foo as a whole it is flagged as an error) unless we perform a much
            // more complicated whole-program analysis. This is probably good enough.
            this->addDefinition(lvalue->as<Swizzle>().base().get(),
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            break;
        case Expression::Kind::kIndex:
            // see comments in Swizzle
            this->addDefinition(lvalue->as<IndexExpression>().base().get(),
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            break;
        case Expression::Kind::kFieldAccess:
            // see comments in Swizzle
            this->addDefinition(lvalue->as<FieldAccess>().base().get(),
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            break;
        case Expression::Kind::kTernary:
            // To simplify analysis, we just pretend that we write to both sides of the ternary.
            // This allows for false positives (meaning we fail to detect that a variable might not
            // have been assigned), but is preferable to false negatives.
            this->addDefinition(lvalue->as<TernaryExpression>().ifTrue().get(),
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            this->addDefinition(lvalue->as<TernaryExpression>().ifFalse().get(),
                                (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                definitions);
            break;
        case Expression::Kind::kExternalValue:
            break;
        default:
            // not an lvalue, can't happen
            SkASSERT(false);
    }
}

// add local variables defined by this node to the set
void Compiler::addDefinitions(const BasicBlock::Node& node, DefinitionMap* definitions) {
    if (node.isExpression()) {
        Expression* expr = node.expression()->get();
        switch (expr->kind()) {
            case Expression::Kind::kBinary: {
                BinaryExpression* b = &expr->as<BinaryExpression>();
                if (b->getOperator() == Token::Kind::TK_EQ) {
                    this->addDefinition(&b->left(), &b->rightPointer(), definitions);
                } else if (Compiler::IsAssignment(b->getOperator())) {
                    this->addDefinition(
                                  &b->left(),
                                  (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                  definitions);

                }
                break;
            }
            case Expression::Kind::kFunctionCall: {
                const FunctionCall& c = expr->as<FunctionCall>();
                const std::vector<const Variable*>& parameters = c.function().parameters();
                for (size_t i = 0; i < parameters.size(); ++i) {
                    if (parameters[i]->modifiers().fFlags & Modifiers::kOut_Flag) {
                        this->addDefinition(
                                  c.arguments()[i].get(),
                                  (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                  definitions);
                    }
                }
                break;
            }
            case Expression::Kind::kPrefix: {
                const PrefixExpression* p = &expr->as<PrefixExpression>();
                if (p->getOperator() == Token::Kind::TK_MINUSMINUS ||
                    p->getOperator() == Token::Kind::TK_PLUSPLUS) {
                    this->addDefinition(
                                  p->operand().get(),
                                  (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                  definitions);
                }
                break;
            }
            case Expression::Kind::kPostfix: {
                const PostfixExpression* p = &expr->as<PostfixExpression>();
                if (p->getOperator() == Token::Kind::TK_MINUSMINUS ||
                    p->getOperator() == Token::Kind::TK_PLUSPLUS) {
                    this->addDefinition(
                                  p->operand().get(),
                                  (std::unique_ptr<Expression>*) &fContext->fDefined_Expression,
                                  definitions);
                }
                break;
            }
            case Expression::Kind::kVariableReference: {
                const VariableReference* v = &expr->as<VariableReference>();
                if (v->refKind() != VariableReference::RefKind::kRead) {
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
    } else if (node.isStatement()) {
        Statement* stmt = node.statement()->get();
        if (stmt->is<VarDeclaration>()) {
            VarDeclaration& vd = stmt->as<VarDeclaration>();
            if (vd.value()) {
                definitions->set(&vd.var(), &vd.value());
            }
        }
    }
}

void Compiler::scanCFG(CFG* cfg, BlockId blockId, SkBitSet* processedSet) {
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
        after.foreach([&](const Variable* var, std::unique_ptr<Expression>** e1Ptr) {
            std::unique_ptr<Expression>* e1 = *e1Ptr;
            std::unique_ptr<Expression>** exitDef = exit.fBefore.find(var);
            if (!exitDef) {
                // exit has no definition for it, just copy it and reprocess exit block
                processedSet->reset(exitId);
                exit.fBefore[var] = e1;
            } else {
                // exit has a (possibly different) value already defined
                std::unique_ptr<Expression>* e2 = *exitDef;
                if (e1 != e2) {
                    // definition has changed, merge and reprocess the exit block
                    processedSet->reset(exitId);
                    if (e1 && e2) {
                        *exitDef = (std::unique_ptr<Expression>*)&fContext->fDefined_Expression;
                    } else {
                        *exitDef = nullptr;
                    }
                }
            }
        });
    }
}

// returns a map which maps all local variables in the function to null, indicating that their value
// is initially unknown
static DefinitionMap compute_start_state(const CFG& cfg) {
    DefinitionMap result;
    for (const auto& block : cfg.fBlocks) {
        for (const auto& node : block.fNodes) {
            if (node.isStatement()) {
                const Statement* s = node.statement()->get();
                if (s->is<VarDeclaration>()) {
                    result[&s->as<VarDeclaration>().var()] = nullptr;
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
    switch (lvalue.kind()) {
        case Expression::Kind::kVariableReference:
            return lvalue.as<VariableReference>().variable()->dead();
        case Expression::Kind::kSwizzle:
            return is_dead(*lvalue.as<Swizzle>().base());
        case Expression::Kind::kFieldAccess:
            return is_dead(*lvalue.as<FieldAccess>().base());
        case Expression::Kind::kIndex: {
            const IndexExpression& idx = lvalue.as<IndexExpression>();
            return is_dead(*idx.base()) &&
                   !idx.index()->hasProperty(Expression::Property::kSideEffects);
        }
        case Expression::Kind::kTernary: {
            const TernaryExpression& t = lvalue.as<TernaryExpression>();
            return !t.test()->hasSideEffects() && is_dead(*t.ifTrue()) && is_dead(*t.ifFalse());
        }
        case Expression::Kind::kExternalValue:
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
    if (!Compiler::IsAssignment(b.getOperator())) {
        return false;
    }
    return is_dead(b.left());
}

void Compiler::computeDataFlow(CFG* cfg) {
    cfg->fBlocks[cfg->fStart].fBefore = compute_start_state(*cfg);

    // We set bits in the "processed" set after a block has been scanned.
    SkBitSet processedSet(cfg->fBlocks.size());
    while (SkBitSet::OptionalIndex blockId = processedSet.findFirstUnset()) {
        processedSet.set(*blockId);
        this->scanCFG(cfg, *blockId, &processedSet);
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
template <typename T = SKSL_FLOAT>
static bool is_constant(const Expression& expr, T value) {
    switch (expr.kind()) {
        case Expression::Kind::kIntLiteral:
            return expr.as<IntLiteral>().value() == value;

        case Expression::Kind::kFloatLiteral:
            return expr.as<FloatLiteral>().value() == value;

        case Expression::Kind::kConstructor: {
            const Constructor& constructor = expr.as<Constructor>();
            if (constructor.isCompileTimeConstant()) {
                const Type& constructorType = constructor.type();
                bool isFloat = constructorType.columns() > 1
                                       ? constructorType.componentType().isFloat()
                                       : constructorType.isFloat();
                switch (constructorType.typeKind()) {
                    case Type::TypeKind::kVector:
                        for (int i = 0; i < constructorType.columns(); ++i) {
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

                    case Type::TypeKind::kScalar:
                        SkASSERT(constructor.arguments().size() == 1);
                        return is_constant<T>(*constructor.arguments()[0], value);

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
    Expression& left = bin.left();
    std::unique_ptr<Expression>& rightPointer = bin.rightPointer();
    SkASSERT(!left.hasSideEffects());
    bool result;
    if (bin.getOperator() == Token::Kind::TK_EQ) {
        result = b->tryRemoveLValueBefore(iter, &left);
    } else {
        result = b->tryRemoveExpressionBefore(iter, &left);
    }
    *target = std::move(rightPointer);
    if (!result) {
        *outNeedsRescan = true;
        return;
    }
    if (*iter == b->fNodes.begin()) {
        *outNeedsRescan = true;
        return;
    }
    --(*iter);
    if (!(*iter)->isExpression() || (*iter)->expression() != &rightPointer) {
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
    std::unique_ptr<Expression>& leftPointer = bin.leftPointer();
    Expression& right = bin.right();
    SkASSERT(!right.hasSideEffects());
    if (!b->tryRemoveExpressionBefore(iter, &right)) {
        *target = std::move(leftPointer);
        *outNeedsRescan = true;
        return;
    }
    *target = std::move(leftPointer);
    if (*iter == b->fNodes.begin()) {
        *outNeedsRescan = true;
        return;
    }
    --(*iter);
    if ((!(*iter)->isExpression() || (*iter)->expression() != &leftPointer)) {
        *outNeedsRescan = true;
        return;
    }
    *iter = b->fNodes.erase(*iter);
    SkASSERT((*iter)->expression() == target);
}

/**
 * Constructs the specified type using a single argument.
 */
static std::unique_ptr<Expression> construct(const Type* type, std::unique_ptr<Expression> v) {
    ExpressionArray args;
    args.push_back(std::move(v));
    std::unique_ptr<Expression> result = std::make_unique<Constructor>(-1, type, std::move(args));
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
    SkASSERT((*(*iter)->expression())->kind() == Expression::Kind::kBinary);
    SkASSERT(type.typeKind() == Type::TypeKind::kVector);
    SkASSERT((*otherExpression)->type().typeKind() == Type::TypeKind::kScalar);
    *outUpdated = true;
    std::unique_ptr<Expression>* target = (*iter)->expression();
    if (!b->tryRemoveExpression(iter)) {
        *target = construct(&type, std::move(*otherExpression));
        *outNeedsRescan = true;
    } else {
        *target = construct(&type, std::move(*otherExpression));
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
    vectorize(b, iter, bin.right().type(), &bin.leftPointer(), outUpdated, outNeedsRescan);
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
    vectorize(b, iter, bin.left().type(), &bin.rightPointer(), outUpdated, outNeedsRescan);
}

// Mark that an expression which we were writing to is no longer being written to
static void clear_write(Expression& expr) {
    switch (expr.kind()) {
        case Expression::Kind::kVariableReference: {
            expr.as<VariableReference>().setRefKind(VariableReference::RefKind::kRead);
            break;
        }
        case Expression::Kind::kFieldAccess:
            clear_write(*expr.as<FieldAccess>().base());
            break;
        case Expression::Kind::kSwizzle:
            clear_write(*expr.as<Swizzle>().base());
            break;
        case Expression::Kind::kIndex:
            clear_write(*expr.as<IndexExpression>().base());
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
            optimized = fIRGenerator->coerce(std::move(optimized), expr->type());
            SkASSERT(optimized);
            if (!try_replace_expression(&b, iter, &optimized)) {
                *outNeedsRescan = true;
                return;
            }
            SkASSERT((*iter)->isExpression());
            expr = (*iter)->expression()->get();
        }
    }
    switch (expr->kind()) {
        case Expression::Kind::kVariableReference: {
            const VariableReference& ref = expr->as<VariableReference>();
            const Variable* var = ref.variable();
            if (ref.refKind() != VariableReference::RefKind::kWrite &&
                ref.refKind() != VariableReference::RefKind::kPointer &&
                var->storage() == Variable::Storage::kLocal && !definitions[var] &&
                (*undefinedVariables).find(var) == (*undefinedVariables).end()) {
                (*undefinedVariables).insert(var);
                this->error(expr->fOffset,
                            "'" + var->name() + "' has not been assigned");
            }
            break;
        }
        case Expression::Kind::kTernary: {
            TernaryExpression* t = &expr->as<TernaryExpression>();
            if (t->test()->is<BoolLiteral>()) {
                // ternary has a constant test, replace it with either the true or
                // false branch
                if (t->test()->as<BoolLiteral>().value()) {
                    (*iter)->setExpression(std::move(t->ifTrue()));
                } else {
                    (*iter)->setExpression(std::move(t->ifFalse()));
                }
                *outUpdated = true;
                *outNeedsRescan = true;
            }
            break;
        }
        case Expression::Kind::kBinary: {
            BinaryExpression* bin = &expr->as<BinaryExpression>();
            if (dead_assignment(*bin)) {
                delete_left(&b, iter, outUpdated, outNeedsRescan);
                break;
            }
            Expression& left = bin->left();
            Expression& right = bin->right();
            const Type& leftType = left.type();
            const Type& rightType = right.type();
            // collapse useless expressions like x * 1 or x + 0
            if (((leftType.typeKind() != Type::TypeKind::kScalar) &&
                 (leftType.typeKind() != Type::TypeKind::kVector)) ||
                ((rightType.typeKind() != Type::TypeKind::kScalar) &&
                 (rightType.typeKind() != Type::TypeKind::kVector))) {
                break;
            }
            switch (bin->getOperator()) {
                case Token::Kind::TK_STAR:
                    if (is_constant(left, 1)) {
                        if (leftType.typeKind() == Type::TypeKind::kVector &&
                            rightType.typeKind() == Type::TypeKind::kScalar) {
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
                        if (leftType.typeKind() == Type::TypeKind::kScalar &&
                            rightType.typeKind() == Type::TypeKind::kVector &&
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
                        if (leftType.typeKind() == Type::TypeKind::kScalar &&
                            rightType.typeKind() == Type::TypeKind::kVector) {
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
                        if (leftType.typeKind() == Type::TypeKind::kVector &&
                            rightType.typeKind() == Type::TypeKind::kScalar &&
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
                case Token::Kind::TK_PLUS:
                    if (is_constant(left, 0)) {
                        if (leftType.typeKind() == Type::TypeKind::kVector &&
                            rightType.typeKind() == Type::TypeKind::kScalar) {
                            // float4(0) + x -> float4(x)
                            vectorize_right(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // 0 + x -> x
                            // 0 + float4(x) -> float4(x)
                            // float4(0) + float4(x) -> float4(x)
                            delete_left(&b, iter, outUpdated, outNeedsRescan);
                        }
                    } else if (is_constant(right, 0)) {
                        if (leftType.typeKind() == Type::TypeKind::kScalar &&
                            rightType.typeKind() == Type::TypeKind::kVector) {
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
                    if (is_constant(right, 0)) {
                        if (leftType.typeKind() == Type::TypeKind::kScalar &&
                            rightType.typeKind() == Type::TypeKind::kVector) {
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
                    if (is_constant(right, 1)) {
                        if (leftType.typeKind() == Type::TypeKind::kScalar &&
                            rightType.typeKind() == Type::TypeKind::kVector) {
                            // x / float4(1) -> float4(x)
                            vectorize_left(&b, iter, outUpdated, outNeedsRescan);
                        } else {
                            // x / 1 -> x
                            // float4(x) / 1 -> float4(x)
                            // float4(x) / float4(1) -> float4(x)
                            delete_right(&b, iter, outUpdated, outNeedsRescan);
                        }
                    } else if (is_constant(left, 0)) {
                        if (leftType.typeKind() == Type::TypeKind::kScalar &&
                            rightType.typeKind() == Type::TypeKind::kVector &&
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
                case Token::Kind::TK_PLUSEQ:
                    if (is_constant(right, 0)) {
                        clear_write(left);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                case Token::Kind::TK_MINUSEQ:
                    if (is_constant(right, 0)) {
                        clear_write(left);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                case Token::Kind::TK_STAREQ:
                    if (is_constant(right, 1)) {
                        clear_write(left);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                case Token::Kind::TK_SLASHEQ:
                    if (is_constant(right, 1)) {
                        clear_write(left);
                        delete_right(&b, iter, outUpdated, outNeedsRescan);
                    }
                    break;
                default:
                    break;
            }
            break;
        }
        case Expression::Kind::kSwizzle: {
            Swizzle& s = expr->as<Swizzle>();
            // detect identity swizzles like foo.rgba
            if ((int) s.components().size() == s.base()->type().columns()) {
                bool identity = true;
                for (int i = 0; i < (int) s.components().size(); ++i) {
                    if (s.components()[i] != i) {
                        identity = false;
                        break;
                    }
                }
                if (identity) {
                    *outUpdated = true;
                    if (!try_replace_expression(&b, iter, &s.base())) {
                        *outNeedsRescan = true;
                        return;
                    }
                    SkASSERT((*iter)->isExpression());
                    break;
                }
            }
            // detect swizzles of swizzles, e.g. replace foo.argb.r000 with foo.a000
            if (s.base()->kind() == Expression::Kind::kSwizzle) {
                Swizzle& base = s.base()->as<Swizzle>();
                std::vector<int> final;
                for (int c : s.components()) {
                    final.push_back(base.components()[c]);
                }
                *outUpdated = true;
                std::unique_ptr<Expression> replacement(new Swizzle(*fContext, base.base()->clone(),
                                                                    std::move(final)));
                if (!try_replace_expression(&b, iter, &replacement)) {
                    *outNeedsRescan = true;
                    return;
                }
                SkASSERT((*iter)->isExpression());
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
            switch (stmt.kind()) {
                case Statement::Kind::kBlock:
                    return this->INHERITED::visitStatement(stmt);

                case Statement::Kind::kBreak:
                    return fInConditional > 0;

                case Statement::Kind::kIf: {
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
            switch (stmt.kind()) {
                case Statement::Kind::kBlock:
                    return this->INHERITED::visitStatement(stmt);

                case Statement::Kind::kBreak:
                    return true;

                default:
                    return false;
            }
        }

        using INHERITED = ProgramVisitor;
    };

    return ContainsUnconditionalBreak{}.visitStatement(stmt);
}

static void move_all_but_break(std::unique_ptr<Statement>& stmt, StatementArray* target) {
    switch (stmt->kind()) {
        case Statement::Kind::kBlock: {
            // Recurse into the block.
            Block& block = static_cast<Block&>(*stmt);

            StatementArray blockStmts;
            blockStmts.reserve_back(block.children().size());
            for (std::unique_ptr<Statement>& stmt : block.children()) {
                move_all_but_break(stmt, &blockStmts);
            }

            target->push_back(std::make_unique<Block>(block.fOffset, std::move(blockStmts),
                                                      block.symbolTable(), block.isScope()));
            break;
        }

        case Statement::Kind::kBreak:
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
    StatementArray caseStmts;

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
    switch (stmt->kind()) {
        case Statement::Kind::kVarDeclaration: {
            const auto& varDecl = stmt->as<VarDeclaration>();
            if (varDecl.var().dead() &&
                (!varDecl.value() ||
                 !varDecl.value()->hasSideEffects())) {
                if (varDecl.value()) {
                    SkASSERT((*iter)->statement()->get() == stmt);
                    if (!b.tryRemoveExpressionBefore(iter, varDecl.value().get())) {
                        *outNeedsRescan = true;
                    }
                }
                (*iter)->setStatement(std::unique_ptr<Statement>(new Nop()));
                *outUpdated = true;
            }
            break;
        }
        case Statement::Kind::kIf: {
            IfStatement& i = stmt->as<IfStatement>();
            if (i.test()->kind() == Expression::Kind::kBoolLiteral) {
                // constant if, collapse down to a single branch
                if (i.test()->as<BoolLiteral>().value()) {
                    SkASSERT(i.ifTrue());
                    (*iter)->setStatement(std::move(i.ifTrue()));
                } else {
                    if (i.ifFalse()) {
                        (*iter)->setStatement(std::move(i.ifFalse()));
                    } else {
                        (*iter)->setStatement(std::unique_ptr<Statement>(new Nop()));
                    }
                }
                *outUpdated = true;
                *outNeedsRescan = true;
                break;
            }
            if (i.ifFalse() && i.ifFalse()->isEmpty()) {
                // else block doesn't do anything, remove it
                i.ifFalse().reset();
                *outUpdated = true;
                *outNeedsRescan = true;
            }
            if (!i.ifFalse() && i.ifTrue()->isEmpty()) {
                // if block doesn't do anything, no else block
                if (i.test()->hasSideEffects()) {
                    // test has side effects, keep it
                    (*iter)->setStatement(std::unique_ptr<Statement>(
                                                     new ExpressionStatement(std::move(i.test()))));
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
        case Statement::Kind::kSwitch: {
            SwitchStatement& s = stmt->as<SwitchStatement>();
            int64_t switchValue;
            if (fIRGenerator->getConstantInt(*s.fValue, &switchValue)) {
                // switch is constant, replace it with the case that matches
                bool found = false;
                SwitchCase* defaultCase = nullptr;
                for (const std::unique_ptr<SwitchCase>& c : s.fCases) {
                    if (!c->fValue) {
                        defaultCase = c.get();
                        continue;
                    }
                    int64_t caseValue;
                    SkAssertResult(fIRGenerator->getConstantInt(*c->fValue, &caseValue));
                    if (caseValue == switchValue) {
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
        case Statement::Kind::kExpression: {
            ExpressionStatement& e = stmt->as<ExpressionStatement>();
            SkASSERT((*iter)->statement()->get() == &e);
            if (!e.expression()->hasSideEffects()) {
                // Expression statement with no side effects, kill it
                if (!b.tryRemoveExpressionBefore(iter, e.expression().get())) {
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

bool Compiler::scanCFG(FunctionDefinition& f) {
    bool madeChanges = false;

    CFG cfg = CFGGenerator().getCFG(f);
    this->computeDataFlow(&cfg);

    // check for unreachable code
    for (size_t i = 0; i < cfg.fBlocks.size(); i++) {
        const BasicBlock& block = cfg.fBlocks[i];
        if (i != cfg.fStart && !block.fIsReachable && block.fNodes.size()) {
            int offset;
            const BasicBlock::Node& node = block.fNodes[0];
            if (node.isStatement()) {
                offset = (*node.statement())->fOffset;
            } else {
                offset = (*node.expression())->fOffset;
                if ((*node.expression())->is<BoolLiteral>()) {
                    // Function inlining can generate do { ... } while(false) loops which always
                    // break, so the boolean condition is considered unreachable. Since not being
                    // able to reach a literal is a non-issue in the first place, we don't report an
                    // error in this case.
                    continue;
                }
            }
            this->error(offset, String("unreachable"));
        }
    }
    if (fErrorCount) {
        return madeChanges;
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
            if (!first && !b.fIsReachable) {
                // Block was reachable before optimization, but has since become unreachable. In
                // addition to being dead code, it's broken - since control flow can't reach it, no
                // prior variable definitions can reach it, and therefore variables might look to
                // have not been properly assigned. Kill it by replacing all statements with Nops.
                for (BasicBlock::Node& node : b.fNodes) {
                    if (node.isStatement() && !(*node.statement())->is<Nop>()) {
                        node.setStatement(std::make_unique<Nop>());
                        madeChanges = true;
                    }
                }
                continue;
            }
            first = false;
            DefinitionMap definitions = b.fBefore;

            for (auto iter = b.fNodes.begin(); iter != b.fNodes.end() && !needsRescan; ++iter) {
                if (iter->isExpression()) {
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

            if (needsRescan) {
                break;
            }
        }
        madeChanges |= updated;
    } while (updated);
    SkASSERT(!needsRescan);

    // verify static ifs & switches, clean up dead variable decls
    for (BasicBlock& b : cfg.fBlocks) {
        for (auto iter = b.fNodes.begin(); iter != b.fNodes.end() && !needsRescan;) {
            if (iter->isStatement()) {
                const Statement& s = **iter->statement();
                switch (s.kind()) {
                    case Statement::Kind::kIf:
                        if (s.as<IfStatement>().isStatic() &&
                            !(fFlags & kPermitInvalidStaticTests_Flag)) {
                            this->error(s.fOffset, "static if has non-static test");
                        }
                        ++iter;
                        break;
                    case Statement::Kind::kSwitch:
                        if (s.as<SwitchStatement>().fIsStatic &&
                            !(fFlags & kPermitInvalidStaticTests_Flag)) {
                            this->error(s.fOffset, "static switch has non-static test");
                        }
                        ++iter;
                        break;
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
    if (f.declaration().returnType() != *fContext->fVoid_Type) {
        if (cfg.fBlocks[cfg.fExit].fIsReachable) {
            this->error(f.fOffset, String("function '" + String(f.declaration().name()) +
                                          "' can exit without returning a value"));
        }
    }

    return madeChanges;
}

std::unique_ptr<Program> Compiler::convertProgram(
        Program::Kind kind,
        String text,
        const Program::Settings& settings,
        const std::vector<std::unique_ptr<ExternalValue>>* externalValues) {
    SkASSERT(!externalValues || (kind == Program::kGeneric_Kind));

    fErrorText = "";
    fErrorCount = 0;
    fInliner.reset(fContext.get(), fIRGenerator->fModifiers.get(), &settings);

    // Not using AutoSource, because caller is likely to call errorText() if we fail to compile
    std::unique_ptr<String> textPtr(new String(std::move(text)));
    fSource = textPtr.get();

    const ParsedModule& baseModule = this->moduleForProgramKind(kind);

    IRGenerator::IRBundle ir =
            fIRGenerator->convertProgram(kind, &settings, baseModule, /*isBuiltinCode=*/false,
                                         textPtr->c_str(), textPtr->size(), externalValues);
    auto result = std::make_unique<Program>(kind,
                                            std::move(textPtr),
                                            settings,
                                            fContext,
                                            std::move(ir.fElements),
                                            std::move(ir.fModifiers),
                                            std::move(ir.fSymbolTable),
                                            ir.fInputs);
    if (fErrorCount) {
        return nullptr;
    }
    if (settings.fOptimize && !this->optimize(*result)) {
        return nullptr;
    }
    return result;
}

bool Compiler::optimize(Program& program) {
    SkASSERT(!fErrorCount);
    fIRGenerator->fKind = program.fKind;
    fIRGenerator->fSettings = &program.fSettings;

    while (fErrorCount == 0) {
        bool madeChanges = false;

        // Scan and optimize based on the control-flow graph for each function.
        for (const auto& element : program.elements()) {
            if (element->is<FunctionDefinition>()) {
                madeChanges |= this->scanCFG(element->as<FunctionDefinition>());
            }
        }

        // Perform inline-candidate analysis and inline any functions deemed suitable.
        madeChanges |= fInliner.analyze(program);

        // Remove dead functions. We wait until after analysis so that we still report errors,
        // even in unused code.
        if (program.fSettings.fRemoveDeadFunctions) {
            program.fElements.erase(
                    std::remove_if(program.fElements.begin(),
                                   program.fElements.end(),
                                   [&](const std::unique_ptr<ProgramElement>& element) {
                                       if (!element->is<FunctionDefinition>()) {
                                           return false;
                                       }
                                       const auto& fn = element->as<FunctionDefinition>();
                                       bool dead = fn.declaration().callCount() == 0 &&
                                                   fn.declaration().name() != "main";
                                       madeChanges |= dead;
                                       return dead;
                                   }),
                    program.fElements.end());
        }

        if (program.fKind != Program::kFragmentProcessor_Kind) {
            // Remove declarations of dead global variables
            program.fElements.erase(
                    std::remove_if(program.fElements.begin(), program.fElements.end(),
                                   [&](const std::unique_ptr<ProgramElement>& element) {
                                       if (!element->is<GlobalVarDeclaration>()) {
                                           return false;
                                       }
                                       const auto& global = element->as<GlobalVarDeclaration>();
                                       const auto& varDecl =
                                                         global.declaration()->as<VarDeclaration>();
                                       bool dead = varDecl.var().dead();
                                       madeChanges |= dead;
                                       return dead;
                                   }),
                    program.fElements.end());
        }

        if (!madeChanges) {
            break;
        }
    }
    return fErrorCount == 0;
}

#if defined(SKSL_STANDALONE) || SK_SUPPORT_GPU

bool Compiler::toSPIRV(Program& program, OutputStream& out) {
#ifdef SK_ENABLE_SPIRV_VALIDATION
    StringStream buffer;
    AutoSource as(this, program.fSource.get());
    SPIRVCodeGenerator cg(fContext.get(), &program, this, &buffer);
    bool result = cg.generateCode();
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
    AutoSource as(this, program.fSource.get());
    SPIRVCodeGenerator cg(fContext.get(), &program, this, &out);
    bool result = cg.generateCode();
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
    AutoSource as(this, program.fSource.get());
    GLSLCodeGenerator cg(fContext.get(), &program, this, &out);
    bool result = cg.generateCode();
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
    MetalCodeGenerator cg(fContext.get(), &program, this, &out);
    bool result = cg.generateCode();
    return result;
}

bool Compiler::toMetal(Program& program, String* out) {
    StringStream buffer;
    bool result = this->toMetal(program, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

#if defined(SKSL_STANDALONE) || GR_TEST_UTILS
bool Compiler::toCPP(Program& program, String name, OutputStream& out) {
    AutoSource as(this, program.fSource.get());
    CPPCodeGenerator cg(fContext.get(), &program, this, name, &out);
    bool result = cg.generateCode();
    return result;
}

bool Compiler::toH(Program& program, String name, OutputStream& out) {
    AutoSource as(this, program.fSource.get());
    HCodeGenerator cg(fContext.get(), &program, this, name, &out);
    bool result = cg.generateCode();
    return result;
}
#endif // defined(SKSL_STANDALONE) || GR_TEST_UTILS

#endif // defined(SKSL_STANDALONE) || SK_SUPPORT_GPU

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
bool Compiler::toPipelineStage(Program& program, PipelineStageArgs* outArgs) {
    AutoSource as(this, program.fSource.get());
    StringStream buffer;
    PipelineStageCodeGenerator cg(fContext.get(), &program, this, &buffer, outArgs);
    bool result = cg.generateCode();
    if (result) {
        outArgs->fCode = buffer.str();
    }
    return result;
}
#endif

std::unique_ptr<ByteCode> Compiler::toByteCode(Program& program) {
#if defined(SK_ENABLE_SKSL_INTERPRETER)
    AutoSource as(this, program.fSource.get());
    std::unique_ptr<ByteCode> result(new ByteCode());
    ByteCodeGenerator cg(fContext.get(), &program, this, result.get());
    bool success = cg.generateCode();
    if (success) {
        return result;
    }
#else
    ABORT("ByteCode interpreter not enabled");
#endif
    return nullptr;
}

const char* Compiler::OperatorName(Token::Kind op) {
    switch (op) {
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
            ABORT("unsupported operator: %d\n", (int) op);
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

Token::Kind Compiler::RemoveAssignment(Token::Kind op) {
    switch (op) {
        case Token::Kind::TK_PLUSEQ:       return Token::Kind::TK_PLUS;
        case Token::Kind::TK_MINUSEQ:      return Token::Kind::TK_MINUS;
        case Token::Kind::TK_STAREQ:       return Token::Kind::TK_STAR;
        case Token::Kind::TK_SLASHEQ:      return Token::Kind::TK_SLASH;
        case Token::Kind::TK_PERCENTEQ:    return Token::Kind::TK_PERCENT;
        case Token::Kind::TK_SHLEQ:        return Token::Kind::TK_SHL;
        case Token::Kind::TK_SHREQ:        return Token::Kind::TK_SHR;
        case Token::Kind::TK_BITWISEOREQ:  return Token::Kind::TK_BITWISEOR;
        case Token::Kind::TK_BITWISEXOREQ: return Token::Kind::TK_BITWISEXOR;
        case Token::Kind::TK_BITWISEANDEQ: return Token::Kind::TK_BITWISEAND;
        case Token::Kind::TK_LOGICALOREQ:  return Token::Kind::TK_LOGICALOR;
        case Token::Kind::TK_LOGICALXOREQ: return Token::Kind::TK_LOGICALXOR;
        case Token::Kind::TK_LOGICALANDEQ: return Token::Kind::TK_LOGICALAND;
        default: return op;
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
