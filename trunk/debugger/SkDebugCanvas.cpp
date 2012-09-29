
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <iostream>
#include "SkDebugCanvas.h"
#include "SkDrawCommand.h"

SkDebugCanvas::SkDebugCanvas(int width, int height) {
    // TODO(chudy): Free up memory from all draw commands in destructor.
    fWidth = width;
    fHeight = height;
    fBm.setConfig(SkBitmap::kNo_Config, fWidth, fHeight);
    this->setBitmapDevice(fBm);
    fFilter = false;
    fIndex = 0;
    fUserOffset.set(0,0);
    fUserScale = 1.0;
}

SkDebugCanvas::~SkDebugCanvas() {
    commandVector.deleteAll();
}

void SkDebugCanvas::addDrawCommand(SkDrawCommand* command) {
    commandVector.push(command);
}

void SkDebugCanvas::draw(SkCanvas* canvas) {
    if(!commandVector.isEmpty()) {
        for (int i = 0; i < commandVector.count(); i++) {
            if (commandVector[i]->isVisible()) {
                commandVector[i]->execute(canvas);
            }
        }
    }
    fIndex = commandVector.count() - 1;
}

void SkDebugCanvas::applyUserTransform(SkCanvas* canvas) {
    canvas->translate(SkIntToScalar(fUserOffset.fX),
                      SkIntToScalar(fUserOffset.fY));
    if (fUserScale < 0) {
        canvas->scale((1.0f / -fUserScale), (1.0f / -fUserScale));
    } else if (fUserScale > 0) {
        canvas->scale(fUserScale, fUserScale);
    }
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
        if (commandVector[i]->isVisible()) {
            commandVector[i]->execute(&canvas);
        }
        if (prev != bitmap.getColor(0,0)) {
            layer = i;
        }
        prev = bitmap.getColor(0,0);
    }
    return layer;
}

void SkDebugCanvas::drawTo(SkCanvas* canvas, int index) {
    int counter = 0;
    SkASSERT(!commandVector.isEmpty());
    SkASSERT(index < commandVector.count());
    int i;

    // This only works assuming the canvas and device are the same ones that
    // were previously drawn into because they need to preserve all saves
    // and restores.
    if (fIndex < index) {
        i = fIndex + 1;
    } else {
        i = 0;
        canvas->clear(0);
        canvas->resetMatrix();
        SkRect rect = SkRect::MakeWH(SkIntToScalar(fWidth),
                                     SkIntToScalar(fHeight));
        canvas->clipRect(rect, SkRegion::kReplace_Op );
        applyUserTransform(canvas);
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

        if (commandVector[i]->isVisible()) {
            commandVector[i]->execute(canvas);
        }
    }
    fMatrix = canvas->getTotalMatrix();
    fClip = canvas->getTotalClip().getBounds();
    fIndex = index;
}

SkDrawCommand* SkDebugCanvas::getDrawCommandAt(int index) {
    SkASSERT(index < commandVector.count());
    return commandVector[index];
}

SkTDArray<SkString*>* SkDebugCanvas::getCommandInfo(int index) {
    SkASSERT(index < commandVector.count());
    return commandVector[index]->Info();
}

bool SkDebugCanvas::getDrawCommandVisibilityAt(int index) {
    SkASSERT(index < commandVector.count());
    return commandVector[index]->isVisible();
}

SkTDArray <SkDrawCommand*> SkDebugCanvas::getDrawCommands() {
    return commandVector;
}

// TODO(chudy): Free command string memory.
SkTDArray<SkString*>* SkDebugCanvas::getDrawCommandsAsStrings() {
    SkTDArray<SkString*>* commandString = new SkTDArray<SkString*>();
    if (!commandVector.isEmpty()) {
        for (int i = 0; i < commandVector.count(); i ++) {
            commandString->push(new SkString(commandVector[i]->toString()));
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

bool SkDebugCanvas::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    addDrawCommand(new ClipPath(path, op, doAA));
    return true;
}

bool SkDebugCanvas::clipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    addDrawCommand(new ClipRect(rect, op, doAA));
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
    addDrawCommand(new DrawBitmap(bitmap, left, top, paint));
}

void SkDebugCanvas::drawBitmapRect(const SkBitmap& bitmap,
        const SkIRect* src, const SkRect& dst, const SkPaint* paint) {
    addDrawCommand(new DrawBitmapRect(bitmap, src, dst, paint));
}

void SkDebugCanvas::drawBitmapMatrix(const SkBitmap& bitmap,
        const SkMatrix& matrix, const SkPaint* paint) {
    addDrawCommand(new DrawBitmapMatrix(bitmap, matrix, paint));
}

void SkDebugCanvas::drawBitmapNine(const SkBitmap& bitmap,
        const SkIRect& center, const SkRect& dst, const SkPaint* paint) {
    addDrawCommand(new DrawBitmapNine(bitmap, center, dst, paint));
}

void SkDebugCanvas::drawData(const void* data, size_t length) {
    addDrawCommand(new DrawData(data, length));
}

void SkDebugCanvas::drawPaint(const SkPaint& paint) {
    addDrawCommand(new DrawPaint(paint));
}

void SkDebugCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    addDrawCommand(new DrawPath(path, paint));
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

void SkDebugCanvas::drawSprite(const SkBitmap& bitmap, int left, int top,
        const SkPaint* paint = NULL) {
    addDrawCommand(new DrawSprite(bitmap, left, top, paint));
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
    SkASSERT(index < commandVector.count());
    commandVector[index]->setVisible(toggle);
}
