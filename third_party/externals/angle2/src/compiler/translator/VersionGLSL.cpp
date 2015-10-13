//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/VersionGLSL.h"

int ShaderOutputTypeToGLSLVersion(ShShaderOutput output)
{
    switch (output)
    {
      case SH_GLSL_130_OUTPUT:           return GLSL_VERSION_130;
      case SH_GLSL_140_OUTPUT:           return GLSL_VERSION_140;
      case SH_GLSL_150_CORE_OUTPUT:      return GLSL_VERSION_150;
      case SH_GLSL_330_CORE_OUTPUT:      return GLSL_VERSION_330;
      case SH_GLSL_400_CORE_OUTPUT:      return GLSL_VERSION_400;
      case SH_GLSL_410_CORE_OUTPUT:      return GLSL_VERSION_410;
      case SH_GLSL_420_CORE_OUTPUT:      return GLSL_VERSION_420;
      case SH_GLSL_430_CORE_OUTPUT:      return GLSL_VERSION_430;
      case SH_GLSL_440_CORE_OUTPUT:      return GLSL_VERSION_440;
      case SH_GLSL_450_CORE_OUTPUT:      return GLSL_VERSION_450;
      case SH_GLSL_COMPATIBILITY_OUTPUT: return GLSL_VERSION_110;
      default: UNREACHABLE();            return 0;
    }
}

// We need to scan for the following:
// 1. "invariant" keyword: This can occur in both - vertex and fragment shaders
//    but only at the global scope.
// 2. "gl_PointCoord" built-in variable: This can only occur in fragment shader
//    but inside any scope.
// 3. Call to a matrix constructor with another matrix as argument.
//    (These constructors were reserved in GLSL version 1.10.)
// 4. Arrays as "out" function parameters.
//    GLSL spec section 6.1.1: "When calling a function, expressions that do
//    not evaluate to l-values cannot be passed to parameters declared as
//    out or inout."
//    GLSL 1.1 section 5.8: "Other binary or unary expressions,
//    non-dereferenced arrays, function names, swizzles with repeated fields,
//    and constants cannot be l-values."
//    GLSL 1.2 relaxed the restriction on arrays, section 5.8: "Variables that
//    are built-in types, entire structures or arrays... are all l-values."
//
TVersionGLSL::TVersionGLSL(sh::GLenum type,
                           const TPragma &pragma,
                           ShShaderOutput output)
    : TIntermTraverser(true, false, false)
{
    mVersion = ShaderOutputTypeToGLSLVersion(output);
    if (pragma.stdgl.invariantAll)
    {
        ensureVersionIsAtLeast(GLSL_VERSION_120);
    }
}

void TVersionGLSL::visitSymbol(TIntermSymbol *node)
{
    if (node->getSymbol() == "gl_PointCoord")
    {
        ensureVersionIsAtLeast(GLSL_VERSION_120);
    }
}

bool TVersionGLSL::visitAggregate(Visit, TIntermAggregate *node)
{
    bool visitChildren = true;

    switch (node->getOp())
    {
      case EOpSequence:
        // We need to visit sequence children to get to global or inner scope.
        visitChildren = true;
        break;
      case EOpDeclaration:
        {
            const TIntermSequence &sequence = *(node->getSequence());
            if (sequence.front()->getAsTyped()->getType().isInvariant())
            {
                ensureVersionIsAtLeast(GLSL_VERSION_120);
            }
            break;
        }
      case EOpInvariantDeclaration:
        ensureVersionIsAtLeast(GLSL_VERSION_120);
        break;
      case EOpParameters:
        {
            const TIntermSequence &params = *(node->getSequence());
            for (TIntermSequence::const_iterator iter = params.begin();
                 iter != params.end(); ++iter)
            {
                const TIntermTyped *param = (*iter)->getAsTyped();
                if (param->isArray())
                {
                    TQualifier qualifier = param->getQualifier();
                    if ((qualifier == EvqOut) || (qualifier ==  EvqInOut))
                    {
                        ensureVersionIsAtLeast(GLSL_VERSION_120);
                        break;
                    }
                }
            }
            // Fully processed. No need to visit children.
            visitChildren = false;
            break;
        }
      case EOpConstructMat2:
      case EOpConstructMat2x3:
      case EOpConstructMat2x4:
      case EOpConstructMat3x2:
      case EOpConstructMat3:
      case EOpConstructMat3x4:
      case EOpConstructMat4x2:
      case EOpConstructMat4x3:
      case EOpConstructMat4:
        {
            const TIntermSequence &sequence = *(node->getSequence());
            if (sequence.size() == 1)
            {
                TIntermTyped *typed = sequence.front()->getAsTyped();
                if (typed && typed->isMatrix())
                {
                    ensureVersionIsAtLeast(GLSL_VERSION_120);
                }
            }
            break;
        }
      default:
        break;
    }

    return visitChildren;
}

void TVersionGLSL::ensureVersionIsAtLeast(int version)
{
    mVersion = std::max(version, mVersion);
}

