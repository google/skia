//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutilsgl.cpp: Queries for GL image formats and their translations to native
// GL formats.

#include "libANGLE/renderer/gl/formatutilsgl.h"

#include <limits>

#include "common/string_utils.h"
#include "libANGLE/formatutils.h"

namespace rx
{

namespace nativegl
{

SupportRequirement::SupportRequirement()
    : version(std::numeric_limits<GLuint>::max(), std::numeric_limits<GLuint>::max()),
      versionExtensions(),
      requiredExtensions()
{
}

InternalFormat::InternalFormat()
    : texture(),
      filter(),
      renderbuffer(),
      framebufferAttachment()
{
}

static inline SupportRequirement VersionOrExts(GLuint major, GLuint minor, const std::string &versionExt)
{
    SupportRequirement requirement;
    requirement.version.major = major;
    requirement.version.minor = minor;
    angle::SplitStringAlongWhitespace(versionExt, &requirement.versionExtensions);
    return requirement;
}

static inline SupportRequirement VersionAndExts(GLuint major, GLuint minor, const std::string &requiredExt)
{
    SupportRequirement requirement;
    requirement.version.major = major;
    requirement.version.minor = minor;
    angle::SplitStringAlongWhitespace(requiredExt, &requirement.requiredExtensions);
    return requirement;
}

static inline SupportRequirement VersionOnly(GLuint major, GLuint minor)
{
    SupportRequirement requirement;
    requirement.version.major = major;
    requirement.version.minor = minor;
    return requirement;
}

static inline SupportRequirement ExtsOnly(const std::string &ext)
{
    SupportRequirement requirement;
    requirement.version.major = 0;
    requirement.version.minor = 0;
    angle::SplitStringAlongWhitespace(ext, &requirement.requiredExtensions);
    return requirement;
}

static inline SupportRequirement Always()
{
    SupportRequirement requirement;
    requirement.version.major = 0;
    requirement.version.minor = 0;
    return requirement;
}

static inline SupportRequirement Never()
{
    SupportRequirement requirement;
    requirement.version.major = std::numeric_limits<GLuint>::max();
    requirement.version.minor = std::numeric_limits<GLuint>::max();
    return requirement;
}

struct InternalFormatInfo
{
    InternalFormat glesInfo;
    InternalFormat glInfo;
};

typedef std::pair<GLenum, InternalFormatInfo> InternalFormatInfoPair;
typedef std::map<GLenum, InternalFormatInfo> InternalFormatInfoMap;

// A helper function to insert data into the format map with fewer characters.
static inline void InsertFormatMapping(InternalFormatInfoMap *map, GLenum internalFormat,
                                       const SupportRequirement &desktopTexture, const SupportRequirement &desktopFilter, const SupportRequirement &desktopRender,
                                       const SupportRequirement &esTexture, const SupportRequirement &esFilter, const SupportRequirement &esRender)
{
    InternalFormatInfo formatInfo;
    formatInfo.glInfo.texture = desktopTexture;
    formatInfo.glInfo.filter = desktopFilter;
    formatInfo.glInfo.renderbuffer = desktopRender;
    formatInfo.glInfo.framebufferAttachment = desktopRender;
    formatInfo.glesInfo.texture = esTexture;
    formatInfo.glesInfo.filter = esTexture;
    formatInfo.glesInfo.renderbuffer = esFilter;
    formatInfo.glesInfo.framebufferAttachment = esRender;
    map->insert(std::make_pair(internalFormat, formatInfo));
}

static InternalFormatInfoMap BuildInternalFormatInfoMap()
{
    InternalFormatInfoMap map;

    // clang-format off
    //                       | Format              | OpenGL texture support                          | Filter  | OpenGL render support                        | OpenGL ES texture support                 | Filter  | OpenGL ES render support                 |
    InsertFormatMapping(&map, GL_R8,                VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOrExts(3, 0, "GL_EXT_texture_rg"),   Always(), VersionOrExts(3, 0, "GL_EXT_texture_rg")  );
    InsertFormatMapping(&map, GL_R8_SNORM,          VersionOnly(3, 1),                                Always(), Never(),                                       VersionOnly(3, 0),                          Always(), Never()                                   );
    InsertFormatMapping(&map, GL_RG8,               VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOrExts(3, 0, "GL_EXT_texture_rg"),   Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg")  );
    InsertFormatMapping(&map, GL_RG8_SNORM,         VersionOnly(3, 1),                                Always(), Never(),                                       VersionOnly(3, 0),                          Always(), Never()                                   );
    InsertFormatMapping(&map, GL_RGB8,              Always(),                                         Always(), Always(),                                      VersionOrExts(3, 0, "GL_OES_rgb8_rgba8"),   Always(), Always()                                  );
    InsertFormatMapping(&map, GL_RG8_SNORM,         VersionOnly(3, 1),                                Always(), Never(),                                       VersionOnly(3, 0),                          Always(), Never()                                   );
    InsertFormatMapping(&map, GL_RGB565,            Always(),                                         Always(), Always(),                                      Always(),                                   Always(), Always()                                  );
    InsertFormatMapping(&map, GL_RGBA4,             Always(),                                         Always(), Always(),                                      Always(),                                   Always(), Always()                                  );
    InsertFormatMapping(&map, GL_RGB5_A1,           Always(),                                         Always(), Always(),                                      Always(),                                   Always(), Always()                                  );
    InsertFormatMapping(&map, GL_RGBA8,             Always(),                                         Always(), Always(),                                      VersionOrExts(3, 0, "GL_OES_rgb8_rgba8"),   Always(), VersionOrExts(3, 0, "GL_OES_rgb8_rgba8")  );
    InsertFormatMapping(&map, GL_RGBA8_SNORM,       VersionOnly(3, 1),                                Always(), Never(),                                       VersionOnly(3, 0),                          Always(), Never()                                   );
    InsertFormatMapping(&map, GL_RGB10_A2,          Always(),                                         Always(), Always(),                                      VersionOnly(3, 0),                          Always(), VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RGB10_A2UI,        VersionOrExts(3, 3, "GL_ARB_texture_rgb10_a2ui"), Never(),  Always(),                                      VersionOnly(3, 0),                          Never(),  Always()                                  );
    InsertFormatMapping(&map, GL_SRGB8,             VersionOrExts(2, 1, "GL_EXT_texture_sRGB"),       Always(), VersionOrExts(2, 1, "GL_EXT_texture_sRGB"),    VersionOrExts(3, 0, "GL_EXT_texture_sRGB"), Always(), VersionOrExts(3, 0, "GL_EXT_texture_sRGB"));
    InsertFormatMapping(&map, GL_SRGB8_ALPHA8,      VersionOrExts(2, 1, "GL_EXT_texture_sRGB"),       Always(), VersionOrExts(2, 1, "GL_EXT_texture_sRGB"),    VersionOrExts(3, 0, "GL_EXT_texture_sRGB"), Always(), VersionOrExts(3, 0, "GL_EXT_texture_sRGB"));
    InsertFormatMapping(&map, GL_R8I,               VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_R8UI,              VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_R16I,              VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_R16UI,             VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_R32I,              VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_R32UI,             VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RG8I,              VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RG8UI,             VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RG16I,             VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RG16UI,            VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RG32I,             VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RG32UI,            VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RGB8I,             VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  Never(),                                       VersionOnly(3, 0),                          Never(),  Never()                                   );
    InsertFormatMapping(&map, GL_RGB8UI,            VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  Never(),                                       VersionOnly(3, 0),                          Never(),  Never()                                   );
    InsertFormatMapping(&map, GL_RGB16I,            VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  Never(),                                       VersionOnly(3, 0),                          Never(),  Never()                                   );
    InsertFormatMapping(&map, GL_RGB16UI,           VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  Never(),                                       VersionOnly(3, 0),                          Never(),  Never()                                   );
    InsertFormatMapping(&map, GL_RGB32I,            VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  Never(),                                       VersionOnly(3, 0),                          Never(),  Never()                                   );
    InsertFormatMapping(&map, GL_RGB32UI,           VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  Never(),                                       VersionOnly(3, 0),                          Never(),  Never()                                   );
    InsertFormatMapping(&map, GL_RGBA8I,            VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  VersionOrExts(3, 0, "GL_EXT_texture_integer"), VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RGBA8UI,           VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  VersionOrExts(3, 0, "GL_EXT_texture_integer"), VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RGBA16I,           VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  VersionOrExts(3, 0, "GL_EXT_texture_integer"), VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RGBA16UI,          VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  VersionOrExts(3, 0, "GL_EXT_texture_integer"), VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RGBA32I,           VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  VersionOrExts(3, 0, "GL_EXT_texture_integer"), VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RGBA32UI,          VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  VersionOrExts(3, 0, "GL_EXT_texture_integer"), VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );

    // Unsized formats
    InsertFormatMapping(&map, GL_ALPHA,             Never(),                                          Never(),  Never(),                                       Never(),                                    Never(),  Never()                                   );
    InsertFormatMapping(&map, GL_LUMINANCE,         Never(),                                          Never(),  Never(),                                       Never(),                                    Never(),  Never()                                   );
    InsertFormatMapping(&map, GL_LUMINANCE_ALPHA,   Never(),                                          Never(),  Never(),                                       Never(),                                    Never(),  Never()                                   );
    InsertFormatMapping(&map, GL_RED,               VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOrExts(3, 0, "GL_EXT_texture_rg"),   Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg")  );
    InsertFormatMapping(&map, GL_RG,                VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOrExts(3, 0, "GL_EXT_texture_rg"),   Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg")  );
    InsertFormatMapping(&map, GL_RGB,               Always(),                                         Always(), Always(),                                      Always(),                                   Always(), Always()                                  );
    InsertFormatMapping(&map, GL_RGBA,              Always(),                                         Always(), Always(),                                      Always(),                                   Always(), Always()                                  );
    InsertFormatMapping(&map, GL_RED_INTEGER,       VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RG_INTEGER,        VersionOrExts(3, 0, "GL_ARB_texture_rg"),         Never(),  VersionOrExts(3, 0, "GL_ARB_texture_rg"),      VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_RGB_INTEGER,       VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  Never(),                                       VersionOnly(3, 0),                          Never(),  Never()                                   );
    InsertFormatMapping(&map, GL_RGBA_INTEGER,      VersionOrExts(3, 0, "GL_EXT_texture_integer"),    Never(),  VersionOrExts(3, 0, "GL_EXT_texture_integer"), VersionOnly(3, 0),                          Never(),  VersionOnly(3, 0)                         );
    InsertFormatMapping(&map, GL_SRGB,              VersionOrExts(2, 1, "GL_EXT_texture_sRGB"),       Always(), VersionOrExts(2, 1, "GL_EXT_texture_sRGB"),    VersionOrExts(3, 0, "GL_EXT_texture_sRGB"), Always(), VersionOrExts(3, 0, "GL_EXT_texture_sRGB"));
    InsertFormatMapping(&map, GL_SRGB_ALPHA,        VersionOrExts(2, 1, "GL_EXT_texture_sRGB"),       Always(), VersionOrExts(2, 1, "GL_EXT_texture_sRGB"),    VersionOrExts(3, 0, "GL_EXT_texture_sRGB"), Always(), VersionOrExts(3, 0, "GL_EXT_texture_sRGB"));

    // From GL_EXT_texture_format_BGRA8888
    InsertFormatMapping(&map, GL_BGRA8_EXT,         VersionOrExts(1, 2, "GL_EXT_bgra"),               Always(), VersionOrExts(1, 2, "GL_EXT_bgra"),            ExtsOnly("GL_EXT_texture_format_BGRA8888"), Always(), ExtsOnly("GL_EXT_texture_format_BGRA8888"));
    InsertFormatMapping(&map, GL_BGRA_EXT,          VersionOrExts(1, 2, "GL_EXT_bgra"),               Always(), VersionOrExts(1, 2, "GL_EXT_bgra"),            ExtsOnly("GL_EXT_texture_format_BGRA8888"), Always(), ExtsOnly("GL_EXT_texture_format_BGRA8888"));

    // Floating point formats
    //                       | Format              | OpenGL texture support                                       | Filter  | OpenGL render support                                                                  | OpenGL ES texture support                                         | Filter                                                 | OpenGL ES render support                                         |
    InsertFormatMapping(&map, GL_R11F_G11F_B10F,    VersionOrExts(3, 0, "GL_EXT_packed_float"),                    Always(), VersionOrExts(3, 0, "GL_EXT_packed_float GL_ARB_color_buffer_float"),                    VersionOnly(3, 0),                                                  Always(),                                                VersionAndExts(3, 0, "GL_EXT_color_buffer_float")                 );
    InsertFormatMapping(&map, GL_RGB9_E5,           VersionOrExts(3, 0, "GL_EXT_texture_shared_exponent"),         Always(), VersionOrExts(3, 0, "GL_EXT_texture_shared_exponent GL_ARB_color_buffer_float"),         VersionOnly(3, 0),                                                  Always(),                                                VersionAndExts(3, 0, "GL_EXT_color_buffer_float")                 );
    InsertFormatMapping(&map, GL_R16F,              VersionOrExts(3, 0, "GL_ARB_texture_rg ARB_texture_float"),    Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg GL_ARB_texture_float GL_ARB_color_buffer_float"), VersionOrExts(3, 0, "GL_OES_texture_half_float GL_EXT_texture_rg"), VersionOrExts(3, 0, "GL_OES_texture_half_float_linear"), VersionOrExts(3, 0, "GL_OES_texture_half_float GL_EXT_texture_rg"));
    InsertFormatMapping(&map, GL_RG16F,             VersionOrExts(3, 0, "GL_ARB_texture_rg ARB_texture_float"),    Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg GL_ARB_texture_float GL_ARB_color_buffer_float"), VersionOrExts(3, 0, "GL_OES_texture_half_float GL_EXT_texture_rg"), VersionOrExts(3, 0, "GL_OES_texture_half_float_linear"), VersionOrExts(3, 0, "GL_OES_texture_half_float GL_EXT_texture_rg"));
    InsertFormatMapping(&map, GL_RGB16F,            VersionOrExts(3, 0, "GL_ARB_texture_float"),                   Always(), VersionOrExts(3, 0, "GL_ARB_texture_float GL_ARB_color_buffer_float"),                   VersionOrExts(3, 0, "GL_OES_texture_half_float"),                   VersionOrExts(3, 0, "GL_OES_texture_half_float_linear"), VersionOrExts(3, 0, "GL_OES_texture_half_float")                  );
    InsertFormatMapping(&map, GL_RGBA16F,           VersionOrExts(3, 0, "GL_ARB_texture_float"),                   Always(), VersionOrExts(3, 0, "GL_ARB_texture_float GL_ARB_color_buffer_float"),                   VersionOrExts(3, 0, "GL_OES_texture_half_float"),                   VersionOrExts(3, 0, "GL_OES_texture_half_float_linear"), VersionOrExts(3, 0, "GL_OES_texture_half_float")                  );
    InsertFormatMapping(&map, GL_R32F,              VersionOrExts(3, 0, "GL_ARB_texture_rg GL_ARB_texture_float"), Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg GL_ARB_texture_float GL_ARB_color_buffer_float"), VersionOrExts(3, 0, "GL_OES_texture_float GL_EXT_texture_rg"),      VersionOrExts(3, 0, "GL_OES_texture_float_linear"),      VersionOrExts(3, 0, "GL_OES_texture_float GL_EXT_texture_rg")     );
    InsertFormatMapping(&map, GL_RG32F,             VersionOrExts(3, 0, "GL_ARB_texture_rg GL_ARB_texture_float"), Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg GL_ARB_texture_float GL_ARB_color_buffer_float"), VersionOrExts(3, 0, "GL_OES_texture_float GL_EXT_texture_rg"),      VersionOrExts(3, 0, "GL_OES_texture_float_linear"),      VersionOrExts(3, 0, "GL_OES_texture_float GL_EXT_texture_rg")     );
    InsertFormatMapping(&map, GL_RGB32F,            VersionOrExts(3, 0, "GL_ARB_texture_float"),                   Always(), VersionOrExts(3, 0, "GL_ARB_texture_float GL_ARB_color_buffer_float"),                   VersionOrExts(3, 0, "GL_OES_texture_float"),                        VersionOrExts(3, 0, "GL_OES_texture_float_linear"),      VersionOrExts(3, 0, "GL_OES_texture_float")                       );
    InsertFormatMapping(&map, GL_RGBA32F,           VersionOrExts(3, 0, "GL_ARB_texture_float"),                   Always(), VersionOrExts(3, 0, "GL_ARB_texture_float GL_ARB_color_buffer_float"),                   VersionOrExts(3, 0, "GL_OES_texture_float"),                        VersionOrExts(3, 0, "GL_OES_texture_float_linear"),      VersionOrExts(3, 0, "GL_OES_texture_float")                       );

    // Depth stencil formats
    //                       | Format                  | OpenGL texture support                            | Filter                                     | OpenGL render support                             | OpenGL ES texture support                  | Filter                                     | OpenGL ES render support                                              |
    InsertFormatMapping(&map, GL_DEPTH_COMPONENT16,     VersionOnly(1, 5),                                  VersionOrExts(1, 5, "GL_ARB_depth_texture"), VersionOnly(1, 5),                                  VersionOnly(2, 0),                           VersionOrExts(3, 0, "GL_OES_depth_texture"), VersionOnly(2, 0)                                                      );
    InsertFormatMapping(&map, GL_DEPTH_COMPONENT24,     VersionOnly(1, 5),                                  VersionOrExts(1, 5, "GL_ARB_depth_texture"), VersionOnly(1, 5),                                  VersionOnly(2, 0),                           VersionOrExts(3, 0, "GL_OES_depth_texture"), VersionOnly(2, 0)                                                      );
    InsertFormatMapping(&map, GL_DEPTH_COMPONENT32_OES, VersionOnly(1, 5),                                  VersionOrExts(1, 5, "GL_ARB_depth_texture"), VersionOnly(1, 5),                                  ExtsOnly("GL_OES_depth_texture"),            Always(),                                    ExtsOnly("GL_OES_depth_texture")                                       );
    InsertFormatMapping(&map, GL_DEPTH_COMPONENT32F,    VersionOrExts(3, 0, "GL_ARB_depth_buffer_float"),   Always(),                                    VersionOrExts(3, 0, "GL_ARB_depth_buffer_float"),   VersionOnly(3, 0),                           VersionOrExts(3, 0, "GL_OES_depth_texture"), VersionOnly(3, 0)                                                      );
    InsertFormatMapping(&map, GL_STENCIL_INDEX8,        VersionOrExts(3, 0, "GL_EXT_packed_depth_stencil"), Never(),                                     VersionOrExts(3, 0, "GL_EXT_packed_depth_stencil"), VersionOnly(2, 0),                           Never(),                                     VersionOnly(2, 0)                                                      );
    InsertFormatMapping(&map, GL_DEPTH24_STENCIL8,      VersionOrExts(3, 0, "GL_ARB_framebuffer_object"),   VersionOrExts(3, 0, "GL_ARB_depth_texture"), VersionOrExts(3, 0, "GL_ARB_framebuffer_object"),   VersionOrExts(3, 0, "GL_OES_depth_texture"), Always(),                                    VersionOrExts(3, 0, "GL_OES_depth_texture GL_OES_packed_depth_stencil"));
    InsertFormatMapping(&map, GL_DEPTH32F_STENCIL8,     VersionOrExts(3, 0, "GL_ARB_depth_buffer_float"),   Always(),                                    VersionOrExts(3, 0, "GL_ARB_depth_buffer_float"),   VersionOnly(3, 0),                           Always(),                                    VersionOnly(3, 0)                                                      );
    InsertFormatMapping(&map, GL_DEPTH_COMPONENT,       VersionOnly(1, 5),                                  VersionOrExts(1, 5, "GL_ARB_depth_texture"), VersionOnly(1, 5),                                  VersionOnly(2, 0),                           VersionOrExts(3, 0, "GL_OES_depth_texture"), VersionOnly(2, 0)                                                      );
    InsertFormatMapping(&map, GL_DEPTH_STENCIL,         VersionOnly(1, 5),                                  VersionOrExts(1, 5, "GL_ARB_depth_texture"), VersionOnly(1, 5),                                  VersionOnly(2, 0),                           VersionOrExts(3, 0, "GL_OES_depth_texture"), VersionOnly(2, 0)                                                      );

    // Luminance alpha formats
    //                       | Format                  | OpenGL texture support                      | Filter  | Render | OpenGL ES texture support         | Filter  | Render |
    InsertFormatMapping(&map, GL_ALPHA8_EXT,             Always(),                                    Always(), Never(), ExtsOnly("GL_EXT_texture_storage"), Always(), Never());
    InsertFormatMapping(&map, GL_LUMINANCE8_EXT,         Always(),                                    Always(), Never(), ExtsOnly("GL_EXT_texture_storage"), Always(), Never());
    InsertFormatMapping(&map, GL_ALPHA32F_EXT,           VersionOrExts(3, 0, "GL_ARB_texture_float"), Always(), Never(), ExtsOnly("GL_EXT_texture_storage"), Always(), Never());
    InsertFormatMapping(&map, GL_LUMINANCE32F_EXT,       VersionOrExts(3, 0, "GL_ARB_texture_float"), Always(), Never(), ExtsOnly("GL_EXT_texture_storage"), Always(), Never());
    InsertFormatMapping(&map, GL_ALPHA16F_EXT,           VersionOrExts(3, 0, "GL_ARB_texture_float"), Always(), Never(), ExtsOnly("GL_EXT_texture_storage"), Always(), Never());
    InsertFormatMapping(&map, GL_LUMINANCE16F_EXT,       VersionOrExts(3, 0, "GL_ARB_texture_float"), Always(), Never(), ExtsOnly("GL_EXT_texture_storage"), Always(), Never());
    InsertFormatMapping(&map, GL_LUMINANCE8_ALPHA8_EXT,  Always(),                                    Always(), Never(), ExtsOnly("GL_EXT_texture_storage"), Always(), Never());
    InsertFormatMapping(&map, GL_LUMINANCE_ALPHA32F_EXT, VersionOrExts(3, 0, "GL_ARB_texture_float"), Always(), Never(), ExtsOnly("GL_EXT_texture_storage"), Always(), Never());
    InsertFormatMapping(&map, GL_LUMINANCE_ALPHA16F_EXT, VersionOrExts(3, 0, "GL_ARB_texture_float"), Always(), Never(), ExtsOnly("GL_EXT_texture_storage"), Always(), Never());

    // Compressed formats, From ES 3.0.1 spec, table 3.16
    InsertFormatMapping(&map, GL_COMPRESSED_R11_EAC,                        VersionOrExts(4, 3, "GL_ARB_ES3_compatibility"), Always(), Never(), VersionOnly(3, 0), Always(), Never());
    InsertFormatMapping(&map, GL_COMPRESSED_SIGNED_R11_EAC,                 VersionOrExts(4, 3, "GL_ARB_ES3_compatibility"), Always(), Never(), VersionOnly(3, 0), Always(), Never());
    InsertFormatMapping(&map, GL_COMPRESSED_RG11_EAC,                       VersionOrExts(4, 3, "GL_ARB_ES3_compatibility"), Always(), Never(), VersionOnly(3, 0), Always(), Never());
    InsertFormatMapping(&map, GL_COMPRESSED_SIGNED_RG11_EAC,                VersionOrExts(4, 3, "GL_ARB_ES3_compatibility"), Always(), Never(), VersionOnly(3, 0), Always(), Never());
    InsertFormatMapping(&map, GL_COMPRESSED_RGB8_ETC2,                      VersionOrExts(4, 3, "GL_ARB_ES3_compatibility"), Always(), Never(), VersionOnly(3, 0), Always(), Never());
    InsertFormatMapping(&map, GL_COMPRESSED_SRGB8_ETC2,                     VersionOrExts(4, 3, "GL_ARB_ES3_compatibility"), Always(), Never(), VersionOnly(3, 0), Always(), Never());
    InsertFormatMapping(&map, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,  VersionOrExts(4, 3, "GL_ARB_ES3_compatibility"), Always(), Never(), VersionOnly(3, 0), Always(), Never());
    InsertFormatMapping(&map, GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, VersionOrExts(4, 3, "GL_ARB_ES3_compatibility"), Always(), Never(), VersionOnly(3, 0), Always(), Never());
    InsertFormatMapping(&map, GL_COMPRESSED_RGBA8_ETC2_EAC,                 VersionOrExts(4, 3, "GL_ARB_ES3_compatibility"), Always(), Never(), VersionOnly(3, 0), Always(), Never());
    InsertFormatMapping(&map, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,          VersionOrExts(4, 3, "GL_ARB_ES3_compatibility"), Always(), Never(), VersionOnly(3, 0), Always(), Never());

    // From GL_EXT_texture_compression_dxt1
    //                       | Format                            | OpenGL texture support                     | Filter  | Render | OpenGL ES texture support                    | Filter  | Render |
    InsertFormatMapping(&map, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,    ExtsOnly("GL_EXT_texture_compression_s3tc"), Always(), Never(), ExtsOnly("GL_EXT_texture_compression_dxt1"),   Always(), Never());
    InsertFormatMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   ExtsOnly("GL_EXT_texture_compression_s3tc"), Always(), Never(), ExtsOnly("GL_EXT_texture_compression_dxt1"),   Always(), Never());

    // From GL_ANGLE_texture_compression_dxt3
    InsertFormatMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE, ExtsOnly("GL_EXT_texture_compression_s3tc"), Always(), Never(), ExtsOnly("GL_ANGLE_texture_compression_dxt3"), Always(), Never());

    // From GL_ANGLE_texture_compression_dxt5
    InsertFormatMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE, ExtsOnly("GL_EXT_texture_compression_s3tc"), Always(), Never(), ExtsOnly("GL_ANGLE_texture_compression_dxt5"), Always(), Never());
    // clang-format on

    return map;
}

static const InternalFormatInfoMap &GetInternalFormatMap()
{
    static const InternalFormatInfoMap formatMap = BuildInternalFormatInfoMap();
    return formatMap;
}

const InternalFormat &GetInternalFormatInfo(GLenum internalFormat, StandardGL standard)
{
    const InternalFormatInfoMap &formatMap = GetInternalFormatMap();
    InternalFormatInfoMap::const_iterator iter = formatMap.find(internalFormat);
    if (iter != formatMap.end())
    {
        const InternalFormatInfo &info = iter->second;
        switch (standard)
        {
          case STANDARD_GL_ES:      return info.glesInfo;
          case STANDARD_GL_DESKTOP: return info.glInfo;
          default: UNREACHABLE();   break;
        }
    }

    static const InternalFormat defaultInternalFormat;
    return defaultInternalFormat;
}

static GLenum GetNativeInternalFormat(const FunctionsGL *functions,
                                      const WorkaroundsGL &workarounds,
                                      GLenum internalFormat,
                                      GLenum sizedInternalFormat)
{
    GLenum result = internalFormat;

    if (functions->standard == STANDARD_GL_DESKTOP)
    {
        // Use sized internal formats whenever possible to guarantee the requested precision.
        // On Desktop GL, passing an internal format of GL_RGBA will generate a GL_RGBA8 texture
        // even if the provided type is GL_FLOAT.
        result = sizedInternalFormat;

        const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(sizedInternalFormat);

        if (workarounds.avoid1BitAlphaTextureFormats && formatInfo.alphaBits == 1)
        {
            // Use an 8-bit format instead
            result = GL_RGBA8;
        }

        if (workarounds.rgba4IsNotSupportedForColorRendering && sizedInternalFormat == GL_RGBA4)
        {
            // Use an 8-bit format instead
            result = GL_RGBA8;
        }

        if (sizedInternalFormat == GL_RGB565 && !functions->isAtLeastGL(gl::Version(4, 1)) &&
            !functions->hasGLExtension("GL_ARB_ES2_compatibility"))
        {
            // GL_RGB565 is required for basic ES2 functionality but was not added to desktop GL
            // until 4.1.
            // Work around this by using an 8-bit format instead.
            result = GL_RGB8;
        }

        if (sizedInternalFormat == GL_BGRA8_EXT)
        {
            // GLES accepts GL_BGRA as an internal format but desktop GL only accepts it as a type.
            // Update the internal format to GL_RGBA.
            result = GL_RGBA8;
        }

        if ((functions->profile & GL_CONTEXT_CORE_PROFILE_BIT) != 0)
        {
            // Work around deprecated luminance alpha formats in the OpenGL core profile by backing
            // them with R or RG textures.
            if (formatInfo.format == GL_LUMINANCE || formatInfo.format == GL_ALPHA)
            {
                result = gl::GetSizedInternalFormat(GL_RED, formatInfo.type);
            }

            if (formatInfo.format == GL_LUMINANCE_ALPHA)
            {
                result = gl::GetSizedInternalFormat(GL_RG, formatInfo.type);
            }
        }
    }
    else if (functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        result = sizedInternalFormat;
    }

    return result;
}

static GLenum GetNativeFormat(const FunctionsGL *functions,
                              const WorkaroundsGL &workarounds,
                              GLenum format)
{
    GLenum result = format;

    if (functions->standard == STANDARD_GL_DESKTOP)
    {
        // The ES SRGB extensions require that the provided format is GL_SRGB or SRGB_ALPHA but
        // the desktop GL extensions only accept GL_RGB or GL_RGBA.  Convert them.
        if (format == GL_SRGB)
        {
            result = GL_RGB;
        }

        if (format == GL_SRGB_ALPHA)
        {
            result = GL_RGBA;
        }

        if ((functions->profile & GL_CONTEXT_CORE_PROFILE_BIT) != 0)
        {
            // Work around deprecated luminance alpha formats in the OpenGL core profile by backing
            // them with R or RG textures.
            if (format == GL_LUMINANCE || format == GL_ALPHA)
            {
                result = GL_RED;
            }

            if (format == GL_LUMINANCE_ALPHA)
            {
                result = GL_RG;
            }
        }
    }

    return result;
}

static GLenum GetNativeType(const FunctionsGL *functions,
                            const WorkaroundsGL &workarounds,
                            GLenum type)
{
    GLenum result = type;

    if (functions->standard == STANDARD_GL_DESKTOP)
    {
        if (type == GL_HALF_FLOAT_OES)
        {
            // The enums differ for the OES half float extensions and desktop GL spec. Update it.
            result = GL_HALF_FLOAT;
        }
    }

    if (functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        if (type == GL_HALF_FLOAT_OES)
        {
            // The enums differ for the OES half float extensions and ES 3 spec. Update it.
            result = GL_HALF_FLOAT;
        }
    }

    return result;
}

TexImageFormat GetTexImageFormat(const FunctionsGL *functions,
                                 const WorkaroundsGL &workarounds,
                                 GLenum internalFormat,
                                 GLenum format,
                                 GLenum type)
{
    TexImageFormat result;
    result.internalFormat = GetNativeInternalFormat(
        functions, workarounds, internalFormat, gl::GetSizedInternalFormat(internalFormat, type));
    result.format = GetNativeFormat(functions, workarounds, format);
    result.type   = GetNativeType(functions, workarounds, type);
    return result;
}

TexSubImageFormat GetTexSubImageFormat(const FunctionsGL *functions,
                                       const WorkaroundsGL &workarounds,
                                       GLenum format,
                                       GLenum type)
{
    TexSubImageFormat result;
    result.format = GetNativeFormat(functions, workarounds, format);
    result.type   = GetNativeType(functions, workarounds, type);
    return result;
}

CompressedTexImageFormat GetCompressedTexImageFormat(const FunctionsGL *functions,
                                                     const WorkaroundsGL &workarounds,
                                                     GLenum internalFormat)
{
    CompressedTexImageFormat result;
    result.internalFormat = internalFormat;
    return result;
}

CompressedTexSubImageFormat GetCompressedSubTexImageFormat(const FunctionsGL *functions,
                                                           const WorkaroundsGL &workarounds,
                                                           GLenum format)
{
    CompressedTexSubImageFormat result;
    result.format = format;
    return result;
}

CopyTexImageImageFormat GetCopyTexImageImageFormat(const FunctionsGL *functions,
                                                   const WorkaroundsGL &workarounds,
                                                   GLenum internalFormat,
                                                   GLenum framebufferType)
{
    CopyTexImageImageFormat result;
    result.internalFormat =
        GetNativeInternalFormat(functions, workarounds, internalFormat,
                                gl::GetSizedInternalFormat(internalFormat, framebufferType));
    return result;
}

TexStorageFormat GetTexStorageFormat(const FunctionsGL *functions,
                                     const WorkaroundsGL &workarounds,
                                     GLenum internalFormat)
{
    TexStorageFormat result;
    result.internalFormat =
        GetNativeInternalFormat(functions, workarounds, internalFormat, internalFormat);
    return result;
}

RenderbufferFormat GetRenderbufferFormat(const FunctionsGL *functions,
                                         const WorkaroundsGL &workarounds,
                                         GLenum internalFormat)
{
    RenderbufferFormat result;
    result.internalFormat =
        GetNativeInternalFormat(functions, workarounds, internalFormat, internalFormat);
    return result;
}
}

}
