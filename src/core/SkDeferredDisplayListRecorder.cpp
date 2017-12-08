/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeferredDisplayListRecorder.h"

#include "GrContextPriv.h"

#include "SkCanvas.h" // TODO: remove
#include "SkGr.h"
#include "SkDeferredDisplayList.h"
#include "SkSurface.h"

SkDeferredDisplayListRecorder::SkDeferredDisplayListRecorder(
                    const SkSurfaceCharacterization& characterization)
        : fCharacterization(characterization) {
}

bool SkDeferredDisplayListRecorder::init() {
    SkASSERT(!fSurface);

    SkColorType colorType = kUnknown_SkColorType;
    if (!GrPixelConfigToColorType(fCharacterization.config(), &colorType)) {
        return false;
    }

    const SkImageInfo ii = SkImageInfo::Make(fCharacterization.width(), fCharacterization.height(),
                                             colorType, kPremul_SkAlphaType,
                                             fCharacterization.refColorSpace());

#ifdef SK_RASTER_RECORDER_IMPLEMENTATION
    // Use raster right now to allow threading
    fSurface = SkSurface::MakeRaster(ii, &fCharacterization.surfaceProps());
    return true;
#else
    SkASSERT(!fContext);

    fContext = GrContextPriv::MakeStubbedOut(fCharacterization.contextInfo());
    if (!fContext) {
        return false;
    }

    fSurface = SkSurface::MakeRenderTarget(fContext.get(), SkBudgeted::kYes,
                                           ii, fCharacterization.stencilCount(),
                                           fCharacterization.origin(),
                                           &fCharacterization.surfaceProps());
    return SkToBool(fSurface.get());
#endif
}

SkCanvas* SkDeferredDisplayListRecorder::getCanvas() {
    if (!fSurface) {
        if (!this->init()) {
            return nullptr;
        }
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
bool SkDeferredDisplayList::draw(SkSurface* surface) {
    surface->getCanvas()->drawImage(fImage.get(), 0, 0);
    return true;
}
