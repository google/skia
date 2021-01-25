/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreText/CoreText.h>
#include <CoreText/CTFontManager.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkColorData.h"
#include "include/private/SkFixed.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkEndian.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkUtils.h"
#include "src/ports/SkScalerContext_mac_ct.h"
#include "src/ports/SkTypeface_mac_ct.h"
#include "src/sfnt/SkOTTableTypes.h"
#include "src/sfnt/SkOTTable_OS_2.h"
#include "src/utils/mac/SkCGBase.h"
#include "src/utils/mac/SkCGGeometry.h"
#include "src/utils/mac/SkCTFont.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <algorithm>

class SkDescriptor;


// Set to make glyph bounding boxes visible.
#define SK_SHOW_TEXT_BLIT_COVERAGE 0

static void sk_memset_rect32(uint32_t* ptr, uint32_t value,
                             int width, int height, size_t rowBytes) {
    SkASSERT(width);
    SkASSERT(width * sizeof(uint32_t) <= rowBytes);

    if (width >= 32) {
        while (height) {
            sk_memset32(ptr, value, width);
            ptr = (uint32_t*)((char*)ptr + rowBytes);
            height -= 1;
        }
        return;
    }

    rowBytes -= width * sizeof(uint32_t);

    if (width >= 8) {
        while (height) {
            int w = width;
            do {
                *ptr++ = value; *ptr++ = value;
                *ptr++ = value; *ptr++ = value;
                *ptr++ = value; *ptr++ = value;
                *ptr++ = value; *ptr++ = value;
                w -= 8;
            } while (w >= 8);
            while (--w >= 0) {
                *ptr++ = value;
            }
            ptr = (uint32_t*)((char*)ptr + rowBytes);
            height -= 1;
        }
    } else {
        while (height) {
            int w = width;
            do {
                *ptr++ = value;
            } while (--w > 0);
            ptr = (uint32_t*)((char*)ptr + rowBytes);
            height -= 1;
        }
    }
}

static unsigned CGRGBPixel_getAlpha(CGRGBPixel pixel) {
    return pixel & 0xFF;
}

static CGAffineTransform MatrixToCGAffineTransform(const SkMatrix& matrix) {
    return CGAffineTransformMake( SkScalarToCGFloat(matrix[SkMatrix::kMScaleX]),
                                 -SkScalarToCGFloat(matrix[SkMatrix::kMSkewY] ),
                                 -SkScalarToCGFloat(matrix[SkMatrix::kMSkewX] ),
                                  SkScalarToCGFloat(matrix[SkMatrix::kMScaleY]),
                                  SkScalarToCGFloat(matrix[SkMatrix::kMTransX]),
                                  SkScalarToCGFloat(matrix[SkMatrix::kMTransY]));
}

SkScalerContext_Mac::SkScalerContext_Mac(sk_sp<SkTypeface_Mac> typeface,
                                         const SkScalerContextEffects& effects,
                                         const SkDescriptor* desc)
        : INHERITED(std::move(typeface), effects, desc)
        , fDoSubPosition(SkToBool(fRec.fFlags & kSubpixelPositioning_Flag))

{
    CTFontRef ctFont = (CTFontRef)this->getTypeface()->internal_private_getCTFontRef();

    // CT on (at least) 10.9 will size color glyphs down from the requested size, but not up.
    // As a result, it is necessary to know the actual device size and request that.
    SkVector scale;
    SkMatrix skTransform;
    bool invertible = fRec.computeMatrices(SkScalerContextRec::kVertical_PreMatrixScale,
                                           &scale, &skTransform, nullptr, nullptr, nullptr);
    fTransform = MatrixToCGAffineTransform(skTransform);
    // CGAffineTransformInvert documents that if the transform is non-invertible it will return the
    // passed transform unchanged. It does so, but then also prints a message to stdout. Avoid this.
    if (invertible) {
        fInvTransform = CGAffineTransformInvert(fTransform);
    } else {
        fInvTransform = fTransform;
    }

    // The transform contains everything except the requested text size.
    // Some properties, like 'trak', are based on the optical text size.
    CGFloat textSize = SkScalarToCGFloat(scale.y());
    fCTFont = SkCTFontCreateExactCopy(ctFont, textSize,
                                      ((SkTypeface_Mac*)this->getTypeface())->fOpszVariation);
    fCGFont.reset(CTFontCopyGraphicsFont(fCTFont.get(), nullptr));
}

