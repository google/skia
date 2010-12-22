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


#include "GrGLVertexBuffer.h"
#include "GrGpuGL.h"

GrGLVertexBuffer::GrGLVertexBuffer(GLuint id, GrGpuGL* gl, uint32_t sizeInBytes,
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
        GR_GL(BindBuffer(GL_ARRAY_BUFFER, fBufferID));
        fGL->notifyVertexBufferBind(this);
        // call bufferData with null ptr to allow driver to perform renaming
        // If this call is removed revisit updateData to be sure it doesn't
        // leave buffer undersized (as it currently does).
        GR_GL(BufferData(GL_ARRAY_BUFFER, size(), NULL, 
                         dynamic() ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
        fLockPtr = GR_GLEXT(fGL->extensions(),
                            MapBuffer(GL_ARRAY_BUFFER, GR_WRITE_ONLY)); 
        return fLockPtr;
    }
    return NULL;
}

void GrGLVertexBuffer::unlock() {
    GrAssert(fBufferID);
    GrAssert(isLocked());
    if (fGL->supportsBufferLocking()) {
        GR_GL(BindBuffer(GL_ARRAY_BUFFER, fBufferID));
        fGL->notifyVertexBufferBind(this);
        GR_GLEXT(fGL->extensions(),
                 UnmapBuffer(GL_ARRAY_BUFFER));
        fLockPtr = NULL;
    }
}

bool GrGLVertexBuffer::isLocked() const {
    GrAssert(fBufferID);
#if GR_DEBUG
    if (fGL->supportsBufferLocking()) {
        GLint mapped;
        GR_GL(BindBuffer(GL_ARRAY_BUFFER, fBufferID));
        fGL->notifyVertexBufferBind(this);
        GR_GL(GetBufferParameteriv(GL_ARRAY_BUFFER, GR_BUFFER_MAPPED, &mapped));
        GrAssert(!!mapped == !!fLockPtr);
    }
#endif
    return NULL != fLockPtr;
}

bool GrGLVertexBuffer::updateData(const void* src, uint32_t srcSizeInBytes) {
    GrAssert(fBufferID);
    GrAssert(!isLocked());
    if (srcSizeInBytes > size()) {
        return false;
    }
    GR_GL(BindBuffer(GL_ARRAY_BUFFER, fBufferID));
    fGL->notifyVertexBufferBind(this);
    GR_GL(BufferData(GL_ARRAY_BUFFER, srcSizeInBytes, src, 
                     dynamic() ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
    return true;
}

