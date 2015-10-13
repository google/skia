
//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// texture_format_table:
//   Queries for full textureFormat information based in internalFormat
//

#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"

#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/swizzle_format_info.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_util.h"
#include "libANGLE/renderer/d3d/loadimage.h"

namespace rx
{

namespace d3d11
{

namespace
{

typedef bool (*FormatSupportFunction)(const Renderer11DeviceCaps &);

bool AnyDevice(const Renderer11DeviceCaps &deviceCaps)
{
    return true;
}

bool OnlyFL10Plus(const Renderer11DeviceCaps &deviceCaps)
{
    return (deviceCaps.featureLevel >= D3D_FEATURE_LEVEL_10_0);
}

bool OnlyFL9_3(const Renderer11DeviceCaps &deviceCaps)
{
    return (deviceCaps.featureLevel == D3D_FEATURE_LEVEL_9_3);
}

template <DXGI_FORMAT format, bool requireSupport>
bool SupportsFormat(const Renderer11DeviceCaps &deviceCaps)
{
    // Must support texture, SRV and RTV support
    UINT mustSupport = D3D11_FORMAT_SUPPORT_TEXTURE2D | D3D11_FORMAT_SUPPORT_TEXTURECUBE |
                       D3D11_FORMAT_SUPPORT_SHADER_SAMPLE | D3D11_FORMAT_SUPPORT_MIP |
                       D3D11_FORMAT_SUPPORT_RENDER_TARGET;

    if (d3d11_gl::GetMaximumClientVersion(deviceCaps.featureLevel) > 2)
    {
        mustSupport |= D3D11_FORMAT_SUPPORT_TEXTURE3D;
    }

    bool fullSupport = false;
    if (format == DXGI_FORMAT_B5G6R5_UNORM)
    {
        // All hardware that supports DXGI_FORMAT_B5G6R5_UNORM should support autogen mipmaps, but
        // check anyway.
        mustSupport |= D3D11_FORMAT_SUPPORT_MIP_AUTOGEN;
        fullSupport = ((deviceCaps.B5G6R5support & mustSupport) == mustSupport);
    }
    else if (format == DXGI_FORMAT_B4G4R4A4_UNORM)
    {
        fullSupport = ((deviceCaps.B4G4R4A4support & mustSupport) == mustSupport);
    }
    else if (format == DXGI_FORMAT_B5G5R5A1_UNORM)
    {
        fullSupport = ((deviceCaps.B5G5R5A1support & mustSupport) == mustSupport);
    }
    else
    {
        UNREACHABLE();
        return false;
    }

    // This 'SupportsFormat' function is used by individual entries in the D3D11 Format Map below,
    // which maps GL formats to DXGI formats.
    if (requireSupport)
    {
        // This means that ANGLE would like to use the entry in the map if the inputted DXGI format
        // *IS* supported.
        // e.g. the entry might map GL_RGB5_A1 to DXGI_FORMAT_B5G5R5A1, which should only be used if
        // DXGI_FORMAT_B5G5R5A1 is supported.
        // In this case, we should only return 'true' if the format *IS* supported.
        return fullSupport;
    }
    else
    {
        // This means that ANGLE would like to use the entry in the map if the inputted DXGI format
        // *ISN'T* supported.
        // This might be a fallback entry. e.g. for ANGLE to use DXGI_FORMAT_R8G8B8A8_UNORM if
        // DXGI_FORMAT_B5G5R5A1 isn't supported.
        // In this case, we should only return 'true' if the format *ISN'T* supported.
        return !fullSupport;
    }
}

// End Format Support Functions

// For sized GL internal formats, there are several possible corresponding D3D11 formats depending
// on device capabilities.
// This map type allows querying for the DXGI texture formats to use for textures, SRVs, RTVs and
// DSVs given a GL internal format.
typedef std::pair<FormatSupportFunction, TextureFormat> TextureFormatWithSupportFunction;
typedef std::map<GLenum, std::vector<TextureFormatWithSupportFunction>> D3D11ES3FormatMap;

inline void InsertD3D11FormatInfo(D3D11ES3FormatMap *formatMap,
                                  GLenum internalFormat,
                                  DXGI_FORMAT texFormat,
                                  DXGI_FORMAT srvFormat,
                                  DXGI_FORMAT rtvFormat,
                                  DXGI_FORMAT dsvFormat,
                                  FormatSupportFunction formatSupportFunction)
{
    TextureFormat info;
    info.texFormat = texFormat;
    info.srvFormat = srvFormat;
    info.rtvFormat = rtvFormat;
    info.dsvFormat = dsvFormat;

    // Given a GL internal format, the renderFormat is the DSV format if it is depth- or
    // stencil-renderable,
    // the RTV format if it is color-renderable, and the (nonrenderable) texture format otherwise.
    if (dsvFormat != DXGI_FORMAT_UNKNOWN)
    {
        info.renderFormat = dsvFormat;
    }
    else if (rtvFormat != DXGI_FORMAT_UNKNOWN)
    {
        info.renderFormat = rtvFormat;
    }
    else if (texFormat != DXGI_FORMAT_UNKNOWN)
    {
        info.renderFormat = texFormat;
    }
    else
    {
        info.renderFormat = DXGI_FORMAT_UNKNOWN;
    }

    // Compute the swizzle formats
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(internalFormat);
    if (internalFormat != GL_NONE && formatInfo.pixelBytes > 0)
    {
        if (formatInfo.componentCount != 4 || texFormat == DXGI_FORMAT_UNKNOWN ||
            srvFormat == DXGI_FORMAT_UNKNOWN || rtvFormat == DXGI_FORMAT_UNKNOWN)
        {
            // Get the maximum sized component
            unsigned int maxBits = 1;
            if (formatInfo.compressed)
            {
                unsigned int compressedBitsPerBlock = formatInfo.pixelBytes * 8;
                unsigned int blockSize =
                    formatInfo.compressedBlockWidth * formatInfo.compressedBlockHeight;
                maxBits = std::max(compressedBitsPerBlock / blockSize, maxBits);
            }
            else
            {
                maxBits = std::max(maxBits, formatInfo.alphaBits);
                maxBits = std::max(maxBits, formatInfo.redBits);
                maxBits = std::max(maxBits, formatInfo.greenBits);
                maxBits = std::max(maxBits, formatInfo.blueBits);
                maxBits = std::max(maxBits, formatInfo.luminanceBits);
                maxBits = std::max(maxBits, formatInfo.depthBits);
            }

            maxBits = roundUp(maxBits, 8U);

            static const SwizzleInfoMap &swizzleMap = BuildSwizzleInfoMap();
            SwizzleInfoMap::const_iterator swizzleIter =
                swizzleMap.find(SwizzleSizeType(maxBits, formatInfo.componentType));
            ASSERT(swizzleIter != swizzleMap.end());

            const SwizzleFormatInfo &swizzleInfo = swizzleIter->second;
            info.swizzleTexFormat                = swizzleInfo.mTexFormat;
            info.swizzleSRVFormat                = swizzleInfo.mSRVFormat;
            info.swizzleRTVFormat                = swizzleInfo.mRTVFormat;
        }
        else
        {
            // The original texture format is suitable for swizzle operations
            info.swizzleTexFormat = texFormat;
            info.swizzleSRVFormat = srvFormat;
            info.swizzleRTVFormat = rtvFormat;
        }
    }
    else
    {
        // Not possible to swizzle with this texture format since it is either unsized or GL_NONE
        info.swizzleTexFormat = DXGI_FORMAT_UNKNOWN;
        info.swizzleSRVFormat = DXGI_FORMAT_UNKNOWN;
        info.swizzleRTVFormat = DXGI_FORMAT_UNKNOWN;
    }

    // Check if there is an initialization function for this texture format
    static const InternalFormatInitializerMap &initializerMap = BuildInternalFormatInitializerMap();
    auto initializerIter =
        initializerMap.find(InitializeTextureFormatPair(internalFormat, texFormat));
    info.dataInitializerFunction =
        (initializerIter != initializerMap.end()) ? initializerIter->second : NULL;

    // Gather all the load functions for this internal format
    static const D3D11LoadFunctionMap &loadFunctions = BuildD3D11LoadFunctionMap();
    auto loadFunctionIter = loadFunctions.find(internalFormat);
    if (loadFunctionIter != loadFunctions.end())
    {
        const std::vector<GLTypeDXGIFunctionPair> &loadFunctionVector = loadFunctionIter->second;
        for (size_t i = 0; i < loadFunctionVector.size(); i++)
        {
            DxgiFormatLoadFunctionPair formatFuncPair = loadFunctionVector[i].second;
            GLenum type                               = loadFunctionVector[i].first;
            DXGI_FORMAT dxgiFormat                    = formatFuncPair.first;
            rx::LoadImageFunction loadFunc            = formatFuncPair.second;

            if (dxgiFormat == texFormat || dxgiFormat == DXGI_FORMAT_UNKNOWN)
            {
                info.loadFunctions.insert(std::make_pair(type, loadFunc));
            }
        }
    }

    ASSERT(info.loadFunctions.size() != 0 || internalFormat == GL_NONE);

    (*formatMap)[internalFormat].push_back(std::make_pair(formatSupportFunction, info));
}

const D3D11ES3FormatMap &BuildD3D11FormatMap()
{
    static D3D11ES3FormatMap map;

    // clang-format off
    //                         | GL internal format                          | D3D11 texture format           | D3D11 SRV format                    | D3D11 RTV format               | D3D11 DSV format                | Requirements
    InsertD3D11FormatInfo(&map, GL_NONE,                                      DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_R8,                                        DXGI_FORMAT_R8_UNORM,            DXGI_FORMAT_R8_UNORM,                 DXGI_FORMAT_R8_UNORM,            DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_R8_SNORM,                                  DXGI_FORMAT_R8_SNORM,            DXGI_FORMAT_R8_SNORM,                 DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RG8,                                       DXGI_FORMAT_R8G8_UNORM,          DXGI_FORMAT_R8G8_UNORM,               DXGI_FORMAT_R8G8_UNORM,          DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RG8_SNORM,                                 DXGI_FORMAT_R8G8_SNORM,          DXGI_FORMAT_R8G8_SNORM,               DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB8,                                      DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB8_SNORM,                                DXGI_FORMAT_R8G8B8A8_SNORM,      DXGI_FORMAT_R8G8B8A8_SNORM,           DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB565,                                    DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              SupportsFormat<DXGI_FORMAT_B5G6R5_UNORM, false>);
    InsertD3D11FormatInfo(&map, GL_RGB565,                                    DXGI_FORMAT_B5G6R5_UNORM,        DXGI_FORMAT_B5G6R5_UNORM,             DXGI_FORMAT_B5G6R5_UNORM,        DXGI_FORMAT_UNKNOWN,              SupportsFormat<DXGI_FORMAT_B5G6R5_UNORM, true>);
    InsertD3D11FormatInfo(&map, GL_RGBA4,                                     DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              SupportsFormat<DXGI_FORMAT_B4G4R4A4_UNORM, false>);
    InsertD3D11FormatInfo(&map, GL_RGBA4,                                     DXGI_FORMAT_B4G4R4A4_UNORM,      DXGI_FORMAT_B4G4R4A4_UNORM,           DXGI_FORMAT_B4G4R4A4_UNORM,      DXGI_FORMAT_UNKNOWN,              SupportsFormat<DXGI_FORMAT_B4G4R4A4_UNORM, true>);
    InsertD3D11FormatInfo(&map, GL_RGB5_A1,                                   DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              SupportsFormat<DXGI_FORMAT_B5G5R5A1_UNORM, false>);
    InsertD3D11FormatInfo(&map, GL_RGB5_A1,                                   DXGI_FORMAT_B5G5R5A1_UNORM,      DXGI_FORMAT_B5G5R5A1_UNORM,           DXGI_FORMAT_B5G5R5A1_UNORM,      DXGI_FORMAT_UNKNOWN,              SupportsFormat<DXGI_FORMAT_B5G5R5A1_UNORM, true>);
    InsertD3D11FormatInfo(&map, GL_RGBA8,                                     DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGBA8_SNORM,                               DXGI_FORMAT_R8G8B8A8_SNORM,      DXGI_FORMAT_R8G8B8A8_SNORM,           DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB10_A2,                                  DXGI_FORMAT_R10G10B10A2_UNORM,   DXGI_FORMAT_R10G10B10A2_UNORM,        DXGI_FORMAT_R10G10B10A2_UNORM,   DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB10_A2UI,                                DXGI_FORMAT_R10G10B10A2_UINT,    DXGI_FORMAT_R10G10B10A2_UINT,         DXGI_FORMAT_R10G10B10A2_UINT,    DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_SRGB8,                                     DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,      DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_SRGB8_ALPHA8,                              DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,      DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_R16F,                                      DXGI_FORMAT_R16_FLOAT,           DXGI_FORMAT_R16_FLOAT,                DXGI_FORMAT_R16_FLOAT,           DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RG16F,                                     DXGI_FORMAT_R16G16_FLOAT,        DXGI_FORMAT_R16G16_FLOAT,             DXGI_FORMAT_R16G16_FLOAT,        DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB16F,                                    DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_R16G16B16A16_FLOAT,       DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGBA16F,                                   DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_R16G16B16A16_FLOAT,       DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_R32F,                                      DXGI_FORMAT_R32_FLOAT,           DXGI_FORMAT_R32_FLOAT,                DXGI_FORMAT_R32_FLOAT,           DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RG32F,                                     DXGI_FORMAT_R32G32_FLOAT,        DXGI_FORMAT_R32G32_FLOAT,             DXGI_FORMAT_R32G32_FLOAT,        DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB32F,                                    DXGI_FORMAT_R32G32B32A32_FLOAT,  DXGI_FORMAT_R32G32B32A32_FLOAT,       DXGI_FORMAT_R32G32B32A32_FLOAT,  DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGBA32F,                                   DXGI_FORMAT_R32G32B32A32_FLOAT,  DXGI_FORMAT_R32G32B32A32_FLOAT,       DXGI_FORMAT_R32G32B32A32_FLOAT,  DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_R11F_G11F_B10F,                            DXGI_FORMAT_R11G11B10_FLOAT,     DXGI_FORMAT_R11G11B10_FLOAT,          DXGI_FORMAT_R11G11B10_FLOAT,     DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB9_E5,                                   DXGI_FORMAT_R9G9B9E5_SHAREDEXP,  DXGI_FORMAT_R9G9B9E5_SHAREDEXP,       DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_R8I,                                       DXGI_FORMAT_R8_SINT,             DXGI_FORMAT_R8_SINT,                  DXGI_FORMAT_R8_SINT,             DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_R8UI,                                      DXGI_FORMAT_R8_UINT,             DXGI_FORMAT_R8_UINT,                  DXGI_FORMAT_R8_UINT,             DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_R16I,                                      DXGI_FORMAT_R16_SINT,            DXGI_FORMAT_R16_SINT,                 DXGI_FORMAT_R16_SINT,            DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_R16UI,                                     DXGI_FORMAT_R16_UINT,            DXGI_FORMAT_R16_UINT,                 DXGI_FORMAT_R16_UINT,            DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_R32I,                                      DXGI_FORMAT_R32_SINT,            DXGI_FORMAT_R32_SINT,                 DXGI_FORMAT_R32_SINT,            DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_R32UI,                                     DXGI_FORMAT_R32_UINT,            DXGI_FORMAT_R32_UINT,                 DXGI_FORMAT_R32_UINT,            DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RG8I,                                      DXGI_FORMAT_R8G8_SINT,           DXGI_FORMAT_R8G8_SINT,                DXGI_FORMAT_R8G8_SINT,           DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RG8UI,                                     DXGI_FORMAT_R8G8_UINT,           DXGI_FORMAT_R8G8_UINT,                DXGI_FORMAT_R8G8_UINT,           DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RG16I,                                     DXGI_FORMAT_R16G16_SINT,         DXGI_FORMAT_R16G16_SINT,              DXGI_FORMAT_R16G16_SINT,         DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RG16UI,                                    DXGI_FORMAT_R16G16_UINT,         DXGI_FORMAT_R16G16_UINT,              DXGI_FORMAT_R16G16_UINT,         DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RG32I,                                     DXGI_FORMAT_R32G32_SINT,         DXGI_FORMAT_R32G32_SINT,              DXGI_FORMAT_R32G32_SINT,         DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RG32UI,                                    DXGI_FORMAT_R32G32_UINT,         DXGI_FORMAT_R32G32_UINT,              DXGI_FORMAT_R32G32_UINT,         DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB8I,                                     DXGI_FORMAT_R8G8B8A8_SINT,       DXGI_FORMAT_R8G8B8A8_SINT,            DXGI_FORMAT_R8G8B8A8_SINT,       DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB8UI,                                    DXGI_FORMAT_R8G8B8A8_UINT,       DXGI_FORMAT_R8G8B8A8_UINT,            DXGI_FORMAT_R8G8B8A8_UINT,       DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB16I,                                    DXGI_FORMAT_R16G16B16A16_SINT,   DXGI_FORMAT_R16G16B16A16_SINT,        DXGI_FORMAT_R16G16B16A16_SINT,   DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB16UI,                                   DXGI_FORMAT_R16G16B16A16_UINT,   DXGI_FORMAT_R16G16B16A16_UINT,        DXGI_FORMAT_R16G16B16A16_UINT,   DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB32I,                                    DXGI_FORMAT_R32G32B32A32_SINT,   DXGI_FORMAT_R32G32B32A32_SINT,        DXGI_FORMAT_R32G32B32A32_SINT,   DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB32UI,                                   DXGI_FORMAT_R32G32B32A32_UINT,   DXGI_FORMAT_R32G32B32A32_UINT,        DXGI_FORMAT_R32G32B32A32_UINT,   DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGBA8I,                                    DXGI_FORMAT_R8G8B8A8_SINT,       DXGI_FORMAT_R8G8B8A8_SINT,            DXGI_FORMAT_R8G8B8A8_SINT,       DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGBA8UI,                                   DXGI_FORMAT_R8G8B8A8_UINT,       DXGI_FORMAT_R8G8B8A8_UINT,            DXGI_FORMAT_R8G8B8A8_UINT,       DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGBA16I,                                   DXGI_FORMAT_R16G16B16A16_SINT,   DXGI_FORMAT_R16G16B16A16_SINT,        DXGI_FORMAT_R16G16B16A16_SINT,   DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGBA16UI,                                  DXGI_FORMAT_R16G16B16A16_UINT,   DXGI_FORMAT_R16G16B16A16_UINT,        DXGI_FORMAT_R16G16B16A16_UINT,   DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGBA32I,                                   DXGI_FORMAT_R32G32B32A32_SINT,   DXGI_FORMAT_R32G32B32A32_SINT,        DXGI_FORMAT_R32G32B32A32_SINT,   DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGBA32UI,                                  DXGI_FORMAT_R32G32B32A32_UINT,   DXGI_FORMAT_R32G32B32A32_UINT,        DXGI_FORMAT_R32G32B32A32_UINT,   DXGI_FORMAT_UNKNOWN,              AnyDevice);

    // Unsized formats, TODO: Are types of float and half float allowed for the unsized types? Would
    // it change the DXGI format?
    InsertD3D11FormatInfo(&map, GL_ALPHA,                                     DXGI_FORMAT_A8_UNORM,            DXGI_FORMAT_A8_UNORM,                 DXGI_FORMAT_A8_UNORM,            DXGI_FORMAT_UNKNOWN,              OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_ALPHA,                                     DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              OnlyFL9_3);
    InsertD3D11FormatInfo(&map, GL_LUMINANCE,                                 DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_LUMINANCE_ALPHA,                           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGB,                                       DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_RGBA,                                      DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_BGRA_EXT,                                  DXGI_FORMAT_B8G8R8A8_UNORM,      DXGI_FORMAT_B8G8R8A8_UNORM,           DXGI_FORMAT_B8G8R8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              AnyDevice);

    // From GL_EXT_texture_storage
    //                           | GL internal format                         | D3D11 texture format          | D3D11 SRV format                    | D3D11 RTV format                | D3D11 DSV format               | Requirements
    InsertD3D11FormatInfo(&map, GL_ALPHA8_EXT,                                DXGI_FORMAT_A8_UNORM,            DXGI_FORMAT_A8_UNORM,                 DXGI_FORMAT_A8_UNORM,            DXGI_FORMAT_UNKNOWN,              OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_ALPHA8_EXT,                                DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              OnlyFL9_3);
    InsertD3D11FormatInfo(&map, GL_LUMINANCE8_EXT,                            DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_ALPHA32F_EXT,                              DXGI_FORMAT_R32G32B32A32_FLOAT,  DXGI_FORMAT_R32G32B32A32_FLOAT,       DXGI_FORMAT_R32G32B32A32_FLOAT,  DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_LUMINANCE32F_EXT,                          DXGI_FORMAT_R32G32B32A32_FLOAT,  DXGI_FORMAT_R32G32B32A32_FLOAT,       DXGI_FORMAT_R32G32B32A32_FLOAT,  DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_ALPHA16F_EXT,                              DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_R16G16B16A16_FLOAT,       DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_LUMINANCE16F_EXT,                          DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_R16G16B16A16_FLOAT,       DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_LUMINANCE8_ALPHA8_EXT,                     DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,           DXGI_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_LUMINANCE_ALPHA32F_EXT,                    DXGI_FORMAT_R32G32B32A32_FLOAT,  DXGI_FORMAT_R32G32B32A32_FLOAT,       DXGI_FORMAT_R32G32B32A32_FLOAT,  DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_LUMINANCE_ALPHA16F_EXT,                    DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_R16G16B16A16_FLOAT,       DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_BGRA8_EXT,                                 DXGI_FORMAT_B8G8R8A8_UNORM,      DXGI_FORMAT_B8G8R8A8_UNORM,           DXGI_FORMAT_B8G8R8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_BGRA4_ANGLEX,                              DXGI_FORMAT_B8G8R8A8_UNORM,      DXGI_FORMAT_B8G8R8A8_UNORM,           DXGI_FORMAT_B8G8R8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_BGR5_A1_ANGLEX,                            DXGI_FORMAT_B8G8R8A8_UNORM,      DXGI_FORMAT_B8G8R8A8_UNORM,           DXGI_FORMAT_B8G8R8A8_UNORM,      DXGI_FORMAT_UNKNOWN,              AnyDevice);

    // Depth stencil formats
    InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT16,                         DXGI_FORMAT_R16_TYPELESS,        DXGI_FORMAT_R16_UNORM,                DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_D16_UNORM,            OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT16,                         DXGI_FORMAT_D16_UNORM,           DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_D16_UNORM,            OnlyFL9_3);
    InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT24,                         DXGI_FORMAT_R24G8_TYPELESS,      DXGI_FORMAT_R24_UNORM_X8_TYPELESS,    DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_D24_UNORM_S8_UINT,    OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT24,                         DXGI_FORMAT_D24_UNORM_S8_UINT,   DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_D24_UNORM_S8_UINT,    OnlyFL9_3);
    InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT32F,                        DXGI_FORMAT_R32_TYPELESS,        DXGI_FORMAT_R32_FLOAT,                DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_D32_FLOAT,            OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT32F,                        DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              OnlyFL9_3);
    InsertD3D11FormatInfo(&map, GL_DEPTH24_STENCIL8,                          DXGI_FORMAT_R24G8_TYPELESS,      DXGI_FORMAT_R24_UNORM_X8_TYPELESS,    DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_D24_UNORM_S8_UINT,    OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_DEPTH24_STENCIL8,                          DXGI_FORMAT_D24_UNORM_S8_UINT,   DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_D24_UNORM_S8_UINT,    OnlyFL9_3);
    InsertD3D11FormatInfo(&map, GL_DEPTH32F_STENCIL8,                         DXGI_FORMAT_R32G8X24_TYPELESS,   DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_D32_FLOAT_S8X24_UINT, OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_DEPTH32F_STENCIL8,                         DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              OnlyFL9_3);
    InsertD3D11FormatInfo(&map, GL_STENCIL_INDEX8,                            DXGI_FORMAT_R24G8_TYPELESS,      DXGI_FORMAT_X24_TYPELESS_G8_UINT,     DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_D24_UNORM_S8_UINT,    OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_STENCIL_INDEX8,                            DXGI_FORMAT_D24_UNORM_S8_UINT,   DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_D24_UNORM_S8_UINT,    OnlyFL9_3);

