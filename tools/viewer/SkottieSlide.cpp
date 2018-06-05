/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieSlide.h"

#if defined(SK_ENABLE_SKOTTIE)

#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "Skottie.h"

#include <cmath>

static void draw_stats_box(SkCanvas* canvas, const skottie::Animation::Stats& stats) {
    static constexpr SkRect kR = { 10, 10, 280, 120 };
    static constexpr SkScalar kTextSize = 20;

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xffeeeeee);
    paint.setTextSize(kTextSize);

    canvas->drawRect(kR, paint);

    paint.setColor(SK_ColorBLACK);

    const auto json_size = SkStringPrintf("Json size: %lu bytes",
                                          stats.fJsonSize);
    canvas->drawText(json_size.c_str(),
                     json_size.size(), kR.x() + 10, kR.y() + kTextSize * 1, paint);
    const auto animator_count = SkStringPrintf("Animator count: %lu",
                                               stats.fAnimatorCount);
    canvas->drawText(animator_count.c_str(),
                     animator_count.size(), kR.x() + 10, kR.y() + kTextSize * 2, paint);
    const auto json_parse_time = SkStringPrintf("Json parse time: %.3f ms",
                                                stats.fJsonParseTimeMS);
    canvas->drawText(json_parse_time.c_str(),
                     json_parse_time.size(), kR.x() + 10, kR.y() + kTextSize * 3, paint);
    const auto scene_parse_time = SkStringPrintf("Scene build time: %.3f ms",
                                                 stats.fSceneParseTimeMS);
    canvas->drawText(scene_parse_time.c_str(),
                     scene_parse_time.size(), kR.x() + 10, kR.y() + kTextSize * 4, paint);
    const auto total_load_time = SkStringPrintf("Total load time: %.3f ms",
                                                stats.fTotalLoadTimeMS);
    canvas->drawText(total_load_time.c_str(),
                     total_load_time.size(), kR.x() + 10, kR.y() + kTextSize * 5, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(kR, paint);
}

SkottieSlide::SkottieSlide(const SkString& name, const SkString& path)
    : fPath(path) {
    fName = name;
}

void SkottieSlide::load(SkScalar w, SkScalar h) {
    fAnimation = skottie::Animation::MakeFromFile(fPath.c_str(), nullptr, &fAnimationStats);
    fWinSize   = SkSize::Make(w, h);
    fTimeBase  = 0; // force a time reset

    if (fAnimation) {
        fAnimation->setShowInval(fShowAnimationInval);
        SkDebugf("loaded Bodymovin animation v: %s, size: [%f %f]\n",
                 fAnimation->version().c_str(),
                 fAnimation->size().width(),
                 fAnimation->size().height());
    } else {
        SkDebugf("failed to load Bodymovin animation: %s\n", fPath.c_str());
    }
}

void SkottieSlide::unload() {
    fAnimation.reset();
}

SkISize SkottieSlide::getDimensions() const {
    // We always scale to fill the window.
    return fWinSize.toCeil();
}

void SkottieSlide::draw(SkCanvas* canvas) {
    if (fAnimation) {
        SkAutoCanvasRestore acr(canvas, true);
        const auto dstR = SkRect::MakeSize(fWinSize);
        fAnimation->render(canvas, &dstR);

        if (fShowAnimationStats) {
            draw_stats_box(canvas, fAnimationStats);
        }
    }
}

bool SkottieSlide::animate(const SkAnimTimer& timer) {
    if (fTimeBase == 0) {
        // Reset the animation time.
        fTimeBase = timer.msec();
    }

    if (fAnimation) {
        const auto t = timer.msec() - fTimeBase;
        const auto d = fAnimation->duration() * 1000;
        fAnimation->seek(std::fmod(t, d) / d);
    }
    return true;
}

bool SkottieSlide::onChar(SkUnichar c) {
    switch (c) {
    case 'I':
        fShowAnimationStats = !fShowAnimationStats;
        break;
    default:
        break;
    }

    return INHERITED::onChar(c);
}

bool SkottieSlide::onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState state, uint32_t) {
    switch (state) {
    case sk_app::Window::kUp_InputState:
        fShowAnimationInval = !fShowAnimationInval;
        fShowAnimationStats = !fShowAnimationStats;
        fAnimation->setShowInval(fShowAnimationInval);
        break;
    default:
        break;
    }

    return false;
}

#endif // SK_ENABLE_SKOTTIE
