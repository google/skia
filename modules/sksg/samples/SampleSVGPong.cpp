/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "include/private/SkTPin.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "tools/timer/TimeUtils.h"

#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGInvalidationController.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGPath.h"
#include "modules/sksg/include/SkSGRect.h"
#include "modules/sksg/include/SkSGScene.h"
#include "modules/sksg/include/SkSGTransform.h"

namespace {

static const SkRect kBounds     = SkRect::MakeLTRB(0.1f, 0.1f, 0.9f, 0.9f);
static const SkSize kPaddleSize = SkSize::Make(0.03f, 0.1f);
static const SkScalar kBallSize = 0.04f;
static const SkScalar kShadowOpacity       = 0.40f;
static const SkScalar kShadowParallax      = 0.04f;
static const SkScalar kBackgroundStroke    = 0.01f;
static const uint32_t kBackgroundDashCount = 20;

static const SkScalar kBallSpeedMax  = 0.0020f;
static const SkScalar kBallSpeedMin  = 0.0005f;
static const SkScalar kBallSpeedFuzz = 0.0002f;

static const SkScalar kTimeScaleMin = 0.0f;
static const SkScalar kTimeScaleMax = 5.0f;

// Box the value within [min, max), by applying infinite reflection on the interval endpoints.
SkScalar box_reflect(SkScalar v, SkScalar min, SkScalar max) {
    const SkScalar intervalLen = max - min;
    SkASSERT(intervalLen > 0);

    // f(v) is periodic in 2 * intervalLen: one normal progression + one reflection
    const SkScalar P = intervalLen * 2;
    // relative to P origin
    const SkScalar vP = v - min;
    // map to [0, P)
    const SkScalar vMod = (vP < 0) ? P - SkScalarMod(-vP, P) : SkScalarMod(vP, P);
    // reflect if needed, to map to [0, intervalLen)
    const SkScalar vInterval = vMod < intervalLen ? vMod : P - vMod;
    // finally, reposition relative to min
    return vInterval + min;
}

// Compute <t, y> for the trajectory intersection with the next vertical edge.
std::tuple<SkScalar, SkScalar> find_yintercept(const SkPoint& pos, const SkVector& spd,
                                               const SkRect& box) {
    const SkScalar edge = spd.fX > 0 ? box.fRight : box.fLeft;
    const SkScalar    t = (edge - pos.fX) / spd.fX;
    SkASSERT(t >= 0);
    const SkScalar   dY = t * spd.fY;

    return std::make_tuple(t, box_reflect(pos.fY + dY, box.fTop, box.fBottom));
}

void update_pos(const sk_sp<sksg::RRect>& rr, const SkPoint& pos) {
    // TODO: position setters on RRect?

    const auto r = rr->getRRect().rect();
    const auto offsetX = pos.x() - r.x(),
               offsetY = pos.y() - r.y();
    rr->setRRect(rr->getRRect().makeOffset(offsetX, offsetY));
}

}  // namespace