static int RoundSize(int dimension) {
    return SkNextPow2(dimension);
}

CGRGBPixel* SkScalerContext_Mac::Offscreen::getCG(const SkScalerContext_Mac& context,
                                                  const SkGlyph& glyph, CGGlyph glyphID,
                                                  size_t* rowBytesPtr,
                                                  bool generateA8FromLCD) {
    if (!fRGBSpace) {
        //It doesn't appear to matter what color space is specified.
        //Regular blends and antialiased text are always (s*a + d*(1-a))
        //and subpixel antialiased text is always g=2.0.
        fRGBSpace.reset(CGColorSpaceCreateDeviceRGB());
    }

    // default to kBW_Format
    bool doAA = false;
    bool doLCD = false;

    if (SkMask::kBW_Format != glyph.maskFormat()) {
        doLCD = true;
        doAA = true;
    }

    // FIXME: lcd smoothed un-hinted rasterization unsupported.
    if (!generateA8FromLCD && SkMask::kA8_Format == glyph.maskFormat()) {
        doLCD = false;
        doAA = true;
    }

    // If this font might have color glyphs, disable LCD as there's no way to support it.
    // CoreText doesn't tell us which format it ended up using, so we can't detect it.
    // A8 will end up black on transparent, but TODO: we can detect gray and set to A8.
    if (SkMask::kARGB32_Format == glyph.maskFormat()) {
        doLCD = false;
    }

    size_t rowBytes = fSize.fWidth * sizeof(CGRGBPixel);
    if (!fCG || fSize.fWidth < glyph.width() || fSize.fHeight < glyph.height()) {
        if (fSize.fWidth < glyph.width()) {
            fSize.fWidth = RoundSize(glyph.width());
        }
        if (fSize.fHeight < glyph.height()) {
            fSize.fHeight = RoundSize(glyph.height());
        }

        rowBytes = fSize.fWidth * sizeof(CGRGBPixel);
        void* image = fImageStorage.reset(rowBytes * fSize.fHeight);
        const CGImageAlphaInfo alpha = (glyph.isColor())
                                     ? kCGImageAlphaPremultipliedFirst
                                     : kCGImageAlphaNoneSkipFirst;
        const CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Host | (CGBitmapInfo)alpha;
        fCG.reset(CGBitmapContextCreate(image, fSize.fWidth, fSize.fHeight, 8,
                                        rowBytes, fRGBSpace.get(), bitmapInfo));

        // Skia handles quantization and subpixel positioning,
        // so disable quantization and enable subpixel positioning in CG.
        CGContextSetAllowsFontSubpixelQuantization(fCG.get(), false);
        CGContextSetShouldSubpixelQuantizeFonts(fCG.get(), false);

        // Because CG always draws from the horizontal baseline,
        // if there is a non-integral translation from the horizontal origin to the vertical origin,
        // then CG cannot draw the glyph in the correct location without subpixel positioning.
        CGContextSetAllowsFontSubpixelPositioning(fCG.get(), true);
        CGContextSetShouldSubpixelPositionFonts(fCG.get(), true);

        CGContextSetTextDrawingMode(fCG.get(), kCGTextFill);

        // Draw black on white to create mask. (Special path exists to speed this up in CG.)
        CGContextSetGrayFillColor(fCG.get(), 0.0f, 1.0f);

        // force our checks below to happen
        fDoAA = !doAA;
        fDoLCD = !doLCD;

        CGContextSetTextMatrix(fCG.get(), context.fTransform);
    }

    if (fDoAA != doAA) {
        CGContextSetShouldAntialias(fCG.get(), doAA);
        fDoAA = doAA;
    }
    if (fDoLCD != doLCD) {
        CGContextSetShouldSmoothFonts(fCG.get(), doLCD);
        fDoLCD = doLCD;
    }

    CGRGBPixel* image = (CGRGBPixel*)fImageStorage.get();
    // skip rows based on the glyph's height
    image += (fSize.fHeight - glyph.height()) * fSize.fWidth;

    // Erase to white (or transparent black if it's a color glyph, to not composite against white).
    uint32_t bgColor = (!glyph.isColor()) ? 0xFFFFFFFF : 0x00000000;
    sk_memset_rect32(image, bgColor, glyph.width(), glyph.height(), rowBytes);

    float subX = 0;
    float subY = 0;
    if (context.fDoSubPosition) {
        subX = SkFixedToFloat(glyph.getSubXFixed());
        subY = SkFixedToFloat(glyph.getSubYFixed());
    }

    CGPoint point = CGPointMake(-glyph.left() + subX, glyph.top() + glyph.height() - subY);
    // Prior to 10.10, CTFontDrawGlyphs acted like CGContextShowGlyphsAtPositions and took
    // 'positions' which are in text space. The glyph location (in device space) must be
    // mapped into text space, so that CG can convert it back into device space.
    // In 10.10.1, this is handled directly in CTFontDrawGlyphs.
    //
    // However, in 10.10.2 color glyphs no longer rotate based on the font transform.
    // So always make the font transform identity and place the transform on the context.
    point = CGPointApplyAffineTransform(point, context.fInvTransform);

    CTFontDrawGlyphs(context.fCTFont.get(), &glyphID, &point, 1, fCG.get());

    SkASSERT(rowBytesPtr);
    *rowBytesPtr = rowBytes;
    return image;
}

