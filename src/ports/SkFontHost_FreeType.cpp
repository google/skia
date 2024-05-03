/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBBHFactory.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkTSearch.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkFDot6.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontScanner.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkScalerContext.h"
#include "src/ports/SkFontHost_FreeType_common.h"
#include "src/ports/SkTypeface_FreeType.h"
#include "src/sfnt/SkOTUtils.h"
#include "src/sfnt/SkSFNTHeader.h"
#include "src/sfnt/SkTTCFHeader.h"
#include "src/utils/SkCallableTraits.h"
#include "src/utils/SkMatrix22.h"

#include <memory>
#include <optional>
#include <tuple>

#include <ft2build.h>
#include <freetype/ftadvanc.h>
#include <freetype/ftimage.h>
#include <freetype/ftbitmap.h>
#ifdef FT_COLOR_H  // 2.10.0
#   include <freetype/ftcolor.h>
#endif
#include <freetype/freetype.h>
#include <freetype/ftlcdfil.h>
#include <freetype/ftmodapi.h>
#include <freetype/ftmm.h>
#include <freetype/ftoutln.h>
#include <freetype/ftsizes.h>
#include <freetype/ftsystem.h>
#include <freetype/tttables.h>
#include <freetype/t1tables.h>
#include <freetype/ftfntfmt.h>

using namespace skia_private;

namespace {
[[maybe_unused]] static inline const constexpr bool kSkShowTextBlitCoverage = false;

using SkUniqueFTFace = std::unique_ptr<FT_FaceRec, SkFunctionObject<FT_Done_Face>>;
using SkUniqueFTSize = std::unique_ptr<FT_SizeRec, SkFunctionObject<FT_Done_Size>>;
}

// SK_FREETYPE_MINIMUM_RUNTIME_VERSION 0x<major><minor><patch><flags>
// Flag SK_FREETYPE_DLOPEN: also try dlopen to get newer features.
#define SK_FREETYPE_DLOPEN (0x1)
#ifndef SK_FREETYPE_MINIMUM_RUNTIME_VERSION
#  if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) || \
      defined(SK_BUILD_FOR_GOOGLE3) ||           \
      defined(SK_FREETYPE_MINIMUM_RUNTIME_VERSION_IS_BUILD_VERSION)
#    define SK_FREETYPE_MINIMUM_RUNTIME_VERSION (((FREETYPE_MAJOR) << 24) | ((FREETYPE_MINOR) << 16) | ((FREETYPE_PATCH) << 8))
#  else
#    define SK_FREETYPE_MINIMUM_RUNTIME_VERSION ((2 << 24) | (8 << 16) | (1 << 8) | (SK_FREETYPE_DLOPEN))
#  endif
#endif
#if SK_FREETYPE_MINIMUM_RUNTIME_VERSION & SK_FREETYPE_DLOPEN
#  include <dlfcn.h>
#endif

#ifdef TT_SUPPORT_COLRV1
// FT_ClipBox and FT_Get_Color_Glyph_ClipBox introduced VER-2-11-0-18-g47cf8ebf4
// FT_COLR_COMPOSITE_PLUS and renumbering introduced VER-2-11-0-21-ge40ae7569
// FT_SIZEOF_LONG_LONG introduced VER-2-11-0-31-gffdac8d67
// FT_PaintRadialGradient changed size and layout at VER-2-11-0-147-gd3d3ff76d
// FT_STATIC_CAST introduced VER-2-11-0-172-g9079c5d91
// So undefine TT_SUPPORT_COLRV1 before 2.11.1 but not if FT_STATIC_CAST is defined.
#if (((FREETYPE_MAJOR)  < 2) || \
     ((FREETYPE_MAJOR) == 2 && (FREETYPE_MINOR)  < 11) || \
     ((FREETYPE_MAJOR) == 2 && (FREETYPE_MINOR) == 11 && (FREETYPE_PATCH) < 1)) && \
    !defined(FT_STATIC_CAST)
#    undef TT_SUPPORT_COLRV1
#endif
#endif

//#define ENABLE_GLYPH_SPEW     // for tracing calls
//#define DUMP_STRIKE_CREATION
//#define SK_FONTHOST_FREETYPE_RUNTIME_VERSION
//#define SK_GAMMA_APPLY_TO_A8

#if 1
    #define LOG_INFO(...)
#else
    #define LOG_INFO SkDEBUGF
#endif

static bool isLCD(const SkScalerContextRec& rec) {
    return SkMask::kLCD16_Format == rec.fMaskFormat;
}

static SkScalar SkFT_FixedToScalar(FT_Fixed x) {
  return SkFixedToScalar(x);
}

//////////////////////////////////////////////////////////////////////////

using FT_Alloc_size_t = SkCallableTraits<FT_Alloc_Func>::argument<1>::type;
static_assert(std::is_same<FT_Alloc_size_t, long  >::value ||
              std::is_same<FT_Alloc_size_t, size_t>::value,"");

extern "C" {
    static void* sk_ft_alloc(FT_Memory, FT_Alloc_size_t size) {
        return sk_malloc_canfail(size);
    }
    static void sk_ft_free(FT_Memory, void* block) {
        sk_free(block);
    }
    static void* sk_ft_realloc(FT_Memory, FT_Alloc_size_t cur_size,
                                          FT_Alloc_size_t new_size, void* block) {
        return sk_realloc_throw(block, new_size);
    }
}
FT_MemoryRec_ gFTMemory = { nullptr, sk_ft_alloc, sk_ft_free, sk_ft_realloc };

class FreeTypeLibrary : SkNoncopyable {
public:
    FreeTypeLibrary() : fLibrary(nullptr) {
        if (FT_New_Library(&gFTMemory, &fLibrary)) {
            return;
        }
        FT_Add_Default_Modules(fLibrary);
        FT_Set_Default_Properties(fLibrary);

        // Subpixel anti-aliasing may be unfiltered until the LCD filter is set.
        // Newer versions may still need this, so this test with side effects must come first.
        // The default has changed over time, so this doesn't mean the same thing to all users.
        FT_Library_SetLcdFilter(fLibrary, FT_LCD_FILTER_DEFAULT);
    }
    ~FreeTypeLibrary() {
        if (fLibrary) {
            FT_Done_Library(fLibrary);
        }
    }

    FT_Library library() { return fLibrary; }

private:
    FT_Library fLibrary;

    // FT_Library_SetLcdFilterWeights 2.4.0
    // FT_LOAD_COLOR 2.5.0
    // FT_Pixel_Mode::FT_PIXEL_MODE_BGRA 2.5.0
    // Thread safety in 2.6.0
    // freetype/ftfntfmt.h (rename) 2.6.0
    // Direct header inclusion 2.6.1
    // FT_Get_Var_Design_Coordinates 2.7.1
    // FT_LOAD_BITMAP_METRICS_ONLY 2.7.1
    // FT_Set_Default_Properties 2.7.2
    // The 'light' hinting is vertical only from 2.8.0
    // FT_Get_Var_Axis_Flags 2.8.1
    // FT_VAR_AXIS_FLAG_HIDDEN was introduced in FreeType 2.8.1
    // --------------------
    // FT_Done_MM_Var 2.9.0 (Currenty setting ft_free to a known allocator.)
    // freetype/ftcolor.h 2.10.0 (Currently assuming if compiled with FT_COLOR_H runtime available.)

    // Ubuntu 18.04       2.8.1
    // Debian 10          2.9.1
    // openSUSE Leap 15.2 2.10.1
    // Fedora 32          2.10.4
    // RHEL 8             2.9.1
};

static SkMutex& f_t_mutex() {
    static SkMutex& mutex = *(new SkMutex);
    return mutex;
}

static FreeTypeLibrary* gFTLibrary;

///////////////////////////////////////////////////////////////////////////

class SkTypeface_FreeType::FaceRec {
public:
    SkUniqueFTFace fFace;
    FT_StreamRec fFTStream;
    std::unique_ptr<SkStreamAsset> fSkStream;
    FT_UShort fFTPaletteEntryCount = 0;
    std::unique_ptr<SkColor[]> fSkPalette;

    static std::unique_ptr<FaceRec> Make(const SkTypeface_FreeType* typeface);
    ~FaceRec();

private:
    FaceRec(std::unique_ptr<SkStreamAsset> stream);
    void setupAxes(const SkFontData& data);
    void setupPalette(const SkFontData& data);

    // Private to ref_ft_library and unref_ft_library
    static int gFTCount;

    // Caller must lock f_t_mutex() before calling this function.
    static bool ref_ft_library() {
        f_t_mutex().assertHeld();
        SkASSERT(gFTCount >= 0);

        if (0 == gFTCount) {
            SkASSERT(nullptr == gFTLibrary);
            gFTLibrary = new FreeTypeLibrary;
        }
        ++gFTCount;
        return gFTLibrary->library();
    }

    // Caller must lock f_t_mutex() before calling this function.
    static void unref_ft_library() {
        f_t_mutex().assertHeld();
        SkASSERT(gFTCount > 0);

        --gFTCount;
        if (0 == gFTCount) {
            SkASSERT(nullptr != gFTLibrary);
            delete gFTLibrary;
            SkDEBUGCODE(gFTLibrary = nullptr;)
        }
    }
};
int SkTypeface_FreeType::FaceRec::gFTCount;

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

SkTypeface_FreeType::FaceRec::FaceRec(std::unique_ptr<SkStreamAsset> stream)
        : fSkStream(std::move(stream))
{
    sk_bzero(&fFTStream, sizeof(fFTStream));
    fFTStream.size = fSkStream->getLength();
    fFTStream.descriptor.pointer = fSkStream.get();
    fFTStream.read  = sk_ft_stream_io;
    fFTStream.close = sk_ft_stream_close;

    f_t_mutex().assertHeld();
    ref_ft_library();
}

SkTypeface_FreeType::FaceRec::~FaceRec() {
    f_t_mutex().assertHeld();
    fFace.reset(); // Must release face before the library, the library frees existing faces.
    unref_ft_library();
}

void SkTypeface_FreeType::FaceRec::setupAxes(const SkFontData& data) {
    if (!(fFace->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS)) {
        return;
    }

    // If a named variation is requested, don't overwrite the named variation's position.
    if (data.getIndex() > 0xFFFF) {
        return;
    }

    SkDEBUGCODE(
        FT_MM_Var* variations = nullptr;
        if (FT_Get_MM_Var(fFace.get(), &variations)) {
            LOG_INFO("INFO: font %s claims variations, but none found.\n",
                     rec->fFace->family_name);
            return;
        }
        UniqueVoidPtr autoFreeVariations(variations);

        if (static_cast<FT_UInt>(data.getAxisCount()) != variations->num_axis) {
            LOG_INFO("INFO: font %s has %d variations, but %d were specified.\n",
                     rec->fFace->family_name, variations->num_axis, data.getAxisCount());
            return;
        }
    )

    AutoSTMalloc<4, FT_Fixed> coords(data.getAxisCount());
    for (int i = 0; i < data.getAxisCount(); ++i) {
        coords[i] = data.getAxis()[i];
    }
    if (FT_Set_Var_Design_Coordinates(fFace.get(), data.getAxisCount(), coords.get())) {
        LOG_INFO("INFO: font %s has variations, but specified variations could not be set.\n",
                 rec->fFace->family_name);
        return;
    }
}

