/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/SlideDir.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkCubicMap.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTPin.h"
#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGPlane.h"
#include "modules/sksg/include/SkSGRect.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "modules/sksg/include/SkSGScene.h"
#include "modules/sksg/include/SkSGText.h"
#include "modules/sksg/include/SkSGTransform.h"
#include "tools/timer/TimeUtils.h"

#include <cmath>
#include <utility>

class SlideDir::Animator : public SkRefCnt {
public:
    Animator(const Animator&) = delete;
    Animator& operator=(const Animator&) = delete;

    void tick(float t) { this->onTick(t); }

protected:
    Animator() = default;

    virtual void onTick(float t) = 0;
};

namespace {

static constexpr float  kAspectRatio   = 1.5f;
static constexpr float  kLabelSize     = 12.0f;
static constexpr SkSize kPadding       = { 12.0f , 24.0f };

static constexpr float   kFocusDuration = 500;
static constexpr SkSize  kFocusInset    = { 100.0f, 100.0f };
static constexpr SkPoint kFocusCtrl0    = {   0.3f,   1.0f };
static constexpr SkPoint kFocusCtrl1    = {   0.0f,   1.0f };
static constexpr SkColor kFocusShade    = 0xa0000000;

// TODO: better unfocus binding?
static constexpr SkUnichar kUnfocusKey = ' ';

class SlideAdapter final : public sksg::RenderNode {
public:
    explicit SlideAdapter(sk_sp<Slide> slide)
        : fSlide(std::move(slide)) {
        SkASSERT(fSlide);
    }

    sk_sp<SlideDir::Animator> makeForwardingAnimator() {
        // Trivial sksg::Animator -> skottie::Animation tick adapter
        class ForwardingAnimator final : public SlideDir::Animator {
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

        return sk_make_sp<ForwardingAnimator>(sk_ref_sp(this));
    }

protected:
    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        const auto isize = fSlide->getDimensions();
        return SkRect::MakeIWH(isize.width(), isize.height());
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->clipRect(SkRect::Make(fSlide->getDimensions()), true);

        // TODO: commit the context?
        fSlide->draw(canvas);
    }

    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; }

private:
    void tick(SkMSec t) {
        fSlide->animate(t * 1e6);
        this->invalidate();
    }

    const sk_sp<Slide> fSlide;

    using INHERITED = sksg::RenderNode;
};

SkMatrix SlideMatrix(const sk_sp<Slide>& slide, const SkRect& dst) {
    const auto slideSize = slide->getDimensions();
    return SkMatrix::RectToRect(SkRect::MakeIWH(slideSize.width(), slideSize.height()), dst,
                                SkMatrix::kCenter_ScaleToFit);
}

} // namespace

struct SlideDir::Rec {
    sk_sp<Slide>                  fSlide;
    sk_sp<sksg::RenderNode>       fSlideRoot;
    sk_sp<sksg::Matrix<SkMatrix>> fMatrix;
    SkRect                        fRect;
};

class SlideDir::FocusController final : public Animator {
public:
    FocusController(const SlideDir* dir, const SkRect& focusRect)
        : fDir(dir)
        , fRect(focusRect)
        , fTarget(nullptr)
        , fMap(kFocusCtrl1, kFocusCtrl0)
        , fState(State::kIdle) {
        fShadePaint = sksg::Color::Make(kFocusShade);
        fShade = sksg::Draw::Make(sksg::Plane::Make(), fShadePaint);
    }

    bool hasFocus() const { return fState == State::kFocused; }

    void startFocus(const Rec* target) {
        if (fState != State::kIdle)
            return;

        fTarget = target;

        // Move the shade & slide to front.
        fDir->fRoot->removeChild(fTarget->fSlideRoot);
        fDir->fRoot->addChild(fShade);
        fDir->fRoot->addChild(fTarget->fSlideRoot);

        fM0 = SlideMatrix(fTarget->fSlide, fTarget->fRect);
        fM1 = SlideMatrix(fTarget->fSlide, fRect);

        fOpacity0 = 0;
        fOpacity1 = 1;

        fTimeBase = 0;
        fState = State::kFocusing;

        // Push initial state to the scene graph.
        this->onTick(fTimeBase);
    }

