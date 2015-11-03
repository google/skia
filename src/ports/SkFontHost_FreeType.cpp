/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAdvancedTypefaceMetrics.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkDescriptor.h"
#include "SkFDot6.h"
#include "SkFontDescriptor.h"
#include "SkFontHost_FreeType_common.h"
#include "SkGlyph.h"
#include "SkMask.h"
#include "SkMaskGamma.h"
#include "SkMatrix22.h"
#include "SkMutex.h"
#include "SkOTUtils.h"
#include "SkPath.h"
#include "SkScalerContext.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "SkTypes.h"

#if defined(SK_CAN_USE_DLOPEN)
#include <dlfcn.h>
#endif
#include <ft2build.h>
#include FT_ADVANCES_H
#include FT_BITMAP_H
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#include FT_MODULE_H
#include FT_MULTIPLE_MASTERS_H
#include FT_OUTLINE_H
#include FT_SIZES_H
#include FT_SYSTEM_H
#include FT_TRUETYPE_TABLES_H
#include FT_TYPE1_TABLES_H
#include FT_XFREE86_H

// FT_LOAD_COLOR and the corresponding FT_Pixel_Mode::FT_PIXEL_MODE_BGRA
// were introduced in FreeType 2.5.0.
// The following may be removed once FreeType 2.5.0 is required to build.
#ifndef FT_LOAD_COLOR
#    define FT_LOAD_COLOR ( 1L << 20 )
#    define FT_PIXEL_MODE_BGRA 7
#endif

//#define ENABLE_GLYPH_SPEW     // for tracing calls
//#define DUMP_STRIKE_CREATION
//#define SK_FONTHOST_FREETYPE_USE_NORMAL_LCD_FILTER
//#define SK_FONTHOST_FREETYPE_RUNTIME_VERSION
//#define SK_GAMMA_APPLY_TO_A8

using namespace skia_advanced_typeface_metrics_utils;

static bool isLCD(const SkScalerContext::Rec& rec) {
    return SkMask::kLCD16_Format == rec.fMaskFormat;
}

//////////////////////////////////////////////////////////////////////////

extern "C" {
    static void* sk_ft_alloc(FT_Memory, long size) {
        return sk_malloc_throw(size);
    }
    static void sk_ft_free(FT_Memory, void* block) {
        sk_free(block);
    }
    static void* sk_ft_realloc(FT_Memory, long cur_size, long new_size, void* block) {
        return sk_realloc_throw(block, new_size);
    }
};
FT_MemoryRec_ gFTMemory = { nullptr, sk_ft_alloc, sk_ft_free, sk_ft_realloc };

class FreeTypeLibrary : SkNoncopyable {
public:
    FreeTypeLibrary() : fLibrary(nullptr), fIsLCDSupported(false), fLCDExtra(0) {
        if (FT_New_Library(&gFTMemory, &fLibrary)) {
            return;
        }
        FT_Add_Default_Modules(fLibrary);

        // Setup LCD filtering. This reduces color fringes for LCD smoothed glyphs.
        // Default { 0x10, 0x40, 0x70, 0x40, 0x10 } adds up to 0x110, simulating ink spread.
        // SetLcdFilter must be called before SetLcdFilterWeights.
        if (FT_Library_SetLcdFilter(fLibrary, FT_LCD_FILTER_DEFAULT) == 0) {
            fIsLCDSupported = true;
            fLCDExtra = 2; //Using a filter adds one full pixel to each side.

#ifdef SK_FONTHOST_FREETYPE_USE_NORMAL_LCD_FILTER
            // Adds to 0x110 simulating ink spread, but provides better results than default.
            static unsigned char gGaussianLikeHeavyWeights[] = { 0x1A, 0x43, 0x56, 0x43, 0x1A, };

#    if SK_FONTHOST_FREETYPE_RUNTIME_VERSION > 0x020400
            FT_Library_SetLcdFilterWeights(fLibrary, gGaussianLikeHeavyWeights);
#    elif SK_CAN_USE_DLOPEN == 1
            //The FreeType library is already loaded, so symbols are available in process.
            void* self = dlopen(nullptr, RTLD_LAZY);
            if (self) {
                FT_Library_SetLcdFilterWeightsProc setLcdFilterWeights;
                //The following cast is non-standard, but safe for POSIX.
                *reinterpret_cast<void**>(&setLcdFilterWeights) =
                        dlsym(self, "FT_Library_SetLcdFilterWeights");
                dlclose(self);

                if (setLcdFilterWeights) {
                    setLcdFilterWeights(fLibrary, gGaussianLikeHeavyWeights);
                }
            }
#    endif
#endif
        }
    }
    ~FreeTypeLibrary() {
        if (fLibrary) {
            FT_Done_Library(fLibrary);
        }
    }

    FT_Library library() { return fLibrary; }
    bool isLCDSupported() { return fIsLCDSupported; }
    int lcdExtra() { return fLCDExtra; }

private:
    FT_Library fLibrary;
    bool fIsLCDSupported;
    int fLCDExtra;

    // FT_Library_SetLcdFilterWeights was introduced in FreeType 2.4.0.
    // The following platforms provide FreeType of at least 2.4.0.
    // Ubuntu >= 11.04 (previous deprecated April 2013)
    // Debian >= 6.0 (good)
    // OpenSuse >= 11.4 (previous deprecated January 2012 / Nov 2013 for Evergreen 11.2)
    // Fedora >= 14 (good)
    // Android >= Gingerbread (good)
    typedef FT_Error (*FT_Library_SetLcdFilterWeightsProc)(FT_Library, unsigned char*);
};

struct SkFaceRec;

SK_DECLARE_STATIC_MUTEX(gFTMutex);
static FreeTypeLibrary* gFTLibrary;
static SkFaceRec* gFaceRecHead;

// Private to RefFreeType and UnrefFreeType
static int gFTCount;

// Caller must lock gFTMutex before calling this function.
static bool ref_ft_library() {
    gFTMutex.assertHeld();
    SkASSERT(gFTCount >= 0);

    if (0 == gFTCount) {
        SkASSERT(nullptr == gFTLibrary);
        gFTLibrary = new FreeTypeLibrary;
    }
    ++gFTCount;
    return gFTLibrary->library();
}

// Caller must lock gFTMutex before calling this function.
static void unref_ft_library() {
    gFTMutex.assertHeld();
    SkASSERT(gFTCount > 0);

    --gFTCount;
    if (0 == gFTCount) {
        SkASSERT(nullptr != gFTLibrary);
        delete gFTLibrary;
        SkDEBUGCODE(gFTLibrary = nullptr;)
    }
}

class SkScalerContext_FreeType : public SkScalerContext_FreeType_Base {
public:
    SkScalerContext_FreeType(SkTypeface*, const SkDescriptor* desc);
    virtual ~SkScalerContext_FreeType();

    bool success() const {
        return fFTSize != nullptr && fFace != nullptr;
    }

protected:
    unsigned generateGlyphCount() override;
    uint16_t generateCharToGlyph(SkUnichar uni) override;
    void generateAdvance(SkGlyph* glyph) override;
    void generateMetrics(SkGlyph* glyph) override;
    void generateImage(const SkGlyph& glyph) override;
    void generatePath(const SkGlyph& glyph, SkPath* path) override;
    void generateFontMetrics(SkPaint::FontMetrics*) override;
    SkUnichar generateGlyphToChar(uint16_t glyph) override;

private:
    FT_Face     fFace;              // reference to shared face in gFaceRecHead
    FT_Size     fFTSize;            // our own copy
    FT_Int      fStrikeIndex;
    SkFixed     fScaleX, fScaleY;
    FT_Matrix   fMatrix22;
    uint32_t    fLoadGlyphFlags;
    bool        fDoLinearMetrics;
    bool        fLCDIsVert;

    // Need scalar versions for generateFontMetrics
    SkVector    fScale;
    SkMatrix    fMatrix22Scalar;

    FT_Error setupSize();
    void getBBoxForCurrentGlyph(SkGlyph* glyph, FT_BBox* bbox,
                                bool snapToPixelBoundary = false);
    bool getCBoxForLetter(char letter, FT_BBox* bbox);
    // Caller must lock gFTMutex before calling this function.
    void updateGlyphIfLCD(SkGlyph* glyph);
    // Caller must lock gFTMutex before calling this function.
    // update FreeType2 glyph slot with glyph emboldened
    void emboldenIfNeeded(FT_Face face, FT_GlyphSlot glyph);
};

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

