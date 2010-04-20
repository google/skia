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

#include "SkFontHost.h"
#include "SkDescriptor.h"
#include "SkThread.h"

#ifdef WIN32
#include "windows.h"
#include "tchar.h"

// client3d has to undefine this for now
#define CAN_USE_LOGFONT_NAME

static SkMutex gFTMutex;

// these globals are loaded (once) by get_default_font()
static LOGFONT gDefaultFont = {0};

static const uint16_t BUFFERSIZE = (16384 - 32);
static uint8_t glyphbuf[BUFFERSIZE];

// Give 1MB font cache budget
#define FONT_CACHE_MEMORY_BUDGET    (1024 * 1024)

static inline FIXED SkFixedToFIXED(SkFixed x) {
    return *(FIXED*)(&x);
}

static inline FIXED SkScalarToFIXED(SkScalar x) {
    return SkFixedToFIXED(SkScalarToFixed(x));
}

// This will generate a unique ID based on the fontname + fontstyle
// and also used by upper layer
uint32_t FontFaceChecksum(const TCHAR *q, SkTypeface::Style style)
{
    if (!q) return style;

    // From "Performance in Practice of String Hashing Functions"
    // Ramakrishna & Zobel
    const uint32_t L = 5;
    const uint32_t R = 2;

    uint32_t h = 0x12345678;
    while (*q) {
        //uint32_t ql = tolower(*q);
        h ^= ((h << L) + (h >> R) + *q);
        q ++;
    }

    // add style
    h = _rotl(h, 3) ^ style;

    return h;
}

static SkTypeface::Style GetFontStyle(const LOGFONT& lf) {
    int style = SkTypeface::kNormal;
    if (lf.lfWeight == FW_SEMIBOLD || lf.lfWeight == FW_DEMIBOLD || lf.lfWeight == FW_BOLD)
        style |= SkTypeface::kBold;
    if (lf.lfItalic)
        style |= SkTypeface::kItalic;

    return (SkTypeface::Style)style;
}

struct SkFaceRec {
    SkFaceRec*      fNext;
    uint32_t        fRefCnt;
    uint32_t        fFontID;    // checksum of fFace
    LOGFONT         fFace;

    SkFaceRec() : fFontID(-1), fRefCnt(0) {
        memset(&fFace, 0, sizeof(LOGFONT));
    }
    ~SkFaceRec() {}

    uint32_t ref() {
        return ++fRefCnt;
    }
};

// Font Face list
static SkFaceRec*   gFaceRecHead = NULL;

static SkFaceRec* find_ft_face(uint32_t fontID) {
    SkFaceRec* rec = gFaceRecHead;
    while (rec) {
        if (rec->fFontID == fontID) {
            return rec;
        }
        rec = rec->fNext;
    }

    return NULL;
}

static SkFaceRec* insert_ft_face(const LOGFONT& lf) {
    // need a const char*
    uint32_t id = FontFaceChecksum(&(lf.lfFaceName[0]), GetFontStyle(lf));
    SkFaceRec* rec = find_ft_face(id);
    if (rec) {
        return rec;  // found?
    }

    rec = SkNEW(SkFaceRec);
    rec->fFontID = id;
    memcpy(&(rec->fFace), &lf, sizeof(LOGFONT));
    rec->fNext = gFaceRecHead;
    gFaceRecHead = rec;

    return rec;
}

static void unref_ft_face(uint32_t fontID) {

    SkFaceRec* rec = gFaceRecHead;
    SkFaceRec* prev = NULL;
    while (rec) {
        SkFaceRec* next = rec->fNext;
        if (rec->fFontID == fontID) {
            if (--rec->fRefCnt == 0) {
                if (prev)
                    prev->fNext = next;
                else
                    gFaceRecHead = next;

                SkDELETE(rec);
            }
            return;
        }
        prev = rec;
        rec = next;
    }
    SkASSERT("shouldn't get here, face not in list");
}

// have to do this because SkTypeface::SkTypeface() is protected
class FontFaceRec_Typeface : public SkTypeface {
public:

    FontFaceRec_Typeface(Style style, uint32_t id) : SkTypeface(style, id) {};

    virtual ~FontFaceRec_Typeface() {};
};

