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
struct Module;
class Type;

using BuiltinTypePtr = const std::unique_ptr<Type> BuiltinTypes::*;

/**
 * Documentation for modules in SkSL: http://go/modules-in-sksl
 * https://docs.google.com/document/d/1P8LkkimNr-nPlxMimUsz3K_7qMM7-tZOxDCWZejPcWg/edit?usp=sharing
 */
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
    const Module* rootModule();

    // These modules are loaded on demand; once loaded, they are kept for the lifetime of the
    // process.
    const Module* loadSharedModule(SkSL::Compiler* compiler);
    const Module* loadGPUModule(SkSL::Compiler* compiler);
    const Module* loadVertexModule(SkSL::Compiler* compiler);
    const Module* loadFragmentModule(SkSL::Compiler* compiler);
    const Module* loadComputeModule(SkSL::Compiler* compiler);
    const Module* loadGraphiteVertexModule(SkSL::Compiler* compiler);
    const Module* loadGraphiteFragmentModule(SkSL::Compiler* compiler);

    const Module* loadPublicModule(SkSL::Compiler* compiler);
    const Module* loadPrivateRTShaderModule(SkSL::Compiler* compiler);

    // This updates an existing Module's symbol table to match Runtime Effect rules. GLSL types like
    // `vec4` are added; SkSL private types like `sampler2D` are replaced with an invalid type.
    void addPublicTypeAliases(const SkSL::Module* module);

    // This unloads every module. It's useful primarily for benchmarking purposes.
    void unloadModules();
};

}  // namespace SkSL

#endif  // SKSL_MODULELOADER
