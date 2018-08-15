/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkinningSlide_DEFINED
#define SkinningSlide_DEFINED

#include "Slide.h"

#include "SkCanvas.h"
#include "SkVertices.h"

class SampleBone;
class SampleAnimation;

class SkinningSlide : public Slide {
public:
    SkinningSlide();
    ~SkinningSlide() override;

    void draw(SkCanvas* canvas) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;
    bool animate(const SkAnimTimer& timer) override;

    bool onChar(SkUnichar c) override;
    bool onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState state,
                 uint32_t modifiers) override;

protected:
    virtual SkISize onSize();
    virtual void onDraw(SkCanvas* canvas);
    virtual void onAnimate(const SkAnimTimer& timer);

private:
    sk_sp<SkVertices> generateQuad(float width, float height,
                                   float imageWidth, float imageHeight,
                                   int xTessellations, int yTessellations);

    void displayBoneInfo(SampleBone& sampleBone);
    void displayControls();

private:
    std::vector<SkVertices::Bone> fBones;

    sk_sp<SkSurface>  fSurface;
    sk_sp<SkVertices> fVertices;
    float fLOD;

    bool fMouseDown;

    bool fDrag;
    SkPoint fDragStart;

    bool fShowBones;
    std::unique_ptr<SampleAnimation> fAnimation;
    float fLastTime;
    bool fPlaying;

    typedef Slide INHERITED;
};

#endif
