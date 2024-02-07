/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/sksl/SkSLModuleLoader.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkMutex.h"
#include "src/base/SkNoDestructor.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#if SKSL_STANDALONE

#include "include/core/SkString.h"
#include "src/utils/SkOSPath.h"
#include "tools/SkGetExecutablePath.h"

    // In standalone mode, we load the original SkSL source files. GN is responsible for copying
    // these files from src/sksl/ to the directory where the executable is located.
    #include <fstream>

    static std::string load_module_file(const char* moduleFilename) {
        std::string exePath = SkGetExecutablePath();
        SkString exeDir = SkOSPath::Dirname(exePath.c_str());
        SkString modulePath = SkOSPath::Join(exeDir.c_str(), moduleFilename);
        std::ifstream in(std::string{modulePath.c_str()});
        std::string moduleSource{std::istreambuf_iterator<char>(in),
                                 std::istreambuf_iterator<char>()};
        if (in.rdstate()) {
            SK_ABORT("Error reading %s\n", modulePath.c_str());
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
        #if defined(SK_GRAPHITE)
        #include "src/sksl/generated/sksl_graphite_frag.minified.sksl"
        #include "src/sksl/generated/sksl_graphite_vert.minified.sksl"
        #include "src/sksl/generated/sksl_graphite_frag_es2.minified.sksl"
        #include "src/sksl/generated/sksl_graphite_vert_es2.minified.sksl"
        #endif
    #else
        #include "src/sksl/generated/sksl_shared.unoptimized.sksl"
        #include "src/sksl/generated/sksl_compute.unoptimized.sksl"
        #include "src/sksl/generated/sksl_frag.unoptimized.sksl"
        #include "src/sksl/generated/sksl_gpu.unoptimized.sksl"
        #include "src/sksl/generated/sksl_public.unoptimized.sksl"
        #include "src/sksl/generated/sksl_rt_shader.unoptimized.sksl"
        #include "src/sksl/generated/sksl_vert.unoptimized.sksl"
        #if defined(SK_GRAPHITE)
        #include "src/sksl/generated/sksl_graphite_frag.unoptimized.sksl"
        #include "src/sksl/generated/sksl_graphite_vert.unoptimized.sksl"
        #include "src/sksl/generated/sksl_graphite_frag_es2.unoptimized.sksl"
        #include "src/sksl/generated/sksl_graphite_vert_es2.unoptimized.sksl"
        #endif
    #endif

    #define MODULE_DATA(name) #name, std::string(SKSL_MINIFIED_##name)

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
    TYPE(Texture2D_sample),
    TYPE(Texture2D), TYPE(ReadOnlyTexture2D), TYPE(WriteOnlyTexture2D),
    TYPE(GenTexture2D), TYPE(ReadableTexture2D), TYPE(WritableTexture2D),

    TYPE(AtomicUInt),
};

#undef TYPE

struct ModuleLoader::Impl {
    Impl();

    void makeRootSymbolTable();

    // This mutex is taken when ModuleLoader::Get is called, and released when the returned
    // ModuleLoader object falls out of scope.
    SkMutex fMutex;
    const BuiltinTypes fBuiltinTypes;

    std::unique_ptr<const Module> fRootModule;

    std::unique_ptr<const Module> fSharedModule;            // [Root] + Public intrinsics
    std::unique_ptr<const Module> fGPUModule;               // [Shared] + Non-public intrinsics/
                                                            //     helper functions
    std::unique_ptr<const Module> fVertexModule;            // [GPU] + Vertex stage decls
    std::unique_ptr<const Module> fFragmentModule;          // [GPU] + Fragment stage decls
    std::unique_ptr<const Module> fComputeModule;           // [GPU] + Compute stage decls
    std::unique_ptr<const Module> fGraphiteVertexModule;    // [Vert] + Graphite vertex helpers
    std::unique_ptr<const Module> fGraphiteFragmentModule;  // [Frag] + Graphite fragment helpers
    std::unique_ptr<const Module> fGraphiteVertexES2Module; // [Vert] + Graphite vertex ES2 helpers
    std::unique_ptr<const Module> fGraphiteFragmentES2Module;//[Frag] + Graphite fragment ES2 "  "

    std::unique_ptr<const Module> fPublicModule;            // [Shared] minus Private types +
                                                            //     Runtime effect intrinsics
    std::unique_ptr<const Module> fRuntimeShaderModule;     // [Public] + Runtime shader decls
};

ModuleLoader ModuleLoader::Get() {
    static SkNoDestructor<ModuleLoader::Impl> sModuleLoaderImpl;
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

static std::unique_ptr<Module> compile_and_shrink(SkSL::Compiler* compiler,
                                                  ProgramKind kind,
                                                  const char* moduleName,
                                                  std::string moduleSource,
                                                  const Module* parent) {
    std::unique_ptr<Module> m = compiler->compileModule(kind,
                                                        moduleName,
                                                        std::move(moduleSource),
                                                        parent,
                                                        /*shouldInline=*/true);
    if (!m) {
        SK_ABORT("Unable to load module %s", moduleName);
    }

    // We can eliminate FunctionPrototypes without changing the meaning of the module; the function
    // declaration is still safely in the symbol table. This only impacts our ability to recreate
    // the input verbatim, which we don't care about at runtime.
    m->fElements.erase(std::remove_if(m->fElements.begin(), m->fElements.end(),
                                      [](const std::unique_ptr<ProgramElement>& element) {
                                          switch (element->kind()) {
                                              case ProgramElement::Kind::kFunction:
                                              case ProgramElement::Kind::kGlobalVar:
                                              case ProgramElement::Kind::kInterfaceBlock:
                                              case ProgramElement::Kind::kStructDefinition:
                                                  // We need to preserve these.
                                                  return false;

                                              case ProgramElement::Kind::kFunctionPrototype:
                                                  // These are already in the symbol table; the
                                                  // ProgramElement isn't needed anymore.
                                                  return true;

                                              default:
                                                  SkDEBUGFAILF("Unsupported element: %s\n",
                                                               element->description().c_str());
                                                  return false;
                                          }
                                      }),
                       m->fElements.end());

    m->fElements.shrink_to_fit();
    return m;
}

const BuiltinTypes& ModuleLoader::builtinTypes() {
    return fModuleLoader.fBuiltinTypes;
}

const Module* ModuleLoader::rootModule() {
    return fModuleLoader.fRootModule.get();
}

void ModuleLoader::addPublicTypeAliases(const SkSL::Module* module) {
    const SkSL::BuiltinTypes& types = this->builtinTypes();
    SymbolTable* symbols = module->fSymbols.get();

    // Add some aliases to the runtime effect modules so that it's friendlier, and more like GLSL.
    symbols->addWithoutOwnershipOrDie(types.fVec2.get());
    symbols->addWithoutOwnershipOrDie(types.fVec3.get());
    symbols->addWithoutOwnershipOrDie(types.fVec4.get());

    symbols->addWithoutOwnershipOrDie(types.fIVec2.get());
    symbols->addWithoutOwnershipOrDie(types.fIVec3.get());
    symbols->addWithoutOwnershipOrDie(types.fIVec4.get());

    symbols->addWithoutOwnershipOrDie(types.fBVec2.get());
    symbols->addWithoutOwnershipOrDie(types.fBVec3.get());
    symbols->addWithoutOwnershipOrDie(types.fBVec4.get());

    symbols->addWithoutOwnershipOrDie(types.fMat2.get());
    symbols->addWithoutOwnershipOrDie(types.fMat3.get());
    symbols->addWithoutOwnershipOrDie(types.fMat4.get());

    symbols->addWithoutOwnershipOrDie(types.fMat2x2.get());
    symbols->addWithoutOwnershipOrDie(types.fMat2x3.get());
    symbols->addWithoutOwnershipOrDie(types.fMat2x4.get());
    symbols->addWithoutOwnershipOrDie(types.fMat3x2.get());
    symbols->addWithoutOwnershipOrDie(types.fMat3x3.get());
    symbols->addWithoutOwnershipOrDie(types.fMat3x4.get());
    symbols->addWithoutOwnershipOrDie(types.fMat4x2.get());
    symbols->addWithoutOwnershipOrDie(types.fMat4x3.get());
    symbols->addWithoutOwnershipOrDie(types.fMat4x4.get());

    // Hide all the private symbols by aliasing them all to "invalid". This will prevent code from
    // using built-in names like `sampler2D` as variable names.
    for (BuiltinTypePtr privateType : kPrivateTypes) {
        symbols->inject(Type::MakeAliasType((types.*privateType)->name(), *types.fInvalid));
    }
}

const Module* ModuleLoader::loadPublicModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fPublicModule) {
        const Module* sharedModule = this->loadSharedModule(compiler);
        fModuleLoader.fPublicModule = compile_and_shrink(compiler,
                                                         ProgramKind::kFragment,
                                                         MODULE_DATA(sksl_public),
                                                         sharedModule);
        this->addPublicTypeAliases(fModuleLoader.fPublicModule.get());
    }
    return fModuleLoader.fPublicModule.get();
}

