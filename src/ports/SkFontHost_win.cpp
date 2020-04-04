/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#include "include/core/SkData.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPath.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/ports/SkTypeface_win.h"
#include "include/private/SkColorData.h"
#include "include/private/SkMacros.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "include/utils/SkBase64.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkLeanWindows.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkTypefaceCache.h"
#include "src/core/SkUtils.h"
#include "src/sfnt/SkOTTable_OS_2.h"
#include "src/sfnt/SkOTTable_maxp.h"
#include "src/sfnt/SkOTTable_name.h"
#include "src/sfnt/SkOTUtils.h"
#include "src/sfnt/SkSFNTHeader.h"
#include "src/utils/SkMatrix22.h"
#include "src/utils/win/SkHRESULT.h"

#include <tchar.h>
#include <usp10.h>
#include <objbase.h>

static void (*gEnsureLOGFONTAccessibleProc)(const LOGFONT&);

void SkTypeface_SetEnsureLOGFONTAccessibleProc(void (*proc)(const LOGFONT&)) {
    gEnsureLOGFONTAccessibleProc = proc;
}

static void call_ensure_accessible(const LOGFONT& lf) {
    if (gEnsureLOGFONTAccessibleProc) {
        gEnsureLOGFONTAccessibleProc(lf);
    }
}

///////////////////////////////////////////////////////////////////////////////

// always packed xxRRGGBB
typedef uint32_t SkGdiRGB;

// define this in your Makefile or .gyp to enforce AA requests
// which GDI ignores at small sizes. This flag guarantees AA
// for rotated text, regardless of GDI's notions.
//#define SK_ENFORCE_ROTATED_TEXT_AA_ON_WINDOWS

static bool isLCD(const SkScalerContextRec& rec) {
    return SkMask::kLCD16_Format == rec.fMaskFormat;
}

static bool bothZero(SkScalar a, SkScalar b) {
    return 0 == a && 0 == b;
}

// returns false if there is any non-90-rotation or skew
static bool isAxisAligned(const SkScalerContextRec& rec) {
    return 0 == rec.fPreSkewX &&
           (bothZero(rec.fPost2x2[0][1], rec.fPost2x2[1][0]) ||
            bothZero(rec.fPost2x2[0][0], rec.fPost2x2[1][1]));
}

static bool needToRenderWithSkia(const SkScalerContextRec& rec) {
#ifdef SK_ENFORCE_ROTATED_TEXT_AA_ON_WINDOWS
    // What we really want to catch is when GDI will ignore the AA request and give
    // us BW instead. Smallish rotated text is one heuristic, so this code is just
    // an approximation. We shouldn't need to do this for larger sizes, but at those
    // sizes, the quality difference gets less and less between our general
    // scanconverter and GDI's.
    if (SkMask::kA8_Format == rec.fMaskFormat && !isAxisAligned(rec)) {
        return true;
    }
#endif
    return rec.getHinting() == SkFontHinting::kNone || rec.getHinting() == SkFontHinting::kSlight;
}

static void tchar_to_skstring(const TCHAR t[], SkString* s) {
#ifdef UNICODE
    size_t sSize = WideCharToMultiByte(CP_UTF8, 0, t, -1, nullptr, 0, nullptr, nullptr);
    s->resize(sSize);
    WideCharToMultiByte(CP_UTF8, 0, t, -1, s->writable_str(), sSize, nullptr, nullptr);
#else
    s->set(t);
#endif
}

static void dcfontname_to_skstring(HDC deviceContext, const LOGFONT& lf, SkString* familyName) {
    int fontNameLen; //length of fontName in TCHARS.
    if (0 == (fontNameLen = GetTextFace(deviceContext, 0, nullptr))) {
        call_ensure_accessible(lf);
        if (0 == (fontNameLen = GetTextFace(deviceContext, 0, nullptr))) {
            fontNameLen = 0;
        }
    }

    SkAutoSTArray<LF_FULLFACESIZE, TCHAR> fontName(fontNameLen+1);
    if (0 == GetTextFace(deviceContext, fontNameLen, fontName.get())) {
        call_ensure_accessible(lf);
        if (0 == GetTextFace(deviceContext, fontNameLen, fontName.get())) {
            fontName[0] = 0;
        }
    }

    tchar_to_skstring(fontName.get(), familyName);
}

static void make_canonical(LOGFONT* lf) {
    lf->lfHeight = -64;
    lf->lfWidth = 0;  // lfWidth is related to lfHeight, not to the OS/2::usWidthClass.
    lf->lfQuality = CLEARTYPE_QUALITY;//PROOF_QUALITY;
    lf->lfCharSet = DEFAULT_CHARSET;
//    lf->lfClipPrecision = 64;
}

static SkFontStyle get_style(const LOGFONT& lf) {
    return SkFontStyle(lf.lfWeight,
                       SkFontStyle::kNormal_Width,
                       lf.lfItalic ? SkFontStyle::kItalic_Slant : SkFontStyle::kUpright_Slant);
}

static inline FIXED SkFixedToFIXED(SkFixed x) {
    return *(FIXED*)(&x);
}
static inline SkFixed SkFIXEDToFixed(FIXED x) {
    return *(SkFixed*)(&x);
}

static inline FIXED SkScalarToFIXED(SkScalar x) {
    return SkFixedToFIXED(SkScalarToFixed(x));
}

static inline SkScalar SkFIXEDToScalar(FIXED x) {
    return SkFixedToScalar(SkFIXEDToFixed(x));
}

static unsigned calculateGlyphCount(HDC hdc, const LOGFONT& lf) {
    TEXTMETRIC textMetric;
    if (0 == GetTextMetrics(hdc, &textMetric)) {
        textMetric.tmPitchAndFamily = TMPF_VECTOR;
        call_ensure_accessible(lf);
        GetTextMetrics(hdc, &textMetric);
    }

    if (!(textMetric.tmPitchAndFamily & TMPF_VECTOR)) {
        return textMetric.tmLastChar;
    }

    // The 'maxp' table stores the number of glyphs at offset 4, in 2 bytes.
    uint16_t glyphs;
    if (GDI_ERROR != GetFontData(hdc, SkOTTableMaximumProfile::TAG, 4, &glyphs, sizeof(glyphs))) {
        return SkEndian_SwapBE16(glyphs);
    }

    // Binary search for glyph count.
    static const MAT2 mat2 = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
    int32_t max = UINT16_MAX + 1;
    int32_t min = 0;
    GLYPHMETRICS gm;
    while (min < max) {
        int32_t mid = min + ((max - min) / 2);
        if (GetGlyphOutlineW(hdc, mid, GGO_METRICS | GGO_GLYPH_INDEX, &gm, 0,
                             nullptr, &mat2) == GDI_ERROR) {
            max = mid;
        } else {
            min = mid + 1;
        }
    }
    SkASSERT(min == max);
    return min;
}

static unsigned calculateUPEM(HDC hdc, const LOGFONT& lf) {
    TEXTMETRIC textMetric;
    if (0 == GetTextMetrics(hdc, &textMetric)) {
        textMetric.tmPitchAndFamily = TMPF_VECTOR;
        call_ensure_accessible(lf);
        GetTextMetrics(hdc, &textMetric);
    }

    if (!(textMetric.tmPitchAndFamily & TMPF_VECTOR)) {
        return textMetric.tmMaxCharWidth;
    }

    OUTLINETEXTMETRIC otm;
    unsigned int otmRet = GetOutlineTextMetrics(hdc, sizeof(otm), &otm);
    if (0 == otmRet) {
        call_ensure_accessible(lf);
        otmRet = GetOutlineTextMetrics(hdc, sizeof(otm), &otm);
    }

    return (0 == otmRet) ? 0 : otm.otmEMSquare;
}

class SkAutoHDC {
public:
    explicit SkAutoHDC(const LOGFONT& lf)
        : fHdc(::CreateCompatibleDC(nullptr))
        , fFont(::CreateFontIndirect(&lf))
        , fSavefont((HFONT)::SelectObject(fHdc, fFont))
    { }
    ~SkAutoHDC() {
        if (fHdc) {
            ::SelectObject(fHdc, fSavefont);
            ::DeleteDC(fHdc);
        }
        if (fFont) {
            ::DeleteObject(fFont);
        }
    }
    operator HDC() { return fHdc; }
private:
    HDC fHdc;
    HFONT fFont;
    HFONT fSavefont;
};
#define SkAutoHDC(...) SK_REQUIRE_LOCAL_VAR(SkAutoHDC)

class LogFontTypeface : public SkTypeface {
public:
    LogFontTypeface(const SkFontStyle& style, const LOGFONT& lf, bool serializeAsStream)
        : SkTypeface(style, false)
        , fLogFont(lf)
        , fSerializeAsStream(serializeAsStream)
    {
        SkAutoHDC hdc(fLogFont);
        TEXTMETRIC textMetric;
        if (0 == GetTextMetrics(hdc, &textMetric)) {
            call_ensure_accessible(lf);
            if (0 == GetTextMetrics(hdc, &textMetric)) {
                textMetric.tmPitchAndFamily = TMPF_TRUETYPE;
            }
        }

        // The fixed pitch bit is set if the font is *not* fixed pitch.
        this->setIsFixedPitch((textMetric.tmPitchAndFamily & TMPF_FIXED_PITCH) == 0);
        this->setFontStyle(SkFontStyle(textMetric.tmWeight, style.width(), style.slant()));

        // Used a logfont on a memory context, should never get a device font.
        // Therefore all TMPF_DEVICE will be PostScript (cubic) fonts.
        // If the font has cubic outlines, it will not be rendered with ClearType.
        fCanBeLCD = !((textMetric.tmPitchAndFamily & TMPF_VECTOR) &&
                      (textMetric.tmPitchAndFamily & TMPF_DEVICE));
    }

    LOGFONT fLogFont;
    bool fSerializeAsStream;
    bool fCanBeLCD;

    static sk_sp<LogFontTypeface> Make(const LOGFONT& lf) {
        return sk_sp<LogFontTypeface>(new LogFontTypeface(get_style(lf), lf, false));
    }

    static void EnsureAccessible(const SkTypeface* face) {
        call_ensure_accessible(static_cast<const LogFontTypeface*>(face)->fLogFont);
    }

protected:
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                           const SkDescriptor*) const override;
    void onFilterRec(SkScalerContextRec*) const override;
    void getGlyphToUnicodeMap(SkUnichar*) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override;
    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;
    int onCountGlyphs() const override;
    void getPostScriptGlyphNames(SkString*) const override;
    int onGetUPEM() const override;
    void onGetFamilyName(SkString* familyName) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override
    {
        return -1;
    }
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override
    {
        return -1;
    }
    int onGetTableTags(SkFontTableTag tags[]) const override;
    size_t onGetTableData(SkFontTableTag, size_t offset, size_t length, void* data) const override;
    sk_sp<SkData> onCopyTableData(SkFontTableTag) const override;
};

class FontMemResourceTypeface : public LogFontTypeface {
public:
    /**
     *  The created FontMemResourceTypeface takes ownership of fontMemResource.
     */
    static sk_sp<FontMemResourceTypeface> Make(const LOGFONT& lf, HANDLE fontMemResource) {
        return sk_sp<FontMemResourceTypeface>(
            new FontMemResourceTypeface(get_style(lf), lf, fontMemResource));
    }

protected:
    void weak_dispose() const override {
        RemoveFontMemResourceEx(fFontMemResource);
        INHERITED::weak_dispose();
    }

private:
    /**
     *  Takes ownership of fontMemResource.
     */
    FontMemResourceTypeface(const SkFontStyle& style, const LOGFONT& lf, HANDLE fontMemResource)
        : LogFontTypeface(style, lf, true), fFontMemResource(fontMemResource)
    { }

    HANDLE fFontMemResource;

