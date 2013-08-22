/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLua.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDocument.h"
#include "SkImage.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPixelRef.h"
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
DEF_MTNAME(SkDocument)
DEF_MTNAME(SkImage)
DEF_MTNAME(SkMatrix)
DEF_MTNAME(SkRRect)
DEF_MTNAME(SkPath)
DEF_MTNAME(SkPaint)
DEF_MTNAME(SkShader)
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
        SkDebugf("--- lua failed: %s\n", lua_tostring(fL, -1));
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

static void setfield_bool_if(lua_State* L, const char key[], bool pred) {
    if (pred) {
        lua_pushboolean(L, true);
        lua_setfield(L, -2, key);
    }
}

static void setfield_string(lua_State* L, const char key[], const char value[]) {
    lua_pushstring(L, value);
    lua_setfield(L, -2, key);
}

static void setfield_number(lua_State* L, const char key[], double value) {
    lua_pushnumber(L, value);
    lua_setfield(L, -2, key);
}

static void setfield_boolean(lua_State* L, const char key[], bool value) {
    lua_pushboolean(L, value);
    lua_setfield(L, -2, key);
}

static void setfield_scalar(lua_State* L, const char key[], SkScalar value) {
    setfield_number(L, key, SkScalarToLua(value));
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
    setfield_scalar(fL, "left", r.fLeft);
    setfield_scalar(fL, "top", r.fTop);
    setfield_scalar(fL, "right", r.fRight);
    setfield_scalar(fL, "bottom", r.fBottom);
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

static SkScalar lua2scalar_def(lua_State* L, int index, SkScalar defaultValue) {
    if (lua_isnumber(L, index)) {
        return SkLuaToScalar(lua_tonumber(L, index));
    } else {
        return defaultValue;
    }
}

static SkScalar getfield_scalar(lua_State* L, int index, const char key[]) {
    SkASSERT(lua_istable(L, index));
    lua_pushstring(L, key);
    lua_gettable(L, index);

    SkScalar value = lua2scalar(L, -1);
    lua_pop(L, 1);
    return value;
}

static SkScalar getfield_scalar_default(lua_State* L, int index, const char key[], SkScalar def) {
    SkASSERT(lua_istable(L, index));
    lua_pushstring(L, key);
    lua_gettable(L, index);

    SkScalar value;
    if (lua_isnil(L, -1)) {
        value = def;
    } else {
        value = lua2scalar(L, -1);
    }
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
    rect->set(getfield_scalar_default(L, index, "left", 0),
              getfield_scalar_default(L, index, "top", 0),
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

static int lcanvas_drawImage(lua_State* L) {
    SkCanvas* canvas = get_ref<SkCanvas>(L, 1);
    SkImage* image = get_ref<SkImage>(L, 2);
    if (NULL == image) {
        return 0;
    }
    SkScalar x = lua2scalar(L, 3);
    SkScalar y = lua2scalar(L, 4);

    SkPaint paint;
    const SkPaint* paintPtr = NULL;
    if (lua_isnumber(L, 5)) {
        paint.setAlpha(SkScalarRoundToInt(lua2scalar(L, 5) * 255));
        paintPtr = &paint;
    }
    image->draw(canvas, x, y, paintPtr);
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

static int lcanvas_save(lua_State* L) {
    lua_pushinteger(L, get_ref<SkCanvas>(L, 1)->save());
    return 1;
}

static int lcanvas_restore(lua_State* L) {
    get_ref<SkCanvas>(L, 1)->restore();
    return 0;
}

static int lcanvas_scale(lua_State* L) {
    SkScalar sx = lua2scalar_def(L, 2, 1);
    SkScalar sy = lua2scalar_def(L, 3, sx);
    get_ref<SkCanvas>(L, 1)->scale(sx, sy);
    return 0;
}

static int lcanvas_translate(lua_State* L) {
    SkScalar tx = lua2scalar_def(L, 2, 0);
    SkScalar ty = lua2scalar_def(L, 3, 0);
    get_ref<SkCanvas>(L, 1)->translate(tx, ty);
    return 0;
}

static int lcanvas_rotate(lua_State* L) {
    SkScalar degrees = lua2scalar_def(L, 2, 0);
    get_ref<SkCanvas>(L, 1)->rotate(degrees);
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
    { "drawImage", lcanvas_drawImage },
    { "drawPath", lcanvas_drawPath },
    { "drawText", lcanvas_drawText },
    { "getSaveCount", lcanvas_getSaveCount },
    { "getTotalMatrix", lcanvas_getTotalMatrix },
    { "save", lcanvas_save },
    { "restore", lcanvas_restore },
    { "scale", lcanvas_scale },
    { "translate", lcanvas_translate },
    { "rotate", lcanvas_rotate },
    { "__gc", lcanvas_gc },
    { NULL, NULL }
};

///////////////////////////////////////////////////////////////////////////////

static int ldocument_beginPage(lua_State* L) {
    const SkRect* contentPtr = NULL;
    push_ref(L, get_ref<SkDocument>(L, 1)->beginPage(lua2scalar(L, 2),
                                                     lua2scalar(L, 3),
                                                     contentPtr));
    return 1;
}

static int ldocument_endPage(lua_State* L) {
    get_ref<SkDocument>(L, 1)->endPage();
    return 0;
}

static int ldocument_close(lua_State* L) {
    get_ref<SkDocument>(L, 1)->close();
    return 0;
}

static int ldocument_gc(lua_State* L) {
    get_ref<SkDocument>(L, 1)->unref();
    return 0;
}

static const struct luaL_Reg gSkDocument_Methods[] = {
    { "beginPage", ldocument_beginPage },
    { "endPage", ldocument_endPage },
    { "close", ldocument_close },
    { "__gc", ldocument_gc },
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

static const struct {
    const char*     fLabel;
    SkPaint::Align  fAlign;
} gAlignRec[] = {
    { "left",   SkPaint::kLeft_Align },
    { "center", SkPaint::kCenter_Align },
    { "right",  SkPaint::kRight_Align },
};

static int lpaint_getTextAlign(lua_State* L) {
    SkPaint::Align align = get_obj<SkPaint>(L, 1)->getTextAlign();
    for (size_t i = 0; i < SK_ARRAY_COUNT(gAlignRec); ++i) {
        if (gAlignRec[i].fAlign == align) {
            lua_pushstring(L, gAlignRec[i].fLabel);
            return 1;
        }
    }
    return 0;
}

static int lpaint_setTextAlign(lua_State* L) {
    if (lua_isstring(L, 2)) {
        size_t len;
        const char* label = lua_tolstring(L, 2, &len);

        for (size_t i = 0; i < SK_ARRAY_COUNT(gAlignRec); ++i) {
            if (!strcmp(gAlignRec[i].fLabel, label)) {
                get_obj<SkPaint>(L, 1)->setTextAlign(gAlignRec[i].fAlign);
                break;
            }
        }
    }
    return 0;
}

static int lpaint_getStroke(lua_State* L) {
    lua_pushboolean(L, SkPaint::kStroke_Style == get_obj<SkPaint>(L, 1)->getStyle());
    return 1;
}

static int lpaint_setStroke(lua_State* L) {
    SkPaint::Style style;

    if (lua_toboolean(L, 2)) {
        style = SkPaint::kStroke_Style;
    } else {
        style = SkPaint::kFill_Style;
    }
    get_obj<SkPaint>(L, 1)->setStyle(style);
    return 0;
}

static int lpaint_getStrokeWidth(lua_State* L) {
    SkLua(L).pushScalar(get_obj<SkPaint>(L, 1)->getStrokeWidth());
    return 1;
}

static int lpaint_setStrokeWidth(lua_State* L) {
    get_obj<SkPaint>(L, 1)->setStrokeWidth(lua2scalar(L, 2));
    return 0;
}

static int lpaint_measureText(lua_State* L) {
    if (lua_isstring(L, 2)) {
        size_t len;
        const char* text = lua_tolstring(L, 2, &len);
        SkLua(L).pushScalar(get_obj<SkPaint>(L, 1)->measureText(text, len));
        return 1;
    }
    return 0;
}

struct FontMetrics {
    SkScalar    fTop;       //!< The greatest distance above the baseline for any glyph (will be <= 0)
    SkScalar    fAscent;    //!< The recommended distance above the baseline (will be <= 0)
    SkScalar    fDescent;   //!< The recommended distance below the baseline (will be >= 0)
    SkScalar    fBottom;    //!< The greatest distance below the baseline for any glyph (will be >= 0)
    SkScalar    fLeading;   //!< The recommended distance to add between lines of text (will be >= 0)
    SkScalar    fAvgCharWidth;  //!< the average charactor width (>= 0)
    SkScalar    fXMin;      //!< The minimum bounding box x value for all glyphs
    SkScalar    fXMax;      //!< The maximum bounding box x value for all glyphs
    SkScalar    fXHeight;   //!< the height of an 'x' in px, or 0 if no 'x' in face
};

static int lpaint_getFontMetrics(lua_State* L) {
    SkPaint::FontMetrics fm;
    SkScalar height = get_obj<SkPaint>(L, 1)->getFontMetrics(&fm);

    lua_newtable(L);
    setfield_scalar(L, "top", fm.fTop);
    setfield_scalar(L, "ascent", fm.fAscent);
    setfield_scalar(L, "descent", fm.fDescent);
    setfield_scalar(L, "bottom", fm.fBottom);
    setfield_scalar(L, "leading", fm.fLeading);
    SkLua(L).pushScalar(height);
    return 2;
}

static int lpaint_getEffects(lua_State* L) {
    const SkPaint* paint = get_obj<SkPaint>(L, 1);

    lua_newtable(L);
    setfield_bool_if(L, "looper", !!paint->getLooper());
    setfield_bool_if(L, "pathEffect", !!paint->getPathEffect());
    setfield_bool_if(L, "rasterizer", !!paint->getRasterizer());
    setfield_bool_if(L, "maskFilter", !!paint->getMaskFilter());
    setfield_bool_if(L, "shader", !!paint->getShader());
    setfield_bool_if(L, "colorFilter", !!paint->getColorFilter());
    setfield_bool_if(L, "imageFilter", !!paint->getImageFilter());
    setfield_bool_if(L, "xfermode", !!paint->getXfermode());
    return 1;
}

static int lpaint_getShader(lua_State* L) {
    const SkPaint* paint = get_obj<SkPaint>(L, 1);
    SkShader* shader = paint->getShader();
    if (shader) {
        push_ref(L, shader);
        return 1;
    }
    return 0;
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
    { "getTextAlign", lpaint_getTextAlign },
    { "setTextAlign", lpaint_setTextAlign },
    { "getStroke", lpaint_getStroke },
    { "setStroke", lpaint_setStroke },
    { "getStrokeWidth", lpaint_getStrokeWidth },
    { "setStrokeWidth", lpaint_setStrokeWidth },
    { "measureText", lpaint_measureText },
    { "getFontMetrics", lpaint_getFontMetrics },
    { "getEffects", lpaint_getEffects },
    { "getShader", lpaint_getShader },
    { "__gc", lpaint_gc },
    { NULL, NULL }
};

///////////////////////////////////////////////////////////////////////////////

static const char* mode2string(SkShader::TileMode mode) {
    static const char* gNames[] = { "clamp", "repeat", "mirror" };
    SkASSERT((unsigned)mode < SK_ARRAY_COUNT(gNames));
    return gNames[mode];
}

static const char* gradtype2string(SkShader::GradientType t) {
    static const char* gNames[] = {
        "none", "color", "linear", "radial", "radial2", "sweep", "conical"
    };
    SkASSERT((unsigned)t < SK_ARRAY_COUNT(gNames));
    return gNames[t];
}

static int lshader_isOpaque(lua_State* L) {
    SkShader* shader = get_ref<SkShader>(L, 1);
    return shader && shader->isOpaque();
}

static int lshader_asABitmap(lua_State* L) {
    SkShader* shader = get_ref<SkShader>(L, 1);
    if (shader) {
        SkBitmap bm;
        SkMatrix matrix;
        SkShader::TileMode modes[2];
        switch (shader->asABitmap(&bm, &matrix, modes)) {
            case SkShader::kDefault_BitmapType:
                lua_newtable(L);
                setfield_number(L, "genID", bm.pixelRef() ? bm.pixelRef()->getGenerationID() : 0);
                setfield_number(L, "width", bm.width());
                setfield_number(L, "height", bm.height());
                setfield_string(L, "tileX", mode2string(modes[0]));
                setfield_string(L, "tileY", mode2string(modes[1]));
                return 1;
            default:
                break;
        }
    }
    return 0;
}

static int lshader_asAGradient(lua_State* L) {
    SkShader* shader = get_ref<SkShader>(L, 1);
    if (shader) {
        SkShader::GradientInfo info;
        sk_bzero(&info, sizeof(info));

        SkColor colors[3];  // hacked in for extracting info on 3 color case.
        SkScalar pos[3];

        info.fColorCount = 3;
        info.fColors = &colors[0];
        info.fColorOffsets = &pos[0];

        SkShader::GradientType t = shader->asAGradient(&info);

        if (SkShader::kNone_GradientType != t) {
            lua_newtable(L);
            setfield_string(L, "type", gradtype2string(t));
            setfield_number(L, "colorCount", info.fColorCount);
            setfield_string(L, "tile", mode2string(info.fTileMode));

            if (info.fColorCount == 3){
                setfield_number(L, "midPos", pos[1]);
            }

            return 1;
        }
    }
    return 0;
}

static int lshader_gc(lua_State* L) {
    get_ref<SkShader>(L, 1)->unref();
    return 0;
}

static const struct luaL_Reg gSkShader_Methods[] = {
    { "isOpaque",       lshader_isOpaque },
    { "asABitmap",      lshader_asABitmap },
    { "asAGradient",    lshader_asAGradient },
    { "__gc",           lshader_gc },
    { NULL, NULL }
};

///////////////////////////////////////////////////////////////////////////////

static int lmatrix_getType(lua_State* L) {
    SkMatrix::TypeMask mask = get_obj<SkMatrix>(L, 1)->getType();

    lua_newtable(L);
    setfield_boolean(L, "translate",   SkToBool(mask & SkMatrix::kTranslate_Mask));
    setfield_boolean(L, "scale",       SkToBool(mask & SkMatrix::kScale_Mask));
    setfield_boolean(L, "affine",      SkToBool(mask & SkMatrix::kAffine_Mask));
    setfield_boolean(L, "perspective", SkToBool(mask & SkMatrix::kPerspective_Mask));
    return 1;
}

static int lmatrix_getScaleX(lua_State* L) {
    lua_pushnumber(L, get_obj<SkMatrix>(L,1)->getScaleX());
    return 1;
}

static int lmatrix_getScaleY(lua_State* L) {
    lua_pushnumber(L, get_obj<SkMatrix>(L,1)->getScaleY());
    return 1;
}

static int lmatrix_getTranslateX(lua_State* L) {
    lua_pushnumber(L, get_obj<SkMatrix>(L,1)->getTranslateX());
    return 1;
}

static int lmatrix_getTranslateY(lua_State* L) {
    lua_pushnumber(L, get_obj<SkMatrix>(L,1)->getTranslateY());
    return 1;
}

static const struct luaL_Reg gSkMatrix_Methods[] = {
    { "getType", lmatrix_getType },
    { "getScaleX", lmatrix_getScaleX },
    { "getScaleY", lmatrix_getScaleY },
    { "getTranslateX", lmatrix_getTranslateX },
    { "getTranslateY", lmatrix_getTranslateY },
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
    SkDEBUGFAIL("never get here");
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

static int limage_width(lua_State* L) {
    lua_pushinteger(L, get_ref<SkImage>(L, 1)->width());
    return 1;
}

static int limage_height(lua_State* L) {
    lua_pushinteger(L, get_ref<SkImage>(L, 1)->height());
    return 1;
}

static int limage_gc(lua_State* L) {
    get_ref<SkImage>(L, 1)->unref();
    return 0;
}

static const struct luaL_Reg gSkImage_Methods[] = {
    { "width", limage_width },
    { "height", limage_height },
    { "__gc", limage_gc },
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

static int lsk_newDocumentPDF(lua_State* L) {
    const char* file = NULL;
    if (lua_gettop(L) > 0 && lua_isstring(L, 1)) {
        file = lua_tolstring(L, 1, NULL);
    }

    SkDocument* doc = SkDocument::CreatePDF(file);
    if (NULL == doc) {
        // do I need to push a nil on the stack and return 1?
        return 0;
    } else {
        push_ref(L, doc);
        doc->unref();
        return 1;
    }
}

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

static int lsk_loadImage(lua_State* L) {
    if (lua_gettop(L) > 0 && lua_isstring(L, 1)) {
        const char* name = lua_tolstring(L, 1, NULL);
        SkAutoDataUnref data(SkData::NewFromFileName(name));
        if (data.get()) {
            SkImage* image = SkImage::NewEncodedData(data.get());
            if (image) {
                push_ref(L, image);
                image->unref();
                return 1;
            }
        }
    }
    return 0;
}

static void register_Sk(lua_State* L) {
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "Sk");
    // the Sk table is still on top

    setfield_function(L, "newDocumentPDF", lsk_newDocumentPDF);
    setfield_function(L, "loadImage", lsk_loadImage);
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
    REG_CLASS(L, SkDocument);
    REG_CLASS(L, SkImage);
    REG_CLASS(L, SkPath);
    REG_CLASS(L, SkPaint);
    REG_CLASS(L, SkRRect);
    REG_CLASS(L, SkShader);
    REG_CLASS(L, SkTypeface);
    REG_CLASS(L, SkMatrix);
}

extern "C" int luaopen_skia(lua_State* L);
extern "C" int luaopen_skia(lua_State* L) {
    SkLua::Load(L);
    return 0;
}
