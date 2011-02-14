/*
    Copyright 2010 Google Inc.

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


#include "GrGLIndexBuffer.h"
#include "GrGpuGL.h"

GrGLIndexBuffer::GrGLIndexBuffer(GLuint id, GrGpuGL* gl, size_t sizeInBytes,
                                   bool dynamic) :
                                INHERITED(sizeInBytes, dynamic),
                                fGL(gl),
                                fBufferID(id),
                                fLockPtr(NULL) {
}

GrGLIndexBuffer::~GrGLIndexBuffer() {
    // make sure we've not been abandoned
    if (fBufferID) {
        fGL->notifyIndexBufferDelete(this);
        GR_GL(DeleteBuffers(1, &fBufferID));
    }
}

void GrGLIndexBuffer::bind() const {
    GR_GL(BindBuffer(GL_ELEMENT_ARRAY_BUFFER, fBufferID));
    fGL->notifyIndexBufferBind(this);
}

GLuint GrGLIndexBuffer::bufferID() const {
    return fBufferID;
}

void GrGLIndexBuffer::abandon() {
    fBufferID = 0;
    fGL = NULL;
    fLockPtr = NULL;
}

void* GrGLIndexBuffer::lock() {
    GrAssert(fBufferID);
    GrAssert(!isLocked());
    if (fGL->supportsBufferLocking()) {
        bind();
        // Let driver know it can discard the old data
        GR_GL(BufferData(GL_ELEMENT_ARRAY_BUFFER, size(), NULL,
                         dynamic() ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
        fLockPtr = GR_GLEXT(fGL->extensions(),
                            MapBuffer(GL_ELEMENT_ARRAY_BUFFER, GR_WRITE_ONLY));

        return fLockPtr;
    }
    return NULL;
}

void* GrGLIndexBuffer::lockPtr() const {
    return fLockPtr;
}

void GrGLIndexBuffer::unlock() {
    GrAssert(fBufferID);
    GrAssert(isLocked());
    GrAssert(fGL->supportsBufferLocking());

    bind();
    GR_GLEXT(fGL->extensions(), UnmapBuffer(GL_ELEMENT_ARRAY_BUFFER));
    fLockPtr = NULL;
}

bool GrGLIndexBuffer::isLocked() const {
    GrAssert(fBufferID);
#if GR_DEBUG
    if (fGL->supportsBufferLocking()) {
        bind();
        GLint mapped;
        GR_GL(GetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER,
                                   GR_BUFFER_MAPPED, &mapped));
        GrAssert(!!mapped == !!fLockPtr);
    }
#endif
    return NULL != fLockPtr;
}

bool GrGLIndexBuffer::updateData(const void* src, size_t srcSizeInBytes) {
    GrAssert(fBufferID);
    GrAssert(!isLocked());
    if (srcSizeInBytes > size()) {
        return false;
    }
    bind();
    GLenum usage = dynamic() ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    if (size() == srcSizeInBytes) {
        GR_GL(BufferData(GL_ELEMENT_ARRAY_BUFFER, srcSizeInBytes, src, usage));
    } else {
        GR_GL(BufferData(GL_ELEMENT_ARRAY_BUFFER, size(), NULL, usage));
        GR_GL(BufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, srcSizeInBytes, src));
    }
    return true;
}

bool GrGLIndexBuffer::updateSubData(const void* src,
                                    size_t srcSizeInBytes,
                                    size_t offset) {
    GrAssert(fBufferID);
    GrAssert(!isLocked());
    if (srcSizeInBytes + offset > size()) {
        return false;
    }
    bind();
    GR_GL(BufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, srcSizeInBytes, src));
    return true;
}

