//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompilerGL:
//   Implementation of the D3D compiler methods.
//

#include "libANGLE/renderer/d3d/CompilerD3D.h"

namespace rx
{

namespace
{

ShShaderOutput GetShaderOutputType(RendererClass rendererClass)
{
    if (rendererClass == RENDERER_D3D11)
    {
        return SH_HLSL11_OUTPUT;
    }
    else
    {
        ASSERT(rendererClass == RENDERER_D3D9);
        return SH_HLSL9_OUTPUT;
    }
}

}  // anonymous namespace

CompilerD3D::CompilerD3D(RendererClass rendererClass)
    : mTranslatorOutputType(GetShaderOutputType(rendererClass))
{
}

}  // namespace rx
