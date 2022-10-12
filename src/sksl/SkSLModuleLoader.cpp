/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/private/SkMutex.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramKind.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLBuiltinMap.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/SkSLModuleLoader.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <string>
#include <type_traits>
#include <utility>

#if SKSL_STANDALONE

    // In standalone mode, we load the original SkSL source files. GN is responsible for copying
    // these files from src/sksl/ to the output directory.
    #include <fstream>

    static std::string load_module_file(const char* moduleFilename) {
        std::ifstream in(std::string{moduleFilename});
        std::string moduleSource{std::istreambuf_iterator<char>(in),
                                 std::istreambuf_iterator<char>()};
        if (in.rdstate()) {
            SK_ABORT("Error reading %s\n", moduleFilename);
        }
        return moduleSource;
    }

    #define MODULE_DATA(name) #name, load_module_file(#name ".sksl")

#else

    // We include minified SkSL module code and pass it directly to the compiler.
    #if defined(SK_ENABLE_OPTIMIZE_SIZE) || !defined(SK_DEBUG)
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
    #else
        #include "src/sksl/generated/sksl_shared.unoptimized.sksl"
        #include "src/sksl/generated/sksl_compute.unoptimized.sksl"
        #include "src/sksl/generated/sksl_frag.unoptimized.sksl"
        #include "src/sksl/generated/sksl_gpu.unoptimized.sksl"
        #include "src/sksl/generated/sksl_public.unoptimized.sksl"
        #include "src/sksl/generated/sksl_rt_shader.unoptimized.sksl"
        #include "src/sksl/generated/sksl_vert.unoptimized.sksl"
        #if defined(SK_GRAPHITE_ENABLED)
        #include "src/sksl/generated/sksl_graphite_frag.unoptimized.sksl"
        #include "src/sksl/generated/sksl_graphite_vert.unoptimized.sksl"
        #endif
    #endif

    #define MODULE_DATA(name) #name, std::string(SKSL_MINIFIED_##name)

#endif

namespace SkSL {

class ProgramElement;

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

    std::unique_ptr<const BuiltinMap> fRootModule;

    std::unique_ptr<const BuiltinMap> fSharedModule;           // [Root] + Public intrinsics
    std::unique_ptr<const BuiltinMap> fGPUModule;              // [Shared] + Non-public intrinsics/
                                                               //     helper functions
    std::unique_ptr<const BuiltinMap> fVertexModule;           // [GPU] + Vertex stage decls
    std::unique_ptr<const BuiltinMap> fFragmentModule;         // [GPU] + Fragment stage decls
    std::unique_ptr<const BuiltinMap> fComputeModule;          // [GPU] + Compute stage decls
    std::unique_ptr<const BuiltinMap> fGraphiteVertexModule;   // [Vert] + Graphite vertex helpers
    std::unique_ptr<const BuiltinMap> fGraphiteFragmentModule; // [Frag] + Graphite fragment helpers

    std::unique_ptr<const BuiltinMap> fPublicModule;           // [Shared] minus Private types +
                                                               //     Runtime effect intrinsics
    std::unique_ptr<const BuiltinMap> fRuntimeShaderModule;    // [Public] + Runtime shader decls
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
    fModuleLoader.fSharedModule           = nullptr;
    fModuleLoader.fGPUModule              = nullptr;
    fModuleLoader.fVertexModule           = nullptr;
    fModuleLoader.fFragmentModule         = nullptr;
    fModuleLoader.fComputeModule          = nullptr;
    fModuleLoader.fGraphiteVertexModule   = nullptr;
    fModuleLoader.fGraphiteFragmentModule = nullptr;
    fModuleLoader.fPublicModule           = nullptr;
    fModuleLoader.fRuntimeShaderModule    = nullptr;
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
}

static void add_compute_type_aliases(SkSL::SymbolTable* symbols, const SkSL::BuiltinTypes& types) {
    // A `texture2D` in a compute shader should generally mean "read-write" texture access, not
    // "sample" texture access. Remap the name `texture2D` to point to `readWriteTexture2D`.
    symbols->add(Type::MakeAliasType("texture2D", *types.fReadWriteTexture2D));
}

static std::unique_ptr<BuiltinMap> compile_to_builtin_map(SkSL::Compiler* compiler,
                                                          ProgramKind kind,
                                                          const char* moduleName,
                                                          std::string moduleSource,
                                                          const BuiltinMap* parent,
                                                          ModifiersPool& modifiersPool) {
    return compiler->compileModule(kind, moduleName, std::move(moduleSource),
                                   parent, modifiersPool, /*shouldInline=*/true)
                   .convertToBuiltinMap(parent);
}

const BuiltinTypes& ModuleLoader::builtinTypes() {
    return fModuleLoader.fBuiltinTypes;
}

ModifiersPool& ModuleLoader::coreModifiers() {
    return fModuleLoader.fCoreModifiers;
}

const BuiltinMap* ModuleLoader::rootModule() {
    return fModuleLoader.fRootModule.get();
}

const BuiltinMap* ModuleLoader::loadPublicModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fPublicModule) {
        const BuiltinMap* sharedModule = this->loadSharedModule(compiler);
        fModuleLoader.fPublicModule = compile_to_builtin_map(compiler,
                                                             ProgramKind::kGeneric,
                                                             MODULE_DATA(sksl_public),
                                                             sharedModule,
                                                             this->coreModifiers());
        add_public_type_aliases(fModuleLoader.fPublicModule->symbols().get(), this->builtinTypes());
    }
    return fModuleLoader.fPublicModule.get();
}

