/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkMatrix.h"
#include "SkColor.h"
#include "SkTDArray.h"
#include "SkRandom.h"
#include "SkRRect.h"

enum RandomAddPath {
    kMoveToPath,
    kRMoveToPath,
    kLineToPath,
    kRLineToPath,
    kQuadToPath,
    kRQuadToPath,
    kConicToPath,
    kRConicToPath,
    kCubicToPath,
    kRCubicToPath,
    kArcToPath,
    kArcTo2Path,
    kClosePath,
    kAddArc,
    kAddRoundRect1,
    kAddRoundRect2,
    kAddRRect,
    kAddPoly,
    kAddPath1,
    kAddPath2,
    kAddPath3,
    kReverseAddPath,
};

const int kRandomAddPath_Last = kReverseAddPath;

const char* gRandomAddPathNames[] = {
    "kMoveToPath",
    "kRMoveToPath",
    "kLineToPath",
    "kRLineToPath",
    "kQuadToPath",
    "kRQuadToPath",
    "kConicToPath",
    "kRConicToPath",
    "kCubicToPath",
    "kRCubicToPath",
    "kArcToPath",
    "kArcTo2Path",
    "kClosePath",
    "kAddArc",
    "kAddRoundRect1",
    "kAddRoundRect2",
    "kAddRRect",
    "kAddPoly",
    "kAddPath1",
    "kAddPath2",
    "kAddPath3",
    "kReverseAddPath",
};

enum RandomSetRRect {
    kSetEmpty,
    kSetRect,
    kSetOval,
    kSetRectXY,
    kSetNinePatch,
    kSetRectRadii,
};

const char* gRandomSetRRectNames[] = {
    "kSetEmpty",
    "kSetRect",
    "kSetOval",
    "kSetRectXY",
    "kSetNinePatch",
    "kSetRectRadii",
};

int kRandomSetRRect_Last = kSetRectRadii;

enum RandomSetMatrix {
    kSetIdentity,
    kSetTranslate,
    kSetTranslateX,
    kSetTranslateY,
    kSetScale,
    kSetScaleTranslate,
    kSetScaleX,
    kSetScaleY,
    kSetSkew,
    kSetSkewTranslate,
    kSetSkewX,
    kSetSkewY,
    kSetRotate,
    kSetRotateTranslate,
    kSetPerspectiveX,
    kSetPerspectiveY,
    kSetAll,
};

int kRandomSetMatrix_Last = kSetAll;

const char* gRandomSetMatrixNames[] = {
    "kSetIdentity",
    "kSetTranslate",
    "kSetTranslateX",
    "kSetTranslateY",
    "kSetScale",
    "kSetScaleTranslate",
    "kSetScaleX",
    "kSetScaleY",
    "kSetSkew",
    "kSetSkewTranslate",
    "kSetSkewX",
    "kSetSkewY",
    "kSetRotate",
    "kSetRotateTranslate",
    "kSetPerspectiveX",
    "kSetPerspectiveY",
    "kSetAll",
};

class FuzzPath {
public:
    FuzzPath()
        : fFloatMin(0)
        , fFloatMax(800)
        , fAddCount(0)
        , fPrintName(false)
        , fStrokeOnly(false)
        , fValidate(false)
    {
        fTab = "                                                                                  ";
    }
    void randomize() {
        fPathDepth = 0;
        fPathDepthLimit = fRand.nextRangeU(1, 2);
        fPathContourCount = fRand.nextRangeU(1, 4);
        fPathSegmentLimit = fRand.nextRangeU(1, 8);
        fClip = makePath();
        SkASSERT(!fPathDepth);
        fMatrix = makeMatrix();
        fPaint = makePaint();
        fPathDepthLimit = fRand.nextRangeU(1, 3);
        fPathContourCount = fRand.nextRangeU(1, 6);
        fPathSegmentLimit = fRand.nextRangeU(1, 16);
        fPath = makePath();
        SkASSERT(!fPathDepth);
    }

    const SkPath& getClip() const {
        return fClip;
    }

    const SkMatrix& getMatrix() const {
        return fMatrix;
    }

    const SkPaint& getPaint() const {
        return fPaint;
    }

    const SkPath& getPath() const {
        return fPath;
    }

    void setSeed(int seed) {
        fRand.setSeed(seed);
    }

