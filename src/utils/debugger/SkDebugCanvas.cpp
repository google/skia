
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkColorPriv.h"
#include "SkDebugCanvas.h"
#include "SkDrawCommand.h"
#include "SkDrawFilter.h"
#include "SkDevice.h"
#include "SkXfermode.h"

SkDebugCanvas::SkDebugCanvas(int width, int height)
        : INHERITED(width, height)
        , fWidth(width)
        , fHeight(height)
        , fFilter(false)
        , fMegaVizMode(false)
        , fIndex(0)
        , fOverdrawViz(false)
        , fOverdrawFilter(NULL)
        , fOverrideTexFiltering(false)
        , fTexOverrideFilter(NULL)
        , fOutstandingSaveCount(0) {
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
    large.roundOut(&largeIRect);
    SkASSERT(!largeIRect.isEmpty());
#endif
    // call the base class' version to avoid adding a draw command
    this->INHERITED::onClipRect(large, SkRegion::kReplace_Op, kHard_ClipEdgeStyle);
}

SkDebugCanvas::~SkDebugCanvas() {
    fCommandVector.deleteAll();
    SkSafeUnref(fOverdrawFilter);
    SkSafeUnref(fTexOverrideFilter);
}

void SkDebugCanvas::addDrawCommand(SkDrawCommand* command) {
    fCommandVector.push(command);
}

