/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLua.h"

#if SK_SUPPORT_GPU
#include "GrReducedClip.h"
#endif

#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkData.h"
#include "SkDocument.h"
#include "SkGradientShader.h"
#include "SkImage.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPictureRecorder.h"
#include "SkPixelRef.h"
#include "SkRRect.h"
#include "SkString.h"
#include "SkSurface.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

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
DEF_MTNAME(SkColorFilter)
DEF_MTNAME(SkDocument)
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
DEF_MTNAME(SkXfermode)

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
    push_ref(fL, canvas);
    CHECK_SETFIELD(key);
}

void SkLua::pushTextBlob(const SkTextBlob* blob, const char key[]) {
    push_ref(fL, const_cast<SkTextBlob*>(blob));
    CHECK_SETFIELD(key);
}

static const char* element_type(SkClipStack::Element::Type type) {
    switch (type) {
        case SkClipStack::Element::kEmpty_Type:
            return "empty";
        case SkClipStack::Element::kRect_Type:
            return "rect";
        case SkClipStack::Element::kRRect_Type:
            return "rrect";
        case SkClipStack::Element::kPath_Type:
            return "path";
    }
    return "unknown";
}

static const char* region_op(SkRegion::Op op) {
    switch (op) {
        case SkRegion::kDifference_Op:
            return "difference";
        case SkRegion::kIntersect_Op:
            return "intersect";
        case SkRegion::kUnion_Op:
            return "union";
        case SkRegion::kXOR_Op:
            return "xor";
        case SkRegion::kReverseDifference_Op:
            return "reverse-difference";
        case SkRegion::kReplace_Op:
            return "replace";
    }
    return "unknown";
}

void SkLua::pushClipStack(const SkClipStack& stack, const char* key) {
    lua_newtable(fL);
    SkClipStack::B2TIter iter(stack);
    const SkClipStack::Element* element;
    int i = 0;
    while ((element = iter.next())) {
        this->pushClipStackElement(*element);
        lua_rawseti(fL, -2, ++i);
    }
    CHECK_SETFIELD(key);
}

