/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkClipStack.h"
#include "SkColorPriv.h"
#include "SkDebugCanvas.h"
#include "SkDrawCommand.h"
#include "SkDevice.h"
#include "SkPaintFilterCanvas.h"
#include "SkXfermode.h"

namespace {

class OverdrawXfermode : public SkXfermode {
public:
    SkPMColor xferColor(SkPMColor src, SkPMColor dst) const override {
        // This table encodes the color progression of the overdraw visualization
        static const SkPMColor gTable[] = {
            SkPackARGB32(0x00, 0x00, 0x00, 0x00),
            SkPackARGB32(0xFF, 128, 158, 255),
            SkPackARGB32(0xFF, 170, 185, 212),
            SkPackARGB32(0xFF, 213, 195, 170),
            SkPackARGB32(0xFF, 255, 192, 127),
            SkPackARGB32(0xFF, 255, 185, 85),
            SkPackARGB32(0xFF, 255, 165, 42),
            SkPackARGB32(0xFF, 255, 135, 0),
            SkPackARGB32(0xFF, 255,  95, 0),
            SkPackARGB32(0xFF, 255,  50, 0),
            SkPackARGB32(0xFF, 255,  0, 0)
        };


        int idx;
        if (SkColorGetR(dst) < 64) { // 0
            idx = 0;
        } else if (SkColorGetG(dst) < 25) { // 10
            idx = 9;  // cap at 9 for upcoming increment
        } else if ((SkColorGetB(dst)+21)/42 > 0) { // 1-6
            idx = 7 - (SkColorGetB(dst)+21)/42;
        } else { // 7-9
            idx = 10 - (SkColorGetG(dst)+22)/45;
        }
        ++idx;
        SkASSERT(idx < (int)SK_ARRAY_COUNT(gTable));

        return gTable[idx];
    }

    Factory getFactory() const override { return NULL; }
#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const override { str->set("OverdrawXfermode"); }
#endif
};

class DebugPaintFilterCanvas : public SkPaintFilterCanvas {
public:
    DebugPaintFilterCanvas(int width, int height, bool overdrawViz, bool overrideFilterQuality,
                           SkFilterQuality quality)
        : INHERITED(width, height)
        , fOverdrawXfermode(overdrawViz ? SkNEW(OverdrawXfermode) : NULL)
        , fOverrideFilterQuality(overrideFilterQuality)
        , fFilterQuality(quality) { }

protected:
    void onFilterPaint(SkPaint* paint, Type) const override {
        if (NULL != fOverdrawXfermode.get()) {
            paint->setAntiAlias(false);
            paint->setXfermode(fOverdrawXfermode.get());
        }

        if (fOverrideFilterQuality) {
            paint->setFilterQuality(fFilterQuality);
        }
    }

    void onDrawPicture(const SkPicture* picture,
                       const SkMatrix* matrix,
                       const SkPaint* paint) override {
        // We need to replay the picture onto this canvas in order to filter its internal paints.
        this->SkCanvas::onDrawPicture(picture, matrix, paint);
    }

private:
    SkAutoTUnref<SkXfermode> fOverdrawXfermode;

    bool fOverrideFilterQuality;
    SkFilterQuality fFilterQuality;

    typedef SkPaintFilterCanvas INHERITED;
};

}

SkDebugCanvas::SkDebugCanvas(int width, int height)
        : INHERITED(width, height)
        , fPicture(NULL)
        , fFilter(false)
        , fMegaVizMode(false)
        , fOverdrawViz(false)
        , fOverrideFilterQuality(false)
        , fFilterQuality(kNone_SkFilterQuality) {
    fUserMatrix.reset();

    // SkPicturePlayback uses the base-class' quickReject calls to cull clipped
    // operations. This can lead to problems in the debugger which expects all
    // the operations in the captured skp to appear in the debug canvas. To
    // circumvent this we create a wide open clip here (an empty clip rect
    // is not sufficient).
    // Internally, the SkRect passed to clipRect is converted to an SkIRect and
    // rounded out. The following code creates a nearly maximal rect that will
    // not get collapsed by the coming conversions (Due to precision loss the
    // inset has to be surprisingly large).
    SkIRect largeIRect = SkIRect::MakeLargest();
    largeIRect.inset(1024, 1024);
    SkRect large = SkRect::Make(largeIRect);
#ifdef SK_DEBUG
    SkASSERT(!large.roundOut().isEmpty());
#endif
    // call the base class' version to avoid adding a draw command
    this->INHERITED::onClipRect(large, SkRegion::kReplace_Op, kHard_ClipEdgeStyle);
}

