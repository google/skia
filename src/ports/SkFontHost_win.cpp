/*
 ** Copyright 2006, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#include "SkString.h"
//#include "SkStream.h"

#include "SkEndian.h"
#include "SkFontHost.h"
#include "SkDescriptor.h"
#include "SkAdvancedTypefaceMetrics.h"
#include "SkStream.h"
#include "SkThread.h"
#include "SkTypeface_win.h"
#include "SkTypefaceCache.h"
#include "SkUtils.h"

#ifdef WIN32
#include "windows.h"
#include "tchar.h"
#include "Usp10.h"

// client3d has to undefine this for now
#define CAN_USE_LOGFONT_NAME

using namespace skia_advanced_typeface_metrics_utils;

static const uint16_t BUFFERSIZE = (16384 - 32);
static uint8_t glyphbuf[BUFFERSIZE];

// Give 1MB font cache budget
#define FONT_CACHE_MEMORY_BUDGET    (1024 * 1024)

/**
 *	Since LOGFONT wants its textsize as an int, and we support fractional sizes,
 *  and since we have a cache of LOGFONTs for our tyepfaces, we always set the
 *  lfHeight to a canonical size, and then we use the 2x2 matrix to achieve the
 *  actual requested size.
 */
static const int gCanonicalTextSize = 64;

static void make_canonical(LOGFONT* lf) {
	lf->lfHeight = -gCanonicalTextSize;
    lf->lfQuality = CLEARTYPE_QUALITY;//PROOF_QUALITY;
    lf->lfCharSet = DEFAULT_CHARSET;
}

static SkTypeface::Style getStyle(const LOGFONT& lf) {
    unsigned style = 0;
    if (lf.lfWeight >= FW_BOLD) {
        style |= SkTypeface::kBold;
    }
    if (lf.lfItalic) {
        style |= SkTypeface::kItalic;
    }
    return (SkTypeface::Style)style;
}

static void setStyle(LOGFONT* lf, SkTypeface::Style style) {
    lf->lfWeight = (style & SkTypeface::kBold) != 0 ? FW_BOLD : FW_NORMAL ;
    lf->lfItalic = ((style & SkTypeface::kItalic) != 0);
}

static inline FIXED SkFixedToFIXED(SkFixed x) {
    return *(FIXED*)(&x);
}

static inline FIXED SkScalarToFIXED(SkScalar x) {
    return SkFixedToFIXED(SkScalarToFixed(x));
}

static unsigned calculateGlyphCount(HDC hdc) {
    // The 'maxp' table stores the number of glyphs at offset 4, in 2 bytes.
    const DWORD maxpTag =
        SkEndian_SwapBE32(SkSetFourByteTag('m', 'a', 'x', 'p'));
    uint16_t glyphs;
    if (GetFontData(hdc, maxpTag, 4, &glyphs, sizeof(glyphs)) != GDI_ERROR) {
        return SkEndian_SwapBE16(glyphs);
    }
    
    // Binary search for glyph count.
    static const MAT2 mat2 = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
    int32_t max = SK_MaxU16 + 1;
    int32_t min = 0;
    GLYPHMETRICS gm;
    while (min < max) {
        int32_t mid = min + ((max - min) / 2);
        if (GetGlyphOutlineW(hdc, mid, GGO_METRICS | GGO_GLYPH_INDEX, &gm, 0,
                             NULL, &mat2) == GDI_ERROR) {
            max = mid;
        } else {
            min = mid + 1;
        }
    }
    SkASSERT(min == max);
    return min;
}

static SkTypeface::Style GetFontStyle(const LOGFONT& lf) {
    int style = SkTypeface::kNormal;
    if (lf.lfWeight == FW_SEMIBOLD || lf.lfWeight == FW_DEMIBOLD || lf.lfWeight == FW_BOLD)
        style |= SkTypeface::kBold;
    if (lf.lfItalic)
        style |= SkTypeface::kItalic;

    return (SkTypeface::Style)style;
}

class LogFontTypeface : public SkTypeface {
public:
    LogFontTypeface(SkTypeface::Style style, SkFontID fontID, const LOGFONT& lf) :
      SkTypeface(style, fontID, false), fLogFont(lf) {}

    LOGFONT fLogFont;

    static LogFontTypeface* Create(const LOGFONT& lf) {
        SkTypeface::Style style = GetFontStyle(lf);
        SkFontID fontID = SkTypefaceCache::NewFontID();
        return new LogFontTypeface(style, fontID, lf);
    }
};

