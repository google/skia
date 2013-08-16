/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLuaCanvas.h"
#include "SkLua.h"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
}

class AutoCallLua : public SkLua {
public:
    AutoCallLua(lua_State* L, const char func[], const char verb[]) : INHERITED(L) {
        lua_getglobal(L, func);
        if (!lua_isfunction(L, -1)) {
            int t = lua_type(L, -1);
            SkDebugf("--- expected function %d\n", t);
        }

        lua_newtable(L);
        this->pushString(verb, "verb");
    }

    ~AutoCallLua() {
        lua_State* L = this->get();
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            SkDebugf("lua err: %s\n", lua_tostring(L, -1));
        }
        lua_settop(L, -1);
    }

    void pushEncodedText(SkPaint::TextEncoding, const void*, size_t);

private:
    typedef SkLua INHERITED;
};

#define AUTO_LUA(verb)  AutoCallLua lua(fL, fFunc.c_str(), verb)


///////////////////////////////////////////////////////////////////////////////

void AutoCallLua::pushEncodedText(SkPaint::TextEncoding enc, const void* text,
                                  size_t length) {
    switch (enc) {
        case SkPaint::kUTF8_TextEncoding:
            this->pushString((const char*)text, length, "text");
            break;
        case SkPaint::kUTF16_TextEncoding: {
            SkString str;
            str.setUTF16((const uint16_t*)text, length);
            this->pushString(str, "text");
        } break;
        case SkPaint::kGlyphID_TextEncoding:
            this->pushArrayU16((const uint16_t*)text, length >> 1, "glyphs");
            break;
        case SkPaint::kUTF32_TextEncoding:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkLuaCanvas::pushThis() {
    SkLua(fL).pushCanvas(this);
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
    AUTO_LUA("save");
    return this->INHERITED::save(flags);
}

int SkLuaCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
                             SaveFlags flags) {
    AUTO_LUA("saveLayer");
    if (bounds) {
        lua.pushRect(*bounds, "bounds");
    }
    if (paint) {
        lua.pushPaint(*paint, "paint");
    }
    return this->INHERITED::save(flags);
}

void SkLuaCanvas::restore() {
    AUTO_LUA("restore");
    this->INHERITED::restore();
}

bool SkLuaCanvas::translate(SkScalar dx, SkScalar dy) {
    AUTO_LUA("translate");
    lua.pushScalar(dx, "dx");
    lua.pushScalar(dy, "dy");
    return this->INHERITED::translate(dx, dy);
}

bool SkLuaCanvas::scale(SkScalar sx, SkScalar sy) {
    AUTO_LUA("scale");
    lua.pushScalar(sx, "sx");
    lua.pushScalar(sy, "sy");
    return this->INHERITED::scale(sx, sy);
}

bool SkLuaCanvas::rotate(SkScalar degrees) {
    AUTO_LUA("rotate");
    lua.pushScalar(degrees, "degrees");
    return this->INHERITED::rotate(degrees);
}

bool SkLuaCanvas::skew(SkScalar kx, SkScalar ky) {
    AUTO_LUA("skew");
    lua.pushScalar(kx, "kx");
    lua.pushScalar(ky, "ky");
    return this->INHERITED::skew(kx, ky);
}

bool SkLuaCanvas::concat(const SkMatrix& matrix) {
    AUTO_LUA("concat");
    return this->INHERITED::concat(matrix);
}

void SkLuaCanvas::setMatrix(const SkMatrix& matrix) {
    this->INHERITED::setMatrix(matrix);
}

bool SkLuaCanvas::clipRect(const SkRect& r, SkRegion::Op op, bool doAA) {
    AUTO_LUA("clipRect");
    lua.pushRect(r, "rect");
    lua.pushBool(doAA, "aa");
    return this->INHERITED::clipRect(r, op, doAA);
}

bool SkLuaCanvas::clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    AUTO_LUA("clipRRect");
    lua.pushRRect(rrect, "rrect");
    lua.pushBool(doAA, "aa");
    return this->INHERITED::clipRRect(rrect, op, doAA);
}

bool SkLuaCanvas::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    AUTO_LUA("clipPath");
    lua.pushPath(path, "path");
    lua.pushBool(doAA, "aa");
    return this->INHERITED::clipPath(path, op, doAA);
}

bool SkLuaCanvas::clipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    AUTO_LUA("clipRegion");
    return this->INHERITED::clipRegion(deviceRgn, op);
}

void SkLuaCanvas::drawPaint(const SkPaint& paint) {
    AUTO_LUA("drawPaint");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawPoints(PointMode mode, size_t count,
                               const SkPoint pts[], const SkPaint& paint) {
    AUTO_LUA("drawPoints");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawOval(const SkRect& rect, const SkPaint& paint) {
    AUTO_LUA("drawOval");
    lua.pushRect(rect, "rect");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    AUTO_LUA("drawRect");
    lua.pushRect(rect, "rect");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    AUTO_LUA("drawRRect");
    lua.pushRRect(rrect, "rrect");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    AUTO_LUA("drawPath");
    lua.pushPath(path, "path");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                             const SkPaint* paint) {
    AUTO_LUA("drawBitmap");
    if (paint) {
        lua.pushPaint(*paint, "paint");
    }
}

void SkLuaCanvas::drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                       const SkRect& dst, const SkPaint* paint,
                                       DrawBitmapRectFlags flags) {
    AUTO_LUA("drawBitmapRectToRect");
    if (paint) {
        lua.pushPaint(*paint, "paint");
    }
}

void SkLuaCanvas::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                   const SkPaint* paint) {
    AUTO_LUA("drawBitmapMatrix");
    if (paint) {
        lua.pushPaint(*paint, "paint");
    }
}

void SkLuaCanvas::drawSprite(const SkBitmap& bitmap, int x, int y,
                               const SkPaint* paint) {
    AUTO_LUA("drawSprite");
    if (paint) {
        lua.pushPaint(*paint, "paint");
    }
}

void SkLuaCanvas::drawText(const void* text, size_t byteLength, SkScalar x,
                             SkScalar y, const SkPaint& paint) {
    AUTO_LUA("drawText");
    lua.pushEncodedText(paint.getTextEncoding(), text, byteLength);
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawPosText(const void* text, size_t byteLength,
                                const SkPoint pos[], const SkPaint& paint) {
    AUTO_LUA("drawPosText");
    lua.pushEncodedText(paint.getTextEncoding(), text, byteLength);
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawPosTextH(const void* text, size_t byteLength,
                                 const SkScalar xpos[], SkScalar constY,
                                 const SkPaint& paint) {
    AUTO_LUA("drawPosTextH");
    lua.pushEncodedText(paint.getTextEncoding(), text, byteLength);
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawTextOnPath(const void* text, size_t byteLength,
                                   const SkPath& path, const SkMatrix* matrix,
                                   const SkPaint& paint) {
    AUTO_LUA("drawTextOnPath");
    lua.pushPath(path, "path");
    lua.pushEncodedText(paint.getTextEncoding(), text, byteLength);
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawPicture(SkPicture& picture) {
    AUTO_LUA("drawPicture");
    // call through so we can see the nested picture ops
    this->INHERITED::drawPicture(picture);
}

void SkLuaCanvas::drawVertices(VertexMode vmode, int vertexCount,
                                 const SkPoint vertices[], const SkPoint texs[],
                                 const SkColor colors[], SkXfermode* xmode,
                                 const uint16_t indices[], int indexCount,
                                 const SkPaint& paint) {
    AUTO_LUA("drawVertices");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawData(const void* data, size_t length) {
    AUTO_LUA("drawData");
}