class PongView final : public Sample {
public:
    PongView() = default;

protected:
    void onOnceBeforeDraw() override {
        const SkRect fieldBounds = kBounds.makeOutset(kBallSize / 2, kBallSize / 2);
        const SkRRect ball = SkRRect::MakeOval(SkRect::MakeWH(kBallSize, kBallSize));
        const SkRRect paddle = SkRRect::MakeRectXY(SkRect::MakeWH(kPaddleSize.width(),
                                                                  kPaddleSize.height()),
                                                   kPaddleSize.width() / 2,
                                                   kPaddleSize.width() / 2);
        fBall.initialize(ball,
                         SkPoint::Make(kBounds.centerX(), kBounds.centerY()),
                         SkVector::Make(fRand.nextRangeScalar(kBallSpeedMin, kBallSpeedMax),
                                        fRand.nextRangeScalar(kBallSpeedMin, kBallSpeedMax)));
        fPaddle0.initialize(paddle,
                            SkPoint::Make(fieldBounds.left() - kPaddleSize.width() / 2,
                                          fieldBounds.centerY()),
                            SkVector::Make(0, 0));
        fPaddle1.initialize(paddle,
                            SkPoint::Make(fieldBounds.right() + kPaddleSize.width() / 2,
                                          fieldBounds.centerY()),
                            SkVector::Make(0, 0));

        // Background decoration.
        SkPathBuilder bgPath;
        bgPath.moveTo(kBounds.left() , fieldBounds.top())
              .lineTo(kBounds.right(), fieldBounds.top())
              .moveTo(kBounds.left() , fieldBounds.bottom())
              .lineTo(kBounds.right(), fieldBounds.bottom());
        // TODO: stroke-dash support would come in handy right about now.
        for (uint32_t i = 0; i < kBackgroundDashCount; ++i) {
            bgPath.moveTo(kBounds.centerX(),
                          kBounds.top() + (i + 0.25f) * kBounds.height() / kBackgroundDashCount)
                  .lineTo(kBounds.centerX(),
                          kBounds.top() + (i + 0.75f) * kBounds.height() / kBackgroundDashCount);
        }

        auto bg_path  = sksg::Path::Make(bgPath.detach());
        auto bg_paint = sksg::Color::Make(SK_ColorBLACK);
        bg_paint->setStyle(SkPaint::kStroke_Style);
        bg_paint->setStrokeWidth(kBackgroundStroke);

        auto ball_paint    = sksg::Color::Make(SK_ColorGREEN),
             paddle0_paint = sksg::Color::Make(SK_ColorBLUE),
             paddle1_paint = sksg::Color::Make(SK_ColorRED),
             shadow_paint  = sksg::Color::Make(SK_ColorBLACK);
        ball_paint->setAntiAlias(true);
        paddle0_paint->setAntiAlias(true);
        paddle1_paint->setAntiAlias(true);
        shadow_paint->setAntiAlias(true);
        shadow_paint->setOpacity(kShadowOpacity);

        // Build the scene graph.
        auto group = sksg::Group::Make();
        group->addChild(sksg::Draw::Make(std::move(bg_path), std::move(bg_paint)));
        group->addChild(sksg::Draw::Make(fPaddle0.shadowNode, shadow_paint));
        group->addChild(sksg::Draw::Make(fPaddle1.shadowNode, shadow_paint));
        group->addChild(sksg::Draw::Make(fBall.shadowNode, shadow_paint));
        group->addChild(sksg::Draw::Make(fPaddle0.objectNode, paddle0_paint));
        group->addChild(sksg::Draw::Make(fPaddle1.objectNode, paddle1_paint));
        group->addChild(sksg::Draw::Make(fBall.objectNode, ball_paint));

        // Handle everything in a normalized 1x1 space.
        fContentMatrix = sksg::Matrix<SkMatrix>::Make(
            SkMatrix::RectToRect(SkRect::MakeWH(1, 1),
                                 SkRect::MakeIWH(this->width(), this->height())));
        auto root = sksg::TransformEffect::Make(std::move(group), fContentMatrix);
        fScene = sksg::Scene::Make(std::move(root));

        // Off we go.
        this->updatePaddleStrategy();
    }

    SkString name() override { return SkString("SGPong"); }

    bool onChar(SkUnichar uni) override {
            switch (uni) {
                case '[':
                    fTimeScale = SkTPin(fTimeScale - 0.1f, kTimeScaleMin, kTimeScaleMax);
                    return true;
                case ']':
                    fTimeScale = SkTPin(fTimeScale + 0.1f, kTimeScaleMin, kTimeScaleMax);
                    return true;
                case 'I':
                    fShowInval = !fShowInval;
                    return true;
                default:
                    break;
            }
            return false;
    }

    void onSizeChange() override {
        if (fContentMatrix) {
            fContentMatrix->setMatrix(SkMatrix::RectToRect(SkRect::MakeWH(1, 1),
                                                           SkRect::MakeIWH(this->width(),
                                                                           this->height())));
        }

        this->INHERITED::onSizeChange();
    }

    void onDrawContent(SkCanvas* canvas) override {
        sksg::InvalidationController ic;
        fScene->render(canvas);

        if (fShowInval) {
            SkPaint fill, stroke;
            fill.setAntiAlias(true);
            fill.setColor(0x40ff0000);
            stroke.setAntiAlias(true);
            stroke.setColor(0xffff0000);
            stroke.setStyle(SkPaint::kStroke_Style);

            for (const auto& r : ic) {
                canvas->drawRect(r, fill);
                canvas->drawRect(r, stroke);
            }
        }
    }

