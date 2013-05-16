/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLuaCanvas.h"
#include "SkRRect.h"

extern "C" {
    #include "lua.h"
}

static void setfield_string(lua_State* L, const char key[], const char value[]) {
    lua_pushstring(L, value);
    lua_setfield(L, -2, key);
}

static void setfield_number(lua_State* L, const char key[], double value) {
    lua_pushnumber(L, value);
    lua_setfield(L, -2, key);
}

static void setfield_bool(lua_State* L, const char key[], bool value) {
    lua_pushboolean(L, value);
    lua_setfield(L, -2, key);
}

static void setfield_rect(lua_State* L, const char key[], const SkRect& r) {
    lua_newtable(L);
    setfield_number(L, "left", r.fLeft);
    setfield_number(L, "top", r.fTop);
    setfield_number(L, "right", r.fRight);
    setfield_number(L, "bottom", r.fBottom);
    lua_setfield(L, -2, key);
}

enum PaintUsage {
    kText_PaintUsage,
    kImage_PaintUsage,
    kGeometry_PaintUsage
};

static const char* color2string(SkColor c, SkString* str) {
    str->printf("0x%08X", c);
    return str->c_str();
}

static void setfield_paint(lua_State* L, const SkPaint& p,
                           PaintUsage pu = kGeometry_PaintUsage) {
    SkString str;

    lua_newtable(L);
    setfield_bool(L, "aa", p.isAntiAlias());
    setfield_string(L, "color", color2string(p.getColor(), &str));

    if (kGeometry_PaintUsage == pu) {
        if (SkPaint::kFill_Style != p.getStyle()) {
            setfield_number(L, "stroke-width", p.getStrokeWidth());
        }
    }
    lua_setfield(L, -2, "paint");
}

class AutoCallLua {
public:
    AutoCallLua(lua_State* L, const char func[], const char verb[]) : fL(L) {
        lua_getglobal(L, func);
        if (!lua_isfunction(L, -1)) {
            int t = lua_type(L, -1);
            SkDebugf("--- expected function %d\n", t);
        }
        
        lua_newtable(L);
        setfield_string(L, "verb", verb);
    }

    ~AutoCallLua() {
        if (lua_pcall(fL, 1, 0, 0) != LUA_OK) {
            SkDebugf("lua err: %s\n", lua_tostring(fL, -1));
        }
        lua_settop(fL, -1);
    }

private:
    lua_State* fL;
};

#define AUTO_LUA(verb)  AutoCallLua acl(fL, fFunc.c_str(), verb)

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
        setfield_rect(fL, "bounds", *bounds);
    }
    if (paint) {
        setfield_paint(fL, *paint);
    }
    return this->INHERITED::save(flags);
}

void SkLuaCanvas::restore() {
    AUTO_LUA("restore");
    this->INHERITED::restore();
}

bool SkLuaCanvas::translate(SkScalar dx, SkScalar dy) {
    AUTO_LUA("translate");
    setfield_number(fL, "dx", dx);
    setfield_number(fL, "dy", dy);
    return this->INHERITED::translate(dx, dy);
}

bool SkLuaCanvas::scale(SkScalar sx, SkScalar sy) {
    AUTO_LUA("scale");
    setfield_number(fL, "sx", sx);
    setfield_number(fL, "sy", sy);
    return this->INHERITED::scale(sx, sy);
}

bool SkLuaCanvas::rotate(SkScalar degrees) {
    AUTO_LUA("rotate");
    setfield_number(fL, "degrees", degrees);
    return this->INHERITED::rotate(degrees);
}

bool SkLuaCanvas::skew(SkScalar sx, SkScalar sy) {
    AUTO_LUA("skew");
    setfield_number(fL, "sx", sx);
    setfield_number(fL, "sy", sy);
    return this->INHERITED::skew(sx, sy);
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
    setfield_rect(fL, "rect", r);
    setfield_bool(fL, "aa", doAA);
    return this->INHERITED::clipRect(r, op, doAA);
}

