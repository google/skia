/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/private/GrCCPerOpListPaths.h"
#include "include/private/GrOpList.h"
#include "include/private/SkDeferredDisplayList.h"
#include <utility>
class SkSurfaceCharacterization;

SkDeferredDisplayList::SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                                             sk_sp<LazyProxyData> lazyProxyData)
        : fCharacterization(characterization)
        , fLazyProxyData(std::move(lazyProxyData)) {
}

SkDeferredDisplayList::~SkDeferredDisplayList() {
}
