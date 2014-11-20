/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGeometryData_DEFINED
#define GrGeometryData_DEFINED

#include <new>
#include "SkTypes.h"

/*
 * A super lightweight base class for GeometryProcessor's to use to store draw data in a reorderable
 * fashion.  Its most important feature is a pool allocator.  Its virtual, but only so subclasses
 * will have their destructors called.
 */

class GrGeometryData : SkNoncopyable {
public:
    virtual ~GrGeometryData() {}

    /**
      * Helper for down-casting to a GrGeometryData subclass
      */
    template <typename T> const T& cast() const { return *static_cast<const T*>(this); }

    void* operator new(size_t size);

    void operator delete(void* target);

    void* operator new(size_t size, void* placement) {
        return ::operator new(size, placement);
    }

    void operator delete(void* target, void* placement) {
        ::operator delete(target, placement);
    }
};

#endif