struct SkFaceRec {
    SkFaceRec* fNext;
    FT_Face fFace;
    FT_StreamRec fFTStream;
    SkAutoTDelete<SkStreamAsset> fSkStream;
    uint32_t fRefCnt;
    uint32_t fFontID;

    // assumes ownership of the stream, will delete when its done
    SkFaceRec(SkStreamAsset* strm, uint32_t fontID);
};

extern "C" {
    static unsigned long sk_ft_stream_io(FT_Stream ftStream,
                                         unsigned long offset,
                                         unsigned char* buffer,
                                         unsigned long count)
    {
        SkStreamAsset* stream = static_cast<SkStreamAsset*>(ftStream->descriptor.pointer);

        if (count) {
            if (!stream->seek(offset)) {
                return 0;
            }
            count = stream->read(buffer, count);
        }
        return count;
    }

    static void sk_ft_stream_close(FT_Stream) {}
}

SkFaceRec::SkFaceRec(SkStreamAsset* stream, uint32_t fontID)
        : fNext(nullptr), fSkStream(stream), fRefCnt(1), fFontID(fontID)
{
    sk_bzero(&fFTStream, sizeof(fFTStream));
    fFTStream.size = fSkStream->getLength();
    fFTStream.descriptor.pointer = fSkStream;
    fFTStream.read  = sk_ft_stream_io;
    fFTStream.close = sk_ft_stream_close;
}

static void ft_face_setup_axes(FT_Face face, const SkFontData& data) {
    if (!(face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS)) {
        return;
    }

    SkDEBUGCODE(
        FT_MM_Var* variations = nullptr;
        if (FT_Get_MM_Var(face, &variations)) {
            SkDEBUGF(("INFO: font %s claims variations, but none found.\n", face->family_name));
            return;
        }
        SkAutoFree autoFreeVariations(variations);

        if (static_cast<FT_UInt>(data.getAxisCount()) != variations->num_axis) {
            SkDEBUGF(("INFO: font %s has %d variations, but %d were specified.\n",
                    face->family_name, variations->num_axis, data.getAxisCount()));
            return;
        }
    )

    SkAutoSTMalloc<4, FT_Fixed> coords(data.getAxisCount());
    for (int i = 0; i < data.getAxisCount(); ++i) {
        coords[i] = data.getAxis()[i];
    }
    if (FT_Set_Var_Design_Coordinates(face, data.getAxisCount(), coords.get())) {
        SkDEBUGF(("INFO: font %s has variations, but specified variations could not be set.\n",
                  face->family_name));
        return;
    }
}

// Will return 0 on failure
// Caller must lock gFTMutex before calling this function.
static FT_Face ref_ft_face(const SkTypeface* typeface) {
    gFTMutex.assertHeld();

    const SkFontID fontID = typeface->uniqueID();
    SkFaceRec* rec = gFaceRecHead;
    while (rec) {
        if (rec->fFontID == fontID) {
            SkASSERT(rec->fFace);
            rec->fRefCnt += 1;
            return rec->fFace;
        }
        rec = rec->fNext;
    }

    SkAutoTDelete<SkFontData> data(typeface->createFontData());
    if (nullptr == data || !data->hasStream()) {
        return nullptr;
    }

    // this passes ownership of stream to the rec
    rec = new SkFaceRec(data->detachStream(), fontID);

    FT_Open_Args args;
    memset(&args, 0, sizeof(args));
    const void* memoryBase = rec->fSkStream->getMemoryBase();
    if (memoryBase) {
        args.flags = FT_OPEN_MEMORY;
        args.memory_base = (const FT_Byte*)memoryBase;
        args.memory_size = rec->fSkStream->getLength();
    } else {
        args.flags = FT_OPEN_STREAM;
        args.stream = &rec->fFTStream;
    }

    FT_Error err = FT_Open_Face(gFTLibrary->library(), &args, data->getIndex(), &rec->fFace);
    if (err) {
        SkDEBUGF(("ERROR: unable to open font '%x'\n", fontID));
        delete rec;
        return nullptr;
    }
    SkASSERT(rec->fFace);

    ft_face_setup_axes(rec->fFace, *data);

    // FreeType will set the charmap to the "most unicode" cmap if it exists.
    // If there are no unicode cmaps, the charmap is set to nullptr.
    // However, "symbol" cmaps should also be considered "fallback unicode" cmaps
    // because they are effectively private use area only (even if they aren't).
    // This is the last on the fallback list at
    // https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
    if (!rec->fFace->charmap) {
        FT_Select_Charmap(rec->fFace, FT_ENCODING_MS_SYMBOL);
    }

    rec->fNext = gFaceRecHead;
    gFaceRecHead = rec;
    return rec->fFace;
}

// Caller must lock gFTMutex before calling this function.
static void unref_ft_face(FT_Face face) {
    gFTMutex.assertHeld();

    SkFaceRec*  rec = gFaceRecHead;
    SkFaceRec*  prev = nullptr;
    while (rec) {
        SkFaceRec* next = rec->fNext;
        if (rec->fFace == face) {
            if (--rec->fRefCnt == 0) {
                if (prev) {
                    prev->fNext = next;
                } else {
                    gFaceRecHead = next;
                }
                FT_Done_Face(face);
                delete rec;
            }
            return;
        }
        prev = rec;
        rec = next;
    }
    SkDEBUGFAIL("shouldn't get here, face not in list");
}

class AutoFTAccess {
public:
    AutoFTAccess(const SkTypeface* tf) : fFace(nullptr) {
        gFTMutex.acquire();
        if (!ref_ft_library()) {
            sk_throw();
        }
        fFace = ref_ft_face(tf);
    }

    ~AutoFTAccess() {
        if (fFace) {
            unref_ft_face(fFace);
        }
        unref_ft_library();
        gFTMutex.release();
    }

    FT_Face face() { return fFace; }

private:
    FT_Face     fFace;
};

///////////////////////////////////////////////////////////////////////////

static bool canEmbed(FT_Face face) {
    FT_UShort fsType = FT_Get_FSType_Flags(face);
    return (fsType & (FT_FSTYPE_RESTRICTED_LICENSE_EMBEDDING |
                      FT_FSTYPE_BITMAP_EMBEDDING_ONLY)) == 0;
}

static bool canSubset(FT_Face face) {
    FT_UShort fsType = FT_Get_FSType_Flags(face);
    return (fsType & FT_FSTYPE_NO_SUBSETTING) == 0;
}

static bool GetLetterCBox(FT_Face face, char letter, FT_BBox* bbox) {
    const FT_UInt glyph_id = FT_Get_Char_Index(face, letter);
    if (!glyph_id)
        return false;
    if (FT_Load_Glyph(face, glyph_id, FT_LOAD_NO_SCALE) != 0)
        return false;
    FT_Outline_Get_CBox(&face->glyph->outline, bbox);
    return true;
}

static bool getWidthAdvance(FT_Face face, int gId, int16_t* data) {
    FT_Fixed advance = 0;
    if (FT_Get_Advances(face, gId, 1, FT_LOAD_NO_SCALE, &advance)) {
        return false;
    }
    SkASSERT(data);
    *data = advance;
    return true;
}

static void populate_glyph_to_unicode(FT_Face& face, SkTDArray<SkUnichar>* glyphToUnicode) {
    glyphToUnicode->setCount(face->num_glyphs);
    sk_bzero(glyphToUnicode->begin(), sizeof((*glyphToUnicode)[0]) * face->num_glyphs);

    FT_UInt glyphIndex;
    SkUnichar charCode = FT_Get_First_Char(face, &glyphIndex);
    while (glyphIndex) {
        (*glyphToUnicode)[glyphIndex] = charCode;
        charCode = FT_Get_Next_Char(face, charCode, &glyphIndex);
    }
}

