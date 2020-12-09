/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOnFlushResourceProvider_DEFINED
#define GrOnFlushResourceProvider_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"
#include "src/core/SkSpan.h"
#include "src/gpu/GrDeferredUpload.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrResourceProvider.h"

class GrDrawingManager;
class GrOnFlushResourceProvider;
class GrSurfaceDrawContext;
class GrSurfaceProxy;
class SkColorSpace;
class SkSurfaceProps;

/*
 * This is the base class from which all pre-flush callback objects must be derived. It
 * provides the "preFlush" / "postFlush" interface.
 */
class GrOnFlushCallbackObject {
public:
    virtual ~GrOnFlushCallbackObject() {}

    /*
     * The preFlush callback allows subsystems (e.g., text, path renderers) to create atlases
     * for a specific flush. All the GrRenderTask IDs required for the flush are passed into the
     * callback.
     */
    virtual void preFlush(GrOnFlushResourceProvider*, SkSpan<const uint32_t> renderTaskIDs) = 0;

    /**
     * Called once flushing is complete and all renderTasks indicated by preFlush have been executed
     * and released. startTokenForNextFlush can be used to track resources used in the current
     * flush.
     */
    virtual void postFlush(GrDeferredUploadToken startTokenForNextFlush,
                           SkSpan<const uint32_t> renderTaskIDs) {}

    /**
     * Tells the callback owner to hold onto this object when freeing GPU resources.
     */
    virtual bool retainOnFreeGpuResources() { return false; }
};

/*
 * This class is a shallow wrapper around the drawing manager. It is passed into the
 * onFlush callbacks and is intended to limit the functionality available to them.
 * It should never have additional data members or virtual methods.
 */
class GrOnFlushResourceProvider {
public:
    using UseAllocator = GrSurfaceProxy::UseAllocator;

    explicit GrOnFlushResourceProvider(GrDrawingManager* drawingMgr) : fDrawingMgr(drawingMgr) {}

    std::unique_ptr<GrSurfaceDrawContext> makeRenderTargetContext(sk_sp<GrSurfaceProxy>,
                                                                   GrSurfaceOrigin, GrColorType,
                                                                   sk_sp<SkColorSpace>,
                                                                   const SkSurfaceProps*);

    void addTextureResolveTask(sk_sp<GrTextureProxy>, GrSurfaceProxy::ResolveFlags);

    // Proxy unique key management. See GrProxyProvider.h.
    bool assignUniqueKeyToProxy(const GrUniqueKey&, GrTextureProxy*);
    void removeUniqueKeyFromProxy(GrTextureProxy*);
    void processInvalidUniqueKey(const GrUniqueKey&);
    sk_sp<GrTextureProxy> findOrCreateProxyByUniqueKey(const GrUniqueKey&, UseAllocator);

    bool instatiateProxy(GrSurfaceProxy*);

    // Creates a GPU buffer with a "dynamic" access pattern.
    sk_sp<GrGpuBuffer> makeBuffer(GrGpuBufferType, size_t, const void* data = nullptr);

    // Either finds and refs, or creates a static GPU buffer with the given data.
    sk_sp<const GrGpuBuffer> findOrMakeStaticBuffer(GrGpuBufferType, size_t, const void* data,
                                                    const GrUniqueKey&);

    uint32_t contextID() const;
    const GrCaps* caps() const;
    GrRecordingContext* recordingContext() const;

    void printWarningMessage(const char* msg) const;

private:
    GrOnFlushResourceProvider(const GrOnFlushResourceProvider&) = delete;
    GrOnFlushResourceProvider& operator=(const GrOnFlushResourceProvider&) = delete;

    GrDrawingManager* fDrawingMgr;
};

#endif