SkDebugCanvas::~SkDebugCanvas() {
    fCommandVector.deleteAll();
}

void SkDebugCanvas::addDrawCommand(SkDrawCommand* command) {
    fCommandVector.push(command);
}

void SkDebugCanvas::draw(SkCanvas* canvas) {
    if (!fCommandVector.isEmpty()) {
        this->drawTo(canvas, fCommandVector.count() - 1);
    }
}

void SkDebugCanvas::applyUserTransform(SkCanvas* canvas) {
    canvas->concat(fUserMatrix);
}

int SkDebugCanvas::getCommandAtPoint(int x, int y, int index) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32Premul(1, 1));

    SkCanvas canvas(bitmap);
    canvas.translate(SkIntToScalar(-x), SkIntToScalar(-y));
    this->applyUserTransform(&canvas);

    int layer = 0;
    SkColor prev = bitmap.getColor(0,0);
    for (int i = 0; i < index; i++) {
        if (fCommandVector[i]->isVisible()) {
            fCommandVector[i]->setUserMatrix(fUserMatrix);
            fCommandVector[i]->execute(&canvas);
        }
        if (prev != bitmap.getColor(0,0)) {
            layer = i;
        }
        prev = bitmap.getColor(0,0);
    }
    return layer;
}

class SkDebugClipVisitor : public SkCanvas::ClipVisitor {
public:
    SkDebugClipVisitor(SkCanvas* canvas) : fCanvas(canvas) {}

    void clipRect(const SkRect& r, SkRegion::Op, bool doAA) override {
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setStyle(SkPaint::kStroke_Style);
        p.setAntiAlias(doAA);
        fCanvas->drawRect(r, p);
    }
    void clipRRect(const SkRRect& rr, SkRegion::Op, bool doAA) override {
        SkPaint p;
        p.setColor(SK_ColorGREEN);
        p.setStyle(SkPaint::kStroke_Style);
        p.setAntiAlias(doAA);
        fCanvas->drawRRect(rr, p);
    }
    void clipPath(const SkPath& path, SkRegion::Op, bool doAA) override {
        SkPaint p;
        p.setColor(SK_ColorBLUE);
        p.setStyle(SkPaint::kStroke_Style);
        p.setAntiAlias(doAA);
        fCanvas->drawPath(path, p);
    }

protected:
    SkCanvas* fCanvas;

private:
    typedef SkCanvas::ClipVisitor INHERITED;
};

// set up the saveLayer commands so that the active ones
// return true in their 'active' method
void SkDebugCanvas::markActiveCommands(int index) {
    fActiveLayers.rewind();

    for (int i = 0; i < fCommandVector.count(); ++i) {
        fCommandVector[i]->setActive(false);
    }

    for (int i = 0; i < index; ++i) {
        SkDrawCommand::Action result = fCommandVector[i]->action();
        if (SkDrawCommand::kPushLayer_Action == result) {
            fActiveLayers.push(fCommandVector[i]);
        } else if (SkDrawCommand::kPopLayer_Action == result) {
            fActiveLayers.pop();
        }
    }

    for (int i = 0; i < fActiveLayers.count(); ++i) {
        fActiveLayers[i]->setActive(true);
    }

}

