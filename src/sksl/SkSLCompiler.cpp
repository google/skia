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
#include "src/sksl/generated/sksl_public.dehydrated.sksl"
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

Compiler::Compiler(const ShaderCapsClass* caps, Flags flags)
        : fContext(std::make_shared<Context>())
        , fCaps(caps)
        , fInliner(fContext.get())
        , fFlags(flags)
        , fErrorCount(0) {
    SkASSERT(fCaps);
    fRootSymbolTable = std::make_shared<SymbolTable>(this, /*builtin=*/true);
    fPrivateSymbolTable = std::make_shared<SymbolTable>(fRootSymbolTable, /*builtin=*/true);
    fIRGenerator = std::make_unique<IRGenerator>(fContext.get(), fCaps, *this);

#define TYPE(t) fContext->f##t##_Type.get()

    const SkSL::Symbol* rootTypes[] = {
        TYPE(Void),

        TYPE( Float), TYPE( Float2), TYPE( Float3), TYPE( Float4),
        TYPE(  Half), TYPE(  Half2), TYPE(  Half3), TYPE(  Half4),
        TYPE(   Int), TYPE(   Int2), TYPE(   Int3), TYPE(   Int4),
        TYPE(  UInt), TYPE(  UInt2), TYPE(  UInt3), TYPE(  UInt4),
        TYPE( Short), TYPE( Short2), TYPE( Short3), TYPE( Short4),
        TYPE(UShort), TYPE(UShort2), TYPE(UShort3), TYPE(UShort4),
        TYPE(  Byte), TYPE(  Byte2), TYPE(  Byte3), TYPE(  Byte4),
        TYPE( UByte), TYPE( UByte2), TYPE( UByte3), TYPE( UByte4),
        TYPE(  Bool), TYPE(  Bool2), TYPE(  Bool3), TYPE(  Bool4),

        TYPE(Float2x2), TYPE(Float2x3), TYPE(Float2x4),
        TYPE(Float3x2), TYPE(Float3x3), TYPE(Float3x4),
        TYPE(Float4x2), TYPE(Float4x3), TYPE(Float4x4),

        TYPE(Half2x2),  TYPE(Half2x3),  TYPE(Half2x4),
        TYPE(Half3x2),  TYPE(Half3x3),  TYPE(Half3x4),
        TYPE(Half4x2),  TYPE(Half4x3),  TYPE(Half4x4),

        TYPE(GenType), TYPE(GenHType), TYPE(GenIType), TYPE(GenUType), TYPE(GenBType),
        TYPE(Mat), TYPE(MatH), TYPE(Vec),
        TYPE(GVec), TYPE(GVec2), TYPE(GVec3), TYPE(GVec4),
        TYPE(HVec), TYPE(IVec), TYPE(UVec), TYPE(SVec), TYPE(USVec),
        TYPE(ByteVec), TYPE(UByteVec), TYPE(BVec),

        TYPE(FragmentProcessor),
    };

    const SkSL::Symbol* privateTypes[] = {
        TYPE(Sampler1D), TYPE(Sampler2D), TYPE(Sampler3D),
        TYPE(SamplerExternalOES),
        TYPE(SamplerCube),
        TYPE(Sampler2DRect),
        TYPE(Sampler1DArray), TYPE(Sampler2DArray), TYPE(SamplerCubeArray),
        TYPE(SamplerBuffer),
        TYPE(Sampler2DMS), TYPE(Sampler2DMSArray),

        TYPE(ISampler2D),
        TYPE(Image2D), TYPE(IImage2D),
        TYPE(SubpassInput), TYPE(SubpassInputMS),

        TYPE(GSampler1D), TYPE(GSampler2D), TYPE(GSampler3D),
        TYPE(GSamplerCube),
        TYPE(GSampler2DRect),
        TYPE(GSampler1DArray), TYPE(GSampler2DArray), TYPE(GSamplerCubeArray),
        TYPE(GSamplerBuffer),
        TYPE(GSampler2DMS), TYPE(GSampler2DMSArray),

        TYPE(Sampler1DShadow), TYPE(Sampler2DShadow), TYPE(SamplerCubeShadow),
        TYPE(Sampler2DRectShadow),
        TYPE(Sampler1DArrayShadow), TYPE(Sampler2DArrayShadow), TYPE(SamplerCubeArrayShadow),

        TYPE(GSampler2DArrayShadow), TYPE(GSamplerCubeArrayShadow),
        TYPE(Sampler),
        TYPE(Texture2D),
    };

    for (const SkSL::Symbol* type : rootTypes) {
        fRootSymbolTable->addWithoutOwnership(type);
    }
    for (const SkSL::Symbol* type : privateTypes) {
        fPrivateSymbolTable->addWithoutOwnership(type);
    }

#undef TYPE

    // sk_Caps is "builtin", but all references to it are resolved to Settings, so we don't need to
    // treat it as builtin (ie, no need to clone it into the Program).
    fPrivateSymbolTable->add(
            std::make_unique<Variable>(/*offset=*/-1,
                                       fIRGenerator->fModifiers->addToPool(Modifiers()),
                                       "sk_Caps",
                                       fContext->fSkCaps_Type.get(),
                                       /*builtin=*/false,
                                       Variable::Storage::kGlobal));

    fRootModule = {fRootSymbolTable, /*fIntrinsics=*/nullptr};
    fPrivateModule = {fPrivateSymbolTable, /*fIntrinsics=*/nullptr};
}

Compiler::~Compiler() {}

const ParsedModule& Compiler::loadGPUModule() {
    if (!fGPUModule.fSymbols) {
        fGPUModule = this->parseModule(Program::kFragment_Kind, MODULE_DATA(gpu), fPrivateModule);
    }
    return fGPUModule;
}

const ParsedModule& Compiler::loadFragmentModule() {
    if (!fFragmentModule.fSymbols) {
        fFragmentModule = this->parseModule(Program::kFragment_Kind, MODULE_DATA(frag),
                                            this->loadGPUModule());
    }
    return fFragmentModule;
}

const ParsedModule& Compiler::loadVertexModule() {
    if (!fVertexModule.fSymbols) {
        fVertexModule = this->parseModule(Program::kVertex_Kind, MODULE_DATA(vert),
                                          this->loadGPUModule());
    }
    return fVertexModule;
}

