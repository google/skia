/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SlideDir_DEFINED
#define SlideDir_DEFINED

#include "Slide.h"

#include "SkTArray.h"

class SkString;

namespace sksg { class Scene; }

class SlideDir final : public Slide {
public:
    SlideDir(const SkString& name, SkTArray<sk_sp<Slide>, true>&&,
             int columns = kDefaultColumnCount);

protected:
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

    SkISize getDimensions() const override;

    void draw(SkCanvas*) override;
    bool animate(const SkAnimTimer&) override;

    bool onChar(SkUnichar) override;
    bool onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState, uint32_t modifiers) override;

private:
    static constexpr int kDefaultColumnCount = 4;

    struct Rec;

    const SkTArray<sk_sp<Slide>, true> fSlides;
    const int                          fColumns;

    SkTArray<Rec, true>          fRecs;
    std::unique_ptr<sksg::Scene> fScene;

    SkISize                      fSize     = SkISize::MakeEmpty();
    SkMSec                       fTimeBase = 0;

    using INHERITED = Slide;
};

#endif // SlideDir_DEFINED
