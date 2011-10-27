
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrPathRendererChain.h"

#include "GrContext.h"
#include "GrDefaultPathRenderer.h"
#include "GrGpu.h"

GrPathRendererChain::GrPathRendererChain(GrContext* context, UsageFlags flags)
    : fInit(false)
    , fOwner(context)
    , fFlags(flags) {
    fInit = false;
}
    
GrPathRendererChain::~GrPathRendererChain() {
    for (int i = 0; i < fChain.count(); ++i) {
        fChain[i]->unref();
    }
}

GrPathRenderer* GrPathRendererChain::addPathRenderer(GrPathRenderer* pr) {
    fChain.push_back() = pr;
    pr->ref();
    return pr;
}

GrPathRenderer* GrPathRendererChain::getPathRenderer(
                                        const GrDrawTarget::Caps& targetCaps,
                                        const GrPath& path,
                                        GrPathFill fill,
                                        bool antiAlias) {
    if (!fInit) {
        this->init();
    }
    for (int i = 0; i < fChain.count(); ++i) {
        if (fChain[i]->canDrawPath(targetCaps, path, fill, antiAlias)) {
            return fChain[i];
        }
    }
    return NULL;
}

void GrPathRendererChain::init() {
    GrAssert(!fInit);
    GrGpu* gpu = fOwner->getGpu();
    bool twoSided = gpu->getCaps().fTwoSidedStencilSupport;
    bool wrapOp = gpu->getCaps().fStencilWrapOpsSupport;
    GrPathRenderer::AddPathRenderers(fOwner, fFlags, this);
    this->addPathRenderer(new GrDefaultPathRenderer(twoSided, wrapOp))->unref();
    fInit = true;
}
