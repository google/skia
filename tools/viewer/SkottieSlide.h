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
#include "modules/sksg/include/SkSGInvalidationController.h"

#include <vector>

namespace sksg    { class Scene;     }

class SkottieSlide : public Slide {
public:
    SkottieSlide(const SkString& name, const SkString& path);
    ~SkottieSlide() override = default;

    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;
    void resize(SkScalar, SkScalar) override;

    SkISize getDimensions() const override;

    void draw(SkCanvas*) override;
    bool animate(double) override;

    bool onChar(SkUnichar) override;
    bool onMouse(SkScalar x, SkScalar y, skui::InputState, skui::ModifierKey modifiers) override;

private:
    SkRect UIArea() const;
    void renderUI();

    const SkString                     fPath;

    sk_sp<skottie::Animation>          fAnimation;
    skottie::Animation::Builder::Stats fAnimationStats;
    sksg::InvalidationController       fInvalController;
    std::vector<float>                 fFrameTimes;
    SkSize                             fWinSize            = SkSize::MakeEmpty();
    double                             fTimeBase           = 0,
                                       fFrameRate          = 0;
    const char*                        fFrameRateLabel     = nullptr;
    float                              fCurrentFrame       = 0;
    bool                               fShowAnimationInval = false,
                                       fShowAnimationStats = false,
                                       fShowUI             = false,
                                       fDraggingProgress   = false,
                                       fPreferGlyphPaths   = false;

    using INHERITED = Slide;
};

#endif // SK_ENABLE_SKOTTIE

#endif // SkottieSlide_DEFINED
