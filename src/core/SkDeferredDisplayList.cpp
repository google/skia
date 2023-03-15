/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDeferredDisplayList.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/base/SkArenaAlloc.h"

#include <utility>

class SkSurfaceCharacterization;

#if defined(SK_GANESH)
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrRenderTask.h"
#endif

SkDeferredDisplayList::SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                                             sk_sp<GrRenderTargetProxy> targetProxy,
                                             sk_sp<LazyProxyData> lazyProxyData)
        : fCharacterization(characterization)
#if defined(SK_GANESH)
        , fArenas(true)
        , fTargetProxy(std::move(targetProxy))
        , fLazyProxyData(std::move(lazyProxyData))
#endif
{
#if defined(SK_GANESH)
    SkASSERT(fTargetProxy->isDDLTarget());
#endif
}

SkDeferredDisplayList::~SkDeferredDisplayList() {
#if defined(SK_GANESH) && defined(SK_DEBUG)
    for (auto& renderTask : fRenderTasks) {
        SkASSERT(renderTask->unique());
    }
#endif
}

//-------------------------------------------------------------------------------------------------
#if defined(SK_GANESH)

SkDeferredDisplayList::ProgramIterator::ProgramIterator(GrDirectContext* dContext,
                                                        SkDeferredDisplayList* ddl)
    : fDContext(dContext)
    , fProgramData(ddl->programData())
    , fIndex(0) {
}

SkDeferredDisplayList::ProgramIterator::~ProgramIterator() {}

bool SkDeferredDisplayList::ProgramIterator::compile() {
    if (!fDContext || fIndex < 0 || fIndex >= (int) fProgramData.size()) {
        return false;
    }

    return fDContext->priv().compile(fProgramData[fIndex].desc(), fProgramData[fIndex].info());
}

bool SkDeferredDisplayList::ProgramIterator::done() const {
    return fIndex >= (int) fProgramData.size();
}

void SkDeferredDisplayList::ProgramIterator::next() {
    ++fIndex;
}

#endif
