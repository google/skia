
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <vector>
#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreText/CoreText.h>
#include <CoreText/CTFontManager.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "SkFontHost.h"
#include "SkCGUtils.h"
#include "SkColorPriv.h"
#include "SkDescriptor.h"
#include "SkEndian.h"
#include "SkFontDescriptor.h"
#include "SkFloatingPoint.h"
#include "SkGlyph.h"
#include "SkMaskGamma.h"
#include "SkSFNTHeader.h"
#include "SkOTTable_glyf.h"
#include "SkOTTable_head.h"
#include "SkOTTable_hhea.h"
#include "SkOTTable_loca.h"
#include "SkOTUtils.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkString.h"
#include "SkStream.h"
#include "SkThread.h"
#include "SkTypeface_mac.h"
#include "SkUtils.h"
#include "SkTypefaceCache.h"
#include "SkFontMgr.h"
#include "SkUtils.h"

//#define HACK_COLORGLYPHS

class SkScalerContext_Mac;

// CTFontManagerCopyAvailableFontFamilyNames() is not always available, so we
// provide a wrapper here that will return an empty array if need be.
static CFArrayRef SkCTFontManagerCopyAvailableFontFamilyNames() {
#ifdef SK_BUILD_FOR_IOS
    return CFArrayCreate(NULL, NULL, 0, NULL);
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
    explicit AutoCFRelease(CFRef cfRef = NULL) : fCFRef(cfRef) { }
    ~AutoCFRelease() { CFSafeRelease(fCFRef); }

    void reset(CFRef that = NULL) {
        CFSafeRetain(that);
        CFSafeRelease(fCFRef);
        fCFRef = that;
    }

    AutoCFRelease& operator =(CFRef that) {
        reset(that);
        return *this;
    }

    operator CFRef() const { return fCFRef; }
    CFRef get() const { return fCFRef; }

    CFRef* operator&() { SkASSERT(fCFRef == NULL); return &fCFRef; }
private:
    CFRef fCFRef;
};

static CFStringRef make_CFString(const char str[]) {
    return CFStringCreateWithCString(NULL, str, kCFStringEncodingUTF8);
}

template<typename T> class AutoCGTable : SkNoncopyable {
public:
    AutoCGTable(CGFontRef font)
    //Undocumented: the tag parameter in this call is expected in machine order and not BE order.
    : fCFData(CGFontCopyTableForTag(font, SkSetFourByteTag(T::TAG0, T::TAG1, T::TAG2, T::TAG3)))
    , fData(fCFData ? reinterpret_cast<const T*>(CFDataGetBytePtr(fCFData)) : NULL)
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
                             size_t width, size_t height, size_t rowBytes) {
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

// The calls to support subpixel are present in 10.5, but are not included in
// the 10.5 SDK. The needed calls have been extracted from the 10.6 SDK and are
// included below. To verify that CGContextSetShouldSubpixelQuantizeFonts, for
// instance, is present in the 10.5 CoreGraphics libary, use:
//   cd /Developer/SDKs/MacOSX10.5.sdk/System/Library/Frameworks/
//   cd ApplicationServices.framework/Frameworks/CoreGraphics.framework/
//   nm CoreGraphics | grep CGContextSetShouldSubpixelQuantizeFonts

#if !defined(MAC_OS_X_VERSION_10_6) || (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_6)
CG_EXTERN void CGContextSetAllowsFontSmoothing(CGContextRef context, bool value);
CG_EXTERN void CGContextSetAllowsFontSubpixelPositioning(CGContextRef context, bool value);
CG_EXTERN void CGContextSetShouldSubpixelPositionFonts(CGContextRef context, bool value);
CG_EXTERN void CGContextSetAllowsFontSubpixelQuantization(CGContextRef context, bool value);
CG_EXTERN void CGContextSetShouldSubpixelQuantizeFonts(CGContextRef context, bool value);
#endif

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
    return SkMask::kLCD16_Format == format || SkMask::kLCD32_Format == format;
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
        return cgFloat;
    } else {
        SkASSERT(sizeof(CGFloat) == sizeof(double));
        return SkDoubleToScalar(cgFloat);
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
#define BITMAP_INFO_GRAY (kCGImageAlphaNone)

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
    Offscreen();

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

Offscreen::Offscreen() : fRGBSpace(NULL), fCG(NULL),
                         fDoAA(false), fDoLCD(false) {
    fSize.set(0, 0);
}

///////////////////////////////////////////////////////////////////////////////

static SkTypeface::Style computeStyleBits(CTFontRef font, bool* isFixedPitch) {
    unsigned style = SkTypeface::kNormal;
    CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(font);

    if (traits & kCTFontBoldTrait) {
        style |= SkTypeface::kBold;
    }
    if (traits & kCTFontItalicTrait) {
        style |= SkTypeface::kItalic;
    }
    if (isFixedPitch) {
        *isFixedPitch = (traits & kCTFontMonoSpaceTrait) != 0;
    }
    return (SkTypeface::Style)style;
}

static SkFontID CTFontRef_to_SkFontID(CTFontRef fontRef) {
    SkFontID id = 0;
// CTFontGetPlatformFont and ATSFontRef are not supported on iOS, so we have to
// bracket this to be Mac only.
#ifdef SK_BUILD_FOR_MAC
    ATSFontRef ats = CTFontGetPlatformFont(fontRef, NULL);
    id = (SkFontID)ats;
    if (id != 0) {
        id &= 0x3FFFFFFF; // make top two bits 00
        return id;
    }
#endif
    // CTFontGetPlatformFont returns NULL if the font is local
    // (e.g., was created by a CSS3 @font-face rule).
    AutoCFRelease<CGFontRef> cgFont(CTFontCopyGraphicsFont(fontRef, NULL));
    AutoCGTable<SkOTTableHead> headTable(cgFont);
    if (headTable.fData) {
        id = (SkFontID) headTable->checksumAdjustment;
        id = (id & 0x3FFFFFFF) | 0x40000000; // make top two bits 01
    }
    // well-formed fonts have checksums, but as a last resort, use the pointer.
    if (id == 0) {
        id = (SkFontID) (uintptr_t) fontRef;
        id = (id & 0x3FFFFFFF) | 0x80000000; // make top two bits 10
    }
    return id;
}

static SkFontStyle stylebits2fontstyle(SkTypeface::Style styleBits) {
    return SkFontStyle((styleBits & SkTypeface::kBold)
                           ? SkFontStyle::kBold_Weight
                           : SkFontStyle::kNormal_Weight,
                       SkFontStyle::kNormal_Width,
                       (styleBits & SkTypeface::kItalic)
                           ? SkFontStyle::kItalic_Slant
                           : SkFontStyle::kUpright_Slant);
}

#define WEIGHT_THRESHOLD    ((SkFontStyle::kNormal_Weight + SkFontStyle::kBold_Weight)/2)

static SkTypeface::Style fontstyle2stylebits(const SkFontStyle& fs) {
    unsigned style = 0;
    if (fs.width() >= WEIGHT_THRESHOLD) {
        style |= SkTypeface::kBold;
    }
    if (fs.isItalic()) {
        style |= SkTypeface::kItalic;
    }
    return (SkTypeface::Style)style;
}

class SkTypeface_Mac : public SkTypeface {
public:
    SkTypeface_Mac(SkTypeface::Style style, SkFontID fontID, bool isFixedPitch,
                   CTFontRef fontRef, const char name[])
        : SkTypeface(style, fontID, isFixedPitch)
        , fName(name)
        , fFontRef(fontRef) // caller has already called CFRetain for us
        , fFontStyle(stylebits2fontstyle(style))
    {
        SkASSERT(fontRef);
    }

    SkTypeface_Mac(const SkFontStyle& fs, SkFontID fontID, bool isFixedPitch,
                   CTFontRef fontRef, const char name[])
        : SkTypeface(fontstyle2stylebits(fs), fontID, isFixedPitch)
        , fName(name)
        , fFontRef(fontRef) // caller has already called CFRetain for us
        , fFontStyle(fs)
    {
        SkASSERT(fontRef);
    }

    SkString fName;
    AutoCFRelease<CTFontRef> fFontRef;
    SkFontStyle fFontStyle;

protected:
    friend class SkFontHost;    // to access our protected members for deprecated methods

    virtual int onGetUPEM() const SK_OVERRIDE;
    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE;
    virtual SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const SK_OVERRIDE;
    virtual int onGetTableTags(SkFontTableTag tags[]) const SK_OVERRIDE;
    virtual size_t onGetTableData(SkFontTableTag, size_t offset,
                                  size_t length, void* data) const SK_OVERRIDE;
    virtual SkScalerContext* onCreateScalerContext(const SkDescriptor*) const SK_OVERRIDE;
    virtual void onFilterRec(SkScalerContextRec*) const SK_OVERRIDE;
    virtual void onGetFontDescriptor(SkFontDescriptor*, bool*) const SK_OVERRIDE;
    virtual SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
                                SkAdvancedTypefaceMetrics::PerGlyphInfo,
                                const uint32_t*, uint32_t) const SK_OVERRIDE;
    virtual int onCharsToGlyphs(const void* chars, Encoding, uint16_t glyphs[],
                                int glyphCount) const SK_OVERRIDE;
    virtual int onCountGlyphs() const SK_OVERRIDE;

private:

    typedef SkTypeface INHERITED;
};

