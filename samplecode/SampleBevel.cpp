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

class ParentControl;

// Abstract base class for all components that a control panel must have
class Control : public SkRefCnt {
public:
    Control(SkString name)
        : fName(name)
        , fParent(nullptr)
        , fRelativePos(SkPoint::Make(0.0f, 0.0f)) {}

    // Use this to propagate a click's position down to a control. Gets modulated by the component's
    // relative position
    bool click(const SkPoint& clickPos) {
        SkPoint relativeClickPos = SkPoint::Make(clickPos.fX - fRelativePos.fX,
                                                 clickPos.fY - fRelativePos.fY);
        return this->onClick(relativeClickPos);
    }

    // Use this to draw the control and its appropriate children. Gets modulated by the component's
    // relative position.
    void drawContent(SkCanvas *canvas) {
        canvas->save();
        canvas->translate(fRelativePos.fX, fRelativePos.fY);
        this->onDrawContent(canvas);
        canvas->restore();
    }

    /* Returns true when click position argumend lands over a control region in this control. Click
     * position gets modulated by the component's relative position.
     *
     * @param click The position of the click in the coordinate space relative to the parent
     */
    bool isInCtrlRegion(const SkPoint& click) {
        SkPoint relativeClickPos = SkPoint::Make(click.fX - fRelativePos.fX,
                                                 click.fY - fRelativePos.fY);
        return this->onIsInCtrlRegion(relativeClickPos);
    }

    // Returns height of content drawn
    virtual SkScalar height() const = 0;

    // Sets the parent of this component. May only be used once. Height must remain constant after
    // parent is set.
    void setParent(ParentControl *parent, const SkPoint& relativePos) {
        SkASSERT(parent);
        SkASSERT(!fParent); // No chidren transfer since relativeY would get invalid for younger kid

        fParent = parent;
        fRelativePos = relativePos;
        this->onSetParent();
    }

    // Overriden by sub-classes that need to recompute fields after parent is set. Called after
    // setting fParent.
    virtual void onSetParent() {}

    // Overriden by sub-classes that need to know when a click is released.
    virtual void onClickRelease() {}

protected:

    // Draws a label for the component, using its name and a passed value. Does NOT modulate by
    // relative height, expects CTM to have been adjusted in advance.
    void drawLabel(SkCanvas *canvas, const SkString& valueStr) const {
        // TODO Cache this
        sk_sp<SkTypeface> fLabelTypeface =
                sk_tool_utils::create_portable_typeface("sans-serif", SkFontStyle());

        SkString label;
        label.append(fName);
        label.append(": ");
        label.append(valueStr);

        SkPaint labelPaint;
        labelPaint.setTypeface(fLabelTypeface);
        labelPaint.setAntiAlias(true);
        labelPaint.setColor(0xFFFFFFFF);
        labelPaint.setTextSize(12.0f);

        canvas->drawString(label, 0, kLabelHeight - 6.0f, labelPaint);
    }

    SkString fName;
    ParentControl* fParent;

    static constexpr SkScalar kLabelHeight = 20.0f;

private:
    // Overriden by sub-class to draw component. Do not call directly, drawContent() modulates by
    // relative position.
    virtual void onDrawContent(SkCanvas *canvas) = 0;

    // Overriden by sub-class to handle clicks. Do not call directly, click() modulates by relative
    // position. Return true if holding mouse capture
    virtual bool onClick(const SkPoint& clickPos) { return false; }

    // Overriden by sub-classes with controls. Should return true if clickPos lands inside a control
    // region, to enable mouse caputre.
    virtual bool onIsInCtrlRegion(const SkPoint& clickPos) const { return false; }

    // The position of the control relative to it's parent
    SkPoint fRelativePos;
};

class ParentControl : public Control { // Interface for all controls that have children
public:
    ParentControl(const SkString& name) : INHERITED(name) {}

    // Adds a child
    virtual void add(sk_sp<Control> control) = 0;

    // Returns the control's width. Used to propagate width down to components that don't specify it
    virtual SkScalar width() const = 0;

private:
    typedef Control INHERITED;
};

class ControlPanel : public ParentControl {
public:

    ControlPanel(SkScalar width)
        : ParentControl(SkString("ControlPanel"))
        , fWidth(width)
        , fHeight(0.0f)
        , fSelectedControl(-1) {}

    // Width unspecified, expectation is inheritance from parent
    ControlPanel() : ControlPanel(-1.0f) {}

