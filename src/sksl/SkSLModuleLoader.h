/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODULELOADER
#define SKSL_MODULELOADER

#include "src/sksl/SkSLBuiltinTypes.h"
#include <memory>

namespace SkSL {

class Compiler;
struct LoadedModule;
class ModifiersPool;
class Type;

using BuiltinTypePtr = const std::unique_ptr<Type> BuiltinTypes::*;

class ModuleLoader {
private:
    struct Impl;
    Impl& fModuleLoader;

public:
    ModuleLoader(ModuleLoader::Impl&);
    ~ModuleLoader();

    // Acquires a mutex-locked reference to the singleton ModuleLoader. When the ModuleLoader is
    // allowed to fall out of scope, the mutex will be released.
    static ModuleLoader Get();

    // The built-in types and root module are universal, immutable, and shared by every Compiler.
    // They are created when the ModuleLoader is instantiated and never change.
    const BuiltinTypes& builtinTypes();
    const LoadedModule* rootModule();

    // This ModifiersPool is shared by every built-in module.
    ModifiersPool& coreModifiers();

    // These modules are loaded on demand; once loaded, they are kept for the lifetime of the
    // process.
    const LoadedModule* loadSharedModule(SkSL::Compiler* compiler);
    const LoadedModule* loadGPUModule(SkSL::Compiler* compiler);
    const LoadedModule* loadVertexModule(SkSL::Compiler* compiler);
    const LoadedModule* loadFragmentModule(SkSL::Compiler* compiler);
    const LoadedModule* loadComputeModule(SkSL::Compiler* compiler);
    const LoadedModule* loadGraphiteVertexModule(SkSL::Compiler* compiler);
    const LoadedModule* loadGraphiteFragmentModule(SkSL::Compiler* compiler);

    const LoadedModule* loadPublicModule(SkSL::Compiler* compiler);
    const LoadedModule* loadPrivateRTShaderModule(SkSL::Compiler* compiler);

    // This unloads every module. It's useful primarily for benchmarking purposes.
    void unloadModules();
};

}  // namespace SkSL

#endif  // SKSL_MODULELOADER