static SkTypeface* NewFromFontRef(CTFontRef fontRef, const char name[]) {
    SkASSERT(fontRef);
    bool isFixedPitch;
    SkTypeface::Style style = computeStyleBits(fontRef, &isFixedPitch);
    SkFontID fontID = CTFontRef_to_SkFontID(fontRef);

    return new SkTypeface_Mac(style, fontID, isFixedPitch, fontRef, name);
}

static SkTypeface* NewFromName(const char familyName[], SkTypeface::Style theStyle) {
    CTFontRef ctFont = NULL;

    CTFontSymbolicTraits ctFontTraits = 0;
    if (theStyle & SkTypeface::kBold) {
        ctFontTraits |= kCTFontBoldTrait;
    }
    if (theStyle & SkTypeface::kItalic) {
        ctFontTraits |= kCTFontItalicTrait;
    }

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

    // Create the font
    if (cfFontName != NULL && cfFontTraits != NULL && cfAttributes != NULL && cfTraits != NULL) {
        CFDictionaryAddValue(cfTraits, kCTFontSymbolicTrait, cfFontTraits);

        CFDictionaryAddValue(cfAttributes, kCTFontFamilyNameAttribute, cfFontName);
        CFDictionaryAddValue(cfAttributes, kCTFontTraitsAttribute, cfTraits);

        AutoCFRelease<CTFontDescriptorRef> ctFontDesc(
                CTFontDescriptorCreateWithAttributes(cfAttributes));

        if (ctFontDesc != NULL) {
            ctFont = CTFontCreateWithFontDescriptor(ctFontDesc, 0, NULL);
        }
    }

    return ctFont ? NewFromFontRef(ctFont, familyName) : NULL;
}

static SkTypeface* GetDefaultFace() {
    SK_DECLARE_STATIC_MUTEX(gMutex);
    SkAutoMutexAcquire ma(gMutex);

    static SkTypeface* gDefaultFace;

    if (NULL == gDefaultFace) {
        gDefaultFace = NewFromName(FONT_DEFAULT_NAME, SkTypeface::kNormal);
        SkTypefaceCache::Add(gDefaultFace, SkTypeface::kNormal);
    }
    return gDefaultFace;
}

///////////////////////////////////////////////////////////////////////////////

extern CTFontRef SkTypeface_GetCTFontRef(const SkTypeface* face);
CTFontRef SkTypeface_GetCTFontRef(const SkTypeface* face) {
    const SkTypeface_Mac* macface = (const SkTypeface_Mac*)face;
    return macface ? macface->fFontRef.get() : NULL;
}

/*  This function is visible on the outside. It first searches the cache, and if
 *  not found, returns a new entry (after adding it to the cache).
 */
SkTypeface* SkCreateTypefaceFromCTFont(CTFontRef fontRef) {
    SkFontID fontID = CTFontRef_to_SkFontID(fontRef);
    SkTypeface* face = SkTypefaceCache::FindByID(fontID);
    if (face) {
        face->ref();
    } else {
        face = NewFromFontRef(fontRef, NULL);
        SkTypefaceCache::Add(face, face->style());
        // NewFromFontRef doesn't retain the parameter, but the typeface it
        // creates does release it in its destructor, so we balance that with
        // a retain call here.
        CFRetain(fontRef);
    }
    SkASSERT(face->getRefCnt() > 1);
    return face;
}

struct NameStyleRec {
    const char*         fName;
    SkTypeface::Style   fStyle;
};

