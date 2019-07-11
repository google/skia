/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieSlide_DEFINED
#define SkottieSlide_DEFINED

#include "tools/viewer/Slide.h"

#if defined(SK_ENABLE_SKOTTIE)
#include "modules/skottie/include/Skottie.h"

namespace sksg    { class Scene;     }

class SkottieSlide : public Slide {
public:
    SkottieSlide(const SkString& name, const SkString& path);
    ~SkottieSlide() override = default;

    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

    SkISize getDimensions() const override;

    void draw(SkCanvas*) override;
    bool animate(double) override;

    bool onChar(SkUnichar) override;
    bool onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState, ModifierKey modifiers) override;

private:
    SkString                           fPath;
    sk_sp<skottie::Animation>          fAnimation;
    skottie::Animation::Builder::Stats fAnimationStats;
    SkSize                             fWinSize = SkSize::MakeEmpty();
    SkMSec                             fTimeBase  = 0;
    bool                               fShowAnimationInval = false,
                                       fShowAnimationStats = false;

    typedef Slide INHERITED;
};

#endif // SK_ENABLE_SKOTTIE

#endif // SkottieSlide_DEFINED
