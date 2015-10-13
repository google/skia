//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Compiler.cpp: implements the gl::Compiler class.

#include "libANGLE/Compiler.h"

#include "common/debug.h"
#include "libANGLE/Data.h"
#include "libANGLE/renderer/CompilerImpl.h"
#include "libANGLE/renderer/ImplFactory.h"

namespace gl
{

namespace
{

// Global count of active shader compiler handles. Needed to know when to call ShInitialize and
// ShFinalize.
size_t activeCompilerHandles = 0;

}  // anonymous namespace

Compiler::Compiler(rx::ImplFactory *implFactory, const gl::Data &data)
    : mImplementation(implFactory->createCompiler()),
      mSpec(data.clientVersion > 2 ? SH_GLES3_SPEC : SH_GLES2_SPEC),
      mOutputType(mImplementation->getTranslatorOutputType()),
      mResources(),
      mFragmentCompiler(nullptr),
      mVertexCompiler(nullptr)
{
    ASSERT(data.clientVersion == 2 || data.clientVersion == 3);

    const gl::Caps &caps             = *data.caps;
    const gl::Extensions &extensions = *data.extensions;

    ShInitBuiltInResources(&mResources);
    mResources.MaxVertexAttribs             = caps.maxVertexAttributes;
    mResources.MaxVertexUniformVectors      = caps.maxVertexUniformVectors;
    mResources.MaxVaryingVectors            = caps.maxVaryingVectors;
    mResources.MaxVertexTextureImageUnits   = caps.maxVertexTextureImageUnits;
    mResources.MaxCombinedTextureImageUnits = caps.maxCombinedTextureImageUnits;
    mResources.MaxTextureImageUnits         = caps.maxTextureImageUnits;
    mResources.MaxFragmentUniformVectors    = caps.maxFragmentUniformVectors;
    mResources.MaxDrawBuffers               = caps.maxDrawBuffers;
    mResources.OES_standard_derivatives     = extensions.standardDerivatives;
    mResources.EXT_draw_buffers             = extensions.drawBuffers;
    mResources.EXT_shader_texture_lod       = extensions.shaderTextureLOD;
    // TODO: disabled until the extension is actually supported.
    mResources.OES_EGL_image_external = 0;
    // TODO: use shader precision caps to determine if high precision is supported?
    mResources.FragmentPrecisionHigh = 1;
    mResources.EXT_frag_depth        = extensions.fragDepth;

    // GLSL ES 3.0 constants
    mResources.MaxVertexOutputVectors  = caps.maxVertexOutputComponents / 4;
    mResources.MaxFragmentInputVectors = caps.maxFragmentInputComponents / 4;
    mResources.MinProgramTexelOffset   = caps.minProgramTexelOffset;
    mResources.MaxProgramTexelOffset   = caps.maxProgramTexelOffset;
}

Compiler::~Compiler()
{
    release();
    SafeDelete(mImplementation);
}

Error Compiler::release()
{
    if (mFragmentCompiler)
    {
        ShDestruct(mFragmentCompiler);
        mFragmentCompiler = nullptr;

        ASSERT(activeCompilerHandles > 0);
        activeCompilerHandles--;
    }

    if (mVertexCompiler)
    {
        ShDestruct(mVertexCompiler);
        mVertexCompiler = nullptr;

        ASSERT(activeCompilerHandles > 0);
        activeCompilerHandles--;
    }

    if (activeCompilerHandles == 0)
    {
        ShFinalize();
    }

    mImplementation->release();

    return gl::Error(GL_NO_ERROR);
}

ShHandle Compiler::getCompilerHandle(GLenum type)
{
    ShHandle *compiler = nullptr;
    switch (type)
    {
        case GL_VERTEX_SHADER:
            compiler = &mVertexCompiler;
            break;

        case GL_FRAGMENT_SHADER:
            compiler = &mFragmentCompiler;
            break;

        default:
            UNREACHABLE();
            return nullptr;
    }

    if (!(*compiler))
    {
        if (activeCompilerHandles == 0)
        {
            ShInitialize();
        }

        *compiler = ShConstructCompiler(type, mSpec, mOutputType, &mResources);
        activeCompilerHandles++;
    }

    return *compiler;
}

}  // namespace gl
