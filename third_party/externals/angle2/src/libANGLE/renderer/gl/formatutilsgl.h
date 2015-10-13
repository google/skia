//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutilsgl.h: Queries for GL image formats and their translations to native
// GL formats.

#ifndef LIBANGLE_RENDERER_GL_FORMATUTILSGL_H_
#define LIBANGLE_RENDERER_GL_FORMATUTILSGL_H_

#include <map>
#include <string>
#include <vector>

#include "angle_gl.h"
#include "libANGLE/Version.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/WorkaroundsGL.h"

namespace rx
{

namespace nativegl
{

struct SupportRequirement
{
    SupportRequirement();

    // Version that this format became supported without extensions
    gl::Version version;

    // Extensions that are required if the minimum version is not met
    std::vector<std::string> versionExtensions;

    // Extensions that are always required to support this format
    std::vector<std::string> requiredExtensions;
};

struct InternalFormat
{
    InternalFormat();

    SupportRequirement texture;
    SupportRequirement filter;
    SupportRequirement renderbuffer;
    SupportRequirement framebufferAttachment;
};
const InternalFormat &GetInternalFormatInfo(GLenum internalFormat, StandardGL standard);

struct TexImageFormat
{
    GLenum internalFormat;
    GLenum format;
    GLenum type;
};
TexImageFormat GetTexImageFormat(const FunctionsGL *functions,
                                 const WorkaroundsGL &workarounds,
                                 GLenum internalFormat,
                                 GLenum format,
                                 GLenum type);

struct TexSubImageFormat
{
    GLenum format;
    GLenum type;
};
TexSubImageFormat GetTexSubImageFormat(const FunctionsGL *functions,
                                       const WorkaroundsGL &workarounds,
                                       GLenum format,
                                       GLenum type);

struct CompressedTexImageFormat
{
    GLenum internalFormat;
};
CompressedTexImageFormat GetCompressedTexImageFormat(const FunctionsGL *functions,
                                                     const WorkaroundsGL &workarounds,
                                                     GLenum internalFormat);

struct CompressedTexSubImageFormat
{
    GLenum format;
};
CompressedTexSubImageFormat GetCompressedSubTexImageFormat(const FunctionsGL *functions,
                                                           const WorkaroundsGL &workarounds,
                                                           GLenum format);

struct CopyTexImageImageFormat
{
    GLenum internalFormat;
};
CopyTexImageImageFormat GetCopyTexImageImageFormat(const FunctionsGL *functions,
                                                   const WorkaroundsGL &workarounds,
                                                   GLenum internalFormat,
                                                   GLenum framebufferType);

struct TexStorageFormat
{
    GLenum internalFormat;
};
TexStorageFormat GetTexStorageFormat(const FunctionsGL *functions,
                                     const WorkaroundsGL &workarounds,
                                     GLenum internalFormat);

struct RenderbufferFormat
{
    GLenum internalFormat;
};
RenderbufferFormat GetRenderbufferFormat(const FunctionsGL *functions,
                                         const WorkaroundsGL &workarounds,
                                         GLenum internalFormat);
}

}

#endif // LIBANGLE_RENDERER_GL_FORMATUTILSGL_H_
