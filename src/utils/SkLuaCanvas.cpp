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
    #include "lauxlib.h"
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

// sets [1]...[count] in the table on the top of the stack
static void setfield_arrayf(lua_State* L, const SkScalar array[], int count) {
    for (int i = 0; i < count; ++i) {
        lua_pushnumber(L, SkScalarToDouble(i + 1));     // key
        lua_pushnumber(L, SkScalarToDouble(array[i]));  // value
        lua_settable(L, -3);
    }
}

static void push_rect(lua_State* L, const SkRect& r) {
    lua_newtable(L);
    setfield_number(L, "left", r.fLeft);
    setfield_number(L, "top", r.fTop);
    setfield_number(L, "right", r.fRight);
    setfield_number(L, "bottom", r.fBottom);
}

static void setfield_rect(lua_State* L, const char key[], const SkRect& r) {
    push_rect(L, r);
    lua_setfield(L, -2, key);
}

static const char* rrect_type(const SkRRect& rr) {
    switch (rr.getType()) {
        case SkRRect::kUnknown_Type: return "unknown";
        case SkRRect::kEmpty_Type: return "empty";
        case SkRRect::kRect_Type: return "rect";
        case SkRRect::kOval_Type: return "oval";
        case SkRRect::kSimple_Type: return "simple";
        case SkRRect::kComplex_Type: return "complex";
    }
    SkASSERT(!"never get here");
    return "";
}

static void setfield_rrect(lua_State* L, const char key[], const SkRRect& rr) {
    lua_newtable(L);
    setfield_rect(L, "rect", rr.getBounds());
    setfield_string(L, "type", rrect_type(rr));

    SkVector rad[4] = {
        rr.radii(SkRRect::kUpperLeft_Corner),
        rr.radii(SkRRect::kUpperRight_Corner),
        rr.radii(SkRRect::kLowerRight_Corner),
        rr.radii(SkRRect::kLowerLeft_Corner),
    };
    setfield_arrayf(L, &rad[0].fX, 8);
    lua_setfield(L, -2, key);
}

