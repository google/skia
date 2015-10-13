//
// Copyright (c) 2013-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutils9.h: Queries for GL image formats and their translations to D3D9
// formats.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_FORMATUTILS9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_FORMATUTILS9_H_

#include <map>

#include "common/platform.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/formatutilsD3D.h"

namespace rx
{

class Renderer9;

namespace d3d9
{

typedef std::map<std::pair<GLenum, GLenum>, ColorCopyFunction> FastCopyFunctionMap;

struct D3DFormat
{
    D3DFormat();

    GLuint pixelBytes;
    GLuint blockWidth;
    GLuint blockHeight;

    GLuint redBits;
    GLuint greenBits;
    GLuint blueBits;
    GLuint alphaBits;
    GLuint luminanceBits;

    GLuint depthBits;
    GLuint stencilBits;

    GLenum internalFormat;

    MipGenerationFunction mipGenerationFunction;
    ColorReadFunction colorReadFunction;

    FastCopyFunctionMap fastCopyFunctions;
    ColorCopyFunction getFastCopyFunction(GLenum format, GLenum type) const;
};
const D3DFormat &GetD3DFormatInfo(D3DFORMAT format);

struct VertexFormat
{
    VertexFormat();

    VertexConversionType conversionType;
    size_t outputElementSize;
    VertexCopyFunction copyFunction;
    D3DDECLTYPE nativeFormat;
    GLenum componentType;
};
const VertexFormat &GetVertexFormatInfo(DWORD supportedDeclTypes, gl::VertexFormatType vertexFormatType);

struct TextureFormat
{
    TextureFormat();

    D3DFORMAT texFormat;
    D3DFORMAT renderFormat;

    InitializeTextureDataFunction dataInitializerFunction;

    LoadImageFunction loadFunction;
};
const TextureFormat &GetTextureFormatInfo(GLenum internalFormat);

}

}

#endif // LIBANGLE_RENDERER_D3D_D3D9_FORMATUTILS9_H_
