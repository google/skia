/*
 * Copyright 2009 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* migrated from chrome/src/skia/ext/SkFontHost_fontconfig_direct.cpp */

#include <string>
#include <unistd.h>
#include <fcntl.h>

#include <fontconfig/fontconfig.h>

#include "SkFontConfigInterface.h"
#include "SkStream.h"

class SkFontConfigInterfaceDirect : public SkFontConfigInterface {
public:
            SkFontConfigInterfaceDirect();
    virtual ~SkFontConfigInterfaceDirect();

    virtual bool matchFamilyName(const char familyName[],
                                 SkTypeface::Style requested,
                                 FontIdentity* outFontIdentifier,
                                 SkString* outFamilyName,
                                 SkTypeface::Style* outStyle) SK_OVERRIDE;
    virtual SkStream* openStream(const FontIdentity&) SK_OVERRIDE;

private:
    SkMutex mutex_;
};

SkFontConfigInterface* SkFontConfigInterface::GetSingletonDirectInterface() {
    static SkFontConfigInterface* gDirect;
    if (NULL == gDirect) {
        gDirect = new SkFontConfigInterfaceDirect;
    }
    return gDirect;
}

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
        { PGOTHIC, "IPAPGothic" },
        { PGOTHIC, "MotoyaG04Gothic" },

        // ＭＳ ゴシック
        { GOTHIC, "MS Gothic" },
        { GOTHIC, "\xef\xbc\xad\xef\xbc\xb3 "
                  "\xe3\x82\xb4\xe3\x82\xb7\xe3\x83\x83\xe3\x82\xaf" },
        { GOTHIC, "IPAGothic" },
        { GOTHIC, "MotoyaG04GothicMono" },

        // ＭＳ Ｐ明朝
        { PMINCHO, "MS PMincho" },
        { PMINCHO, "\xef\xbc\xad\xef\xbc\xb3 \xef\xbc\xb0"
                   "\xe6\x98\x8e\xe6\x9c\x9d"},
        { PMINCHO, "IPAPMincho" },
        { PMINCHO, "MotoyaG04Mincho" },

        // ＭＳ 明朝
        { MINCHO, "MS Mincho" },
        { MINCHO, "\xef\xbc\xad\xef\xbc\xb3 \xe6\x98\x8e\xe6\x9c\x9d" },
        { MINCHO, "IPAMincho" },
        { MINCHO, "MotoyaG04MinchoMono" },

        // 宋体
        { SIMSUN, "Simsun" },
        { SIMSUN, "\xe5\xae\x8b\xe4\xbd\x93" },
        { SIMSUN, "MSung GB18030" },
        { SIMSUN, "Song ASC" },

        // 新宋体
        { NSIMSUN, "NSimsun" },
        { NSIMSUN, "\xe6\x96\xb0\xe5\xae\x8b\xe4\xbd\x93" },
        { NSIMSUN, "MSung GB18030" },
        { NSIMSUN, "N Song ASC" },

        // 黑体
        { SIMHEI, "Simhei" },
        { SIMHEI, "\xe9\xbb\x91\xe4\xbd\x93" },
        { SIMHEI, "MYingHeiGB18030" },
        { SIMHEI, "MYingHeiB5HK" },

        // 新細明體
        { PMINGLIU, "PMingLiU"},
        { PMINGLIU, "\xe6\x96\xb0\xe7\xb4\xb0\xe6\x98\x8e\xe9\xab\x94" },
        { PMINGLIU, "MSung B5HK"},

        // 細明體
        { MINGLIU, "MingLiU"},
        { MINGLIU, "\xe7\xb4\xb0\xe6\x98\x8e\xe9\xab\x94" },
        { MINGLIU, "MSung B5HK"},

        // 新細明體
        { PMINGLIUHK, "PMingLiU_HKSCS"},
        { PMINGLIUHK, "\xe6\x96\xb0\xe7\xb4\xb0\xe6\x98\x8e\xe9\xab\x94_HKSCS" },
        { PMINGLIUHK, "MSung B5HK"},

        // 細明體
        { MINGLIUHK, "MingLiU_HKSCS"},
        { MINGLIUHK, "\xe7\xb4\xb0\xe6\x98\x8e\xe9\xab\x94_HKSCS" },
        { MINGLIUHK, "MSung B5HK"},

        // Cambria
        { CAMBRIA, "Cambria" },
        { CAMBRIA, "Caladea" },
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
bool IsFallbackFontAllowed(const std::string& family) {
  const char* family_cstr = family.c_str();
  return family.empty() ||
         strcasecmp(family_cstr, "sans") == 0 ||
         strcasecmp(family_cstr, "serif") == 0 ||
         strcasecmp(family_cstr, "monospace") == 0;
}

