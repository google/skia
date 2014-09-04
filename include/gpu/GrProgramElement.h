/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramElement_DEFINED
#define GrProgramElement_DEFINED

#include "SkRefCnt.h"
#include "SkTArray.h"

class GrProgramResource;

/**
 * Base class for GrEffect (and future GrGeometryProcessor). GrDrawState uses this to manage
 * transitioning a GrEffect from being owned by a client to being scheduled for execution. It
 * converts resources owned by the effect from being ref'ed to having pending reads/writes.
 *
 * All GrGpuResource objects owned by a GrProgramElement or derived classes (either directly or
 * indirectly) must be wrapped in a GrProgramResource and registered with the GrProgramElement using
 * addGrProgramResource(). This allows the regular refs to be converted to pending IO events
 * when the program element is scheduled for deferred execution.
 */
class GrProgramElement : public SkNoncopyable {
public:
    SK_DECLARE_INST_COUNT_ROOT(GrProgramElement)

    virtual ~GrProgramElement() {
        // fRefCnt can be one when an effect is created statically using GR_CREATE_STATIC_EFFECT
        SkASSERT((0 == fRefCnt || 1 == fRefCnt) && 0 == fPendingExecutions);
        // Set to invalid values.
        SkDEBUGCODE(fRefCnt = fPendingExecutions = -10;)
    }

    void ref() const {
        // Once the ref cnt reaches zero it should never be ref'ed again.
        SkASSERT(fRefCnt > 0);
        this->validate();
        ++fRefCnt;
    }

    void unref() const {
        this->validate();
        --fRefCnt;
        if (0 == fRefCnt && 0 == fPendingExecutions) {
            SkDELETE(this);
        }
    }

    void validate() const {
#ifdef SK_DEBUG
        SkASSERT(fRefCnt >= 0);
        SkASSERT(fPendingExecutions >= 0);
        SkASSERT(fRefCnt + fPendingExecutions > 0);
#endif
    }

protected:
    GrProgramElement() : fRefCnt(1), fPendingExecutions(0) {}

    /** Subclasses registers their resources using this function. It is assumed the GrProgramResouce
        is and will remain owned by the subclass and this function will retain a raw ptr. Once a
        GrProgramResource is registered its setResource must not be called.
     */
    void addProgramResource(const GrProgramResource* res) {
        fProgramResources.push_back(res);
    }

private:
    void convertRefToPendingExecution() const;

    void completedExecution() const;

    mutable int32_t fRefCnt;
    // Count of deferred executions not yet issued to the 3D API.
    mutable int32_t fPendingExecutions;

    SkSTArray<4, const GrProgramResource*, true> fProgramResources;

    // Only this class can access convertRefToPendingExecution() and completedExecution().
    template <typename T> friend class GrProgramElementRef;

    typedef SkNoncopyable INHERITED;
};

#endif