    typedef LogFontTypeface INHERITED;
};

static const LOGFONT& get_default_font() {
    static LOGFONT gDefaultFont;
    return gDefaultFont;
}

static bool FindByLogFont(SkTypeface* face, void* ctx) {
    LogFontTypeface* lface = static_cast<LogFontTypeface*>(face);
    const LOGFONT* lf = reinterpret_cast<const LOGFONT*>(ctx);

    return !memcmp(&lface->fLogFont, lf, sizeof(LOGFONT));
}

/**
 *  This guy is public. It first searches the cache, and if a match is not found,
 *  it creates a new face.
 */
SkTypeface* SkCreateTypefaceFromLOGFONT(const LOGFONT& origLF) {
    LOGFONT lf = origLF;
    make_canonical(&lf);
    sk_sp<SkTypeface> face = SkTypefaceCache::FindByProcAndRef(FindByLogFont, &lf);
    if (!face) {
        face = LogFontTypeface::Make(lf);
        SkTypefaceCache::Add(face);
    }
    return face.release();
}

/**
 *  The created SkTypeface takes ownership of fontMemResource.
 */
sk_sp<SkTypeface> SkCreateFontMemResourceTypefaceFromLOGFONT(const LOGFONT& origLF, HANDLE fontMemResource) {
    LOGFONT lf = origLF;
    make_canonical(&lf);
    // We'll never get a cache hit, so no point in putting this in SkTypefaceCache.
    return FontMemResourceTypeface::Make(lf, fontMemResource);
}

/**
 *  This guy is public
 */
void SkLOGFONTFromTypeface(const SkTypeface* face, LOGFONT* lf) {
    if (nullptr == face) {
        *lf = get_default_font();
    } else {
        *lf = static_cast<const LogFontTypeface*>(face)->fLogFont;
    }
}

