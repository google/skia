//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of the state class for mananging GLES 3 Vertex Array Objects.
//

#include "libANGLE/VertexAttribute.h"

namespace gl
{

VertexAttribute::VertexAttribute()
    : enabled(false),
      type(GL_FLOAT),
      size(4),
      normalized(false),
      pureInteger(false),
      stride(0),
      pointer(NULL),
      divisor(0)
{
}

size_t ComputeVertexAttributeTypeSize(const VertexAttribute& attrib)
{
    GLuint size = attrib.size;
    switch (attrib.type)
    {
      case GL_BYTE:                        return size * sizeof(GLbyte);
      case GL_UNSIGNED_BYTE:               return size * sizeof(GLubyte);
      case GL_SHORT:                       return size * sizeof(GLshort);
      case GL_UNSIGNED_SHORT:              return size * sizeof(GLushort);
      case GL_INT:                         return size * sizeof(GLint);
      case GL_UNSIGNED_INT:                return size * sizeof(GLuint);
      case GL_INT_2_10_10_10_REV:          return 4;
      case GL_UNSIGNED_INT_2_10_10_10_REV: return 4;
      case GL_FIXED:                       return size * sizeof(GLfixed);
      case GL_HALF_FLOAT:                  return size * sizeof(GLhalf);
      case GL_FLOAT:                       return size * sizeof(GLfloat);
      default: UNREACHABLE();              return size * sizeof(GLfloat);
    }
}

size_t ComputeVertexAttributeStride(const VertexAttribute& attrib)
{
    if (!attrib.enabled)
    {
        return 16;
    }
    return attrib.stride ? attrib.stride : ComputeVertexAttributeTypeSize(attrib);
}

size_t ComputeVertexAttributeElementCount(const VertexAttribute &attrib,
                                          size_t drawCount,
                                          size_t instanceCount)
{
    // For instanced rendering, we draw "instanceDrawCount" sets of "vertexDrawCount" vertices.
    //
    // A vertex attribute with a positive divisor loads one instanced vertex for every set of
    // non-instanced vertices, and the instanced vertex index advances once every "mDivisor"
    // instances.
    if (instanceCount > 0 && attrib.divisor > 0)
    {
        // When instanceDrawCount is not a multiple attrib.divisor, the division must round up.
        // For instance, with 5 non-instanced vertices and a divisor equal to 3, we need 2 instanced
        // vertices.
        return (instanceCount + attrib.divisor - 1u) / attrib.divisor;
    }

    return drawCount;
}
}
