//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Helper structure describing a single vertex attribute
//

#ifndef LIBANGLE_VERTEXATTRIBUTE_H_
#define LIBANGLE_VERTEXATTRIBUTE_H_

#include "libANGLE/Buffer.h"

namespace gl
{

struct VertexAttribute
{
    bool enabled; // From glEnable/DisableVertexAttribArray

    GLenum type;
    GLuint size;
    bool normalized;
    bool pureInteger;
    GLuint stride; // 0 means natural stride

    union
    {
        const GLvoid *pointer;
        GLintptr offset;
    };
    BindingPointer<Buffer> buffer; // Captured when glVertexAttribPointer is called.

    GLuint divisor;

    VertexAttribute();
};

bool operator==(const VertexAttribute &a, const VertexAttribute &b);
bool operator!=(const VertexAttribute &a, const VertexAttribute &b);

template <typename T>
T QuerySingleVertexAttributeParameter(const VertexAttribute& attrib, GLenum pname);

size_t ComputeVertexAttributeTypeSize(const VertexAttribute& attrib);
size_t ComputeVertexAttributeStride(const VertexAttribute& attrib);
size_t ComputeVertexAttributeElementCount(const VertexAttribute &attrib,
                                          size_t drawCount,
                                          size_t instanceCount);

struct VertexAttribCurrentValueData
{
    union
    {
        GLfloat FloatValues[4];
        GLint IntValues[4];
        GLuint UnsignedIntValues[4];
    };
    GLenum Type;

    VertexAttribCurrentValueData();

    void setFloatValues(const GLfloat floatValues[4]);
    void setIntValues(const GLint intValues[4]);
    void setUnsignedIntValues(const GLuint unsignedIntValues[4]);
};

bool operator==(const VertexAttribCurrentValueData &a, const VertexAttribCurrentValueData &b);
bool operator!=(const VertexAttribCurrentValueData &a, const VertexAttribCurrentValueData &b);

}

#include "VertexAttribute.inl"

#endif // LIBANGLE_VERTEXATTRIBUTE_H_
