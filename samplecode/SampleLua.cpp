/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkView.h"
#include "SkLua.h"
#include "SkCanvas.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

static const char gDrawName[] = "onDrawContent";

static const char gCode[] = ""
    "require \"math\" "
    ""
    "local r = { left = 10, top = 10, right = 100, bottom = 80 } "
    "local x = 0;"
    ""
    "local paint = Sk.newPaint();"
    "paint:setAntiAlias(true);"
    ""
    "local image = Sk.loadImage('/skia/trunk/sailboat.jpg');"
    ""
    "local color = {a = 1, r = 1, g = 0, b = 0};"
    ""
    "function rnd(range) "
    "   return math.random() * range;"
    "end "
    ""
    "rndX = function () return rnd(640) end "
    "rndY = function () return rnd(480) end "
    ""
    "function draw_rand_path(canvas);"
    "   if not path_paint then "
    "       path_paint = Sk.newPaint();"
    "       path_paint:setAntiAlias(true);"
    "   end "
    "   path_paint:setColor({a = 1, r = math.random(), g = math.random(), b = math.random() });"
    ""
    "   local path = Sk.newPath();"
    "   path:moveTo(rndX(), rndY());"
    "   for i = 0, 50 do "
    "       path:quadTo(rndX(), rndY(), rndX(), rndY());"
    "   end "
    "   canvas:drawPath(path, path_paint);"
    ""
    "   paint:setColor{a=1,r=0,g=0,b=1};"
    "   local align = { 'left', 'center', 'right' };"
    "   paint:setTextSize(30);"
    "   for k, v in next, align do "
    "       paint:setTextAlign(v);"
    "       canvas:drawText('Hamburgefons', 320, 200 + 30*k, paint);"
    "   end "
    "end "
    ""
    "function onStartup() "
    "   local paint = Sk.newPaint();"
    "   paint:setColor{a=1, r=1, g=0, b=0};"
    "   local doc = Sk.newDocumentPDF('/skia/trunk/test.pdf');"
    "   local canvas = doc:beginPage(72*8.5, 72*11);"
    "   canvas:drawText('Hello Lua', 300, 300, paint);"
    "   doc:close();"
    "   doc = nil;"
    "end "
    ""
    "function onDrawContent(canvas) "
    "   draw_rand_path(canvas);"
    "   color.g = x / 100;"
    "   paint:setColor(color) "
    "   canvas:translate(x, 0);"
    "   canvas:drawOval(r, paint) "
    "   x = x + 1;"
    "   canvas:drawImage(image, x, r.bottom + 50, 0.5);"
    "   if x > 100 then x = 0 end;"
    "end "
    ""
    "onStartup();";

class LuaView : public SampleView {
public:
    LuaView() : fLua(NULL) {}

    virtual ~LuaView() {
        SkDELETE(fLua);
    }

    lua_State* ensureLua() {
        if (NULL == fLua) {
            fLua = SkNEW(SkLua);
            fLua->runCode(gCode);
        }
        return fLua->get();
    }

protected:
    virtual bool onQuery(SkEvent* evt) SK_OVERRIDE {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Lua");
            return true;
        }
        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) SK_OVERRIDE {
        lua_State* L = this->ensureLua();

        lua_getglobal(L, gDrawName);
        if (!lua_isfunction(L, -1)) {
            int t = lua_type(L, -1);
            SkDebugf("--- expected %s function %d, ignoring.\n", gDrawName, t);
            lua_pop(L, 1);
        } else {
            // does it make sense to try to "cache" the lua version of this
            // canvas between draws?
            fLua->pushCanvas(canvas);
            if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
                SkDebugf("lua err: %s\n", lua_tostring(L, -1));
            }
        }
        // need a way for the lua-sample to tell us if they want animations...
        // hard-code it ON for now.
        this->inval(NULL);
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y,
                                              unsigned modi) SK_OVERRIDE {
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    virtual bool onClick(Click* click) SK_OVERRIDE {
        return this->INHERITED::onClick(click);
    }

private:
    SkLua* fLua;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new LuaView; }
static SkViewRegister reg(MyFactory);