void SkDebugCanvas::drawTo(SkCanvas* canvas, int index) {
    SkASSERT(!fCommandVector.isEmpty());
    SkASSERT(index < fCommandVector.count());

    int saveCount = canvas->save();

    SkRect windowRect = SkRect::MakeWH(SkIntToScalar(canvas->getBaseLayerSize().width()),
                                       SkIntToScalar(canvas->getBaseLayerSize().height()));

    bool pathOpsMode = getAllowSimplifyClip();
    canvas->setAllowSimplifyClip(pathOpsMode);
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->resetMatrix();
    if (!windowRect.isEmpty()) {
        canvas->clipRect(windowRect, SkRegion::kReplace_Op);
    }
    this->applyUserTransform(canvas);

    if (fPaintFilterCanvas) {
        fPaintFilterCanvas->addCanvas(canvas);
        canvas = fPaintFilterCanvas.get();
    }

    if (fMegaVizMode) {
        this->markActiveCommands(index);
    }

    for (int i = 0; i <= index; i++) {
        if (i == index && fFilter) {
            canvas->clear(0xAAFFFFFF);
        }

        if (fCommandVector[i]->isVisible()) {
            if (fMegaVizMode && fCommandVector[i]->active()) {
                // "active" commands execute their visualization behaviors:
                //     All active saveLayers get replaced with saves so all draws go to the
                //     visible canvas.
                //     All active culls draw their cull box
                fCommandVector[i]->vizExecute(canvas);
            } else {
                fCommandVector[i]->setUserMatrix(fUserMatrix);
                fCommandVector[i]->execute(canvas);
            }
        }
    }

    if (fMegaVizMode) {
        canvas->save();
        // nuke the CTM
        canvas->resetMatrix();
        // turn off clipping
        if (!windowRect.isEmpty()) {
            SkRect r = windowRect;
            r.outset(SK_Scalar1, SK_Scalar1);
            canvas->clipRect(r, SkRegion::kReplace_Op);
        }
        // visualize existing clips
        SkDebugClipVisitor visitor(canvas);

        canvas->replayClips(&visitor);

        canvas->restore();
    }
    if (pathOpsMode) {
        this->resetClipStackData();
        const SkClipStack* clipStack = canvas->getClipStack();
        SkClipStack::Iter iter(*clipStack, SkClipStack::Iter::kBottom_IterStart);
        const SkClipStack::Element* element;
        SkPath devPath;
        while ((element = iter.next())) {
            SkClipStack::Element::Type type = element->getType();
            SkPath operand;
            if (type != SkClipStack::Element::kEmpty_Type) {
               element->asPath(&operand);
            }
            SkRegion::Op elementOp = element->getOp();
            this->addClipStackData(devPath, operand, elementOp);
            if (elementOp == SkRegion::kReplace_Op) {
                devPath = operand;
            } else {
                Op(devPath, operand, (SkPathOp) elementOp, &devPath);
            }
        }
        this->lastClipStackData(devPath);
    }
    fMatrix = canvas->getTotalMatrix();
    if (!canvas->getClipDeviceBounds(&fClip)) {
        fClip.setEmpty();
    }

    canvas->restoreToCount(saveCount);

    if (fPaintFilterCanvas) {
        fPaintFilterCanvas->removeAll();
    }
}

void SkDebugCanvas::deleteDrawCommandAt(int index) {
    SkASSERT(index < fCommandVector.count());
    delete fCommandVector[index];
    fCommandVector.remove(index);
}

SkDrawCommand* SkDebugCanvas::getDrawCommandAt(int index) {
    SkASSERT(index < fCommandVector.count());
    return fCommandVector[index];
}

void SkDebugCanvas::setDrawCommandAt(int index, SkDrawCommand* command) {
    SkASSERT(index < fCommandVector.count());
    delete fCommandVector[index];
    fCommandVector[index] = command;
}

const SkTDArray<SkString*>* SkDebugCanvas::getCommandInfo(int index) const {
    SkASSERT(index < fCommandVector.count());
    return fCommandVector[index]->Info();
}

bool SkDebugCanvas::getDrawCommandVisibilityAt(int index) {
    SkASSERT(index < fCommandVector.count());
    return fCommandVector[index]->isVisible();
}

const SkTDArray <SkDrawCommand*>& SkDebugCanvas::getDrawCommands() const {
    return fCommandVector;
}