    // Use this for introducing clicks on a ControlPanel from outside of the framework. It
    // propagates click release or position down the chain. Returns false when click capture is
    // being released.
    bool inClick(SkView::Click *inClick) {
        if (SkView::Click::State::kUp_State == inClick->fState) {
            this->onClickRelease();
            return false;
        }
        return this->click(inClick->fCurr);
    }

    // Add children
    void add(sk_sp<Control> control) override {
        SkASSERT(!fParent); // Validity of parent's relativeY and fHeight depends on immutability
        fControls.push_back(control);
        control->setParent(this, SkPoint::Make(0.0f, fHeight));
        fHeight += control->height();
    }

    SkScalar width() const override {
        return fParent ? fParent->width() : fWidth; // Width inherited from parent if there is one
    }

    SkScalar height() const override {
        return fHeight;
    }

    // Propagate click release to selected control, deselect control
    void onClickRelease() override {
        if (fSelectedControl >= 0) {
            fControls[fSelectedControl]->onClickRelease();
        }
        fSelectedControl = -1;
    }

    // Propagate onSetParent() down to children, some might need fParent->width() refresh
    void onSetParent() override {
        for (int i = 0; i < fControls.count(); i++) {
            fControls[i]->onSetParent();
        }
    }

    // Holds a vertical shelf of controls. Can't be hierarchy root if not given a width value.
    static sk_sp<ParentControl> Make() {
        return sk_sp<ParentControl>(new ControlPanel());
    }

    // Holds a vertical shelf of controls. Only control that can be hooked from outside the
    // framework.
    static sk_sp<ParentControl> Make(SkScalar width) {
        return sk_sp<ParentControl>(new ControlPanel(width));
    }

protected:
    // Returns true if control panel has mouse captured, false when it is ready to release
    // capture
    bool onClick(const SkPoint& click) override {

        if (fSelectedControl == -1) { // If no child control selected, check every child
            for (int i = 0; i < fControls.count(); i++) {
                if (fControls[i]->isInCtrlRegion(click)) {
                    fSelectedControl = i;
                    break;
                }
            }
        }

        if (fSelectedControl >= 0) { // If child control selected, propagate click
            bool keepSelection = fControls[fSelectedControl]->click(click);
            if (!keepSelection) {
                fSelectedControl = -1;
            }
            return keepSelection;
        }

        return false;
    }

    // Draw all children
    void onDrawContent(SkCanvas* canvas) override {
        canvas->save();
        for (int i = 0; i < fControls.count(); i++) {
            fControls[i]->drawContent(canvas);
        }
        canvas->restore();
    }

    // Check all children's control regions
    bool onIsInCtrlRegion(const SkPoint& clickPos) const override {
        for (int i = 0; i < fControls.count(); i++) {
            if (fControls[i]->isInCtrlRegion(clickPos)) {
                return true;
            }
        }

        return false;
    }

private:
    SkScalar fWidth;
    SkScalar fHeight;

    SkTArray<sk_sp<Control>> fControls;
    int fSelectedControl;
};

class DiscreteSliderControl : public Control {
public:
    SkScalar height() const override {
        return 2.0f * kLabelHeight;
    }

    // Set width-dependant variables when new parent is set
    void onSetParent() override {
        fCtrlRegion = SkRect::MakeXYWH(0.0f, kLabelHeight, fParent->width(), kSliderHeight);
        fSliderRange = fParent->width() - kSliderWidth;
    }

    /* Make a slider for an integer value. Snaps to discrete positions.
     *
     * @params name    The name of the control, displayed in the label
     * @params output  Pointer to the integer that will be set by the slider
     * @params min     Min value for output.
     * @params max     Max value for output.
     */
    static sk_sp<Control> Make(SkString name, int* output, int min, int max) {
        return sk_sp<Control>(new DiscreteSliderControl(name, output, min, max));
    }

protected:
    void onDrawContent(SkCanvas* canvas) override {
        SkASSERT(fParent);
        int numChoices = fMax - fMin + 1;
        fSlider.offsetTo(fSliderRange * ( (*fOutput)/SkIntToScalar(numChoices)
                                          + 1.0f/(2.0f * numChoices) ),
                         fSlider.fTop);

        SkString valueStr;
        valueStr.appendS32(*fOutput);
        this->drawLabel(canvas, valueStr);

        SkPaint sliderPaint;
        sliderPaint.setColor(0xFFF3F3F3);
        canvas->drawRect(fSlider, sliderPaint);

        SkPaint ctrlRegionPaint;
        ctrlRegionPaint.setColor(0xFFFFFFFF);
        ctrlRegionPaint.setStyle(SkPaint::kStroke_Style);
        ctrlRegionPaint.setStrokeWidth(2.0f);
        canvas->drawRect(fCtrlRegion, ctrlRegionPaint);
    }

