/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SlideDir_DEFINED
#define SlideDir_DEFINED

#include "tools/viewer/Slide.h"

#include "include/private/SkTArray.h"

class SkString;

namespace sksg {

class Group;
class Scene;

}

class SlideDir final : public Slide {
public:
    SlideDir(const SkString& name, SkTArray<sk_sp<Slide>>&&,
             int columns = kDefaultColumnCount);

protected:
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

    SkISize getDimensions() const override;

    void draw(SkCanvas*) override;
    bool animate(const AnimTimer&) override;

    bool onChar(SkUnichar) override;
    bool onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState, ModifierKey modifiers) override;

private:
    struct Rec;
    class FocusController;

    static constexpr int kDefaultColumnCount = 4;

    const Rec* findCell(float x, float y) const;

    const SkTArray<sk_sp<Slide>>       fSlides;
    std::unique_ptr<FocusController>   fFocusController;
    const int                          fColumns;

    SkTArray<Rec, true>                fRecs;
    std::unique_ptr<sksg::Scene>       fScene;
    sk_sp<sksg::Group>                 fRoot;

    SkSize                             fWinSize  = SkSize::MakeEmpty();
    SkSize                             fCellSize = SkSize::MakeEmpty();
    SkMSec                             fTimeBase = 0;

    const Rec*                         fTrackingCell = nullptr;
    SkPoint                            fTrackingPos  = SkPoint::Make(0, 0);

    using INHERITED = Slide;
};

#endif // SlideDir_DEFINED