void SkTypeface_FreeType::FaceRec::setupPalette(const SkFontData& data) {
#ifdef FT_COLOR_H
    FT_Palette_Data paletteData;
    if (FT_Palette_Data_Get(fFace.get(), &paletteData)) {
        return;
    }

    // Treat out of range values as 0. Still apply overrides.
    // https://www.w3.org/TR/css-fonts-4/#base-palette-desc
    FT_UShort basePaletteIndex = 0;
    if (SkTFitsIn<FT_UShort>(data.getPaletteIndex()) &&
        SkTo<FT_UShort>(data.getPaletteIndex()) < paletteData.num_palettes)
    {
        basePaletteIndex = data.getPaletteIndex();
    }

    FT_Color* ftPalette = nullptr;
    if (FT_Palette_Select(fFace.get(), basePaletteIndex, &ftPalette)) {
        return;
    }
    fFTPaletteEntryCount = paletteData.num_palette_entries;

    for (int i = 0; i < data.getPaletteOverrideCount(); ++i) {
        const SkFontArguments::Palette::Override& paletteOverride = data.getPaletteOverrides()[i];
        if (paletteOverride.index < fFTPaletteEntryCount) {
            const SkColor& skColor = paletteOverride.color;
            FT_Color& ftColor = ftPalette[paletteOverride.index];
            ftColor.blue  = SkColorGetB(skColor);
            ftColor.green = SkColorGetG(skColor);
            ftColor.red   = SkColorGetR(skColor);
            ftColor.alpha = SkColorGetA(skColor);
        }
    }

    fSkPalette.reset(new SkColor[fFTPaletteEntryCount]);
    for (int i = 0; i < fFTPaletteEntryCount; ++i) {
        fSkPalette[i] = SkColorSetARGB(ftPalette[i].alpha,
                                       ftPalette[i].red,
                                       ftPalette[i].green,
                                       ftPalette[i].blue);
    }
#endif
}

// Will return nullptr on failure
// Caller must lock f_t_mutex() before calling this function.
std::unique_ptr<SkTypeface_FreeType::FaceRec>
SkTypeface_FreeType::FaceRec::Make(const SkTypeface_FreeType* typeface) {
    f_t_mutex().assertHeld();

    std::unique_ptr<SkFontData> data = typeface->makeFontData();
    if (nullptr == data || !data->hasStream()) {
        return nullptr;
    }

    std::unique_ptr<FaceRec> rec(new FaceRec(data->detachStream()));

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

    {
        FT_Face rawFace;
        FT_Error err = FT_Open_Face(gFTLibrary->library(), &args, data->getIndex(), &rawFace);
        if (err) {
            SK_TRACEFTR(err, "unable to open font '%x'", (uint32_t)typeface->uniqueID());
            return nullptr;
        }
        rec->fFace.reset(rawFace);
    }
    SkASSERT(rec->fFace);

    rec->setupAxes(*data);
    rec->setupPalette(*data);

    // FreeType will set the charmap to the "most unicode" cmap if it exists.
    // If there are no unicode cmaps, the charmap is set to nullptr.
    // However, "symbol" cmaps should also be considered "fallback unicode" cmaps
    // because they are effectively private use area only (even if they aren't).
    // This is the last on the fallback list at
    // https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
    if (!rec->fFace->charmap) {
        FT_Select_Charmap(rec->fFace.get(), FT_ENCODING_MS_SYMBOL);
    }

    return rec;
}

class AutoFTAccess {
public:
    AutoFTAccess(const SkTypeface_FreeType* tf) : fFaceRec(nullptr) {
        f_t_mutex().acquire();
        fFaceRec = tf->getFaceRec();
    }

    ~AutoFTAccess() {
        f_t_mutex().release();
    }

    FT_Face face() { return fFaceRec ? fFaceRec->fFace.get() : nullptr; }

private:
    SkTypeface_FreeType::FaceRec* fFaceRec;
};

///////////////////////////////////////////////////////////////////////////

class SkScalerContext_FreeType : public SkScalerContext {
public:
    SkScalerContext_FreeType(sk_sp<SkTypeface_FreeType>,
                             const SkScalerContextEffects&,
                             const SkDescriptor* desc);
    ~SkScalerContext_FreeType() override;

    bool success() const {
        return fFTSize != nullptr && fFace != nullptr;
    }

protected:
    GlyphMetrics generateMetrics(const SkGlyph&, SkArenaAlloc*) override;
    void generateImage(const SkGlyph&, void*) override;
    bool generatePath(const SkGlyph& glyph, SkPath* path) override;
    sk_sp<SkDrawable> generateDrawable(const SkGlyph&) override;
    void generateFontMetrics(SkFontMetrics*) override;

private:
    struct ScalerContextBits {
        static const constexpr uint32_t COLRv0 = 1;
        static const constexpr uint32_t COLRv1 = 2;
        static const constexpr uint32_t SVG    = 3;
    };

    // See http://freetype.sourceforge.net/freetype2/docs/reference/ft2-bitmap_handling.html#FT_Bitmap_Embolden
    // This value was chosen by eyeballing the result in Firefox and trying to match it.
    static const FT_Pos kBitmapEmboldenStrength = 1 << 6;

    SkTypeface_FreeType::FaceRec* fFaceRec; // Borrowed face from the typeface's FaceRec.
    FT_Face   fFace;  // Borrowed face from fFaceRec.
    FT_Size   fFTSize;  // The size to apply to the fFace.
    FT_Int    fStrikeIndex; // The bitmap strike for the fFace (or -1 if none).

    SkScalerContextFTUtils fUtils;

    /** The rest of the matrix after FreeType handles the size.
     *  With outline font rasterization this is handled by FreeType with FT_Set_Transform.
     *  With bitmap only fonts this matrix must be applied to scale the bitmap.
     */
    SkMatrix  fMatrix22Scalar;
    /** Same as fMatrix22Scalar, but in FreeType units and space. */
    FT_Matrix fMatrix22;
    /** The actual size requested. */
    SkVector  fScale;

    uint32_t  fLoadGlyphFlags;
    bool      fDoLinearMetrics;
    bool      fLCDIsVert;

    FT_Error setupSize();
    // Caller must lock f_t_mutex() before calling this function.
    static bool getBoundsOfCurrentOutlineGlyph(FT_GlyphSlot glyph, SkRect* bounds);
    // Caller must lock f_t_mutex() before calling this function.
    bool getCBoxForLetter(char letter, FT_BBox* bbox);
    static void updateGlyphBoundsIfSubpixel(const SkGlyph&, SkRect* bounds, bool subpixel);
    void updateGlyphBoundsIfLCD(GlyphMetrics* mx);
    // Caller must lock f_t_mutex() before calling this function.
    // update FreeType2 glyph slot with glyph emboldened
    void emboldenIfNeeded(FT_Face face, FT_GlyphSlot glyph, SkGlyphID gid);
    bool shouldSubpixelBitmap(const SkGlyph&, const SkMatrix&);
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

static SkAdvancedTypefaceMetrics::FontType get_font_type(FT_Face face) {
    const char* fontType = FT_Get_X11_Font_Format(face);
    static struct { const char* s; SkAdvancedTypefaceMetrics::FontType t; } values[] = {
        { "Type 1",     SkAdvancedTypefaceMetrics::kType1_Font    },
        { "CID Type 1", SkAdvancedTypefaceMetrics::kType1CID_Font },
        { "CFF",        SkAdvancedTypefaceMetrics::kCFF_Font      },
        { "TrueType",   SkAdvancedTypefaceMetrics::kTrueType_Font },
    };
    for(const auto& v : values) { if (strcmp(fontType, v.s) == 0) { return v.t; } }
    return SkAdvancedTypefaceMetrics::kOther_Font;
}

static bool is_opentype_font_data_standard_format(const SkTypeface& typeface) {
    // FreeType reports TrueType for any data that can be decoded to TrueType or OpenType.
    // However, there are alternate data formats for OpenType, like wOFF and wOF2.
    std::unique_ptr<SkStreamAsset> stream = typeface.openStream(nullptr);
    if (!stream) {
        return false;
    }
    char buffer[4];
    if (stream->read(buffer, 4) < 4) {
        return false;
    }

    SkFourByteTag tag = SkSetFourByteTag(buffer[0], buffer[1], buffer[2], buffer[3]);
    SK_OT_ULONG otTag = SkEndian_SwapBE32(tag);
    return otTag == SkSFNTHeader::fontType_WindowsTrueType::TAG ||
           otTag == SkSFNTHeader::fontType_MacTrueType::TAG ||
           otTag == SkSFNTHeader::fontType_PostScript::TAG ||
           otTag == SkSFNTHeader::fontType_OpenTypeCFF::TAG ||
           otTag == SkTTCFHeader::TAG;
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkTypeface_FreeType::onGetAdvancedMetrics() const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return nullptr;
    }

    std::unique_ptr<SkAdvancedTypefaceMetrics> info(new SkAdvancedTypefaceMetrics);
    info->fPostScriptName.set(FT_Get_Postscript_Name(face));

    if (FT_HAS_MULTIPLE_MASTERS(face)) {
        info->fFlags |= SkAdvancedTypefaceMetrics::kVariable_FontFlag;
    }
    if (!canEmbed(face)) {
        info->fFlags |= SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag;
    }
    if (!canSubset(face)) {
        info->fFlags |= SkAdvancedTypefaceMetrics::kNotSubsettable_FontFlag;
    }

    info->fType = get_font_type(face);
    if ((info->fType == SkAdvancedTypefaceMetrics::kTrueType_Font ||
         info->fType == SkAdvancedTypefaceMetrics::kCFF_Font) &&
        !is_opentype_font_data_standard_format(*this))
    {
        info->fFlags |= SkAdvancedTypefaceMetrics::kAltDataFormat_FontFlag;
    }

