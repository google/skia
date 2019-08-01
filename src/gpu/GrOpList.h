/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOpList_DEFINED
#define GrOpList_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkColorData.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/GrTextureProxy.h"

class GrAuditTrail;
class GrCaps;
class GrOpFlushState;
class GrOpMemoryPool;
class GrRecordingContext;
class GrRenderTargetOpList;
class GrResourceAllocator;
class GrResourceProvider;
class GrSurfaceProxy;
class GrTextureOpList;
class GrGpuBuffer;

struct SkIPoint;
struct SkIRect;

class GrOpList : public SkRefCnt {
public:
    GrOpList(sk_sp<GrOpMemoryPool>, sk_sp<GrSurfaceProxy>, GrAuditTrail*);
    ~GrOpList() override;

    // These two methods are only invoked at flush time
    void prepare(GrOpFlushState* flushState);
    bool execute(GrOpFlushState* flushState) { return this->onExecute(flushState); }

    virtual bool copySurface(GrRecordingContext*,
                             GrSurfaceProxy* dst,
                             GrSurfaceProxy* src,
                             const SkIRect& srcRect,
                             const SkIPoint& dstPoint) = 0;

    virtual void transferFrom(GrRecordingContext*,
                              const SkIRect& srcRect,
                              GrColorType surfaceColorType,
                              GrColorType dstColorType,
                              sk_sp<GrGpuBuffer> dst,
                              size_t dstOffset) = 0;

    virtual void makeClosed(const GrCaps&) {
        if (!this->isClosed()) {
            this->setFlag(kClosed_Flag);
        }
    }

    // Called when this class will survive a flush and needs to truncate its ops and start over.
    // TODO: ultimately it should be invalid for an op list to survive a flush.
    // https://bugs.chromium.org/p/skia/issues/detail?id=7111
    virtual void endFlush();

    bool isClosed() const { return this->isSetFlag(kClosed_Flag); }

    /*
     * Notify this GrOpList that it relies on the contents of 'dependedOn'
     */
    void addDependency(GrSurfaceProxy* dependedOn, const GrCaps& caps);

    /*
     * Does this opList depend on 'dependedOn'?
     */
    bool dependsOn(const GrOpList* dependedOn) const;

    /*
     * Safely cast this GrOpList to a GrTextureOpList (if possible).
     */
    virtual GrTextureOpList* asTextureOpList() { return nullptr; }

    /*
     * Safely cast this GrOpList to a GrRenderTargetOpList (if possible).
     */
    virtual GrRenderTargetOpList* asRenderTargetOpList() { return nullptr; }

    uint32_t uniqueID() const { return fUniqueID; }

    /*
     * Dump out the GrOpList dependency DAG
     */
    SkDEBUGCODE(virtual void dump(bool printDependencies) const;)

    SkDEBUGCODE(virtual int numClips() const { return 0; })

protected:
    // In addition to just the GrSurface being allocated, has the stencil buffer been allocated (if
    // it is required)?
    bool isInstantiated() const;

    SkDEBUGCODE(bool deferredProxiesAreInstantiated() const;)

    // This is a backpointer to the GrOpMemoryPool that holds the memory for this opLists' ops.
    // In the DDL case, these back pointers keep the DDL's GrOpMemoryPool alive as long as its
    // constituent opLists survive.
    sk_sp<GrOpMemoryPool> fOpMemoryPool;
    sk_sp<GrSurfaceProxy> fTarget;
    GrAuditTrail*         fAuditTrail;

    GrLoadOp              fColorLoadOp    = GrLoadOp::kLoad;
    SkPMColor4f           fLoadClearColor = SK_PMColor4fTRANSPARENT;
    GrLoadOp              fStencilLoadOp  = GrLoadOp::kLoad;

    // List of texture proxies whose contents are being prepared on a worker thread
    // TODO: this list exists so we can fire off the proper upload when an opList begins
    // executing. Can this be replaced?
    SkTArray<GrTextureProxy*, true> fDeferredProxies;

private:
    friend class GrDrawingManager; // for resetFlag, TopoSortTraits & gatherProxyIntervals

    virtual bool onIsUsed(GrSurfaceProxy*) const = 0;

    bool isUsed(GrSurfaceProxy* proxy) const {
        if (proxy == fTarget.get()) {
            return true;
        }

        return this->onIsUsed(proxy);
    }

    void addDependency(GrOpList* dependedOn);
    void addDependent(GrOpList* dependent);
    SkDEBUGCODE(bool isDependedent(const GrOpList* dependent) const;)
    SkDEBUGCODE(void validate() const;)
    void closeThoseWhoDependOnMe(const GrCaps&);

    // Remove all Ops which reference proxies that are not instantiated.
    virtual void purgeOpsWithUninstantiatedProxies() = 0;

    // Feed proxy usage intervals to the GrResourceAllocator class
    virtual void gatherProxyIntervals(GrResourceAllocator*) const = 0;

    static uint32_t CreateUniqueID();

    enum Flags {
        kClosed_Flag    = 0x01,   //!< This GrOpList can't accept any more ops

        kWasOutput_Flag = 0x02,   //!< Flag for topological sorting
        kTempMark_Flag  = 0x04,   //!< Flag for topological sorting
    };

    void setFlag(uint32_t flag) {
        fFlags |= flag;
    }

    void resetFlag(uint32_t flag) {
        fFlags &= ~flag;
    }

    bool isSetFlag(uint32_t flag) const {
        return SkToBool(fFlags & flag);
    }

    struct TopoSortTraits {
        static void Output(GrOpList* opList, int /* index */) {
            opList->setFlag(GrOpList::kWasOutput_Flag);
        }
        static bool WasOutput(const GrOpList* opList) {
            return opList->isSetFlag(GrOpList::kWasOutput_Flag);
        }
        static void SetTempMark(GrOpList* opList) {
            opList->setFlag(GrOpList::kTempMark_Flag);
        }
        static void ResetTempMark(GrOpList* opList) {
            opList->resetFlag(GrOpList::kTempMark_Flag);
        }
        static bool IsTempMarked(const GrOpList* opList) {
            return opList->isSetFlag(GrOpList::kTempMark_Flag);
        }
        static int NumDependencies(const GrOpList* opList) {
            return opList->fDependencies.count();
        }
        static GrOpList* Dependency(GrOpList* opList, int index) {
            return opList->fDependencies[index];
        }
    };

    virtual void onPrepare(GrOpFlushState* flushState) = 0;
    virtual bool onExecute(GrOpFlushState* flushState) = 0;

    const uint32_t         fUniqueID;
    uint32_t               fFlags;

    // 'this' GrOpList relies on the output of the GrOpLists in 'fDependencies'
    SkSTArray<1, GrOpList*, true> fDependencies;
    // 'this' GrOpList's output is relied on by the GrOpLists in 'fDependents'
    SkSTArray<1, GrOpList*, true> fDependents;

    typedef SkRefCnt INHERITED;
};

#endif
