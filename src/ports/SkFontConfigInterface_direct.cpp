/*
 * Copyright 2009-2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* migrated from chrome/src/skia/ext/SkFontHost_fontconfig_direct.cpp */

#include "include/core/SkFontStyle.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkFixed.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkBuffer.h"
#include "src/ports/SkFontConfigInterface_direct.h"

#include <fontconfig/fontconfig.h>
#include <unistd.h>

#ifdef SK_DEBUG
#    include "src/core/SkTLS.h"
#endif

namespace {

// Fontconfig is not threadsafe before 2.10.91. Before that, we lock with a global mutex.
// See https://bug.skia.org/1497 for background.
static SkMutex& f_c_mutex() {
    static SkMutex& mutex = *(new SkMutex);
    return mutex;
}

#ifdef SK_DEBUG
void* CreateThreadFcLocked() { return new bool(false); }
void DeleteThreadFcLocked(void* v) { delete static_cast<bool*>(v); }
#   define THREAD_FC_LOCKED \
        static_cast<bool*>(SkTLS::Get(CreateThreadFcLocked, DeleteThreadFcLocked))
#endif

struct FCLocker {
    // Assume FcGetVersion() has always been thread safe.

    FCLocker() {
        if (FcGetVersion() < 21091) {
            f_c_mutex().acquire();
        } else {
            SkDEBUGCODE(bool* threadLocked = THREAD_FC_LOCKED);
            SkASSERT(false == *threadLocked);
            SkDEBUGCODE(*threadLocked = true);
        }
    }

    ~FCLocker() {
        AssertHeld();
        if (FcGetVersion() < 21091) {
            f_c_mutex().release();
        } else {
            SkDEBUGCODE(*THREAD_FC_LOCKED = false);
        }
    }

    static void AssertHeld() { SkDEBUGCODE(
        if (FcGetVersion() < 21091) {
            f_c_mutex().assertHeld();
        } else {
            SkASSERT(true == *THREAD_FC_LOCKED);
        }
    ) }
};

} // namespace

size_t SkFontConfigInterface::FontIdentity::writeToMemory(void* addr) const {
    size_t size = sizeof(fID) + sizeof(fTTCIndex);
    size += sizeof(int32_t) + sizeof(int32_t) + sizeof(uint8_t); // weight, width, italic
    size += sizeof(int32_t) + fString.size();    // store length+data
    if (addr) {
        SkWBuffer buffer(addr, size);

        buffer.write32(fID);
        buffer.write32(fTTCIndex);
        buffer.write32(fString.size());
        buffer.write32(fStyle.weight());
        buffer.write32(fStyle.width());
        buffer.write8(fStyle.slant());
        buffer.write(fString.c_str(), fString.size());
        buffer.padToAlign4();

        SkASSERT(buffer.pos() == size);
    }
    return size;
}

size_t SkFontConfigInterface::FontIdentity::readFromMemory(const void* addr,
                                                           size_t size) {
    SkRBuffer buffer(addr, size);

    (void)buffer.readU32(&fID);
    (void)buffer.readS32(&fTTCIndex);
    uint32_t strLen, weight, width;
    (void)buffer.readU32(&strLen);
    (void)buffer.readU32(&weight);
    (void)buffer.readU32(&width);
    uint8_t u8;
    (void)buffer.readU8(&u8);
    SkFontStyle::Slant slant = (SkFontStyle::Slant)u8;
    fStyle = SkFontStyle(weight, width, slant);
    fString.resize(strLen);
    (void)buffer.read(fString.writable_str(), strLen);
    buffer.skipToAlign4();

    return buffer.pos();    // the actual number of bytes read
}

#ifdef SK_DEBUG
static void make_iden(SkFontConfigInterface::FontIdentity* iden) {
    iden->fID = 10;
    iden->fTTCIndex = 2;
    iden->fString.set("Hello world");
    iden->fStyle = SkFontStyle(300, 6, SkFontStyle::kItalic_Slant);
}

