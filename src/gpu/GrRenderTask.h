/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTask_DEFINED
#define GrRenderTask_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"
#include "src/gpu/GrSurfaceProxyView.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrTextureResolveManager.h"
#include "src/gpu/ops/GrOp.h"

class GrOpFlushState;
class GrOpsTask;
class GrResourceAllocator;
class GrTextureResolveRenderTask;

// This class abstracts a task that targets a single GrSurfaceProxy, participates in the
// GrDrawingManager's DAG, and implements the onExecute method to modify its target proxy's
// contents. (e.g., an opsTask that executes a command buffer, a task to regenerate mipmaps, etc.)
class GrRenderTask : public SkRefCnt {
public:
    GrRenderTask();
    SkDEBUGCODE(~GrRenderTask() override);

    void makeClosed(const GrCaps&);

    void prePrepare(GrRecordingContext* context) { this->onPrePrepare(context); }

    // These two methods are only invoked at flush time
    void prepare(GrOpFlushState* flushState);
    bool execute(GrOpFlushState* flushState) { return this->onExecute(flushState); }

    virtual bool requiresExplicitCleanup() const { return false; }

    // Called when this class will survive a flush and needs to truncate its ops and start over.
    // TODO: ultimately it should be invalid for an op list to survive a flush.
    // https://bugs.chromium.org/p/skia/issues/detail?id=7111
    virtual void endFlush(GrDrawingManager*) {}

    // This method "disowns" all the GrSurfaceProxies this RenderTask modifies. In
    // practice this just means telling the drawingManager to forget the relevant
    // mappings from surface proxy to last modifying rendertask.
    virtual void disown(GrDrawingManager*);

    bool isClosed() const { return this->isSetFlag(kClosed_Flag); }

    /*
     * Notify this GrRenderTask that it relies on the contents of 'dependedOn'
     */
    void addDependency(GrDrawingManager*, GrSurfaceProxy* dependedOn, GrMipmapped,
                       GrTextureResolveManager, const GrCaps& caps);

    /*
     * Notify this GrRenderTask that it relies on the contents of all GrRenderTasks which otherTask
     * depends on.
     */
    void addDependenciesFromOtherTask(GrRenderTask* otherTask);

    /*
     * Does this renderTask depend on 'dependedOn'?
     */
    bool dependsOn(const GrRenderTask* dependedOn) const;

    virtual void gatherIDs(SkSTArray<8, uint32_t, true>* idArray) const {
        idArray->push_back(fUniqueID);
    }
    uint32_t uniqueID() const { return fUniqueID; }
    virtual int numTargets() const { return fTargets.count(); }
    const GrSurfaceProxyView& target(int i) const { return fTargets[i]; }

    /*
     * Safely cast this GrRenderTask to a GrOpsTask (if possible).
     */
    virtual GrOpsTask* asOpsTask() { return nullptr; }

#if GR_TEST_UTILS
    /*
     * Dump out the GrRenderTask dependency DAG
     */
    virtual void dump(bool printDependencies) const;
    virtual const char* name() const = 0;
#endif

#ifdef SK_DEBUG
    virtual int numClips() const { return 0; }

    virtual void visitProxies_debugOnly(const GrOp::VisitProxyFunc&) const = 0;

    void visitTargetAndSrcProxies_debugOnly(const GrOp::VisitProxyFunc& fn) const {
        this->visitProxies_debugOnly(fn);
       for (const GrSurfaceProxyView& target : fTargets) {
            fn(target.proxy(), GrMipmapped::kNo);
        }
    }
#endif

    bool isUsed(GrSurfaceProxy* proxy) const {
        for (const GrSurfaceProxyView& target : fTargets) {
            if (target.proxy() == proxy) {
                return true;
            }
        }

        return this->onIsUsed(proxy);
    }

    // Drops any pending operations that reference proxies that are not instantiated.
    // NOTE: Derived classes don't need to check targets. That is handled when the
    // drawingManager calls isInstantiated.
    virtual void handleInternalAllocationFailure() = 0;

    // Feed proxy usage intervals to the GrResourceAllocator class
    virtual void gatherProxyIntervals(GrResourceAllocator*) const = 0;

    // In addition to just the GrSurface being allocated, has the stencil buffer been allocated (if
    // it is required)?
    bool isInstantiated() const;

protected:
    SkDEBUGCODE(bool deferredProxiesAreInstantiated() const;)

    // Add a target surface proxy to the list of targets for this task.
    // This also informs the drawing manager to update the lastRenderTask association.
    void addTarget(GrDrawingManager*, GrSurfaceProxyView);