    void setStrokeOnly() {
        fStrokeOnly = true;
    }

private:

SkPath::AddPathMode makeAddPathMode() {
    return (SkPath::AddPathMode) fRand.nextRangeU(SkPath::kAppend_AddPathMode,
        SkPath::kExtend_AddPathMode);
}

RandomAddPath makeAddPathType() {
    return (RandomAddPath) fRand.nextRangeU(0, kRandomAddPath_Last);
}

SkScalar makeAngle() {
    SkScalar angle;
    angle = fRand.nextF();
    return angle;
}

bool makeBool() {
    return fRand.nextBool();
}

SkPath::Direction makeDirection() {
    return (SkPath::Direction) fRand.nextRangeU(SkPath::kCW_Direction, SkPath::kCCW_Direction);
}

SkMatrix makeMatrix() {
    SkMatrix matrix;
    matrix.reset();
    RandomSetMatrix setMatrix = (RandomSetMatrix) fRand.nextRangeU(0, kRandomSetMatrix_Last);
    if (fPrintName) {
        SkDebugf("%.*s%s\n", fPathDepth * 3, fTab, gRandomSetMatrixNames[setMatrix]);
    }
    switch (setMatrix) {
        case kSetIdentity:
            break;
        case kSetTranslateX:
            matrix.setTranslateX(makeScalar());
            break;
        case kSetTranslateY:
            matrix.setTranslateY(makeScalar());
            break;
        case kSetTranslate:
            matrix.setTranslate(makeScalar(), makeScalar());
            break;
        case kSetScaleX:
            matrix.setScaleX(makeScalar());
            break;
        case kSetScaleY:
            matrix.setScaleY(makeScalar());
            break;
        case kSetScale:
            matrix.setScale(makeScalar(), makeScalar());
            break;
        case kSetScaleTranslate:
            matrix.setScale(makeScalar(), makeScalar(), makeScalar(), makeScalar());
            break;
        case kSetSkewX:
            matrix.setSkewX(makeScalar());
            break;
        case kSetSkewY:
            matrix.setSkewY(makeScalar());
            break;
        case kSetSkew:
            matrix.setSkew(makeScalar(), makeScalar());
            break;
        case kSetSkewTranslate:
            matrix.setSkew(makeScalar(), makeScalar(), makeScalar(), makeScalar());
            break;
        case kSetRotate:
            matrix.setRotate(makeScalar());
            break;
        case kSetRotateTranslate:
            matrix.setRotate(makeScalar(), makeScalar(), makeScalar());
            break;
        case kSetPerspectiveX:
            matrix.setPerspX(makeScalar());
            break;
        case kSetPerspectiveY:
            matrix.setPerspY(makeScalar());
            break;
        case kSetAll:
            matrix.setAll(makeScalar(), makeScalar(), makeScalar(),
                          makeScalar(), makeScalar(), makeScalar(),
                          makeScalar(), makeScalar(), makeScalar());
            break;
    }
    return matrix;
}

SkPaint makePaint() {
    SkPaint paint;
    bool antiAlias = fRand.nextBool();
    paint.setAntiAlias(antiAlias);
    SkPaint::Style style = fStrokeOnly ? SkPaint::kStroke_Style :
        (SkPaint::Style) fRand.nextRangeU(SkPaint::kFill_Style, SkPaint::kStrokeAndFill_Style);
    paint.setStyle(style);
    SkColor color = (SkColor) fRand.nextU();
    paint.setColor(color);
    SkScalar width = fRand.nextRangeF(0, 10);
    paint.setStrokeWidth(width);
    SkScalar miter = makeScalar();
    paint.setStrokeMiter(miter);
    SkPaint::Cap cap = (SkPaint::Cap) fRand.nextRangeU(SkPaint::kButt_Cap, SkPaint::kSquare_Cap);
    paint.setStrokeCap(cap);
    SkPaint::Join join = (SkPaint::Join) fRand.nextRangeU(SkPaint::kMiter_Join,
        SkPaint::kBevel_Join);
    paint.setStrokeJoin(join);
    return paint;
}

SkPoint makePoint() {
    SkPoint result;
    makeScalarArray(2, &result.fX);
    return result;
}

void makePointArray(size_t arrayCount, SkPoint* points) {
    for (size_t index = 0; index < arrayCount; ++index) {
        points[index] = makePoint();
    }
}

void makePointArray(SkTDArray<SkPoint>* points) {
    size_t arrayCount = fRand.nextRangeU(1, 10);
    for (size_t index = 0; index < arrayCount; ++index) {
        *points->append() = makePoint();
    }
}

SkRect makeRect() {
    SkRect result;
    makeScalarArray(4, &result.fLeft);
    return result;
}

SkRRect makeRRect() {
    SkRRect rrect;
    RandomSetRRect rrectType = makeSetRRectType();
    if (fPrintName) {
        SkDebugf("%.*s%s\n", fPathDepth * 3, fTab, gRandomSetRRectNames[rrectType]);
    }
    switch (rrectType) {
        case kSetEmpty:
            rrect.setEmpty();
            break;
        case kSetRect: {
            SkRect rect = makeRect();
            rrect.setRect(rect);
            } break;
        case kSetOval: {
            SkRect oval = makeRect();
            rrect.setOval(oval);
            } break;
        case kSetRectXY: {
            SkRect rect = makeRect();
            SkScalar xRad = makeScalar();
            SkScalar yRad = makeScalar();
            rrect.setRectXY(rect, xRad, yRad);
            } break;
        case kSetNinePatch: {
            SkRect rect = makeRect();
            SkScalar leftRad = makeScalar();
            SkScalar topRad = makeScalar();
            SkScalar rightRad = makeScalar();
            SkScalar bottomRad = makeScalar();
            rrect.setNinePatch(rect, leftRad, topRad, rightRad, bottomRad);
            SkDebugf("");  // keep locals in scope
            } break;
        case kSetRectRadii: {
            SkRect rect = makeRect();
            SkVector radii[4];
            makeVectorArray(SK_ARRAY_COUNT(radii), radii);
            rrect.setRectRadii(rect, radii);
            } break;
    }
    return rrect;
}

SkPath makePath() {
    SkPath path;
    for (uint32_t cIndex = 0; cIndex < fPathContourCount; ++cIndex) {
        uint32_t segments = makeSegmentCount();
        for (uint32_t sIndex = 0; sIndex < segments; ++sIndex) {
            RandomAddPath addPathType = makeAddPathType();
            ++fAddCount;
            if (fPrintName) {
                SkDebugf("%.*s%s\n", fPathDepth * 3, fTab,
                        gRandomAddPathNames[addPathType]);
            }
            switch (addPathType) {
                case kAddArc: {
                    SkRect oval = makeRect();
                    SkScalar startAngle = makeAngle();
                    SkScalar sweepAngle = makeAngle();
                    path.addArc(oval, startAngle, sweepAngle);
                    validate(path);
                    } break;
                case kAddRoundRect1: {
                    SkRect rect = makeRect();
                    SkScalar rx = makeScalar(), ry = makeScalar();
                    SkPath::Direction dir = makeDirection();
                    path.addRoundRect(rect, rx, ry, dir);
                    validate(path);
                    } break;
                case kAddRoundRect2: {
                    SkRect rect = makeRect();
                    SkScalar radii[8];
                    makeScalarArray(SK_ARRAY_COUNT(radii), radii);
                    SkPath::Direction dir = makeDirection();
                    path.addRoundRect(rect, radii, dir);
                    validate(path);
                    } break;
                case kAddRRect: {
                    SkRRect rrect = makeRRect();
                    SkPath::Direction dir = makeDirection();
                    path.addRRect(rrect, dir);
                    validate(path);
                    } break;
                case kAddPoly: {
                    SkTDArray<SkPoint> points;
                    makePointArray(&points);
                    bool close = makeBool();
                    path.addPoly(&points[0], points.count(), close);
                    validate(path);
                    } break;
                case kAddPath1:
                    if (fPathDepth < fPathDepthLimit) {
                        ++fPathDepth;
                        SkPath src = makePath();
                        validate(src);
                        SkScalar dx = makeScalar();
                        SkScalar dy = makeScalar();
                        SkPath::AddPathMode mode = makeAddPathMode();
                        path.addPath(src, dx, dy, mode);
                        --fPathDepth;
                        validate(path);
                    }
                    break;
                case kAddPath2:
                    if (fPathDepth < fPathDepthLimit) {
                        ++fPathDepth;
                        SkPath src = makePath();
                        validate(src);
                        SkPath::AddPathMode mode = makeAddPathMode();
                        path.addPath(src, mode);
                        --fPathDepth;
                        validate(path);
                    }
                    break;
                case kAddPath3:
                    if (fPathDepth < fPathDepthLimit) {
                        ++fPathDepth;
                        SkPath src = makePath();
                        validate(src);
                        SkMatrix matrix = makeMatrix();
                        SkPath::AddPathMode mode = makeAddPathMode();
                        path.addPath(src, matrix, mode);
                        --fPathDepth;
                        validate(path);
                    }
                    break;
                case kReverseAddPath:
                    if (fPathDepth < fPathDepthLimit) {
                        ++fPathDepth;
                        SkPath src = makePath();
                        validate(src);
                        path.reverseAddPath(src);
                        --fPathDepth;
                        validate(path);
                    }
                    break;
                case kMoveToPath: {
                    SkScalar x = makeScalar();
                    SkScalar y = makeScalar();
                    path.moveTo(x, y);
                    validate(path);
                    } break;
                case kRMoveToPath: {
                    SkScalar x = makeScalar();
                    SkScalar y = makeScalar();
                    path.rMoveTo(x, y);
                    validate(path);
                    } break;
                case kLineToPath: {
                    SkScalar x = makeScalar();
                    SkScalar y = makeScalar();
                    path.lineTo(x, y);
                    validate(path);
                    } break;
                case kRLineToPath: {
                    SkScalar x = makeScalar();
                    SkScalar y = makeScalar();
                    path.rLineTo(x, y);
                    validate(path);
                    } break;
                case kQuadToPath: {
                    SkPoint pt[2];
                    makePointArray(SK_ARRAY_COUNT(pt), pt);
                    path.quadTo(pt[0], pt[1]);
                    validate(path);
                    } break;
                case kRQuadToPath: {
                    SkPoint pt[2];
                    makePointArray(SK_ARRAY_COUNT(pt), pt);
                    path.rQuadTo(pt[0].fX, pt[0].fY, pt[1].fX, pt[1].fY);
                    validate(path);
                    } break;
                case kConicToPath: {
                    SkPoint pt[2];
                    makePointArray(SK_ARRAY_COUNT(pt), pt);
                    SkScalar weight = makeScalar();
                    path.conicTo(pt[0], pt[1], weight);
                    validate(path);
                    } break;
                case kRConicToPath: {
                    SkPoint pt[2];
                    makePointArray(SK_ARRAY_COUNT(pt), pt);
                    SkScalar weight = makeScalar();
                    path.rConicTo(pt[0].fX, pt[0].fY, pt[1].fX, pt[1].fY, weight);
                    validate(path);
                    } break;
                case kCubicToPath: {
                    SkPoint pt[3];
                    makePointArray(SK_ARRAY_COUNT(pt), pt);
                    path.cubicTo(pt[0], pt[1], pt[2]);
                    validate(path);
                    } break;
                case kRCubicToPath: {
                    SkPoint pt[3];
                    makePointArray(SK_ARRAY_COUNT(pt), pt);
                    path.rCubicTo(pt[0].fX, pt[0].fY, pt[1].fX, pt[1].fY, pt[2].fX, pt[2].fY);
                    validate(path);
                    } break;
                case kArcToPath: {
                    SkPoint pt[2];
                    makePointArray(SK_ARRAY_COUNT(pt), pt);
                    SkScalar radius = makeScalar();
                    path.arcTo(pt[0], pt[1], radius);
                    validate(path);
                    } break;
                case kArcTo2Path: {
                    SkRect oval = makeRect();
                    SkScalar startAngle = makeAngle();
                    SkScalar sweepAngle = makeAngle();
                    bool forceMoveTo = makeBool();
                    path.arcTo(oval, startAngle, sweepAngle, forceMoveTo);
                    validate(path);
                    } break;
                case kClosePath:
                    path.close();
                    validate(path);
                    break;
            }
        }
    }
    return path;
}

uint32_t makeSegmentCount() {
    return fRand.nextRangeU(1, fPathSegmentLimit);
}

RandomSetRRect makeSetRRectType() {
    return (RandomSetRRect) fRand.nextRangeU(0, kRandomSetRRect_Last);
}

SkScalar makeScalar() {
    SkScalar scalar;
    scalar = fRand.nextRangeF(fFloatMin, fFloatMax);
    return scalar;
}

void makeScalarArray(size_t arrayCount, SkScalar* array) {
    for (size_t index = 0; index < arrayCount; ++index) {
        array[index] = makeScalar();
    }
}

void makeVectorArray(size_t arrayCount, SkVector* array) {
    for (size_t index = 0; index < arrayCount; ++index) {
        array[index] = makeVector();
    }
}

SkVector makeVector() {
    SkVector result;
    makeScalarArray(2, &result.fX);
    return result;
}

void validate(const SkPath& path) {
    if (fValidate) {
        SkDEBUGCODE(path.experimentalValidateRef());
    }
}

SkRandom fRand;
SkMatrix fMatrix;
SkPath fClip;
SkPaint fPaint;
SkPath fPath;
SkScalar fFloatMin;
SkScalar fFloatMax;
uint32_t fPathContourCount;
int fPathDepth;
int fPathDepthLimit;
uint32_t fPathSegmentLimit;
int fAddCount;
bool fPrintName;
bool fStrokeOnly;
bool fValidate;
const char* fTab;
};

