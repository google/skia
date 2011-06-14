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


#ifndef SkDataRef_DEFINED
#define SkDataRef_DEFINED

#include "SkRefCnt.h"

/**
 *  SkDataRef holds an immutable data buffer. Not only is the data immutable,
 *  but the actual ptr that is returned (by data() or bytes()) is guaranteed
 *  to always be the same for the life of this instance.
 */
class SkDataRef : public SkRefCnt {
public:
    /**
     *  Returns the number of bytes stored.
     */
    size_t size() const { return fSize; }

    /**
     *  Returns the ptr to the data.
     */
    const void* data() const { return fPtr; }

    /**
     *  Like data(), returns a read-only ptr into the data, but in this case
     *  it is cast to uint8_t*, to make it easy to add an offset to it.
     */
    const uint8_t* bytes() const {
        return reinterpret_cast<const uint8_t*>(fPtr);
    }

    /**
     *  Helper to copy a range of the data into a caller-provided buffer.
     *  Returns the actual number of bytes copied, after clamping offset and
     *  length to the size of the data. If buffer is NULL, it is ignored, and
     *  only the computed number of bytes is returned.
     */
    size_t copyRange(size_t offset, size_t length, void* buffer) const;

    /**
     *  Function that, if provided, will be called when the SkDataRef goes out
     *  of scope, allowing for custom allocation/freeing of the data.
     */
    typedef void (*ReleaseProc)(const void* ptr, size_t length, void* context);
    
    /**
     *  Create a new dataref by copying the specified data
     */
    static SkDataRef* NewWithCopy(const void* data, size_t length);

    /**
     *  Create a new dataref, taking the data ptr as is, and using the
     *  releaseproc to free it. The proc may be NULL.
     */
    static SkDataRef* NewWithProc(const void* data, size_t length,
                                  ReleaseProc proc, void* context);

    /**
     *  Create a new dataref using a subset of the data in the specified
     *  src dataref.
     */
    static SkDataRef* NewSubset(const SkDataRef* src, size_t offset, size_t length);

    /**
     *  Returns a new empty dataref (or a reference to a shared empty dataref).
     *  New or shared, the caller must see that unref() is eventually called.
     */
    static SkDataRef* NewEmpty();

private:
    ReleaseProc fReleaseProc;
    void*       fReleaseProcContext;

    const void* fPtr;
    size_t      fSize;

    SkDataRef(const void* ptr, size_t size, ReleaseProc, void* context);
    ~SkDataRef();
};

#endif
