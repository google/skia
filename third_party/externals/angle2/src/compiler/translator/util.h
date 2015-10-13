//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_UTIL_H_
#define COMPILER_TRANSLATOR_UTIL_H_

#include <stack>

#include "angle_gl.h"
#include <GLSLANG/ShaderLang.h>

#include "compiler/translator/Types.h"

// strtof_clamp is like strtof but
//   1. it forces C locale, i.e. forcing '.' as decimal point.
//   2. it clamps the value to -FLT_MAX or FLT_MAX if overflow happens.
// Return false if overflow happens.
bool strtof_clamp(const std::string &str, float *value);

// If overflow happens, clamp the value to INT_MIN or INT_MAX.
// Return false if overflow happens.
bool atoi_clamp(const char *str, int *value);

class TSymbolTable;

namespace sh
{

GLenum GLVariableType(const TType &type);
GLenum GLVariablePrecision(const TType &type);
bool IsVaryingIn(TQualifier qualifier);
bool IsVaryingOut(TQualifier qualifier);
bool IsVarying(TQualifier qualifier);
InterpolationType GetInterpolationType(TQualifier qualifier);
TString ArrayString(const TType &type);

class GetVariableTraverser : angle::NonCopyable
{
  public:
    GetVariableTraverser(const TSymbolTable &symbolTable);
    virtual ~GetVariableTraverser() {}

    template <typename VarT>
    void traverse(const TType &type, const TString &name, std::vector<VarT> *output);

  protected:
    // May be overloaded
    virtual void visitVariable(ShaderVariable *newVar) {}

  private:
    // Helper function called by traverse() to fill specific fields
    // for attributes/varyings/uniforms.
    template <typename VarT>
    void setTypeSpecificInfo(
        const TType &type, const TString &name, VarT *variable) {}

    const TSymbolTable &mSymbolTable;
};

}

#endif // COMPILER_TRANSLATOR_UTIL_H_
