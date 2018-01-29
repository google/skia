/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeferredDisplayListRecorder.h"

#if SK_SUPPORT_GPU
#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "SkGr.h"
#endif

#include "SkCanvas.h" // TODO: remove
#include "SkDeferredDisplayList.h"
#include "SkSurface.h"
#include "SkSurfaceCharacterization.h"

SkDeferredDisplayListRecorder::SkDeferredDisplayListRecorder(
                    const SkSurfaceCharacterization& characterization)
        : fCharacterization(characterization) {
}

SkDeferredDisplayListRecorder::~SkDeferredDisplayListRecorder() {
    auto proxyProvider = fContext->contextPriv().proxyProvider();

    // DDL TODO: Remove this. DDL contexts should allow for deletion while still having live
    // uniquely keyed proxies.
    proxyProvider->removeAllUniqueKeys();
}


bool SkDeferredDisplayListRecorder::init() {
    SkASSERT(!fSurface);

#ifdef SK_RASTER_RECORDER_IMPLEMENTATION
    // Use raster right now to allow threading
    const SkImageInfo ii = SkImageInfo::Make(fCharacterization.width(), fCharacterization.height(),
                                             kN32_SkColorType, kOpaque_SkAlphaType,
                                             fCharacterization.refColorSpace());

    fSurface = SkSurface::MakeRaster(ii, &fCharacterization.surfaceProps());
#else
    if (!fContext) {
        fContext = GrContextPriv::MakeDDL(fCharacterization.contextInfo());
        if (!fContext) {
            return false;
        }
    }

    SkColorType colorType = kUnknown_SkColorType;
    if (!GrPixelConfigToColorType(fCharacterization.config(), &colorType)) {
        return false;
    }

    const SkImageInfo ii = SkImageInfo::Make(fCharacterization.width(), fCharacterization.height(),
                                             colorType, kPremul_SkAlphaType,
                                             fCharacterization.refColorSpace());

    fSurface = SkSurface::MakeRenderTarget(fContext.get(), SkBudgeted::kYes,
                                           ii, fCharacterization.stencilCount(),
                                           fCharacterization.origin(),
                                           &fCharacterization.surfaceProps());
#endif
    return SkToBool(fSurface.get());
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
#ifdef SK_RASTER_RECORDER_IMPLEMENTATION
    sk_sp<SkImage> img = fSurface->makeImageSnapshot();
    fSurface.reset();

    // TODO: need to wrap the opLists associated with the deferred draws
    // in the SkDeferredDisplayList.
    return std::unique_ptr<SkDeferredDisplayList>(
                            new SkDeferredDisplayList(fCharacterization, std::move(img)));
#else
    return fContext->contextPriv().detachDDL(fCharacterization);
#endif
}
