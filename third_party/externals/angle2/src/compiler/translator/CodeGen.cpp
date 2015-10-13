//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/TranslatorESSL.h"
#include "compiler/translator/TranslatorGLSL.h"
#ifdef ANGLE_ENABLE_HLSL
#include "compiler/translator/TranslatorHLSL.h"
#endif // ANGLE_ENABLE_HLSL

//
// This function must be provided to create the actual
// compile object used by higher level code.  It returns
// a subclass of TCompiler.
//
TCompiler* ConstructCompiler(
    sh::GLenum type, ShShaderSpec spec, ShShaderOutput output)
{
    switch (output) {
      case SH_ESSL_OUTPUT:
        return new TranslatorESSL(type, spec);
      case SH_GLSL_130_OUTPUT:
      case SH_GLSL_140_OUTPUT:
      case SH_GLSL_150_CORE_OUTPUT:
      case SH_GLSL_330_CORE_OUTPUT:
      case SH_GLSL_400_CORE_OUTPUT:
      case SH_GLSL_410_CORE_OUTPUT:
      case SH_GLSL_420_CORE_OUTPUT:
      case SH_GLSL_430_CORE_OUTPUT:
      case SH_GLSL_440_CORE_OUTPUT:
      case SH_GLSL_450_CORE_OUTPUT:
      case SH_GLSL_COMPATIBILITY_OUTPUT:
        return new TranslatorGLSL(type, spec, output);
      case SH_HLSL9_OUTPUT:
      case SH_HLSL11_OUTPUT:
#ifdef ANGLE_ENABLE_HLSL
        return new TranslatorHLSL(type, spec, output);
#else
        // This compiler is not supported in this
        // configuration. Return NULL per the ShConstructCompiler API.
        return NULL;
#endif // ANGLE_ENABLE_HLSL
      default:
        // Unknown format. Return NULL per the ShConstructCompiler API.
        return NULL;
    }
}

//
// Delete the compiler made by ConstructCompiler
//
void DeleteCompiler(TCompiler* compiler)
{
    delete compiler;
}
