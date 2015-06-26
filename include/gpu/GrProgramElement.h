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

class GrGpuResourceRef;

/**
 * Base class for GrProcessor. GrDrawState uses this to manage
 * transitioning a GrProcessor from being owned by a client to being scheduled for execution. It
 * converts resources owned by the effect from being ref'ed to having pending reads/writes.
 *
 * All GrGpuResource objects owned by a GrProgramElement or derived classes (either directly or
 * indirectly) must be wrapped in a GrGpuResourceRef and registered with the GrProgramElement using
 * addGpuResource(). This allows the regular refs to be converted to pending IO events
 * when the program element is scheduled for deferred execution.
 */
class GrProgramElement : public SkNoncopyable {
public:
    virtual ~GrProgramElement() {
        // fRefCnt can be one when an effect is created statically using GR_CREATE_STATIC_EFFECT
        SkASSERT((0 == fRefCnt || 1 == fRefCnt) && 0 == fPendingExecutions);
        // Set to invalid values.
        SkDEBUGCODE(fRefCnt = fPendingExecutions = -10;)
    }

    void ref() const {
        this->validate();
        // Once the ref cnt reaches zero it should never be ref'ed again.
        SkASSERT(fRefCnt > 0);
        ++fRefCnt;
        this->validate();
    }

    void unref() const {
        this->validate();
        --fRefCnt;
        if (0 == fRefCnt) {
            if (0 == fPendingExecutions) {
                SkDELETE(this);
                return;
            } else {
                this->removeRefs();
            }
        }
        this->validate();
    }

    /**
     * Gets an id that is unique for this GrProgramElement object. This will never return 0.
     */
    uint32_t getUniqueID() const { return fUniqueID; }

    void validate() const {
#ifdef SK_DEBUG
        SkASSERT(fRefCnt >= 0);
        SkASSERT(fPendingExecutions >= 0);
        SkASSERT(fRefCnt + fPendingExecutions > 0);
#endif
    }

protected:
    GrProgramElement() : fRefCnt(1), fPendingExecutions(0), fUniqueID(CreateUniqueID()) {}

    /** Subclasses registers their resources using this function. It is assumed the GrProgramResouce
        is and will remain owned by the subclass and this function will retain a raw ptr. Once a
        GrGpuResourceRef is registered its setResource must not be called.
     */
    void addGpuResource(const GrGpuResourceRef* res) {
        fGpuResources.push_back(res);
    }

private:
    static uint32_t CreateUniqueID();

    void addPendingExecution() const {
        this->validate();
        SkASSERT(fRefCnt > 0);
        if (0 == fPendingExecutions) {
            this->addPendingIOs();
        }
        ++fPendingExecutions;
        this->validate();
    }

    void completedExecution() const {
        this->validate();
        --fPendingExecutions;
        if (0 == fPendingExecutions) {
            if (0 == fRefCnt) {
                SkDELETE(this);
                return;
            } else {
                this->pendingIOComplete();
            }
        }
        this->validate();
    }

    void removeRefs() const;
    void addPendingIOs() const;
    void pendingIOComplete() const;

    mutable int32_t fRefCnt;
    // Count of deferred executions not yet issued to the 3D API.
    mutable int32_t fPendingExecutions;
    uint32_t        fUniqueID;

    SkSTArray<4, const GrGpuResourceRef*, true> fGpuResources;

    // Only this class can access addPendingExecution() and completedExecution().
    template <typename T> friend class GrPendingProgramElement;

    typedef SkNoncopyable INHERITED;
};

#endif
