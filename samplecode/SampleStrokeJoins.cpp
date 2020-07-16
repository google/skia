/*
 * Copyright 2015,2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkString.h"
#include "include/private/SkMacros.h"
#include "include/utils/SkTextUtils.h"
#include "samplecode/Sample.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPointPriv.h"
#include "src/pathops/SkIntersections.h"
#include "src/pathops/SkOpEdgeBuilder.h"
#include "tools/ToolUtils.h"

extern SkScalar gCurveDistance;

static SkScalar get_path_weight(int index, const SkPath& path) {
    SkPoint pts[4];
    SkPath::Verb verb;
    SkPath::Iter iter(path, true);
    int counter = -1;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (++counter < index) {
            continue;
        }
        return verb == SkPath::kConic_Verb ? iter.conicWeight() : 1;
    }
    SkASSERT(0);
    return 0;
}

static void add_path_segment(int index, SkPath* path) {
    SkPath result;
    SkPoint firstPt = { 0, 0 };  // init to avoid warning
    SkPoint lastPt = { 0, 0 };  // init to avoid warning
    int counter = -1;
    SkPoint chop[7];
    SkConic conicChop[2];
    for (auto [verb, pts, w] : SkPathPriv::Iterate(*path)) {
        if (++counter == index) {
            switch (verb) {
                case SkPathVerb::kLine:
                    result.lineTo((pts[0].fX + pts[1].fX) / 2, (pts[0].fY + pts[1].fY) / 2);
                    break;
                case SkPathVerb::kQuad: {
                    SkChopQuadAtHalf(pts, chop);
                    result.quadTo(chop[1], chop[2]);
                    pts = chop + 2;
                    } break;
                case SkPathVerb::kConic: {
                    SkConic conic;
                    conic.set(pts, *w);
                    if (!conic.chopAt(0.5f, conicChop)) {
                        return;
                    }
                    result.conicTo(conicChop[0].fPts[1], conicChop[0].fPts[2], conicChop[0].fW);
                    pts = conicChop[1].fPts;
                    w = &conicChop[1].fW;
                    } break;
                case SkPathVerb::kCubic: {
                    SkChopCubicAtHalf(pts, chop);
                    result.cubicTo(chop[1], chop[2], chop[3]);
                    pts = chop + 3;
                    } break;
                case SkPathVerb::kClose: {
                    result.lineTo((lastPt.fX + firstPt.fX) / 2, (lastPt.fY + firstPt.fY) / 2);
                    } break;
                default:
                    SkASSERT(0);
            }
        }
        switch (verb) {
            case SkPathVerb::kMove:
                result.moveTo(firstPt = pts[0]);
                break;
            case SkPathVerb::kLine:
                result.lineTo(lastPt = pts[1]);
                break;
            case SkPathVerb::kQuad:
                result.quadTo(pts[1], lastPt = pts[2]);
                break;
            case SkPathVerb::kConic:
                result.conicTo(pts[1], lastPt = pts[2], *w);
                break;
            case SkPathVerb::kCubic:
                result.cubicTo(pts[1], pts[2], lastPt = pts[3]);
                break;
            case SkPathVerb::kClose:
                result.close();
                break;
            default:
                SkASSERT(0);
        }
    }
    *path = result;
}

static void delete_path_segment(int index, SkPath* path) {
    SkPath result;
    int counter = -1;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(*path)) {
        if (++counter == index) {
            continue;
        }
        switch (verb) {
            case SkPathVerb::kMove:
                result.moveTo(pts[0]);
                break;
            case SkPathVerb::kLine:
                result.lineTo(pts[1]);
                break;
            case SkPathVerb::kQuad:
                result.quadTo(pts[1], pts[2]);
                break;
            case SkPathVerb::kConic:
                result.conicTo(pts[1], pts[2], *w);
                break;
            case SkPathVerb::kCubic:
                result.cubicTo(pts[1], pts[2], pts[3]);
                break;
            case SkPathVerb::kClose:
                result.close();
                break;
            default:
                SkASSERT(0);
        }
    }
    *path = result;
}

static void set_path_weight(int index, SkScalar w, SkPath* path) {
    SkPath result;
    SkPoint pts[4];
    SkPath::Verb verb;
    SkPath::Iter iter(*path, true);
    int counter = -1;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        ++counter;
        switch (verb) {
            case SkPath::kMove_Verb:
                result.moveTo(pts[0]);
                break;
            case SkPath::kLine_Verb:
                result.lineTo(pts[1]);
                break;
            case SkPath::kQuad_Verb:
                result.quadTo(pts[1], pts[2]);
                break;
            case SkPath::kConic_Verb:
                result.conicTo(pts[1], pts[2], counter == index ? w : iter.conicWeight());
                break;
            case SkPath::kCubic_Verb:
                result.cubicTo(pts[1], pts[2], pts[3]);
                break;
            case SkPath::kClose_Verb:
                result.close();
                break;
            case SkPath::kDone_Verb:
                break;
            default:
                SkASSERT(0);
        }
    }
    *path = result;
}

static void set_path_verb(int index, SkPath::Verb v, SkPath* path, SkScalar w) {
    SkASSERT(SkPath::kLine_Verb <= v && v <= SkPath::kCubic_Verb);
    SkPath result;
    SkPoint pts[4];
    SkPath::Verb verb;
    SkPath::Iter iter(*path, true);
    int counter = -1;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        SkScalar weight = verb == SkPath::kConic_Verb ? iter.conicWeight() : 1;
        if (++counter == index && v != verb) {
            SkASSERT(SkPath::kLine_Verb <= verb && verb <= SkPath::kCubic_Verb);
            switch (verb) {
                case SkPath::kLine_Verb:
                    switch (v) {
                        case SkPath::kConic_Verb:
                            weight = w;
                            [[fallthrough]];
                        case SkPath::kQuad_Verb:
                            pts[2] = pts[1];
                            pts[1].fX = (pts[0].fX + pts[2].fX) / 2;
                            pts[1].fY = (pts[0].fY + pts[2].fY) / 2;
                            break;
                        case SkPath::kCubic_Verb:
                            pts[3] = pts[1];
                            pts[1].fX = (pts[0].fX * 2 + pts[3].fX) / 3;
                            pts[1].fY = (pts[0].fY * 2 + pts[3].fY) / 3;
                            pts[2].fX = (pts[0].fX + pts[3].fX * 2) / 3;
                            pts[2].fY = (pts[0].fY + pts[3].fY * 2) / 3;
                            break;
                         default:
                            SkASSERT(0);
                            break;
                    }
                    break;
                case SkPath::kQuad_Verb:
                case SkPath::kConic_Verb:
                    switch (v) {
                        case SkPath::kLine_Verb:
                            pts[1] = pts[2];
                            break;
                        case SkPath::kConic_Verb:
                            weight = w;
                            [[fallthrough]];
                        case SkPath::kQuad_Verb:
                            break;
                        case SkPath::kCubic_Verb: {
                            SkDQuad dQuad;
                            dQuad.set(pts);
                            SkDCubic dCubic = dQuad.debugToCubic();
                            pts[3] = pts[2];
                            pts[1] = dCubic[1].asSkPoint();
                            pts[2] = dCubic[2].asSkPoint();
                            } break;
                         default:
                            SkASSERT(0);
                            break;
                    }
                    break;
                case SkPath::kCubic_Verb:
                    switch (v) {
                        case SkPath::kLine_Verb:
                            pts[1] = pts[3];
                            break;
                        case SkPath::kConic_Verb:
                            weight = w;
                            [[fallthrough]];
                        case SkPath::kQuad_Verb: {
                            SkDCubic dCubic;
                            dCubic.set(pts);
                            SkDQuad dQuad = dCubic.toQuad();
                            pts[1] = dQuad[1].asSkPoint();
                            pts[2] = pts[3];
                            } break;
                        default:
                            SkASSERT(0);
                            break;
                    }
                    break;
                default:
                    SkASSERT(0);
                    break;
            }
            verb = v;
        }
        switch (verb) {
            case SkPath::kMove_Verb:
                result.moveTo(pts[0]);
                break;
            case SkPath::kLine_Verb:
                result.lineTo(pts[1]);
                break;
            case SkPath::kQuad_Verb:
                result.quadTo(pts[1], pts[2]);
                break;
            case SkPath::kConic_Verb:
                result.conicTo(pts[1], pts[2], weight);
                break;
            case SkPath::kCubic_Verb:
                result.cubicTo(pts[1], pts[2], pts[3]);
                break;
            case SkPath::kClose_Verb:
                result.close();
                break;
            default:
                SkASSERT(0);
                break;
        }
    }
    *path = result;
}



static void construct_path(SkPath& path) {
    path.reset();
    path.moveTo(592, 515.5f);
    path.quadTo(871.5f, 383, 940, 610);
    path.quadTo(871.5f, 383, 498, 751.5f);
    path.cubicTo(249.667f, 703.667f, 170.333f, 346.833f, 218, 239);
    path.conicTo(465, 330.75f, 686, 167.5f, 0.6375f);
    path.lineTo(592, 515.5f);
    path.close();
}

struct ButtonPaintsJ {
    static const int kMaxStateCount = 3;
    SkPaint fDisabled;
    SkPaint fStates[kMaxStateCount];
    SkFont  fLabelFont;

    ButtonPaintsJ() {
        fStates[0].setAntiAlias(true);
        fStates[0].setStyle(SkPaint::kStroke_Style);
        fStates[0].setColor(0xFF3F0000);
        fStates[1] = fStates[0];
        fStates[1].setStrokeWidth(3);
        fStates[2] = fStates[1];
        fStates[2].setColor(0xFFcf0000);
        fLabelFont.setSize(25.0f);
    }
};

struct ButtonJ {
    SkRect fBounds;
    int fStateCount;
    int fState;
    char fLabel;
    bool fVisible;

    ButtonJ(char label) {
        fStateCount = 2;
        fState = 0;
        fLabel = label;
        fVisible = false;
    }

    ButtonJ(char label, int stateCount) {
        SkASSERT(stateCount <= ButtonPaintsJ::kMaxStateCount);
        fStateCount = stateCount;
        fState = 0;
        fLabel = label;
        fVisible = false;
    }

    bool contains(const SkRect& rect) {
        return fVisible && fBounds.contains(rect);
    }

    bool enabled() {
        return SkToBool(fState);
    }

    void draw(SkCanvas* canvas, const ButtonPaintsJ& paints) {
        if (!fVisible) {
            return;
        }
        canvas->drawRect(fBounds, paints.fStates[fState]);
        SkTextUtils::Draw(canvas, &fLabel, 1, SkTextEncoding::kUTF8, fBounds.centerX(), fBounds.fBottom - 5,
                          paints.fLabelFont, SkPaint(), SkTextUtils::kCenter_Align);
    }

    void toggle() {
        if (++fState == fStateCount) {
            fState = 0;
        }
    }

    void setEnabled(bool enabled) {
        fState = (int) enabled;
    }
};

struct ControlPaintsJ {
    SkPaint fOutline;
    SkPaint fIndicator;
    SkPaint fFill;
    SkPaint fLabel;
    SkPaint fValue;

    SkFont fLabelFont;
    SkFont fValueFont;

    ControlPaintsJ() {
        fOutline.setAntiAlias(true);
        fOutline.setStyle(SkPaint::kStroke_Style);
        fIndicator = fOutline;
        fIndicator.setColor(SK_ColorRED);
        fFill.setAntiAlias(true);
        fFill.setColor(0x7fff0000);
        fLabel.setAntiAlias(true);
        fLabelFont.setSize(13.0f);
        fValue.setAntiAlias(true);
        fValueFont.setSize(11.0f);
    }
};

struct UniControlJ {
    SkString fName;
    SkRect fBounds;
    SkScalar fMin;
    SkScalar fMax;
    SkScalar fVal;
    bool fVisible, fExponential;

    UniControlJ(const char* name, SkScalar min, SkScalar max) {
        fName = name;
        fVal = fMin = min;
        fMax = max;
        fVisible = fExponential = false;
    }

    virtual ~UniControlJ() {}

    bool contains(const SkRect& rect) {
        return fVisible && fBounds.contains(rect);
    }

    void setValByY(int y) {
        if (y < fBounds.fTop || y > fBounds.fBottom) {
            return;
        }
        if (fExponential) {
            SkScalar scale = (SkScalarLog(fMax) - SkScalarLog(fMin)) / fBounds.height();
            fVal = SkScalarExp(SkScalarLog(fMin) + scale * (SkIntToScalar(y) - fBounds.fTop));
        } else {
            fVal = ((SkIntToScalar(y) - fBounds.fTop) / fBounds.height()) * (fMax - fMin) + fMin;
        }
    }

    virtual void draw(SkCanvas* canvas, const ControlPaintsJ& paints) {
        if (!fVisible) {
            return;
        }
        canvas->drawRect(fBounds, paints.fOutline);
        SkScalar fY;
        if (fExponential) {
            SkScalar scale = (SkScalarLog(fMax) - SkScalarLog(fMin)) / fBounds.height();
            fY = fBounds.fTop + (SkScalarLog(fVal) - SkScalarLog(fMin)) / scale;
        } else {
            fY = fBounds.fTop + (fVal - fMin) * fBounds.height() / (fMax - fMin);
        }
        canvas->drawLine(fBounds.fLeft - 5, fY, fBounds.fRight + 5, fY, paints.fIndicator);
        SkString label;
        label.printf("%0.3g", fVal);
        canvas->drawString(label, fBounds.fLeft + 5, fY- 5, paints.fValueFont, paints.fValue);
        canvas->drawString(fName, fBounds.fLeft, fBounds.bottom() + 11, paints.fLabelFont,
                           paints.fLabel);
    }
};

class MyClickJ : public Sample::Click {
public:
    enum ClickType {
        kInvalidType = -1,
        kPtType,
        kVerbType,
        kControlType,
        kPathType,
    } fType;

    enum ControlType {
        kInvalidControl = -1,
        kFirstControl,
        kWeightControl = kFirstControl,
        kJoinLimitControl,
        kWidthControl,
        kLastControl = kWidthControl,
        kFirstButton,
        kBevelButton = kFirstButton,
        kRoundButton,
        kMiterButton,
        kMiterClipButton,
        kArcsButton,
        kLastJoinButton = kArcsButton,
        kFirstVerbButton,
        kCubicButton = kFirstVerbButton,
        kConicButton,
        kQuadButton,
        kLineButton,
        kLastVerbButton = kLineButton,
        kAddButton,
        kDeleteButton,
        kLastButton = kDeleteButton,
        kPathMove,
    } fControl;

    SkPath::Verb fVerb;
    SkScalar fWeight;

    MyClickJ(ClickType type, ControlType control)
        : fType(type)
        , fControl(control)
        , fVerb((SkPath::Verb) -1)
        , fWeight(1) {
    }

    MyClickJ(ClickType type, int index)
        : fType(type)
        , fControl((ControlType) index)
        , fVerb((SkPath::Verb) -1)
        , fWeight(1) {
    }

    MyClickJ(ClickType type, int index, SkPath::Verb verb, SkScalar weight)
        : fType(type)
        , fControl((ControlType) index)
        , fVerb(verb)
        , fWeight(weight) {
    }

    bool isButton() {
        return kFirstButton <= fControl && fControl <= kLastButton;
    }

    int ptHit() const {
        SkASSERT(fType == kPtType);
        return (int) fControl;
    }

    int verbHit() const {
        SkASSERT(fType == kVerbType);
        return (int) fControl;
    }
};

enum {
    kControlCount = MyClickJ::kLastControl - MyClickJ::kFirstControl + 1,
};

static struct ControlPair {
    UniControlJ* fControl;
    MyClickJ::ControlType fControlType;
} kControlList[kControlCount];

enum {
    kButtonCount = MyClickJ::kLastButton - MyClickJ::kFirstButton + 1,
    kJoinCount = MyClickJ::kLastJoinButton - MyClickJ::kFirstButton + 1,
    kVerbCount = MyClickJ::kLastVerbButton - MyClickJ::kFirstVerbButton + 1,
};

static struct ButtonPair {
    ButtonJ* fButton;
    MyClickJ::ControlType fButtonType;
} kButtonList[kButtonCount];

static void enable_verb_button(MyClickJ::ControlType type) {
    for (int index = 0; index < kButtonCount; ++index) {
        MyClickJ::ControlType testType = kButtonList[index].fButtonType;
        if (MyClickJ::kFirstVerbButton <= testType && testType <= MyClickJ::kLastVerbButton) {
            ButtonJ* button = kButtonList[index].fButton;
            button->setEnabled(testType == type);
        }
    }
}

static void enable_join_button(MyClickJ::ControlType type) {
    for (int index = 0; index < kButtonCount; ++index) {
        MyClickJ::ControlType testType = kButtonList[index].fButtonType;
        if (MyClickJ::kFirstButton <= testType && testType <= MyClickJ::kLastJoinButton) {
            ButtonJ* button = kButtonList[index].fButton;
            button->setEnabled(testType == type);
        }
    }
}

struct StrokeJ;

struct ActiveJ {
    ActiveJ* fNext;
    StrokeJ* fParent;
    SkScalar fStart;
    SkScalar fEnd;

    void reset() {
        fNext = nullptr;
        fStart = 0;
        fEnd = 1;
    }
};

struct StrokeJ {
    SkPath fPath;
    ActiveJ fActive;
    bool fInner;

    void reset() {
        fPath.reset();
        fActive.reset();
    }
};

struct PathUndoJ {
    SkPath fPath;
    std::unique_ptr<PathUndoJ> fNext;
};

class StrokeJoins : public Sample {
    SkPaint fActivePaint;
    SkFont fLegendLeftFont;
    SkFont fLegendRightFont;
    SkPaint fStrokePaint;
    SkPaint fPointPaint;
    SkPaint fSkeletonPaint;
    SkPath fPath;
    ControlPaintsJ fControlPaints;
    UniControlJ fWeightControl;
    UniControlJ fJoinLimitControl;
    UniControlJ fWidthControl;
    ButtonPaintsJ fButtonPaints;
    ButtonJ fBevelButton;
    ButtonJ fRoundButton;
    ButtonJ fMiterButton;
    ButtonJ fMiterClipButton;
    ButtonJ fArcsButton;
    ButtonJ fCubicButton;
    ButtonJ fConicButton;
    ButtonJ fQuadButton;
    ButtonJ fLineButton;
    ButtonJ fAddButton;
    ButtonJ fDeleteButton;
    SkTArray<StrokeJ> fStrokes;
    std::unique_ptr<PathUndoJ> fUndo;
    int fActivePt;
    int fActiveVerb;
    bool fHandlePathMove;
    bool fShowLegend;
    bool fHideAll;
    const int kHitToleranace = 25;

public:

    StrokeJoins()
        : fWeightControl("weight", 0, 5)
        , fJoinLimitControl("limit", .01, 300)
        , fWidthControl("width", FLT_EPSILON, 250)
        , fBevelButton('B')
        , fRoundButton('R')
        , fMiterButton('M')
        , fMiterClipButton('T')
        , fArcsButton('A')
        , fCubicButton('C')
        , fConicButton('K')
        , fQuadButton('Q')
        , fLineButton('L')
        , fAddButton('+')
        , fDeleteButton('x')
        , fActivePt(-1)
        , fActiveVerb(-1)
        , fHandlePathMove(true)
        , fShowLegend(false)
        , fHideAll(false)
    {
        fStrokePaint.setAntiAlias(true);
        fStrokePaint.setStyle(SkPaint::kStroke_Style);
        fStrokePaint.setColor(SK_ColorLTGRAY);
        fPointPaint = fStrokePaint;
        fPointPaint.setColor(0x99ee3300);
        fSkeletonPaint = fStrokePaint;
        fSkeletonPaint.setColor(SK_ColorRED);
        fActivePaint = fStrokePaint;
        fActivePaint.setColor(0x99ee3300);
        fActivePaint.setStrokeWidth(5);
        fStrokePaint.setStrokeJoin(SkPaint::kBevel_Join);
        fLegendLeftFont.setSize(13);
        fLegendRightFont = fLegendLeftFont;
        construct_path(fPath);
        fWidthControl.fVal = 100;
        fWidthControl.fVisible = true;
        fJoinLimitControl.fVal = 4;
        fJoinLimitControl.fExponential = true;
        fBevelButton.fVisible = true;
        fBevelButton.fVisible = true;
        fRoundButton.fVisible = true;
        fMiterButton.fVisible = true;
        fMiterClipButton.fVisible = true;
        fArcsButton.fVisible = true;
        fBevelButton.setEnabled(true);
        init_controlList();
        init_buttonList();
    }

    ~StrokeJoins() override {
        // Free linked list without deep recursion.
        std::unique_ptr<PathUndoJ> undo = std::move(fUndo);
        while (undo) {
            undo = std::move(undo->fNext);
        }
    }

    bool constructPath() {
        construct_path(fPath);
        return true;
    }

    void savePath(skui::InputState state) {
        if (state != skui::InputState::kDown) {
            return;
        }
        if (fUndo && fUndo->fPath == fPath) {
            return;
        }
        std::unique_ptr<PathUndoJ> undo(new PathUndoJ);
        undo->fPath = fPath;
        undo->fNext = std::move(fUndo);
        fUndo = std::move(undo);
    }

    bool undo() {
        if (!fUndo) {
            return false;
        }
        fPath = std::move(fUndo->fPath);
        fUndo = std::move(fUndo->fNext);
        validatePath();
        return true;
    }

    void validatePath() {}

    void set_controlList(int index, UniControlJ* control, MyClickJ::ControlType type) {
        kControlList[index].fControl = control;
        kControlList[index].fControlType = type;
    }

    #define SET_CONTROL(Name) set_controlList(index++, &f##Name##Control, \
        MyClickJ::k##Name##Control)

    bool hideAll() {
        fHideAll ^= true;
        return true;
    }

    void init_controlList() {
        int index = 0;
        SET_CONTROL(Width);
        SET_CONTROL(JoinLimit);
        SET_CONTROL(Weight);
    }

    #undef SET_CONTROL

    void set_buttonList(int index, ButtonJ* button, MyClickJ::ControlType type) {
        kButtonList[index].fButton = button;
        kButtonList[index].fButtonType = type;
    }

    #define SET_BUTTON(Name) set_buttonList(index++, &f##Name##Button, \
            MyClickJ::k##Name##Button)

    void init_buttonList() {
        int index = 0;
        SET_BUTTON(Bevel);
        SET_BUTTON(Round);
        SET_BUTTON(Miter);
        SET_BUTTON(MiterClip);
        SET_BUTTON(Arcs);
        SET_BUTTON(Cubic);
        SET_BUTTON(Conic);
        SET_BUTTON(Quad);
        SET_BUTTON(Line);
        SET_BUTTON(Add);
        SET_BUTTON(Delete);
    }

    #undef SET_BUTTON

    SkString name() override { return SkString("StrokeJoins"); }

    bool onChar(SkUnichar) override;

    void onSizeChange() override {
        setControlButtonsPos();
        this->INHERITED::onSizeChange();
    }

    bool pathDump() {
        fPath.dump();
        return true;
    }

/*
    bool pathRDump() {
        fPath.rDump();
        return true;
    }
*/

    bool scaleDown() {
        SkMatrix matrix;
        SkRect bounds = fPath.getBounds();
        matrix.setScale(1.f / 1.5f, 1.f / 1.5f, bounds.centerX(), bounds.centerY());
        fPath.transform(matrix);
        validatePath();
        return true;
    }

    bool scaleToFit() {
        SkMatrix matrix;
        SkRect bounds = fPath.getBounds();
        SkScalar scale = std::min(this->width() / bounds.width(), this->height() / bounds.height())
                * 0.8f;
        matrix.setScale(scale, scale, bounds.centerX(), bounds.centerY());
        fPath.transform(matrix);
        bounds = fPath.getBounds();
        SkScalar offsetX = (this->width() - bounds.width()) / 2 - bounds.fLeft;
        SkScalar offsetY = (this->height() - bounds.height()) / 2 - bounds.fTop;
        fPath.offset(offsetX, offsetY);
        validatePath();
        return true;
    }

    bool scaleUp() {
        SkMatrix matrix;
        SkRect bounds = fPath.getBounds();
        matrix.setScale(1.5f, 1.5f, bounds.centerX(), bounds.centerY());
        fPath.transform(matrix);
        validatePath();
        return true;
    }

    void setControlButtonsPos() {
        SkScalar widthOffset = this->width() - 100;
        for (int index = 0; index < kControlCount; ++index) {
            if (kControlList[index].fControl->fVisible) {
                kControlList[index].fControl->fBounds.setXYWH(widthOffset, 30, 30, 400);
                widthOffset -= 50;
            }
        }
        SkScalar buttonOffset = 0;
        for (int index = 0; index < kButtonCount; ++index) {
            kButtonList[index].fButton->fBounds.setXYWH(this->width() - 50,
                    buttonOffset += 50, 30, 30);
        }
    }

    bool showLegend() {
        fShowLegend ^= true;
        return true;
    }

    void draw_legend(SkCanvas* canvas);

    void draw_segment(SkCanvas* canvas) {
        SkPoint pts[4];
        SkPath::Verb verb;
        SkPath::Iter iter(fPath, true);
        int counter = -1;
        while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
            if (++counter < fActiveVerb) {
                continue;
            }
            switch (verb) {
                case SkPath::kLine_Verb:
                    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, pts, fActivePaint);
                    draw_points(canvas, pts, 2);
                    break;
                case SkPath::kQuad_Verb: {
                    SkPath qPath;
                    qPath.moveTo(pts[0]);
                    qPath.quadTo(pts[1], pts[2]);
                    canvas->drawPath(qPath, fActivePaint);
                    draw_points(canvas, pts, 3);
                    } break;
                case SkPath::kConic_Verb: {
                    SkPath conicPath;
                    conicPath.moveTo(pts[0]);
                    conicPath.conicTo(pts[1], pts[2], iter.conicWeight());
                    canvas->drawPath(conicPath, fActivePaint);
                    draw_points(canvas, pts, 3);
                    } break;
                case SkPath::kCubic_Verb: {
                    SkPath cPath;
                    cPath.moveTo(pts[0]);
                    cPath.cubicTo(pts[1], pts[2], pts[3]);
                    canvas->drawPath(cPath, fActivePaint);
                    draw_points(canvas, pts, 4);
                    } break;
                default:
                    break;
            }
            return;
        }
    }

    void draw_points(SkCanvas* canvas, SkPoint* points, int count) {
        for (int index = 0; index < count; ++index) {
            canvas->drawCircle(points[index].fX, points[index].fY, 10, fPointPaint);
        }
    }

    int hittest_verb(SkPoint pt, SkPath::Verb* verbPtr, SkScalar* weight) {
        SkIntersections i;
        SkDLine hHit = {{{pt.fX - kHitToleranace, pt.fY }, {pt.fX + kHitToleranace, pt.fY}}};
        SkDLine vHit = {{{pt.fX, pt.fY - kHitToleranace }, {pt.fX, pt.fY + kHitToleranace}}};
        SkPoint pts[4];
        SkPath::Verb verb;
        SkPath::Iter iter(fPath, true);
        int counter = -1;
        while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
            ++counter;
            switch (verb) {
                case SkPath::kLine_Verb: {
                    SkDLine line;
                    line.set(pts);
                    if (i.intersect(line, hHit) || i.intersect(line, vHit)) {
                        *verbPtr = verb;
                        *weight = 1;
                        return counter;
                    }
                    } break;
                case SkPath::kQuad_Verb: {
                    SkDQuad quad;
                    quad.set(pts);
                    if (i.intersect(quad, hHit) || i.intersect(quad, vHit)) {
                        *verbPtr = verb;
                        *weight = 1;
                        return counter;
                    }
                    } break;
                case SkPath::kConic_Verb: {
                    SkDConic conic;
                    SkScalar w = iter.conicWeight();
                    conic.set(pts, w);
                    if (i.intersect(conic, hHit) || i.intersect(conic, vHit)) {
                        *verbPtr = verb;
                        *weight = w;
                        return counter;
                    }
                    } break;
                case SkPath::kCubic_Verb: {
                    SkDCubic cubic;
                    cubic.set(pts);
                    if (i.intersect(cubic, hHit) || i.intersect(cubic, vHit)) {
                        *verbPtr = verb;
                        *weight = 1;
                        return counter;
                    }
                    } break;
                default:
                    break;
            }
        }
        return -1;
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkRect bounds = fPath.getBounds();
        SkScalar width = fWidthControl.fVal, radius = width*.5f;
        SkScalar joinLimit = fJoinLimitControl.fVal;
        int w = (int) (bounds.fRight + radius + 1);
        int h = (int) (bounds.fBottom + radius + 1);
        SkImageInfo imageInfo = SkImageInfo::MakeA8(w, h);
        fStrokePaint.setStrokeWidth(width);
        fStrokePaint.setStrokeMiter(joinLimit);
        canvas->drawPath(fPath, fStrokePaint);
        canvas->drawPath(fPath, fSkeletonPaint);
        if (fActiveVerb >= 0) {
            draw_segment(canvas);
        }
        if (fHideAll) {
            return;
        }
        for (int index = 0; index < kControlCount; ++index) {
            kControlList[index].fControl->draw(canvas, fControlPaints);
        }
        for (int index = 0; index < kButtonCount; ++index) {
            kButtonList[index].fButton->draw(canvas, fButtonPaints);
        }
        if (fShowLegend) {
            draw_legend(canvas);
        }
    }

    int hittest_pt(SkPoint pt) {
        for (int index = 0; index < fPath.countPoints(); ++index) {
            if (SkPoint::Distance(fPath.getPoint(index), pt) <= kHitToleranace * 2) {
                return index;
            }
        }
        return -1;
    }

    virtual Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        SkPoint pt = {x, y};
        int ptHit = hittest_pt(pt);
        if (ptHit >= 0) {
            return new MyClickJ(MyClickJ::kPtType, ptHit);
        }
        SkPath::Verb verb;
        SkScalar weight;
        int verbHit = hittest_verb(pt, &verb, &weight);
        if (verbHit >= 0) {
            return new MyClickJ(MyClickJ::kVerbType, verbHit, verb, weight);
        }
        if (!fHideAll) {
            const SkRect& rectPt = SkRect::MakeXYWH(x, y, 1, 1);
            for (int index = 0; index < kControlCount; ++index) {
                if (kControlList[index].fControl->contains(rectPt)) {
                    return new MyClickJ(MyClickJ::kControlType,
                            kControlList[index].fControlType);
                }
            }
            for (int index = 0; index < kButtonCount; ++index) {
                if (kButtonList[index].fButton->contains(rectPt)) {
                    return new MyClickJ(MyClickJ::kControlType, kButtonList[index].fButtonType);
                }
            }
        }
        fLineButton.fVisible
                = fQuadButton.fVisible = fConicButton.fVisible = fCubicButton.fVisible
                = fWeightControl.fVisible = fAddButton.fVisible
                = fDeleteButton.fVisible = false;
        fActiveVerb = -1;
        fActivePt = -1;
        if (fHandlePathMove) {
            return new MyClickJ(MyClickJ::kPathType, MyClickJ::kPathMove);
        }
        return nullptr;
    }

    bool onClick(Click* click) override {
        MyClickJ* myClick = (MyClickJ*) click;
        switch (myClick->fType) {
            case MyClickJ::kPtType: {
                savePath(click->fState);
                fActivePt = myClick->ptHit();
                SkPoint pt = fPath.getPoint((int) myClick->fControl);
                pt.offset(SkIntToScalar(click->fCurr.fX - click->fPrev.fX),
                        SkIntToScalar(click->fCurr.fY - click->fPrev.fY));
                SkPathPriv::UpdatePathPoint(&fPath, fActivePt, pt);
                validatePath();
                return true;
                }
            case MyClickJ::kPathType:
                savePath(click->fState);
                fPath.offset(SkIntToScalar(click->fCurr.fX - click->fPrev.fX),
                        SkIntToScalar(click->fCurr.fY - click->fPrev.fY));
                validatePath();
                return true;
            case MyClickJ::kVerbType: {
                fActiveVerb = myClick->verbHit();
                fLineButton.fVisible = fQuadButton.fVisible = fConicButton.fVisible
                        = fCubicButton.fVisible = fAddButton.fVisible = fDeleteButton.fVisible
                        = true;
                fLineButton.setEnabled(myClick->fVerb == SkPath::kLine_Verb);
                fQuadButton.setEnabled(myClick->fVerb == SkPath::kQuad_Verb);
                fConicButton.setEnabled(myClick->fVerb == SkPath::kConic_Verb);
                fCubicButton.setEnabled(myClick->fVerb == SkPath::kCubic_Verb);
                fWeightControl.fVal = myClick->fWeight;
                fWeightControl.fVisible = myClick->fVerb == SkPath::kConic_Verb;
                } break;
            case MyClickJ::kControlType: {
                if (click->fState != skui::InputState::kDown && myClick->isButton()) {
                    return true;
                }
                switch (myClick->fControl) {
                    case MyClickJ::kWeightControl: {
                        savePath(click->fState);
                        fWeightControl.setValByY(click->fCurr.fY);
                        set_path_weight(fActiveVerb, fWeightControl.fVal, &fPath);
                        validatePath();
                        } break;
                    case MyClickJ::kWidthControl:
                        fWidthControl.setValByY(click->fCurr.fY);
                        break;
                    case MyClickJ::kJoinLimitControl:
                        fJoinLimitControl.setValByY(click->fCurr.fY);
                        break;
                    case MyClickJ::kBevelButton:
                        enable_join_button(myClick->fControl);
                        fJoinLimitControl.fVisible = false;
                        fStrokePaint.setStrokeJoin(SkPaint::kBevel_Join);
                        break;
                    case MyClickJ::kRoundButton:
                        enable_join_button(myClick->fControl);
                        fJoinLimitControl.fVisible = false;
                        fStrokePaint.setStrokeJoin(SkPaint::kRound_Join);
                        break;
                    case MyClickJ::kMiterButton:
                        enable_join_button(myClick->fControl);
                        fJoinLimitControl.fVisible = true;
                        fStrokePaint.setStrokeJoin(SkPaint::kMiter_Join);
                        break;
                    case MyClickJ::kMiterClipButton:
                        enable_join_button(myClick->fControl);
                        fJoinLimitControl.fVisible = true;
                        fStrokePaint.setStrokeJoin(SkPaint::kMiterClip_Join);
                        break;
                    case MyClickJ::kArcsButton:
                        enable_join_button(myClick->fControl);
                        fJoinLimitControl.fVisible = true;
                        fStrokePaint.setStrokeJoin(SkPaint::kArcs_Join);
                        break;
                    case MyClickJ::kLineButton:
                        savePath(click->fState);
                        enable_verb_button(myClick->fControl);
                        fWeightControl.fVisible = false;
                        set_path_verb(fActiveVerb, SkPath::kLine_Verb, &fPath, 1);
                        validatePath();
                        break;
                    case MyClickJ::kQuadButton:
                        savePath(click->fState);
                        enable_verb_button(myClick->fControl);
                        fWeightControl.fVisible = false;
                        set_path_verb(fActiveVerb, SkPath::kQuad_Verb, &fPath, 1);
                        validatePath();
                        break;
                    case MyClickJ::kConicButton: {
                        savePath(click->fState);
                        enable_verb_button(myClick->fControl);
                        fWeightControl.fVisible = true;
                        const SkScalar defaultConicWeight = 1.f / SkScalarSqrt(2);
                        set_path_verb(fActiveVerb, SkPath::kConic_Verb, &fPath, defaultConicWeight);
                        validatePath();
                        fWeightControl.fVal = get_path_weight(fActiveVerb, fPath);
                        } break;
                    case MyClickJ::kCubicButton:
                        savePath(click->fState);
                        enable_verb_button(myClick->fControl);
                        fWeightControl.fVisible = false;
                        set_path_verb(fActiveVerb, SkPath::kCubic_Verb, &fPath, 1);
                        validatePath();
                        break;
                    case MyClickJ::kAddButton:
                        savePath(click->fState);
                        add_path_segment(fActiveVerb, &fPath);
                        validatePath();
                        if (fWeightControl.fVisible) {
                            fWeightControl.fVal = get_path_weight(fActiveVerb, fPath);
                        }
                        break;
                    case MyClickJ::kDeleteButton:
                        savePath(click->fState);
                        delete_path_segment(fActiveVerb, &fPath);
                        validatePath();
                        break;
                    default:
                        SkASSERT(0);
                        break;
                }
            } break;
            default:
                SkASSERT(0);
                break;
        }
        setControlButtonsPos();
        return true;
    }

