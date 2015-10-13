//
// Copyright (c) 2013-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutils.cpp: Queries for GL image formats.

#include "common/mathutil.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/renderer/Renderer.h"

namespace gl
{

// ES2 requires that format is equal to internal format at all glTex*Image2D entry points and the implementation
// can decide the true, sized, internal format. The ES2FormatMap determines the internal format for all valid
// format and type combinations.

typedef std::pair<GLenum, GLenum> FormatTypePair;
typedef std::pair<FormatTypePair, GLenum> FormatPair;
typedef std::map<FormatTypePair, GLenum> FormatMap;

// A helper function to insert data into the format map with fewer characters.
static inline void InsertFormatMapping(FormatMap *map, GLenum format, GLenum type, GLenum internalFormat)
{
    map->insert(FormatPair(FormatTypePair(format, type), internalFormat));
}

FormatMap BuildFormatMap()
{
    FormatMap map;

    //                       | Format               | Type                             | Internal format          |
    InsertFormatMapping(&map, GL_RGBA,               GL_UNSIGNED_BYTE,                  GL_RGBA8);
    InsertFormatMapping(&map, GL_RGBA,               GL_BYTE,                           GL_RGBA8_SNORM);
    InsertFormatMapping(&map, GL_RGBA,               GL_UNSIGNED_SHORT_4_4_4_4,         GL_RGBA4);
    InsertFormatMapping(&map, GL_RGBA,               GL_UNSIGNED_SHORT_5_5_5_1,         GL_RGB5_A1);
    InsertFormatMapping(&map, GL_RGBA,               GL_UNSIGNED_INT_2_10_10_10_REV,    GL_RGB10_A2);
    InsertFormatMapping(&map, GL_RGBA,               GL_FLOAT,                          GL_RGBA32F);
    InsertFormatMapping(&map, GL_RGBA,               GL_HALF_FLOAT,                     GL_RGBA16F);
    InsertFormatMapping(&map, GL_RGBA,               GL_HALF_FLOAT_OES,                 GL_RGBA16F);

    InsertFormatMapping(&map, GL_RGBA_INTEGER,       GL_UNSIGNED_BYTE,                  GL_RGBA8UI);
    InsertFormatMapping(&map, GL_RGBA_INTEGER,       GL_BYTE,                           GL_RGBA8I);
    InsertFormatMapping(&map, GL_RGBA_INTEGER,       GL_UNSIGNED_SHORT,                 GL_RGBA16UI);
    InsertFormatMapping(&map, GL_RGBA_INTEGER,       GL_SHORT,                          GL_RGBA16I);
    InsertFormatMapping(&map, GL_RGBA_INTEGER,       GL_UNSIGNED_INT,                   GL_RGBA32UI);
    InsertFormatMapping(&map, GL_RGBA_INTEGER,       GL_INT,                            GL_RGBA32I);
    InsertFormatMapping(&map, GL_RGBA_INTEGER,       GL_UNSIGNED_INT_2_10_10_10_REV,    GL_RGB10_A2UI);

    InsertFormatMapping(&map, GL_RGB,                GL_UNSIGNED_BYTE,                  GL_RGB8);
    InsertFormatMapping(&map, GL_RGB,                GL_BYTE,                           GL_RGB8_SNORM);
    InsertFormatMapping(&map, GL_RGB,                GL_UNSIGNED_SHORT_5_6_5,           GL_RGB565);
    InsertFormatMapping(&map, GL_RGB,                GL_UNSIGNED_INT_10F_11F_11F_REV,   GL_R11F_G11F_B10F);
    InsertFormatMapping(&map, GL_RGB,                GL_UNSIGNED_INT_5_9_9_9_REV,       GL_RGB9_E5);
    InsertFormatMapping(&map, GL_RGB,                GL_FLOAT,                          GL_RGB32F);
    InsertFormatMapping(&map, GL_RGB,                GL_HALF_FLOAT,                     GL_RGB16F);
    InsertFormatMapping(&map, GL_RGB,                GL_HALF_FLOAT_OES,                 GL_RGB16F);

    InsertFormatMapping(&map, GL_RGB_INTEGER,        GL_UNSIGNED_BYTE,                  GL_RGB8UI);
    InsertFormatMapping(&map, GL_RGB_INTEGER,        GL_BYTE,                           GL_RGB8I);
    InsertFormatMapping(&map, GL_RGB_INTEGER,        GL_UNSIGNED_SHORT,                 GL_RGB16UI);
    InsertFormatMapping(&map, GL_RGB_INTEGER,        GL_SHORT,                          GL_RGB16I);
    InsertFormatMapping(&map, GL_RGB_INTEGER,        GL_UNSIGNED_INT,                   GL_RGB32UI);
    InsertFormatMapping(&map, GL_RGB_INTEGER,        GL_INT,                            GL_RGB32I);

    InsertFormatMapping(&map, GL_RG,                 GL_UNSIGNED_BYTE,                  GL_RG8);
    InsertFormatMapping(&map, GL_RG,                 GL_BYTE,                           GL_RG8_SNORM);
    InsertFormatMapping(&map, GL_RG,                 GL_FLOAT,                          GL_RG32F);
    InsertFormatMapping(&map, GL_RG,                 GL_HALF_FLOAT,                     GL_RG16F);
    InsertFormatMapping(&map, GL_RG,                 GL_HALF_FLOAT_OES,                 GL_RG16F);

    InsertFormatMapping(&map, GL_RG_INTEGER,         GL_UNSIGNED_BYTE,                  GL_RG8UI);
    InsertFormatMapping(&map, GL_RG_INTEGER,         GL_BYTE,                           GL_RG8I);
    InsertFormatMapping(&map, GL_RG_INTEGER,         GL_UNSIGNED_SHORT,                 GL_RG16UI);
    InsertFormatMapping(&map, GL_RG_INTEGER,         GL_SHORT,                          GL_RG16I);
    InsertFormatMapping(&map, GL_RG_INTEGER,         GL_UNSIGNED_INT,                   GL_RG32UI);
    InsertFormatMapping(&map, GL_RG_INTEGER,         GL_INT,                            GL_RG32I);

    InsertFormatMapping(&map, GL_RED,                GL_UNSIGNED_BYTE,                  GL_R8);
    InsertFormatMapping(&map, GL_RED,                GL_BYTE,                           GL_R8_SNORM);
    InsertFormatMapping(&map, GL_RED,                GL_FLOAT,                          GL_R32F);
    InsertFormatMapping(&map, GL_RED,                GL_HALF_FLOAT,                     GL_R16F);
    InsertFormatMapping(&map, GL_RED,                GL_HALF_FLOAT_OES,                 GL_R16F);

    InsertFormatMapping(&map, GL_RED_INTEGER,        GL_UNSIGNED_BYTE,                  GL_R8UI);
    InsertFormatMapping(&map, GL_RED_INTEGER,        GL_BYTE,                           GL_R8I);
    InsertFormatMapping(&map, GL_RED_INTEGER,        GL_UNSIGNED_SHORT,                 GL_R16UI);
    InsertFormatMapping(&map, GL_RED_INTEGER,        GL_SHORT,                          GL_R16I);
    InsertFormatMapping(&map, GL_RED_INTEGER,        GL_UNSIGNED_INT,                   GL_R32UI);
    InsertFormatMapping(&map, GL_RED_INTEGER,        GL_INT,                            GL_R32I);

    InsertFormatMapping(&map, GL_LUMINANCE_ALPHA,    GL_UNSIGNED_BYTE,                  GL_LUMINANCE8_ALPHA8_EXT);
    InsertFormatMapping(&map, GL_LUMINANCE,          GL_UNSIGNED_BYTE,                  GL_LUMINANCE8_EXT);
    InsertFormatMapping(&map, GL_ALPHA,              GL_UNSIGNED_BYTE,                  GL_ALPHA8_EXT);
    InsertFormatMapping(&map, GL_LUMINANCE_ALPHA,    GL_FLOAT,                          GL_LUMINANCE_ALPHA32F_EXT);
    InsertFormatMapping(&map, GL_LUMINANCE,          GL_FLOAT,                          GL_LUMINANCE32F_EXT);
    InsertFormatMapping(&map, GL_ALPHA,              GL_FLOAT,                          GL_ALPHA32F_EXT);
    InsertFormatMapping(&map, GL_LUMINANCE_ALPHA,    GL_HALF_FLOAT,                     GL_LUMINANCE_ALPHA16F_EXT);
    InsertFormatMapping(&map, GL_LUMINANCE_ALPHA,    GL_HALF_FLOAT_OES,                 GL_LUMINANCE_ALPHA16F_EXT);
    InsertFormatMapping(&map, GL_LUMINANCE,          GL_HALF_FLOAT,                     GL_LUMINANCE16F_EXT);
    InsertFormatMapping(&map, GL_LUMINANCE,          GL_HALF_FLOAT_OES,                 GL_LUMINANCE16F_EXT);
    InsertFormatMapping(&map, GL_ALPHA,              GL_HALF_FLOAT,                     GL_ALPHA16F_EXT);
    InsertFormatMapping(&map, GL_ALPHA,              GL_HALF_FLOAT_OES,                 GL_ALPHA16F_EXT);

    InsertFormatMapping(&map, GL_BGRA_EXT,           GL_UNSIGNED_BYTE,                  GL_BGRA8_EXT);
    InsertFormatMapping(&map, GL_BGRA_EXT,           GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT, GL_BGRA4_ANGLEX);
    InsertFormatMapping(&map, GL_BGRA_EXT,           GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT, GL_BGR5_A1_ANGLEX);

    InsertFormatMapping(&map, GL_SRGB_EXT,           GL_UNSIGNED_BYTE,                  GL_SRGB8);
    InsertFormatMapping(&map, GL_SRGB_ALPHA_EXT,     GL_UNSIGNED_BYTE,                  GL_SRGB8_ALPHA8);

    InsertFormatMapping(&map, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,    GL_UNSIGNED_BYTE,     GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
    InsertFormatMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   GL_UNSIGNED_BYTE,     GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
    InsertFormatMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE, GL_UNSIGNED_BYTE,     GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE);
    InsertFormatMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE, GL_UNSIGNED_BYTE,     GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE);