    info->fStyle = (SkAdvancedTypefaceMetrics::StyleFlags)0;
    if (FT_IS_FIXED_WIDTH(face)) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kFixedPitch_Style;
    }
    if (face->style_flags & FT_STYLE_FLAG_ITALIC) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kItalic_Style;
    }

    PS_FontInfoRec psFontInfo;
    TT_Postscript* postTable;
    if (FT_Get_PS_Font_Info(face, &psFontInfo) == 0) {
        info->fItalicAngle = psFontInfo.italic_angle;
    } else if ((postTable = (TT_Postscript*)FT_Get_Sfnt_Table(face, ft_sfnt_post)) != nullptr) {
        info->fItalicAngle = SkFixedFloorToInt(postTable->italicAngle);
    } else {
        info->fItalicAngle = 0;
    }

    info->fAscent = face->ascender;
    info->fDescent = face->descender;

    TT_PCLT* pcltTable;
    TT_OS2* os2Table;
    if ((pcltTable = (TT_PCLT*)FT_Get_Sfnt_Table(face, ft_sfnt_pclt)) != nullptr) {
        info->fCapHeight = pcltTable->CapHeight;
        uint8_t serif_style = pcltTable->SerifStyle & 0x3F;
        if (2 <= serif_style && serif_style <= 6) {
            info->fStyle |= SkAdvancedTypefaceMetrics::kSerif_Style;
        } else if (9 <= serif_style && serif_style <= 12) {
            info->fStyle |= SkAdvancedTypefaceMetrics::kScript_Style;
        }
    } else if (((os2Table = (TT_OS2*)FT_Get_Sfnt_Table(face, ft_sfnt_os2)) != nullptr) &&
               // sCapHeight is available only when version 2 or later.
               os2Table->version != 0xFFFF &&
               os2Table->version >= 2)
    {
        info->fCapHeight = os2Table->sCapHeight;
    }
    info->fBBox = SkIRect::MakeLTRB(face->bbox.xMin, face->bbox.yMax,
                                    face->bbox.xMax, face->bbox.yMin);
    return info;
}

void SkTypeface_FreeType::getGlyphToUnicodeMap(SkUnichar* dstArray) const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return;
    }

    FT_Long numGlyphs = face->num_glyphs;
    if (!dstArray) { SkASSERT(numGlyphs == 0); }
    sk_bzero(dstArray, sizeof(SkUnichar) * numGlyphs);

    FT_UInt glyphIndex;
    SkUnichar charCode = FT_Get_First_Char(face, &glyphIndex);
    while (glyphIndex) {
        SkASSERT(glyphIndex < SkToUInt(numGlyphs));
        // Use the first character that maps to this glyphID. https://crbug.com/359065
        if (0 == dstArray[glyphIndex]) {
            dstArray[glyphIndex] = charCode;
        }
        charCode = FT_Get_Next_Char(face, charCode, &glyphIndex);
    }
}

void SkTypeface_FreeType::getPostScriptGlyphNames(SkString* dstArray) const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return;
    }

    FT_Long numGlyphs = face->num_glyphs;
    if (!dstArray) { SkASSERT(numGlyphs == 0); }

    if (FT_HAS_GLYPH_NAMES(face)) {
        for (int gID = 0; gID < numGlyphs; ++gID) {
            char glyphName[128];  // PS limit for names is 127 bytes.
            FT_Get_Glyph_Name(face, gID, glyphName, 128);
            dstArray[gID] = glyphName;
        }
    }
}

bool SkTypeface_FreeType::onGetPostScriptName(SkString* skPostScriptName) const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return false;
    }

    const char* ftPostScriptName = FT_Get_Postscript_Name(face);
    if (!ftPostScriptName) {
        return false;
    }
    if (skPostScriptName) {
        *skPostScriptName = ftPostScriptName;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////

static bool bothZero(SkScalar a, SkScalar b) {
    return 0 == a && 0 == b;
}

// returns false if there is any non-90-rotation or skew
static bool isAxisAligned(const SkScalerContextRec& rec) {
    return 0 == rec.fPreSkewX &&
           (bothZero(rec.fPost2x2[0][1], rec.fPost2x2[1][0]) ||
            bothZero(rec.fPost2x2[0][0], rec.fPost2x2[1][1]));
}

std::unique_ptr<SkScalerContext> SkTypeface_FreeType::onCreateScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc) const
{
    auto c = std::make_unique<SkScalerContext_FreeType>(
            sk_ref_sp(const_cast<SkTypeface_FreeType*>(this)), effects, desc);
    if (c->success()) {
        return c;
    }
    return SkScalerContext::MakeEmpty(
            sk_ref_sp(const_cast<SkTypeface_FreeType*>(this)), effects, desc);
}

/** Copy the design variation coordinates into 'coordinates'.
 *
 *  @param coordinates the buffer into which to write the design variation coordinates.
 *  @param coordinateCount the number of entries available through 'coordinates'.
 *
 *  @return The number of axes, or -1 if there is an error.
 *  If 'coordinates != nullptr' and 'coordinateCount >= numAxes' then 'coordinates' will be
 *  filled with the variation coordinates describing the position of this typeface in design
 *  variation space. It is possible the number of axes can be retrieved but actual position
 *  cannot.
 */
static int GetVariationDesignPosition(AutoFTAccess& fta,
    SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount)
{
    FT_Face face = fta.face();
    if (!face) {
        return -1;
    }

    if (!(face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS)) {
        return 0;
    }

    FT_MM_Var* variations = nullptr;
    if (FT_Get_MM_Var(face, &variations)) {
        return -1;
    }
    UniqueVoidPtr autoFreeVariations(variations);

    if (!coordinates || coordinateCount < SkToInt(variations->num_axis)) {
        return variations->num_axis;
    }

    AutoSTMalloc<4, FT_Fixed> coords(variations->num_axis);
    if (FT_Get_Var_Design_Coordinates(face, variations->num_axis, coords.get())) {
        return -1;
    }
    for (FT_UInt i = 0; i < variations->num_axis; ++i) {
        coordinates[i].axis = variations->axis[i].tag;
        coordinates[i].value = SkFixedToScalar(coords[i]);
    }

    return variations->num_axis;
}

std::unique_ptr<SkFontData> SkTypeface_FreeType::cloneFontData(const SkFontArguments& args,
                                                               SkFontStyle* style) const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return nullptr;
    }

    SkFontScanner::AxisDefinitions axisDefinitions;
    if (!SkFontScanner_FreeType::GetAxes(face, &axisDefinitions)) {
        return nullptr;
    }
    int axisCount = axisDefinitions.size();

    AutoSTMalloc<4, SkFontArguments::VariationPosition::Coordinate> currentPosition(axisCount);
    int currentAxisCount = GetVariationDesignPosition(fta, currentPosition, axisCount);

    SkString name;
    AutoSTMalloc<4, SkFixed> axisValues(axisCount);
    SkFontScanner_FreeType::computeAxisValues(
            axisDefinitions,
            args.getVariationDesignPosition(),
            axisValues,
            name, style,
            currentAxisCount == axisCount ? currentPosition.get() : nullptr);

    int ttcIndex;
    std::unique_ptr<SkStreamAsset> stream = this->openStream(&ttcIndex);

    return std::make_unique<SkFontData>(std::move(stream),
                                        ttcIndex,
                                        args.getPalette().index,
                                        axisValues.get(),
                                        axisCount,
                                        args.getPalette().overrides,
                                        args.getPalette().overrideCount);
}

void SkTypeface_FreeType::onFilterRec(SkScalerContextRec* rec) const {
    //BOGUS: http://code.google.com/p/chromium/issues/detail?id=121119
    //Cap the requested size as larger sizes give bogus values.
    //Remove when http://code.google.com/p/skia/issues/detail?id=554 is fixed.
    //Note that this also currently only protects against large text size requests,
    //the total matrix is not taken into account here.
    if (rec->fTextSize > SkIntToScalar(1 << 14)) {
        rec->fTextSize = SkIntToScalar(1 << 14);
    }

    SkFontHinting h = rec->getHinting();
    if (SkFontHinting::kFull == h && !isLCD(*rec)) {
        // collapse full->normal hinting if we're not doing LCD
        h = SkFontHinting::kNormal;
    }

    // rotated text looks bad with hinting, so we disable it as needed
    if (!isAxisAligned(*rec)) {
        h = SkFontHinting::kNone;
    }
    rec->setHinting(h);

#ifndef SK_GAMMA_APPLY_TO_A8
    if (!isLCD(*rec)) {
        // SRGBTODO: Is this correct? Do we want contrast boost?
        rec->ignorePreBlend();
    }
#endif
}

int SkTypeface_FreeType::GetUnitsPerEm(FT_Face face) {
    SkASSERT(face);

    SkScalar upem = SkIntToScalar(face->units_per_EM);
    // At least some versions of FreeType set face->units_per_EM to 0 for bitmap only fonts.
    if (upem == 0) {
        TT_Header* ttHeader = (TT_Header*)FT_Get_Sfnt_Table(face, ft_sfnt_head);
        if (ttHeader) {
            upem = SkIntToScalar(ttHeader->Units_Per_EM);
        }
    }
    return upem;
}

int SkTypeface_FreeType::onGetUPEM() const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return 0;
    }
    return GetUnitsPerEm(face);
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

/** Returns the bitmap strike equal to or just larger than the requested size. */
static FT_Int chooseBitmapStrike(FT_Face face, FT_F26Dot6 scaleY) {
    if (face == nullptr) {
        LOG_INFO("chooseBitmapStrike aborted due to nullptr face.\n");
        return -1;
    }

    FT_Pos requestedPPEM = scaleY;  // FT_Bitmap_Size::y_ppem is in 26.6 format.
    FT_Int chosenStrikeIndex = -1;
    FT_Pos chosenPPEM = 0;
    for (FT_Int strikeIndex = 0; strikeIndex < face->num_fixed_sizes; ++strikeIndex) {
        FT_Pos strikePPEM = face->available_sizes[strikeIndex].y_ppem;
        if (strikePPEM == requestedPPEM) {
            // exact match - our search stops here
            return strikeIndex;
        } else if (chosenPPEM < requestedPPEM) {
            // attempt to increase chosenPPEM
            if (chosenPPEM < strikePPEM) {
                chosenPPEM = strikePPEM;
                chosenStrikeIndex = strikeIndex;
            }
        } else {
            // attempt to decrease chosenPPEM, but not below requestedPPEM
            if (requestedPPEM < strikePPEM && strikePPEM < chosenPPEM) {
                chosenPPEM = strikePPEM;
                chosenStrikeIndex = strikeIndex;
            }
        }
    }
    return chosenStrikeIndex;
}

