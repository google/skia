/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasHelper_DEFINED
#define GrAtlasHelper_DEFINED

#include "GrTypes.h"

// These two are just for GrPerFlushCallbackObject
#include "SkRefCnt.h"
#include "SkTDArray.h"

class GrDrawingManager;
class GrOpList;
class GrPerFlushResourceProvider;
class GrRenderTargetOpList;
class GrRenderTargetContext;
class GrSurfaceProxy;

class SkColorSpace;
class SkSurfaceProps;

class GrPerFlushCallbackObject : public SkRefCnt {
public:
    ~GrPerFlushCallbackObject() override { }

    /*
     * An the preFlush callback allows subsystems (e.g., text, path renderers) to create an atlas
     * for a specific flush. All the GrOpLists required for the flush are passed into the callback.
     * The callback must return a single opList containing all the ops needed to create the
     * atlas.
     */
    virtual sk_sp<GrRenderTargetOpList> preFlush(GrPerFlushResourceProvider*, const SkTDArray<GrOpList*>&) = 0;

private:
    typedef SkRefCnt INHERITED;
};

/*
 * This class is a shallow wrapper around the drawing manager. It is passed into the
 * createAtlas callbacks and is intended to limit the functionality available to them.
 * It should never have additional data members or virtual methods.
 */
class GrPerFlushResourceProvider {
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
    explicit GrPerFlushResourceProvider(GrDrawingManager* drawingMgr) : fDrawingMgr(drawingMgr) {}
    GrPerFlushResourceProvider(const GrPerFlushResourceProvider&); // unimpl
    GrPerFlushResourceProvider& operator=(const GrPerFlushResourceProvider&); // unimpl

    GrDrawingManager* fDrawingMgr;

    friend class GrDrawingManager; // to construct this type.
};

#endif
