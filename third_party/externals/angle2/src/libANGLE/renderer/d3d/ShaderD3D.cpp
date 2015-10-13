//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderD3D.cpp: Defines the rx::ShaderD3D class which implements rx::ShaderImpl.

#include "libANGLE/renderer/d3d/ShaderD3D.h"

#include "common/utilities.h"
#include "libANGLE/Compiler.h"
#include "libANGLE/Shader.h"
#include "libANGLE/features.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"

// Definitions local to the translation unit
namespace
{

const char *GetShaderTypeString(GLenum type)
{
    switch (type)
    {
        case GL_VERTEX_SHADER:
            return "VERTEX";

        case GL_FRAGMENT_SHADER:
            return "FRAGMENT";

        default:
            UNREACHABLE();
            return "";
    }
}

}  // anonymous namespace

namespace rx
{

ShaderD3D::ShaderD3D(const gl::Shader::Data &data) : ShaderImpl(data)
{
    uncompile();
}

ShaderD3D::~ShaderD3D()
{
}

std::string ShaderD3D::getDebugInfo() const
{
    return mDebugInfo + std::string("\n// ") + GetShaderTypeString(mData.getShaderType()) +
           " SHADER END\n";
}

// initialize/clean up previous state
void ShaderD3D::uncompile()
{
    // set by compileToHLSL
    mCompilerOutputType = SH_ESSL_OUTPUT;

    mUsesMultipleRenderTargets = false;
    mUsesFragColor = false;
    mUsesFragData = false;
    mUsesFragCoord = false;
    mUsesFrontFacing = false;
    mUsesPointSize = false;
    mUsesPointCoord = false;
    mUsesDepthRange = false;
    mUsesFragDepth = false;
    mUsesDiscardRewriting = false;
    mUsesNestedBreak = false;
    mUsesDeferredInit = false;
    mRequiresIEEEStrictCompiling = false;

    mDebugInfo.clear();
}

void ShaderD3D::generateWorkarounds(D3DCompilerWorkarounds *workarounds) const
{
    if (mUsesDiscardRewriting)
    {
        // ANGLE issue 486:
        // Work-around a D3D9 compiler bug that presents itself when using conditional discard, by disabling optimization
        workarounds->skipOptimization = true;
    }
    else if (mUsesNestedBreak)
    {
        // ANGLE issue 603:
        // Work-around a D3D9 compiler bug that presents itself when using break in a nested loop, by maximizing optimization
        // We want to keep the use of ANGLE_D3D_WORKAROUND_MAX_OPTIMIZATION minimal to prevent hangs, so usesDiscard takes precedence
        workarounds->useMaxOptimization = true;
    }

    if (mRequiresIEEEStrictCompiling)
    {
        // IEEE Strictness for D3D compiler needs to be enabled for NaNs to work.
        workarounds->enableIEEEStrictness = true;
    }
}

unsigned int ShaderD3D::getUniformRegister(const std::string &uniformName) const
{
    ASSERT(mUniformRegisterMap.count(uniformName) > 0);
    return mUniformRegisterMap.find(uniformName)->second;
}

unsigned int ShaderD3D::getInterfaceBlockRegister(const std::string &blockName) const
{
    ASSERT(mInterfaceBlockRegisterMap.count(blockName) > 0);
    return mInterfaceBlockRegisterMap.find(blockName)->second;
}

ShShaderOutput ShaderD3D::getCompilerOutputType() const
{
    return mCompilerOutputType;
}

int ShaderD3D::prepareSourceAndReturnOptions(std::stringstream *shaderSourceStream)
{
    uncompile();

    int additionalOptions = 0;

#if !defined(ANGLE_ENABLE_WINDOWS_STORE)
    const std::string &source = mData.getSource();

    if (gl::DebugAnnotationsActive())
    {
        std::string sourcePath = getTempPath();
        writeFile(sourcePath.c_str(), source.c_str(), source.length());
        additionalOptions |= SH_LINE_DIRECTIVES | SH_SOURCE_PATH;
        *shaderSourceStream << sourcePath;
    }
#endif
    *shaderSourceStream << source;
    return additionalOptions;
}

bool ShaderD3D::postTranslateCompile(gl::Compiler *compiler, std::string *infoLog)
{
    // TODO(jmadill): We shouldn't need to cache this.
    mCompilerOutputType = compiler->getShaderOutputType();

    const std::string &translatedSource = mData.getTranslatedSource();

    mUsesMultipleRenderTargets = translatedSource.find("GL_USES_MRT") != std::string::npos;
    mUsesFragColor             = translatedSource.find("GL_USES_FRAG_COLOR") != std::string::npos;
    mUsesFragData              = translatedSource.find("GL_USES_FRAG_DATA") != std::string::npos;
    mUsesFragCoord             = translatedSource.find("GL_USES_FRAG_COORD") != std::string::npos;
    mUsesFrontFacing           = translatedSource.find("GL_USES_FRONT_FACING") != std::string::npos;
    mUsesPointSize             = translatedSource.find("GL_USES_POINT_SIZE") != std::string::npos;
    mUsesPointCoord            = translatedSource.find("GL_USES_POINT_COORD") != std::string::npos;
    mUsesDepthRange            = translatedSource.find("GL_USES_DEPTH_RANGE") != std::string::npos;
    mUsesFragDepth = translatedSource.find("GL_USES_FRAG_DEPTH") != std::string::npos;
    mUsesDiscardRewriting =
        translatedSource.find("ANGLE_USES_DISCARD_REWRITING") != std::string::npos;
    mUsesNestedBreak  = translatedSource.find("ANGLE_USES_NESTED_BREAK") != std::string::npos;
    mUsesDeferredInit = translatedSource.find("ANGLE_USES_DEFERRED_INIT") != std::string::npos;
    mRequiresIEEEStrictCompiling =
        translatedSource.find("ANGLE_REQUIRES_IEEE_STRICT_COMPILING") != std::string::npos;

    ShHandle compilerHandle = compiler->getCompilerHandle(mData.getShaderType());

    for (const sh::Uniform &uniform : mData.getUniforms())
    {
        if (uniform.staticUse && !uniform.isBuiltIn())
        {
            unsigned int index = static_cast<unsigned int>(-1);
            bool getUniformRegisterResult =
                ShGetUniformRegister(compilerHandle, uniform.name, &index);
            UNUSED_ASSERTION_VARIABLE(getUniformRegisterResult);
            ASSERT(getUniformRegisterResult);

            mUniformRegisterMap[uniform.name] = index;
        }
    }

    for (const sh::InterfaceBlock &interfaceBlock : mData.getInterfaceBlocks())
    {
        if (interfaceBlock.staticUse)
        {
            unsigned int index = static_cast<unsigned int>(-1);
            bool blockRegisterResult =
                ShGetInterfaceBlockRegister(compilerHandle, interfaceBlock.name, &index);
            UNUSED_ASSERTION_VARIABLE(blockRegisterResult);
            ASSERT(blockRegisterResult);

            mInterfaceBlockRegisterMap[interfaceBlock.name] = index;
        }
    }

#if ANGLE_SHADER_DEBUG_INFO == ANGLE_ENABLED
    mDebugInfo +=
        std::string("// ") + GetShaderTypeString(mData->getShaderType()) + " SHADER BEGIN\n";
    mDebugInfo += "\n// GLSL BEGIN\n\n" + source + "\n\n// GLSL END\n\n\n";
    mDebugInfo += "// INITIAL HLSL BEGIN\n\n" + translatedSource + "\n// INITIAL HLSL END\n\n\n";
    // Successive steps will append more info
#else
    mDebugInfo += translatedSource;
#endif
    return true;
}

}  // namespace rx