static bool contains_only_moveTo(const SkPath& path) {
    int verbCount = path.countVerbs();
    if (verbCount == 0) {
        return true;
    }
    SkTDArray<uint8_t> verbs;
    verbs.setCount(verbCount);
    SkDEBUGCODE(int getVerbResult = ) path.getVerbs(verbs.begin(), verbCount);
    SkASSERT(getVerbResult == verbCount);
    for (int index = 0; index < verbCount; ++index) {
        if (verbs[index] != SkPath::kMove_Verb) {
            return false;
        }
    }
    return true;
}

#include "SkGraphics.h"
#include "SkSurface.h"
#include "SkTaskGroup.h"
#include "SkTDArray.h"

static void path_fuzz_stroker(SkBitmap* bitmap, int seed) {
    SkTaskGroup().batch(100, [&](int i) {
        int localSeed = seed + i;

        FuzzPath fuzzPath;
        fuzzPath.setStrokeOnly();
        fuzzPath.setSeed(localSeed);
        fuzzPath.randomize();
        const SkPath& path = fuzzPath.getPath();
        const SkPaint& paint = fuzzPath.getPaint();
        const SkImageInfo& info = bitmap->info();
        std::unique_ptr<SkCanvas> canvas(
            SkCanvas::MakeRasterDirect(info, bitmap->getPixels(), bitmap->rowBytes()));
        int w = info.width() / 4;
        int h = info.height() / 4;
        int x = localSeed / 4 % 4;
        int y = localSeed % 4;
        SkRect clipBounds = SkRect::MakeXYWH(SkIntToScalar(x) * w, SkIntToScalar(y) * h,
            SkIntToScalar(w), SkIntToScalar(h));
        canvas->save();
            canvas->clipRect(clipBounds);
            canvas->translate(SkIntToScalar(x) * w, SkIntToScalar(y) * h);
            canvas->drawPath(path, paint);
        canvas->restore();
    });
}

