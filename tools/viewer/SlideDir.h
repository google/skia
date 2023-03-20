/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SlideDir_DEFINED
#define SlideDir_DEFINED

#include "tools/viewer/Slide.h"

#include "include/private/base/SkTArray.h"

#include <memory>
#include <vector>

class SkString;

namespace sksg {

class Group;
class Scene;

}  // namespace sksg

class SlideDir final : public Slide {
public:
    SlideDir(const SkString& name, skia_private::TArray<sk_sp<Slide>>&&,
             int columns = kDefaultColumnCount);

    class Animator;

protected:
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

    SkISize getDimensions() const override;

    void draw(SkCanvas*) override;
    bool animate(double) override;

    bool onChar(SkUnichar) override;
    bool onMouse(SkScalar x, SkScalar y, skui::InputState, skui::ModifierKey modifiers) override;

private:
    struct Rec;
    class FocusController;

    static constexpr int kDefaultColumnCount = 4;

    const Rec* findCell(float x, float y) const;

    const skia_private::TArray<sk_sp<Slide>>       fSlides;
    std::unique_ptr<FocusController>   fFocusController;
    const int                          fColumns;

    std::vector<Rec>                fRecs;
    std::unique_ptr<sksg::Scene>       fScene;
    std::vector<sk_sp<Animator>>       fSceneAnimators;
    sk_sp<sksg::Group>                 fRoot;

    SkSize                             fWinSize  = SkSize::MakeEmpty();
    SkSize                             fCellSize = SkSize::MakeEmpty();
    SkMSec                             fTimeBase = 0;

    const Rec*                         fTrackingCell = nullptr;
    SkPoint                            fTrackingPos  = SkPoint::Make(0, 0);
};

#endif // SlideDir_DEFINED
