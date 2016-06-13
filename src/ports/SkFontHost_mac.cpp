/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"  // Keep this before any #ifdef ...
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

#include "SkAdvancedTypefaceMetrics.h"
#include "SkCGUtils.h"
#include "SkColorPriv.h"
#include "SkDescriptor.h"
#include "SkEndian.h"
#include "SkFloatingPoint.h"
#include "SkFontDescriptor.h"
#include "SkFontMgr.h"
#include "SkGlyph.h"
#include "SkMaskGamma.h"
#include "SkMathPriv.h"
#include "SkMutex.h"
#include "SkOTTable_glyf.h"
#include "SkOTTable_head.h"
#include "SkOTTable_hhea.h"
#include "SkOTTable_loca.h"
#include "SkOTUtils.h"
#include "SkOnce.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkSFNTHeader.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "SkTypefaceCache.h"
#include "SkTypeface_mac.h"
#include "SkUtils.h"
#include "SkUtils.h"

#include <dlfcn.h>

// Experimental code to use a global lock whenever we access CG, to see if this reduces
// crashes in Chrome
#define USE_GLOBAL_MUTEX_FOR_CG_ACCESS

#ifdef USE_GLOBAL_MUTEX_FOR_CG_ACCESS
    SK_DECLARE_STATIC_MUTEX(gCGMutex);
    #define AUTO_CG_LOCK()  SkAutoMutexAcquire amc(gCGMutex)
#else
    #define AUTO_CG_LOCK()
#endif

// Set to make glyph bounding boxes visible.
#define SK_SHOW_TEXT_BLIT_COVERAGE 0

class SkScalerContext_Mac;

// CTFontManagerCopyAvailableFontFamilyNames() is not always available, so we
// provide a wrapper here that will return an empty array if need be.
static CFArrayRef SkCTFontManagerCopyAvailableFontFamilyNames() {
#ifdef SK_BUILD_FOR_IOS
    return CFArrayCreate(nullptr, nullptr, 0, nullptr);
#else
    return CTFontManagerCopyAvailableFontFamilyNames();
#endif
}


// Being templated and taking const T* prevents calling
// CFSafeRelease(autoCFRelease) through implicit conversion.
template <typename T> static void CFSafeRelease(/*CFTypeRef*/const T* cfTypeRef) {
    if (cfTypeRef) {
        CFRelease(cfTypeRef);
    }
}

// Being templated and taking const T* prevents calling
// CFSafeRetain(autoCFRelease) through implicit conversion.
template <typename T> static void CFSafeRetain(/*CFTypeRef*/const T* cfTypeRef) {
    if (cfTypeRef) {
        CFRetain(cfTypeRef);
    }
}

/** Acts like a CFRef, but calls CFSafeRelease when it goes out of scope. */
template<typename CFRef> class AutoCFRelease : private SkNoncopyable {
public:
    explicit AutoCFRelease(CFRef cfRef = nullptr) : fCFRef(cfRef) { }
    ~AutoCFRelease() { CFSafeRelease(fCFRef); }

    void reset(CFRef that = nullptr) {
        if (that != fCFRef) {
            CFSafeRelease(fCFRef);
            fCFRef = that;
        }
    }

    CFRef release() {
        CFRef self = fCFRef;
        fCFRef = nullptr;
        return self;
    }

    operator CFRef() const { return fCFRef; }
    CFRef get() const { return fCFRef; }

    CFRef* operator&() { SkASSERT(fCFRef == nullptr); return &fCFRef; }
private:
    CFRef fCFRef;
};

static CFStringRef make_CFString(const char str[]) {
    return CFStringCreateWithCString(nullptr, str, kCFStringEncodingUTF8);
}

template<typename T> class AutoCGTable : SkNoncopyable {
public:
    AutoCGTable(CGFontRef font)
    //Undocumented: the tag parameter in this call is expected in machine order and not BE order.
    : fCFData(CGFontCopyTableForTag(font, SkSetFourByteTag(T::TAG0, T::TAG1, T::TAG2, T::TAG3)))
    , fData(fCFData ? reinterpret_cast<const T*>(CFDataGetBytePtr(fCFData)) : nullptr)
    { }

    const T* operator->() const { return fData; }

private:
    AutoCFRelease<CFDataRef> fCFData;
public:
    const T* fData;
};

// inline versions of these rect helpers

static bool CGRectIsEmpty_inline(const CGRect& rect) {
    return rect.size.width <= 0 || rect.size.height <= 0;
}

static CGFloat CGRectGetMinX_inline(const CGRect& rect) {
    return rect.origin.x;
}

static CGFloat CGRectGetMaxX_inline(const CGRect& rect) {
    return rect.origin.x + rect.size.width;
}

static CGFloat CGRectGetMinY_inline(const CGRect& rect) {
    return rect.origin.y;
}

static CGFloat CGRectGetMaxY_inline(const CGRect& rect) {
    return rect.origin.y + rect.size.height;
}

static CGFloat CGRectGetWidth_inline(const CGRect& rect) {
    return rect.size.width;
}

///////////////////////////////////////////////////////////////////////////////

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

#include <sys/utsname.h>

typedef uint32_t CGRGBPixel;

static unsigned CGRGBPixel_getAlpha(CGRGBPixel pixel) {
    return pixel & 0xFF;
}

static const char FONT_DEFAULT_NAME[] = "Lucida Sans";

// See Source/WebKit/chromium/base/mac/mac_util.mm DarwinMajorVersionInternal for original source.
static int readVersion() {
    struct utsname info;
    if (uname(&info) != 0) {
        SkDebugf("uname failed\n");
        return 0;
    }
    if (strcmp(info.sysname, "Darwin") != 0) {
        SkDebugf("unexpected uname sysname %s\n", info.sysname);
        return 0;
    }
    char* dot = strchr(info.release, '.');
    if (!dot) {
        SkDebugf("expected dot in uname release %s\n", info.release);
        return 0;
    }
    int version = atoi(info.release);
    if (version == 0) {
        SkDebugf("could not parse uname release %s\n", info.release);
    }
    return version;
}

static int darwinVersion() {
    static int darwin_version = readVersion();
    return darwin_version;
}

static bool isSnowLeopard() {
    return darwinVersion() == 10;
}

static bool isLion() {
    return darwinVersion() == 11;
}

static bool isMountainLion() {
    return darwinVersion() == 12;
}

static bool isLCDFormat(unsigned format) {
    return SkMask::kLCD16_Format == format;
}

static CGFloat ScalarToCG(SkScalar scalar) {
    if (sizeof(CGFloat) == sizeof(float)) {
        return SkScalarToFloat(scalar);
    } else {
        SkASSERT(sizeof(CGFloat) == sizeof(double));
        return (CGFloat) SkScalarToDouble(scalar);
    }
}

static SkScalar CGToScalar(CGFloat cgFloat) {
    if (sizeof(CGFloat) == sizeof(float)) {
        return SkFloatToScalar(cgFloat);
    } else {
        SkASSERT(sizeof(CGFloat) == sizeof(double));
        return SkDoubleToScalar(cgFloat);
    }
}

static float CGToFloat(CGFloat cgFloat) {
    if (sizeof(CGFloat) == sizeof(float)) {
        return cgFloat;
    } else {
        SkASSERT(sizeof(CGFloat) == sizeof(double));
        return static_cast<float>(cgFloat);
    }
}

static CGAffineTransform MatrixToCGAffineTransform(const SkMatrix& matrix,
                                                   SkScalar sx = SK_Scalar1,
                                                   SkScalar sy = SK_Scalar1) {
    return CGAffineTransformMake( ScalarToCG(matrix[SkMatrix::kMScaleX] * sx),
                                 -ScalarToCG(matrix[SkMatrix::kMSkewY]  * sy),
                                 -ScalarToCG(matrix[SkMatrix::kMSkewX]  * sx),
                                  ScalarToCG(matrix[SkMatrix::kMScaleY] * sy),
                                  ScalarToCG(matrix[SkMatrix::kMTransX] * sx),
                                  ScalarToCG(matrix[SkMatrix::kMTransY] * sy));
}

///////////////////////////////////////////////////////////////////////////////

#define BITMAP_INFO_RGB (kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Host)

/**
 * There does not appear to be a publicly accessable API for determining if lcd
 * font smoothing will be applied if we request it. The main issue is that if
 * smoothing is applied a gamma of 2.0 will be used, if not a gamma of 1.0.
 */
static bool supports_LCD() {
    static int gSupportsLCD = -1;
    if (gSupportsLCD >= 0) {
        return (bool) gSupportsLCD;
    }
    uint32_t rgb = 0;
    AutoCFRelease<CGColorSpaceRef> colorspace(CGColorSpaceCreateDeviceRGB());
    AutoCFRelease<CGContextRef> cgContext(CGBitmapContextCreate(&rgb, 1, 1, 8, 4,
                                                                colorspace, BITMAP_INFO_RGB));
    CGContextSelectFont(cgContext, "Helvetica", 16, kCGEncodingMacRoman);
    CGContextSetShouldSmoothFonts(cgContext, true);
    CGContextSetShouldAntialias(cgContext, true);
    CGContextSetTextDrawingMode(cgContext, kCGTextFill);
    CGContextSetGrayFillColor(cgContext, 1, 1);
    CGContextShowTextAtPoint(cgContext, -1, 0, "|", 1);
    uint32_t r = (rgb >> 16) & 0xFF;
    uint32_t g = (rgb >>  8) & 0xFF;
    uint32_t b = (rgb >>  0) & 0xFF;
    gSupportsLCD = (r != g || r != b);
    return (bool) gSupportsLCD;
}

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
                      CGGlyph glyphID, size_t* rowBytesPtr,
                      bool generateA8FromLCD);

private:
    enum {
        kSize = 32 * 32 * sizeof(CGRGBPixel)
    };
    SkAutoSMalloc<kSize> fImageStorage;
    AutoCFRelease<CGColorSpaceRef> fRGBSpace;

    // cached state
    AutoCFRelease<CGContextRef> fCG;
    SkISize fSize;
    bool fDoAA;
    bool fDoLCD;

    static int RoundSize(int dimension) {
        return SkNextPow2(dimension);
    }
};

///////////////////////////////////////////////////////////////////////////////

static bool find_dict_float(CFDictionaryRef dict, CFStringRef name, float* value) {
    CFNumberRef num;
    return CFDictionaryGetValueIfPresent(dict, name, (const void**)&num)
    && CFNumberIsFloatType(num)
    && CFNumberGetValue(num, kCFNumberFloatType, value);
}

static int unit_weight_to_fontstyle(float unit) {
    float value;
    if (unit < 0) {
        value = 100 + (1 + unit) * 300;
    } else {
        value = 400 + unit * 500;
    }
    return sk_float_round2int(value);
}

static int unit_width_to_fontstyle(float unit) {
    float value;
    if (unit < 0) {
        value = 1 + (1 + unit) * 4;
    } else {
        value = 5 + unit * 4;
    }
    return sk_float_round2int(value);
}

static SkFontStyle fontstyle_from_descriptor(CTFontDescriptorRef desc) {
    AutoCFRelease<CFDictionaryRef> dict(
            (CFDictionaryRef)CTFontDescriptorCopyAttribute(desc, kCTFontTraitsAttribute));
    if (nullptr == dict.get()) {
        return SkFontStyle();
    }

    float weight, width, slant;
    if (!find_dict_float(dict, kCTFontWeightTrait, &weight)) {
        weight = 0;
    }
    if (!find_dict_float(dict, kCTFontWidthTrait, &width)) {
        width = 0;
    }
    if (!find_dict_float(dict, kCTFontSlantTrait, &slant)) {
        slant = 0;
    }

    return SkFontStyle(unit_weight_to_fontstyle(weight),
                       unit_width_to_fontstyle(width),
                       slant ? SkFontStyle::kItalic_Slant
                             : SkFontStyle::kUpright_Slant);
}

#define WEIGHT_THRESHOLD    ((SkFontStyle::kNormal_Weight + SkFontStyle::kBold_Weight)/2)

// kCTFontColorGlyphsTrait was added in the Mac 10.7 and iPhone 4.3 SDKs.
// Being an enum value it is not guarded by version macros, but old SDKs must still be supported.
#if defined(__MAC_10_7) || defined(__IPHONE_4_3)
static const uint32_t SkCTFontColorGlyphsTrait = kCTFontColorGlyphsTrait;
#else
static const uint32_t SkCTFontColorGlyphsTrait = (1 << 13);
#endif