SkAdvancedTypefaceMetrics* SkTypeface_FreeType::onGetAdvancedTypefaceMetrics(
        PerGlyphInfo perGlyphInfo,
        const uint32_t* glyphIDs,
        uint32_t glyphIDsCount) const {
#if defined(SK_BUILD_FOR_MAC)
    return nullptr;
#else
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return nullptr;
    }

    SkAdvancedTypefaceMetrics* info = new SkAdvancedTypefaceMetrics;
    info->fFontName.set(FT_Get_Postscript_Name(face));
    info->fFlags = SkAdvancedTypefaceMetrics::kEmpty_FontFlag;
    if (FT_HAS_MULTIPLE_MASTERS(face)) {
        info->fFlags = SkTBitOr<SkAdvancedTypefaceMetrics::FontFlags>(
                info->fFlags, SkAdvancedTypefaceMetrics::kMultiMaster_FontFlag);
    }
    if (!canEmbed(face)) {
        info->fFlags = SkTBitOr<SkAdvancedTypefaceMetrics::FontFlags>(
                info->fFlags,
                SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag);
    }
    if (!canSubset(face)) {
        info->fFlags = SkTBitOr<SkAdvancedTypefaceMetrics::FontFlags>(
                info->fFlags,
                SkAdvancedTypefaceMetrics::kNotSubsettable_FontFlag);
    }
    info->fLastGlyphID = face->num_glyphs - 1;
    info->fEmSize = 1000;

    bool cid = false;
    const char* fontType = FT_Get_X11_Font_Format(face);
    if (strcmp(fontType, "Type 1") == 0) {
        info->fType = SkAdvancedTypefaceMetrics::kType1_Font;
    } else if (strcmp(fontType, "CID Type 1") == 0) {
        info->fType = SkAdvancedTypefaceMetrics::kType1CID_Font;
        cid = true;
    } else if (strcmp(fontType, "CFF") == 0) {
        info->fType = SkAdvancedTypefaceMetrics::kCFF_Font;
    } else if (strcmp(fontType, "TrueType") == 0) {
        info->fType = SkAdvancedTypefaceMetrics::kTrueType_Font;
        cid = true;
        TT_Header* ttHeader;
        if ((ttHeader = (TT_Header*)FT_Get_Sfnt_Table(face,
                                                      ft_sfnt_head)) != nullptr) {
            info->fEmSize = ttHeader->Units_Per_EM;
        }
    } else {
        info->fType = SkAdvancedTypefaceMetrics::kOther_Font;
    }

    info->fStyle = 0;
    if (FT_IS_FIXED_WIDTH(face))
        info->fStyle |= SkAdvancedTypefaceMetrics::kFixedPitch_Style;
    if (face->style_flags & FT_STYLE_FLAG_ITALIC)
        info->fStyle |= SkAdvancedTypefaceMetrics::kItalic_Style;

    PS_FontInfoRec ps_info;
    TT_Postscript* tt_info;
    if (FT_Get_PS_Font_Info(face, &ps_info) == 0) {
        info->fItalicAngle = ps_info.italic_angle;
    } else if ((tt_info =
                (TT_Postscript*)FT_Get_Sfnt_Table(face,
                                                  ft_sfnt_post)) != nullptr) {
        info->fItalicAngle = SkFixedToScalar(tt_info->italicAngle);
    } else {
        info->fItalicAngle = 0;
    }

    info->fAscent = face->ascender;
    info->fDescent = face->descender;

    // Figure out a good guess for StemV - Min width of i, I, !, 1.
    // This probably isn't very good with an italic font.
    int16_t min_width = SHRT_MAX;
    info->fStemV = 0;
    char stem_chars[] = {'i', 'I', '!', '1'};
    for (size_t i = 0; i < SK_ARRAY_COUNT(stem_chars); i++) {
        FT_BBox bbox;
        if (GetLetterCBox(face, stem_chars[i], &bbox)) {
            int16_t width = bbox.xMax - bbox.xMin;
            if (width > 0 && width < min_width) {
                min_width = width;
                info->fStemV = min_width;
            }
        }
    }

    TT_PCLT* pclt_info;
    TT_OS2* os2_table;
    if ((pclt_info = (TT_PCLT*)FT_Get_Sfnt_Table(face, ft_sfnt_pclt)) != nullptr) {
        info->fCapHeight = pclt_info->CapHeight;
        uint8_t serif_style = pclt_info->SerifStyle & 0x3F;
        if (serif_style >= 2 && serif_style <= 6)
            info->fStyle |= SkAdvancedTypefaceMetrics::kSerif_Style;
        else if (serif_style >= 9 && serif_style <= 12)
            info->fStyle |= SkAdvancedTypefaceMetrics::kScript_Style;
    } else if (((os2_table = (TT_OS2*)FT_Get_Sfnt_Table(face, ft_sfnt_os2)) != nullptr) &&
               // sCapHeight is available only when version 2 or later.
               os2_table->version != 0xFFFF &&
               os2_table->version >= 2) {
        info->fCapHeight = os2_table->sCapHeight;
    } else {
        // Figure out a good guess for CapHeight: average the height of M and X.
        FT_BBox m_bbox, x_bbox;
        bool got_m, got_x;
        got_m = GetLetterCBox(face, 'M', &m_bbox);
        got_x = GetLetterCBox(face, 'X', &x_bbox);
        if (got_m && got_x) {
            info->fCapHeight = (m_bbox.yMax - m_bbox.yMin + x_bbox.yMax -
                    x_bbox.yMin) / 2;
        } else if (got_m && !got_x) {
            info->fCapHeight = m_bbox.yMax - m_bbox.yMin;
        } else if (!got_m && got_x) {
            info->fCapHeight = x_bbox.yMax - x_bbox.yMin;
        } else {
            // Last resort, use the ascent.
            info->fCapHeight = info->fAscent;
        }
    }

    info->fBBox = SkIRect::MakeLTRB(face->bbox.xMin, face->bbox.yMax,
                                    face->bbox.xMax, face->bbox.yMin);

    if (!FT_IS_SCALABLE(face)) {
        perGlyphInfo = kNo_PerGlyphInfo;
    }

    if (perGlyphInfo & kHAdvance_PerGlyphInfo) {
        if (FT_IS_FIXED_WIDTH(face)) {
            appendRange(&info->fGlyphWidths, 0);
            int16_t advance = face->max_advance_width;
            info->fGlyphWidths->fAdvance.append(1, &advance);
            finishRange(info->fGlyphWidths.get(), 0,
                        SkAdvancedTypefaceMetrics::WidthRange::kDefault);
        } else if (!cid) {
            appendRange(&info->fGlyphWidths, 0);
            // So as to not blow out the stack, get advances in batches.
            for (int gID = 0; gID < face->num_glyphs; gID += 128) {
                FT_Fixed advances[128];
                int advanceCount = 128;
                if (gID + advanceCount > face->num_glyphs) {
                    advanceCount = face->num_glyphs - gID;
                }
                FT_Get_Advances(face, gID, advanceCount, FT_LOAD_NO_SCALE, advances);
                for (int i = 0; i < advanceCount; i++) {
                    int16_t advance = advances[i];
                    info->fGlyphWidths->fAdvance.append(1, &advance);
                }
            }
            finishRange(info->fGlyphWidths.get(), face->num_glyphs - 1,
                        SkAdvancedTypefaceMetrics::WidthRange::kRange);
        } else {
            info->fGlyphWidths.reset(
                getAdvanceData(face,
                               face->num_glyphs,
                               glyphIDs,
                               glyphIDsCount,
                               &getWidthAdvance));
        }
    }

    if (perGlyphInfo & kVAdvance_PerGlyphInfo &&
            FT_HAS_VERTICAL(face)) {
        SkASSERT(false);  // Not implemented yet.
    }

    if (perGlyphInfo & kGlyphNames_PerGlyphInfo &&
            info->fType == SkAdvancedTypefaceMetrics::kType1_Font) {
        // Postscript fonts may contain more than 255 glyphs, so we end up
        // using multiple font descriptions with a glyph ordering.  Record
        // the name of each glyph.
        info->fGlyphNames.reset(
                new SkAutoTArray<SkString>(face->num_glyphs));
        for (int gID = 0; gID < face->num_glyphs; gID++) {
            char glyphName[128];  // PS limit for names is 127 bytes.
            FT_Get_Glyph_Name(face, gID, glyphName, 128);
            info->fGlyphNames->get()[gID].set(glyphName);
        }
    }

    if (perGlyphInfo & kToUnicode_PerGlyphInfo &&
           info->fType != SkAdvancedTypefaceMetrics::kType1_Font &&
           face->num_charmaps) {
        populate_glyph_to_unicode(face, &(info->fGlyphToUnicode));
    }

    return info;
#endif
}

///////////////////////////////////////////////////////////////////////////

static bool bothZero(SkScalar a, SkScalar b) {
    return 0 == a && 0 == b;
}

// returns false if there is any non-90-rotation or skew
static bool isAxisAligned(const SkScalerContext::Rec& rec) {
    return 0 == rec.fPreSkewX &&
           (bothZero(rec.fPost2x2[0][1], rec.fPost2x2[1][0]) ||
            bothZero(rec.fPost2x2[0][0], rec.fPost2x2[1][1]));
}