private:
    typedef Sample INHERITED;
};

static struct KeyCommandJ {
    char fKey;
    char fAlternate;
    const char* fDescriptionL;
    const char* fDescriptionR;
    bool (StrokeJoins::*fFunction)();
} kKeyCommandList[] = {
    { ' ',  0,  "space",   "center path", &StrokeJoins::scaleToFit },
    { '-',  0,  "-",          "zoom out", &StrokeJoins::scaleDown },
    { '+', '=', "+/=",         "zoom in", &StrokeJoins::scaleUp },
    { 'D',  0,  "D",   "dump to console", &StrokeJoins::pathDump },
//    { 'P',  0,  "P",  "rDump to console", &StrokeJoins::pathRDump },
    { 'H',  0,  "H",     "hide controls", &StrokeJoins::hideAll },
    { '0',  0,  "0",        "reset path", &StrokeJoins::constructPath },
    { 'Z',  0,  "Z",              "undo", &StrokeJoins::undo },
    { '?',  0,  "?",       "show legend", &StrokeJoins::showLegend },
};

const int kKeyCommandCount = (int) SK_ARRAY_COUNT(kKeyCommandList);

void StrokeJoins::draw_legend(SkCanvas* canvas) {
    SkScalar bottomOffset = this->height() - 10;
    for (int index = kKeyCommandCount - 1; index >= 0; --index) {
        bottomOffset -= 15;
        SkTextUtils::DrawString(canvas, kKeyCommandList[index].fDescriptionL, this->width() - 160, bottomOffset,
                                fLegendLeftFont, SkPaint());
        SkTextUtils::DrawString(canvas, kKeyCommandList[index].fDescriptionR,
                this->width() - 20, bottomOffset,
                fLegendRightFont, SkPaint(), SkTextUtils::kRight_Align);
    }
}

