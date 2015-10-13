//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/TranslatorHLSL.h"

#include "compiler/translator/ArrayReturnValueToOutParameter.h"
#include "compiler/translator/OutputHLSL.h"
#include "compiler/translator/RemoveDynamicIndexing.h"
#include "compiler/translator/RewriteElseBlocks.h"
#include "compiler/translator/SeparateArrayInitialization.h"
#include "compiler/translator/SeparateDeclarations.h"
#include "compiler/translator/SeparateExpressionsReturningArrays.h"
#include "compiler/translator/UnfoldShortCircuitToIf.h"

TranslatorHLSL::TranslatorHLSL(sh::GLenum type, ShShaderSpec spec, ShShaderOutput output)
    : TCompiler(type, spec, output)
{
}

void TranslatorHLSL::translate(TIntermNode *root, int compileOptions)
{
    const ShBuiltInResources &resources = getResources();
    int numRenderTargets = resources.EXT_draw_buffers ? resources.MaxDrawBuffers : 1;

    SeparateDeclarations(root);

    unsigned int temporaryIndex = 0;

    // Note that SeparateDeclarations needs to be run before UnfoldShortCircuitToIf.
    UnfoldShortCircuitToIf(root, &temporaryIndex);

    SeparateExpressionsReturningArrays(root, &temporaryIndex);

    // Note that SeparateDeclarations needs to be run before SeparateArrayInitialization.
    SeparateArrayInitialization(root);

    // HLSL doesn't support arrays as return values, we'll need to make functions that have an array
    // as a return value to use an out parameter to transfer the array data instead.
    ArrayReturnValueToOutParameter(root, &temporaryIndex);

    if (!shouldRunLoopAndIndexingValidation(compileOptions))
    {
        // HLSL doesn't support dynamic indexing of vectors and matrices.
        RemoveDynamicIndexing(root, &temporaryIndex, getSymbolTable(), getShaderVersion());
    }

    // Work around D3D9 bug that would manifest in vertex shaders with selection blocks which
    // use a vertex attribute as a condition, and some related computation in the else block.
    if (getOutputType() == SH_HLSL9_OUTPUT && getShaderType() == GL_VERTEX_SHADER)
    {
        sh::RewriteElseBlocks(root, &temporaryIndex);
    }

    sh::OutputHLSL outputHLSL(getShaderType(), getShaderVersion(), getExtensionBehavior(),
        getSourcePath(), getOutputType(), numRenderTargets, getUniforms(), compileOptions);

    outputHLSL.output(root, getInfoSink().obj);

    mInterfaceBlockRegisterMap = outputHLSL.getInterfaceBlockRegisterMap();
    mUniformRegisterMap = outputHLSL.getUniformRegisterMap();
}

bool TranslatorHLSL::hasInterfaceBlock(const std::string &interfaceBlockName) const
{
    return (mInterfaceBlockRegisterMap.count(interfaceBlockName) > 0);
}

unsigned int TranslatorHLSL::getInterfaceBlockRegister(const std::string &interfaceBlockName) const
{
    ASSERT(hasInterfaceBlock(interfaceBlockName));
    return mInterfaceBlockRegisterMap.find(interfaceBlockName)->second;
}

bool TranslatorHLSL::hasUniform(const std::string &uniformName) const
{
    return (mUniformRegisterMap.count(uniformName) > 0);
}

unsigned int TranslatorHLSL::getUniformRegister(const std::string &uniformName) const
{
    ASSERT(hasUniform(uniformName));
    return mUniformRegisterMap.find(uniformName)->second;
}