static const LOGFONT& get_default_font() {
    static LOGFONT gDefaultFont;
    // don't hardcode on Windows, Win2000, XP, Vista, and international all have different default
    // and the user could change too


//  lfMessageFont is garbage on my XP, so skip for now
#if 0
    if (gDefaultFont.lfFaceName[0] != 0) {
        return gDefaultFont;
    }

    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

    //memcpy(&gDefaultFont, &(ncm.lfMessageFont), sizeof(LOGFONT));
#endif

    return gDefaultFont;
}

static bool FindByLogFont(SkTypeface* face, SkTypeface::Style requestedStyle, void* ctx) {
    LogFontTypeface* lface = reinterpret_cast<LogFontTypeface*>(face);
    const LOGFONT* lf = reinterpret_cast<const LOGFONT*>(ctx);

    return getStyle(lface->fLogFont) == requestedStyle &&
           !memcmp(&lface->fLogFont, lf, sizeof(LOGFONT));
}

/**
 *  This guy is public. It first searches the cache, and if a match is not found,
 *  it creates a new face.
 */
SkTypeface* SkCreateTypefaceFromLOGFONT(const LOGFONT& origLF) {
    LOGFONT lf = origLF;
    make_canonical(&lf);
    SkTypeface* face = SkTypefaceCache::FindByProc(FindByLogFont, &lf);
    if (face) {
        face->ref();
    } else {
        face = LogFontTypeface::Create(lf);
        SkTypefaceCache::Add(face, getStyle(lf));
    }
    return face;
}

SkFontID SkFontHost::NextLogicalFont(SkFontID currFontID, SkFontID origFontID) {
  // Zero means that we don't have any fallback fonts for this fontID.
  // This function is implemented on Android, but doesn't have much
  // meaning here.
  return 0;
}

static void GetLogFontByID(SkFontID fontID, LOGFONT* lf) {
    LogFontTypeface* face = (LogFontTypeface*)SkTypefaceCache::FindByID(fontID);
    if (face) {
        *lf = face->fLogFont;
    } else {
        sk_bzero(lf, sizeof(LOGFONT));
    }
}

