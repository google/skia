/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTask_DEFINED
#define GrRenderTask_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkTInternalLList.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrTextureResolveManager.h"

class GrMockRenderTask;
class GrOpFlushState;
class GrResourceAllocator;
class GrTextureResolveRenderTask;
namespace skgpu {
namespace ganesh {
class OpsTask;
}
}  // namespace skgpu

// This class abstracts a task that targets a single GrSurfaceProxy, participates in the
// GrDrawingManager's DAG, and implements the onExecute method to modify its target proxy's
// contents. (e.g., an opsTask that executes a command buffer, a task to regenerate mipmaps, etc.)
class GrRenderTask : public SkRefCnt {
public:
    GrRenderTask();
    SkDEBUGCODE(~GrRenderTask() override);

    void makeClosed(GrRecordingContext*);

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

    /**
     * Make this task skippable. This must be used purely for optimization purposes
     * at this point as not all tasks will actually skip their work. It would be better if we could
     * detect tasks that can be skipped automatically. We'd need to support minimal flushes (i.e.,
     * only flush that which is required for SkSurfaces/SkImages) and the ability to detect
     * "orphaned tasks" and clean them out from the DAG so they don't indefinitely accumulate.
     * Finally, we'd probably have to track whether a proxy's backing store was imported or ever
     * exported to the client in case the client is doing direct reads outside of Skia and thus
     * may require tasks targeting the proxy to execute even if our DAG contains no reads.
     */
    void makeSkippable();

    bool isSkippable() const { return this->isSetFlag(kSkippable_Flag); }

    /** If true no other task should be reordered relative to this task. */
    bool blocksReordering() const { return this->isSetFlag(kBlocksReordering_Flag); }

    /*
     * Notify this GrRenderTask that it relies on the contents of 'dependedOn'
     */
    void addDependency(GrDrawingManager*,
                       GrSurfaceProxy* dependedOn,
                       skgpu::Mipmapped,
                       GrTextureResolveManager,
                       const GrCaps& caps);

    /*
     * Notify this GrRenderTask that it relies on the contents of all GrRenderTasks which otherTask
     * depends on.
     */
    void addDependenciesFromOtherTask(GrRenderTask* otherTask);

    SkSpan<GrRenderTask*> dependencies() { return SkSpan(fDependencies); }
    SkSpan<GrRenderTask*> dependents() { return SkSpan(fDependents); }

    void replaceDependency(const GrRenderTask* toReplace, GrRenderTask* replaceWith);
    void replaceDependent(const GrRenderTask* toReplace, GrRenderTask* replaceWith);


    /*
     * Does this renderTask depend on 'dependedOn'?
     */
    bool dependsOn(const GrRenderTask* dependedOn) const;

    uint32_t uniqueID() const { return fUniqueID; }
    int numTargets() const { return fTargets.size(); }
    GrSurfaceProxy* target(int i) const { return fTargets[i].get(); }

    /*
     * Safely cast this GrRenderTask to a OpsTask (if possible).
     */
    virtual skgpu::ganesh::OpsTask* asOpsTask() { return nullptr; }

#if defined(GR_TEST_UTILS)
    /*
     * Dump out the GrRenderTask dependency DAG
     */
    virtual void dump(const SkString& label,
                      SkString indent,
                      bool printDependencies,
                      bool close) const;
    virtual const char* name() const = 0;
#endif

#ifdef SK_DEBUG
    virtual int numClips() const { return 0; }

    virtual void visitProxies_debugOnly(const GrVisitProxyFunc&) const = 0;

    void visitTargetAndSrcProxies_debugOnly(const GrVisitProxyFunc& func) const {
        this->visitProxies_debugOnly(func);
        for (const sk_sp<GrSurfaceProxy>& target : fTargets) {
            func(target.get(), skgpu::Mipmapped::kNo);
        }
    }
#endif

    bool isUsed(GrSurfaceProxy* proxy) const {
        for (const sk_sp<GrSurfaceProxy>& target : fTargets) {
            if (target.get() == proxy) {
                return true;
            }
        }

        return this->onIsUsed(proxy);
    }

    // Feed proxy usage intervals to the GrResourceAllocator class
    virtual void gatherProxyIntervals(GrResourceAllocator*) const = 0;

    // In addition to just the GrSurface being allocated, has the stencil buffer been allocated (if
    // it is required)?
    bool isInstantiated() const;

