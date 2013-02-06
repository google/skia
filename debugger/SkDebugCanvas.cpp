
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

#ifdef SK_BUILD_FOR_WIN
    // iostream includes xlocale which generates warning 4530 because we're compiling without
    // exceptions
    #pragma warning(push)
    #pragma warning(disable : 4530)
#endif
#include <iostream>
#ifdef SK_BUILD_FOR_WIN
    #pragma warning(pop)
#endif

static SkBitmap make_noconfig_bm(int width, int height) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kNo_Config, width, height);
    return bm;
}

SkDebugCanvas::SkDebugCanvas(int width, int height)
        : INHERITED(make_noconfig_bm(width, height))
        , fOutstandingSaveCount(0)
        , fOverdrawViz(false)
        , fOverdrawFilter(NULL) {
    // TODO(chudy): Free up memory from all draw commands in destructor.
    fWidth = width;
    fHeight = height;
    // do we need fBm anywhere?
    fBm.setConfig(SkBitmap::kNo_Config, fWidth, fHeight);
    fFilter = false;
    fIndex = 0;
    fUserMatrix.reset();
}

SkDebugCanvas::~SkDebugCanvas() {
    fCommandVector.deleteAll();
    SkSafeUnref(fOverdrawFilter);
}

void SkDebugCanvas::addDrawCommand(SkDrawCommand* command) {
    fCommandVector.push(command);
}

