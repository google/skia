/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramElement_DEFINED
#define GrProgramElement_DEFINED

#include "../private/SkTArray.h"
#include "SkRefCnt.h"

class GrGpuResourceRef;

/**
 * Base class for GrProcessor. This exists to manage transitioning a GrProcessor from being owned by
 * a client to being scheduled for execution. While a GrProcessor is ref'ed by drawing code its
 * GrGpu resources must also be ref'ed to prevent incorrectly recycling them through the cache.
 * However, once the GrProcessor is baked into a GrPipeline and the drawing code has stopped ref'ing
 * it, it's internal resources can be recycled in some cases.
 *
 * We track this using two types of refs on GrProgramElement. A regular ref is owned by any client
 * that may continue to issue draws that use the GrProgramElement. The GrPipeline owns "pending
 * executions" instead of refs. A pending execution is cleared by ~GrPipeline().
 *
 * While a GrProgramElement is ref'ed any resources it owns are also ref'ed. However, once it gets
 * into the state where it has pending executions AND no refs then it converts its ownership of
 * its GrGpuResources from refs to pending IOs. The pending IOs allow the cache to track when it is
 * safe to recycle a resource even though we still have buffered GrBatches that read or write to the
 * the resource.
 *
 * To make this work all GrGpuResource objects owned by a GrProgramElement or derived classes
 * (either directly or indirectly) must be wrapped in a GrGpuResourceRef and registered with the
 * GrProgramElement using addGpuResource(). This allows the regular refs to be converted to pending
 * IO events when the program element is scheduled for deferred execution.
 *
 * Moreover, a GrProgramElement that in turn owns other GrProgramElements must convert its ownership
 * of its children to pending executions when its ref count reaches zero so that the GrGpuResources
 * owned by the children GrProgramElements are correctly converted from ownership by ref to
 * ownership by pending IO. Any GrProgramElement hierarchy is managed by subclasses which must
 * implement notifyRefCntIsZero() in order to convert refs of children to pending executions.
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
            this->notifyRefCntIsZero();
            if (0 == fPendingExecutions) {
                delete this;
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
                delete this;
                return;
            } else {
                this->pendingIOComplete();
            }
        }
        this->validate();
    }

private:
    /** This will be called when the ref cnt is zero. The object may or may not have pending
        executions. */
    virtual void notifyRefCntIsZero() const = 0;

    static uint32_t CreateUniqueID();

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