// Find matching font from |font_set| for the given font family.
FcPattern* MatchFont(FcFontSet* font_set,
                     FcChar8* post_config_family,
                     const std::string& family) {
  // Older versions of fontconfig have a bug where they cannot select
  // only scalable fonts so we have to manually filter the results.
  FcPattern* match = NULL;
  for (int i = 0; i < font_set->nfont; ++i) {
    FcPattern* current = font_set->fonts[i];
    FcBool is_scalable;

    if (FcPatternGetBool(current, FC_SCALABLE, 0,
                         &is_scalable) != FcResultMatch ||
        !is_scalable) {
      continue;
    }

    // fontconfig can also return fonts which are unreadable
    FcChar8* c_filename;
    if (FcPatternGetString(current, FC_FILE, 0, &c_filename) != FcResultMatch)
      continue;

    if (access(reinterpret_cast<char*>(c_filename), R_OK) != 0)
      continue;

    match = current;
    break;
  }

  if (match && !IsFallbackFontAllowed(family)) {
    bool acceptable_substitute = false;
    for (int id = 0; id < 255; ++id) {
      FcChar8* post_match_family;
      if (FcPatternGetString(match, FC_FAMILY, id, &post_match_family) !=
          FcResultMatch)
        break;
      acceptable_substitute =
          (strcasecmp(reinterpret_cast<char*>(post_config_family),
                      reinterpret_cast<char*>(post_match_family)) == 0 ||
           // Workaround for Issue 12530:
           //   requested family: "Bitstream Vera Sans"
           //   post_config_family: "Arial"
           //   post_match_family: "Bitstream Vera Sans"
           // -> We should treat this case as a good match.
           strcasecmp(family.c_str(),
                      reinterpret_cast<char*>(post_match_family)) == 0) ||
           IsMetricCompatibleReplacement(family.c_str(),
               reinterpret_cast<char*>(post_match_family));
      if (acceptable_substitute)
        break;
    }
    if (!acceptable_substitute)
      return NULL;
  }

  return match;
}

// Retrieves |is_bold|, |is_italic| and |font_family| properties from |font|.
SkTypeface::Style GetFontStyle(FcPattern* font) {
    int resulting_bold;
    if (FcPatternGetInteger(font, FC_WEIGHT, 0, &resulting_bold))
        resulting_bold = FC_WEIGHT_NORMAL;

    int resulting_italic;
    if (FcPatternGetInteger(font, FC_SLANT, 0, &resulting_italic))
        resulting_italic = FC_SLANT_ROMAN;

    // If we ask for an italic font, fontconfig might take a roman font and set
    // the undocumented property FC_MATRIX to a skew matrix. It'll then say
    // that the font is italic or oblique. So, if we see a matrix, we don't
    // believe that it's italic.
    FcValue matrix;
    const bool have_matrix = FcPatternGet(font, FC_MATRIX, 0, &matrix) == 0;

    // If we ask for an italic font, fontconfig might take a roman font and set
    // FC_EMBOLDEN.
    FcValue embolden;
    const bool have_embolden = FcPatternGet(font, FC_EMBOLDEN, 0, &embolden) == 0;

    int styleBits = 0;
    if (resulting_bold > FC_WEIGHT_MEDIUM && !have_embolden) {
        styleBits |= SkTypeface::kBold;
    }
    if (resulting_italic > FC_SLANT_ROMAN && !have_matrix) {
        styleBits |= SkTypeface::kItalic;
    }

    return (SkTypeface::Style)styleBits;
}

}  // anonymous namespace