SkScalerContext_FreeType::SkScalerContext_FreeType(sk_sp<SkTypeface_FreeType> typeface,
                                                   const SkScalerContextEffects& effects,
                                                   const SkDescriptor* desc)
    : SkScalerContext(std::move(typeface), effects, desc)
    , fFace(nullptr)
    , fFTSize(nullptr)
    , fStrikeIndex(-1)
{
    SkAutoMutexExclusive  ac(f_t_mutex());
    fFaceRec = static_cast<SkTypeface_FreeType*>(this->getTypeface())->getFaceRec();

    // load the font file
    if (nullptr == fFaceRec) {
        LOG_INFO("Could not create FT_Face.\n");
        return;
    }

    fLCDIsVert = SkToBool(fRec.fFlags & SkScalerContext::kLCD_Vertical_Flag);

    // compute the flags we send to Load_Glyph
    bool linearMetrics = this->isLinearMetrics();
    {
        FT_Int32 loadFlags = FT_LOAD_DEFAULT;

        if (SkMask::kBW_Format == fRec.fMaskFormat) {
            // See http://code.google.com/p/chromium/issues/detail?id=43252#c24
            loadFlags = FT_LOAD_TARGET_MONO;
            if (fRec.getHinting() == SkFontHinting::kNone) {
                loadFlags |= FT_LOAD_NO_HINTING;
                linearMetrics = true;
            }
        } else {
            switch (fRec.getHinting()) {
            case SkFontHinting::kNone:
                loadFlags = FT_LOAD_NO_HINTING;
                linearMetrics = true;
                break;
            case SkFontHinting::kSlight:
                loadFlags = FT_LOAD_TARGET_LIGHT;  // This implies FORCE_AUTOHINT
                linearMetrics = true;
                break;
            case SkFontHinting::kNormal:
                loadFlags = FT_LOAD_TARGET_NORMAL;
                break;
            case SkFontHinting::kFull:
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
                LOG_INFO("---------- UNKNOWN hinting %d\n", fRec.getHinting());
                break;
            }
        }

        if (fRec.fFlags & SkScalerContext::kForceAutohinting_Flag) {
            loadFlags |= FT_LOAD_FORCE_AUTOHINT;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        } else {
            loadFlags |= FT_LOAD_NO_AUTOHINT;
#endif
        }

        if ((fRec.fFlags & SkScalerContext::kEmbeddedBitmapText_Flag) == 0) {
            loadFlags |= FT_LOAD_NO_BITMAP;
        }

        // Always using FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH to get correct
        // advances, as fontconfig and cairo do.
        // See http://code.google.com/p/skia/issues/detail?id=222.
        loadFlags |= FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH;

        // Use vertical layout if requested.
        if (this->isVertical()) {
            loadFlags |= FT_LOAD_VERTICAL_LAYOUT;
        }

        fLoadGlyphFlags = loadFlags;
    }

    SkUniqueFTSize ftSize([this]() -> FT_Size {
        FT_Size size;
        FT_Error err = FT_New_Size(fFaceRec->fFace.get(), &size);
        if (err != 0) {
            SK_TRACEFTR(err, "FT_New_Size(%s) failed.", fFaceRec->fFace->family_name);
            return nullptr;
        }
        return size;
    }());
    if (nullptr == ftSize) {
        LOG_INFO("Could not create FT_Size.\n");
        return;
    }

    FT_Error err = FT_Activate_Size(ftSize.get());
    if (err != 0) {
        SK_TRACEFTR(err, "FT_Activate_Size(%s) failed.", fFaceRec->fFace->family_name);
        return;
    }

    fRec.computeMatrices(SkScalerContextRec::PreMatrixScale::kFull, &fScale, &fMatrix22Scalar);
    FT_F26Dot6 scaleX = SkScalarToFDot6(fScale.fX);
    FT_F26Dot6 scaleY = SkScalarToFDot6(fScale.fY);

    if (FT_IS_SCALABLE(fFaceRec->fFace)) {
        err = FT_Set_Char_Size(fFaceRec->fFace.get(), scaleX, scaleY, 72, 72);
        if (err != 0) {
            SK_TRACEFTR(err, "FT_Set_CharSize(%s, %f, %f) failed.",
                        fFaceRec->fFace->family_name, fScale.fX, fScale.fY);
            return;
        }

        // Adjust the matrix to reflect the actually chosen scale.
        // FreeType currently does not allow requesting sizes less than 1, this allow for scaling.
        // Don't do this at all sizes as that will interfere with hinting.
        if (fScale.fX < 1 || fScale.fY < 1) {
            SkScalar upem = fFaceRec->fFace->units_per_EM;
            FT_Size_Metrics& ftmetrics = fFaceRec->fFace->size->metrics;
            SkScalar x_ppem = upem * SkFT_FixedToScalar(ftmetrics.x_scale) / 64.0f;
            SkScalar y_ppem = upem * SkFT_FixedToScalar(ftmetrics.y_scale) / 64.0f;
            fMatrix22Scalar.preScale(fScale.x() / x_ppem, fScale.y() / y_ppem);
        }

        // FT_LOAD_COLOR with scalable fonts means allow SVG.
        // It also implies attempt to render COLR if available, but this is not used.
#if defined(FT_CONFIG_OPTION_SVG)
        if (SkGraphics::GetOpenTypeSVGDecoderFactory()) {
            fLoadGlyphFlags |= FT_LOAD_COLOR;
        }
#endif
    } else if (FT_HAS_FIXED_SIZES(fFaceRec->fFace)) {
        fStrikeIndex = chooseBitmapStrike(fFaceRec->fFace.get(), scaleY);
        if (fStrikeIndex == -1) {
            LOG_INFO("No glyphs for font \"%s\" size %f.\n",
                     fFaceRec->fFace->family_name, fScale.fY);
            return;
        }

        err = FT_Select_Size(fFaceRec->fFace.get(), fStrikeIndex);
        if (err != 0) {
            SK_TRACEFTR(err, "FT_Select_Size(%s, %d) failed.",
                        fFaceRec->fFace->family_name, fStrikeIndex);
            fStrikeIndex = -1;
            return;
        }

        // Adjust the matrix to reflect the actually chosen scale.
        // It is likely that the ppem chosen was not the one requested, this allows for scaling.
        fMatrix22Scalar.preScale(fScale.x() / fFaceRec->fFace->size->metrics.x_ppem,
                                 fScale.y() / fFaceRec->fFace->size->metrics.y_ppem);

        // FreeType does not provide linear metrics for bitmap fonts.
        linearMetrics = false;

        // FreeType documentation says:
        // FT_LOAD_NO_BITMAP -- Ignore bitmap strikes when loading.
        // Bitmap-only fonts ignore this flag.
        //
        // However, in FreeType 2.5.1 color bitmap only fonts do not ignore this flag.
        // Force this flag off for bitmap only fonts.
        fLoadGlyphFlags &= ~FT_LOAD_NO_BITMAP;

        // Color bitmaps are supported.
        fLoadGlyphFlags |= FT_LOAD_COLOR;
    } else {
        LOG_INFO("Unknown kind of font \"%s\" size %f.\n", fFaceRec->fFace->family_name, fScale.fY);
        return;
    }

    fMatrix22.xx = SkScalarToFixed(fMatrix22Scalar.getScaleX());
    fMatrix22.xy = SkScalarToFixed(-fMatrix22Scalar.getSkewX());
    fMatrix22.yx = SkScalarToFixed(-fMatrix22Scalar.getSkewY());
    fMatrix22.yy = SkScalarToFixed(fMatrix22Scalar.getScaleY());

    fFTSize = ftSize.release();
    fFace = fFaceRec->fFace.get();
    fDoLinearMetrics = linearMetrics;
    fUtils.init(fRec.fForegroundColor, (SkScalerContext::Flags)fRec.fFlags);
}

SkScalerContext_FreeType::~SkScalerContext_FreeType() {
    SkAutoMutexExclusive  ac(f_t_mutex());

    if (fFTSize != nullptr) {
        FT_Done_Size(fFTSize);
    }

    fFaceRec = nullptr;
}

/*  We call this before each use of the fFace, since we may be sharing
    this face with other context (at different sizes).
*/
FT_Error SkScalerContext_FreeType::setupSize() {
    f_t_mutex().assertHeld();
    FT_Error err = FT_Activate_Size(fFTSize);
    if (err != 0) {
        return err;
    }
    FT_Set_Transform(fFace, &fMatrix22, nullptr);
    return 0;
}

bool SkScalerContext_FreeType::getBoundsOfCurrentOutlineGlyph(FT_GlyphSlot glyph, SkRect* bounds) {
    if (glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
        SkASSERT(false);
        return false;
    }
    if (0 == glyph->outline.n_contours) {
        return false;
    }

    FT_BBox bbox;
    FT_Outline_Get_CBox(&glyph->outline, &bbox);
    *bounds = SkRect::MakeLTRB(SkFDot6ToScalar(bbox.xMin), -SkFDot6ToScalar(bbox.yMax),
                               SkFDot6ToScalar(bbox.xMax), -SkFDot6ToScalar(bbox.yMin));
    return true;
}

bool SkScalerContext_FreeType::getCBoxForLetter(char letter, FT_BBox* bbox) {
    const FT_UInt glyph_id = FT_Get_Char_Index(fFace, letter);
    if (!glyph_id) {
        return false;
    }
    if (FT_Load_Glyph(fFace, glyph_id, fLoadGlyphFlags)) {
        return false;
    }
    if (fFace->glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
        return false;
    }
    emboldenIfNeeded(fFace, fFace->glyph, SkTo<SkGlyphID>(glyph_id));
    FT_Outline_Get_CBox(&fFace->glyph->outline, bbox);
    return true;
}

void SkScalerContext_FreeType::updateGlyphBoundsIfSubpixel(const SkGlyph& glyph, SkRect* bounds,
                                                           bool subpixel) {
    if (subpixel && !bounds->isEmpty()) {
        bounds->offset(SkFixedToScalar(glyph.getSubXFixed()),
                       SkFixedToScalar(glyph.getSubYFixed()));
    }
}

void SkScalerContext_FreeType::updateGlyphBoundsIfLCD(GlyphMetrics* mx) {
    if (mx->maskFormat == SkMask::kLCD16_Format && !mx->bounds.isEmpty()) {
        mx->bounds.roundOut(&mx->bounds);
        if (fLCDIsVert) {
            mx->bounds.fBottom += 1;
            mx->bounds.fTop -= 1;
        } else {
            mx->bounds.fRight += 1;
            mx->bounds.fLeft -= 1;
        }
    }
}

bool SkScalerContext_FreeType::shouldSubpixelBitmap(const SkGlyph& glyph, const SkMatrix& matrix) {
    // If subpixel rendering of a bitmap *can* be done.
    bool mechanism = fFace->glyph->format == FT_GLYPH_FORMAT_BITMAP &&
                     this->isSubpixel() &&
                     (glyph.getSubXFixed() || glyph.getSubYFixed());

    // If subpixel rendering of a bitmap *should* be done.
    // 1. If the face is not scalable then always allow subpixel rendering.
    //    Otherwise, if the font has an 8ppem strike 7 will subpixel render but 8 won't.
    // 2. If the matrix is already not identity the bitmap will already be resampled,
    //    so resampling slightly differently shouldn't make much difference.
    bool policy = !FT_IS_SCALABLE(fFace) || !matrix.isIdentity();

    return mechanism && policy;
}

