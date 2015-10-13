//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// swizzle_format_info:
//   Provides information for swizzle format and a map from type->formatinfo
//

#include "libANGLE/renderer/d3d/d3d11/swizzle_format_info.h"

#include <GLES3/gl3.h>

namespace rx
{

namespace d3d11
{

SwizzleSizeType::SwizzleSizeType() : maxComponentSize(0), componentType(GL_NONE)
{
}

SwizzleSizeType::SwizzleSizeType(size_t maxComponentSize, GLenum componentType)
    : maxComponentSize(maxComponentSize), componentType(componentType)
{
}

bool SwizzleSizeType::operator<(const SwizzleSizeType &other) const
{
    return (maxComponentSize != other.maxComponentSize)
               ? (maxComponentSize < other.maxComponentSize)
               : (componentType < other.componentType);
}

SwizzleFormatInfo::SwizzleFormatInfo()
    : mTexFormat(DXGI_FORMAT_UNKNOWN),
      mSRVFormat(DXGI_FORMAT_UNKNOWN),
      mRTVFormat(DXGI_FORMAT_UNKNOWN)
{
}

SwizzleFormatInfo::SwizzleFormatInfo(DXGI_FORMAT texFormat,
                                     DXGI_FORMAT srvFormat,
                                     DXGI_FORMAT rtvFormat)
    : mTexFormat(texFormat), mSRVFormat(srvFormat), mRTVFormat(rtvFormat)
{
}

typedef std::pair<SwizzleSizeType, SwizzleFormatInfo> SwizzleInfoPair;

const SwizzleInfoMap &BuildSwizzleInfoMap()
{
    static SwizzleInfoMap map;

    map.insert(
        SwizzleInfoPair(SwizzleSizeType(8, GL_UNSIGNED_NORMALIZED),
                        SwizzleFormatInfo(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM,
                                          DXGI_FORMAT_R8G8B8A8_UNORM)));
    map.insert(SwizzleInfoPair(
        SwizzleSizeType(16, GL_UNSIGNED_NORMALIZED),
        SwizzleFormatInfo(DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R16G16B16A16_UNORM,
                          DXGI_FORMAT_R16G16B16A16_UNORM)));
    map.insert(SwizzleInfoPair(
        SwizzleSizeType(24, GL_UNSIGNED_NORMALIZED),
        SwizzleFormatInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,
                          DXGI_FORMAT_R32G32B32A32_FLOAT)));
    map.insert(SwizzleInfoPair(
        SwizzleSizeType(32, GL_UNSIGNED_NORMALIZED),
        SwizzleFormatInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,
                          DXGI_FORMAT_R32G32B32A32_FLOAT)));

    map.insert(
        SwizzleInfoPair(SwizzleSizeType(8, GL_SIGNED_NORMALIZED),
                        SwizzleFormatInfo(DXGI_FORMAT_R8G8B8A8_SNORM, DXGI_FORMAT_R8G8B8A8_SNORM,
                                          DXGI_FORMAT_R8G8B8A8_SNORM)));

    map.insert(SwizzleInfoPair(
        SwizzleSizeType(16, GL_FLOAT),
        SwizzleFormatInfo(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT,
                          DXGI_FORMAT_R16G16B16A16_FLOAT)));
    map.insert(SwizzleInfoPair(
        SwizzleSizeType(32, GL_FLOAT),
        SwizzleFormatInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,
                          DXGI_FORMAT_R32G32B32A32_FLOAT)));

    map.insert(
        SwizzleInfoPair(SwizzleSizeType(8, GL_UNSIGNED_INT),
                        SwizzleFormatInfo(DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT,
                                          DXGI_FORMAT_R8G8B8A8_UINT)));
    map.insert(SwizzleInfoPair(
        SwizzleSizeType(16, GL_UNSIGNED_INT),
        SwizzleFormatInfo(DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_R16G16B16A16_UINT,
                          DXGI_FORMAT_R16G16B16A16_UINT)));
    map.insert(SwizzleInfoPair(
        SwizzleSizeType(32, GL_UNSIGNED_INT),
        SwizzleFormatInfo(DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_UINT,
                          DXGI_FORMAT_R32G32B32A32_UINT)));

    map.insert(
        SwizzleInfoPair(SwizzleSizeType(8, GL_INT),
                        SwizzleFormatInfo(DXGI_FORMAT_R8G8B8A8_SINT, DXGI_FORMAT_R8G8B8A8_SINT,
                                          DXGI_FORMAT_R8G8B8A8_SINT)));
    map.insert(SwizzleInfoPair(
        SwizzleSizeType(16, GL_INT),
        SwizzleFormatInfo(DXGI_FORMAT_R16G16B16A16_SINT, DXGI_FORMAT_R16G16B16A16_SINT,
                          DXGI_FORMAT_R16G16B16A16_SINT)));
    map.insert(SwizzleInfoPair(
        SwizzleSizeType(32, GL_INT),
        SwizzleFormatInfo(DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_SINT,
                          DXGI_FORMAT_R32G32B32A32_SINT)));

    return map;
}

}  // namespace d3d11

}  // namespace rx