const Module* ModuleLoader::loadPrivateRTShaderModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fRuntimeShaderModule) {
        const Module* publicModule = this->loadPublicModule(compiler);
        fModuleLoader.fRuntimeShaderModule = compile_and_shrink(compiler,
                                                                ProgramKind::kFragment,
                                                                MODULE_DATA(sksl_rt_shader),
                                                                publicModule);
    }
    return fModuleLoader.fRuntimeShaderModule.get();
}

const Module* ModuleLoader::loadSharedModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fSharedModule) {
        const Module* rootModule = this->rootModule();
        fModuleLoader.fSharedModule = compile_and_shrink(compiler,
                                                         ProgramKind::kFragment,
                                                         MODULE_DATA(sksl_shared),
                                                         rootModule);
    }
    return fModuleLoader.fSharedModule.get();
}

const Module* ModuleLoader::loadGPUModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fGPUModule) {
        const Module* sharedModule = this->loadSharedModule(compiler);
        fModuleLoader.fGPUModule = compile_and_shrink(compiler,
                                                      ProgramKind::kFragment,
                                                      MODULE_DATA(sksl_gpu),
                                                      sharedModule);
    }
    return fModuleLoader.fGPUModule.get();
}

const Module* ModuleLoader::loadFragmentModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fFragmentModule) {
        const Module* gpuModule = this->loadGPUModule(compiler);
        fModuleLoader.fFragmentModule = compile_and_shrink(compiler,
                                                           ProgramKind::kFragment,
                                                           MODULE_DATA(sksl_frag),
                                                           gpuModule);
    }
    return fModuleLoader.fFragmentModule.get();
}

