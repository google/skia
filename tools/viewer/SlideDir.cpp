/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SlideDir.h"

#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkMakeUnique.h"
#include "SkSGColor.h"
#include "SkSGDraw.h"
#include "SkSGGroup.h"
#include "SkSGRenderNode.h"
#include "SkSGScene.h"
#include "SkSGText.h"
#include "SkSGTransform.h"
#include "SkTypeface.h"

namespace {

static constexpr float  kAspectRatio = 1.5f;
static constexpr float  kLabelSize   = 12.0f;
static constexpr SkSize kPadding     = { 12.0f, 24.0f };

class SlideAdapter final : public sksg::RenderNode {
public:
    explicit SlideAdapter(sk_sp<Slide> slide)
        : fSlide(std::move(slide)) {
        SkASSERT(fSlide);
    }

    std::unique_ptr<sksg::Animator> makeForwardingAnimator() {
        // Trivial sksg::Animator -> skottie::Animation tick adapter
        class ForwardingAnimator final : public sksg::Animator {
        public:
            explicit ForwardingAnimator(sk_sp<SlideAdapter> adapter)
                : fAdapter(std::move(adapter)) {}

        protected:
            void onTick(float t) override {
                fAdapter->tick(SkScalarRoundToInt(t));
            }

        private:
            sk_sp<SlideAdapter> fAdapter;
        };

        return skstd::make_unique<ForwardingAnimator>(sk_ref_sp(this));
    }

protected:
    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        const auto isize = fSlide->getDimensions();
        return SkRect::MakeIWH(isize.width(), isize.height());
    }

    void onRender(SkCanvas* canvas) const override {
        fSlide->draw(canvas);
    }

private:
    void tick(SkMSec t) {
        fSlide->animate(SkAnimTimer(0, t * 1e6, SkAnimTimer::kRunning_State));
        this->invalidate();
    }

    const sk_sp<Slide> fSlide;

    using INHERITED = sksg::RenderNode;
};

} // namespace

struct SlideDir::Rec {
    sk_sp<sksg::Matrix> fMatrix;
    SkRect              fRect;
};

SlideDir::SlideDir(const SkString& name, SkTArray<sk_sp<Slide>, true>&& slides, int columns)
    : fSlides(std::move(slides))
    , fColumns(columns) {
    fName = name;
}

static sk_sp<sksg::RenderNode> MakeLabel(const SkString& txt, const SkRect& dst) {
    auto text = sksg::Text::Make(nullptr, txt);
    text->setFlags(SkPaint::kAntiAlias_Flag);
    text->setSize(kLabelSize);
    text->setAlign(SkPaint::kCenter_Align);
    text->setPosition(SkPoint::Make(dst.centerX(), dst.bottom()));

    return sksg::Draw::Make(std::move(text), sksg::Color::Make(SK_ColorBLACK));
}

void SlideDir::load(SkScalar winWidth, SkScalar winHeight) {
    // Build a global scene using transformed animation fragments:
    //
    // [Group(root)]
    //     [Transform]
    //         [Group]
    //             [AnimationWrapper]
    //             [Draw]
    //                 [Text]
    //                 [Color]
    //     [Transform]
    //         [Group]
    //             [AnimationWrapper]
    //             [Draw]
    //                 [Text]
    //                 [Color]
    //     ...
    //

    fSize = SkSize::Make(winWidth, winHeight).toCeil();

    const auto  cellWidth =  winWidth / fColumns,
               cellHeight = cellWidth / kAspectRatio;

    sksg::AnimatorList sceneAnimators;
    auto root = sksg::Group::Make();

    for (int i = 0; i < fSlides.count(); ++i) {
        const auto& slide     = fSlides[i];
        slide->load(winWidth, winHeight);

        const auto  slideSize = slide->getDimensions();
        const auto  cell      = SkRect::MakeXYWH(cellWidth  * (i % fColumns),
                                                 cellHeight * (i / fColumns),
                                                 cellWidth,
                                                 cellHeight),
                    slideRect = cell.makeInset(kPadding.width(), kPadding.height());

        auto matrix = sksg::Matrix::Make(
            SkMatrix::MakeRectToRect(SkRect::MakeIWH(slideSize.width(), slideSize.height()),
                                     slideRect,
                                     SkMatrix::kCenter_ScaleToFit));

        auto adapter  = sk_make_sp<SlideAdapter>(slide);
        auto slideGrp = sksg::Group::Make();
        slideGrp->addChild(sksg::Transform::Make(adapter, matrix));
        slideGrp->addChild(MakeLabel(slide->getName(), cell));

        sceneAnimators.push_back(adapter->makeForwardingAnimator());
        root->addChild(std::move(slideGrp));

        fRecs.push_back({ matrix, slideRect });
    }

    fScene = sksg::Scene::Make(std::move(root), std::move(sceneAnimators));
}

void SlideDir::unload() {
    for (const auto& slide : fSlides) {
        slide->unload();
    }

    fRecs.reset();
    fScene.reset();
    fTimeBase = 0;
}

SkISize SlideDir::getDimensions() const {
    return fSize;
}

void SlideDir::draw(SkCanvas* canvas) {
    fScene->render(canvas);
}

bool SlideDir::animate(const SkAnimTimer& timer) {
    if (fTimeBase == 0) {
        // Reset the animation time.
        fTimeBase = timer.msec();
    }
    fScene->animate(timer.msec() - fTimeBase);

    return true;
}

bool SlideDir::onChar(SkUnichar c) {
    return false;
}

bool SlideDir::onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState, uint32_t modifiers) {
    return false;
}
