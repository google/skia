// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(PaintDump, 256, 256, true, 0) {
static const char* str(SkPaint::Cap v) {
    switch (v) {
        case SkPaint::kButt_Cap:   return "SkPaint::kButt_Cap";
        case SkPaint::kRound_Cap:  return "SkPaint::kRound_Cap";
        case SkPaint::kSquare_Cap: return "SkPaint::kSquare_Cap";
        default: return "?";
    }
}
static const char* str(SkPaint::Join v) {
    switch (v) {
        case SkPaint::kMiter_Join: return "SkPaint::kMiter_Join";
        case SkPaint::kRound_Join: return "SkPaint::kRound_Join";
        case SkPaint::kBevel_Join: return "SkPaint::kBevel_Join";
        default: return "?";
    }
}
static const char* str(SkPaint::Style v) {
    switch (v) {
        case SkPaint::kFill_Style:          return "SkPaint::kFill_Style";
        case SkPaint::kStroke_Style:        return "SkPaint::kStroke_Style";
        case SkPaint::kStrokeAndFill_Style: return "SkPaint::kStrokeAndFill_Style";
        default: return "?";
    }
}

static const char* str(bool v) { return v ? "true" : "false"; }

SkString PaintStringDump(const SkPaint& p) {
    SkString s("SkPaint p;\n");
    SkPaint d;
    if (d.getStrokeWidth() != p.getStrokeWidth()) {
        s.appendf("p.setStrokeWidth(%.9g);\n", p.getStrokeWidth());
    }
    if (d.getStrokeMiter() != p.getStrokeMiter()) {
        s.appendf("p.setStrokeMiter(%.9g);\n", p.getStrokeMiter());
    }
    SkColor4f c = p.getColor4f();
    if (c != d.getColor4f()) {
        s.appendf("p.setColor4f({%.9g, %.9g, %.9g, %.9g}, nullptr);\n", c.fR, c.fG, c.fB, c.fA);
    }
    if (d.isAntiAlias() != p.isAntiAlias()) {
        s.appendf("p.setAntiAlias(%s);\n", str(p.isAntiAlias()));
    }
    if (d.isDither() != p.isDither()) {
        s.appendf("p.setDither(%s);\n", str(p.isDither()));
    }
    if (d.getStrokeCap() != p.getStrokeCap()) {
        s.appendf("p.setStrokeCap(%s);\n", str(p.getStrokeCap()));
    }
    if (d.getStrokeJoin() != p.getStrokeJoin()) {
        s.appendf("p.setStrokeJoin(%s);\n", str(p.getStrokeJoin()));
    }
    if (d.getStyle() != p.getStyle()) {
        s.appendf("p.setStyle(%s);\n", str(p.getStyle()));
    }
    if (d.getBlendMode() != p.getBlendMode()) {
        s.appendf("p.setBlendMode(SkBlendMode::k%s);\n", SkBlendMode_Name(p.getBlendMode()));
    }
    if (p.getPathEffect()) {
        s.appendf("p.setPathEffect(/*FIXME*/);\n");
    }
    if (p.getShader()) {
        s.appendf("p.setShader(/*FIXME*/);\n");
    }
    if (p.getMaskFilter()) {
        s.appendf("p.setMaskFilter(/*FIXME*/);\n");
    }
    if (p.getColorFilter()) {
        s.appendf("p.setColorFilter(/*FIXME*/);\n");
    }
    if (p.getImageFilter()) {
        s.appendf("p.setImageFilter(/*FIXME*/);\n");
    }
    return s;
}

void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);
    p.setBlendMode(SkBlendMode::kDstOver);
    p.setStrokeCap(SkPaint::kRound_Cap);
    p.setShader(image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions()));

    auto s = PaintStringDump(p);
    SkDebugf("%s", s.c_str());
}

}  // END FIDDLE
