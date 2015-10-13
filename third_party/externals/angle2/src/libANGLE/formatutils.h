//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutils.h: Queries for GL image formats.

#ifndef LIBANGLE_FORMATUTILS_H_
#define LIBANGLE_FORMATUTILS_H_

#include "libANGLE/Caps.h"
#include "libANGLE/angletypes.h"

#include "angle_gl.h"

#include <cstddef>
#include <stdint.h>

namespace gl
{

struct Type
{
    Type();

    GLuint bytes;
    GLuint bytesShift; // Bit shift by this value to effectively divide/multiply by "bytes" in a more optimal way
    bool specialInterpretation;
};
const Type &GetTypeInfo(GLenum type);

struct InternalFormat
{
    InternalFormat();

    GLuint redBits;
    GLuint greenBits;
    GLuint blueBits;

    GLuint luminanceBits;

    GLuint alphaBits;
    GLuint sharedBits;

    GLuint depthBits;
    GLuint stencilBits;

    GLuint pixelBytes;

    GLuint componentCount;

    bool compressed;
    GLuint compressedBlockWidth;
    GLuint compressedBlockHeight;

    GLenum format;
    GLenum type;

    GLenum componentType;
    GLenum colorEncoding;

    typedef bool (*SupportCheckFunction)(GLuint, const Extensions &);
    SupportCheckFunction textureSupport;
    SupportCheckFunction renderSupport;
    SupportCheckFunction filterSupport;

