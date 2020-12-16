/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScalerContext_mac_ct_DEFINED
#define SkScalerContext_mac_ct_DEFINED

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkScalerContext.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreText/CoreText.h>
#include <CoreText/CTFontManager.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <memory>

class SkDescriptor;
class SkGlyph;
class SkPath;
class SkTypeface_Mac;
struct SkFontMetrics;


typedef uint32_t CGRGBPixel;

class SkScalerContext_Mac : public SkScalerContext {
public:
    SkScalerContext_Mac(sk_sp<SkTypeface_Mac>, const SkScalerContextEffects&, const SkDescriptor*);

protected:
    bool generateAdvance(SkGlyph* glyph) override;
    void generateMetrics(SkGlyph* glyph) override;
    void generateImage(const SkGlyph& glyph) override;
    bool generatePath(SkGlyphID glyph, SkPath* path) override;
    void generateFontMetrics(SkFontMetrics*) override;

private:
    class Offscreen {
    public:
        Offscreen()
            : fRGBSpace(nullptr)
            , fCG(nullptr)
            , fDoAA(false)
            , fDoLCD(false)
        {
            fSize.set(0, 0);
        }

        CGRGBPixel* getCG(const SkScalerContext_Mac& context, const SkGlyph& glyph,
                          CGGlyph glyphID, size_t* rowBytesPtr, bool generateA8FromLCD);

    private:
        enum {
            kSize = 32 * 32 * sizeof(CGRGBPixel)
        };
        SkAutoSMalloc<kSize> fImageStorage;
        SkUniqueCFRef<CGColorSpaceRef> fRGBSpace;

        // cached state
        SkUniqueCFRef<CGContextRef> fCG;
        SkISize fSize;
        bool fDoAA;
        bool fDoLCD;
    };
    Offscreen fOffscreen;

    /** Unrotated variant of fCTFont.
     *
     *  In 10.10.1 CTFontGetAdvancesForGlyphs applies the font transform to the width of the
     *  advances, but always sets the height to 0. This font is used to get the advances of the
     *  unrotated glyph, and then the rotation is applied separately.
     *
     *  CT vertical metrics are pre-rotated (in em space, before transform) 90deg clock-wise.
     *  This makes kCTFontOrientationDefault dangerous, because the metrics from
     *  kCTFontOrientationHorizontal are in a different space from kCTFontOrientationVertical.
     *  With kCTFontOrientationVertical the advances must be unrotated.
     *
     *  Sometimes, creating a copy of a CTFont with the same size but different trasform will select
     *  different underlying font data. As a result, avoid ever creating more than one CTFont per
     *  SkScalerContext to ensure that only one CTFont is used.
     *
     *  As a result of the above (and other constraints) this font contains the size, but not the
     *  transform. The transform must always be applied separately.
     */
    SkUniqueCFRef<CTFontRef> fCTFont;

    /** The transform without the font size. */
    CGAffineTransform fTransform;
    CGAffineTransform fInvTransform;

    SkUniqueCFRef<CGFontRef> fCGFont;
    const bool fDoSubPosition;

    friend class Offscreen;

    using INHERITED = SkScalerContext;
};

#endif
#endif //SkScalerContext_mac_ct_DEFINED
