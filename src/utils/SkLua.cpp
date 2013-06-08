/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLua.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkMatrix.h"
#include "SkRRect.h"
#include "SkString.h"
#include "SkTypeface.h"

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

// return the metatable name for a given class
template <typename T> const char* get_mtname();
#define DEF_MTNAME(T)                           \
    template <> const char* get_mtname<T>() {   \
        return #T "_LuaMetaTableName";          \
    }

DEF_MTNAME(SkCanvas)
DEF_MTNAME(SkMatrix)
DEF_MTNAME(SkRRect)
DEF_MTNAME(SkPath)
DEF_MTNAME(SkPaint)
DEF_MTNAME(SkTypeface)

template <typename T> T* push_new(lua_State* L) {
    T* addr = (T*)lua_newuserdata(L, sizeof(T));
    new (addr) T;
    luaL_getmetatable(L, get_mtname<T>());
    lua_setmetatable(L, -2);
    return addr;
}

template <typename T> void push_obj(lua_State* L, const T& obj) {
    new (lua_newuserdata(L, sizeof(T))) T(obj);
    luaL_getmetatable(L, get_mtname<T>());
    lua_setmetatable(L, -2);
}

template <typename T> void push_ref(lua_State* L, T* ref) {
    *(T**)lua_newuserdata(L, sizeof(T*)) = SkRef(ref);
    luaL_getmetatable(L, get_mtname<T>());
    lua_setmetatable(L, -2);
}

template <typename T> T* get_ref(lua_State* L, int index) {
    return *(T**)luaL_checkudata(L, index, get_mtname<T>());
}

template <typename T> T* get_obj(lua_State* L, int index) {
    return (T*)luaL_checkudata(L, index, get_mtname<T>());
}

static bool lua2bool(lua_State* L, int index) {
    return !!lua_toboolean(L, index);
}

///////////////////////////////////////////////////////////////////////////////

SkLua::SkLua(const char termCode[]) : fTermCode(termCode), fWeOwnL(true) {
    fL = luaL_newstate();
    luaL_openlibs(fL);
    SkLua::Load(fL);
}

SkLua::SkLua(lua_State* L) : fL(L), fWeOwnL(false) {}

SkLua::~SkLua() {
    if (fWeOwnL) {
        if (fTermCode.size() > 0) {
            lua_getglobal(fL, fTermCode.c_str());
            if (lua_pcall(fL, 0, 0, 0) != LUA_OK) {
                SkDebugf("lua err: %s\n", lua_tostring(fL, -1));
            }
        }
        lua_close(fL);
    }
}

bool SkLua::runCode(const char code[]) {
    int err = luaL_loadstring(fL, code) || lua_pcall(fL, 0, 0, 0);
    if (err) {
        SkDebugf("--- lua failed\n");
        return false;
    }
    return true;
}