class SkTypeface_Mac : public SkTypeface {
public:
    SkTypeface_Mac(CTFontRef fontRef, CFTypeRef resourceRef,
                   const SkFontStyle& fs, bool isFixedPitch,
                   bool isLocalStream)
        : SkTypeface(fs, SkTypefaceCache::NewFontID(), isFixedPitch)
        , fFontRef(fontRef) // caller has already called CFRetain for us
        , fOriginatingCFTypeRef(resourceRef) // caller has already called CFRetain for us
        , fHasColorGlyphs(SkToBool(CTFontGetSymbolicTraits(fFontRef) & SkCTFontColorGlyphsTrait))
        , fIsLocalStream(isLocalStream)
    {
        SkASSERT(fontRef);
    }

    AutoCFRelease<CTFontRef> fFontRef;
    AutoCFRelease<CFTypeRef> fOriginatingCFTypeRef;
    const bool fHasColorGlyphs;

protected:
    int onGetUPEM() const override;
    SkStreamAsset* onOpenStream(int* ttcIndex) const override;
    SkFontData* onCreateFontData() const override;
    void onGetFamilyName(SkString* familyName) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;
    int onGetTableTags(SkFontTableTag tags[]) const override;
    virtual size_t onGetTableData(SkFontTableTag, size_t offset,
                                  size_t length, void* data) const override;
    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                           const SkDescriptor*) const override;
    void onFilterRec(SkScalerContextRec*) const override;
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override;
    virtual SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
                                PerGlyphInfo,
                                const uint32_t*, uint32_t) const override;
    virtual int onCharsToGlyphs(const void* chars, Encoding, uint16_t glyphs[],
                                int glyphCount) const override;
    int onCountGlyphs() const override;

private:
    bool fIsLocalStream;

    typedef SkTypeface INHERITED;
};

/** Creates a typeface without searching the cache. Takes ownership of the CTFontRef. */
static SkTypeface* NewFromFontRef(CTFontRef fontRef, CFTypeRef resourceRef, bool isLocalStream) {
    SkASSERT(fontRef);

    AutoCFRelease<CTFontDescriptorRef> desc(CTFontCopyFontDescriptor(fontRef));
    SkFontStyle style = fontstyle_from_descriptor(desc);

    CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(fontRef);
    bool isFixedPitch = SkToBool(traits & kCTFontMonoSpaceTrait);
    return new SkTypeface_Mac(fontRef, resourceRef, style, isFixedPitch, isLocalStream);
}

static bool find_by_CTFontRef(SkTypeface* cached, void* context) {
    CTFontRef self = (CTFontRef)context;
    CTFontRef other = ((SkTypeface_Mac*)cached)->fFontRef;

    return CFEqual(self, other);
}

/** Creates a typeface from a name, searching the cache. */
static SkTypeface* NewFromName(const char familyName[], const SkFontStyle& theStyle) {
    CTFontSymbolicTraits ctFontTraits = 0;
    if (theStyle.weight() >= SkFontStyle::kBold_Weight) {
        ctFontTraits |= kCTFontBoldTrait;
    }
    if (theStyle.slant() != SkFontStyle::kUpright_Slant) {
        ctFontTraits |= kCTFontItalicTrait;
    }

    //TODO: add weight width slant

    // Create the font info
    AutoCFRelease<CFStringRef> cfFontName(make_CFString(familyName));

    AutoCFRelease<CFNumberRef> cfFontTraits(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &ctFontTraits));

    AutoCFRelease<CFMutableDictionaryRef> cfAttributes(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));

    AutoCFRelease<CFMutableDictionaryRef> cfTraits(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));

    if (!cfFontName || !cfFontTraits || !cfAttributes || !cfTraits) {
        return nullptr;
    }

    CFDictionaryAddValue(cfTraits, kCTFontSymbolicTrait, cfFontTraits);

    CFDictionaryAddValue(cfAttributes, kCTFontFamilyNameAttribute, cfFontName);
    CFDictionaryAddValue(cfAttributes, kCTFontTraitsAttribute, cfTraits);

    AutoCFRelease<CTFontDescriptorRef> ctFontDesc(
            CTFontDescriptorCreateWithAttributes(cfAttributes));
    if (!ctFontDesc) {
        return nullptr;
    }

    AutoCFRelease<CTFontRef> ctFont(CTFontCreateWithFontDescriptor(ctFontDesc, 0, nullptr));
    if (!ctFont) {
        return nullptr;
    }

    SkTypeface* face = SkTypefaceCache::FindByProcAndRef(find_by_CTFontRef, (void*)ctFont.get());
    if (face) {
        return face;
    }
    face = NewFromFontRef(ctFont.release(), nullptr, false);
    SkTypefaceCache::Add(face);
    return face;
}

SK_DECLARE_STATIC_MUTEX(gGetDefaultFaceMutex);
static SkTypeface* GetDefaultFace() {
    SkAutoMutexAcquire ma(gGetDefaultFaceMutex);

    static SkTypeface* gDefaultFace;

    if (nullptr == gDefaultFace) {
        gDefaultFace = NewFromName(FONT_DEFAULT_NAME, SkFontStyle());
    }
    return gDefaultFace;
}

///////////////////////////////////////////////////////////////////////////////

extern CTFontRef SkTypeface_GetCTFontRef(const SkTypeface* face);
CTFontRef SkTypeface_GetCTFontRef(const SkTypeface* face) {
    const SkTypeface_Mac* macface = (const SkTypeface_Mac*)face;
    return macface ? macface->fFontRef.get() : nullptr;
}

/*  This function is visible on the outside. It first searches the cache, and if
 *  not found, returns a new entry (after adding it to the cache).
 */
SkTypeface* SkCreateTypefaceFromCTFont(CTFontRef fontRef, CFTypeRef resourceRef) {
    SkTypeface* face = SkTypefaceCache::FindByProcAndRef(find_by_CTFontRef, (void*)fontRef);
    if (face) {
        return face;
    }
    CFRetain(fontRef);
    if (resourceRef) {
        CFRetain(resourceRef);
    }
    face = NewFromFontRef(fontRef, resourceRef, false);
    SkTypefaceCache::Add(face);
    return face;
}

static const char* map_css_names(const char* name) {
    static const struct {
        const char* fFrom;  // name the caller specified
        const char* fTo;    // "canonical" name we map to
    } gPairs[] = {
        { "sans-serif", "Helvetica" },
        { "serif",      "Times"     },
        { "monospace",  "Courier"   }
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gPairs); i++) {
        if (strcmp(name, gPairs[i].fFrom) == 0) {
            return gPairs[i].fTo;
        }
    }
    return name;    // no change
}

///////////////////////////////////////////////////////////////////////////////

/** GlyphRect is in FUnits (em space, y up). */
struct GlyphRect {
    int16_t fMinX;
    int16_t fMinY;
    int16_t fMaxX;
    int16_t fMaxY;
};

class SkScalerContext_Mac : public SkScalerContext {
public:
    SkScalerContext_Mac(SkTypeface_Mac*, const SkScalerContextEffects&, const SkDescriptor*);

protected:
    unsigned generateGlyphCount(void) override;
    uint16_t generateCharToGlyph(SkUnichar uni) override;
    void generateAdvance(SkGlyph* glyph) override;
    void generateMetrics(SkGlyph* glyph) override;
    void generateImage(const SkGlyph& glyph) override;
    void generatePath(const SkGlyph& glyph, SkPath* path) override;
    void generateFontMetrics(SkPaint::FontMetrics*) override;

private:
    static void CTPathElement(void *info, const CGPathElement *element);

    /** Returns the offset from the horizontal origin to the vertical origin in SkGlyph units. */
    void getVerticalOffset(CGGlyph glyphID, SkPoint* offset) const;

    /** Initializes and returns the value of fFBoundingBoxesGlyphOffset.
     *
     *  For use with (and must be called before) generateBBoxes.
     */
    uint16_t getFBoundingBoxesGlyphOffset();

    /** Initializes fFBoundingBoxes and returns true on success.
     *
     *  On Lion and Mountain Lion, CTFontGetBoundingRectsForGlyphs has a bug which causes it to
     *  return a bad value in bounds.origin.x for SFNT fonts whose hhea::numberOfHMetrics is
     *  less than its maxp::numGlyphs. When this is the case we try to read the bounds from the
     *  font directly.
     *
     *  This routine initializes fFBoundingBoxes to an array of
     *  fGlyphCount - fFBoundingBoxesGlyphOffset GlyphRects which contain the bounds in FUnits
     *  (em space, y up) of glyphs with ids in the range [fFBoundingBoxesGlyphOffset, fGlyphCount).
     *
     *  Returns true if fFBoundingBoxes is properly initialized. The table can only be properly
     *  initialized for a TrueType font with 'head', 'loca', and 'glyf' tables.
     *
     *  TODO: A future optimization will compute fFBoundingBoxes once per fCTFont.
     */
    bool generateBBoxes();

    /** Converts from FUnits (em space, y up) to SkGlyph units (pixels, y down).
     *
     *  Used on Snow Leopard to correct CTFontGetVerticalTranslationsForGlyphs.
     *  Used on Lion to correct CTFontGetBoundingRectsForGlyphs.
     */
    SkMatrix fFUnitMatrix;

    Offscreen fOffscreen;

    /** Unrotated variant of fCTFont.
     *
     *  In 10.10.1 CTFontGetAdvancesForGlyphs applies the font transform to the width of the
     *  advances, but always sets the height to 0. This font is used to get the advances of the
     *  unrotated glyph, and then the rotation is applied separately.
     *
     *  CT vertical metrics are pre-rotated (in em space, before transform) 90deg clock-wise.
     *  This makes kCTFontDefaultOrientation dangerous, because the metrics from
     *  kCTFontHorizontalOrientation are in a different space from kCTFontVerticalOrientation.
     *  With kCTFontVerticalOrientation the advances must be unrotated.
     *
     *  Sometimes, creating a copy of a CTFont with the same size but different trasform will select
     *  different underlying font data. As a result, avoid ever creating more than one CTFont per
     *  SkScalerContext to ensure that only one CTFont is used.
     *
     *  As a result of the above (and other constraints) this font contains the size, but not the
     *  transform. The transform must always be applied separately.
     */
    AutoCFRelease<CTFontRef> fCTFont;

    /** The transform without the font size. */
    CGAffineTransform fTransform;
    CGAffineTransform fInvTransform;

    AutoCFRelease<CGFontRef> fCGFont;
    SkAutoTMalloc<GlyphRect> fFBoundingBoxes;
    uint16_t fFBoundingBoxesGlyphOffset;
    uint16_t fGlyphCount;
    bool fGeneratedFBoundingBoxes;
    const bool fDoSubPosition;
    const bool fVertical;

    friend class Offscreen;

    typedef SkScalerContext INHERITED;
};

// CTFontCreateCopyWithAttributes or CTFontCreateCopyWithSymbolicTraits cannot be used on 10.10
// and later, as they will return different underlying fonts depending on the size requested.
// It is not possible to use descriptors with CTFontCreateWithFontDescriptor, since that does not
// work with non-system fonts. As a result, create the strike specific CTFonts from the underlying
// CGFont.
static CTFontRef ctfont_create_exact_copy(CTFontRef baseFont, CGFloat textSize,
                                          const CGAffineTransform* transform)
{
    AutoCFRelease<CGFontRef> baseCGFont(CTFontCopyGraphicsFont(baseFont, nullptr));

    // The last parameter (CTFontDescriptorRef attributes) *must* be nullptr.
    // If non-nullptr then with fonts with variation axes, the copy will fail in
    // CGFontVariationFromDictCallback when it assumes kCGFontVariationAxisName is CFNumberRef
    // which it quite obviously is not.

    // Because we cannot setup the CTFont descriptor to match, the same restriction applies here
    // as other uses of CTFontCreateWithGraphicsFont which is that such CTFonts should not escape
    // the scaler context, since they aren't 'normal'.
    return CTFontCreateWithGraphicsFont(baseCGFont, textSize, transform, nullptr);
}

SkScalerContext_Mac::SkScalerContext_Mac(SkTypeface_Mac* typeface,
                                         const SkScalerContextEffects& effects,
                                         const SkDescriptor* desc)
        : INHERITED(typeface, effects, desc)
        , fFBoundingBoxes()
        , fFBoundingBoxesGlyphOffset(0)
        , fGeneratedFBoundingBoxes(false)
        , fDoSubPosition(SkToBool(fRec.fFlags & kSubpixelPositioning_Flag))
        , fVertical(SkToBool(fRec.fFlags & kVertical_Flag))

