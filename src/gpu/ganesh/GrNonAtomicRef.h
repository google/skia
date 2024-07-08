/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNonAtomicRef_DEFINED
#define GrNonAtomicRef_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/base/SkNoncopyable.h"

#include <cstdint>

/**
 * A simple non-atomic ref used in the GrBackendApi when we don't want to pay for the overhead of a
 * threadsafe ref counted object
 */
template<typename TSubclass> class GrNonAtomicRef : public SkNoncopyable {
public:
    GrNonAtomicRef() : fRefCnt(1) {}

#ifdef SK_DEBUG
    ~GrNonAtomicRef() {
        // fRefCnt can be one when a subclass is created statically
        SkASSERT((0 == fRefCnt || 1 == fRefCnt));
        // Set to invalid values.
        fRefCnt = -10;
    }
#endif

    bool unique() const { return 1 == fRefCnt; }

    // We allow this getter because this type is not thread-safe, meaning only one thread should
    // have ownership and be manipulating the ref count or querying this.
    int refCnt() const { return fRefCnt; }

    void ref() const {
        // Once the ref cnt reaches zero it should never be ref'ed again.
        SkASSERT(fRefCnt > 0);
        ++fRefCnt;
    }

    void unref() const {
        SkASSERT(fRefCnt > 0);
        --fRefCnt;
        if (0 == fRefCnt) {
            GrTDeleteNonAtomicRef(static_cast<const TSubclass*>(this));
            return;
        }
    }

private:
    mutable int32_t fRefCnt;

    using INHERITED = SkNoncopyable;
};

template<typename T> inline void GrTDeleteNonAtomicRef(const T* ref) {
    delete ref;
}

#endif
