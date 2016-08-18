/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkLightingShader.h"
#include "SkNormalSource.h"
#include "sk_tool_utils.h"


class BevelView : public SampleView {
public:
    BevelView()
        : fShapeBounds(SkRect::MakeWH(kShapeBoundsSize, kShapeBoundsSize))
        , fRedLight(SkLights::Light::MakeDirectional(SkColor3f::Make(0.6f, 0.45f, 0.3f),
                                                     SkVector3::Make(0.0f, -5.0f, 1.0f)))
        , fBlueLight(SkLights::Light::MakeDirectional(SkColor3f::Make(0.3f, 0.45f, 0.6f),
                                                      SkVector3::Make(0.0f, 5.0f, 1.0f))) {
        this->setBGColor(0xFF666868); // Slightly colorized gray for contrast

        // Lights
        SkLights::Builder builder;
        builder.add(fRedLight);
        builder.add(fBlueLight);
        builder.add(SkLights::Light::MakeAmbient(SkColor3f::Make(0.4f, 0.4f, 0.4f)));
        fLights = builder.finish();

        // Controls

        SkScalar currY = kSliderHeight;

        const SkScalar kWidthCtrlInitialPos = 0.2f;
        fCtrlRangeRects[0] = SkRect::MakeXYWH(0.0f, currY,
                                              kCtrlRange + kSliderWidth,
                                              kSliderHeight);
        fWidthCtrlRect = SkRect::MakeXYWH(kWidthCtrlInitialPos * kCtrlRange, currY,
                                          kSliderWidth, kSliderHeight);
        fBevelWidth = kBevelWidthMax * kWidthCtrlInitialPos;
        currY += 2 * kSliderHeight;

        const SkScalar kHeightCtrlInitialPos = 0.75f;
        fCtrlRangeRects[1] = SkRect::MakeXYWH(0.0f, currY,
                                              kCtrlRange + kSliderWidth,
                                              kSliderHeight);
        fHeightCtrlRect = SkRect::MakeXYWH(kHeightCtrlInitialPos * kCtrlRange, currY,
                                           kSliderWidth, kSliderHeight);
        // Mapping from (0, 1) to (-1, 1)
        fBevelHeight = kBevelHeightMax * (kHeightCtrlInitialPos * 2.0f - 1.0f);
        currY += 2 * kSliderHeight;

        const SkScalar kTypeCtrlInitialPos = 1.0f / (2.0f * kBevelTypeCount);
        fCtrlRangeRects[2] = SkRect::MakeXYWH(0.0f, currY,
                                              kCtrlRange + kSliderWidth,
                                              kSliderHeight);
        fTypeCtrlRect = SkRect::MakeXYWH(kTypeCtrlInitialPos * kCtrlRange, currY,
                                         kSliderWidth, kSliderHeight);
        fBevelType = (SkNormalSource::BevelType) SkScalarFloorToInt(kTypeCtrlInitialPos);
        currY += 2 * kSliderHeight;

        fSelectedCtrlRect = nullptr;
        fDirtyNormalSource = true;

        fLabelTypeface = sk_tool_utils::create_portable_typeface("sans-serif", SkFontStyle());
    }

protected:
    bool onQuery(SkEvent *evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Bevel");
            return true;
        }