static const LOGFONT* get_default_font() {
    // don't hardcode on Windows, Win2000, XP, Vista, and international all have different default
    // and the user could change too

    if (gDefaultFont.lfFaceName[0] != 0) {
        return &gDefaultFont;
    }

    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

//  lfMessageFont is garbage on my XP, so skip for now
//  memcpy(&gDefaultFont, &(ncm.lfMessageFont), sizeof(LOGFONT));

    return &gDefaultFont;
}

static SkTypeface* CreateTypeface_(const LOGFONT& lf) {

    SkTypeface::Style style = GetFontStyle(lf);
    FontFaceRec_Typeface* ptypeface = new FontFaceRec_Typeface(style, FontFaceChecksum(lf.lfFaceName, style));

    if (NULL == ptypeface) {
        SkASSERT(false);
        return NULL;
    }

    SkFaceRec* rec = insert_ft_face(lf);
    SkASSERT(rec);

    return ptypeface;
}

uint32_t SkFontHost::NextLogicalFont(uint32_t fontID) {
  // Zero means that we don't have any fallback fonts for this fontID.
  // This function is implemented on Android, but doesn't have much
  // meaning here.
  return 0;
}

class SkScalerContext_Windows : public SkScalerContext {
public:
    SkScalerContext_Windows(const SkDescriptor* desc);
    virtual ~SkScalerContext_Windows();

protected:
    virtual unsigned generateGlyphCount() const;
    virtual uint16_t generateCharToGlyph(SkUnichar uni);
    virtual void generateAdvance(SkGlyph* glyph);
    virtual void generateMetrics(SkGlyph* glyph);
    virtual void generateImage(const SkGlyph& glyph);
    virtual void generatePath(const SkGlyph& glyph, SkPath* path);
    virtual void generateLineHeight(SkPoint* ascent, SkPoint* descent);
    virtual void generateFontMetrics(SkPaint::FontMetrics* mX, SkPaint::FontMetrics* mY);
    //virtual SkDeviceContext getDC() {return ddc;}
private:
    uint32_t    fFontID;
    LOGFONT     lf;
    MAT2        mat22;
    HDC         ddc;
    HFONT       savefont;
    HFONT       font;
};

SkScalerContext_Windows::SkScalerContext_Windows(const SkDescriptor* desc) : SkScalerContext(desc), ddc(0), font(0), savefont(0) {
    SkAutoMutexAcquire  ac(gFTMutex);

    fFontID = fRec.fFontID;
    SkFaceRec* rec = find_ft_face(fRec.fFontID);
    if (rec) {
        rec->ref();
        memcpy(&lf, &(rec->fFace), sizeof(LOGFONT));
    }
    else {
        SkASSERT(false);
        memcpy(&lf, &gDefaultFont, sizeof(LOGFONT));
    }

    mat22.eM11 = SkScalarToFIXED(fRec.fPost2x2[0][0]);
    mat22.eM12 = SkScalarToFIXED(-fRec.fPost2x2[0][1]);
    mat22.eM21 = SkScalarToFIXED(fRec.fPost2x2[1][0]);
    mat22.eM22 = SkScalarToFIXED(-fRec.fPost2x2[1][1]);

    ddc = ::CreateCompatibleDC(NULL);
    SetBkMode(ddc, TRANSPARENT);

    // Perform the dpi adjustment.
    SkScalar height = -(fRec.fTextSize * GetDeviceCaps(ddc, LOGPIXELSY) / 72);
    lf.lfHeight = SkScalarRound(height);
    font = CreateFontIndirect(&lf);
    savefont = (HFONT)SelectObject(ddc, font);
}

SkScalerContext_Windows::~SkScalerContext_Windows() {
    unref_ft_face(fFontID);

    if (ddc) {
        ::SelectObject(ddc, savefont);
        ::DeleteDC(ddc);
        ddc = NULL;
    }
    if (font) {
        ::DeleteObject(font);
    }
}

unsigned SkScalerContext_Windows::generateGlyphCount() const {
    return 0xFFFF;
    //    return fFace->num_glyphs;
}

uint16_t SkScalerContext_Windows::generateCharToGlyph(SkUnichar uni) {

    //uint16_t index = 0;
    //GetGlyphIndicesW(ddc, &(uint16_t&)uni, 1, &index, 0);
    //return index;

    // let's just use the uni as index on Windows
    return SkToU16(uni);
}

