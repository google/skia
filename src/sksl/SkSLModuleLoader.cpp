/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkMutex.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramKind.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/SkSLModuleLoader.h"
#include "src/sksl/SkSLParsedModule.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <type_traits>

#if SKSL_STANDALONE

    // In standalone mode, we load the textual sksl source files. GN generates or copies these files
    // to the skslc executable directory. The "data" in this mode is just the filename.
    #define MODULE_DATA(name) #name ".sksl"

#else

    // At runtime, we compile minified SkSL module code.
    #include "src/sksl/generated/sksl_shared.minified.sksl"
    #include "src/sksl/generated/sksl_compute.minified.sksl"
    #include "src/sksl/generated/sksl_frag.minified.sksl"
    #include "src/sksl/generated/sksl_gpu.minified.sksl"
    #include "src/sksl/generated/sksl_public.minified.sksl"
    #include "src/sksl/generated/sksl_rt_shader.minified.sksl"
    #include "src/sksl/generated/sksl_vert.minified.sksl"
    #if defined(SK_GRAPHITE_ENABLED)
    #include "src/sksl/generated/sksl_graphite_frag.minified.sksl"
    #include "src/sksl/generated/sksl_graphite_vert.minified.sksl"
    #endif

    #define MODULE_DATA(name) SKSL_MINIFIED_##name

#endif

namespace SkSL {

#define TYPE(t) &BuiltinTypes::f ## t

static constexpr BuiltinTypePtr kRootTypes[] = {
    TYPE(Void),

    TYPE( Float), TYPE( Float2), TYPE( Float3), TYPE( Float4),
    TYPE(  Half), TYPE(  Half2), TYPE(  Half3), TYPE(  Half4),
    TYPE(   Int), TYPE(   Int2), TYPE(   Int3), TYPE(   Int4),
    TYPE(  UInt), TYPE(  UInt2), TYPE(  UInt3), TYPE(  UInt4),
    TYPE( Short), TYPE( Short2), TYPE( Short3), TYPE( Short4),
    TYPE(UShort), TYPE(UShort2), TYPE(UShort3), TYPE(UShort4),
    TYPE(  Bool), TYPE(  Bool2), TYPE(  Bool3), TYPE(  Bool4),

    TYPE(Float2x2), TYPE(Float2x3), TYPE(Float2x4),
    TYPE(Float3x2), TYPE(Float3x3), TYPE(Float3x4),
    TYPE(Float4x2), TYPE(Float4x3), TYPE(Float4x4),

    TYPE(Half2x2),  TYPE(Half2x3),  TYPE(Half2x4),
    TYPE(Half3x2),  TYPE(Half3x3),  TYPE(Half3x4),
    TYPE(Half4x2),  TYPE(Half4x3),  TYPE(Half4x4),

    TYPE(SquareMat), TYPE(SquareHMat),
    TYPE(Mat),       TYPE(HMat),

    // TODO(skia:12349): generic short/ushort
    TYPE(GenType),   TYPE(GenIType), TYPE(GenUType),
    TYPE(GenHType),   /* (GenSType)      (GenUSType) */
    TYPE(GenBType),
    TYPE(IntLiteral),
    TYPE(FloatLiteral),

    TYPE(Vec),     TYPE(IVec),     TYPE(UVec),
    TYPE(HVec),    TYPE(SVec),     TYPE(USVec),
    TYPE(BVec),

    TYPE(ColorFilter),
    TYPE(Shader),
    TYPE(Blender),
};

static constexpr BuiltinTypePtr kPrivateTypes[] = {
    TYPE(Sampler2D), TYPE(SamplerExternalOES), TYPE(Sampler2DRect),

    TYPE(SubpassInput), TYPE(SubpassInputMS),

    TYPE(Sampler),
    TYPE(Texture2D),
    TYPE(ReadWriteTexture2D), TYPE(ReadOnlyTexture2D), TYPE(WriteOnlyTexture2D),
    TYPE(GenTexture2D), TYPE(ReadableTexture2D), TYPE(WritableTexture2D),
};

#undef TYPE

struct ModuleLoader::Impl {
    Impl();