SkScalerContext::GlyphMetrics SkScalerContext_FreeType::generateMetrics(const SkGlyph& glyph,
                                                                        SkArenaAlloc* alloc) {
    SkAutoMutexExclusive  ac(f_t_mutex());

    GlyphMetrics mx(glyph.maskFormat());

    if (this->setupSize()) {
        return mx;
    }

    FT_Bool haveLayers = false;
#ifdef FT_COLOR_H
    // See https://skbug.com/12945, if the face isn't marked scalable then paths cannot be loaded.
    if (FT_IS_SCALABLE(fFace)) {
        SkRect bounds = SkRect::MakeEmpty();
#ifdef TT_SUPPORT_COLRV1
        FT_OpaquePaint opaqueLayerPaint{nullptr, 1};
        if (FT_Get_Color_Glyph_Paint(fFace, glyph.getGlyphID(),
                                     FT_COLOR_INCLUDE_ROOT_TRANSFORM, &opaqueLayerPaint)) {
            haveLayers = true;
            mx.extraBits = ScalerContextBits::COLRv1;

            // COLRv1 optionally provides a ClipBox.
            FT_ClipBox clipBox;
            if (FT_Get_Color_Glyph_ClipBox(fFace, glyph.getGlyphID(), &clipBox)) {
                // Find bounding box of clip box corner points, needed when clipbox is transformed.
                FT_BBox bbox;
                bbox.xMin = clipBox.bottom_left.x;
                bbox.xMax = clipBox.bottom_left.x;
                bbox.yMin = clipBox.bottom_left.y;
                bbox.yMax = clipBox.bottom_left.y;
                for (auto& corner : {clipBox.top_left, clipBox.top_right, clipBox.bottom_right}) {
                    bbox.xMin = std::min(bbox.xMin, corner.x);
                    bbox.yMin = std::min(bbox.yMin, corner.y);
                    bbox.xMax = std::max(bbox.xMax, corner.x);
                    bbox.yMax = std::max(bbox.yMax, corner.y);
                }
                bounds = SkRect::MakeLTRB(SkFDot6ToScalar(bbox.xMin), -SkFDot6ToScalar(bbox.yMax),
                                          SkFDot6ToScalar(bbox.xMax), -SkFDot6ToScalar(bbox.yMin));
            } else {
                // Traverse the glyph graph with a focus on measuring the required bounding box.
                // The call to computeColrV1GlyphBoundingBox may modify the face.
                // Reset the face to load the base glyph for metrics.
                if (!SkScalerContextFTUtils::computeColrV1GlyphBoundingBox(fFace,
                                                                           glyph.getGlyphID(),
                                                                           &bounds) ||
                    this->setupSize())
                {
                    return mx;
                }
            }
        }
#endif // #TT_SUPPORT_COLRV1

        if (!haveLayers) {
            FT_LayerIterator layerIterator = { 0, 0, nullptr };
            FT_UInt layerGlyphIndex;
            FT_UInt layerColorIndex;
            FT_Int32 flags = fLoadGlyphFlags;
            flags |= FT_LOAD_BITMAP_METRICS_ONLY;  // Don't decode any bitmaps.
            flags |= FT_LOAD_NO_BITMAP; // Ignore embedded bitmaps.
            flags &= ~FT_LOAD_RENDER;  // Don't scan convert.
            flags &= ~FT_LOAD_COLOR;  // Ignore SVG.
            // For COLRv0 compute the glyph bounding box from the union of layer bounding boxes.
            while (FT_Get_Color_Glyph_Layer(fFace, glyph.getGlyphID(), &layerGlyphIndex,
                                            &layerColorIndex, &layerIterator)) {
                haveLayers = true;
                if (FT_Load_Glyph(fFace, layerGlyphIndex, flags)) {
                    return mx;
                }

                SkRect currentBounds;
                if (getBoundsOfCurrentOutlineGlyph(fFace->glyph, &currentBounds)) {
                    bounds.join(currentBounds);
                }
            }
            if (haveLayers) {
                mx.extraBits = ScalerContextBits::COLRv0;
            }
        }

        if (haveLayers) {
            mx.maskFormat = SkMask::kARGB32_Format;
            mx.neverRequestPath = true;
            updateGlyphBoundsIfSubpixel(glyph, &bounds, this->isSubpixel());
            mx.bounds = bounds;
        }
    }
#endif  //FT_COLOR_H

    // Even if haveLayers, the base glyph must be loaded to get the metrics.
    if (FT_Load_Glyph(fFace, glyph.getGlyphID(), fLoadGlyphFlags | FT_LOAD_BITMAP_METRICS_ONLY)) {
        return mx;
    }

    if (!haveLayers) {
        emboldenIfNeeded(fFace, fFace->glyph, glyph.getGlyphID());

        if (fFace->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
            getBoundsOfCurrentOutlineGlyph(fFace->glyph, &mx.bounds);
            updateGlyphBoundsIfSubpixel(glyph, &mx.bounds, this->isSubpixel());
            updateGlyphBoundsIfLCD(&mx);

        } else if (fFace->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
            mx.neverRequestPath = true;

            if (this->isVertical()) {
                FT_Vector vector;
                vector.x =  fFace->glyph->metrics.vertBearingX - fFace->glyph->metrics.horiBearingX;
                vector.y = -fFace->glyph->metrics.vertBearingY - fFace->glyph->metrics.horiBearingY;
                FT_Vector_Transform(&vector, &fMatrix22);
                fFace->glyph->bitmap_left += SkFDot6Floor(vector.x);
                fFace->glyph->bitmap_top  += SkFDot6Floor(vector.y);
            }

            if (fFace->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA) {
                mx.maskFormat = SkMask::kARGB32_Format;
            }

            mx.bounds = SkRect::MakeXYWH(SkIntToScalar(fFace->glyph->bitmap_left ),
                                        -SkIntToScalar(fFace->glyph->bitmap_top  ),
                                         SkIntToScalar(fFace->glyph->bitmap.width),
                                         SkIntToScalar(fFace->glyph->bitmap.rows ));
            fMatrix22Scalar.mapRect(&mx.bounds);
            updateGlyphBoundsIfSubpixel(glyph, &mx.bounds,
                                        this->shouldSubpixelBitmap(glyph, fMatrix22Scalar));

#if defined(FT_CONFIG_OPTION_SVG)
        } else if (fFace->glyph->format == FT_GLYPH_FORMAT_SVG) {
            mx.extraBits = ScalerContextBits::SVG;
            mx.maskFormat = SkMask::kARGB32_Format;
            mx.neverRequestPath = true;

            SkPictureRecorder recorder;
            SkRect infiniteRect = SkRect::MakeLTRB(-SK_ScalarInfinity, -SK_ScalarInfinity,
                                                    SK_ScalarInfinity,  SK_ScalarInfinity);
            sk_sp<SkBBoxHierarchy> bboxh = SkRTreeFactory()();
            SkSpan<SkColor> palette(fFaceRec->fSkPalette.get(), fFaceRec->fFTPaletteEntryCount);
            SkCanvas* recordingCanvas = recorder.beginRecording(infiniteRect, bboxh);
            if (!fUtils.drawSVGGlyph(fFace, glyph, fLoadGlyphFlags, palette, recordingCanvas)) {
                return mx;
            }
            sk_sp<SkPicture> pic = recorder.finishRecordingAsPicture();
            mx.bounds = pic->cullRect();
            SkASSERT(mx.bounds.isFinite());
            // drawSVGGlyph already applied the subpixel positioning.
#endif  // FT_CONFIG_OPTION_SVG

        } else {
            SkDEBUGFAIL("unknown glyph format");
            return mx;
        }
    }

    if (this->isVertical()) {
        if (fDoLinearMetrics) {
            const SkScalar advanceScalar = SkFT_FixedToScalar(fFace->glyph->linearVertAdvance);
            mx.advance.fX = fMatrix22Scalar.getSkewX() * advanceScalar;
            mx.advance.fY = fMatrix22Scalar.getScaleY() * advanceScalar;
        } else {
            mx.advance.fX = -SkFDot6ToFloat(fFace->glyph->advance.x);
            mx.advance.fY =  SkFDot6ToFloat(fFace->glyph->advance.y);
        }
    } else {
        if (fDoLinearMetrics) {
            const SkScalar advanceScalar = SkFT_FixedToScalar(fFace->glyph->linearHoriAdvance);
            mx.advance.fX = fMatrix22Scalar.getScaleX() * advanceScalar;
            mx.advance.fY = fMatrix22Scalar.getSkewY() * advanceScalar;
        } else {
            mx.advance.fX =  SkFDot6ToFloat(fFace->glyph->advance.x);
            mx.advance.fY = -SkFDot6ToFloat(fFace->glyph->advance.y);
        }
    }

#ifdef ENABLE_GLYPH_SPEW
    LOG_INFO("Metrics(glyph:%d flags:0x%x) w:%d\n", glyph->getGlyphID(), fLoadGlyphFlags, glyph->width());
#endif
    return mx;
}

void SkScalerContext_FreeType::generateImage(const SkGlyph& glyph, void* imageBuffer) {
    SkAutoMutexExclusive  ac(f_t_mutex());

    if (this->setupSize()) {
        sk_bzero(imageBuffer, glyph.imageSize());
        return;
    }

    if (glyph.extraBits() == ScalerContextBits::COLRv0 ||
        glyph.extraBits() == ScalerContextBits::COLRv1 ||
        glyph.extraBits() == ScalerContextBits::SVG     )
    {
        SkASSERT(glyph.maskFormat() == SkMask::kARGB32_Format);
        SkBitmap dstBitmap;
        // TODO: mark this as sRGB when the blits will be sRGB.
        dstBitmap.setInfo(SkImageInfo::Make(glyph.width(), glyph.height(),
                                            kN32_SkColorType,
                                            kPremul_SkAlphaType),
                                            glyph.rowBytes());
        dstBitmap.setPixels(imageBuffer);

        SkCanvas canvas(dstBitmap);
        if constexpr (kSkShowTextBlitCoverage) {
            canvas.clear(0x33FF0000);
        } else {
            canvas.clear(SK_ColorTRANSPARENT);
        }
        canvas.translate(-glyph.left(), -glyph.top());

        SkSpan<SkColor> palette(fFaceRec->fSkPalette.get(), fFaceRec->fFTPaletteEntryCount);
        if (glyph.extraBits() == ScalerContextBits::COLRv0) {
#ifdef FT_COLOR_H
            fUtils.drawCOLRv0Glyph(fFace, glyph, fLoadGlyphFlags, palette, &canvas);
#endif
        } else if (glyph.extraBits() == ScalerContextBits::COLRv1) {
#ifdef TT_SUPPORT_COLRV1
            fUtils.drawCOLRv1Glyph(fFace, glyph, fLoadGlyphFlags, palette, &canvas);
#endif
        } else if (glyph.extraBits() == ScalerContextBits::SVG) {
#if defined(FT_CONFIG_OPTION_SVG)
            if (FT_Load_Glyph(fFace, glyph.getGlyphID(), fLoadGlyphFlags)) {
                return;
            }
            fUtils.drawSVGGlyph(fFace, glyph, fLoadGlyphFlags, palette, &canvas);
#endif
        }
        return;
    }

    if (FT_Load_Glyph(fFace, glyph.getGlyphID(), fLoadGlyphFlags)) {
        sk_bzero(imageBuffer, glyph.imageSize());
        return;
    }
    emboldenIfNeeded(fFace, fFace->glyph, glyph.getGlyphID());

    SkMatrix* bitmapMatrix = &fMatrix22Scalar;
    SkMatrix subpixelBitmapMatrix;
    if (this->shouldSubpixelBitmap(glyph, *bitmapMatrix)) {
        subpixelBitmapMatrix = fMatrix22Scalar;
        subpixelBitmapMatrix.postTranslate(SkFixedToScalar(glyph.getSubXFixed()),
                                           SkFixedToScalar(glyph.getSubYFixed()));
        bitmapMatrix = &subpixelBitmapMatrix;
    }

    fUtils.generateGlyphImage(fFace, glyph, imageBuffer, *bitmapMatrix, fPreBlend);
}

