/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOpList_DEFINED
#define GrOpList_DEFINED

#include "GrGpuResourceRef.h"
#include "SkRefCnt.h"
#include "SkTDArray.h"

//#define ENABLE_MDB 1

class GrAuditTrail;
class GrCaps;
class GrOpFlushState;
class GrRenderTargetOpList;
class GrResourceProvider;
class GrSurfaceProxy;
class GrTextureProxy;
class GrTextureOpList;

struct SkIPoint;
struct SkIRect;

class GrOpList : public SkRefCnt {
public:
    GrOpList(GrResourceProvider*, GrSurfaceProxy*, GrAuditTrail*);
    ~GrOpList() override;

    // These three methods are invoked at flush time
    bool instantiate(GrResourceProvider* resourceProvider);
    virtual void prepareOps(GrOpFlushState* flushState) = 0;
    virtual bool executeOps(GrOpFlushState* flushState) = 0;

    virtual bool copySurface(const GrCaps& caps,
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

    virtual void reset();

    // TODO: in an MDB world, where the OpLists don't allocate GPU resources, it seems like
    // these could go away
    virtual void abandonGpuResources() = 0;
    virtual void freeGpuResources() = 0;

    bool isClosed() const { return this->isSetFlag(kClosed_Flag); }

    /*
     * Notify this GrOpList that it relies on the contents of 'dependedOn'
     */
    void addDependency(GrSurfaceProxy* dependedOn, const GrCaps& caps);

    /*
     * Does this opList depend on 'dependedOn'?
     */
    bool dependsOn(GrOpList* dependedOn) const {
        return fDependencies.find(dependedOn) >= 0;
    }

    /*
     * Safely cast this GrOpList to a GrTextureOpList (if possible).
     */
    virtual GrTextureOpList* asTextureOpList() { return nullptr; }

    /*
     * Safely case this GrOpList to a GrRenderTargetOpList (if possible).
     */
    virtual GrRenderTargetOpList* asRenderTargetOpList() { return nullptr; }

    int32_t uniqueID() const { return fUniqueID; }

    /*
     * Dump out the GrOpList dependency DAG
     */
    SkDEBUGCODE(virtual void dump() const;)

    SkDEBUGCODE(virtual int numOps() const = 0;)
    SkDEBUGCODE(virtual int numClips() const { return 0; })

protected:
    GrSurfaceProxyRef fTarget;
    GrAuditTrail*     fAuditTrail;

private:
    friend class GrDrawingManager; // for resetFlag & TopoSortTraits

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
        static void Output(GrOpList* dt, int /* index */) {
            dt->setFlag(GrOpList::kWasOutput_Flag);
        }
        static bool WasOutput(const GrOpList* dt) {
            return dt->isSetFlag(GrOpList::kWasOutput_Flag);
        }
        static void SetTempMark(GrOpList* dt) {
            dt->setFlag(GrOpList::kTempMark_Flag);
        }
        static void ResetTempMark(GrOpList* dt) {
            dt->resetFlag(GrOpList::kTempMark_Flag);
        }
        static bool IsTempMarked(const GrOpList* dt) {
            return dt->isSetFlag(GrOpList::kTempMark_Flag);
        }
        static int NumDependencies(const GrOpList* dt) {
            return dt->fDependencies.count();
        }
        static GrOpList* Dependency(GrOpList* dt, int index) {
            return dt->fDependencies[index];
        }
    };

    void addDependency(GrOpList* dependedOn);

    uint32_t              fUniqueID;
    uint32_t              fFlags;

    // 'this' GrOpList relies on the output of the GrOpLists in 'fDependencies'
    SkTDArray<GrOpList*>  fDependencies;

    typedef SkRefCnt INHERITED;
};

#endif