static bool FindByNameStyle(SkTypeface* face, SkTypeface::Style style,
                            void* ctx) {
    const SkTypeface_Mac* mface = reinterpret_cast<SkTypeface_Mac*>(face);
    const NameStyleRec* rec = reinterpret_cast<const NameStyleRec*>(ctx);

    return rec->fStyle == style && mface->fName.equals(rec->fName);
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

static SkTypeface* create_typeface(const SkTypeface* familyFace,
                                   const char familyName[],
                                   SkTypeface::Style style) {
    if (familyName) {
        familyName = map_css_names(familyName);
    }

    // Clone an existing typeface
    // TODO: only clone if style matches the familyFace's style...
    if (familyName == NULL && familyFace != NULL) {
        familyFace->ref();
        return const_cast<SkTypeface*>(familyFace);
    }

    if (!familyName || !*familyName) {
        familyName = FONT_DEFAULT_NAME;
    }

    NameStyleRec rec = { familyName, style };
    SkTypeface* face = SkTypefaceCache::FindByProcAndRef(FindByNameStyle, &rec);

    if (NULL == face) {
        face = NewFromName(familyName, style);
        if (face) {
            SkTypefaceCache::Add(face, style);
        } else {
            face = GetDefaultFace();
            face->ref();
        }
    }
    return face;
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
    SkScalerContext_Mac(SkTypeface_Mac*, const SkDescriptor*);

protected:
    unsigned generateGlyphCount(void) SK_OVERRIDE;
    uint16_t generateCharToGlyph(SkUnichar uni) SK_OVERRIDE;
    void generateAdvance(SkGlyph* glyph) SK_OVERRIDE;
    void generateMetrics(SkGlyph* glyph) SK_OVERRIDE;
    void generateImage(const SkGlyph& glyph) SK_OVERRIDE;
    void generatePath(const SkGlyph& glyph, SkPath* path) SK_OVERRIDE;
    void generateFontMetrics(SkPaint::FontMetrics* mX, SkPaint::FontMetrics* mY) SK_OVERRIDE;

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
    AutoCFRelease<CTFontRef> fCTFont;

    /** Vertical variant of fCTFont.
     *
     *  CT vertical metrics are pre-rotated (in em space, before transform) 90deg clock-wise.
     *  This makes kCTFontDefaultOrientation dangerous, because the metrics from
     *  kCTFontHorizontalOrientation are in a different space from kCTFontVerticalOrientation.
     *  Use fCTVerticalFont with kCTFontVerticalOrientation to get metrics in the same space.
     */
    AutoCFRelease<CTFontRef> fCTVerticalFont;

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

SkScalerContext_Mac::SkScalerContext_Mac(SkTypeface_Mac* typeface,
                                         const SkDescriptor* desc)
        : INHERITED(typeface, desc)
        , fFBoundingBoxes()
        , fFBoundingBoxesGlyphOffset(0)
        , fGeneratedFBoundingBoxes(false)
        , fDoSubPosition(SkToBool(fRec.fFlags & kSubpixelPositioning_Flag))
        , fVertical(SkToBool(fRec.fFlags & kVertical_Flag))

{
    CTFontRef ctFont = typeface->fFontRef.get();
    CFIndex numGlyphs = CTFontGetGlyphCount(ctFont);
    SkASSERT(numGlyphs >= 1 && numGlyphs <= 0xFFFF);
    fGlyphCount = SkToU16(numGlyphs);

    fRec.getSingleMatrix(&fFUnitMatrix);
    CGAffineTransform transform = MatrixToCGAffineTransform(fFUnitMatrix);

    AutoCFRelease<CTFontDescriptorRef> ctFontDesc;
    if (fVertical) {
        AutoCFRelease<CFMutableDictionaryRef> cfAttributes(CFDictionaryCreateMutable(
                kCFAllocatorDefault, 0,
                &kCFTypeDictionaryKeyCallBacks,
                &kCFTypeDictionaryValueCallBacks));
        if (cfAttributes) {
            CTFontOrientation ctOrientation = kCTFontVerticalOrientation;
            AutoCFRelease<CFNumberRef> cfVertical(CFNumberCreate(
                    kCFAllocatorDefault, kCFNumberSInt32Type, &ctOrientation));
            CFDictionaryAddValue(cfAttributes, kCTFontOrientationAttribute, cfVertical);
            ctFontDesc = CTFontDescriptorCreateWithAttributes(cfAttributes);
        }
    }
    // Since our matrix includes everything, we pass 1 for size.
    fCTFont = CTFontCreateCopyWithAttributes(ctFont, 1, &transform, ctFontDesc);
    fCGFont = CTFontCopyGraphicsFont(fCTFont, NULL);
    if (fVertical) {
        CGAffineTransform rotateLeft = CGAffineTransformMake(0, -1, 1, 0, 0, 0);
        transform = CGAffineTransformConcat(rotateLeft, transform);
        fCTVerticalFont = CTFontCreateCopyWithAttributes(ctFont, 1, &transform, NULL);
    }

    SkScalar emPerFUnit = SkScalarInvert(SkIntToScalar(CGFontGetUnitsPerEm(fCGFont)));
    fFUnitMatrix.preScale(emPerFUnit, -emPerFUnit);
}

CGRGBPixel* Offscreen::getCG(const SkScalerContext_Mac& context, const SkGlyph& glyph,
                             CGGlyph glyphID, size_t* rowBytesPtr,
                             bool generateA8FromLCD) {
    if (!fRGBSpace) {
        //It doesn't appear to matter what color space is specified.
        //Regular blends and antialiased text are always (s*a + d*(1-a))
        //and smoothed text is always g=2.0.
        fRGBSpace = CGColorSpaceCreateDeviceRGB();
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
        fCG = CGBitmapContextCreate(image, fSize.fWidth, fSize.fHeight, 8,
                                    rowBytes, fRGBSpace, BITMAP_INFO_RGB);

        // skia handles quantization itself, so we disable this for cg to get
        // full fractional data from them.
        CGContextSetAllowsFontSubpixelQuantization(fCG, false);
        CGContextSetShouldSubpixelQuantizeFonts(fCG, false);

        CGContextSetTextDrawingMode(fCG, kCGTextFill);
        CGContextSetFont(fCG, context.fCGFont);
        CGContextSetFontSize(fCG, 1 /*CTFontGetSize(context.fCTFont)*/);
        CGContextSetTextMatrix(fCG, CTFontGetMatrix(context.fCTFont));

        // Because CG always draws from the horizontal baseline,
        // if there is a non-integral translation from the horizontal origin to the vertical origin,
        // then CG cannot draw the glyph in the correct location without subpixel positioning.
        CGContextSetAllowsFontSubpixelPositioning(fCG, context.fDoSubPosition || context.fVertical);
        CGContextSetShouldSubpixelPositionFonts(fCG, context.fDoSubPosition || context.fVertical);

        // Draw white on black to create mask.
        // TODO: Draw black on white and invert, CG has a special case codepath.
        CGContextSetGrayFillColor(fCG, 1.0f, 1.0f);

        // force our checks below to happen
        fDoAA = !doAA;
        fDoLCD = !doLCD;
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

    // erase to black
    sk_memset_rect32(image, 0, glyph.fWidth, glyph.fHeight, rowBytes);

    float subX = 0;
    float subY = 0;
    if (context.fDoSubPosition) {
        subX = SkFixedToFloat(glyph.getSubXFixed());
        subY = SkFixedToFloat(glyph.getSubYFixed());
    }

    // CGContextShowGlyphsAtPoint always draws using the horizontal baseline origin.
    if (context.fVertical) {
        SkPoint offset;
        context.getVerticalOffset(glyphID, &offset);
        subX += offset.fX;
        subY += offset.fY;
    }

    CGContextShowGlyphsAtPoint(fCG, -glyph.fLeft + subX,
                               glyph.fTop + glyph.fHeight - subY,
                               &glyphID, 1);

    SkASSERT(rowBytesPtr);
    *rowBytesPtr = rowBytes;
    return image;
}

void SkScalerContext_Mac::getVerticalOffset(CGGlyph glyphID, SkPoint* offset) const {
    // Snow Leopard returns cgVertOffset in completely un-transformed FUnits (em space, y up).
    // Lion and Leopard return cgVertOffset in CG units (pixels, y up).
    CGSize cgVertOffset;
    CTFontGetVerticalTranslationsForGlyphs(fCTFont, &glyphID, &cgVertOffset, 1);

    SkPoint skVertOffset = { CGToScalar(cgVertOffset.width), CGToScalar(cgVertOffset.height) };
    if (isSnowLeopard()) {
        // From FUnits (em space, y up) to SkGlyph units (pixels, y down).
        fFUnitMatrix.mapPoints(&skVertOffset, 1);
    } else {
        // From CG units (pixels, y up) to SkGlyph units (pixels, y down).
        skVertOffset.fY = -skVertOffset.fY;
    }

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
        return NULL != fFBoundingBoxes.get();
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
    const CGGlyph cgGlyph = (CGGlyph) glyph->getGlyphID(fBaseGlyphCount);
    glyph->zeroMetrics();

    // The following block produces cgAdvance in CG units (pixels, y up).
    CGSize cgAdvance;
    if (fVertical) {
        CTFontGetAdvancesForGlyphs(fCTVerticalFont, kCTFontVerticalOrientation,
                                   &cgGlyph, &cgAdvance, 1);
    } else {
        CTFontGetAdvancesForGlyphs(fCTFont, kCTFontHorizontalOrientation,
                                   &cgGlyph, &cgAdvance, 1);
    }
    glyph->fAdvanceX =  SkFloatToFixed_Check(cgAdvance.width);
    glyph->fAdvanceY = -SkFloatToFixed_Check(cgAdvance.height);

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

        // BUG?
        // 0x200B (zero-advance space) seems to return a huge (garbage) bounds, when
        // it should be empty. So, if we see a zero-advance, we check if it has an
        // empty path or not, and if so, we jam the bounds to 0. Hopefully a zero-advance
        // is rare, so we won't incur a big performance cost for this extra check.
        if (0 == cgAdvance.width && 0 == cgAdvance.height) {
            AutoCFRelease<CGPathRef> path(CTFontCreatePathForGlyph(fCTFont, cgGlyph, NULL));
            if (NULL == path || CGPathIsEmpty(path)) {
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

#ifdef HACK_COLORGLYPHS
    glyph->fMaskFormat = SkMask::kARGB32_Format;
#endif
}

#include "SkColorPriv.h"

static void build_power_table(uint8_t table[], float ee) {
    for (int i = 0; i < 256; i++) {
        float x = i / 255.f;
        x = sk_float_pow(x, ee);
        int xx = SkScalarRoundToInt(x * 255);
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
        build_power_table(gTableCoreGraphicsSmoothing, 2.0f);
        gInited = true;
    }
    return gTableCoreGraphicsSmoothing;
}

static void cgpixels_to_bits(uint8_t dst[], const CGRGBPixel src[], int count) {
    while (count > 0) {
        uint8_t mask = 0;
        for (int i = 7; i >= 0; --i) {
            mask |= (CGRGBPixel_getAlpha(*src++) >> 7) << i;
            if (0 == --count) {
                break;
            }
        }
        *dst++ = mask;
    }
}

template<bool APPLY_PREBLEND>
static inline uint8_t rgb_to_a8(CGRGBPixel rgb, const uint8_t* table8) {
    U8CPU r = (rgb >> 16) & 0xFF;
    U8CPU g = (rgb >>  8) & 0xFF;
    U8CPU b = (rgb >>  0) & 0xFF;
    return sk_apply_lut_if<APPLY_PREBLEND>(SkComputeLuminance(r, g, b), table8);
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
    U8CPU r = sk_apply_lut_if<APPLY_PREBLEND>((rgb >> 16) & 0xFF, tableR);
    U8CPU g = sk_apply_lut_if<APPLY_PREBLEND>((rgb >>  8) & 0xFF, tableG);
    U8CPU b = sk_apply_lut_if<APPLY_PREBLEND>((rgb >>  0) & 0xFF, tableB);
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

template<bool APPLY_PREBLEND>
static inline uint32_t rgb_to_lcd32(CGRGBPixel rgb, const uint8_t* tableR,
                                                    const uint8_t* tableG,
                                                    const uint8_t* tableB) {
    U8CPU r = sk_apply_lut_if<APPLY_PREBLEND>((rgb >> 16) & 0xFF, tableR);
    U8CPU g = sk_apply_lut_if<APPLY_PREBLEND>((rgb >>  8) & 0xFF, tableG);
    U8CPU b = sk_apply_lut_if<APPLY_PREBLEND>((rgb >>  0) & 0xFF, tableB);
    return SkPackARGB32(0xFF, r, g, b);
}
template<bool APPLY_PREBLEND>
static void rgb_to_lcd32(const CGRGBPixel* SK_RESTRICT cgPixels, size_t cgRowBytes, const SkGlyph& glyph,
                         const uint8_t* tableR, const uint8_t* tableG, const uint8_t* tableB) {
    const int width = glyph.fWidth;
    size_t dstRB = glyph.rowBytes();
    uint32_t* SK_RESTRICT dst = (uint32_t*)glyph.fImage;
    for (int y = 0; y < glyph.fHeight; y++) {
        for (int i = 0; i < width; i++) {
            dst[i] = rgb_to_lcd32<APPLY_PREBLEND>(cgPixels[i], tableR, tableG, tableB);
        }
        cgPixels = (CGRGBPixel*)((char*)cgPixels + cgRowBytes);
        dst = (uint32_t*)((char*)dst + dstRB);
    }
}

#ifdef HACK_COLORGLYPHS
// hack to colorize the output for testing kARGB32_Format
static SkPMColor cgpixels_to_pmcolor(CGRGBPixel rgb, const SkGlyph& glyph,
                                     int x, int y) {
    U8CPU r = (rgb >> 16) & 0xFF;
    U8CPU g = (rgb >>  8) & 0xFF;
    U8CPU b = (rgb >>  0) & 0xFF;
    unsigned a = SkComputeLuminance(r, g, b);

    // compute gradient from x,y
    r = x * 255 / glyph.fWidth;
    g = 0;
    b = (glyph.fHeight - y) * 255 / glyph.fHeight;
    return SkPreMultiplyARGB(a, r, g, b);    // red
}
#endif

template <typename T> T* SkTAddByteOffset(T* ptr, size_t byteOffset) {
    return (T*)((char*)ptr + byteOffset);
}

void SkScalerContext_Mac::generateImage(const SkGlyph& glyph) {
    CGGlyph cgGlyph = (CGGlyph) glyph.getGlyphID(fBaseGlyphCount);

    // FIXME: lcd smoothed un-hinted rasterization unsupported.
    bool generateA8FromLCD = fRec.getHinting() != SkPaint::kNo_Hinting;

    // Draw the glyph
    size_t cgRowBytes;
    CGRGBPixel* cgPixels = fOffscreen.getCG(*this, glyph, cgGlyph, &cgRowBytes, generateA8FromLCD);
    if (cgPixels == NULL) {
        return;
    }

    //TODO: see if drawing black on white and inverting is faster (at least in
    //lcd case) as core graphics appears to have special case code for drawing
    //black text.

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
            addr = SkTAddByteOffset(addr, cgRowBytes);
        }
    }

    // Convert glyph to mask
    switch (glyph.fMaskFormat) {
        case SkMask::kLCD32_Format: {
            if (fPreBlend.isApplicable()) {
                rgb_to_lcd32<true>(cgPixels, cgRowBytes, glyph,
                                   fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            } else {
                rgb_to_lcd32<false>(cgPixels, cgRowBytes, glyph,
                                    fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            }
        } break;
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
#ifdef HACK_COLORGLYPHS
        case SkMask::kARGB32_Format: {
            const int width = glyph.fWidth;
            size_t dstRB = glyph.rowBytes();
            SkPMColor* dst = (SkPMColor*)glyph.fImage;
            for (int y = 0; y < glyph.fHeight; y++) {
                for (int x = 0; x < width; ++x) {
                    dst[x] = cgpixels_to_pmcolor(cgPixels[x], glyph, x, y);
                }
                cgPixels = (CGRGBPixel*)((char*)cgPixels + cgRowBytes);
                dst = (SkPMColor*)((char*)dst + dstRB);
            }
        } break;
#endif
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
    CTFontRef font = fCTFont;
    SkScalar scaleX = SK_Scalar1;
    SkScalar scaleY = SK_Scalar1;

    /*
     *  For subpixel positioning, we want to return an unhinted outline, so it
     *  can be positioned nicely at fractional offsets. However, we special-case
     *  if the baseline of the (horizontal) text is axis-aligned. In those cases
     *  we want to retain hinting in the direction orthogonal to the baseline.
     *  e.g. for horizontal baseline, we want to retain hinting in Y.
     *  The way we remove hinting is to scale the font by some value (4) in that
     *  direction, ask for the path, and then scale the path back down.
     */
    if (fRec.fFlags & SkScalerContext::kSubpixelPositioning_Flag) {
        SkMatrix m;
        fRec.getSingleMatrix(&m);

        // start out by assuming that we want no hining in X and Y
        scaleX = scaleY = kScaleForSubPixelPositionHinting;
        // now see if we need to restore hinting for axis-aligned baselines
        switch (SkComputeAxisAlignmentForHText(m)) {
            case kX_SkAxisAlignment:
                scaleY = SK_Scalar1; // want hinting in the Y direction
                break;
            case kY_SkAxisAlignment:
                scaleX = SK_Scalar1; // want hinting in the X direction
                break;
            default:
                break;
        }

        CGAffineTransform xform = MatrixToCGAffineTransform(m, scaleX, scaleY);
        // need to release font when we're done
        font = CTFontCreateCopyWithAttributes(fCTFont, 1, &xform, NULL);
    }

    CGGlyph cgGlyph = (CGGlyph)glyph.getGlyphID(fBaseGlyphCount);
    AutoCFRelease<CGPathRef> cgPath(CTFontCreatePathForGlyph(font, cgGlyph, NULL));

    path->reset();
    if (cgPath != NULL) {
        CGPathApply(cgPath, path, SkScalerContext_Mac::CTPathElement);
    }

    if (fDoSubPosition) {
        SkMatrix m;
        m.setScale(SkScalarInvert(scaleX), SkScalarInvert(scaleY));
        path->transform(m);
        // balance the call to CTFontCreateCopyWithAttributes
        CFSafeRelease(font);
    }
    if (fVertical) {
        SkPoint offset;
        getVerticalOffset(cgGlyph, &offset);
        path->offset(offset.fX, offset.fY);
    }
}

void SkScalerContext_Mac::generateFontMetrics(SkPaint::FontMetrics* mx,
                                              SkPaint::FontMetrics* my) {
    CGRect theBounds = CTFontGetBoundingBox(fCTFont);

    SkPaint::FontMetrics theMetrics;
    theMetrics.fTop          = CGToScalar(-CGRectGetMaxY_inline(theBounds));
    theMetrics.fAscent       = CGToScalar(-CTFontGetAscent(fCTFont));
    theMetrics.fDescent      = CGToScalar( CTFontGetDescent(fCTFont));
    theMetrics.fBottom       = CGToScalar(-CGRectGetMinY_inline(theBounds));
    theMetrics.fLeading      = CGToScalar( CTFontGetLeading(fCTFont));
    theMetrics.fAvgCharWidth = CGToScalar( CGRectGetWidth_inline(theBounds));
    theMetrics.fXMin         = CGToScalar( CGRectGetMinX_inline(theBounds));
    theMetrics.fXMax         = CGToScalar( CGRectGetMaxX_inline(theBounds));
    theMetrics.fXHeight      = CGToScalar( CTFontGetXHeight(fCTFont));

    if (mx != NULL) {
        *mx = theMetrics;
    }
    if (my != NULL) {
        *my = theMetrics;
    }
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

// Returns NULL on failure
// Call must still manage its ownership of provider
static SkTypeface* create_from_dataProvider(CGDataProviderRef provider) {
    AutoCFRelease<CGFontRef> cg(CGFontCreateWithDataProvider(provider));
    if (NULL == cg) {
        return NULL;
    }
    CTFontRef ct = CTFontCreateWithGraphicsFont(cg, 0, NULL, NULL);
    return cg ? SkCreateTypefaceFromCTFont(ct) : NULL;
}

// Web fonts added to the the CTFont registry do not return their character set.
// Iterate through the font in this case. The existing caller caches the result,
// so the performance impact isn't too bad.
static void populate_glyph_to_unicode_slow(CTFontRef ctFont, CFIndex glyphCount,
                                           SkTDArray<SkUnichar>* glyphToUnicode) {
    glyphToUnicode->setCount(glyphCount);
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
    glyphToUnicode->setCount(glyphCount);
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

static bool getWidthAdvance(CTFontRef ctFont, int gId, int16_t* data) {
    CGSize advance;
    advance.width = 0;
    CGGlyph glyph = gId;
    CTFontGetAdvancesForGlyphs(ctFont, kCTFontHorizontalOrientation, &glyph, &advance, 1);
    *data = sk_float_round2int(advance.width);
    return true;
}

// we might move this into our CGUtils...
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
        SkAdvancedTypefaceMetrics::PerGlyphInfo perGlyphInfo,
        const uint32_t* glyphIDs,
        uint32_t glyphIDsCount) const {

    CTFontRef originalCTFont = fFontRef.get();
    AutoCFRelease<CTFontRef> ctFont(CTFontCreateCopyWithAttributes(
            originalCTFont, CTFontGetUnitsPerEm(originalCTFont), NULL, NULL));
    SkAdvancedTypefaceMetrics* info = new SkAdvancedTypefaceMetrics;

    {
        AutoCFRelease<CFStringRef> fontName(CTFontCopyPostScriptName(ctFont));
        CFStringToSkString(fontName, &info->fFontName);
    }

    info->fMultiMaster = false;
    CFIndex glyphCount = CTFontGetGlyphCount(ctFont);
    info->fLastGlyphID = SkToU16(glyphCount - 1);
    info->fEmSize = CTFontGetUnitsPerEm(ctFont);

    if (perGlyphInfo & SkAdvancedTypefaceMetrics::kToUnicode_PerGlyphInfo) {
        populate_glyph_to_unicode(ctFont, glyphCount, &info->fGlyphToUnicode);
    }

    info->fStyle = 0;

    // If it's not a truetype font, mark it as 'other'. Assume that TrueType
    // fonts always have both glyf and loca tables. At the least, this is what
    // sfntly needs to subset the font. CTFontCopyAttribute() does not always
    // succeed in determining this directly.
    if (!this->getTableSize('glyf') || !this->getTableSize('loca')) {
        info->fType = SkAdvancedTypefaceMetrics::kOther_Font;
        info->fItalicAngle = 0;
        info->fAscent = 0;
        info->fDescent = 0;
        info->fStemV = 0;
        info->fCapHeight = 0;
        info->fBBox = SkIRect::MakeEmpty();
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

    if (false) { // TODO: haven't figured out how to know if font is embeddable
        // (information is in the OS/2 table)
        info->fType = SkAdvancedTypefaceMetrics::kNotEmbeddable_Font;
    } else if (perGlyphInfo & SkAdvancedTypefaceMetrics::kHAdvance_PerGlyphInfo) {
        if (info->fStyle & SkAdvancedTypefaceMetrics::kFixedPitch_Style) {
            skia_advanced_typeface_metrics_utils::appendRange(&info->fGlyphWidths, 0);
            info->fGlyphWidths->fAdvance.append(1, &min_width);
            skia_advanced_typeface_metrics_utils::finishRange(info->fGlyphWidths.get(), 0,
                        SkAdvancedTypefaceMetrics::WidthRange::kDefault);
        } else {
            info->fGlyphWidths.reset(
                skia_advanced_typeface_metrics_utils::getAdvanceData(ctFont.get(),
                               glyphCount,
                               glyphIDs,
                               glyphIDsCount,
                               &getWidthAdvance));
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

SkStream* SkTypeface_Mac::onOpenStream(int* ttcIndex) const {
    SK_SFNT_ULONG fontType = get_font_type_tag(this);
    if (0 == fontType) {
        return NULL;
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
        entry->offset = SkEndian_SwapBE32(dataPtr - dataStart);
        entry->logicalLength = SkEndian_SwapBE32(tableSize);

        dataPtr += (tableSize + 3) & ~3;
        ++entry;
    }

    return stream;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int SkTypeface_Mac::onGetUPEM() const {
    AutoCFRelease<CGFontRef> cgFont(CTFontCopyGraphicsFont(fFontRef, NULL));
    return CGFontGetUnitsPerEm(cgFont);
}

SkTypeface::LocalizedStrings* SkTypeface_Mac::onCreateFamilyNameIterator() const {
    SkTypeface::LocalizedStrings* nameIter =
        SkOTUtils::LocalizedStrings_NameTable::CreateForFamilyNames(*this);
    if (NULL == nameIter) {
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
    if (NULL == data) {
        AutoCFRelease<CGFontRef> cgFont(CTFontCopyGraphicsFont(ctFont, NULL));
        data = CGFontCopyTableForTag(cgFont, tag);
    }
    return data;
}

int SkTypeface_Mac::onGetTableTags(SkFontTableTag tags[]) const {
    AutoCFRelease<CFArrayRef> cfArray(CTFontCopyAvailableTables(fFontRef,
                                                kCTFontTableOptionNoOptions));
    if (NULL == cfArray) {
        return 0;
    }
    int count = CFArrayGetCount(cfArray);
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
    if (NULL == srcData) {
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

SkScalerContext* SkTypeface_Mac::onCreateScalerContext(const SkDescriptor* desc) const {
    return new SkScalerContext_Mac(const_cast<SkTypeface_Mac*>(this), desc);
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
                                  SkScalerContext::kAutohinting_Flag  |
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

    // Unhinted A8 masks (those not derived from LCD masks) must respect SK_GAMMA_APPLY_TO_A8.
    // All other masks can use regular gamma.
    if (SkMask::kA8_Format == rec->fMaskFormat && SkPaint::kNo_Hinting == hinting) {
#ifndef SK_GAMMA_APPLY_TO_A8
        rec->ignorePreBlend();
#endif
    } else {
        //CoreGraphics dialates smoothed text as needed.
        rec->setContrast(0);
    }
}

// we take ownership of the ref
static const char* get_str(CFStringRef ref, SkString* str) {
    CFStringToSkString(ref, str);
    CFSafeRelease(ref);
    return str->c_str();
}

void SkTypeface_Mac::onGetFontDescriptor(SkFontDescriptor* desc,
                                         bool* isLocalStream) const {
    SkString tmpStr;

    desc->setFamilyName(get_str(CTFontCopyFamilyName(fFontRef), &tmpStr));
    desc->setFullName(get_str(CTFontCopyFullName(fFontRef), &tmpStr));
    desc->setPostscriptName(get_str(CTFontCopyPostScriptName(fFontRef), &tmpStr));
    // TODO: need to add support for local-streams (here and openStream)
    *isLocalStream = false;
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
            srcCount = utf16 - src;
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
            srcCount = utf16 - src;
            break;
        }
    }

    // If glyphs is NULL, CT still needs glyph storage for finding the first failure.
    // Also, if there are any non-bmp code points, the provided 'glyphs' storage will be inadequate.
    SkAutoSTMalloc<1024, uint16_t> glyphStorage;
    uint16_t* macGlyphs = glyphs;
    if (NULL == macGlyphs || srcCount > glyphCount) {
        macGlyphs = glyphStorage.reset(srcCount);
    }

    bool allEncoded = CTFontGetGlyphsForCharacters(fFontRef, src, macGlyphs, srcCount);

    // If there were any non-bmp, then copy and compact.
    // If 'glyphs' is NULL, then compact glyphStorage in-place.
    // If all are bmp and 'glyphs' is non-NULL, 'glyphs' already contains the compact glyphs.
    // If some are non-bmp and 'glyphs' is non-NULL, copy and compact into 'glyphs'.
    uint16_t* compactedGlyphs = glyphs;
    if (NULL == compactedGlyphs) {
        compactedGlyphs = macGlyphs;
    }
    if (srcCount > glyphCount) {
        int extra = 0;
        for (int i = 0; i < glyphCount; ++i) {
            if (SkUTF16_IsHighSurrogate(src[i + extra])) {
                ++extra;
            }
            compactedGlyphs[i] = macGlyphs[i + extra];
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
    return CTFontGetGlyphCount(fFontRef);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if 1

static bool find_desc_str(CTFontDescriptorRef desc, CFStringRef name, SkString* value) {
    AutoCFRelease<CFStringRef> ref((CFStringRef)CTFontDescriptorCopyAttribute(desc, name));
    if (NULL == ref.get()) {
        return false;
    }
    CFStringToSkString(ref, value);
    return true;
}

static bool find_dict_float(CFDictionaryRef dict, CFStringRef name, float* value) {
    CFNumberRef num;
    return CFDictionaryGetValueIfPresent(dict, name, (const void**)&num)
    && CFNumberIsFloatType(num)
    && CFNumberGetValue(num, kCFNumberFloatType, value);
}

#include "SkFontMgr.h"

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

static inline int sqr(int value) {
    SkASSERT(SkAbs32(value) < 0x7FFF);  // check for overflow
    return value * value;
}

// We normalize each axis (weight, width, italic) to be base-900
static int compute_metric(const SkFontStyle& a, const SkFontStyle& b) {
    return sqr(a.weight() - b.weight()) +
           sqr((a.width() - b.width()) * 100) +
           sqr((a.isItalic() != b.isItalic()) * 900);
}

static SkFontStyle desc2fontstyle(CTFontDescriptorRef desc) {
    AutoCFRelease<CFDictionaryRef> dict(
        (CFDictionaryRef)CTFontDescriptorCopyAttribute(desc,
                                                       kCTFontTraitsAttribute));
    if (NULL == dict.get()) {
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

struct NameFontStyleRec {
    SkString    fFamilyName;
    SkFontStyle fFontStyle;
};

static bool nameFontStyleProc(SkTypeface* face, SkTypeface::Style,
                              void* ctx) {
    SkTypeface_Mac* macFace = (SkTypeface_Mac*)face;
    const NameFontStyleRec* rec = (const NameFontStyleRec*)ctx;

    return macFace->fFontStyle == rec->fFontStyle &&
           macFace->fName == rec->fFamilyName;
}

static SkTypeface* createFromDesc(CFStringRef cfFamilyName,
                                  CTFontDescriptorRef desc) {
    NameFontStyleRec rec;
    CFStringToSkString(cfFamilyName, &rec.fFamilyName);
    rec.fFontStyle = desc2fontstyle(desc);

    SkTypeface* face = SkTypefaceCache::FindByProcAndRef(nameFontStyleProc,
                                                         &rec);
    if (face) {
        return face;
    }

    AutoCFRelease<CFDictionaryRef> fontFamilyNameDictionary(
        CFDictionaryCreate(kCFAllocatorDefault,
                           (const void**)&kCTFontFamilyNameAttribute, (const void**)&cfFamilyName,
                           1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    AutoCFRelease<CTFontDescriptorRef> fontDescriptor(
        CTFontDescriptorCreateWithAttributes(fontFamilyNameDictionary));
    AutoCFRelease<CTFontRef> ctNamed(CTFontCreateWithFontDescriptor(fontDescriptor, 0, NULL));
    CTFontRef ctFont = CTFontCreateCopyWithAttributes(ctNamed, 1, NULL, desc);
    if (NULL == ctFont) {
        return NULL;
    }

    SkString str;
    CFStringToSkString(cfFamilyName, &str);

    bool isFixedPitch;
    (void)computeStyleBits(ctFont, &isFixedPitch);
    SkFontID fontID = CTFontRef_to_SkFontID(ctFont);

    face = SkNEW_ARGS(SkTypeface_Mac, (rec.fFontStyle, fontID, isFixedPitch,
                                       ctFont, str.c_str()));
    SkTypefaceCache::Add(face, face->style());
    return face;
}

class SkFontStyleSet_Mac : public SkFontStyleSet {
public:
    SkFontStyleSet_Mac(CFStringRef familyName, CTFontDescriptorRef desc)
        : fArray(CTFontDescriptorCreateMatchingFontDescriptors(desc, NULL))
        , fFamilyName(familyName)
        , fCount(0) {
        CFRetain(familyName);
        if (NULL == fArray) {
            fArray = CFArrayCreate(NULL, NULL, 0, NULL);
        }
        fCount = CFArrayGetCount(fArray);
    }

    virtual ~SkFontStyleSet_Mac() {
        CFRelease(fArray);
        CFRelease(fFamilyName);
    }

    virtual int count() SK_OVERRIDE {
        return fCount;
    }

    virtual void getStyle(int index, SkFontStyle* style,
                          SkString* name) SK_OVERRIDE {
        SkASSERT((unsigned)index < (unsigned)fCount);
        CTFontDescriptorRef desc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fArray, index);
        if (style) {
            *style = desc2fontstyle(desc);
        }
        if (name) {
            if (!find_desc_str(desc, kCTFontStyleNameAttribute, name)) {
                name->reset();
            }
        }
    }

    virtual SkTypeface* createTypeface(int index) SK_OVERRIDE {
        SkASSERT((unsigned)index < (unsigned)CFArrayGetCount(fArray));
        CTFontDescriptorRef desc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fArray, index);

        return createFromDesc(fFamilyName, desc);
    }

    virtual SkTypeface* matchStyle(const SkFontStyle& pattern) SK_OVERRIDE {
        if (0 == fCount) {
            return NULL;
        }
        return createFromDesc(fFamilyName, findMatchingDesc(pattern));
    }

private:
    CFArrayRef  fArray;
    CFStringRef fFamilyName;
    int         fCount;

    CTFontDescriptorRef findMatchingDesc(const SkFontStyle& pattern) const {
        int bestMetric = SK_MaxS32;
        CTFontDescriptorRef bestDesc = NULL;

        for (int i = 0; i < fCount; ++i) {
            CTFontDescriptorRef desc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fArray, i);
            int metric = compute_metric(pattern, desc2fontstyle(desc));
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
    int         fCount;
    CFArrayRef  fNames;

    CFStringRef stringAt(int index) const {
        SkASSERT((unsigned)index < (unsigned)fCount);
        return (CFStringRef)CFArrayGetValueAtIndex(fNames, index);
    }

    void lazyInit() {
        if (NULL == fNames) {
            fNames = SkCTFontManagerCopyAvailableFontFamilyNames();
            fCount = fNames ? CFArrayGetCount(fNames) : 0;
        }
    }

    static SkFontStyleSet* CreateSet(CFStringRef cfFamilyName) {
        AutoCFRelease<CFMutableDictionaryRef> cfAttr(
                 CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                           &kCFTypeDictionaryKeyCallBacks,
                                           &kCFTypeDictionaryValueCallBacks));

        CFDictionaryAddValue(cfAttr, kCTFontFamilyNameAttribute, cfFamilyName);

        AutoCFRelease<CTFontDescriptorRef> desc(
                                CTFontDescriptorCreateWithAttributes(cfAttr));
        return SkNEW_ARGS(SkFontStyleSet_Mac, (cfFamilyName, desc));
    }

public:
    SkFontMgr_Mac() : fCount(0), fNames(NULL) {}

    virtual ~SkFontMgr_Mac() {
        CFSafeRelease(fNames);
    }

protected:
    virtual int onCountFamilies() SK_OVERRIDE {
        this->lazyInit();
        return fCount;
    }

    virtual void onGetFamilyName(int index, SkString* familyName) SK_OVERRIDE {
        this->lazyInit();
        if ((unsigned)index < (unsigned)fCount) {
            CFStringToSkString(this->stringAt(index), familyName);
        } else {
            familyName->reset();
        }
    }

    virtual SkFontStyleSet* onCreateStyleSet(int index) SK_OVERRIDE {
        this->lazyInit();
        if ((unsigned)index >= (unsigned)fCount) {
            return NULL;
        }
        return CreateSet(this->stringAt(index));
    }

    virtual SkFontStyleSet* onMatchFamily(const char familyName[]) SK_OVERRIDE {
        AutoCFRelease<CFStringRef> cfName(make_CFString(familyName));
        return CreateSet(cfName);
    }

    virtual SkTypeface* onMatchFamilyStyle(const char familyName[],
                                           const SkFontStyle&) SK_OVERRIDE {
        return NULL;
    }

    virtual SkTypeface* onMatchFaceStyle(const SkTypeface* familyMember,
                                         const SkFontStyle&) SK_OVERRIDE {
        return NULL;
    }

    virtual SkTypeface* onCreateFromData(SkData* data,
                                         int ttcIndex) SK_OVERRIDE {
        AutoCFRelease<CGDataProviderRef> pr(SkCreateDataProviderFromData(data));
        if (NULL == pr) {
            return NULL;
        }
        return create_from_dataProvider(pr);
    }

    virtual SkTypeface* onCreateFromStream(SkStream* stream,
                                           int ttcIndex) SK_OVERRIDE {
        AutoCFRelease<CGDataProviderRef> pr(SkCreateDataProviderFromStream(stream));
        if (NULL == pr) {
            return NULL;
        }
        return create_from_dataProvider(pr);
    }

    virtual SkTypeface* onCreateFromFile(const char path[],
                                         int ttcIndex) SK_OVERRIDE {
        AutoCFRelease<CGDataProviderRef> pr(CGDataProviderCreateWithFilename(path));
        if (NULL == pr) {
            return NULL;
        }
        return create_from_dataProvider(pr);
    }

    virtual SkTypeface* onLegacyCreateTypeface(const char familyName[],
                                               unsigned styleBits) SK_OVERRIDE {
        return create_typeface(NULL, familyName, (SkTypeface::Style)styleBits);
    }
};

///////////////////////////////////////////////////////////////////////////////

SkFontMgr* SkFontMgr::Factory() {
    return SkNEW(SkFontMgr_Mac);
}
#endif
