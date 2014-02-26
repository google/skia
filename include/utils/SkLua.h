/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLua_DEFINED
#define SkLua_DEFINED

#include "SkClipStack.h"
#include "SkColor.h"
#include "SkScalar.h"
#include "SkString.h"

struct lua_State;

class SkCanvas;
class SkMatrix;
class SkPaint;
class SkPath;
struct SkRect;
class SkRRect;

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
    void pushColor(SkColor, const char tableKey[] = NULL);
    void pushU32(uint32_t, const char tableKey[] = NULL);
    void pushScalar(SkScalar, const char tableKey[] = NULL);
    void pushRect(const SkRect&, const char tableKey[] = NULL);
    void pushRRect(const SkRRect&, const char tableKey[] = NULL);
    void pushMatrix(const SkMatrix&, const char tableKey[] = NULL);
    void pushPaint(const SkPaint&, const char tableKey[] = NULL);
    void pushPath(const SkPath&, const char tableKey[] = NULL);
    void pushCanvas(SkCanvas*, const char tableKey[] = NULL);
    void pushClipStack(const SkClipStack&, const char tableKey[] = NULL);
    void pushClipStackElement(const SkClipStack::Element& element, const char tableKey[] = NULL);

    // This SkCanvas lua methods is declared here to benefit from SkLua's friendship with SkCanvas.
    static int lcanvas_getReducedClipStack(lua_State* L);

private:
    lua_State*  fL;
    SkString    fTermCode;
    bool        fWeOwnL;
};

#endif
