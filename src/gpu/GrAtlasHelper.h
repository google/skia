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

        // TODO: we need to ensure that the GrRenderTarget we get in here has no pendingIO!
//        static const uint32_t kFlags = GrResourceProvider::kNoPendingIO_Flag;

//        sk_sp<GrTexture> texture(this->createApproxTexture(desc, kFlags));
//        if (!texture) {
//            return nullptr;
//        }

        sk_sp<GrSurfaceProxy> proxy = GrSurfaceProxy::MakeDeferred(
                                                                *fDrawingMgr->getContext()->caps(),
                                                                tmpDesc,
                                                                SkBackingFit::kExact,
                                                                SkBudgeted::kYes);
        if (!proxy->asRenderTargetProxy()) {
            return nullptr;
        }

        sk_sp<GrRenderTargetOpList> opList(new GrRenderTargetOpList(
                                                        proxy->asRenderTargetProxy(),
                                                        fDrawingMgr->fContext->getGpu(),
                                                        fDrawingMgr->fContext->resourceProvider(),
                                                        fDrawingMgr->fContext->getAuditTrail(),
                                                        fDrawingMgr->fOptionsForOpLists));
        proxy->setLastOpList(opList.get());

        return fDrawingMgr->makeRenderTargetContext(std::move(proxy),
                                                    std::move(colorSpace),
                                                    props);
    }

    // TODO: we only need this entry point as long as we have to pre-allocate the atlas
    sk_sp<GrRenderTargetContext> makeRenderTargetContext(sk_sp<GrSurfaceProxy> proxy,
                                                         sk_sp<SkColorSpace> colorSpace,
                                                         const SkSurfaceProps* props) {

        sk_sp<GrRenderTargetOpList> opList(new GrRenderTargetOpList(
                                                        proxy->asRenderTargetProxy(),
                                                        fDrawingMgr->fContext->getGpu(),
                                                        fDrawingMgr->fContext->resourceProvider(),
                                                        fDrawingMgr->fContext->getAuditTrail(),
                                                        fDrawingMgr->fOptionsForOpLists));
        proxy->setLastOpList(opList.get());

        return fDrawingMgr->makeRenderTargetContext(std::move(proxy),
                                                    std::move(colorSpace),
                                                    props);
    }

private:
    explicit GrAtlasHelper(GrDrawingManager* drawingMgr) : fDrawingMgr(drawingMgr) {}
    GrAtlasHelper(const GrAtlasHelper&); // unimpl
    GrAtlasHelper& operator=(const GrAtlasHelper&); // unimpl

    GrDrawingManager* fDrawingMgr;

    friend class GrDrawingManager; // to construct this type.
};

#endif