    bool onClick(const SkPoint& clickPos) override {
        SkASSERT(fParent);
        SkScalar x = SkScalarPin(clickPos.fX, 0.0f, fSliderRange);
        int numChoices = fMax - fMin + 1;
        *fOutput = SkTMin(SkScalarFloorToInt(numChoices * x / fSliderRange) + fMin, fMax);

        return true;
    }

    bool onIsInCtrlRegion(const SkPoint& clickPos) const override {
        SkASSERT(fParent);
        return fCtrlRegion.contains(SkRect::MakeXYWH(clickPos.fX, clickPos.fY, 1, 1));
    }

private:
    DiscreteSliderControl(SkString name, int* output, int min, int max)
            : INHERITED(name)
            , fOutput(output)
            , fMin(min)
            , fMax(max) {
        fSlider = SkRect::MakeXYWH(0, kLabelHeight, kSliderWidth, kSliderHeight);
    }

    int* fOutput;
    int fMin;
    int fMax;
    SkRect fSlider; // The rectangle that slides
    // The region in which the rectangle slides. Also the region in which mouse is caputred
    SkRect fCtrlRegion;
    SkScalar fSliderRange; // The width in pixels over which the slider can slide

    static constexpr SkScalar kSliderHeight = 20.0f;
    static constexpr SkScalar kSliderWidth = 10.0f;

    typedef Control INHERITED;
};

class ControlSwitcher : public ParentControl {
public:
    // Add children
    void add(sk_sp<Control> control) override {
        SkASSERT(!fParent); // Validity of parent's relativeY and fHeight depends on immutability
        fControls.push_back(control);
        control->setParent(this, SkPoint::Make(0.0f, kSelectorHeight));
        fHeight = SkMaxScalar(fHeight, control->height()); // Setting height to max child height.
    }

    SkScalar width() const override { return fParent ? (fParent->width()) : 0; }

    SkScalar height() const override {
        return fHeight;
    }

    // Propagate onClickRelease to control that currently captures mouse
    void onClickRelease() override {
        if (fCtrlOnClick) {
            fCtrlOnClick->onClickRelease();
        }
        fCtrlOnClick = nullptr;
    }

    void onSetParent() override {
        for (int i = 0; i < fControls.count(); i++) {
            fControls[i]->onSetParent(); // Propagate to children
        }

        // Finalize control selector
        // TODO can be moved to constructor if list-initialized
        if (!finalizedChildren) {
            fControlSelector = DiscreteSliderControl::Make(
                    SkString(fName), &fSelectedControl, 0, fControls.count()-1);
            fControlSelector->setParent(this, SkPoint::Make(0.0f, 0.0f));
            fHeight += kSelectorHeight;

            SkASSERT(fControlSelector->height() <= kSelectorHeight);
        }
    }

    /* A set of a selector and a list of controls. Displays the control from the list of controls
     * with the index set by the aforementioned selector.
     *
     * @param name The name of the switcher. Will be displayed in the selector's label.
     */
    static sk_sp<ParentControl> Make(const SkString& name) {
        return sk_sp<ParentControl>(new ControlSwitcher(name));
    }

protected:
    // Draw selector and currently selected control
    void onDrawContent(SkCanvas* canvas) override {
        fControlSelector->drawContent(canvas);
        fControls[fSelectedControl]->drawContent(canvas);
    }

    // Returns true if control panel has mouse captured, false when it is ready to release
    // capture
    bool onClick(const SkPoint& click) override {
        if (!fCtrlOnClick) {
            if (fControlSelector->isInCtrlRegion(click)) {
                fCtrlOnClick = fControlSelector.get();
            } else if (fControls[fSelectedControl]->isInCtrlRegion(click)) {
                fCtrlOnClick = fControls[fSelectedControl].get();
            }
        }
        if (fCtrlOnClick) {
            return fCtrlOnClick->click(click);
        }

        return false;
    }

