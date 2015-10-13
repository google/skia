//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// texture_format_util:
//   Contains helper functions for texture_format_table
//

#ifndef LIBANGLE_RENDERER_D3D_D3D11_TEXTUREFORMATUTIL_H_
#define LIBANGLE_RENDERER_D3D_D3D11_TEXTUREFORMATUTIL_H_

#include <map>

#include "libANGLE/renderer/d3d/d3d11/renderer11.h"

namespace rx
{

namespace d3d11
{

typedef std::pair<DXGI_FORMAT, LoadImageFunction> DxgiFormatLoadFunctionPair;
typedef std::pair<GLenum, DxgiFormatLoadFunctionPair> GLTypeDXGIFunctionPair;
typedef std::map<GLenum, std::vector<GLTypeDXGIFunctionPair>> D3D11LoadFunctionMap;

const D3D11LoadFunctionMap &BuildD3D11LoadFunctionMap();

typedef std::pair<GLint, DXGI_FORMAT> InitializeTextureFormatPair;
typedef std::map<InitializeTextureFormatPair, InitializeTextureDataFunction>
    InternalFormatInitializerMap;

const InternalFormatInitializerMap &BuildInternalFormatInitializerMap();

}  // namespace d3d11

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_TEXTUREFORMATUTIL_H_
