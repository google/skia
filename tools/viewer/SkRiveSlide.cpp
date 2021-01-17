/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/SkRiveSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"

#if defined(SK_ENABLE_SKRIVE)

SkRiveSlide::SkRiveSlide(const SkString& name, const SkString& path)
    : fPath(path) {
    fName = name;
}

SkRiveSlide::~SkRiveSlide() = default;

void SkRiveSlide::load(SkScalar w, SkScalar h) {
    fWinSize    = {w , h};
    fRive       = skrive::SkRive::Builder().make(SkFILEStream::Make(fPath.c_str()));
    fRiveBounds = SkRect::MakeEmpty();

    if (fRive) {
        SkDebugf("Loaded Rive animation: %zu artboards\n", fRive->artboards().size());
        for (const auto& ab : fRive->artboards()) {
            const auto& pos  = ab->getTranslation();
            const auto& size = ab->getSize();

            fRiveBounds.join(SkRect::MakeXYWH(pos.x, pos.y, size.x, size.y));
        }
    } else {
        SkDebugf("Failed to load Rive animation: %s\n", fPath.c_str());
    }
}

void SkRiveSlide::unload() {
    fRive.reset();
}

void SkRiveSlide::resize(SkScalar w, SkScalar h) {
    fWinSize = {w , h};
}

SkISize SkRiveSlide::getDimensions() const {
    // We always scale to fill the window.
    return fWinSize.toCeil();
}

void SkRiveSlide::draw(SkCanvas* canvas) {
    if (!fRive) {
        return;
    }

    // Scale the Rive artboards to fill our window.
    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(SkMatrix::RectToRect(fRiveBounds, SkRect::MakeSize(fWinSize),
                                        SkMatrix::kCenter_ScaleToFit));

    for (const auto& ab : fRive->artboards()) {
        ab->render(canvas);
    }
}

#endif // defined(SK_ENABLE_SKRIVE)
