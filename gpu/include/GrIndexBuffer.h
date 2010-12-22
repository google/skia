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


#ifndef GrIndexBuffer_DEFINED
#define GrIndexBuffer_DEFINED

#include "GrRefCnt.h"

class GrIndexBuffer : public GrRefCnt {
protected:
    GrIndexBuffer(uint32_t sizeInBytes, bool dynamic) : 
                  fSizeInBytes(sizeInBytes),
                  fDynamic(dynamic) {}
public:
    virtual ~GrIndexBuffer() {}
    
    /**
     Retrieves the size of the index buffer

     @return the size of the index buffer in bytes
     */
    uint32_t size() const { return fSizeInBytes; }
   
    /**
     Retrieves whether the index buffer was created with the dynamic flag

     @return true if the index buffer was created with the dynamic flag
     */
    bool dynamic() const { return fDynamic; }

    /**
     Indicates that GPU context in which this veretx buffer was created is 
     destroyed and that Ganesh should not attempt to free the texture with the
     underlying API.
     */
    virtual void abandon() = 0;

    /**
     Locks the index buffer to be written by the CPU.
     
     The previous content of the index buffer is invalidated. It is an error to
     draw whil the buffer is locked.  It is an error to call lock on an already
     locked index buffer.
     
     @return a pointer to the index data or NULL if the lock fails.
     */
    virtual void* lock() = 0;

    /** 
     Unlocks the index buffer. 
     
     The pointer returned by the previous lock call will no longer be valid.
     */
    virtual void unlock() = 0;

    /** 
     Queries whether the index buffer has been locked.
     
     @return true if the index buffer is locked, false otherwise.
     */
    virtual bool isLocked() const = 0;

    /**
     Updates the index buffer data. 
     
     The size of the index buffer will be preserved. However, only the updated 
     region will have defined contents.

     @return returns true if the update succeeds, false otherwise.
     */
    virtual bool updateData(const void* src, uint32_t srcSizeInBytes) = 0;

private:
    uint32_t fSizeInBytes;
    bool     fDynamic;
};

#endif