    void makeRootSymbolTable();

    // This mutex is taken when ModuleLoader::Get is called, and released when the returned
    // ModuleLoader object falls out of scope.
    SkMutex fMutex;
    const BuiltinTypes fBuiltinTypes;
    ModifiersPool fCoreModifiers;

    ParsedModule fRootModule;
    std::shared_ptr<SymbolTable> fRootSymbolTableWithPublicTypes;

    ParsedModule fSharedModule;              // [Root] + Public intrinsics
    ParsedModule fGPUModule;                 // [Shared] + Non-public intrinsics/helper functions
    ParsedModule fVertexModule;              // [GPU] + Vertex stage decls
    ParsedModule fFragmentModule;            // [GPU] + Fragment stage decls
    ParsedModule fComputeModule;             // [GPU] + Compute stage decls
    ParsedModule fGraphiteVertexModule;      // [Vert] + Graphite vertex helpers
    ParsedModule fGraphiteFragmentModule;    // [Frag] + Graphite fragment helpers

    ParsedModule fPublicModule;              // [Shared] + Runtime effect intrinsics - Private types
    ParsedModule fRuntimeShaderModule;       // [Public] + Runtime shader decls
};

ModuleLoader ModuleLoader::Get() {
    static ModuleLoader::Impl* sModuleLoaderImpl = new ModuleLoader::Impl;
    return ModuleLoader(*sModuleLoaderImpl);
}

ModuleLoader::ModuleLoader(ModuleLoader::Impl& m) : fModuleLoader(m) {
    fModuleLoader.fMutex.acquire();
}

ModuleLoader::~ModuleLoader() {
    fModuleLoader.fMutex.release();
}

void ModuleLoader::unloadModules() {
    fModuleLoader.fSharedModule           = ParsedModule{};
    fModuleLoader.fGPUModule              = ParsedModule{};
    fModuleLoader.fVertexModule           = ParsedModule{};
    fModuleLoader.fFragmentModule         = ParsedModule{};
    fModuleLoader.fComputeModule          = ParsedModule{};
    fModuleLoader.fGraphiteVertexModule   = ParsedModule{};
    fModuleLoader.fGraphiteFragmentModule = ParsedModule{};
    fModuleLoader.fPublicModule           = ParsedModule{};
    fModuleLoader.fRuntimeShaderModule    = ParsedModule{};
}

ModuleLoader::Impl::Impl() {
    this->makeRootSymbolTable();
}

static void add_public_type_aliases(SkSL::SymbolTable* symbols, const SkSL::BuiltinTypes& types) {
    // Add some aliases to the runtime effect modules so that it's friendlier, and more like GLSL.
    symbols->addWithoutOwnership(types.fVec2.get());
    symbols->addWithoutOwnership(types.fVec3.get());
    symbols->addWithoutOwnership(types.fVec4.get());

    symbols->addWithoutOwnership(types.fIVec2.get());
    symbols->addWithoutOwnership(types.fIVec3.get());
    symbols->addWithoutOwnership(types.fIVec4.get());

    symbols->addWithoutOwnership(types.fBVec2.get());
    symbols->addWithoutOwnership(types.fBVec3.get());
    symbols->addWithoutOwnership(types.fBVec4.get());

    symbols->addWithoutOwnership(types.fMat2.get());
    symbols->addWithoutOwnership(types.fMat3.get());
    symbols->addWithoutOwnership(types.fMat4.get());

    symbols->addWithoutOwnership(types.fMat2x2.get());
    symbols->addWithoutOwnership(types.fMat2x3.get());
    symbols->addWithoutOwnership(types.fMat2x4.get());
    symbols->addWithoutOwnership(types.fMat3x2.get());
    symbols->addWithoutOwnership(types.fMat3x3.get());
    symbols->addWithoutOwnership(types.fMat3x4.get());
    symbols->addWithoutOwnership(types.fMat4x2.get());
    symbols->addWithoutOwnership(types.fMat4x3.get());
    symbols->addWithoutOwnership(types.fMat4x4.get());

    // Hide all the private symbols by aliasing them all to "invalid". This will prevent code from
    // using built-in names like `sampler2D` as variable names.
    for (BuiltinTypePtr privateType : kPrivateTypes) {
        symbols->add(Type::MakeAliasType((types.*privateType)->name(), *types.fInvalid));
    }
    symbols->add(Type::MakeAliasType("sk_Caps", *types.fInvalid));
}

static void add_compute_type_aliases(SkSL::SymbolTable* symbols, const SkSL::BuiltinTypes& types) {
    // A `texture2D` in a compute shader should generally mean "read-write" texture access, not
    // "sample" texture access. Remap the name `texture2D` to point to `readWriteTexture2D`.
    symbols->add(Type::MakeAliasType("texture2D", *types.fReadWriteTexture2D));
}

const BuiltinTypes& ModuleLoader::builtinTypes() {
    return fModuleLoader.fBuiltinTypes;
}

ModifiersPool& ModuleLoader::coreModifiers() {
    return fModuleLoader.fCoreModifiers;
}

const ParsedModule& ModuleLoader::rootModule() {
    return fModuleLoader.fRootModule;
}

std::shared_ptr<SymbolTable>& ModuleLoader::rootSymbolTableWithPublicTypes() {
    if (!fModuleLoader.fRootSymbolTableWithPublicTypes) {
        fModuleLoader.fRootSymbolTableWithPublicTypes =
                std::make_shared<SymbolTable>(this->rootModule().fSymbols, /*builtin=*/true);
        add_public_type_aliases(fModuleLoader.fRootSymbolTableWithPublicTypes.get(),
                                this->builtinTypes());
    }
    return fModuleLoader.fRootSymbolTableWithPublicTypes;
}

const ParsedModule& ModuleLoader::loadPublicModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fPublicModule.fSymbols) {
        const ParsedModule& sharedModule = this->loadSharedModule(compiler);
        fModuleLoader.fPublicModule = compiler->parseModule(ProgramKind::kGeneric,
                                                            MODULE_DATA(sksl_public),
                                                            sharedModule,
                                                            this->coreModifiers());
        add_public_type_aliases(fModuleLoader.fPublicModule.fSymbols.get(), this->builtinTypes());
    }
    return fModuleLoader.fPublicModule;
}

