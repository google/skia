/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"

#include "SkSemaphore.h"
#include "SkTypeface_remote.h"
#include <iostream>

SkScalerContextProxy::SkScalerContextProxy(
        sk_sp<SkTypeface> tf,
        const SkScalerContextEffects& effects,
        const SkDescriptor* desc,
        SkRemoteScalerContext* rsc)
    : SkScalerContext{std::move(tf), effects, desc}
    , fRemote{rsc} {}

void SkScalerContextProxy::generateMetrics(SkGlyph* glyph) {
    fRemote->generateMetricsAndImage(*this->typefaceProxy(), this->getRec(), &fAlloc, glyph);
}

void SkScalerContextProxy::generateImage(const SkGlyph& glyph) {
    fRemote->generateImage(*this->typefaceProxy(), this->getRec(), glyph);
}

void SkScalerContextProxy::generatePath(SkGlyphID glyphID, SkPath* path) {
    fRemote->generatePath(*this->typefaceProxy(), this->getRec(), glyphID, path);
}

void SkScalerContextProxy::generateFontMetrics(SkPaint::FontMetrics* metrics) {
    fRemote->generateFontMetrics(*this->typefaceProxy(), this->getRec(), metrics);
}

SkTypefaceProxy* SkScalerContextProxy::typefaceProxy() {
    auto up = this->getTypeface();
    return (SkTypefaceProxy *)up;
}