{
    AUTO_CG_LOCK();

    CTFontRef ctFont = typeface->fFontRef.get();
    CFIndex numGlyphs = CTFontGetGlyphCount(ctFont);
    SkASSERT(numGlyphs >= 1 && numGlyphs <= 0xFFFF);
    fGlyphCount = SkToU16(numGlyphs);

    // CT on (at least) 10.9 will size color glyphs down from the requested size, but not up.
    // As a result, it is necessary to know the actual device size and request that.
    SkVector scale;
    SkMatrix skTransform;
    fRec.computeMatrices(SkScalerContextRec::kVertical_PreMatrixScale, &scale, &skTransform,
                         nullptr, nullptr, &fFUnitMatrix);
    fTransform = MatrixToCGAffineTransform(skTransform);
    fInvTransform = CGAffineTransformInvert(fTransform);

    // The transform contains everything except the requested text size.
    // Some properties, like 'trak', are based on the text size (before applying the matrix).
    CGFloat textSize = ScalarToCG(scale.y());
    fCTFont.reset(ctfont_create_exact_copy(ctFont, textSize, nullptr));
    fCGFont.reset(CTFontCopyGraphicsFont(fCTFont, nullptr));

    // The fUnitMatrix includes the text size (and em) as it is used to scale the raw font data.
    SkScalar emPerFUnit = SkScalarInvert(SkIntToScalar(CGFontGetUnitsPerEm(fCGFont)));
    fFUnitMatrix.preScale(emPerFUnit, -emPerFUnit);
}

/** This is an implementation of CTFontDrawGlyphs for 10.6; it was introduced in 10.7. */
static void legacy_CTFontDrawGlyphs(CTFontRef, const CGGlyph glyphs[], const CGPoint points[],
                                    size_t count, CGContextRef cg) {
    CGContextShowGlyphsAtPositions(cg, glyphs, points, count);
}

typedef decltype(legacy_CTFontDrawGlyphs) CTFontDrawGlyphsProc;

static CTFontDrawGlyphsProc* choose_CTFontDrawGlyphs() {
    if (void* real = dlsym(RTLD_DEFAULT, "CTFontDrawGlyphs")) {
        return (CTFontDrawGlyphsProc*)real;
    }
    return &legacy_CTFontDrawGlyphs;
}

CGRGBPixel* Offscreen::getCG(const SkScalerContext_Mac& context, const SkGlyph& glyph,
                             CGGlyph glyphID, size_t* rowBytesPtr,
                             bool generateA8FromLCD) {
    static SkOnce once;
    static CTFontDrawGlyphsProc* ctFontDrawGlyphs;
    once([]{ ctFontDrawGlyphs = choose_CTFontDrawGlyphs(); });

    if (!fRGBSpace) {
        //It doesn't appear to matter what color space is specified.
        //Regular blends and antialiased text are always (s*a + d*(1-a))
        //and smoothed text is always g=2.0.
        fRGBSpace.reset(CGColorSpaceCreateDeviceRGB());
    }

    // default to kBW_Format
    bool doAA = false;
    bool doLCD = false;

    if (SkMask::kBW_Format != glyph.fMaskFormat) {
        doLCD = true;
        doAA = true;
    }

    // FIXME: lcd smoothed un-hinted rasterization unsupported.
    if (!generateA8FromLCD && SkMask::kA8_Format == glyph.fMaskFormat) {
        doLCD = false;
        doAA = true;
    }

    // If this font might have color glyphs, disable LCD as there's no way to support it.
    // CoreText doesn't tell us which format it ended up using, so we can't detect it.
    // A8 will end up black on transparent, but TODO: we can detect gray and set to A8.
    if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
        doLCD = false;
    }

    size_t rowBytes = fSize.fWidth * sizeof(CGRGBPixel);
    if (!fCG || fSize.fWidth < glyph.fWidth || fSize.fHeight < glyph.fHeight) {
        if (fSize.fWidth < glyph.fWidth) {
            fSize.fWidth = RoundSize(glyph.fWidth);
        }
        if (fSize.fHeight < glyph.fHeight) {
            fSize.fHeight = RoundSize(glyph.fHeight);
        }

        rowBytes = fSize.fWidth * sizeof(CGRGBPixel);
        void* image = fImageStorage.reset(rowBytes * fSize.fHeight);
        const CGImageAlphaInfo alpha = (SkMask::kARGB32_Format == glyph.fMaskFormat)
                                     ? kCGImageAlphaPremultipliedFirst
                                     : kCGImageAlphaNoneSkipFirst;
        const CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Host | alpha;
        fCG.reset(CGBitmapContextCreate(image, fSize.fWidth, fSize.fHeight, 8,
                                        rowBytes, fRGBSpace, bitmapInfo));

        // Skia handles quantization and subpixel positioning,
        // so disable quantization and enabe subpixel positioning in CG.
        CGContextSetAllowsFontSubpixelQuantization(fCG, false);
        CGContextSetShouldSubpixelQuantizeFonts(fCG, false);

        // Because CG always draws from the horizontal baseline,
        // if there is a non-integral translation from the horizontal origin to the vertical origin,
        // then CG cannot draw the glyph in the correct location without subpixel positioning.
        CGContextSetAllowsFontSubpixelPositioning(fCG, true);
        CGContextSetShouldSubpixelPositionFonts(fCG, true);

        CGContextSetTextDrawingMode(fCG, kCGTextFill);

        // Draw black on white to create mask. (Special path exists to speed this up in CG.)
        CGContextSetGrayFillColor(fCG, 0.0f, 1.0f);

        // force our checks below to happen
        fDoAA = !doAA;
        fDoLCD = !doLCD;

        if (legacy_CTFontDrawGlyphs == ctFontDrawGlyphs) {
            // CTFontDrawGlyphs will apply the font, font size, and font matrix to the CGContext.
            // Our 'fake' one does not, so set up the CGContext here.
            CGContextSetFont(fCG, context.fCGFont);
            CGContextSetFontSize(fCG, CTFontGetSize(context.fCTFont));
        }
        CGContextSetTextMatrix(fCG, context.fTransform);
    }

    if (fDoAA != doAA) {
        CGContextSetShouldAntialias(fCG, doAA);
        fDoAA = doAA;
    }
    if (fDoLCD != doLCD) {
        CGContextSetShouldSmoothFonts(fCG, doLCD);
        fDoLCD = doLCD;
    }

    CGRGBPixel* image = (CGRGBPixel*)fImageStorage.get();
    // skip rows based on the glyph's height
    image += (fSize.fHeight - glyph.fHeight) * fSize.fWidth;

    // Erase to white (or transparent black if it's a color glyph, to not composite against white).
    uint32_t bgColor = (SkMask::kARGB32_Format != glyph.fMaskFormat) ? 0xFFFFFFFF : 0x00000000;
    sk_memset_rect32(image, bgColor, glyph.fWidth, glyph.fHeight, rowBytes);

    float subX = 0;
    float subY = 0;
    if (context.fDoSubPosition) {
        subX = SkFixedToFloat(glyph.getSubXFixed());
        subY = SkFixedToFloat(glyph.getSubYFixed());
    }

    // CoreText and CoreGraphics always draw using the horizontal baseline origin.
    if (context.fVertical) {
        SkPoint offset;
        context.getVerticalOffset(glyphID, &offset);
        subX += offset.fX;
        subY += offset.fY;
    }

    CGPoint point = CGPointMake(-glyph.fLeft + subX, glyph.fTop + glyph.fHeight - subY);
    // Prior to 10.10, CTFontDrawGlyphs acted like CGContextShowGlyphsAtPositions and took
    // 'positions' which are in text space. The glyph location (in device space) must be
    // mapped into text space, so that CG can convert it back into device space.
    // In 10.10.1, this is handled directly in CTFontDrawGlyphs.
    //
    // However, in 10.10.2 color glyphs no longer rotate based on the font transform.
    // So always make the font transform identity and place the transform on the context.
    point = CGPointApplyAffineTransform(point, context.fInvTransform);

    ctFontDrawGlyphs(context.fCTFont, &glyphID, &point, 1, fCG);

    SkASSERT(rowBytesPtr);
    *rowBytesPtr = rowBytes;
    return image;
}

void SkScalerContext_Mac::getVerticalOffset(CGGlyph glyphID, SkPoint* offset) const {
    // Snow Leopard returns cgVertOffset in completely un-transformed FUnits (em space, y up).
    // Lion and Leopard return cgVertOffset in CG units (pixels, y up).
    CGSize cgVertOffset;
    CTFontGetVerticalTranslationsForGlyphs(fCTFont, &glyphID, &cgVertOffset, 1);
    if (isSnowLeopard()) {
        SkPoint skVertOffset = { CGToScalar(cgVertOffset.width), CGToScalar(cgVertOffset.height) };
        // From FUnits (em space, y up) to SkGlyph units (pixels, y down).
        fFUnitMatrix.mapPoints(&skVertOffset, 1);
        *offset = skVertOffset;
        return;
    }
    cgVertOffset = CGSizeApplyAffineTransform(cgVertOffset, fTransform);
    SkPoint skVertOffset = { CGToScalar(cgVertOffset.width), CGToScalar(cgVertOffset.height) };
    // From CG units (pixels, y up) to SkGlyph units (pixels, y down).
    skVertOffset.fY = -skVertOffset.fY;
    *offset = skVertOffset;
}

uint16_t SkScalerContext_Mac::getFBoundingBoxesGlyphOffset() {
    if (fFBoundingBoxesGlyphOffset) {
        return fFBoundingBoxesGlyphOffset;
    }
    fFBoundingBoxesGlyphOffset = fGlyphCount; // fallback for all fonts
    AutoCGTable<SkOTTableHorizontalHeader> hheaTable(fCGFont);
    if (hheaTable.fData) {
        fFBoundingBoxesGlyphOffset = SkEndian_SwapBE16(hheaTable->numberOfHMetrics);
    }
    return fFBoundingBoxesGlyphOffset;
}

bool SkScalerContext_Mac::generateBBoxes() {
    if (fGeneratedFBoundingBoxes) {
        return SkToBool(fFBoundingBoxes.get());
    }
    fGeneratedFBoundingBoxes = true;

    AutoCGTable<SkOTTableHead> headTable(fCGFont);
    if (!headTable.fData) {
        return false;
    }

    AutoCGTable<SkOTTableIndexToLocation> locaTable(fCGFont);
    if (!locaTable.fData) {
        return false;
    }

    AutoCGTable<SkOTTableGlyph> glyfTable(fCGFont);
    if (!glyfTable.fData) {
        return false;
    }

    uint16_t entries = fGlyphCount - fFBoundingBoxesGlyphOffset;
    fFBoundingBoxes.reset(entries);

    SkOTTableHead::IndexToLocFormat locaFormat = headTable->indexToLocFormat;
    SkOTTableGlyph::Iterator glyphDataIter(*glyfTable.fData, *locaTable.fData, locaFormat);
    glyphDataIter.advance(fFBoundingBoxesGlyphOffset);
    for (uint16_t boundingBoxesIndex = 0; boundingBoxesIndex < entries; ++boundingBoxesIndex) {
        const SkOTTableGlyphData* glyphData = glyphDataIter.next();
        GlyphRect& rect = fFBoundingBoxes[boundingBoxesIndex];
        rect.fMinX = SkEndian_SwapBE16(glyphData->xMin);
        rect.fMinY = SkEndian_SwapBE16(glyphData->yMin);
        rect.fMaxX = SkEndian_SwapBE16(glyphData->xMax);
        rect.fMaxY = SkEndian_SwapBE16(glyphData->yMax);
    }

    return true;
}

unsigned SkScalerContext_Mac::generateGlyphCount(void) {
    return fGlyphCount;
}

uint16_t SkScalerContext_Mac::generateCharToGlyph(SkUnichar uni) {
    AUTO_CG_LOCK();

    CGGlyph cgGlyph[2];
    UniChar theChar[2]; // UniChar is a UTF-16 16-bit code unit.

    // Get the glyph
    size_t numUniChar = SkUTF16_FromUnichar(uni, theChar);
    SkASSERT(sizeof(CGGlyph) <= sizeof(uint16_t));

    // Undocumented behavior of CTFontGetGlyphsForCharacters with non-bmp code points:
    // When a surrogate pair is detected, the glyph index used is the index of the high surrogate.
    // It is documented that if a mapping is unavailable, the glyph will be set to 0.
    CTFontGetGlyphsForCharacters(fCTFont, theChar, cgGlyph, numUniChar);
    return cgGlyph[0];
}

void SkScalerContext_Mac::generateAdvance(SkGlyph* glyph) {
    this->generateMetrics(glyph);
}

