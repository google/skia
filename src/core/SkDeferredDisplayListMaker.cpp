/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeferredDisplayListMaker.h"

#include "SkDeferredDisplayList.h"
#include "SkMakeUnique.h"

SkDeferredDisplayListMaker::SkDeferredDisplayListMaker(
                    const SkSurfaceCharacterization& characterization)
        : fCharacterization(characterization) {
}

SkCanvas* SkDeferredDisplayListMaker::canvas() {
#if SK_SUPPORT_GPU
    // TODO: need to create a GPU-backed SkCanvas that has a dummy GrContext.
    // The dummy GrContext basically has no contact with the real GPU but
    // has the caps from the originating SkSurface.
    return nullptr;
#else
    return nullptr;
#endif
}

std::unique_ptr<SkDeferredDisplayList> SkDeferredDisplayListMaker::finish() {
#if SK_SUPPORT_GPU
    // TODO: need to wrap the opLists associated with the deferred draws
    // in the SkDeferredDisplayList.
    return skstd::make_unique<SkDeferredDisplayList>();
#else
    return nullptr;
#endif
}