    InsertFormatMapping(&map, GL_DEPTH_COMPONENT,    GL_UNSIGNED_SHORT,                 GL_DEPTH_COMPONENT16);
    InsertFormatMapping(&map, GL_DEPTH_COMPONENT,    GL_UNSIGNED_INT,                   GL_DEPTH_COMPONENT32_OES);
    InsertFormatMapping(&map, GL_DEPTH_COMPONENT,    GL_FLOAT,                          GL_DEPTH_COMPONENT32F);

    InsertFormatMapping(&map, GL_STENCIL,            GL_UNSIGNED_BYTE,                  GL_STENCIL_INDEX8);

    InsertFormatMapping(&map, GL_DEPTH_STENCIL,      GL_UNSIGNED_INT_24_8,              GL_DEPTH24_STENCIL8);
    InsertFormatMapping(&map, GL_DEPTH_STENCIL,      GL_FLOAT_32_UNSIGNED_INT_24_8_REV, GL_DEPTH32F_STENCIL8);

    return map;
}

Type::Type()
    : bytes(0),
      bytesShift(0),
      specialInterpretation(false)
{
}

static Type GenTypeInfo(GLuint bytes, bool specialInterpretation)
{
    Type info;
    info.bytes = bytes;
    GLuint i = 0;
    while ((1u << i) < bytes)
    {
        ++i;
    }
    info.bytesShift = i;
    ASSERT((1u << info.bytesShift) == bytes);
    info.specialInterpretation = specialInterpretation;
    return info;
}

bool operator<(const Type& a, const Type& b)
{
    return memcmp(&a, &b, sizeof(Type)) < 0;
}

// Information about internal formats
static bool AlwaysSupported(GLuint, const Extensions &)
{
    return true;
}

static bool NeverSupported(GLuint, const Extensions &)
{
    return false;
}

template <GLuint minCoreGLVersion>
static bool RequireES(GLuint clientVersion, const Extensions &)
{
    return clientVersion >= minCoreGLVersion;
}

// Pointer to a boolean memeber of the Extensions struct
typedef bool(Extensions::*ExtensionBool);

// Check support for a single extension
template <ExtensionBool bool1>
static bool RequireExt(GLuint, const Extensions & extensions)
{
    return extensions.*bool1;
}

// Check for a minimum client version or a single extension
template <GLuint minCoreGLVersion, ExtensionBool bool1>
static bool RequireESOrExt(GLuint clientVersion, const Extensions &extensions)
{
    return clientVersion >= minCoreGLVersion || extensions.*bool1;
}

// Check for a minimum client version or two extensions
template <GLuint minCoreGLVersion, ExtensionBool bool1, ExtensionBool bool2>
static bool RequireESOrExtAndExt(GLuint clientVersion, const Extensions &extensions)
{
    return clientVersion >= minCoreGLVersion || (extensions.*bool1 && extensions.*bool2);
}

// Check for a minimum client version or at least one of two extensions
template <GLuint minCoreGLVersion, ExtensionBool bool1, ExtensionBool bool2>
static bool RequireESOrExtOrExt(GLuint clientVersion, const Extensions &extensions)
{
    return clientVersion >= minCoreGLVersion || extensions.*bool1 || extensions.*bool2;
}

// Check support for two extensions
template <ExtensionBool bool1, ExtensionBool bool2>
static bool RequireExtAndExt(GLuint, const Extensions &extensions)
{
    return extensions.*bool1 && extensions.*bool2;
}

InternalFormat::InternalFormat()
    : redBits(0),
      greenBits(0),
      blueBits(0),
      luminanceBits(0),
      alphaBits(0),
      sharedBits(0),
      depthBits(0),
      stencilBits(0),
      pixelBytes(0),
      componentCount(0),
      compressed(false),
      compressedBlockWidth(0),
      compressedBlockHeight(0),
      format(GL_NONE),
      type(GL_NONE),
      componentType(GL_NONE),
      colorEncoding(GL_NONE),
      textureSupport(NeverSupported),
      renderSupport(NeverSupported),
      filterSupport(NeverSupported)
{
}

static InternalFormat UnsizedFormat(GLenum format, InternalFormat::SupportCheckFunction textureSupport,
                                    InternalFormat::SupportCheckFunction renderSupport,
                                    InternalFormat::SupportCheckFunction filterSupport)
{
    InternalFormat formatInfo;
    formatInfo.format = format;
    formatInfo.textureSupport = textureSupport;
    formatInfo.renderSupport = renderSupport;
    formatInfo.filterSupport = filterSupport;
    return formatInfo;
}

static InternalFormat RGBAFormat(GLuint red, GLuint green, GLuint blue, GLuint alpha, GLuint shared,
                                 GLenum format, GLenum type, GLenum componentType, bool srgb,
                                 InternalFormat::SupportCheckFunction textureSupport,
                                 InternalFormat::SupportCheckFunction renderSupport,
                                 InternalFormat::SupportCheckFunction filterSupport)
{
    InternalFormat formatInfo;
    formatInfo.redBits = red;
    formatInfo.greenBits = green;
    formatInfo.blueBits = blue;
    formatInfo.alphaBits = alpha;
    formatInfo.sharedBits = shared;
    formatInfo.pixelBytes = (red + green + blue + alpha + shared) / 8;
    formatInfo.componentCount = ((red > 0) ? 1 : 0) + ((green > 0) ? 1 : 0) + ((blue > 0) ? 1 : 0) + ((alpha > 0) ? 1 : 0);
    formatInfo.format = format;
    formatInfo.type = type;
    formatInfo.componentType = componentType;
    formatInfo.colorEncoding = (srgb ? GL_SRGB : GL_LINEAR);
    formatInfo.textureSupport = textureSupport;
    formatInfo.renderSupport = renderSupport;
    formatInfo.filterSupport = filterSupport;
    return formatInfo;
}

static InternalFormat LUMAFormat(GLuint luminance, GLuint alpha, GLenum format, GLenum type, GLenum componentType,
                                 InternalFormat::SupportCheckFunction textureSupport,
                                 InternalFormat::SupportCheckFunction renderSupport,
                                 InternalFormat::SupportCheckFunction filterSupport)
{
    InternalFormat formatInfo;
    formatInfo.luminanceBits = luminance;
    formatInfo.alphaBits = alpha;
    formatInfo.pixelBytes = (luminance + alpha) / 8;
    formatInfo.componentCount = ((luminance > 0) ? 1 : 0) + ((alpha > 0) ? 1 : 0);
    formatInfo.format = format;
    formatInfo.type = type;
    formatInfo.componentType = componentType;
    formatInfo.colorEncoding = GL_LINEAR;
    formatInfo.textureSupport = textureSupport;
    formatInfo.renderSupport = renderSupport;
    formatInfo.filterSupport = filterSupport;
    return formatInfo;
}

