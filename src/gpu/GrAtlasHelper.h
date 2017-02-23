/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasHelper_DEFINED
#define GrAtlasHelper_DEFINED

#include "GrDrawingManager.h"
#include "GrSurfaceProxy.h"

/*
 * This class is a shallow wrapper around the drawing manager. It is passed into the 
 * createAtlas callbacks and is intended to limit the functionality available to them.
 * It should never have additional data members or virtual methods.
 */
class GrAtlasHelper {
public:
    sk_sp<GrRenderTargetContext> makeRenderTargetContext(const GrSurfaceDesc& desc,
                                                         sk_sp<SkColorSpace> colorSpace,
                                                         const SkSurfaceProps* props) {
        GrSurfaceDesc tmpDesc = desc;
        tmpDesc.fFlags |= kRenderTarget_GrSurfaceFlag;

        sk_sp<GrSurfaceProxy> proxy = GrSurfaceProxy::MakeDeferred(*fDrawingMgr->getContext()->caps(),
                                                                   tmpDesc,
                                                                   SkBackingFit::kExact,
                                                                   SkBudgeted::kYes);


        return fDrawingMgr->makeRenderTargetContext(std::move(proxy),
                                                    std::move(colorSpace),
                                                    props);
    }

private:
    explicit GrAtlasHelper(GrDrawingManager* drawingMgr) : fDrawingMgr(drawingMgr) {}
    GrAtlasHelper(const GrAtlasHelper&); // unimpl
    GrAtlasHelper& operator=(const GrAtlasHelper&); // unimpl

    GrDrawingManager* fDrawingMgr;

    friend class GrDrawingManager; // to construct/copy this type.
};

#endif