// Construct Glyph to Unicode table.
// Unicode code points that require conjugate pairs in utf16 are not
// supported.
// TODO(arthurhsu): Add support for conjugate pairs. It looks like that may
// require parsing the TTF cmap table (platform 4, encoding 12) directly instead
// of calling GetFontUnicodeRange().
static void populate_glyph_to_unicode(HDC fontHdc, const unsigned glyphCount,
                                      SkTDArray<SkUnichar>* glyphToUnicode) {
    DWORD glyphSetBufferSize = GetFontUnicodeRanges(fontHdc, NULL);
    if (!glyphSetBufferSize) {
        return;
    }

    SkAutoTDeleteArray<BYTE> glyphSetBuffer(new BYTE[glyphSetBufferSize]);
    GLYPHSET* glyphSet =
        reinterpret_cast<LPGLYPHSET>(glyphSetBuffer.get());
    if (GetFontUnicodeRanges(fontHdc, glyphSet) != glyphSetBufferSize) {
        return;
    }

    glyphToUnicode->setCount(glyphCount);
    memset(glyphToUnicode->begin(), 0, glyphCount * sizeof(SkUnichar));
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
            if (glyph[j] != 0xffff && glyph[j] < glyphCount &&
                (*glyphToUnicode)[glyph[j]] == 0) {
                (*glyphToUnicode)[glyph[j]] = chars[j];
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////

class SkScalerContext_Windows : public SkScalerContext {
public:
    SkScalerContext_Windows(const SkDescriptor* desc);
    virtual ~SkScalerContext_Windows();

protected:
    virtual unsigned generateGlyphCount();
    virtual uint16_t generateCharToGlyph(SkUnichar uni);
    virtual void generateAdvance(SkGlyph* glyph);
    virtual void generateMetrics(SkGlyph* glyph);
    virtual void generateImage(const SkGlyph& glyph);
    virtual void generatePath(const SkGlyph& glyph, SkPath* path);
    virtual void generateFontMetrics(SkPaint::FontMetrics* mX, SkPaint::FontMetrics* mY);
    //virtual SkDeviceContext getDC() {return ddc;}
private:
	SkScalar	 fScale;	// to get from canonical size to real size
    MAT2         fMat22;
    XFORM        fXform;
    HDC          fDDC;
    HFONT        fSavefont;
    HFONT        fFont;
    SCRIPT_CACHE fSC;
    int          fGlyphCount;

    HFONT        fHiResFont;
    MAT2         fMat22Identity;
    SkMatrix     fHiResMatrix;
};

static float mul2float(SkScalar a, SkScalar b) {
    return SkScalarToFloat(SkScalarMul(a, b));
}

static FIXED float2FIXED(float x) {
    return SkFixedToFIXED(SkFloatToFixed(x));
}

static SkMutex gFTMutex;

#define HIRES_TEXTSIZE  2048
#define HIRES_SHIFT     11
static inline SkFixed HiResToFixed(int value) {
    return value << (16 - HIRES_SHIFT);
}

static bool needHiResMetrics(const SkScalar mat[2][2]) {
    return mat[1][0] || mat[0][1];
}

SkScalerContext_Windows::SkScalerContext_Windows(const SkDescriptor* desc)
        : SkScalerContext(desc), fDDC(0), fFont(0), fSavefont(0), fSC(0)
        , fGlyphCount(-1) {
    SkAutoMutexAcquire  ac(gFTMutex);

	fScale = fRec.fTextSize / gCanonicalTextSize;

    fXform.eM11 = mul2float(fScale, fRec.fPost2x2[0][0]);
    fXform.eM12 = mul2float(fScale, fRec.fPost2x2[1][0]);
    fXform.eM21 = mul2float(fScale, fRec.fPost2x2[0][1]);
    fXform.eM22 = mul2float(fScale, fRec.fPost2x2[1][1]);
    fXform.eDx = 0;
    fXform.eDy = 0;

    fMat22.eM11 = float2FIXED(fXform.eM11);
    fMat22.eM12 = float2FIXED(fXform.eM12);
    fMat22.eM21 = float2FIXED(-fXform.eM21);
    fMat22.eM22 = float2FIXED(-fXform.eM22);

    fDDC = ::CreateCompatibleDC(NULL);
    SetGraphicsMode(fDDC, GM_ADVANCED);
    SetBkMode(fDDC, TRANSPARENT);

    // Scaling by the DPI is inconsistent with how Skia draws elsewhere
    //SkScalar height = -(fRec.fTextSize * GetDeviceCaps(ddc, LOGPIXELSY) / 72);
    LOGFONT lf;
    GetLogFontByID(fRec.fFontID, &lf);
    lf.lfHeight = -gCanonicalTextSize;
    fFont = CreateFontIndirect(&lf);

    // if we're rotated, or want fractional widths, create a hires font
    fHiResFont = 0;
    if (needHiResMetrics(fRec.fPost2x2) || (fRec.fFlags & kSubpixelPositioning_Flag)) {
        lf.lfHeight = -HIRES_TEXTSIZE;
        fHiResFont = CreateFontIndirect(&lf);

        fMat22Identity.eM11 = fMat22Identity.eM22 = SkFixedToFIXED(SK_Fixed1);
        fMat22Identity.eM12 = fMat22Identity.eM21 = SkFixedToFIXED(0);

        // construct a matrix to go from HIRES logical units to our device units
        fRec.getSingleMatrix(&fHiResMatrix);
        SkScalar scale = SkScalarInvert(SkIntToScalar(HIRES_TEXTSIZE));
        fHiResMatrix.preScale(scale, scale);
    }
    fSavefont = (HFONT)SelectObject(fDDC, fFont);
}

SkScalerContext_Windows::~SkScalerContext_Windows() {
    if (fDDC) {
        ::SelectObject(fDDC, fSavefont);
        ::DeleteDC(fDDC);
    }
    if (fFont) {
        ::DeleteObject(fFont);
    }
    if (fHiResFont) {
        ::DeleteObject(fHiResFont);
    }
    if (fSC) {
        ::ScriptFreeCache(&fSC);
    }
}

unsigned SkScalerContext_Windows::generateGlyphCount() {
    if (fGlyphCount < 0) {
        fGlyphCount = calculateGlyphCount(fDDC);
    }
    return fGlyphCount;
}

uint16_t SkScalerContext_Windows::generateCharToGlyph(SkUnichar uni) {
    uint16_t index = 0;
    WCHAR c[2];
    // TODO(ctguil): Support characters that generate more than one glyph.
    if (SkUTF16_FromUnichar(uni, (uint16_t*)c) == 1) {
        // Type1 fonts fail with uniscribe API. Use GetGlyphIndices for plane 0.
        SkAssertResult(GetGlyphIndicesW(fDDC, c, 1, &index, 0));
    } else {
        // Use uniscribe to detemine glyph index for non-BMP characters.
        // Need to add extra item to SCRIPT_ITEM to work around a bug in older
        // windows versions. https://bugzilla.mozilla.org/show_bug.cgi?id=366643
        SCRIPT_ITEM si[2 + 1];
        int items;
        SkAssertResult(
            SUCCEEDED(ScriptItemize(c, 2, 2, NULL, NULL, si, &items)));

        WORD log[2];
        SCRIPT_VISATTR vsa;
        int glyphs;
        SkAssertResult(SUCCEEDED(ScriptShape(
            fDDC, &fSC, c, 2, 1, &si[0].a, &index, log, &vsa, &glyphs)));
    }
    return index;
}

void SkScalerContext_Windows::generateAdvance(SkGlyph* glyph) {
    this->generateMetrics(glyph);
}

void SkScalerContext_Windows::generateMetrics(SkGlyph* glyph) {

    SkASSERT(fDDC);

    GLYPHMETRICS gm;
    sk_bzero(&gm, sizeof(gm));

    glyph->fRsbDelta = 0;
    glyph->fLsbDelta = 0;

    // Note: need to use GGO_GRAY8_BITMAP instead of GGO_METRICS because GGO_METRICS returns a smaller
    // BlackBlox; we need the bigger one in case we need the image.  fAdvance is the same.
    uint32_t ret = GetGlyphOutlineW(fDDC, glyph->getGlyphID(0), GGO_GRAY8_BITMAP | GGO_GLYPH_INDEX, &gm, 0, NULL, &fMat22);

    if (GDI_ERROR != ret) {
        if (ret == 0) {
            // for white space, ret is zero and gmBlackBoxX, gmBlackBoxY are 1 incorrectly!
            gm.gmBlackBoxX = gm.gmBlackBoxY = 0;
        }
        glyph->fWidth   = gm.gmBlackBoxX;
        glyph->fHeight  = gm.gmBlackBoxY;
        glyph->fTop     = SkToS16(gm.gmptGlyphOrigin.y - gm.gmBlackBoxY);
        glyph->fLeft    = SkToS16(gm.gmptGlyphOrigin.x);
        glyph->fAdvanceX = SkIntToFixed(gm.gmCellIncX);
        glyph->fAdvanceY = -SkIntToFixed(gm.gmCellIncY);

        // we outset by 1 in all dimensions, since the lcd image may bleed outside
        // of the computed bounds returned by GetGlyphOutline.
        // This was deduced by trial and error for small text (e.g. 8pt), so there
        // maybe a more precise way to make this adjustment...
        if (SkMask::kLCD16_Format == fRec.fMaskFormat) {
            glyph->fWidth += 2;
            glyph->fHeight += 2;
            glyph->fTop -= 1;
            glyph->fLeft -= 1;
        }

        if (fHiResFont) {
            SelectObject(fDDC, fHiResFont);
            sk_bzero(&gm, sizeof(gm));
            ret = GetGlyphOutlineW(fDDC, glyph->getGlyphID(0), GGO_METRICS | GGO_GLYPH_INDEX, &gm, 0, NULL, &fMat22Identity);
            if (GDI_ERROR != ret) {
                SkPoint advance;
                fHiResMatrix.mapXY(SkIntToScalar(gm.gmCellIncX), SkIntToScalar(gm.gmCellIncY), &advance);
                glyph->fAdvanceX = SkScalarToFixed(advance.fX);
                glyph->fAdvanceY = SkScalarToFixed(advance.fY);
            }
            SelectObject(fDDC, fFont);
        }
    } else {
        glyph->fWidth = 0;
    }
}

void SkScalerContext_Windows::generateFontMetrics(SkPaint::FontMetrics* mx, SkPaint::FontMetrics* my) {
// Note: This code was borrowed from generateLineHeight, which has a note
// stating that it may be incorrect.
    if (!(mx || my))
      return;

    SkASSERT(fDDC);

    OUTLINETEXTMETRIC otm;

    uint32_t ret = GetOutlineTextMetrics(fDDC, sizeof(otm), &otm);
    if (sizeof(otm) != ret) {
      return;
    }

    if (mx) {
        mx->fTop = -fScale * otm.otmTextMetrics.tmAscent;
		mx->fAscent = -fScale * otm.otmAscent;
		mx->fDescent = -fScale * otm.otmDescent;
		mx->fBottom = fScale * otm.otmTextMetrics.tmDescent;
		mx->fLeading = fScale * (otm.otmTextMetrics.tmInternalLeading
								 + otm.otmTextMetrics.tmExternalLeading);
    }

    if (my) {
		my->fTop = -fScale * otm.otmTextMetrics.tmAscent;
		my->fAscent = -fScale * otm.otmAscent;
		my->fDescent = -fScale * otm.otmDescent;
		my->fBottom = fScale * otm.otmTextMetrics.tmDescent;
		my->fLeading = fScale * (otm.otmTextMetrics.tmInternalLeading
								 + otm.otmTextMetrics.tmExternalLeading);
    }
}

#include "SkColorPriv.h"

static inline uint16_t rgb_to_lcd16(uint32_t rgb) {
    int r = (rgb >> 16) & 0xFF;
    int g = (rgb >>  8) & 0xFF;
    int b = (rgb >>  0) & 0xFF;

    // invert, since we draw black-on-white, but we want the original
    // src mask values.
    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
    return SkPackRGB16(SkR32ToR16(r), SkG32ToG16(g), SkB32ToB16(b));
}

static int alignTo32(int n) {
    return (n + 31) & ~31;
}

struct MyBitmapInfo : public BITMAPINFO {
    RGBQUAD fMoreSpaceForColors[1];
};

void SkScalerContext_Windows::generateImage(const SkGlyph& glyph) {

    SkAutoMutexAcquire  ac(gFTMutex);

    SkASSERT(fDDC);

    const bool isBW = SkMask::kBW_Format == fRec.fMaskFormat;
    if ((SkMask::kLCD16_Format == fRec.fMaskFormat) || isBW) {
        HDC dc = CreateCompatibleDC(0);
        void* bits = 0;
        int biWidth = isBW ? alignTo32(glyph.fWidth) : glyph.fWidth;
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
        info.bmiHeader.biHeight = glyph.fHeight;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = isBW ? 1 : 32;
        info.bmiHeader.biCompression = BI_RGB;
        if (isBW) {
            info.bmiHeader.biClrUsed = 2;
        }
        HBITMAP bm = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, 0, 0);
        SelectObject(dc, bm);

        // erase to white
        size_t srcRB = isBW ? (biWidth >> 3) : (glyph.fWidth << 2);
        size_t size = glyph.fHeight * srcRB;
        memset(bits, isBW ? 0 : 0xFF, size);

        SetGraphicsMode(dc, GM_ADVANCED);
        SetBkMode(dc, TRANSPARENT);
        SetTextAlign(dc, TA_LEFT | TA_BASELINE);

        XFORM xform = fXform;
        xform.eDx = (float)-glyph.fLeft;
        xform.eDy = (float)-glyph.fTop;
        SetWorldTransform(dc, &xform);

        HGDIOBJ prevFont = SelectObject(dc, fFont);
        COLORREF color = SetTextColor(dc, isBW ? 0xFFFFFF : 0);
        SkASSERT(color != CLR_INVALID);
        uint16_t glyphID = glyph.getGlyphID();
#if defined(UNICODE)
        ExtTextOut(dc, 0, 0, ETO_GLYPH_INDEX, NULL, (LPCWSTR)&glyphID, 1, NULL);
#else
        ExtTextOut(dc, 0, 0, ETO_GLYPH_INDEX, NULL, (LPCSTR)&glyphID, 1, NULL);
#endif
        GdiFlush();

        // downsample from rgba to rgb565
        int width = glyph.fWidth;
        size_t dstRB = glyph.rowBytes();
        if (isBW) {
            const uint8_t* src = (const uint8_t*)bits;
            // gdi's bitmap is upside-down, so we reverse dst walking in Y
            uint8_t* dst = (uint8_t*)((char*)glyph.fImage + (glyph.fHeight - 1) * dstRB);
            for (int y = 0; y < glyph.fHeight; y++) {
                memcpy(dst, src, dstRB);
                src += srcRB;
                dst -= dstRB;
            }
        } else {    // LCD16
            const uint32_t* src = (const uint32_t*)bits;
            // gdi's bitmap is upside-down, so we reverse dst walking in Y
            uint16_t* dst = (uint16_t*)((char*)glyph.fImage + (glyph.fHeight - 1) * dstRB);
            for (int y = 0; y < glyph.fHeight; y++) {
                for (int i = 0; i < width; i++) {
                    dst[i] = rgb_to_lcd16(src[i]);
                }
                src = (const uint32_t*)((const char*)src + srcRB);
                dst = (uint16_t*)((char*)dst - dstRB);
            }
        }

        DeleteDC(dc);
        DeleteObject(bm);
        return;
    }

    GLYPHMETRICS gm;
    memset(&gm, 0, sizeof(gm));
    uint32_t bytecount = 0;
    uint32_t total_size = GetGlyphOutlineW(fDDC, glyph.fID, GGO_GRAY8_BITMAP | GGO_GLYPH_INDEX, &gm, 0, NULL, &fMat22);
    if (GDI_ERROR != total_size && total_size > 0) {
        uint8_t *pBuff = new uint8_t[total_size];
        if (NULL != pBuff) {
            total_size = GetGlyphOutlineW(fDDC, glyph.fID, GGO_GRAY8_BITMAP | GGO_GLYPH_INDEX, &gm, total_size, pBuff, &fMat22);

            SkASSERT(total_size != GDI_ERROR);

            SkASSERT(glyph.fWidth == gm.gmBlackBoxX);
            SkASSERT(glyph.fHeight == gm.gmBlackBoxY);

            uint8_t* dst = (uint8_t*)glyph.fImage;
            uint32_t pitch = (gm.gmBlackBoxX + 3) & ~0x3;
            if (pitch != glyph.rowBytes()) {
                SkASSERT(false); // glyph.fImage has different rowsize!?
            }

            for (int32_t y = gm.gmBlackBoxY - 1; y >= 0; y--) {
                uint8_t* src = pBuff + pitch * y;

                for (uint32_t x = 0; x < gm.gmBlackBoxX; x++) {
                    if (*src > 63) {
                        *dst = 0xFF;
                    }
                    else {
                        *dst = *src << 2; // scale to 0-255
                    }
                    dst++;
                    src++;
                    bytecount++;
                }
                memset(dst, 0, glyph.rowBytes() - glyph.fWidth);
                dst += glyph.rowBytes() - glyph.fWidth;
            }

            delete[] pBuff;
        }
    }

    SkASSERT(GDI_ERROR != total_size && total_size >= 0);

}

void SkScalerContext_Windows::generatePath(const SkGlyph& glyph, SkPath* path) {

    SkAutoMutexAcquire  ac(gFTMutex);

    SkASSERT(&glyph && path);
    SkASSERT(fDDC);

    path->reset();

#if 0
    char buf[1024];
    sprintf(buf, "generatePath: id:%d, w=%d, h=%d, font:%s,fh:%d\n", glyph.fID, glyph.fWidth, glyph.fHeight, lf.lfFaceName, lf.lfHeight);
    OutputDebugString(buf);
#endif

    GLYPHMETRICS gm;
    uint32_t total_size = GetGlyphOutlineW(fDDC, glyph.fID, GGO_NATIVE | GGO_GLYPH_INDEX, &gm, BUFFERSIZE, glyphbuf, &fMat22);

    if (GDI_ERROR != total_size) {

        const uint8_t* cur_glyph = glyphbuf;
        const uint8_t* end_glyph = glyphbuf + total_size;

        while(cur_glyph < end_glyph) {
            const TTPOLYGONHEADER* th = (TTPOLYGONHEADER*)cur_glyph;

            const uint8_t* end_poly = cur_glyph + th->cb;
            const uint8_t* cur_poly = cur_glyph + sizeof(TTPOLYGONHEADER);

            path->moveTo(SkFixedToScalar(*(SkFixed*)(&th->pfxStart.x)), SkFixedToScalar(*(SkFixed*)(&th->pfxStart.y)));

            while(cur_poly < end_poly) {
                const TTPOLYCURVE* pc = (const TTPOLYCURVE*)cur_poly;

                if (pc->wType == TT_PRIM_LINE) {
                    for (uint16_t i = 0; i < pc->cpfx; i++) {
                        path->lineTo(SkFixedToScalar(*(SkFixed*)(&pc->apfx[i].x)), SkFixedToScalar(*(SkFixed*)(&pc->apfx[i].y)));
                    }
                }

                if (pc->wType == TT_PRIM_QSPLINE) {
                    for (uint16_t u = 0; u < pc->cpfx - 1; u++) { // Walk through points in spline
                        POINTFX pnt_b = pc->apfx[u];    // B is always the current point
                        POINTFX pnt_c = pc->apfx[u+1];

                        if (u < pc->cpfx - 2) {          // If not on last spline, compute C
                            pnt_c.x = SkFixedToFIXED(SkFixedAve(*(SkFixed*)(&pnt_b.x), *(SkFixed*)(&pnt_c.x)));
                            pnt_c.y = SkFixedToFIXED(SkFixedAve(*(SkFixed*)(&pnt_b.y), *(SkFixed*)(&pnt_c.y)));
                        }

                        path->quadTo(SkFixedToScalar(*(SkFixed*)(&pnt_b.x)), SkFixedToScalar(*(SkFixed*)(&pnt_b.y)), SkFixedToScalar(*(SkFixed*)(&pnt_c.x)), SkFixedToScalar(*(SkFixed*)(&pnt_c.y)));
                    }
                }
                cur_poly += sizeof(uint16_t) * 2 + sizeof(POINTFX) * pc->cpfx;
            }
            cur_glyph += th->cb;
            path->close();
        }
    }
    else {
        SkASSERT(false);
    }
    //char buf[1024];
    //sprintf(buf, "generatePath: count:%d\n", count);
    //OutputDebugString(buf);
}

void SkFontHost::Serialize(const SkTypeface* face, SkWStream* stream) {
    SkASSERT(!"SkFontHost::Serialize unimplemented");
}

SkTypeface* SkFontHost::Deserialize(SkStream* stream) {
    SkASSERT(!"SkFontHost::Deserialize unimplemented");
    return NULL;
}

static bool getWidthAdvance(HDC hdc, int gId, int16_t* advance) {
    // Initialize the MAT2 structure to the identify transformation matrix.
    static const MAT2 mat2 = {SkScalarToFIXED(1), SkScalarToFIXED(0),
                        SkScalarToFIXED(0), SkScalarToFIXED(1)};
    int flags = GGO_METRICS | GGO_GLYPH_INDEX;
    GLYPHMETRICS gm;
    if (GDI_ERROR == GetGlyphOutline(hdc, gId, flags, &gm, 0, NULL, &mat2)) {
        return false;
    }
    SkASSERT(advance);
    *advance = gm.gmCellIncX;
    return true;
}

// static
SkAdvancedTypefaceMetrics* SkFontHost::GetAdvancedTypefaceMetrics(
        uint32_t fontID,
        SkAdvancedTypefaceMetrics::PerGlyphInfo perGlyphInfo) {
    LOGFONT lf;
    GetLogFontByID(fontID, &lf);
    SkAdvancedTypefaceMetrics* info = NULL;

    HDC hdc = CreateCompatibleDC(NULL);
    HFONT font = CreateFontIndirect(&lf);
    HFONT savefont = (HFONT)SelectObject(hdc, font);
    HFONT designFont = NULL;

    // To request design units, create a logical font whose height is specified
    // as unitsPerEm.
    OUTLINETEXTMETRIC otm;
    if (!GetOutlineTextMetrics(hdc, sizeof(otm), &otm) ||
        !GetTextFace(hdc, LF_FACESIZE, lf.lfFaceName)) {
        goto Error;
    }
    lf.lfHeight = -SkToS32(otm.otmEMSquare);
    designFont = CreateFontIndirect(&lf);
    SelectObject(hdc, designFont);
    if (!GetOutlineTextMetrics(hdc, sizeof(otm), &otm)) {
        goto Error;
    }
    const unsigned glyphCount = calculateGlyphCount(hdc);

    info = new SkAdvancedTypefaceMetrics;
    info->fEmSize = otm.otmEMSquare;
    info->fMultiMaster = false;
    info->fLastGlyphID = SkToU16(glyphCount - 1);
    info->fStyle = 0;
#ifdef UNICODE
    // Get the buffer size needed first.
    size_t str_len = WideCharToMultiByte(CP_UTF8, 0, lf.lfFaceName, -1, NULL,
                                         0, NULL, NULL);
    // Allocate a buffer (str_len already has terminating null accounted for).
    char *familyName = new char[str_len];
    // Now actually convert the string.
    WideCharToMultiByte(CP_UTF8, 0, lf.lfFaceName, -1, familyName, str_len,
                          NULL, NULL);
    info->fFontName.set(familyName);
    delete [] familyName;
#else
    info->fFontName.set(lf.lfFaceName);
#endif

    if (perGlyphInfo & SkAdvancedTypefaceMetrics::kToUnicode_PerGlyphInfo) {
        populate_glyph_to_unicode(hdc, glyphCount, &(info->fGlyphToUnicode));
    }

    if (otm.otmTextMetrics.tmPitchAndFamily & TMPF_TRUETYPE) {
        info->fType = SkAdvancedTypefaceMetrics::kTrueType_Font;
    } else {
        info->fType = SkAdvancedTypefaceMetrics::kOther_Font;
        info->fItalicAngle = 0;
        info->fAscent = 0;
        info->fDescent = 0;
        info->fStemV = 0;
        info->fCapHeight = 0;
        info->fBBox = SkIRect::MakeEmpty();
        return info;
    }
    
    // If this bit is clear the font is a fixed pitch font.
    if (!(otm.otmTextMetrics.tmPitchAndFamily & TMPF_FIXED_PITCH)) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kFixedPitch_Style;
    }
    if (otm.otmTextMetrics.tmItalic) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kItalic_Style;
    }
    // Setting symbolic style by default for now.
    info->fStyle |= SkAdvancedTypefaceMetrics::kSymbolic_Style;
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
    int16_t min_width = SHRT_MAX;
    info->fStemV = 0;
    char stem_chars[] = {'i', 'I', '!', '1'};
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

    // If bit 1 is set, the font may not be embedded in a document.
    // If bit 1 is clear, the font can be embedded.
    // If bit 2 is set, the embedding is read-only.
    if (otm.otmfsType & 0x1) {
        info->fType = SkAdvancedTypefaceMetrics::kNotEmbeddable_Font;
    } else if (perGlyphInfo &
               SkAdvancedTypefaceMetrics::kHAdvance_PerGlyphInfo) {
        info->fGlyphWidths.reset(
            getAdvanceData(hdc, glyphCount, &getWidthAdvance));
    }