        return this->INHERITED::onQuery(evt);
    }

    enum Shape {
        kCircle_Shape,
        kRect_Shape,
    };
    void drawShape(enum Shape shape, SkCanvas* canvas) {
        canvas->save();

        SkPaint paint;

        if (fDirtyNormalSource) {
            fNormalSource = SkNormalSource::MakeBevel(fBevelType, fBevelWidth, fBevelHeight);
            fDirtyNormalSource = false;
        }

        paint.setShader(SkLightingShader::Make(nullptr, fNormalSource, fLights));
        paint.setAntiAlias(true);
        paint.setColor(0xFFDDDDDD);
        switch (shape) {
            case kCircle_Shape:
                canvas->drawCircle(fShapeBounds.centerX(), fShapeBounds.centerY(),
                                   fShapeBounds.width()/2.0f, paint);
                break;
            case kRect_Shape:
                canvas->drawRect(fShapeBounds, paint);
                break;
            default:
                SkDEBUGFAIL("Invalid shape enum for drawShape");
        }

        canvas->restore();
    }

    void onDrawContent(SkCanvas *canvas) override {

        canvas->save();
        canvas->resetMatrix(); // Force static controls and labels

        // Draw controls

        SkPaint ctrlRectPaint;
        ctrlRectPaint.setColor(0xFFF3F3F3);
        canvas->drawRect(fWidthCtrlRect, ctrlRectPaint);
        canvas->drawRect(fHeightCtrlRect, ctrlRectPaint);
        canvas->drawRect(fTypeCtrlRect, ctrlRectPaint);

        SkPaint ctrlRectRangePaint;
        ctrlRectRangePaint.setColor(0xFFFFFFFF);
        ctrlRectRangePaint.setStyle(SkPaint::kStroke_Style);
        ctrlRectRangePaint.setStrokeWidth(2.0f);

        for (size_t i = 0; i < kNumControls; i++) {
            canvas->drawRect(fCtrlRangeRects[i], ctrlRectRangePaint);
        }

        // Draw labels
        constexpr SkScalar kTextSize = 12.0f;
        SkString widthLabel, heightLabel, typeLabel;
        SkPaint labelPaint;
        labelPaint.setTypeface(fLabelTypeface);
        labelPaint.setAntiAlias(true);
        labelPaint.setColor(0xFFFFFFFF);
        labelPaint.setTextSize(kTextSize);

        widthLabel.appendf("BevelWidth: %f", fBevelWidth);
        heightLabel.appendf("BevelHeight: %f", fBevelHeight);
        typeLabel.append("BevelType: ");

        switch (fBevelType) {
            case SkNormalSource::BevelType::kLinear:
                typeLabel.append("Linear");
                break;
            case SkNormalSource::BevelType::kRoundedIn:
                typeLabel.append("RoundedIn");
                break;
            case SkNormalSource::BevelType::kRoundedOut:
                typeLabel.append("RoundedOut");
                break;
        }

        canvas->drawText(widthLabel.c_str(), widthLabel.size(), 0,
                         fWidthCtrlRect.fTop - kTextSize/2.0f, labelPaint);
        canvas->drawText(heightLabel.c_str(), heightLabel.size(), 0,
                         fHeightCtrlRect.fTop - kTextSize/2.0f, labelPaint);
        canvas->drawText(typeLabel.c_str(), typeLabel.size(), 0,
                         fTypeCtrlRect.fTop - kTextSize/2.0f, labelPaint);

        canvas->restore(); // Return to modified matrix when drawing shapes

        // Draw shapes
        SkScalar xPos = kCtrlRange + 25.0f;
        SkScalar yPos = fShapeBounds.height();
        for (Shape shape : { kCircle_Shape, kRect_Shape }) {
            canvas->save();
            canvas->translate(xPos, yPos);
            this->drawShape(shape, canvas);
            canvas->restore();

            xPos += 1.2f * fShapeBounds.width();
        }
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        return new SkView::Click(this);
    }

    bool onClick(Click *click) override {
        SkScalar x = click->fCurr.fX;
        SkScalar y = click->fCurr.fY;

        SkScalar dx = x - click->fPrev.fX;
        SkScalar dy = y - click->fPrev.fY;

        // Control deselection
        if (Click::State::kUp_State == click->fState) {
            fSelectedCtrlRect = nullptr;
            return true;
        }

        // Control selection
        if (nullptr == fSelectedCtrlRect && Click::State::kDown_State == click->fState) {
            if (fWidthCtrlRect.contains(SkRect::MakeXYWH(x, y, 1, 1))) {
                fSelectedCtrlRect = &fWidthCtrlRect;
            } else if (fHeightCtrlRect.contains(SkRect::MakeXYWH(x, y, 1, 1))) {
                fSelectedCtrlRect = &fHeightCtrlRect;
            } else if (fTypeCtrlRect.contains(SkRect::MakeXYWH(x, y, 1, 1))) {
                fSelectedCtrlRect = &fTypeCtrlRect;
            }
        }

        if (nullptr != fSelectedCtrlRect) { // Control modification
            fSelectedCtrlRect->offsetTo(SkScalarPin(x, 0.0f, kCtrlRange), fSelectedCtrlRect->fTop);

            fBevelHeight = (fHeightCtrlRect.fLeft / kCtrlRange) * kBevelHeightMax * 2.0f
                           - kBevelHeightMax;
            fBevelWidth = (fWidthCtrlRect.fLeft / kCtrlRange) * kBevelWidthMax;
            fBevelType = (SkNormalSource::BevelType)SkTMin(
                    SkScalarFloorToInt(kBevelTypeCount * fTypeCtrlRect.fLeft / kCtrlRange),
                    kBevelTypeCount - 1);

            // Snap type controls to 3 positions
            fTypeCtrlRect.offsetTo(kCtrlRange * ( ((int)fBevelType)/SkIntToScalar(kBevelTypeCount)
                                                  + 1.0f/(2.0f * kBevelTypeCount) ),
                                   fTypeCtrlRect.fTop);

            // Ensuring width is non-zero
            fBevelWidth = SkMaxScalar(1.0f, fBevelWidth);

            fDirtyNormalSource = true;

            this->inval(nullptr);
            return true;
        } else { // Moving light
            if (dx != 0 || dy != 0) {
                float recipX = 1.0f / kAppWidth;
                float recipY = 1.0f / kAppHeight;

                if (0 == click->fModifierKeys) { // No modifier
                    fBlueLight = SkLights::Light::MakeDirectional(fBlueLight.color(),
                            SkVector3::Make((kAppWidth/2.0f - x) * recipX * -3.0f,
                                            (kAppHeight/2.0f - y) * recipY * -3.0f,
                                            1.0f));
                } else if (1 == click->fModifierKeys) { // Shift key
                    fRedLight = SkLights::Light::MakeDirectional(fRedLight.color(),
                            SkVector3::Make((kAppWidth/2.0f - x) * recipX * -3.0f,
                                            (kAppHeight/2.0f - y) * recipY * -3.0f,
                                            1.0f));
                }

                SkLights::Builder builder;
                builder.add(fRedLight);
                builder.add(fBlueLight);
                builder.add(SkLights::Light::MakeAmbient(
                        SkColor3f::Make(0.4f, 0.4f, 0.4f)));
                fLights = builder.finish();

                this->inval(nullptr);
            }
            return true;
        }

        return true;
    }