SkTDArray <SkDrawCommand*>& SkDebugCanvas::getDrawCommands() {
    return fCommandVector;
}

void SkDebugCanvas::updatePaintFilterCanvas() {
    if (!fOverdrawViz && !fOverrideFilterQuality) {
        fPaintFilterCanvas.reset(NULL);
        return;
    }

    const SkImageInfo info = this->imageInfo();
    fPaintFilterCanvas.reset(SkNEW_ARGS(DebugPaintFilterCanvas, (info.width(),
                                                                 info.height(),
                                                                 fOverdrawViz,
                                                                 fOverrideFilterQuality,
                                                                 fFilterQuality)));
}

void SkDebugCanvas::setOverdrawViz(bool overdrawViz) {
    if (fOverdrawViz == overdrawViz) {
        return;
    }

    fOverdrawViz = overdrawViz;
    this->updatePaintFilterCanvas();
}

void SkDebugCanvas::overrideTexFiltering(bool overrideTexFiltering, SkFilterQuality quality) {
    if (fOverrideFilterQuality == overrideTexFiltering && fFilterQuality == quality) {
        return;
    }

    fOverrideFilterQuality = overrideTexFiltering;
    fFilterQuality = quality;
    this->updatePaintFilterCanvas();
}

void SkDebugCanvas::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    this->addDrawCommand(new SkClipPathCommand(path, op, kSoft_ClipEdgeStyle == edgeStyle));
}

void SkDebugCanvas::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    this->addDrawCommand(new SkClipRectCommand(rect, op, kSoft_ClipEdgeStyle == edgeStyle));
}

void SkDebugCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    this->addDrawCommand(new SkClipRRectCommand(rrect, op, kSoft_ClipEdgeStyle == edgeStyle));
}

void SkDebugCanvas::onClipRegion(const SkRegion& region, SkRegion::Op op) {
    this->addDrawCommand(new SkClipRegionCommand(region, op));
}

void SkDebugCanvas::didConcat(const SkMatrix& matrix) {
    this->addDrawCommand(new SkConcatCommand(matrix));
    this->INHERITED::didConcat(matrix);
}

void SkDebugCanvas::onDrawBitmap(const SkBitmap& bitmap, SkScalar left,
                                 SkScalar top, const SkPaint* paint) {
    this->addDrawCommand(new SkDrawBitmapCommand(bitmap, left, top, paint));
}

void SkDebugCanvas::onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                                     const SkPaint* paint, SrcRectConstraint constraint) {
    this->addDrawCommand(new SkDrawBitmapRectCommand(bitmap, src, dst, paint,
                                                     (SrcRectConstraint)constraint));
}

void SkDebugCanvas::onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                     const SkRect& dst, const SkPaint* paint) {
    this->addDrawCommand(new SkDrawBitmapNineCommand(bitmap, center, dst, paint));
}

void SkDebugCanvas::onDrawImage(const SkImage* image, SkScalar left, SkScalar top,
                                const SkPaint* paint) {
    this->addDrawCommand(new SkDrawImageCommand(image, left, top, paint));
}

void SkDebugCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                    const SkPaint* paint, SrcRectConstraint constraint) {
    this->addDrawCommand(new SkDrawImageRectCommand(image, src, dst, paint, constraint));
}

void SkDebugCanvas::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    this->addDrawCommand(new SkDrawOvalCommand(oval, paint));
}

void SkDebugCanvas::onDrawPaint(const SkPaint& paint) {
    this->addDrawCommand(new SkDrawPaintCommand(paint));
}

void SkDebugCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    this->addDrawCommand(new SkDrawPathCommand(path, paint));
}

void SkDebugCanvas::onDrawPicture(const SkPicture* picture,
                                  const SkMatrix* matrix,
                                  const SkPaint* paint) {
    this->addDrawCommand(new SkBeginDrawPictureCommand(picture, matrix, paint));
    this->INHERITED::onDrawPicture(picture, matrix, paint);
    this->addDrawCommand(new SkEndDrawPictureCommand(SkToBool(matrix) || SkToBool(paint)));
}