const ParsedModule& ModuleLoader::loadPrivateRTShaderModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fRuntimeShaderModule.fSymbols) {
        const ParsedModule& publicModule = this->loadPublicModule(compiler);
        fModuleLoader.fRuntimeShaderModule = compiler->parseModule(ProgramKind::kFragment,
                                                                   MODULE_DATA(sksl_rt_shader),
                                                                   publicModule,
                                                                   this->coreModifiers());
    }
    return fModuleLoader.fRuntimeShaderModule;
}

const ParsedModule& ModuleLoader::loadSharedModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fSharedModule.fSymbols) {
        const ParsedModule& rootModule = this->rootModule();
        fModuleLoader.fSharedModule = compiler->parseModule(ProgramKind::kFragment,
                                                            MODULE_DATA(sksl_shared),
                                                            rootModule,
                                                            this->coreModifiers());
    }
    return fModuleLoader.fSharedModule;
}

const ParsedModule& ModuleLoader::loadGPUModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fGPUModule.fSymbols) {
        const ParsedModule& sharedModule = this->loadSharedModule(compiler);
        fModuleLoader.fGPUModule = compiler->parseModule(ProgramKind::kFragment,
                                                         MODULE_DATA(sksl_gpu),
                                                         sharedModule,
                                                         this->coreModifiers());
    }
    return fModuleLoader.fGPUModule;
}

