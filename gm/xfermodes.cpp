/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkBitmap.h"
#include "SkShader.h"
#include "SkPM4f.h"

enum SrcType {
    //! A WxH image with a rectangle in the lower right.
    kRectangleImage_SrcType               = 0x01,
    //! kRectangleImage_SrcType with an alpha of 34.5%.
    kRectangleImageWithAlpha_SrcType      = 0x02,
    //! kRectnagleImageWithAlpha_SrcType scaled down by half.
    kSmallRectangleImageWithAlpha_SrcType = 0x04,
    //! kRectangleImage_SrcType drawn directly instead in an image.
    kRectangle_SrcType                    = 0x08,
    //! Two rectangles, first on the right half, second on the bottom half.
    kQuarterClear_SrcType                 = 0x10,
    //! kQuarterClear_SrcType in a layer.
    kQuarterClearInLayer_SrcType          = 0x20,
    //! A W/2xH/2 transparent image.
    kSmallTransparentImage_SrcType        = 0x40,
    //! kRectangleImage_SrcType drawn directly with a mask.
    kRectangleWithMask_SrcType            = 0x80,

    kAll_SrcType                          = 0xFF, //!< All the source types.
    kBasic_SrcType                        = 0x03, //!< Just basic source types.
};

const struct {
    SkBlendMode fMode;
    int         fSourceTypeMask;  // The source types to use this
    // mode with. See draw_mode for
    // an explanation of each type.
    // PDF has to play some tricks
    // to support the base modes,
    // test those more extensively.
} gModes[] = {
    { SkBlendMode::kClear,        kAll_SrcType   },
    { SkBlendMode::kSrc,          kAll_SrcType   },
    { SkBlendMode::kDst,          kAll_SrcType   },
    { SkBlendMode::kSrcOver,      kAll_SrcType   },
    { SkBlendMode::kDstOver,      kAll_SrcType   },
    { SkBlendMode::kSrcIn,        kAll_SrcType   },
    { SkBlendMode::kDstIn,        kAll_SrcType   },
    { SkBlendMode::kSrcOut,       kAll_SrcType   },
    { SkBlendMode::kDstOut,       kAll_SrcType   },
    { SkBlendMode::kSrcATop,      kAll_SrcType   },
    { SkBlendMode::kDstATop,      kAll_SrcType   },

    { SkBlendMode::kXor,          kBasic_SrcType },
    { SkBlendMode::kPlus,         kBasic_SrcType },
    { SkBlendMode::kModulate,     kAll_SrcType   },
    { SkBlendMode::kScreen,       kBasic_SrcType },
    { SkBlendMode::kOverlay,      kBasic_SrcType },
    { SkBlendMode::kDarken,       kBasic_SrcType },
    { SkBlendMode::kLighten,      kBasic_SrcType },
    { SkBlendMode::kColorDodge,   kBasic_SrcType },
    { SkBlendMode::kColorBurn,    kBasic_SrcType },
    { SkBlendMode::kHardLight,    kBasic_SrcType },
    { SkBlendMode::kSoftLight,    kBasic_SrcType },
    { SkBlendMode::kDifference,   kBasic_SrcType },
    { SkBlendMode::kExclusion,    kBasic_SrcType },
    { SkBlendMode::kMultiply,     kAll_SrcType   },
    { SkBlendMode::kHue,          kBasic_SrcType },
    { SkBlendMode::kSaturation,   kBasic_SrcType },
    { SkBlendMode::kColor,        kBasic_SrcType },
    { SkBlendMode::kLuminosity,   kBasic_SrcType },
};

static void make_bitmaps(int w, int h, SkBitmap* src, SkBitmap* dst,
                         SkBitmap* transparent) {
    src->allocN32Pixels(w, h);
    src->eraseColor(SK_ColorTRANSPARENT);

    SkPaint p;
    p.setAntiAlias(true);

    SkRect r;
    SkScalar ww = SkIntToScalar(w);
    SkScalar hh = SkIntToScalar(h);

    {
        SkCanvas c(*src);
        p.setColor(sk_tool_utils::color_to_565(0xFFFFCC44));
        r.set(0, 0, ww*3/4, hh*3/4);
        c.drawOval(r, p);
    }

    dst->allocN32Pixels(w, h);
    dst->eraseColor(SK_ColorTRANSPARENT);

    {
        SkCanvas c(*dst);
        p.setColor(sk_tool_utils::color_to_565(0xFF66AAFF));
        r.set(ww/3, hh/3, ww*19/20, hh*19/20);
        c.drawRect(r, p);
    }

    transparent->allocN32Pixels(w, h);
    transparent->eraseColor(SK_ColorTRANSPARENT);
}

static uint16_t gData[] = { 0xFFFF, 0xCCCF, 0xCCCF, 0xFFFF };

class XfermodesGM : public skiagm::GM {
    SkBitmap    fBG;
    SkBitmap    fSrcB, fDstB, fTransparent;

