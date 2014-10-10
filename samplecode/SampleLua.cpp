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
    "local r = { left = 10, top = 10, right = 100, bottom = 80 } \n"
    "local x = 0;\n"
    "\n"
    "local paint = Sk.newPaint();\n"
    "paint:setAntiAlias(true);\n"
    "\n"
    "local image = Sk.loadImage('/skia/sailboat.jpg');\n"
    "\n"
    "local color = {a = 1, r = 1, g = 0, b = 0};\n"
    "\n"
    "function rnd(range) \n"
    "   return math.random() * range;\n"
    "end \n"
    "\n"
    "rndX = function () return rnd(640) end \n"
    "rndY = function () return rnd(480) end \n"
    "\n"
    "function draw_rand_path(canvas);\n"
    "   if not path_paint then \n"
    "       path_paint = Sk.newPaint();\n"
    "       path_paint:setAntiAlias(true);\n"
    "   end \n"
    "   path_paint:setColor({a = 1, r = math.random(), g = math.random(), b = math.random() });\n"
    "\n"
    "   local path = Sk.newPath();\n"
    "   path:moveTo(rndX(), rndY());\n"
    "   for i = 0, 50 do \n"
    "       path:quadTo(rndX(), rndY(), rndX(), rndY());\n"
    "   end \n"
    "   canvas:drawPath(path, path_paint);\n"
    "\n"
    "   paint:setColor{a=1,r=0,g=0,b=1};\n"
    "   local align = { 'left', 'center', 'right' };\n"
    "   paint:setTextSize(30);\n"
    "   for k, v in next, align do \n"
    "       paint:setTextAlign(v);\n"
    "       canvas:drawText('Hamburgefons', 320, 200 + 30*k, paint);\n"
    "   end \n"
    "end \n"
    "\n"
    "function onStartup() \n"
    "   local paint = Sk.newPaint();\n"
    "   paint:setColor{a=1, r=1, g=0, b=0};\n"
    "   local doc = Sk.newDocumentPDF('/skia/trunk/test.pdf');\n"
    "   local canvas = doc:beginPage(72*8.5, 72*11);\n"
    "   canvas:drawText('Hello Lua', 300, 300, paint);\n"
    "   doc:close();\n"
    "   doc = nil;\n"
    "end \n"
    "\n"
    "function onDrawContent(canvas) \n"
    "   draw_rand_path(canvas);\n"
    "   color.g = x / 100;\n"
    "   paint:setColor(color) \n"
    "   canvas:translate(x, 0);\n"
    "   canvas:drawOval(r, paint) \n"
    "   x = x + 1;\n"
    "   local r2 = {}\n"
    "   r2.left = x;\n"
    "   r2.top = r.bottom + 50;\n"
    "   r2.right = r2.left + image:width() * 0.1;\n"
    "   r2.bottom = r2.top + image:height() * 0.1;\n"
    "   canvas:drawImageRect(image, nil, r2, 0.75);\n"
    "   if x > 100 then x = 0 end;\n"
    "end \n"
    "\n"
    "onStartup();\n";

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
