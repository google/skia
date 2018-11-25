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
#include "Resources.h"
#include "SkData.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

//#define LUA_FILENAME    "lua/test.lua"
#define LUA_FILENAME    "lua/slides.lua"

static const char gDrawName[] = "onDrawContent";
static const char gClickName[] = "onClickHandler";
static const char gUnicharName[] = "onCharHandler";

static const char gMissingCode[] = ""
    "local paint = Sk.newPaint()"
    "paint:setAntiAlias(true)"
    "paint:setTextSize(30)"
    ""
    "function onDrawContent(canvas)"
    "   canvas:drawText('missing \"test.lua\"', 20, 50, paint)"
    "end"
    ;

class LuaView : public SampleView {
public:
    LuaView() : fLua(nullptr) {}

    ~LuaView() override { delete fLua; }

    void setImageFilename(lua_State* L) {
        SkString str = GetResourcePath("images/mandrill_256.png");

        lua_getglobal(L, "setImageFilename");
        if (lua_isfunction(L, -1)) {
            fLua->pushString(str.c_str());
            if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
                SkDebugf("lua err: %s\n", lua_tostring(L, -1));
            }
        }
    }

    lua_State* ensureLua() {
        if (nullptr == fLua) {
            fLua = new SkLua;

            SkString str = GetResourcePath(LUA_FILENAME);
            sk_sp<SkData> data(SkData::MakeFromFileName(str.c_str()));
            if (data) {
                fLua->runCode(data->data(), data->size());
                this->setImageFilename(fLua->get());
            } else {
                fLua->runCode(gMissingCode);
            }
        }
        return fLua->get();
    }

protected:
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Lua");
            return true;
        }
        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            lua_State* L = this->ensureLua();
            lua_getglobal(L, gUnicharName);
            if (lua_isfunction(L, -1)) {
                SkString str;
                str.appendUnichar(uni);
                fLua->pushString(str.c_str());
                if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
                    SkDebugf("lua err: %s\n", lua_tostring(L, -1));
                } else {
                    if (lua_isboolean(L, -1) && lua_toboolean(L, -1)) {
                        return true;
                    }
                }
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
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
            fLua->pushScalar(this->width());
            fLua->pushScalar(this->height());
            if (lua_pcall(L, 3, 1, 0) != LUA_OK) {
                SkDebugf("lua err: %s\n", lua_tostring(L, -1));
            }
        }
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y,
                                              unsigned modi) override {
        lua_State* L = this->ensureLua();
        lua_getglobal(L, gClickName);
        if (lua_isfunction(L, -1)) {
            fLua->pushScalar(x);
            fLua->pushScalar(y);
            fLua->pushString("down");
            if (lua_pcall(L, 3, 1, 0) != LUA_OK) {
                SkDebugf("lua err: %s\n", lua_tostring(L, -1));
            } else {
                if (lua_isboolean(L, -1) && lua_toboolean(L, -1)) {
                    return new Click(this);
                }
            }
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    bool onClick(Click* click) override {
        const char* state = nullptr;
        switch (click->fState) {
            case Click::kMoved_State:
                state = "moved";
                break;
            case Click::kUp_State:
                state = "up";
                break;
            default:
                break;
        }
        if (state) {
            lua_State* L = fLua->get();
            lua_getglobal(L, gClickName);
            fLua->pushScalar(click->fCurr.x());
            fLua->pushScalar(click->fCurr.y());
            fLua->pushString(state);
            lua_pcall(L, 3, 1, 0);
            return lua_isboolean(L, -1) && lua_toboolean(L, -1);
        }
        return true;
    }

private:
    SkLua* fLua;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new LuaView; }
static SkViewRegister reg(MyFactory);