bool SkScalerContext_Mac::generateAdvance(SkGlyph* glyph) {
    return false;
}

void SkScalerContext_Mac::generateMetrics(SkGlyph* glyph) {
    glyph->fMaskFormat = fRec.fMaskFormat;

    const CGGlyph cgGlyph = (CGGlyph) glyph->getGlyphID();
    glyph->zeroMetrics();

    // The following block produces cgAdvance in CG units (pixels, y up).
    CGSize cgAdvance;
    CTFontGetAdvancesForGlyphs(fCTFont.get(), kCTFontOrientationHorizontal,
                               &cgGlyph, &cgAdvance, 1);
    cgAdvance = CGSizeApplyAffineTransform(cgAdvance, fTransform);
    glyph->fAdvanceX =  SkFloatFromCGFloat(cgAdvance.width);
    glyph->fAdvanceY = -SkFloatFromCGFloat(cgAdvance.height);

    // The following produces skBounds in SkGlyph units (pixels, y down),
    // or returns early if skBounds would be empty.
    SkRect skBounds;

    // Glyphs are always drawn from the horizontal origin. The caller must manually use the result
    // of CTFontGetVerticalTranslationsForGlyphs to calculate where to draw the glyph for vertical
    // glyphs. As a result, always get the horizontal bounds of a glyph and translate it if the
    // glyph is vertical. This avoids any diagreement between the various means of retrieving
    // vertical metrics.
    {
        // CTFontGetBoundingRectsForGlyphs produces cgBounds in CG units (pixels, y up).
        CGRect cgBounds;
        CTFontGetBoundingRectsForGlyphs(fCTFont.get(), kCTFontOrientationHorizontal,
                                        &cgGlyph, &cgBounds, 1);
        cgBounds = CGRectApplyAffineTransform(cgBounds, fTransform);

        // BUG?
        // 0x200B (zero-advance space) seems to return a huge (garbage) bounds, when
        // it should be empty. So, if we see a zero-advance, we check if it has an
        // empty path or not, and if so, we jam the bounds to 0. Hopefully a zero-advance
        // is rare, so we won't incur a big performance cost for this extra check.
        if (0 == cgAdvance.width && 0 == cgAdvance.height) {
            SkUniqueCFRef<CGPathRef> path(CTFontCreatePathForGlyph(fCTFont.get(), cgGlyph,nullptr));
            if (!path || CGPathIsEmpty(path.get())) {
                return;
            }
        }

        if (SkCGRectIsEmpty(cgBounds)) {
            return;
        }

        // Convert cgBounds to SkGlyph units (pixels, y down).
        skBounds = SkRect::MakeXYWH(cgBounds.origin.x, -cgBounds.origin.y - cgBounds.size.height,
                                    cgBounds.size.width, cgBounds.size.height);
    }

    // Currently the bounds are based on being rendered at (0,0).
    // The top left must not move, since that is the base from which subpixel positioning is offset.
    if (fDoSubPosition) {
        skBounds.fRight += SkFixedToFloat(glyph->getSubXFixed());
        skBounds.fBottom += SkFixedToFloat(glyph->getSubYFixed());
    }

    // We're trying to pack left and top into int16_t,
    // and width and height into uint16_t, after outsetting by 1.
    if (!SkRect::MakeXYWH(-32767, -32767, 65535, 65535).contains(skBounds)) {
        return;
    }

    SkIRect skIBounds;
    skBounds.roundOut(&skIBounds);
    // Expand the bounds by 1 pixel, to give CG room for anti-aliasing.
    // Note that this outset is to allow room for LCD smoothed glyphs. However, the correct outset
    // is not currently known, as CG dilates the outlines by some percentage.
    // Note that if this context is A8 and not back-forming from LCD, there is no need to outset.
    skIBounds.outset(1, 1);
    glyph->fLeft = SkToS16(skIBounds.fLeft);
    glyph->fTop = SkToS16(skIBounds.fTop);
    glyph->fWidth = SkToU16(skIBounds.width());
    glyph->fHeight = SkToU16(skIBounds.height());
}