    // Is in control region of selector or currently selected control
    bool onIsInCtrlRegion(const SkPoint& clickPos) const override {
        if (fControlSelector->isInCtrlRegion(clickPos)) {
            return true;
        }
        if (fControls[fSelectedControl]->isInCtrlRegion(clickPos)) {
            return true;
        }

        return false;
    }

private:
    ControlSwitcher(const SkString& name)
        : INHERITED(name)
        , fHeight(0.0)
        , fSelectedControl(0)
        , fCtrlOnClick(nullptr){}

    bool finalizedChildren = false;

    sk_sp<Control> fControlSelector;
    SkScalar fHeight;
    SkTArray<sk_sp<Control>> fControls;
    int fSelectedControl;

    Control* fCtrlOnClick;

    static constexpr SkScalar kSelectorHeight = 40.0f;

    typedef ParentControl INHERITED;
};

class ContinuousSliderControl : public Control {
public:
    SkScalar height() const override {
        return 2.0f * kLabelHeight;
    }

    void onSetParent() override {
        fSlider = SkRect::MakeXYWH(0, kLabelHeight, kSliderWidth, kSliderHeight);
        fCtrlRegion = SkRect::MakeXYWH(0.0f, kLabelHeight, fParent->width(), kSliderHeight);
        fSliderRange = fParent->width() - kSliderWidth;
    }

    /* Make a slider for an SkScalar.
     *
     * @params name    The name of the control, displayed in the label
     * @params output  Pointer to the SkScalar that will be set by the slider
     * @params min     Min value for output
     * @params max     Max value for output
     */
    static sk_sp<Control> Make(const SkString& name, SkScalar* output, SkScalar min, SkScalar max) {
       return sk_sp<Control>(new ContinuousSliderControl(name, output, min, max));
    }

protected:
    void onDrawContent(SkCanvas* canvas) override {
        SkASSERT(fParent);
        SkScalar x = fSliderRange * (*fOutput - fMin) / (fMax - fMin);
        fSlider.offsetTo(SkScalarPin(x, 0.0f, fSliderRange), fSlider.fTop);

        SkString valueStr;
        valueStr.appendScalar(*fOutput);
        this->drawLabel(canvas, valueStr);

        SkPaint sliderPaint;
        sliderPaint.setColor(0xFFF3F3F3);
        canvas->drawRect(fSlider, sliderPaint);

        SkPaint ctrlRegionPaint;
        ctrlRegionPaint.setColor(0xFFFFFFFF);
        ctrlRegionPaint.setStyle(SkPaint::kStroke_Style);
        ctrlRegionPaint.setStrokeWidth(2.0f);
        canvas->drawRect(fCtrlRegion, ctrlRegionPaint);
    }

    bool onClick(const SkPoint& clickPos) override {
        SkASSERT(fParent);
        SkScalar x = SkScalarPin(clickPos.fX, 0.0f, fSliderRange);
        *fOutput = (x/fSliderRange) * (fMax - fMin) + fMin;
        return true;
    }

    bool onIsInCtrlRegion(const SkPoint& clickPos) const override {
        SkASSERT(fParent);
        return fCtrlRegion.contains(SkRect::MakeXYWH(clickPos.fX, clickPos.fY, 1, 1));
    }

private:
    ContinuousSliderControl(const SkString& name, SkScalar* output, SkScalar min, SkScalar max)
            : INHERITED(name)
            , fOutput(output)
            , fMin(min)
            , fMax(max) {}

    SkScalar* fOutput;
    SkScalar fMin;
    SkScalar fMax;
    SkRect fSlider;
    SkRect fCtrlRegion;
    SkScalar fSliderRange;

    static constexpr SkScalar kSliderHeight = 20.0f;
    static constexpr SkScalar kSliderWidth = 10.0f;

    typedef Control INHERITED;
};

class RadialDirectionControl : public Control {
public:
    SkScalar height() const override {
        return kLabelHeight + 2.0f * kRegionRadius;
    }