    enum class ExpectedOutcome : bool {
        kTargetUnchanged,
        kTargetDirty,
    };

    // Performs any work to finalize this renderTask prior to execution. If returning
    // ExpectedOutcome::kTargetDiry, the caller is also responsible to fill out the area it will
    // modify in targetUpdateBounds.
    //
    // targetUpdateBounds must not extend beyond the proxy bounds.
    virtual ExpectedOutcome onMakeClosed(const GrCaps&, SkIRect* targetUpdateBounds) = 0;

    SkSTArray<1, GrSurfaceProxyView> fTargets;

    // List of texture proxies whose contents are being prepared on a worker thread
    // TODO: this list exists so we can fire off the proper upload when an renderTask begins
    // executing. Can this be replaced?
    SkTArray<GrTextureProxy*, true> fDeferredProxies;

    enum Flags {
        kClosed_Flag    = 0x01,   //!< This task can't accept any more dependencies.
        kDisowned_Flag  = 0x02,   //!< This task is disowned by its creating GrDrawingManager.

        kWasOutput_Flag = 0x04,   //!< Flag for topological sorting
        kTempMark_Flag  = 0x08,   //!< Flag for topological sorting
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

    void setIndex(uint32_t index) {
        SkASSERT(!this->isSetFlag(kWasOutput_Flag));
        SkASSERT(index < (1 << 28));
        fFlags |= index << 4;
    }

    uint32_t getIndex() const {
        SkASSERT(this->isSetFlag(kWasOutput_Flag));
        return fFlags >> 4;
    }

private:
    // for TopoSortTraits, fTextureResolveTask, closeThoseWhoDependOnMe, addDependency
    friend class GrDrawingManager;

    // Derived classes can override to indicate usage of proxies _other than target proxies_.
    // GrRenderTask itself will handle checking the target proxies.
    virtual bool onIsUsed(GrSurfaceProxy*) const = 0;

    void addDependency(GrRenderTask* dependedOn);
    void addDependent(GrRenderTask* dependent);
    SkDEBUGCODE(bool isDependedent(const GrRenderTask* dependent) const;)
    SkDEBUGCODE(void validate() const;)
    void closeThoseWhoDependOnMe(const GrCaps&);

    static uint32_t CreateUniqueID();

    struct TopoSortTraits {
        static uint32_t GetIndex(GrRenderTask* renderTask) {
            return renderTask->getIndex();
        }
        static void Output(GrRenderTask* renderTask, uint32_t index) {
            renderTask->setIndex(index);
            renderTask->setFlag(kWasOutput_Flag);
        }
        static bool WasOutput(const GrRenderTask* renderTask) {
            return renderTask->isSetFlag(kWasOutput_Flag);
        }
        static void SetTempMark(GrRenderTask* renderTask) {
            renderTask->setFlag(kTempMark_Flag);
        }
        static void ResetTempMark(GrRenderTask* renderTask) {
            renderTask->resetFlag(kTempMark_Flag);
        }
        static bool IsTempMarked(const GrRenderTask* renderTask) {
            return renderTask->isSetFlag(kTempMark_Flag);
        }
        static int NumDependencies(const GrRenderTask* renderTask) {
            return renderTask->fDependencies.count();
        }
        static GrRenderTask* Dependency(GrRenderTask* renderTask, int index) {
            return renderTask->fDependencies[index];
        }
    };


    virtual void onPrePrepare(GrRecordingContext*) {} // Only the GrOpsTask currently overrides this
    virtual void onPrepare(GrOpFlushState*) {} // Only GrOpsTask and GrDDLTask override this virtual
    virtual bool onExecute(GrOpFlushState* flushState) = 0;

    const uint32_t         fUniqueID;
    uint32_t               fFlags;

    // 'this' GrRenderTask relies on the output of the GrRenderTasks in 'fDependencies'
    SkSTArray<1, GrRenderTask*, true> fDependencies;
    // 'this' GrRenderTask's output is relied on by the GrRenderTasks in 'fDependents'
    SkSTArray<1, GrRenderTask*, true> fDependents;

    // For performance reasons, we should perform texture resolves back-to-back as much as possible.
    // (http://skbug.com/9406). To accomplish this, we make and reuse one single resolve task for
    // each render task, then add it as a dependency during makeClosed().
    GrTextureResolveRenderTask* fTextureResolveTask = nullptr;

    SkDEBUGCODE(GrDrawingManager *fDrawingMgr = nullptr;)
};

#endif
