/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkLua.h"

#if SK_SUPPORT_GPU
//#include "src/gpu/GrReducedClip.h"
#endif

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/docs/SkPDFDocument.h"
#include "include/effects/SkBlurImageFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkTo.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/core/SkMakeUnique.h"
#include <new>

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

struct DocHolder {
    sk_sp<SkDocument>           fDoc;
    std::unique_ptr<SkWStream>  fStream;
};

// return the metatable name for a given class
template <typename T> const char* get_mtname();
#define DEF_MTNAME(T)                           \
    template <> const char* get_mtname<T>() {   \
        return #T "_LuaMetaTableName";          \
    }

DEF_MTNAME(SkCanvas)
DEF_MTNAME(SkColorFilter)
DEF_MTNAME(DocHolder)
DEF_MTNAME(SkFont)
DEF_MTNAME(SkImage)
DEF_MTNAME(SkImageFilter)
DEF_MTNAME(SkMatrix)
DEF_MTNAME(SkRRect)
DEF_MTNAME(SkPath)
DEF_MTNAME(SkPaint)
DEF_MTNAME(SkPathEffect)
DEF_MTNAME(SkPicture)
DEF_MTNAME(SkPictureRecorder)
DEF_MTNAME(SkShader)
DEF_MTNAME(SkSurface)
DEF_MTNAME(SkTextBlob)
DEF_MTNAME(SkTypeface)
DEF_MTNAME(SkFontStyle)

template <typename T, typename... Args> T* push_new(lua_State* L, Args&&... args) {
    T* addr = (T*)lua_newuserdata(L, sizeof(T));
    new (addr) T(std::forward<Args>(args)...);
    luaL_getmetatable(L, get_mtname<T>());
    lua_setmetatable(L, -2);
    return addr;
}

template <typename T> void push_obj(lua_State* L, const T& obj) {
    new (lua_newuserdata(L, sizeof(T))) T(obj);
    luaL_getmetatable(L, get_mtname<T>());
    lua_setmetatable(L, -2);
}

template <typename T> T* push_ptr(lua_State* L, T* ptr) {
    *(T**)lua_newuserdata(L, sizeof(T*)) = ptr;
    luaL_getmetatable(L, get_mtname<T>());
    lua_setmetatable(L, -2);
    return ptr;
}

template <typename T> T* push_ref(lua_State* L, T* ref) {
    *(T**)lua_newuserdata(L, sizeof(T*)) = SkSafeRef(ref);
    luaL_getmetatable(L, get_mtname<T>());
    lua_setmetatable(L, -2);
    return ref;
}

