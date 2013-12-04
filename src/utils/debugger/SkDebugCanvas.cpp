
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

static SkBitmap make_noconfig_bm(int width, int height) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kNo_Config, width, height);
    return bm;
}

SkDebugCanvas::SkDebugCanvas(int width, int height)
        : INHERITED(make_noconfig_bm(width, height))
        , fWidth(width)
        , fHeight(height)
        , fFilter(false)
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
    INHERITED::clipRect(large, SkRegion::kReplace_Op, false);
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
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, 1, 1);
    bitmap.allocPixels();

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
        fXferMode = new SkProcXfermode(OverdrawXferModeProc);
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

void SkDebugCanvas::drawTo(SkCanvas* canvas, int index) {
    SkASSERT(!fCommandVector.isEmpty());
    SkASSERT(index < fCommandVector.count());
    int i = 0;

    // This only works assuming the canvas and device are the same ones that
    // were previously drawn into because they need to preserve all saves
    // and restores.
    // The visibility filter also requires a full re-draw - otherwise we can
    // end up drawing the filter repeatedly.
    if (fIndex < index && !fFilter) {
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
            fCommandVector[i]->execute(canvas);
            fCommandVector[i]->trackSaveState(&fOutstandingSaveCount);
        }
    }
    fMatrix = canvas->getTotalMatrix();
    fClip = canvas->getTotalClip().getBounds();
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

void SkDebugCanvas::toggleFilter(bool toggle) {
    fFilter = toggle;
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

bool SkDebugCanvas::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    addDrawCommand(new SkClipPathCommand(path, op, doAA));
    return true;
}

bool SkDebugCanvas::clipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    addDrawCommand(new SkClipRectCommand(rect, op, doAA));
    return true;
}

bool SkDebugCanvas::clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    addDrawCommand(new SkClipRRectCommand(rrect, op, doAA));
    return true;
}

bool SkDebugCanvas::clipRegion(const SkRegion& region, SkRegion::Op op) {
    addDrawCommand(new SkClipRegionCommand(region, op));
    return true;
}

bool SkDebugCanvas::concat(const SkMatrix& matrix) {
    addDrawCommand(new SkConcatCommand(matrix));
    return true;
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

void SkDebugCanvas::restore() {
    addDrawCommand(new SkRestoreCommand());
}

bool SkDebugCanvas::rotate(SkScalar degrees) {
    addDrawCommand(new SkRotateCommand(degrees));
    return true;
}

int SkDebugCanvas::save(SaveFlags flags) {
    addDrawCommand(new SkSaveCommand(flags));
    return true;
}

int SkDebugCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
        SaveFlags flags) {
    addDrawCommand(new SkSaveLayerCommand(bounds, paint, flags));
    return true;
}

bool SkDebugCanvas::scale(SkScalar sx, SkScalar sy) {
    addDrawCommand(new SkScaleCommand(sx, sy));
    return true;
}

void SkDebugCanvas::setMatrix(const SkMatrix& matrix) {
    addDrawCommand(new SkSetMatrixCommand(matrix));
}

bool SkDebugCanvas::skew(SkScalar sx, SkScalar sy) {
    addDrawCommand(new SkSkewCommand(sx, sy));
    return true;
}

bool SkDebugCanvas::translate(SkScalar dx, SkScalar dy) {
    addDrawCommand(new SkTranslateCommand(dx, dy));
    return true;
}

void SkDebugCanvas::toggleCommand(int index, bool toggle) {
    SkASSERT(index < fCommandVector.count());
    fCommandVector[index]->setVisible(toggle);
}