static void test_writeToMemory(const SkFontConfigInterface::FontIdentity& iden0,
                               int initValue) {
    SkFontConfigInterface::FontIdentity iden1;

    size_t size0 = iden0.writeToMemory(nullptr);

    SkAutoMalloc storage(size0);
    memset(storage.get(), initValue, size0);

    size_t size1 = iden0.writeToMemory(storage.get());
    SkASSERT(size0 == size1);

    SkASSERT(iden0 != iden1);
    size_t size2 = iden1.readFromMemory(storage.get(), size1);
    SkASSERT(size2 == size1);
    SkASSERT(iden0 == iden1);
}

static void fontconfiginterface_unittest() {
    SkFontConfigInterface::FontIdentity iden0, iden1;

    SkASSERT(iden0 == iden1);

    make_iden(&iden0);
    SkASSERT(iden0 != iden1);

    make_iden(&iden1);
    SkASSERT(iden0 == iden1);

    test_writeToMemory(iden0, 0);
    test_writeToMemory(iden0, 0);
}
#endif

///////////////////////////////////////////////////////////////////////////////

// Returns the string from the pattern, or nullptr
static const char* get_string(FcPattern* pattern, const char field[], int index = 0) {
    const char* name;
    if (FcPatternGetString(pattern, field, index, (FcChar8**)&name) != FcResultMatch) {
        name = nullptr;
    }
    return name;
}

///////////////////////////////////////////////////////////////////////////////