Error:
    SelectObject(hdc, savefont);
    DeleteObject(designFont);
    DeleteObject(font);
    DeleteDC(hdc);

    return info;
}

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream) {

    //Should not be used on Windows, keep linker happy
    SkASSERT(false);
    return SkCreateTypefaceFromLOGFONT(get_default_font());
}

SkStream* SkFontHost::OpenStream(SkFontID uniqueID) {
    const DWORD kTTCTag =
        SkEndian_SwapBE32(SkSetFourByteTag('t', 't', 'c', 'f'));
    LOGFONT lf;
    GetLogFontByID(uniqueID, &lf);

    HDC hdc = ::CreateCompatibleDC(NULL);
    HFONT font = CreateFontIndirect(&lf);
    HFONT savefont = (HFONT)SelectObject(hdc, font);

    SkMemoryStream* stream = NULL;
    DWORD tables[2] = {kTTCTag, 0};
    for (int i = 0; i < SK_ARRAY_COUNT(tables); i++) {
        size_t bufferSize = GetFontData(hdc, tables[i], 0, NULL, 0);
        if (bufferSize != GDI_ERROR) {
            stream = new SkMemoryStream(bufferSize);
            if (GetFontData(hdc, tables[i], 0, (void*)stream->getMemoryBase(),
                            bufferSize)) {
                break;
            } else {
                delete stream;
                stream = NULL;
            }
        }
    }

    SelectObject(hdc, savefont);
    DeleteObject(font);
    DeleteDC(hdc);

    return stream;
}

