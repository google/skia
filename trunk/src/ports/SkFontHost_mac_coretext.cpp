
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
#include "SkPaint.h"
#include "SkPath.h"
#include "SkString.h"
#include "SkStream.h"
#include "SkThread.h"
#include "SkTypeface_mac.h"
#include "SkUtils.h"
#include "SkTypefaceCache.h"

class SkScalerContext_Mac;

static void CFSafeRelease(CFTypeRef obj) {
    if (obj) {
        CFRelease(obj);
    }
}

class AutoCFRelease : SkNoncopyable {
public:
    AutoCFRelease(CFTypeRef obj) : fObj(obj) {}
    ~AutoCFRelease() { CFSafeRelease(fObj); }

private:
    CFTypeRef fObj;
};

// inline versions of these rect helpers

static bool CGRectIsEmpty_inline(const CGRect& rect) {
    return rect.size.width <= 0 || rect.size.height <= 0;
}

static void CGRectInset_inline(CGRect* rect, CGFloat dx, CGFloat dy) {
    rect->origin.x += dx;
    rect->origin.y += dy;
    rect->size.width -= dx * 2;
    rect->size.height -= dy * 2;
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

static void sk_memset_rect32(uint32_t* ptr, uint32_t value, size_t width,
                             size_t height, size_t rowBytes) {
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

// Potentially this should be made (1) public (2) optimized when width is small.
// Also might want 16 and 32 bit version
//
#if 0 // UNUSED
static void sk_memset_rect(void* ptr, U8CPU byte, size_t width, size_t height,
                           size_t rowBytes) {
    uint8_t* dst = (uint8_t*)ptr;
    while (height) {
        memset(dst, byte, width);
        dst += rowBytes;
        height -= 1;
    }
}
#endif

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

#if !defined(MAC_OS_X_VERSION_10_6) || \
        MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_6
    CG_EXTERN void CGContextSetAllowsFontSmoothing(CGContextRef context,
        bool allowsFontSmoothing);
    CG_EXTERN void CGContextSetAllowsFontSubpixelPositioning(
        CGContextRef context,
        bool allowsFontSubpixelPositioning);
    CG_EXTERN void CGContextSetShouldSubpixelPositionFonts(CGContextRef context,
        bool shouldSubpixelPositionFonts);
    CG_EXTERN void CGContextSetAllowsFontSubpixelQuantization(
        CGContextRef context,
        bool allowsFontSubpixelQuantization);
    CG_EXTERN void CGContextSetShouldSubpixelQuantizeFonts(
        CGContextRef context,
        bool shouldSubpixelQuantizeFonts);
#endif

static const char FONT_DEFAULT_NAME[]           = "Lucida Sans";

// see Source/WebKit/chromium/base/mac/mac_util.mm DarwinMajorVersionInternal
// for original source
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

static bool isLeopard() {
    return darwinVersion() == 9;
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
        return SkFloatToScalar(cgFloat);
    } else {
        SkASSERT(sizeof(CGFloat) == sizeof(double));
        return SkDoubleToScalar(cgFloat);
    }
}

static CGAffineTransform MatrixToCGAffineTransform(const SkMatrix& matrix,
                                                   float sx = 1, float sy = 1) {
    return CGAffineTransformMake(ScalarToCG(matrix[SkMatrix::kMScaleX]) * sx,
                                 -ScalarToCG(matrix[SkMatrix::kMSkewY]) * sy,
                                 -ScalarToCG(matrix[SkMatrix::kMSkewX]) * sx,
                                 ScalarToCG(matrix[SkMatrix::kMScaleY]) * sy,
                                 ScalarToCG(matrix[SkMatrix::kMTransX]) * sx,
                                 ScalarToCG(matrix[SkMatrix::kMTransY]) * sy);
}

static SkScalar getFontScale(CGFontRef cgFont) {
    int unitsPerEm = CGFontGetUnitsPerEm(cgFont);
    return SkScalarInvert(SkIntToScalar(unitsPerEm));
}

///////////////////////////////////////////////////////////////////////////////

#define BITMAP_INFO_RGB     (kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Host)
#define BITMAP_INFO_GRAY    (kCGImageAlphaNone)

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
    int rgb = 0;
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    CGContextRef cgContext = CGBitmapContextCreate(&rgb, 1, 1, 8, 4, colorspace,
                                                   BITMAP_INFO_RGB);
    CGContextSelectFont(cgContext, "Helvetica", 16, kCGEncodingMacRoman);
    CGContextSetShouldSmoothFonts(cgContext, true);
    CGContextSetShouldAntialias(cgContext, true);
    CGContextSetTextDrawingMode(cgContext, kCGTextFill);
    CGContextSetGrayFillColor(  cgContext, 1, 1);
    CGContextShowTextAtPoint(cgContext, -1, 0, "|", 1);
    CFSafeRelease(colorspace);
    CFSafeRelease(cgContext);
    int r = (rgb >> 16) & 0xFF;
    int g = (rgb >>  8) & 0xFF;
    int b = (rgb >>  0) & 0xFF;
    gSupportsLCD = r != g || r != b;
    return (bool) gSupportsLCD;
}

class Offscreen {
public:
    Offscreen();
    ~Offscreen();

    CGRGBPixel* getCG(const SkScalerContext_Mac& context, const SkGlyph& glyph,
                      CGGlyph glyphID, size_t* rowBytesPtr);

private:
    enum {
        kSize = 32 * 32 * sizeof(CGRGBPixel)
    };
    SkAutoSMalloc<kSize> fImageStorage;
    CGColorSpaceRef fRGBSpace;

    // cached state
    CGContextRef    fCG;
    SkISize         fSize;
    bool            fDoAA;
    bool            fDoLCD;

    static int RoundSize(int dimension) {
        return SkNextPow2(dimension);
    }
};

Offscreen::Offscreen() : fRGBSpace(NULL), fCG(NULL) {
    fSize.set(0,0);
}

Offscreen::~Offscreen() {
    CFSafeRelease(fCG);
    CFSafeRelease(fRGBSpace);
}

///////////////////////////////////////////////////////////////////////////////

static SkTypeface::Style computeStyleBits(CTFontRef font, bool* isMonospace) {
    unsigned style = SkTypeface::kNormal;
    CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(font);

    if (traits & kCTFontBoldTrait) {
        style |= SkTypeface::kBold;
    }
    if (traits & kCTFontItalicTrait) {
        style |= SkTypeface::kItalic;
    }
    if (isMonospace) {
        *isMonospace = (traits & kCTFontMonoSpaceTrait) != 0;
    }
    return (SkTypeface::Style)style;
}

