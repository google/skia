/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOpList_DEFINED
#define GrOpList_DEFINED

#include "SkRefCnt.h"
#include "SkTDArray.h"

//#define ENABLE_MDB 1

class GrAuditTrail;
class GrOpFlushState;
class GrRenderTargetOpList;
class GrSurface;
class GrSurfaceProxy;
class GrTextureOpList;

class GrOpList : public SkRefCnt {
public:
    GrOpList(GrSurfaceProxy* surfaceProxy, GrAuditTrail* auditTrail);
    ~GrOpList() override;

    // These two methods are invoked as flush time
    virtual void prepareOps(GrOpFlushState* flushState) = 0;
    virtual bool executeOps(GrOpFlushState* flushState) = 0;

    virtual void makeClosed() {
        // We only close GrOpLists when MDB is enabled. When MDB is disabled there is only
        // ever one GrOpLists and all calls will be funnelled into it.
#ifdef ENABLE_MDB
        this->setFlag(kClosed_Flag);
#endif    
    }

    // TODO: it seems a bit odd that GrOpList has nothing to clear on reset
    virtual void reset() = 0;

    // TODO: in an MDB world, where the OpLists don't allocate GPU resources, it seems like
    // these could go away
    virtual void abandonGpuResources() = 0;
    virtual void freeGpuResources() = 0;

    // TODO: this entry point is only needed in the non-MDB world. Remove when
    // we make the switch to MDB
    void clearTarget() { fTarget = nullptr; }

    bool isClosed() const { return this->isSetFlag(kClosed_Flag); }

    /*
     * Notify this GrOpList that it relies on the contents of 'dependedOn'
     */
    void addDependency(GrSurface* dependedOn);

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

    uint32_t             fUniqueID;
    uint32_t             fFlags;
    GrSurfaceProxy*      fTarget;

    // 'this' GrOpList relies on the output of the GrOpLists in 'fDependencies'
    SkTDArray<GrOpList*> fDependencies;

protected:
    GrAuditTrail*        fAuditTrail;

    typedef SkRefCnt INHERITED;
};

#endif