bool StrokeJoins::onChar(SkUnichar uni) {
        for (int index = 0; index < kButtonCount; ++index) {
            ButtonJ* button = kButtonList[index].fButton;
            if (button->fVisible && uni == button->fLabel) {
                MyClickJ click(MyClickJ::kControlType, kButtonList[index].fButtonType);
                click.fState = skui::InputState::kDown;
                (void) this->onClick(&click);
                return true;
            }
        }
        for (int index = 0; index < kKeyCommandCount; ++index) {
            KeyCommandJ& keyCommand = kKeyCommandList[index];
            if (uni == keyCommand.fKey || uni == keyCommand.fAlternate) {
                return (this->*keyCommand.fFunction)();
            }
        }
        if (('A' <= uni && uni <= 'Z') || ('a' <= uni && uni <= 'z')) {
            for (int index = 0; index < kButtonCount; ++index) {
                ButtonJ* button = kButtonList[index].fButton;
                if (button->fVisible && (uni & ~0x20) == (button->fLabel & ~0x20)) {
                    MyClickJ click(MyClickJ::kControlType, kButtonList[index].fButtonType);
                    click.fState = skui::InputState::kDown;
                    (void) this->onClick(&click);
                    return true;
                }
            }
        }
        return false;
}

DEF_SAMPLE( return new StrokeJoins; )
