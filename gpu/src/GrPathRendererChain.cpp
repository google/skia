
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
    , fFlags(flags)
    , fChain(fStorage.get(), kPreAllocCount) {
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

GrPathRenderer* GrPathRendererChain::getPathRenderer(const GrDrawTarget* target,
                                                     const GrPath& path,
                                                     GrPathFill fill) {
    if (!fInit) {
        this->init();
    }
    bool preferAA = target->isAntialiasState() && 
                    !target->getRenderTarget()->isMultisampled();
    GrPathRenderer* nonAAPR = NULL;
    for (int i = 0; i < fChain.count(); ++i) {
        if (fChain[i]->canDrawPath(target, path, fill)) {
            if (!preferAA || fChain[i]->supportsAA(target, path, fill)) {
                return fChain[i];
            } else {
                nonAAPR = fChain[i];
            }
        }
    }
    return nonAAPR;
}

void GrPathRendererChain::init() {
    GrAssert(!fInit);
    GrGpu* gpu = fOwner->getGpu();
    bool twoSided = gpu->getCaps().fTwoSidedStencilSupport;
    bool wrapOp = gpu->getCaps().fStencilWrapOpsSupport;
    this->addPathRenderer(new GrDefaultPathRenderer(twoSided, wrapOp))->unref();
    GrPathRenderer::AddPathRenderers(fOwner, fFlags, this);
    fInit = true;
}
