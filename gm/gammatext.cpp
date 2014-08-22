/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkGradientShader.h"
#include "SkTypeface.h"

static SkShader* make_heatGradient(const SkPoint pts[2]) {
#if 0 // UNUSED
    const SkColor colors[] = {
        SK_ColorBLACK, SK_ColorBLUE, SK_ColorCYAN, SK_ColorGREEN,
        SK_ColorYELLOW, SK_ColorRED, SK_ColorWHITE
    };
#endif
    const SkColor bw[] = { SK_ColorBLACK, SK_ColorWHITE };

    return SkGradientShader::CreateLinear(pts, bw, NULL,
                                          SK_ARRAY_COUNT(bw),
                                          SkShader::kClamp_TileMode);
}

static bool setFont(SkPaint* paint, const char name[]) {
    SkTypeface* tf = sk_tool_utils::create_portable_typeface(name, SkTypeface::kNormal);
    if (tf) {
        paint->setTypeface(tf)->unref();
        return true;
    }
    return false;
}

#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#define BITMAP_INFO_RGB     (kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Host)

static CGContextRef makeCG(const SkImageInfo& info, const void* addr,
                           size_t rowBytes) {
    if (kN32_SkColorType != info.colorType() || NULL == addr) {
        return NULL;
    }
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGContextRef cg = CGBitmapContextCreate((void*)addr, info.width(), info.height(),
                                            8, rowBytes, space, BITMAP_INFO_RGB);
    CFRelease(space);

    CGContextSetAllowsFontSubpixelQuantization(cg, false);
    CGContextSetShouldSubpixelQuantizeFonts(cg, false);

    return cg;
}

extern CTFontRef SkTypeface_GetCTFontRef(const SkTypeface* face);

static CGFontRef typefaceToCGFont(const SkTypeface* face) {
    if (NULL == face) {
        return 0;
    }

    CTFontRef ct = SkTypeface_GetCTFontRef(face);
    return CTFontCopyGraphicsFont(ct, NULL);
}

static void cgSetPaintForText(CGContextRef cg, const SkPaint& paint) {
    SkColor c = paint.getColor();
    CGFloat rgba[] = {
        SkColorGetB(c) / 255.0f,
        SkColorGetG(c) / 255.0f,
        SkColorGetR(c) / 255.0f,
        SkColorGetA(c) / 255.0f,
    };
    CGContextSetRGBFillColor(cg, rgba[0], rgba[1], rgba[2], rgba[3]);

    CGContextSetTextDrawingMode(cg, kCGTextFill);
    CGContextSetFont(cg, typefaceToCGFont(paint.getTypeface()));
    CGContextSetFontSize(cg, SkScalarToFloat(paint.getTextSize()));

    CGContextSetAllowsFontSubpixelPositioning(cg, paint.isSubpixelText());
    CGContextSetShouldSubpixelPositionFonts(cg, paint.isSubpixelText());

    CGContextSetShouldAntialias(cg, paint.isAntiAlias());
    CGContextSetShouldSmoothFonts(cg, paint.isLCDRenderText());
}

static void cgDrawText(CGContextRef cg, const void* text, size_t len,
                       float x, float y, const SkPaint& paint) {
    if (cg) {
        cgSetPaintForText(cg, paint);

        uint16_t glyphs[200];
        int count = paint.textToGlyphs(text, len, glyphs);

        CGContextShowGlyphsAtPoint(cg, x, y, glyphs, count);
    }
}
#endif

/**
   Test a set of clipping problems discovered while writing blitAntiRect,
   and test all the code paths through the clipping blitters.
   Each region should show as a blue center surrounded by a 2px green
   border, with no red.
*/

#define HEIGHT 480

class GammaTextGM : public skiagm::GM {
public:
    GammaTextGM() {

    }

protected:
    virtual SkString onShortName() {
        return SkString("gammatext");
    }

    virtual SkISize onISize() {
        return SkISize::Make(1024, HEIGHT);
    }

    static void drawGrad(SkCanvas* canvas) {
        SkPoint pts[] = { { 0, 0 }, { 0, SkIntToScalar(HEIGHT) } };
#if 0
        const SkColor colors[] = { SK_ColorBLACK, SK_ColorWHITE };
        SkShader* s = SkGradientShader::CreateLinear(pts, colors, NULL, 2, SkShader::kClamp_TileMode);
#else
        SkShader* s = make_heatGradient(pts);
#endif

        canvas->clear(SK_ColorRED);
        SkPaint paint;
        paint.setShader(s)->unref();
        SkRect r = { 0, 0, SkIntToScalar(1024), SkIntToScalar(HEIGHT) };
        canvas->drawRect(r, paint);
    }

