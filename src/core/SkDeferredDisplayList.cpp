/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkDeferredDisplayList.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"

SkDeferredDisplayList::SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                                             sk_sp<LazyProxyData> lazyProxyData)
        : fCharacterization(characterization)
        , fLazyProxyData(std::move(lazyProxyData)) {
}

SkDeferredDisplayList::~SkDeferredDisplayList() {
}