///////////////////////////////////////////////////////////////////////////////

#define kMaxFontFamilyLength    2048

SkFontConfigInterfaceDirect::SkFontConfigInterfaceDirect() {
    FcInit();
}

SkFontConfigInterfaceDirect::~SkFontConfigInterfaceDirect() {
}

bool SkFontConfigInterfaceDirect::matchFamilyName(const char familyName[],
                                                  SkTypeface::Style style,
                                                  FontIdentity* outIdentity,
                                                  SkString* outFamilyName,
                                                  SkTypeface::Style* outStyle) {
    std::string familyStr(familyName ? familyName : "");
    if (familyStr.length() > kMaxFontFamilyLength) {
        return false;
    }

    SkAutoMutexAcquire ac(mutex_);

    FcPattern* pattern = FcPatternCreate();

    if (familyName) {
        FcPatternAddString(pattern, FC_FAMILY, (FcChar8*)familyName);
    }
    FcPatternAddInteger(pattern, FC_WEIGHT,
                        (style & SkTypeface::kBold) ? FC_WEIGHT_BOLD
                                                    : FC_WEIGHT_NORMAL);
    FcPatternAddInteger(pattern, FC_SLANT,
                        (style & SkTypeface::kItalic) ? FC_SLANT_ITALIC
                                                      : FC_SLANT_ROMAN);
    FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);

    FcConfigSubstitute(NULL, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    // Font matching:
    // CSS often specifies a fallback list of families:
    //    font-family: a, b, c, serif;
    // However, fontconfig will always do its best to find *a* font when asked
    // for something so we need a way to tell if the match which it has found is
    // "good enough" for us. Otherwise, we can return NULL which gets piped up
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
    FcChar8* post_config_family;
    FcPatternGetString(pattern, FC_FAMILY, 0, &post_config_family);

    FcResult result;
    FcFontSet* font_set = FcFontSort(0, pattern, 0, 0, &result);
    if (!font_set) {
        FcPatternDestroy(pattern);
        return false;
    }

    FcPattern* match = MatchFont(font_set, post_config_family, familyStr);
    if (!match) {
        FcPatternDestroy(pattern);
        FcFontSetDestroy(font_set);
        return false;
    }

    FcPatternDestroy(pattern);

    // From here out we just extract our results from 'match'

    if (FcPatternGetString(match, FC_FAMILY, 0, &post_config_family) != FcResultMatch) {
        FcFontSetDestroy(font_set);
        return false;
    }

    FcChar8* c_filename;
    if (FcPatternGetString(match, FC_FILE, 0, &c_filename) != FcResultMatch) {
        FcFontSetDestroy(font_set);
        return false;
    }

    int face_index;
    if (FcPatternGetInteger(match, FC_INDEX, 0, &face_index) != FcResultMatch) {
        FcFontSetDestroy(font_set);
        return false;
    }

    FcFontSetDestroy(font_set);

    if (outIdentity) {
        outIdentity->fTTCIndex = face_index;
        outIdentity->fString.set((const char*)c_filename);
    }
    if (outFamilyName) {
        outFamilyName->set((const char*)post_config_family);
    }
    if (outStyle) {
        *outStyle = GetFontStyle(match);
    }
    return true;
}

SkStream* SkFontConfigInterfaceDirect::openStream(const FontIdentity& identity) {
    return SkStream::NewFromFile(identity.fString.c_str());
}
