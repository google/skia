
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <iostream>
#include "SkDebugCanvas.h"
#include "SkDrawCommand.h"

SkDebugCanvas::SkDebugCanvas() {
    // TODO(chudy): Free up memory from all draw commands in destructor.
    fWidth = 100;
    fHeight = 100;
    fBm.setConfig(SkBitmap::kNo_Config, fWidth, fHeight);
    this->setBitmapDevice(fBm);
    fFilter = false;
}

SkDebugCanvas::~SkDebugCanvas() {}

void SkDebugCanvas::addDrawCommand(SkDrawCommand* command) {
    commandVector.push_back(command);
}

void SkDebugCanvas::draw(SkCanvas* canvas) {
    if(!commandVector.empty()) {
        for(it = commandVector.begin(); it != commandVector.end(); ++it) {
            (*it)->execute(canvas);
        }
    }
}

void SkDebugCanvas::drawTo(SkCanvas* canvas, int index, SkBitmap* bitmap) {
    int counter = 0;
    if(!commandVector.empty()) {
        for(it = commandVector.begin(); it != commandVector.end(); ++it) {
            if (counter != (index-1)) {
                 if ((*it)->getVisibility()) {
                     (*it)->execute(canvas);
                 }
             } else {
                 if (fFilter) {
                     SkPaint* p = new SkPaint();
                     p->setColor(0xAAFFFFFF);
                     canvas->save();
                     canvas->resetMatrix();
                     SkRect dump;
                     // TODO(chudy): Replace with a call to QtWidget to get dimensions.
                     dump.set(SkIntToScalar(0), SkIntToScalar(0), SkIntToScalar(fWidth), SkIntToScalar(fHeight));
                     canvas->clipRect(dump,  SkRegion::kReplace_Op, false );
                     canvas->drawRectCoords(SkIntToScalar(0),SkIntToScalar(0),SkIntToScalar(fWidth),SkIntToScalar(fHeight), *p);
                     canvas->restore();
                 }
                 if ((*it)->getVisibility()) {
                     (*it)->execute(canvas);
                 }
             }
            if (fCalculateHits == true && bitmap != NULL) {
                fHitBox.updateHitPoint(bitmap, counter);
            }

            /* TODO(chudy): Implement a bitmap wide function that will take
             *  ~50 out of each R,G,B. This will make everything but the last
             *  command brighter.
             */
            if (++counter == index) return;
        }
    }
}

SkDrawCommand* SkDebugCanvas::getDrawCommandAt(int index) {
    SkASSERT(index < commandVector.size());
    return commandVector[index];
}

std::vector<std::string>* SkDebugCanvas::getCommandInfoAt(int index) {
    SkASSERT(index < commandVector.size());
    return commandVector[index]->Info();
}

bool SkDebugCanvas::getDrawCommandVisibilityAt(int index) {
    SkASSERT(index < commandVector.size());
    return commandVector[index]->getVisibility();
}

std::vector<SkDrawCommand*> SkDebugCanvas::getDrawCommands() {
    return commandVector;
}

// TODO(chudy): Free command string memory.
std::vector<std::string>* SkDebugCanvas::getDrawCommandsAsStrings() {
    std::vector<std::string>* commandString = new std::vector<std::string>();
    if (!commandVector.empty()) {
        for(it = commandVector.begin(); it != commandVector.end(); ++it) {
            commandString->push_back((*it)->toString());
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
    SkASSERT(index < commandVector.size());
    commandVector[index]->setVisibility(toggle);
}