const Module* ModuleLoader::loadVertexModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fVertexModule) {
        const Module* gpuModule = this->loadGPUModule(compiler);
        fModuleLoader.fVertexModule = compile_and_shrink(compiler,
                                                         ProgramKind::kVertex,
                                                         MODULE_DATA(sksl_vert),
                                                         gpuModule);
    }
    return fModuleLoader.fVertexModule.get();
}

const Module* ModuleLoader::loadComputeModule(SkSL::Compiler* compiler) {
    if (!fModuleLoader.fComputeModule) {
        const Module* gpuModule = this->loadGPUModule(compiler);
        fModuleLoader.fComputeModule = compile_and_shrink(compiler,
                                                          ProgramKind::kCompute,
                                                          MODULE_DATA(sksl_compute),
                                                          gpuModule);
    }
    return fModuleLoader.fComputeModule.get();
}

const Module* ModuleLoader::loadGraphiteFragmentModule(SkSL::Compiler* compiler) {
#if defined(SK_GRAPHITE)
    if (!fModuleLoader.fGraphiteFragmentModule) {
        const Module* fragmentModule = this->loadFragmentModule(compiler);
        fModuleLoader.fGraphiteFragmentModule = compile_and_shrink(compiler,
                                                                   ProgramKind::kGraphiteFragment,
                                                                   MODULE_DATA(sksl_graphite_frag),
                                                                   fragmentModule);
    }
    return fModuleLoader.fGraphiteFragmentModule.get();
#else
    return this->loadFragmentModule(compiler);
#endif
}

