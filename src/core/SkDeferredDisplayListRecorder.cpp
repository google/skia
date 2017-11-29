/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeferredDisplayListRecorder.h"

#include "GrContext.h"

#include "SkCanvas.h" // TODO: remove
#include "SkDeferredDisplayList.h"
#include "SkSurface.h" // TODO: remove

SkDeferredDisplayListRecorder::SkDeferredDisplayListRecorder(
                    const SkSurfaceCharacterization& characterization)
        : fCharacterization(characterization) {
}

SkCanvas* SkDeferredDisplayListRecorder::getCanvas() {
    if (!fSurface) {
        SkImageInfo ii = SkImageInfo::MakeN32(fCharacterization.width(),
                                              fCharacterization.height(),
                                              kOpaque_SkAlphaType);

        // Use raster right now to allow threading
        fSurface = SkSurface::MakeRaster(ii, nullptr);
    }

    return fSurface->getCanvas();
}

std::unique_ptr<SkDeferredDisplayList> SkDeferredDisplayListRecorder::detach() {
    sk_sp<SkImage> img = fSurface->makeImageSnapshot();
    fSurface.reset();

    // TODO: need to wrap the opLists associated with the deferred draws
    // in the SkDeferredDisplayList.
    return std::unique_ptr<SkDeferredDisplayList>(
                            new SkDeferredDisplayList(fCharacterization, std::move(img)));
}

// Placeholder. Ultimately, the SkSurface_Gpu will pass the wrapped opLists to its
// renderTargetContext.
void SkDeferredDisplayList::draw(SkSurface* surface) {
    surface->getCanvas()->drawImage(fImage.get(), 0, 0);
}
