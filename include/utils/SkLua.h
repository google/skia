/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLua_DEFINED
#define SkLua_DEFINED

#include "SkColor.h"
#include "SkScalar.h"

struct lua_State;

class SkCanvas;
class SkMatrix;
class SkPaint;
class SkPath;
struct SkRect;
class SkRRect;
class SkString;

#define SkScalarToLua(x)    SkScalarToDouble(x)
#define SkLuaToScalar(x)    SkDoubleToScalar(x)

class SkLua {
public:
    static void Load(lua_State*);

    SkLua(lua_State*);
    ~SkLua();

    lua_State* getL() const { return fL; }

    void pushBool(bool, const char tableKey[] = NULL);
    void pushString(const char[], const char tableKey[] = NULL);
    void pushString(const SkString&, const char tableKey[] = NULL);
    void pushColor(SkColor, const char tableKey[] = NULL);
    void pushScalar(SkScalar, const char tableKey[] = NULL);
    void pushRect(const SkRect&, const char tableKey[] = NULL);
    void pushRRect(const SkRRect&, const char tableKey[] = NULL);
    void pushMatrix(const SkMatrix&, const char tableKey[] = NULL);
    void pushPaint(const SkPaint&, const char tableKey[] = NULL);
    void pushPath(const SkPath&, const char tableKey[] = NULL);
    void pushCanvas(SkCanvas*, const char tableKey[] = NULL);

private:
    lua_State* fL;
};

#endif