const Module* ModuleLoader::loadGraphiteFragmentES2Module(SkSL::Compiler* compiler) {
#if defined(SK_GRAPHITE)
    if (!fModuleLoader.fGraphiteFragmentES2Module) {
        const Module* fragmentModule = this->loadFragmentModule(compiler);
        fModuleLoader.fGraphiteFragmentES2Module =
                compile_and_shrink(compiler,
                                   ProgramKind::kGraphiteFragmentES2,
                                   MODULE_DATA(sksl_graphite_frag_es2),
                                   fragmentModule);
    }
    return fModuleLoader.fGraphiteFragmentES2Module.get();
#else
    return this->loadFragmentModule(compiler);
#endif
}

const Module* ModuleLoader::loadGraphiteVertexModule(SkSL::Compiler* compiler) {
#if defined(SK_GRAPHITE)
    if (!fModuleLoader.fGraphiteVertexModule) {
        const Module* vertexModule = this->loadVertexModule(compiler);
        fModuleLoader.fGraphiteVertexModule = compile_and_shrink(compiler,
                                                                 ProgramKind::kGraphiteVertex,
                                                                 MODULE_DATA(sksl_graphite_vert),
                                                                 vertexModule);
    }
    return fModuleLoader.fGraphiteVertexModule.get();
#else
    return this->loadVertexModule(compiler);
#endif
}

const Module* ModuleLoader::loadGraphiteVertexES2Module(SkSL::Compiler* compiler) {
#if defined(SK_GRAPHITE)
    if (!fModuleLoader.fGraphiteVertexES2Module) {
        const Module* vertexModule = this->loadVertexModule(compiler);
        fModuleLoader.fGraphiteVertexES2Module =
                compile_and_shrink(compiler,
                                   ProgramKind::kGraphiteVertexES2,
                                   MODULE_DATA(sksl_graphite_vert_es2),
                                   vertexModule);
    }
    return fModuleLoader.fGraphiteVertexES2Module.get();
#else
    return this->loadVertexModule(compiler);
#endif
}

void ModuleLoader::Impl::makeRootSymbolTable() {
    auto rootModule = std::make_unique<Module>();
    rootModule->fSymbols = std::make_unique<SymbolTable>(/*builtin=*/true);

    for (BuiltinTypePtr rootType : kRootTypes) {
        rootModule->fSymbols->addWithoutOwnershipOrDie((fBuiltinTypes.*rootType).get());
    }

    for (BuiltinTypePtr privateType : kPrivateTypes) {
        rootModule->fSymbols->addWithoutOwnershipOrDie((fBuiltinTypes.*privateType).get());
    }

    // sk_Caps is "builtin", but all references to it are resolved to Settings, so we don't need to
    // treat it as builtin (ie, no need to clone it into the Program).
    rootModule->fSymbols->addOrDie(Variable::Make(/*pos=*/Position(),
                                                  /*modifiersPosition=*/Position(),
                                                  Layout{},
                                                  ModifierFlag::kNone,
                                                  fBuiltinTypes.fSkCaps.get(),
                                                  "sk_Caps",
                                                  /*mangledName=*/"",
                                                  /*builtin=*/false,
                                                  Variable::Storage::kGlobal));
    fRootModule = std::move(rootModule);
}

}  // namespace SkSL
