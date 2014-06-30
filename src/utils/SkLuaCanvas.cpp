/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLuaCanvas.h"
#include "SkLua.h"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
}

class AutoCallLua : public SkLua {
public:
    AutoCallLua(lua_State* L, const char func[], const char verb[]) : INHERITED(L) {
        lua_getglobal(L, func);
        if (!lua_isfunction(L, -1)) {
            int t = lua_type(L, -1);
            SkDebugf("--- expected function %d\n", t);
        }

        lua_newtable(L);
        this->pushString(verb, "verb");
    }

    ~AutoCallLua() {
        lua_State* L = this->get();
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            SkDebugf("lua err: %s\n", lua_tostring(L, -1));
        }
        lua_settop(L, -1);
    }

    void pushEncodedText(SkPaint::TextEncoding, const void*, size_t);

private:
    typedef SkLua INHERITED;
};

#define AUTO_LUA(verb)  AutoCallLua lua(fL, fFunc.c_str(), verb)


///////////////////////////////////////////////////////////////////////////////

void AutoCallLua::pushEncodedText(SkPaint::TextEncoding enc, const void* text,
                                  size_t length) {
    switch (enc) {
        case SkPaint::kUTF8_TextEncoding:
            this->pushString((const char*)text, length, "text");
            break;
        case SkPaint::kUTF16_TextEncoding: {
            SkString str;
            str.setUTF16((const uint16_t*)text, length);
            this->pushString(str, "text");
        } break;
        case SkPaint::kGlyphID_TextEncoding:
            this->pushArrayU16((const uint16_t*)text, SkToInt(length >> 1),
                               "glyphs");
            break;
        case SkPaint::kUTF32_TextEncoding:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkLuaCanvas::pushThis() {
    SkLua(fL).pushCanvas(this);
}

///////////////////////////////////////////////////////////////////////////////

SkLuaCanvas::SkLuaCanvas(int width, int height, lua_State* L, const char func[])
    : INHERITED(width, height)
    , fL(L)
    , fFunc(func) {
}

SkLuaCanvas::~SkLuaCanvas() {}

void SkLuaCanvas::willSave() {
    AUTO_LUA("save");
    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy SkLuaCanvas::willSaveLayer(const SkRect* bounds, const SkPaint* paint,
                                                       SaveFlags flags) {
    AUTO_LUA("saveLayer");
    if (bounds) {
        lua.pushRect(*bounds, "bounds");
    }
    if (paint) {
        lua.pushPaint(*paint, "paint");
    }

    this->INHERITED::willSaveLayer(bounds, paint, flags);
    // No need for a layer.
    return kNoLayer_SaveLayerStrategy;
}

void SkLuaCanvas::willRestore() {
    AUTO_LUA("restore");
    this->INHERITED::willRestore();
}

void SkLuaCanvas::didConcat(const SkMatrix& matrix) {
    switch (matrix.getType()) {
        case SkMatrix::kTranslate_Mask: {
            AUTO_LUA("translate");
            lua.pushScalar(matrix.getTranslateX(), "dx");
            lua.pushScalar(matrix.getTranslateY(), "dy");
            break;
        }
        case SkMatrix::kScale_Mask: {
            AUTO_LUA("scale");
            lua.pushScalar(matrix.getScaleX(), "sx");
            lua.pushScalar(matrix.getScaleY(), "sy");
            break;
        }
        default: {
            AUTO_LUA("concat");
            // pushMatrix added in https://codereview.chromium.org/203203004/
            // Doesn't seem to have ever been working correctly since added
            // lua.pushMatrix(matrix);
            break;
        }
    }

    this->INHERITED::didConcat(matrix);
}

void SkLuaCanvas::didSetMatrix(const SkMatrix& matrix) {
    this->INHERITED::didSetMatrix(matrix);
}

void SkLuaCanvas::onClipRect(const SkRect& r, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    AUTO_LUA("clipRect");
    lua.pushRect(r, "rect");
    lua.pushBool(kSoft_ClipEdgeStyle == edgeStyle, "aa");
    this->INHERITED::onClipRect(r, op, edgeStyle);
}

void SkLuaCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    AUTO_LUA("clipRRect");
    lua.pushRRect(rrect, "rrect");
    lua.pushBool(kSoft_ClipEdgeStyle == edgeStyle, "aa");
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

void SkLuaCanvas::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    AUTO_LUA("clipPath");
    lua.pushPath(path, "path");
    lua.pushBool(kSoft_ClipEdgeStyle == edgeStyle, "aa");
    this->INHERITED::onClipPath(path, op, edgeStyle);
}

void SkLuaCanvas::onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    AUTO_LUA("clipRegion");
    this->INHERITED::onClipRegion(deviceRgn, op);
}

void SkLuaCanvas::drawPaint(const SkPaint& paint) {
    AUTO_LUA("drawPaint");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawPoints(PointMode mode, size_t count,
                               const SkPoint pts[], const SkPaint& paint) {
    AUTO_LUA("drawPoints");
    lua.pushArrayPoint(pts, SkToInt(count), "points");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawOval(const SkRect& rect, const SkPaint& paint) {
    AUTO_LUA("drawOval");
    lua.pushRect(rect, "rect");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    AUTO_LUA("drawRect");
    lua.pushRect(rect, "rect");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    AUTO_LUA("drawRRect");
    lua.pushRRect(rrect, "rrect");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                               const SkPaint& paint) {
    AUTO_LUA("drawDRRect");
    lua.pushRRect(outer, "outer");
    lua.pushRRect(inner, "inner");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    AUTO_LUA("drawPath");
    lua.pushPath(path, "path");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                             const SkPaint* paint) {
    AUTO_LUA("drawBitmap");
    if (paint) {
        lua.pushPaint(*paint, "paint");
    }
}

void SkLuaCanvas::drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                       const SkRect& dst, const SkPaint* paint,
                                       DrawBitmapRectFlags flags) {
    AUTO_LUA("drawBitmapRectToRect");
    if (paint) {
        lua.pushPaint(*paint, "paint");
    }
}

void SkLuaCanvas::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                   const SkPaint* paint) {
    AUTO_LUA("drawBitmapMatrix");
    if (paint) {
        lua.pushPaint(*paint, "paint");
    }
}

void SkLuaCanvas::drawSprite(const SkBitmap& bitmap, int x, int y,
                               const SkPaint* paint) {
    AUTO_LUA("drawSprite");
    if (paint) {
        lua.pushPaint(*paint, "paint");
    }
}

void SkLuaCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                             const SkPaint& paint) {
    AUTO_LUA("drawText");
    lua.pushEncodedText(paint.getTextEncoding(), text, byteLength);
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                                const SkPaint& paint) {
    AUTO_LUA("drawPosText");
    lua.pushEncodedText(paint.getTextEncoding(), text, byteLength);
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                 SkScalar constY, const SkPaint& paint) {
    AUTO_LUA("drawPosTextH");
    lua.pushEncodedText(paint.getTextEncoding(), text, byteLength);
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                   const SkMatrix* matrix, const SkPaint& paint) {
    AUTO_LUA("drawTextOnPath");
    lua.pushPath(path, "path");
    lua.pushEncodedText(paint.getTextEncoding(), text, byteLength);
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawPicture(const SkPicture* picture) {
    AUTO_LUA("drawPicture");
    // call through so we can see the nested picture ops
    this->INHERITED::onDrawPicture(picture);
}

void SkLuaCanvas::drawVertices(VertexMode vmode, int vertexCount,
                                 const SkPoint vertices[], const SkPoint texs[],
                                 const SkColor colors[], SkXfermode* xmode,
                                 const uint16_t indices[], int indexCount,
                                 const SkPaint& paint) {
    AUTO_LUA("drawVertices");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::drawData(const void* data, size_t length) {
    AUTO_LUA("drawData");
}