private:
    static constexpr int kNumTestRects = 3;

    static constexpr SkScalar kAppWidth = 400.0f;
    static constexpr SkScalar kAppHeight = 400.0f;
    static constexpr SkScalar kShapeBoundsSize = 120.0f;

    static constexpr SkScalar kCtrlRange = 150.0f;
    static constexpr SkScalar kBevelWidthMax = kShapeBoundsSize;
    static constexpr SkScalar kBevelHeightMax = 50.0f;
    static constexpr int      kBevelTypeCount = 3;

    static constexpr SkScalar kSliderHeight = 20.0f;
    static constexpr SkScalar kSliderWidth = 10.0f;

    const SkRect fShapeBounds;

    static constexpr int kNumControls = 3;
    SkRect fCtrlRangeRects[kNumControls];
    SkRect* fSelectedCtrlRect;
    SkRect fWidthCtrlRect;
    SkRect fHeightCtrlRect;
    SkRect fTypeCtrlRect;

    SkScalar fBevelWidth;
    SkScalar fBevelHeight;
    SkNormalSource::BevelType fBevelType;
    sk_sp<SkNormalSource> fNormalSource;
    bool fDirtyNormalSource;

    sk_sp<SkLights> fLights;
    SkLights::Light fRedLight;
    SkLights::Light fBlueLight;

    sk_sp<SkTypeface> fLabelTypeface;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new BevelView; }
static SkViewRegister reg(MyFactory);