namespace {

// Equivalence classes, used to match the Liberation and other fonts
// with their metric-compatible replacements.  See the discussion in
// GetFontEquivClass().
enum FontEquivClass
{
    OTHER,
    SANS,
    SERIF,
    MONO,
    SYMBOL,
    PGOTHIC,
    GOTHIC,
    PMINCHO,
    MINCHO,
    SIMSUN,
    NSIMSUN,
    SIMHEI,
    PMINGLIU,
    MINGLIU,
    PMINGLIUHK,
    MINGLIUHK,
    CAMBRIA,
    CALIBRI,
};

// Match the font name against a whilelist of fonts, returning the equivalence
// class.
FontEquivClass GetFontEquivClass(const char* fontname)
{
    // It would be nice for fontconfig to tell us whether a given suggested
    // replacement is a "strong" match (that is, an equivalent font) or
    // a "weak" match (that is, fontconfig's next-best attempt at finding a
    // substitute).  However, I played around with the fontconfig API for
    // a good few hours and could not make it reveal this information.
    //
    // So instead, we hardcode.  Initially this function emulated
    //   /etc/fonts/conf.d/30-metric-aliases.conf
    // from my Ubuntu system, but we're better off being very conservative.

    // Arimo, Tinos and Cousine are a set of fonts metric-compatible with
    // Arial, Times New Roman and Courier New  with a character repertoire
    // much larger than Liberation. Note that Cousine is metrically
    // compatible with Courier New, but the former is sans-serif while
    // the latter is serif.


    struct FontEquivMap {
        FontEquivClass clazz;
        const char name[40];
    };

    static const FontEquivMap kFontEquivMap[] = {
        { SANS, "Arial" },
        { SANS, "Arimo" },
        { SANS, "Liberation Sans" },

        { SERIF, "Times New Roman" },
        { SERIF, "Tinos" },
        { SERIF, "Liberation Serif" },

        { MONO, "Courier New" },
        { MONO, "Cousine" },
        { MONO, "Liberation Mono" },

        { SYMBOL, "Symbol" },
        { SYMBOL, "Symbol Neu" },

        // ＭＳ Ｐゴシック
        { PGOTHIC, "MS PGothic" },
        { PGOTHIC, "\xef\xbc\xad\xef\xbc\xb3 \xef\xbc\xb0"
                   "\xe3\x82\xb4\xe3\x82\xb7\xe3\x83\x83\xe3\x82\xaf" },
        { PGOTHIC, "Noto Sans CJK JP" },
        { PGOTHIC, "IPAPGothic" },
        { PGOTHIC, "MotoyaG04Gothic" },

        // ＭＳ ゴシック
        { GOTHIC, "MS Gothic" },
        { GOTHIC, "\xef\xbc\xad\xef\xbc\xb3 "
                  "\xe3\x82\xb4\xe3\x82\xb7\xe3\x83\x83\xe3\x82\xaf" },
        { GOTHIC, "Noto Sans Mono CJK JP" },
        { GOTHIC, "IPAGothic" },
        { GOTHIC, "MotoyaG04GothicMono" },

        // ＭＳ Ｐ明朝
        { PMINCHO, "MS PMincho" },
        { PMINCHO, "\xef\xbc\xad\xef\xbc\xb3 \xef\xbc\xb0"
                   "\xe6\x98\x8e\xe6\x9c\x9d"},
        { PMINCHO, "Noto Serif CJK JP" },
        { PMINCHO, "IPAPMincho" },
        { PMINCHO, "MotoyaG04Mincho" },

        // ＭＳ 明朝
        { MINCHO, "MS Mincho" },
        { MINCHO, "\xef\xbc\xad\xef\xbc\xb3 \xe6\x98\x8e\xe6\x9c\x9d" },
        { MINCHO, "Noto Serif CJK JP" },
        { MINCHO, "IPAMincho" },
        { MINCHO, "MotoyaG04MinchoMono" },

        // 宋体
        { SIMSUN, "Simsun" },
        { SIMSUN, "\xe5\xae\x8b\xe4\xbd\x93" },
        { SIMSUN, "Noto Serif CJK SC" },
        { SIMSUN, "MSung GB18030" },
        { SIMSUN, "Song ASC" },

        // 新宋体
        { NSIMSUN, "NSimsun" },
        { NSIMSUN, "\xe6\x96\xb0\xe5\xae\x8b\xe4\xbd\x93" },
        { NSIMSUN, "Noto Serif CJK SC" },
        { NSIMSUN, "MSung GB18030" },
        { NSIMSUN, "N Song ASC" },

        // 黑体
        { SIMHEI, "Simhei" },
        { SIMHEI, "\xe9\xbb\x91\xe4\xbd\x93" },
        { SIMHEI, "Noto Sans CJK SC" },
        { SIMHEI, "MYingHeiGB18030" },
        { SIMHEI, "MYingHeiB5HK" },

        // 新細明體
        { PMINGLIU, "PMingLiU"},
        { PMINGLIU, "\xe6\x96\xb0\xe7\xb4\xb0\xe6\x98\x8e\xe9\xab\x94" },
        { PMINGLIU, "Noto Serif CJK TC"},
        { PMINGLIU, "MSung B5HK"},

        // 細明體
        { MINGLIU, "MingLiU"},
        { MINGLIU, "\xe7\xb4\xb0\xe6\x98\x8e\xe9\xab\x94" },
        { MINGLIU, "Noto Serif CJK TC"},
        { MINGLIU, "MSung B5HK"},

        // 新細明體
        { PMINGLIUHK, "PMingLiU_HKSCS"},
        { PMINGLIUHK, "\xe6\x96\xb0\xe7\xb4\xb0\xe6\x98\x8e\xe9\xab\x94_HKSCS" },
        { PMINGLIUHK, "Noto Serif CJK TC"},
        { PMINGLIUHK, "MSung B5HK"},

        // 細明體
        { MINGLIUHK, "MingLiU_HKSCS"},
        { MINGLIUHK, "\xe7\xb4\xb0\xe6\x98\x8e\xe9\xab\x94_HKSCS" },
        { MINGLIUHK, "Noto Serif CJK TC"},
        { MINGLIUHK, "MSung B5HK"},

        // Cambria
        { CAMBRIA, "Cambria" },
        { CAMBRIA, "Caladea" },

        // Calibri
        { CALIBRI, "Calibri" },
        { CALIBRI, "Carlito" },
    };

    static const size_t kFontCount =
        sizeof(kFontEquivMap)/sizeof(kFontEquivMap[0]);

    // TODO(jungshik): If this loop turns out to be hot, turn
    // the array to a static (hash)map to speed it up.
    for (size_t i = 0; i < kFontCount; ++i) {
        if (strcasecmp(kFontEquivMap[i].name, fontname) == 0)
            return kFontEquivMap[i].clazz;
    }
    return OTHER;
}


// Return true if |font_a| and |font_b| are visually and at the metrics
// level interchangeable.
bool IsMetricCompatibleReplacement(const char* font_a, const char* font_b)
{
    FontEquivClass class_a = GetFontEquivClass(font_a);
    FontEquivClass class_b = GetFontEquivClass(font_b);

    return class_a != OTHER && class_a == class_b;
}

// Normally we only return exactly the font asked for. In last-resort
// cases, the request either doesn't specify a font or is one of the
// basic font names like "Sans", "Serif" or "Monospace". This function
// tells you whether a given request is for such a fallback.
bool IsFallbackFontAllowed(const SkString& family) {
  const char* family_cstr = family.c_str();
  return family.isEmpty() ||
         strcasecmp(family_cstr, "sans") == 0 ||
         strcasecmp(family_cstr, "serif") == 0 ||
         strcasecmp(family_cstr, "monospace") == 0;
}

// Retrieves |is_bold|, |is_italic| and |font_family| properties from |font|.
static int get_int(FcPattern* pattern, const char object[], int missing) {
    int value;
    if (FcPatternGetInteger(pattern, object, 0, &value) != FcResultMatch) {
        return missing;
    }
    return value;
}

static int map_range(SkScalar value,
                     SkScalar old_min, SkScalar old_max,
                     SkScalar new_min, SkScalar new_max)
{
    SkASSERT(old_min < old_max);
    SkASSERT(new_min <= new_max);
    return new_min + ((value - old_min) * (new_max - new_min) / (old_max - old_min));
}

struct MapRanges {
    SkScalar old_val;
    SkScalar new_val;
};

static SkScalar map_ranges(SkScalar val, MapRanges const ranges[], int rangesCount) {
    // -Inf to [0]
    if (val < ranges[0].old_val) {
        return ranges[0].new_val;
    }

    // Linear from [i] to [i+1]
    for (int i = 0; i < rangesCount - 1; ++i) {
        if (val < ranges[i+1].old_val) {
            return map_range(val, ranges[i].old_val, ranges[i+1].old_val,
                                  ranges[i].new_val, ranges[i+1].new_val);
        }
    }

    // From [n] to +Inf
    // if (fcweight < Inf)
    return ranges[rangesCount-1].new_val;
}

#ifndef FC_WEIGHT_DEMILIGHT
#define FC_WEIGHT_DEMILIGHT        65
#endif

static SkFontStyle skfontstyle_from_fcpattern(FcPattern* pattern) {
    typedef SkFontStyle SkFS;

    static constexpr MapRanges weightRanges[] = {
        { FC_WEIGHT_THIN,       SkFS::kThin_Weight },
        { FC_WEIGHT_EXTRALIGHT, SkFS::kExtraLight_Weight },
        { FC_WEIGHT_LIGHT,      SkFS::kLight_Weight },
        { FC_WEIGHT_DEMILIGHT,  350 },
        { FC_WEIGHT_BOOK,       380 },
        { FC_WEIGHT_REGULAR,    SkFS::kNormal_Weight },
        { FC_WEIGHT_MEDIUM,     SkFS::kMedium_Weight },
        { FC_WEIGHT_DEMIBOLD,   SkFS::kSemiBold_Weight },
        { FC_WEIGHT_BOLD,       SkFS::kBold_Weight },
        { FC_WEIGHT_EXTRABOLD,  SkFS::kExtraBold_Weight },
        { FC_WEIGHT_BLACK,      SkFS::kBlack_Weight },
        { FC_WEIGHT_EXTRABLACK, SkFS::kExtraBlack_Weight },
    };
    SkScalar weight = map_ranges(get_int(pattern, FC_WEIGHT, FC_WEIGHT_REGULAR),
                                 weightRanges, SK_ARRAY_COUNT(weightRanges));

    static constexpr MapRanges widthRanges[] = {
        { FC_WIDTH_ULTRACONDENSED, SkFS::kUltraCondensed_Width },
        { FC_WIDTH_EXTRACONDENSED, SkFS::kExtraCondensed_Width },
        { FC_WIDTH_CONDENSED,      SkFS::kCondensed_Width },
        { FC_WIDTH_SEMICONDENSED,  SkFS::kSemiCondensed_Width },
        { FC_WIDTH_NORMAL,         SkFS::kNormal_Width },
        { FC_WIDTH_SEMIEXPANDED,   SkFS::kSemiExpanded_Width },
        { FC_WIDTH_EXPANDED,       SkFS::kExpanded_Width },
        { FC_WIDTH_EXTRAEXPANDED,  SkFS::kExtraExpanded_Width },
        { FC_WIDTH_ULTRAEXPANDED,  SkFS::kUltraExpanded_Width },
    };
    SkScalar width = map_ranges(get_int(pattern, FC_WIDTH, FC_WIDTH_NORMAL),
                                widthRanges, SK_ARRAY_COUNT(widthRanges));

    SkFS::Slant slant = SkFS::kUpright_Slant;
    switch (get_int(pattern, FC_SLANT, FC_SLANT_ROMAN)) {
        case FC_SLANT_ROMAN:   slant = SkFS::kUpright_Slant; break;
        case FC_SLANT_ITALIC : slant = SkFS::kItalic_Slant ; break;
        case FC_SLANT_OBLIQUE: slant = SkFS::kOblique_Slant; break;
        default: SkASSERT(false); break;
    }

    return SkFontStyle(SkScalarRoundToInt(weight), SkScalarRoundToInt(width), slant);
}

static void fcpattern_from_skfontstyle(SkFontStyle style, FcPattern* pattern) {
    typedef SkFontStyle SkFS;

    static constexpr MapRanges weightRanges[] = {
        { SkFS::kThin_Weight,       FC_WEIGHT_THIN },
        { SkFS::kExtraLight_Weight, FC_WEIGHT_EXTRALIGHT },
        { SkFS::kLight_Weight,      FC_WEIGHT_LIGHT },
        { 350,                      FC_WEIGHT_DEMILIGHT },
        { 380,                      FC_WEIGHT_BOOK },
        { SkFS::kNormal_Weight,     FC_WEIGHT_REGULAR },
        { SkFS::kMedium_Weight,     FC_WEIGHT_MEDIUM },
        { SkFS::kSemiBold_Weight,   FC_WEIGHT_DEMIBOLD },
        { SkFS::kBold_Weight,       FC_WEIGHT_BOLD },
        { SkFS::kExtraBold_Weight,  FC_WEIGHT_EXTRABOLD },
        { SkFS::kBlack_Weight,      FC_WEIGHT_BLACK },
        { SkFS::kExtraBlack_Weight, FC_WEIGHT_EXTRABLACK },
    };
    int weight = map_ranges(style.weight(), weightRanges, SK_ARRAY_COUNT(weightRanges));

    static constexpr MapRanges widthRanges[] = {
        { SkFS::kUltraCondensed_Width, FC_WIDTH_ULTRACONDENSED },
        { SkFS::kExtraCondensed_Width, FC_WIDTH_EXTRACONDENSED },
        { SkFS::kCondensed_Width,      FC_WIDTH_CONDENSED },
        { SkFS::kSemiCondensed_Width,  FC_WIDTH_SEMICONDENSED },
        { SkFS::kNormal_Width,         FC_WIDTH_NORMAL },
        { SkFS::kSemiExpanded_Width,   FC_WIDTH_SEMIEXPANDED },
        { SkFS::kExpanded_Width,       FC_WIDTH_EXPANDED },
        { SkFS::kExtraExpanded_Width,  FC_WIDTH_EXTRAEXPANDED },
        { SkFS::kUltraExpanded_Width,  FC_WIDTH_ULTRAEXPANDED },
    };
    int width = map_ranges(style.width(), widthRanges, SK_ARRAY_COUNT(widthRanges));

    int slant = FC_SLANT_ROMAN;
    switch (style.slant()) {
        case SkFS::kUpright_Slant: slant = FC_SLANT_ROMAN  ; break;
        case SkFS::kItalic_Slant : slant = FC_SLANT_ITALIC ; break;
        case SkFS::kOblique_Slant: slant = FC_SLANT_OBLIQUE; break;
        default: SkASSERT(false); break;
    }

    FcPatternAddInteger(pattern, FC_WEIGHT, weight);
    FcPatternAddInteger(pattern, FC_WIDTH , width);
    FcPatternAddInteger(pattern, FC_SLANT , slant);
}

}  // anonymous namespace