SkScalerContext* SkTypeface_FreeType::onCreateScalerContext(
                                               const SkDescriptor* desc) const {
    SkScalerContext_FreeType* c =
            new SkScalerContext_FreeType(const_cast<SkTypeface_FreeType*>(this), desc);
    if (!c->success()) {
        delete c;
        c = nullptr;
    }
    return c;
}

void SkTypeface_FreeType::onFilterRec(SkScalerContextRec* rec) const {
    //BOGUS: http://code.google.com/p/chromium/issues/detail?id=121119
    //Cap the requested size as larger sizes give bogus values.
    //Remove when http://code.google.com/p/skia/issues/detail?id=554 is fixed.
    if (rec->fTextSize > SkIntToScalar(1 << 14)) {
        rec->fTextSize = SkIntToScalar(1 << 14);
    }

    if (isLCD(*rec)) {
        // TODO: re-work so that FreeType is set-up and selected by the SkFontMgr.
        SkAutoMutexAcquire ama(gFTMutex);
        ref_ft_library();
        if (!gFTLibrary->isLCDSupported()) {
            // If the runtime Freetype library doesn't support LCD, disable it here.
            rec->fMaskFormat = SkMask::kA8_Format;
        }
        unref_ft_library();
    }

    SkPaint::Hinting h = rec->getHinting();
    if (SkPaint::kFull_Hinting == h && !isLCD(*rec)) {
        // collapse full->normal hinting if we're not doing LCD
        h = SkPaint::kNormal_Hinting;
    }
    if ((rec->fFlags & SkScalerContext::kSubpixelPositioning_Flag)) {
        if (SkPaint::kNo_Hinting != h) {
            h = SkPaint::kSlight_Hinting;
        }
    }

    // rotated text looks bad with hinting, so we disable it as needed
    if (!isAxisAligned(*rec)) {
        h = SkPaint::kNo_Hinting;
    }
    rec->setHinting(h);

#ifndef SK_GAMMA_APPLY_TO_A8
    if (!isLCD(*rec)) {
      rec->ignorePreBlend();
    }
#endif
}

int SkTypeface_FreeType::onGetUPEM() const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    return face ? face->units_per_EM : 0;
}

bool SkTypeface_FreeType::onGetKerningPairAdjustments(const uint16_t glyphs[],
                                      int count, int32_t adjustments[]) const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face || !FT_HAS_KERNING(face)) {
        return false;
    }

    for (int i = 0; i < count - 1; ++i) {
        FT_Vector delta;
        FT_Error err = FT_Get_Kerning(face, glyphs[i], glyphs[i+1],
                                      FT_KERNING_UNSCALED, &delta);
        if (err) {
            return false;
        }
        adjustments[i] = delta.x;
    }
    return true;
}

static FT_Int chooseBitmapStrike(FT_Face face, SkFixed scaleY) {
    // early out if face is bad
    if (face == nullptr) {
        SkDEBUGF(("chooseBitmapStrike aborted due to nullptr face\n"));
        return -1;
    }
    // determine target ppem
    FT_Pos targetPPEM = SkFixedToFDot6(scaleY);
    // find a bitmap strike equal to or just larger than the requested size
    FT_Int chosenStrikeIndex = -1;
    FT_Pos chosenPPEM = 0;
    for (FT_Int strikeIndex = 0; strikeIndex < face->num_fixed_sizes; ++strikeIndex) {
        FT_Pos thisPPEM = face->available_sizes[strikeIndex].y_ppem;
        if (thisPPEM == targetPPEM) {
            // exact match - our search stops here
            chosenPPEM = thisPPEM;
            chosenStrikeIndex = strikeIndex;
            break;
        } else if (chosenPPEM < targetPPEM) {
            // attempt to increase chosenPPEM
            if (thisPPEM > chosenPPEM) {
                chosenPPEM = thisPPEM;
                chosenStrikeIndex = strikeIndex;
            }
        } else {
            // attempt to decrease chosenPPEM, but not below targetPPEM
            if (thisPPEM < chosenPPEM && thisPPEM > targetPPEM) {
                chosenPPEM = thisPPEM;
                chosenStrikeIndex = strikeIndex;
            }
        }
    }
    if (chosenStrikeIndex != -1) {
        // use the chosen strike
        FT_Error err = FT_Select_Size(face, chosenStrikeIndex);
        if (err != 0) {
            SkDEBUGF(("FT_Select_Size(%s, %d) returned 0x%x\n", face->family_name,
                      chosenStrikeIndex, err));
            chosenStrikeIndex = -1;
        }
    }
    return chosenStrikeIndex;
}

SkScalerContext_FreeType::SkScalerContext_FreeType(SkTypeface* typeface, const SkDescriptor* desc)
    : SkScalerContext_FreeType_Base(typeface, desc)
{
    SkAutoMutexAcquire  ac(gFTMutex);

    if (!ref_ft_library()) {
        sk_throw();
    }

    // load the font file
    fStrikeIndex = -1;
    fFTSize = nullptr;
    fFace = ref_ft_face(typeface);
    if (nullptr == fFace) {
        return;
    }

    fRec.computeMatrices(SkScalerContextRec::kFull_PreMatrixScale, &fScale, &fMatrix22Scalar);
    fMatrix22Scalar.setSkewX(-fMatrix22Scalar.getSkewX());
    fMatrix22Scalar.setSkewY(-fMatrix22Scalar.getSkewY());

    fScaleX = SkScalarToFixed(fScale.fX);
    fScaleY = SkScalarToFixed(fScale.fY);
    fMatrix22.xx = SkScalarToFixed(fMatrix22Scalar.getScaleX());
    fMatrix22.xy = SkScalarToFixed(fMatrix22Scalar.getSkewX());
    fMatrix22.yx = SkScalarToFixed(fMatrix22Scalar.getSkewY());
    fMatrix22.yy = SkScalarToFixed(fMatrix22Scalar.getScaleY());

    fLCDIsVert = SkToBool(fRec.fFlags & SkScalerContext::kLCD_Vertical_Flag);

    // compute the flags we send to Load_Glyph
    bool linearMetrics = SkToBool(fRec.fFlags & SkScalerContext::kSubpixelPositioning_Flag);
    {
        FT_Int32 loadFlags = FT_LOAD_DEFAULT;

        if (SkMask::kBW_Format == fRec.fMaskFormat) {
            // See http://code.google.com/p/chromium/issues/detail?id=43252#c24
            loadFlags = FT_LOAD_TARGET_MONO;
            if (fRec.getHinting() == SkPaint::kNo_Hinting) {
                loadFlags = FT_LOAD_NO_HINTING;
                linearMetrics = true;
            }
        } else {
            switch (fRec.getHinting()) {
            case SkPaint::kNo_Hinting:
                loadFlags = FT_LOAD_NO_HINTING;
                linearMetrics = true;
                break;
            case SkPaint::kSlight_Hinting:
                loadFlags = FT_LOAD_TARGET_LIGHT;  // This implies FORCE_AUTOHINT
                break;
            case SkPaint::kNormal_Hinting:
                if (fRec.fFlags & SkScalerContext::kForceAutohinting_Flag) {
                    loadFlags = FT_LOAD_FORCE_AUTOHINT;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
                } else {
                    loadFlags = FT_LOAD_NO_AUTOHINT;
#endif
                }
                break;
            case SkPaint::kFull_Hinting:
                if (fRec.fFlags & SkScalerContext::kForceAutohinting_Flag) {
                    loadFlags = FT_LOAD_FORCE_AUTOHINT;
                    break;
                }
                loadFlags = FT_LOAD_TARGET_NORMAL;
                if (isLCD(fRec)) {
                    if (fLCDIsVert) {
                        loadFlags = FT_LOAD_TARGET_LCD_V;
                    } else {
                        loadFlags = FT_LOAD_TARGET_LCD;
                    }
                }
                break;
            default:
                SkDebugf("---------- UNKNOWN hinting %d\n", fRec.getHinting());
                break;
            }
        }

        if ((fRec.fFlags & SkScalerContext::kEmbeddedBitmapText_Flag) == 0) {
            loadFlags |= FT_LOAD_NO_BITMAP;
        }

        // Always using FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH to get correct
        // advances, as fontconfig and cairo do.
        // See http://code.google.com/p/skia/issues/detail?id=222.
        loadFlags |= FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH;

        // Use vertical layout if requested.
        if (fRec.fFlags & SkScalerContext::kVertical_Flag) {
            loadFlags |= FT_LOAD_VERTICAL_LAYOUT;
        }

        loadFlags |= FT_LOAD_COLOR;

        fLoadGlyphFlags = loadFlags;
    }

    FT_Error err = FT_New_Size(fFace, &fFTSize);
    if (err != 0) {
        SkDEBUGF(("FT_New_Size returned %x for face %s\n", err, fFace->family_name));
        fFace = nullptr;
        return;
    }
    err = FT_Activate_Size(fFTSize);
    if (err != 0) {
        SkDEBUGF(("FT_Activate_Size(%08x, 0x%x, 0x%x) returned 0x%x\n", fFace, fScaleX, fScaleY,
                  err));
        fFTSize = nullptr;
        return;
    }

    if (FT_IS_SCALABLE(fFace)) {
        err = FT_Set_Char_Size(fFace, SkFixedToFDot6(fScaleX), SkFixedToFDot6(fScaleY), 72, 72);
        if (err != 0) {
            SkDEBUGF(("FT_Set_CharSize(%08x, 0x%x, 0x%x) returned 0x%x\n",
                                    fFace, fScaleX, fScaleY,      err));
            fFace = nullptr;
            return;
        }
        FT_Set_Transform(fFace, &fMatrix22, nullptr);
    } else if (FT_HAS_FIXED_SIZES(fFace)) {
        fStrikeIndex = chooseBitmapStrike(fFace, fScaleY);
        if (fStrikeIndex == -1) {
            SkDEBUGF(("no glyphs for font \"%s\" size %f?\n",
                            fFace->family_name,       SkFixedToScalar(fScaleY)));
        } else {
            // FreeType does no provide linear metrics for bitmap fonts.
            linearMetrics = false;

            // FreeType documentation says:
            // FT_LOAD_NO_BITMAP -- Ignore bitmap strikes when loading.
            // Bitmap-only fonts ignore this flag.
            //
            // However, in FreeType 2.5.1 color bitmap only fonts do not ignore this flag.
            // Force this flag off for bitmap only fonts.
            fLoadGlyphFlags &= ~FT_LOAD_NO_BITMAP;
        }
    } else {
        SkDEBUGF(("unknown kind of font \"%s\" size %f?\n",
                            fFace->family_name,       SkFixedToScalar(fScaleY)));
    }

    fDoLinearMetrics = linearMetrics;
}

