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

#ifndef SkSkTScopedPtr_DEFINED
#define SkSkTScopedPtr_DEFINED

#include "SkTemplates.h"

template<typename T>
class SkTScopedComPtr : SkNoncopyable {
private:
    T *fPtr;

public:
    explicit SkTScopedComPtr(T *ptr = NULL) : fPtr(ptr) { }
    ~SkTScopedComPtr() {
        if (NULL != fPtr) {
            fPtr->Release();
            fPtr = NULL;
        }
    }
    T &operator*() const { return *fPtr; }
    T *operator->() const { return fPtr; }
    /**
     * Returns the address of the underlying pointer.
     * This is dangerous -- it breaks encapsulation and the reference escapes.
     * Must only be used on instances currently pointing to NULL,
     * and only to initialize the instance.
     */
    T **operator&() { SkASSERT(fPtr == NULL); return &fPtr; }
    T *get() const { return fPtr; }
};

#endif