void SkDebugCanvas::draw(SkCanvas* canvas) {
    if(!fCommandVector.isEmpty()) {
        for (int i = 0; i < fCommandVector.count(); i++) {
            if (fCommandVector[i]->isVisible()) {
                fCommandVector[i]->execute(canvas);
            }
        }
    }
    fIndex = fCommandVector.count() - 1;
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

SkPMColor OverdrawXferModeProc(SkPMColor src, SkPMColor dst) {
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

    for (int i = 0; i < SK_ARRAY_COUNT(gTable)-1; ++i) {
        if (gTable[i] == dst) {
            return gTable[i+1];
        }
    }

    return gTable[SK_ARRAY_COUNT(gTable)-1];
}

// The OverdrawFilter modifies every paint to use an SkProcXfermode which
// in turn invokes OverdrawXferModeProc
class OverdrawFilter : public SkDrawFilter {
public:
    OverdrawFilter() {
        fXferMode = new SkProcXfermode(OverdrawXferModeProc);
    }

    virtual ~OverdrawFilter() {
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

void SkDebugCanvas::drawTo(SkCanvas* canvas, int index) {
    SkASSERT(!fCommandVector.isEmpty());
    SkASSERT(index < fCommandVector.count());
    int i;

    // This only works assuming the canvas and device are the same ones that
    // were previously drawn into because they need to preserve all saves
    // and restores.
    if (fIndex < index) {
        i = fIndex + 1;
    } else {
        for (int j = 0; j < fOutstandingSaveCount; j++) {
            canvas->restore();
        }
        i = 0;
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->resetMatrix();
        SkRect rect = SkRect::MakeWH(SkIntToScalar(fWidth),
                                     SkIntToScalar(fHeight));
        canvas->clipRect(rect, SkRegion::kReplace_Op );
        applyUserTransform(canvas);
        fOutstandingSaveCount = 0;

        // The setting of the draw filter has to go here (rather than in
        // SkRasterWidget) due to the canvas restores this class performs.
        // Since the draw filter is stored in the layer stack if we
        // call setDrawFilter on anything but the root layer odd things happen
        if (fOverdrawViz) {
            if (NULL == fOverdrawFilter) {
                fOverdrawFilter = new OverdrawFilter;
            }

            if (fOverdrawFilter != canvas->getDrawFilter()) {
                canvas->setDrawFilter(fOverdrawFilter);
            }
        } else {
            canvas->setDrawFilter(NULL);
        }
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

SkDrawCommand* SkDebugCanvas::getDrawCommandAt(int index) {
    SkASSERT(index < fCommandVector.count());
    return fCommandVector[index];
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

void SkDebugCanvas::clear(SkColor color) {
    addDrawCommand(new Clear(color));
}

static SkBitmap createBitmap(const SkPath& path) {
    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config,
                     SkDebugCanvas::kVizImageWidth,
                     SkDebugCanvas::kVizImageHeight);
    bitmap.allocPixels();
    bitmap.eraseColor(SK_ColorWHITE);
    SkDevice* device = new SkDevice(bitmap);

    SkCanvas canvas(device);
    device->unref();

    const SkRect& bounds = path.getBounds();

    if (bounds.width() > bounds.height()) {
        canvas.scale(SkDoubleToScalar((0.9*SkDebugCanvas::kVizImageWidth)/bounds.width()),
                     SkDoubleToScalar((0.9*SkDebugCanvas::kVizImageHeight)/bounds.width()));
    } else {
        canvas.scale(SkDoubleToScalar((0.9*SkDebugCanvas::kVizImageWidth)/bounds.height()),
                     SkDoubleToScalar((0.9*SkDebugCanvas::kVizImageHeight)/bounds.height()));
    }
    canvas.translate(-bounds.fLeft+2, -bounds.fTop+2);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas.drawPath(path, p);

    return bitmap;
}

static SkBitmap createBitmap(const SkBitmap& input, const SkRect* srcRect) {
    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config,
                     SkDebugCanvas::kVizImageWidth,
                     SkDebugCanvas::kVizImageHeight);
    bitmap.allocPixels();
    bitmap.eraseColor(SK_ColorLTGRAY);
    SkDevice* device = new SkDevice(bitmap);

    SkCanvas canvas(device);
    device->unref();

    SkScalar xScale = SkIntToScalar(SkDebugCanvas::kVizImageWidth-2) / input.width();
    SkScalar yScale = SkIntToScalar(SkDebugCanvas::kVizImageHeight-2) / input.height();

    if (input.width() > input.height()) {
        yScale *= input.height() / (float) input.width();
    } else {
        xScale *= input.width() / (float) input.height();
    }

    SkRect dst = SkRect::MakeXYWH(SK_Scalar1, SK_Scalar1,
                                  xScale * input.width(),
                                  yScale * input.height());

    canvas.drawBitmapRect(input, NULL, dst);

    if (NULL != srcRect) {
        SkRect r = SkRect::MakeLTRB(srcRect->fLeft * xScale + SK_Scalar1,
                                    srcRect->fTop * yScale + SK_Scalar1,
                                    srcRect->fRight * xScale + SK_Scalar1,
                                    srcRect->fBottom * yScale + SK_Scalar1);
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setStyle(SkPaint::kStroke_Style);

        canvas.drawRect(r, p);
    }

    return bitmap;
}

bool SkDebugCanvas::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    SkBitmap bitmap = createBitmap(path);
    addDrawCommand(new ClipPath(path, op, doAA, bitmap));
    return true;
}

bool SkDebugCanvas::clipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    addDrawCommand(new ClipRect(rect, op, doAA));
    return true;
}

bool SkDebugCanvas::clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    addDrawCommand(new ClipRRect(rrect, op, doAA));
    return true;
}

bool SkDebugCanvas::clipRegion(const SkRegion& region, SkRegion::Op op) {
    addDrawCommand(new ClipRegion(region, op));
    return true;
}

bool SkDebugCanvas::concat(const SkMatrix& matrix) {
    addDrawCommand(new Concat(matrix));
    return true;
}

void SkDebugCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar left,
        SkScalar top, const SkPaint* paint = NULL) {
    SkBitmap resizedBitmap = createBitmap(bitmap, NULL);
    addDrawCommand(new DrawBitmap(bitmap, left, top, paint, resizedBitmap));
}

void SkDebugCanvas::drawBitmapRectToRect(const SkBitmap& bitmap,
        const SkRect* src, const SkRect& dst, const SkPaint* paint) {
    SkBitmap resizedBitmap = createBitmap(bitmap, src);
    addDrawCommand(new DrawBitmapRect(bitmap, src, dst, paint, resizedBitmap));
}

void SkDebugCanvas::drawBitmapMatrix(const SkBitmap& bitmap,
        const SkMatrix& matrix, const SkPaint* paint) {
    SkBitmap resizedBitmap = createBitmap(bitmap, NULL);
    addDrawCommand(new DrawBitmapMatrix(bitmap, matrix, paint, resizedBitmap));
}

