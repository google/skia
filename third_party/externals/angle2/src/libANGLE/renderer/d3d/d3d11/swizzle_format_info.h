//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// swizzle_format_info:
//   Provides information for swizzle format and a map from type->formatinfo
//

#ifndef LIBANGLE_RENDERER_D3D_D3D11_SWIZZLEFORMATINFO_H_
#define LIBANGLE_RENDERER_D3D_D3D11_SWIZZLEFORMATINFO_H_

#include <GLES2/gl2.h>
#include <map>

#include "common/platform.h"

namespace rx
{

namespace d3d11
{

struct SwizzleSizeType
{
    size_t maxComponentSize;
    GLenum componentType;

    SwizzleSizeType();
    SwizzleSizeType(size_t maxComponentSize, GLenum componentType);

    bool operator<(const SwizzleSizeType &other) const;
};

struct SwizzleFormatInfo
{
    DXGI_FORMAT mTexFormat;
    DXGI_FORMAT mSRVFormat;
    DXGI_FORMAT mRTVFormat;

    SwizzleFormatInfo();
    SwizzleFormatInfo(DXGI_FORMAT texFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvFormat);
};

typedef std::map<SwizzleSizeType, SwizzleFormatInfo> SwizzleInfoMap;

const SwizzleInfoMap &BuildSwizzleInfoMap();

}  // namespace d3d11

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_SWIZZLEFORMATINFO_H_
