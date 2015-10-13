//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_INITIALIZE_H_
#define COMPILER_TRANSLATOR_INITIALIZE_H_

#include "compiler/translator/Common.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/SymbolTable.h"

void InsertBuiltInFunctions(sh::GLenum type, ShShaderSpec spec, const ShBuiltInResources &resources, TSymbolTable &table);

void IdentifyBuiltIns(sh::GLenum type, ShShaderSpec spec,
                      const ShBuiltInResources& resources,
                      TSymbolTable& symbolTable);

void InitExtensionBehavior(const ShBuiltInResources& resources,
                           TExtensionBehavior& extensionBehavior);

// Resets the behavior of the extensions listed in |extensionBehavior| to the
// undefined state. These extensions will only be those initially supported in
// the ShBuiltInResources object for this compiler instance. All other
// extensions will remain unsupported.
void ResetExtensionBehavior(TExtensionBehavior &extensionBehavior);

#endif // COMPILER_TRANSLATOR_INITIALIZE_H_