SkScalerContext_FreeType::~SkScalerContext_FreeType() {
    SkAutoMutexAcquire  ac(gFTMutex);

    if (fFTSize != nullptr) {
        FT_Done_Size(fFTSize);
    }

    if (fFace != nullptr) {
        unref_ft_face(fFace);
    }

    unref_ft_library();
}

/*  We call this before each use of the fFace, since we may be sharing
    this face with other context (at different sizes).
*/
FT_Error SkScalerContext_FreeType::setupSize() {
    gFTMutex.assertHeld();
    FT_Error err = FT_Activate_Size(fFTSize);
    if (err != 0) {
        SkDEBUGF(("SkScalerContext_FreeType::FT_Activate_Size(%s %s, 0x%x, 0x%x) returned 0x%x\n",
                  fFace->family_name, fFace->style_name, fScaleX, fScaleY, err));
        fFTSize = nullptr;
        return err;
    }

    // seems we need to reset this every time (not sure why, but without it
    // I get random italics from some other fFTSize)
    FT_Set_Transform(fFace, &fMatrix22, nullptr);
    return 0;
}

unsigned SkScalerContext_FreeType::generateGlyphCount() {
    return fFace->num_glyphs;
}

uint16_t SkScalerContext_FreeType::generateCharToGlyph(SkUnichar uni) {
    SkAutoMutexAcquire  ac(gFTMutex);
    return SkToU16(FT_Get_Char_Index( fFace, uni ));
}

SkUnichar SkScalerContext_FreeType::generateGlyphToChar(uint16_t glyph) {
    SkAutoMutexAcquire  ac(gFTMutex);
    // iterate through each cmap entry, looking for matching glyph indices
    FT_UInt glyphIndex;
    SkUnichar charCode = FT_Get_First_Char( fFace, &glyphIndex );

    while (glyphIndex != 0) {
        if (glyphIndex == glyph) {
            return charCode;
        }
        charCode = FT_Get_Next_Char( fFace, charCode, &glyphIndex );
    }

    return 0;
}

void SkScalerContext_FreeType::generateAdvance(SkGlyph* glyph) {
   /* unhinted and light hinted text have linearly scaled advances
    * which are very cheap to compute with some font formats...
    */
    if (fDoLinearMetrics) {
        SkAutoMutexAcquire  ac(gFTMutex);

        if (this->setupSize()) {
            glyph->zeroMetrics();
            return;
        }

        FT_Error    error;
        FT_Fixed    advance;

        error = FT_Get_Advance( fFace, glyph->getGlyphID(),
                                fLoadGlyphFlags | FT_ADVANCE_FLAG_FAST_ONLY,
                                &advance );
        if (0 == error) {
            glyph->fRsbDelta = 0;
            glyph->fLsbDelta = 0;
            glyph->fAdvanceX = SkFixedMul(fMatrix22.xx, advance);
            glyph->fAdvanceY = - SkFixedMul(fMatrix22.yx, advance);
            return;
        }
    }

    /* otherwise, we need to load/hint the glyph, which is slower */
    this->generateMetrics(glyph);
    return;
}

void SkScalerContext_FreeType::getBBoxForCurrentGlyph(SkGlyph* glyph,
                                                      FT_BBox* bbox,
                                                      bool snapToPixelBoundary) {

    FT_Outline_Get_CBox(&fFace->glyph->outline, bbox);

    if (fRec.fFlags & SkScalerContext::kSubpixelPositioning_Flag) {
        int dx = SkFixedToFDot6(glyph->getSubXFixed());
        int dy = SkFixedToFDot6(glyph->getSubYFixed());
        // negate dy since freetype-y-goes-up and skia-y-goes-down
        bbox->xMin += dx;
        bbox->yMin -= dy;
        bbox->xMax += dx;
        bbox->yMax -= dy;
    }

    // outset the box to integral boundaries
    if (snapToPixelBoundary) {
        bbox->xMin &= ~63;
        bbox->yMin &= ~63;
        bbox->xMax  = (bbox->xMax + 63) & ~63;
        bbox->yMax  = (bbox->yMax + 63) & ~63;
    }

    // Must come after snapToPixelBoundary so that the width and height are
    // consistent. Otherwise asserts will fire later on when generating the
    // glyph image.
    if (fRec.fFlags & SkScalerContext::kVertical_Flag) {
        FT_Vector vector;
        vector.x = fFace->glyph->metrics.vertBearingX - fFace->glyph->metrics.horiBearingX;
        vector.y = -fFace->glyph->metrics.vertBearingY - fFace->glyph->metrics.horiBearingY;
        FT_Vector_Transform(&vector, &fMatrix22);
        bbox->xMin += vector.x;
        bbox->xMax += vector.x;
        bbox->yMin += vector.y;
        bbox->yMax += vector.y;
    }
}

bool SkScalerContext_FreeType::getCBoxForLetter(char letter, FT_BBox* bbox) {
    const FT_UInt glyph_id = FT_Get_Char_Index(fFace, letter);
    if (!glyph_id) {
        return false;
    }
    if (FT_Load_Glyph(fFace, glyph_id, fLoadGlyphFlags) != 0) {
        return false;
    }
    emboldenIfNeeded(fFace, fFace->glyph);
    FT_Outline_Get_CBox(&fFace->glyph->outline, bbox);
    return true;
}

void SkScalerContext_FreeType::updateGlyphIfLCD(SkGlyph* glyph) {
    if (isLCD(fRec)) {
        if (fLCDIsVert) {
            glyph->fHeight += gFTLibrary->lcdExtra();
            glyph->fTop -= gFTLibrary->lcdExtra() >> 1;
        } else {
            glyph->fWidth += gFTLibrary->lcdExtra();
            glyph->fLeft -= gFTLibrary->lcdExtra() >> 1;
        }
    }
}

inline void scaleGlyphMetrics(SkGlyph& glyph, SkScalar scale) {
    glyph.fWidth *= scale;
    glyph.fHeight *= scale;
    glyph.fTop *= scale;
    glyph.fLeft *= scale;

    SkFixed fixedScale = SkScalarToFixed(scale);
    glyph.fAdvanceX = SkFixedMul(glyph.fAdvanceX, fixedScale);
    glyph.fAdvanceY = SkFixedMul(glyph.fAdvanceY, fixedScale);
}