void SkLua::pushClipStackElement(const SkClipStack::Element& element, const char* key) {
    lua_newtable(fL);
    SkClipStack::Element::Type type = element.getType();
    this->pushString(element_type(type), "type");
    switch (type) {
        case SkClipStack::Element::kEmpty_Type:
            break;
        case SkClipStack::Element::kRect_Type:
            this->pushRect(element.getRect(), "rect");
            break;
        case SkClipStack::Element::kRRect_Type:
            this->pushRRect(element.getRRect(), "rrect");
            break;
        case SkClipStack::Element::kPath_Type:
            this->pushPath(element.getPath(), "path");
            break;
    }
    this->pushString(region_op(element.getOp()), "op");
    this->pushBool(element.isAA(), "aa");
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

    get_ref<SkCanvas>(L, 1)->drawPatch(cubics, colors, texs, nullptr, *get_obj<SkPaint>(L, 5));
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

    if (lua_isstring(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4)) {
        size_t len;
        const char* text = lua_tolstring(L, 2, &len);
        get_ref<SkCanvas>(L, 1)->drawText(text, len,
                                          lua2scalar(L, 3), lua2scalar(L, 4),
                                          *get_obj<SkPaint>(L, 5));
    }
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

static int lcanvas_getClipStack(lua_State* L) {
    SkLua(L).pushClipStack(*get_ref<SkCanvas>(L, 1)->getClipStack());
    return 1;
}

int SkLua::lcanvas_getReducedClipStack(lua_State* L) {
#if SK_SUPPORT_GPU
    const SkCanvas* canvas = get_ref<SkCanvas>(L, 1);
    SkIRect queryBounds = canvas->getTopLayerBounds();

    GrReducedClip::ElementList elements;
    GrReducedClip::InitialState initialState;
    int32_t genID;
    SkIRect resultBounds;

    const SkClipStack& stack = *canvas->getClipStack();

    GrReducedClip::ReduceClipStack(stack,
                                   queryBounds,
                                   &elements,
                                   &genID,
                                   &initialState,
                                   &resultBounds,
                                   nullptr);

    GrReducedClip::ElementList::Iter iter(elements);
    int i = 0;
    lua_newtable(L);
    while(iter.get()) {
        SkLua(L).pushClipStackElement(*iter.get());
        iter.next();
        lua_rawseti(L, -2, ++i);
    }
    // Currently this only returns the element list to lua, not the initial state or result bounds.
    // It could return these as additional items on the lua stack.
    return 1;
#else
    return 0;
#endif
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
    get_ref<SkCanvas>(L, 1)->unref();
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
    { "getClipStack", lcanvas_getClipStack },
#if SK_SUPPORT_GPU
    { "getReducedClipStack", SkLua::lcanvas_getReducedClipStack },
#endif
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

static int lpaint_isUnderlineText(lua_State* L) {
    lua_pushboolean(L, get_obj<SkPaint>(L, 1)->isUnderlineText());
    return 1;
}

static int lpaint_isStrikeThruText(lua_State* L) {
    lua_pushboolean(L, get_obj<SkPaint>(L, 1)->isStrikeThruText());
    return 1;
}

static int lpaint_isFakeBoldText(lua_State* L) {
    lua_pushboolean(L, get_obj<SkPaint>(L, 1)->isFakeBoldText());
    return 1;
}

static int lpaint_isLinearText(lua_State* L) {
    lua_pushboolean(L, get_obj<SkPaint>(L, 1)->isLinearText());
    return 1;
}

static int lpaint_isSubpixelText(lua_State* L) {
    lua_pushboolean(L, get_obj<SkPaint>(L, 1)->isSubpixelText());
    return 1;
}

static int lpaint_setSubpixelText(lua_State* L) {
    get_obj<SkPaint>(L, 1)->setSubpixelText(lua2bool(L, 2));
    return 1;
}

static int lpaint_isDevKernText(lua_State* L) {
    lua_pushboolean(L, get_obj<SkPaint>(L, 1)->isDevKernText());
    return 1;
}

static int lpaint_isLCDRenderText(lua_State* L) {
    lua_pushboolean(L, get_obj<SkPaint>(L, 1)->isLCDRenderText());
    return 1;
}

static int lpaint_setLCDRenderText(lua_State* L) {
    get_obj<SkPaint>(L, 1)->setLCDRenderText(lua2bool(L, 2));
    return 1;
}

static int lpaint_isEmbeddedBitmapText(lua_State* L) {
    lua_pushboolean(L, get_obj<SkPaint>(L, 1)->isEmbeddedBitmapText());
    return 1;
}

static int lpaint_isAutohinted(lua_State* L) {
    lua_pushboolean(L, get_obj<SkPaint>(L, 1)->isAutohinted());
    return 1;
}

static int lpaint_isVerticalText(lua_State* L) {
    lua_pushboolean(L, get_obj<SkPaint>(L, 1)->isVerticalText());
    return 1;
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

static int lpaint_getTextSize(lua_State* L) {
    SkLua(L).pushScalar(get_obj<SkPaint>(L, 1)->getTextSize());
    return 1;
}

static int lpaint_getTextScaleX(lua_State* L) {
    SkLua(L).pushScalar(get_obj<SkPaint>(L, 1)->getTextScaleX());
    return 1;
}

static int lpaint_getTextSkewX(lua_State* L) {
    SkLua(L).pushScalar(get_obj<SkPaint>(L, 1)->getTextSkewX());
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
    get_obj<SkPaint>(L, 1)->setTypeface(sk_ref_sp(get_ref<SkTypeface>(L, 2)));
    return 0;
}

static int lpaint_getHinting(lua_State* L) {
    SkLua(L).pushU32(get_obj<SkPaint>(L, 1)->getHinting());
    return 1;
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

static int lpaint_getStrokeCap(lua_State* L) {
    SkLua(L).pushU32(get_obj<SkPaint>(L, 1)->getStrokeCap());
    return 1;
}

static int lpaint_getStrokeJoin(lua_State* L) {
    SkLua(L).pushU32(get_obj<SkPaint>(L, 1)->getStrokeJoin());
    return 1;
}

static int lpaint_getTextEncoding(lua_State* L) {
    SkLua(L).pushU32(get_obj<SkPaint>(L, 1)->getTextEncoding());
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
    setfield_bool_if(L, "looper",      !!paint->getLooper());
    setfield_bool_if(L, "pathEffect",  !!paint->getPathEffect());
    setfield_bool_if(L, "rasterizer",  !!paint->getRasterizer());
    setfield_bool_if(L, "maskFilter",  !!paint->getMaskFilter());
    setfield_bool_if(L, "shader",      !!paint->getShader());
    setfield_bool_if(L, "colorFilter", !!paint->getColorFilter());
    setfield_bool_if(L, "imageFilter", !!paint->getImageFilter());
    setfield_bool_if(L, "xfermode",    !!paint->getXfermode());
    return 1;
}

static int lpaint_getXfermode(lua_State* L) {
    const SkPaint* paint = get_obj<SkPaint>(L, 1);
    SkXfermode* xfermode = paint->getXfermode();
    if (xfermode) {
        push_ref(L, xfermode);
        return 1;
    }
    return 0;
}

static int lpaint_setXfermode(lua_State* L) {
    SkPaint* paint = get_obj<SkPaint>(L, 1);
    paint->setXfermode(sk_ref_sp(get_ref<SkXfermode>(L, 2)));
    return 0;
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
    paint->setImageFilter(get_ref<SkImageFilter>(L, 2));
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
    { "isUnderlineText", lpaint_isUnderlineText },
    { "isStrikeThruText", lpaint_isStrikeThruText },
    { "isFakeBoldText", lpaint_isFakeBoldText },
    { "isLinearText", lpaint_isLinearText },
    { "isSubpixelText", lpaint_isSubpixelText },
    { "setSubpixelText", lpaint_setSubpixelText },
    { "isDevKernText", lpaint_isDevKernText },
    { "isLCDRenderText", lpaint_isLCDRenderText },
    { "setLCDRenderText", lpaint_setLCDRenderText },
    { "isEmbeddedBitmapText", lpaint_isEmbeddedBitmapText },
    { "isAutohinted", lpaint_isAutohinted },
    { "isVerticalText", lpaint_isVerticalText },
    { "getAlpha", lpaint_getAlpha },
    { "setAlpha", lpaint_setAlpha },
    { "getColor", lpaint_getColor },
    { "setColor", lpaint_setColor },
    { "getTextSize", lpaint_getTextSize },
    { "setTextSize", lpaint_setTextSize },
    { "getTextScaleX", lpaint_getTextScaleX },
    { "getTextSkewX", lpaint_getTextSkewX },
    { "getTypeface", lpaint_getTypeface },
    { "setTypeface", lpaint_setTypeface },
    { "getHinting", lpaint_getHinting },
    { "getFontID", lpaint_getFontID },
    { "getTextAlign", lpaint_getTextAlign },
    { "setTextAlign", lpaint_setTextAlign },
    { "getStroke", lpaint_getStroke },
    { "setStroke", lpaint_setStroke },
    { "getStrokeCap", lpaint_getStrokeCap },
    { "getStrokeJoin", lpaint_getStrokeJoin },
    { "getTextEncoding", lpaint_getTextEncoding },
    { "getStrokeWidth", lpaint_getStrokeWidth },
    { "setStrokeWidth", lpaint_setStrokeWidth },
    { "getStrokeMiter", lpaint_getStrokeMiter },
    { "measureText", lpaint_measureText },
    { "getFontMetrics", lpaint_getFontMetrics },
    { "getEffects", lpaint_getEffects },
    { "getColorFilter", lpaint_getColorFilter },
    { "setColorFilter", lpaint_setColorFilter },
    { "getImageFilter", lpaint_getImageFilter },
    { "setImageFilter", lpaint_setImageFilter },
    { "getXfermode", lpaint_getXfermode },
    { "setXfermode", lpaint_setXfermode },
    { "getShader", lpaint_getShader },
    { "setShader", lpaint_setShader },
    { "getPathEffect", lpaint_getPathEffect },
    { "__gc", lpaint_gc },
    { nullptr, nullptr }
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

static int lshader_isABitmap(lua_State* L) {
    SkShader* shader = get_ref<SkShader>(L, 1);
    if (shader) {
        SkBitmap bm;
        SkMatrix matrix;
        SkShader::TileMode modes[2];
        if (shader->isABitmap(&bm, &matrix, modes)) {
            lua_newtable(L);
            setfield_number(L, "genID", bm.pixelRef() ? bm.pixelRef()->getGenerationID() : 0);
            setfield_number(L, "width", bm.width());
            setfield_number(L, "height", bm.height());
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

            bool containsHardStops = false;
            bool isEvenlySpaced    = true;
            for (int i = 1; i < info.fColorCount; i++) {
                if (SkScalarNearlyEqual(info.fColorOffsets[i], info.fColorOffsets[i-1])) {
                    containsHardStops = true;
                }
                if (!SkScalarNearlyEqual(info.fColorOffsets[i], i/(info.fColorCount - 1.0f))) {
                    isEvenlySpaced = false;
                }
            }

            lua_newtable(L);
            setfield_string(L,  "type",               gradtype2string(t));
            setfield_string(L,  "tile",               mode2string(info.fTileMode));
            setfield_number(L,  "colorCount",         info.fColorCount);
            setfield_boolean(L, "containsHardStops",  containsHardStops);
            setfield_boolean(L, "isEvenlySpaced",     isEvenlySpaced);

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
    { "isABitmap",      lshader_isABitmap },
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

static int lpxfermode_getTypeName(lua_State* L) {
    lua_pushstring(L, get_ref<SkXfermode>(L, 1)->getTypeName());
    return 1;
}

static int lpxfermode_gc(lua_State* L) {
    get_ref<SkXfermode>(L, 1)->unref();
    return 0;
}

static const struct luaL_Reg gSkXfermode_Methods[] = {
    { "getTypeName",    lpxfermode_getTypeName },
    { "__gc",           lpxfermode_gc },
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
    SkShader::TileMode tmode = SkShader::kClamp_TileMode;
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
        push_ref(L, canvas);
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

    push_ref(L, canvas);
    return 1;
}

static int lpicturerecorder_getCanvas(lua_State* L) {
    SkCanvas* canvas = get_obj<SkPictureRecorder>(L, 1)->getRecordingCanvas();
    if (nullptr == canvas) {
        lua_pushnil(L);
        return 1;
    }
    push_ref(L, canvas);
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
    lua_pushnumber(L, (double)get_ref<SkTypeface>(L, 1)->style());
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
    const char* file = nullptr;
    if (lua_gettop(L) > 0 && lua_isstring(L, 1)) {
        file = lua_tolstring(L, 1, nullptr);
    }

    sk_sp<SkDocument> doc = SkDocument::MakePDF(file);
    if (nullptr == doc) {
        // do I need to push a nil on the stack and return 1?
        return 0;
    } else {
        push_ref(L, std::move(doc));
        return 1;
    }
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
    sk_sp<SkShader> s(SkGradientShader::MakeLinear(pts, colors, nullptr, 2,
                                                   SkShader::kClamp_TileMode));
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

#include "SkTextBox.h"
// Sk.newTextBlob(text, rect, paint)
static int lsk_newTextBlob(lua_State* L) {
    const char* text = lua_tolstring(L, 1, nullptr);
    SkRect bounds;
    lua2rect(L, 2, &bounds);
    const SkPaint& paint = *get_obj<SkPaint>(L, 3);

    SkTextBox box;
    box.setMode(SkTextBox::kLineBreak_Mode);
    box.setBox(bounds);
    box.setText(text, strlen(text), paint);

    SkScalar newBottom;
    SkAutoTUnref<SkTextBlob> blob(box.snapshotTextBlob(&newBottom));
    push_ref<SkTextBlob>(L, blob);
    SkLua(L).pushScalar(newBottom);
    return 2;
}

static int lsk_newTypeface(lua_State* L) {
    const char* name = nullptr;
    int style = SkTypeface::kNormal;

    int count = lua_gettop(L);
    if (count > 0 && lua_isstring(L, 1)) {
        name = lua_tolstring(L, 1, nullptr);
        if (count > 1 && lua_isnumber(L, 2)) {
            style = lua_tointegerx(L, 2, nullptr) & SkTypeface::kBoldItalic;
        }
    }

    sk_sp<SkTypeface> face(SkTypeface::MakeFromName(name, SkFontStyle::FromOldStyle(style)));
//    SkDebugf("---- name <%s> style=%d, face=%p ref=%d\n", name, style, face, face->getRefCnt());
    if (nullptr == face) {
        face = SkTypeface::MakeDefault();
    }
    push_ref(L, std::move(face));
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
    REG_CLASS(L, SkDocument);
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
    REG_CLASS(L, SkXfermode);
}

extern "C" int luaopen_skia(lua_State* L);
extern "C" int luaopen_skia(lua_State* L) {
    SkLua::Load(L);
    return 0;
}
