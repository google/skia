/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PerfectStroke.h"
#include "SampleCode.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkGeometry.h"
#include "SkIntersections.h"
#include "SkOpEdgeBuilder.h"
#include "SkPathMeasure.h"
#include "SkPointPriv.h"
#include "SkString.h"
#include "SkView.h"

static inline bool degenerate_vector(const SkVector& v) {
    return !SkPointPriv::CanNormalize(v.fX, v.fY);
}

SkScalar gCurveDistance = 10;

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

static void set_path_pt(int index, const SkPoint& pt, SkPath* path) {
    SkPath result;
    SkPoint pts[4];
    SkPath::Verb verb;
    SkPath::RawIter iter(*path);
    int startIndex = 0;
    int endIndex = 0;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                endIndex += 1;
                break;
            case SkPath::kLine_Verb:
                endIndex += 1;
                break;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
                endIndex += 2;
                break;
            case SkPath::kCubic_Verb:
                endIndex += 3;
                break;
            case SkPath::kClose_Verb:
                break;
            case SkPath::kDone_Verb:
                break;
            default:
                SkASSERT(0);
        }
        if (startIndex <= index && index < endIndex) {
            pts[index - startIndex] = pt;
            index = -1;
        }
        switch (verb) {
            case SkPath::kMove_Verb:
                result.moveTo(pts[0]);
                break;
            case SkPath::kLine_Verb:
                result.lineTo(pts[1]);
                startIndex += 1;
                break;
            case SkPath::kQuad_Verb:
                result.quadTo(pts[1], pts[2]);
                startIndex += 2;
                break;
            case SkPath::kConic_Verb:
                result.conicTo(pts[1], pts[2], iter.conicWeight());
                startIndex += 2;
                break;
            case SkPath::kCubic_Verb:
                result.cubicTo(pts[1], pts[2], pts[3]);
                startIndex += 3;
                break;
            case SkPath::kClose_Verb:
                result.close();
                startIndex += 1;
                break;
            case SkPath::kDone_Verb:
                break;
            default:
                SkASSERT(0);
        }
    }
#if 0
    SkDebugf("\n\noriginal\n");
    path->dump();
    SkDebugf("\nedited\n");
    result.dump();
#endif
    *path = result;
}