const ParsedModule& Compiler::loadGeometryModule() {
    if (!fGeometryModule.fSymbols) {
        fGeometryModule = this->parseModule(Program::kGeometry_Kind, MODULE_DATA(geom),
                                            this->loadGPUModule());
    }
    return fGeometryModule;
}

const ParsedModule& Compiler::loadFPModule() {
    if (!fFPModule.fSymbols) {
        fFPModule = this->parseModule(Program::kFragmentProcessor_Kind, MODULE_DATA(fp),
                                      this->loadGPUModule());
    }
    return fFPModule;
}

const ParsedModule& Compiler::loadPublicModule() {
    if (!fPublicModule.fSymbols) {
        fPublicModule = this->parseModule(Program::kGeneric_Kind, MODULE_DATA(public), fRootModule);
    }
    return fPublicModule;
}

const ParsedModule& Compiler::loadPipelineModule() {
    if (!fPipelineModule.fSymbols) {
        fPipelineModule = this->parseModule(Program::kPipelineStage_Kind, MODULE_DATA(pipeline),
                                            this->loadPublicModule());

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
        fInterpreterModule = this->parseModule(Program::kGeneric_Kind, MODULE_DATA(interp),
                                               this->loadPublicModule());
    }
    return fInterpreterModule;
}

const ParsedModule& Compiler::moduleForProgramKind(Program::Kind kind) {
    switch (kind) {
        case Program::kVertex_Kind:            return this->loadVertexModule();      break;
        case Program::kFragment_Kind:          return this->loadFragmentModule();    break;
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
        // NOTE: This is a workaround. The only time 'base' is null is when dehydrating includes.
        // In that case, skslc doesn't know which module it's preparing, nor what the correct base
        // module is. We can't use 'Root', because many GPU intrinsics reference private types,
        // like samplers or textures. Today, 'Private' does contain the union of all known types,
        // so this is safe. If we ever have types that only exist in 'Public' (for example), this
        // logic needs to be smarter (by choosing the correct base for the module we're compiling).
        base = fPrivateSymbolTable;
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
    IRGenerator::IRBundle ir =
            fIRGenerator->convertProgram(kind, &settings, baseModule,
                                         /*isBuiltinCode=*/true, source->c_str(), source->length(),
                                         /*externalValues=*/nullptr);
    SkASSERT(ir.fSharedElements.empty());
    LoadedModule module = { kind, std::move(ir.fSymbolTable), std::move(ir.fElements) };
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
    LoadedModule module = { kind, rehydrator.symbolTable(), rehydrator.elements() };
    fModifiers.push_back(fIRGenerator->releaseModifiers());
#endif

    return module;
}

ParsedModule Compiler::parseModule(Program::Kind kind, ModuleData data, const ParsedModule& base) {
    LoadedModule module = this->loadModule(kind, data, base.fSymbols);
    this->optimize(module);

    // For modules that just declare (but don't define) intrinsic functions, there will be no new
    // program elements. In that case, we can share our parent's intrinsic map:
    if (module.fElements.empty()) {
        return {module.fSymbols, base.fIntrinsics};
    }

    auto intrinsics = std::make_shared<IRIntrinsicMap>(base.fIntrinsics.get());

    // Now, transfer all of the program elements to an intrinsic map. This maps certain types of
    // global objects to the declaring ProgramElement.
    for (std::unique_ptr<ProgramElement>& element : module.fElements) {
        switch (element->kind()) {
            case ProgramElement::Kind::kFunction: {
                const FunctionDefinition& f = element->as<FunctionDefinition>();
                SkASSERT(f.declaration().isBuiltin());
                intrinsics->insertOrDie(f.declaration().description(), std::move(element));
                break;
            }
            case ProgramElement::Kind::kFunctionPrototype: {
                // These are already in the symbol table.
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

    return {module.fSymbols, std::move(intrinsics)};
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
                    this->addDefinition(b->left().get(), &b->right(), definitions);
                } else if (Compiler::IsAssignment(b->getOperator())) {
                    this->addDefinition(
                                  b->left().get(),
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
static bool is_dead(const Expression& lvalue, ProgramUsage* usage) {
    switch (lvalue.kind()) {
        case Expression::Kind::kVariableReference:
            return usage->isDead(*lvalue.as<VariableReference>().variable());
        case Expression::Kind::kSwizzle:
            return is_dead(*lvalue.as<Swizzle>().base(), usage);
        case Expression::Kind::kFieldAccess:
            return is_dead(*lvalue.as<FieldAccess>().base(), usage);
        case Expression::Kind::kIndex: {
            const IndexExpression& idx = lvalue.as<IndexExpression>();
            return is_dead(*idx.base(), usage) &&
                   !idx.index()->hasProperty(Expression::Property::kSideEffects);
        }
        case Expression::Kind::kTernary: {
            const TernaryExpression& t = lvalue.as<TernaryExpression>();
            return !t.test()->hasSideEffects() &&
                   is_dead(*t.ifTrue(), usage) &&
                   is_dead(*t.ifFalse(), usage);
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
static bool dead_assignment(const BinaryExpression& b, ProgramUsage* usage) {
    if (!Compiler::IsAssignment(b.getOperator())) {
        return false;
    }
    return is_dead(*b.left(), usage);
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
                switch (constructorType.typeKind()) {
                    case Type::TypeKind::kVector:
                        if (constructor.componentType().isFloat()) {
                            for (int i = 0; i < constructorType.columns(); ++i) {
                                if (constructor.getFVecComponent(i) != value) {
                                    return false;
                                }
                            }
                            return true;
                        } else if (constructor.componentType().isInteger()) {
                            for (int i = 0; i < constructorType.columns(); ++i) {
                                if (constructor.getIVecComponent(i) != value) {
                                    return false;
                                }
                            }
                            return true;
                        }
                        // Other types (e.g. boolean) might occur, but aren't supported here.
                        return false;

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
                        Compiler::OptimizationContext* optimizationContext) {
    optimizationContext->fUpdated = true;
    std::unique_ptr<Expression>* target = (*iter)->expression();
    BinaryExpression& bin = (*target)->as<BinaryExpression>();
    Expression& left = *bin.left();
    std::unique_ptr<Expression>& rightPointer = bin.right();
    SkASSERT(!left.hasSideEffects());
    bool result;
    if (bin.getOperator() == Token::Kind::TK_EQ) {
        result = b->tryRemoveLValueBefore(iter, &left);
    } else {
        result = b->tryRemoveExpressionBefore(iter, &left);
    }
    // Remove references within LHS.
    optimizationContext->fUsage->remove(&left);
    *target = std::move(rightPointer);
    if (!result) {
        optimizationContext->fNeedsRescan = true;
        return;
    }
    if (*iter == b->fNodes.begin()) {
        optimizationContext->fNeedsRescan = true;
        return;
    }
    --(*iter);
    if (!(*iter)->isExpression() || (*iter)->expression() != &rightPointer) {
        optimizationContext->fNeedsRescan = true;
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
                         Compiler::OptimizationContext* optimizationContext) {
    optimizationContext->fUpdated = true;
    std::unique_ptr<Expression>* target = (*iter)->expression();
    BinaryExpression& bin = (*target)->as<BinaryExpression>();
    std::unique_ptr<Expression>& leftPointer = bin.left();
    Expression& right = *bin.right();
    SkASSERT(!right.hasSideEffects());
    // Remove references within RHS.
    optimizationContext->fUsage->remove(&right);
    if (!b->tryRemoveExpressionBefore(iter, &right)) {
        *target = std::move(leftPointer);
        optimizationContext->fNeedsRescan = true;
        return;
    }
    *target = std::move(leftPointer);
    if (*iter == b->fNodes.begin()) {
        optimizationContext->fNeedsRescan = true;
        return;
    }
    --(*iter);
    if ((!(*iter)->isExpression() || (*iter)->expression() != &leftPointer)) {
        optimizationContext->fNeedsRescan = true;
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
                      Compiler::OptimizationContext* optimizationContext) {
    SkASSERT((*(*iter)->expression())->kind() == Expression::Kind::kBinary);
    SkASSERT(type.isVector());
    SkASSERT((*otherExpression)->type().isScalar());
    optimizationContext->fUpdated = true;
    std::unique_ptr<Expression>* target = (*iter)->expression();
    if (!b->tryRemoveExpression(iter)) {
        *target = construct(&type, std::move(*otherExpression));
        optimizationContext->fNeedsRescan = true;
    } else {
        *target = construct(&type, std::move(*otherExpression));
        if (!b->tryInsertExpression(iter, target)) {
            optimizationContext->fNeedsRescan = true;
        }
    }
}

/**
 * Given a binary expression of the form x <op> vec<n>(y), deletes the right side and vectorizes the
 * left to yield vec<n>(x).
 */
static void vectorize_left(BasicBlock* b,
                           std::vector<BasicBlock::Node>::iterator* iter,
                           Compiler::OptimizationContext* optimizationContext) {
    BinaryExpression& bin = (*(*iter)->expression())->as<BinaryExpression>();
    // Remove references within RHS. Vectorization of LHS doesn't change reference counts.
    optimizationContext->fUsage->remove(bin.right().get());
    vectorize(b, iter, bin.right()->type(), &bin.left(), optimizationContext);
}

/**
 * Given a binary expression of the form vec<n>(x) <op> y, deletes the left side and vectorizes the
 * right to yield vec<n>(y).
 */
static void vectorize_right(BasicBlock* b,
                            std::vector<BasicBlock::Node>::iterator* iter,
                            Compiler::OptimizationContext* optimizationContext) {
    BinaryExpression& bin = (*(*iter)->expression())->as<BinaryExpression>();
    // Remove references within LHS. Vectorization of RHS doesn't change reference counts.
    optimizationContext->fUsage->remove(bin.left().get());
    vectorize(b, iter, bin.left()->type(), &bin.right(), optimizationContext);
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
                                  OptimizationContext* optimizationContext) {
    Expression* expr = (*iter)->expression()->get();
    SkASSERT(expr);

    if ((*iter)->fConstantPropagation) {
        std::unique_ptr<Expression> optimized = expr->constantPropagate(*fIRGenerator,
                                                                        definitions);
        if (optimized) {
            optimizationContext->fUpdated = true;
            optimized = fIRGenerator->coerce(std::move(optimized), expr->type());
            SkASSERT(optimized);
            // Remove references within 'expr', add references within 'optimized'
            optimizationContext->fUsage->replace(expr, optimized.get());
            if (!try_replace_expression(&b, iter, &optimized)) {
                optimizationContext->fNeedsRescan = true;
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
                optimizationContext->fSilences.find(var) == optimizationContext->fSilences.end()) {
                optimizationContext->fSilences.insert(var);
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
                    (*iter)->setExpression(std::move(t->ifTrue()), optimizationContext->fUsage);
                } else {
                    (*iter)->setExpression(std::move(t->ifFalse()), optimizationContext->fUsage);
                }
                optimizationContext->fUpdated = true;
                optimizationContext->fNeedsRescan = true;
            }
            break;
        }
        case Expression::Kind::kBinary: {
            BinaryExpression* bin = &expr->as<BinaryExpression>();
            if (dead_assignment(*bin, optimizationContext->fUsage)) {
                delete_left(&b, iter, optimizationContext);
                break;
            }
            Expression& left = *bin->left();
            Expression& right = *bin->right();
            const Type& leftType = left.type();
            const Type& rightType = right.type();
            // collapse useless expressions like x * 1 or x + 0
            if ((!leftType.isScalar() && !leftType.isVector()) ||
                (!rightType.isScalar() && !rightType.isVector())) {
                break;
            }
            switch (bin->getOperator()) {
                case Token::Kind::TK_STAR:
                    if (is_constant(left, 1)) {
                        if (leftType.isVector() && rightType.isScalar()) {
                            // float4(1) * x -> float4(x)
                            vectorize_right(&b, iter, optimizationContext);
                        } else {
                            // 1 * x -> x
                            // 1 * float4(x) -> float4(x)
                            // float4(1) * float4(x) -> float4(x)
                            delete_left(&b, iter, optimizationContext);
                        }
                    }
                    else if (is_constant(left, 0)) {
                        if (leftType.isScalar() && rightType.isVector() &&
                            !right.hasSideEffects()) {
                            // 0 * float4(x) -> float4(0)
                            vectorize_left(&b, iter, optimizationContext);
                        } else {
                            // 0 * x -> 0
                            // float4(0) * x -> float4(0)
                            // float4(0) * float4(x) -> float4(0)
                            if (!right.hasSideEffects()) {
                                delete_right(&b, iter, optimizationContext);
                            }
                        }
                    }
                    else if (is_constant(right, 1)) {
                        if (leftType.isScalar() && rightType.isVector()) {
                            // x * float4(1) -> float4(x)
                            vectorize_left(&b, iter, optimizationContext);
                        } else {
                            // x * 1 -> x
                            // float4(x) * 1 -> float4(x)
                            // float4(x) * float4(1) -> float4(x)
                            delete_right(&b, iter, optimizationContext);
                        }
                    }
                    else if (is_constant(right, 0)) {
                        if (leftType.isVector() && rightType.isScalar() && !left.hasSideEffects()) {
                            // float4(x) * 0 -> float4(0)
                            vectorize_right(&b, iter, optimizationContext);
                        } else {
                            // x * 0 -> 0
                            // x * float4(0) -> float4(0)
                            // float4(x) * float4(0) -> float4(0)
                            if (!left.hasSideEffects()) {
                                delete_left(&b, iter, optimizationContext);
                            }
                        }
                    }
                    break;
                case Token::Kind::TK_PLUS:
                    if (is_constant(left, 0)) {
                        if (leftType.isVector() && rightType.isScalar()) {
                            // float4(0) + x -> float4(x)
                            vectorize_right(&b, iter, optimizationContext);
                        } else {
                            // 0 + x -> x
                            // 0 + float4(x) -> float4(x)
                            // float4(0) + float4(x) -> float4(x)
                            delete_left(&b, iter, optimizationContext);
                        }
                    } else if (is_constant(right, 0)) {
                        if (leftType.isScalar() && rightType.isVector()) {
                            // x + float4(0) -> float4(x)
                            vectorize_left(&b, iter, optimizationContext);
                        } else {
                            // x + 0 -> x
                            // float4(x) + 0 -> float4(x)
                            // float4(x) + float4(0) -> float4(x)
                            delete_right(&b, iter, optimizationContext);
                        }
                    }
                    break;
                case Token::Kind::TK_MINUS:
                    if (is_constant(right, 0)) {
                        if (leftType.isScalar() && rightType.isVector()) {
                            // x - float4(0) -> float4(x)
                            vectorize_left(&b, iter, optimizationContext);
                        } else {
                            // x - 0 -> x
                            // float4(x) - 0 -> float4(x)
                            // float4(x) - float4(0) -> float4(x)
                            delete_right(&b, iter, optimizationContext);
                        }
                    }
                    break;
                case Token::Kind::TK_SLASH:
                    if (is_constant(right, 1)) {
                        if (leftType.isScalar() && rightType.isVector()) {
                            // x / float4(1) -> float4(x)
                            vectorize_left(&b, iter, optimizationContext);
                        } else {
                            // x / 1 -> x
                            // float4(x) / 1 -> float4(x)
                            // float4(x) / float4(1) -> float4(x)
                            delete_right(&b, iter, optimizationContext);
                        }
                    } else if (is_constant(left, 0)) {
                        if (leftType.isScalar() && rightType.isVector() &&
                            !right.hasSideEffects()) {
                            // 0 / float4(x) -> float4(0)
                            vectorize_left(&b, iter, optimizationContext);
                        } else {
                            // 0 / x -> 0
                            // float4(0) / x -> float4(0)
                            // float4(0) / float4(x) -> float4(0)
                            if (!right.hasSideEffects()) {
                                delete_right(&b, iter, optimizationContext);
                            }
                        }
                    }
                    break;
                case Token::Kind::TK_PLUSEQ:
                    if (is_constant(right, 0)) {
                        clear_write(left);
                        delete_right(&b, iter, optimizationContext);
                    }
                    break;
                case Token::Kind::TK_MINUSEQ:
                    if (is_constant(right, 0)) {
                        clear_write(left);
                        delete_right(&b, iter, optimizationContext);
                    }
                    break;
                case Token::Kind::TK_STAREQ:
                    if (is_constant(right, 1)) {
                        clear_write(left);
                        delete_right(&b, iter, optimizationContext);
                    }
                    break;
                case Token::Kind::TK_SLASHEQ:
                    if (is_constant(right, 1)) {
                        clear_write(left);
                        delete_right(&b, iter, optimizationContext);
                    }
                    break;
                default:
                    break;
            }
            break;
        }
        case Expression::Kind::kConstructor: {
            // Find constructors embedded inside constructors and flatten them out where possible.
            //   -  float4(float2(1, 2), 3, 4)                -->  float4(1, 2, 3, 4)
            //   -  float4(w, float3(sin(x), cos(y), tan(z))) -->  float4(w, sin(x), cos(y), tan(z))
            // Leave single-argument constructors alone, though. These might be casts or splats.
            Constructor& c = expr->as<Constructor>();
            if (c.type().columns() > 1) {
                // Inspect each constructor argument to see if it's a candidate for flattening.
                // Remember matched arguments in a bitfield, "argsToOptimize".
                int argsToOptimize = 0;
                int currBit = 1;
                for (const std::unique_ptr<Expression>& arg : c.arguments()) {
                    if (arg->is<Constructor>()) {
                        Constructor& inner = arg->as<Constructor>();
                        if (inner.arguments().size() > 1 &&
                            inner.type().componentType() == c.type().componentType()) {
                            argsToOptimize |= currBit;
                        }
                    }
                    currBit <<= 1;
                }
                if (argsToOptimize) {
                    // We found at least one argument that could be flattened out. Re-walk the
                    // constructor args and flatten the candidates we found during our initial pass.
                    ExpressionArray flattened;
                    flattened.reserve_back(c.type().columns());
                    currBit = 1;
                    for (const std::unique_ptr<Expression>& arg : c.arguments()) {
                        if (argsToOptimize & currBit) {
                            Constructor& inner = arg->as<Constructor>();
                            for (const std::unique_ptr<Expression>& innerArg : inner.arguments()) {
                                flattened.push_back(innerArg->clone());
                            }
                        } else {
                            flattened.push_back(arg->clone());
                        }
                        currBit <<= 1;
                    }
                    auto optimized = std::unique_ptr<Expression>(
                            new Constructor(c.fOffset, &c.type(), std::move(flattened)));
                    // No fUsage change; no references have been added or removed anywhere.
                    optimizationContext->fUpdated = true;
                    if (!try_replace_expression(&b, iter, &optimized)) {
                        optimizationContext->fNeedsRescan = true;
                        return;
                    }
                    SkASSERT((*iter)->isExpression());
                    break;
                }
            }
            break;
        }
        case Expression::Kind::kSwizzle: {
            Swizzle& s = expr->as<Swizzle>();
            // Detect identity swizzles like `foo.rgba`.
            if ((int) s.components().size() == s.base()->type().columns()) {
                bool identity = true;
                for (int i = 0; i < (int) s.components().size(); ++i) {
                    if (s.components()[i] != i) {
                        identity = false;
                        break;
                    }
                }
                if (identity) {
                    optimizationContext->fUpdated = true;
                    // No fUsage change: foo.rgba and foo have equivalent reference counts
                    if (!try_replace_expression(&b, iter, &s.base())) {
                        optimizationContext->fNeedsRescan = true;
                        return;
                    }
                    SkASSERT((*iter)->isExpression());
                    break;
                }
            }
            // Detect swizzles of swizzles, e.g. replace `foo.argb.r000` with `foo.a000`.
            if (s.base()->is<Swizzle>()) {
                Swizzle& base = s.base()->as<Swizzle>();
                ComponentArray final;
                for (int c : s.components()) {
                    final.push_back(base.components()[c]);
                }
                optimizationContext->fUpdated = true;
                std::unique_ptr<Expression> replacement(new Swizzle(*fContext, base.base()->clone(),
                                                                    final));
                // No fUsage change: `foo.gbr.gbr` and `foo.brg` have equivalent reference counts
                if (!try_replace_expression(&b, iter, &replacement)) {
                    optimizationContext->fNeedsRescan = true;
                    return;
                }
                SkASSERT((*iter)->isExpression());
                break;
            }
            // Optimize swizzles of constructors.
            if (s.base()->is<Constructor>()) {
                Constructor& base = s.base()->as<Constructor>();
                std::unique_ptr<Expression> replacement;
                const Type& componentType = base.type().componentType();
                int swizzleSize = s.components().size();

                // The IR generator has already converted any zero/one swizzle components into
                // constructors containing zero/one args. Confirm that this is true by checking that
                // our swizzle components are all `xyzw` (values 0 through 3).
                SkASSERT(std::all_of(s.components().begin(), s.components().end(),
                                     [](int8_t c) { return c >= 0 && c <= 3; }));

                if (base.arguments().size() == 1 && base.arguments().front()->type().isScalar()) {
                    // `half4(scalar).zyy` can be optimized to `half3(scalar)`. The swizzle
                    // components don't actually matter since all fields are the same.
                    ExpressionArray newArgs;
                    newArgs.push_back(base.arguments().front()->clone());
                    replacement = std::make_unique<Constructor>(
                            base.fOffset,
                            &componentType.toCompound(*fContext, swizzleSize, /*rows=*/1),
                            std::move(newArgs));

                    // No fUsage change: `half4(foo).xy` and `half2(foo)` have equivalent reference
                    // counts.
                    optimizationContext->fUpdated = true;
                    if (!try_replace_expression(&b, iter, &replacement)) {
                        optimizationContext->fNeedsRescan = true;
                        return;
                    }
                    SkASSERT((*iter)->isExpression());
                    break;
                }

                // Swizzles can duplicate some elements and discard others, e.g.
                // `half4(1, 2, 3, 4).xxz` --> `half3(1, 1, 3)`. However, there are constraints:
                // - Expressions with side effects need to occur exactly once, even if they
                //   would otherwise be swizzle-eliminated
                // - Non-trivial expressions should not be repeated, but elimination is OK.
                //
                // Look up the argument for the constructor at each index. This is typically simple
                // but for weird cases like `half4(bar.yz, half2(foo))`, it can be harder than it
                // seems. This example would result in:
                //     argMap[0] = {.fArgIndex = 0, .fComponent = 0}   (bar.yz     .x)
                //     argMap[1] = {.fArgIndex = 0, .fComponent = 1}   (bar.yz     .y)
                //     argMap[2] = {.fArgIndex = 1, .fComponent = 0}   (half2(foo) .x)
                //     argMap[3] = {.fArgIndex = 1, .fComponent = 1}   (half2(foo) .y)
                struct ConstructorArgMap {
                    int8_t fArgIndex;
                    int8_t fComponent;
                };

                int numConstructorArgs = base.type().columns();
                ConstructorArgMap argMap[4] = {};
                int writeIdx = 0;
                for (int argIdx = 0; argIdx < (int) base.arguments().size(); ++argIdx) {
                    const Expression& expr = *base.arguments()[argIdx];
                    int argWidth = expr.type().columns();
                    for (int componentIdx = 0; componentIdx < argWidth; ++componentIdx) {
                        argMap[writeIdx].fArgIndex = argIdx;
                        argMap[writeIdx].fComponent = componentIdx;
                        ++writeIdx;
                    }
                }
                SkASSERT(writeIdx == numConstructorArgs);

                // Count up the number of times each constructor argument is used by the
                // swizzle.
                //    `half4(bar.yz, half2(foo)).xwxy` -> { 3, 1 }
                // - bar.yz    is referenced 3 times, by `.x_xy`
                // - half(foo) is referenced 1 time,  by `._w__`
                int8_t exprUsed[4] = {};
                for (int c : s.components()) {
                    exprUsed[argMap[c].fArgIndex]++;
                }

                bool safeToOptimize = true;
                for (int index = 0; index < numConstructorArgs; ++index) {
                    int8_t constructorArgIndex = argMap[index].fArgIndex;
                    const Expression& baseArg = *base.arguments()[constructorArgIndex];

                    // Check that non-trivial expressions are not swizzled in more than once.
                    if (exprUsed[constructorArgIndex] > 1 &&
                            !Analysis::IsTrivialExpression(baseArg)) {
                        safeToOptimize = false;
                        break;
                    }
                    // Check that side-effect-bearing expressions are swizzled in exactly once.
                    if (exprUsed[constructorArgIndex] != 1 && baseArg.hasSideEffects()) {
                        safeToOptimize = false;
                        break;
                    }
                }

                if (safeToOptimize) {
                    struct ReorderedArgument {
                        int8_t fArgIndex;
                        ComponentArray fComponents;
                    };
                    SkSTArray<4, ReorderedArgument> reorderedArgs;
                    for (int c : s.components()) {
                        const ConstructorArgMap& argument = argMap[c];
                        const Expression& baseArg = *base.arguments()[argument.fArgIndex];

                        if (baseArg.type().isScalar()) {
                            // This argument is a scalar; add it to the list as-is.
                            SkASSERT(argument.fComponent == 0);
                            reorderedArgs.push_back({argument.fArgIndex,
                                                     ComponentArray{}});
                        } else {
                            // This argument is a component from a vector.
                            SkASSERT(argument.fComponent < baseArg.type().columns());
                            if (reorderedArgs.empty() ||
                                reorderedArgs.back().fArgIndex != argument.fArgIndex) {
                                // This can't be combined with the previous argument. Add a new one.
                                reorderedArgs.push_back({argument.fArgIndex,
                                                         ComponentArray{argument.fComponent}});
                            } else {
                                // Since we know this argument uses components, it should already
                                // have at least one component set.
                                SkASSERT(!reorderedArgs.back().fComponents.empty());
                                // Build up the current argument with one more component.
                                reorderedArgs.back().fComponents.push_back(argument.fComponent);
                            }
                        }
                    }

                    // Convert our reordered argument list to an actual array of expressions, with
                    // the new order and any new inner swizzles that need to be applied. Note that
                    // we expect followup passes to clean up the inner swizzles.
                    ExpressionArray newArgs;
                    newArgs.reserve_back(swizzleSize);
                    for (const ReorderedArgument& reorderedArg : reorderedArgs) {
                        const Expression& baseArg = *base.arguments()[reorderedArg.fArgIndex];
                        if (reorderedArg.fComponents.empty()) {
                            newArgs.push_back(baseArg.clone());
                        } else {
                            newArgs.push_back(std::make_unique<Swizzle>(*fContext, baseArg.clone(),
                                                                        reorderedArg.fComponents));
                        }
                    }

                    // Create a new constructor.
                    replacement = std::make_unique<Constructor>(
                            base.fOffset,
                            &componentType.toCompound(*fContext, swizzleSize, /*rows=*/1),
                            std::move(newArgs));

                    // Remove references within 'expr', add references within 'optimized'
                    optimizationContext->fUpdated = true;
                    optimizationContext->fUsage->replace(expr, replacement.get());
                    if (!try_replace_expression(&b, iter, &replacement)) {
                        optimizationContext->fNeedsRescan = true;
                        return;
                    }
                    SkASSERT((*iter)->isExpression());
                }
                break;
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
    auto iter = switchStatement->cases().begin();
    for (; iter != switchStatement->cases().end(); ++iter) {
        if (iter->get() == caseToCapture) {
            break;
        }
    }

    // Next, walk forward through the rest of the switch. If we find a conditional break, we're
    // stuck and can't simplify at all. If we find an unconditional break, we have a range of
    // statements that we can use for simplification.
    auto startIter = iter;
    Statement* unconditionalBreakStmt = nullptr;
    for (; iter != switchStatement->cases().end(); ++iter) {
        for (std::unique_ptr<Statement>& stmt : (*iter)->statements()) {
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
        for (std::unique_ptr<Statement>& stmt : (*startIter)->statements()) {
            caseStmts.push_back(std::move(stmt));
        }
        ++startIter;
    }

    // If we found an unconditional break at the end, we need to move what we can while avoiding
    // that break.
    if (unconditionalBreakStmt != nullptr) {
        for (std::unique_ptr<Statement>& stmt : (*startIter)->statements()) {
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
    return std::make_unique<Block>(/*offset=*/-1, std::move(caseStmts), switchStatement->symbols());
}

void Compiler::simplifyStatement(DefinitionMap& definitions,
                                 BasicBlock& b,
                                 std::vector<BasicBlock::Node>::iterator* iter,
                                 OptimizationContext* optimizationContext) {
    ProgramUsage* usage = optimizationContext->fUsage;
    Statement* stmt = (*iter)->statement()->get();
    switch (stmt->kind()) {
        case Statement::Kind::kVarDeclaration: {
            const auto& varDecl = stmt->as<VarDeclaration>();
            if (usage->isDead(varDecl.var()) &&
                (!varDecl.value() ||
                 !varDecl.value()->hasSideEffects())) {
                if (varDecl.value()) {
                    SkASSERT((*iter)->statement()->get() == stmt);
                    if (!b.tryRemoveExpressionBefore(iter, varDecl.value().get())) {
                        optimizationContext->fNeedsRescan = true;
                    }
                }
                (*iter)->setStatement(std::make_unique<Nop>(), usage);
                optimizationContext->fUpdated = true;
            }
            break;
        }
        case Statement::Kind::kIf: {
            IfStatement& i = stmt->as<IfStatement>();
            if (i.test()->kind() == Expression::Kind::kBoolLiteral) {
                // constant if, collapse down to a single branch
                if (i.test()->as<BoolLiteral>().value()) {
                    SkASSERT(i.ifTrue());
                    (*iter)->setStatement(std::move(i.ifTrue()), usage);
                } else {
                    if (i.ifFalse()) {
                        (*iter)->setStatement(std::move(i.ifFalse()), usage);
                    } else {
                        (*iter)->setStatement(std::make_unique<Nop>(), usage);
                    }
                }
                optimizationContext->fUpdated = true;
                optimizationContext->fNeedsRescan = true;
                break;
            }
            if (i.ifFalse() && i.ifFalse()->isEmpty()) {
                // else block doesn't do anything, remove it
                i.ifFalse().reset();
                optimizationContext->fUpdated = true;
                optimizationContext->fNeedsRescan = true;
            }
            if (!i.ifFalse() && i.ifTrue()->isEmpty()) {
                // if block doesn't do anything, no else block
                if (i.test()->hasSideEffects()) {
                    // test has side effects, keep it
                    (*iter)->setStatement(
                            std::make_unique<ExpressionStatement>(std::move(i.test())), usage);
                } else {
                    // no if, no else, no test side effects, kill the whole if
                    // statement
                    (*iter)->setStatement(std::make_unique<Nop>(), usage);
                }
                optimizationContext->fUpdated = true;
                optimizationContext->fNeedsRescan = true;
            }
            break;
        }
        case Statement::Kind::kSwitch: {
            SwitchStatement& s = stmt->as<SwitchStatement>();
            int64_t switchValue;
            if (fIRGenerator->getConstantInt(*s.value(), &switchValue)) {
                // switch is constant, replace it with the case that matches
                bool found = false;
                SwitchCase* defaultCase = nullptr;
                for (const std::unique_ptr<SwitchCase>& c : s.cases()) {
                    if (!c->value()) {
                        defaultCase = c.get();
                        continue;
                    }
                    int64_t caseValue;
                    SkAssertResult(fIRGenerator->getConstantInt(*c->value(), &caseValue));
                    if (caseValue == switchValue) {
                        std::unique_ptr<Statement> newBlock = block_for_case(&s, c.get());
                        if (newBlock) {
                            (*iter)->setStatement(std::move(newBlock), usage);
                            found = true;
                            break;
                        } else {
                            if (s.isStatic() && !(fFlags & kPermitInvalidStaticTests_Flag) &&
                                optimizationContext->fSilences.find(&s) ==
                                optimizationContext->fSilences.end()) {
                                this->error(s.fOffset,
                                            "static switch contains non-static conditional break");
                                optimizationContext->fSilences.insert(&s);
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
                            (*iter)->setStatement(std::move(newBlock), usage);
                        } else {
                            if (s.isStatic() && !(fFlags & kPermitInvalidStaticTests_Flag) &&
                                optimizationContext->fSilences.find(&s) ==
                                optimizationContext->fSilences.end()) {
                                this->error(s.fOffset,
                                            "static switch contains non-static conditional break");
                                optimizationContext->fSilences.insert(&s);
                            }
                            return; // can't simplify
                        }
                    } else {
                        (*iter)->setStatement(std::make_unique<Nop>(), usage);
                    }
                }
                optimizationContext->fUpdated = true;
                optimizationContext->fNeedsRescan = true;
            }
            break;
        }
        case Statement::Kind::kExpression: {
            ExpressionStatement& e = stmt->as<ExpressionStatement>();
            SkASSERT((*iter)->statement()->get() == &e);
            if (!e.expression()->hasSideEffects()) {
                // Expression statement with no side effects, kill it
                if (!b.tryRemoveExpressionBefore(iter, e.expression().get())) {
                    optimizationContext->fNeedsRescan = true;
                }
                SkASSERT((*iter)->statement()->get() == stmt);
                (*iter)->setStatement(std::make_unique<Nop>(), usage);
                optimizationContext->fUpdated = true;
            }
            break;
        }
        default:
            break;
    }
}

bool Compiler::scanCFG(FunctionDefinition& f, ProgramUsage* usage) {
    bool madeChanges = false;

    CFG cfg = CFGGenerator().getCFG(f);
    this->computeDataFlow(&cfg);

    // check for unreachable code
    for (size_t i = 0; i < cfg.fBlocks.size(); i++) {
        const BasicBlock& block = cfg.fBlocks[i];
        if (!block.fIsReachable && !block.fAllowUnreachable && block.fNodes.size()) {
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
    OptimizationContext optimizationContext;
    optimizationContext.fUsage = usage;
    SkBitSet eliminatedBlockIds(cfg.fBlocks.size());
    do {
        if (optimizationContext.fNeedsRescan) {
            cfg = CFGGenerator().getCFG(f);
            this->computeDataFlow(&cfg);
            optimizationContext.fNeedsRescan = false;
        }

        eliminatedBlockIds.reset();
        optimizationContext.fUpdated = false;

        for (BlockId blockId = 0; blockId < cfg.fBlocks.size(); ++blockId) {
            if (eliminatedBlockIds.test(blockId)) {
                // We reached a block ID that might have been eliminated. Be cautious and rescan.
                optimizationContext.fUpdated = true;
                optimizationContext.fNeedsRescan = true;
                break;
            }

            BasicBlock& b = cfg.fBlocks[blockId];
            if (blockId > 0 && !b.fIsReachable) {
                // Block was reachable before optimization, but has since become unreachable. In
                // addition to being dead code, it's broken - since control flow can't reach it, no
                // prior variable definitions can reach it, and therefore variables might look to
                // have not been properly assigned. Kill it by replacing all statements with Nops.
                for (BasicBlock::Node& node : b.fNodes) {
                    if (node.isStatement() && !(*node.statement())->is<Nop>()) {
                        // Eliminating a node runs the risk of eliminating that node's exits as
                        // well. Keep track of this and do a rescan if we are about to access one
                        // of these.
                        for (BlockId id : b.fExits) {
                            eliminatedBlockIds.set(id);
                        }
                        node.setStatement(std::make_unique<Nop>(), usage);
                        madeChanges = true;
                    }
                }
                continue;
            }
            DefinitionMap definitions = b.fBefore;

            for (auto iter = b.fNodes.begin(); iter != b.fNodes.end() &&
                !optimizationContext.fNeedsRescan; ++iter) {
                if (iter->isExpression()) {
                    this->simplifyExpression(definitions, b, &iter, &optimizationContext);
                } else {
                    this->simplifyStatement(definitions, b, &iter, &optimizationContext);
                }
                if (optimizationContext.fNeedsRescan) {
                    break;
                }
                this->addDefinitions(*iter, &definitions);
            }

            if (optimizationContext.fNeedsRescan) {
                break;
            }
        }
        madeChanges |= optimizationContext.fUpdated;
    } while (optimizationContext.fUpdated);
    SkASSERT(!optimizationContext.fNeedsRescan);

    // verify static ifs & switches, clean up dead variable decls
    for (BasicBlock& b : cfg.fBlocks) {
        for (auto iter = b.fNodes.begin(); iter != b.fNodes.end() &&
             !optimizationContext.fNeedsRescan;) {
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
                        if (s.as<SwitchStatement>().isStatic() &&
                            !(fFlags & kPermitInvalidStaticTests_Flag) &&
                            optimizationContext.fSilences.find(&s) ==
                            optimizationContext.fSilences.end()) {
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

    // Loading and optimizing our base module might reset the inliner, so do that first,
    // *then* configure the inliner with the settings for this program.
    const ParsedModule& baseModule = this->moduleForProgramKind(kind);

    fErrorText = "";
    fErrorCount = 0;
    fInliner.reset(fIRGenerator->fModifiers.get(), &settings);

    // Not using AutoSource, because caller is likely to call errorText() if we fail to compile
    std::unique_ptr<String> textPtr(new String(std::move(text)));
    fSource = textPtr.get();

    // Enable node pooling while converting and optimizing the program for a performance boost.
    // The Program will take ownership of the pool.
    std::unique_ptr<Pool> pool = Pool::Create();
    pool->attachToThread();
    IRGenerator::IRBundle ir =
            fIRGenerator->convertProgram(kind, &settings, baseModule, /*isBuiltinCode=*/false,
                                         textPtr->c_str(), textPtr->size(), externalValues);
    auto program = std::make_unique<Program>(kind,
                                             std::move(textPtr),
                                             settings,
                                             fCaps,
                                             fContext,
                                             std::move(ir.fElements),
                                             std::move(ir.fSharedElements),
                                             std::move(ir.fModifiers),
                                             std::move(ir.fSymbolTable),
                                             std::move(pool),
                                             ir.fInputs);
    bool success = false;
    if (fErrorCount) {
        // Do not return programs that failed to compile.
    } else if (settings.fOptimize && !this->optimize(*program)) {
        // Do not return programs that failed to optimize.
    } else {
        // We have a successful program!
        success = true;
    }

    program->fPool->detachFromThread();
    return success ? std::move(program) : nullptr;
}

bool Compiler::optimize(LoadedModule& module) {
    SkASSERT(!fErrorCount);
    Program::Settings settings;
    fIRGenerator->fKind = module.fKind;
    fIRGenerator->fSettings = &settings;
    std::unique_ptr<ProgramUsage> usage = Analysis::GetUsage(module);

    fInliner.reset(fModifiers.back().get(), &settings);

    while (fErrorCount == 0) {
        bool madeChanges = false;

        // Scan and optimize based on the control-flow graph for each function.
        for (const auto& element : module.fElements) {
            if (element->is<FunctionDefinition>()) {
                madeChanges |= this->scanCFG(element->as<FunctionDefinition>(), usage.get());
            }
        }

        // Perform inline-candidate analysis and inline any functions deemed suitable.
        madeChanges |= fInliner.analyze(module.fElements, module.fSymbols, usage.get());

        if (!madeChanges) {
            break;
        }
    }
    return fErrorCount == 0;
}

bool Compiler::optimize(Program& program) {
    SkASSERT(!fErrorCount);
    fIRGenerator->fKind = program.fKind;
    fIRGenerator->fSettings = &program.fSettings;
    ProgramUsage* usage = program.fUsage.get();

    while (fErrorCount == 0) {
        bool madeChanges = false;

        // Scan and optimize based on the control-flow graph for each function.
        for (const auto& element : program.ownedElements()) {
            if (element->is<FunctionDefinition>()) {
                madeChanges |= this->scanCFG(element->as<FunctionDefinition>(), usage);
            }
        }

        // Perform inline-candidate analysis and inline any functions deemed suitable.
        madeChanges |= fInliner.analyze(program.ownedElements(), program.fSymbols, usage);

        // Remove dead functions. We wait until after analysis so that we still report errors,
        // even in unused code.
        if (program.fSettings.fRemoveDeadFunctions) {
            auto isDeadFunction = [&](const ProgramElement* element) {
                if (!element->is<FunctionDefinition>()) {
                    return false;
                }
                const FunctionDefinition& fn = element->as<FunctionDefinition>();
                if (fn.declaration().name() != "main" && usage->get(fn.declaration()) == 0) {
                    usage->remove(*element);
                    madeChanges = true;
                    return true;
                }
                return false;
            };
            program.fElements.erase(
                    std::remove_if(program.fElements.begin(), program.fElements.end(),
                                   [&](const std::unique_ptr<ProgramElement>& element) {
                                       return isDeadFunction(element.get());
                                   }),
                    program.fElements.end());
            program.fSharedElements.erase(
                    std::remove_if(program.fSharedElements.begin(), program.fSharedElements.end(),
                                   isDeadFunction),
                    program.fSharedElements.end());
        }

        if (program.fKind != Program::kFragmentProcessor_Kind) {
            // Remove declarations of dead global variables
            auto isDeadVariable = [&](const ProgramElement* element) {
                if (!element->is<GlobalVarDeclaration>()) {
                    return false;
                }
                const GlobalVarDeclaration& global = element->as<GlobalVarDeclaration>();
                const VarDeclaration& varDecl = global.declaration()->as<VarDeclaration>();
                if (usage->isDead(varDecl.var())) {
                    madeChanges = true;
                    return true;
                }
                return false;
            };
            program.fElements.erase(
                    std::remove_if(program.fElements.begin(), program.fElements.end(),
                                   [&](const std::unique_ptr<ProgramElement>& element) {
                                       return isDeadVariable(element.get());
                                   }),
                    program.fElements.end());
            program.fSharedElements.erase(
                    std::remove_if(program.fSharedElements.begin(), program.fSharedElements.end(),
                                   isDeadVariable),
                    program.fSharedElements.end());
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
    if (result && program.fSettings.fValidateSPIRV) {
        spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_0);
        const String& data = buffer.str();
        SkASSERT(0 == data.size() % 4);
        String errors;
        auto dumpmsg = [&errors](spv_message_level_t, const char*, const spv_position_t&,
                                 const char* m) {
            errors.appendf("SPIR-V validation error: %s\n", m);
        };
        tools.SetMessageConsumer(dumpmsg);

        // Verify that the SPIR-V we produced is valid. At runtime, we will abort() with a message
        // explaining the error. In standalone mode (skslc), we will send the message, plus the
        // entire disassembled SPIR-V (for easier context & debugging) as *our* error message.
        result = tools.Validate((const uint32_t*) data.c_str(), data.size() / 4);

        if (!result) {
#if defined(SKSL_STANDALONE)
            // Convert the string-stream to a SPIR-V disassembly.
            std::string disassembly;
            if (tools.Disassemble((const uint32_t*)data.data(), data.size() / 4, &disassembly)) {
                errors.append(disassembly);
            }
            this->error(-1, errors);
#else
            SkDEBUGFAILF("%s", errors.c_str());
#endif
        }
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
    AutoSource as(this, program.fSource.get());
    std::unique_ptr<ByteCode> result(new ByteCode());
    ByteCodeGenerator cg(fContext.get(), &program, this, result.get());
    bool success = cg.generateCode();
    if (success) {
        return result;
    }
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
        case Token::Kind::TK_BITWISEANDEQ:
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
        default: return op;
    }
}

Position Compiler::position(int offset) {
    if (fSource && offset >= 0) {
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
    } else {
        return Position(-1, -1);
    }
}

void Compiler::error(int offset, String msg) {
    fErrorCount++;
    Position pos = this->position(offset);
    fErrorText += "error: " + (pos.fLine >= 1 ? to_string(pos.fLine) + ": " : "") + msg + "\n";
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
