/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef NIMASlide_DEFINED
#define NIMASlide_DEFINED

#include "tools/viewer/Slide.h"

#include "experimental/nima/NimaActor.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkVertices.h"

class NIMASlide : public Slide {
public:
    NIMASlide(const SkString& name, const SkString& path);
    ~NIMASlide() override;

    SkISize getDimensions() const override;

    void draw(SkCanvas* canvas) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;
    bool animate(const AnimTimer& timer) override;

    bool onChar(SkUnichar c) override;
    bool onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState state,
                 uint32_t modifiers) override;

private:
    void resetActor();

    void renderGUI();

private:
    std::string                fBasePath;
    std::unique_ptr<NimaActor> fActor;
    int                        fAnimationIndex;

    bool fPlaying;
    float fTime;
    uint32_t fRenderFlags;
};

#endif