bool SkLua::runCode(const void* code, size_t size) {
    SkString str((const char*)code, size);
    return this->runCode(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////

#define CHECK_SETFIELD(key) do if (key) lua_setfield(fL, -2, key); while (0)

static void setfield_string(lua_State* L, const char key[], const char value[]) {
    lua_pushstring(L, value);
    lua_setfield(L, -2, key);
}

static void setfield_number(lua_State* L, const char key[], double value) {
    lua_pushnumber(L, value);
    lua_setfield(L, -2, key);
}

static void setfield_function(lua_State* L,
                              const char key[], lua_CFunction value) {
    lua_pushcfunction(L, value);
    lua_setfield(L, -2, key);
}

static void setarray_number(lua_State* L, int index, double value) {
    lua_pushnumber(L, value);
    lua_rawseti(L, -2, index);
}

void SkLua::pushBool(bool value, const char key[]) {
    lua_pushboolean(fL, value);
    CHECK_SETFIELD(key);
}

void SkLua::pushString(const char str[], const char key[]) {
    lua_pushstring(fL, str);
    CHECK_SETFIELD(key);
}

void SkLua::pushString(const char str[], size_t length, const char key[]) {
    // TODO: how to do this w/o making a copy?
    SkString s(str, length);
    lua_pushstring(fL, s.c_str());
    CHECK_SETFIELD(key);
}

void SkLua::pushString(const SkString& str, const char key[]) {
    lua_pushstring(fL, str.c_str());
    CHECK_SETFIELD(key);
}

void SkLua::pushColor(SkColor color, const char key[]) {
    lua_newtable(fL);
    setfield_number(fL, "a", SkColorGetA(color) / 255.0);
    setfield_number(fL, "r", SkColorGetR(color) / 255.0);
    setfield_number(fL, "g", SkColorGetG(color) / 255.0);
    setfield_number(fL, "b", SkColorGetB(color) / 255.0);
    CHECK_SETFIELD(key);
}

void SkLua::pushU32(uint32_t value, const char key[]) {
    lua_pushnumber(fL, (double)value);
    CHECK_SETFIELD(key);
}

void SkLua::pushScalar(SkScalar value, const char key[]) {
    lua_pushnumber(fL, SkScalarToLua(value));
    CHECK_SETFIELD(key);
}

void SkLua::pushArrayU16(const uint16_t array[], int count, const char key[]) {
    lua_newtable(fL);
    for (int i = 0; i < count; ++i) {
        // make it base-1 to match lua convention
        setarray_number(fL, i + 1, (double)array[i]);
    }
    CHECK_SETFIELD(key);
}

void SkLua::pushRect(const SkRect& r, const char key[]) {
    lua_newtable(fL);
    setfield_number(fL, "left", SkScalarToLua(r.fLeft));
    setfield_number(fL, "top", SkScalarToLua(r.fTop));
    setfield_number(fL, "right", SkScalarToLua(r.fRight));
    setfield_number(fL, "bottom", SkScalarToLua(r.fBottom));
    CHECK_SETFIELD(key);
}

void SkLua::pushRRect(const SkRRect& rr, const char key[]) {
    push_obj(fL, rr);
    CHECK_SETFIELD(key);
}

void SkLua::pushMatrix(const SkMatrix& matrix, const char key[]) {
    push_obj(fL, matrix);
    CHECK_SETFIELD(key);
}

void SkLua::pushPaint(const SkPaint& paint, const char key[]) {
    push_obj(fL, paint);
    CHECK_SETFIELD(key);
}

void SkLua::pushPath(const SkPath& path, const char key[]) {
    push_obj(fL, path);
    CHECK_SETFIELD(key);
}

void SkLua::pushCanvas(SkCanvas* canvas, const char key[]) {
    push_ref(fL, canvas);
    CHECK_SETFIELD(key);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static SkScalar lua2scalar(lua_State* L, int index) {
    SkASSERT(lua_isnumber(L, index));
    return SkLuaToScalar(lua_tonumber(L, index));
}

static SkScalar getfield_scalar(lua_State* L, int index, const char key[]) {
    SkASSERT(lua_istable(L, index));
    lua_pushstring(L, key);
    lua_gettable(L, index);

    SkScalar value = lua2scalar(L, -1);
    lua_pop(L, 1);
    return value;
}

static U8CPU unit2byte(SkScalar x) {
    if (x <= 0) {
        return 0;
    } else if (x >= 1) {
        return 255;
    } else {
        return SkScalarRoundToInt(x * 255);
    }
}

static SkColor lua2color(lua_State* L, int index) {
    return SkColorSetARGB(unit2byte(getfield_scalar(L, index, "a")),
                          unit2byte(getfield_scalar(L, index, "r")),
                          unit2byte(getfield_scalar(L, index, "g")),
                          unit2byte(getfield_scalar(L, index, "b")));
}

static SkRect* lua2rect(lua_State* L, int index, SkRect* rect) {
    rect->set(getfield_scalar(L, index, "left"),
              getfield_scalar(L, index, "top"),
              getfield_scalar(L, index, "right"),
              getfield_scalar(L, index, "bottom"));
    return rect;
}

static int lcanvas_drawColor(lua_State* L) {
    get_ref<SkCanvas>(L, 1)->drawColor(lua2color(L, 2));
    return 0;
}

static int lcanvas_drawRect(lua_State* L) {
    SkRect rect;
    get_ref<SkCanvas>(L, 1)->drawRect(*lua2rect(L, 2, &rect),
                                      *get_obj<SkPaint>(L, 3));
    return 0;
}

static int lcanvas_drawOval(lua_State* L) {
    SkRect rect;
    get_ref<SkCanvas>(L, 1)->drawOval(*lua2rect(L, 2, &rect),
                                      *get_obj<SkPaint>(L, 3));
    return 0;
}

static int lcanvas_drawCircle(lua_State* L) {
    get_ref<SkCanvas>(L, 1)->drawCircle(lua2scalar(L, 2),
                                        lua2scalar(L, 3),
                                        lua2scalar(L, 4),
                                        *get_obj<SkPaint>(L, 5));
    return 0;
}

static int lcanvas_drawPath(lua_State* L) {
    get_ref<SkCanvas>(L, 1)->drawPath(*get_obj<SkPath>(L, 2),
                                      *get_obj<SkPaint>(L, 3));
    return 0;
}

static int lcanvas_drawText(lua_State* L) {
    if (lua_gettop(L) < 5) {
        return 0;
    }

    if (lua_isstring(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4)) {
        size_t len;
        const char* text = lua_tolstring(L, 2, &len);
        get_ref<SkCanvas>(L, 1)->drawText(text, len,
                                          lua2scalar(L, 3), lua2scalar(L, 4),
                                          *get_obj<SkPaint>(L, 5));
    }
    return 0;
}

static int lcanvas_getSaveCount(lua_State* L) {
    lua_pushnumber(L, get_ref<SkCanvas>(L, 1)->getSaveCount());
    return 1;
}

static int lcanvas_getTotalMatrix(lua_State* L) {
    SkLua(L).pushMatrix(get_ref<SkCanvas>(L, 1)->getTotalMatrix());
    return 1;
}

static int lcanvas_translate(lua_State* L) {
    get_ref<SkCanvas>(L, 1)->translate(lua2scalar(L, 2), lua2scalar(L, 3));
    return 0;
}

static int lcanvas_gc(lua_State* L) {
    get_ref<SkCanvas>(L, 1)->unref();
    return 0;
}

static const struct luaL_Reg gSkCanvas_Methods[] = {
    { "drawColor", lcanvas_drawColor },
    { "drawRect", lcanvas_drawRect },
    { "drawOval", lcanvas_drawOval },
    { "drawCircle", lcanvas_drawCircle },
    { "drawPath", lcanvas_drawPath },
    { "drawText", lcanvas_drawText },
    { "getSaveCount", lcanvas_getSaveCount },
    { "getTotalMatrix", lcanvas_getTotalMatrix },
    { "translate", lcanvas_translate },
    { "__gc", lcanvas_gc },
    { NULL, NULL }
};

///////////////////////////////////////////////////////////////////////////////

static int lpaint_isAntiAlias(lua_State* L) {
    lua_pushboolean(L, get_obj<SkPaint>(L, 1)->isAntiAlias());
    return 1;
}

static int lpaint_setAntiAlias(lua_State* L) {
    get_obj<SkPaint>(L, 1)->setAntiAlias(lua2bool(L, 2));
    return 0;
}

static int lpaint_getColor(lua_State* L) {
    SkLua(L).pushColor(get_obj<SkPaint>(L, 1)->getColor());
    return 1;
}

static int lpaint_setColor(lua_State* L) {
    get_obj<SkPaint>(L, 1)->setColor(lua2color(L, 2));
    return 0;
}

static int lpaint_getTextSize(lua_State* L) {
    SkLua(L).pushScalar(get_obj<SkPaint>(L, 1)->getTextSize());
    return 1;
}

static int lpaint_setTextSize(lua_State* L) {
    get_obj<SkPaint>(L, 1)->setTextSize(lua2scalar(L, 2));
    return 0;
}

static int lpaint_getTypeface(lua_State* L) {
    push_ref(L, get_obj<SkPaint>(L, 1)->getTypeface());
    return 1;
}

static int lpaint_setTypeface(lua_State* L) {
    get_obj<SkPaint>(L, 1)->setTypeface(get_ref<SkTypeface>(L, 2));
    return 0;
}

static int lpaint_getFontID(lua_State* L) {
    SkTypeface* face = get_obj<SkPaint>(L, 1)->getTypeface();
    SkLua(L).pushU32(SkTypeface::UniqueID(face));
    return 1;
}

static int lpaint_gc(lua_State* L) {
    get_obj<SkPaint>(L, 1)->~SkPaint();
    return 0;
}

static const struct luaL_Reg gSkPaint_Methods[] = {
    { "isAntiAlias", lpaint_isAntiAlias },
    { "setAntiAlias", lpaint_setAntiAlias },
    { "getColor", lpaint_getColor },
    { "setColor", lpaint_setColor },
    { "getTextSize", lpaint_getTextSize },
    { "setTextSize", lpaint_setTextSize },
    { "getTypeface", lpaint_getTypeface },
    { "setTypeface", lpaint_setTypeface },
    { "getFontID", lpaint_getFontID },
    { "__gc", lpaint_gc },
    { NULL, NULL }
};

///////////////////////////////////////////////////////////////////////////////

static int lpath_getBounds(lua_State* L) {
    SkLua(L).pushRect(get_obj<SkPath>(L, 1)->getBounds());
    return 1;
}

static int lpath_isEmpty(lua_State* L) {
    lua_pushboolean(L, get_obj<SkPath>(L, 1)->isEmpty());
    return 1;
}

static int lpath_isRect(lua_State* L) {
    SkRect r;
    bool pred = get_obj<SkPath>(L, 1)->isRect(&r);
    int ret_count = 1;
    lua_pushboolean(L, pred);
    if (pred) {
        SkLua(L).pushRect(r);
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
    SkRect rects[2];
    SkPath::Direction dirs[2];
    bool pred = get_obj<SkPath>(L, 1)->isNestedRects(rects, dirs);
    int ret_count = 1;
    lua_pushboolean(L, pred);
    if (pred) {
        SkLua lua(L);
        lua.pushRect(rects[0]);
        lua.pushRect(rects[1]);
        lua_pushstring(L, dir2string(dirs[0]));
        lua_pushstring(L, dir2string(dirs[0]));
        ret_count += 4;
    }
    return ret_count;
}

static int lpath_reset(lua_State* L) {
    get_obj<SkPath>(L, 1)->reset();
    return 0;
}

static int lpath_moveTo(lua_State* L) {
    get_obj<SkPath>(L, 1)->moveTo(lua2scalar(L, 2), lua2scalar(L, 3));
    return 0;
}

static int lpath_lineTo(lua_State* L) {
    get_obj<SkPath>(L, 1)->lineTo(lua2scalar(L, 2), lua2scalar(L, 3));
    return 0;
}

static int lpath_quadTo(lua_State* L) {
    get_obj<SkPath>(L, 1)->quadTo(lua2scalar(L, 2), lua2scalar(L, 3),
                                  lua2scalar(L, 4), lua2scalar(L, 5));
    return 0;
}

static int lpath_cubicTo(lua_State* L) {
    get_obj<SkPath>(L, 1)->cubicTo(lua2scalar(L, 2), lua2scalar(L, 3),
                                   lua2scalar(L, 4), lua2scalar(L, 5),
                                   lua2scalar(L, 6), lua2scalar(L, 7));
    return 0;
}

static int lpath_close(lua_State* L) {
    get_obj<SkPath>(L, 1)->close();
    return 0;
}

static int lpath_gc(lua_State* L) {
    get_obj<SkPath>(L, 1)->~SkPath();
    return 0;
}

static const struct luaL_Reg gSkPath_Methods[] = {
    { "getBounds", lpath_getBounds },
    { "isEmpty", lpath_isEmpty },
    { "isRect", lpath_isRect },
    { "isNestedRects", lpath_isNestedRects },
    { "reset", lpath_reset },
    { "moveTo", lpath_moveTo },
    { "lineTo", lpath_lineTo },
    { "quadTo", lpath_quadTo },
    { "cubicTo", lpath_cubicTo },
    { "close", lpath_close },
    { "__gc", lpath_gc },
    { NULL, NULL }
};

///////////////////////////////////////////////////////////////////////////////

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

static int lrrect_rect(lua_State* L) {
    SkLua(L).pushRect(get_obj<SkRRect>(L, 1)->rect());
    return 1;
}

static int lrrect_type(lua_State* L) {
    lua_pushstring(L, rrect_type(*get_obj<SkRRect>(L, 1)));
    return 1;
}

static int lrrect_radii(lua_State* L) {
    int corner = lua_tointeger(L, 2);
    SkVector v;
    if (corner < 0 || corner > 3) {
        SkDebugf("bad corner index %d", corner);
        v.set(0, 0);
    } else {
        v = get_obj<SkRRect>(L, 1)->radii((SkRRect::Corner)corner);
    }
    lua_pushnumber(L, v.fX);
    lua_pushnumber(L, v.fY);
    return 2;
}

static int lrrect_gc(lua_State* L) {
    get_obj<SkRRect>(L, 1)->~SkRRect();
    return 0;
}

static const struct luaL_Reg gSkRRect_Methods[] = {
    { "rect", lrrect_rect },
    { "type", lrrect_type },
    { "radii", lrrect_radii },
    { "__gc", lrrect_gc },
    { NULL, NULL }
};

///////////////////////////////////////////////////////////////////////////////

static int ltypeface_gc(lua_State* L) {
    get_ref<SkTypeface>(L, 1)->unref();
    return 0;
}

static const struct luaL_Reg gSkTypeface_Methods[] = {
    { "__gc", ltypeface_gc },
    { NULL, NULL }
};

///////////////////////////////////////////////////////////////////////////////

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

static int lsk_newPaint(lua_State* L) {
    push_new<SkPaint>(L);
    return 1;
}

static int lsk_newPath(lua_State* L) {
    push_new<SkPath>(L);
    return 1;
}

static int lsk_newRRect(lua_State* L) {
    SkRRect* rr = push_new<SkRRect>(L);
    rr->setEmpty();
    return 1;
}

static int lsk_newTypeface(lua_State* L) {
    const char* name = NULL;
    int style = SkTypeface::kNormal;
    
    int count = lua_gettop(L);
    if (count > 0 && lua_isstring(L, 1)) {
        name = lua_tolstring(L, 1, NULL);
        if (count > 1 && lua_isnumber(L, 2)) {
            style = lua_tointegerx(L, 2, NULL) & SkTypeface::kBoldItalic;
        }
    }

    SkTypeface* face = SkTypeface::CreateFromName(name,
                                                  (SkTypeface::Style)style);
//    SkDebugf("---- name <%s> style=%d, face=%p ref=%d\n", name, style, face, face->getRefCnt());
    if (NULL == face) {
        face = SkTypeface::RefDefault();
    }
    push_ref(L, face);
    face->unref();
    return 1;
}

static void register_Sk(lua_State* L) {
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "Sk");
    // the Sk table is still on top

    setfield_function(L, "newPaint", lsk_newPaint);
    setfield_function(L, "newPath", lsk_newPath);
    setfield_function(L, "newRRect", lsk_newRRect);
    setfield_function(L, "newTypeface", lsk_newTypeface);
    lua_pop(L, 1);  // pop off the Sk table
}

#define REG_CLASS(L, C)                             \
    do {                                            \
        luaL_newmetatable(L, get_mtname<C>());      \
        lua_pushvalue(L, -1);                       \
        lua_setfield(L, -2, "__index");             \
        luaL_setfuncs(L, g##C##_Methods, 0);        \
        lua_pop(L, 1); /* pop off the meta-table */ \
    } while (0)

void SkLua::Load(lua_State* L) {
    register_Sk(L);
    REG_CLASS(L, SkCanvas);
    REG_CLASS(L, SkPath);
    REG_CLASS(L, SkPaint);
    REG_CLASS(L, SkRRect);
    REG_CLASS(L, SkTypeface);
}