    /* Make a direction selector.
     *
     * @params name    The name of the control, displayed in the label
     * @params output  Pointer to the SkVector that will be set by the slider
     */
    static sk_sp<Control> Make(const SkString& name, SkVector* output) {
        return sk_sp<Control>(new RadialDirectionControl(name, output));
    }

protected:
    void onDrawContent(SkCanvas* canvas) override {
        SkASSERT(fParent);

        SkString valueStr;
        valueStr.appendf("%.2f, %.2f", fOutput->fX, fOutput->fY);
        this->drawLabel(canvas, valueStr);

        SkPoint lineEnd = SkPoint::Make(fCtrlRegion.centerX(), fCtrlRegion.centerY())
                          + (*fOutput * (kRegionRadius - kCapRadius));
        SkPaint linePaint;
        linePaint.setColor(0xFFF3F3F3);
        linePaint.setStrokeWidth(kStrokeWidth);
        linePaint.setAntiAlias(true);
        linePaint.setStrokeCap(SkPaint::kRound_Cap);
        canvas->drawLine(fCtrlRegion.centerX(), fCtrlRegion.centerY(),
                         lineEnd.fX, lineEnd.fY, linePaint);

        SkPaint ctrlRegionPaint;
        ctrlRegionPaint.setColor(0xFFFFFFFF);
        ctrlRegionPaint.setStyle(SkPaint::kStroke_Style);
        ctrlRegionPaint.setStrokeWidth(2.0f);
        ctrlRegionPaint.setAntiAlias(true);
        canvas->drawCircle(fCtrlRegion.centerX(), fCtrlRegion.centerY(), kRegionRadius,
                           ctrlRegionPaint);
    }

    bool onClick(const SkPoint& clickPos) override {
        SkASSERT(fParent);
        fOutput->fX = clickPos.fX - fCtrlRegion.centerX();
        fOutput->fY = clickPos.fY - fCtrlRegion.centerY();
        fOutput->normalize();

        return true;
    }

    bool onIsInCtrlRegion(const SkPoint& clickPos) const override {
        SkASSERT(fParent);
        return fCtrlRegion.contains(SkRect::MakeXYWH(clickPos.fX, clickPos.fY,
                                                     1, 1));
    }

private:
    RadialDirectionControl(const SkString& name, SkVector* output)
            : INHERITED(name)
            , fOutput(output) {
        fCtrlRegion = SkRect::MakeXYWH(0.0f, kLabelHeight,
                                       kRegionRadius * 2.0f, kRegionRadius * 2.0f);
    }

    SkVector* fOutput;
    SkRect fCtrlRegion;

    static constexpr SkScalar kRegionRadius = 50.0f;
    static constexpr SkScalar kStrokeWidth = 6.0f;
    static constexpr SkScalar kCapRadius = kStrokeWidth / 2.0f;

    typedef Control INHERITED;
};

class ColorDisplay: public Control {
public:
    SkScalar height() const override {
        return kHeight;
    }

    void onSetParent() override {
        fDisplayRect = SkRect::MakeXYWH(0.0f, kPadding, fParent->width(), kHeight - kPadding);
    }

    /* Make a display that shows an SkColor3f.
     *
     * @params output  Pointer to the SkColor3f that will be displayed
     */
    static sk_sp<Control> Make(SkColor3f* input) {
        return sk_sp<Control>(new ColorDisplay(SkString("ColorDisplay"), input));
    }

protected:
    void onDrawContent(SkCanvas* canvas) override {
        SkASSERT(fParent);

        SkPaint displayPaint;
        displayPaint.setColor(SkColor4f::FromColor3f(*fInput, 1.0f).toSkColor());
        canvas->drawRect(fDisplayRect, displayPaint);
    }

private:
    ColorDisplay(const SkString& name, SkColor3f* input)
            : INHERITED(name)
            , fInput(input) {}

    SkColor3f* fInput;
    SkRect fDisplayRect;

    static constexpr SkScalar kHeight = 24.0f;
    static constexpr SkScalar kPadding = 4.0f;

    typedef Control INHERITED;
};