static void push_matrix(lua_State* L, const SkMatrix& mat) {
    SkScalar m[9];
    for (int i = 0; i < 9; ++i) {
        m[i] = mat[i];
    }
    lua_newtable(L);
    setfield_arrayf(L, m, 9);
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

static const char gCanvasMetaTableName[] = "SkCanvas_MetaTable";

static int lcanvas_getSaveCount(lua_State* L) {
    SkCanvas* c = *(SkCanvas**)luaL_checkudata(L, 1, gCanvasMetaTableName);
    lua_pushnumber(L, (double)c->getSaveCount());
    return 1;
}

static int lcanvas_getTotalMatrix(lua_State* L) {
    SkCanvas* c = *(SkCanvas**)luaL_checkudata(L, 1, gCanvasMetaTableName);
    push_matrix(L, c->getTotalMatrix());
    return 1;
}

static int lcanvas_gc(lua_State* L) {
    SkCanvas** cptr = (SkCanvas**)luaL_checkudata(L, 1, gCanvasMetaTableName);
    SkSafeUnref(*cptr);
    *cptr = NULL;
    return 0;
}

static const struct luaL_Reg gLuaCanvasMethods[] = {
    { "getSaveCount", lcanvas_getSaveCount },
    { "getTotalMatrix", lcanvas_getTotalMatrix },
    { "__gc", lcanvas_gc },
    { NULL, NULL }
};

static void ensure_canvas_metatable(lua_State* L) {
    static bool gOnce;
    if (gOnce) {
        return;
    }
    gOnce = true;
    
    luaL_newmetatable(L, gCanvasMetaTableName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    
    luaL_setfuncs(L, gLuaCanvasMethods, 0);
    lua_settop(L, -2);  // pop off the meta-table
}

///////////////////////////////////////////////////////////////////////////////

static const char gPathMetaTableName[] = "SkPath_MetaTable";

static int lpath_getBounds(lua_State* L) {
    SkPath* p = (SkPath*)luaL_checkudata(L, 1, gPathMetaTableName);
    push_rect(L, p->getBounds());
    return 1;
}

static int lpath_isEmpty(lua_State* L) {
    SkPath* p = (SkPath*)luaL_checkudata(L, 1, gPathMetaTableName);
    lua_pushboolean(L, p->isEmpty());
    return 1;
}

static int lpath_isRect(lua_State* L) {
    SkPath* p = (SkPath*)luaL_checkudata(L, 1, gPathMetaTableName);
    SkRect r;
    bool pred = p->isRect(&r);
    int ret_count = 1;
    lua_pushboolean(L, pred);
    if (pred) {
        push_rect(L, r);
        ret_count += 1;
    }
    return ret_count;
}

static const char* dir2string(SkPath::Direction dir) {
    static const char* gStr[] = {
        "unknown", "cw", "ccw"
    };
    SkASSERT((unsigned)dir < SK_ARRAY_COUNT(gStr));
    return gStr[dir];
}

static int lpath_isNestedRects(lua_State* L) {
    SkPath* p = (SkPath*)luaL_checkudata(L, 1, gPathMetaTableName);
    SkRect rects[2];
    SkPath::Direction dirs[2];
    bool pred = p->isNestedRects(rects, dirs);
    int ret_count = 1;
    lua_pushboolean(L, pred);
    if (pred) {
        push_rect(L, rects[0]);
        push_rect(L, rects[1]);
        lua_pushstring(L, dir2string(dirs[0]));
        lua_pushstring(L, dir2string(dirs[0]));
        ret_count += 4;
    }
    return ret_count;
}

static int lpath_gc(lua_State* L) {
    SkPath* p = (SkPath*)luaL_checkudata(L, 1, gPathMetaTableName);
    p->~SkPath();
    return 0;
}

static const struct luaL_Reg gLuaPathMethods[] = {
    { "getBounds", lpath_getBounds },
    { "isEmpty", lpath_isEmpty },
    { "isRect", lpath_isRect },
    { "isNestedRects", lpath_isNestedRects },
    { "__gc", lpath_gc },
    { NULL, NULL }
};

static void ensure_path_metatable(lua_State* L) {
    static bool gOnce;
    if (gOnce) {
        return;
    }
    gOnce = true;
    
    luaL_newmetatable(L, gPathMetaTableName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    
    luaL_setfuncs(L, gLuaPathMethods, 0);
    lua_settop(L, -2);  // pop off the meta-table
}

static void push_path(lua_State* L, const SkPath& src) {
    ensure_path_metatable(L);
    
    SkPath* path = (SkPath*)lua_newuserdata(L, sizeof(SkPath));
    new (path) SkPath(src);

    luaL_getmetatable(L, gPathMetaTableName);
    lua_setmetatable(L, -2);
}

static void setfield_path(lua_State* L, const char key[], const SkPath& path) {
    push_path(L, path);
    lua_setfield(L, -2, key);
}

///////////////////////////////////////////////////////////////////////////////

void SkLuaCanvas::pushThis() {
    ensure_canvas_metatable(fL);

    SkCanvas** canvasPtr = (SkCanvas**)lua_newuserdata(fL, sizeof(SkCanvas*));
    luaL_getmetatable(fL, gCanvasMetaTableName);
    lua_setmetatable(fL, -2);

    this->ref();
    *canvasPtr = this;
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
    setfield_rrect(fL, "rrect", rrect);
    setfield_bool(fL, "aa", doAA);
    return this->INHERITED::clipRRect(rrect, op, doAA);
}

bool SkLuaCanvas::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    AUTO_LUA("clipPath");
    setfield_path(fL, "path", path);
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
    setfield_rect(fL, "oval", rect);
    setfield_paint(fL, paint);
}

void SkLuaCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    AUTO_LUA("drawRect");
    setfield_rect(fL, "rect", rect);
    setfield_paint(fL, paint);
}

void SkLuaCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    AUTO_LUA("drawRRect");
    setfield_rrect(fL, "rrect", rrect);
    setfield_paint(fL, paint);
}

void SkLuaCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    AUTO_LUA("drawPath");
    setfield_path(fL, "path", path);
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
    setfield_path(fL, "path", path);
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
