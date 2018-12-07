/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOpList_DEFINED
#define GrOpList_DEFINED

#include "GrProxyRef.h"
#include "GrTextureProxy.h"
#include "SkColorData.h"
#include "SkRefCnt.h"
#include "SkTDArray.h"

class GrAuditTrail;
class GrCaps;
class GrOpFlushState;
class GrOpMemoryPool;
class GrRenderTargetOpList;
class GrResourceAllocator;
class GrResourceProvider;
class GrSurfaceProxy;
class GrTextureOpList;

struct SkIPoint;
struct SkIRect;

class GrOpList : public SkRefCnt {
public:
    GrOpList(GrResourceProvider*, sk_sp<GrOpMemoryPool>, GrSurfaceProxy*, GrAuditTrail*);
    ~GrOpList() override;

    // These four methods are invoked at flush time
    bool instantiate(GrResourceProvider* resourceProvider);
    // Instantiates any "threaded" texture proxies that are being prepared elsewhere
    void instantiateDeferredProxies(GrResourceProvider* resourceProvider);
    void prepare(GrOpFlushState* flushState);
    bool execute(GrOpFlushState* flushState) { return this->onExecute(flushState); }

    virtual bool copySurface(GrContext*,
                             GrSurfaceProxy* dst,
                             GrSurfaceProxy* src,
                             const SkIRect& srcRect,
                             const SkIPoint& dstPoint) = 0;

    virtual void makeClosed(const GrCaps&) {
        if (!this->isClosed()) {
            this->setFlag(kClosed_Flag);
            fTarget.removeRef();
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
     * Safely case this GrOpList to a GrRenderTargetOpList (if possible).
     */
    virtual GrRenderTargetOpList* asRenderTargetOpList() { return nullptr; }

    uint32_t uniqueID() const { return fUniqueID; }

    /*
     * Dump out the GrOpList dependency DAG
     */
    SkDEBUGCODE(virtual void dump(bool printDependencies) const;)

    SkDEBUGCODE(virtual int numClips() const { return 0; })

    // TODO: it would be nice for this to be hidden
    void setStencilLoadOp(GrLoadOp loadOp) { fStencilLoadOp = loadOp; }

protected:
    bool isInstantiated() const;

    // In addition to just the GrSurface being allocated, has the stencil buffer been allocated (if
    // it is required)?
    bool isFullyInstantiated() const;

    // This is a backpointer to the GrOpMemoryPool that holds the memory for this opLists' ops.
    // In the DDL case, these back pointers keep the DDL's GrOpMemoryPool alive as long as its
    // constituent opLists survive.
    sk_sp<GrOpMemoryPool> fOpMemoryPool;
    GrSurfaceProxyRef     fTarget;
    GrAuditTrail*         fAuditTrail;

    GrLoadOp              fColorLoadOp    = GrLoadOp::kLoad;
    SkPMColor4f           fLoadClearColor = SK_PMColor4fTRANSPARENT;
    GrLoadOp              fStencilLoadOp  = GrLoadOp::kLoad;

    // List of texture proxies whose contents are being prepared on a worker thread
    SkTArray<GrTextureProxy*, true> fDeferredProxies;

private:
    friend class GrDrawingManager; // for resetFlag, TopoSortTraits & gatherProxyIntervals

    void addDependency(GrOpList* dependedOn);
    void addDependent(GrOpList* dependent);
    SkDEBUGCODE(bool isDependedent(const GrOpList* dependent) const);
    SkDEBUGCODE(void validate() const);
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

    uint32_t               fUniqueID;
    uint32_t               fFlags;

    // 'this' GrOpList relies on the output of the GrOpLists in 'fDependencies'
    SkSTArray<1, GrOpList*, true> fDependencies;
    // 'this' GrOpList's output is relied on by the GrOpLists in 'fDependents'
    SkSTArray<1, GrOpList*, true> fDependents;

    typedef SkRefCnt INHERITED;
};

#endif
