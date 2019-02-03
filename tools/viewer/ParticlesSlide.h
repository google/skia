/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef ParticlesSlide_DEFINED
#define ParticlesSlide_DEFINED

#include "Slide.h"

#include "SkPath.h"
#include "SkRandom.h"

class SkParticleEffect;

class ParticlesSlide : public Slide {
public:
    ParticlesSlide();
    ~ParticlesSlide() override;

    SkISize getDimensions() const override;

    void draw(SkCanvas* canvas) override;
    bool animate(const SkAnimTimer& timer) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

    bool onChar(SkUnichar c) override;
    bool onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState state,
                 uint32_t modifiers) override;

private:
    SkRandom fRandom;
    sk_sp<SkParticleEffect>  fEffect;
};

#endif