void SkDebugCanvas::drawBitmapNine(const SkBitmap& bitmap,
        const SkIRect& center, const SkRect& dst, const SkPaint* paint) {
    SkBitmap resizedBitmap = createBitmap(bitmap, NULL);
    addDrawCommand(new DrawBitmapNine(bitmap, center, dst, paint, resizedBitmap));
}

void SkDebugCanvas::drawData(const void* data, size_t length) {
    addDrawCommand(new DrawData(data, length));
}

void SkDebugCanvas::drawOval(const SkRect& oval, const SkPaint& paint) {
    addDrawCommand(new DrawOval(oval, paint));
}

void SkDebugCanvas::drawPaint(const SkPaint& paint) {
    addDrawCommand(new DrawPaint(paint));
}

void SkDebugCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    SkBitmap bitmap = createBitmap(path);
    addDrawCommand(new DrawPath(path, paint, bitmap));
}

void SkDebugCanvas::drawPicture(SkPicture& picture) {
    addDrawCommand(new DrawPicture(picture));
}

void SkDebugCanvas::drawPoints(PointMode mode, size_t count,
        const SkPoint pts[], const SkPaint& paint) {
    addDrawCommand(new DrawPoints(mode, count, pts, paint));
}

void SkDebugCanvas::drawPosText(const void* text, size_t byteLength,
        const SkPoint pos[], const SkPaint& paint) {
    addDrawCommand(new DrawPosText(text, byteLength, pos, paint));
}

void SkDebugCanvas::drawPosTextH(const void* text, size_t byteLength,
        const SkScalar xpos[], SkScalar constY, const SkPaint& paint) {
    addDrawCommand(new DrawPosTextH(text, byteLength, xpos, constY, paint));
}

void SkDebugCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    // NOTE(chudy): Messing up when renamed to DrawRect... Why?
    addDrawCommand(new DrawRectC(rect, paint));
}

void SkDebugCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    addDrawCommand(new DrawRRect(rrect, paint));
}

void SkDebugCanvas::drawSprite(const SkBitmap& bitmap, int left, int top,
        const SkPaint* paint = NULL) {
    SkBitmap resizedBitmap = createBitmap(bitmap, NULL);
    addDrawCommand(new DrawSprite(bitmap, left, top, paint, resizedBitmap));
}

void SkDebugCanvas::drawText(const void* text, size_t byteLength, SkScalar x,
        SkScalar y, const SkPaint& paint) {
    addDrawCommand(new DrawTextC(text, byteLength, x, y, paint));
}

void SkDebugCanvas::drawTextOnPath(const void* text, size_t byteLength,
        const SkPath& path, const SkMatrix* matrix, const SkPaint& paint) {
    addDrawCommand(new DrawTextOnPath(text, byteLength, path, matrix, paint));
}

void SkDebugCanvas::drawVertices(VertexMode vmode, int vertexCount,
        const SkPoint vertices[], const SkPoint texs[], const SkColor colors[],
        SkXfermode*, const uint16_t indices[], int indexCount,
        const SkPaint& paint) {
    addDrawCommand(new DrawVertices(vmode, vertexCount, vertices, texs, colors,
            NULL, indices, indexCount, paint));
}

void SkDebugCanvas::restore() {
    addDrawCommand(new Restore());
}

bool SkDebugCanvas::rotate(SkScalar degrees) {
    addDrawCommand(new Rotate(degrees));
    return true;
}

int SkDebugCanvas::save(SaveFlags flags) {
    addDrawCommand(new Save(flags));
    return true;
}

int SkDebugCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
        SaveFlags flags) {
    addDrawCommand(new SaveLayer(bounds, paint, flags));
    return true;
}

bool SkDebugCanvas::scale(SkScalar sx, SkScalar sy) {
    addDrawCommand(new Scale(sx, sy));
    return true;
}

void SkDebugCanvas::setMatrix(const SkMatrix& matrix) {
    addDrawCommand(new SetMatrix(matrix));
}

bool SkDebugCanvas::skew(SkScalar sx, SkScalar sy) {
    addDrawCommand(new Skew(sx, sy));
    return true;
}

bool SkDebugCanvas::translate(SkScalar dx, SkScalar dy) {
    addDrawCommand(new Translate(dx, dy));
    return true;
}

void SkDebugCanvas::toggleCommand(int index, bool toggle) {
    SkASSERT(index < fCommandVector.count());
    fCommandVector[index]->setVisible(toggle);
}
