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


#include "SkData.h"

SkData::SkData(const void* ptr, size_t size, ReleaseProc proc, void* context) {
    fPtr = ptr;
    fSize = size;
    fReleaseProc = proc;
    fReleaseProcContext = context;
}

SkData::~SkData() {
    if (fReleaseProc) {
        fReleaseProc(fPtr, fSize, fReleaseProcContext);
    }
}

size_t SkData::copyRange(size_t offset, size_t length, void* buffer) const {
    size_t available = fSize;
    if (offset >= available || 0 == length) {
        return 0;
    }
    available -= offset;
    if (length > available) {
        length = available;
    }
    SkASSERT(length > 0);

    memcpy(buffer, this->bytes() + offset, length);
    return length;
}

///////////////////////////////////////////////////////////////////////////////

SkData* SkData::NewEmpty() {
    static SkData* gEmptyRef;
    if (NULL == gEmptyRef) {
        gEmptyRef = new SkData(NULL, 0, NULL, NULL);
    }
    gEmptyRef->ref();
    return gEmptyRef;
}

// assumes fPtr was allocated via sk_malloc
static void sk_free_releaseproc(const void* ptr, size_t, void*) {
    sk_free((void*)ptr);
}

SkData* SkData::NewFromMalloc(const void* data, size_t length) {
    return new SkData(data, length, sk_free_releaseproc, NULL);
}

SkData* SkData::NewWithCopy(const void* data, size_t length) {
    if (0 == length) {
        return SkData::NewEmpty();
    }

    void* copy = sk_malloc_throw(length); // balanced in sk_free_releaseproc
    memcpy(copy, data, length);
    return new SkData(copy, length, sk_free_releaseproc, NULL);
}

SkData* SkData::NewWithProc(const void* data, size_t length,
                            ReleaseProc proc, void* context) {
    return new SkData(data, length, proc, context);
}

// assumes context is a SkData
static void sk_dataref_releaseproc(const void*, size_t, void* context) {
    SkData* src = reinterpret_cast<SkData*>(context);
    src->unref();
}

SkData* SkData::NewSubset(const SkData* src, size_t offset, size_t length) {
    /*
        We could, if we wanted/need to, just make a deep copy of src's data,
        rather than referencing it. This would duplicate the storage (of the
        subset amount) but would possibly allow src to go out of scope sooner.
     */

    size_t available = src->size();
    if (offset >= available || 0 == length) {
        return SkData::NewEmpty();
    }
    available -= offset;
    if (length > available) {
        length = available;
    }
    SkASSERT(length > 0);

    src->ref(); // this will be balanced in sk_dataref_releaseproc
    return new SkData(src->bytes() + offset, length, sk_dataref_releaseproc,
                         const_cast<SkData*>(src));
}