class AutoCFDataRelease {
public:
    AutoCFDataRelease(CFDataRef obj) : fObj(obj) {}
    const uint16_t* getShortPtr() {
        return fObj ? (const uint16_t*) CFDataGetBytePtr(fObj) : NULL;
    }
    ~AutoCFDataRelease() { CFSafeRelease(fObj); }
private:
    CFDataRef fObj;
};

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
    CGFontRef cgFont = CTFontCopyGraphicsFont(fontRef, NULL);
    AutoCFDataRelease headRef(CGFontCopyTableForTag(cgFont, 'head'));
    const uint16_t* headData = headRef.getShortPtr();
    if (headData) {
        id = (SkFontID) (headData[4] | headData[5] << 16); // checksum
        id = (id & 0x3FFFFFFF) | 0x40000000; // make top two bits 01
    }
    // well-formed fonts have checksums, but as a last resort, use the pointer.
    if (id == 0) {
        id = (SkFontID) (uintptr_t) fontRef;
        id = (id & 0x3FFFFFFF) | 0x80000000; // make top two bits 10
    }
    CGFontRelease(cgFont);
    return id;
}

class SkTypeface_Mac : public SkTypeface {
public:
    SkTypeface_Mac(SkTypeface::Style style, SkFontID fontID, bool isMonospace,
                   CTFontRef fontRef, const char name[])
    : SkTypeface(style, fontID, isMonospace) {
        SkASSERT(fontRef);
        fFontRef = fontRef; // caller has already called CFRetain for us
        fName.set(name);
    }

    virtual ~SkTypeface_Mac() { CFRelease(fFontRef); }

    SkString    fName;
    CTFontRef   fFontRef;
};

static CTFontRef typeface_to_fontref(const SkTypeface* face) {
    const SkTypeface_Mac* macface = reinterpret_cast<const SkTypeface_Mac*>(face);
    return macface->fFontRef;
}

static SkTypeface* NewFromFontRef(CTFontRef fontRef, const char name[]) {
    SkASSERT(fontRef);
    bool isMonospace;
    SkTypeface::Style style = computeStyleBits(fontRef, &isMonospace);
    SkFontID fontID = CTFontRef_to_SkFontID(fontRef);

    return new SkTypeface_Mac(style, fontID, isMonospace, fontRef, name);
}

static SkTypeface* NewFromName(const char familyName[],
                               SkTypeface::Style theStyle) {
    CFMutableDictionaryRef      cfAttributes, cfTraits;
    CFNumberRef                 cfFontTraits;
    CTFontSymbolicTraits        ctFontTraits;
    CTFontDescriptorRef         ctFontDesc;
    CFStringRef                 cfFontName;
    CTFontRef                   ctFont;


    // Get the state we need
    ctFontDesc   = NULL;
    ctFont       = NULL;
    ctFontTraits = 0;

    if (theStyle & SkTypeface::kBold) {
        ctFontTraits |= kCTFontBoldTrait;
    }

    if (theStyle & SkTypeface::kItalic) {
        ctFontTraits |= kCTFontItalicTrait;
    }

    // Create the font info
    cfFontName   = CFStringCreateWithCString(NULL, familyName, kCFStringEncodingUTF8);
    cfFontTraits = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &ctFontTraits);
    cfAttributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    cfTraits     = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);


    // Create the font
    if (cfFontName != NULL && cfFontTraits != NULL && cfAttributes != NULL && cfTraits != NULL) {
        CFDictionaryAddValue(cfTraits, kCTFontSymbolicTrait, cfFontTraits);

        CFDictionaryAddValue(cfAttributes, kCTFontFamilyNameAttribute, cfFontName);
        CFDictionaryAddValue(cfAttributes, kCTFontTraitsAttribute,     cfTraits);

        ctFontDesc = CTFontDescriptorCreateWithAttributes(cfAttributes);
        if (ctFontDesc != NULL) {
            if (isLeopard()) {
                // CTFontCreateWithFontDescriptor on Leopard ignores the name
                CTFontRef ctNamed = CTFontCreateWithName(cfFontName, 1, NULL);
                ctFont = CTFontCreateCopyWithAttributes(ctNamed, 1, NULL,
                                                        ctFontDesc);
                CFSafeRelease(ctNamed);
            } else {
                ctFont = CTFontCreateWithFontDescriptor(ctFontDesc, 0, NULL);
            }
        }
    }

    CFSafeRelease(cfFontName);
    CFSafeRelease(cfFontTraits);
    CFSafeRelease(cfAttributes);
    CFSafeRelease(cfTraits);
    CFSafeRelease(ctFontDesc);

    return ctFont ? NewFromFontRef(ctFont, familyName) : NULL;
}