static InternalFormat DepthStencilFormat(GLuint depthBits, GLuint stencilBits, GLuint unusedBits, GLenum format,
                                         GLenum type, GLenum componentType, InternalFormat::SupportCheckFunction textureSupport,
                                         InternalFormat::SupportCheckFunction renderSupport,
                                         InternalFormat::SupportCheckFunction filterSupport)
{
    InternalFormat formatInfo;
    formatInfo.depthBits = depthBits;
    formatInfo.stencilBits = stencilBits;
    formatInfo.pixelBytes = (depthBits + stencilBits + unusedBits) / 8;
    formatInfo.componentCount = ((depthBits > 0) ? 1 : 0) + ((stencilBits > 0) ? 1 : 0);
    formatInfo.format = format;
    formatInfo.type = type;
    formatInfo.componentType = componentType;
    formatInfo.colorEncoding = GL_LINEAR;
    formatInfo.textureSupport = textureSupport;
    formatInfo.renderSupport = renderSupport;
    formatInfo.filterSupport = filterSupport;
    return formatInfo;
}

static InternalFormat CompressedFormat(GLuint compressedBlockWidth, GLuint compressedBlockHeight, GLuint compressedBlockSize,
                                       GLuint componentCount, GLenum format, GLenum type, bool srgb,
                                       InternalFormat::SupportCheckFunction textureSupport,
                                       InternalFormat::SupportCheckFunction renderSupport,
                                       InternalFormat::SupportCheckFunction filterSupport)
{
    InternalFormat formatInfo;
    formatInfo.compressedBlockWidth = compressedBlockWidth;
    formatInfo.compressedBlockHeight = compressedBlockHeight;
    formatInfo.pixelBytes = compressedBlockSize / 8;
    formatInfo.componentCount = componentCount;
    formatInfo.format = format;
    formatInfo.type = type;
    formatInfo.componentType = GL_UNSIGNED_NORMALIZED;
    formatInfo.colorEncoding = (srgb ? GL_SRGB : GL_LINEAR);
    formatInfo.compressed = true;
    formatInfo.textureSupport = textureSupport;
    formatInfo.renderSupport = renderSupport;
    formatInfo.filterSupport = filterSupport;
    return formatInfo;
}

typedef std::pair<GLenum, InternalFormat> InternalFormatInfoPair;
typedef std::map<GLenum, InternalFormat> InternalFormatInfoMap;

