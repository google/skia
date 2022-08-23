/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODULELOADER
#define SKSL_MODULELOADER

#include "include/core/SkSpan.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include <memory>

namespace SkSL {

class ModifiersPool;
struct ParsedModule;
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
    const ParsedModule& rootModule();

    // A list of all the root (public) and private types. You don't need the lock to use this.
    static SkSpan<const BuiltinTypePtr> RootTypeList();
    static SkSpan<const BuiltinTypePtr> PrivateTypeList();

    // This ModifiersPool is shared by every built-in module.
    ModifiersPool& coreModifiers();
};

}  // namespace SkSL

#endif  // SKSL_MODULELOADER
