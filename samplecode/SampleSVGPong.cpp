/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkColor.h"
#include "SkRandom.h"
#include "SkRRect.h"
#include "SkSVGDOM.h"
#include "SkSVGG.h"
#include "SkSVGPath.h"
#include "SkSVGRect.h"
#include "SkSVGSVG.h"

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

sk_sp<SkSVGRect> make_svg_rrect(const SkRRect& rrect) {
    sk_sp<SkSVGRect> node = SkSVGRect::Make();
    node->setX(SkSVGLength(rrect.rect().x()));
    node->setY(SkSVGLength(rrect.rect().y()));
    node->setWidth(SkSVGLength(rrect.width()));
    node->setHeight(SkSVGLength(rrect.height()));
    node->setRx(SkSVGLength(rrect.getSimpleRadii().x()));
    node->setRy(SkSVGLength(rrect.getSimpleRadii().y()));

    return node;
}

} // anonymous ns

class SVGPongView final : public SampleView {
public:
    SVGPongView() {}

protected:
    void onOnceBeforeDraw() override {
        const SkRect fieldBounds = kBounds.makeOutset(kBallSize / 2, kBallSize / 2);
        const SkRRect ball = SkRRect::MakeOval(SkRect::MakeWH(kBallSize, kBallSize));
        const SkRRect paddle = SkRRect::MakeRectXY(SkRect::MakeWH(kPaddleSize.width(),
                                                                  kPaddleSize.height()),
                                                   kPaddleSize.width() / 2,
                                                   kPaddleSize.width() / 2);
        fBall.initialize(ball,
                         SK_ColorGREEN,
                         SkPoint::Make(kBounds.centerX(), kBounds.centerY()),
                         SkVector::Make(fRand.nextRangeScalar(kBallSpeedMin, kBallSpeedMax),
                                        fRand.nextRangeScalar(kBallSpeedMin, kBallSpeedMax)));
        fPaddle0.initialize(paddle,
                            SK_ColorBLUE,
                            SkPoint::Make(fieldBounds.left() - kPaddleSize.width() / 2,
                                          fieldBounds.centerY()),
                            SkVector::Make(0, 0));
        fPaddle1.initialize(paddle,
                            SK_ColorRED,
                            SkPoint::Make(fieldBounds.right() + kPaddleSize.width() / 2,
                                          fieldBounds.centerY()),
                            SkVector::Make(0, 0));

        // Background decoration.
        SkPath bgPath;
        bgPath.moveTo(kBounds.left() , fieldBounds.top());
        bgPath.lineTo(kBounds.right(), fieldBounds.top());
        bgPath.moveTo(kBounds.left() , fieldBounds.bottom());
        bgPath.lineTo(kBounds.right(), fieldBounds.bottom());
        // TODO: stroke-dash support would come in handy right about now.
        for (uint32_t i = 0; i < kBackgroundDashCount; ++i) {
            bgPath.moveTo(kBounds.centerX(),
                          kBounds.top() + (i + 0.25f) * kBounds.height() / kBackgroundDashCount);
            bgPath.lineTo(kBounds.centerX(),
                          kBounds.top() + (i + 0.75f) * kBounds.height() / kBackgroundDashCount);
        }

        sk_sp<SkSVGPath> bg = SkSVGPath::Make();
        bg->setPath(bgPath);
        bg->setFill(SkSVGPaint(SkSVGPaint::Type::kNone));
        bg->setStroke(SkSVGPaint(SkSVGColorType(SK_ColorBLACK)));
        bg->setStrokeWidth(SkSVGLength(kBackgroundStroke));

        // Build the SVG DOM tree.
        sk_sp<SkSVGSVG> root = SkSVGSVG::Make();
        root->appendChild(std::move(bg));
        root->appendChild(fPaddle0.shadowNode);
        root->appendChild(fPaddle1.shadowNode);
        root->appendChild(fBall.shadowNode);
        root->appendChild(fPaddle0.objectNode);
        root->appendChild(fPaddle1.objectNode);
        root->appendChild(fBall.objectNode);

        // Handle everything in a normalized 1x1 space.
        root->setViewBox(SkSVGViewBoxType(SkRect::MakeWH(1, 1)));

        fDom = sk_sp<SkSVGDOM>(new SkSVGDOM());
        fDom->setContainerSize(SkSize::Make(this->width(), this->height()));
        fDom->setRoot(std::move(root));

        // Off we go.
        this->updatePaddleStrategy();
    }

    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "SVGPong");
            return true;
        }

        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            switch (uni) {
                case '[':
                    fTimeScale = SkTPin(fTimeScale - 0.1f, kTimeScaleMin, kTimeScaleMax);
                    return true;
                case ']':
                    fTimeScale = SkTPin(fTimeScale + 0.1f, kTimeScaleMin, kTimeScaleMax);
                    return true;
                default:
                    break;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void onSizeChange() override {
        if (fDom) {
            fDom->setContainerSize(SkSize::Make(this->width(), this->height()));
        }

        this->INHERITED::onSizeChange();
    }

    void onDrawContent(SkCanvas* canvas) override {
        fDom->render(canvas);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        SkScalar dt = (timer.msec() - fLastTick) * fTimeScale;
        fLastTick = timer.msec();

        fPaddle0.posTick(dt);
        fPaddle1.posTick(dt);
        fBall.posTick(dt);

        this->enforceConstraints();

        fPaddle0.updateDom();
        fPaddle1.updateDom();
        fBall.updateDom();

        return true;
    }

private:
    struct Object {
        void initialize(const SkRRect& rrect, SkColor color,
                        const SkPoint& p, const SkVector& s) {
            objectNode = make_svg_rrect(rrect);
            objectNode->setFill(SkSVGPaint(SkSVGColorType(color)));

            shadowNode = make_svg_rrect(rrect);
            shadowNode->setFillOpacity(SkSVGNumberType(kShadowOpacity));

            pos = p;
            spd = s;
            size = SkSize::Make(rrect.width(), rrect.height());
        }

        void posTick(SkScalar dt) {
            pos += spd * dt;
        }

        void updateDom() {
            const SkPoint corner = pos - SkPoint::Make(size.width() / 2, size.height() / 2);
            objectNode->setX(SkSVGLength(corner.x()));
            objectNode->setY(SkSVGLength(corner.y()));

            // Simulate parallax shadow for a centered light source.
            SkPoint shadowOffset = pos - SkPoint::Make(kBounds.centerX(), kBounds.centerY());
            shadowOffset.scale(kShadowParallax);
            const SkPoint shadowCorner = corner + shadowOffset;

            shadowNode->setX(SkSVGLength(shadowCorner.x()));
            shadowNode->setY(SkSVGLength(shadowCorner.y()));
        }

        sk_sp<SkSVGRect> objectNode;
        sk_sp<SkSVGRect> shadowNode;
        SkPoint          pos;
        SkVector         spd;
        SkSize           size;
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

    sk_sp<SkSVGDOM> fDom;
    Object          fPaddle0, fPaddle1, fBall;
    SkRandom        fRand;

    SkMSec          fLastTick  = 0;
    SkScalar        fTimeScale = 1.0f;

    typedef SampleView INHERITED;
};

static SkView* SVGPongFactory() { return new SVGPongView; }
static SkViewRegister reg(SVGPongFactory);
