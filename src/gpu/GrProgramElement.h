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
 * Note: We are converting GrProcessor from ref counting to a single owner model using move
 * semantics. This class will be removed.
 *
 * This is used to track "refs" for two separate types GrProcessor ownership. A regular ref is owned
 * by any client that may continue to issue draws that use the GrProgramElement. A recorded op or
 * GrPipeline uses "pending executions" instead of refs. A pending execution is cleared after the
 * draw is executed (or aborted).
 *
 * While a GrProgramElement is ref'ed any resources it owns are also ref'ed. However, once it gets
 * into the state where it has pending executions AND no refs then it converts its ownership of
 * its GrGpuResources from refs to pending IOs. The pending IOs allow the cache to track when it is
 * safe to recycle a resource even though we still have buffered GrOps that read or write to the
 * the resource.
 *
 * To make this work the subclass GrProcessor implements addPendingIOs, removeRefs, and
 * pendingIOComplete. addPendingIOs adds pending reads/writes to GrGpuResources owned by the
 * processor as appropriate when the processor is recorded in a GrOpList. removeRefs is called when
 * the ref count reaches 0 and the GrProcessor is only owned by "pending executions".
 * pendingIOComplete occurs if the resource is still owned by a ref but all recorded draws have been
 * completed. Whenever pending executions and refs reach zero the processor is deleted.
 *
 * The GrProcessor may also implement notifyRefCntIsZero in order to change its ownership of child
 * processors from ref to pending execution when the processor is first owned exclusively in pending
 * execution mode.
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

    void validate() const {
#ifdef SK_DEBUG
        SkASSERT(fRefCnt >= 0);
        SkASSERT(fPendingExecutions >= 0);
        SkASSERT(fRefCnt + fPendingExecutions > 0);
#endif
    }

protected:
    GrProgramElement() : fRefCnt(1), fPendingExecutions(0) {}

    void addPendingExecution() const {
        this->validate();
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
    virtual void addPendingIOs() const = 0;
    virtual void removeRefs() const = 0;
    virtual void pendingIOComplete() const = 0;

    /** This will be called when the ref cnt is zero. The object may or may not have pending
        executions. */
    virtual void notifyRefCntIsZero() const = 0;

    mutable int32_t fRefCnt;
    // Count of deferred executions not yet issued to the 3D API.
    mutable int32_t fPendingExecutions;

    // Only these classes can access addPendingExecution() and completedExecution().
    template <typename T> friend class GrPendingProgramElement;
    friend class GrProcessorSet;

    typedef SkNoncopyable INHERITED;
};

#endif