SkScalerContext* SkFontHost::CreateScalerContext(const SkDescriptor* desc) {
    return SkNEW_ARGS(SkScalerContext_Windows, (desc));
}

/** Return the closest matching typeface given either an existing family
 (specified by a typeface in that family) or by a familyName, and a
 requested style.
 1) If familyFace is null, use famillyName.
 2) If famillyName is null, use familyFace.
 3) If both are null, return the default font that best matches style
 This MUST not return NULL.
 */

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                                       const char familyName[],
                                       const void* data, size_t bytelength,
                                       SkTypeface::Style style) {
    LOGFONT lf;
    if (NULL == familyFace && NULL == familyName) {
        lf = get_default_font();
    } else if (familyFace) {
        LogFontTypeface* face = (LogFontTypeface*)familyFace;
        lf = face->fLogFont;
    } else {
        memset(&lf, 0, sizeof(LOGFONT));
#ifdef UNICODE
        // Get the buffer size needed first.
        size_t str_len = ::MultiByteToWideChar(CP_UTF8, 0, familyName,
                                                -1, NULL, 0);
        // Allocate a buffer (str_len already has terminating null
        // accounted for).
        wchar_t *wideFamilyName = new wchar_t[str_len];
        // Now actually convert the string.
        ::MultiByteToWideChar(CP_UTF8, 0, familyName, -1,
                                wideFamilyName, str_len);
        ::wcsncpy(lf.lfFaceName, wideFamilyName, LF_FACESIZE);
        delete [] wideFamilyName;
#else
        ::strncpy(lf.lfFaceName, familyName, LF_FACESIZE);
#endif
        lf.lfFaceName[LF_FACESIZE-1] = '\0';
    }
    setStyle(&lf, style);
    return SkCreateTypefaceFromLOGFONT(lf);
}