void SkScalerContext_Windows::generateAdvance(SkGlyph* glyph) {
    this->generateMetrics(glyph);
}

void SkScalerContext_Windows::generateMetrics(SkGlyph* glyph) {

    SkASSERT(ddc);

    GLYPHMETRICS gm;
    memset(&gm, 0, sizeof(gm));

    glyph->fRsbDelta = 0;
    glyph->fLsbDelta = 0;

    UINT glyphIndexFlag = 0; //glyph->fIsCodePoint ? 0 : GGO_GLYPH_INDEX;
    //    UINT glyphIndexFlag = GGO_GLYPH_INDEX;
    // Note: need to use GGO_GRAY8_BITMAP instead of GGO_METRICS because GGO_METRICS returns a smaller
    // BlackBlox; we need the bigger one in case we need the image.  fAdvance is the same.
    uint32_t ret = GetGlyphOutlineW(ddc, glyph->getGlyphID(0), GGO_GRAY8_BITMAP | glyphIndexFlag, &gm, 0, NULL, &mat22);

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
    } else {
        glyph->fWidth = 0;
    }

#if 0
    char buf[1024];
    sprintf(buf, "generateMetrics: id:%d, w=%d, h=%d, font:%s, fh:%d\n", glyph->fID, glyph->fWidth, glyph->fHeight, lf.lfFaceName, lf.lfHeight);
    OutputDebugString(buf);
#endif
}

void SkScalerContext_Windows::generateFontMetrics(SkPaint::FontMetrics* mx, SkPaint::FontMetrics* my) {
// Note: This code was borrowed from generateLineHeight, which has a note
// stating that it may be incorrect.
    if (!(mx || my))
      return;

    SkASSERT(ddc);

    OUTLINETEXTMETRIC otm;

    uint32_t ret = GetOutlineTextMetrics(ddc, sizeof(otm), &otm);
    if (sizeof(otm) != ret) {
      return;
    }

    if (mx) {
      mx->fTop = -SkIntToScalar(otm.otmTextMetrics.tmAscent);  // Actually a long.
      mx->fAscent = -SkIntToScalar(otm.otmAscent);
      mx->fDescent = -SkIntToScalar(otm.otmDescent);
      mx->fBottom = SkIntToScalar(otm.otmTextMetrics.tmDescent);  // Long
      mx->fLeading = SkIntToScalar(otm.otmTextMetrics.tmInternalLeading
          + otm.otmTextMetrics.tmExternalLeading);  // Long
    }

    if (my) {
      my->fTop = -SkIntToScalar(otm.otmTextMetrics.tmAscent);  // Actually a long.
      my->fAscent = -SkIntToScalar(otm.otmAscent);
      my->fDescent = -SkIntToScalar(otm.otmDescent);
      my->fBottom = SkIntToScalar(otm.otmTextMetrics.tmDescent);  // Long
      my->fLeading = SkIntToScalar(otm.otmTextMetrics.tmInternalLeading
          + otm.otmTextMetrics.tmExternalLeading);  // Long
    }
}