    GLuint computeRowPitch(GLenum formatType, GLsizei width, GLint alignment, GLint rowLength) const;
    GLuint computeDepthPitch(GLenum formatType, GLsizei width, GLsizei height, GLint alignment, GLint rowLength) const;
    GLuint computeBlockSize(GLenum formatType, GLsizei width, GLsizei height) const;
};
const InternalFormat &GetInternalFormatInfo(GLenum internalFormat);

GLenum GetSizedInternalFormat(GLenum internalFormat, GLenum type);

typedef std::set<GLenum> FormatSet;
const FormatSet &GetAllSizedInternalFormats();

// From the ESSL 3.00.4 spec:
// Vertex shader inputs can only be float, floating-point vectors, matrices, signed and unsigned
// integers and integer vectors. Vertex shader inputs cannot be arrays or structures.

enum AttributeType
{
    ATTRIBUTE_FLOAT,
    ATTRIBUTE_VEC2,
    ATTRIBUTE_VEC3,
    ATTRIBUTE_VEC4,
    ATTRIBUTE_INT,
    ATTRIBUTE_IVEC2,
    ATTRIBUTE_IVEC3,
    ATTRIBUTE_IVEC4,
    ATTRIBUTE_UINT,
    ATTRIBUTE_UVEC2,
    ATTRIBUTE_UVEC3,
    ATTRIBUTE_UVEC4,
    ATTRIBUTE_MAT2,
    ATTRIBUTE_MAT3,
    ATTRIBUTE_MAT4,
    ATTRIBUTE_MAT2x3,
    ATTRIBUTE_MAT2x4,
    ATTRIBUTE_MAT3x2,
    ATTRIBUTE_MAT3x4,
    ATTRIBUTE_MAT4x2,
    ATTRIBUTE_MAT4x3,
};

AttributeType GetAttributeType(GLenum enumValue);

enum VertexFormatType
{
    VERTEX_FORMAT_INVALID,
    VERTEX_FORMAT_SBYTE1,
    VERTEX_FORMAT_SBYTE1_NORM,
    VERTEX_FORMAT_SBYTE2,
    VERTEX_FORMAT_SBYTE2_NORM,
    VERTEX_FORMAT_SBYTE3,
    VERTEX_FORMAT_SBYTE3_NORM,
    VERTEX_FORMAT_SBYTE4,
    VERTEX_FORMAT_SBYTE4_NORM,
    VERTEX_FORMAT_UBYTE1,
    VERTEX_FORMAT_UBYTE1_NORM,
    VERTEX_FORMAT_UBYTE2,
    VERTEX_FORMAT_UBYTE2_NORM,
    VERTEX_FORMAT_UBYTE3,
    VERTEX_FORMAT_UBYTE3_NORM,
    VERTEX_FORMAT_UBYTE4,
    VERTEX_FORMAT_UBYTE4_NORM,
    VERTEX_FORMAT_SSHORT1,
    VERTEX_FORMAT_SSHORT1_NORM,
    VERTEX_FORMAT_SSHORT2,
    VERTEX_FORMAT_SSHORT2_NORM,
    VERTEX_FORMAT_SSHORT3,
    VERTEX_FORMAT_SSHORT3_NORM,
    VERTEX_FORMAT_SSHORT4,
    VERTEX_FORMAT_SSHORT4_NORM,
    VERTEX_FORMAT_USHORT1,
    VERTEX_FORMAT_USHORT1_NORM,
    VERTEX_FORMAT_USHORT2,
    VERTEX_FORMAT_USHORT2_NORM,
    VERTEX_FORMAT_USHORT3,
    VERTEX_FORMAT_USHORT3_NORM,
    VERTEX_FORMAT_USHORT4,
    VERTEX_FORMAT_USHORT4_NORM,
    VERTEX_FORMAT_SINT1,
    VERTEX_FORMAT_SINT1_NORM,
    VERTEX_FORMAT_SINT2,
    VERTEX_FORMAT_SINT2_NORM,
    VERTEX_FORMAT_SINT3,
    VERTEX_FORMAT_SINT3_NORM,
    VERTEX_FORMAT_SINT4,
    VERTEX_FORMAT_SINT4_NORM,
    VERTEX_FORMAT_UINT1,
    VERTEX_FORMAT_UINT1_NORM,
    VERTEX_FORMAT_UINT2,
    VERTEX_FORMAT_UINT2_NORM,
    VERTEX_FORMAT_UINT3,
    VERTEX_FORMAT_UINT3_NORM,
    VERTEX_FORMAT_UINT4,
    VERTEX_FORMAT_UINT4_NORM,
    VERTEX_FORMAT_SBYTE1_INT,
    VERTEX_FORMAT_SBYTE2_INT,
    VERTEX_FORMAT_SBYTE3_INT,
    VERTEX_FORMAT_SBYTE4_INT,
    VERTEX_FORMAT_UBYTE1_INT,
    VERTEX_FORMAT_UBYTE2_INT,
    VERTEX_FORMAT_UBYTE3_INT,
    VERTEX_FORMAT_UBYTE4_INT,
    VERTEX_FORMAT_SSHORT1_INT,
    VERTEX_FORMAT_SSHORT2_INT,
    VERTEX_FORMAT_SSHORT3_INT,
    VERTEX_FORMAT_SSHORT4_INT,
    VERTEX_FORMAT_USHORT1_INT,
    VERTEX_FORMAT_USHORT2_INT,
    VERTEX_FORMAT_USHORT3_INT,
    VERTEX_FORMAT_USHORT4_INT,
    VERTEX_FORMAT_SINT1_INT,
    VERTEX_FORMAT_SINT2_INT,
    VERTEX_FORMAT_SINT3_INT,
    VERTEX_FORMAT_SINT4_INT,
    VERTEX_FORMAT_UINT1_INT,
    VERTEX_FORMAT_UINT2_INT,
    VERTEX_FORMAT_UINT3_INT,
    VERTEX_FORMAT_UINT4_INT,
    VERTEX_FORMAT_FIXED1,
    VERTEX_FORMAT_FIXED2,
    VERTEX_FORMAT_FIXED3,
    VERTEX_FORMAT_FIXED4,
    VERTEX_FORMAT_HALF1,
    VERTEX_FORMAT_HALF2,
    VERTEX_FORMAT_HALF3,
    VERTEX_FORMAT_HALF4,
    VERTEX_FORMAT_FLOAT1,
    VERTEX_FORMAT_FLOAT2,
    VERTEX_FORMAT_FLOAT3,
    VERTEX_FORMAT_FLOAT4,
    VERTEX_FORMAT_SINT210,
    VERTEX_FORMAT_UINT210,
    VERTEX_FORMAT_SINT210_NORM,
    VERTEX_FORMAT_UINT210_NORM,
    VERTEX_FORMAT_SINT210_INT,
    VERTEX_FORMAT_UINT210_INT,
};

typedef std::vector<gl::VertexFormatType> InputLayout;

struct VertexFormat : angle::NonCopyable
{
    VertexFormat(GLenum typeIn, GLboolean normalizedIn, GLuint componentsIn, bool pureIntegerIn);

    GLenum type;
    GLboolean normalized;
    GLuint components;
    bool pureInteger;
};

VertexFormatType GetVertexFormatType(GLenum type, GLboolean normalized, GLuint components, bool pureInteger);
VertexFormatType GetVertexFormatType(const VertexAttribute &attrib);
VertexFormatType GetVertexFormatType(const VertexAttribute &attrib, GLenum currentValueType);
const VertexFormat &GetVertexFormatFromType(VertexFormatType vertexFormatType);

}

#endif // LIBANGLE_FORMATUTILS_H_