    // From GL_ANGLE_depth_texture
    // Since D3D11 doesn't have a D32_UNORM format, use D24S8 which has comparable precision and
    // matches the ES3 format.
    InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT32_OES,                     DXGI_FORMAT_R24G8_TYPELESS,      DXGI_FORMAT_R24_UNORM_X8_TYPELESS,    DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_D24_UNORM_S8_UINT,    OnlyFL10Plus);

    // Compressed formats, From ES 3.0.1 spec, table 3.16
    //                           | GL internal format                        | D3D11 texture format           | D3D11 SRV format                    | D3D11 RTV format               | D3D11 DSV format                | Requirements
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_R11_EAC,                        DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_SIGNED_R11_EAC,                 DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_RG11_EAC,                       DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_SIGNED_RG11_EAC,                DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGB8_ETC2,                      DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_SRGB8_ETC2,                     DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGBA8_ETC2_EAC,                 DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              OnlyFL10Plus);
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,          DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,                  DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              OnlyFL10Plus);

    // From GL_EXT_texture_compression_dxt1
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,              DXGI_FORMAT_BC1_UNORM,           DXGI_FORMAT_BC1_UNORM,                DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              AnyDevice);
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,             DXGI_FORMAT_BC1_UNORM,           DXGI_FORMAT_BC1_UNORM,                DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              AnyDevice);

    // From GL_ANGLE_texture_compression_dxt3
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE,           DXGI_FORMAT_BC2_UNORM,           DXGI_FORMAT_BC2_UNORM,                DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              AnyDevice);

    // From GL_ANGLE_texture_compression_dxt5
    InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE,           DXGI_FORMAT_BC3_UNORM,           DXGI_FORMAT_BC3_UNORM,                DXGI_FORMAT_UNKNOWN,             DXGI_FORMAT_UNKNOWN,              AnyDevice);
    // clang-format on

    return map;
}

}  // namespace