void SkDebugCanvas::onDrawPoints(PointMode mode, size_t count,
                                 const SkPoint pts[], const SkPaint& paint) {
    this->addDrawCommand(new SkDrawPointsCommand(mode, count, pts, paint));
}

void SkDebugCanvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                                  const SkPaint& paint) {
    this->addDrawCommand(new SkDrawPosTextCommand(text, byteLength, pos, paint));
}

void SkDebugCanvas::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                   SkScalar constY, const SkPaint& paint) {
    this->addDrawCommand(
        new SkDrawPosTextHCommand(text, byteLength, xpos, constY, paint));
}

void SkDebugCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    // NOTE(chudy): Messing up when renamed to DrawRect... Why?
    addDrawCommand(new SkDrawRectCommand(rect, paint));
}

void SkDebugCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    this->addDrawCommand(new SkDrawRRectCommand(rrect, paint));
}

void SkDebugCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                 const SkPaint& paint) {
    this->addDrawCommand(new SkDrawDRRectCommand(outer, inner, paint));
}

void SkDebugCanvas::onDrawSprite(const SkBitmap& bitmap, int left, int top, const SkPaint* paint) {
    this->addDrawCommand(new SkDrawSpriteCommand(bitmap, left, top, paint));
}

void SkDebugCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                               const SkPaint& paint) {
    this->addDrawCommand(new SkDrawTextCommand(text, byteLength, x, y, paint));
}

void SkDebugCanvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                     const SkMatrix* matrix, const SkPaint& paint) {
    this->addDrawCommand(
        new SkDrawTextOnPathCommand(text, byteLength, path, matrix, paint));
}

void SkDebugCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                   const SkPaint& paint) {
    this->addDrawCommand(new SkDrawTextBlobCommand(blob, x, y, paint));
}

void SkDebugCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                                const SkPoint texCoords[4], SkXfermode* xmode,
                                const SkPaint& paint) {
    this->addDrawCommand(new SkDrawPatchCommand(cubics, colors, texCoords, xmode, paint));
}

void SkDebugCanvas::onDrawVertices(VertexMode vmode, int vertexCount, const SkPoint vertices[],
                                   const SkPoint texs[], const SkColor colors[],
                                   SkXfermode*, const uint16_t indices[], int indexCount,
                                   const SkPaint& paint) {
    this->addDrawCommand(new SkDrawVerticesCommand(vmode, vertexCount, vertices,
                         texs, colors, NULL, indices, indexCount, paint));
}

void SkDebugCanvas::willRestore() {
    this->addDrawCommand(new SkRestoreCommand());
    this->INHERITED::willRestore();
}

