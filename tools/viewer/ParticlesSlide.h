/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef ParticlesSlide_DEFINED
#define ParticlesSlide_DEFINED

#include "tools/viewer/Slide.h"

#include "include/core/SkPath.h"
#include "include/private/SkTArray.h"
#include "include/utils/SkRandom.h"

class SkParticleEffect;
class SkParticleEffectParams;

namespace skresources { class ResourceProvider; }

class ParticlesSlide : public Slide {
public:
    ParticlesSlide();

    void load(SkScalar winWidth, SkScalar winHeight) override;
    void draw(SkCanvas* canvas) override;
    bool animate(double) override;

    bool onMouse(SkScalar x, SkScalar y, skui::InputState state,
                 skui::ModifierKey modifiers) override;

private:
    class TestingResourceProvider;

    void loadEffects(const char* dirname);

    SkRandom fRandom;
    bool fAnimated = false;
    double fAnimationTime = 0;
    SkPoint fMousePos = { 0, 0 };

    struct LoadedEffect {
        SkString fName;
        sk_sp<SkParticleEffectParams> fParams;
    };
    SkTArray<LoadedEffect> fLoaded;

    struct RunningEffect {
        SkString fName;
        sk_sp<SkParticleEffect> fEffect;
        bool fTrackMouse;
    };
    SkTArray<RunningEffect> fRunning;

    sk_sp<TestingResourceProvider> fResourceProvider;
};

#endif