static constexpr uint8_t sk_pow2_table(size_t i) {
    return SkToU8(((i * i + 128) / 255));
}

/**
 *  This will invert the gamma applied by CoreGraphics, so we can get linear
 *  values.
 *
 *  CoreGraphics obscurely defaults to 2.0 as the subpixel coverage gamma value.
 *  The color space used does not appear to affect this choice.
 */
static constexpr auto gLinearCoverageFromCGLCDValue = SkMakeArray<256>(sk_pow2_table);

static void cgpixels_to_bits(uint8_t dst[], const CGRGBPixel src[], int count) {
    while (count > 0) {
        uint8_t mask = 0;
        for (int i = 7; i >= 0; --i) {
            mask |= ((CGRGBPixel_getAlpha(*src++) >> 7) ^ 0x1) << i;
            if (0 == --count) {
                break;
            }
        }
        *dst++ = mask;
    }
}

template<bool APPLY_PREBLEND>
static inline uint8_t rgb_to_a8(CGRGBPixel rgb, const uint8_t* table8) {
    U8CPU r = 0xFF - ((rgb >> 16) & 0xFF);
    U8CPU g = 0xFF - ((rgb >>  8) & 0xFF);
    U8CPU b = 0xFF - ((rgb >>  0) & 0xFF);
    U8CPU lum = sk_apply_lut_if<APPLY_PREBLEND>(SkComputeLuminance(r, g, b), table8);
#if SK_SHOW_TEXT_BLIT_COVERAGE
    lum = std::max(lum, (U8CPU)0x30);
#endif
    return lum;
}