const BuiltinMap* ModuleLoader::loadPrivateRTShaderModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fRuntimeShaderModule) {
        const BuiltinMap* publicModule = this->loadPublicModule(compiler);
        fModuleLoader.fRuntimeShaderModule = compile_to_builtin_map(compiler,
                                                                    ProgramKind::kFragment,
                                                                    MODULE_DATA(sksl_rt_shader),
                                                                    publicModule,
                                                                    this->coreModifiers());
    }
    return fModuleLoader.fRuntimeShaderModule.get();
}

const BuiltinMap* ModuleLoader::loadSharedModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fSharedModule) {
        const BuiltinMap* rootModule = this->rootModule();
        fModuleLoader.fSharedModule = compile_to_builtin_map(compiler,
                                                             ProgramKind::kFragment,
                                                             MODULE_DATA(sksl_shared),
                                                             rootModule,
                                                             this->coreModifiers());
    }
    return fModuleLoader.fSharedModule.get();
}

const BuiltinMap* ModuleLoader::loadGPUModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fGPUModule) {
        const BuiltinMap* sharedModule = this->loadSharedModule(compiler);
        fModuleLoader.fGPUModule = compile_to_builtin_map(compiler,
                                                          ProgramKind::kFragment,
                                                          MODULE_DATA(sksl_gpu),
                                                          sharedModule,
                                                          this->coreModifiers());
    }
    return fModuleLoader.fGPUModule.get();
}

const BuiltinMap* ModuleLoader::loadFragmentModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fFragmentModule) {
        const BuiltinMap* gpuModule = this->loadGPUModule(compiler);
        fModuleLoader.fFragmentModule = compile_to_builtin_map(compiler,
                                                               ProgramKind::kFragment,
                                                               MODULE_DATA(sksl_frag),
                                                               gpuModule,
                                                               this->coreModifiers());
    }
    return fModuleLoader.fFragmentModule.get();
}

const BuiltinMap* ModuleLoader::loadVertexModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fVertexModule) {
        const BuiltinMap* gpuModule = this->loadGPUModule(compiler);
        fModuleLoader.fVertexModule = compile_to_builtin_map(compiler,
                                                             ProgramKind::kVertex,
                                                             MODULE_DATA(sksl_vert),
                                                             gpuModule,
                                                             this->coreModifiers());
    }
    return fModuleLoader.fVertexModule.get();
}

const BuiltinMap* ModuleLoader::loadComputeModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fComputeModule) {
        const BuiltinMap* gpuModule = this->loadGPUModule(compiler);
        fModuleLoader.fComputeModule = compile_to_builtin_map(compiler,
                                                              ProgramKind::kCompute,
                                                              MODULE_DATA(sksl_compute),
                                                              gpuModule,
                                                              this->coreModifiers());
        add_compute_type_aliases(fModuleLoader.fComputeModule->symbols().get(),
                                 this->builtinTypes());
    }
    return fModuleLoader.fComputeModule.get();
}

const BuiltinMap* ModuleLoader::loadGraphiteFragmentModule(SkSL::Compiler* compiler) {
#if defined(SK_GRAPHITE_ENABLED)
    if (!fModuleLoader.fGraphiteFragmentModule) {
        const BuiltinMap* fragmentModule = this->loadFragmentModule(compiler);
        fModuleLoader.fGraphiteFragmentModule = compile_to_builtin_map(compiler,
                                                                    ProgramKind::kGraphiteFragment,
                                                                    MODULE_DATA(sksl_graphite_frag),
                                                                    fragmentModule,
                                                                    this->coreModifiers());
    }
    return fModuleLoader.fGraphiteFragmentModule.get();
#else
    return this->loadFragmentModule(compiler);
#endif
}

const BuiltinMap* ModuleLoader::loadGraphiteVertexModule(SkSL::Compiler* compiler) {
#if defined(SK_GRAPHITE_ENABLED)
    if (!fModuleLoader.fGraphiteVertexModule) {
        const BuiltinMap* vertexModule = this->loadVertexModule(compiler);
        fModuleLoader.fGraphiteVertexModule = compile_to_builtin_map(compiler,
                                                                    ProgramKind::kGraphiteVertex,
                                                                    MODULE_DATA(sksl_graphite_vert),
                                                                    vertexModule,
                                                                    this->coreModifiers());
    }
    return fModuleLoader.fGraphiteVertexModule.get();
#else
    return this->loadVertexModule(compiler);
#endif
}

void ModuleLoader::Impl::makeRootSymbolTable() {
    auto symbols = std::make_shared<SymbolTable>(/*builtin=*/true);

    for (BuiltinTypePtr rootType : kRootTypes) {
        symbols->addWithoutOwnership((fBuiltinTypes.*rootType).get());
    }

    for (BuiltinTypePtr privateType : kPrivateTypes) {
        symbols->addWithoutOwnership((fBuiltinTypes.*privateType).get());
    }

    // sk_Caps is "builtin", but all references to it are resolved to Settings, so we don't need to
    // treat it as builtin (ie, no need to clone it into the Program).
    symbols->add(std::make_unique<Variable>(/*pos=*/Position(),
                                            /*modifiersPosition=*/Position(),
                                            fCoreModifiers.add(Modifiers{}),
                                            "sk_Caps",
                                            fBuiltinTypes.fSkCaps.get(),
                                            /*builtin=*/false,
                                            Variable::Storage::kGlobal));
    fRootModule =
            std::make_unique<BuiltinMap>(/*parent=*/nullptr,
                                         std::move(symbols),
                                         /*elements=*/SkSpan<std::unique_ptr<ProgramElement>>());
}

}  // namespace SkSL
