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


#ifndef GrGLVertexBuffer_DEFINED
#define GrGLVertexBuffer_DEFINED

#include "GrVertexBuffer.h"
#include "GrGLConfig.h"

class GrGpuGL;

class GrGLVertexBuffer : public GrVertexBuffer {
protected:
    GrGLVertexBuffer(GLuint id,
                     GrGpuGL* gl,
                     size_t sizeInBytes,
                     bool dynamic);

public:
    virtual ~GrGLVertexBuffer();
    
    // overrides of GrVertexBuffer
    virtual void abandon();
    virtual void* lock();
    virtual void* lockPtr() const;
    virtual void unlock();
    virtual bool isLocked() const;
    virtual bool updateData(const void* src, size_t srcSizeInBytes);
    virtual bool updateSubData(const void* src,  
                               size_t srcSizeInBytes, 
                               size_t offset);
    GLuint bufferID() const;

private:
    void bind() const;
    
    GrGpuGL*     fGL;
    GLuint       fBufferID;
    void*        fLockPtr;

    friend class GrGpuGL;

    typedef GrVertexBuffer INHERITED;
};

#endif