template <typename T> void push_ref(lua_State* L, sk_sp<T> sp) {
    *(T**)lua_newuserdata(L, sizeof(T*)) = sp.release();
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

static int lua2int_def(lua_State* L, int index, int defaultValue) {
    if (lua_isnumber(L, index)) {
        return (int)lua_tonumber(L, index);
    } else {
        return defaultValue;
    }
}

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

static SkScalar getarray_scalar(lua_State* L, int stackIndex, int arrayIndex) {
    SkASSERT(lua_istable(L, stackIndex));
    lua_rawgeti(L, stackIndex, arrayIndex);

    SkScalar value = lua2scalar(L, -1);
    lua_pop(L, 1);
    return value;
}

static void getarray_scalars(lua_State* L, int stackIndex, SkScalar dst[], int count) {
    for (int i = 0; i < count; ++i) {
        dst[i] = getarray_scalar(L, stackIndex, i + 1);
    }
}

static void getarray_points(lua_State* L, int stackIndex, SkPoint pts[], int count) {
    getarray_scalars(L, stackIndex, &pts[0].fX, count * 2);
}

static void setarray_number(lua_State* L, int index, double value) {
    lua_pushnumber(L, value);
    lua_rawseti(L, -2, index);
}

static void setarray_scalar(lua_State* L, int index, SkScalar value) {
    setarray_number(L, index, SkScalarToLua(value));
}

static void setarray_string(lua_State* L, int index, const char str[]) {
    lua_pushstring(L, str);
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

void SkLua::pushArrayPoint(const SkPoint array[], int count, const char key[]) {
    lua_newtable(fL);
    for (int i = 0; i < count; ++i) {
        // make it base-1 to match lua convention
        lua_newtable(fL);
        this->pushScalar(array[i].fX, "x");
        this->pushScalar(array[i].fY, "y");
        lua_rawseti(fL, -2, i + 1);
    }
    CHECK_SETFIELD(key);
}

void SkLua::pushArrayScalar(const SkScalar array[], int count, const char key[]) {
    lua_newtable(fL);
    for (int i = 0; i < count; ++i) {
        // make it base-1 to match lua convention
        setarray_scalar(fL, i + 1, array[i]);
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

void SkLua::pushDash(const SkPathEffect::DashInfo& info, const char key[]) {
    lua_newtable(fL);
    setfield_scalar(fL, "phase", info.fPhase);
    this->pushArrayScalar(info.fIntervals, info.fCount, "intervals");
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
    push_ptr(fL, canvas);
    CHECK_SETFIELD(key);
}

void SkLua::pushTextBlob(const SkTextBlob* blob, const char key[]) {
    push_ref(fL, const_cast<SkTextBlob*>(blob));
    CHECK_SETFIELD(key);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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

static SkScalar byte2unit(U8CPU byte) {
    return byte / 255.0f;
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
    return SkColorSetARGB(unit2byte(getfield_scalar_default(L, index, "a", 1)),
                          unit2byte(getfield_scalar_default(L, index, "r", 0)),
                          unit2byte(getfield_scalar_default(L, index, "g", 0)),
                          unit2byte(getfield_scalar_default(L, index, "b", 0)));
}

static SkRect* lua2rect(lua_State* L, int index, SkRect* rect) {
    rect->set(getfield_scalar_default(L, index, "left", 0),
              getfield_scalar_default(L, index, "top", 0),
              getfield_scalar(L, index, "right"),
              getfield_scalar(L, index, "bottom"));
    return rect;
}

static int lcanvas_clear(lua_State* L) {
    get_ref<SkCanvas>(L, 1)->clear(0);
    return 0;
}

static int lcanvas_drawColor(lua_State* L) {
    get_ref<SkCanvas>(L, 1)->drawColor(lua2color(L, 2));
    return 0;
}

static int lcanvas_drawPaint(lua_State* L) {
    get_ref<SkCanvas>(L, 1)->drawPaint(*get_obj<SkPaint>(L, 2));
    return 0;
}

static int lcanvas_drawRect(lua_State* L) {
    SkRect rect;
    lua2rect(L, 2, &rect);
    const SkPaint* paint = get_obj<SkPaint>(L, 3);
    get_ref<SkCanvas>(L, 1)->drawRect(rect, *paint);
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

static SkPaint* lua2OptionalPaint(lua_State* L, int index, SkPaint* paint) {
    if (lua_isnumber(L, index)) {
        paint->setAlpha(SkScalarRoundToInt(lua2scalar(L, index) * 255));
        return paint;
    } else if (lua_isuserdata(L, index)) {
        const SkPaint* ptr = get_obj<SkPaint>(L, index);
        if (ptr) {
            *paint = *ptr;
            return paint;
        }
    }
    return nullptr;
}

static int lcanvas_drawImage(lua_State* L) {
    SkCanvas* canvas = get_ref<SkCanvas>(L, 1);
    SkImage* image = get_ref<SkImage>(L, 2);
    if (nullptr == image) {
        return 0;
    }
    SkScalar x = lua2scalar(L, 3);
    SkScalar y = lua2scalar(L, 4);

    SkPaint paint;
    canvas->drawImage(image, x, y, lua2OptionalPaint(L, 5, &paint));
    return 0;
}

static int lcanvas_drawImageRect(lua_State* L) {
    SkCanvas* canvas = get_ref<SkCanvas>(L, 1);
    SkImage* image = get_ref<SkImage>(L, 2);
    if (nullptr == image) {
        return 0;
    }

    SkRect srcR, dstR;
    SkRect* srcRPtr = nullptr;
    if (!lua_isnil(L, 3)) {
        srcRPtr = lua2rect(L, 3, &srcR);
    }
    lua2rect(L, 4, &dstR);

    SkPaint paint;
    canvas->legacy_drawImageRect(image, srcRPtr, dstR, lua2OptionalPaint(L, 5, &paint));
    return 0;
}

static int lcanvas_drawPatch(lua_State* L) {
    SkPoint cubics[12];
    SkColor colorStorage[4];
    SkPoint texStorage[4];

    const SkColor* colors = nullptr;
    const SkPoint* texs = nullptr;

    getarray_points(L, 2, cubics, 12);

    colorStorage[0] = SK_ColorRED;
    colorStorage[1] = SK_ColorGREEN;
    colorStorage[2] = SK_ColorBLUE;
    colorStorage[3] = SK_ColorGRAY;

    if (lua_isnil(L, 4)) {
        colors = colorStorage;
    } else {
        getarray_points(L, 4, texStorage, 4);
        texs = texStorage;
    }

    get_ref<SkCanvas>(L, 1)->drawPatch(cubics, colors, texs, *get_obj<SkPaint>(L, 5));
    return 0;
}

static int lcanvas_drawPath(lua_State* L) {
    get_ref<SkCanvas>(L, 1)->drawPath(*get_obj<SkPath>(L, 2),
                                      *get_obj<SkPaint>(L, 3));
    return 0;
}

// drawPicture(pic, x, y, paint)
static int lcanvas_drawPicture(lua_State* L) {
    SkCanvas* canvas = get_ref<SkCanvas>(L, 1);
    SkPicture* picture = get_ref<SkPicture>(L, 2);
    SkScalar x = lua2scalar_def(L, 3, 0);
    SkScalar y = lua2scalar_def(L, 4, 0);
    SkMatrix matrix, *matrixPtr = nullptr;
    if (x || y) {
        matrix.setTranslate(x, y);
        matrixPtr = &matrix;
    }
    SkPaint paint;
    canvas->drawPicture(picture, matrixPtr, lua2OptionalPaint(L, 5, &paint));
    return 0;
}

static int lcanvas_drawText(lua_State* L) {
    if (lua_gettop(L) < 5) {
        return 0;
    }

    // TODO: restore this logic based on SkFont instead of SkPaint
#if 0
    if (lua_isstring(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4)) {
        size_t len;
        const char* text = lua_tolstring(L, 2, &len);
        get_ref<SkCanvas>(L, 1)->drawSimpleText(
                text, len, kUTF8_SkTextEncoding,
                lua2scalar(L, 3), lua2scalar(L, 4),
                SkFont::LEGACY_ExtractFromPaint(*get_obj<SkPaint>(L, 5)),
                *get_obj<SkPaint>(L, 5));
    }
#endif
    return 0;
}

static int lcanvas_drawTextBlob(lua_State* L) {
    const SkTextBlob* blob = get_ref<SkTextBlob>(L, 2);
    SkScalar x = lua2scalar(L, 3);
    SkScalar y = lua2scalar(L, 4);
    const SkPaint& paint = *get_obj<SkPaint>(L, 5);
    get_ref<SkCanvas>(L, 1)->drawTextBlob(blob, x, y, paint);
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

static int lcanvas_saveLayer(lua_State* L) {
    SkPaint paint;
    lua_pushinteger(L, get_ref<SkCanvas>(L, 1)->saveLayer(nullptr, lua2OptionalPaint(L, 2, &paint)));
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

static int lcanvas_concat(lua_State* L) {
    get_ref<SkCanvas>(L, 1)->concat(*get_obj<SkMatrix>(L, 2));
    return 0;
}

static int lcanvas_newSurface(lua_State* L) {
    int width = lua2int_def(L, 2, 0);
    int height = lua2int_def(L, 3, 0);
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    auto surface = get_ref<SkCanvas>(L, 1)->makeSurface(info);
    if (nullptr == surface) {
        lua_pushnil(L);
    } else {
        push_ref(L, surface);
    }
    return 1;
}

static int lcanvas_gc(lua_State* L) {
    // don't know how to track a ptr...
    return 0;
}

const struct luaL_Reg gSkCanvas_Methods[] = {
    { "clear", lcanvas_clear },
    { "drawColor", lcanvas_drawColor },
    { "drawPaint", lcanvas_drawPaint },
    { "drawRect", lcanvas_drawRect },
    { "drawOval", lcanvas_drawOval },
    { "drawCircle", lcanvas_drawCircle },
    { "drawImage", lcanvas_drawImage },
    { "drawImageRect", lcanvas_drawImageRect },
    { "drawPatch", lcanvas_drawPatch },
    { "drawPath", lcanvas_drawPath },
    { "drawPicture", lcanvas_drawPicture },
    { "drawText", lcanvas_drawText },
    { "drawTextBlob", lcanvas_drawTextBlob },
    { "getSaveCount", lcanvas_getSaveCount },
    { "getTotalMatrix", lcanvas_getTotalMatrix },
    { "save", lcanvas_save },
    { "saveLayer", lcanvas_saveLayer },
    { "restore", lcanvas_restore },
    { "scale", lcanvas_scale },
    { "translate", lcanvas_translate },
    { "rotate", lcanvas_rotate },
    { "concat", lcanvas_concat },

    { "newSurface", lcanvas_newSurface },

    { "__gc", lcanvas_gc },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static int ldocument_beginPage(lua_State* L) {
    const SkRect* contentPtr = nullptr;
    push_ptr(L, get_obj<DocHolder>(L, 1)->fDoc->beginPage(lua2scalar(L, 2),
                                                          lua2scalar(L, 3),
                                                          contentPtr));
    return 1;
}

static int ldocument_endPage(lua_State* L) {
    get_obj<DocHolder>(L, 1)->fDoc->endPage();
    return 0;
}

static int ldocument_close(lua_State* L) {
    get_obj<DocHolder>(L, 1)->fDoc->close();
    return 0;
}

static int ldocument_gc(lua_State* L) {
    get_obj<DocHolder>(L, 1)->~DocHolder();
    return 0;
}

static const struct luaL_Reg gDocHolder_Methods[] = {
    { "beginPage", ldocument_beginPage },
    { "endPage", ldocument_endPage },
    { "close", ldocument_close },
    { "__gc", ldocument_gc },
    { nullptr, nullptr }
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

static int lpaint_isDither(lua_State* L) {
    lua_pushboolean(L, get_obj<SkPaint>(L, 1)->isDither());
    return 1;
}

static int lpaint_setDither(lua_State* L) {
    get_obj<SkPaint>(L, 1)->setDither(lua2bool(L, 2));
    return 0;
}

static int lpaint_getAlpha(lua_State* L) {
    SkLua(L).pushScalar(byte2unit(get_obj<SkPaint>(L, 1)->getAlpha()));
    return 1;
}

static int lpaint_setAlpha(lua_State* L) {
    get_obj<SkPaint>(L, 1)->setAlpha(unit2byte(lua2scalar(L, 2)));
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

static int lpaint_getFilterQuality(lua_State* L) {
    SkLua(L).pushU32(get_obj<SkPaint>(L, 1)->getFilterQuality());
    return 1;
}

static int lpaint_setFilterQuality(lua_State* L) {
    int level = lua2int_def(L, 2, -1);
    if (level >= 0 && level <= 3) {
        get_obj<SkPaint>(L, 1)->setFilterQuality((SkFilterQuality)level);
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

static int lpaint_getStrokeCap(lua_State* L) {
    SkLua(L).pushU32(get_obj<SkPaint>(L, 1)->getStrokeCap());
    return 1;
}

static int lpaint_getStrokeJoin(lua_State* L) {
    SkLua(L).pushU32(get_obj<SkPaint>(L, 1)->getStrokeJoin());
    return 1;
}

static int lpaint_getStrokeWidth(lua_State* L) {
    SkLua(L).pushScalar(get_obj<SkPaint>(L, 1)->getStrokeWidth());
    return 1;
}

static int lpaint_setStrokeWidth(lua_State* L) {
    get_obj<SkPaint>(L, 1)->setStrokeWidth(lua2scalar(L, 2));
    return 0;
}

static int lpaint_getStrokeMiter(lua_State* L) {
    SkLua(L).pushScalar(get_obj<SkPaint>(L, 1)->getStrokeMiter());
    return 1;
}

static int lpaint_getEffects(lua_State* L) {
    const SkPaint* paint = get_obj<SkPaint>(L, 1);

    lua_newtable(L);
    setfield_bool_if(L, "looper",      !!paint->getLooper());
    setfield_bool_if(L, "pathEffect",  !!paint->getPathEffect());
    setfield_bool_if(L, "maskFilter",  !!paint->getMaskFilter());
    setfield_bool_if(L, "shader",      !!paint->getShader());
    setfield_bool_if(L, "colorFilter", !!paint->getColorFilter());
    setfield_bool_if(L, "imageFilter", !!paint->getImageFilter());
    return 1;
}

static int lpaint_getColorFilter(lua_State* L) {
    const SkPaint* paint = get_obj<SkPaint>(L, 1);
    SkColorFilter* cf = paint->getColorFilter();
    if (cf) {
        push_ref(L, cf);
        return 1;
    }
    return 0;
}

static int lpaint_setColorFilter(lua_State* L) {
    SkPaint* paint = get_obj<SkPaint>(L, 1);
    paint->setColorFilter(sk_ref_sp(get_ref<SkColorFilter>(L, 2)));
    return 0;
}

static int lpaint_getImageFilter(lua_State* L) {
    const SkPaint* paint = get_obj<SkPaint>(L, 1);
    SkImageFilter* imf = paint->getImageFilter();
    if (imf) {
        push_ref(L, imf);
        return 1;
    }
    return 0;
}

static int lpaint_setImageFilter(lua_State* L) {
    SkPaint* paint = get_obj<SkPaint>(L, 1);
    paint->setImageFilter(sk_ref_sp(get_ref<SkImageFilter>(L, 2)));
    return 0;
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

static int lpaint_setShader(lua_State* L) {
    SkPaint* paint = get_obj<SkPaint>(L, 1);
    paint->setShader(sk_ref_sp(get_ref<SkShader>(L, 2)));
    return 0;
}

static int lpaint_getPathEffect(lua_State* L) {
    const SkPaint* paint = get_obj<SkPaint>(L, 1);
    SkPathEffect* pe = paint->getPathEffect();
    if (pe) {
        push_ref(L, pe);
        return 1;
    }
    return 0;
}

static int lpaint_getFillPath(lua_State* L) {
    const SkPaint* paint = get_obj<SkPaint>(L, 1);
    const SkPath* path = get_obj<SkPath>(L, 2);

    SkPath fillpath;
    paint->getFillPath(*path, &fillpath);

    SkLua lua(L);
    lua.pushPath(fillpath);

    return 1;
}

static int lpaint_gc(lua_State* L) {
    get_obj<SkPaint>(L, 1)->~SkPaint();
    return 0;
}

static const struct luaL_Reg gSkPaint_Methods[] = {
    { "isAntiAlias", lpaint_isAntiAlias },
    { "setAntiAlias", lpaint_setAntiAlias },
    { "isDither", lpaint_isDither },
    { "setDither", lpaint_setDither },
    { "getFilterQuality", lpaint_getFilterQuality },
    { "setFilterQuality", lpaint_setFilterQuality },
    { "getAlpha", lpaint_getAlpha },
    { "setAlpha", lpaint_setAlpha },
    { "getColor", lpaint_getColor },
    { "setColor", lpaint_setColor },
    { "getStroke", lpaint_getStroke },
    { "setStroke", lpaint_setStroke },
    { "getStrokeCap", lpaint_getStrokeCap },
    { "getStrokeJoin", lpaint_getStrokeJoin },
    { "getStrokeWidth", lpaint_getStrokeWidth },
    { "setStrokeWidth", lpaint_setStrokeWidth },
    { "getStrokeMiter", lpaint_getStrokeMiter },
    { "getEffects", lpaint_getEffects },
    { "getColorFilter", lpaint_getColorFilter },
    { "setColorFilter", lpaint_setColorFilter },
    { "getImageFilter", lpaint_getImageFilter },
    { "setImageFilter", lpaint_setImageFilter },
    { "getShader", lpaint_getShader },
    { "setShader", lpaint_setShader },
    { "getPathEffect", lpaint_getPathEffect },
    { "getFillPath", lpaint_getFillPath },
    { "__gc", lpaint_gc },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static int lfont_getSize(lua_State* L) {
    SkLua(L).pushScalar(get_obj<SkFont>(L, 1)->getSize());
    return 1;
}

static int lfont_getScaleX(lua_State* L) {
    SkLua(L).pushScalar(get_obj<SkFont>(L, 1)->getScaleX());
    return 1;
}

static int lfont_getSkewX(lua_State* L) {
    SkLua(L).pushScalar(get_obj<SkFont>(L, 1)->getSkewX());
    return 1;
}

static int lfont_setSize(lua_State* L) {
    get_obj<SkFont>(L, 1)->setSize(lua2scalar(L, 2));
    return 0;
}

static int lfont_getTypeface(lua_State* L) {
    push_ref(L, get_obj<SkFont>(L, 1)->getTypefaceOrDefault());
    return 1;
}

static int lfont_setTypeface(lua_State* L) {
    get_obj<SkFont>(L, 1)->setTypeface(sk_ref_sp(get_ref<SkTypeface>(L, 2)));
    return 0;
}

static int lfont_getHinting(lua_State* L) {
    SkLua(L).pushU32((unsigned)get_obj<SkFont>(L, 1)->getHinting());
    return 1;
}

static int lfont_getFontID(lua_State* L) {
    SkTypeface* face = get_obj<SkFont>(L, 1)->getTypefaceOrDefault();
    SkLua(L).pushU32(SkTypeface::UniqueID(face));
    return 1;
}

static int lfont_measureText(lua_State* L) {
    if (lua_isstring(L, 2)) {
        size_t len;
        const char* text = lua_tolstring(L, 2, &len);
        SkLua(L).pushScalar(get_obj<SkFont>(L, 1)->measureText(text, len, kUTF8_SkTextEncoding));
        return 1;
    }
    return 0;
}

static int lfont_getMetrics(lua_State* L) {
    SkFontMetrics fm;
    SkScalar height = get_obj<SkFont>(L, 1)->getMetrics(&fm);

    lua_newtable(L);
    setfield_scalar(L, "top", fm.fTop);
    setfield_scalar(L, "ascent", fm.fAscent);
    setfield_scalar(L, "descent", fm.fDescent);
    setfield_scalar(L, "bottom", fm.fBottom);
    setfield_scalar(L, "leading", fm.fLeading);
    SkLua(L).pushScalar(height);
    return 2;
}

static int lfont_gc(lua_State* L) {
    get_obj<SkFont>(L, 1)->~SkFont();
    return 0;
}

static const struct luaL_Reg gSkFont_Methods[] = {
    { "getSize", lfont_getSize },
    { "setSize", lfont_setSize },
    { "getScaleX", lfont_getScaleX },
    { "getSkewX", lfont_getSkewX },
    { "getTypeface", lfont_getTypeface },
    { "setTypeface", lfont_setTypeface },
    { "getHinting", lfont_getHinting },
    { "getFontID", lfont_getFontID },
    { "measureText", lfont_measureText },
    { "getMetrics", lfont_getMetrics },
    { "__gc", lfont_gc },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static const char* mode2string(SkTileMode mode) {
    static const char* gNames[] = { "clamp", "repeat", "mirror", "decal" };
    SkASSERT((unsigned)mode < SK_ARRAY_COUNT(gNames));
    return gNames[static_cast<int>(mode)];
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

static int lshader_isAImage(lua_State* L) {
    SkShader* shader = get_ref<SkShader>(L, 1);
    if (shader) {
        SkMatrix matrix;
        SkTileMode modes[2];
        if (SkImage* image = shader->isAImage(&matrix, modes)) {
            lua_newtable(L);
            setfield_number(L, "id", image->uniqueID());
            setfield_number(L, "width", image->width());
            setfield_number(L, "height", image->height());
            setfield_string(L, "tileX", mode2string(modes[0]));
            setfield_string(L, "tileY", mode2string(modes[1]));
            return 1;
        }
    }
    return 0;
}

static int lshader_asAGradient(lua_State* L) {
    SkShader* shader = get_ref<SkShader>(L, 1);
    if (shader) {
        SkShader::GradientInfo info;
        sk_bzero(&info, sizeof(info));

        SkShader::GradientType t = shader->asAGradient(&info);

        if (SkShader::kNone_GradientType != t) {
            SkAutoTArray<SkScalar> pos(info.fColorCount);
            info.fColorOffsets = pos.get();
            shader->asAGradient(&info);

            lua_newtable(L);
            setfield_string(L,  "type",           gradtype2string(t));
            setfield_string(L,  "tile",           mode2string((SkTileMode)info.fTileMode));
            setfield_number(L,  "colorCount",     info.fColorCount);

            lua_newtable(L);
            for (int i = 0; i < info.fColorCount; i++) {
                // Lua uses 1-based indexing
                setarray_scalar(L, i+1, pos[i]);
            }
            lua_setfield(L, -2, "positions");

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
    { "isAImage",       lshader_isAImage },
    { "asAGradient",    lshader_asAGradient },
    { "__gc",           lshader_gc },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static int lpatheffect_asADash(lua_State* L) {
    SkPathEffect* pe = get_ref<SkPathEffect>(L, 1);
    if (pe) {
        SkPathEffect::DashInfo info;
        SkPathEffect::DashType dashType = pe->asADash(&info);
        if (SkPathEffect::kDash_DashType == dashType) {
            SkAutoTArray<SkScalar> intervals(info.fCount);
            info.fIntervals = intervals.get();
            pe->asADash(&info);
            SkLua(L).pushDash(info);
            return 1;
        }
    }
    return 0;
}

static int lpatheffect_gc(lua_State* L) {
    get_ref<SkPathEffect>(L, 1)->unref();
    return 0;
}

static const struct luaL_Reg gSkPathEffect_Methods[] = {
    { "asADash",        lpatheffect_asADash },
    { "__gc",           lpatheffect_gc },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static int lpcolorfilter_gc(lua_State* L) {
    get_ref<SkColorFilter>(L, 1)->unref();
    return 0;
}

static const struct luaL_Reg gSkColorFilter_Methods[] = {
    { "__gc",       lpcolorfilter_gc },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static int lpimagefilter_gc(lua_State* L) {
    get_ref<SkImageFilter>(L, 1)->unref();
    return 0;
}

static const struct luaL_Reg gSkImageFilter_Methods[] = {
    { "__gc",       lpimagefilter_gc },
    { nullptr, nullptr }
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

static int lmatrix_invert(lua_State* L) {
    lua_pushboolean(L, get_obj<SkMatrix>(L, 1)->invert(get_obj<SkMatrix>(L, 2)));
    return 1;
}

static int lmatrix_mapXY(lua_State* L) {
    SkPoint pt = { lua2scalar(L, 2), lua2scalar(L, 3) };
    get_obj<SkMatrix>(L, 1)->mapPoints(&pt, &pt, 1);
    lua_pushnumber(L, pt.x());
    lua_pushnumber(L, pt.y());
    return 2;
}

static int lmatrix_setRectToRect(lua_State* L) {
    SkMatrix* matrix = get_obj<SkMatrix>(L, 1);
    SkRect srcR, dstR;
    lua2rect(L, 2, &srcR);
    lua2rect(L, 3, &dstR);
    const char* scaleToFitStr = lua_tostring(L, 4);
    SkMatrix::ScaleToFit scaleToFit = SkMatrix::kFill_ScaleToFit;

    if (scaleToFitStr) {
        const struct {
            const char* fName;
            SkMatrix::ScaleToFit fScaleToFit;
        } rec[] = {
            { "fill",   SkMatrix::kFill_ScaleToFit },
            { "start",  SkMatrix::kStart_ScaleToFit },
            { "center", SkMatrix::kCenter_ScaleToFit },
            { "end",    SkMatrix::kEnd_ScaleToFit },
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(rec); ++i) {
            if (strcmp(rec[i].fName, scaleToFitStr) == 0) {
                scaleToFit = rec[i].fScaleToFit;
                break;
            }
        }
    }

    matrix->setRectToRect(srcR, dstR, scaleToFit);
    return 0;
}

static const struct luaL_Reg gSkMatrix_Methods[] = {
    { "getType", lmatrix_getType },
    { "getScaleX", lmatrix_getScaleX },
    { "getScaleY", lmatrix_getScaleY },
    { "getTranslateX", lmatrix_getTranslateX },
    { "getTranslateY", lmatrix_getTranslateY },
    { "setRectToRect", lmatrix_setRectToRect },
    { "invert", lmatrix_invert },
    { "mapXY", lmatrix_mapXY },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static int lpath_getBounds(lua_State* L) {
    SkLua(L).pushRect(get_obj<SkPath>(L, 1)->getBounds());
    return 1;
}

static const char* fill_type_to_str(SkPath::FillType fill) {
    switch (fill) {
        case SkPath::kEvenOdd_FillType:
            return "even-odd";
        case SkPath::kWinding_FillType:
            return "winding";
        case SkPath::kInverseEvenOdd_FillType:
            return "inverse-even-odd";
        case SkPath::kInverseWinding_FillType:
            return "inverse-winding";
    }
    return "unknown";
}

static int lpath_getFillType(lua_State* L) {
    SkPath::FillType fill = get_obj<SkPath>(L, 1)->getFillType();
    SkLua(L).pushString(fill_type_to_str(fill));
    return 1;
}

static SkString segment_masks_to_str(uint32_t segmentMasks) {
    SkString result;
    bool first = true;
    if (SkPath::kLine_SegmentMask & segmentMasks) {
        result.append("line");
        first = false;
        SkDEBUGCODE(segmentMasks &= ~SkPath::kLine_SegmentMask;)
    }
    if (SkPath::kQuad_SegmentMask & segmentMasks) {
        if (!first) {
            result.append(" ");
        }
        result.append("quad");
        first = false;
        SkDEBUGCODE(segmentMasks &= ~SkPath::kQuad_SegmentMask;)
    }
    if (SkPath::kConic_SegmentMask & segmentMasks) {
        if (!first) {
            result.append(" ");
        }
        result.append("conic");
        first = false;
        SkDEBUGCODE(segmentMasks &= ~SkPath::kConic_SegmentMask;)
    }
    if (SkPath::kCubic_SegmentMask & segmentMasks) {
        if (!first) {
            result.append(" ");
        }
        result.append("cubic");
        SkDEBUGCODE(segmentMasks &= ~SkPath::kCubic_SegmentMask;)
    }
    SkASSERT(0 == segmentMasks);
    return result;
}

static int lpath_getSegmentTypes(lua_State* L) {
    uint32_t segMasks = get_obj<SkPath>(L, 1)->getSegmentMasks();
    SkLua(L).pushString(segment_masks_to_str(segMasks));
    return 1;
}

static int lpath_isConvex(lua_State* L) {
    bool isConvex = SkPath::kConvex_Convexity == get_obj<SkPath>(L, 1)->getConvexity();
    SkLua(L).pushBool(isConvex);
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

static int lpath_isNestedFillRects(lua_State* L) {
    SkRect rects[2];
    SkPath::Direction dirs[2];
    bool pred = get_obj<SkPath>(L, 1)->isNestedFillRects(rects, dirs);
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

static int lpath_countPoints(lua_State* L) {
    lua_pushinteger(L, get_obj<SkPath>(L, 1)->countPoints());
    return 1;
}

static int lpath_getVerbs(lua_State* L) {
    const SkPath* path = get_obj<SkPath>(L, 1);
    SkPath::Iter iter(*path, false);
    SkPoint pts[4];

    lua_newtable(L);

    bool done = false;
    int i = 0;
    do {
        switch (iter.next(pts, true)) {
            case SkPath::kMove_Verb:
                setarray_string(L, ++i, "move");
                break;
            case SkPath::kClose_Verb:
                setarray_string(L, ++i, "close");
                break;
            case SkPath::kLine_Verb:
                setarray_string(L, ++i, "line");
                break;
            case SkPath::kQuad_Verb:
                setarray_string(L, ++i, "quad");
                break;
            case SkPath::kConic_Verb:
                setarray_string(L, ++i, "conic");
                break;
            case SkPath::kCubic_Verb:
                setarray_string(L, ++i, "cubic");
                break;
            case SkPath::kDone_Verb:
                setarray_string(L, ++i, "done");
                done = true;
                break;
        }
    } while (!done);

    return 1;
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
    { "getFillType", lpath_getFillType },
    { "getSegmentTypes", lpath_getSegmentTypes },
    { "getVerbs", lpath_getVerbs },
    { "isConvex", lpath_isConvex },
    { "isEmpty", lpath_isEmpty },
    { "isRect", lpath_isRect },
    { "isNestedFillRects", lpath_isNestedFillRects },
    { "countPoints", lpath_countPoints },
    { "reset", lpath_reset },
    { "moveTo", lpath_moveTo },
    { "lineTo", lpath_lineTo },
    { "quadTo", lpath_quadTo },
    { "cubicTo", lpath_cubicTo },
    { "close", lpath_close },
    { "__gc", lpath_gc },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static const char* rrect_type(const SkRRect& rr) {
    switch (rr.getType()) {
        case SkRRect::kEmpty_Type: return "empty";
        case SkRRect::kRect_Type: return "rect";
        case SkRRect::kOval_Type: return "oval";
        case SkRRect::kSimple_Type: return "simple";
        case SkRRect::kNinePatch_Type: return "nine-patch";
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
    int corner = SkToInt(lua_tointeger(L, 2));
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
    { nullptr, nullptr }
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

static int limage_newShader(lua_State* L) {
    SkTileMode tmode = SkTileMode::kClamp;
    const SkMatrix* localM = nullptr;
    push_ref(L, get_ref<SkImage>(L, 1)->makeShader(tmode, tmode, localM));
    return 1;
}

static int limage_gc(lua_State* L) {
    get_ref<SkImage>(L, 1)->unref();
    return 0;
}

static const struct luaL_Reg gSkImage_Methods[] = {
    { "width", limage_width },
    { "height", limage_height },
    { "newShader", limage_newShader },
    { "__gc", limage_gc },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static int lsurface_width(lua_State* L) {
    lua_pushinteger(L, get_ref<SkSurface>(L, 1)->width());
    return 1;
}

static int lsurface_height(lua_State* L) {
    lua_pushinteger(L, get_ref<SkSurface>(L, 1)->height());
    return 1;
}

static int lsurface_getCanvas(lua_State* L) {
    SkCanvas* canvas = get_ref<SkSurface>(L, 1)->getCanvas();
    if (nullptr == canvas) {
        lua_pushnil(L);
    } else {
        push_ptr(L, canvas);
        // note: we don't unref canvas, since getCanvas did not ref it.
        // warning: this is weird: now Lua owns a ref on this canvas, but what if they let
        // the real owner (the surface) go away, but still hold onto the canvas?
        // *really* we want to sort of ref the surface again, but have the native object
        // know that it is supposed to be treated as a canvas...
    }
    return 1;
}

static int lsurface_newImageSnapshot(lua_State* L) {
    sk_sp<SkImage> image = get_ref<SkSurface>(L, 1)->makeImageSnapshot();
    if (!image) {
        lua_pushnil(L);
    } else {
        push_ref(L, image);
    }
    return 1;
}

static int lsurface_newSurface(lua_State* L) {
    int width = lua2int_def(L, 2, 0);
    int height = lua2int_def(L, 3, 0);
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    auto surface = get_ref<SkSurface>(L, 1)->makeSurface(info);
    if (nullptr == surface) {
        lua_pushnil(L);
    } else {
        push_ref(L, surface);
    }
    return 1;
}

static int lsurface_gc(lua_State* L) {
    get_ref<SkSurface>(L, 1)->unref();
    return 0;
}

static const struct luaL_Reg gSkSurface_Methods[] = {
    { "width", lsurface_width },
    { "height", lsurface_height },
    { "getCanvas", lsurface_getCanvas },
    { "newImageSnapshot", lsurface_newImageSnapshot },
    { "newSurface", lsurface_newSurface },
    { "__gc", lsurface_gc },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static int lpicturerecorder_beginRecording(lua_State* L) {
    const SkScalar w = lua2scalar_def(L, 2, -1);
    const SkScalar h = lua2scalar_def(L, 3, -1);
    if (w <= 0 || h <= 0) {
        lua_pushnil(L);
        return 1;
    }

    SkCanvas* canvas = get_obj<SkPictureRecorder>(L, 1)->beginRecording(w, h);
    if (nullptr == canvas) {
        lua_pushnil(L);
        return 1;
    }

    push_ptr(L, canvas);
    return 1;
}

static int lpicturerecorder_getCanvas(lua_State* L) {
    SkCanvas* canvas = get_obj<SkPictureRecorder>(L, 1)->getRecordingCanvas();
    if (nullptr == canvas) {
        lua_pushnil(L);
        return 1;
    }
    push_ptr(L, canvas);
    return 1;
}

static int lpicturerecorder_endRecording(lua_State* L) {
    sk_sp<SkPicture> pic = get_obj<SkPictureRecorder>(L, 1)->finishRecordingAsPicture();
    if (!pic) {
        lua_pushnil(L);
        return 1;
    }
    push_ref(L, std::move(pic));
    return 1;
}

static int lpicturerecorder_gc(lua_State* L) {
    get_obj<SkPictureRecorder>(L, 1)->~SkPictureRecorder();
    return 0;
}

static const struct luaL_Reg gSkPictureRecorder_Methods[] = {
    { "beginRecording", lpicturerecorder_beginRecording },
    { "getCanvas", lpicturerecorder_getCanvas },
    { "endRecording", lpicturerecorder_endRecording },
    { "__gc", lpicturerecorder_gc },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static int lpicture_width(lua_State* L) {
    lua_pushnumber(L, get_ref<SkPicture>(L, 1)->cullRect().width());
    return 1;
}

static int lpicture_height(lua_State* L) {
    lua_pushnumber(L, get_ref<SkPicture>(L, 1)->cullRect().height());
    return 1;
}

static int lpicture_gc(lua_State* L) {
    get_ref<SkPicture>(L, 1)->unref();
    return 0;
}

static const struct luaL_Reg gSkPicture_Methods[] = {
    { "width", lpicture_width },
    { "height", lpicture_height },
    { "__gc", lpicture_gc },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static int ltextblob_bounds(lua_State* L) {
    SkLua(L).pushRect(get_ref<SkTextBlob>(L, 1)->bounds());
    return 1;
}

static int ltextblob_gc(lua_State* L) {
    SkSafeUnref(get_ref<SkTextBlob>(L, 1));
    return 0;
}

static const struct luaL_Reg gSkTextBlob_Methods[] = {
    { "bounds", ltextblob_bounds },
    { "__gc", ltextblob_gc },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static int ltypeface_getFamilyName(lua_State* L) {
    SkString str;
    get_ref<SkTypeface>(L, 1)->getFamilyName(&str);
    lua_pushstring(L, str.c_str());
    return 1;
}

static int ltypeface_getStyle(lua_State* L) {
    push_obj(L, get_ref<SkTypeface>(L, 1)->fontStyle());
    return 1;
}

static int ltypeface_gc(lua_State* L) {
    SkSafeUnref(get_ref<SkTypeface>(L, 1));
    return 0;
}

static const struct luaL_Reg gSkTypeface_Methods[] = {
    { "getFamilyName", ltypeface_getFamilyName },
    { "getStyle", ltypeface_getStyle },
    { "__gc", ltypeface_gc },
    { nullptr, nullptr }
};

///////////////////////////////////////////////////////////////////////////////

static int lfontstyle_weight(lua_State* L) {
    lua_pushnumber(L, get_ref<SkFontStyle>(L, 1)->weight());
    return 1;
}

static int lfontstyle_width(lua_State* L) {
    lua_pushnumber(L, get_ref<SkFontStyle>(L, 1)->width());
    return 1;
}

static int lfontstyle_slant(lua_State* L) {
    lua_pushnumber(L, get_ref<SkFontStyle>(L, 1)->slant());
    return 1;
}

static int lfontstyle_gc(lua_State* L) {
    get_obj<SkFontStyle>(L, 1)->~SkFontStyle();
    return 0;
}

static const struct luaL_Reg gSkFontStyle_Methods[] = {
    { "weight", lfontstyle_weight },
    { "width", lfontstyle_width },
    { "slant", lfontstyle_slant },
    { "__gc", lfontstyle_gc },
    { nullptr, nullptr }
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
    const char* filename = nullptr;
    if (lua_gettop(L) > 0 && lua_isstring(L, 1)) {
        filename = lua_tolstring(L, 1, nullptr);
    }
    if (!filename) {
        return 0;
    }
    auto file = skstd::make_unique<SkFILEWStream>(filename);
    if (!file->isValid()) {
        return 0;
    }
    auto doc = SkPDF::MakeDocument(file.get());
    if (!doc) {
        return 0;
    }
    push_ptr(L, new DocHolder{std::move(doc), std::move(file)});
    return 1;
}

static int lsk_newBlurImageFilter(lua_State* L) {
    SkScalar sigmaX = lua2scalar_def(L, 1, 0);
    SkScalar sigmaY = lua2scalar_def(L, 2, 0);
    sk_sp<SkImageFilter> imf(SkBlurImageFilter::Make(sigmaX, sigmaY, nullptr));
    if (!imf) {
        lua_pushnil(L);
    } else {
        push_ref(L, std::move(imf));
    }
    return 1;
}

static int lsk_newLinearGradient(lua_State* L) {
    SkScalar x0 = lua2scalar_def(L, 1, 0);
    SkScalar y0 = lua2scalar_def(L, 2, 0);
    SkColor  c0 = lua2color(L, 3);
    SkScalar x1 = lua2scalar_def(L, 4, 0);
    SkScalar y1 = lua2scalar_def(L, 5, 0);
    SkColor  c1 = lua2color(L, 6);

    SkPoint pts[] = { { x0, y0 }, { x1, y1 } };
    SkColor colors[] = { c0, c1 };
    sk_sp<SkShader> s(SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp));
    if (!s) {
        lua_pushnil(L);
    } else {
        push_ref(L, std::move(s));
    }
    return 1;
}

static int lsk_newMatrix(lua_State* L) {
    push_new<SkMatrix>(L)->reset();
    return 1;
}

static int lsk_newPaint(lua_State* L) {
    push_new<SkPaint>(L);
    return 1;
}

static int lsk_newPath(lua_State* L) {
    push_new<SkPath>(L);
    return 1;
}

static int lsk_newPictureRecorder(lua_State* L) {
    push_new<SkPictureRecorder>(L);
    return 1;
}

static int lsk_newRRect(lua_State* L) {
    push_new<SkRRect>(L)->setEmpty();
    return 1;
}

// Sk.newTextBlob(text, rect, paint)
static int lsk_newTextBlob(lua_State* L) {
    const char* text = lua_tolstring(L, 1, nullptr);
    SkRect bounds;
    lua2rect(L, 2, &bounds);

    std::unique_ptr<SkShaper> shaper = SkShaper::Make();

    // TODO: restore this logic based on SkFont instead of SkPaint
#if 0
    const SkPaint& paint = *get_obj<SkPaint>(L, 3);
    SkFont font = SkFont::LEGACY_ExtractFromPaint(paint);
#else
    SkFont font;
#endif
    SkTextBlobBuilderRunHandler builder(text, { bounds.left(), bounds.top() });
    shaper->shape(text, strlen(text), font, true, bounds.width(), &builder);

    push_ref<SkTextBlob>(L, builder.makeBlob());
    SkLua(L).pushScalar(builder.endPoint().fY);
    return 2;
}

static int lsk_newTypeface(lua_State* L) {
    const char* name = nullptr;
    SkFontStyle style;

    int count = lua_gettop(L);
    if (count > 0 && lua_isstring(L, 1)) {
        name = lua_tolstring(L, 1, nullptr);
        if (count > 1) {
            SkFontStyle* passedStyle = get_obj<SkFontStyle>(L, 2);
            if (passedStyle) {
                style = *passedStyle;
            }
        }
    }

    sk_sp<SkTypeface> face(SkTypeface::MakeFromName(name, style));
//    SkDebugf("---- name <%s> style=%d, face=%p ref=%d\n", name, style, face, face->getRefCnt());
    if (nullptr == face) {
        face = SkTypeface::MakeDefault();
    }
    push_ref(L, std::move(face));
    return 1;
}

static int lsk_newFontStyle(lua_State* L) {
    int count = lua_gettop(L);
    int weight = SkFontStyle::kNormal_Weight;
    int width = SkFontStyle::kNormal_Width;
    SkFontStyle::Slant slant = SkFontStyle::kUpright_Slant;
    if (count >= 1 && lua_isnumber(L, 1)) {
        weight = lua_tointegerx(L, 1, nullptr);
    }
    if (count >= 2 && lua_isnumber(L, 2)) {
        width = lua_tointegerx(L, 2, nullptr);
    }
    if (count >= 3 && lua_isnumber(L, 3)) {
        slant = static_cast<SkFontStyle::Slant>(lua_tointegerx(L, 3, nullptr));
    }
    push_new<SkFontStyle>(L, weight, width, slant);
    return 1;
}

static int lsk_newRasterSurface(lua_State* L) {
    int width = lua2int_def(L, 1, 0);
    int height = lua2int_def(L, 2, 0);
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    auto surface = SkSurface::MakeRaster(info, &props);
    if (nullptr == surface) {
        lua_pushnil(L);
    } else {
        push_ref(L, surface);
    }
    return 1;
}

static int lsk_loadImage(lua_State* L) {
    if (lua_gettop(L) > 0 && lua_isstring(L, 1)) {
        const char* name = lua_tolstring(L, 1, nullptr);
        sk_sp<SkData> data(SkData::MakeFromFileName(name));
        if (data) {
            auto image = SkImage::MakeFromEncoded(std::move(data));
            if (image) {
                push_ref(L, std::move(image));
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
    setfield_function(L, "newBlurImageFilter", lsk_newBlurImageFilter);
    setfield_function(L, "newLinearGradient", lsk_newLinearGradient);
    setfield_function(L, "newMatrix", lsk_newMatrix);
    setfield_function(L, "newPaint", lsk_newPaint);
    setfield_function(L, "newPath", lsk_newPath);
    setfield_function(L, "newPictureRecorder", lsk_newPictureRecorder);
    setfield_function(L, "newRRect", lsk_newRRect);
    setfield_function(L, "newRasterSurface", lsk_newRasterSurface);
    setfield_function(L, "newTextBlob", lsk_newTextBlob);
    setfield_function(L, "newTypeface", lsk_newTypeface);
    setfield_function(L, "newFontStyle", lsk_newFontStyle);
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
    REG_CLASS(L, SkColorFilter);
    REG_CLASS(L, DocHolder);
    REG_CLASS(L, SkFont);
    REG_CLASS(L, SkImage);
    REG_CLASS(L, SkImageFilter);
    REG_CLASS(L, SkMatrix);
    REG_CLASS(L, SkPaint);
    REG_CLASS(L, SkPath);
    REG_CLASS(L, SkPathEffect);
    REG_CLASS(L, SkPicture);
    REG_CLASS(L, SkPictureRecorder);
    REG_CLASS(L, SkRRect);
    REG_CLASS(L, SkShader);
    REG_CLASS(L, SkSurface);
    REG_CLASS(L, SkTextBlob);
    REG_CLASS(L, SkTypeface);
    REG_CLASS(L, SkFontStyle);
}

extern "C" int luaopen_skia(lua_State* L);
extern "C" int luaopen_skia(lua_State* L) {
    SkLua::Load(L);
    return 0;
}