void SkScalerContext_FreeType::generateMetrics(SkGlyph* glyph) {
    SkAutoMutexAcquire  ac(gFTMutex);

    glyph->fRsbDelta = 0;
    glyph->fLsbDelta = 0;

    FT_Error    err;

    if (this->setupSize()) {
        glyph->zeroMetrics();
        return;
    }

    err = FT_Load_Glyph( fFace, glyph->getGlyphID(), fLoadGlyphFlags );
    if (err != 0) {
        glyph->zeroMetrics();
        return;
    }
    emboldenIfNeeded(fFace, fFace->glyph);

    switch ( fFace->glyph->format ) {
      case FT_GLYPH_FORMAT_OUTLINE:
        if (0 == fFace->glyph->outline.n_contours) {
            glyph->fWidth = 0;
            glyph->fHeight = 0;
            glyph->fTop = 0;
            glyph->fLeft = 0;
        } else {
            FT_BBox bbox;
            getBBoxForCurrentGlyph(glyph, &bbox, true);

            glyph->fWidth   = SkToU16(SkFDot6Floor(bbox.xMax - bbox.xMin));
            glyph->fHeight  = SkToU16(SkFDot6Floor(bbox.yMax - bbox.yMin));
            glyph->fTop     = -SkToS16(SkFDot6Floor(bbox.yMax));
            glyph->fLeft    = SkToS16(SkFDot6Floor(bbox.xMin));

            updateGlyphIfLCD(glyph);
        }
        break;

      case FT_GLYPH_FORMAT_BITMAP:
        if (fRec.fFlags & SkScalerContext::kVertical_Flag) {
            FT_Vector vector;
            vector.x = fFace->glyph->metrics.vertBearingX - fFace->glyph->metrics.horiBearingX;
            vector.y = -fFace->glyph->metrics.vertBearingY - fFace->glyph->metrics.horiBearingY;
            FT_Vector_Transform(&vector, &fMatrix22);
            fFace->glyph->bitmap_left += SkFDot6Floor(vector.x);
            fFace->glyph->bitmap_top  += SkFDot6Floor(vector.y);
        }

        if (fFace->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA) {
            glyph->fMaskFormat = SkMask::kARGB32_Format;
        }

        glyph->fWidth   = SkToU16(fFace->glyph->bitmap.width);
        glyph->fHeight  = SkToU16(fFace->glyph->bitmap.rows);
        glyph->fTop     = -SkToS16(fFace->glyph->bitmap_top);
        glyph->fLeft    = SkToS16(fFace->glyph->bitmap_left);
        break;

      default:
        SkDEBUGFAIL("unknown glyph format");
        glyph->zeroMetrics();
        return;
    }

    if (fRec.fFlags & SkScalerContext::kVertical_Flag) {
        if (fDoLinearMetrics) {
            glyph->fAdvanceX = -SkFixedMul(fMatrix22.xy, fFace->glyph->linearVertAdvance);
            glyph->fAdvanceY = SkFixedMul(fMatrix22.yy, fFace->glyph->linearVertAdvance);
        } else {
            glyph->fAdvanceX = -SkFDot6ToFixed(fFace->glyph->advance.x);
            glyph->fAdvanceY = SkFDot6ToFixed(fFace->glyph->advance.y);
        }
    } else {
        if (fDoLinearMetrics) {
            glyph->fAdvanceX = SkFixedMul(fMatrix22.xx, fFace->glyph->linearHoriAdvance);
            glyph->fAdvanceY = -SkFixedMul(fMatrix22.yx, fFace->glyph->linearHoriAdvance);
        } else {
            glyph->fAdvanceX = SkFDot6ToFixed(fFace->glyph->advance.x);
            glyph->fAdvanceY = -SkFDot6ToFixed(fFace->glyph->advance.y);

            if (fRec.fFlags & kDevKernText_Flag) {
                glyph->fRsbDelta = SkToS8(fFace->glyph->rsb_delta);
                glyph->fLsbDelta = SkToS8(fFace->glyph->lsb_delta);
            }
        }
    }

    // If the font isn't scalable, scale the metrics from the non-scalable strike.
    // This means do not try to scale embedded bitmaps; only scale bitmaps in bitmap only fonts.
    if (!FT_IS_SCALABLE(fFace) && fScaleY && fFace->size->metrics.y_ppem) {
        // NOTE: both dimensions are scaled by y_ppem. this is WAI.
        scaleGlyphMetrics(*glyph, SkFixedToScalar(fScaleY) / fFace->size->metrics.y_ppem);
    }

#ifdef ENABLE_GLYPH_SPEW
    SkDEBUGF(("FT_Set_Char_Size(this:%p sx:%x sy:%x ", this, fScaleX, fScaleY));
    SkDEBUGF(("Metrics(glyph:%d flags:0x%x) w:%d\n", glyph->getGlyphID(), fLoadGlyphFlags, glyph->fWidth));
#endif
}

static void clear_glyph_image(const SkGlyph& glyph) {
    sk_bzero(glyph.fImage, glyph.rowBytes() * glyph.fHeight);
}

void SkScalerContext_FreeType::generateImage(const SkGlyph& glyph) {
    SkAutoMutexAcquire  ac(gFTMutex);

    if (this->setupSize()) {
        clear_glyph_image(glyph);
        return;
    }

    FT_Error err = FT_Load_Glyph(fFace, glyph.getGlyphID(), fLoadGlyphFlags);
    if (err != 0) {
        SkDEBUGF(("SkScalerContext_FreeType::generateImage: FT_Load_Glyph(glyph:%d width:%d height:%d rb:%d flags:%d) returned 0x%x\n",
                  glyph.getGlyphID(), glyph.fWidth, glyph.fHeight, glyph.rowBytes(), fLoadGlyphFlags, err));
        clear_glyph_image(glyph);
        return;
    }

    emboldenIfNeeded(fFace, fFace->glyph);
    generateGlyphImage(fFace, glyph);
}


void SkScalerContext_FreeType::generatePath(const SkGlyph& glyph, SkPath* path) {
    SkAutoMutexAcquire  ac(gFTMutex);

    SkASSERT(path);

    if (this->setupSize()) {
        path->reset();
        return;
    }

    uint32_t flags = fLoadGlyphFlags;
    flags |= FT_LOAD_NO_BITMAP; // ignore embedded bitmaps so we're sure to get the outline
    flags &= ~FT_LOAD_RENDER;   // don't scan convert (we just want the outline)

    FT_Error err = FT_Load_Glyph( fFace, glyph.getGlyphID(), flags);

    if (err != 0) {
        SkDEBUGF(("SkScalerContext_FreeType::generatePath: FT_Load_Glyph(glyph:%d flags:%d) returned 0x%x\n",
                    glyph.getGlyphID(), flags, err));
        path->reset();
        return;
    }
    emboldenIfNeeded(fFace, fFace->glyph);

    generateGlyphPath(fFace, path);

    // The path's origin from FreeType is always the horizontal layout origin.
    // Offset the path so that it is relative to the vertical origin if needed.
    if (fRec.fFlags & SkScalerContext::kVertical_Flag) {
        FT_Vector vector;
        vector.x = fFace->glyph->metrics.vertBearingX - fFace->glyph->metrics.horiBearingX;
        vector.y = -fFace->glyph->metrics.vertBearingY - fFace->glyph->metrics.horiBearingY;
        FT_Vector_Transform(&vector, &fMatrix22);
        path->offset(SkFDot6ToScalar(vector.x), -SkFDot6ToScalar(vector.y));
    }
}

