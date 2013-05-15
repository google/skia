/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLuaCanvas.h"

extern "C" {
    #include "lua.h"
}

void SkLuaCanvas::sendverb(const char str[]) {
    lua_getglobal(fL, fFunc.c_str());
    if (!lua_isfunction(fL, -1)) {
        int t = lua_type(fL, -1);
        SkDebugf("--- expected function %d\n", t);
    }
    lua_pushstring(fL, str);

    if (lua_pcall(fL, 1, 0, 0) != LUA_OK) {
        SkDebugf("lua err: %s\n", lua_tostring(fL, -1));
    }
    lua_settop(fL, -1);
}

///////////////////////////////////////////////////////////////////////////////

static SkBitmap make_bm(int width, int height) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kNo_Config, width, height);
    return bm;
}

SkLuaCanvas::SkLuaCanvas(int width, int height, lua_State* L, const char func[])
    : INHERITED(make_bm(width, height))
    , fL(L)
    , fFunc(func) {
}

SkLuaCanvas::~SkLuaCanvas() {}

int SkLuaCanvas::save(SaveFlags flags) {
    sendverb("save");
    return this->INHERITED::save(flags);
}

int SkLuaCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
                             SaveFlags flags) {
    sendverb("saveLayer");
    return this->INHERITED::save(flags);
}

void SkLuaCanvas::restore() {
    sendverb("restore");
    this->INHERITED::restore();
}

bool SkLuaCanvas::translate(SkScalar dx, SkScalar dy) {
    sendverb("translate");
    return this->INHERITED::translate(dx, dy);
}

bool SkLuaCanvas::scale(SkScalar sx, SkScalar sy) {
    sendverb("scale");
    return this->INHERITED::scale(sx, sy);
}

bool SkLuaCanvas::rotate(SkScalar degrees) {
    sendverb("rotate");
    return this->INHERITED::rotate(degrees);
}

bool SkLuaCanvas::skew(SkScalar sx, SkScalar sy) {
    sendverb("skew");
    return this->INHERITED::skew(sx, sy);
}

bool SkLuaCanvas::concat(const SkMatrix& matrix) {
    sendverb("concat");
    return this->INHERITED::concat(matrix);
}

void SkLuaCanvas::setMatrix(const SkMatrix& matrix) {
    this->INHERITED::setMatrix(matrix);
}

bool SkLuaCanvas::clipRect(const SkRect& r, SkRegion::Op op, bool doAA) {
    sendverb("clipRect");
    return this->INHERITED::clipRect(r, op, doAA);
}

bool SkLuaCanvas::clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    sendverb("clipRRect");
    return this->INHERITED::clipRRect(rrect, op, doAA);
}

bool SkLuaCanvas::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    sendverb("clipPath");
    return this->INHERITED::clipPath(path, op, doAA);
}

bool SkLuaCanvas::clipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    sendverb("clipRegion");
    return this->INHERITED::clipRegion(deviceRgn, op);
}

void SkLuaCanvas::drawPaint(const SkPaint& paint) {
    sendverb("drawPaint");
}

void SkLuaCanvas::drawPoints(PointMode mode, size_t count,
                               const SkPoint pts[], const SkPaint& paint) {
    sendverb("drawPoints");
}

void SkLuaCanvas::drawOval(const SkRect& rect, const SkPaint& paint) {
    sendverb("drawOval");
}

void SkLuaCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    sendverb("drawRect");
}

void SkLuaCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    sendverb("drawRRect");
}

void SkLuaCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    sendverb("drawPath");
}

void SkLuaCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                               const SkPaint* paint) {
    sendverb("drawBitmap");
}

void SkLuaCanvas::drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                   const SkRect& dst, const SkPaint* paint) {
    sendverb("drawBitmapRectToRect");
}

void SkLuaCanvas::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                     const SkPaint* paint) {
    sendverb("drawBitmapMatrix");
}

void SkLuaCanvas::drawSprite(const SkBitmap& bitmap, int x, int y,
                               const SkPaint* paint) {
    sendverb("drawSprite");
}

void SkLuaCanvas::drawText(const void* text, size_t byteLength, SkScalar x,
                             SkScalar y, const SkPaint& paint) {
    sendverb("drawText");
}

void SkLuaCanvas::drawPosText(const void* text, size_t byteLength,
                                const SkPoint pos[], const SkPaint& paint) {
    sendverb("drawPosText");
}

void SkLuaCanvas::drawPosTextH(const void* text, size_t byteLength,
                                 const SkScalar xpos[], SkScalar constY,
                                 const SkPaint& paint) {
    sendverb("drawPosTextH");
}

void SkLuaCanvas::drawTextOnPath(const void* text, size_t byteLength,
                                   const SkPath& path, const SkMatrix* matrix,
                                   const SkPaint& paint) {
    sendverb("drawTextOnPath");
}

void SkLuaCanvas::drawPicture(SkPicture& picture) {
    sendverb("drawPicture");
    // call through so we can see the nested picture ops
    this->INHERITED::drawPicture(picture);
}

void SkLuaCanvas::drawVertices(VertexMode vmode, int vertexCount,
                                 const SkPoint vertices[], const SkPoint texs[],
                                 const SkColor colors[], SkXfermode* xmode,
                                 const uint16_t indices[], int indexCount,
                                 const SkPaint& paint) {
    sendverb("drawVertices");
}

void SkLuaCanvas::drawData(const void* data, size_t length) {
    sendverb("drawData");
}
