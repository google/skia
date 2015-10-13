//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexArrayGL.h: Defines the class interface for VertexArrayGL.

#ifndef LIBANGLE_RENDERER_GL_VERTEXARRAYGL_H_
#define LIBANGLE_RENDERER_GL_VERTEXARRAYGL_H_

#include "libANGLE/renderer/VertexArrayImpl.h"

namespace rx
{

class FunctionsGL;
class StateManagerGL;

class VertexArrayGL : public VertexArrayImpl
{
  public:
    VertexArrayGL(const gl::VertexArray::Data &data, const FunctionsGL *functions, StateManagerGL *stateManager);
    ~VertexArrayGL() override;

    gl::Error syncDrawArraysState(const gl::AttributesMask &activeAttributesMask,
                                  GLint first,
                                  GLsizei count,
                                  GLsizei instanceCount) const;
    gl::Error syncDrawElementsState(const gl::AttributesMask &activeAttributesMask,
                                    GLsizei count,
                                    GLenum type,
                                    const GLvoid *indices,
                                    GLsizei instanceCount,
                                    bool primitiveRestartEnabled,
                                    const GLvoid **outIndices) const;

    GLuint getVertexArrayID() const;
    GLuint getAppliedElementArrayBufferID() const;

    void syncState(const gl::VertexArray::DirtyBits &dirtyBits) override;

  private:
    gl::Error syncDrawState(const gl::AttributesMask &activeAttributesMask,
                            GLint first,
                            GLsizei count,
                            GLenum type,
                            const GLvoid *indices,
                            GLsizei instanceCount,
                            bool primitiveRestartEnabled,
                            const GLvoid **outIndices) const;

    // Apply index data, only sets outIndexRange if attributesNeedStreaming is true
    gl::Error syncIndexData(GLsizei count,
                            GLenum type,
                            const GLvoid *indices,
                            bool primitiveRestartEnabled,
                            bool attributesNeedStreaming,
                            gl::IndexRange *outIndexRange,
                            const GLvoid **outIndices) const;

    // Returns the amount of space needed to stream all attributes that need streaming
    // and the data size of the largest attribute
    void computeStreamingAttributeSizes(const gl::AttributesMask &activeAttributesMask,
                                        GLsizei instanceCount,
                                        const gl::IndexRange &indexRange,
                                        size_t *outStreamingDataSize,
                                        size_t *outMaxAttributeDataSize) const;

    // Stream attributes that have client data
    gl::Error streamAttributes(const gl::AttributesMask &activeAttributesMask,
                               GLsizei instanceCount,
                               const gl::IndexRange &indexRange) const;

    void updateNeedsStreaming(size_t attribIndex);
    void updateAttribEnabled(size_t attribIndex);
    void updateAttribPointer(size_t attribIndex);

    const FunctionsGL *mFunctions;
    StateManagerGL *mStateManager;

    GLuint mVertexArrayID;

    mutable BindingPointer<gl::Buffer> mAppliedElementArrayBuffer;
    mutable std::vector<gl::VertexAttribute> mAppliedAttributes;

    mutable size_t mStreamingElementArrayBufferSize;
    mutable GLuint mStreamingElementArrayBuffer;

    mutable size_t mStreamingArrayBufferSize;
    mutable GLuint mStreamingArrayBuffer;

    gl::AttributesMask mAttributesNeedStreaming;
};

}

#endif // LIBANGLE_RENDERER_GL_VERTEXARRAYGL_H_
