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

    SkLua(const char termCode[] = NULL);    // creates a new L, will close it
    SkLua(lua_State*);                      // uses L, will not close it
    ~SkLua();

    lua_State* get() const { return fL; }
    lua_State* operator*() const { return fL; }
    lua_State* operator->() const { return fL; }

    bool runCode(const char code[]);
    bool runCode(const void* code, size_t size);

    void pushBool(bool, const char tableKey[] = NULL);
    void pushString(const char[], const char tableKey[] = NULL);
    void pushString(const char[], size_t len, const char tableKey[] = NULL);
    void pushString(const SkString&, const char tableKey[] = NULL);
    void pushArrayU16(const uint16_t[], int count, const char tableKey[] = NULL);
    void pushArrayPoint(const SkPoint[], int count, const char key[] = NULL);
    void pushArrayScalar(const SkScalar[], int count, const char key[] = NULL);
    void pushColor(SkColor, const char tableKey[] = NULL);
    void pushU32(uint32_t, const char tableKey[] = NULL);
    void pushScalar(SkScalar, const char tableKey[] = NULL);
    void pushRect(const SkRect&, const char tableKey[] = NULL);
    void pushRRect(const SkRRect&, const char tableKey[] = NULL);
    void pushDash(const SkPathEffect::DashInfo&, const char tableKey[] = NULL);
    void pushMatrix(const SkMatrix&, const char tableKey[] = NULL);
    void pushPaint(const SkPaint&, const char tableKey[] = NULL);
    void pushPath(const SkPath&, const char tableKey[] = NULL);
    void pushCanvas(SkCanvas*, const char tableKey[] = NULL);
    void pushTextBlob(const SkTextBlob*, const char tableKey[] = NULL);

private:
    lua_State*  fL;
    SkString    fTermCode;
    bool        fWeOwnL;
};

#endif
