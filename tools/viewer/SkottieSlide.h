/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieSlide_DEFINED
#define SkottieSlide_DEFINED

#include "Slide.h"

namespace skottie { class Animation; }

class SkottieSlide : public Slide {
public:
    SkottieSlide(const SkString& name, const SkString& path);
    ~SkottieSlide() override = default;

    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

    SkISize getDimensions() const override;

    void draw(SkCanvas*) override;
    bool animate(const SkAnimTimer&) override;

    bool onChar(SkUnichar) override;

private:
    SkString                            fPath;
    std::unique_ptr<skottie::Animation> fAnimation;
    SkMSec                              fTimeBase  = 0;
    bool                                fShowAnimationInval = false;

    typedef Slide INHERITED;
};

class SkottieSlide2 : public Slide {
public:
    SkottieSlide2(const SkString& path);
    ~SkottieSlide2() override = default;

    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

    SkISize getDimensions() const override;

    void draw(SkCanvas*) override;
    bool animate(const SkAnimTimer&) override;
    bool onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState, uint32_t modifiers) override;
private:
    struct Rec {
        std::unique_ptr<skottie::Animation> fAnimation;
        SkMSec                              fTimeBase = 0;
        SkString                            fName;
        bool                                fShowAnimationInval = false;

        Rec(std::unique_ptr<skottie::Animation> anim);
        Rec(Rec&& o);
    };

    int findCell(float x, float y) const;

    SkString        fPath;
    SkTArray<Rec>   fAnims;

    int fTrackingCell = -1;

    typedef Slide INHERITED;
};

#endif // SkottieSlide_DEFINED
