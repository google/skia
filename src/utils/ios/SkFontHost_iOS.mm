#import <UIKit/UIKit.h>

#include "SkStream_NSData.h"
#include "SkTypeface.h"
#include "SkFontHost.h"
#include "SkThread.h"
#include "SkTemplates.h"

enum FontDesign {
    kUnknown_Design,
    kSans_FontDesign,
    kSerif_FontDesign,

    kIllegal_FontDesign,    // never use with a real font
};

// returns kIllegal_FontDesign if not found
static FontDesign find_design_from_name(const char name[]) {
    static const struct {
        const char* fName;
        FontDesign  fDesign;
    } gRec[] = {
        { "sans-serif", kSans_FontDesign },
        { "serif",      kSerif_FontDesign },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
        if (!strcasecmp(name, gRec[i].fName)) {
            return gRec[i].fDesign;
        }
    }
    return kIllegal_FontDesign;
}

struct FontRes {
    const char*         fName;
    SkTypeface::Style   fStyle;
    FontDesign          fDesign;
};

static const FontRes gFontRes[] = {
    { "DroidSans",          SkTypeface::kNormal,    kSans_FontDesign    },
    { "DroidSans",          SkTypeface::kBold,      kSans_FontDesign    },
    { "DroidSerif-Regular", SkTypeface::kNormal,    kSerif_FontDesign    },
    { "DroidSerif-Bold",    SkTypeface::kBold,      kSerif_FontDesign    },
//    { "PescaderoPro",       SkTypeface::kNormal,    kSerif_FontDesign   },
//    { "PescaderoPro-Bold",  SkTypeface::kBold,      kSerif_FontDesign   },
};
#define FONTRES_COUNT SK_ARRAY_COUNT(gFontRes)

#define DEFAULT_INDEX_REGULAR   1
#define DEFAULT_INDEX_BOLD      2

///////////////////////////////////////////////////////////////////////////////

class SkTypeface_Stream : public SkTypeface {
public:
    SkTypeface_Stream(SkStream* stream, Style style);
    virtual ~SkTypeface_Stream();

    SkStream* refStream() {
        fStream->ref();
        return fStream;
    }

private:
    SkStream*   fStream;
};

static int32_t gUniqueFontID;

SkTypeface_Stream::SkTypeface_Stream(SkStream* stream, Style style)
: SkTypeface(style, sk_atomic_inc(&gUniqueFontID) + 1) {
    fStream = stream;
    fStream->ref();
}

SkTypeface_Stream::~SkTypeface_Stream() {
    fStream->unref();
}

static SkTypeface_Stream* create_from_fontres(const FontRes& res) {
    SkStream* stream = SkStream_NSData::CreateFromResource(res.fName, "ttf");
    SkAutoUnref aur(stream);

    return SkNEW_ARGS(SkTypeface_Stream, (stream, res.fStyle));
}

///////////////////////////////////////////////////////////////////////////////

static int compute_style_distance(SkTypeface::Style a, SkTypeface::Style b) {
    int dist = 0;
    int diff = a ^ b;
    if (diff & SkTypeface::kBold) {
        dist += 2;
    }
    if (diff & SkTypeface::kItalic) {
        dist += 1;
    }
    return dist;
}

static SkTypeface_Stream* gFonts[FONTRES_COUNT];

static void assure_init_fonts() {
    static bool gOnce;
    if (!gOnce) {
        for (size_t i = 0; i < FONTRES_COUNT; i++) {
            gFonts[i] = create_from_fontres(gFontRes[i]);
            gOnce = true;
        }
    }
}

static SkTypeface_Stream* get_default_font(SkTypeface::Style style) {
    assure_init_fonts();

    if (style & SkTypeface::kBold) {
        return gFonts[DEFAULT_INDEX_BOLD];
    } else {
        return gFonts[DEFAULT_INDEX_REGULAR];
    }
}