///////////////////////////////////////////////////////////////////////////////

#define kMaxFontFamilyLength    2048
#ifdef SK_FONT_CONFIG_INTERFACE_ONLY_ALLOW_SFNT_FONTS
const char* kFontFormatTrueType = "TrueType";
const char* kFontFormatCFF = "CFF";
#endif

SkFontConfigInterfaceDirect::SkFontConfigInterfaceDirect() {
    FCLocker lock;
    fcConfig.reset(FcConfigReference(nullptr));

    SkDEBUGCODE(fontconfiginterface_unittest();)
}

SkFontConfigInterfaceDirect::~SkFontConfigInterfaceDirect() {
}

bool SkFontConfigInterfaceDirect::isAccessible(const char* filename) {
    if (access(filename, R_OK) != 0) {
        return false;
    }
    return true;
}

bool SkFontConfigInterfaceDirect::isValidPattern(FcPattern* pattern) {
#ifdef SK_FONT_CONFIG_INTERFACE_ONLY_ALLOW_SFNT_FONTS
    const char* font_format = get_string(pattern, FC_FONTFORMAT);
    if (font_format
        && strcmp(font_format, kFontFormatTrueType) != 0
        && strcmp(font_format, kFontFormatCFF) != 0)
    {
        return false;
    }
#endif

    // fontconfig can also return fonts which are unreadable
    const char* c_filename = get_string(pattern, FC_FILE);
    if (!c_filename) {
        return false;
    }
    const char* sysroot = (const char*)FcConfigGetSysRoot(fcConfig.get());
    SkString resolvedFilename;
    if (sysroot) {
        resolvedFilename = sysroot;
        resolvedFilename += c_filename;
        c_filename = resolvedFilename.c_str();
    }
    return this->isAccessible(c_filename);
}

