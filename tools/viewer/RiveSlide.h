/*
 * Copyright 2022 Rive Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RiveSlide_DEFINED
#define RiveSlide_DEFINED

#include "tools/viewer/Slide.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"

#include <rive/artboard.hpp>
#include <rive/file.hpp>
#include <rive/animation/state_machine_instance.hpp>
#include <rive/animation/linear_animation_instance.hpp>

#include <vector>

class RiveSlide : public Slide {
public:
    RiveSlide(const SkString& name, const SkString& path);
    ~RiveSlide() override = default;

    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;
    void resize(SkScalar, SkScalar) override;

    SkISize getDimensions() const override;

    void draw(SkCanvas*) override;
    bool animate(double) override;

    bool onMouse(SkScalar, SkScalar, skui::InputState, skui::ModifierKey) override;

private:
    const SkString fPath;
    SkSize fWinSize;
    double fSecsBase = 0;
    SkRect fBounds;
    SkMatrix fMat;

    std::unique_ptr<rive::File> fFile;
    std::unique_ptr<rive::ArtboardInstance> fArtboard;
    std::unique_ptr<rive::Scene> fScene;

    using INHERITED = Slide;
};

#endif // RiveSlide_DEFINED