void SkScalerContext_Mac::generateMetrics(SkGlyph* glyph) {
    AUTO_CG_LOCK();

    const CGGlyph cgGlyph = (CGGlyph) glyph->getGlyphID();
    glyph->zeroMetrics();

    // The following block produces cgAdvance in CG units (pixels, y up).
    CGSize cgAdvance;
    if (fVertical) {
        CTFontGetAdvancesForGlyphs(fCTFont, kCTFontVerticalOrientation,
                                   &cgGlyph, &cgAdvance, 1);
        // Vertical advances are returned as widths instead of heights.
        SkTSwap(cgAdvance.height, cgAdvance.width);
        cgAdvance.height = -cgAdvance.height;
    } else {
        CTFontGetAdvancesForGlyphs(fCTFont, kCTFontHorizontalOrientation,
                                   &cgGlyph, &cgAdvance, 1);
    }
    cgAdvance = CGSizeApplyAffineTransform(cgAdvance, fTransform);
    glyph->fAdvanceX =  CGToFloat(cgAdvance.width);
    glyph->fAdvanceY = -CGToFloat(cgAdvance.height);

    // The following produces skBounds in SkGlyph units (pixels, y down),
    // or returns early if skBounds would be empty.
    SkRect skBounds;

    // On Mountain Lion, CTFontGetBoundingRectsForGlyphs with kCTFontVerticalOrientation and
    // CTFontGetVerticalTranslationsForGlyphs do not agree when using OTF CFF fonts.
    // For TTF fonts these two do agree and we can use CTFontGetBoundingRectsForGlyphs to get
    // the bounding box and CTFontGetVerticalTranslationsForGlyphs to then draw the glyph
    // inside that bounding box. However, with OTF CFF fonts this does not work. It appears that
    // CTFontGetBoundingRectsForGlyphs with kCTFontVerticalOrientation on OTF CFF fonts tries
    // to center the glyph along the vertical baseline and also perform some mysterious shift
    // along the baseline. CTFontGetVerticalTranslationsForGlyphs does not appear to perform
    // these steps.
    //
    // It is not known which is correct (or if either is correct). However, we must always draw
    // from the horizontal origin and must use CTFontGetVerticalTranslationsForGlyphs to draw.
    // As a result, we do not call CTFontGetBoundingRectsForGlyphs for vertical glyphs.

    // On Snow Leopard, CTFontGetBoundingRectsForGlyphs ignores kCTFontVerticalOrientation and
    // returns horizontal bounds.

    // On Lion and Mountain Lion, CTFontGetBoundingRectsForGlyphs has a bug which causes it to
    // return a bad value in cgBounds.origin.x for SFNT fonts whose hhea::numberOfHMetrics is
    // less than its maxp::numGlyphs. When this is the case we try to read the bounds from the
    // font directly.
    if ((isLion() || isMountainLion()) &&
        (cgGlyph < fGlyphCount && cgGlyph >= getFBoundingBoxesGlyphOffset() && generateBBoxes()))
    {
        const GlyphRect& gRect = fFBoundingBoxes[cgGlyph - fFBoundingBoxesGlyphOffset];
        if (gRect.fMinX >= gRect.fMaxX || gRect.fMinY >= gRect.fMaxY) {
            return;
        }
        skBounds = SkRect::MakeLTRB(gRect.fMinX, gRect.fMinY, gRect.fMaxX, gRect.fMaxY);
        // From FUnits (em space, y up) to SkGlyph units (pixels, y down).
        fFUnitMatrix.mapRect(&skBounds);

    } else {
        // CTFontGetBoundingRectsForGlyphs produces cgBounds in CG units (pixels, y up).
        CGRect cgBounds;
        CTFontGetBoundingRectsForGlyphs(fCTFont, kCTFontHorizontalOrientation,
                                        &cgGlyph, &cgBounds, 1);
        cgBounds = CGRectApplyAffineTransform(cgBounds, fTransform);

        // BUG?
        // 0x200B (zero-advance space) seems to return a huge (garbage) bounds, when
        // it should be empty. So, if we see a zero-advance, we check if it has an
        // empty path or not, and if so, we jam the bounds to 0. Hopefully a zero-advance
        // is rare, so we won't incur a big performance cost for this extra check.
        if (0 == cgAdvance.width && 0 == cgAdvance.height) {
            AutoCFRelease<CGPathRef> path(CTFontCreatePathForGlyph(fCTFont, cgGlyph, nullptr));
            if (nullptr == path || CGPathIsEmpty(path)) {
                return;
            }
        }

        if (CGRectIsEmpty_inline(cgBounds)) {
            return;
        }

        // Convert cgBounds to SkGlyph units (pixels, y down).
        skBounds = SkRect::MakeXYWH(cgBounds.origin.x, -cgBounds.origin.y - cgBounds.size.height,
                                    cgBounds.size.width, cgBounds.size.height);
    }

    if (fVertical) {
        // Due to all of the vertical bounds bugs, skBounds is always the horizontal bounds.
        // Convert these horizontal bounds into vertical bounds.
        SkPoint offset;
        getVerticalOffset(cgGlyph, &offset);
        skBounds.offset(offset);
    }

    // Currently the bounds are based on being rendered at (0,0).
    // The top left must not move, since that is the base from which subpixel positioning is offset.
    if (fDoSubPosition) {
        skBounds.fRight += SkFixedToFloat(glyph->getSubXFixed());
        skBounds.fBottom += SkFixedToFloat(glyph->getSubYFixed());
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

#include "SkColorPriv.h"

static void build_power_table(uint8_t table[]) {
    for (int i = 0; i < 256; i++) {
        float x = i / 255.f;
        int xx = SkScalarRoundToInt(x * x * 255);
        table[i] = SkToU8(xx);
    }
}

/**
 *  This will invert the gamma applied by CoreGraphics, so we can get linear
 *  values.
 *
 *  CoreGraphics obscurely defaults to 2.0 as the smoothing gamma value.
 *  The color space used does not appear to affect this choice.
 */
static const uint8_t* getInverseGammaTableCoreGraphicSmoothing() {
    static bool gInited;
    static uint8_t gTableCoreGraphicsSmoothing[256];
    if (!gInited) {
        build_power_table(gTableCoreGraphicsSmoothing);
        gInited = true;
    }
    return gTableCoreGraphicsSmoothing;
}

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
    lum = SkTMax(lum, (U8CPU)0x30);
#endif
    return lum;
}
template<bool APPLY_PREBLEND>
static void rgb_to_a8(const CGRGBPixel* SK_RESTRICT cgPixels, size_t cgRowBytes,
                      const SkGlyph& glyph, const uint8_t* table8) {
    const int width = glyph.fWidth;
    size_t dstRB = glyph.rowBytes();
    uint8_t* SK_RESTRICT dst = (uint8_t*)glyph.fImage;

    for (int y = 0; y < glyph.fHeight; y++) {
        for (int i = 0; i < width; ++i) {
            dst[i] = rgb_to_a8<APPLY_PREBLEND>(cgPixels[i], table8);
        }
        cgPixels = (CGRGBPixel*)((char*)cgPixels + cgRowBytes);
        dst += dstRB;
    }
}

template<bool APPLY_PREBLEND>
static inline uint16_t rgb_to_lcd16(CGRGBPixel rgb, const uint8_t* tableR,
                                                    const uint8_t* tableG,
                                                    const uint8_t* tableB) {
    U8CPU r = sk_apply_lut_if<APPLY_PREBLEND>(0xFF - ((rgb >> 16) & 0xFF), tableR);
    U8CPU g = sk_apply_lut_if<APPLY_PREBLEND>(0xFF - ((rgb >>  8) & 0xFF), tableG);
    U8CPU b = sk_apply_lut_if<APPLY_PREBLEND>(0xFF - ((rgb >>  0) & 0xFF), tableB);
#if SK_SHOW_TEXT_BLIT_COVERAGE
    r = SkTMax(r, (U8CPU)0x30);
    g = SkTMax(g, (U8CPU)0x30);
    b = SkTMax(b, (U8CPU)0x30);
#endif
    return SkPack888ToRGB16(r, g, b);
}
template<bool APPLY_PREBLEND>
static void rgb_to_lcd16(const CGRGBPixel* SK_RESTRICT cgPixels, size_t cgRowBytes, const SkGlyph& glyph,
                         const uint8_t* tableR, const uint8_t* tableG, const uint8_t* tableB) {
    const int width = glyph.fWidth;
    size_t dstRB = glyph.rowBytes();
    uint16_t* SK_RESTRICT dst = (uint16_t*)glyph.fImage;

    for (int y = 0; y < glyph.fHeight; y++) {
        for (int i = 0; i < width; i++) {
            dst[i] = rgb_to_lcd16<APPLY_PREBLEND>(cgPixels[i], tableR, tableG, tableB);
        }
        cgPixels = (CGRGBPixel*)((char*)cgPixels + cgRowBytes);
        dst = (uint16_t*)((char*)dst + dstRB);
    }
}

static SkPMColor cgpixels_to_pmcolor(CGRGBPixel rgb) {
    U8CPU a = (rgb >> 24) & 0xFF;
    U8CPU r = (rgb >> 16) & 0xFF;
    U8CPU g = (rgb >>  8) & 0xFF;
    U8CPU b = (rgb >>  0) & 0xFF;
#if SK_SHOW_TEXT_BLIT_COVERAGE
    a = SkTMax(a, (U8CPU)0x30);
#endif
    return SkPackARGB32(a, r, g, b);
}

void SkScalerContext_Mac::generateImage(const SkGlyph& glyph) {
    CGGlyph cgGlyph = (CGGlyph) glyph.getGlyphID();

    // FIXME: lcd smoothed un-hinted rasterization unsupported.
    bool generateA8FromLCD = fRec.getHinting() != SkPaint::kNo_Hinting;

    // Draw the glyph
    size_t cgRowBytes;
    CGRGBPixel* cgPixels = fOffscreen.getCG(*this, glyph, cgGlyph, &cgRowBytes, generateA8FromLCD);
    if (cgPixels == nullptr) {
        return;
    }

    // Fix the glyph
    const bool isLCD = isLCDFormat(glyph.fMaskFormat);
    if (isLCD || (glyph.fMaskFormat == SkMask::kA8_Format && supports_LCD() && generateA8FromLCD)) {
        const uint8_t* table = getInverseGammaTableCoreGraphicSmoothing();

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
                addr[x] = (table[r] << 16) | (table[g] << 8) | table[b];
            }
            addr = SkTAddOffset<CGRGBPixel>(addr, cgRowBytes);
        }
    }

    // Convert glyph to mask
    switch (glyph.fMaskFormat) {
        case SkMask::kLCD16_Format: {
            if (fPreBlend.isApplicable()) {
                rgb_to_lcd16<true>(cgPixels, cgRowBytes, glyph,
                                   fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            } else {
                rgb_to_lcd16<false>(cgPixels, cgRowBytes, glyph,
                                    fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            }
        } break;
        case SkMask::kA8_Format: {
            if (fPreBlend.isApplicable()) {
                rgb_to_a8<true>(cgPixels, cgRowBytes, glyph, fPreBlend.fG);
            } else {
                rgb_to_a8<false>(cgPixels, cgRowBytes, glyph, fPreBlend.fG);
            }
        } break;
        case SkMask::kBW_Format: {
            const int width = glyph.fWidth;
            size_t dstRB = glyph.rowBytes();
            uint8_t* dst = (uint8_t*)glyph.fImage;
            for (int y = 0; y < glyph.fHeight; y++) {
                cgpixels_to_bits(dst, cgPixels, width);
                cgPixels = (CGRGBPixel*)((char*)cgPixels + cgRowBytes);
                dst += dstRB;
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
                cgPixels = (CGRGBPixel*)((char*)cgPixels + cgRowBytes);
                dst = (SkPMColor*)((char*)dst + dstRB);
            }
        } break;
        default:
            SkDEBUGFAIL("unexpected mask format");
            break;
    }
}

/*
 *  Our subpixel resolution is only 2 bits in each direction, so a scale of 4
 *  seems sufficient, and possibly even correct, to allow the hinted outline
 *  to be subpixel positioned.
 */
#define kScaleForSubPixelPositionHinting (4.0f)

void SkScalerContext_Mac::generatePath(const SkGlyph& glyph, SkPath* path) {
    AUTO_CG_LOCK();

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

        CGAffineTransform scale(CGAffineTransformMakeScale(ScalarToCG(scaleX), ScalarToCG(scaleY)));
        xform = CGAffineTransformConcat(fTransform, scale);
    }

    CGGlyph cgGlyph = (CGGlyph)glyph.getGlyphID();
    AutoCFRelease<CGPathRef> cgPath(CTFontCreatePathForGlyph(fCTFont, cgGlyph, &xform));

    path->reset();
    if (cgPath != nullptr) {
        CGPathApply(cgPath, path, SkScalerContext_Mac::CTPathElement);
    }

    if (fDoSubPosition) {
        SkMatrix m;
        m.setScale(SkScalarInvert(scaleX), SkScalarInvert(scaleY));
        path->transform(m);
    }
    if (fVertical) {
        SkPoint offset;
        getVerticalOffset(cgGlyph, &offset);
        path->offset(offset.fX, offset.fY);
    }
}