void SkDebugCanvas::willSave() {
    this->addDrawCommand(new SkSaveCommand());
    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy SkDebugCanvas::willSaveLayer(const SkRect* bounds, const SkPaint* paint,
                                                         SaveFlags flags) {
    this->addDrawCommand(new SkSaveLayerCommand(bounds, paint, flags));
    this->INHERITED::willSaveLayer(bounds, paint, flags);
    // No need for a full layer.
    return kNoLayer_SaveLayerStrategy;
}

void SkDebugCanvas::didSetMatrix(const SkMatrix& matrix) {
    this->addDrawCommand(new SkSetMatrixCommand(matrix));
    this->INHERITED::didSetMatrix(matrix);
}

void SkDebugCanvas::toggleCommand(int index, bool toggle) {
    SkASSERT(index < fCommandVector.count());
    fCommandVector[index]->setVisible(toggle);
}

static const char* gFillTypeStrs[] = {
    "kWinding_FillType",
    "kEvenOdd_FillType",
    "kInverseWinding_FillType",
    "kInverseEvenOdd_FillType"
};

static const char* gOpStrs[] = {
    "kDifference_PathOp",
    "kIntersect_PathOp",
    "kUnion_PathOp",
    "kXor_PathOp",
    "kReverseDifference_PathOp",
};

static const char kHTML4SpaceIndent[] = "&nbsp;&nbsp;&nbsp;&nbsp;";

void SkDebugCanvas::outputScalar(SkScalar num) {
    if (num == (int) num) {
        fClipStackData.appendf("%d", (int) num);
    } else {
        SkString str;
        str.printf("%1.9g", num);
        int width = (int) str.size();
        const char* cStr = str.c_str();
        while (cStr[width - 1] == '0') {
            --width;
        }
        str.resize(width);
        fClipStackData.appendf("%sf", str.c_str());
    }
}

void SkDebugCanvas::outputPointsCommon(const SkPoint* pts, int count) {
    for (int index = 0; index < count; ++index) {
        this->outputScalar(pts[index].fX);
        fClipStackData.appendf(", ");
        this->outputScalar(pts[index].fY);
        if (index + 1 < count) {
            fClipStackData.appendf(", ");
        }
    }
}

void SkDebugCanvas::outputPoints(const SkPoint* pts, int count) {
    this->outputPointsCommon(pts, count);
    fClipStackData.appendf(");<br>");
}

void SkDebugCanvas::outputConicPoints(const SkPoint* pts, SkScalar weight) {
    this->outputPointsCommon(pts, 2);
    fClipStackData.appendf(", ");
    this->outputScalar(weight);
    fClipStackData.appendf(");<br>");
}

void SkDebugCanvas::addPathData(const SkPath& path, const char* pathName) {
    SkPath::RawIter iter(path);
    SkPath::FillType fillType = path.getFillType();
    fClipStackData.appendf("%sSkPath %s;<br>", kHTML4SpaceIndent, pathName);
    fClipStackData.appendf("%s%s.setFillType(SkPath::%s);<br>", kHTML4SpaceIndent, pathName,
            gFillTypeStrs[fillType]);
    iter.setPath(path);
    uint8_t verb;
    SkPoint pts[4];
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                fClipStackData.appendf("%s%s.moveTo(", kHTML4SpaceIndent, pathName);
                this->outputPoints(&pts[0], 1);
                continue;
            case SkPath::kLine_Verb:
                fClipStackData.appendf("%s%s.lineTo(", kHTML4SpaceIndent, pathName);
                this->outputPoints(&pts[1], 1);
                break;
            case SkPath::kQuad_Verb:
                fClipStackData.appendf("%s%s.quadTo(", kHTML4SpaceIndent, pathName);
                this->outputPoints(&pts[1], 2);
                break;
            case SkPath::kConic_Verb:
                fClipStackData.appendf("%s%s.conicTo(", kHTML4SpaceIndent, pathName);
                this->outputConicPoints(&pts[1], iter.conicWeight());
                break;
            case SkPath::kCubic_Verb:
                fClipStackData.appendf("%s%s.cubicTo(", kHTML4SpaceIndent, pathName);
                this->outputPoints(&pts[1], 3);
                break;
            case SkPath::kClose_Verb:
                fClipStackData.appendf("%s%s.close();<br>", kHTML4SpaceIndent, pathName);
                break;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
    }
}

void SkDebugCanvas::addClipStackData(const SkPath& devPath, const SkPath& operand,
                                     SkRegion::Op elementOp) {
    if (elementOp == SkRegion::kReplace_Op) {
        if (!lastClipStackData(devPath)) {
            fSaveDevPath = operand;
        }
        fCalledAddStackData = false;
    } else {
        fClipStackData.appendf("<br>static void test(skiatest::Reporter* reporter,"
            " const char* filename) {<br>");
        addPathData(fCalledAddStackData ? devPath : fSaveDevPath, "path");
        addPathData(operand, "pathB");
        fClipStackData.appendf("%stestPathOp(reporter, path, pathB, %s, filename);<br>",
            kHTML4SpaceIndent, gOpStrs[elementOp]);
        fClipStackData.appendf("}<br>");
        fCalledAddStackData = true;
    }
}

bool SkDebugCanvas::lastClipStackData(const SkPath& devPath) {
    if (fCalledAddStackData) {
        fClipStackData.appendf("<br>");
        addPathData(devPath, "pathOut");
        return true;
    }
    return false;
}
