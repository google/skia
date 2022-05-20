/*
 * Copyright 2022 Rive Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/RiveSlide.h"

#include <rive/span.hpp>
#include <skia_renderer.hpp>
#include <skia_factory.hpp>

#include "include/core/SkCanvas.h"
#include "include/core/SkTime.h"
#include "tools/timer/TimeUtils.h"

static rive::SkiaFactory gFactory;

RiveSlide::RiveSlide(const SkString& name, const SkString& path) : fPath(path) {}

void RiveSlide::load(SkScalar w, SkScalar h) {
    FILE* f = fopen(fPath.c_str(), "rb");
    if (!f) {
        return;
    }

    auto data = SkData::MakeFromFILE(f);
    auto span = rive::Span<const uint8_t>((const uint8_t*)data->data(), data->size());
    fFile = rive::File::import(span, &gFactory);
    fclose(f);

    if (fFile) {
        fArtboard = fFile->artboardAt(0);
        fScene = fArtboard->stateMachineAt(0);
        if (!fScene) {
            fScene = fArtboard->animationAt(0);
        }
        auto b = fScene->bounds();
        fBounds = SkRect{b.left(), b.top(), b.right(), b.bottom()};
    }

    this->resize(w, h);
}

void RiveSlide::unload() {
    fScene = nullptr;
    fArtboard = nullptr;
    fFile = nullptr;
}

void RiveSlide::resize(SkScalar w, SkScalar h) {
    fWinSize = { w, h };

    if (fScene) {
        fMat = SkMatrix::RectToRect(fBounds, SkRect::MakeWH(w, h), SkMatrix::kCenter_ScaleToFit);
    }
}

SkISize RiveSlide::getDimensions() const {
    return fWinSize.toCeil();
}

void RiveSlide::draw(SkCanvas* canvas) {
    if (fScene) {
        rive::SkiaRenderer renderer(canvas);

        canvas->save();
        canvas->concat(fMat);
        fScene->draw(&renderer);
        canvas->restore();
    }
}

static double nanos_to_secs(double nanos) {
    return nanos * 1e-9;
}

bool RiveSlide::animate(double nanos) {
    const double secs = nanos_to_secs(nanos);

    if (fScene) {
        if (!fSecsBase) {
            // Reset the animation time.
            fSecsBase = secs;
        }
        fScene->advanceAndApply(secs - fSecsBase);
        fSecsBase = secs;
        return true;
    }
    return false;
}

bool RiveSlide::onMouse(SkScalar x, SkScalar y, skui::InputState state, skui::ModifierKey) {
    if (fScene) {
        SkMatrix inverse;
        if (!fMat.invert(&inverse)) {
            return false;
        }

        auto pt = inverse.mapXY(x, y);
        auto pos = rive::Vec2D(pt.fX, pt.fY);
        switch (state) {
            case skui::InputState::kUp:     fScene->pointerUp(pos); break;
            case skui::InputState::kDown:   fScene->pointerDown(pos); break;
            case skui::InputState::kMove:   fScene->pointerMove(pos); break;
            default: break;
        }
        return true;
    }
    return false;
}
