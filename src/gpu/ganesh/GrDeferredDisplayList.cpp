/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/chromium/GrDeferredDisplayList.h"

#include "include/gpu/ganesh/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrRenderTask.h"
#include "src/gpu/ganesh/surface/SkSurface_Ganesh.h"
#include "src/image/SkSurface_Base.h"

#include <utility>

GrDeferredDisplayList::GrDeferredDisplayList(const GrSurfaceCharacterization& characterization,
                                             sk_sp<GrRenderTargetProxy> targetProxy,
                                             sk_sp<LazyProxyData> lazyProxyData)
        : fCharacterization(characterization)
        , fArenas(true)
        , fTargetProxy(std::move(targetProxy))
        , fLazyProxyData(std::move(lazyProxyData)) {
    SkASSERT(fTargetProxy->isDDLTarget());
}

GrDeferredDisplayList::~GrDeferredDisplayList() {
#if defined(SK_DEBUG)
    for (auto& renderTask : fRenderTasks) {
        SkASSERT(renderTask->unique());
    }
#endif
}

GrDeferredDisplayList::ProgramIterator::ProgramIterator(GrDirectContext* dContext,
                                                        GrDeferredDisplayList* ddl)
    : fDContext(dContext)
    , fProgramData(ddl->programData())
    , fIndex(0) {
}

GrDeferredDisplayList::ProgramIterator::~ProgramIterator() {}

bool GrDeferredDisplayList::ProgramIterator::compile() {
    if (!fDContext || fIndex < 0 || fIndex >= (int) fProgramData.size()) {
        return false;
    }

    return fDContext->priv().compile(fProgramData[fIndex].desc(), fProgramData[fIndex].info());
}

bool GrDeferredDisplayList::ProgramIterator::done() const {
    return fIndex >= (int) fProgramData.size();
}

void GrDeferredDisplayList::ProgramIterator::next() {
    ++fIndex;
}

namespace skgpu::ganesh {

bool DrawDDL(SkSurface* surface, sk_sp<const GrDeferredDisplayList> ddl) {
    if (!surface || !ddl) {
        return false;
    }
    auto sb = asSB(surface);
    if (!sb->isGaneshBacked()) {
        return false;
    }
    auto gs = static_cast<SkSurface_Ganesh*>(surface);
    return gs->draw(ddl);
}

bool DrawDDL(sk_sp<SkSurface> surface, sk_sp<const GrDeferredDisplayList> ddl) {
    return DrawDDL(surface.get(), ddl);
}

}