TextureFormat::TextureFormat()
    : texFormat(DXGI_FORMAT_UNKNOWN),
      srvFormat(DXGI_FORMAT_UNKNOWN),
      rtvFormat(DXGI_FORMAT_UNKNOWN),
      dsvFormat(DXGI_FORMAT_UNKNOWN),
      renderFormat(DXGI_FORMAT_UNKNOWN),
      swizzleTexFormat(DXGI_FORMAT_UNKNOWN),
      swizzleSRVFormat(DXGI_FORMAT_UNKNOWN),
      swizzleRTVFormat(DXGI_FORMAT_UNKNOWN),
      dataInitializerFunction(NULL),
      loadFunctions()
{
}

const TextureFormat &GetTextureFormatInfo(GLenum internalFormat,
                                          const Renderer11DeviceCaps &renderer11DeviceCaps)
{
    static const D3D11ES3FormatMap &formatMap = BuildD3D11FormatMap();

    D3D11ES3FormatMap::const_iterator iter = formatMap.find(internalFormat);
    if (iter != formatMap.end())
    {
        const std::vector<TextureFormatWithSupportFunction> &formatVector = iter->second;
        for (size_t i = 0; i < formatVector.size(); i++)
        {
            const FormatSupportFunction supportFunction = formatVector[i].first;
            const TextureFormat &textureFormat          = formatVector[i].second;

            if (supportFunction(renderer11DeviceCaps))
            {
                return textureFormat;
            }
        }
    }

    static const TextureFormat defaultInfo;
    return defaultInfo;
}

}  // namespace d3d11

}  // namespace rx