    void startUnfocus() {
        SkASSERT(fTarget);

        using std::swap;
        swap(fM0, fM1);
        swap(fOpacity0, fOpacity1);

        fTimeBase = 0;
        fState = State::kUnfocusing;
    }

    bool onMouse(SkScalar x, SkScalar y, skui::InputState state, skui::ModifierKey modifiers) {
        SkASSERT(fTarget);

        if (!fRect.contains(x, y)) {
            this->startUnfocus();
            return true;
        }

        // Map coords to slide space.
        const auto xform = SkMatrix::RectToRect(fRect, SkRect::MakeSize(fDir->fWinSize),
                                                SkMatrix::kCenter_ScaleToFit);
        const auto pt = xform.mapXY(x, y);

        return fTarget->fSlide->onMouse(pt.x(), pt.y(), state, modifiers);
    }

    bool onChar(SkUnichar c) {
        SkASSERT(fTarget);

        return fTarget->fSlide->onChar(c);
    }

protected:
    void onTick(float t) override {
        if (!this->isAnimating())
            return;

        if (!fTimeBase) {
            fTimeBase = t;
        }

        const auto rel_t = (t - fTimeBase) / kFocusDuration,
                   map_t = SkTPin(fMap.computeYFromX(rel_t), 0.0f, 1.0f);

        SkMatrix m;
        for (int i = 0; i < 9; ++i) {
            m[i] = fM0[i] + map_t * (fM1[i] - fM0[i]);
        }

        SkASSERT(fTarget);
        fTarget->fMatrix->setMatrix(m);

        const auto shadeOpacity = fOpacity0 + map_t * (fOpacity1 - fOpacity0);
        fShadePaint->setOpacity(shadeOpacity);

        if (rel_t < 1)
            return;

        switch (fState) {
        case State::kFocusing:
            fState = State::kFocused;
            break;
        case State::kUnfocusing:
            fState  = State::kIdle;
            fDir->fRoot->removeChild(fShade);
            break;

        case State::kIdle:
        case State::kFocused:
            SkASSERT(false);
            break;
        }
    }

private:
    enum class State {
        kIdle,
        kFocusing,
        kUnfocusing,
        kFocused,
    };

    bool isAnimating() const { return fState == State::kFocusing || fState == State::kUnfocusing; }

    const SlideDir*         fDir;
    const SkRect            fRect;
    const Rec*              fTarget;

    SkCubicMap              fMap;
    sk_sp<sksg::RenderNode> fShade;
    sk_sp<sksg::PaintNode>  fShadePaint;

    SkMatrix        fM0       = SkMatrix::I(),
                    fM1       = SkMatrix::I();
    float           fOpacity0 = 0,
                    fOpacity1 = 1,
                    fTimeBase = 0;
    State           fState    = State::kIdle;

    using INHERITED = Animator;
};

SlideDir::SlideDir(const SkString& name, SkTArray<sk_sp<Slide>>&& slides, int columns)
    : fSlides(std::move(slides))
    , fColumns(columns) {
    fName = name;
}