class BevelView : public SampleView {
public:
    BevelView()
        : fShapeBounds(SkRect::MakeWH(kShapeBoundsSize, kShapeBoundsSize))
        , fControlPanel(kCtrlRange) {
        this->setBGColor(0xFF666868); // Slightly colorized gray for contrast

        // Controls
        fBevelWidth = 25.0f;
        fBevelHeight = 25.0f;
        fBevelType = 0;

        int currLight = 0;
        fLightDefs[currLight++] =
                {SkVector::Make(0.0f, 1.0f), 1.0f, SkColor3f::Make(0.6f, 0.45f, 0.3f)};
        fLightDefs[currLight++] =
                {SkVector::Make(0.0f, -1.0f), 1.0f, SkColor3f::Make(0.3f, 0.45f, 0.6f)};
        fLightDefs[currLight++] =
                {SkVector::Make(1.0f, 0.0f), 1.0f, SkColor3f::Make(0.0f, 0.0f, 0.0f)};
        // Making sure we initialized all lights
        SkASSERT(currLight == kNumLights);

        fControlPanel.add(ContinuousSliderControl::Make(SkString("BevelWidth"), &fBevelWidth,
                                                        1.0f, kShapeBoundsSize));
        fControlPanel.add(ContinuousSliderControl::Make(SkString("BevelHeight"), &fBevelHeight,
                                                        -50.0f, 50.0f));
        fControlPanel.add(DiscreteSliderControl::Make(SkString("BevelType"), &fBevelType,
                                                      0, 2));
        sk_sp<ParentControl> lightCtrlSelector = ControlSwitcher::Make(SkString("SelectedLight"));
        for (int i = 0; i < kNumLights; i++) {
            SkString name("Light");
            name.appendS32(i);
            sk_sp<ParentControl> currLightPanel = ControlPanel::Make();
            SkString dirName(name);
            dirName.append("Dir");
            currLightPanel->add(RadialDirectionControl::Make(dirName, &(fLightDefs[i].fDirXY)));
            SkString heightName(name);
            heightName.append("Height");
            currLightPanel->add(ContinuousSliderControl::Make(heightName, &(fLightDefs[i].fDirZ),
                                                             0.0f, 2.0f));
            SkString redName(name);
            redName.append("Red");
            currLightPanel->add(ContinuousSliderControl::Make(redName, &(fLightDefs[i].fColor.fX),
                                                              0.0f, 1.0f));
            SkString greenName(name);
            greenName.append("Green");
            currLightPanel->add(ContinuousSliderControl::Make(greenName, &(fLightDefs[i].fColor.fY),
                                                              0.0f, 1.0f));
            SkString blueName(name);
            blueName.append("Blue");
            currLightPanel->add(ContinuousSliderControl::Make(blueName, &(fLightDefs[i].fColor.fZ),
                                                              0.0f, 1.0f));
            currLightPanel->add(ColorDisplay::Make(&(fLightDefs[i].fColor)));
            lightCtrlSelector->add(currLightPanel);
        }
        fControlPanel.add(lightCtrlSelector);

        fControlPanelSelected = false;
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
            fNormalSource = SkNormalSource::MakeBevel((SkNormalSource::BevelType)fBevelType,
                                                      fBevelWidth, fBevelHeight);
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
        canvas->resetMatrix(); // Force static control panel position
        fControlPanel.drawContent(canvas);
        canvas->restore();

        SkLights::Builder builder;
        for (int i = 0; i < kNumLights; i++) {
            builder.add(SkLights::Light::MakeDirectional(fLightDefs[i].fColor,
                                                         SkPoint3::Make(fLightDefs[i].fDirXY.fX,
                                                                        fLightDefs[i].fDirXY.fY,
                                                                        fLightDefs[i].fDirZ)));
        }
        builder.setAmbientLightColor(SkColor3f::Make(0.4f, 0.4f, 0.4f));
        fLights = builder.finish();

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
        // Control panel mouse handling
        fControlPanelSelected = fControlPanel.inClick(click);

        if (fControlPanelSelected) { // Control modification
            fDirtyNormalSource = true;

            this->inval(nullptr);
            return true;
        }

        // TODO move shapes
        this->inval(nullptr);
        return true;
    }

private:
    static constexpr int kNumTestRects = 3;

    static constexpr SkScalar kShapeBoundsSize = 120.0f;

    static constexpr SkScalar kCtrlRange = 150.0f;

    static constexpr int kNumLights = 3;

    const SkRect fShapeBounds;

    SkScalar fBevelWidth;
    SkScalar fBevelHeight;
    int      fBevelType;

    sk_sp<SkNormalSource> fNormalSource;
    bool fDirtyNormalSource;

    sk_sp<SkLights> fLights;

    struct LightDef {
        SkVector fDirXY;
        SkScalar fDirZ;
        SkColor3f fColor;

        LightDef() {}
        LightDef(SkVector dirXY, SkScalar dirZ, SkColor3f color)
            : fDirXY(dirXY)
            , fDirZ(dirZ)
            , fColor(color) {}
    };
    LightDef fLightDefs[kNumLights];

    ControlPanel fControlPanel;
    bool fControlPanelSelected;

    sk_sp<SkTypeface> fLabelTypeface;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new BevelView; }
static SkViewRegister reg(MyFactory);

