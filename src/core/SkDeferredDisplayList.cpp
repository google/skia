/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeferredDisplayList.h"

#include "SkCanvas.h"
#include "SkSurface.h"

#if SK_SUPPORT_GPU
#include "ccpr/GrCoverageCountingPathRenderer.h"
#endif

SkDeferredDisplayList::SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                                             sk_sp<LazyProxyData> lazyProxyData)
        : fCharacterization(characterization)
        , fLazyProxyData(std::move(lazyProxyData)) {
}

SkDeferredDisplayList::~SkDeferredDisplayList() {
}