size_t SkFontHost::ShouldPurgeFontCache(size_t sizeAllocatedSoFar) {
    if (sizeAllocatedSoFar > FONT_CACHE_MEMORY_BUDGET)
        return sizeAllocatedSoFar - FONT_CACHE_MEMORY_BUDGET;
    else
        return 0;   // nothing to do
}

int SkFontHost::ComputeGammaFlag(const SkPaint& paint) {
    return 0;
}

void SkFontHost::GetGammaTables(const uint8_t* tables[2]) {
    tables[0] = NULL;   // black gamma (e.g. exp=1.4)
    tables[1] = NULL;   // white gamma (e.g. exp= 1/1.4)
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(const char path[]) {
    printf("SkFontHost::CreateTypefaceFromFile unimplemented");
    return NULL;
}

void SkFontHost::FilterRec(SkScalerContext::Rec* rec) {
    // We don't control the hinting nor ClearType settings here
    rec->setHinting(SkPaint::kNormal_Hinting);

    // we do support LCD16
    if (SkMask::kLCD16_Format == rec->fMaskFormat) {
        return;
    }

    if (SkMask::FormatIsLCD((SkMask::Format)rec->fMaskFormat)) {
        rec->fMaskFormat = SkMask::kA8_Format;
    }
}

#endif // WIN32