void SkDebugCanvas::draw(SkCanvas* canvas) {
    if (!fCommandVector.isEmpty()) {
        drawTo(canvas, fCommandVector.count() - 1);
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
    applyUserTransform(&canvas);

    int layer = 0;
    SkColor prev = bitmap.getColor(0,0);
    for (int i = 0; i < index; i++) {
        if (fCommandVector[i]->isVisible()) {
            fCommandVector[i]->execute(&canvas);
        }
        if (prev != bitmap.getColor(0,0)) {
            layer = i;
        }
        prev = bitmap.getColor(0,0);
    }
    return layer;
}

static SkPMColor OverdrawXferModeProc(SkPMColor src, SkPMColor dst) {
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

    for (size_t i = 0; i < SK_ARRAY_COUNT(gTable)-1; ++i) {
        if (gTable[i] == dst) {
            return gTable[i+1];
        }
    }

    return gTable[SK_ARRAY_COUNT(gTable)-1];
}

// The OverdrawFilter modifies every paint to use an SkProcXfermode which
// in turn invokes OverdrawXferModeProc
class SkOverdrawFilter : public SkDrawFilter {
public:
    SkOverdrawFilter() {
        fXferMode = SkProcXfermode::Create(OverdrawXferModeProc);
    }

    virtual ~SkOverdrawFilter() {
        delete fXferMode;
    }

    virtual bool filter(SkPaint* p, Type) SK_OVERRIDE {
        p->setXfermode(fXferMode);
        return true;
    }

protected:
    SkXfermode* fXferMode;

private:
    typedef SkDrawFilter INHERITED;
};

// SkTexOverrideFilter modifies every paint to use the specified
// texture filtering mode
class SkTexOverrideFilter : public SkDrawFilter {
public:
    SkTexOverrideFilter() : fFilterLevel(SkPaint::kNone_FilterLevel) {
    }

    void setFilterLevel(SkPaint::FilterLevel filterLevel) {
        fFilterLevel = filterLevel;
    }

    virtual bool filter(SkPaint* p, Type) SK_OVERRIDE {
        p->setFilterLevel(fFilterLevel);
        return true;
    }

protected:
    SkPaint::FilterLevel fFilterLevel;

private:
    typedef SkDrawFilter INHERITED;
};

class SkDebugClipVisitor : public SkCanvas::ClipVisitor {
public:
    SkDebugClipVisitor(SkCanvas* canvas) : fCanvas(canvas) {}

    virtual void clipRect(const SkRect& r, SkRegion::Op, bool doAA) SK_OVERRIDE {
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setStyle(SkPaint::kStroke_Style);
        p.setAntiAlias(doAA);
        fCanvas->drawRect(r, p);
    }
    virtual void clipRRect(const SkRRect& rr, SkRegion::Op, bool doAA) SK_OVERRIDE {
        SkPaint p;
        p.setColor(SK_ColorGREEN);
        p.setStyle(SkPaint::kStroke_Style);
        p.setAntiAlias(doAA);
        fCanvas->drawRRect(rr, p);
    }
    virtual void clipPath(const SkPath& path, SkRegion::Op, bool doAA) SK_OVERRIDE {
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
    fActiveCulls.rewind();

    for (int i = 0; i < fCommandVector.count(); ++i) {
        fCommandVector[i]->setActive(false);
    }

    for (int i = 0; i < index; ++i) {
        SkDrawCommand::Action result = fCommandVector[i]->action();
        if (SkDrawCommand::kPushLayer_Action == result) {
            fActiveLayers.push(fCommandVector[i]);
        } else if (SkDrawCommand::kPopLayer_Action == result) {
            fActiveLayers.pop();
        } else if (SkDrawCommand::kPushCull_Action == result) {
            fActiveCulls.push(fCommandVector[i]);
        } else if (SkDrawCommand::kPopCull_Action == result) {
            fActiveCulls.pop();
        }
    }

    for (int i = 0; i < fActiveLayers.count(); ++i) {
        fActiveLayers[i]->setActive(true);
    }

    for (int i = 0; i < fActiveCulls.count(); ++i) {
        fActiveCulls[i]->setActive(true);
    }
}

void SkDebugCanvas::drawTo(SkCanvas* canvas, int index) {
    SkASSERT(!fCommandVector.isEmpty());
    SkASSERT(index < fCommandVector.count());
    int i = 0;

    // This only works assuming the canvas and device are the same ones that
    // were previously drawn into because they need to preserve all saves
    // and restores.
    // The visibility filter also requires a full re-draw - otherwise we can
    // end up drawing the filter repeatedly.
    if (fIndex < index && !fFilter && !fMegaVizMode) {
        i = fIndex + 1;
    } else {
        for (int j = 0; j < fOutstandingSaveCount; j++) {
            canvas->restore();
        }
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->resetMatrix();
        SkRect rect = SkRect::MakeWH(SkIntToScalar(fWidth),
                                     SkIntToScalar(fHeight));
        canvas->clipRect(rect, SkRegion::kReplace_Op );
        applyUserTransform(canvas);
        fOutstandingSaveCount = 0;
    }

    // The setting of the draw filter has to go here (rather than in
    // SkRasterWidget) due to the canvas restores this class performs.
    // Since the draw filter is stored in the layer stack if we
    // call setDrawFilter on anything but the root layer odd things happen.
    if (fOverdrawViz) {
        if (NULL == fOverdrawFilter) {
            fOverdrawFilter = new SkOverdrawFilter;
        }

        if (fOverdrawFilter != canvas->getDrawFilter()) {
            canvas->setDrawFilter(fOverdrawFilter);
        }
    } else if (fOverrideTexFiltering) {
        if (NULL == fTexOverrideFilter) {
            fTexOverrideFilter = new SkTexOverrideFilter;
        }

        if (fTexOverrideFilter != canvas->getDrawFilter()) {
            canvas->setDrawFilter(fTexOverrideFilter);
        }
    } else {
        canvas->setDrawFilter(NULL);
    }

    if (fMegaVizMode) {
        this->markActiveCommands(index);
    }

    for (; i <= index; i++) {
        if (i == index && fFilter) {
            SkPaint p;
            p.setColor(0xAAFFFFFF);
            canvas->save();
            canvas->resetMatrix();
            SkRect mask;
            mask.set(SkIntToScalar(0), SkIntToScalar(0),
                    SkIntToScalar(fWidth), SkIntToScalar(fHeight));
            canvas->clipRect(mask, SkRegion::kReplace_Op, false);
            canvas->drawRectCoords(SkIntToScalar(0), SkIntToScalar(0),
                    SkIntToScalar(fWidth), SkIntToScalar(fHeight), p);
            canvas->restore();
        }

        if (fCommandVector[i]->isVisible()) {
            if (fMegaVizMode && fCommandVector[i]->active()) {
                // "active" commands execute their visualization behaviors:
                //     All active saveLayers get replaced with saves so all draws go to the
                //     visible canvas.
                //     All active culls draw their cull box
                fCommandVector[i]->vizExecute(canvas);
            } else {
                fCommandVector[i]->execute(canvas);
            }

            fCommandVector[i]->trackSaveState(&fOutstandingSaveCount);
        }
    }

    if (fMegaVizMode) {
        SkRect r = SkRect::MakeWH(SkIntToScalar(fWidth), SkIntToScalar(fHeight));
        r.outset(SK_Scalar1, SK_Scalar1);

        canvas->save();
        // nuke the CTM
        canvas->setMatrix(SkMatrix::I());
        // turn off clipping
        canvas->clipRect(r, SkRegion::kReplace_Op);

        // visualize existing clips
        SkDebugClipVisitor visitor(canvas);

        canvas->replayClips(&visitor);

        canvas->restore();
    }
    fMatrix = canvas->getTotalMatrix();
    if (!canvas->getClipDeviceBounds(&fClip)) {
        fClip.setEmpty();
    }
    fIndex = index;
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

SkTDArray<SkString*>* SkDebugCanvas::getCommandInfo(int index) {
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

// TODO(chudy): Free command string memory.
SkTArray<SkString>* SkDebugCanvas::getDrawCommandsAsStrings() const {
    SkTArray<SkString>* commandString = new SkTArray<SkString>(fCommandVector.count());
    if (!fCommandVector.isEmpty()) {
        for (int i = 0; i < fCommandVector.count(); i ++) {
            commandString->push_back() = fCommandVector[i]->toString();
        }
    }
    return commandString;
}

void SkDebugCanvas::overrideTexFiltering(bool overrideTexFiltering, SkPaint::FilterLevel level) {
    if (NULL == fTexOverrideFilter) {
        fTexOverrideFilter = new SkTexOverrideFilter;
    }

    fOverrideTexFiltering = overrideTexFiltering;
    fTexOverrideFilter->setFilterLevel(level);
}

void SkDebugCanvas::clear(SkColor color) {
    addDrawCommand(new SkClearCommand(color));
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
    addDrawCommand(new SkConcatCommand(matrix));
    this->INHERITED::didConcat(matrix);
}

void SkDebugCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar left,
                               SkScalar top, const SkPaint* paint = NULL) {
    addDrawCommand(new SkDrawBitmapCommand(bitmap, left, top, paint));
}

void SkDebugCanvas::drawBitmapRectToRect(const SkBitmap& bitmap,
                                         const SkRect* src, const SkRect& dst,
                                         const SkPaint* paint,
                                         SkCanvas::DrawBitmapRectFlags flags) {
    addDrawCommand(new SkDrawBitmapRectCommand(bitmap, src, dst, paint, flags));
}

void SkDebugCanvas::drawBitmapMatrix(const SkBitmap& bitmap,
                                     const SkMatrix& matrix, const SkPaint* paint) {
    addDrawCommand(new SkDrawBitmapMatrixCommand(bitmap, matrix, paint));
}

void SkDebugCanvas::drawBitmapNine(const SkBitmap& bitmap,
        const SkIRect& center, const SkRect& dst, const SkPaint* paint) {
    addDrawCommand(new SkDrawBitmapNineCommand(bitmap, center, dst, paint));
}

void SkDebugCanvas::drawData(const void* data, size_t length) {
    addDrawCommand(new SkDrawDataCommand(data, length));
}

void SkDebugCanvas::beginCommentGroup(const char* description) {
    addDrawCommand(new SkBeginCommentGroupCommand(description));
}

void SkDebugCanvas::addComment(const char* kywd, const char* value) {
    addDrawCommand(new SkCommentCommand(kywd, value));
}

void SkDebugCanvas::endCommentGroup() {
    addDrawCommand(new SkEndCommentGroupCommand());
}

void SkDebugCanvas::drawOval(const SkRect& oval, const SkPaint& paint) {
    addDrawCommand(new SkDrawOvalCommand(oval, paint));
}

void SkDebugCanvas::drawPaint(const SkPaint& paint) {
    addDrawCommand(new SkDrawPaintCommand(paint));
}

void SkDebugCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    addDrawCommand(new SkDrawPathCommand(path, paint));
}

void SkDebugCanvas::drawPicture(SkPicture& picture) {
    addDrawCommand(new SkDrawPictureCommand(picture));
}

void SkDebugCanvas::drawPoints(PointMode mode, size_t count,
                               const SkPoint pts[], const SkPaint& paint) {
    addDrawCommand(new SkDrawPointsCommand(mode, count, pts, paint));
}

void SkDebugCanvas::drawPosText(const void* text, size_t byteLength,
        const SkPoint pos[], const SkPaint& paint) {
    addDrawCommand(new SkDrawPosTextCommand(text, byteLength, pos, paint));
}

void SkDebugCanvas::drawPosTextH(const void* text, size_t byteLength,
        const SkScalar xpos[], SkScalar constY, const SkPaint& paint) {
    addDrawCommand(
        new SkDrawPosTextHCommand(text, byteLength, xpos, constY, paint));
}

void SkDebugCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    // NOTE(chudy): Messing up when renamed to DrawRect... Why?
    addDrawCommand(new SkDrawRectCommand(rect, paint));
}

void SkDebugCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    addDrawCommand(new SkDrawRRectCommand(rrect, paint));
}

void SkDebugCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                 const SkPaint& paint) {
    this->addDrawCommand(new SkDrawDRRectCommand(outer, inner, paint));
}

void SkDebugCanvas::drawSprite(const SkBitmap& bitmap, int left, int top,
                               const SkPaint* paint = NULL) {
    addDrawCommand(new SkDrawSpriteCommand(bitmap, left, top, paint));
}

void SkDebugCanvas::drawText(const void* text, size_t byteLength, SkScalar x,
        SkScalar y, const SkPaint& paint) {
    addDrawCommand(new SkDrawTextCommand(text, byteLength, x, y, paint));
}

void SkDebugCanvas::drawTextOnPath(const void* text, size_t byteLength,
        const SkPath& path, const SkMatrix* matrix, const SkPaint& paint) {
    addDrawCommand(
        new SkDrawTextOnPathCommand(text, byteLength, path, matrix, paint));
}

void SkDebugCanvas::drawVertices(VertexMode vmode, int vertexCount,
        const SkPoint vertices[], const SkPoint texs[], const SkColor colors[],
        SkXfermode*, const uint16_t indices[], int indexCount,
        const SkPaint& paint) {
    addDrawCommand(new SkDrawVerticesCommand(vmode, vertexCount, vertices,
                   texs, colors, NULL, indices, indexCount, paint));
}