// Find matching font from |font_set| for the given font family.
FcPattern* SkFontConfigInterfaceDirect::MatchFont(FcFontSet* font_set,
                                                  const char* post_config_family,
                                                  const SkString& family) {
  // Older versions of fontconfig have a bug where they cannot select
  // only scalable fonts so we have to manually filter the results.
  FcPattern* match = nullptr;
  for (int i = 0; i < font_set->nfont; ++i) {
    FcPattern* current = font_set->fonts[i];
    if (this->isValidPattern(current)) {
      match = current;
      break;
    }
  }

  if (match && !IsFallbackFontAllowed(family)) {
    bool acceptable_substitute = false;
    for (int id = 0; id < 255; ++id) {
      const char* post_match_family = get_string(match, FC_FAMILY, id);
      if (!post_match_family)
        break;
      acceptable_substitute =
          (strcasecmp(post_config_family, post_match_family) == 0 ||
           // Workaround for Issue 12530:
           //   requested family: "Bitstream Vera Sans"
           //   post_config_family: "Arial"
           //   post_match_family: "Bitstream Vera Sans"
           // -> We should treat this case as a good match.
           strcasecmp(family.c_str(), post_match_family) == 0) ||
           IsMetricCompatibleReplacement(family.c_str(), post_match_family);
      if (acceptable_substitute)
        break;
    }
    if (!acceptable_substitute)
      return nullptr;
  }

  return match;
}