class PathFuzzView : public SampleView {
public:
    PathFuzzView()
        : fOneDraw(false)
    {
    }
protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "PathFuzzer");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onOnceBeforeDraw() override {
        fIndex = 0;
        SkImageInfo info(SkImageInfo::MakeN32Premul(SkScalarRoundToInt(width()),
                SkScalarRoundToInt(height())));
        offscreen.allocPixels(info);
        path_fuzz_stroker(&offscreen, fIndex);
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (fOneDraw) {
            fuzzPath.randomize();
            const SkPath& path = fuzzPath.getPath();
            const SkPaint& paint = fuzzPath.getPaint();
            const SkPath& clip = fuzzPath.getClip();
            const SkMatrix& matrix = fuzzPath.getMatrix();
            if (!contains_only_moveTo(clip)) {
                canvas->clipPath(clip);
            }
            canvas->setMatrix(matrix);
            canvas->drawPath(path, paint);
        } else {
            path_fuzz_stroker(&offscreen, fIndex += 100);
            canvas->drawBitmap(offscreen, 0, 0);
        }
        this->inval(nullptr);
    }

private:
    int fIndex;
    SkBitmap offscreen;
    FuzzPath fuzzPath;
    bool fOneDraw;
    typedef SkView INHERITED;
};

static SkView* MyFactory() { return new PathFuzzView; }
static SkViewRegister reg(MyFactory);