bool SkLuaCanvas::clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    AUTO_LUA("clipRRect");
    setfield_bool(fL, "aa", doAA);
    return this->INHERITED::clipRRect(rrect, op, doAA);
}

bool SkLuaCanvas::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    AUTO_LUA("clipPath");
    setfield_bool(fL, "aa", doAA);
    return this->INHERITED::clipPath(path, op, doAA);
}

bool SkLuaCanvas::clipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    AUTO_LUA("clipRegion");
    return this->INHERITED::clipRegion(deviceRgn, op);
}

void SkLuaCanvas::drawPaint(const SkPaint& paint) {
    AUTO_LUA("drawPaint");
    setfield_paint(fL, paint);
}

void SkLuaCanvas::drawPoints(PointMode mode, size_t count,
                               const SkPoint pts[], const SkPaint& paint) {
    AUTO_LUA("drawPoints");
    setfield_paint(fL, paint);
}

void SkLuaCanvas::drawOval(const SkRect& rect, const SkPaint& paint) {
    AUTO_LUA("drawOval");
    setfield_rect(fL, "rect", rect);
    setfield_paint(fL, paint);
}

void SkLuaCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    AUTO_LUA("drawRect");
    setfield_rect(fL, "rect", rect);
    setfield_paint(fL, paint);
}

void SkLuaCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    AUTO_LUA("drawRRect");
    setfield_rect(fL, "rect", rrect.getBounds());
    setfield_paint(fL, paint);
}

void SkLuaCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    AUTO_LUA("drawPath");
    setfield_rect(fL, "bounds", path.getBounds());
    setfield_paint(fL, paint);
}

void SkLuaCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                               const SkPaint* paint) {
    AUTO_LUA("drawBitmap");
    if (paint) {
        setfield_paint(fL, *paint, kImage_PaintUsage);
    }
}

void SkLuaCanvas::drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                   const SkRect& dst, const SkPaint* paint) {
    AUTO_LUA("drawBitmapRectToRect");
    if (paint) {
        setfield_paint(fL, *paint, kImage_PaintUsage);
    }
}

void SkLuaCanvas::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                     const SkPaint* paint) {
    AUTO_LUA("drawBitmapMatrix");
    if (paint) {
        setfield_paint(fL, *paint, kImage_PaintUsage);
    }
}

void SkLuaCanvas::drawSprite(const SkBitmap& bitmap, int x, int y,
                               const SkPaint* paint) {
    AUTO_LUA("drawSprite");
    if (paint) {
        setfield_paint(fL, *paint, kImage_PaintUsage);
    }
}

void SkLuaCanvas::drawText(const void* text, size_t byteLength, SkScalar x,
                             SkScalar y, const SkPaint& paint) {
    AUTO_LUA("drawText");
    setfield_paint(fL, paint, kText_PaintUsage);
}

void SkLuaCanvas::drawPosText(const void* text, size_t byteLength,
                                const SkPoint pos[], const SkPaint& paint) {
    AUTO_LUA("drawPosText");
    setfield_paint(fL, paint, kText_PaintUsage);
}

void SkLuaCanvas::drawPosTextH(const void* text, size_t byteLength,
                                 const SkScalar xpos[], SkScalar constY,
                                 const SkPaint& paint) {
    AUTO_LUA("drawPosTextH");
    setfield_paint(fL, paint, kText_PaintUsage);
}

void SkLuaCanvas::drawTextOnPath(const void* text, size_t byteLength,
                                   const SkPath& path, const SkMatrix* matrix,
                                   const SkPaint& paint) {
    AUTO_LUA("drawTextOnPath");
    setfield_paint(fL, paint, kText_PaintUsage);
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
    setfield_paint(fL, paint);
}

void SkLuaCanvas::drawData(const void* data, size_t length) {
    AUTO_LUA("drawData");
}