void SkScalerContext_Mac::generateFontMetrics(SkPaint::FontMetrics* metrics) {
    if (nullptr == metrics) {
        return;
    }

    AUTO_CG_LOCK();

    CGRect theBounds = CTFontGetBoundingBox(fCTFont);

    metrics->fTop          = CGToScalar(-CGRectGetMaxY_inline(theBounds));
    metrics->fAscent       = CGToScalar(-CTFontGetAscent(fCTFont));
    metrics->fDescent      = CGToScalar( CTFontGetDescent(fCTFont));
    metrics->fBottom       = CGToScalar(-CGRectGetMinY_inline(theBounds));
    metrics->fLeading      = CGToScalar( CTFontGetLeading(fCTFont));
    metrics->fAvgCharWidth = CGToScalar( CGRectGetWidth_inline(theBounds));
    metrics->fXMin         = CGToScalar( CGRectGetMinX_inline(theBounds));
    metrics->fXMax         = CGToScalar( CGRectGetMaxX_inline(theBounds));
    metrics->fMaxCharWidth = metrics->fXMax - metrics->fXMin;
    metrics->fXHeight      = CGToScalar( CTFontGetXHeight(fCTFont));
    metrics->fCapHeight    = CGToScalar( CTFontGetCapHeight(fCTFont));
    metrics->fUnderlineThickness = CGToScalar( CTFontGetUnderlineThickness(fCTFont));
    metrics->fUnderlinePosition = -CGToScalar( CTFontGetUnderlinePosition(fCTFont));

    metrics->fFlags |= SkPaint::FontMetrics::kUnderlineThinknessIsValid_Flag;
    metrics->fFlags |= SkPaint::FontMetrics::kUnderlinePositionIsValid_Flag;
}

void SkScalerContext_Mac::CTPathElement(void *info, const CGPathElement *element) {
    SkPath* skPath = (SkPath*)info;

    // Process the path element
    switch (element->type) {
        case kCGPathElementMoveToPoint:
            skPath->moveTo(element->points[0].x, -element->points[0].y);
            break;

        case kCGPathElementAddLineToPoint:
            skPath->lineTo(element->points[0].x, -element->points[0].y);
            break;

        case kCGPathElementAddQuadCurveToPoint:
            skPath->quadTo(element->points[0].x, -element->points[0].y,
                           element->points[1].x, -element->points[1].y);
            break;

        case kCGPathElementAddCurveToPoint:
            skPath->cubicTo(element->points[0].x, -element->points[0].y,
                            element->points[1].x, -element->points[1].y,
                            element->points[2].x, -element->points[2].y);
            break;

        case kCGPathElementCloseSubpath:
            skPath->close();
            break;

        default:
            SkDEBUGFAIL("Unknown path element!");
            break;
        }
}


///////////////////////////////////////////////////////////////////////////////

// Returns nullptr on failure
// Call must still manage its ownership of provider
static SkTypeface* create_from_dataProvider(CGDataProviderRef provider) {
    AutoCFRelease<CGFontRef> cg(CGFontCreateWithDataProvider(provider));
    if (nullptr == cg) {
        return nullptr;
    }
    CTFontRef ct = CTFontCreateWithGraphicsFont(cg, 0, nullptr, nullptr);
    return ct ? NewFromFontRef(ct, nullptr, true) : nullptr;
}

// Web fonts added to the the CTFont registry do not return their character set.
// Iterate through the font in this case. The existing caller caches the result,
// so the performance impact isn't too bad.
static void populate_glyph_to_unicode_slow(CTFontRef ctFont, CFIndex glyphCount,
                                           SkTDArray<SkUnichar>* glyphToUnicode) {
    glyphToUnicode->setCount(SkToInt(glyphCount));
    SkUnichar* out = glyphToUnicode->begin();
    sk_bzero(out, glyphCount * sizeof(SkUnichar));
    UniChar unichar = 0;
    while (glyphCount > 0) {
        CGGlyph glyph;
        if (CTFontGetGlyphsForCharacters(ctFont, &unichar, &glyph, 1)) {
            out[glyph] = unichar;
            --glyphCount;
        }
        if (++unichar == 0) {
            break;
        }
    }
}

// Construct Glyph to Unicode table.
// Unicode code points that require conjugate pairs in utf16 are not
// supported.
static void populate_glyph_to_unicode(CTFontRef ctFont, CFIndex glyphCount,
                                      SkTDArray<SkUnichar>* glyphToUnicode) {
    AutoCFRelease<CFCharacterSetRef> charSet(CTFontCopyCharacterSet(ctFont));
    if (!charSet) {
        populate_glyph_to_unicode_slow(ctFont, glyphCount, glyphToUnicode);
        return;
    }

    AutoCFRelease<CFDataRef> bitmap(CFCharacterSetCreateBitmapRepresentation(kCFAllocatorDefault,
                                                                             charSet));
    if (!bitmap) {
        return;
    }
    CFIndex length = CFDataGetLength(bitmap);
    if (!length) {
        return;
    }
    if (length > 8192) {
        // TODO: Add support for Unicode above 0xFFFF
        // Consider only the BMP portion of the Unicode character points.
        // The bitmap may contain other planes, up to plane 16.
        // See http://developer.apple.com/library/ios/#documentation/CoreFoundation/Reference/CFCharacterSetRef/Reference/reference.html
        length = 8192;
    }
    const UInt8* bits = CFDataGetBytePtr(bitmap);
    glyphToUnicode->setCount(SkToInt(glyphCount));
    SkUnichar* out = glyphToUnicode->begin();
    sk_bzero(out, glyphCount * sizeof(SkUnichar));
    for (int i = 0; i < length; i++) {
        int mask = bits[i];
        if (!mask) {
            continue;
        }
        for (int j = 0; j < 8; j++) {
            CGGlyph glyph;
            UniChar unichar = static_cast<UniChar>((i << 3) + j);
            if (mask & (1 << j) && CTFontGetGlyphsForCharacters(ctFont, &unichar, &glyph, 1)) {
                out[glyph] = unichar;
            }
        }
    }
}

/** Assumes src and dst are not nullptr. */
static void CFStringToSkString(CFStringRef src, SkString* dst) {
    // Reserve enough room for the worst-case string,
    // plus 1 byte for the trailing null.
    CFIndex length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(src),
                                                       kCFStringEncodingUTF8) + 1;
    dst->resize(length);
    CFStringGetCString(src, dst->writable_str(), length, kCFStringEncodingUTF8);
    // Resize to the actual UTF-8 length used, stripping the null character.
    dst->resize(strlen(dst->c_str()));
}

SkAdvancedTypefaceMetrics* SkTypeface_Mac::onGetAdvancedTypefaceMetrics(
        PerGlyphInfo perGlyphInfo,
        const uint32_t* glyphIDs,
        uint32_t glyphIDsCount) const {

    AUTO_CG_LOCK();

    CTFontRef originalCTFont = fFontRef.get();
    AutoCFRelease<CTFontRef> ctFont(ctfont_create_exact_copy(
            originalCTFont, CTFontGetUnitsPerEm(originalCTFont), nullptr));

    SkAdvancedTypefaceMetrics* info = new SkAdvancedTypefaceMetrics;

    {
        AutoCFRelease<CFStringRef> fontName(CTFontCopyPostScriptName(ctFont));
        if (fontName.get()) {
            CFStringToSkString(fontName, &info->fFontName);
        }
    }

    CFIndex glyphCount = CTFontGetGlyphCount(ctFont);
    info->fLastGlyphID = SkToU16(glyphCount - 1);
    info->fEmSize = CTFontGetUnitsPerEm(ctFont);

    if (perGlyphInfo & kToUnicode_PerGlyphInfo) {
        populate_glyph_to_unicode(ctFont, glyphCount, &info->fGlyphToUnicode);
    }

    // If it's not a truetype font, mark it as 'other'. Assume that TrueType
    // fonts always have both glyf and loca tables. At the least, this is what
    // sfntly needs to subset the font. CTFontCopyAttribute() does not always
    // succeed in determining this directly.
    if (!this->getTableSize('glyf') || !this->getTableSize('loca')) {
        return info;
    }

    info->fType = SkAdvancedTypefaceMetrics::kTrueType_Font;
    CTFontSymbolicTraits symbolicTraits = CTFontGetSymbolicTraits(ctFont);
    if (symbolicTraits & kCTFontMonoSpaceTrait) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kFixedPitch_Style;
    }
    if (symbolicTraits & kCTFontItalicTrait) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kItalic_Style;
    }
    CTFontStylisticClass stylisticClass = symbolicTraits & kCTFontClassMaskTrait;
    if (stylisticClass >= kCTFontOldStyleSerifsClass && stylisticClass <= kCTFontSlabSerifsClass) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kSerif_Style;
    } else if (stylisticClass & kCTFontScriptsClass) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kScript_Style;
    }
    info->fItalicAngle = (int16_t) CTFontGetSlantAngle(ctFont);
    info->fAscent = (int16_t) CTFontGetAscent(ctFont);
    info->fDescent = (int16_t) CTFontGetDescent(ctFont);
    info->fCapHeight = (int16_t) CTFontGetCapHeight(ctFont);
    CGRect bbox = CTFontGetBoundingBox(ctFont);

    SkRect r;
    r.set( CGToScalar(CGRectGetMinX_inline(bbox)),   // Left
           CGToScalar(CGRectGetMaxY_inline(bbox)),   // Top
           CGToScalar(CGRectGetMaxX_inline(bbox)),   // Right
           CGToScalar(CGRectGetMinY_inline(bbox)));  // Bottom

    r.roundOut(&(info->fBBox));

    // Figure out a good guess for StemV - Min width of i, I, !, 1.
    // This probably isn't very good with an italic font.
    int16_t min_width = SHRT_MAX;
    info->fStemV = 0;
    static const UniChar stem_chars[] = {'i', 'I', '!', '1'};
    const size_t count = sizeof(stem_chars) / sizeof(stem_chars[0]);
    CGGlyph glyphs[count];
    CGRect boundingRects[count];
    if (CTFontGetGlyphsForCharacters(ctFont, stem_chars, glyphs, count)) {
        CTFontGetBoundingRectsForGlyphs(ctFont, kCTFontHorizontalOrientation,
                                        glyphs, boundingRects, count);
        for (size_t i = 0; i < count; i++) {
            int16_t width = (int16_t) boundingRects[i].size.width;
            if (width > 0 && width < min_width) {
                min_width = width;
                info->fStemV = min_width;
            }
        }
    }

    if (perGlyphInfo & kHAdvance_PerGlyphInfo) {
        if (info->fStyle & SkAdvancedTypefaceMetrics::kFixedPitch_Style) {
            SkAdvancedTypefaceMetrics::WidthRange range(0);
            range.fAdvance.append(1, &min_width);
            SkAdvancedTypefaceMetrics::FinishRange(
                    &range, 0, SkAdvancedTypefaceMetrics::WidthRange::kDefault);
            info->fGlyphWidths.emplace_back(std::move(range));
        } else {
            CTFontRef borrowedCTFont = ctFont.get();
            info->setGlyphWidths(
                SkToInt(glyphCount),
                glyphIDs,
                glyphIDsCount,
                SkAdvancedTypefaceMetrics::GetAdvance([borrowedCTFont](int gId, int16_t* data) {
                    CGSize advance;
                    advance.width = 0;
                    CGGlyph glyph = gId;
                    CTFontGetAdvancesForGlyphs(borrowedCTFont, kCTFontHorizontalOrientation,
                                               &glyph, &advance, 1);
                    *data = sk_float_round2int(advance.width);
                    return true;
                })
            );
        }
    }
    return info;
}

///////////////////////////////////////////////////////////////////////////////

static SK_SFNT_ULONG get_font_type_tag(const SkTypeface_Mac* typeface) {
    CTFontRef ctFont = typeface->fFontRef.get();
    AutoCFRelease<CFNumberRef> fontFormatRef(
            static_cast<CFNumberRef>(CTFontCopyAttribute(ctFont, kCTFontFormatAttribute)));
    if (!fontFormatRef) {
        return 0;
    }

    SInt32 fontFormatValue;
    if (!CFNumberGetValue(fontFormatRef, kCFNumberSInt32Type, &fontFormatValue)) {
        return 0;
    }

    switch (fontFormatValue) {
        case kCTFontFormatOpenTypePostScript:
            return SkSFNTHeader::fontType_OpenTypeCFF::TAG;
        case kCTFontFormatOpenTypeTrueType:
            return SkSFNTHeader::fontType_WindowsTrueType::TAG;
        case kCTFontFormatTrueType:
            return SkSFNTHeader::fontType_MacTrueType::TAG;
        case kCTFontFormatPostScript:
            return SkSFNTHeader::fontType_PostScript::TAG;
        case kCTFontFormatBitmap:
            return SkSFNTHeader::fontType_MacTrueType::TAG;
        case kCTFontFormatUnrecognized:
        default:
            //CT seems to be unreliable in being able to obtain the type,
            //even if all we want is the first four bytes of the font resource.
            //Just the presence of the FontForge 'FFTM' table seems to throw it off.
            return SkSFNTHeader::fontType_WindowsTrueType::TAG;
    }
}

