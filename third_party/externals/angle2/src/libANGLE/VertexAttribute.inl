//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VertexAttribute.inl: Inline vertex attribute methods
//

namespace gl
{

inline bool operator==(const VertexAttribute &a, const VertexAttribute &b)
{
    return a.enabled == b.enabled &&
           a.type == b.type &&
           a.size == b.size &&
           a.normalized == b.normalized &&
           a.pureInteger == b.pureInteger &&
           a.stride == b.stride &&
           a.pointer == b.pointer &&
           a.buffer.get() == b.buffer.get() &&
           a.divisor == b.divisor;
}

inline bool operator!=(const VertexAttribute &a, const VertexAttribute &b)
{
    return !(a == b);
}

template <typename T>
T QuerySingleVertexAttributeParameter(const VertexAttribute& attrib, GLenum pname)
{
  switch (pname)
  {
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
      return static_cast<T>(attrib.enabled ? GL_TRUE : GL_FALSE);
    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
      return static_cast<T>(attrib.size);
    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
      return static_cast<T>(attrib.stride);
    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
      return static_cast<T>(attrib.type);
    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
      return static_cast<T>(attrib.normalized ? GL_TRUE : GL_FALSE);
    case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
      return static_cast<T>(attrib.buffer.id());
    case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
      return static_cast<T>(attrib.divisor);
    case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
      return static_cast<T>(attrib.pureInteger ? GL_TRUE : GL_FALSE);
    default:
      UNREACHABLE();
      return static_cast<T>(0);
  }
}

inline VertexAttribCurrentValueData::VertexAttribCurrentValueData()
    : Type(GL_FLOAT)
{
    FloatValues[0] = 0.0f;
    FloatValues[1] = 0.0f;
    FloatValues[2] = 0.0f;
    FloatValues[3] = 1.0f;
}

inline void VertexAttribCurrentValueData::setFloatValues(const GLfloat floatValues[4])
{
    for (unsigned int valueIndex = 0; valueIndex < 4; valueIndex++)
    {
        FloatValues[valueIndex] = floatValues[valueIndex];
    }
    Type = GL_FLOAT;
}

inline void VertexAttribCurrentValueData::setIntValues(const GLint intValues[4])
{
    for (unsigned int valueIndex = 0; valueIndex < 4; valueIndex++)
    {
        IntValues[valueIndex] = intValues[valueIndex];
    }
    Type = GL_INT;
}

inline void VertexAttribCurrentValueData::setUnsignedIntValues(const GLuint unsignedIntValues[4])
{
    for (unsigned int valueIndex = 0; valueIndex < 4; valueIndex++)
    {
        UnsignedIntValues[valueIndex] = unsignedIntValues[valueIndex];
    }
    Type = GL_UNSIGNED_INT;
}

inline bool operator==(const VertexAttribCurrentValueData &a, const VertexAttribCurrentValueData &b)
{
    return (a.Type == b.Type && memcmp(a.FloatValues, b.FloatValues, sizeof(float) * 4) == 0);
}

inline bool operator!=(const VertexAttribCurrentValueData &a, const VertexAttribCurrentValueData &b)
{
    return !(a == b);
}

}