    // Used by GrRenderTaskCluster.
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrRenderTask);

#if defined(GR_TEST_UTILS)
    const GrTextureResolveRenderTask* resolveTask() const { return fTextureResolveTask; }
#endif
protected:
    SkDEBUGCODE(bool deferredProxiesAreInstantiated() const;)

    // Add a target surface proxy to the list of targets for this task.
    // This also informs the drawing manager to update the lastRenderTask association.
    void addTarget(GrDrawingManager*, sk_sp<GrSurfaceProxy>);

    // Helper that adds the proxy owned by a view.
    void addTarget(GrDrawingManager* dm, const GrSurfaceProxyView& view) {
        this->addTarget(dm, view.refProxy());
    }

    enum class ExpectedOutcome : bool {
        kTargetUnchanged,
        kTargetDirty,
    };

    // Performs any work to finalize this renderTask prior to execution. If returning
    // ExpectedOutcome::kTargetDirty, the caller is also responsible to fill out the area it will
    // modify in targetUpdateBounds.
    //
    // targetUpdateBounds must not extend beyond the proxy bounds.
    virtual ExpectedOutcome onMakeClosed(GrRecordingContext*, SkIRect* targetUpdateBounds) = 0;

    skia_private::STArray<1, sk_sp<GrSurfaceProxy>> fTargets;

    // List of texture proxies whose contents are being prepared on a worker thread
    // TODO: this list exists so we can fire off the proper upload when an renderTask begins
    // executing. Can this be replaced?
    skia_private::TArray<GrTextureProxy*, true> fDeferredProxies;

    enum Flags {
        kClosed_Flag           = 0x01,   //!< This task can't accept any more dependencies.
        kDisowned_Flag         = 0x02,   //!< This task is disowned by its GrDrawingManager.
        kSkippable_Flag        = 0x04,   //!< This task is skippable.
        kAtlas_Flag            = 0x08,   //!< This task is atlas.
        kBlocksReordering_Flag = 0x10,   //!< No task can be reordered with respect to this task.

        kWasOutput_Flag        = 0x20,   //!< Flag for topological sorting
        kTempMark_Flag         = 0x40,   //!< Flag for topological sorting
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
        SkASSERT(index < (1 << 25));
        fFlags |= index << 7;
    }

    uint32_t getIndex() const {
        SkASSERT(this->isSetFlag(kWasOutput_Flag));
        return fFlags >> 7;
    }

private:
    // for TopoSortTraits, fTextureResolveTask, addDependency
    friend class GrDrawingManager;
    friend class GrMockRenderTask;

    // Derived classes can override to indicate usage of proxies _other than target proxies_.
    // GrRenderTask itself will handle checking the target proxies.
    virtual bool onIsUsed(GrSurfaceProxy*) const = 0;

    void addDependency(GrRenderTask* dependedOn);
    void addDependent(GrRenderTask* dependent);
    SkDEBUGCODE(bool isDependent(const GrRenderTask* dependent) const;)
    SkDEBUGCODE(void validate() const;)

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
            return renderTask->fDependencies.size();
        }
        static GrRenderTask* Dependency(GrRenderTask* renderTask, int index) {
            return renderTask->fDependencies[index];
        }
    };

    virtual void onMakeSkippable() {}
    virtual void onPrePrepare(GrRecordingContext*) {} // Only OpsTask currently overrides this
    virtual void onPrepare(GrOpFlushState*) {} // OpsTask and GrDDLTask override this
    virtual bool onExecute(GrOpFlushState* flushState) = 0;

    const uint32_t         fUniqueID;
    uint32_t               fFlags;

    // 'this' GrRenderTask relies on the output of the GrRenderTasks in 'fDependencies'
    skia_private::STArray<1, GrRenderTask*, true> fDependencies;
    // 'this' GrRenderTask's output is relied on by the GrRenderTasks in 'fDependents'
    skia_private::STArray<1, GrRenderTask*, true> fDependents;

    // For performance reasons, we should perform texture resolves back-to-back as much as possible.
    // (http://skbug.com/9406). To accomplish this, we make and reuse one single resolve task for
    // each render task, then add it as a dependency during makeClosed().
    GrTextureResolveRenderTask* fTextureResolveTask = nullptr;

    SkDEBUGCODE(GrDrawingManager *fDrawingMgr = nullptr;)
};

#endif