SkStreamAsset* SkTypeface_Mac::onOpenStream(int* ttcIndex) const {
    SK_SFNT_ULONG fontType = get_font_type_tag(this);
    if (0 == fontType) {
        return nullptr;
    }

    // get table tags
    int numTables = this->countTables();
    SkTDArray<SkFontTableTag> tableTags;
    tableTags.setCount(numTables);
    this->getTableTags(tableTags.begin());

    // calc total size for font, save sizes
    SkTDArray<size_t> tableSizes;
    size_t totalSize = sizeof(SkSFNTHeader) + sizeof(SkSFNTHeader::TableDirectoryEntry) * numTables;
    for (int tableIndex = 0; tableIndex < numTables; ++tableIndex) {
        size_t tableSize = this->getTableSize(tableTags[tableIndex]);
        totalSize += (tableSize + 3) & ~3;
        *tableSizes.append() = tableSize;
    }

    // reserve memory for stream, and zero it (tables must be zero padded)
    SkMemoryStream* stream = new SkMemoryStream(totalSize);
    char* dataStart = (char*)stream->getMemoryBase();
    sk_bzero(dataStart, totalSize);
    char* dataPtr = dataStart;

    // compute font header entries
    uint16_t entrySelector = 0;
    uint16_t searchRange = 1;
    while (searchRange < numTables >> 1) {
        entrySelector++;
        searchRange <<= 1;
    }
    searchRange <<= 4;
    uint16_t rangeShift = (numTables << 4) - searchRange;

    // write font header
    SkSFNTHeader* header = (SkSFNTHeader*)dataPtr;
    header->fontType = fontType;
    header->numTables = SkEndian_SwapBE16(numTables);
    header->searchRange = SkEndian_SwapBE16(searchRange);
    header->entrySelector = SkEndian_SwapBE16(entrySelector);
    header->rangeShift = SkEndian_SwapBE16(rangeShift);
    dataPtr += sizeof(SkSFNTHeader);

    // write tables
    SkSFNTHeader::TableDirectoryEntry* entry = (SkSFNTHeader::TableDirectoryEntry*)dataPtr;
    dataPtr += sizeof(SkSFNTHeader::TableDirectoryEntry) * numTables;
    for (int tableIndex = 0; tableIndex < numTables; ++tableIndex) {
        size_t tableSize = tableSizes[tableIndex];
        this->getTableData(tableTags[tableIndex], 0, tableSize, dataPtr);
        entry->tag = SkEndian_SwapBE32(tableTags[tableIndex]);
        entry->checksum = SkEndian_SwapBE32(SkOTUtils::CalcTableChecksum((SK_OT_ULONG*)dataPtr,
                                                                         tableSize));
        entry->offset = SkEndian_SwapBE32(SkToU32(dataPtr - dataStart));
        entry->logicalLength = SkEndian_SwapBE32(SkToU32(tableSize));

        dataPtr += (tableSize + 3) & ~3;
        ++entry;
    }

    *ttcIndex = 0;
    return stream;
}

