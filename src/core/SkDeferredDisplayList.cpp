/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeferredDisplayList.h"

#include "SkCanvas.h"
#include "SkSurface.h"

#ifndef SK_RASTER_RECORDER_IMPLEMENTATION
SkDeferredDisplayList::SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                                             sk_sp<LazyProxyData> lazyProxyData)
        : fCharacterization(characterization)
        , fLazyProxyData(std::move(lazyProxyData)) {
}
#endif

#ifdef SK_RASTER_RECORDER_IMPLEMENTATION
// Placeholder. Ultimately, the SkSurface_Gpu will pass the wrapped opLists to its
// renderTargetContext.
bool SkDeferredDisplayList::draw(SkSurface* surface) const {
    surface->getCanvas()->drawImage(fImage.get(), 0, 0);
    return true;
}
#endif