sk_sp<SkDrawable> SkScalerContext_FreeType::generateDrawable(const SkGlyph& glyph) {
    // Because FreeType's FT_Face is stateful (not thread safe) and the current design of this
    // SkTypeface and SkScalerContext does not work around this, it is necessary lock at least the
    // FT_Face when using it (this implementation currently locks the whole FT_Library).
    // It should be possible to draw the drawable straight out of the FT_Face. However, this would
    // mean locking each time any such drawable is drawn. To avoid locking, this implementation
    // creates drawables backed as pictures so that they can be played back later without locking.
    SkAutoMutexExclusive  ac(f_t_mutex());

    if (this->setupSize()) {
        return nullptr;
    }

#if defined(FT_COLOR_H) || defined(TT_SUPPORT_COLRV1) || defined(FT_CONFIG_OPTION_SVG)
    if (glyph.extraBits() == ScalerContextBits::COLRv0 ||
        glyph.extraBits() == ScalerContextBits::COLRv1 ||
        glyph.extraBits() == ScalerContextBits::SVG     )
    {
        SkSpan<SkColor> palette(fFaceRec->fSkPalette.get(), fFaceRec->fFTPaletteEntryCount);
        SkPictureRecorder recorder;
        SkCanvas* recordingCanvas = recorder.beginRecording(SkRect::Make(glyph.mask().fBounds));
        if (glyph.extraBits() == ScalerContextBits::COLRv0) {
#ifdef FT_COLOR_H
            if (!fUtils.drawCOLRv0Glyph(fFace, glyph, fLoadGlyphFlags, palette, recordingCanvas)) {
                return nullptr;
            }
#else
            return nullptr;
#endif
        } else if (glyph.extraBits() == ScalerContextBits::COLRv1) {
#ifdef TT_SUPPORT_COLRV1
            if (!fUtils.drawCOLRv1Glyph(fFace, glyph, fLoadGlyphFlags, palette, recordingCanvas)) {
                return nullptr;
            }
#else
            return nullptr;
#endif
        } else if (glyph.extraBits() == ScalerContextBits::SVG) {
#if defined(FT_CONFIG_OPTION_SVG)
            if (FT_Load_Glyph(fFace, glyph.getGlyphID(), fLoadGlyphFlags)) {
                return nullptr;
            }
            if (!fUtils.drawSVGGlyph(fFace, glyph, fLoadGlyphFlags, palette, recordingCanvas)) {
                return nullptr;
            }
#else
            return nullptr;
#endif
        }
        return recorder.finishRecordingAsDrawable();
    }
#endif
    return nullptr;
}

bool SkScalerContext_FreeType::generatePath(const SkGlyph& glyph, SkPath* path) {
    SkASSERT(path);

    SkAutoMutexExclusive  ac(f_t_mutex());

    SkGlyphID glyphID = glyph.getGlyphID();
    // FT_IS_SCALABLE is documented to mean the face contains outline glyphs.
    if (!FT_IS_SCALABLE(fFace) || this->setupSize()) {
        path->reset();
        return false;
    }

    uint32_t flags = fLoadGlyphFlags;
    flags |= FT_LOAD_NO_BITMAP; // ignore embedded bitmaps so we're sure to get the outline
    flags &= ~FT_LOAD_RENDER;   // don't scan convert (we just want the outline)

    FT_Error err = FT_Load_Glyph(fFace, glyphID, flags);
    if (err != 0 || fFace->glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
        path->reset();
        return false;
    }
    emboldenIfNeeded(fFace, fFace->glyph, glyphID);

    if (!fUtils.generateGlyphPath(fFace, path)) {
        path->reset();
        return false;
    }

    // The path's origin from FreeType is always the horizontal layout origin.
    // Offset the path so that it is relative to the vertical origin if needed.
    if (this->isVertical()) {
        FT_Vector vector;
        vector.x = fFace->glyph->metrics.vertBearingX - fFace->glyph->metrics.horiBearingX;
        vector.y = -fFace->glyph->metrics.vertBearingY - fFace->glyph->metrics.horiBearingY;
        FT_Vector_Transform(&vector, &fMatrix22);
        path->offset(SkFDot6ToScalar(vector.x), -SkFDot6ToScalar(vector.y));
    }
    return true;
}

void SkScalerContext_FreeType::generateFontMetrics(SkFontMetrics* metrics) {
    if (nullptr == metrics) {
        return;
    }

    SkAutoMutexExclusive ac(f_t_mutex());

    if (this->setupSize()) {
        sk_bzero(metrics, sizeof(*metrics));
        return;
    }

    FT_Face face = fFace;
    metrics->fFlags = 0;

    SkScalar upem = SkIntToScalar(SkTypeface_FreeType::GetUnitsPerEm(face));

    // use the os/2 table as a source of reasonable defaults.
    SkScalar x_height = 0.0f;
    SkScalar avgCharWidth = 0.0f;
    SkScalar cap_height = 0.0f;
    SkScalar strikeoutThickness = 0.0f, strikeoutPosition = 0.0f;
    TT_OS2* os2 = (TT_OS2*) FT_Get_Sfnt_Table(face, ft_sfnt_os2);
    if (os2) {
        x_height = SkIntToScalar(os2->sxHeight) / upem * fScale.y();
        avgCharWidth = SkIntToScalar(os2->xAvgCharWidth) / upem;
        strikeoutThickness = SkIntToScalar(os2->yStrikeoutSize) / upem;
        strikeoutPosition = -SkIntToScalar(os2->yStrikeoutPosition) / upem;
        metrics->fFlags |= SkFontMetrics::kStrikeoutThicknessIsValid_Flag;
        metrics->fFlags |= SkFontMetrics::kStrikeoutPositionIsValid_Flag;
        if (os2->version != 0xFFFF && os2->version >= 2) {
            cap_height = SkIntToScalar(os2->sCapHeight) / upem * fScale.y();
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

        metrics->fFlags |= SkFontMetrics::kUnderlineThicknessIsValid_Flag;
        metrics->fFlags |= SkFontMetrics::kUnderlinePositionIsValid_Flag;

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
        leading = (SkIntToScalar(face->size->metrics.height) / (yppem * 64.0f)) + ascent - descent;

        xmin = 0.0f;
        xmax = SkIntToScalar(face->available_sizes[fStrikeIndex].width) / xppem;
        ymin = descent;
        ymax = ascent;
        // The actual bitmaps may be any size and placed at any offset.
        metrics->fFlags |= SkFontMetrics::kBoundsInvalid_Flag;

        underlineThickness = 0;
        underlinePosition = 0;
        metrics->fFlags &= ~SkFontMetrics::kUnderlineThicknessIsValid_Flag;
        metrics->fFlags &= ~SkFontMetrics::kUnderlinePositionIsValid_Flag;

        TT_Postscript* post = (TT_Postscript*) FT_Get_Sfnt_Table(face, ft_sfnt_post);
        if (post) {
            underlineThickness = SkIntToScalar(post->underlineThickness) / upem;
            underlinePosition = -SkIntToScalar(post->underlinePosition) / upem;
            metrics->fFlags |= SkFontMetrics::kUnderlineThicknessIsValid_Flag;
            metrics->fFlags |= SkFontMetrics::kUnderlinePositionIsValid_Flag;
        }
    } else {
        sk_bzero(metrics, sizeof(*metrics));
        return;
    }

    // synthesize elements that were not provided by the os/2 table or format-specific metrics
    if (!x_height) {
        x_height = -ascent * fScale.y();
    }
    if (!avgCharWidth) {
        avgCharWidth = xmax - xmin;
    }
    if (!cap_height) {
      cap_height = -ascent * fScale.y();
    }

    // disallow negative linespacing
    if (leading < 0.0f) {
        leading = 0.0f;
    }

    metrics->fTop = ymax * fScale.y();
    metrics->fAscent = ascent * fScale.y();
    metrics->fDescent = descent * fScale.y();
    metrics->fBottom = ymin * fScale.y();
    metrics->fLeading = leading * fScale.y();
    metrics->fAvgCharWidth = avgCharWidth * fScale.y();
    metrics->fXMin = xmin * fScale.y();
    metrics->fXMax = xmax * fScale.y();
    metrics->fMaxCharWidth = metrics->fXMax - metrics->fXMin;
    metrics->fXHeight = x_height;
    metrics->fCapHeight = cap_height;
    metrics->fUnderlineThickness = underlineThickness * fScale.y();
    metrics->fUnderlinePosition = underlinePosition * fScale.y();
    metrics->fStrikeoutThickness = strikeoutThickness * fScale.y();
    metrics->fStrikeoutPosition = strikeoutPosition * fScale.y();

    if (face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS
#if defined(FT_CONFIG_OPTION_SVG)
        || face->face_flags & FT_FACE_FLAG_SVG
#endif  // FT_CONFIG_OPTION_SVG
    ) {
        // The bounds are only valid for the default variation of variable glyphs.
        // https://docs.microsoft.com/en-us/typography/opentype/spec/head
        // For SVG glyphs this number is often incorrect for its non-`glyf` points.
        // https://github.com/fonttools/fonttools/issues/2566
        metrics->fFlags |= SkFontMetrics::kBoundsInvalid_Flag;
    }
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

void SkScalerContext_FreeType::emboldenIfNeeded(FT_Face face, FT_GlyphSlot glyph, SkGlyphID gid) {
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
            if (!fFace->glyph->bitmap.buffer) {
                FT_Load_Glyph(fFace, gid, fLoadGlyphFlags);
            }
            FT_GlyphSlot_Own_Bitmap(glyph);
            FT_Bitmap_Embolden(glyph->library, &glyph->bitmap, kBitmapEmboldenStrength, 0);
            break;
        default:
            SkDEBUGFAIL("unknown glyph format");
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "src/base/SkUtils.h"

SkTypeface_FreeType::SkTypeface_FreeType(const SkFontStyle& style, bool isFixedPitch)
    : INHERITED(style, isFixedPitch)
{}

SkTypeface_FreeType::~SkTypeface_FreeType() {
    if (fFaceRec) {
        SkAutoMutexExclusive ac(f_t_mutex());
        fFaceRec.reset();
    }
}

// Just made up, so we don't end up storing 1000s of entries
constexpr int kMaxC2GCacheCount = 512;

void SkTypeface_FreeType::onCharsToGlyphs(const SkUnichar uni[], int count,
                                          SkGlyphID glyphs[]) const {
    // Try the cache first, *before* accessing freetype lib/face, as that
    // can be very slow. If we do need to compute a new glyphID, then
    // access those freetype objects and continue the loop.

    int i;
    {
        // Optimistically use a shared lock.
        SkAutoSharedMutexShared ama(fC2GCacheMutex);
        for (i = 0; i < count; ++i) {
            int index = fC2GCache.findGlyphIndex(uni[i]);
            if (index < 0) {
                break;
            }
            glyphs[i] = SkToU16(index);
        }
        if (i == count) {
            // we're done, no need to access the freetype objects
            return;
        }
    }

    // Need to add more so grab an exclusive lock.
    SkAutoSharedMutexExclusive ama(fC2GCacheMutex);
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        sk_bzero(glyphs, count * sizeof(glyphs[0]));
        return;
    }

    for (; i < count; ++i) {
        SkUnichar c = uni[i];
        int index = fC2GCache.findGlyphIndex(c);
        if (index >= 0) {
            glyphs[i] = SkToU16(index);
        } else {
            glyphs[i] = SkToU16(FT_Get_Char_Index(face, c));
            fC2GCache.insertCharAndGlyph(~index, c, glyphs[i]);
        }
    }

    if (fC2GCache.count() > kMaxC2GCacheCount) {
        fC2GCache.reset();
    }
}

int SkTypeface_FreeType::onCountGlyphs() const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    return face ? face->num_glyphs : 0;
}

SkTypeface::LocalizedStrings* SkTypeface_FreeType::onCreateFamilyNameIterator() const {
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

bool SkTypeface_FreeType::onGlyphMaskNeedsCurrentColor() const {
    fGlyphMasksMayNeedCurrentColorOnce([this]{
        static constexpr SkFourByteTag COLRTag = SkSetFourByteTag('C', 'O', 'L', 'R');
        fGlyphMasksMayNeedCurrentColor = this->getTableSize(COLRTag) > 0;
#if defined(FT_CONFIG_OPTION_SVG)
        static constexpr SkFourByteTag SVGTag = SkSetFourByteTag('S', 'V', 'G', ' ');
        fGlyphMasksMayNeedCurrentColor |= this->getTableSize(SVGTag) > 0 ;
#endif  // FT_CONFIG_OPTION_SVG
    });
    return fGlyphMasksMayNeedCurrentColor;
}

int SkTypeface_FreeType::onGetVariationDesignPosition(
    SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount) const
{
    AutoFTAccess fta(this);
    return GetVariationDesignPosition(fta, coordinates, coordinateCount);
}

int SkTypeface_FreeType::onGetVariationDesignParameters(
    SkFontParameters::Variation::Axis parameters[], int parameterCount) const
{
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return -1;
    }

    if (!(face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS)) {
        return 0;
    }

    FT_MM_Var* variations = nullptr;
    if (FT_Get_MM_Var(face, &variations)) {
        return -1;
    }
    UniqueVoidPtr autoFreeVariations(variations);

    if (!parameters || parameterCount < SkToInt(variations->num_axis)) {
        return variations->num_axis;
    }

    for (FT_UInt i = 0; i < variations->num_axis; ++i) {
        parameters[i].tag = variations->axis[i].tag;
        parameters[i].min = SkFixedToScalar(variations->axis[i].minimum);
        parameters[i].def = SkFixedToScalar(variations->axis[i].def);
        parameters[i].max = SkFixedToScalar(variations->axis[i].maximum);
        FT_UInt flags = 0;
        bool hidden = !FT_Get_Var_Axis_Flags(variations, i, &flags) &&
                      (flags & FT_VAR_AXIS_FLAG_HIDDEN);
        parameters[i].setHidden(hidden);
    }

    return variations->num_axis;
}

int SkTypeface_FreeType::onGetTableTags(SkFontTableTag tags[]) const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return 0;
    }

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
    if (!face) {
        return 0;
    }

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
    FT_ULong size = std::min((FT_ULong)length, tableLength - (FT_ULong)offset);
    if (data) {
        error = FT_Load_Sfnt_Table(face, tag, offset, reinterpret_cast<FT_Byte*>(data), &size);
        if (error) {
            return 0;
        }
    }

    return size;
}