    bool onAnimate(double nanos) override {
        // onAnimate may fire before the first draw.
        if (fScene) {
            SkScalar dt = (TimeUtils::NanosToMSec(nanos) - fLastTick) * fTimeScale;
            fLastTick = TimeUtils::NanosToMSec(nanos);

            fPaddle0.posTick(dt);
            fPaddle1.posTick(dt);
            fBall.posTick(dt);

            this->enforceConstraints();

            fPaddle0.updateDom();
            fPaddle1.updateDom();
            fBall.updateDom();
        }
        return true;
    }

private:
    struct Object {
        void initialize(const SkRRect& rrect, const SkPoint& p, const SkVector& s) {
            objectNode = sksg::RRect::Make(rrect);
            shadowNode = sksg::RRect::Make(rrect);

            pos = p;
            spd = s;
            size = SkSize::Make(rrect.width(), rrect.height());
        }

        void posTick(SkScalar dt) {
            pos += spd * dt;
        }

        void updateDom() {
            const SkPoint corner = pos - SkPoint::Make(size.width() / 2, size.height() / 2);
            update_pos(objectNode, corner);

            // Simulate parallax shadow for a centered light source.
            SkPoint shadowOffset = pos - SkPoint::Make(kBounds.centerX(), kBounds.centerY());
            shadowOffset.scale(kShadowParallax);
            const SkPoint shadowCorner = corner + shadowOffset;

            update_pos(shadowNode, shadowCorner);
        }

        sk_sp<sksg::RRect> objectNode,
                           shadowNode;
        SkPoint            pos;
        SkVector           spd;
        SkSize             size;
    };

    void enforceConstraints() {
        // Perfect vertical reflection.
        if (fBall.pos.fY < kBounds.fTop || fBall.pos.fY >= kBounds.fBottom) {
            fBall.spd.fY = -fBall.spd.fY;
            fBall.pos.fY = box_reflect(fBall.pos.fY, kBounds.fTop, kBounds.fBottom);
        }

        // Horizontal bounce - introduces a speed fuzz.
        if (fBall.pos.fX < kBounds.fLeft || fBall.pos.fX >= kBounds.fRight) {
            fBall.spd.fX = this->fuzzBallSpeed(-fBall.spd.fX);
            fBall.spd.fY = this->fuzzBallSpeed(fBall.spd.fY);
            fBall.pos.fX = box_reflect(fBall.pos.fX, kBounds.fLeft, kBounds.fRight);
            this->updatePaddleStrategy();
        }
    }

    SkScalar fuzzBallSpeed(SkScalar spd) {
        // The speed limits are absolute values.
        const SkScalar   sign = spd >= 0 ? 1.0f : -1.0f;
        const SkScalar fuzzed = fabs(spd) + fRand.nextRangeScalar(-kBallSpeedFuzz, kBallSpeedFuzz);

        return sign * SkTPin(fuzzed, kBallSpeedMin, kBallSpeedMax);
    }

    void updatePaddleStrategy() {
        Object* pitcher = fBall.spd.fX > 0 ? &fPaddle0 : &fPaddle1;
        Object* catcher = fBall.spd.fX > 0 ? &fPaddle1 : &fPaddle0;

        SkScalar t, yIntercept;
        std::tie(t, yIntercept) = find_yintercept(fBall.pos, fBall.spd, kBounds);

        // The pitcher aims for a neutral/centered position.
        pitcher->spd.fY = (kBounds.centerY() - pitcher->pos.fY) / t;

        // The catcher goes for the ball.  Duh.
        catcher->spd.fY = (yIntercept - catcher->pos.fY) / t;
    }

    std::unique_ptr<sksg::Scene>  fScene;
    sk_sp<sksg::Matrix<SkMatrix>> fContentMatrix;
    Object                        fPaddle0, fPaddle1, fBall;
    SkRandom                      fRand;

    SkMSec                        fLastTick  = 0;
    SkScalar                      fTimeScale = 1.0f;
    bool                          fShowInval = false;

    using INHERITED = Sample;
};

DEF_SAMPLE( return new PongView(); )