void SkScalerContext_FreeType::generateFontMetrics(SkPaint::FontMetrics* metrics) {
    if (nullptr == metrics) {
        return;
    }

    SkAutoMutexAcquire ac(gFTMutex);

    if (this->setupSize()) {
        ERROR:
        sk_bzero(metrics, sizeof(*metrics));
        return;
    }

    FT_Face face = fFace;
    SkScalar scaleX = fScale.x();
    SkScalar scaleY = fScale.y();
    SkScalar mxy = fMatrix22Scalar.getSkewX() * scaleY;
    SkScalar myy = fMatrix22Scalar.getScaleY() * scaleY;

    // fetch units/EM from "head" table if needed (ie for bitmap fonts)
    SkScalar upem = SkIntToScalar(face->units_per_EM);
    if (!upem) {
        TT_Header* ttHeader = (TT_Header*)FT_Get_Sfnt_Table(face, ft_sfnt_head);
        if (ttHeader) {
            upem = SkIntToScalar(ttHeader->Units_Per_EM);
        }
    }

    // use the os/2 table as a source of reasonable defaults.
    SkScalar x_height = 0.0f;
    SkScalar avgCharWidth = 0.0f;
    SkScalar cap_height = 0.0f;
    TT_OS2* os2 = (TT_OS2*) FT_Get_Sfnt_Table(face, ft_sfnt_os2);
    if (os2) {
        x_height = scaleX * SkIntToScalar(os2->sxHeight) / upem;
        avgCharWidth = SkIntToScalar(os2->xAvgCharWidth) / upem;
        if (os2->version != 0xFFFF && os2->version >= 2) {
            cap_height = scaleX * SkIntToScalar(os2->sCapHeight) / upem;
        }
    }

    // pull from format-specific metrics as needed
    SkScalar ascent, descent, leading, xmin, xmax, ymin, ymax;
    SkScalar underlineThickness, underlinePosition;
    if (face->face_flags & FT_FACE_FLAG_SCALABLE) { // scalable outline font
        // FreeType will always use HHEA metrics if they're not zero.
        // It completely ignores the OS/2 fsSelection::UseTypoMetrics bit.
        // It also ignores the VDMX tables, which are also of interest here
        // (and override everything else when they apply).
        static const int kUseTypoMetricsMask = (1 << 7);
        if (os2 && os2->version != 0xFFFF && (os2->fsSelection & kUseTypoMetricsMask)) {
            ascent = -SkIntToScalar(os2->sTypoAscender) / upem;
            descent = -SkIntToScalar(os2->sTypoDescender) / upem;
            leading = SkIntToScalar(os2->sTypoLineGap) / upem;
        } else {
            ascent = -SkIntToScalar(face->ascender) / upem;
            descent = -SkIntToScalar(face->descender) / upem;
            leading = SkIntToScalar(face->height + (face->descender - face->ascender)) / upem;
        }
        xmin = SkIntToScalar(face->bbox.xMin) / upem;
        xmax = SkIntToScalar(face->bbox.xMax) / upem;
        ymin = -SkIntToScalar(face->bbox.yMin) / upem;
        ymax = -SkIntToScalar(face->bbox.yMax) / upem;
        underlineThickness = SkIntToScalar(face->underline_thickness) / upem;
        underlinePosition = -SkIntToScalar(face->underline_position +
                                           face->underline_thickness / 2) / upem;

        metrics->fFlags |= SkPaint::FontMetrics::kUnderlineThinknessIsValid_Flag;
        metrics->fFlags |= SkPaint::FontMetrics::kUnderlinePositionIsValid_Flag;

        // we may be able to synthesize x_height and cap_height from outline
        if (!x_height) {
            FT_BBox bbox;
            if (getCBoxForLetter('x', &bbox)) {
                x_height = SkIntToScalar(bbox.yMax) / 64.0f;
            }
        }
        if (!cap_height) {
            FT_BBox bbox;
            if (getCBoxForLetter('H', &bbox)) {
                cap_height = SkIntToScalar(bbox.yMax) / 64.0f;
            }
        }
    } else if (fStrikeIndex != -1) { // bitmap strike metrics
        SkScalar xppem = SkIntToScalar(face->size->metrics.x_ppem);
        SkScalar yppem = SkIntToScalar(face->size->metrics.y_ppem);
        ascent = -SkIntToScalar(face->size->metrics.ascender) / (yppem * 64.0f);
        descent = -SkIntToScalar(face->size->metrics.descender) / (yppem * 64.0f);
        leading = (SkIntToScalar(face->size->metrics.height) / (yppem * 64.0f))
                + ascent - descent;
        xmin = 0.0f;
        xmax = SkIntToScalar(face->available_sizes[fStrikeIndex].width) / xppem;
        ymin = descent + leading;
        ymax = ascent - descent;
        underlineThickness = 0;
        underlinePosition = 0;

        metrics->fFlags &= ~SkPaint::FontMetrics::kUnderlineThinknessIsValid_Flag;
        metrics->fFlags &= ~SkPaint::FontMetrics::kUnderlinePositionIsValid_Flag;
    } else {
        goto ERROR;
    }

    // synthesize elements that were not provided by the os/2 table or format-specific metrics
    if (!x_height) {
        x_height = -ascent;
    }
    if (!avgCharWidth) {
        avgCharWidth = xmax - xmin;
    }
    if (!cap_height) {
      cap_height = -ascent;
    }

    // disallow negative linespacing
    if (leading < 0.0f) {
        leading = 0.0f;
    }

    SkScalar scale = myy;
    if (this->isVertical()) {
        scale = mxy;
    }
    metrics->fTop = ymax * scale;
    metrics->fAscent = ascent * scale;
    metrics->fDescent = descent * scale;
    metrics->fBottom = ymin * scale;
    metrics->fLeading = leading * scale;
    metrics->fAvgCharWidth = avgCharWidth * scale;
    metrics->fXMin = xmin * scale;
    metrics->fXMax = xmax * scale;
    metrics->fXHeight = x_height;
    metrics->fCapHeight = cap_height;
    metrics->fUnderlineThickness = underlineThickness * scale;
    metrics->fUnderlinePosition = underlinePosition * scale;
}

///////////////////////////////////////////////////////////////////////////////

// hand-tuned value to reduce outline embolden strength
#ifndef SK_OUTLINE_EMBOLDEN_DIVISOR
    #ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        #define SK_OUTLINE_EMBOLDEN_DIVISOR   34
    #else
        #define SK_OUTLINE_EMBOLDEN_DIVISOR   24
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////

void SkScalerContext_FreeType::emboldenIfNeeded(FT_Face face, FT_GlyphSlot glyph)
{
    // check to see if the embolden bit is set
    if (0 == (fRec.fFlags & SkScalerContext::kEmbolden_Flag)) {
        return;
    }

    switch (glyph->format) {
        case FT_GLYPH_FORMAT_OUTLINE:
            FT_Pos strength;
            strength = FT_MulFix(face->units_per_EM, face->size->metrics.y_scale)
                       / SK_OUTLINE_EMBOLDEN_DIVISOR;
            FT_Outline_Embolden(&glyph->outline, strength);
            break;
        case FT_GLYPH_FORMAT_BITMAP:
            FT_GlyphSlot_Own_Bitmap(glyph);
            FT_Bitmap_Embolden(glyph->library, &glyph->bitmap, kBitmapEmboldenStrength, 0);
            break;
        default:
            SkDEBUGFAIL("unknown glyph format");
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkUtils.h"

static SkUnichar next_utf8(const void** chars) {
    return SkUTF8_NextUnichar((const char**)chars);
}

static SkUnichar next_utf16(const void** chars) {
    return SkUTF16_NextUnichar((const uint16_t**)chars);
}

static SkUnichar next_utf32(const void** chars) {
    const SkUnichar** uniChars = (const SkUnichar**)chars;
    SkUnichar uni = **uniChars;
    *uniChars += 1;
    return uni;
}

typedef SkUnichar (*EncodingProc)(const void**);

static EncodingProc find_encoding_proc(SkTypeface::Encoding enc) {
    static const EncodingProc gProcs[] = {
        next_utf8, next_utf16, next_utf32
    };
    SkASSERT((size_t)enc < SK_ARRAY_COUNT(gProcs));
    return gProcs[enc];
}

int SkTypeface_FreeType::onCharsToGlyphs(const void* chars, Encoding encoding,
                                         uint16_t glyphs[], int glyphCount) const
{
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        if (glyphs) {
            sk_bzero(glyphs, glyphCount * sizeof(glyphs[0]));
        }
        return 0;
    }

    EncodingProc next_uni_proc = find_encoding_proc(encoding);

    if (nullptr == glyphs) {
        for (int i = 0; i < glyphCount; ++i) {
            if (0 == FT_Get_Char_Index(face, next_uni_proc(&chars))) {
                return i;
            }
        }
        return glyphCount;
    } else {
        int first = glyphCount;
        for (int i = 0; i < glyphCount; ++i) {
            unsigned id = FT_Get_Char_Index(face, next_uni_proc(&chars));
            glyphs[i] = SkToU16(id);
            if (0 == id && i < first) {
                first = i;
            }
        }
        return first;
    }
}

int SkTypeface_FreeType::onCountGlyphs() const {
    // we cache this value, using -1 as a sentinel for "not computed"
    if (fGlyphCount < 0) {
        AutoFTAccess fta(this);
        FT_Face face = fta.face();
        // if the face failed, we still assign a non-negative value
        fGlyphCount = face ? face->num_glyphs : 0;
    }
    return fGlyphCount;
}

SkTypeface::LocalizedStrings* SkTypeface_FreeType::onCreateFamilyNameIterator() const {
    SkTypeface::LocalizedStrings* nameIter =
        SkOTUtils::LocalizedStrings_NameTable::CreateForFamilyNames(*this);
    if (nullptr == nameIter) {
        SkString familyName;
        this->getFamilyName(&familyName);
        SkString language("und"); //undetermined
        nameIter = new SkOTUtils::LocalizedStrings_SingleName(familyName, language);
    }
    return nameIter;
}

int SkTypeface_FreeType::onGetTableTags(SkFontTableTag tags[]) const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();

    FT_ULong tableCount = 0;
    FT_Error error;

    // When 'tag' is nullptr, returns number of tables in 'length'.
    error = FT_Sfnt_Table_Info(face, 0, nullptr, &tableCount);
    if (error) {
        return 0;
    }

    if (tags) {
        for (FT_ULong tableIndex = 0; tableIndex < tableCount; ++tableIndex) {
            FT_ULong tableTag;
            FT_ULong tablelength;
            error = FT_Sfnt_Table_Info(face, tableIndex, &tableTag, &tablelength);
            if (error) {
                return 0;
            }
            tags[tableIndex] = static_cast<SkFontTableTag>(tableTag);
        }
    }
    return tableCount;
}