template<bool APPLY_PREBLEND>
static void RGBToA8(const CGRGBPixel* SK_RESTRICT cgPixels, size_t cgRowBytes,
                    const SkGlyph& glyph, void* glyphImage, const uint8_t* table8) {
    const int width = glyph.width();
    const int height = glyph.height();
    size_t dstRB = glyph.rowBytes();
    uint8_t* SK_RESTRICT dst = (uint8_t*)glyphImage;

    for (int y = 0; y < height; y++) {
        for (int i = 0; i < width; ++i) {
            dst[i] = rgb_to_a8<APPLY_PREBLEND>(cgPixels[i], table8);
        }
        cgPixels = SkTAddOffset<const CGRGBPixel>(cgPixels, cgRowBytes);
        dst = SkTAddOffset<uint8_t>(dst, dstRB);
    }
}

template<bool APPLY_PREBLEND>
static uint16_t RGBToLcd16(CGRGBPixel rgb,
                           const uint8_t* tableR, const uint8_t* tableG, const uint8_t* tableB) {
    U8CPU r = sk_apply_lut_if<APPLY_PREBLEND>(0xFF - ((rgb >> 16) & 0xFF), tableR);
    U8CPU g = sk_apply_lut_if<APPLY_PREBLEND>(0xFF - ((rgb >>  8) & 0xFF), tableG);
    U8CPU b = sk_apply_lut_if<APPLY_PREBLEND>(0xFF - ((rgb >>  0) & 0xFF), tableB);
#if SK_SHOW_TEXT_BLIT_COVERAGE
    r = std::max(r, (U8CPU)0x30);
    g = std::max(g, (U8CPU)0x30);
    b = std::max(b, (U8CPU)0x30);
#endif
    return SkPack888ToRGB16(r, g, b);
}

template<bool APPLY_PREBLEND>
static void RGBToLcd16(const CGRGBPixel* SK_RESTRICT cgPixels, size_t cgRowBytes,
                       const SkGlyph& glyph, void* glyphImage,
                       const uint8_t* tableR, const uint8_t* tableG, const uint8_t* tableB) {
    const int width = glyph.width();
    const int height = glyph.height();
    size_t dstRB = glyph.rowBytes();
    uint16_t* SK_RESTRICT dst = (uint16_t*)glyphImage;

    for (int y = 0; y < height; y++) {
        for (int i = 0; i < width; i++) {
            dst[i] = RGBToLcd16<APPLY_PREBLEND>(cgPixels[i], tableR, tableG, tableB);
        }
        cgPixels = SkTAddOffset<const CGRGBPixel>(cgPixels, cgRowBytes);
        dst = SkTAddOffset<uint16_t>(dst, dstRB);
    }
}

static SkPMColor cgpixels_to_pmcolor(CGRGBPixel rgb) {
    U8CPU a = (rgb >> 24) & 0xFF;
    U8CPU r = (rgb >> 16) & 0xFF;
    U8CPU g = (rgb >>  8) & 0xFF;
    U8CPU b = (rgb >>  0) & 0xFF;
#if SK_SHOW_TEXT_BLIT_COVERAGE
    a = std::max(a, (U8CPU)0x30);
#endif
    return SkPackARGB32(a, r, g, b);
}