struct NonDefaultAxesContext {
    SkFixed* axisValue;
    CFArrayRef cgAxes;
};
static void set_non_default_axes(CFTypeRef key, CFTypeRef value, void* context) {
    NonDefaultAxesContext* self = static_cast<NonDefaultAxesContext*>(context);

    if (CFGetTypeID(key) != CFStringGetTypeID() || CFGetTypeID(value) != CFNumberGetTypeID()) {
        return;
    }

    // The key is a CFString which is a string from the 'name' table.
    // Search the cgAxes for an axis with this name, and use its index to store the value.
    CFIndex keyIndex = -1;
    CFStringRef keyString = static_cast<CFStringRef>(key);
    for (CFIndex i = 0; i < CFArrayGetCount(self->cgAxes); ++i) {
        CFTypeRef cgAxis = CFArrayGetValueAtIndex(self->cgAxes, i);
        if (CFGetTypeID(cgAxis) != CFDictionaryGetTypeID()) {
            continue;
        }

        CFDictionaryRef cgAxisDict = static_cast<CFDictionaryRef>(cgAxis);
        CFTypeRef cgAxisName = CFDictionaryGetValue(cgAxisDict, kCGFontVariationAxisName);
        if (!cgAxisName || CFGetTypeID(cgAxisName) != CFStringGetTypeID()) {
            continue;
        }
        CFStringRef cgAxisNameString = static_cast<CFStringRef>(cgAxisName);
        if (CFStringCompare(keyString, cgAxisNameString, 0) == kCFCompareEqualTo) {
            keyIndex = i;
            break;
        }
    }
    if (keyIndex == -1) {
        return;
    }

    CFNumberRef valueNumber = static_cast<CFNumberRef>(value);
    double valueDouble;
    if (!CFNumberGetValue(valueNumber, kCFNumberDoubleType, &valueDouble) ||
        valueDouble < SkFixedToDouble(SK_FixedMin) || SkFixedToDouble(SK_FixedMax) < valueDouble)
    {
        return;
    }
    self->axisValue[keyIndex] = SkDoubleToFixed(valueDouble);
}
static bool get_variations(CTFontRef fFontRef, CFIndex* cgAxisCount,
                           SkAutoSTMalloc<4, SkFixed>* axisValues)
{
    // CTFontCopyVariationAxes and CTFontCopyVariation do not work when applied to fonts which
    // started life with CGFontCreateWithDataProvider (they simply always return nullptr).
    // As a result, we are limited to CGFontCopyVariationAxes and CGFontCopyVariations.
    AutoCFRelease<CGFontRef> cgFont(CTFontCopyGraphicsFont(fFontRef, nullptr));

    AutoCFRelease<CFDictionaryRef> cgVariations(CGFontCopyVariations(cgFont));
    // If a font has no variations CGFontCopyVariations returns nullptr (instead of an empty dict).
    if (!cgVariations.get()) {
        return false;
    }

    AutoCFRelease<CFArrayRef> cgAxes(CGFontCopyVariationAxes(cgFont));
    *cgAxisCount = CFArrayGetCount(cgAxes);
    axisValues->reset(*cgAxisCount);

    // Set all of the axes to their default values.
    // Fail if any default value cannot be determined.
    for (CFIndex i = 0; i < *cgAxisCount; ++i) {
        CFTypeRef cgAxis = CFArrayGetValueAtIndex(cgAxes, i);
        if (CFGetTypeID(cgAxis) != CFDictionaryGetTypeID()) {
            return false;
        }

        CFDictionaryRef cgAxisDict = static_cast<CFDictionaryRef>(cgAxis);
        CFTypeRef axisDefaultValue = CFDictionaryGetValue(cgAxisDict,
                                                          kCGFontVariationAxisDefaultValue);
        if (!axisDefaultValue || CFGetTypeID(axisDefaultValue) != CFNumberGetTypeID()) {
            return false;
        }
        CFNumberRef axisDefaultValueNumber = static_cast<CFNumberRef>(axisDefaultValue);
        double axisDefaultValueDouble;
        if (!CFNumberGetValue(axisDefaultValueNumber, kCFNumberDoubleType, &axisDefaultValueDouble))
        {
            return false;
        }
        if (axisDefaultValueDouble < SkFixedToDouble(SK_FixedMin) ||
                                     SkFixedToDouble(SK_FixedMax) < axisDefaultValueDouble)
        {
            return false;
        }
        (*axisValues)[(int)i] = SkDoubleToFixed(axisDefaultValueDouble);
    }

    // Override the default values with the given font's stated axis values.
    NonDefaultAxesContext c = { axisValues->get(), cgAxes.get() };
    CFDictionaryApplyFunction(cgVariations, set_non_default_axes, &c);

    return true;
}
SkFontData* SkTypeface_Mac::onCreateFontData() const {
    int index;
    SkAutoTDelete<SkStreamAsset> stream(this->onOpenStream(&index));

    CFIndex cgAxisCount;
    SkAutoSTMalloc<4, SkFixed> axisValues;
    if (get_variations(fFontRef, &cgAxisCount, &axisValues)) {
        return new SkFontData(stream.release(), index, axisValues.get(), cgAxisCount);
    }
    return new SkFontData(stream.release(), index, nullptr, 0);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int SkTypeface_Mac::onGetUPEM() const {
    AutoCFRelease<CGFontRef> cgFont(CTFontCopyGraphicsFont(fFontRef, nullptr));
    return CGFontGetUnitsPerEm(cgFont);
}

SkTypeface::LocalizedStrings* SkTypeface_Mac::onCreateFamilyNameIterator() const {
    SkTypeface::LocalizedStrings* nameIter =
        SkOTUtils::LocalizedStrings_NameTable::CreateForFamilyNames(*this);
    if (nullptr == nameIter) {
        AutoCFRelease<CFStringRef> cfLanguage;
        AutoCFRelease<CFStringRef> cfFamilyName(
            CTFontCopyLocalizedName(fFontRef, kCTFontFamilyNameKey, &cfLanguage));

        SkString skLanguage;
        SkString skFamilyName;
        if (cfLanguage.get()) {
            CFStringToSkString(cfLanguage.get(), &skLanguage);
        } else {
            skLanguage = "und"; //undetermined
        }
        if (cfFamilyName.get()) {
            CFStringToSkString(cfFamilyName.get(), &skFamilyName);
        }

        nameIter = new SkOTUtils::LocalizedStrings_SingleName(skFamilyName, skLanguage);
    }
    return nameIter;
}

// If, as is the case with web fonts, the CTFont data isn't available,
// the CGFont data may work. While the CGFont may always provide the
// right result, leave the CTFont code path to minimize disruption.
static CFDataRef copyTableFromFont(CTFontRef ctFont, SkFontTableTag tag) {
    CFDataRef data = CTFontCopyTable(ctFont, (CTFontTableTag) tag,
                                     kCTFontTableOptionNoOptions);
    if (nullptr == data) {
        AutoCFRelease<CGFontRef> cgFont(CTFontCopyGraphicsFont(ctFont, nullptr));
        data = CGFontCopyTableForTag(cgFont, tag);
    }
    return data;
}

int SkTypeface_Mac::onGetTableTags(SkFontTableTag tags[]) const {
    AutoCFRelease<CFArrayRef> cfArray(CTFontCopyAvailableTables(fFontRef,
                                                kCTFontTableOptionNoOptions));
    if (nullptr == cfArray) {
        return 0;
    }
    int count = SkToInt(CFArrayGetCount(cfArray));
    if (tags) {
        for (int i = 0; i < count; ++i) {
            uintptr_t fontTag = reinterpret_cast<uintptr_t>(CFArrayGetValueAtIndex(cfArray, i));
            tags[i] = static_cast<SkFontTableTag>(fontTag);
        }
    }
    return count;
}

size_t SkTypeface_Mac::onGetTableData(SkFontTableTag tag, size_t offset,
                                      size_t length, void* dstData) const {
    AutoCFRelease<CFDataRef> srcData(copyTableFromFont(fFontRef, tag));
    if (nullptr == srcData) {
        return 0;
    }

    size_t srcSize = CFDataGetLength(srcData);
    if (offset >= srcSize) {
        return 0;
    }
    if (length > srcSize - offset) {
        length = srcSize - offset;
    }
    if (dstData) {
        memcpy(dstData, CFDataGetBytePtr(srcData) + offset, length);
    }
    return length;
}

SkScalerContext* SkTypeface_Mac::onCreateScalerContext(const SkScalerContextEffects& effects,
                                                       const SkDescriptor* desc) const {
    return new SkScalerContext_Mac(const_cast<SkTypeface_Mac*>(this), effects, desc);
}

void SkTypeface_Mac::onFilterRec(SkScalerContextRec* rec) const {
    if (rec->fFlags & SkScalerContext::kLCD_BGROrder_Flag ||
        rec->fFlags & SkScalerContext::kLCD_Vertical_Flag)
    {
        rec->fMaskFormat = SkMask::kA8_Format;
        // Render the glyphs as close as possible to what was requested.
        // The above turns off subpixel rendering, but the user requested it.
        // Normal hinting will cause the A8 masks to be generated from CoreGraphics subpixel masks.
        // See comments below for more details.
        rec->setHinting(SkPaint::kNormal_Hinting);
    }

    unsigned flagsWeDontSupport = SkScalerContext::kDevKernText_Flag  |
                                  SkScalerContext::kForceAutohinting_Flag  |
                                  SkScalerContext::kLCD_BGROrder_Flag |
                                  SkScalerContext::kLCD_Vertical_Flag;

    rec->fFlags &= ~flagsWeDontSupport;

    bool lcdSupport = supports_LCD();

    // Only two levels of hinting are supported.
    // kNo_Hinting means avoid CoreGraphics outline dilation.
    // kNormal_Hinting means CoreGraphics outline dilation is allowed.
    // If there is no lcd support, hinting (dilation) cannot be supported.
    SkPaint::Hinting hinting = rec->getHinting();
    if (SkPaint::kSlight_Hinting == hinting || !lcdSupport) {
        hinting = SkPaint::kNo_Hinting;
    } else if (SkPaint::kFull_Hinting == hinting) {
        hinting = SkPaint::kNormal_Hinting;
    }
    rec->setHinting(hinting);

    // FIXME: lcd smoothed un-hinted rasterization unsupported.
    // Tracked by http://code.google.com/p/skia/issues/detail?id=915 .
    // There is no current means to honor a request for unhinted lcd,
    // so arbitrarilly ignore the hinting request and honor lcd.

    // Hinting and smoothing should be orthogonal, but currently they are not.
    // CoreGraphics has no API to influence hinting. However, its lcd smoothed
    // output is drawn from auto-dilated outlines (the amount of which is
    // determined by AppleFontSmoothing). Its regular anti-aliased output is
    // drawn from un-dilated outlines.

    // The behavior of Skia is as follows:
    // [AA][no-hint]: generate AA using CoreGraphic's AA output.
    // [AA][yes-hint]: use CoreGraphic's LCD output and reduce it to a single
    // channel. This matches [LCD][yes-hint] in weight.
    // [LCD][no-hint]: curently unable to honor, and must pick which to respect.
    // Currenly side with LCD, effectively ignoring the hinting setting.
    // [LCD][yes-hint]: generate LCD using CoreGraphic's LCD output.

    if (isLCDFormat(rec->fMaskFormat)) {
        if (lcdSupport) {
            //CoreGraphics creates 555 masks for smoothed text anyway.
            rec->fMaskFormat = SkMask::kLCD16_Format;
            rec->setHinting(SkPaint::kNormal_Hinting);
        } else {
            rec->fMaskFormat = SkMask::kA8_Format;
        }
    }

    // CoreText provides no information as to whether a glyph will be color or not.
    // Fonts may mix outlines and bitmaps, so information is needed on a glyph by glyph basis.
    // If a font contains an 'sbix' table, consider it to be a color font, and disable lcd.
    if (fHasColorGlyphs) {
        rec->fMaskFormat = SkMask::kARGB32_Format;
    }

    // Unhinted A8 masks (those not derived from LCD masks) must respect SK_GAMMA_APPLY_TO_A8.
    // All other masks can use regular gamma.
    if (SkMask::kA8_Format == rec->fMaskFormat && SkPaint::kNo_Hinting == hinting) {
#ifndef SK_GAMMA_APPLY_TO_A8
        // SRGBTODO: Is this correct? Do we want contrast boost?
        rec->ignorePreBlend();
#endif
    } else {
        //CoreGraphics dialates smoothed text as needed.
        rec->setContrast(0);
    }
}

// we take ownership of the ref
static const char* get_str(CFStringRef ref, SkString* str) {
    if (nullptr == ref) {
        return nullptr;
    }
    CFStringToSkString(ref, str);
    CFSafeRelease(ref);
    return str->c_str();
}

void SkTypeface_Mac::onGetFamilyName(SkString* familyName) const {
    get_str(CTFontCopyFamilyName(fFontRef), familyName);
}

void SkTypeface_Mac::onGetFontDescriptor(SkFontDescriptor* desc,
                                         bool* isLocalStream) const {
    SkString tmpStr;

    desc->setFamilyName(get_str(CTFontCopyFamilyName(fFontRef), &tmpStr));
    desc->setFullName(get_str(CTFontCopyFullName(fFontRef), &tmpStr));
    desc->setPostscriptName(get_str(CTFontCopyPostScriptName(fFontRef), &tmpStr));
    *isLocalStream = fIsLocalStream;
}

int SkTypeface_Mac::onCharsToGlyphs(const void* chars, Encoding encoding,
                                    uint16_t glyphs[], int glyphCount) const
{
    // Undocumented behavior of CTFontGetGlyphsForCharacters with non-bmp code points:
    // When a surrogate pair is detected, the glyph index used is the index of the high surrogate.
    // It is documented that if a mapping is unavailable, the glyph will be set to 0.

    SkAutoSTMalloc<1024, UniChar> charStorage;
    const UniChar* src; // UniChar is a UTF-16 16-bit code unit.
    int srcCount;
    switch (encoding) {
        case kUTF8_Encoding: {
            const char* utf8 = reinterpret_cast<const char*>(chars);
            UniChar* utf16 = charStorage.reset(2 * glyphCount);
            src = utf16;
            for (int i = 0; i < glyphCount; ++i) {
                SkUnichar uni = SkUTF8_NextUnichar(&utf8);
                utf16 += SkUTF16_FromUnichar(uni, utf16);
            }
            srcCount = SkToInt(utf16 - src);
            break;
        }
        case kUTF16_Encoding: {
            src = reinterpret_cast<const UniChar*>(chars);
            int extra = 0;
            for (int i = 0; i < glyphCount; ++i) {
                if (SkUTF16_IsHighSurrogate(src[i + extra])) {
                    ++extra;
                }
            }
            srcCount = glyphCount + extra;
            break;
        }
        case kUTF32_Encoding: {
            const SkUnichar* utf32 = reinterpret_cast<const SkUnichar*>(chars);
            UniChar* utf16 = charStorage.reset(2 * glyphCount);
            src = utf16;
            for (int i = 0; i < glyphCount; ++i) {
                utf16 += SkUTF16_FromUnichar(utf32[i], utf16);
            }
            srcCount = SkToInt(utf16 - src);
            break;
        }
    }

    // If glyphs is nullptr, CT still needs glyph storage for finding the first failure.
    // Also, if there are any non-bmp code points, the provided 'glyphs' storage will be inadequate.
    SkAutoSTMalloc<1024, uint16_t> glyphStorage;
    uint16_t* macGlyphs = glyphs;
    if (nullptr == macGlyphs || srcCount > glyphCount) {
        macGlyphs = glyphStorage.reset(srcCount);
    }

    bool allEncoded = CTFontGetGlyphsForCharacters(fFontRef, src, macGlyphs, srcCount);

    // If there were any non-bmp, then copy and compact.
    // If 'glyphs' is nullptr, then compact glyphStorage in-place.
    // If all are bmp and 'glyphs' is non-nullptr, 'glyphs' already contains the compact glyphs.
    // If some are non-bmp and 'glyphs' is non-nullptr, copy and compact into 'glyphs'.
    uint16_t* compactedGlyphs = glyphs;
    if (nullptr == compactedGlyphs) {
        compactedGlyphs = macGlyphs;
    }
    if (srcCount > glyphCount) {
        int extra = 0;
        for (int i = 0; i < glyphCount; ++i) {
            compactedGlyphs[i] = macGlyphs[i + extra];
            if (SkUTF16_IsHighSurrogate(src[i + extra])) {
                ++extra;
            }
        }
    }

    if (allEncoded) {
        return glyphCount;
    }

    // If we got false, then we need to manually look for first failure.
    for (int i = 0; i < glyphCount; ++i) {
        if (0 == compactedGlyphs[i]) {
            return i;
        }
    }
    // Odd to get here, as we expected CT to have returned true up front.
    return glyphCount;
}

int SkTypeface_Mac::onCountGlyphs() const {
    return SkToInt(CTFontGetGlyphCount(fFontRef));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static bool find_desc_str(CTFontDescriptorRef desc, CFStringRef name, SkString* value) {
    AutoCFRelease<CFStringRef> ref((CFStringRef)CTFontDescriptorCopyAttribute(desc, name));
    if (nullptr == ref.get()) {
        return false;
    }
    CFStringToSkString(ref, value);
    return true;
}

#include "SkFontMgr.h"

static inline int sqr(int value) {
    SkASSERT(SkAbs32(value) < 0x7FFF);  // check for overflow
    return value * value;
}

// We normalize each axis (weight, width, italic) to be base-900
static int compute_metric(const SkFontStyle& a, const SkFontStyle& b) {
    return sqr(a.weight() - b.weight()) +
           sqr((a.width() - b.width()) * 100) +
           sqr((a.slant() != b.slant()) * 900);
}

static SkTypeface* createFromDesc(CTFontDescriptorRef desc) {
    AutoCFRelease<CTFontRef> ctFont(CTFontCreateWithFontDescriptor(desc, 0, nullptr));
    if (!ctFont) {
        return nullptr;
    }

    SkTypeface* face = SkTypefaceCache::FindByProcAndRef(find_by_CTFontRef, (void*)ctFont.get());
    if (face) {
        return face;
    }

    face = NewFromFontRef(ctFont.release(), nullptr, false);
    SkTypefaceCache::Add(face);
    return face;
}

class SkFontStyleSet_Mac : public SkFontStyleSet {
public:
    SkFontStyleSet_Mac(CTFontDescriptorRef desc)
        : fArray(CTFontDescriptorCreateMatchingFontDescriptors(desc, nullptr))
        , fCount(0) {
        if (nullptr == fArray) {
            fArray = CFArrayCreate(nullptr, nullptr, 0, nullptr);
        }
        fCount = SkToInt(CFArrayGetCount(fArray));
    }

    virtual ~SkFontStyleSet_Mac() {
        CFRelease(fArray);
    }

    int count() override {
        return fCount;
    }

    void getStyle(int index, SkFontStyle* style, SkString* name) override {
        SkASSERT((unsigned)index < (unsigned)fCount);
        CTFontDescriptorRef desc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fArray, index);
        if (style) {
            *style = fontstyle_from_descriptor(desc);
        }
        if (name) {
            if (!find_desc_str(desc, kCTFontStyleNameAttribute, name)) {
                name->reset();
            }
        }
    }

    SkTypeface* createTypeface(int index) override {
        SkASSERT((unsigned)index < (unsigned)CFArrayGetCount(fArray));
        CTFontDescriptorRef desc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fArray, index);

        return createFromDesc(desc);
    }

    SkTypeface* matchStyle(const SkFontStyle& pattern) override {
        if (0 == fCount) {
            return nullptr;
        }
        return createFromDesc(findMatchingDesc(pattern));
    }

private:
    CFArrayRef  fArray;
    int         fCount;

    CTFontDescriptorRef findMatchingDesc(const SkFontStyle& pattern) const {
        int bestMetric = SK_MaxS32;
        CTFontDescriptorRef bestDesc = nullptr;

        for (int i = 0; i < fCount; ++i) {
            CTFontDescriptorRef desc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fArray, i);
            int metric = compute_metric(pattern, fontstyle_from_descriptor(desc));
            if (0 == metric) {
                return desc;
            }
            if (metric < bestMetric) {
                bestMetric = metric;
                bestDesc = desc;
            }
        }
        SkASSERT(bestDesc);
        return bestDesc;
    }
};

class SkFontMgr_Mac : public SkFontMgr {
    CFArrayRef  fNames;
    int         fCount;

    CFStringRef stringAt(int index) const {
        SkASSERT((unsigned)index < (unsigned)fCount);
        return (CFStringRef)CFArrayGetValueAtIndex(fNames, index);
    }

    static SkFontStyleSet* CreateSet(CFStringRef cfFamilyName) {
        AutoCFRelease<CFMutableDictionaryRef> cfAttr(
                 CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                           &kCFTypeDictionaryKeyCallBacks,
                                           &kCFTypeDictionaryValueCallBacks));

        CFDictionaryAddValue(cfAttr, kCTFontFamilyNameAttribute, cfFamilyName);

        AutoCFRelease<CTFontDescriptorRef> desc(
                                CTFontDescriptorCreateWithAttributes(cfAttr));
        return new SkFontStyleSet_Mac(desc);
    }

public:
    SkFontMgr_Mac()
        : fNames(SkCTFontManagerCopyAvailableFontFamilyNames())
        , fCount(fNames ? SkToInt(CFArrayGetCount(fNames)) : 0) {}

    virtual ~SkFontMgr_Mac() {
        CFSafeRelease(fNames);
    }