const ParsedModule& ModuleLoader::loadFragmentModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fFragmentModule.fSymbols) {
        const ParsedModule& gpuModule = this->loadGPUModule(compiler);
        fModuleLoader.fFragmentModule = compiler->parseModule(ProgramKind::kFragment,
                                                              MODULE_DATA(sksl_frag),
                                                              gpuModule,
                                                              this->coreModifiers());
    }
    return fModuleLoader.fFragmentModule;
}

const ParsedModule& ModuleLoader::loadVertexModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fVertexModule.fSymbols) {
        const ParsedModule& gpuModule = this->loadGPUModule(compiler);
        fModuleLoader.fVertexModule = compiler->parseModule(ProgramKind::kVertex,
                                                            MODULE_DATA(sksl_vert),
                                                            gpuModule,
                                                            this->coreModifiers());
    }
    return fModuleLoader.fVertexModule;
}

const ParsedModule& ModuleLoader::loadComputeModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fComputeModule.fSymbols) {
        const ParsedModule& gpuModule = this->loadGPUModule(compiler);
        fModuleLoader.fComputeModule = compiler->parseModule(ProgramKind::kCompute,
                                                             MODULE_DATA(sksl_compute),
                                                             gpuModule,
                                                             this->coreModifiers());
        add_compute_type_aliases(fModuleLoader.fComputeModule.fSymbols.get(), this->builtinTypes());
    }
    return fModuleLoader.fComputeModule;
}

const ParsedModule& ModuleLoader::loadGraphiteFragmentModule(SkSL::Compiler* compiler) {
#if defined(SK_GRAPHITE_ENABLED)
    if (!fModuleLoader.fGraphiteFragmentModule.fSymbols) {
        const ParsedModule& fragmentModule = this->loadFragmentModule(compiler);
        fModuleLoader.fGraphiteFragmentModule= compiler->parseModule(ProgramKind::kGraphiteFragment,
                                                                    MODULE_DATA(sksl_graphite_frag),
                                                                    fragmentModule,
                                                                    this->coreModifiers());
    }
    return fModuleLoader.fGraphiteFragmentModule;
#else
    return this->loadFragmentModule(compiler);
#endif
}

const ParsedModule& ModuleLoader::loadGraphiteVertexModule(SkSL::Compiler* compiler) {
#if defined(SK_GRAPHITE_ENABLED)
    if (!fModuleLoader.fGraphiteVertexModule.fSymbols) {
        const ParsedModule& vertexModule = this->loadVertexModule(compiler);
        fModuleLoader.fGraphiteVertexModule = compiler->parseModule(ProgramKind::kGraphiteVertex,
                                                                    MODULE_DATA(sksl_graphite_vert),
                                                                    vertexModule,
                                                                    this->coreModifiers());
    }
    return fModuleLoader.fGraphiteVertexModule;
#else
    return this->loadVertexModule(compiler);
#endif
}

void ModuleLoader::Impl::makeRootSymbolTable() {
    fRootModule.fSymbols = std::make_shared<SymbolTable>(/*builtin=*/true);

    for (BuiltinTypePtr rootType : kRootTypes) {
        fRootModule.fSymbols->addWithoutOwnership((fBuiltinTypes.*rootType).get());
    }

    for (BuiltinTypePtr privateType : kPrivateTypes) {
        fRootModule.fSymbols->addWithoutOwnership((fBuiltinTypes.*privateType).get());
    }

    // sk_Caps is "builtin", but all references to it are resolved to Settings, so we don't need to
    // treat it as builtin (ie, no need to clone it into the Program).
    fRootModule.fSymbols->add(std::make_unique<Variable>(/*pos=*/Position(),
                                                         /*modifiersPosition=*/Position(),
                                                         fCoreModifiers.add(Modifiers{}),
                                                         "sk_Caps",
                                                         fBuiltinTypes.fSkCaps.get(),
                                                         /*builtin=*/false,
                                                         Variable::Storage::kGlobal));
}

}  // namespace SkSL