bool SkFontConfigInterfaceDirect::matchFamilyName(const char familyName[],
                                                  SkFontStyle style,
                                                  FontIdentity* outIdentity,
                                                  SkString* outFamilyName,
                                                  SkFontStyle* outStyle) {
    SkString familyStr(familyName ? familyName : "");
    if (familyStr.size() > kMaxFontFamilyLength) {
        return false;
    }

    FCLocker lock;

    FcPattern* pattern = FcPatternCreate();

    if (familyName) {
        FcPatternAddString(pattern, FC_FAMILY, (FcChar8*)familyName);
    }
    fcpattern_from_skfontstyle(style, pattern);

    FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);

    FcConfigSubstitute(fcConfig.get(), pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    // Font matching:
    // CSS often specifies a fallback list of families:
    //    font-family: a, b, c, serif;
    // However, fontconfig will always do its best to find *a* font when asked
    // for something so we need a way to tell if the match which it has found is
    // "good enough" for us. Otherwise, we can return nullptr which gets piped up
    // and lets WebKit know to try the next CSS family name. However, fontconfig
    // configs allow substitutions (mapping "Arial -> Helvetica" etc) and we
    // wish to support that.
    //
    // Thus, if a specific family is requested we set @family_requested. Then we
    // record two strings: the family name after config processing and the
    // family name after resolving. If the two are equal, it's a good match.
    //
    // So consider the case where a user has mapped Arial to Helvetica in their
    // config.
    //    requested family: "Arial"
    //    post_config_family: "Helvetica"
    //    post_match_family: "Helvetica"
    //      -> good match
    //
    // and for a missing font:
    //    requested family: "Monaco"
    //    post_config_family: "Monaco"
    //    post_match_family: "Times New Roman"
    //      -> BAD match
    //
    // However, we special-case fallback fonts; see IsFallbackFontAllowed().

    const char* post_config_family = get_string(pattern, FC_FAMILY);
    if (!post_config_family) {
        // we can just continue with an empty name, e.g. default font
        post_config_family = "";
    }

    FcResult result;
    FcFontSet* font_set = FcFontSort(fcConfig.get(), pattern, 0, nullptr, &result);
    if (!font_set) {
        FcPatternDestroy(pattern);
        return false;
    }

    FcPattern* match = this->MatchFont(font_set, post_config_family, familyStr);
    if (!match) {
        FcPatternDestroy(pattern);
        FcFontSetDestroy(font_set);
        return false;
    }

    FcPatternDestroy(pattern);

    // From here out we just extract our results from 'match'

    post_config_family = get_string(match, FC_FAMILY);
    if (!post_config_family) {
        FcFontSetDestroy(font_set);
        return false;
    }

    const char* c_filename = get_string(match, FC_FILE);
    if (!c_filename) {
        FcFontSetDestroy(font_set);
        return false;
    }
    const char* sysroot = (const char*)FcConfigGetSysRoot(fcConfig.get());
    SkString resolvedFilename;
    if (sysroot) {
        resolvedFilename = sysroot;
        resolvedFilename += c_filename;
        c_filename = resolvedFilename.c_str();
    }

    int face_index = get_int(match, FC_INDEX, 0);

    FcFontSetDestroy(font_set);

    if (outIdentity) {
        outIdentity->fTTCIndex = face_index;
        outIdentity->fString.set(c_filename);
    }
    if (outFamilyName) {
        outFamilyName->set(post_config_family);
    }
    if (outStyle) {
        *outStyle = skfontstyle_from_fcpattern(match);
    }
    return true;
}

SkStreamAsset* SkFontConfigInterfaceDirect::openStream(const FontIdentity& identity) {
    return SkStream::MakeFromFile(identity.fString.c_str()).release();
}