    /* The srcType argument indicates what to draw for the source part. Skia
     * uses the implied shape of the drawing command and these modes
     * demonstrate that.
     */
    void draw_mode(SkCanvas* canvas, SkBlendMode mode, SrcType srcType, SkScalar x, SkScalar y) {
        SkPaint p;
        SkMatrix m;
        bool restoreNeeded = false;
        m.setTranslate(x, y);

        canvas->drawBitmap(fSrcB, x, y, &p);
        p.setBlendMode(mode);
        switch (srcType) {
            case kSmallTransparentImage_SrcType: {
                m.postScale(SK_ScalarHalf, SK_ScalarHalf, x, y);

                SkAutoCanvasRestore acr(canvas, true);
                canvas->concat(m);
                canvas->drawBitmap(fTransparent, 0, 0, &p);
                break;
            }
            case kQuarterClearInLayer_SrcType: {
                SkRect bounds = SkRect::MakeXYWH(x, y, SkIntToScalar(W),
                                                 SkIntToScalar(H));
                canvas->saveLayer(&bounds, &p);
                restoreNeeded = true;
                p.setBlendMode(SkBlendMode::kSrcOver);
                // Fall through.
            }
            case kQuarterClear_SrcType: {
                SkScalar halfW = SkIntToScalar(W) / 2;
                SkScalar halfH = SkIntToScalar(H) / 2;
                p.setColor(sk_tool_utils::color_to_565(0xFF66AAFF));
                SkRect r = SkRect::MakeXYWH(x + halfW, y, halfW,
                                            SkIntToScalar(H));
                canvas->drawRect(r, p);
                p.setColor(sk_tool_utils::color_to_565(0xFFAA66FF));
                r = SkRect::MakeXYWH(x, y + halfH, SkIntToScalar(W), halfH);
                canvas->drawRect(r, p);
                break;
            }
            case kRectangleWithMask_SrcType: {
                canvas->save();
                restoreNeeded = true;
                SkScalar w = SkIntToScalar(W);
                SkScalar h = SkIntToScalar(H);
                SkRect r = SkRect::MakeXYWH(x, y + h / 4, w, h * 23 / 60);
                canvas->clipRect(r);
                // Fall through.
            }
            case kRectangle_SrcType: {
                SkScalar w = SkIntToScalar(W);
                SkScalar h = SkIntToScalar(H);
                SkRect r = SkRect::MakeXYWH(x + w / 3, y + h / 3,
                                            w * 37 / 60, h * 37 / 60);
                p.setColor(sk_tool_utils::color_to_565(0xFF66AAFF));
                canvas->drawRect(r, p);
                break;
            }
            case kSmallRectangleImageWithAlpha_SrcType:
                m.postScale(SK_ScalarHalf, SK_ScalarHalf, x, y);
                // Fall through.
            case kRectangleImageWithAlpha_SrcType:
                p.setAlpha(0x88);
                // Fall through.
            case kRectangleImage_SrcType: {
                SkAutoCanvasRestore acr(canvas, true);
                canvas->concat(m);
                canvas->drawBitmap(fDstB, 0, 0, &p);
                break;
            }
            default:
                break;
        }

        if (restoreNeeded) {
            canvas->restore();
        }
    }

    void onOnceBeforeDraw() override {
        fBG.installPixels(SkImageInfo::Make(2, 2, kARGB_4444_SkColorType,
                                            kOpaque_SkAlphaType),
                          gData, 4);

        make_bitmaps(W, H, &fSrcB, &fDstB, &fTransparent);
    }

public:
    const static int W = 64;
    const static int H = 64;
    XfermodesGM() {}

protected:
    SkString onShortName() override {
        return SkString("xfermodes");
    }

    SkISize onISize() override {
        return SkISize::Make(1990, 570);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        const SkScalar w = SkIntToScalar(W);
        const SkScalar h = SkIntToScalar(H);
        SkMatrix m;
        m.setScale(SkIntToScalar(6), SkIntToScalar(6));
        auto s = SkShader::MakeBitmapShader(fBG, SkShader::kRepeat_TileMode,
                                            SkShader::kRepeat_TileMode, &m);

        SkPaint labelP;
        labelP.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&labelP);
        labelP.setTextAlign(SkPaint::kCenter_Align);

        const int W = 5;

        SkScalar x0 = 0;
        SkScalar y0 = 0;
        for (int sourceType = 1; sourceType & kAll_SrcType; sourceType <<= 1) {
            SkScalar x = x0, y = y0;
            for (size_t i = 0; i < SK_ARRAY_COUNT(gModes); i++) {
                if ((gModes[i].fSourceTypeMask & sourceType) == 0) {
                    continue;
                }
                SkRect r{ x, y, x+w, y+h };

                SkPaint p;
                p.setStyle(SkPaint::kFill_Style);
                p.setShader(s);
                canvas->drawRect(r, p);

                canvas->saveLayer(&r, nullptr);
                draw_mode(canvas, gModes[i].fMode, static_cast<SrcType>(sourceType),
                          r.fLeft, r.fTop);
                canvas->restore();

                r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
                p.setStyle(SkPaint::kStroke_Style);
                p.setShader(nullptr);
                canvas->drawRect(r, p);

#if 1
                const char* label = SkBlendMode_Name(gModes[i].fMode);
                canvas->drawString(label,
                                 x + w/2, y - labelP.getTextSize()/2, labelP);
#endif
                x += w + SkIntToScalar(10);
                if ((i % W) == W - 1) {
                    x = x0;
                    y += h + SkIntToScalar(30);
                }
            }
            if (y < 320) {
                if (x > x0) {
                    y += h + SkIntToScalar(30);
                }
                y0 = y;
            } else {
                x0 += SkIntToScalar(400);
                y0 = 0;
            }
        }
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new XfermodesGM; )
