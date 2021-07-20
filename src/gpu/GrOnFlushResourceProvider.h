/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOnFlushResourceProvider_DEFINED
#define GrOnFlushResourceProvider_DEFINED

#include "include/core/SkSpan.h"
#include "src/gpu/GrDeferredUpload.h"

class GrCaps;
class GrDrawingManager;
class GrOnFlushResourceProvider;
class GrSurfaceProxy;

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
    explicit GrOnFlushResourceProvider(GrDrawingManager* drawingMgr) : fDrawingMgr(drawingMgr) {}

    bool instatiateProxy(GrSurfaceProxy*);

    const GrCaps* caps() const;

private:
    GrOnFlushResourceProvider(const GrOnFlushResourceProvider&) = delete;
    GrOnFlushResourceProvider& operator=(const GrOnFlushResourceProvider&) = delete;

    GrDrawingManager* fDrawingMgr;
};

#endif
