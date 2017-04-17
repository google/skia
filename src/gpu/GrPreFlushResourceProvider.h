/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPreFlushResourceProvider_DEFINED
#define GrPreFlushResourceProvider_DEFINED

#include "GrTypes.h"
#include "GrNonAtomicRef.h"

// These two are just for GrPreFlushCallbackObject
#include "SkRefCnt.h"
#include "SkTDArray.h"

class GrDrawingManager;
class GrOpList;
class GrPreFlushResourceProvider;
class GrRenderTargetOpList;
class GrRenderTargetContext;
class GrSurfaceProxy;

class SkColorSpace;
class SkSurfaceProps;

/*
 * This is the base class from which all per-flush callback objects must be derived. It
 * provides the "preFlush" interface.
 */
class GrPreFlushCallbackObject : public GrNonAtomicRef<GrPreFlushCallbackObject> {
public:
    virtual ~GrPreFlushCallbackObject() { }

    /*
     * The preFlush callback allows subsystems (e.g., text, path renderers) to create atlases
     * for a specific flush. All the GrOpList IDs required for the flush are passed into the
     * callback. The callback should return the render target contexts used to render the atlases
     * in 'results'.
     */
    virtual void preFlush(GrPreFlushResourceProvider*,
                          const uint32_t* opListIDs, int numOpListIDs,
                          SkTArray<sk_sp<GrRenderTargetContext>>* results) = 0;

private:
    typedef SkRefCnt INHERITED;
};

/*
 * This class is a shallow wrapper around the drawing manager. It is passed into the
 * preFlush callbacks and is intended to limit the functionality available to them.
 * It should never have additional data members or virtual methods.
 */
class GrPreFlushResourceProvider {
public:
    sk_sp<GrRenderTargetContext> makeRenderTargetContext(const GrSurfaceDesc& desc,
                                                         sk_sp<SkColorSpace> colorSpace,
                                                         const SkSurfaceProps* props);

    // TODO: we only need this entry point as long as we have to pre-allocate the atlas.
    // Remove it ASAP.
    sk_sp<GrRenderTargetContext> makeRenderTargetContext(sk_sp<GrSurfaceProxy> proxy,
                                                         sk_sp<SkColorSpace> colorSpace,
                                                         const SkSurfaceProps* props);

private:
    explicit GrPreFlushResourceProvider(GrDrawingManager* drawingMgr) : fDrawingMgr(drawingMgr) {}
    GrPreFlushResourceProvider(const GrPreFlushResourceProvider&); // unimpl
    GrPreFlushResourceProvider& operator=(const GrPreFlushResourceProvider&); // unimpl

    GrDrawingManager* fDrawingMgr;

    friend class GrDrawingManager; // to construct this type.
};

#endif