    virtual void onDraw(SkCanvas* canvas) {
#ifdef SK_BUILD_FOR_MAC
        CGContextRef cg = 0;
        {
            SkImageInfo info;
            size_t rowBytes;
            const void* addr = canvas->peekPixels(&info, &rowBytes);
            if (addr) {
                cg = makeCG(info, addr, rowBytes);
            }
        }
#endif

        drawGrad(canvas);

        const SkColor fg[] = {
            0xFFFFFFFF,
            0xFFFFFF00, 0xFFFF00FF, 0xFF00FFFF,
            0xFFFF0000, 0xFF00FF00, 0xFF0000FF,
            0xFF000000,
        };

        const char* text = "Hamburgefons";
        size_t len = strlen(text);

        SkPaint paint;
        setFont(&paint, "Times");
        paint.setTextSize(SkIntToScalar(16));
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);

        SkScalar x = SkIntToScalar(10);
        for (size_t i = 0; i < SK_ARRAY_COUNT(fg); ++i) {
            paint.setColor(fg[i]);

            SkScalar y = SkIntToScalar(40);
            SkScalar stopy = SkIntToScalar(HEIGHT);
            while (y < stopy) {
                if (true) {
                    canvas->drawText(text, len, x, y, paint);
                }
#ifdef SK_BUILD_FOR_MAC
                else {
                    cgDrawText(cg, text, len, SkScalarToFloat(x),
                               static_cast<float>(HEIGHT) - SkScalarToFloat(y),
                               paint);
                }
#endif
                y += paint.getTextSize() * 2;
            }
            x += SkIntToScalar(1024) / SK_ARRAY_COUNT(fg);
        }
#ifdef SK_BUILD_FOR_MAC
        CGContextRelease(cg);
#endif
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return new GammaTextGM; )

//////////////////////////////////////////////////////////////////////////////

static SkShader* make_gradient(SkColor c) {
    const SkPoint pts[] = { { 0, 0 }, { 240, 0 } };
    SkColor colors[2];
    colors[0] = c;
    colors[1] = SkColorSetA(c, 0);
    return SkGradientShader::CreateLinear(pts, colors, NULL, 2, SkShader::kClamp_TileMode);
}

static void set_face(SkPaint* paint) {
    SkTypeface* face = SkTypeface::CreateFromName("serif", SkTypeface::kItalic);
    SkSafeUnref(paint->setTypeface(face));
}

static void draw_pair(SkCanvas* canvas, SkPaint* paint, SkShader* shader) {
    const char text[] = "Now is the time for all good";
    const size_t len = strlen(text);
    
    paint->setShader(NULL);
    canvas->drawText(text, len, 10, 20, *paint);
    paint->setShader(SkShader::CreateColorShader(paint->getColor()))->unref();
    canvas->drawText(text, len, 10, 40, *paint);
    paint->setShader(shader);
    canvas->drawText(text, len, 10, 60, *paint);
}

class GammaShaderTextGM : public skiagm::GM {
    SkShader* fShaders[3];
    SkColor fColors[3];

public:
    GammaShaderTextGM() {
        const SkColor colors[] = { SK_ColorBLACK, SK_ColorRED, SK_ColorBLUE };
        for (size_t i = 0; i < SK_ARRAY_COUNT(fShaders); ++i) {
            fShaders[i] = NULL;
            fColors[i] = colors[i];
        }
    }

    virtual ~GammaShaderTextGM() {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fShaders); ++i) {
            SkSafeUnref(fShaders[i]);
        }
    }

protected:
    virtual SkString onShortName() {
        return SkString("gammagradienttext");
    }
    
    virtual SkISize onISize() {
        return SkISize::Make(300, 300);
    }

    virtual void onOnceBeforeDraw() {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fShaders); ++i) {
            fShaders[i] = make_gradient(fColors[i]);
        }
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        paint.setTextSize(18);
        set_face(&paint);

        for (size_t i = 0; i < SK_ARRAY_COUNT(fShaders); ++i) {
            paint.setColor(fColors[i]);
            draw_pair(canvas, &paint, fShaders[i]);
            canvas->translate(0, 80);
        }
    }
    
private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return new GammaShaderTextGM; )