void SkScalerContext_Mac::generateImage(const SkGlyph& glyph) {
    CGGlyph cgGlyph = SkTo<CGGlyph>(glyph.getGlyphID());

    // FIXME: lcd smoothed un-hinted rasterization unsupported.
    bool requestSmooth = fRec.getHinting() != SkFontHinting::kNone;

    // Draw the glyph
    size_t cgRowBytes;
    CGRGBPixel* cgPixels = fOffscreen.getCG(*this, glyph, cgGlyph, &cgRowBytes, requestSmooth);
    if (cgPixels == nullptr) {
        return;
    }

    // Fix the glyph
    if ((glyph.fMaskFormat == SkMask::kLCD16_Format) ||
        (glyph.fMaskFormat == SkMask::kA8_Format
         && requestSmooth
         && SkCTFontGetSmoothBehavior() != SkCTFontSmoothBehavior::none))
    {
        const uint8_t* linear = gLinearCoverageFromCGLCDValue.data();

        //Note that the following cannot really be integrated into the
        //pre-blend, since we may not be applying the pre-blend; when we aren't
        //applying the pre-blend it means that a filter wants linear anyway.
        //Other code may also be applying the pre-blend, so we'd need another
        //one with this and one without.
        CGRGBPixel* addr = cgPixels;
        for (int y = 0; y < glyph.fHeight; ++y) {
            for (int x = 0; x < glyph.fWidth; ++x) {
                int r = (addr[x] >> 16) & 0xFF;
                int g = (addr[x] >>  8) & 0xFF;
                int b = (addr[x] >>  0) & 0xFF;
                addr[x] = (linear[r] << 16) | (linear[g] << 8) | linear[b];
            }
            addr = SkTAddOffset<CGRGBPixel>(addr, cgRowBytes);
        }
    }

    // Convert glyph to mask
    switch (glyph.fMaskFormat) {
        case SkMask::kLCD16_Format: {
            if (fPreBlend.isApplicable()) {
                RGBToLcd16<true>(cgPixels, cgRowBytes, glyph, glyph.fImage,
                                 fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            } else {
                RGBToLcd16<false>(cgPixels, cgRowBytes, glyph, glyph.fImage,
                                  fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            }
        } break;
        case SkMask::kA8_Format: {
            if (fPreBlend.isApplicable()) {
                RGBToA8<true>(cgPixels, cgRowBytes, glyph, glyph.fImage, fPreBlend.fG);
            } else {
                RGBToA8<false>(cgPixels, cgRowBytes, glyph, glyph.fImage, fPreBlend.fG);
            }
        } break;
        case SkMask::kBW_Format: {
            const int width = glyph.fWidth;
            size_t dstRB = glyph.rowBytes();
            uint8_t* dst = (uint8_t*)glyph.fImage;
            for (int y = 0; y < glyph.fHeight; y++) {
                cgpixels_to_bits(dst, cgPixels, width);
                cgPixels = SkTAddOffset<CGRGBPixel>(cgPixels, cgRowBytes);
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
            }
        } break;
        case SkMask::kARGB32_Format: {
            const int width = glyph.fWidth;
            size_t dstRB = glyph.rowBytes();
            SkPMColor* dst = (SkPMColor*)glyph.fImage;
            for (int y = 0; y < glyph.fHeight; y++) {
                for (int x = 0; x < width; ++x) {
                    dst[x] = cgpixels_to_pmcolor(cgPixels[x]);
                }
                cgPixels = SkTAddOffset<CGRGBPixel>(cgPixels, cgRowBytes);
                dst = SkTAddOffset<SkPMColor>(dst, dstRB);
            }
        } break;
        default:
            SkDEBUGFAIL("unexpected mask format");
            break;
    }
}

namespace {
class SkCTPathGeometrySink {
    SkPathBuilder fBuilder;
    bool fStarted;
    CGPoint fCurrent;

    void goingTo(const CGPoint pt) {
        if (!fStarted) {
            fStarted = true;
            fBuilder.moveTo(fCurrent.x, -fCurrent.y);
        }
        fCurrent = pt;
    }

    bool currentIsNot(const CGPoint pt) {
        return fCurrent.x != pt.x || fCurrent.y != pt.y;
    }

public:
    SkCTPathGeometrySink() : fStarted{false}, fCurrent{0,0} {}

    SkPath detach() { return fBuilder.detach(); }

    static void ApplyElement(void *ctx, const CGPathElement *element) {
        SkCTPathGeometrySink& self = *(SkCTPathGeometrySink*)ctx;
        CGPoint* points = element->points;

        switch (element->type) {
            case kCGPathElementMoveToPoint:
                self.fStarted = false;
                self.fCurrent = points[0];
                break;

            case kCGPathElementAddLineToPoint:
                if (self.currentIsNot(points[0])) {
                    self.goingTo(points[0]);
                    self.fBuilder.lineTo(points[0].x, -points[0].y);
                }
                break;

            case kCGPathElementAddQuadCurveToPoint:
                if (self.currentIsNot(points[0]) || self.currentIsNot(points[1])) {
                    self.goingTo(points[1]);
                    self.fBuilder.quadTo(points[0].x, -points[0].y,
                                         points[1].x, -points[1].y);
                }
                break;

            case kCGPathElementAddCurveToPoint:
                if (self.currentIsNot(points[0]) ||
                    self.currentIsNot(points[1]) ||
                    self.currentIsNot(points[2]))
                {
                    self.goingTo(points[2]);
                    self.fBuilder.cubicTo(points[0].x, -points[0].y,
                                          points[1].x, -points[1].y,
                                          points[2].x, -points[2].y);
                }
                break;

            case kCGPathElementCloseSubpath:
                if (self.fStarted) {
                    self.fBuilder.close();
                }
                break;

            default:
                SkDEBUGFAIL("Unknown path element!");
                break;
            }
    }
};
} // namespace

/*
 *  Our subpixel resolution is only 2 bits in each direction, so a scale of 4
 *  seems sufficient, and possibly even correct, to allow the hinted outline
 *  to be subpixel positioned.
 */
#define kScaleForSubPixelPositionHinting (4.0f)

bool SkScalerContext_Mac::generatePath(SkGlyphID glyph, SkPath* path) {
    SkScalar scaleX = SK_Scalar1;
    SkScalar scaleY = SK_Scalar1;

    CGAffineTransform xform = fTransform;
    /*
     *  For subpixel positioning, we want to return an unhinted outline, so it
     *  can be positioned nicely at fractional offsets. However, we special-case
     *  if the baseline of the (horizontal) text is axis-aligned. In those cases
     *  we want to retain hinting in the direction orthogonal to the baseline.
     *  e.g. for horizontal baseline, we want to retain hinting in Y.
     *  The way we remove hinting is to scale the font by some value (4) in that
     *  direction, ask for the path, and then scale the path back down.
     */
    if (fDoSubPosition) {
        // start out by assuming that we want no hining in X and Y
        scaleX = scaleY = kScaleForSubPixelPositionHinting;
        // now see if we need to restore hinting for axis-aligned baselines
        switch (this->computeAxisAlignmentForHText()) {
            case kX_SkAxisAlignment:
                scaleY = SK_Scalar1; // want hinting in the Y direction
                break;
            case kY_SkAxisAlignment:
                scaleX = SK_Scalar1; // want hinting in the X direction
                break;
            default:
                break;
        }

        CGAffineTransform scale(CGAffineTransformMakeScale(SkScalarToCGFloat(scaleX),
                                                           SkScalarToCGFloat(scaleY)));
        xform = CGAffineTransformConcat(fTransform, scale);
    }

    CGGlyph cgGlyph = SkTo<CGGlyph>(glyph);
    SkUniqueCFRef<CGPathRef> cgPath(CTFontCreatePathForGlyph(fCTFont.get(), cgGlyph, &xform));

    path->reset();
    if (!cgPath) {
        return false;
    }

    SkCTPathGeometrySink sink;
    CGPathApply(cgPath.get(), &sink, SkCTPathGeometrySink::ApplyElement);
    *path = sink.detach();
    if (fDoSubPosition) {
        SkMatrix m;
        m.setScale(SkScalarInvert(scaleX), SkScalarInvert(scaleY));
        path->transform(m);
    }
    return true;
}

void SkScalerContext_Mac::generateFontMetrics(SkFontMetrics* metrics) {
    if (nullptr == metrics) {
        return;
    }

    CGRect theBounds = CTFontGetBoundingBox(fCTFont.get());

    metrics->fTop          = SkScalarFromCGFloat(-SkCGRectGetMaxY(theBounds));
    metrics->fAscent       = SkScalarFromCGFloat(-CTFontGetAscent(fCTFont.get()));
    metrics->fDescent      = SkScalarFromCGFloat( CTFontGetDescent(fCTFont.get()));
    metrics->fBottom       = SkScalarFromCGFloat(-SkCGRectGetMinY(theBounds));
    metrics->fLeading      = SkScalarFromCGFloat( CTFontGetLeading(fCTFont.get()));
    metrics->fAvgCharWidth = SkScalarFromCGFloat( SkCGRectGetWidth(theBounds));
    metrics->fXMin         = SkScalarFromCGFloat( SkCGRectGetMinX(theBounds));
    metrics->fXMax         = SkScalarFromCGFloat( SkCGRectGetMaxX(theBounds));
    metrics->fMaxCharWidth = metrics->fXMax - metrics->fXMin;
    metrics->fXHeight      = SkScalarFromCGFloat( CTFontGetXHeight(fCTFont.get()));
    metrics->fCapHeight    = SkScalarFromCGFloat( CTFontGetCapHeight(fCTFont.get()));
    metrics->fUnderlineThickness = SkScalarFromCGFloat( CTFontGetUnderlineThickness(fCTFont.get()));
    metrics->fUnderlinePosition = -SkScalarFromCGFloat( CTFontGetUnderlinePosition(fCTFont.get()));

    metrics->fFlags = 0;
    metrics->fFlags |= SkFontMetrics::kUnderlineThicknessIsValid_Flag;
    metrics->fFlags |= SkFontMetrics::kUnderlinePositionIsValid_Flag;

    SkUniqueCFRef<CFArrayRef> ctAxes(CTFontCopyVariationAxes(fCTFont.get()));
    if (ctAxes && CFArrayGetCount(ctAxes.get()) > 0) {
        // The bounds are only valid for the default variation.
        metrics->fFlags |= SkFontMetrics::kBoundsInvalid_Flag;
    }

    // See https://bugs.chromium.org/p/skia/issues/detail?id=6203
    // At least on 10.12.3 with memory based fonts the x-height is always 0.6666 of the ascent and
    // the cap-height is always 0.8888 of the ascent. It appears that the values from the 'OS/2'
    // table are read, but then overwritten if the font is not a system font. As a result, if there
    // is a valid 'OS/2' table available use the values from the table if they aren't too strange.
    struct OS2HeightMetrics {
        SK_OT_SHORT sxHeight;
        SK_OT_SHORT sCapHeight;
    } heights;
    size_t bytesRead = this->getTypeface()->getTableData(
            SkTEndian_SwapBE32(SkOTTableOS2::TAG), offsetof(SkOTTableOS2, version.v2.sxHeight),
            sizeof(heights), &heights);
    if (bytesRead == sizeof(heights)) {
        // 'fontSize' is correct because the entire resolved size is set by the constructor.
        CGFloat fontSize = CTFontGetSize(this->fCTFont.get());
        unsigned upem = CTFontGetUnitsPerEm(this->fCTFont.get());
        unsigned maxSaneHeight = upem * 2;
        uint16_t xHeight = SkEndian_SwapBE16(heights.sxHeight);
        if (xHeight && xHeight < maxSaneHeight) {
            metrics->fXHeight = SkScalarFromCGFloat(xHeight * fontSize / upem);
        }
        uint16_t capHeight = SkEndian_SwapBE16(heights.sCapHeight);
        if (capHeight && capHeight < maxSaneHeight) {
            metrics->fCapHeight = SkScalarFromCGFloat(capHeight * fontSize / upem);
        }
    }
}

#endif
