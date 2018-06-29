/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef NIMASlide_DEFINED
#define NIMASlide_DEFINED

#include "Slide.h"

#include "SkCanvas.h"
#include "SkVertices.h"
#include <nima/Actor.hpp>
#include <nima/ActorImage.hpp>
#include <nima/Animation/ActorAnimationInstance.hpp>
#include <nima/Vec2D.hpp>

class NIMAActor;
class NIMAActorImage;

enum RenderMode {
    kBackend_RenderMode   = 0,
    kImmediate_RenderMode = 1,
};

class NIMASlide : public Slide {
public:
    NIMASlide(const SkString& name, const SkString& path);
    ~NIMASlide() override;

    void draw(SkCanvas* canvas) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;
    bool animate(const SkAnimTimer& timer) override;

    bool onChar(SkUnichar c) override;
    bool onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState state,
                 uint32_t modifiers) override;

private:
    void resetActor();

    void renderGUI();

private:
    std::string                fBasePath;
    std::unique_ptr<NIMAActor> fActor;

    bool fPlaying;
    float fTime;
    RenderMode fRenderMode;

    nima::ActorAnimationInstance* fAnimation;
    int                           fAnimationIndex;
};

#endif
