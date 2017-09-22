/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOnFlushResourceProvider_DEFINED
#define GrOnFlushResourceProvider_DEFINED

#include "GrTypes.h"
#include "GrResourceProvider.h"
#include "SkRefCnt.h"
#include "SkTArray.h"

class GrDrawingManager;
class GrOpList;
class GrOnFlushResourceProvider;
class GrRenderTargetOpList;
class GrRenderTargetContext;
class GrSurfaceProxy;

class SkColorSpace;
class SkSurfaceProps;

/*
 * This is the base class from which all pre-flush callback objects must be derived. It
 * provides the "preFlush" / "postFlush" interface.
 */
class GrOnFlushCallbackObject {
public:
    virtual ~GrOnFlushCallbackObject() { }

    /*
     * The onFlush callback allows subsystems (e.g., text, path renderers) to create atlases
     * for a specific flush. All the GrOpList IDs required for the flush are passed into the
     * callback. The callback should return the render target contexts used to render the atlases
     * in 'results'.
     */
    virtual void preFlush(GrOnFlushResourceProvider*,
                          const uint32_t* opListIDs, int numOpListIDs,
                          SkTArray<sk_sp<GrRenderTargetContext>>* results) = 0;

    /**
     * Called once flushing is complete and all ops indicated by preFlush have been executed and
     * released.
     */
    virtual void postFlush() {}

private:
    typedef SkRefCnt INHERITED;
};

/*
 * This class is a shallow wrapper around the drawing manager. It is passed into the
 * onFlush callbacks and is intended to limit the functionality available to them.
 * It should never have additional data members or virtual methods.
 */
class GrOnFlushResourceProvider {
public:
    sk_sp<GrRenderTargetContext> makeRenderTargetContext(const GrSurfaceDesc& desc,
                                                         sk_sp<SkColorSpace> colorSpace,
                                                         const SkSurfaceProps* props);

    // TODO: we only need this entry point as long as we have to pre-allocate the atlas.
    // Remove it ASAP.
    sk_sp<GrRenderTargetContext> makeRenderTargetContext(sk_sp<GrSurfaceProxy> proxy,
                                                         sk_sp<SkColorSpace> colorSpace,
                                                         const SkSurfaceProps* props);

    // Creates a GPU buffer with a "dynamic" access pattern.
    sk_sp<GrBuffer> makeBuffer(GrBufferType, size_t, const void* data = nullptr);

    // Either finds and refs, or creates a static GPU buffer with the given data.
    sk_sp<GrBuffer> findOrMakeStaticBuffer(const GrUniqueKey&, GrBufferType,
                                           size_t, const void* data);

    const GrCaps* caps() const;

private:
    explicit GrOnFlushResourceProvider(GrDrawingManager* drawingMgr) : fDrawingMgr(drawingMgr) {}
    GrOnFlushResourceProvider(const GrOnFlushResourceProvider&) = delete;
    GrOnFlushResourceProvider& operator=(const GrOnFlushResourceProvider&) = delete;

    GrDrawingManager* fDrawingMgr;

    friend class GrDrawingManager; // to construct this type.
};

#endif