static SkTypeface_Stream* find_by_id(SkFontID fontID) {
    assure_init_fonts();
    
    for (size_t i = 0; i < FONTRES_COUNT; i++) {
        if (gFonts[i]->uniqueID() == fontID) {
            return gFonts[i];
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T> T* ref_and_return(T* obj) {
    obj->ref();
    return obj;
}

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                                     const char familyName[],
                                     const void* data, size_t bytelength,
                                     SkTypeface::Style style) {
    assure_init_fonts();

    if (familyName) {
        FontDesign design = find_design_from_name(familyName);
        if (kIllegal_FontDesign != design) {
            familyName = "$#@*&%*#$@ never match any name";
        }

        int bestDistance = 999;
        int bestIndex = -1;
        for (size_t i = 0; i < FONTRES_COUNT; i++) {
            if (design == gFontRes[i].fDesign || !strcmp(gFontRes[i].fName, familyName)) {
                int dist = compute_style_distance(style, gFontRes[i].fStyle);
                if (dist < bestDistance) {
                    bestDistance = dist;
                    bestIndex = i;
                }
            }
        }
        if (bestIndex >= 0) {
            return ref_and_return(gFonts[bestIndex]);
        }
    }

    return ref_and_return(get_default_font(style));
}

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream) {
    SkDEBUGFAIL("SkFontHost::CreateTypeface unimplemented");
    return NULL;
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(char const*) {
//    SkDEBUGFAIL("SkFontHost::CreateTypefaceFromFile unimplemented");
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

SkStream* SkFontHost::OpenStream(uint32_t uniqueID) {
    SkTypeface_Stream* tf = find_by_id(uniqueID);
    SkASSERT(tf);
    return tf->refStream();
}

size_t SkFontHost::GetFileName(SkFontID fontID, char path[], size_t length,
                               int32_t* index) {
    SkDebugf("SkFontHost::GetFileName unimplemented\n");
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void SkFontHost::Serialize(const SkTypeface* face, SkWStream* stream) {
    SkDEBUGFAIL("SkFontHost::Serialize unimplemented");
}

SkTypeface* SkFontHost::Deserialize(SkStream* stream) {
    int style = stream->readU8();
    int len = stream->readPackedUInt();
    const char* name = NULL;
    if (len > 0) {
        SkString str;
        str.resize(len);
        stream->read(str.writable_str(), len);
        
        if (str.startsWith("DroidSans")) {
            name = "sans-serif";
        } else if (str.startsWith("DroidSerif")) {
            name = "serif";
        }
        SkDebugf("---- deserialize typeface <%s> %d %s\n", str.c_str(), style, name);
    }
//    name = NULL; style = 0;
    return SkFontHost::CreateTypeface(NULL, name, NULL, NULL,
                                      (SkTypeface::Style)style);
}

SkFontID SkFontHost::NextLogicalFont(SkFontID currFontID, SkFontID origFontID) {
    return 0;
}

#define FONT_CACHE_MEMORY_BUDGET    1 * 1024 * 1024

size_t SkFontHost::ShouldPurgeFontCache(size_t sizeAllocatedSoFar) {
    if (sizeAllocatedSoFar > FONT_CACHE_MEMORY_BUDGET)
        return sizeAllocatedSoFar - FONT_CACHE_MEMORY_BUDGET;
    else
        return 0;   // nothing to do
}

///////////////////////////////////////////////////////////////////////////////
int SkFontHost::ComputeGammaFlag(const SkPaint& paint) {
    return 0;
}

void SkFontHost::GetGammaTables(const uint8_t* tables[2]) {
    tables[0] = NULL;   // black gamma (e.g. exp=1.4)
    tables[1] = NULL;   // white gamma (e.g. exp= 1/1.4)
}

// static
SkAdvancedTypefaceMetrics* SkFontHost::GetAdvancedTypefaceMetrics(
                                                                  uint32_t fontID,
                                                                  SkAdvancedTypefaceMetrics::PerGlyphInfo perGlyphInfo) {
    SkDEBUGFAIL("SkFontHost::GetAdvancedTypefaceMetrics unimplemented");
    return NULL;
}

void SkFontHost::FilterRec(SkScalerContext::Rec* rec, SkTypeface*) {
}

SkScalerContext* SkFontHost::CreateScalerContext(const SkDescriptor* desc) {
    SkDEBUGFAIL("SkFontHost::CreateScalarContext unimplemented");
    return NULL;
}