static InternalFormatInfoMap BuildInternalFormatInfoMap()
{
    InternalFormatInfoMap map;

    // clang-format off
    // From ES 3.0.1 spec, table 3.12
    map.insert(InternalFormatInfoPair(GL_NONE,              InternalFormat()));

    //                               | Internal format     |          | R | G | B | A |S | Format         | Type                           | Component type        | SRGB | Texture supported                        | Renderable                               | Filterable    |
    map.insert(InternalFormatInfoPair(GL_R8,                RGBAFormat( 8,  0,  0,  0, 0, GL_RED,          GL_UNSIGNED_BYTE,                GL_UNSIGNED_NORMALIZED, false, RequireESOrExt<3, &Extensions::textureRG>, RequireESOrExt<3, &Extensions::textureRG>, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_R8_SNORM,          RGBAFormat( 8,  0,  0,  0, 0, GL_RED,          GL_BYTE,                         GL_SIGNED_NORMALIZED,   false, RequireES<3>,                              NeverSupported,                            AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RG8,               RGBAFormat( 8,  8,  0,  0, 0, GL_RG,           GL_UNSIGNED_BYTE,                GL_UNSIGNED_NORMALIZED, false, RequireESOrExt<3, &Extensions::textureRG>, RequireESOrExt<3, &Extensions::textureRG>, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RG8_SNORM,         RGBAFormat( 8,  8,  0,  0, 0, GL_RG,           GL_BYTE,                         GL_SIGNED_NORMALIZED,   false, RequireES<3>,                              NeverSupported,                            AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB8,              RGBAFormat( 8,  8,  8,  0, 0, GL_RGB,          GL_UNSIGNED_BYTE,                GL_UNSIGNED_NORMALIZED, false, RequireESOrExt<3, &Extensions::rgb8rgba8>, RequireESOrExt<3, &Extensions::rgb8rgba8>, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB8_SNORM,        RGBAFormat( 8,  8,  8,  0, 0, GL_RGB,          GL_BYTE,                         GL_SIGNED_NORMALIZED,   false, RequireES<3>,                              NeverSupported,                            AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB565,            RGBAFormat( 5,  6,  5,  0, 0, GL_RGB,          GL_UNSIGNED_SHORT_5_6_5,         GL_UNSIGNED_NORMALIZED, false, RequireES<2>,                              RequireES<2>,                              AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RGBA4,             RGBAFormat( 4,  4,  4,  4, 0, GL_RGBA,         GL_UNSIGNED_SHORT_4_4_4_4,       GL_UNSIGNED_NORMALIZED, false, RequireES<2>,                              RequireES<2>,                              AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB5_A1,           RGBAFormat( 5,  5,  5,  1, 0, GL_RGBA,         GL_UNSIGNED_SHORT_5_5_5_1,       GL_UNSIGNED_NORMALIZED, false, RequireES<2>,                              RequireES<2>,                              AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RGBA8,             RGBAFormat( 8,  8,  8,  8, 0, GL_RGBA,         GL_UNSIGNED_BYTE,                GL_UNSIGNED_NORMALIZED, false, RequireESOrExt<3, &Extensions::rgb8rgba8>, RequireESOrExt<3, &Extensions::rgb8rgba8>, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RGBA8_SNORM,       RGBAFormat( 8,  8,  8,  8, 0, GL_RGBA,         GL_BYTE,                         GL_SIGNED_NORMALIZED,   false, RequireES<3>,                              NeverSupported,                            AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB10_A2,          RGBAFormat(10, 10, 10,  2, 0, GL_RGBA,         GL_UNSIGNED_INT_2_10_10_10_REV,  GL_UNSIGNED_NORMALIZED, false, RequireES<3>,                              RequireES<3>,                              AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB10_A2UI,        RGBAFormat(10, 10, 10,  2, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV,  GL_UNSIGNED_INT,        false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_SRGB8,             RGBAFormat( 8,  8,  8,  0, 0, GL_RGB,          GL_UNSIGNED_BYTE,                GL_UNSIGNED_NORMALIZED, true,  RequireESOrExt<3, &Extensions::sRGB>,      NeverSupported,                            AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_SRGB8_ALPHA8,      RGBAFormat( 8,  8,  8,  8, 0, GL_RGBA,         GL_UNSIGNED_BYTE,                GL_UNSIGNED_NORMALIZED, true,  RequireESOrExt<3, &Extensions::sRGB>,      RequireESOrExt<3, &Extensions::sRGB>,      AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_R11F_G11F_B10F,    RGBAFormat(11, 11, 10,  0, 0, GL_RGB,          GL_UNSIGNED_INT_10F_11F_11F_REV, GL_FLOAT,               false, RequireES<3>,                              RequireExt<&Extensions::colorBufferFloat>, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB9_E5,           RGBAFormat( 9,  9,  9,  0, 5, GL_RGB,          GL_UNSIGNED_INT_5_9_9_9_REV,     GL_FLOAT,               false, RequireES<3>,                              NeverSupported,                            AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_R8I,               RGBAFormat( 8,  0,  0,  0, 0, GL_RED_INTEGER,  GL_BYTE,                         GL_INT,                 false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_R8UI,              RGBAFormat( 8,  0,  0,  0, 0, GL_RED_INTEGER,  GL_UNSIGNED_BYTE,                GL_UNSIGNED_INT,        false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_R16I,              RGBAFormat(16,  0,  0,  0, 0, GL_RED_INTEGER,  GL_SHORT,                        GL_INT,                 false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_R16UI,             RGBAFormat(16,  0,  0,  0, 0, GL_RED_INTEGER,  GL_UNSIGNED_SHORT,               GL_UNSIGNED_INT,        false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_R32I,              RGBAFormat(32,  0,  0,  0, 0, GL_RED_INTEGER,  GL_INT,                          GL_INT,                 false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_R32UI,             RGBAFormat(32,  0,  0,  0, 0, GL_RED_INTEGER,  GL_UNSIGNED_INT,                 GL_UNSIGNED_INT,        false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RG8I,              RGBAFormat( 8,  8,  0,  0, 0, GL_RG_INTEGER,   GL_BYTE,                         GL_INT,                 false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RG8UI,             RGBAFormat( 8,  8,  0,  0, 0, GL_RG_INTEGER,   GL_UNSIGNED_BYTE,                GL_UNSIGNED_INT,        false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RG16I,             RGBAFormat(16, 16,  0,  0, 0, GL_RG_INTEGER,   GL_SHORT,                        GL_INT,                 false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RG16UI,            RGBAFormat(16, 16,  0,  0, 0, GL_RG_INTEGER,   GL_UNSIGNED_SHORT,               GL_UNSIGNED_INT,        false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RG32I,             RGBAFormat(32, 32,  0,  0, 0, GL_RG_INTEGER,   GL_INT,                          GL_INT,                 false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RG32UI,            RGBAFormat(32, 32,  0,  0, 0, GL_RG_INTEGER,   GL_UNSIGNED_INT,                 GL_UNSIGNED_INT,        false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB8I,             RGBAFormat( 8,  8,  8,  0, 0, GL_RGB_INTEGER,  GL_BYTE,                         GL_INT,                 false, RequireES<3>,                              NeverSupported,                            NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB8UI,            RGBAFormat( 8,  8,  8,  0, 0, GL_RGB_INTEGER,  GL_UNSIGNED_BYTE,                GL_UNSIGNED_INT,        false, RequireES<3>,                              NeverSupported,                            NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB16I,            RGBAFormat(16, 16, 16,  0, 0, GL_RGB_INTEGER,  GL_SHORT,                        GL_INT,                 false, RequireES<3>,                              NeverSupported,                            NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB16UI,           RGBAFormat(16, 16, 16,  0, 0, GL_RGB_INTEGER,  GL_UNSIGNED_SHORT,               GL_UNSIGNED_INT,        false, RequireES<3>,                              NeverSupported,                            NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB32I,            RGBAFormat(32, 32, 32,  0, 0, GL_RGB_INTEGER,  GL_INT,                          GL_INT,                 false, RequireES<3>,                              NeverSupported,                            NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB32UI,           RGBAFormat(32, 32, 32,  0, 0, GL_RGB_INTEGER,  GL_UNSIGNED_INT,                 GL_UNSIGNED_INT,        false, RequireES<3>,                              NeverSupported,                            NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RGBA8I,            RGBAFormat( 8,  8,  8,  8, 0, GL_RGBA_INTEGER, GL_BYTE,                         GL_INT,                 false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RGBA8UI,           RGBAFormat( 8,  8,  8,  8, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE,                GL_UNSIGNED_INT,        false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RGBA16I,           RGBAFormat(16, 16, 16, 16, 0, GL_RGBA_INTEGER, GL_SHORT,                        GL_INT,                 false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RGBA16UI,          RGBAFormat(16, 16, 16, 16, 0, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT,               GL_UNSIGNED_INT,        false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RGBA32I,           RGBAFormat(32, 32, 32, 32, 0, GL_RGBA_INTEGER, GL_INT,                          GL_INT,                 false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));
    map.insert(InternalFormatInfoPair(GL_RGBA32UI,          RGBAFormat(32, 32, 32, 32, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT,                 GL_UNSIGNED_INT,        false, RequireES<3>,                              RequireES<3>,                              NeverSupported)));

    map.insert(InternalFormatInfoPair(GL_BGRA8_EXT,         RGBAFormat( 8,  8,  8,  8, 0, GL_BGRA_EXT,     GL_UNSIGNED_BYTE,                  GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureFormatBGRA8888>, RequireExt<&Extensions::textureFormatBGRA8888>, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_BGRA4_ANGLEX,      RGBAFormat( 4,  4,  4,  4, 0, GL_BGRA_EXT,     GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT, GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureFormatBGRA8888>, RequireExt<&Extensions::textureFormatBGRA8888>, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_BGR5_A1_ANGLEX,    RGBAFormat( 5,  5,  5,  1, 0, GL_BGRA_EXT,     GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT, GL_UNSIGNED_NORMALIZED, false, RequireExt<&Extensions::textureFormatBGRA8888>, RequireExt<&Extensions::textureFormatBGRA8888>, AlwaysSupported)));

    // Floating point renderability and filtering is provided by OES_texture_float and OES_texture_half_float
    //                               | Internal format     |          | D |S | Format             | Type                                   | Comp   | SRGB |  Texture supported                                                             | Renderable                                                                    | Filterable                                    |
    //                               |                     |          |   |  |                    |                                        | type   |      |                                                                                |                                                                               |                                               |
    map.insert(InternalFormatInfoPair(GL_R16F,              RGBAFormat(16,  0,  0,  0, 0, GL_RED,          GL_HALF_FLOAT,                   GL_FLOAT, false, RequireESOrExtAndExt<3, &Extensions::textureHalfFloat, &Extensions::textureRG>, RequireESOrExtAndExt<3, &Extensions::textureHalfFloat, &Extensions::textureRG>, RequireExt<&Extensions::textureHalfFloatLinear>)));
    map.insert(InternalFormatInfoPair(GL_RG16F,             RGBAFormat(16, 16,  0,  0, 0, GL_RG,           GL_HALF_FLOAT,                   GL_FLOAT, false, RequireESOrExtAndExt<3, &Extensions::textureHalfFloat, &Extensions::textureRG>, RequireESOrExtAndExt<3, &Extensions::textureHalfFloat, &Extensions::textureRG>, RequireExt<&Extensions::textureHalfFloatLinear>)));
    map.insert(InternalFormatInfoPair(GL_RGB16F,            RGBAFormat(16, 16, 16,  0, 0, GL_RGB,          GL_HALF_FLOAT,                   GL_FLOAT, false, RequireESOrExt<3, &Extensions::textureHalfFloat>,                               RequireESOrExt<3, &Extensions::textureHalfFloat>,                               RequireExt<&Extensions::textureHalfFloatLinear>)));
    map.insert(InternalFormatInfoPair(GL_RGBA16F,           RGBAFormat(16, 16, 16, 16, 0, GL_RGBA,         GL_HALF_FLOAT,                   GL_FLOAT, false, RequireESOrExt<3, &Extensions::textureHalfFloat>,                               RequireESOrExt<3, &Extensions::textureHalfFloat>,                               RequireExt<&Extensions::textureHalfFloatLinear>)));
    map.insert(InternalFormatInfoPair(GL_R32F,              RGBAFormat(32,  0,  0,  0, 0, GL_RED,          GL_FLOAT,                        GL_FLOAT, false, RequireESOrExtAndExt<3, &Extensions::textureFloat, &Extensions::textureRG>,     RequireESOrExtAndExt<3, &Extensions::textureFloat, &Extensions::textureRG>,     RequireExt<&Extensions::textureFloatLinear>    )));
    map.insert(InternalFormatInfoPair(GL_RG32F,             RGBAFormat(32, 32,  0,  0, 0, GL_RG,           GL_FLOAT,                        GL_FLOAT, false, RequireESOrExtAndExt<3, &Extensions::textureFloat, &Extensions::textureRG>,     RequireESOrExtAndExt<3, &Extensions::textureFloat, &Extensions::textureRG>,     RequireExt<&Extensions::textureFloatLinear>    )));
    map.insert(InternalFormatInfoPair(GL_RGB32F,            RGBAFormat(32, 32, 32,  0, 0, GL_RGB,          GL_FLOAT,                        GL_FLOAT, false, RequireESOrExt<3, &Extensions::textureFloat>,                                   RequireESOrExt<3, &Extensions::textureFloat>,                                   RequireExt<&Extensions::textureFloatLinear>    )));
    map.insert(InternalFormatInfoPair(GL_RGBA32F,           RGBAFormat(32, 32, 32, 32, 0, GL_RGBA,         GL_FLOAT,                        GL_FLOAT, false, RequireESOrExt<3, &Extensions::textureFloat>,                                   RequireESOrExt<3, &Extensions::textureFloat>,                                   RequireExt<&Extensions::textureFloatLinear>    )));

    // Depth stencil formats
    //                               | Internal format         |                  | D |S | X | Format            | Type                             | Component type        | Supported                                    | Renderable                                                                         | Filterable                                  |
    map.insert(InternalFormatInfoPair(GL_DEPTH_COMPONENT16,     DepthStencilFormat(16, 0,  0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT,                 GL_UNSIGNED_NORMALIZED, RequireES<2>,                                  RequireES<2>,                                                                        RequireESOrExt<3, &Extensions::depthTextures>)));
    map.insert(InternalFormatInfoPair(GL_DEPTH_COMPONENT24,     DepthStencilFormat(24, 0,  0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,                   GL_UNSIGNED_NORMALIZED, RequireES<3>,                                  RequireES<3>,                                                                        RequireESOrExt<3, &Extensions::depthTextures>)));
    map.insert(InternalFormatInfoPair(GL_DEPTH_COMPONENT32F,    DepthStencilFormat(32, 0,  0, GL_DEPTH_COMPONENT, GL_FLOAT,                          GL_FLOAT,               RequireES<3>,                                  RequireES<3>,                                                                        RequireESOrExt<3, &Extensions::depthTextures>)));
    map.insert(InternalFormatInfoPair(GL_DEPTH_COMPONENT32_OES, DepthStencilFormat(32, 0,  0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,                   GL_UNSIGNED_NORMALIZED, RequireExt<&Extensions::depthTextures>,        RequireExt<&Extensions::depthTextures>,                                              AlwaysSupported                              )));
    map.insert(InternalFormatInfoPair(GL_DEPTH24_STENCIL8,      DepthStencilFormat(24, 8,  0, GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8,              GL_UNSIGNED_NORMALIZED, RequireESOrExt<3, &Extensions::depthTextures>, RequireESOrExtOrExt<3, &Extensions::depthTextures, &Extensions::packedDepthStencil>, AlwaysSupported                              )));
    map.insert(InternalFormatInfoPair(GL_DEPTH32F_STENCIL8,     DepthStencilFormat(32, 8, 24, GL_DEPTH_STENCIL,   GL_FLOAT_32_UNSIGNED_INT_24_8_REV, GL_FLOAT,               RequireES<3>,                                  RequireES<3>,                                                                        AlwaysSupported                              )));
    // STENCIL_INDEX8 is special-cased, see around the bottom of the list.

    // Luminance alpha formats
    //                               | Internal format          |          | L | A | Format            | Type            | Component type        | Supported                                                                    | Renderable    | Filterable    |
    map.insert(InternalFormatInfoPair(GL_ALPHA8_EXT,             LUMAFormat( 0,  8, GL_ALPHA,           GL_UNSIGNED_BYTE, GL_UNSIGNED_NORMALIZED, RequireExt<&Extensions::textureStorage>,                                      NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_LUMINANCE8_EXT,         LUMAFormat( 8,  0, GL_LUMINANCE,       GL_UNSIGNED_BYTE, GL_UNSIGNED_NORMALIZED, RequireExt<&Extensions::textureStorage>,                                      NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_ALPHA32F_EXT,           LUMAFormat( 0, 32, GL_ALPHA,           GL_FLOAT,         GL_FLOAT,               RequireExtAndExt<&Extensions::textureStorage, &Extensions::textureFloat>,     NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_LUMINANCE32F_EXT,       LUMAFormat(32,  0, GL_LUMINANCE,       GL_FLOAT,         GL_FLOAT,               RequireExtAndExt<&Extensions::textureStorage, &Extensions::textureFloat>,     NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_ALPHA16F_EXT,           LUMAFormat( 0, 16, GL_ALPHA,           GL_HALF_FLOAT,    GL_FLOAT,               RequireExtAndExt<&Extensions::textureStorage, &Extensions::textureHalfFloat>, NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_LUMINANCE16F_EXT,       LUMAFormat(16,  0, GL_LUMINANCE,       GL_HALF_FLOAT,    GL_FLOAT,               RequireExtAndExt<&Extensions::textureStorage, &Extensions::textureHalfFloat>, NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_LUMINANCE8_ALPHA8_EXT,  LUMAFormat( 8,  8, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, GL_UNSIGNED_NORMALIZED, RequireExt<&Extensions::textureStorage>,                                      NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_LUMINANCE_ALPHA32F_EXT, LUMAFormat(32, 32, GL_LUMINANCE_ALPHA, GL_FLOAT,         GL_FLOAT,               RequireExtAndExt<&Extensions::textureStorage, &Extensions::textureFloat>,     NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_LUMINANCE_ALPHA16F_EXT, LUMAFormat(16, 16, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT,    GL_FLOAT,               RequireExtAndExt<&Extensions::textureStorage, &Extensions::textureHalfFloat>, NeverSupported, AlwaysSupported)));

    // Unsized formats
    //                               | Internal format   |             | Format            | Supported                                         | Renderable                                        | Filterable    |
    map.insert(InternalFormatInfoPair(GL_ALPHA,           UnsizedFormat(GL_ALPHA,           RequireES<2>,                                       NeverSupported,                                     AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_LUMINANCE,       UnsizedFormat(GL_LUMINANCE,       RequireES<2>,                                       NeverSupported,                                     AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_LUMINANCE_ALPHA, UnsizedFormat(GL_LUMINANCE_ALPHA, RequireES<2>,                                       NeverSupported,                                     AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RED,             UnsizedFormat(GL_RED,             RequireESOrExt<3, &Extensions::textureRG>,          NeverSupported,                                     AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RG,              UnsizedFormat(GL_RG,              RequireESOrExt<3, &Extensions::textureRG>,          NeverSupported,                                     AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RGB,             UnsizedFormat(GL_RGB,             RequireES<2>,                                       RequireES<2>,                                       AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RGBA,            UnsizedFormat(GL_RGBA,            RequireES<2>,                                       RequireES<2>,                                       AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_RED_INTEGER,     UnsizedFormat(GL_RED_INTEGER,     RequireES<3>,                                       NeverSupported,                                     NeverSupported )));
    map.insert(InternalFormatInfoPair(GL_RG_INTEGER,      UnsizedFormat(GL_RG_INTEGER,      RequireES<3>,                                       NeverSupported,                                     NeverSupported )));
    map.insert(InternalFormatInfoPair(GL_RGB_INTEGER,     UnsizedFormat(GL_RGB_INTEGER,     RequireES<3>,                                       NeverSupported,                                     NeverSupported )));
    map.insert(InternalFormatInfoPair(GL_RGBA_INTEGER,    UnsizedFormat(GL_RGBA_INTEGER,    RequireES<3>,                                       NeverSupported,                                     NeverSupported )));
    map.insert(InternalFormatInfoPair(GL_BGRA_EXT,        UnsizedFormat(GL_BGRA_EXT,        RequireExt<&Extensions::textureFormatBGRA8888>,     RequireExt<&Extensions::textureFormatBGRA8888>,     AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_DEPTH_COMPONENT, UnsizedFormat(GL_DEPTH_COMPONENT, RequireES<2>,                                       RequireES<2>,                                       AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_DEPTH_STENCIL,   UnsizedFormat(GL_DEPTH_STENCIL,   RequireESOrExt<3, &Extensions::packedDepthStencil>, RequireESOrExt<3, &Extensions::packedDepthStencil>, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_SRGB_EXT,        UnsizedFormat(GL_RGB,             RequireESOrExt<3, &Extensions::sRGB>,               NeverSupported,                                     AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_SRGB_ALPHA_EXT,  UnsizedFormat(GL_RGBA,            RequireESOrExt<3, &Extensions::sRGB>,               RequireESOrExt<3, &Extensions::sRGB>,               AlwaysSupported)));

    // Compressed formats, From ES 3.0.1 spec, table 3.16
    //                               | Internal format                             |                |W |H | BS |CC| Format                                      | Type            | SRGB | Supported   | Renderable    | Filterable    |
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_R11_EAC,                        CompressedFormat(4, 4,  64, 1, GL_COMPRESSED_R11_EAC,                        GL_UNSIGNED_BYTE, false, RequireES<3>, NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_SIGNED_R11_EAC,                 CompressedFormat(4, 4,  64, 1, GL_COMPRESSED_SIGNED_R11_EAC,                 GL_UNSIGNED_BYTE, false, RequireES<3>, NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_RG11_EAC,                       CompressedFormat(4, 4, 128, 2, GL_COMPRESSED_RG11_EAC,                       GL_UNSIGNED_BYTE, false, RequireES<3>, NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_SIGNED_RG11_EAC,                CompressedFormat(4, 4, 128, 2, GL_COMPRESSED_SIGNED_RG11_EAC,                GL_UNSIGNED_BYTE, false, RequireES<3>, NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_RGB8_ETC2,                      CompressedFormat(4, 4,  64, 3, GL_COMPRESSED_RGB8_ETC2,                      GL_UNSIGNED_BYTE, false, RequireES<3>, NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_SRGB8_ETC2,                     CompressedFormat(4, 4,  64, 3, GL_COMPRESSED_SRGB8_ETC2,                     GL_UNSIGNED_BYTE, true,  RequireES<3>, NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,  CompressedFormat(4, 4,  64, 3, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,  GL_UNSIGNED_BYTE, false, RequireES<3>, NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, CompressedFormat(4, 4,  64, 3, GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, GL_UNSIGNED_BYTE, true,  RequireES<3>, NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_RGBA8_ETC2_EAC,                 CompressedFormat(4, 4, 128, 4, GL_COMPRESSED_RGBA8_ETC2_EAC,                 GL_UNSIGNED_BYTE, false, RequireES<3>, NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,          CompressedFormat(4, 4, 128, 4, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,          GL_UNSIGNED_BYTE, true,  RequireES<3>, NeverSupported, AlwaysSupported)));

    // From GL_EXT_texture_compression_dxt1
    //                               | Internal format                   |                |W |H | BS |CC| Format                            | Type            | SRGB | Supported                                      | Renderable    | Filterable    |
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_RGB_S3TC_DXT1_EXT,    CompressedFormat(4, 4,  64, 3, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,    GL_UNSIGNED_BYTE, false, RequireExt<&Extensions::textureCompressionDXT1>, NeverSupported, AlwaysSupported)));
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   CompressedFormat(4, 4,  64, 4, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   GL_UNSIGNED_BYTE, false, RequireExt<&Extensions::textureCompressionDXT1>, NeverSupported, AlwaysSupported)));

    // From GL_ANGLE_texture_compression_dxt3
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE, CompressedFormat(4, 4, 128, 4, GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE, GL_UNSIGNED_BYTE, false, RequireExt<&Extensions::textureCompressionDXT5>, NeverSupported, AlwaysSupported)));

    // From GL_ANGLE_texture_compression_dxt5
    map.insert(InternalFormatInfoPair(GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE, CompressedFormat(4, 4, 128, 4, GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE, GL_UNSIGNED_BYTE, false, RequireExt<&Extensions::textureCompressionDXT5>, NeverSupported, AlwaysSupported)));

    // For STENCIL_INDEX8 we chose a normalized component type for the following reasons:
    // - Multisampled buffer are disallowed for non-normalized integer component types and we want to support it for STENCIL_INDEX8
    // - All other stencil formats (all depth-stencil) are either float or normalized
    // - It affects only validation of internalformat in RenderbufferStorageMultisample.
    //                               | Internal format  |                  |D |S |X | Format          | Type            | Component type        | Supported   | Renderable  | Filterable   |
    map.insert(InternalFormatInfoPair(GL_STENCIL_INDEX8, DepthStencilFormat(0, 8, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_BYTE, GL_UNSIGNED_NORMALIZED, RequireES<2>, RequireES<2>, NeverSupported)));
    // clang-format on

    return map;
}

static const InternalFormatInfoMap &GetInternalFormatMap()
{
    static const InternalFormatInfoMap formatMap = BuildInternalFormatInfoMap();
    return formatMap;
}

static FormatSet BuildAllSizedInternalFormatSet()
{
    FormatSet result;

    const InternalFormatInfoMap &formats = GetInternalFormatMap();
    for (InternalFormatInfoMap::const_iterator i = formats.begin(); i != formats.end(); i++)
    {
        if (i->second.pixelBytes > 0)
        {
            result.insert(i->first);
        }
    }

    return result;
}

const Type &GetTypeInfo(GLenum type)
{
    switch (type)
    {
      case GL_UNSIGNED_BYTE:
      case GL_BYTE:
        {
            static const Type info = GenTypeInfo(1, false);
            return info;
        }
      case GL_UNSIGNED_SHORT:
      case GL_SHORT:
      case GL_HALF_FLOAT:
      case GL_HALF_FLOAT_OES:
        {
            static const Type info = GenTypeInfo(2, false);
            return info;
        }
      case GL_UNSIGNED_INT:
      case GL_INT:
      case GL_FLOAT:
        {
            static const Type info = GenTypeInfo(4, false);
            return info;
        }
      case GL_UNSIGNED_SHORT_5_6_5:
      case GL_UNSIGNED_SHORT_4_4_4_4:
      case GL_UNSIGNED_SHORT_5_5_5_1:
      case GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT:
      case GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT:
        {
            static const Type info = GenTypeInfo(2, true);
            return info;
        }
      case GL_UNSIGNED_INT_2_10_10_10_REV:
      case GL_UNSIGNED_INT_24_8:
      case GL_UNSIGNED_INT_10F_11F_11F_REV:
      case GL_UNSIGNED_INT_5_9_9_9_REV:
        {
            ASSERT(GL_UNSIGNED_INT_24_8_OES == GL_UNSIGNED_INT_24_8);
            static const Type info = GenTypeInfo(4, true);
            return info;
        }
      case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
        {
            static const Type info = GenTypeInfo(8, true);
            return info;
        }
      default:
        {
            static const Type defaultInfo;
            return defaultInfo;
        }
    }
}

const InternalFormat &GetInternalFormatInfo(GLenum internalFormat)
{
    const InternalFormatInfoMap &formatMap = GetInternalFormatMap();
    InternalFormatInfoMap::const_iterator iter = formatMap.find(internalFormat);
    if (iter != formatMap.end())
    {
        return iter->second;
    }
    else
    {
        static const InternalFormat defaultInternalFormat;
        return defaultInternalFormat;
    }
}

GLuint InternalFormat::computeRowPitch(GLenum formatType, GLsizei width, GLint alignment, GLint rowLength) const
{
    ASSERT(alignment > 0 && isPow2(alignment));
    GLuint rowBytes;
    if (rowLength > 0)
    {
        ASSERT(!compressed);
        rowBytes = pixelBytes * rowLength;
    }
    else
    {
        rowBytes = computeBlockSize(formatType, width, 1);
    }
    return rx::roundUp(rowBytes, static_cast<GLuint>(alignment));
}

GLuint InternalFormat::computeDepthPitch(GLenum formatType, GLsizei width, GLsizei height, GLint alignment, GLint rowLength) const
{
    return computeRowPitch(formatType, width, alignment, rowLength) * height;
}

GLuint InternalFormat::computeBlockSize(GLenum formatType, GLsizei width, GLsizei height) const
{
    if (compressed)
    {
        GLsizei numBlocksWide = (width + compressedBlockWidth - 1) / compressedBlockWidth;
        GLsizei numBlocksHight = (height + compressedBlockHeight - 1) / compressedBlockHeight;
        return (pixelBytes * numBlocksWide * numBlocksHight);
    }
    else
    {
        const Type &typeInfo = GetTypeInfo(formatType);
        if (typeInfo.specialInterpretation)
        {
            return typeInfo.bytes * width * height;
        }
        else
        {
            return componentCount * typeInfo.bytes * width * height;
        }
    }
}

GLenum GetSizedInternalFormat(GLenum internalFormat, GLenum type)
{
    const InternalFormat& formatInfo = GetInternalFormatInfo(internalFormat);
    if (formatInfo.pixelBytes > 0)
    {
        return internalFormat;
    }
    else
    {
        static const FormatMap formatMap = BuildFormatMap();
        FormatMap::const_iterator iter = formatMap.find(FormatTypePair(internalFormat, type));
        if (iter != formatMap.end())
        {
            return iter->second;
        }
        else
        {
            return GL_NONE;
        }
    }
}

const FormatSet &GetAllSizedInternalFormats()
{
    static FormatSet formatSet = BuildAllSizedInternalFormatSet();
    return formatSet;
}

AttributeType GetAttributeType(GLenum enumValue)
{
    switch (enumValue)
    {
        case GL_FLOAT:
            return ATTRIBUTE_FLOAT;
        case GL_FLOAT_VEC2:
            return ATTRIBUTE_VEC2;
        case GL_FLOAT_VEC3:
            return ATTRIBUTE_VEC3;
        case GL_FLOAT_VEC4:
            return ATTRIBUTE_VEC4;
        case GL_INT:
            return ATTRIBUTE_INT;
        case GL_INT_VEC2:
            return ATTRIBUTE_IVEC2;
        case GL_INT_VEC3:
            return ATTRIBUTE_IVEC3;
        case GL_INT_VEC4:
            return ATTRIBUTE_IVEC4;
        case GL_UNSIGNED_INT:
            return ATTRIBUTE_UINT;
        case GL_UNSIGNED_INT_VEC2:
            return ATTRIBUTE_UVEC2;
        case GL_UNSIGNED_INT_VEC3:
            return ATTRIBUTE_UVEC3;
        case GL_UNSIGNED_INT_VEC4:
            return ATTRIBUTE_UVEC4;
        case GL_FLOAT_MAT2:
            return ATTRIBUTE_MAT2;
        case GL_FLOAT_MAT3:
            return ATTRIBUTE_MAT3;
        case GL_FLOAT_MAT4:
            return ATTRIBUTE_MAT4;
        case GL_FLOAT_MAT2x3:
            return ATTRIBUTE_MAT2x3;
        case GL_FLOAT_MAT2x4:
            return ATTRIBUTE_MAT2x4;
        case GL_FLOAT_MAT3x2:
            return ATTRIBUTE_MAT3x2;
        case GL_FLOAT_MAT3x4:
            return ATTRIBUTE_MAT3x4;
        case GL_FLOAT_MAT4x2:
            return ATTRIBUTE_MAT4x2;
        case GL_FLOAT_MAT4x3:
            return ATTRIBUTE_MAT4x3;
        default:
            UNREACHABLE();
            return ATTRIBUTE_FLOAT;
    }
}

VertexFormatType GetVertexFormatType(GLenum type, GLboolean normalized, GLuint components, bool pureInteger)
{
    switch (type)
    {
        case GL_BYTE:
            switch (components)
            {
                case 1:
                    if (pureInteger)
                        return VERTEX_FORMAT_SBYTE1_INT;
                    if (normalized)
                        return VERTEX_FORMAT_SBYTE1_NORM;
                    return VERTEX_FORMAT_SBYTE1;
                case 2:
                    if (pureInteger)
                        return VERTEX_FORMAT_SBYTE2_INT;
                    if (normalized)
                        return VERTEX_FORMAT_SBYTE2_NORM;
                    return VERTEX_FORMAT_SBYTE2;
                case 3:
                    if (pureInteger)
                        return VERTEX_FORMAT_SBYTE3_INT;
                    if (normalized)
                        return VERTEX_FORMAT_SBYTE3_NORM;
                    return VERTEX_FORMAT_SBYTE3;
                case 4:
                    if (pureInteger)
                        return VERTEX_FORMAT_SBYTE4_INT;
                    if (normalized)
                        return VERTEX_FORMAT_SBYTE4_NORM;
                    return VERTEX_FORMAT_SBYTE4;
                default:
                    UNREACHABLE();
                    break;
            }
        case GL_UNSIGNED_BYTE:
            switch (components)
            {
                case 1:
                    if (pureInteger)
                        return VERTEX_FORMAT_UBYTE1_INT;
                    if (normalized)
                        return VERTEX_FORMAT_UBYTE1_NORM;
                    return VERTEX_FORMAT_UBYTE1;
                case 2:
                    if (pureInteger)
                        return VERTEX_FORMAT_UBYTE2_INT;
                    if (normalized)
                        return VERTEX_FORMAT_UBYTE2_NORM;
                    return VERTEX_FORMAT_UBYTE2;
                case 3:
                    if (pureInteger)
                        return VERTEX_FORMAT_UBYTE3_INT;
                    if (normalized)
                        return VERTEX_FORMAT_UBYTE3_NORM;
                    return VERTEX_FORMAT_UBYTE3;
                case 4:
                    if (pureInteger)
                        return VERTEX_FORMAT_UBYTE4_INT;
                    if (normalized)
                        return VERTEX_FORMAT_UBYTE4_NORM;
                    return VERTEX_FORMAT_UBYTE4;
                default:
                    UNREACHABLE();
                    break;
            }
        case GL_SHORT:
            switch (components)
            {
                case 1:
                    if (pureInteger)
                        return VERTEX_FORMAT_SSHORT1_INT;
                    if (normalized)
                        return VERTEX_FORMAT_SSHORT1_NORM;
                    return VERTEX_FORMAT_SSHORT1;
                case 2:
                    if (pureInteger)
                        return VERTEX_FORMAT_SSHORT2_INT;
                    if (normalized)
                        return VERTEX_FORMAT_SSHORT2_NORM;
                    return VERTEX_FORMAT_SSHORT2;
                case 3:
                    if (pureInteger)
                        return VERTEX_FORMAT_SSHORT3_INT;
                    if (normalized)
                        return VERTEX_FORMAT_SSHORT3_NORM;
                    return VERTEX_FORMAT_SSHORT3;
                case 4:
                    if (pureInteger)
                        return VERTEX_FORMAT_SSHORT4_INT;
                    if (normalized)
                        return VERTEX_FORMAT_SSHORT4_NORM;
                    return VERTEX_FORMAT_SSHORT4;
                default:
                    UNREACHABLE();
                    break;
            }
        case GL_UNSIGNED_SHORT:
            switch (components)
            {
                case 1:
                    if (pureInteger)
                        return VERTEX_FORMAT_USHORT1_INT;
                    if (normalized)
                        return VERTEX_FORMAT_USHORT1_NORM;
                    return VERTEX_FORMAT_USHORT1;
                case 2:
                    if (pureInteger)
                        return VERTEX_FORMAT_USHORT2_INT;
                    if (normalized)
                        return VERTEX_FORMAT_USHORT2_NORM;
                    return VERTEX_FORMAT_USHORT2;
                case 3:
                    if (pureInteger)
                        return VERTEX_FORMAT_USHORT3_INT;
                    if (normalized)
                        return VERTEX_FORMAT_USHORT3_NORM;
                    return VERTEX_FORMAT_USHORT3;
                case 4:
                    if (pureInteger)
                        return VERTEX_FORMAT_USHORT4_INT;
                    if (normalized)
                        return VERTEX_FORMAT_USHORT4_NORM;
                    return VERTEX_FORMAT_USHORT4;
                default:
                    UNREACHABLE();
                    break;
            }
        case GL_INT:
            switch (components)
            {
                case 1:
                    if (pureInteger)
                        return VERTEX_FORMAT_SINT1_INT;
                    if (normalized)
                        return VERTEX_FORMAT_SINT1_NORM;
                    return VERTEX_FORMAT_SINT1;
                case 2:
                    if (pureInteger)
                        return VERTEX_FORMAT_SINT2_INT;
                    if (normalized)
                        return VERTEX_FORMAT_SINT2_NORM;
                    return VERTEX_FORMAT_SINT2;
                case 3:
                    if (pureInteger)
                        return VERTEX_FORMAT_SINT3_INT;
                    if (normalized)
                        return VERTEX_FORMAT_SINT3_NORM;
                    return VERTEX_FORMAT_SINT3;
                case 4:
                    if (pureInteger)
                        return VERTEX_FORMAT_SINT4_INT;
                    if (normalized)
                        return VERTEX_FORMAT_SINT4_NORM;
                    return VERTEX_FORMAT_SINT4;
                default:
                    UNREACHABLE();
                    break;
            }
        case GL_UNSIGNED_INT:
            switch (components)
            {
                case 1:
                    if (pureInteger)
                        return VERTEX_FORMAT_UINT1_INT;
                    if (normalized)
                        return VERTEX_FORMAT_UINT1_NORM;
                    return VERTEX_FORMAT_UINT1;
                case 2:
                    if (pureInteger)
                        return VERTEX_FORMAT_UINT2_INT;
                    if (normalized)
                        return VERTEX_FORMAT_UINT2_NORM;
                    return VERTEX_FORMAT_UINT2;
                case 3:
                    if (pureInteger)
                        return VERTEX_FORMAT_UINT3_INT;
                    if (normalized)
                        return VERTEX_FORMAT_UINT3_NORM;
                    return VERTEX_FORMAT_UINT3;
                case 4:
                    if (pureInteger)
                        return VERTEX_FORMAT_UINT4_INT;
                    if (normalized)
                        return VERTEX_FORMAT_UINT4_NORM;
                    return VERTEX_FORMAT_UINT4;
                default:
                    UNREACHABLE();
                    break;
            }
        case GL_FLOAT:
            switch (components)
            {
                case 1:
                    return VERTEX_FORMAT_FLOAT1;
                case 2:
                    return VERTEX_FORMAT_FLOAT2;
                case 3:
                    return VERTEX_FORMAT_FLOAT3;
                case 4:
                    return VERTEX_FORMAT_FLOAT4;
                default:
                    UNREACHABLE();
                    break;
            }
        case GL_HALF_FLOAT:
            switch (components)
            {
                case 1:
                    return VERTEX_FORMAT_HALF1;
                case 2:
                    return VERTEX_FORMAT_HALF2;
                case 3:
                    return VERTEX_FORMAT_HALF3;
                case 4:
                    return VERTEX_FORMAT_HALF4;
                default:
                    UNREACHABLE();
                    break;
            }
        case GL_FIXED:
            switch (components)
            {
                case 1:
                    return VERTEX_FORMAT_FIXED1;
                case 2:
                    return VERTEX_FORMAT_FIXED2;
                case 3:
                    return VERTEX_FORMAT_FIXED3;
                case 4:
                    return VERTEX_FORMAT_FIXED4;
                default:
                    UNREACHABLE();
                    break;
            }
        case GL_INT_2_10_10_10_REV:
            if (pureInteger)
                return VERTEX_FORMAT_SINT210_INT;
            if (normalized)
                return VERTEX_FORMAT_SINT210_NORM;
            return VERTEX_FORMAT_SINT210;
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            if (pureInteger)
                return VERTEX_FORMAT_UINT210_INT;
            if (normalized)
                return VERTEX_FORMAT_UINT210_NORM;
            return VERTEX_FORMAT_UINT210;
        default:
            UNREACHABLE();
            break;
    }
    return VERTEX_FORMAT_UBYTE1;
}

VertexFormatType GetVertexFormatType(const VertexAttribute &attrib)
{
    return GetVertexFormatType(attrib.type, attrib.normalized, attrib.size, attrib.pureInteger);
}

VertexFormatType GetVertexFormatType(const VertexAttribute &attrib, GLenum currentValueType)
{
    if (!attrib.enabled)
    {
        return GetVertexFormatType(currentValueType, GL_FALSE, 4, (currentValueType != GL_FLOAT));
    }
    return GetVertexFormatType(attrib);
}

const VertexFormat &GetVertexFormatFromType(VertexFormatType vertexFormatType)
{
    switch (vertexFormatType)
    {
        case VERTEX_FORMAT_SBYTE1:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_SBYTE1_NORM:
        {
            static const VertexFormat format(GL_BYTE, GL_TRUE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_SBYTE2:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_SBYTE2_NORM:
        {
            static const VertexFormat format(GL_BYTE, GL_TRUE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_SBYTE3:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_SBYTE3_NORM:
        {
            static const VertexFormat format(GL_BYTE, GL_TRUE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_SBYTE4:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_SBYTE4_NORM:
        {
            static const VertexFormat format(GL_BYTE, GL_TRUE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_UBYTE1:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_UBYTE1_NORM:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_TRUE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_UBYTE2:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_UBYTE2_NORM:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_TRUE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_UBYTE3:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_UBYTE3_NORM:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_TRUE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_UBYTE4:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_UBYTE4_NORM:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_TRUE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_SSHORT1:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_SSHORT1_NORM:
        {
            static const VertexFormat format(GL_SHORT, GL_TRUE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_SSHORT2:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_SSHORT2_NORM:
        {
            static const VertexFormat format(GL_SHORT, GL_TRUE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_SSHORT3:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_SSHORT3_NORM:
        {
            static const VertexFormat format(GL_SHORT, GL_TRUE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_SSHORT4:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_SSHORT4_NORM:
        {
            static const VertexFormat format(GL_SHORT, GL_TRUE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_USHORT1:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_USHORT1_NORM:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_TRUE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_USHORT2:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_USHORT2_NORM:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_TRUE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_USHORT3:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_USHORT3_NORM:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_TRUE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_USHORT4:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_USHORT4_NORM:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_TRUE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_SINT1:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_SINT1_NORM:
        {
            static const VertexFormat format(GL_INT, GL_TRUE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_SINT2:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_SINT2_NORM:
        {
            static const VertexFormat format(GL_INT, GL_TRUE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_SINT3:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_SINT3_NORM:
        {
            static const VertexFormat format(GL_INT, GL_TRUE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_SINT4:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_SINT4_NORM:
        {
            static const VertexFormat format(GL_INT, GL_TRUE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_UINT1:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_UINT1_NORM:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_TRUE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_UINT2:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_UINT2_NORM:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_TRUE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_UINT3:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_UINT3_NORM:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_TRUE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_UINT4:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_UINT4_NORM:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_TRUE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_SBYTE1_INT:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 1, true);
            return format;
        }
        case VERTEX_FORMAT_SBYTE2_INT:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 2, true);
            return format;
        }
        case VERTEX_FORMAT_SBYTE3_INT:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 3, true);
            return format;
        }
        case VERTEX_FORMAT_SBYTE4_INT:
        {
            static const VertexFormat format(GL_BYTE, GL_FALSE, 4, true);
            return format;
        }
        case VERTEX_FORMAT_UBYTE1_INT:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 1, true);
            return format;
        }
        case VERTEX_FORMAT_UBYTE2_INT:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 2, true);
            return format;
        }
        case VERTEX_FORMAT_UBYTE3_INT:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 3, true);
            return format;
        }
        case VERTEX_FORMAT_UBYTE4_INT:
        {
            static const VertexFormat format(GL_UNSIGNED_BYTE, GL_FALSE, 4, true);
            return format;
        }
        case VERTEX_FORMAT_SSHORT1_INT:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 1, true);
            return format;
        }
        case VERTEX_FORMAT_SSHORT2_INT:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 2, true);
            return format;
        }
        case VERTEX_FORMAT_SSHORT3_INT:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 3, true);
            return format;
        }
        case VERTEX_FORMAT_SSHORT4_INT:
        {
            static const VertexFormat format(GL_SHORT, GL_FALSE, 4, true);
            return format;
        }
        case VERTEX_FORMAT_USHORT1_INT:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 1, true);
            return format;
        }
        case VERTEX_FORMAT_USHORT2_INT:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 2, true);
            return format;
        }
        case VERTEX_FORMAT_USHORT3_INT:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 3, true);
            return format;
        }
        case VERTEX_FORMAT_USHORT4_INT:
        {
            static const VertexFormat format(GL_UNSIGNED_SHORT, GL_FALSE, 4, true);
            return format;
        }
        case VERTEX_FORMAT_SINT1_INT:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 1, true);
            return format;
        }
        case VERTEX_FORMAT_SINT2_INT:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 2, true);
            return format;
        }
        case VERTEX_FORMAT_SINT3_INT:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 3, true);
            return format;
        }
        case VERTEX_FORMAT_SINT4_INT:
        {
            static const VertexFormat format(GL_INT, GL_FALSE, 4, true);
            return format;
        }
        case VERTEX_FORMAT_UINT1_INT:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 1, true);
            return format;
        }
        case VERTEX_FORMAT_UINT2_INT:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 2, true);
            return format;
        }
        case VERTEX_FORMAT_UINT3_INT:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 3, true);
            return format;
        }
        case VERTEX_FORMAT_UINT4_INT:
        {
            static const VertexFormat format(GL_UNSIGNED_INT, GL_FALSE, 4, true);
            return format;
        }
        case VERTEX_FORMAT_FIXED1:
        {
            static const VertexFormat format(GL_FIXED, GL_FALSE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_FIXED2:
        {
            static const VertexFormat format(GL_FIXED, GL_FALSE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_FIXED3:
        {
            static const VertexFormat format(GL_FIXED, GL_FALSE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_FIXED4:
        {
            static const VertexFormat format(GL_FIXED, GL_FALSE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_HALF1:
        {
            static const VertexFormat format(GL_HALF_FLOAT, GL_FALSE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_HALF2:
        {
            static const VertexFormat format(GL_HALF_FLOAT, GL_FALSE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_HALF3:
        {
            static const VertexFormat format(GL_HALF_FLOAT, GL_FALSE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_HALF4:
        {
            static const VertexFormat format(GL_HALF_FLOAT, GL_FALSE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_FLOAT1:
        {
            static const VertexFormat format(GL_FLOAT, GL_FALSE, 1, false);
            return format;
        }
        case VERTEX_FORMAT_FLOAT2:
        {
            static const VertexFormat format(GL_FLOAT, GL_FALSE, 2, false);
            return format;
        }
        case VERTEX_FORMAT_FLOAT3:
        {
            static const VertexFormat format(GL_FLOAT, GL_FALSE, 3, false);
            return format;
        }
        case VERTEX_FORMAT_FLOAT4:
        {
            static const VertexFormat format(GL_FLOAT, GL_FALSE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_SINT210:
        {
            static const VertexFormat format(GL_INT_2_10_10_10_REV, GL_FALSE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_UINT210:
        {
            static const VertexFormat format(GL_UNSIGNED_INT_2_10_10_10_REV, GL_FALSE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_SINT210_NORM:
        {
            static const VertexFormat format(GL_INT_2_10_10_10_REV, GL_TRUE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_UINT210_NORM:
        {
            static const VertexFormat format(GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, 4, false);
            return format;
        }
        case VERTEX_FORMAT_SINT210_INT:
        {
            static const VertexFormat format(GL_INT_2_10_10_10_REV, GL_FALSE, 4, true);
            return format;
        }
        case VERTEX_FORMAT_UINT210_INT:
        {
            static const VertexFormat format(GL_UNSIGNED_INT_2_10_10_10_REV, GL_FALSE, 4, true);
            return format;
        }
        default:
        {
            static const VertexFormat format(GL_NONE, GL_FALSE, 0, false);
            return format;
        }
    }
}

VertexFormat::VertexFormat(GLenum typeIn, GLboolean normalizedIn, GLuint componentsIn, bool pureIntegerIn)
    : type(typeIn),
      normalized(normalizedIn),
      components(componentsIn),
      pureInteger(pureIntegerIn)
{
    // float -> !normalized
    ASSERT(!(type == GL_FLOAT || type == GL_HALF_FLOAT || type == GL_FIXED) || normalized == GL_FALSE);
}

}