void SkDebugCanvas::onPushCull(const SkRect& cullRect) {
    this->addDrawCommand(new SkPushCullCommand(cullRect));
}

void SkDebugCanvas::onPopCull() {
    this->addDrawCommand(new SkPopCullCommand());
}

void SkDebugCanvas::willRestore() {
    this->addDrawCommand(new SkRestoreCommand());
    this->INHERITED::willRestore();
}

void SkDebugCanvas::didRotate(SkScalar degrees) {
    addDrawCommand(new SkRotateCommand(degrees));
    this->INHERITED::didRotate(degrees);
}

void SkDebugCanvas::willSave(SaveFlags flags) {
    this->addDrawCommand(new SkSaveCommand(flags));
    this->INHERITED::willSave(flags);
}

SkCanvas::SaveLayerStrategy SkDebugCanvas::willSaveLayer(const SkRect* bounds, const SkPaint* paint,
                                                         SaveFlags flags) {
    this->addDrawCommand(new SkSaveLayerCommand(bounds, paint, flags));
    this->INHERITED::willSaveLayer(bounds, paint, flags);
    // No need for a full layer.
    return kNoLayer_SaveLayerStrategy;
}

void SkDebugCanvas::didScale(SkScalar sx, SkScalar sy) {
    addDrawCommand(new SkScaleCommand(sx, sy));
    this->INHERITED::didScale(sx, sy);
}

void SkDebugCanvas::didSetMatrix(const SkMatrix& matrix) {
    addDrawCommand(new SkSetMatrixCommand(matrix));
    this->INHERITED::didSetMatrix(matrix);
}

void SkDebugCanvas::didSkew(SkScalar sx, SkScalar sy) {
    addDrawCommand(new SkSkewCommand(sx, sy));
    this->INHERITED::didSkew(sx, sy);
}

void SkDebugCanvas::didTranslate(SkScalar dx, SkScalar dy) {
    addDrawCommand(new SkTranslateCommand(dx, dy));
    this->INHERITED::didTranslate(dx, dy);
}

void SkDebugCanvas::toggleCommand(int index, bool toggle) {
    SkASSERT(index < fCommandVector.count());
    fCommandVector[index]->setVisible(toggle);
}
