/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLua_DEFINED
#define SkLua_DEFINED

#include "SkColor.h"
#include "SkPathEffect.h"
#include "SkScalar.h"
#include "SkString.h"

struct lua_State;

class SkCanvas;
class SkMatrix;
class SkPaint;
class SkPath;
struct SkRect;
class SkRRect;
class SkTextBlob;

#define SkScalarToLua(x)    SkScalarToDouble(x)
#define SkLuaToScalar(x)    SkDoubleToScalar(x)

class SkLua {
public:
    static void Load(lua_State*);

    SkLua(const char termCode[] = nullptr); // creates a new L, will close it
    SkLua(lua_State*);                      // uses L, will not close it
    ~SkLua();

    lua_State* get() const { return fL; }
    lua_State* operator*() const { return fL; }
    lua_State* operator->() const { return fL; }

    bool runCode(const char code[]);
    bool runCode(const void* code, size_t size);

    void pushBool(bool, const char tableKey[] = nullptr);
    void pushString(const char[], const char tableKey[] = nullptr);
    void pushString(const char[], size_t len, const char tableKey[] = nullptr);
    void pushString(const SkString&, const char tableKey[] = nullptr);
    void pushArrayU16(const uint16_t[], int count, const char tableKey[] = nullptr);
    void pushArrayPoint(const SkPoint[], int count, const char key[] = nullptr);
    void pushArrayScalar(const SkScalar[], int count, const char key[] = nullptr);
    void pushColor(SkColor, const char tableKey[] = nullptr);
    void pushU32(uint32_t, const char tableKey[] = nullptr);
    void pushScalar(SkScalar, const char tableKey[] = nullptr);
    void pushRect(const SkRect&, const char tableKey[] = nullptr);
    void pushRRect(const SkRRect&, const char tableKey[] = nullptr);
    void pushDash(const SkPathEffect::DashInfo&, const char tableKey[] = nullptr);
    void pushMatrix(const SkMatrix&, const char tableKey[] = nullptr);
    void pushPaint(const SkPaint&, const char tableKey[] = nullptr);
    void pushPath(const SkPath&, const char tableKey[] = nullptr);
    void pushCanvas(SkCanvas*, const char tableKey[] = nullptr);
    void pushTextBlob(const SkTextBlob*, const char tableKey[] = nullptr);

private:
    lua_State*  fL;
    SkString    fTermCode;
    bool        fWeOwnL;
};

#endif
