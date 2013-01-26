/*
 * Copyright 2008 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <map>
#include <string>

#include <fontconfig/fontconfig.h>

#include "SkFontHost.h"
#include "SkStream.h"

/** An extern from SkFontHost_FreeType. */
SkTypeface::Style find_name_and_style(SkStream* stream, SkString* name);

/** This lock must be held while modifying global_fc_* globals. */
SK_DECLARE_STATIC_MUTEX(global_fc_map_lock);

/** Map from file names to file ids. */
static std::map<std::string, unsigned> global_fc_map;
/** Map from file ids to file names. */
static std::map<unsigned, std::string> global_fc_map_inverted;
/** The next file id. */
static unsigned global_fc_map_next_id = 0;

/**
 * Check to see if the filename has already been assigned a fileid and, if so, use it.
 * Otherwise, assign one. Return the resulting fileid.
 */
static unsigned FileIdFromFilename(const char* filename) {
    SkAutoMutexAcquire ac(global_fc_map_lock);

    std::map<std::string, unsigned>::const_iterator i = global_fc_map.find(filename);
    if (i == global_fc_map.end()) {
        const unsigned fileid = global_fc_map_next_id++;
        global_fc_map[filename] = fileid;
        global_fc_map_inverted[fileid] = filename;
        return fileid;
    } else {
        return i->second;
    }
}

static unsigned FileIdFromUniqueId(unsigned uniqueid) {
    return uniqueid >> 8;
}

static SkTypeface::Style StyleFromUniqueId(unsigned uniqueid) {
    return static_cast<SkTypeface::Style>(uniqueid & 0xff);
}

static unsigned UniqueIdFromFileIdAndStyle(unsigned fileid, SkTypeface::Style style) {
    SkASSERT((style & 0xff) == style);
    return (fileid << 8) | static_cast<int>(style);
}

class FontConfigTypeface : public SkTypeface {
public:
    FontConfigTypeface(Style style, uint32_t id) : SkTypeface(style, id) { }
};

/**
 * Find a matching font where @type (one of FC_*) is equal to @value. For a
 * list of types, see http://fontconfig.org/fontconfig-devel/x19.html#AEN27.
 * The variable arguments are a list of triples, just like the first three
 * arguments, and must be NULL terminated.
 *
 * For example,
 *   FontMatchString(FC_FILE, FcTypeString, "/usr/share/fonts/myfont.ttf", NULL);
 */