static void add_path_segment(int index, SkPath* path) {
    SkPath result;
    SkPoint pts[4];
    SkPoint firstPt = { 0, 0 };  // init to avoid warning
    SkPoint lastPt = { 0, 0 };  // init to avoid warning
    SkPath::Verb verb;
    SkPath::RawIter iter(*path);
    int counter = -1;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        SkScalar weight  SK_INIT_TO_AVOID_WARNING;
        if (++counter == index) {
            switch (verb) {
                case SkPath::kLine_Verb:
                    result.lineTo((pts[0].fX + pts[1].fX) / 2, (pts[0].fY + pts[1].fY) / 2);
                    break;
                case SkPath::kQuad_Verb: {
                    SkPoint chop[5];
                    SkChopQuadAtHalf(pts, chop);
                    result.quadTo(chop[1], chop[2]);
                    pts[1] = chop[3];
                    } break;
                case SkPath::kConic_Verb: {
                    SkConic chop[2];
                    SkConic conic;
                    conic.set(pts, iter.conicWeight());
                    if (!conic.chopAt(0.5f, chop)) {
                        return;
                    }
                    result.conicTo(chop[0].fPts[1], chop[0].fPts[2], chop[0].fW);
                    pts[1] = chop[1].fPts[1];
                    weight = chop[1].fW;
                    } break;
                case SkPath::kCubic_Verb: {
                    SkPoint chop[7];
                    SkChopCubicAtHalf(pts, chop);
                    result.cubicTo(chop[1], chop[2], chop[3]);
                    pts[1] = chop[4];
                    pts[2] = chop[5];
                    } break;
                case SkPath::kClose_Verb: {
                    result.lineTo((lastPt.fX + firstPt.fX) / 2, (lastPt.fY + firstPt.fY) / 2);
                    } break;
                default:
                    SkASSERT(0);
            }
        } else if (verb == SkPath::kConic_Verb) {
            weight = iter.conicWeight();
        }
        switch (verb) {
            case SkPath::kMove_Verb:
                result.moveTo(firstPt = pts[0]);
                break;
            case SkPath::kLine_Verb:
                result.lineTo(lastPt = pts[1]);
                break;
            case SkPath::kQuad_Verb:
                result.quadTo(pts[1], lastPt = pts[2]);
                break;
            case SkPath::kConic_Verb:
                result.conicTo(pts[1], lastPt = pts[2], weight);
                break;
            case SkPath::kCubic_Verb:
                result.cubicTo(pts[1], pts[2], lastPt = pts[3]);
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

static void delete_path_segment(int index, SkPath* path) {
    SkPath result;
    SkPoint pts[4];
    SkPath::Verb verb;
    SkPath::RawIter iter(*path);
    int counter = -1;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (++counter == index) {
            continue;
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
                result.conicTo(pts[1], pts[2], iter.conicWeight());
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

static void add_to_map(SkScalar coverage, int x, int y, uint8_t* distanceMap, int w, int h) {
    int byteCoverage = (int) (coverage * 256);
    if (byteCoverage < 0) {
        byteCoverage = 0;
    } else if (byteCoverage > 255) {
        byteCoverage = 255;
    }
    SkASSERT(x < w);
    SkASSERT(y < h);
    distanceMap[y * w + x] = SkTMax(distanceMap[y * w + x], (uint8_t) byteCoverage);
}

static void filter_coverage(const uint8_t* map, int len, uint8_t min, uint8_t max,
        uint8_t* filter) {
    for (int index = 0; index < len; ++index) {
        uint8_t in = map[index];
        filter[index] = in < min ? 0 : max < in ? 0 : in;
    }
}

static void construct_path(SkPath& path) {
    path.reset();
    path.moveTo(442, 101.5f);
    path.quadTo(413.5f, 691, 772, 514);
    path.lineTo(346, 721.5f);
    path.lineTo(154, 209);
    path.lineTo(442, 101.5f);
    path.close();
}

struct ButtonPaints {
    static const int kMaxStateCount = 4;
    SkPaint fDisabled;
    SkPaint fStates[kMaxStateCount];
    SkPaint fLabel;

    ButtonPaints() {
        fStates[0].setAntiAlias(true);
        fStates[0].setStyle(SkPaint::kStroke_Style);
        fStates[0].setColor(0xFF3F0000);
        fStates[1] = fStates[0];
        fStates[1].setStrokeWidth(3);
        fStates[2] = fStates[1];
        fStates[2].setColor(0xFFcf0000);
        fLabel.setAntiAlias(true);
        fLabel.setTextSize(25.0f);
        fLabel.setTextAlign(SkPaint::kCenter_Align);
        fLabel.setStyle(SkPaint::kFill_Style);
    }
};

struct Button {
    SkRect fBounds;
    int fStateCount;
    int fState;
    char fLabel;
    bool fVisible;

    Button(char label) {
        fStateCount = 2;
        fState = 0;
        fLabel = label;
        fVisible = false;
    }

    Button(char label, int stateCount) {
        SkASSERT(stateCount <= ButtonPaints::kMaxStateCount);
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

    void draw(SkCanvas* canvas, const ButtonPaints& paints) {
        if (!fVisible) {
            return;
        }
        canvas->drawRect(fBounds, paints.fStates[fState]);
        canvas->drawText(&fLabel, 1, fBounds.centerX(), fBounds.fBottom - 5, paints.fLabel);
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

struct ControlPaints {
    SkPaint fOutline;
    SkPaint fIndicator;
    SkPaint fFill;
    SkPaint fLabel;
    SkPaint fValue;

    ControlPaints() {
        fOutline.setAntiAlias(true);
        fOutline.setStyle(SkPaint::kStroke_Style);
        fIndicator = fOutline;
        fIndicator.setColor(SK_ColorRED);
        fFill.setAntiAlias(true);
        fFill.setColor(0x7fff0000);
        fLabel.setAntiAlias(true);
        fLabel.setTextSize(13.0f);
        fValue.setAntiAlias(true);
        fValue.setTextSize(11.0f);
    }
};

struct UniControl {
    SkString fName;
    SkRect fBounds;
    SkScalar fMin;
    SkScalar fMax;
    SkScalar fValLo;
    SkScalar fYLo;
    bool fVisible;

    UniControl(const char* name, SkScalar min, SkScalar max) {
        fName = name;
        fValLo =  fMin = min;
        fMax = max;
        fVisible = false;

    }

    virtual ~UniControl() {}

    bool contains(const SkRect& rect) {
        return fVisible && fBounds.contains(rect);
    }

    virtual void draw(SkCanvas* canvas, const ControlPaints& paints) {
        if (!fVisible) {
            return;
        }
        canvas->drawRect(fBounds, paints.fOutline);
        fYLo = fBounds.fTop + (fValLo - fMin) * fBounds.height() / (fMax - fMin);
        canvas->drawLine(fBounds.fLeft - 5, fYLo, fBounds.fRight + 5, fYLo, paints.fIndicator);
        SkString label;
        label.printf("%0.3g", fValLo);
        canvas->drawString(label, fBounds.fLeft + 5, fYLo - 5, paints.fValue);
        canvas->drawString(fName, fBounds.fLeft, fBounds.bottom() + 11,
                paints.fLabel);
    }
};

struct BiControl : public UniControl {
    SkScalar fValHi;

    BiControl(const char* name, SkScalar min, SkScalar max)
        : UniControl(name, min, max)
        ,  fValHi(fMax) {
    }

    virtual ~BiControl() {}

    virtual void draw(SkCanvas* canvas, const ControlPaints& paints) {
        UniControl::draw(canvas, paints);
        if (!fVisible || fValHi == fValLo) {
            return;
        }
        SkScalar yPos = fBounds.fTop + (fValHi - fMin) * fBounds.height() / (fMax - fMin);
        canvas->drawLine(fBounds.fLeft - 5, yPos, fBounds.fRight + 5, yPos, paints.fIndicator);
        SkString label;
        label.printf("%0.3g", fValHi);
        if (yPos < fYLo + 10) {
            yPos = fYLo + 10;
        }
        canvas->drawString(label, fBounds.fLeft + 5, yPos - 5, paints.fValue);
        SkRect fill = { fBounds.fLeft, fYLo, fBounds.fRight, yPos };
        canvas->drawRect(fill, paints.fFill);
    }
};


class MyClick : public SampleView::Click {
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
        kFilterControl = kFirstControl,
        kResControl,
        kWeightControl,
        kWidthControl,
        kLastControl = kWidthControl,
        kFirstButton,
        kCubicButton = kFirstButton,
        kConicButton,
        kQuadButton,
        kLineButton,
        kLastVerbButton = kLineButton,
        kAddButton,
        kDeleteButton,
        kInOutButton,
        kFillButton,
        kSkeletonButton,
        kFilterButton,
        kBisectButton,
        kJoinButton,
        kLastButton = kJoinButton,
        kPathMove,
    } fControl;

    SkPath::Verb fVerb;
    SkScalar fWeight;

    MyClick(SkView* target, ClickType type, ControlType control)
        : Click(target)
        , fType(type)
        , fControl(control)
        , fVerb((SkPath::Verb) -1)
        , fWeight(1) {
    }

    MyClick(SkView* target, ClickType type, int index)
        : Click(target)
        , fType(type)
        , fControl((ControlType) index)
        , fVerb((SkPath::Verb) -1)
        , fWeight(1) {
    }

    MyClick(SkView* target, ClickType type, int index, SkPath::Verb verb, SkScalar weight)
        : Click(target)
        , fType(type)
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

constexpr int kControlCount = MyClick::kLastControl - MyClick::kFirstControl + 1;

static struct ControlPair {
    UniControl* fControl;
    MyClick::ControlType fControlType;
} kControlList[kControlCount];

constexpr int kButtonCount = MyClick::kLastButton - MyClick::kFirstButton + 1;
constexpr int kVerbCount = MyClick::kLastVerbButton - MyClick::kFirstButton + 1;

static struct ButtonPair {
    Button* fButton;
    MyClick::ControlType fButtonType;
} kButtonList[kButtonCount];

static void enable_verb_button(MyClick::ControlType type) {
    for (int index = 0; index < kButtonCount; ++index) {
        MyClick::ControlType testType = kButtonList[index].fButtonType;
        if (MyClick::kFirstButton <= testType && testType <= MyClick::kLastVerbButton) {
            Button* button = kButtonList[index].fButton;
            button->setEnabled(testType == type);
        }
    }
}

struct PathUndo {
    SkPath fPath;
    PathUndo* fNext;
};

class AAGeometryView : public SampleView {
    SkPaint fActivePaint;
    SkPaint fComplexPaint;
    SkPaint fCoveragePaint;
    SkPaint fLegendLeftPaint;
    SkPaint fLegendRightPaint;
    SkPaint fPointPaint;
    SkPaint fSkeletonPaint;
    SkPaint fDebugSkeletonPaint;
    SkPaint fLightSkeletonPaint;
    ControlPaints fControlPaints;
    UniControl fResControl;
    UniControl fWeightControl;
    UniControl fWidthControl;
    BiControl fFilterControl;
    ButtonPaints fButtonPaints;
    Button fCubicButton;
    Button fConicButton;
    Button fQuadButton;
    Button fLineButton;
    Button fAddButton;
    Button fDeleteButton;
    Button fFillButton;
    Button fSkeletonButton;
    Button fFilterButton;
    Button fBisectButton;
    Button fJoinButton;
    Button fInOutButton;
    PathUndo* fUndo;
    SkPoint fFirstBisect[2];
    SkPoint fLastBisect[2];
    SkPoint fLastPt;
    SkScalar fBallInterval;
    int fActivePt;
    int fActiveVerb;
    SkPath::Verb fFirstVerb;
    SkPath::Verb fLastVerb;
    bool fHandlePathMove;
    bool fShowLegend;
    bool fHideAll;
    const int kHitToleranace = 25;
    SkPath fPath;
    StrokePath fStrokePath;
public:

    AAGeometryView()
        : fResControl("error", 0, 10)
        , fWeightControl("weight", 0, 5)
        , fWidthControl("width", FLT_EPSILON, 100)
        , fFilterControl("filter", 0, 255)
        , fCubicButton('C')
        , fConicButton('K')
        , fQuadButton('Q')
        , fLineButton('L')
        , fAddButton('+')
        , fDeleteButton('x')
        , fFillButton('p')
        , fSkeletonButton('s')
        , fFilterButton('f', 4)
        , fBisectButton('b')
        , fJoinButton('j')
        , fInOutButton('|')
        , fUndo(nullptr)
        , fBallInterval(5)
        , fActivePt(-1)
        , fActiveVerb(-1)
        , fHandlePathMove(true)
        , fShowLegend(false)
        , fHideAll(false)
    {
        fCoveragePaint.setAntiAlias(true);
        fCoveragePaint.setColor(SK_ColorBLUE);
        SkPaint strokePaint;
        strokePaint.setAntiAlias(true);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        fPointPaint = strokePaint;
        fPointPaint.setColor(0x99ee3300);
        fSkeletonPaint = strokePaint;
        fSkeletonPaint.setColor(SK_ColorRED);
        fDebugSkeletonPaint = fSkeletonPaint;
        fDebugSkeletonPaint.setColor(SK_ColorBLUE);
        fLightSkeletonPaint = fSkeletonPaint;
        fLightSkeletonPaint.setColor(0xFFFF7f7f);
        fActivePaint = strokePaint;
        fActivePaint.setColor(0x99ee3300);
        fActivePaint.setStrokeWidth(5);
        fComplexPaint = fActivePaint;
        fComplexPaint.setColor(SK_ColorBLUE);
        fLegendLeftPaint.setAntiAlias(true);
        fLegendLeftPaint.setTextSize(13);
        fLegendRightPaint = fLegendLeftPaint;
        fLegendRightPaint.setTextAlign(SkPaint::kRight_Align);
        construct_path(fPath);
        fFillButton.fVisible = fSkeletonButton.fVisible = fFilterButton.fVisible
                = fBisectButton.fVisible = fJoinButton.fVisible = fInOutButton.fVisible = true;
        fSkeletonButton.setEnabled(true);
        fInOutButton.setEnabled(true);
        fJoinButton.setEnabled(true);
        fFilterControl.fValLo = 120;
        fFilterControl.fValHi = 141;
        fFilterControl.fVisible = fFilterButton.fState == 2;
        fResControl.fValLo = 5;
        fResControl.fVisible = true;
        fWidthControl.fValLo = 50;
        fWidthControl.fVisible = true;
        init_controlList();
        init_buttonList();
    }

    bool constructPath() {
        construct_path(fPath);
        return true;
    }

    void savePath(Click::State state) {
        if (state != Click::kDown_State) {
            return;
        }
        if (fUndo && fUndo->fPath == fPath) {
            return;
        }
        PathUndo* undo = new PathUndo;
        undo->fPath = fPath;
        undo->fNext = fUndo;
        fUndo = undo;
    }

    bool undo() {
        if (!fUndo) {
            return false;
        }
        fPath = fUndo->fPath;
        validatePath();
        PathUndo* next = fUndo->fNext;
        delete fUndo;
        fUndo = next;
        return true;
    }

    void validatePath() {
        PathUndo* undo = fUndo;
        int match = 0;
        while (undo) {
            match += fPath == undo->fPath;
            undo = undo->fNext;
        }
    }

    void set_controlList(int index, UniControl* control, MyClick::ControlType type) {
        kControlList[index].fControl = control;
        kControlList[index].fControlType = type;
    }

    #define SET_CONTROL(Name) set_controlList(index++, &f##Name##Control, \
        MyClick::k##Name##Control)

    bool hideAll() {
        fHideAll ^= true;
        return true;
    }

    void init_controlList() {
        int index = 0;
        SET_CONTROL(Width);
        SET_CONTROL(Res);
        SET_CONTROL(Filter);
        SET_CONTROL(Weight);
    }

    #undef SET_CONTROL

    void set_buttonList(int index, Button* button, MyClick::ControlType type) {
        kButtonList[index].fButton = button;
        kButtonList[index].fButtonType = type;
    }

    #define SET_BUTTON(Name) set_buttonList(index++, &f##Name##Button, \
            MyClick::k##Name##Button)

    void init_buttonList() {
        int index = 0;
        SET_BUTTON(Fill);
        SET_BUTTON(Skeleton);
        SET_BUTTON(Filter);
        SET_BUTTON(Bisect);
        SET_BUTTON(Join);
        SET_BUTTON(InOut);
        SET_BUTTON(Cubic);
        SET_BUTTON(Conic);
        SET_BUTTON(Quad);
        SET_BUTTON(Line);
        SET_BUTTON(Add);
        SET_BUTTON(Delete);
    }

    #undef SET_BUTTON

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override;

    void onSizeChange() override {
        setControlButtonsPos();
        this->INHERITED::onSizeChange();
    }

    bool pathDump() {
        fPath.dump();
        return true;
    }

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
        SkScalar scale = SkTMin(this->width() / bounds.width(), this->height() / bounds.height())
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

    void draw_bisect(SkCanvas* canvas, const CurveData* curve) {
        const CurveData* prev = curve->fPrev;
        if (!prev) {
            return;
        }
        const StrokeData& innerPrev = prev->fInner;
        const StrokeData& outerPrev = prev->fOuter;
        const SkPoint& pt = curve->lastPt();
        if (fBisectButton.enabled()) {
            canvas->drawLine(pt, curve->fInner.fBisect, fSkeletonPaint);
            canvas->drawLine(innerPrev.fBisect, curve->fInner.fBisect, fDebugSkeletonPaint);
            canvas->drawLine(pt, curve->fOuter.fBisect, fSkeletonPaint);
            canvas->drawLine(outerPrev.fBisect, curve->fOuter.fBisect, fSkeletonPaint);
        }
        const SkVector& lastV = curve->fPrev->fTangents[1];
        const SkVector& nextV = curve->fTangents[0];
        if (fBisectButton.enabled()) {
            canvas->drawLine(pt, {pt.fX - lastV.fY, pt.fY + lastV.fX}, fSkeletonPaint);
            canvas->drawLine(pt, {pt.fX + nextV.fY, pt.fY - nextV.fX}, fSkeletonPaint);
        }
        if (fJoinButton.enabled()) {
            SkScalar r = fWidthControl.fValLo;
            SkRect oval = { pt.fX - r, pt.fY - r, pt.fX + r, pt.fY + r};
            SkScalar startAngle = SkScalarATan2(lastV.fX, -lastV.fY) * 180.f / SK_ScalarPI;
            SkScalar endAngle = SkScalarATan2(-nextV.fX, nextV.fY) * 180.f / SK_ScalarPI;
            if (endAngle > startAngle) {
                canvas->drawArc(oval, startAngle, endAngle - startAngle, false, fSkeletonPaint);
            } else {
                canvas->drawArc(oval, startAngle, 360 - (startAngle - endAngle), false,
                        fSkeletonPaint);
            }
            canvas->drawLine(curve->fOuter.fBisect.fX, curve->fOuter.fBisect.fY,
                pt.fX - lastV.fY, pt.fY + lastV.fX, fSkeletonPaint);
            canvas->drawLine(curve->fOuter.fBisect.fX, curve->fOuter.fBisect.fY,
                pt.fX + nextV.fY, pt.fY - nextV.fX, fSkeletonPaint);
        }
    }

    void draw_bisects(SkCanvas* canvas, bool activeOnly) {
        int counter = -1;
        for (int ptIndex = 0; ptIndex < fStrokePath.fCurveData.count(); ++ptIndex) {
            CurveData* curPtData = &fStrokePath.fCurveData[ptIndex];
            ++counter;
            if (activeOnly && counter != fActiveVerb && counter - 1 != fActiveVerb
                    && counter + 1 != fActiveVerb
                    && (fActiveVerb != 1 || counter != fPath.countVerbs())) {
                continue;
            }
            if (!fBisectButton.enabled()) {
                continue;
            }
            canvas->drawPath(curPtData->fInner.fPath, fSkeletonPaint);
            canvas->drawPath(curPtData->fOuter.fPath, fSkeletonPaint);
            if (SkPath::kQuad_Verb == curPtData->fVerb) {
                const SkPoint* pts = curPtData->fPts;
                SkScalar t = SkFindQuadMaxCurvature(pts);
                if (0 < t && t < 1) {
                    SkPoint maxPt = SkEvalQuadAt(pts, t);
                    SkVector tangent = SkEvalQuadTangentAt(pts, t);
                    tangent.setLength(fWidthControl.fValLo);
                    canvas->drawLine(maxPt, {maxPt.fX + tangent.fY, maxPt.fY - tangent.fX},
                                        fSkeletonPaint);
                    canvas->drawLine(maxPt, {maxPt.fX - tangent.fY, maxPt.fY + tangent.fX},
                                        fSkeletonPaint);
                }
            } else if (SkPath::kConic_Verb == curPtData->fVerb) {
                ;
                // FIXME : need max curvature or equivalent here ?
            } else if (SkPath::kCubic_Verb == curPtData->fVerb) {
                SkScalar tMax[2];
                int tMaxCount = SkFindCubicMaxCurvature(curPtData->fPts, tMax);
                for (int tIndex = 0; tIndex < tMaxCount; ++tIndex) {
                    if (0 >= tMax[tIndex] || tMax[tIndex] >= 1) {
                        continue;
                    }
                    SkPoint maxPt;
                    SkVector tangent;
                    SkEvalCubicAt(curPtData->fPts, tMax[tIndex], &maxPt, &tangent,
                            nullptr);
                    tangent.setLength(fWidthControl.fValLo * 2);
                    canvas->drawLine(maxPt, {maxPt.fX + tangent.fY, maxPt.fY - tangent.fX},
                            fSkeletonPaint);
                }
            }
            if (SkPath::kLine_Verb == curPtData->fPrev->fVerb &&
                    SkPath::kLine_Verb == curPtData->fVerb) {
                draw_bisect(canvas, curPtData);
            }
        }
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
                    SkScalar loopT[3];
                    int complex = SkDCubic::ComplexBreak(pts, loopT);
                    SkPath cPath;
                    cPath.moveTo(pts[0]);
                    cPath.cubicTo(pts[1], pts[2], pts[3]);
                    canvas->drawPath(cPath, complex ? fComplexPaint : fActivePaint);
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

    SkScalar pt_to_line(SkPoint s, SkPoint e, int x, int y) {
        SkScalar radius = fWidthControl.fValLo;
        SkVector adjOpp = e - s;
        SkScalar lenSq = SkPointPriv::LengthSqd(adjOpp);
        SkPoint rotated = {
                (y - s.fY) * adjOpp.fY + (x - s.fX) * adjOpp.fX,
                (y - s.fY) * adjOpp.fX - (x - s.fX) * adjOpp.fY,
        };
        if (rotated.fX < 0 || rotated.fX > lenSq) {
                return -radius;
        }
        rotated.fY /= SkScalarSqrt(lenSq);
        return SkTMax(-radius, SkTMin(radius, rotated.fY));
    }

    // given a line, compute the interior and exterior gradient coverage
    bool coverage(SkPoint s, SkPoint e, uint8_t* distanceMap, int w, int h) {
        SkScalar radius = fWidthControl.fValLo;
        int minX = SkTMax(0, (int) (SkTMin(s.fX, e.fX) - radius));
        int minY = SkTMax(0, (int) (SkTMin(s.fY, e.fY) - radius));
        int maxX = SkTMin(w, (int) (SkTMax(s.fX, e.fX) + radius) + 1);
        int maxY = SkTMin(h, (int) (SkTMax(s.fY, e.fY) + radius) + 1);
        for (int y = minY; y < maxY; ++y) {
            for (int x = minX; x < maxX; ++x) {
                SkScalar ptToLineDist = pt_to_line(s, e, x, y);
                if (ptToLineDist > -radius && ptToLineDist < radius) {
                    SkScalar coverage = ptToLineDist / radius;
                    add_to_map(1 - SkScalarAbs(coverage), x, y, distanceMap, w, h);
                }
                SkVector ptToS = { x - s.fX, y - s.fY };
                SkScalar dist = ptToS.length();
                if (dist < radius) {
                    SkScalar coverage = dist / radius;
                    add_to_map(1 - SkScalarAbs(coverage), x, y, distanceMap, w, h);
                }
                SkVector ptToE = { x - e.fX, y - e.fY };
                dist = ptToE.length();
                if (dist < radius) {
                    SkScalar coverage = dist / radius;
                    add_to_map(1 - SkScalarAbs(coverage), x, y, distanceMap, w, h);
                }
            }
        }
        return true;
    }

    void quad_coverage(SkPoint pts[3], uint8_t* distanceMap, int w, int h) {
        SkScalar dist = pts[0].Distance(pts[0], pts[2]);
        if (dist < gCurveDistance) {
            (void) coverage(pts[0], pts[2], distanceMap, w, h);
            return;
        }
        SkPoint split[5];
        SkChopQuadAt(pts, split, 0.5f);
        quad_coverage(&split[0], distanceMap, w, h);
        quad_coverage(&split[2], distanceMap, w, h);
    }

    void conic_coverage(SkPoint pts[3], SkScalar weight, uint8_t* distanceMap, int w, int h) {
        SkScalar dist = pts[0].Distance(pts[0], pts[2]);
        if (dist < gCurveDistance) {
            (void) coverage(pts[0], pts[2], distanceMap, w, h);
            return;
        }
        SkConic split[2];
        SkConic conic;
        conic.set(pts, weight);
        if (conic.chopAt(0.5f, split)) {
            conic_coverage(split[0].fPts, split[0].fW, distanceMap, w, h);
            conic_coverage(split[1].fPts, split[1].fW, distanceMap, w, h);
        }
    }

    void cubic_coverage(SkPoint pts[4], uint8_t* distanceMap, int w, int h) {
        SkScalar dist = pts[0].Distance(pts[0], pts[3]);
        if (dist < gCurveDistance) {
            (void) coverage(pts[0], pts[3], distanceMap, w, h);
            return;
        }
        SkPoint split[7];
        SkChopCubicAt(pts, split, 0.5f);
        cubic_coverage(&split[0], distanceMap, w, h);
        cubic_coverage(&split[3], distanceMap, w, h);
    }

    void path_coverage(const SkPath& path, uint8_t* distanceMap, int w, int h) {
        memset(distanceMap, 0, sizeof(distanceMap[0]) * w * h);
        SkPoint pts[4];
        SkPath::Verb verb;
        SkPath::Iter iter(path, true);
        while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kLine_Verb:
                    (void) coverage(pts[0], pts[1], distanceMap, w, h);
                    break;
                case SkPath::kQuad_Verb:
                    quad_coverage(pts, distanceMap, w, h);
                    break;
                case SkPath::kConic_Verb:
                    conic_coverage(pts, iter.conicWeight(), distanceMap, w, h);
                    break;
                case SkPath::kCubic_Verb:
                    cubic_coverage(pts, distanceMap, w, h);
                    break;
                default:
                    break;
            }
        }
    }

    static uint8_t* set_up_dist_map(const SkImageInfo& imageInfo, SkBitmap* distMap) {
        distMap->setInfo(imageInfo);
        distMap->setIsVolatile(true);
        SkAssertResult(distMap->tryAllocPixels());
        SkASSERT((int) distMap->rowBytes() == imageInfo.width());
        return distMap->getAddr8(0, 0);
    }

    void draw_coverage(SkCanvas* canvas) {
        SkRect bounds = fPath.getBounds();
        SkScalar radius = fWidthControl.fValLo;
        int w = (int) (bounds.fRight + radius + 1);
        int h = (int) (bounds.fBottom + radius + 1);
        SkImageInfo imageInfo = SkImageInfo::MakeA8(w, h);
        SkBitmap distMap;
        uint8_t* distanceMap = set_up_dist_map(imageInfo, &distMap);
        path_coverage(fPath, distanceMap, w, h);
        if (fFillButton.enabled()) {
            canvas->drawPath(fPath, fCoveragePaint);
        }
        if (fFilterButton.fState == 2
                && (0 < fFilterControl.fValLo || fFilterControl.fValHi < 255)) {
            SkBitmap filteredMap;
            uint8_t* filtered = set_up_dist_map(imageInfo, &filteredMap);
            filter_coverage(distanceMap, sizeof(uint8_t) * w * h, (uint8_t) fFilterControl.fValLo,
                    (uint8_t) fFilterControl.fValHi, filtered);
            canvas->drawBitmap(filteredMap, 0, 0, &fCoveragePaint);
        } else if (fFilterButton.enabled()) {
            canvas->drawBitmap(distMap, 0, 0, &fCoveragePaint);
        }
    }

    void draw_ball(SkCanvas* canvas) {
        // get dash positions along path
        // for each color in gradient
        // draw ball at the desired diameter and translucency to its own bitmap
        // keep max value
        // compute partial pixel coverage as well
        SkPath fWidth;
        fWidth.moveTo(0, .3f);
        fWidth.lineTo(1, .3f);
        fWidth.quadTo(1.5f, .6f, 2, .3f);
        fWidth.quadTo(2.5f, .8f, 3, .4f);
        fWidth.quadTo(3.5f, .9f, 4, .3f);
        fWidth.lineTo(5, .3f);
        SkPathMeasure measure(fPath, false);
        SkPathMeasure wMeasure(fWidth, false);
        SkScalar length = measure.getLength();
        SkScalar wLength = wMeasure.getLength();
        SkPaint paint(fCoveragePaint);
        for (SkScalar interval = 0; interval < length; interval += fBallInterval) {
            SkPoint position;
            if (!measure.getPosTan(interval, &position, nullptr)) {
                break;
            }
            SkPoint wPos;
            wMeasure.getPosTan(interval * wLength / length, &wPos, nullptr);
            int r = (int) (255 * interval * 4 / length) % 510;
            if (r > 255) r = 510 - r;
            int g = (int) (255 * wPos.fY * 6) % 510;
            if (g > 255) g = 510 - g;
            paint.setARGB(0xFF,  SkTMin(255, r), SkTMin(255, g), 0);
            canvas->drawCircle(position, fWidthControl.fValLo * wPos.fY, paint);
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
#if 0
        SkDEBUGCODE(SkDebugStrokeGlobals debugGlobals);
        SkOpAA aaResult(fPath, fWidthControl.fValLo, fResControl.fValLo
                SkDEBUGPARAMS(&debugGlobals));
#endif
        fStrokePath.setPointArray(fPath, fWidthControl.fValLo);
        fStrokePath.calcBisects(fWidthControl.fValLo);
        if (fFilterButton.fState == 1 || fFillButton.fState == 2) {
            draw_coverage(canvas);
        } else if (fFilterButton.fState == 3) {
            draw_ball(canvas);
        }
        if (fSkeletonButton.enabled()) {
            canvas->drawPath(fPath, fActiveVerb >= 0 ? fLightSkeletonPaint : fSkeletonPaint);
        }
        if (fActiveVerb >= 0) {
            draw_segment(canvas);
        }
        if (fBisectButton.enabled() || fJoinButton.enabled()) {
            draw_bisects(canvas, fActiveVerb >= 0);
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

#if 0
        SkPaint paint;
        paint.setARGB(255, 34, 31, 31);
        paint.setAntiAlias(true);

        SkPath path;
        path.moveTo(18,439);
        path.lineTo(414,439);
        path.lineTo(414,702);
        path.lineTo(18,702);
        path.lineTo(18,439);

        path.moveTo(19,701);
        path.lineTo(413,701);
        path.lineTo(413,440);
        path.lineTo(19,440);
        path.lineTo(19,701);
        path.close();
        canvas->drawPath(path, paint);

        canvas->scale(1.0f, -1.0f);
        canvas->translate(0.0f, -800.0f);
        canvas->drawPath(path, paint);
#endif

    }

    int hittest_pt(SkPoint pt) {
        for (int index = 0; index < fPath.countPoints(); ++index) {
            if (SkPoint::Distance(fPath.getPoint(index), pt) <= kHitToleranace * 2) {
                return index;
            }
        }
        return -1;
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        SkPoint pt = {x, y};
        int ptHit = hittest_pt(pt);
        if (ptHit >= 0) {
            return new MyClick(this, MyClick::kPtType, ptHit);
        }
        SkPath::Verb verb;
        SkScalar weight;
        int verbHit = hittest_verb(pt, &verb, &weight);
        if (verbHit >= 0) {
            return new MyClick(this, MyClick::kVerbType, verbHit, verb, weight);
        }
        if (!fHideAll) {
            const SkRect& rectPt = SkRect::MakeXYWH(x, y, 1, 1);
            for (int index = 0; index < kControlCount; ++index) {
                if (kControlList[index].fControl->contains(rectPt)) {
                    return new MyClick(this, MyClick::kControlType,
                            kControlList[index].fControlType);
                }
            }
            for (int index = 0; index < kButtonCount; ++index) {
                if (kButtonList[index].fButton->contains(rectPt)) {
                    return new MyClick(this, MyClick::kControlType, kButtonList[index].fButtonType);
                }
            }
        }
        fLineButton.fVisible = fQuadButton.fVisible = fConicButton.fVisible
                = fCubicButton.fVisible = fWeightControl.fVisible = fAddButton.fVisible
                = fDeleteButton.fVisible = false;
        fActiveVerb = -1;
        fActivePt = -1;
        if (fHandlePathMove) {
            return new MyClick(this, MyClick::kPathType, MyClick::kPathMove);
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    static SkScalar MapScreenYtoValue(int y, const UniControl& control) {
        return SkTMin(1.f, SkTMax(0.f,
                SkIntToScalar(y) - control.fBounds.fTop) / control.fBounds.height())
                * (control.fMax - control.fMin) + control.fMin;
    }

    bool onClick(Click* click) override {
        MyClick* myClick = (MyClick*) click;
        switch (myClick->fType) {
            case MyClick::kPtType: {
                savePath(click->fState);
                fActivePt = myClick->ptHit();
                SkPoint pt = fPath.getPoint((int) myClick->fControl);
                pt.offset(SkIntToScalar(click->fICurr.fX - click->fIPrev.fX),
                        SkIntToScalar(click->fICurr.fY - click->fIPrev.fY));
                set_path_pt(fActivePt, pt, &fPath);
                validatePath();
                return true;
                }
            case MyClick::kPathType:
                savePath(click->fState);
                fPath.offset(SkIntToScalar(click->fICurr.fX - click->fIPrev.fX),
                        SkIntToScalar(click->fICurr.fY - click->fIPrev.fY));
                validatePath();
                return true;
            case MyClick::kVerbType: {
                fActiveVerb = myClick->verbHit();
                fLineButton.fVisible = fQuadButton.fVisible = fConicButton.fVisible
                        = fCubicButton.fVisible = fAddButton.fVisible = fDeleteButton.fVisible
                        = true;
                fLineButton.setEnabled(myClick->fVerb == SkPath::kLine_Verb);
                fQuadButton.setEnabled(myClick->fVerb == SkPath::kQuad_Verb);
                fConicButton.setEnabled(myClick->fVerb == SkPath::kConic_Verb);
                fCubicButton.setEnabled(myClick->fVerb == SkPath::kCubic_Verb);
                fWeightControl.fValLo = myClick->fWeight;
                fWeightControl.fVisible = myClick->fVerb == SkPath::kConic_Verb;
                } break;
            case MyClick::kControlType: {
                if (click->fState != Click::kDown_State && myClick->isButton()) {
                    return true;
                }
                switch (myClick->fControl) {
                    case MyClick::kFilterControl: {
                        SkScalar val = MapScreenYtoValue(click->fICurr.fY, fFilterControl);
                        if (val - fFilterControl.fValLo < fFilterControl.fValHi - val) {
                            fFilterControl.fValLo = SkTMax(0.f, val);
                        } else {
                            fFilterControl.fValHi = SkTMin(255.f, val);
                        }
                        } break;
                    case MyClick::kResControl:
                        fResControl.fValLo = MapScreenYtoValue(click->fICurr.fY, fResControl);
                        break;
                    case MyClick::kWeightControl: {
                        savePath(click->fState);
                        SkScalar w = MapScreenYtoValue(click->fICurr.fY, fWeightControl);
                        set_path_weight(fActiveVerb, w, &fPath);
                        validatePath();
                        fWeightControl.fValLo = w;
                        } break;
                    case MyClick::kWidthControl:
                        fWidthControl.fValLo = MapScreenYtoValue(click->fICurr.fY, fWidthControl);
                        break;
                    case MyClick::kLineButton:
                        savePath(click->fState);
                        enable_verb_button(myClick->fControl);
                        fWeightControl.fVisible = false;
                        set_path_verb(fActiveVerb, SkPath::kLine_Verb, &fPath, 1);
                        validatePath();
                        break;
                    case MyClick::kQuadButton:
                        savePath(click->fState);
                        enable_verb_button(myClick->fControl);
                        fWeightControl.fVisible = false;
                        set_path_verb(fActiveVerb, SkPath::kQuad_Verb, &fPath, 1);
                        validatePath();
                        break;
                    case MyClick::kConicButton: {
                        savePath(click->fState);
                        enable_verb_button(myClick->fControl);
                        fWeightControl.fVisible = true;
                        const SkScalar defaultConicWeight = 1.f / SkScalarSqrt(2);
                        set_path_verb(fActiveVerb, SkPath::kConic_Verb, &fPath, defaultConicWeight);
                        validatePath();
                        fWeightControl.fValLo = get_path_weight(fActiveVerb, fPath);
                        } break;
                    case MyClick::kCubicButton:
                        savePath(click->fState);
                        enable_verb_button(myClick->fControl);
                        fWeightControl.fVisible = false;
                        set_path_verb(fActiveVerb, SkPath::kCubic_Verb, &fPath, 1);
                        validatePath();
                        break;
                    case MyClick::kAddButton:
                        savePath(click->fState);
                        add_path_segment(fActiveVerb, &fPath);
                        validatePath();
                        if (fWeightControl.fVisible) {
                            fWeightControl.fValLo = get_path_weight(fActiveVerb, fPath);
                        }
                        break;
                    case MyClick::kDeleteButton:
                        savePath(click->fState);
                        delete_path_segment(fActiveVerb, &fPath);
                        validatePath();
                        break;
                    case MyClick::kFillButton:
                        fFillButton.toggle();
                        break;
                    case MyClick::kSkeletonButton:
                        fSkeletonButton.toggle();
                        break;
                    case MyClick::kFilterButton:
                        fFilterButton.toggle();
                        fFilterControl.fVisible = fFilterButton.fState == 2;
                        break;
                    case MyClick::kBisectButton:
                        fBisectButton.toggle();
                        break;
                    case MyClick::kJoinButton:
                        fJoinButton.toggle();
                        break;
                    case MyClick::kInOutButton:
                        fInOutButton.toggle();
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
    typedef SampleView INHERITED;
};

static struct KeyCommand {
    char fKey;
    char fAlternate;
    const char* fDescriptionL;
    const char* fDescriptionR;
    bool (AAGeometryView::*fFunction)();
} kKeyCommandList[] = {
    { ' ',  0,  "space",   "center path", &AAGeometryView::scaleToFit },
    { '-',  0,  "-",          "zoom out", &AAGeometryView::scaleDown },
    { '+', '=', "+/=",         "zoom in", &AAGeometryView::scaleUp },
    { 'D',  0,  "D",   "dump to console", &AAGeometryView::pathDump },
    { 'H',  0,  "H",     "hide controls", &AAGeometryView::hideAll },
    { 'R',  0,  "R",        "reset path", &AAGeometryView::constructPath },
    { 'Z',  0,  "Z",              "undo", &AAGeometryView::undo },
    { '?',  0,  "?",       "show legend", &AAGeometryView::showLegend },
};

const int kKeyCommandCount = (int) SK_ARRAY_COUNT(kKeyCommandList);

void AAGeometryView::draw_legend(SkCanvas* canvas) {
    SkScalar bottomOffset = this->height() - 10;
    for (int index = kKeyCommandCount - 1; index >= 0; --index) {
        bottomOffset -= 15;
        canvas->drawString(kKeyCommandList[index].fDescriptionL,
                this->width() - 160, bottomOffset,
                fLegendLeftPaint);
        canvas->drawString(kKeyCommandList[index].fDescriptionR,
                this->width() - 20, bottomOffset,
                fLegendRightPaint);
    }
}

// overrides from SkEventSink
bool AAGeometryView::onQuery(SkEvent* evt) {
    if (SampleCode::TitleQ(*evt)) {
        SampleCode::TitleR(evt, "AAGeometry");
        return true;
    }
    SkUnichar uni;
    if (false) {
        return this->INHERITED::onQuery(evt);
    }
    if (SampleCode::CharQ(*evt, &uni)) {
        for (int index = 0; index < kButtonCount; ++index) {
            Button* button = kButtonList[index].fButton;
            if (button->fVisible && uni == button->fLabel) {
                MyClick click(this, MyClick::kControlType, kButtonList[index].fButtonType);
                click.fState = Click::kDown_State;
                (void) this->onClick(&click);
                return true;
            }
        }
        for (int index = 0; index < kKeyCommandCount; ++index) {
            KeyCommand& keyCommand = kKeyCommandList[index];
            if (uni == keyCommand.fKey || uni == keyCommand.fAlternate) {
                return (this->*keyCommand.fFunction)();
            }
        }
        if (('A' <= uni && uni <= 'Z') || ('a' <= uni && uni <= 'z')) {
            for (int index = 0; index < kButtonCount; ++index) {
                Button* button = kButtonList[index].fButton;
                if (button->fVisible && (uni & ~0x20) == (button->fLabel & ~0x20)) {
                    MyClick click(this, MyClick::kControlType, kButtonList[index].fButtonType);
                    click.fState = Click::kDown_State;
                    (void) this->onClick(&click);
                    return true;
                }
            }
        }
    }
    return this->INHERITED::onQuery(evt);
}

DEF_SAMPLE( return new AAGeometryView; )