void SkScalerContext_Windows::generateImage(const SkGlyph& glyph) {

    SkAutoMutexAcquire  ac(gFTMutex);

    SkASSERT(ddc);

    GLYPHMETRICS gm;
    memset(&gm, 0, sizeof(gm));

#if 0
    char buf[1024];
    sprintf(buf, "generateImage: id:%d, w=%d, h=%d, font:%s,fh:%d\n", glyph.fID, glyph.fWidth, glyph.fHeight, lf.lfFaceName, lf.lfHeight);
    OutputDebugString(buf);
#endif

    uint32_t bytecount = 0;
    UINT glyphIndexFlag = 0; //glyph.fIsCodePoint ? 0 : GGO_GLYPH_INDEX;
    //    UINT glyphIndexFlag = GGO_GLYPH_INDEX;
    uint32_t total_size = GetGlyphOutlineW(ddc, glyph.fID, GGO_GRAY8_BITMAP | glyphIndexFlag, &gm, 0, NULL, &mat22);
    if (GDI_ERROR != total_size && total_size > 0) {
        uint8_t *pBuff = new uint8_t[total_size];
        if (NULL != pBuff) {
            total_size = GetGlyphOutlineW(ddc, glyph.fID, GGO_GRAY8_BITMAP | glyphIndexFlag, &gm, total_size, pBuff, &mat22);

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
    SkASSERT(ddc);

    path->reset();

#if 0
    char buf[1024];
    sprintf(buf, "generatePath: id:%d, w=%d, h=%d, font:%s,fh:%d\n", glyph.fID, glyph.fWidth, glyph.fHeight, lf.lfFaceName, lf.lfHeight);
    OutputDebugString(buf);
#endif

    GLYPHMETRICS gm;
    UINT glyphIndexFlag = 0; //glyph.fIsCodePoint ? 0 : GGO_GLYPH_INDEX;
    uint32_t total_size = GetGlyphOutlineW(ddc, glyph.fID, GGO_NATIVE | glyphIndexFlag, &gm, BUFFERSIZE, glyphbuf, &mat22);

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


// Note:  not sure this is the correct implementation
void SkScalerContext_Windows::generateLineHeight(SkPoint* ascent, SkPoint* descent) {

    SkASSERT(ddc);

    OUTLINETEXTMETRIC otm;

    uint32_t ret = GetOutlineTextMetrics(ddc, sizeof(otm), &otm);

    if (sizeof(otm) == ret) {
        if (ascent)
            ascent->iset(0, otm.otmAscent);
        if (descent)
            descent->iset(0, otm.otmDescent);
    }

    return;
}

void SkFontHost::Serialize(const SkTypeface* face, SkWStream* stream) {
    SkASSERT(!"SkFontHost::Serialize unimplemented");
}

SkTypeface* SkFontHost::Deserialize(SkStream* stream) {
    SkASSERT(!"SkFontHost::Deserialize unimplemented");
    return NULL;
}

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream) {

    //Should not be used on Windows, keep linker happy
    SkASSERT(false);
    get_default_font();
    return CreateTypeface_(gDefaultFont);
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

    SkAutoMutexAcquire  ac(gFTMutex);

#ifndef CAN_USE_LOGFONT_NAME
    familyName = NULL;
    familyFace = NULL;
#endif

    // clip to legal style bits
    style = (SkTypeface::Style)(style & SkTypeface::kBoldItalic);

    SkTypeface* tf = NULL;
    if (NULL == familyFace && NULL == familyName) {
        LOGFONT lf;
        get_default_font();
        memcpy(&lf, &gDefaultFont, sizeof(LOGFONT));
        lf.lfWeight = (style & SkTypeface::kBold) != 0 ? FW_BOLD : FW_NORMAL ;
        lf.lfItalic = ((style & SkTypeface::kItalic) != 0);
        tf = CreateTypeface_(lf);
    } else {
#ifdef CAN_USE_LOGFONT_NAME
        LOGFONT lf;
        if (NULL != familyFace) {
            uint32_t id = familyFace->uniqueID();
            SkFaceRec* rec = find_ft_face(id);
            if (!rec) {
                SkASSERT(false);
                get_default_font();
                memcpy(&lf, &gDefaultFont, sizeof(LOGFONT));
            }
            else {
                memcpy(&lf, &(rec->fFace), sizeof(LOGFONT));
            }
        }
        else {
            memset(&lf, 0, sizeof(LOGFONT));

            lf.lfHeight = -11; // default
            lf.lfQuality = PROOF_QUALITY;
            lf.lfCharSet = DEFAULT_CHARSET;

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

        // use the style desired
        lf.lfWeight = (style & SkTypeface::kBold) != 0 ? FW_BOLD : FW_NORMAL ;
        lf.lfItalic = ((style & SkTypeface::kItalic) != 0);
        tf = CreateTypeface_(lf);
#endif
    }

    if (NULL == tf) {
        get_default_font();
        tf = CreateTypeface_(gDefaultFont);
    }
    return tf;
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
    SkASSERT(!"SkFontHost::CreateTypefaceFromFile unimplemented");
    return NULL;
}

void SkFontHost::FilterRec(SkScalerContext::Rec* rec) {
    // We don't control the hinting nor ClearType settings here
    rec->setHinting(SkPaint::kNormal_Hinting);
    if (SkMask::FormatIsLCD((SkMask::Format)rec->fMaskFormat)) {
        rec->fMaskFormat = SkMask::kA8_Format;
    }
}

#endif // WIN32