static sk_sp<sksg::RenderNode> MakeLabel(const SkString& txt,
                                         const SkPoint& pos,
                                         const SkMatrix& dstXform) {
    const auto size = kLabelSize / std::sqrt(dstXform.getScaleX() * dstXform.getScaleY());
    auto text = sksg::Text::Make(nullptr, txt);
    text->setEdging(SkFont::Edging::kAntiAlias);
    text->setSize(size);
    text->setAlign(SkTextUtils::kCenter_Align);
    text->setPosition(pos + SkPoint::Make(0, size));

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

    fWinSize = SkSize::Make(winWidth, winHeight);
    const auto  cellWidth =  winWidth / fColumns;
    fCellSize = SkSize::Make(cellWidth, cellWidth / kAspectRatio);

    fRoot = sksg::Group::Make();

    for (int i = 0; i < fSlides.count(); ++i) {
        const auto& slide     = fSlides[i];
        slide->load(winWidth, winHeight);

        const auto  slideSize = slide->getDimensions();
        const auto  cell      = SkRect::MakeXYWH(fCellSize.width()  * (i % fColumns),
                                                 fCellSize.height() * (i / fColumns),
                                                 fCellSize.width(),
                                                 fCellSize.height()),
                    slideRect = cell.makeInset(kPadding.width(), kPadding.height());

        auto slideMatrix = sksg::Matrix<SkMatrix>::Make(SlideMatrix(slide, slideRect));
        auto adapter     = sk_make_sp<SlideAdapter>(slide);
        auto slideGrp    = sksg::Group::Make();
        slideGrp->addChild(sksg::Draw::Make(sksg::Rect::Make(SkRect::MakeIWH(slideSize.width(),
                                                                             slideSize.height())),
                                            sksg::Color::Make(0xfff0f0f0)));
        slideGrp->addChild(adapter);
        slideGrp->addChild(MakeLabel(slide->getName(),
                                     SkPoint::Make(slideSize.width() / 2, slideSize.height()),
                                     slideMatrix->getMatrix()));
        auto slideRoot = sksg::TransformEffect::Make(std::move(slideGrp), slideMatrix);

        fSceneAnimators.push_back(adapter->makeForwardingAnimator());

        fRoot->addChild(slideRoot);
        fRecs.push_back({ slide, slideRoot, slideMatrix, slideRect });
    }

    fScene = sksg::Scene::Make(fRoot);

    const auto focusRect = SkRect::MakeSize(fWinSize).makeInset(kFocusInset.width(),
                                                                kFocusInset.height());
    fFocusController = std::make_unique<FocusController>(this, focusRect);
}

void SlideDir::unload() {
    for (const auto& slide : fSlides) {
        slide->unload();
    }

    fRecs.reset();
    fScene.reset();
    fFocusController.reset();
    fRoot.reset();
    fTimeBase = 0;
}

SkISize SlideDir::getDimensions() const {
    return SkSize::Make(fWinSize.width(),
                        fCellSize.height() * (1 + (fSlides.count() - 1) / fColumns)).toCeil();
}

void SlideDir::draw(SkCanvas* canvas) {
    fScene->render(canvas);
}

bool SlideDir::animate(double nanos) {
    SkMSec msec = TimeUtils::NanosToMSec(nanos);
    if (fTimeBase == 0) {
        // Reset the animation time.
        fTimeBase = msec;
    }

    const auto t = msec - fTimeBase;
    for (const auto& anim : fSceneAnimators) {
        anim->tick(t);
    }
    fFocusController->tick(t);

    return true;
}

bool SlideDir::onChar(SkUnichar c) {
    if (fFocusController->hasFocus()) {
        if (c == kUnfocusKey) {
            fFocusController->startUnfocus();
            return true;
        }
        return fFocusController->onChar(c);
    }

    return false;
}

bool SlideDir::onMouse(SkScalar x, SkScalar y, skui::InputState state,
                       skui::ModifierKey modifiers) {
    modifiers &= ~skui::ModifierKey::kFirstPress;
    if (state == skui::InputState::kMove || sknonstd::Any(modifiers))
        return false;

    if (fFocusController->hasFocus()) {
        return fFocusController->onMouse(x, y, state, modifiers);
    }

    const auto* cell = this->findCell(x, y);
    if (!cell)
        return false;

    static constexpr SkScalar kClickMoveTolerance = 4;

    switch (state) {
    case skui::InputState::kDown:
        fTrackingCell = cell;
        fTrackingPos = SkPoint::Make(x, y);
        break;
    case skui::InputState::kUp:
        if (cell == fTrackingCell &&
            SkPoint::Distance(fTrackingPos, SkPoint::Make(x, y)) < kClickMoveTolerance) {
            fFocusController->startFocus(cell);
        }
        break;
    default:
        break;
    }

    return false;
}

const SlideDir::Rec* SlideDir::findCell(float x, float y) const {
    // TODO: use SG hit testing instead of layout info?
    const auto size = this->getDimensions();
    if (x < 0 || y < 0 || x >= size.width() || y >= size.height()) {
        return nullptr;
    }

    const int col = static_cast<int>(x / fCellSize.width()),
              row = static_cast<int>(y / fCellSize.height()),
              idx = row * fColumns + col;

    return idx < fRecs.count() ? &fRecs[idx] : nullptr;
}