static CTFontRef GetFontRefFromFontID(SkFontID fontID) {
    SkTypeface_Mac* face = reinterpret_cast<SkTypeface_Mac*>(SkTypefaceCache::FindByID(fontID));
    return face ? face->fFontRef : 0;
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
    return macface ? macface->fFontRef : NULL;
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

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
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

static void flip(SkMatrix* matrix) {
    matrix->setSkewX(-matrix->getSkewX());
    matrix->setSkewY(-matrix->getSkewY());
}

///////////////////////////////////////////////////////////////////////////////

struct GlyphRect {
    int16_t fMinX;
    int16_t fMinY;
    int16_t fMaxX;
    int16_t fMaxY;
};

class SkScalerContext_Mac : public SkScalerContext {
public:
                                        SkScalerContext_Mac(const SkDescriptor* desc);
    virtual                            ~SkScalerContext_Mac(void);


protected:
    unsigned                            generateGlyphCount(void) SK_OVERRIDE;
    uint16_t                            generateCharToGlyph(SkUnichar uni) SK_OVERRIDE;
    void                                generateAdvance(SkGlyph* glyph) SK_OVERRIDE;
    void                                generateMetrics(SkGlyph* glyph) SK_OVERRIDE;
    void                                generateImage(const SkGlyph& glyph, SkMaskGamma::PreBlend* maskPreBlend) SK_OVERRIDE;
    void                                generatePath(const SkGlyph& glyph, SkPath* path) SK_OVERRIDE;
    void                                generateFontMetrics(SkPaint::FontMetrics* mX, SkPaint::FontMetrics* mY) SK_OVERRIDE;


private:
    static void                         CTPathElement(void *info, const CGPathElement *element);
    uint16_t                            getAdjustStart();
    void                                getVerticalOffset(CGGlyph glyphID, SkIPoint* offset) const;
    bool                                generateBBoxes();

private:
    CGAffineTransform                   fTransform;
    SkMatrix                            fUnitMatrix; // without font size
    SkMatrix                            fVerticalMatrix; // unit rotated
    SkMatrix                            fMatrix; // with font size
    SkMatrix                            fAdjustBadMatrix; // lion-specific fix
    Offscreen                           fOffscreen;
    CTFontRef                           fCTFont;
    CTFontRef                           fCTVerticalFont; // for vertical advance
    CGFontRef                           fCGFont;
    GlyphRect*                          fAdjustBad;
    uint16_t                            fAdjustStart;
    uint16_t                            fGlyphCount;
    bool                                fGeneratedBBoxes;
    bool                                fDoSubPosition;
    bool                                fVertical;

    friend class                        Offscreen;
};

SkScalerContext_Mac::SkScalerContext_Mac(const SkDescriptor* desc)
        : SkScalerContext(desc)
        , fCTVerticalFont(NULL)
        , fAdjustBad(NULL)
        , fAdjustStart(0)
        , fGeneratedBBoxes(false)
{
    CTFontRef ctFont = GetFontRefFromFontID(fRec.fFontID);
    CFIndex numGlyphs = CTFontGetGlyphCount(ctFont);

    // Get the state we need
    fRec.getSingleMatrix(&fMatrix);
    fUnitMatrix = fMatrix;

    // extract the font size out of the matrix, but leave the skewing for italic
    SkScalar reciprocal = SkScalarInvert(fRec.fTextSize);
    fUnitMatrix.preScale(reciprocal, reciprocal);

    SkASSERT(numGlyphs >= 1 && numGlyphs <= 0xFFFF);

    fTransform = MatrixToCGAffineTransform(fMatrix);

    CGAffineTransform transform;
    CGFloat unitFontSize;
    if (isLeopard()) {
        // passing 1 for pointSize to Leopard sets the font size to 1 pt.
        // pass the CoreText size explicitly
        transform = MatrixToCGAffineTransform(fUnitMatrix);
        unitFontSize = SkScalarToFloat(fRec.fTextSize);
    } else {
        // since our matrix includes everything, we pass 1 for pointSize
        transform = fTransform;
        unitFontSize = 1;
    }
    flip(&fUnitMatrix); // flip to fix up bounds later
    fVertical = SkToBool(fRec.fFlags & kVertical_Flag);
    CTFontDescriptorRef ctFontDesc = NULL;
    if (fVertical) {
        CFMutableDictionaryRef cfAttributes = CFDictionaryCreateMutable(
                kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
                &kCFTypeDictionaryValueCallBacks);
        if (cfAttributes) {
            CTFontOrientation ctOrientation = kCTFontVerticalOrientation;
            CFNumberRef cfVertical = CFNumberCreate(kCFAllocatorDefault,
                    kCFNumberSInt32Type, &ctOrientation);
            CFDictionaryAddValue(cfAttributes, kCTFontOrientationAttribute,
                    cfVertical);
            CFSafeRelease(cfVertical);
            ctFontDesc = CTFontDescriptorCreateWithAttributes(cfAttributes);
            CFRelease(cfAttributes);
        }
    }
    fCTFont = CTFontCreateCopyWithAttributes(ctFont, unitFontSize, &transform,
            ctFontDesc);
    CFSafeRelease(ctFontDesc);
    fCGFont = CTFontCopyGraphicsFont(fCTFont, NULL);
    if (fVertical) {
        CGAffineTransform rotateLeft = CGAffineTransformMake(0, -1, 1, 0, 0, 0);
        transform = CGAffineTransformConcat(rotateLeft, transform);
        fCTVerticalFont = CTFontCreateCopyWithAttributes(ctFont, unitFontSize,
                &transform, NULL);
        fVerticalMatrix = fUnitMatrix;
        if (isSnowLeopard()) {
            SkScalar scale = SkScalarMul(fRec.fTextSize, getFontScale(fCGFont));
            fVerticalMatrix.preScale(scale, scale);
        } else {
            fVerticalMatrix.preRotate(SkIntToScalar(90));
        }
        fVerticalMatrix.postScale(SK_Scalar1, -SK_Scalar1);
    }
    fGlyphCount = SkToU16(numGlyphs);
    fDoSubPosition = SkToBool(fRec.fFlags & kSubpixelPositioning_Flag);
}

SkScalerContext_Mac::~SkScalerContext_Mac() {
    delete[] fAdjustBad;
    CFSafeRelease(fCTFont);
    CFSafeRelease(fCTVerticalFont);
    CFSafeRelease(fCGFont);
}

CGRGBPixel* Offscreen::getCG(const SkScalerContext_Mac& context, const SkGlyph& glyph,
                             CGGlyph glyphID, size_t* rowBytesPtr) {
    if (!fRGBSpace) {
        //It doesn't appear to matter what color space is specified.
        //Regular blends and antialiased text are always (s*a + d*(1-a))
        //and smoothed text is always g=2.0.
        fRGBSpace = CGColorSpaceCreateDeviceRGB();
    }

    // default to kBW_Format
    bool doAA = false;
    bool doLCD = false;

    switch (glyph.fMaskFormat) {
        case SkMask::kLCD16_Format:
        case SkMask::kLCD32_Format:
        case SkMask::kA8_Format: //Draw A8 as LCD, then downsample
            doLCD = true;
            doAA = true;
            break;
        //case SkMask::kA8_Format:
        //    doLCD = false;
        //    doAA = true;
        //    break;
        default:
            break;
    }

    size_t rowBytes = fSize.fWidth * sizeof(CGRGBPixel);
    if (!fCG || fSize.fWidth < glyph.fWidth || fSize.fHeight < glyph.fHeight) {
        CFSafeRelease(fCG);
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
        CGContextSetFontSize(fCG, 1);
        CGContextSetTextMatrix(fCG, context.fTransform);

        CGContextSetAllowsFontSubpixelPositioning(fCG, context.fDoSubPosition);
        CGContextSetShouldSubpixelPositionFonts(fCG, context.fDoSubPosition);

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
    if (context.fVertical) {
        SkIPoint offset;
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

void SkScalerContext_Mac::getVerticalOffset(CGGlyph glyphID, SkIPoint* offset) const {
    CGSize vertOffset;
    CTFontGetVerticalTranslationsForGlyphs(fCTVerticalFont, &glyphID, &vertOffset, 1);
    const SkPoint trans = {CGToScalar(vertOffset.width),
                           CGToScalar(vertOffset.height)};
    SkPoint floatOffset;
    fVerticalMatrix.mapPoints(&floatOffset, &trans, 1);
    if (!isSnowLeopard()) {
    // SnowLeopard fails to apply the font's matrix to the vertical metrics,
    // but Lion and Leopard do. The unit matrix describes the font's matrix at
    // point size 1. There may be some way to avoid mapping here by setting up
    // fVerticalMatrix differently, but this works for now.
        fUnitMatrix.mapPoints(&floatOffset, 1);
    }
    offset->fX = SkScalarRound(floatOffset.fX);
    offset->fY = SkScalarRound(floatOffset.fY);
}

/* from http://developer.apple.com/fonts/TTRefMan/RM06/Chap6loca.html
 * There are two versions of this table, the short and the long. The version
 * used is specified in the Font Header ('head') table in the indexToLocFormat
 * field. The choice of long or short offsets is dependent on the maximum
 * possible offset distance.
 *
 * 'loca' short version: The actual local offset divided by 2 is stored.
 * 'loca' long version: The actual local offset is stored.
 *
 * The result is a offset into a table of 2 byte (16 bit) entries.
 */
static uint32_t getLocaTableEntry(const uint16_t*& locaPtr, int locaFormat) {
    uint32_t data = SkEndian_SwapBE16(*locaPtr++); // short
    if (locaFormat) {
        data = data << 15 | SkEndian_SwapBE16(*locaPtr++) >> 1; // long
    }
    return data;
}

// see http://developer.apple.com/fonts/TTRefMan/RM06/Chap6hhea.html
static uint16_t getNumLongMetrics(const uint16_t* hheaData) {
    const int kNumOfLongHorMetrics = 17;
    return SkEndian_SwapBE16(hheaData[kNumOfLongHorMetrics]);
}

// see http://developer.apple.com/fonts/TTRefMan/RM06/Chap6head.html
static int getLocaFormat(const uint16_t* headData) {
    const int kIndexToLocFormat = 25;
    return SkEndian_SwapBE16(headData[kIndexToLocFormat]);
}

uint16_t SkScalerContext_Mac::getAdjustStart() {
    if (fAdjustStart) {
        return fAdjustStart;
    }
    fAdjustStart = fGlyphCount; // fallback for all fonts
    AutoCFDataRelease hheaRef(CGFontCopyTableForTag(fCGFont, 'hhea'));
    const uint16_t* hheaData = hheaRef.getShortPtr();
    if (hheaData) {
        fAdjustStart = getNumLongMetrics(hheaData);
    }
    return fAdjustStart;
}

/*
 * Lion has a bug in CTFontGetBoundingRectsForGlyphs which returns a bad value
 * in theBounds.origin.x for fonts whose numOfLogHorMetrics is less than its
 * glyph count. This workaround reads the glyph bounds from the font directly.
 *
 * The table is computed only if the font is a TrueType font, if the glyph
 * value is >= fAdjustStart. (called only if fAdjustStart < fGlyphCount).
 *
 * TODO: A future optimization will compute fAdjustBad once per CGFont, and
 * compute fAdjustBadMatrix once per font context.
 */
bool SkScalerContext_Mac::generateBBoxes() {
    if (fGeneratedBBoxes) {
        return NULL != fAdjustBad;
    }
    fGeneratedBBoxes = true;
    AutoCFDataRelease headRef(CGFontCopyTableForTag(fCGFont, 'head'));
    const uint16_t* headData = headRef.getShortPtr();
    if (!headData) {
        return false;
    }
    AutoCFDataRelease locaRef(CGFontCopyTableForTag(fCGFont, 'loca'));
    const uint16_t* locaData = locaRef.getShortPtr();
    if (!locaData) {
        return false;
    }
    AutoCFDataRelease glyfRef(CGFontCopyTableForTag(fCGFont, 'glyf'));
    const uint16_t* glyfData = glyfRef.getShortPtr();
    if (!glyfData) {
        return false;
    }
    CFIndex entries = fGlyphCount - fAdjustStart;
    fAdjustBad = new GlyphRect[entries];
    int locaFormat = getLocaFormat(headData);
    const uint16_t* locaPtr = &locaData[fAdjustStart << locaFormat];
    uint32_t last = getLocaTableEntry(locaPtr, locaFormat);
    for (CFIndex index = 0; index < entries; ++index) {
        uint32_t offset = getLocaTableEntry(locaPtr, locaFormat);
        GlyphRect& rect = fAdjustBad[index];
        if (offset != last) {
            rect.fMinX = SkEndian_SwapBE16(glyfData[last + 1]);
            rect.fMinY = SkEndian_SwapBE16(glyfData[last + 2]);
            rect.fMaxX = SkEndian_SwapBE16(glyfData[last + 3]);
            rect.fMaxY = SkEndian_SwapBE16(glyfData[last + 4]);
        } else {
            sk_bzero(&rect, sizeof(GlyphRect));
        }
        last = offset;
    }
    fAdjustBadMatrix = fMatrix;
    flip(&fAdjustBadMatrix);
    SkScalar fontScale = getFontScale(fCGFont);
    fAdjustBadMatrix.preScale(fontScale, fontScale);
    return true;
}

unsigned SkScalerContext_Mac::generateGlyphCount(void)
{
    return(fGlyphCount);
}

uint16_t SkScalerContext_Mac::generateCharToGlyph(SkUnichar uni)
{   CGGlyph     cgGlyph;
    UniChar     theChar;


    // Validate our parameters and state
    SkASSERT(uni             <= 0x0000FFFF);
    SkASSERT(sizeof(CGGlyph) <= sizeof(uint16_t));


    // Get the glyph
    theChar = (UniChar) uni;

    if (!CTFontGetGlyphsForCharacters(fCTFont, &theChar, &cgGlyph, 1))
        cgGlyph = 0;

    return(cgGlyph);
}

void SkScalerContext_Mac::generateAdvance(SkGlyph* glyph) {
    this->generateMetrics(glyph);
}

void SkScalerContext_Mac::generateMetrics(SkGlyph* glyph) {
    CGSize      theAdvance;
    CGRect      theBounds;
    CGGlyph     cgGlyph;

    // Get the state we need
    cgGlyph = (CGGlyph) glyph->getGlyphID(fBaseGlyphCount);

    if (fVertical) {
        if (!isSnowLeopard()) {
        // Lion and Leopard respect the vertical font metrics.
            CTFontGetBoundingRectsForGlyphs(fCTVerticalFont,
                                            kCTFontVerticalOrientation,
                                            &cgGlyph, &theBounds,  1);
        } else {
        // Snow Leopard and earlier respect the vertical font metrics for
        // advances, but not bounds, so use the default box and adjust it below.
            CTFontGetBoundingRectsForGlyphs(fCTFont, kCTFontDefaultOrientation,
                                            &cgGlyph, &theBounds,  1);
        }
        CTFontGetAdvancesForGlyphs(fCTVerticalFont, kCTFontVerticalOrientation,
                                   &cgGlyph, &theAdvance, 1);
    } else {
        CTFontGetBoundingRectsForGlyphs(fCTFont, kCTFontDefaultOrientation,
                                        &cgGlyph, &theBounds, 1);
        CTFontGetAdvancesForGlyphs(fCTFont, kCTFontDefaultOrientation,
                                   &cgGlyph, &theAdvance, 1);
    }

    // BUG?
    // 0x200B (zero-advance space) seems to return a huge (garbage) bounds, when
    // it should be empty. So, if we see a zero-advance, we check if it has an
    // empty path or not, and if so, we jam the bounds to 0. Hopefully a zero-advance
    // is rare, so we won't incur a big performance cost for this extra check.
    if (0 == theAdvance.width && 0 == theAdvance.height) {
        CGPathRef path = CTFontCreatePathForGlyph(fCTFont, cgGlyph, NULL);
        if (NULL == path || CGPathIsEmpty(path)) {
            theBounds = CGRectMake(0, 0, 0, 0);
        }
        if (path) {
            CGPathRelease(path);
        }
    }

    glyph->zeroMetrics();
    glyph->fAdvanceX =  SkFloatToFixed_Check(theAdvance.width);
    glyph->fAdvanceY = -SkFloatToFixed_Check(theAdvance.height);

    if (CGRectIsEmpty_inline(theBounds)) {
        return;
    }

    if (isLeopard() && !fVertical) {
        // Leopard does not consider the matrix skew in its bounds.
        // Run the bounding rectangle through the skew matrix to determine
        // the true bounds. However, this doesn't work if the font is vertical.
        // FIXME (Leopard): If the font has synthetic italic (e.g., matrix skew)
        // and the font is vertical, the bounds need to be recomputed.
        SkRect glyphBounds = SkRect::MakeXYWH(
                theBounds.origin.x, theBounds.origin.y,
                theBounds.size.width, theBounds.size.height);
        fUnitMatrix.mapRect(&glyphBounds);
        theBounds.origin.x = glyphBounds.fLeft;
        theBounds.origin.y = glyphBounds.fTop;
        theBounds.size.width = glyphBounds.width();
        theBounds.size.height = glyphBounds.height();
    }
    // Adjust the bounds
    //
    // CTFontGetBoundingRectsForGlyphs ignores the font transform, so we need
    // to transform the bounding box ourselves.
    //
    // The bounds are also expanded by 1 pixel, to give CG room for anti-aliasing.
    CGRectInset_inline(&theBounds, -1, -1);

    // Get the metrics
    bool lionAdjustedMetrics = false;
    if (isLion() || isMountainLion()) {
        if (cgGlyph < fGlyphCount && cgGlyph >= getAdjustStart()
                    && generateBBoxes()) {
            lionAdjustedMetrics = true;
            SkRect adjust;
            const GlyphRect& gRect = fAdjustBad[cgGlyph - fAdjustStart];
            adjust.set(gRect.fMinX, gRect.fMinY, gRect.fMaxX, gRect.fMaxY);
            fAdjustBadMatrix.mapRect(&adjust);
            theBounds.origin.x = SkScalarToFloat(adjust.fLeft) - 1;
            theBounds.origin.y = SkScalarToFloat(adjust.fTop) - 1;
        }
        // Lion returns fractions in the bounds
        glyph->fWidth = SkToU16(sk_float_ceil2int(theBounds.size.width));
        glyph->fHeight = SkToU16(sk_float_ceil2int(theBounds.size.height));
    } else {
        glyph->fWidth = SkToU16(sk_float_round2int(theBounds.size.width));
        glyph->fHeight = SkToU16(sk_float_round2int(theBounds.size.height));
    }
    glyph->fTop      = SkToS16(-sk_float_round2int(CGRectGetMaxY_inline(theBounds)));
    glyph->fLeft     = SkToS16(sk_float_round2int(CGRectGetMinX_inline(theBounds)));
    SkIPoint offset;
    if (fVertical && (isSnowLeopard() || lionAdjustedMetrics)) {
    // SnowLeopard doesn't respect vertical metrics, so compute them manually.
    // Also compute them for Lion when the metrics were computed by hand.
        getVerticalOffset(cgGlyph, &offset);
        glyph->fLeft += offset.fX;
        glyph->fTop += offset.fY;
    }
}

#include "SkColorPriv.h"

static void build_power_table(uint8_t table[], float ee) {
    for (int i = 0; i < 256; i++) {
        float x = i / 255.f;
        x = sk_float_pow(x, ee);
        int xx = SkScalarRoundToInt(SkFloatToScalar(x * 255));
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

template <typename T> T* SkTAddByteOffset(T* ptr, size_t byteOffset) {
    return (T*)((char*)ptr + byteOffset);
}

void SkScalerContext_Mac::generateImage(const SkGlyph& glyph, SkMaskGamma::PreBlend* maskPreBlend) {
    CGGlyph cgGlyph = (CGGlyph) glyph.getGlyphID(fBaseGlyphCount);

    // Draw the glyph
    size_t cgRowBytes;
    CGRGBPixel* cgPixels = fOffscreen.getCG(*this, glyph, cgGlyph, &cgRowBytes);
    if (cgPixels == NULL) {
        return;
    }

    //TODO: see if drawing black on white and inverting is faster (at least in
    //lcd case) as core graphics appears to have special case code for drawing
    //black text.

    // Fix the glyph
    const bool isLCD = isLCDFormat(glyph.fMaskFormat);
    if (isLCD || (glyph.fMaskFormat == SkMask::kA8_Format && supports_LCD())) {
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

    // Must be careful not to use these if maskPreBlend == NULL
    const uint8_t* tableR = NULL;
    const uint8_t* tableG = NULL;
    const uint8_t* tableB = NULL;
    if (maskPreBlend) {
        tableR = maskPreBlend->fR;
        tableG = maskPreBlend->fG;
        tableB = maskPreBlend->fB;
    }

    // Convert glyph to mask
    switch (glyph.fMaskFormat) {
        case SkMask::kLCD32_Format: {
            if (maskPreBlend) {
                rgb_to_lcd32<true>(cgPixels, cgRowBytes, glyph, tableR, tableG, tableB);
            } else {
                rgb_to_lcd32<false>(cgPixels, cgRowBytes, glyph, tableR, tableG, tableB);
            }
        } break;
        case SkMask::kLCD16_Format: {
            if (maskPreBlend) {
                rgb_to_lcd16<true>(cgPixels, cgRowBytes, glyph, tableR, tableG, tableB);
            } else {
                rgb_to_lcd16<false>(cgPixels, cgRowBytes, glyph, tableR, tableG, tableB);
            }
        } break;
        case SkMask::kA8_Format: {
            if (maskPreBlend) {
                rgb_to_a8<true>(cgPixels, cgRowBytes, glyph, tableG);
            } else {
                rgb_to_a8<false>(cgPixels, cgRowBytes, glyph, tableG);
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
#define kScaleForSubPixelPositionHinting  4

void SkScalerContext_Mac::generatePath(const SkGlyph& glyph, SkPath* path) {
    CTFontRef font = fCTFont;
    float scaleX = 1;
    float scaleY = 1;

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
                scaleY = 1; // want hinting in the Y direction
                break;
            case kY_SkAxisAlignment:
                scaleX = 1; // want hinting in the X direction
                break;
            default:
                break;
        }

        CGAffineTransform xform = MatrixToCGAffineTransform(m, scaleX, scaleY);
        // need to release font when we're done
        font = CTFontCreateCopyWithAttributes(fCTFont, 1, &xform, NULL);
    }

    CGGlyph   cgGlyph = (CGGlyph)glyph.getGlyphID(fBaseGlyphCount);
    CGPathRef cgPath  = CTFontCreatePathForGlyph(font, cgGlyph, NULL);

    path->reset();
    if (cgPath != NULL) {
        CGPathApply(cgPath, path, SkScalerContext_Mac::CTPathElement);
        CFRelease(cgPath);
    }

    if (fRec.fFlags & SkScalerContext::kSubpixelPositioning_Flag) {
        SkMatrix m;
        m.setScale(SkFloatToScalar(1 / scaleX), SkFloatToScalar(1 / scaleY));
        path->transform(m);
        // balance the call to CTFontCreateCopyWithAttributes
        CFRelease(font);
    }
    if (fRec.fFlags & SkScalerContext::kVertical_Flag) {
        SkIPoint offset;
        getVerticalOffset(cgGlyph, &offset);
        path->offset(SkIntToScalar(offset.fX), SkIntToScalar(offset.fY));
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

void SkScalerContext_Mac::CTPathElement(void *info, const CGPathElement *element)
{   SkPath      *skPath = (SkPath *) info;


    // Process the path element
    switch (element->type) {
        case kCGPathElementMoveToPoint:
            skPath->moveTo( element->points[0].x, -element->points[0].y);
            break;

        case kCGPathElementAddLineToPoint:
            skPath->lineTo( element->points[0].x, -element->points[0].y);
            break;

        case kCGPathElementAddQuadCurveToPoint:
            skPath->quadTo( element->points[0].x, -element->points[0].y,
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
    CGFontRef cg = CGFontCreateWithDataProvider(provider);
    if (NULL == cg) {
        return NULL;
    }
    CTFontRef ct = CTFontCreateWithGraphicsFont(cg, 0, NULL, NULL);
    CGFontRelease(cg);
    return cg ? SkCreateTypefaceFromCTFont(ct) : NULL;
}

class AutoCGDataProviderRelease : SkNoncopyable {
public:
    AutoCGDataProviderRelease(CGDataProviderRef provider) : fProvider(provider) {}
    ~AutoCGDataProviderRelease() { CGDataProviderRelease(fProvider); }

private:
    CGDataProviderRef fProvider;
};

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream) {
    CGDataProviderRef provider = SkCreateDataProviderFromStream(stream);
    if (NULL == provider) {
        return NULL;
    }
    AutoCGDataProviderRelease ar(provider);
    return create_from_dataProvider(provider);
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(const char path[]) {
    CGDataProviderRef provider = CGDataProviderCreateWithFilename(path);
    if (NULL == provider) {
        return NULL;
    }
    AutoCGDataProviderRelease ar(provider);
    return create_from_dataProvider(provider);
}

// Web fonts added to the the CTFont registry do not return their character set.
// Iterate through the font in this case. The existing caller caches the result,
// so the performance impact isn't too bad.
static void populate_glyph_to_unicode_slow(CTFontRef ctFont,
        unsigned glyphCount, SkTDArray<SkUnichar>* glyphToUnicode) {
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
static void populate_glyph_to_unicode(CTFontRef ctFont,
        const unsigned glyphCount, SkTDArray<SkUnichar>* glyphToUnicode) {
    CFCharacterSetRef charSet = CTFontCopyCharacterSet(ctFont);
    AutoCFRelease autoSetRelease(charSet);
    if (!charSet) {
        populate_glyph_to_unicode_slow(ctFont, glyphCount, glyphToUnicode);
        return;
    }

    CFDataRef bitmap = CFCharacterSetCreateBitmapRepresentation(
        kCFAllocatorDefault, charSet);
    if (!bitmap) {
        return;
    }
    CFIndex length = CFDataGetLength(bitmap);
    if (!length) {
        CFSafeRelease(bitmap);
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
            if (mask & (1 << j) && CTFontGetGlyphsForCharacters(ctFont,
                    &unichar, &glyph, 1)) {
                out[glyph] = unichar;
            }
        }
    }
    CFSafeRelease(bitmap);
}

static bool getWidthAdvance(CTFontRef ctFont, int gId, int16_t* data) {
    CGSize advance;
    advance.width = 0;
    CGGlyph glyph = gId;
    CTFontGetAdvancesForGlyphs(ctFont, kCTFontHorizontalOrientation, &glyph,
        &advance, 1);
    *data = sk_float_round2int(advance.width);
    return true;
}

// we might move this into our CGUtils...
static void CFStringToSkString(CFStringRef src, SkString* dst) {
    // Reserve enough room for the worst-case string,
    // plus 1 byte for the trailing null.
    int length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(src),
                                                   kCFStringEncodingUTF8) + 1;
    dst->resize(length);
    CFStringGetCString(src, dst->writable_str(), length,
                       kCFStringEncodingUTF8);
    // Resize to the actual UTF-8 length used, stripping the null character.
    dst->resize(strlen(dst->c_str()));
}

// static
SkAdvancedTypefaceMetrics* SkFontHost::GetAdvancedTypefaceMetrics(
        uint32_t fontID,
        SkAdvancedTypefaceMetrics::PerGlyphInfo perGlyphInfo,
        const uint32_t* glyphIDs,
        uint32_t glyphIDsCount) {
    CTFontRef ctFont = GetFontRefFromFontID(fontID);
    ctFont = CTFontCreateCopyWithAttributes(ctFont, CTFontGetUnitsPerEm(ctFont),
                                            NULL, NULL);
    SkAdvancedTypefaceMetrics* info = new SkAdvancedTypefaceMetrics;

    {
        CFStringRef fontName = CTFontCopyPostScriptName(ctFont);
        CFStringToSkString(fontName, &info->fFontName);
        CFRelease(fontName);
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
    if (!GetTableSize(fontID, 'glyf') || !GetTableSize(fontID, 'loca')) {
        info->fType = SkAdvancedTypefaceMetrics::kOther_Font;
        info->fItalicAngle = 0;
        info->fAscent = 0;
        info->fDescent = 0;
        info->fStemV = 0;
        info->fCapHeight = 0;
        info->fBBox = SkIRect::MakeEmpty();
        CFSafeRelease(ctFont);
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
    CTFontStylisticClass stylisticClass = symbolicTraits &
            kCTFontClassMaskTrait;
    if (stylisticClass & kCTFontSymbolicClass) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kSymbolic_Style;
    }
    if (stylisticClass >= kCTFontOldStyleSerifsClass
            && stylisticClass <= kCTFontSlabSerifsClass) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kSerif_Style;
    } else if (stylisticClass & kCTFontScriptsClass) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kScript_Style;
    }
    info->fItalicAngle = (int16_t) CTFontGetSlantAngle(ctFont);
    info->fAscent = (int16_t) CTFontGetAscent(ctFont);
    info->fDescent = (int16_t) CTFontGetDescent(ctFont);
    info->fCapHeight = (int16_t) CTFontGetCapHeight(ctFont);
    CGRect bbox = CTFontGetBoundingBox(ctFont);
    info->fBBox = SkIRect::MakeLTRB(
            CGToScalar(CGRectGetMinX_inline(bbox)),   // Left
            CGToScalar(CGRectGetMaxY_inline(bbox)),   // Top
            CGToScalar(CGRectGetMaxX_inline(bbox)),   // Right
            CGToScalar(CGRectGetMinY_inline(bbox)));  // Bottom

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
    } else if (perGlyphInfo &
               SkAdvancedTypefaceMetrics::kHAdvance_PerGlyphInfo) {
        if (info->fStyle & SkAdvancedTypefaceMetrics::kFixedPitch_Style) {
            skia_advanced_typeface_metrics_utils::appendRange(&info->fGlyphWidths, 0);
            info->fGlyphWidths->fAdvance.append(1, &min_width);
            skia_advanced_typeface_metrics_utils::finishRange(info->fGlyphWidths.get(), 0,
                        SkAdvancedTypefaceMetrics::WidthRange::kDefault);
        } else {
            info->fGlyphWidths.reset(
                skia_advanced_typeface_metrics_utils::getAdvanceData(ctFont,
                               glyphCount,
                               glyphIDs,
                               glyphIDsCount,
                               &getWidthAdvance));
        }
    }

    CFSafeRelease(ctFont);
    return info;
}

///////////////////////////////////////////////////////////////////////////////

struct FontHeader {
    SkFixed fVersion;
    uint16_t fNumTables;
    uint16_t fSearchRange;
    uint16_t fEntrySelector;
    uint16_t fRangeShift;
};

struct TableEntry {
    uint32_t fTag;
    uint32_t fCheckSum;
    uint32_t fOffset;
    uint32_t fLength;
};

static uint32_t CalcTableCheckSum(uint32_t *table, uint32_t numberOfBytesInTable) {
    uint32_t sum = 0;
    uint32_t nLongs = (numberOfBytesInTable + 3) / 4;

    while (nLongs-- > 0) {
        sum += SkEndian_SwapBE32(*table++);
    }
    return sum;
}

SkStream* SkFontHost::OpenStream(SkFontID uniqueID) {
    // get table tags
    int tableCount = CountTables(uniqueID);
    SkTDArray<SkFontTableTag> tableTags;
    tableTags.setCount(tableCount);
    GetTableTags(uniqueID, tableTags.begin());

    // calc total size for font, save sizes
    SkTDArray<size_t> tableSizes;
    size_t totalSize = sizeof(FontHeader) + sizeof(TableEntry) * tableCount;
    for (int index = 0; index < tableCount; ++index) {
        size_t tableSize = GetTableSize(uniqueID, tableTags[index]);
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
    while (searchRange < tableCount >> 1) {
        entrySelector++;
        searchRange <<= 1;
    }
    searchRange <<= 4;
    uint16_t rangeShift = (tableCount << 4) - searchRange;

    // write font header (also called sfnt header, offset subtable)
    FontHeader* offsetTable = (FontHeader*)dataPtr;
    offsetTable->fVersion = SkEndian_SwapBE32(SK_Fixed1);
    offsetTable->fNumTables = SkEndian_SwapBE16(tableCount);
    offsetTable->fSearchRange = SkEndian_SwapBE16(searchRange);
    offsetTable->fEntrySelector = SkEndian_SwapBE16(entrySelector);
    offsetTable->fRangeShift = SkEndian_SwapBE16(rangeShift);
    dataPtr += sizeof(FontHeader);

    // write tables
    TableEntry* entry = (TableEntry*)dataPtr;
    dataPtr += sizeof(TableEntry) * tableCount;
    for (int index = 0; index < tableCount; ++index) {
        size_t tableSize = tableSizes[index];
        GetTableData(uniqueID, tableTags[index], 0, tableSize, dataPtr);
        entry->fTag = SkEndian_SwapBE32(tableTags[index]);
        entry->fCheckSum = SkEndian_SwapBE32(CalcTableCheckSum(
            (uint32_t*)dataPtr, tableSize));
        entry->fOffset = SkEndian_SwapBE32(dataPtr - dataStart);
        entry->fLength = SkEndian_SwapBE32(tableSize);
        dataPtr += (tableSize + 3) & ~3;
        ++entry;
    }

    return stream;
}

size_t SkFontHost::GetFileName(SkFontID fontID, char path[], size_t length,
                               int32_t* index) {
    SkDEBUGFAIL("SkFontHost::GetFileName unimplemented");
    return(0);
}

///////////////////////////////////////////////////////////////////////////////

#include "SkStream.h"

// we take ownership of the ref
static const char* get_str(CFStringRef ref, SkString* str) {
    CFStringToSkString(ref, str);
    CFSafeRelease(ref);
    return str->c_str();
}

void SkFontHost::Serialize(const SkTypeface* face, SkWStream* stream) {
    CTFontRef        ctFont = typeface_to_fontref(face);
    SkFontDescriptor desc(face->style());
    SkString         tmpStr;

    desc.setFamilyName(get_str(CTFontCopyFamilyName(ctFont), &tmpStr));
    desc.setFullName(get_str(CTFontCopyFullName(ctFont), &tmpStr));
    desc.setPostscriptName(get_str(CTFontCopyPostScriptName(ctFont), &tmpStr));

    desc.serialize(stream);

    // by convention, we also write out the actual sfnt data, preceeded by
    // a packed-length. For now we skip that, so we just write the zero.
    stream->writePackedUInt(0);
}

SkTypeface* SkFontHost::Deserialize(SkStream* stream) {
    SkFontDescriptor desc(stream);

    // by convention, Serialize will have also written the actual sfnt data.
    // for now, we just want to skip it.
    size_t size = stream->readPackedUInt();
    stream->skip(size);

    return SkFontHost::CreateTypeface(NULL, desc.getFamilyName(),
                                      desc.getStyle());
}

///////////////////////////////////////////////////////////////////////////////

SkScalerContext* SkFontHost::CreateScalerContext(const SkDescriptor* desc) {
    return new SkScalerContext_Mac(desc);
}

SkFontID SkFontHost::NextLogicalFont(SkFontID currFontID, SkFontID origFontID) {
    SkFontID nextFontID = 0;
    SkTypeface* face = GetDefaultFace();
    if (face->uniqueID() != currFontID) {
        nextFontID = face->uniqueID();
    }
    return nextFontID;
}

void SkFontHost::FilterRec(SkScalerContext::Rec* rec) {
    unsigned flagsWeDontSupport = SkScalerContext::kDevKernText_Flag |
                                  SkScalerContext::kAutohinting_Flag;

    rec->fFlags &= ~flagsWeDontSupport;

    // we only support 2 levels of hinting
    SkPaint::Hinting h = rec->getHinting();
    if (SkPaint::kSlight_Hinting == h) {
        h = SkPaint::kNo_Hinting;
    } else if (SkPaint::kFull_Hinting == h) {
        h = SkPaint::kNormal_Hinting;
    }
    rec->setHinting(h);

    bool lcdSupport = supports_LCD();
    if (isLCDFormat(rec->fMaskFormat)) {
        if (lcdSupport) {
            //CoreGraphics creates 555 masks for smoothed text anyway.
            rec->fMaskFormat = SkMask::kLCD16_Format;
        } else {
            rec->fMaskFormat = SkMask::kA8_Format;
        }
    }

    if (lcdSupport) {
        //CoreGraphics dialates smoothed text as needed.
        rec->setContrast(0);
    } else {
#ifndef SK_GAMMA_APPLY_TO_A8
        rec->ignorePreBlend();
#endif
    }
}

///////////////////////////////////////////////////////////////////////////

int SkFontHost::CountTables(SkFontID fontID) {
    CTFontRef ctFont = GetFontRefFromFontID(fontID);
    CFArrayRef cfArray = CTFontCopyAvailableTables(ctFont,
                                                   kCTFontTableOptionNoOptions);
    if (NULL == cfArray) {
        return 0;
    }

    AutoCFRelease ar(cfArray);
    return CFArrayGetCount(cfArray);
}

int SkFontHost::GetTableTags(SkFontID fontID, SkFontTableTag tags[]) {
    CTFontRef ctFont = GetFontRefFromFontID(fontID);
    CFArrayRef cfArray = CTFontCopyAvailableTables(ctFont,
                                                   kCTFontTableOptionNoOptions);
    if (NULL == cfArray) {
        return 0;
    }

    AutoCFRelease ar(cfArray);

    int count = CFArrayGetCount(cfArray);
    if (tags) {
        for (int i = 0; i < count; ++i) {
            uintptr_t fontTag = reinterpret_cast<uintptr_t>(
                    CFArrayGetValueAtIndex(cfArray, i));
            tags[i] = static_cast<SkFontTableTag>(fontTag);
        }
    }
    return count;
}

// If, as is the case with web fonts, the CTFont data isn't available,
// the CGFont data may work. While the CGFont may always provide the
// right result, leave the CTFont code path to minimize disruption.
static CFDataRef copyTableFromFont(CTFontRef ctFont, SkFontTableTag tag) {
    CFDataRef data = CTFontCopyTable(ctFont, (CTFontTableTag) tag,
                                     kCTFontTableOptionNoOptions);
    if (NULL == data) {
        CGFontRef cgFont = CTFontCopyGraphicsFont(ctFont, NULL);
        data = CGFontCopyTableForTag(cgFont, tag);
        CGFontRelease(cgFont);
    }
    return data;
}

size_t SkFontHost::GetTableSize(SkFontID fontID, SkFontTableTag tag) {
    CTFontRef ctFont = GetFontRefFromFontID(fontID);
    CFDataRef srcData = copyTableFromFont(ctFont, tag);
    if (NULL == srcData) {
        return 0;
    }

    AutoCFRelease ar(srcData);
    return CFDataGetLength(srcData);
}

size_t SkFontHost::GetTableData(SkFontID fontID, SkFontTableTag tag,
                                size_t offset, size_t length, void* dst) {
    CTFontRef ctFont = GetFontRefFromFontID(fontID);
    CFDataRef srcData = copyTableFromFont(ctFont, tag);
    if (NULL == srcData) {
        return 0;
    }

    AutoCFRelease ar(srcData);

    size_t srcSize = CFDataGetLength(srcData);
    if (offset >= srcSize) {
        return 0;
    }

    if ((offset + length) > srcSize) {
        length = srcSize - offset;
    }

    if (dst) {
        memcpy(dst, CFDataGetBytePtr(srcData) + offset, length);
    }
    return length;
}