size_t SkTypeface_FreeType::onGetTableData(SkFontTableTag tag, size_t offset,
                                           size_t length, void* data) const
{
    AutoFTAccess fta(this);
    FT_Face face = fta.face();

    FT_ULong tableLength = 0;
    FT_Error error;

    // When 'length' is 0 it is overwritten with the full table length; 'offset' is ignored.
    error = FT_Load_Sfnt_Table(face, tag, 0, nullptr, &tableLength);
    if (error) {
        return 0;
    }

    if (offset > tableLength) {
        return 0;
    }
    FT_ULong size = SkTMin((FT_ULong)length, tableLength - (FT_ULong)offset);
    if (data) {
        error = FT_Load_Sfnt_Table(face, tag, offset, reinterpret_cast<FT_Byte*>(data), &size);
        if (error) {
            return 0;
        }
    }

    return size;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkTypeface_FreeType::Scanner::Scanner() : fLibrary(nullptr) {
    if (FT_New_Library(&gFTMemory, &fLibrary)) {
        return;
    }
    FT_Add_Default_Modules(fLibrary);
}
SkTypeface_FreeType::Scanner::~Scanner() {
    if (fLibrary) {
        FT_Done_Library(fLibrary);
    }
}

FT_Face SkTypeface_FreeType::Scanner::openFace(SkStream* stream, int ttcIndex,
                                               FT_Stream ftStream) const
{
    if (fLibrary == nullptr) {
        return nullptr;
    }

    FT_Open_Args args;
    memset(&args, 0, sizeof(args));

    const void* memoryBase = stream->getMemoryBase();

    if (memoryBase) {
        args.flags = FT_OPEN_MEMORY;
        args.memory_base = (const FT_Byte*)memoryBase;
        args.memory_size = stream->getLength();
    } else {
        memset(ftStream, 0, sizeof(*ftStream));
        ftStream->size = stream->getLength();
        ftStream->descriptor.pointer = stream;
        ftStream->read  = sk_ft_stream_io;
        ftStream->close = sk_ft_stream_close;

        args.flags = FT_OPEN_STREAM;
        args.stream = ftStream;
    }

    FT_Face face;
    if (FT_Open_Face(fLibrary, &args, ttcIndex, &face)) {
        return nullptr;
    }
    return face;
}

bool SkTypeface_FreeType::Scanner::recognizedFont(SkStream* stream, int* numFaces) const {
    SkAutoMutexAcquire libraryLock(fLibraryMutex);

    FT_StreamRec streamRec;
    FT_Face face = this->openFace(stream, -1, &streamRec);
    if (nullptr == face) {
        return false;
    }

    *numFaces = face->num_faces;

    FT_Done_Face(face);
    return true;
}

#include "SkTSearch.h"
bool SkTypeface_FreeType::Scanner::scanFont(
    SkStream* stream, int ttcIndex,
    SkString* name, SkFontStyle* style, bool* isFixedPitch, AxisDefinitions* axes) const
{
    SkAutoMutexAcquire libraryLock(fLibraryMutex);

    FT_StreamRec streamRec;
    FT_Face face = this->openFace(stream, ttcIndex, &streamRec);
    if (nullptr == face) {
        return false;
    }

    int weight = SkFontStyle::kNormal_Weight;
    int width = SkFontStyle::kNormal_Width;
    SkFontStyle::Slant slant = SkFontStyle::kUpright_Slant;
    if (face->style_flags & FT_STYLE_FLAG_BOLD) {
        weight = SkFontStyle::kBold_Weight;
    }
    if (face->style_flags & FT_STYLE_FLAG_ITALIC) {
        slant = SkFontStyle::kItalic_Slant;
    }

    PS_FontInfoRec psFontInfo;
    TT_OS2* os2 = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(face, ft_sfnt_os2));
    if (os2 && os2->version != 0xffff) {
        weight = os2->usWeightClass;
        width = os2->usWidthClass;
    } else if (0 == FT_Get_PS_Font_Info(face, &psFontInfo) && psFontInfo.weight) {
        static const struct {
            char const * const name;
            int const weight;
        } commonWeights [] = {
            // There are probably more common names, but these are known to exist.
            { "all", SkFontStyle::kNormal_Weight }, // Multiple Masters usually default to normal.
            { "black", SkFontStyle::kBlack_Weight },
            { "bold", SkFontStyle::kBold_Weight },
            { "book", (SkFontStyle::kNormal_Weight + SkFontStyle::kLight_Weight)/2 },
            { "demi", SkFontStyle::kSemiBold_Weight },
            { "demibold", SkFontStyle::kSemiBold_Weight },
            { "extra", SkFontStyle::kExtraBold_Weight },
            { "extrabold", SkFontStyle::kExtraBold_Weight },
            { "extralight", SkFontStyle::kExtraLight_Weight },
            { "hairline", SkFontStyle::kThin_Weight },
            { "heavy", SkFontStyle::kBlack_Weight },
            { "light", SkFontStyle::kLight_Weight },
            { "medium", SkFontStyle::kMedium_Weight },
            { "normal", SkFontStyle::kNormal_Weight },
            { "plain", SkFontStyle::kNormal_Weight },
            { "regular", SkFontStyle::kNormal_Weight },
            { "roman", SkFontStyle::kNormal_Weight },
            { "semibold", SkFontStyle::kSemiBold_Weight },
            { "standard", SkFontStyle::kNormal_Weight },
            { "thin", SkFontStyle::kThin_Weight },
            { "ultra", SkFontStyle::kExtraBold_Weight },
            { "ultrablack", 1000 },
            { "ultrabold", SkFontStyle::kExtraBold_Weight },
            { "ultraheavy", 1000 },
            { "ultralight", SkFontStyle::kExtraLight_Weight },
        };
        int const index = SkStrLCSearch(&commonWeights[0].name, SK_ARRAY_COUNT(commonWeights),
                                        psFontInfo.weight, sizeof(commonWeights[0]));
        if (index >= 0) {
            weight = commonWeights[index].weight;
        } else {
            SkDEBUGF(("Do not know weight for: %s (%s) \n", face->family_name, psFontInfo.weight));
        }
    }

    if (name) {
        name->set(face->family_name);
    }
    if (style) {
        *style = SkFontStyle(weight, width, slant);
    }
    if (isFixedPitch) {
        *isFixedPitch = FT_IS_FIXED_WIDTH(face);
    }

    if (axes && face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS) {
        FT_MM_Var* variations = nullptr;
        FT_Error err = FT_Get_MM_Var(face, &variations);
        if (err) {
            SkDEBUGF(("INFO: font %s claims to have variations, but none found.\n",
                      face->family_name));
            return false;
        }
        SkAutoFree autoFreeVariations(variations);

        axes->reset(variations->num_axis);
        for (FT_UInt i = 0; i < variations->num_axis; ++i) {
            const FT_Var_Axis& ftAxis = variations->axis[i];
            (*axes)[i].fTag = ftAxis.tag;
            (*axes)[i].fMinimum = ftAxis.minimum;
            (*axes)[i].fDefault = ftAxis.def;
            (*axes)[i].fMaximum = ftAxis.maximum;
        }
    }

    FT_Done_Face(face);
    return true;
}