// Construct Glyph to Unicode table.
// Unicode code points that require conjugate pairs in utf16 are not
// supported.
// TODO(arthurhsu): Add support for conjugate pairs. It looks like that may
// require parsing the TTF cmap table (platform 4, encoding 12) directly instead
// of calling GetFontUnicodeRange().
static void populate_glyph_to_unicode(HDC fontHdc, const unsigned glyphCount,
                                      SkUnichar* glyphToUnicode) {
    sk_bzero(glyphToUnicode, sizeof(SkUnichar) * glyphCount);
    DWORD glyphSetBufferSize = GetFontUnicodeRanges(fontHdc, nullptr);
    if (!glyphSetBufferSize) {
        return;
    }

    std::unique_ptr<BYTE[]> glyphSetBuffer(new BYTE[glyphSetBufferSize]);
    GLYPHSET* glyphSet =
        reinterpret_cast<LPGLYPHSET>(glyphSetBuffer.get());
    if (GetFontUnicodeRanges(fontHdc, glyphSet) != glyphSetBufferSize) {
        return;
    }

    for (DWORD i = 0; i < glyphSet->cRanges; ++i) {
        // There is no guarantee that within a Unicode range, the corresponding
        // glyph id in a font file are continuous. So, even if we have ranges,
        // we can't just use the first and last entry of the range to compute
        // result. We need to enumerate them one by one.
        int count = glyphSet->ranges[i].cGlyphs;
        SkAutoTArray<WCHAR> chars(count + 1);
        chars[count] = 0;  // termintate string
        SkAutoTArray<WORD> glyph(count);
        for (USHORT j = 0; j < count; ++j) {
            chars[j] = glyphSet->ranges[i].wcLow + j;
        }
        GetGlyphIndicesW(fontHdc, chars.get(), count, glyph.get(),
                         GGI_MARK_NONEXISTING_GLYPHS);
        // If the glyph ID is valid, and the glyph is not mapped, then we will
        // fill in the char id into the vector. If the glyph is mapped already,
        // skip it.
        // TODO(arthurhsu): better improve this. e.g. Get all used char ids from
        // font cache, then generate this mapping table from there. It's
        // unlikely to have collisions since glyph reuse happens mostly for
        // different Unicode pages.
        for (USHORT j = 0; j < count; ++j) {
            if (glyph[j] != 0xFFFF && glyph[j] < glyphCount && glyphToUnicode[glyph[j]] == 0) {
                glyphToUnicode[glyph[j]] = chars[j];
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////

static int alignTo32(int n) {
    return (n + 31) & ~31;
}

struct MyBitmapInfo : public BITMAPINFO {
    RGBQUAD fMoreSpaceForColors[1];
};

class HDCOffscreen {
public:
    HDCOffscreen() = default;

    ~HDCOffscreen() {
        if (fDC) {
            ::SelectObject(fDC, fSavefont);
            ::DeleteDC(fDC);
        }
        if (fBM) {
            DeleteObject(fBM);
        }
    }

    void init(HFONT font, const XFORM& xform) {
        fFont = font;
        fXform = xform;
    }

    const void* draw(const SkGlyph&, bool isBW, size_t* srcRBPtr);

private:
    HDC     fDC{0};
    HFONT   fSavefont{0};
    HBITMAP fBM{0};
    HFONT   fFont{0};
    XFORM   fXform{1, 0, 0, 1, 0, 0};
    void*   fBits{nullptr};  // points into fBM
    int     fWidth{0};
    int     fHeight{0};
    bool    fIsBW{false};
};

const void* HDCOffscreen::draw(const SkGlyph& glyph, bool isBW,
                               size_t* srcRBPtr) {
    // Can we share the scalercontext's fDDC, so we don't need to create
    // a separate fDC here?
    if (0 == fDC) {
        fDC = CreateCompatibleDC(0);
        if (0 == fDC) {
            return nullptr;
        }
        SetGraphicsMode(fDC, GM_ADVANCED);
        SetBkMode(fDC, TRANSPARENT);
        SetTextAlign(fDC, TA_LEFT | TA_BASELINE);
        fSavefont = (HFONT)SelectObject(fDC, fFont);

        COLORREF color = 0x00FFFFFF;
        SkDEBUGCODE(COLORREF prev =) SetTextColor(fDC, color);
        SkASSERT(prev != CLR_INVALID);
    }

    if (fBM && (fIsBW != isBW || fWidth < glyph.width() || fHeight < glyph.height())) {
        DeleteObject(fBM);
        fBM = 0;
    }
    fIsBW = isBW;

    fWidth = SkMax32(fWidth, glyph.width());
    fHeight = SkMax32(fHeight, glyph.height());

    int biWidth = isBW ? alignTo32(fWidth) : fWidth;

    if (0 == fBM) {
        MyBitmapInfo info;
        sk_bzero(&info, sizeof(info));
        if (isBW) {
            RGBQUAD blackQuad = { 0, 0, 0, 0 };
            RGBQUAD whiteQuad = { 0xFF, 0xFF, 0xFF, 0 };
            info.bmiColors[0] = blackQuad;
            info.bmiColors[1] = whiteQuad;
        }
        info.bmiHeader.biSize = sizeof(info.bmiHeader);
        info.bmiHeader.biWidth = biWidth;
        info.bmiHeader.biHeight = fHeight;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = isBW ? 1 : 32;
        info.bmiHeader.biCompression = BI_RGB;
        if (isBW) {
            info.bmiHeader.biClrUsed = 2;
        }
        fBM = CreateDIBSection(fDC, &info, DIB_RGB_COLORS, &fBits, 0, 0);
        if (0 == fBM) {
            return nullptr;
        }
        SelectObject(fDC, fBM);
    }

    // erase
    size_t srcRB = isBW ? (biWidth >> 3) : (fWidth << 2);
    size_t size = fHeight * srcRB;
    memset(fBits, 0, size);

    XFORM xform = fXform;
    xform.eDx = (float)-glyph.left();
    xform.eDy = (float)-glyph.top();
    SetWorldTransform(fDC, &xform);

    uint16_t glyphID = glyph.getGlyphID();
    BOOL ret = ExtTextOutW(fDC, 0, 0, ETO_GLYPH_INDEX, nullptr, reinterpret_cast<LPCWSTR>(&glyphID), 1, nullptr);
    GdiFlush();
    if (0 == ret) {
        return nullptr;
    }
    *srcRBPtr = srcRB;
    // offset to the start of the image
    return (const char*)fBits + (fHeight - glyph.height()) * srcRB;
}

//////////////////////////////////////////////////////////////////////////////
#define BUFFERSIZE (1 << 13)

class SkScalerContext_GDI : public SkScalerContext {
public:
    SkScalerContext_GDI(sk_sp<LogFontTypeface>,
                        const SkScalerContextEffects&,
                        const SkDescriptor* desc);
    ~SkScalerContext_GDI() override;

    // Returns true if the constructor was able to complete all of its
    // initializations (which may include calling GDI).
    bool isValid() const;

protected:
    unsigned generateGlyphCount() override;
    bool generateAdvance(SkGlyph* glyph) override;
    void generateMetrics(SkGlyph* glyph) override;
    void generateImage(const SkGlyph& glyph) override;
    bool generatePath(SkGlyphID glyph, SkPath* path) override;
    void generateFontMetrics(SkFontMetrics*) override;

private:
    DWORD getGDIGlyphPath(SkGlyphID glyph, UINT flags,
                          SkAutoSTMalloc<BUFFERSIZE, uint8_t>* glyphbuf);
    template<bool APPLY_PREBLEND>
    static void RGBToA8(const SkGdiRGB* SK_RESTRICT src, size_t srcRB,
                        const SkGlyph& glyph, const uint8_t* table8);

    template<bool APPLY_PREBLEND>
    static void RGBToLcd16(const SkGdiRGB* SK_RESTRICT src, size_t srcRB, const SkGlyph& glyph,
                           const uint8_t* tableR, const uint8_t* tableG, const uint8_t* tableB);

    HDCOffscreen fOffscreen;
    /** fGsA is the non-rotational part of total matrix without the text height scale.
     *  Used to find the magnitude of advances.
     */
    MAT2         fGsA;
    /** The total matrix without the textSize. */
    MAT2         fMat22;
    /** Scales font to EM size. */
    MAT2         fHighResMat22;
    HDC          fDDC;
    HFONT        fSavefont;
    HFONT        fFont;
    SCRIPT_CACHE fSC;
    int          fGlyphCount;

    /** The total matrix which also removes EM scale. */
    SkMatrix     fHiResMatrix;
    /** fG_inv is the inverse of the rotational part of the total matrix.
     *  Used to set the direction of advances.
     */
    SkMatrix     fG_inv;
    enum Type {
        kTrueType_Type, kBitmap_Type, kLine_Type
    } fType;
    TEXTMETRIC fTM;
};

static FIXED float2FIXED(float x) {
    return SkFixedToFIXED(SkFloatToFixed(x));
}

static inline float FIXED2float(FIXED x) {
    return SkFixedToFloat(SkFIXEDToFixed(x));
}

static BYTE compute_quality(const SkScalerContextRec& rec) {
    switch (rec.fMaskFormat) {
        case SkMask::kBW_Format:
            return NONANTIALIASED_QUALITY;
        case SkMask::kLCD16_Format:
            return CLEARTYPE_QUALITY;
        default:
            if (rec.fFlags & SkScalerContext::kGenA8FromLCD_Flag) {
                return CLEARTYPE_QUALITY;
            } else {
                return ANTIALIASED_QUALITY;
            }
    }
}

SkScalerContext_GDI::SkScalerContext_GDI(sk_sp<LogFontTypeface> rawTypeface,
                                         const SkScalerContextEffects& effects,
                                         const SkDescriptor* desc)
        : SkScalerContext(std::move(rawTypeface), effects, desc)
        , fDDC(0)
        , fSavefont(0)
        , fFont(0)
        , fSC(0)
        , fGlyphCount(-1)
{
    LogFontTypeface* typeface = static_cast<LogFontTypeface*>(this->getTypeface());

    fDDC = ::CreateCompatibleDC(nullptr);
    if (!fDDC) {
        return;
    }
    SetGraphicsMode(fDDC, GM_ADVANCED);
    SetBkMode(fDDC, TRANSPARENT);

    // When GDI hinting, remove the entire Y scale from sA and GsA. (Prevents 'linear' metrics.)
    // When not hinting, remove only the integer Y scale from sA and GsA. (Applied by GDI.)
    SkScalerContextRec::PreMatrixScale scaleConstraints =
        (fRec.getHinting() == SkFontHinting::kNone || fRec.getHinting() == SkFontHinting::kSlight)
                   ? SkScalerContextRec::kVerticalInteger_PreMatrixScale
                   : SkScalerContextRec::kVertical_PreMatrixScale;
    SkVector scale;
    SkMatrix sA;
    SkMatrix GsA;
    SkMatrix A;
    fRec.computeMatrices(scaleConstraints, &scale, &sA, &GsA, &fG_inv, &A);

    fGsA.eM11 = SkScalarToFIXED(GsA.get(SkMatrix::kMScaleX));
    fGsA.eM12 = SkScalarToFIXED(-GsA.get(SkMatrix::kMSkewY)); // This should be ~0.
    fGsA.eM21 = SkScalarToFIXED(-GsA.get(SkMatrix::kMSkewX));
    fGsA.eM22 = SkScalarToFIXED(GsA.get(SkMatrix::kMScaleY));

    // When not hinting, scale was computed with kVerticalInteger, so is already an integer.
    // The sA and GsA transforms will be used to create 'linear' metrics.

    // When hinting, scale was computed with kVertical, stating that our port can handle
    // non-integer scales. This is done so that sA and GsA are computed without any 'residual'
    // scale in them, preventing 'linear' metrics. However, GDI cannot actually handle non-integer
    // scales so we need to round in this case. This is fine, since all of the scale has been
    // removed from sA and GsA, so GDI will be handling the scale completely.
    SkScalar gdiTextSize = SkScalarRoundToScalar(scale.fY);

    // GDI will not accept a size of zero, so round the range [0, 1] to 1.
    // If the size was non-zero, the scale factors will also be non-zero and 1px tall text is drawn.
    // If the size actually was zero, the scale factors will also be zero, so GDI will draw nothing.
    if (gdiTextSize == 0) {
        gdiTextSize = SK_Scalar1;
    }

    LOGFONT lf = typeface->fLogFont;
    lf.lfHeight = -SkScalarTruncToInt(gdiTextSize);
    lf.lfQuality = compute_quality(fRec);
    fFont = CreateFontIndirect(&lf);
    if (!fFont) {
        return;
    }

    fSavefont = (HFONT)SelectObject(fDDC, fFont);

    if (0 == GetTextMetrics(fDDC, &fTM)) {
        call_ensure_accessible(lf);
        if (0 == GetTextMetrics(fDDC, &fTM)) {
            fTM.tmPitchAndFamily = TMPF_TRUETYPE;
        }
    }

    XFORM xform;
    if (fTM.tmPitchAndFamily & TMPF_VECTOR) {
        // Used a logfont on a memory context, should never get a device font.
        // Therefore all TMPF_DEVICE will be PostScript fonts.

        // If TMPF_VECTOR is set, one of TMPF_TRUETYPE or TMPF_DEVICE means that
        // we have an outline font. Otherwise we have a vector FON, which is
        // scalable, but not an outline font.
        // This was determined by testing with Type1 PFM/PFB and
        // OpenTypeCFF OTF, as well as looking at Wine bugs and sources.
        if (fTM.tmPitchAndFamily & (TMPF_TRUETYPE | TMPF_DEVICE)) {
            // Truetype or PostScript.
            fType = SkScalerContext_GDI::kTrueType_Type;
        } else {
            // Stroked FON.
            fType = SkScalerContext_GDI::kLine_Type;
        }

        // fPost2x2 is column-major, left handed (y down).
        // XFORM 2x2 is row-major, left handed (y down).
        xform.eM11 = SkScalarToFloat(sA.get(SkMatrix::kMScaleX));
        xform.eM12 = SkScalarToFloat(sA.get(SkMatrix::kMSkewY));
        xform.eM21 = SkScalarToFloat(sA.get(SkMatrix::kMSkewX));
        xform.eM22 = SkScalarToFloat(sA.get(SkMatrix::kMScaleY));
        xform.eDx = 0;
        xform.eDy = 0;

        // MAT2 is row major, right handed (y up).
        fMat22.eM11 = float2FIXED(xform.eM11);
        fMat22.eM12 = float2FIXED(-xform.eM12);
        fMat22.eM21 = float2FIXED(-xform.eM21);
        fMat22.eM22 = float2FIXED(xform.eM22);

        if (needToRenderWithSkia(fRec)) {
            this->forceGenerateImageFromPath();
        }

        // Create a hires matrix if we need linear metrics.
        if (this->isLinearMetrics()) {
            OUTLINETEXTMETRIC otm;
            UINT success = GetOutlineTextMetrics(fDDC, sizeof(otm), &otm);
            if (0 == success) {
                call_ensure_accessible(lf);
                success = GetOutlineTextMetrics(fDDC, sizeof(otm), &otm);
            }
            if (0 != success) {
                SkScalar upem = SkIntToScalar(otm.otmEMSquare);

                SkScalar gdiTextSizeToEMScale = upem / gdiTextSize;
                fHighResMat22.eM11 = float2FIXED(gdiTextSizeToEMScale);
                fHighResMat22.eM12 = float2FIXED(0);
                fHighResMat22.eM21 = float2FIXED(0);
                fHighResMat22.eM22 = float2FIXED(gdiTextSizeToEMScale);

                SkScalar removeEMScale = SkScalarInvert(upem);
                fHiResMatrix = A;
                fHiResMatrix.preScale(removeEMScale, removeEMScale);
            }
        }

    } else {
        // Assume bitmap
        fType = SkScalerContext_GDI::kBitmap_Type;

        xform.eM11 = 1.0f;
        xform.eM12 = 0.0f;
        xform.eM21 = 0.0f;
        xform.eM22 = 1.0f;
        xform.eDx = 0.0f;
        xform.eDy = 0.0f;

        // fPost2x2 is column-major, left handed (y down).
        // MAT2 is row major, right handed (y up).
        fMat22.eM11 = SkScalarToFIXED(fRec.fPost2x2[0][0]);
        fMat22.eM12 = SkScalarToFIXED(-fRec.fPost2x2[1][0]);
        fMat22.eM21 = SkScalarToFIXED(-fRec.fPost2x2[0][1]);
        fMat22.eM22 = SkScalarToFIXED(fRec.fPost2x2[1][1]);
    }

    fOffscreen.init(fFont, xform);
}

SkScalerContext_GDI::~SkScalerContext_GDI() {
    if (fDDC) {
        ::SelectObject(fDDC, fSavefont);
        ::DeleteDC(fDDC);
    }
    if (fFont) {
        ::DeleteObject(fFont);
    }
    if (fSC) {
        ::ScriptFreeCache(&fSC);
    }
}

bool SkScalerContext_GDI::isValid() const {
    return fDDC && fFont;
}

unsigned SkScalerContext_GDI::generateGlyphCount() {
    if (fGlyphCount < 0) {
        fGlyphCount = calculateGlyphCount(
                          fDDC, static_cast<const LogFontTypeface*>(this->getTypeface())->fLogFont);
    }
    return fGlyphCount;
}

bool SkScalerContext_GDI::generateAdvance(SkGlyph* glyph) {
    return false;
}

void SkScalerContext_GDI::generateMetrics(SkGlyph* glyph) {
    SkASSERT(fDDC);

    glyph->fMaskFormat = fRec.fMaskFormat;

    if (fType == SkScalerContext_GDI::kBitmap_Type || fType == SkScalerContext_GDI::kLine_Type) {
        SIZE size;
        WORD glyphs = glyph->getGlyphID();
        if (0 == GetTextExtentPointI(fDDC, &glyphs, 1, &size)) {
            glyph->fWidth = SkToS16(fTM.tmMaxCharWidth);
            glyph->fHeight = SkToS16(fTM.tmHeight);
        } else {
            glyph->fWidth = SkToS16(size.cx);
            glyph->fHeight = SkToS16(size.cy);
        }

        glyph->fTop = SkToS16(-fTM.tmAscent);
        // Bitmap FON cannot underhang, but vector FON may.
        // There appears no means of determining underhang of vector FON.
        glyph->fLeft = SkToS16(0);
        glyph->fAdvanceX = glyph->width();
        glyph->fAdvanceY = 0;

        // Vector FON will transform nicely, but bitmap FON do not.
        if (fType == SkScalerContext_GDI::kLine_Type) {
            SkRect bounds = SkRect::MakeXYWH(glyph->fLeft, glyph->fTop,
                                             glyph->width(), glyph->height());
            SkMatrix m;
            m.setAll(SkFIXEDToScalar(fMat22.eM11), -SkFIXEDToScalar(fMat22.eM21), 0,
                     -SkFIXEDToScalar(fMat22.eM12), SkFIXEDToScalar(fMat22.eM22), 0,
                     0,  0, 1);
            m.mapRect(&bounds);
            bounds.roundOut(&bounds);
            glyph->fLeft = SkScalarTruncToInt(bounds.fLeft);
            glyph->fTop = SkScalarTruncToInt(bounds.fTop);
            glyph->fWidth = SkScalarTruncToInt(bounds.width());
            glyph->fHeight = SkScalarTruncToInt(bounds.height());
        }

        // Apply matrix to advance.
        glyph->fAdvanceY = -FIXED2float(fMat22.eM12) * glyph->fAdvanceX;
        glyph->fAdvanceX *= FIXED2float(fMat22.eM11);

        return;
    }

    UINT glyphId = glyph->getGlyphID();

    GLYPHMETRICS gm;
    sk_bzero(&gm, sizeof(gm));

    DWORD status = GetGlyphOutlineW(fDDC, glyphId, GGO_METRICS | GGO_GLYPH_INDEX, &gm, 0, nullptr, &fMat22);
    if (GDI_ERROR == status) {
        LogFontTypeface::EnsureAccessible(this->getTypeface());
        status = GetGlyphOutlineW(fDDC, glyphId, GGO_METRICS | GGO_GLYPH_INDEX, &gm, 0, nullptr, &fMat22);
        if (GDI_ERROR == status) {
            glyph->zeroMetrics();
            return;
        }
    }

    bool empty = false;
    // The black box is either the embedded bitmap size or the outline extent.
    // It is 1x1 if nothing is to be drawn, but will also be 1x1 if something very small
    // is to be drawn, like a '.'. We need to outset '.' but do not wish to outset ' '.
    if (1 == gm.gmBlackBoxX && 1 == gm.gmBlackBoxY) {
        // If GetGlyphOutline with GGO_NATIVE returns 0, we know there was no outline.
        DWORD bufferSize = GetGlyphOutlineW(fDDC, glyphId, GGO_NATIVE | GGO_GLYPH_INDEX, &gm, 0, nullptr, &fMat22);
        empty = (0 == bufferSize);
    }

    glyph->fTop = SkToS16(-gm.gmptGlyphOrigin.y);
    glyph->fLeft = SkToS16(gm.gmptGlyphOrigin.x);
    if (empty) {
        glyph->fWidth = 0;
        glyph->fHeight = 0;
    } else {
        // Outset, since the image may bleed out of the black box.
        // For embedded bitmaps the black box should be exact.
        // For outlines we need to outset by 1 in all directions for bleed.
        // For ClearType we need to outset by 2 for bleed.
        glyph->fWidth = gm.gmBlackBoxX + 4;
        glyph->fHeight = gm.gmBlackBoxY + 4;
        glyph->fTop -= 2;
        glyph->fLeft -= 2;
    }
    // TODO(benjaminwagner): What is the type of gm.gmCellInc[XY]?
    glyph->fAdvanceX = (float)((int)gm.gmCellIncX);
    glyph->fAdvanceY = (float)((int)gm.gmCellIncY);

    if ((fTM.tmPitchAndFamily & TMPF_VECTOR) && this->isLinearMetrics()) {
        sk_bzero(&gm, sizeof(gm));
        status = GetGlyphOutlineW(fDDC, glyphId, GGO_METRICS | GGO_GLYPH_INDEX, &gm, 0, nullptr, &fHighResMat22);
        if (GDI_ERROR != status) {
            SkPoint advance;
            fHiResMatrix.mapXY(SkIntToScalar(gm.gmCellIncX), SkIntToScalar(gm.gmCellIncY), &advance);
            glyph->fAdvanceX = SkScalarToFloat(advance.fX);
            glyph->fAdvanceY = SkScalarToFloat(advance.fY);
        }
    } else if (!isAxisAligned(this->fRec)) {
        status = GetGlyphOutlineW(fDDC, glyphId, GGO_METRICS | GGO_GLYPH_INDEX, &gm, 0, nullptr, &fGsA);
        if (GDI_ERROR != status) {
            SkPoint advance;
            fG_inv.mapXY(SkIntToScalar(gm.gmCellIncX), SkIntToScalar(gm.gmCellIncY), &advance);
            glyph->fAdvanceX = SkScalarToFloat(advance.fX);
            glyph->fAdvanceY = SkScalarToFloat(advance.fY);
        }
    }
}

static const MAT2 gMat2Identity = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
void SkScalerContext_GDI::generateFontMetrics(SkFontMetrics* metrics) {
    if (nullptr == metrics) {
        return;
    }
    sk_bzero(metrics, sizeof(*metrics));

    SkASSERT(fDDC);

#ifndef SK_GDI_ALWAYS_USE_TEXTMETRICS_FOR_FONT_METRICS
    if (fType == SkScalerContext_GDI::kBitmap_Type || fType == SkScalerContext_GDI::kLine_Type) {
#endif
        metrics->fTop = SkIntToScalar(-fTM.tmAscent);
        metrics->fAscent = SkIntToScalar(-fTM.tmAscent);
        metrics->fDescent = SkIntToScalar(fTM.tmDescent);
        metrics->fBottom = SkIntToScalar(fTM.tmDescent);
        metrics->fLeading = SkIntToScalar(fTM.tmExternalLeading);
        metrics->fAvgCharWidth = SkIntToScalar(fTM.tmAveCharWidth);
        metrics->fMaxCharWidth = SkIntToScalar(fTM.tmMaxCharWidth);
        metrics->fXMin = 0;
        metrics->fXMax = metrics->fMaxCharWidth;
        //metrics->fXHeight = 0;
#ifndef SK_GDI_ALWAYS_USE_TEXTMETRICS_FOR_FONT_METRICS
        return;
    }
#endif

    OUTLINETEXTMETRIC otm;

    uint32_t ret = GetOutlineTextMetrics(fDDC, sizeof(otm), &otm);
    if (0 == ret) {
        LogFontTypeface::EnsureAccessible(this->getTypeface());
        ret = GetOutlineTextMetrics(fDDC, sizeof(otm), &otm);
    }
    if (0 == ret) {
        return;
    }

#ifndef SK_GDI_ALWAYS_USE_TEXTMETRICS_FOR_FONT_METRICS
    metrics->fTop = SkIntToScalar(-otm.otmrcFontBox.top);
    metrics->fAscent = SkIntToScalar(-otm.otmAscent);
    metrics->fDescent = SkIntToScalar(-otm.otmDescent);
    metrics->fBottom = SkIntToScalar(-otm.otmrcFontBox.bottom);
    metrics->fLeading = SkIntToScalar(otm.otmLineGap);
    metrics->fAvgCharWidth = SkIntToScalar(otm.otmTextMetrics.tmAveCharWidth);
    metrics->fMaxCharWidth = SkIntToScalar(otm.otmTextMetrics.tmMaxCharWidth);
    metrics->fXMin = SkIntToScalar(otm.otmrcFontBox.left);
    metrics->fXMax = SkIntToScalar(otm.otmrcFontBox.right);
#endif
    metrics->fUnderlineThickness = SkIntToScalar(otm.otmsUnderscoreSize);
    metrics->fUnderlinePosition = -SkIntToScalar(otm.otmsUnderscorePosition);

    metrics->fFlags |= SkFontMetrics::kUnderlineThicknessIsValid_Flag;
    metrics->fFlags |= SkFontMetrics::kUnderlinePositionIsValid_Flag;

    metrics->fXHeight = SkIntToScalar(otm.otmsXHeight);
    GLYPHMETRICS gm;
    sk_bzero(&gm, sizeof(gm));
    DWORD len = GetGlyphOutlineW(fDDC, 'x', GGO_METRICS, &gm, 0, 0, &gMat2Identity);
    if (len != GDI_ERROR && gm.gmBlackBoxY > 0) {
        metrics->fXHeight = SkIntToScalar(gm.gmBlackBoxY);
    }
}

////////////////////////////////////////////////////////////////////////////////////////

#define SK_SHOW_TEXT_BLIT_COVERAGE 0

static void build_power_table(uint8_t table[], float ee) {
    for (int i = 0; i < 256; i++) {
        float x = i / 255.f;
        x = sk_float_pow(x, ee);
        int xx = SkScalarRoundToInt(x * 255);
        table[i] = SkToU8(xx);
    }
}

/**
 *  This will invert the gamma applied by GDI (gray-scale antialiased), so we
 *  can get linear values.
 *
 *  GDI grayscale appears to use a hard-coded gamma of 2.3.
 *
 *  GDI grayscale appears to draw using the black and white rasterizer at four
 *  times the size and then downsamples to compute the coverage mask. As a
 *  result there are only seventeen total grays. This lack of fidelity means
 *  that shifting into other color spaces is imprecise.
 */
static const uint8_t* getInverseGammaTableGDI() {
    static SkOnce once;
    static uint8_t gTableGdi[256];
    once([]{
        build_power_table(gTableGdi, 2.3f);
    });
    return gTableGdi;
}

/**
 *  This will invert the gamma applied by GDI ClearType, so we can get linear
 *  values.
 *
 *  GDI ClearType uses SPI_GETFONTSMOOTHINGCONTRAST / 1000 as the gamma value.
 *  If this value is not specified, the default is a gamma of 1.4.
 */
static const uint8_t* getInverseGammaTableClearType() {
    static SkOnce once;
    static uint8_t gTableClearType[256];
    once([]{
        UINT level = 0;
        if (!SystemParametersInfo(SPI_GETFONTSMOOTHINGCONTRAST, 0, &level, 0) || !level) {
            // can't get the data, so use a default
            level = 1400;
        }
        build_power_table(gTableClearType, level / 1000.0f);
    });
    return gTableClearType;
}

#include "include/private/SkColorData.h"

//Cannot assume that the input rgb is gray due to possible setting of kGenA8FromLCD_Flag.
template<bool APPLY_PREBLEND>
static inline uint8_t rgb_to_a8(SkGdiRGB rgb, const uint8_t* table8) {
    U8CPU r = (rgb >> 16) & 0xFF;
    U8CPU g = (rgb >>  8) & 0xFF;
    U8CPU b = (rgb >>  0) & 0xFF;
    return sk_apply_lut_if<APPLY_PREBLEND>(SkComputeLuminance(r, g, b), table8);
}

template<bool APPLY_PREBLEND>
static inline uint16_t rgb_to_lcd16(SkGdiRGB rgb, const uint8_t* tableR,
                                                  const uint8_t* tableG,
                                                  const uint8_t* tableB) {
    U8CPU r = sk_apply_lut_if<APPLY_PREBLEND>((rgb >> 16) & 0xFF, tableR);
    U8CPU g = sk_apply_lut_if<APPLY_PREBLEND>((rgb >>  8) & 0xFF, tableG);
    U8CPU b = sk_apply_lut_if<APPLY_PREBLEND>((rgb >>  0) & 0xFF, tableB);
#if SK_SHOW_TEXT_BLIT_COVERAGE
    r = SkMax32(r, 10); g = SkMax32(g, 10); b = SkMax32(b, 10);
#endif
    return SkPack888ToRGB16(r, g, b);
}

template<bool APPLY_PREBLEND>
void SkScalerContext_GDI::RGBToA8(const SkGdiRGB* SK_RESTRICT src, size_t srcRB,
                                  const SkGlyph& glyph, const uint8_t* table8) {
    const size_t dstRB = glyph.rowBytes();
    const int width = glyph.width();
    uint8_t* SK_RESTRICT dst = (uint8_t*)((char*)glyph.fImage + (glyph.height() - 1) * dstRB);

    for (int y = 0; y < glyph.fHeight; y++) {
        for (int i = 0; i < width; i++) {
            dst[i] = rgb_to_a8<APPLY_PREBLEND>(src[i], table8);
#if SK_SHOW_TEXT_BLIT_COVERAGE
            dst[i] = SkMax32(dst[i], 10);
#endif
        }
        src = SkTAddOffset<const SkGdiRGB>(src, srcRB);
        dst -= dstRB;
    }
}

template<bool APPLY_PREBLEND>
void SkScalerContext_GDI::RGBToLcd16(
        const SkGdiRGB* SK_RESTRICT src, size_t srcRB, const SkGlyph& glyph,
        const uint8_t* tableR, const uint8_t* tableG, const uint8_t* tableB) {
    const size_t dstRB = glyph.rowBytes();
    const int width = glyph.width();
    uint16_t* SK_RESTRICT dst = (uint16_t*)((char*)glyph.fImage + (glyph.height() - 1) * dstRB);

    for (int y = 0; y < glyph.fHeight; y++) {
        for (int i = 0; i < width; i++) {
            dst[i] = rgb_to_lcd16<APPLY_PREBLEND>(src[i], tableR, tableG, tableB);
        }
        src = SkTAddOffset<const SkGdiRGB>(src, srcRB);
        dst = (uint16_t*)((char*)dst - dstRB);
    }
}

void SkScalerContext_GDI::generateImage(const SkGlyph& glyph) {
    SkASSERT(fDDC);

    const bool isBW = SkMask::kBW_Format == fRec.fMaskFormat;
    const bool isAA = !isLCD(fRec);

    size_t srcRB;
    const void* bits = fOffscreen.draw(glyph, isBW, &srcRB);
    if (nullptr == bits) {
        LogFontTypeface::EnsureAccessible(this->getTypeface());
        bits = fOffscreen.draw(glyph, isBW, &srcRB);
        if (nullptr == bits) {
            sk_bzero(glyph.fImage, glyph.imageSize());
            return;
        }
    }

    if (!isBW) {
        const uint8_t* table;
        //The offscreen contains a GDI blit if isAA and kGenA8FromLCD_Flag is not set.
        //Otherwise the offscreen contains a ClearType blit.
        if (isAA && !(fRec.fFlags & SkScalerContext::kGenA8FromLCD_Flag)) {
            table = getInverseGammaTableGDI();
        } else {
            table = getInverseGammaTableClearType();
        }
        //Note that the following cannot really be integrated into the
        //pre-blend, since we may not be applying the pre-blend; when we aren't
        //applying the pre-blend it means that a filter wants linear anyway.
        //Other code may also be applying the pre-blend, so we'd need another
        //one with this and one without.
        SkGdiRGB* addr = (SkGdiRGB*)bits;
        for (int y = 0; y < glyph.fHeight; ++y) {
            for (int x = 0; x < glyph.width(); ++x) {
                int r = (addr[x] >> 16) & 0xFF;
                int g = (addr[x] >>  8) & 0xFF;
                int b = (addr[x] >>  0) & 0xFF;
                addr[x] = (table[r] << 16) | (table[g] << 8) | table[b];
            }
            addr = SkTAddOffset<SkGdiRGB>(addr, srcRB);
        }
    }

    size_t dstRB = glyph.rowBytes();
    if (isBW) {
        const uint8_t* src = (const uint8_t*)bits;
        uint8_t* dst = (uint8_t*)((char*)glyph.fImage + (glyph.fHeight - 1) * dstRB);
        for (int y = 0; y < glyph.fHeight; y++) {
            memcpy(dst, src, dstRB);
            src += srcRB;
            dst -= dstRB;
        }
#if SK_SHOW_TEXT_BLIT_COVERAGE
            if (glyph.width() > 0 && glyph.fHeight > 0) {
                int bitCount = glyph.width() & 7;
                uint8_t* first = (uint8_t*)glyph.fImage;
                uint8_t* last = (uint8_t*)((char*)glyph.fImage + glyph.height() * dstRB - 1);
                *first |= 1 << 7;
                *last |= bitCount == 0 ? 1 : 1 << (8 - bitCount);
            }
#endif
    } else if (isAA) {
        // since the caller may require A8 for maskfilters, we can't check for BW
        // ... until we have the caller tell us that explicitly
        const SkGdiRGB* src = (const SkGdiRGB*)bits;
        if (fPreBlend.isApplicable()) {
            RGBToA8<true>(src, srcRB, glyph, fPreBlend.fG);
        } else {
            RGBToA8<false>(src, srcRB, glyph, fPreBlend.fG);
        }
    } else {    // LCD16
        const SkGdiRGB* src = (const SkGdiRGB*)bits;
        SkASSERT(SkMask::kLCD16_Format == glyph.fMaskFormat);
        if (fPreBlend.isApplicable()) {
            RGBToLcd16<true>(src, srcRB, glyph, fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
        } else {
            RGBToLcd16<false>(src, srcRB, glyph, fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
        }
    }
}

class GDIGlyphbufferPointIter {
public:
    GDIGlyphbufferPointIter(const uint8_t* glyphbuf, DWORD total_size)
        : fHeaderIter(glyphbuf, total_size), fCurveIter(), fPointIter()
    { }

    POINTFX const * next() {
nextHeader:
        if (!fCurveIter.isSet()) {
            const TTPOLYGONHEADER* header = fHeaderIter.next();
            if (nullptr == header) {
                return nullptr;
            }
            fCurveIter.set(header);
            const TTPOLYCURVE* curve = fCurveIter.next();
            if (nullptr == curve) {
                return nullptr;
            }
            fPointIter.set(curve);
            return &header->pfxStart;
        }

        const POINTFX* nextPoint = fPointIter.next();
        if (nullptr == nextPoint) {
            const TTPOLYCURVE* curve = fCurveIter.next();
            if (nullptr == curve) {
                fCurveIter.set();
                goto nextHeader;
            } else {
                fPointIter.set(curve);
            }
            nextPoint = fPointIter.next();
        }
        return nextPoint;
    }

    WORD currentCurveType() {
        return fPointIter.fCurveType;
    }

private:
    /** Iterates over all of the polygon headers in a glyphbuf. */
    class GDIPolygonHeaderIter {
    public:
        GDIPolygonHeaderIter(const uint8_t* glyphbuf, DWORD total_size)
            : fCurPolygon(reinterpret_cast<const TTPOLYGONHEADER*>(glyphbuf))
            , fEndPolygon(SkTAddOffset<const TTPOLYGONHEADER>(glyphbuf, total_size))
        { }

        const TTPOLYGONHEADER* next() {
            if (fCurPolygon >= fEndPolygon) {
                return nullptr;
            }
            const TTPOLYGONHEADER* thisPolygon = fCurPolygon;
            fCurPolygon = SkTAddOffset<const TTPOLYGONHEADER>(fCurPolygon, fCurPolygon->cb);
            return thisPolygon;
        }
    private:
        const TTPOLYGONHEADER* fCurPolygon;
        const TTPOLYGONHEADER* fEndPolygon;
    };

    /** Iterates over all of the polygon curves in a polygon header. */
    class GDIPolygonCurveIter {
    public:
        GDIPolygonCurveIter() : fCurCurve(nullptr), fEndCurve(nullptr) { }

        GDIPolygonCurveIter(const TTPOLYGONHEADER* curPolygon)
            : fCurCurve(SkTAddOffset<const TTPOLYCURVE>(curPolygon, sizeof(TTPOLYGONHEADER)))
            , fEndCurve(SkTAddOffset<const TTPOLYCURVE>(curPolygon, curPolygon->cb))
        { }

        bool isSet() { return fCurCurve != nullptr; }

        void set(const TTPOLYGONHEADER* curPolygon) {
            fCurCurve = SkTAddOffset<const TTPOLYCURVE>(curPolygon, sizeof(TTPOLYGONHEADER));
            fEndCurve = SkTAddOffset<const TTPOLYCURVE>(curPolygon, curPolygon->cb);
        }
        void set() {
            fCurCurve = nullptr;
            fEndCurve = nullptr;
        }

        const TTPOLYCURVE* next() {
            if (fCurCurve >= fEndCurve) {
                return nullptr;
            }
            const TTPOLYCURVE* thisCurve = fCurCurve;
            fCurCurve = SkTAddOffset<const TTPOLYCURVE>(fCurCurve, size_of_TTPOLYCURVE(*fCurCurve));
            return thisCurve;
        }
    private:
        size_t size_of_TTPOLYCURVE(const TTPOLYCURVE& curve) {
            return 2*sizeof(WORD) + curve.cpfx*sizeof(POINTFX);
        }
        const TTPOLYCURVE* fCurCurve;
        const TTPOLYCURVE* fEndCurve;
    };

    /** Iterates over all of the polygon points in a polygon curve. */
    class GDIPolygonCurvePointIter {
    public:
        GDIPolygonCurvePointIter() : fCurveType(0), fCurPoint(nullptr), fEndPoint(nullptr) { }

        GDIPolygonCurvePointIter(const TTPOLYCURVE* curPolygon)
            : fCurveType(curPolygon->wType)
            , fCurPoint(&curPolygon->apfx[0])
            , fEndPoint(&curPolygon->apfx[curPolygon->cpfx])
        { }

        bool isSet() { return fCurPoint != nullptr; }

        void set(const TTPOLYCURVE* curPolygon) {
            fCurveType = curPolygon->wType;
            fCurPoint = &curPolygon->apfx[0];
            fEndPoint = &curPolygon->apfx[curPolygon->cpfx];
        }
        void set() {
            fCurPoint = nullptr;
            fEndPoint = nullptr;
        }

        const POINTFX* next() {
            if (fCurPoint >= fEndPoint) {
                return nullptr;
            }
            const POINTFX* thisPoint = fCurPoint;
            ++fCurPoint;
            return thisPoint;
        }

        WORD fCurveType;
    private:
        const POINTFX* fCurPoint;
        const POINTFX* fEndPoint;
    };

    GDIPolygonHeaderIter fHeaderIter;
    GDIPolygonCurveIter fCurveIter;
    GDIPolygonCurvePointIter fPointIter;
};

static void sk_path_from_gdi_path(SkPath* path, const uint8_t* glyphbuf, DWORD total_size) {
    const uint8_t* cur_glyph = glyphbuf;
    const uint8_t* end_glyph = glyphbuf + total_size;

    while (cur_glyph < end_glyph) {
        const TTPOLYGONHEADER* th = (TTPOLYGONHEADER*)cur_glyph;

        const uint8_t* end_poly = cur_glyph + th->cb;
        const uint8_t* cur_poly = cur_glyph + sizeof(TTPOLYGONHEADER);

        path->moveTo(SkFixedToScalar( SkFIXEDToFixed(th->pfxStart.x)),
                     SkFixedToScalar(-SkFIXEDToFixed(th->pfxStart.y)));

        while (cur_poly < end_poly) {
            const TTPOLYCURVE* pc = (const TTPOLYCURVE*)cur_poly;

            if (pc->wType == TT_PRIM_LINE) {
                for (uint16_t i = 0; i < pc->cpfx; i++) {
                    path->lineTo(SkFixedToScalar( SkFIXEDToFixed(pc->apfx[i].x)),
                                 SkFixedToScalar(-SkFIXEDToFixed(pc->apfx[i].y)));
                }
            }

            if (pc->wType == TT_PRIM_QSPLINE) {
                for (uint16_t u = 0; u < pc->cpfx - 1; u++) { // Walk through points in spline
                    POINTFX pnt_b = pc->apfx[u];    // B is always the current point
                    POINTFX pnt_c = pc->apfx[u+1];

                    if (u < pc->cpfx - 2) {          // If not on last spline, compute C
                        pnt_c.x = SkFixedToFIXED(SkFixedAve(SkFIXEDToFixed(pnt_b.x),
                                                            SkFIXEDToFixed(pnt_c.x)));
                        pnt_c.y = SkFixedToFIXED(SkFixedAve(SkFIXEDToFixed(pnt_b.y),
                                                            SkFIXEDToFixed(pnt_c.y)));
                    }

                    path->quadTo(SkFixedToScalar( SkFIXEDToFixed(pnt_b.x)),
                                 SkFixedToScalar(-SkFIXEDToFixed(pnt_b.y)),
                                 SkFixedToScalar( SkFIXEDToFixed(pnt_c.x)),
                                 SkFixedToScalar(-SkFIXEDToFixed(pnt_c.y)));
                }
            }
            // Advance past this TTPOLYCURVE.
            cur_poly += sizeof(WORD) * 2 + sizeof(POINTFX) * pc->cpfx;
        }
        cur_glyph += th->cb;
        path->close();
    }
}

#define move_next_expected_hinted_point(iter, pElem) do {\
    pElem = iter.next(); \
    if (nullptr == pElem) return false; \
} while(0)

// It is possible for the hinted and unhinted versions of the same path to have
// a different number of points due to GDI's handling of flipped points.
// If this is detected, this will return false.
static bool sk_path_from_gdi_paths(SkPath* path, const uint8_t* glyphbuf, DWORD total_size,
                                   GDIGlyphbufferPointIter hintedYs) {
    const uint8_t* cur_glyph = glyphbuf;
    const uint8_t* end_glyph = glyphbuf + total_size;

    POINTFX const * hintedPoint;

    while (cur_glyph < end_glyph) {
        const TTPOLYGONHEADER* th = (TTPOLYGONHEADER*)cur_glyph;

        const uint8_t* end_poly = cur_glyph + th->cb;
        const uint8_t* cur_poly = cur_glyph + sizeof(TTPOLYGONHEADER);

        move_next_expected_hinted_point(hintedYs, hintedPoint);
        path->moveTo(SkFixedToScalar( SkFIXEDToFixed(th->pfxStart.x)),
                     SkFixedToScalar(-SkFIXEDToFixed(hintedPoint->y)));

        while (cur_poly < end_poly) {
            const TTPOLYCURVE* pc = (const TTPOLYCURVE*)cur_poly;

            if (pc->wType == TT_PRIM_LINE) {
                for (uint16_t i = 0; i < pc->cpfx; i++) {
                    move_next_expected_hinted_point(hintedYs, hintedPoint);
                    path->lineTo(SkFixedToScalar( SkFIXEDToFixed(pc->apfx[i].x)),
                                 SkFixedToScalar(-SkFIXEDToFixed(hintedPoint->y)));
                }
            }

            if (pc->wType == TT_PRIM_QSPLINE) {
                POINTFX currentPoint = pc->apfx[0];
                move_next_expected_hinted_point(hintedYs, hintedPoint);
                // only take the hinted y if it wasn't flipped
                if (hintedYs.currentCurveType() == TT_PRIM_QSPLINE) {
                    currentPoint.y = hintedPoint->y;
                }
                for (uint16_t u = 0; u < pc->cpfx - 1; u++) { // Walk through points in spline
                    POINTFX pnt_b = currentPoint;//pc->apfx[u]; // B is always the current point
                    POINTFX pnt_c = pc->apfx[u+1];
                    move_next_expected_hinted_point(hintedYs, hintedPoint);
                    // only take the hinted y if it wasn't flipped
                    if (hintedYs.currentCurveType() == TT_PRIM_QSPLINE) {
                        pnt_c.y = hintedPoint->y;
                    }
                    currentPoint.x = pnt_c.x;
                    currentPoint.y = pnt_c.y;

                    if (u < pc->cpfx - 2) {          // If not on last spline, compute C
                        pnt_c.x = SkFixedToFIXED(SkFixedAve(SkFIXEDToFixed(pnt_b.x),
                                                            SkFIXEDToFixed(pnt_c.x)));
                        pnt_c.y = SkFixedToFIXED(SkFixedAve(SkFIXEDToFixed(pnt_b.y),
                                                            SkFIXEDToFixed(pnt_c.y)));
                    }

                    path->quadTo(SkFixedToScalar( SkFIXEDToFixed(pnt_b.x)),
                                 SkFixedToScalar(-SkFIXEDToFixed(pnt_b.y)),
                                 SkFixedToScalar( SkFIXEDToFixed(pnt_c.x)),
                                 SkFixedToScalar(-SkFIXEDToFixed(pnt_c.y)));
                }
            }
            // Advance past this TTPOLYCURVE.
            cur_poly += sizeof(WORD) * 2 + sizeof(POINTFX) * pc->cpfx;
        }
        cur_glyph += th->cb;
        path->close();
    }
    return true;
}

DWORD SkScalerContext_GDI::getGDIGlyphPath(SkGlyphID glyph, UINT flags,
                                           SkAutoSTMalloc<BUFFERSIZE, uint8_t>* glyphbuf)
{
    GLYPHMETRICS gm;

    DWORD total_size = GetGlyphOutlineW(fDDC, glyph, flags, &gm, BUFFERSIZE, glyphbuf->get(), &fMat22);
    // Sometimes GetGlyphOutlineW returns a number larger than BUFFERSIZE even if BUFFERSIZE > 0.
    // It has been verified that this does not involve a buffer overrun.
    if (GDI_ERROR == total_size || total_size > BUFFERSIZE) {
        // GDI_ERROR because the BUFFERSIZE was too small, or because the data was not accessible.
        // When the data is not accessable GetGlyphOutlineW fails rather quickly,
        // so just try to get the size. If that fails then ensure the data is accessible.
        total_size = GetGlyphOutlineW(fDDC, glyph, flags, &gm, 0, nullptr, &fMat22);
        if (GDI_ERROR == total_size) {
            LogFontTypeface::EnsureAccessible(this->getTypeface());
            total_size = GetGlyphOutlineW(fDDC, glyph, flags, &gm, 0, nullptr, &fMat22);
            if (GDI_ERROR == total_size) {
                // GetGlyphOutlineW is known to fail for some characters, such as spaces.
                // In these cases, just return that the glyph does not have a shape.
                return 0;
            }
        }

        glyphbuf->reset(total_size);

        DWORD ret = GetGlyphOutlineW(fDDC, glyph, flags, &gm, total_size, glyphbuf->get(), &fMat22);
        if (GDI_ERROR == ret) {
            LogFontTypeface::EnsureAccessible(this->getTypeface());
            ret = GetGlyphOutlineW(fDDC, glyph, flags, &gm, total_size, glyphbuf->get(), &fMat22);
            if (GDI_ERROR == ret) {
                SkASSERT(false);
                return 0;
            }
        }
    }
    return total_size;
}

bool SkScalerContext_GDI::generatePath(SkGlyphID glyph, SkPath* path) {
    SkASSERT(path);
    SkASSERT(fDDC);

    path->reset();

    // Out of all the fonts on a typical Windows box,
    // 25% of glyphs require more than 2KB.
    // 1% of glyphs require more than 4KB.
    // 0.01% of glyphs require more than 8KB.
    // 8KB is less than 1% of the normal 1MB stack on Windows.
    // Note that some web fonts glyphs require more than 20KB.
    //static const DWORD BUFFERSIZE = (1 << 13);

    //GDI only uses hinted outlines when axis aligned.
    UINT format = GGO_NATIVE | GGO_GLYPH_INDEX;
    if (fRec.getHinting() == SkFontHinting::kNone || fRec.getHinting() == SkFontHinting::kSlight){
        format |= GGO_UNHINTED;
    }
    SkAutoSTMalloc<BUFFERSIZE, uint8_t> glyphbuf(BUFFERSIZE);
    DWORD total_size = getGDIGlyphPath(glyph, format, &glyphbuf);
    if (0 == total_size) {
        return false;
    }

    if (fRec.getHinting() != SkFontHinting::kSlight) {
        sk_path_from_gdi_path(path, glyphbuf, total_size);
    } else {
        //GDI only uses hinted outlines when axis aligned.
        UINT format = GGO_NATIVE | GGO_GLYPH_INDEX;

        SkAutoSTMalloc<BUFFERSIZE, uint8_t> hintedGlyphbuf(BUFFERSIZE);
        DWORD hinted_total_size = getGDIGlyphPath(glyph, format, &hintedGlyphbuf);
        if (0 == hinted_total_size) {
            return false;
        }

        if (!sk_path_from_gdi_paths(path, glyphbuf, total_size,
                                    GDIGlyphbufferPointIter(hintedGlyphbuf, hinted_total_size)))
        {
            path->reset();
            sk_path_from_gdi_path(path, glyphbuf, total_size);
        }
    }
    return true;
}

static void logfont_for_name(const char* familyName, LOGFONT* lf) {
    sk_bzero(lf, sizeof(LOGFONT));
#ifdef UNICODE
    // Get the buffer size needed first.
    size_t str_len = ::MultiByteToWideChar(CP_UTF8, 0, familyName,
                                            -1, nullptr, 0);
    // Allocate a buffer (str_len already has terminating null
    // accounted for).
    wchar_t *wideFamilyName = new wchar_t[str_len];
    // Now actually convert the string.
    ::MultiByteToWideChar(CP_UTF8, 0, familyName, -1,
                            wideFamilyName, str_len);
    ::wcsncpy(lf->lfFaceName, wideFamilyName, LF_FACESIZE - 1);
    delete [] wideFamilyName;
    lf->lfFaceName[LF_FACESIZE-1] = L'\0';
#else
    ::strncpy(lf->lfFaceName, familyName, LF_FACESIZE - 1);
    lf->lfFaceName[LF_FACESIZE - 1] = '\0';
#endif
}

void LogFontTypeface::onGetFamilyName(SkString* familyName) const {
    // Get the actual name of the typeface. The logfont may not know this.
    SkAutoHDC hdc(fLogFont);
    dcfontname_to_skstring(hdc, fLogFont, familyName);
}

void LogFontTypeface::onGetFontDescriptor(SkFontDescriptor* desc,
                                          bool* isLocalStream) const {
    SkString familyName;
    this->onGetFamilyName(&familyName);
    desc->setFamilyName(familyName.c_str());
    desc->setStyle(this->fontStyle());
    *isLocalStream = this->fSerializeAsStream;
}

void LogFontTypeface::getGlyphToUnicodeMap(SkUnichar* dstArray) const {
    SkAutoHDC hdc(fLogFont);
    unsigned int glyphCount = calculateGlyphCount(hdc, fLogFont);
    populate_glyph_to_unicode(hdc, glyphCount, dstArray);
}

std::unique_ptr<SkAdvancedTypefaceMetrics> LogFontTypeface::onGetAdvancedMetrics() const {
    LOGFONT lf = fLogFont;
    std::unique_ptr<SkAdvancedTypefaceMetrics> info(nullptr);

    // The design HFONT must be destroyed after the HDC
    using HFONT_T = typename std::remove_pointer<HFONT>::type;
    std::unique_ptr<HFONT_T, SkFunctionWrapper<decltype(DeleteObject), DeleteObject>> designFont;
    SkAutoHDC hdc(lf);

    const char stem_chars[] = {'i', 'I', '!', '1'};
    int16_t min_width;
    unsigned glyphCount;

    // To request design units, create a logical font whose height is specified
    // as unitsPerEm.
    OUTLINETEXTMETRIC otm;
    unsigned int otmRet = GetOutlineTextMetrics(hdc, sizeof(otm), &otm);
    if (0 == otmRet) {
        call_ensure_accessible(lf);
        otmRet = GetOutlineTextMetrics(hdc, sizeof(otm), &otm);
    }
    if (!otmRet || !GetTextFace(hdc, LF_FACESIZE, lf.lfFaceName)) {
        return info;
    }
    lf.lfHeight = -SkToS32(otm.otmEMSquare);
    designFont.reset(CreateFontIndirect(&lf));
    SelectObject(hdc, designFont.get());
    if (!GetOutlineTextMetrics(hdc, sizeof(otm), &otm)) {
        return info;
    }
    glyphCount = calculateGlyphCount(hdc, fLogFont);

    info.reset(new SkAdvancedTypefaceMetrics);
    tchar_to_skstring(lf.lfFaceName, &info->fFontName);

    SkOTTableOS2_V4::Type fsType;
    if (sizeof(fsType) == this->getTableData(SkTEndian_SwapBE32(SkOTTableOS2::TAG),
                                             offsetof(SkOTTableOS2_V4, fsType),
                                             sizeof(fsType),
                                             &fsType)) {
        SkOTUtils::SetAdvancedTypefaceFlags(fsType, info.get());
    } else {
        // If bit 1 is set, the font may not be embedded in a document.
        // If bit 1 is clear, the font can be embedded.
        // If bit 2 is set, the embedding is read-only.
        if (otm.otmfsType & 0x1) {
            info->fFlags |= SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag;
        }
    }

    if (glyphCount == 0 || (otm.otmTextMetrics.tmPitchAndFamily & TMPF_TRUETYPE) == 0) {
        return info;
    }
    info->fType = SkAdvancedTypefaceMetrics::kTrueType_Font;

    // If this bit is clear the font is a fixed pitch font.
    if (!(otm.otmTextMetrics.tmPitchAndFamily & TMPF_FIXED_PITCH)) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kFixedPitch_Style;
    }
    if (otm.otmTextMetrics.tmItalic) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kItalic_Style;
    }
    if (otm.otmTextMetrics.tmPitchAndFamily & FF_ROMAN) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kSerif_Style;
    } else if (otm.otmTextMetrics.tmPitchAndFamily & FF_SCRIPT) {
            info->fStyle |= SkAdvancedTypefaceMetrics::kScript_Style;
    }

    // The main italic angle of the font, in tenths of a degree counterclockwise
    // from vertical.
    info->fItalicAngle = otm.otmItalicAngle / 10;
    info->fAscent = SkToS16(otm.otmTextMetrics.tmAscent);
    info->fDescent = SkToS16(-otm.otmTextMetrics.tmDescent);
    // TODO(ctguil): Use alternate cap height calculation.
    // MSDN says otmsCapEmHeight is not support but it is returning a value on
    // my Win7 box.
    info->fCapHeight = otm.otmsCapEmHeight;
    info->fBBox =
        SkIRect::MakeLTRB(otm.otmrcFontBox.left, otm.otmrcFontBox.top,
                          otm.otmrcFontBox.right, otm.otmrcFontBox.bottom);

    // Figure out a good guess for StemV - Min width of i, I, !, 1.
    // This probably isn't very good with an italic font.
    min_width = SHRT_MAX;
    info->fStemV = 0;
    for (size_t i = 0; i < SK_ARRAY_COUNT(stem_chars); i++) {
        ABC abcWidths;
        if (GetCharABCWidths(hdc, stem_chars[i], stem_chars[i], &abcWidths)) {
            int16_t width = abcWidths.abcB;
            if (width > 0 && width < min_width) {
                min_width = width;
                info->fStemV = min_width;
            }
        }
    }

    return info;
}

//Dummy representation of a Base64 encoded GUID from create_unique_font_name.
#define BASE64_GUID_ID "XXXXXXXXXXXXXXXXXXXXXXXX"
//Length of GUID representation from create_id, including nullptr terminator.
#define BASE64_GUID_ID_LEN SK_ARRAY_COUNT(BASE64_GUID_ID)

static_assert(BASE64_GUID_ID_LEN < LF_FACESIZE, "GUID_longer_than_facesize");

/**
   NameID 6 Postscript names cannot have the character '/'.
   It would be easier to hex encode the GUID, but that is 32 bytes,
   and many systems have issues with names longer than 28 bytes.
   The following need not be any standard base64 encoding.
   The encoded value is never decoded.
*/
static const char postscript_safe_base64_encode[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789-_=";

/**
   Formats a GUID into Base64 and places it into buffer.
   buffer should have space for at least BASE64_GUID_ID_LEN characters.
   The string will always be null terminated.
   XXXXXXXXXXXXXXXXXXXXXXXX0
 */
static void format_guid_b64(const GUID& guid, char* buffer, size_t bufferSize) {
    SkASSERT(bufferSize >= BASE64_GUID_ID_LEN);
    size_t written = SkBase64::Encode(&guid, sizeof(guid), buffer, postscript_safe_base64_encode);
    SkASSERT(written < LF_FACESIZE);
    buffer[written] = '\0';
}

/**
   Creates a Base64 encoded GUID and places it into buffer.
   buffer should have space for at least BASE64_GUID_ID_LEN characters.
   The string will always be null terminated.
   XXXXXXXXXXXXXXXXXXXXXXXX0
 */
static HRESULT create_unique_font_name(char* buffer, size_t bufferSize) {
    GUID guid = {};
    if (FAILED(CoCreateGuid(&guid))) {
        return E_UNEXPECTED;
    }
    format_guid_b64(guid, buffer, bufferSize);

    return S_OK;
}

/**
   Introduces a font to GDI. On failure will return nullptr. The returned handle
   should eventually be passed to RemoveFontMemResourceEx.
*/
static HANDLE activate_font(SkData* fontData) {
    DWORD numFonts = 0;
    //AddFontMemResourceEx just copies the data, but does not specify const.
    HANDLE fontHandle = AddFontMemResourceEx(const_cast<void*>(fontData->data()),
                                             static_cast<DWORD>(fontData->size()),
                                             0,
                                             &numFonts);

    if (fontHandle != nullptr && numFonts < 1) {
        RemoveFontMemResourceEx(fontHandle);
        return nullptr;
    }

    return fontHandle;
}

// Does not affect ownership of stream.
static sk_sp<SkTypeface> create_from_stream(std::unique_ptr<SkStreamAsset> stream) {
    // Create a unique and unpredictable font name.
    // Avoids collisions and access from CSS.
    char familyName[BASE64_GUID_ID_LEN];
    const int familyNameSize = SK_ARRAY_COUNT(familyName);
    if (FAILED(create_unique_font_name(familyName, familyNameSize))) {
        return nullptr;
    }

    // Change the name of the font.
    sk_sp<SkData> rewrittenFontData(SkOTUtils::RenameFont(stream.get(), familyName, familyNameSize-1));
    if (nullptr == rewrittenFontData.get()) {
        return nullptr;
    }

    // Register the font with GDI.
    HANDLE fontReference = activate_font(rewrittenFontData.get());
    if (nullptr == fontReference) {
        return nullptr;
    }

    // Create the typeface.
    LOGFONT lf;
    logfont_for_name(familyName, &lf);

    return sk_sp<SkTypeface>(SkCreateFontMemResourceTypefaceFromLOGFONT(lf, fontReference));
}

std::unique_ptr<SkStreamAsset> LogFontTypeface::onOpenStream(int* ttcIndex) const {
    *ttcIndex = 0;

    const DWORD kTTCTag = SkEndian_SwapBE32(SkSetFourByteTag('t', 't', 'c', 'f'));
    LOGFONT lf = fLogFont;

    SkAutoHDC hdc(lf);

    std::unique_ptr<SkStreamAsset> stream;
    DWORD tables[2] = {kTTCTag, 0};
    for (size_t i = 0; i < SK_ARRAY_COUNT(tables); i++) {
        DWORD bufferSize = GetFontData(hdc, tables[i], 0, nullptr, 0);
        if (bufferSize == GDI_ERROR) {
            call_ensure_accessible(lf);
            bufferSize = GetFontData(hdc, tables[i], 0, nullptr, 0);
        }
        if (bufferSize != GDI_ERROR) {
            stream.reset(new SkMemoryStream(bufferSize));
            if (GetFontData(hdc, tables[i], 0, (void*)stream->getMemoryBase(), bufferSize)) {
                break;
            } else {
                stream.reset();
            }
        }
    }
    return stream;
}

sk_sp<SkTypeface> LogFontTypeface::onMakeClone(const SkFontArguments& args) const {
    return sk_ref_sp(this);
}

static void bmpCharsToGlyphs(HDC hdc, const WCHAR* bmpChars, int count, uint16_t* glyphs,
                             bool Ox1FHack)
{
    // Type1 fonts fail with uniscribe API. Use GetGlyphIndices for plane 0.

    /** Real documentation for GetGlyphIndicesW:
     *
     *  When GGI_MARK_NONEXISTING_GLYPHS is not specified and a character does not map to a
     *  glyph, then the 'default character's glyph is returned instead. The 'default character'
     *  is available in fTM.tmDefaultChar. FON fonts have a default character, and there exists
     *  a usDefaultChar in the 'OS/2' table, version 2 and later. If there is no
     *  'default character' specified by the font, then often the first character found is used.
     *
     *  When GGI_MARK_NONEXISTING_GLYPHS is specified and a character does not map to a glyph,
     *  then the glyph 0xFFFF is used. In Windows XP and earlier, Bitmap/Vector FON usually use
     *  glyph 0x1F instead ('Terminal' appears to be special, returning 0xFFFF).
     *  Type1 PFM/PFB, TT, OT TT, OT CFF all appear to use 0xFFFF, even on XP.
     */
    DWORD result = GetGlyphIndicesW(hdc, bmpChars, count, glyphs, GGI_MARK_NONEXISTING_GLYPHS);
    if (GDI_ERROR == result) {
        for (int i = 0; i < count; ++i) {
            glyphs[i] = 0;
        }
        return;
    }

    if (Ox1FHack) {
        for (int i = 0; i < count; ++i) {
            if (0xFFFF == glyphs[i] || 0x1F == glyphs[i]) {
                glyphs[i] = 0;
            }
        }
    } else {
        for (int i = 0; i < count; ++i) {
            if (0xFFFF == glyphs[i]){
                glyphs[i] = 0;
            }
        }
    }
}

static uint16_t nonBmpCharToGlyph(HDC hdc, SCRIPT_CACHE* scriptCache, const WCHAR utf16[2]) {
    uint16_t index = 0;
    // Use uniscribe to detemine glyph index for non-BMP characters.
    static const int numWCHAR = 2;
    static const int maxItems = 2;
    // MSDN states that this can be nullptr, but some things don't work then.
    SCRIPT_CONTROL scriptControl;
    memset(&scriptControl, 0, sizeof(scriptControl));
    // Add extra item to SCRIPT_ITEM to work around a bug (now documented).
    // https://bugzilla.mozilla.org/show_bug.cgi?id=366643
    SCRIPT_ITEM si[maxItems + 1];
    int numItems;
    HRZM(ScriptItemize(utf16, numWCHAR, maxItems, &scriptControl, nullptr, si, &numItems),
         "Could not itemize character.");

    // Sometimes ScriptShape cannot find a glyph for a non-BMP and returns 2 space glyphs.
    static const int maxGlyphs = 2;
    SCRIPT_VISATTR vsa[maxGlyphs];
    WORD outGlyphs[maxGlyphs];
    WORD logClust[numWCHAR];
    int numGlyphs;
    SCRIPT_ANALYSIS& script = si[0].a;
    script.eScript = SCRIPT_UNDEFINED;
    script.fRTL = FALSE;
    script.fLayoutRTL = FALSE;
    script.fLinkBefore = FALSE;
    script.fLinkAfter = FALSE;
    script.fLogicalOrder = FALSE;
    script.fNoGlyphIndex = FALSE;
    script.s.uBidiLevel = 0;
    script.s.fOverrideDirection = 0;
    script.s.fInhibitSymSwap = TRUE;
    script.s.fCharShape = FALSE;
    script.s.fDigitSubstitute = FALSE;
    script.s.fInhibitLigate = FALSE;
    script.s.fDisplayZWG = TRUE;
    script.s.fArabicNumContext = FALSE;
    script.s.fGcpClusters = FALSE;
    script.s.fReserved = 0;
    script.s.fEngineReserved = 0;
    // For the future, 0x80040200 from here is USP_E_SCRIPT_NOT_IN_FONT
    HRZM(ScriptShape(hdc, scriptCache, utf16, numWCHAR, maxGlyphs, &script,
                     outGlyphs, logClust, vsa, &numGlyphs),
         "Could not shape character.");
    if (1 == numGlyphs) {
        index = outGlyphs[0];
    }
    return index;
}

void LogFontTypeface::onCharsToGlyphs(const SkUnichar* uni, int glyphCount,
                                      SkGlyphID glyphs[]) const
{
    SkAutoHDC hdc(fLogFont);

    TEXTMETRIC tm;
    if (0 == GetTextMetrics(hdc, &tm)) {
        call_ensure_accessible(fLogFont);
        if (0 == GetTextMetrics(hdc, &tm)) {
            tm.tmPitchAndFamily = TMPF_TRUETYPE;
        }
    }
    bool Ox1FHack = !(tm.tmPitchAndFamily & TMPF_VECTOR) /*&& winVer < Vista */;

    SCRIPT_CACHE sc = 0;
    static const int scratchCount = 256;
    WCHAR scratch[scratchCount];
    int glyphIndex = 0;
    const uint32_t* utf32 = reinterpret_cast<const uint32_t*>(uni);
    while (glyphIndex < glyphCount) {
        // Try a run of bmp.
        int glyphsLeft = SkTMin(glyphCount - glyphIndex, scratchCount);
        int runLength = 0;
        while (runLength < glyphsLeft && utf32[glyphIndex + runLength] <= 0xFFFF) {
            scratch[runLength] = static_cast<WCHAR>(utf32[glyphIndex + runLength]);
            ++runLength;
        }
        if (runLength) {
            bmpCharsToGlyphs(hdc, scratch, runLength, &glyphs[glyphIndex], Ox1FHack);
            glyphIndex += runLength;
        }

        // Try a run of non-bmp.
        while (glyphIndex < glyphCount && utf32[glyphIndex] > 0xFFFF) {
            SkUTF::ToUTF16(utf32[glyphIndex], reinterpret_cast<uint16_t*>(scratch));
            glyphs[glyphIndex] = nonBmpCharToGlyph(hdc, &sc, scratch);
            ++glyphIndex;
        }
    }

    if (sc) {
        ::ScriptFreeCache(&sc);
    }
}

int LogFontTypeface::onCountGlyphs() const {
    SkAutoHDC hdc(fLogFont);
    return calculateGlyphCount(hdc, fLogFont);
}

void LogFontTypeface::getPostScriptGlyphNames(SkString*) const {}

int LogFontTypeface::onGetUPEM() const {
    SkAutoHDC hdc(fLogFont);
    return calculateUPEM(hdc, fLogFont);
}

SkTypeface::LocalizedStrings* LogFontTypeface::onCreateFamilyNameIterator() const {
    sk_sp<SkTypeface::LocalizedStrings> nameIter =
        SkOTUtils::LocalizedStrings_NameTable::MakeForFamilyNames(*this);
    if (!nameIter) {
        SkString familyName;
        this->getFamilyName(&familyName);
        SkString language("und"); //undetermined
        nameIter = sk_make_sp<SkOTUtils::LocalizedStrings_SingleName>(familyName, language);
    }
    return nameIter.release();
}

int LogFontTypeface::onGetTableTags(SkFontTableTag tags[]) const {
    SkSFNTHeader header;
    if (sizeof(header) != this->onGetTableData(0, 0, sizeof(header), &header)) {
        return 0;
    }

    int numTables = SkEndian_SwapBE16(header.numTables);

    if (tags) {
        size_t size = numTables * sizeof(SkSFNTHeader::TableDirectoryEntry);
        SkAutoSTMalloc<0x20, SkSFNTHeader::TableDirectoryEntry> dir(numTables);
        if (size != this->onGetTableData(0, sizeof(header), size, dir.get())) {
            return 0;
        }

        for (int i = 0; i < numTables; ++i) {
            tags[i] = SkEndian_SwapBE32(dir[i].tag);
        }
    }
    return numTables;
}

size_t LogFontTypeface::onGetTableData(SkFontTableTag tag, size_t offset,
                                       size_t length, void* data) const
{
    LOGFONT lf = fLogFont;
    SkAutoHDC hdc(lf);

    tag = SkEndian_SwapBE32(tag);
    if (nullptr == data) {
        length = 0;
    }
    DWORD bufferSize = GetFontData(hdc, tag, (DWORD) offset, data, (DWORD) length);
    if (bufferSize == GDI_ERROR) {
        call_ensure_accessible(lf);
        bufferSize = GetFontData(hdc, tag, (DWORD) offset, data, (DWORD) length);
    }
    return bufferSize == GDI_ERROR ? 0 : bufferSize;
}

sk_sp<SkData> LogFontTypeface::onCopyTableData(SkFontTableTag tag) const {
    LOGFONT lf = fLogFont;
    SkAutoHDC hdc(lf);

    tag = SkEndian_SwapBE32(tag);
    DWORD size = GetFontData(hdc, tag, 0, nullptr, 0);
    if (size == GDI_ERROR) {
        call_ensure_accessible(lf);
        size = GetFontData(hdc, tag, 0, nullptr, 0);
    }

    sk_sp<SkData> data;
    if (size != GDI_ERROR) {
        data = SkData::MakeUninitialized(size);
        if (GetFontData(hdc, tag, 0, data->writable_data(), size) == GDI_ERROR) {
            data.reset();
        }
    }
    return data;
}

SkScalerContext* LogFontTypeface::onCreateScalerContext(const SkScalerContextEffects& effects,
                                                        const SkDescriptor* desc) const {
    auto ctx = std::make_unique<SkScalerContext_GDI>(
            sk_ref_sp(const_cast<LogFontTypeface*>(this)), effects, desc);
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx.release();
}

void LogFontTypeface::onFilterRec(SkScalerContextRec* rec) const {
    if (rec->fFlags & SkScalerContext::kLCD_BGROrder_Flag ||
        rec->fFlags & SkScalerContext::kLCD_Vertical_Flag)
    {
        rec->fMaskFormat = SkMask::kA8_Format;
        rec->fFlags |= SkScalerContext::kGenA8FromLCD_Flag;
    }

    unsigned flagsWeDontSupport = SkScalerContext::kForceAutohinting_Flag |
                                  SkScalerContext::kEmbeddedBitmapText_Flag |
                                  SkScalerContext::kEmbolden_Flag |
                                  SkScalerContext::kLCD_BGROrder_Flag |
                                  SkScalerContext::kLCD_Vertical_Flag;
    rec->fFlags &= ~flagsWeDontSupport;

    SkFontHinting h = rec->getHinting();
    switch (h) {
        case SkFontHinting::kNone:
            break;
        case SkFontHinting::kSlight:
            // Only do slight hinting when axis aligned.
            // TODO: re-enable slight hinting when FontHostTest can pass.
            //if (!isAxisAligned(*rec)) {
                h = SkFontHinting::kNone;
            //}
            break;
        case SkFontHinting::kNormal:
        case SkFontHinting::kFull:
            // TODO: need to be able to distinguish subpixel positioned glyphs
            // and linear metrics.
            //rec->fFlags &= ~SkScalerContext::kSubpixelPositioning_Flag;
            h = SkFontHinting::kNormal;
            break;
        default:
            SkDEBUGFAIL("unknown hinting");
    }
    //TODO: if this is a bitmap font, squash hinting and subpixel.
    rec->setHinting(h);

// turn this off since GDI might turn A8 into BW! Need a bigger fix.
#if 0
    // Disable LCD when rotated, since GDI's output is ugly
    if (isLCD(*rec) && !isAxisAligned(*rec)) {
        rec->fMaskFormat = SkMask::kA8_Format;
    }
#endif

    if (!fCanBeLCD && isLCD(*rec)) {
        rec->fMaskFormat = SkMask::kA8_Format;
        rec->fFlags &= ~SkScalerContext::kGenA8FromLCD_Flag;
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "include/core/SkDataTable.h"
#include "include/core/SkFontMgr.h"

static bool valid_logfont_for_enum(const LOGFONT& lf) {
    // TODO: Vector FON is unsupported and should not be listed.
    return
        // Ignore implicit vertical variants.
        lf.lfFaceName[0] && lf.lfFaceName[0] != '@'

        // DEFAULT_CHARSET is used to get all fonts, but also implies all
        // character sets. Filter assuming all fonts support ANSI_CHARSET.
        && ANSI_CHARSET == lf.lfCharSet
    ;
}

/** An EnumFontFamExProc implementation which interprets builderParam as
 *  an SkTDArray<ENUMLOGFONTEX>* and appends logfonts which
 *  pass the valid_logfont_for_enum predicate.
 */
static int CALLBACK enum_family_proc(const LOGFONT* lf, const TEXTMETRIC*,
                                     DWORD fontType, LPARAM builderParam) {
    if (valid_logfont_for_enum(*lf)) {
        SkTDArray<ENUMLOGFONTEX>* array = (SkTDArray<ENUMLOGFONTEX>*)builderParam;
        *array->append() = *(ENUMLOGFONTEX*)lf;
    }
    return 1; // non-zero means continue
}

class SkFontStyleSetGDI : public SkFontStyleSet {
public:
    SkFontStyleSetGDI(const TCHAR familyName[]) {
        LOGFONT lf;
        sk_bzero(&lf, sizeof(lf));
        lf.lfCharSet = DEFAULT_CHARSET;
        _tcscpy_s(lf.lfFaceName, familyName);

        HDC hdc = ::CreateCompatibleDC(nullptr);
        ::EnumFontFamiliesEx(hdc, &lf, enum_family_proc, (LPARAM)&fArray, 0);
        ::DeleteDC(hdc);
    }

    int count() override {
        return fArray.count();
    }

    void getStyle(int index, SkFontStyle* fs, SkString* styleName) override {
        if (fs) {
            *fs = get_style(fArray[index].elfLogFont);
        }
        if (styleName) {
            const ENUMLOGFONTEX& ref = fArray[index];
            // For some reason, ENUMLOGFONTEX and LOGFONT disagree on their type in the
            // non-unicode version.
            //      ENUMLOGFONTEX uses BYTE
            //      LOGFONT uses CHAR
            // Here we assert they that the style name is logically the same (size) as
            // a TCHAR, so we can use the same converter function.
            SkASSERT(sizeof(TCHAR) == sizeof(ref.elfStyle[0]));
            tchar_to_skstring((const TCHAR*)ref.elfStyle, styleName);
        }
    }

    SkTypeface* createTypeface(int index) override {
        return SkCreateTypefaceFromLOGFONT(fArray[index].elfLogFont);
    }

    SkTypeface* matchStyle(const SkFontStyle& pattern) override {
        return this->matchStyleCSS3(pattern);
    }

private:
    SkTDArray<ENUMLOGFONTEX> fArray;
};

class SkFontMgrGDI : public SkFontMgr {
public:
    SkFontMgrGDI() {
        LOGFONT lf;
        sk_bzero(&lf, sizeof(lf));
        lf.lfCharSet = DEFAULT_CHARSET;

        HDC hdc = ::CreateCompatibleDC(nullptr);
        ::EnumFontFamiliesEx(hdc, &lf, enum_family_proc, (LPARAM)&fLogFontArray, 0);
        ::DeleteDC(hdc);
    }

protected:
    int onCountFamilies() const override {
        return fLogFontArray.count();
    }

    void onGetFamilyName(int index, SkString* familyName) const override {
        SkASSERT((unsigned)index < (unsigned)fLogFontArray.count());
        tchar_to_skstring(fLogFontArray[index].elfLogFont.lfFaceName, familyName);
    }

    SkFontStyleSet* onCreateStyleSet(int index) const override {
        SkASSERT((unsigned)index < (unsigned)fLogFontArray.count());
        return new SkFontStyleSetGDI(fLogFontArray[index].elfLogFont.lfFaceName);
    }

    SkFontStyleSet* onMatchFamily(const char familyName[]) const override {
        if (nullptr == familyName) {
            familyName = "";    // do we need this check???
        }
        LOGFONT lf;
        logfont_for_name(familyName, &lf);
        return new SkFontStyleSetGDI(lf.lfFaceName);
    }

    virtual SkTypeface* onMatchFamilyStyle(const char familyName[],
                                           const SkFontStyle& fontstyle) const override {
        // could be in base impl
        sk_sp<SkFontStyleSet> sset(this->matchFamily(familyName));
        return sset->matchStyle(fontstyle);
    }

    virtual SkTypeface* onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                                    const char* bcp47[], int bcp47Count,
                                                    SkUnichar character) const override {
        return nullptr;
    }

    virtual SkTypeface* onMatchFaceStyle(const SkTypeface* familyMember,
                                         const SkFontStyle& fontstyle) const override {
        // could be in base impl
        SkString familyName;
        ((LogFontTypeface*)familyMember)->getFamilyName(&familyName);
        return this->matchFamilyStyle(familyName.c_str(), fontstyle);
    }

    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        if (ttcIndex != 0) {
            return nullptr;
        }
        return create_from_stream(std::move(stream));
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        // could be in base impl
        return this->makeFromStream(std::unique_ptr<SkStreamAsset>(new SkMemoryStream(std::move(data))),
                                    ttcIndex);
    }

    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        // could be in base impl
        auto stream = SkStream::MakeFromFile(path);
        return stream ? this->makeFromStream(std::move(stream), ttcIndex) : nullptr;
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const override {
        LOGFONT lf;
        if (nullptr == familyName) {
            lf = get_default_font();
        } else {
            logfont_for_name(familyName, &lf);
        }

        lf.lfWeight = style.weight();
        lf.lfItalic = style.slant() == SkFontStyle::kUpright_Slant ? FALSE : TRUE;
        return sk_sp<SkTypeface>(SkCreateTypefaceFromLOGFONT(lf));
    }

private:
    SkTDArray<ENUMLOGFONTEX> fLogFontArray;
};

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFontMgr> SkFontMgr_New_GDI() { return sk_make_sp<SkFontMgrGDI>(); }

#endif//defined(SK_BUILD_FOR_WIN)