protected:
    int onCountFamilies() const override {
        return fCount;
    }

    void onGetFamilyName(int index, SkString* familyName) const override {
        if ((unsigned)index < (unsigned)fCount) {
            CFStringToSkString(this->stringAt(index), familyName);
        } else {
            familyName->reset();
        }
    }

    SkFontStyleSet* onCreateStyleSet(int index) const override {
        if ((unsigned)index >= (unsigned)fCount) {
            return nullptr;
        }
        return CreateSet(this->stringAt(index));
    }

    SkFontStyleSet* onMatchFamily(const char familyName[]) const override {
        AutoCFRelease<CFStringRef> cfName(make_CFString(familyName));
        return CreateSet(cfName);
    }

    virtual SkTypeface* onMatchFamilyStyle(const char familyName[],
                                           const SkFontStyle& fontStyle) const override {
        SkAutoTUnref<SkFontStyleSet> sset(this->matchFamily(familyName));
        return sset->matchStyle(fontStyle);
    }

    virtual SkTypeface* onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                                    const char* bcp47[], int bcp47Count,
                                                    SkUnichar character) const override {
        return nullptr;
    }

    virtual SkTypeface* onMatchFaceStyle(const SkTypeface* familyMember,
                                         const SkFontStyle&) const override {
        return nullptr;
    }

    SkTypeface* onCreateFromData(SkData* data, int ttcIndex) const override {
        AutoCFRelease<CGDataProviderRef> pr(SkCreateDataProviderFromData(data));
        if (nullptr == pr) {
            return nullptr;
        }
        return create_from_dataProvider(pr);
    }

    SkTypeface* onCreateFromStream(SkStreamAsset* stream, int ttcIndex) const override {
        AutoCFRelease<CGDataProviderRef> pr(SkCreateDataProviderFromStream(stream));
        if (nullptr == pr) {
            return nullptr;
        }
        return create_from_dataProvider(pr);
    }

    static CFNumberRef get_tag_for_name(CFStringRef name, CFArrayRef ctAxes) {
        CFIndex ctAxisCount = CFArrayGetCount(ctAxes);
        for (int i = 0; i < ctAxisCount; ++i) {
            CFTypeRef ctAxisInfo = CFArrayGetValueAtIndex(ctAxes, i);
            if (CFDictionaryGetTypeID() != CFGetTypeID(ctAxisInfo)) {
                return nullptr;
            }
            CFDictionaryRef ctAxisInfoDict = static_cast<CFDictionaryRef>(ctAxisInfo);

            CFTypeRef ctAxisName = CFDictionaryGetValue(ctAxisInfoDict,
                                                        kCTFontVariationAxisNameKey);
            if (!ctAxisName || CFGetTypeID(ctAxisName) != CFStringGetTypeID()) {
                return nullptr;
            }

            if (CFEqual(name, ctAxisName)) {
                CFTypeRef tag = CFDictionaryGetValue(ctAxisInfoDict,
                                                     kCTFontVariationAxisIdentifierKey);
                if (!tag || CFGetTypeID(tag) != CFNumberGetTypeID()) {
                    return nullptr;
                }
                return static_cast<CFNumberRef>(tag);
            }
        }
        return nullptr;
    }
    static CFDictionaryRef get_axes(CGFontRef cg, const FontParameters& params) {
        AutoCFRelease<CFArrayRef> cgAxes(CGFontCopyVariationAxes(cg));
        if (!cgAxes) {
            return nullptr;
        }
        CFIndex axisCount = CFArrayGetCount(cgAxes);

        // The CGFont variation data is keyed by name, and lacks the tag.
        // The CTFont variation data is keyed by tag, and also has the name.
        // We would like to work with CTFont variaitons, but creating a CTFont font with
        // CTFont variation dictionary runs into bugs. So use the CTFont variation data
        // to match names to tags to create the appropriate CGFont.
        AutoCFRelease<CTFontRef> ct(CTFontCreateWithGraphicsFont(cg, 0, nullptr, nullptr));
        AutoCFRelease<CFArrayRef> ctAxes(CTFontCopyVariationAxes(ct));
        if (!ctAxes || CFArrayGetCount(ctAxes) != axisCount) {
            return nullptr;
        }

        int paramAxisCount;
        const FontParameters::Axis* paramAxes = params.getAxes(&paramAxisCount);

        CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, axisCount,
                                                                &kCFTypeDictionaryKeyCallBacks,
                                                                &kCFTypeDictionaryValueCallBacks);
        for (int i = 0; i < axisCount; ++i) {
            CFTypeRef axisInfo = CFArrayGetValueAtIndex(cgAxes, i);
            if (CFDictionaryGetTypeID() != CFGetTypeID(axisInfo)) {
                return nullptr;
            }
            CFDictionaryRef axisInfoDict = static_cast<CFDictionaryRef>(axisInfo);

            CFTypeRef axisName = CFDictionaryGetValue(axisInfoDict, kCGFontVariationAxisName);
            if (!axisName || CFGetTypeID(axisName) != CFStringGetTypeID()) {
                return nullptr;
            }

            CFNumberRef tagNumber = get_tag_for_name(static_cast<CFStringRef>(axisName), ctAxes);
            if (!tagNumber) {
                // Could not find a tag to go with the name of this index.
                // This would be a bug in CG/CT.
                continue;
            }
            int64_t tagLong;
            if (!CFNumberGetValue(tagNumber, kCFNumberSInt64Type, &tagLong)) {
                return nullptr;
            }

            // The variation axes can be set to any value, but cg will effectively pin them.
            // Pin them here to normalize.
            CFTypeRef min = CFDictionaryGetValue(axisInfoDict, kCGFontVariationAxisMinValue);
            CFTypeRef max = CFDictionaryGetValue(axisInfoDict, kCGFontVariationAxisMaxValue);
            CFTypeRef def = CFDictionaryGetValue(axisInfoDict, kCGFontVariationAxisDefaultValue);
            if (!min || CFGetTypeID(min) != CFNumberGetTypeID() ||
                !max || CFGetTypeID(max) != CFNumberGetTypeID() ||
                !def || CFGetTypeID(def) != CFNumberGetTypeID())
            {
                return nullptr;
            }
            CFNumberRef minNumber = static_cast<CFNumberRef>(min);
            CFNumberRef maxNumber = static_cast<CFNumberRef>(max);
            CFNumberRef defNumber = static_cast<CFNumberRef>(def);
            double minDouble;
            double maxDouble;
            double defDouble;
            if (!CFNumberGetValue(minNumber, kCFNumberDoubleType, &minDouble) ||
                !CFNumberGetValue(maxNumber, kCFNumberDoubleType, &maxDouble) ||
                !CFNumberGetValue(defNumber, kCFNumberDoubleType, &defDouble))
            {
                return nullptr;
            }

            double value = defDouble;
            for (int j = 0; j < paramAxisCount; ++j) {
                if (paramAxes[j].fTag == tagLong) {
                    value = SkTPin(SkScalarToDouble(paramAxes[j].fStyleValue),minDouble,maxDouble);
                    break;
                }
            }
            CFNumberRef valueNumber = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType,
                                                     &value);
            CFDictionaryAddValue(dict, axisName, valueNumber);
            CFRelease(valueNumber);
        }
        return dict;
    }
    SkTypeface* onCreateFromStream(SkStreamAsset* s, const FontParameters& params) const override {
        AutoCFRelease<CGDataProviderRef> provider(SkCreateDataProviderFromStream(s));
        if (nullptr == provider) {
            return nullptr;
        }
        AutoCFRelease<CGFontRef> cg(CGFontCreateWithDataProvider(provider));
        if (nullptr == cg) {
            return nullptr;
        }

        AutoCFRelease<CFDictionaryRef> cgVariations(get_axes(cg, params));
        // The CGFontRef returned by CGFontCreateCopyWithVariations when the passed CGFontRef was
        // created from a data provider does not appear to have any ownership of the underlying
        // data. The original CGFontRef must be kept alive until the copy will no longer be used.
        AutoCFRelease<CGFontRef> cgVariant;
        if (cgVariations) {
            cgVariant.reset(CGFontCreateCopyWithVariations(cg, cgVariations));
        } else {
            cgVariant.reset(cg.release());
        }

        CTFontRef ct = CTFontCreateWithGraphicsFont(cgVariant, 0, nullptr, nullptr);
        if (!ct) {
            return nullptr;
        }
        return NewFromFontRef(ct, cg.release(), true);
    }

    static CFDictionaryRef get_axes(CGFontRef cg, SkFontData* fontData) {
        AutoCFRelease<CFArrayRef> cgAxes(CGFontCopyVariationAxes(cg));
        if (!cgAxes) {
            return nullptr;
        }

        CFIndex axisCount = CFArrayGetCount(cgAxes);
        if (0 == axisCount || axisCount != fontData->getAxisCount()) {
            return nullptr;
        }

        CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, axisCount,
                                                                &kCFTypeDictionaryKeyCallBacks,
                                                                &kCFTypeDictionaryValueCallBacks);
        for (int i = 0; i < fontData->getAxisCount(); ++i) {
            CFTypeRef axisInfo = CFArrayGetValueAtIndex(cgAxes, i);
            if (CFDictionaryGetTypeID() != CFGetTypeID(axisInfo)) {
                return nullptr;
            }
            CFDictionaryRef axisInfoDict = static_cast<CFDictionaryRef>(axisInfo);

            CFTypeRef axisName = CFDictionaryGetValue(axisInfoDict, kCGFontVariationAxisName);
            if (!axisName || CFGetTypeID(axisName) != CFStringGetTypeID()) {
                return nullptr;
            }

            // The variation axes can be set to any value, but cg will effectively pin them.
            // Pin them here to normalize.
            CFTypeRef min = CFDictionaryGetValue(axisInfoDict, kCGFontVariationAxisMinValue);
            CFTypeRef max = CFDictionaryGetValue(axisInfoDict, kCGFontVariationAxisMaxValue);
            if (!min || CFGetTypeID(min) != CFNumberGetTypeID() ||
                !max || CFGetTypeID(max) != CFNumberGetTypeID())
            {
                return nullptr;
            }
            CFNumberRef minNumber = static_cast<CFNumberRef>(min);
            CFNumberRef maxNumber = static_cast<CFNumberRef>(max);
            double minDouble;
            double maxDouble;
            if (!CFNumberGetValue(minNumber, kCFNumberDoubleType, &minDouble) ||
                !CFNumberGetValue(maxNumber, kCFNumberDoubleType, &maxDouble))
            {
                return nullptr;
            }
            double value = SkTPin(SkFixedToDouble(fontData->getAxis()[i]), minDouble, maxDouble);
            CFNumberRef valueNumber = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType,
                                                     &value);

            CFDictionaryAddValue(dict, axisName, valueNumber);
            CFRelease(valueNumber);
        }
        return dict;
    }
    SkTypeface* onCreateFromFontData(SkFontData* data) const override {
        SkAutoTDelete<SkFontData> fontData(data);
        SkStreamAsset* stream = fontData->detachStream();
        AutoCFRelease<CGDataProviderRef> provider(SkCreateDataProviderFromStream(stream));
        if (nullptr == provider) {
            return nullptr;
        }
        AutoCFRelease<CGFontRef> cg(CGFontCreateWithDataProvider(provider));
        if (nullptr == cg) {
            return nullptr;
        }

        AutoCFRelease<CFDictionaryRef> cgVariations(get_axes(cg, fontData));
        // The CGFontRef returned by CGFontCreateCopyWithVariations when the passed CGFontRef was
        // created from a data provider does not appear to have any ownership of the underlying
        // data. The original CGFontRef must be kept alive until the copy will no longer be used.
        AutoCFRelease<CGFontRef> cgVariant;
        if (cgVariations) {
            cgVariant.reset(CGFontCreateCopyWithVariations(cg, cgVariations));
        } else {
            cgVariant.reset(cg.release());
        }

        CTFontRef ct = CTFontCreateWithGraphicsFont(cgVariant, 0, nullptr, nullptr);
        if (!ct) {
            return nullptr;
        }
        return NewFromFontRef(ct, cg.release(), true);
    }

    SkTypeface* onCreateFromFile(const char path[], int ttcIndex) const override {
        AutoCFRelease<CGDataProviderRef> pr(CGDataProviderCreateWithFilename(path));
        if (nullptr == pr) {
            return nullptr;
        }
        return create_from_dataProvider(pr);
    }

    SkTypeface* onLegacyCreateTypeface(const char familyName[], SkFontStyle style) const override {
        if (familyName) {
            familyName = map_css_names(familyName);
        }

        if (!familyName || !*familyName) {
            familyName = FONT_DEFAULT_NAME;
        }

        SkTypeface* face = NewFromName(familyName, style);
        if (face) {
            return face;
        }

        return SkSafeRef(GetDefaultFace());
    }
};

///////////////////////////////////////////////////////////////////////////////

SkFontMgr* SkFontMgr::Factory() { return new SkFontMgr_Mac; }

#endif//defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