sk_sp<SkData> SkTypeface_FreeType::onCopyTableData(SkFontTableTag tag) const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return nullptr;
    }

    FT_ULong tableLength = 0;
    FT_Error error;

    // When 'length' is 0 it is overwritten with the full table length; 'offset' is ignored.
    error = FT_Load_Sfnt_Table(face, tag, 0, nullptr, &tableLength);
    if (error) {
        return nullptr;
    }

    sk_sp<SkData> data = SkData::MakeUninitialized(tableLength);
    if (data) {
        error = FT_Load_Sfnt_Table(face, tag, 0,
                                   reinterpret_cast<FT_Byte*>(data->writable_data()), &tableLength);
        if (error) {
            data.reset();
        }
    }
    return data;
}

SkTypeface_FreeType::FaceRec* SkTypeface_FreeType::getFaceRec() const {
    f_t_mutex().assertHeld();
    fFTFaceOnce([this]{ fFaceRec = SkTypeface_FreeType::FaceRec::Make(this); });
    return fFaceRec.get();
}

std::unique_ptr<SkFontData> SkTypeface_FreeType::makeFontData() const {
    return this->onMakeFontData();
}

void SkTypeface_FreeType::FontDataPaletteToDescriptorPalette(const SkFontData& fontData,
                                                             SkFontDescriptor* desc) {
    desc->setPaletteIndex(fontData.getPaletteIndex());
    int paletteOverrideCount = fontData.getPaletteOverrideCount();
    auto overrides = desc->setPaletteEntryOverrides(paletteOverrideCount);
    for (int i = 0; i < paletteOverrideCount; ++i) {
        overrides[i] = fontData.getPaletteOverrides()[i];
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkTypeface_FreeTypeStream::SkTypeface_FreeTypeStream(std::unique_ptr<SkFontData> fontData,
                                                     const SkString familyName,
                                                     const SkFontStyle& style, bool isFixedPitch)
    : SkTypeface_FreeType(style, isFixedPitch)
    , fFamilyName(std::move(familyName))
    , fData(std::move(fontData))
{ }

SkTypeface_FreeTypeStream::~SkTypeface_FreeTypeStream() {}

void SkTypeface_FreeTypeStream::onGetFamilyName(SkString* familyName) const {
    *familyName = fFamilyName;
}

std::unique_ptr<SkStreamAsset> SkTypeface_FreeTypeStream::onOpenStream(int* ttcIndex) const {
    *ttcIndex = fData->getIndex();
    return fData->getStream()->duplicate();
}

std::unique_ptr<SkFontData> SkTypeface_FreeTypeStream::onMakeFontData() const {
    return std::make_unique<SkFontData>(*fData);
}

sk_sp<SkTypeface> SkTypeface_FreeTypeStream::onMakeClone(const SkFontArguments& args) const {
    SkFontStyle style = this->fontStyle();
    std::unique_ptr<SkFontData> data = this->cloneFontData(args, &style);
    if (!data) {
        return nullptr;
    }

    SkString familyName;
    this->getFamilyName(&familyName);

    return sk_make_sp<SkTypeface_FreeTypeStream>(
        std::move(data), familyName, style, this->isFixedPitch());
}

void SkTypeface_FreeTypeStream::onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const {
    desc->setFamilyName(fFamilyName.c_str());
    desc->setStyle(this->fontStyle());
    desc->setFactoryId(SkTypeface_FreeType::FactoryId);
    SkTypeface_FreeType::FontDataPaletteToDescriptorPalette(*fData, desc);
    *serialize = true;
}

sk_sp<SkTypeface> SkTypeface_FreeType::MakeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                                      const SkFontArguments& args) {
    static SkFontScanner_FreeType scanner;
    bool isFixedPitch;
    SkFontStyle style;
    SkString name;
    SkFontScanner::AxisDefinitions axisDefinitions;
    if (!scanner.scanInstance(stream.get(), args.getCollectionIndex(), 0,
                              &name, &style, &isFixedPitch, &axisDefinitions)) {
        return nullptr;
    }

    const SkFontArguments::VariationPosition position = args.getVariationDesignPosition();
    AutoSTMalloc<4, SkFixed> axisValues(axisDefinitions.size());
    SkFontScanner_FreeType::computeAxisValues(axisDefinitions, position, axisValues, name, &style);

    auto data = std::make_unique<SkFontData>(
        std::move(stream), args.getCollectionIndex(), args.getPalette().index,
        axisValues.get(), axisDefinitions.size(),
        args.getPalette().overrides, args.getPalette().overrideCount);
    return sk_make_sp<SkTypeface_FreeTypeStream>(std::move(data), name, style, isFixedPitch);
}

SkFontScanner_FreeType::SkFontScanner_FreeType() : fLibrary(nullptr) {
    if (FT_New_Library(&gFTMemory, &fLibrary)) {
        return;
    }
    FT_Add_Default_Modules(fLibrary);
    FT_Set_Default_Properties(fLibrary);
}

SkFontScanner_FreeType::~SkFontScanner_FreeType() {
    if (fLibrary) {
        FT_Done_Library(fLibrary);
    }
}

FT_Face SkFontScanner_FreeType::openFace(SkStreamAsset* stream, int ttcIndex,
                                         FT_Stream ftStream) const
{
    if (fLibrary == nullptr || stream == nullptr) {
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

bool SkFontScanner_FreeType::scanFile(SkStreamAsset* stream, int* numFaces) const {
    SkAutoMutexExclusive libraryLock(fLibraryMutex);

    FT_StreamRec streamRec;
    SkUniqueFTFace face(this->openFace(stream, -1, &streamRec));
    if (!face) {
        return false;
    }

    *numFaces = face->num_faces;
    return true;
}

bool SkFontScanner_FreeType::scanFace(SkStreamAsset* stream,
                                      int faceIndex,
                                      int* numInstances) const {
    SkAutoMutexExclusive libraryLock(fLibraryMutex);

    FT_StreamRec streamRec;
    SkUniqueFTFace face(this->openFace(stream, -(faceIndex + 1), &streamRec));
    if (!face) {
        return false;
    }

    *numInstances = face->style_flags >> 16;
    return true;
}

bool SkFontScanner_FreeType::scanInstance(SkStreamAsset* stream,
                                          int faceIndex,
                                          int instanceIndex,
                                          SkString* name,
                                          SkFontStyle* style,
                                          bool* isFixedPitch,
                                          AxisDefinitions* axes) const {

    SkAutoMutexExclusive libraryLock(fLibraryMutex);

    FT_StreamRec streamRec;
    SkUniqueFTFace face(this->openFace(stream, (instanceIndex << 16) + faceIndex, &streamRec));
    if (!face) {
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

    bool hasAxes = face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS;
    TT_OS2* os2 = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(face.get(), ft_sfnt_os2));
    bool hasOs2 = os2 && os2->version != 0xffff;

    PS_FontInfoRec psFontInfo;

    if (hasOs2) {
        weight = os2->usWeightClass;
        width = os2->usWidthClass;

        // OS/2::fsSelection bit 9 indicates oblique.
        if (SkToBool(os2->fsSelection & (1u << 9))) {
            slant = SkFontStyle::kOblique_Slant;
        }
    }

    // Let variable axes override properties from the OS/2 table.
    if (hasAxes) {
        AxisDefinitions axisDefinitions;
        if (GetAxes(face.get(), &axisDefinitions)) {
            size_t numAxes = axisDefinitions.size();
            static constexpr SkFourByteTag wghtTag = SkSetFourByteTag('w', 'g', 'h', 't');
            static constexpr SkFourByteTag wdthTag = SkSetFourByteTag('w', 'd', 't', 'h');
            static constexpr SkFourByteTag slntTag = SkSetFourByteTag('s', 'l', 'n', 't');
            std::optional<size_t> wghtIndex;
            std::optional<size_t> wdthIndex;
            std::optional<size_t> slntIndex;
            for(size_t i = 0; i < numAxes; ++i) {
                if (axisDefinitions[i].fTag == wghtTag) {
                    // Rough validity check, sufficient spread and ranges within 0-1000.
                    SkScalar wghtRange = axisDefinitions[i].fMaximum - axisDefinitions[i].fMinimum;
                    if (wghtRange > 5 && wghtRange <= 1000 && axisDefinitions[i].fMaximum <= 1000) {
                        wghtIndex = i;
                    }
                }
                if (axisDefinitions[i].fTag == wdthTag) {
                    // Rough validity check, sufficient spread and are ranges within 0-500.
                    SkScalar wdthRange = axisDefinitions[i].fMaximum - axisDefinitions[i].fMinimum;
                    if (wdthRange > 0 && wdthRange <= 500 && axisDefinitions[i].fMaximum <= 500) {
                        wdthIndex = i;
                    }
                }
                if (axisDefinitions[i].fTag == slntTag) {
                    slntIndex = i;
                }
            }
            AutoSTMalloc<4, FT_Fixed> coords(numAxes);
            if ((wghtIndex || wdthIndex || slntIndex) &&
                !FT_Get_Var_Design_Coordinates(face.get(), numAxes, coords.get())) {
                if (wghtIndex) {
                    SkASSERT(*wghtIndex < numAxes);
                    weight = SkFixedRoundToInt(coords[*wghtIndex]);
                }
                if (wdthIndex) {
                    SkASSERT(*wdthIndex < numAxes);
                    SkScalar wdthValue = SkFixedToScalar(coords[*wdthIndex]);
                    width = SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(wdthValue);
                }
                if (slntIndex) {
                    SkASSERT(*slntIndex < numAxes);
                    // https://docs.microsoft.com/en-us/typography/opentype/spec/dvaraxistag_slnt
                    // "Scale interpretation: Values can be interpreted as the angle,
                    // in counter-clockwise degrees, of oblique slant from whatever
                    // the designer considers to be upright for that font design."
                    if (SkFixedToScalar(coords[*slntIndex]) < 0) {
                        slant = SkFontStyle::kOblique_Slant;
                    }
                }
            }
        }
    }

    if (!hasOs2 && !hasAxes && 0 == FT_Get_PS_Font_Info(face.get(), &psFontInfo) && psFontInfo.weight) {
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
                { "ultrablack", SkFontStyle::kExtraBlack_Weight },
                { "ultrabold", SkFontStyle::kExtraBold_Weight },
                { "ultraheavy", SkFontStyle::kExtraBlack_Weight },
                { "ultralight", SkFontStyle::kExtraLight_Weight },
        };
        int const index = SkStrLCSearch(&commonWeights[0].name, std::size(commonWeights),
                                        psFontInfo.weight, sizeof(commonWeights[0]));
        if (index >= 0) {
            weight = commonWeights[index].weight;
        } else {
            LOG_INFO("Do not know weight for: %s (%s) \n", face->family_name, psFontInfo.weight);
        }
    }

    if (name != nullptr) {
        name->set(face->family_name);
    }
    if (style != nullptr) {
        *style = SkFontStyle(weight, width, slant);
    }
    if (isFixedPitch != nullptr) {
        *isFixedPitch = FT_IS_FIXED_WIDTH(face);
    }

    if (axes != nullptr && !GetAxes(face.get(), axes)) {
        return false;
    }
    return true;
}

bool SkFontScanner_FreeType::GetAxes(FT_Face face, AxisDefinitions* axes) {
    SkASSERT(face && axes);
    if (face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS) {
        FT_MM_Var* variations = nullptr;
        FT_Error err = FT_Get_MM_Var(face, &variations);
        if (err) {
            LOG_INFO("INFO: font %s claims to have variations, but none found.\n",
                     face->family_name);
            return false;
        }
        UniqueVoidPtr autoFreeVariations(variations);

        axes->reset(variations->num_axis);
        for (FT_UInt i = 0; i < variations->num_axis; ++i) {
            const FT_Var_Axis& ftAxis = variations->axis[i];
            (*axes)[i].fTag = ftAxis.tag;
            (*axes)[i].fMinimum = SkFT_FixedToScalar(ftAxis.minimum);
            (*axes)[i].fDefault = SkFT_FixedToScalar(ftAxis.def);
            (*axes)[i].fMaximum = SkFT_FixedToScalar(ftAxis.maximum);
        }
    }
    return true;
}

/*static*/ void SkFontScanner_FreeType::computeAxisValues(
        AxisDefinitions axisDefinitions,
        const SkFontArguments::VariationPosition position,
        SkFixed* axisValues,
        const SkString& name,
        SkFontStyle* style,
        const SkFontArguments::VariationPosition::Coordinate* current)
{
    static constexpr SkFourByteTag wghtTag = SkSetFourByteTag('w', 'g', 'h', 't');
    static constexpr SkFourByteTag wdthTag = SkSetFourByteTag('w', 'd', 't', 'h');
    static constexpr SkFourByteTag slntTag = SkSetFourByteTag('s', 'l', 'n', 't');
    int weight = SkFontStyle::kNormal_Weight;
    int width = SkFontStyle::kNormal_Width;
    SkFontStyle::Slant slant = SkFontStyle::kUpright_Slant;
    if (style) {
        weight = style->weight();
        width = style->width();
        slant = style->slant();
    }

    for (int i = 0; i < axisDefinitions.size(); ++i) {
        const AxisDefinition& axisDefinition = axisDefinitions[i];
        const SkScalar axisMin = axisDefinition.fMinimum;
        const SkScalar axisMax = axisDefinition.fMaximum;

        // Start with the default value.
        axisValues[i] = SkScalarToFixed(axisDefinition.fDefault);

        // Then the current value.
        if (current) {
            for (int j = 0; j < axisDefinitions.size(); ++j) {
                const auto& coordinate = current[j];
                if (axisDefinition.fTag == coordinate.axis) {
                    const SkScalar axisValue = SkTPin(coordinate.value, axisMin, axisMax);
                    axisValues[i] = SkScalarToFixed(axisValue);
                    break;
                }
            }
        }

        // Then the requested value.
        // The position may be over specified. If there are multiple values for a given axis,
        // use the last one since that's what css-fonts-4 requires.
        for (int j = position.coordinateCount; j --> 0;) {
            const auto& coordinate = position.coordinates[j];
            if (axisDefinition.fTag == coordinate.axis) {
                const SkScalar axisValue = SkTPin(coordinate.value, axisMin, axisMax);
                if (coordinate.value != axisValue) {
                    LOG_INFO("Requested font axis value out of range: "
                            "%s '%c%c%c%c' %f; pinned to %f.\n",
                            name.c_str(),
                            (axisDefinition.fTag >> 24) & 0xFF,
                            (axisDefinition.fTag >> 16) & 0xFF,
                            (axisDefinition.fTag >>  8) & 0xFF,
                            (axisDefinition.fTag      ) & 0xFF,
                            SkScalarToDouble(coordinate.value),
                            SkScalarToDouble(axisValue));
                }
                axisValues[i] = SkScalarToFixed(axisValue);
                break;
            }
        }

        if (style) {
            if (axisDefinition.fTag == wghtTag) {
                // Rough validity check, is there sufficient spread and are ranges within 0-1000.
                SkScalar wghtRange = axisMax - axisMin;
                if (wghtRange > 5 && wghtRange <= 1000 && axisMax <= 1000) {
                    weight = SkFixedRoundToInt(axisValues[i]);
                }
            }
            if (axisDefinition.fTag == wdthTag) {
                // Rough validity check, is there a spread and are ranges within 0-500.
                SkScalar wdthRange = axisMax - axisMin;
                if (wdthRange > 0 && wdthRange <= 500 && axisMax <= 500) {
                    SkScalar wdthValue = SkFixedToScalar(axisValues[i]);
                    width = SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(wdthValue);
                }
            }
            if (axisDefinition.fTag == slntTag && slant != SkFontStyle::kItalic_Slant) {
                // https://docs.microsoft.com/en-us/typography/opentype/spec/dvaraxistag_slnt
                // "Scale interpretation: Values can be interpreted as the angle,
                // in counter-clockwise degrees, of oblique slant from whatever
                // the designer considers to be upright for that font design."
                if (axisValues[i] == 0) {
                    slant = SkFontStyle::kUpright_Slant;
                } else {
                    slant = SkFontStyle::kOblique_Slant;
                }
            }
        }
        // TODO: warn on defaulted axis?
    }

    if (style) {
        *style = SkFontStyle(weight, width, slant);
    }

    SkDEBUGCODE(
        // Check for axis specified, but not matched in font.
        for (int i = 0; i < position.coordinateCount; ++i) {
            SkFourByteTag skTag = position.coordinates[i].axis;
            bool found = false;
            for (int j = 0; j < axisDefinitions.size(); ++j) {
                if (skTag == axisDefinitions[j].fTag) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                LOG_INFO("Requested font axis not found: %s '%c%c%c%c'\n",
                            name.c_str(),
                            (skTag >> 24) & 0xFF,
                            (skTag >> 16) & 0xFF,
                            (skTag >>  8) & 0xFF,
                            (skTag)       & 0xFF);
            }
        }
    )
}