static FcPattern* FontMatch(const char* type, FcType vtype, const void* value, ...) {
    va_list ap;
    va_start(ap, value);

    FcPattern* pattern = FcPatternCreate();

    for (;;) {
        FcValue fcvalue;
        fcvalue.type = vtype;
        switch (vtype) {
            case FcTypeString:
                fcvalue.u.s = (FcChar8*) value;
                break;
            case FcTypeInteger:
                fcvalue.u.i = (int)(intptr_t)value;
                break;
            default:
                SkDEBUGFAIL("FontMatch unhandled type");
        }
        FcPatternAdd(pattern, type, fcvalue, FcFalse);

        type = va_arg(ap, const char *);
        if (!type)
            break;
        // FcType is promoted to int when passed through ...
        vtype = static_cast<FcType>(va_arg(ap, int));
        value = va_arg(ap, const void *);
    };
    va_end(ap);

    FcConfigSubstitute(NULL, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult result;
    FcPattern* match = FcFontMatch(NULL, pattern, &result);
    FcPatternDestroy(pattern);

    return match;
}

// static
SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                                       const char familyName[],
                                       SkTypeface::Style style)
{
    const char* resolved_family_name = NULL;
    FcPattern* face_match = NULL;

    {
        SkAutoMutexAcquire ac(global_fc_map_lock);
        if (FcTrue != FcInit()) {
            SkASSERT(false && "Could not initialize fontconfig.");
        }
    }

    if (familyFace) {
        // Here we use the inverted global id map to find the filename from the
        // SkTypeface object. Given the filename we can ask fontconfig for the
        // familyname of the font.
        SkAutoMutexAcquire ac(global_fc_map_lock);

        const unsigned fileid = FileIdFromUniqueId(familyFace->uniqueID());
        std::map<unsigned, std::string>::const_iterator i = global_fc_map_inverted.find(fileid);
        if (i == global_fc_map_inverted.end()) {
            return NULL;
        }

        face_match = FontMatch(FC_FILE, FcTypeString, i->second.c_str(), NULL);
        if (!face_match) {
            return NULL;
        }

        FcChar8* family;
        if (FcPatternGetString(face_match, FC_FAMILY, 0, &family)) {
            FcPatternDestroy(face_match);
            return NULL;
        }
        // At this point, @family is pointing into the @face_match object so we
        // cannot release it yet.

        resolved_family_name = reinterpret_cast<char*>(family);
    } else if (familyName) {
        resolved_family_name = familyName;
    }

    const int bold = (style & SkTypeface::kBold) ? FC_WEIGHT_BOLD : FC_WEIGHT_NORMAL;
    const int italic = (style & SkTypeface::kItalic) ? FC_SLANT_ITALIC : FC_SLANT_ROMAN;

    FcPattern* match;
    if (resolved_family_name) {
        match = FontMatch(FC_FAMILY, FcTypeString, resolved_family_name,
                          FC_WEIGHT, FcTypeInteger, bold,
                          FC_SLANT, FcTypeInteger, italic,
                          NULL);
    } else {
        match = FontMatch(FC_WEIGHT, FcTypeInteger, reinterpret_cast<void*>(bold),
                          FC_SLANT, FcTypeInteger, italic,
                          NULL);
    }

    if (face_match)
        FcPatternDestroy(face_match);

    if (!match)
        return NULL;

    FcChar8* filename;
    if (FcPatternGetString(match, FC_FILE, 0, &filename) != FcResultMatch) {
        FcPatternDestroy(match);
        return NULL;
    }
    // Now @filename is pointing into @match

    const unsigned fileid = FileIdFromFilename(reinterpret_cast<char*>(filename));
    const unsigned id = UniqueIdFromFileIdAndStyle(fileid, style);
    SkTypeface* typeface = SkNEW_ARGS(FontConfigTypeface, (style, id));
    FcPatternDestroy(match);

    return typeface;
}

// static
SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream) {
    SkDEBUGFAIL("SkFontHost::CreateTypefaceFromStream unimplemented");
    return NULL;
}

// static
SkTypeface* SkFontHost::CreateTypefaceFromFile(const char path[]) {
    SkDEBUGFAIL("SkFontHost::CreateTypefaceFromFile unimplemented");
    return NULL;
}

// static
SkStream* SkFontHost::OpenStream(uint32_t id) {
    SkAutoMutexAcquire ac(global_fc_map_lock);
    const unsigned fileid = FileIdFromUniqueId(id);

    std::map<unsigned, std::string>::const_iterator i = global_fc_map_inverted.find(fileid);
    if (i == global_fc_map_inverted.end()) {
        return NULL;
    }

    return SkNEW_ARGS(SkFILEStream, (i->second.c_str()));
}

size_t SkFontHost::GetFileName(SkFontID fontID, char path[], size_t length, int32_t* index) {
    SkAutoMutexAcquire ac(global_fc_map_lock);
    const unsigned fileid = FileIdFromUniqueId(fontID);

    std::map<unsigned, std::string>::const_iterator i = global_fc_map_inverted.find(fileid);
    if (i == global_fc_map_inverted.end()) {
        return 0;
    }

    const std::string& str = i->second;
    if (path) {
        memcpy(path, str.c_str(), SkMin32(str.size(), length));
    }
    if (index) {    // TODO: check if we're in a TTC
        *index = 0;
    }
    return str.size();
}

void SkFontHost::Serialize(const SkTypeface*, SkWStream*) {
    SkDEBUGFAIL("SkFontHost::Serialize unimplemented");
}

SkTypeface* SkFontHost::Deserialize(SkStream* stream) {
    SkDEBUGFAIL("SkFontHost::Deserialize unimplemented");
    return NULL;
}

SkFontID SkFontHost::NextLogicalFont(SkFontID currFontID, SkFontID origFontID) {
    // We don't handle font fallback, WebKit does.
    return 0;
}
