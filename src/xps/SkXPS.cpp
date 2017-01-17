/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXPS.h"

#ifdef SK_BUILD_FOR_WIN

#include "SkHRESULT.h"
#include "SkImageFilter.h"
#include "SkLeanWindows.h"
#include "SkMakeUnique.h"
#include "SkTScopedComPtr.h"


#ifndef UNICODE
  #define UNICODE
#endif
#ifndef _UNICODE
  #define _UNICODE
#endif

#include <FontSub.h>
#include <ObjBase.h>
#include <T2EmbApi.h>
#include <XpsObjectModel.h>

namespace {
// The XPSLayer represents a layer created with canvas->saveLayer().
struct XPSLayer {
    SkTScopedComPtr<IXpsOMCanvas> fCanvas;
    SkPaint                       fPaint;
    sk_sp<const SkImageFilter>    fBackDrop;
};
} // namespace

struct SkXPS::Impl {
    SkTArray<XPSLayer, true> fLayers;
    SkTScopedComPtr<IXpsOMObjectFactory> fXpsFactory;
    SkScalar fDpi;
};

SkXPS::SkXPS(SkWStream* wStream,
             IXpsOMObjectFactory* xpsFactory,
             SkScalar dpi)
    : fImpl(skstd::make_unique<SkXPS::Impl>()) {
    SkASSERT(xpsFactory);
    fImpl->fXpsFactory.reset(SkRefComPtr(xpsFactory));
    fImpl->fDpi = dpi;
}

SkXPS::~SkXPS() {}

void SkXPS::endPage() {
    if (!fImpl->fLayers.count()) {
        return;
    }
    // FIXME
    // clean out fImpl->fLayers
}

void SkXPS::newPage(SkSize pageSize) {
    if (!fImpl) { return; }
    this->endPage();
    XPSLayer& topLayer = fImpl->fLayers.push_back();
    this->saveLayer(SkCanvas::SaveLayerRec());

}

void SkXPS::endPortfolio() {
    if (!fImpl) { return; }
    this->endPage();
    // FIXME
}

void SkXPS::saveLayer(const SkCanvas::SaveLayerRec& rec) {
    if (!fImpl) { return; }
    XPSLayer& topLayer = fImpl->fLayers.push_back();
    IXpsOMCanvas* canvas = nullptr;
    HRV(fImpl->fXpsFactory->CreateCanvas(&canvas));
    topLayer.fCanvas.reset(canvas);
    if (rec.fPaint) {
        topLayer.fPaint = *rec.fPaint;
    }
    topLayer.fBackDrop.reset(SkSafeRef(rec.fBackdrop));
}

void SkXPS::restoreLayer() {
    if (!fImpl) { return; }
}

void SkXPS::drawText(const SkMatrix& ctm, const SkPath& clip,
                     const void* text, size_t textBytes,
                     SkTextBlob::GlyphPositioning positioning,
                     SkPoint origin, const SkScalar* pos, const SkPaint& paint) {
    if (!fImpl) { return; }

}

void SkXPS::drawPoints(const SkMatrix& ctm, const SkPath& clip,
                       SkCanvas::PointMode, size_t, const SkPoint[], const SkPaint&) {
    if (!fImpl) { return; }

}

void SkXPS::drawPath(const SkMatrix& ctm, const SkPath& clip,
                     const SkPath& path, const SkPaint& paint) {
    if (!fImpl) { return; }

}

void SkXPS::drawBitmap(const SkMatrix& ctm, const SkPath& clip,
                       const SkBitmap&, SkPoint, const SkPaint*) {
    if (!fImpl) { return; }
}

#endif  // SK_BUILD_FOR_WIN
