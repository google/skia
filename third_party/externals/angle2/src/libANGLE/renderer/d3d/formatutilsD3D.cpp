//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutils9.cpp: Queries for GL image formats and their translations to D3D
// formats.

#include "libANGLE/renderer/d3d/formatutilsD3D.h"

#include <map>

#include "common/debug.h"
#include "libANGLE/renderer/d3d/imageformats.h"
#include "libANGLE/renderer/d3d/copyimage.h"

namespace rx
{

typedef std::pair<GLenum, GLenum> FormatTypePair;
typedef std::pair<FormatTypePair, ColorWriteFunction> FormatWriteFunctionPair;
typedef std::map<FormatTypePair, ColorWriteFunction> FormatWriteFunctionMap;

static inline void InsertFormatWriteFunctionMapping(FormatWriteFunctionMap *map, GLenum format, GLenum type,
                                                    ColorWriteFunction writeFunc)
{
    map->insert(FormatWriteFunctionPair(FormatTypePair(format, type), writeFunc));
}

static FormatWriteFunctionMap BuildFormatWriteFunctionMap()
{
    FormatWriteFunctionMap map;

    //                                    | Format               | Type                             | Color write function             |
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_UNSIGNED_BYTE,                  WriteColor<R8G8B8A8, GLfloat>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_BYTE,                           WriteColor<R8G8B8A8S, GLfloat>    );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_UNSIGNED_SHORT_4_4_4_4,         WriteColor<R4G4B4A4, GLfloat>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_UNSIGNED_SHORT_5_5_5_1,         WriteColor<R5G5B5A1, GLfloat>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_UNSIGNED_INT_2_10_10_10_REV,    WriteColor<R10G10B10A2, GLfloat>  );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_FLOAT,                          WriteColor<R32G32B32A32F, GLfloat>);
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_HALF_FLOAT,                     WriteColor<R16G16B16A16F, GLfloat>);
    InsertFormatWriteFunctionMapping(&map, GL_RGBA,               GL_HALF_FLOAT_OES,                 WriteColor<R16G16B16A16F, GLfloat>);

    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_UNSIGNED_BYTE,                  WriteColor<R8G8B8A8, GLuint>      );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_BYTE,                           WriteColor<R8G8B8A8S, GLint>      );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_UNSIGNED_SHORT,                 WriteColor<R16G16B16A16, GLuint>  );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_SHORT,                          WriteColor<R16G16B16A16S, GLint>  );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_UNSIGNED_INT,                   WriteColor<R32G32B32A32, GLuint>  );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_INT,                            WriteColor<R32G32B32A32S, GLint>  );
    InsertFormatWriteFunctionMapping(&map, GL_RGBA_INTEGER,       GL_UNSIGNED_INT_2_10_10_10_REV,    WriteColor<R10G10B10A2, GLuint>   );

    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_UNSIGNED_BYTE,                  WriteColor<R8G8B8, GLfloat>       );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_BYTE,                           WriteColor<R8G8B8S, GLfloat>      );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_UNSIGNED_SHORT_5_6_5,           WriteColor<R5G6B5, GLfloat>       );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_UNSIGNED_INT_10F_11F_11F_REV,   WriteColor<R11G11B10F, GLfloat>   );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_UNSIGNED_INT_5_9_9_9_REV,       WriteColor<R9G9B9E5, GLfloat>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_FLOAT,                          WriteColor<R32G32B32F, GLfloat>   );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_HALF_FLOAT,                     WriteColor<R16G16B16F, GLfloat>   );
    InsertFormatWriteFunctionMapping(&map, GL_RGB,                GL_HALF_FLOAT_OES,                 WriteColor<R16G16B16F, GLfloat>   );

    InsertFormatWriteFunctionMapping(&map, GL_RGB_INTEGER,        GL_UNSIGNED_BYTE,                  WriteColor<R8G8B8, GLuint>        );
    InsertFormatWriteFunctionMapping(&map, GL_RGB_INTEGER,        GL_BYTE,                           WriteColor<R8G8B8S, GLint>        );
    InsertFormatWriteFunctionMapping(&map, GL_RGB_INTEGER,        GL_UNSIGNED_SHORT,                 WriteColor<R16G16B16, GLuint>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGB_INTEGER,        GL_SHORT,                          WriteColor<R16G16B16S, GLint>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGB_INTEGER,        GL_UNSIGNED_INT,                   WriteColor<R32G32B32, GLuint>     );
    InsertFormatWriteFunctionMapping(&map, GL_RGB_INTEGER,        GL_INT,                            WriteColor<R32G32B32S, GLint>     );

    InsertFormatWriteFunctionMapping(&map, GL_RG,                 GL_UNSIGNED_BYTE,                  WriteColor<R8G8, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_RG,                 GL_BYTE,                           WriteColor<R8G8S, GLfloat>        );
    InsertFormatWriteFunctionMapping(&map, GL_RG,                 GL_FLOAT,                          WriteColor<R32G32F, GLfloat>      );
    InsertFormatWriteFunctionMapping(&map, GL_RG,                 GL_HALF_FLOAT,                     WriteColor<R16G16F, GLfloat>      );
    InsertFormatWriteFunctionMapping(&map, GL_RG,                 GL_HALF_FLOAT_OES,                 WriteColor<R16G16F, GLfloat>      );

    InsertFormatWriteFunctionMapping(&map, GL_RG_INTEGER,         GL_UNSIGNED_BYTE,                  WriteColor<R8G8, GLuint>          );
    InsertFormatWriteFunctionMapping(&map, GL_RG_INTEGER,         GL_BYTE,                           WriteColor<R8G8S, GLint>          );
    InsertFormatWriteFunctionMapping(&map, GL_RG_INTEGER,         GL_UNSIGNED_SHORT,                 WriteColor<R16G16, GLuint>        );
    InsertFormatWriteFunctionMapping(&map, GL_RG_INTEGER,         GL_SHORT,                          WriteColor<R16G16S, GLint>        );
    InsertFormatWriteFunctionMapping(&map, GL_RG_INTEGER,         GL_UNSIGNED_INT,                   WriteColor<R32G32, GLuint>        );
    InsertFormatWriteFunctionMapping(&map, GL_RG_INTEGER,         GL_INT,                            WriteColor<R32G32S, GLint>        );

    InsertFormatWriteFunctionMapping(&map, GL_RED,                GL_UNSIGNED_BYTE,                  WriteColor<R8, GLfloat>           );
    InsertFormatWriteFunctionMapping(&map, GL_RED,                GL_BYTE,                           WriteColor<R8S, GLfloat>          );
    InsertFormatWriteFunctionMapping(&map, GL_RED,                GL_FLOAT,                          WriteColor<R32F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_RED,                GL_HALF_FLOAT,                     WriteColor<R16F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_RED,                GL_HALF_FLOAT_OES,                 WriteColor<R16F, GLfloat>         );

    InsertFormatWriteFunctionMapping(&map, GL_RED_INTEGER,        GL_UNSIGNED_BYTE,                  WriteColor<R8, GLuint>            );
    InsertFormatWriteFunctionMapping(&map, GL_RED_INTEGER,        GL_BYTE,                           WriteColor<R8S, GLint>            );
    InsertFormatWriteFunctionMapping(&map, GL_RED_INTEGER,        GL_UNSIGNED_SHORT,                 WriteColor<R16, GLuint>           );
    InsertFormatWriteFunctionMapping(&map, GL_RED_INTEGER,        GL_SHORT,                          WriteColor<R16S, GLint>           );
    InsertFormatWriteFunctionMapping(&map, GL_RED_INTEGER,        GL_UNSIGNED_INT,                   WriteColor<R32, GLuint>           );
    InsertFormatWriteFunctionMapping(&map, GL_RED_INTEGER,        GL_INT,                            WriteColor<R32S, GLint>           );

    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE_ALPHA,    GL_UNSIGNED_BYTE,                  WriteColor<L8A8, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE,          GL_UNSIGNED_BYTE,                  WriteColor<L8, GLfloat>           );
    InsertFormatWriteFunctionMapping(&map, GL_ALPHA,              GL_UNSIGNED_BYTE,                  WriteColor<A8, GLfloat>           );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE_ALPHA,    GL_FLOAT,                          WriteColor<L32A32F, GLfloat>      );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE,          GL_FLOAT,                          WriteColor<L32F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_ALPHA,              GL_FLOAT,                          WriteColor<A32F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE_ALPHA,    GL_HALF_FLOAT,                     WriteColor<L16A16F, GLfloat>      );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE_ALPHA,    GL_HALF_FLOAT_OES,                 WriteColor<L16A16F, GLfloat>      );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE,          GL_HALF_FLOAT,                     WriteColor<L16F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_LUMINANCE,          GL_HALF_FLOAT_OES,                 WriteColor<L16F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_ALPHA,              GL_HALF_FLOAT,                     WriteColor<A16F, GLfloat>         );
    InsertFormatWriteFunctionMapping(&map, GL_ALPHA,              GL_HALF_FLOAT_OES,                 WriteColor<A16F, GLfloat>         );

    InsertFormatWriteFunctionMapping(&map, GL_BGRA_EXT,           GL_UNSIGNED_BYTE,                  WriteColor<B8G8R8A8, GLfloat>     );
    InsertFormatWriteFunctionMapping(&map, GL_BGRA_EXT,           GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT, WriteColor<A4R4G4B4, GLfloat>     );
    InsertFormatWriteFunctionMapping(&map, GL_BGRA_EXT,           GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT, WriteColor<A1R5G5B5, GLfloat>     );

    InsertFormatWriteFunctionMapping(&map, GL_SRGB_EXT,           GL_UNSIGNED_BYTE,                  WriteColor<R8G8B8, GLfloat>       );
    InsertFormatWriteFunctionMapping(&map, GL_SRGB_ALPHA_EXT,     GL_UNSIGNED_BYTE,                  WriteColor<R8G8B8A8, GLfloat>     );

    InsertFormatWriteFunctionMapping(&map, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,    GL_UNSIGNED_BYTE,     NULL                              );
    InsertFormatWriteFunctionMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   GL_UNSIGNED_BYTE,     NULL                              );
    InsertFormatWriteFunctionMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE, GL_UNSIGNED_BYTE,     NULL                              );
    InsertFormatWriteFunctionMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE, GL_UNSIGNED_BYTE,     NULL                              );

    InsertFormatWriteFunctionMapping(&map, GL_DEPTH_COMPONENT,    GL_UNSIGNED_SHORT,                 NULL                              );
    InsertFormatWriteFunctionMapping(&map, GL_DEPTH_COMPONENT,    GL_UNSIGNED_INT,                   NULL                              );
    InsertFormatWriteFunctionMapping(&map, GL_DEPTH_COMPONENT,    GL_FLOAT,                          NULL                              );

    InsertFormatWriteFunctionMapping(&map, GL_STENCIL,            GL_UNSIGNED_BYTE,                  NULL                              );

    InsertFormatWriteFunctionMapping(&map, GL_DEPTH_STENCIL,      GL_UNSIGNED_INT_24_8,              NULL                              );
    InsertFormatWriteFunctionMapping(&map, GL_DEPTH_STENCIL,      GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL                              );

    return map;
}

ColorWriteFunction GetColorWriteFunction(GLenum format, GLenum type)
{
    static const FormatWriteFunctionMap formatTypeMap = BuildFormatWriteFunctionMap();
    FormatWriteFunctionMap::const_iterator iter = formatTypeMap.find(FormatTypePair(format, type));
    ASSERT(iter != formatTypeMap.end());
    if (iter != formatTypeMap.end())
    {
        return iter->second;
    }
    else
    {
        return NULL;
    }
}

}
