/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrGLVertexBuffer.h"
#include "GrGpuGL.h"

GrGLVertexBuffer::GrGLVertexBuffer(GLuint id, GrGpuGL* gl, size_t sizeInBytes,
                                   bool dynamic) :
                                   INHERITED(sizeInBytes, dynamic),
                                   fGL(gl),
                                   fBufferID(id),
                                   fLockPtr(NULL) {
}

GrGLVertexBuffer::~GrGLVertexBuffer() {
    // make sure we've not been abandoned
    if (fBufferID) {
        fGL->notifyVertexBufferDelete(this);
        GR_GL(DeleteBuffers(1, &fBufferID));
    }
}

void GrGLVertexBuffer::bind() const {
    GR_GL(BindBuffer(GL_ARRAY_BUFFER, fBufferID));
    fGL->notifyVertexBufferBind(this);
}

GLuint GrGLVertexBuffer::bufferID() const {
    return fBufferID;
}

void GrGLVertexBuffer::abandon() {
    fBufferID = 0;
    fGL = NULL;
    fLockPtr = NULL;
}

void* GrGLVertexBuffer::lock() {
    GrAssert(fBufferID);
    GrAssert(!isLocked());
    if (fGL->supportsBufferLocking()) {
        bind();
        // Let driver know it can discard the old data
        GR_GL(BufferData(GL_ARRAY_BUFFER, size(), NULL,
                         dynamic() ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
        fLockPtr = GR_GL(MapBuffer(GL_ARRAY_BUFFER, GR_WRITE_ONLY));
        return fLockPtr;
    }
    return NULL;
}

void* GrGLVertexBuffer::lockPtr() const {
    return fLockPtr;
}

void GrGLVertexBuffer::unlock() {
    GrAssert(fBufferID);
    GrAssert(isLocked());
    GrAssert(fGL->supportsBufferLocking());

    bind();
    GR_GL(UnmapBuffer(GL_ARRAY_BUFFER));
    fLockPtr = NULL;
}

bool GrGLVertexBuffer::isLocked() const {
    GrAssert(fBufferID);
#if GR_DEBUG
    if (fGL->supportsBufferLocking()) {
        GLint mapped;
        bind();
        GR_GL(GetBufferParameteriv(GL_ARRAY_BUFFER, GR_BUFFER_MAPPED, &mapped));
        GrAssert(!!mapped == !!fLockPtr);
    }
#endif
    return NULL != fLockPtr;
}

bool GrGLVertexBuffer::updateData(const void* src, size_t srcSizeInBytes) {
    GrAssert(fBufferID);
    GrAssert(!isLocked());
    if (srcSizeInBytes > size()) {
        return false;
    }
    bind();
    GLenum usage = dynamic() ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    if (size() == srcSizeInBytes) {
        GR_GL(BufferData(GL_ARRAY_BUFFER, srcSizeInBytes, src, usage));
    } else {
        GR_GL(BufferData(GL_ARRAY_BUFFER, size(), NULL, usage));
        GR_GL(BufferSubData(GL_ARRAY_BUFFER, 0, srcSizeInBytes, src));
    }
    return true;
}

bool GrGLVertexBuffer::updateSubData(const void* src,
                                     size_t srcSizeInBytes,
                                     size_t offset) {
    GrAssert(fBufferID);
    GrAssert(!isLocked());
    if (srcSizeInBytes + offset > size()) {
        return false;
    }
    bind();
    GR_GL(BufferSubData(GL_ARRAY_BUFFER, offset, srcSizeInBytes, src));
    return true;
}

