/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNonAtomicRef_DEFINED
#define GrNonAtomicRef_DEFINED

#include "SkRefCnt.h"
#include "SkTArray.h"

/**
 * A simple non-atomic ref used in the GrBackend when we don't want to pay for the overhead of a
 * threadsafe ref counted object
 */
class GrNonAtomicRef : public SkNoncopyable {
public:
    GrNonAtomicRef() : fRefCnt(1) {}
    virtual ~GrNonAtomicRef() {
        // fRefCnt can be one when a subclass is created statically
        SkASSERT((0 == fRefCnt || 1 == fRefCnt));
        // Set to invalid values.
        SkDEBUGCODE(fRefCnt = -10;)
    }

    void ref() const {
        // Once the ref cnt reaches zero it should never be ref'ed again.
        SkASSERT(fRefCnt > 0);
        ++fRefCnt;
    }

    void unref() const {
        SkASSERT(fRefCnt > 0);
        --fRefCnt;
        if (0 == fRefCnt) {
            SkDELETE(this);
            return;
        }
    }

private:
    mutable int32_t fRefCnt;

    typedef SkNoncopyable INHERITED;
};

#endif
