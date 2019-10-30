/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
*/

#ifndef GrResourceHandle_DEFINED
#define GrResourceHandle_DEFINED

#include "include/core/SkTypes.h"

// Opaque handle to a resource. Users should always use the macro below to create a specific
// template instantiation of GrResourceHandle.
template <typename kind> class GrResourceHandle {
public:
    GrResourceHandle(int value) : fValue(value) {
        SkASSERT(this->isValid());
    }

    GrResourceHandle() : fValue(kInvalid_ResourceHandle) {}

    bool operator==(const GrResourceHandle& other) const { return other.fValue == fValue; }
    bool isValid() const { return kInvalid_ResourceHandle != fValue; }
    int toIndex() const { SkASSERT(this->isValid()); return fValue; }

private:
    static const int kInvalid_ResourceHandle = -1;
    int fValue;
};

// Creates a type "name", which is a specfic template instantiation of GrResourceHandle.
#define GR_DEFINE_RESOURCE_HANDLE_CLASS(name) \
    struct name##Kind {};  \
    using name = GrResourceHandle<name##Kind>;
#endif
