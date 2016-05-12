/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkChecksum.h"
#include "SkFontDescriptor.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTypeface.h"
#include "SkUtils.h"
#include "../sfnt/SkOTUtils.h"

#include "SkWhitelistChecksums.cpp"

#define WHITELIST_DEBUG 0

extern void WhitelistSerializeTypeface(const SkTypeface*, SkWStream* );
extern SkTypeface* WhitelistDeserializeTypeface(SkStream* );
extern bool CheckChecksums();
extern bool GenerateChecksums();

#if WHITELIST_DEBUG
static bool timesNewRomanSerializedNameOnly = false;
#endif

#define SUBNAME_PREFIX "sk_"

static bool font_name_is_local(const char* fontName, SkTypeface::Style style) {
    if (!strcmp(fontName, "DejaVu Sans")) {
        return true;
    }
    SkTypeface* defaultFace = SkTypeface::CreateFromName(nullptr, style);
    SkTypeface* foundFace = SkTypeface::CreateFromName(fontName, style);
    return defaultFace != foundFace;
}

static int whitelist_name_index(const SkTypeface* tf) {

    SkString fontNameStr;
    SkAutoTUnref<SkTypeface::LocalizedStrings> nameIter(
        SkOTUtils::LocalizedStrings_NameTable::CreateForFamilyNames(*tf));
    SkTypeface::LocalizedString familyNameLocalized;
    while (nameIter->next(&familyNameLocalized)) {
        fontNameStr = familyNameLocalized.fString;
        // check against permissible list of names
        for (int i = 0; i < whitelistCount; ++i) {
            if (fontNameStr.equals(whitelist[i].fFontName)) {
                return i;
            }
        }
    }
#if WHITELIST_DEBUG
    SkAutoTUnref<SkTypeface::LocalizedStrings> debugIter(
        SkOTUtils::LocalizedStrings_NameTable::CreateForFamilyNames(*tf));
    while (debugIter->next(&familyNameLocalized)) {
        SkDebugf("no match fontName=\"%s\"\n", familyNameLocalized.fString.c_str());
    }
#endif
    return -1;
}

static uint32_t compute_checksum(const SkTypeface* tf) {
    SkFontData* fontData = tf->createFontData();
    if (!fontData) {
        return 0;
    }
    SkStreamAsset* fontStream = fontData->getStream();
    if (!fontStream) {
        return 0;
    }
    SkTDArray<char> data;
    size_t length = fontStream->getLength();
    if (!length) {
        return 0;
    }
    data.setCount((int) length);
    if (!fontStream->peek(data.begin(), length)) {
        return 0;
    }
    return SkChecksum::Murmur3(data.begin(), length);
}

static void serialize_sub(const char* fontName, SkTypeface::Style style, SkWStream* wstream) {
    SkFontDescriptor desc(style);
    SkString subName(SUBNAME_PREFIX);
    subName.append(fontName);
    const char* familyName = subName.c_str();
    desc.setFamilyName(familyName);
    desc.serialize(wstream);
#if WHITELIST_DEBUG
    for (int i = 0; i < whitelistCount; ++i) {
        if (!strcmp(fontName, whitelist[i].fFontName)) {
            if (!whitelist[i].fSerializedSub) {
                whitelist[i].fSerializedSub = true;
                SkDebugf("%s %s\n", __FUNCTION__, familyName);
            }
            break;
        }
    }
#endif
}

static bool is_local(const SkTypeface* tf) {
    bool isLocal = false;
    SkFontDescriptor desc(tf->style());
    tf->getFontDescriptor(&desc, &isLocal);
    return isLocal;
}

static void serialize_full(const SkTypeface* tf, SkWStream* wstream) {
    bool isLocal = false;
    SkFontDescriptor desc(tf->style());
    tf->getFontDescriptor(&desc, &isLocal);

    // Embed font data if it's a local font.
    if (isLocal && !desc.hasFontData()) {
        desc.setFontData(tf->createFontData());
    }
    desc.serialize(wstream);
}

static void serialize_name_only(const SkTypeface* tf, SkWStream* wstream) {
    bool isLocal = false;
    SkFontDescriptor desc(tf->style());
    tf->getFontDescriptor(&desc, &isLocal);
    SkASSERT(!isLocal);
#if WHITELIST_DEBUG
    const char* familyName = desc.getFamilyName();
    if (familyName) {
        if (!strcmp(familyName, "Times New Roman")) {
            if (!timesNewRomanSerializedNameOnly) {
                timesNewRomanSerializedNameOnly = true;
                SkDebugf("%s %s\n", __FUNCTION__, familyName);
            }
        } else {
            for (int i = 0; i < whitelistCount; ++i) {
                if (!strcmp(familyName, whitelist[i].fFontName)) {
                    if (!whitelist[i].fSerializedNameOnly) {
                        whitelist[i].fSerializedNameOnly = true;
                        SkDebugf("%s %s\n", __FUNCTION__, familyName);
                    }
                    break;
                }
            }
        }
    }
#endif
    desc.serialize(wstream);
}

void WhitelistSerializeTypeface(const SkTypeface* tf, SkWStream* wstream) {
    if (!is_local(tf)) {
        serialize_name_only(tf, wstream);
        return;
    }
    int whitelistIndex = whitelist_name_index(tf);
    if (whitelistIndex < 0) {
        serialize_full(tf, wstream);
        return;
    }
    const char* fontName = whitelist[whitelistIndex].fFontName;
    if (!font_name_is_local(fontName, tf->style())) {
#if WHITELIST_DEBUG
        SkDebugf("name not found locally \"%s\" style=%d\n", fontName, tf->style());
#endif
        serialize_full(tf, wstream);
        return;
    }
    uint32_t checksum = compute_checksum(tf);
    if (whitelist[whitelistIndex].fChecksum != checksum) {
#if WHITELIST_DEBUG
        if (whitelist[whitelistIndex].fChecksum) {
            SkDebugf("!!! checksum changed !!!\n");
        }
        SkDebugf("checksum updated\n");
        SkDebugf("    { \"%s\", 0x%08x },\n", fontName, checksum);
#endif
        whitelist[whitelistIndex].fChecksum = checksum;
    }
    serialize_sub(fontName, tf->style(), wstream);
}

SkTypeface* WhitelistDeserializeTypeface(SkStream* stream) {
    SkFontDescriptor desc;
    if (!SkFontDescriptor::Deserialize(stream, &desc)) {
        return nullptr;
    }

    SkFontData* data = desc.detachFontData();
    if (data) {
        SkTypeface* typeface = SkTypeface::CreateFromFontData(data);
        if (typeface) {
            return typeface;
        }
    }
    const char* familyName = desc.getFamilyName();
    if (!strncmp(SUBNAME_PREFIX, familyName, sizeof(SUBNAME_PREFIX) - 1)) {
        familyName += sizeof(SUBNAME_PREFIX) - 1;
    }
    return SkTypeface::CreateFromName(familyName, desc.getStyle());
}

bool CheckChecksums() {
    for (int i = 0; i < whitelistCount; ++i) {
        const char* fontName = whitelist[i].fFontName;
        SkTypeface* tf = SkTypeface::CreateFromName(fontName, SkTypeface::kNormal);
        uint32_t checksum = compute_checksum(tf);
        if (whitelist[i].fChecksum != checksum) {
            return false;
        }
    }
    return true;
}

const char checksumFileName[] = "SkWhitelistChecksums.cpp";

const char checksumHeader[] =
"/*"                                                                        "\n"
" * Copyright 2015 Google Inc."                                             "\n"
" *"                                                                        "\n"
" * Use of this source code is governed by a BSD-style license that can be" "\n"
" * found in the LICENSE file."                                             "\n"
" *"                                                                        "\n"
" * %s() in %s generated %s."                                               "\n"
" * Run 'whitelist_typefaces --generate' to create anew."                   "\n"
" */"                                                                       "\n"
""                                                                          "\n"
"#include \"SkTDArray.h\""                                                  "\n"
""                                                                          "\n"
"struct Whitelist {"                                                        "\n"
"    const char* fFontName;"                                                "\n"
"    uint32_t fChecksum;"                                                   "\n"
"    bool fSerializedNameOnly;"                                             "\n"
"    bool fSerializedSub;"                                                  "\n"
"};"                                                                        "\n"
""                                                                          "\n"
"static Whitelist whitelist[] = {"                                          "\n";

const char checksumEntry[] =
"    { \"%s\", 0x%08x, false, false },"                                     "\n";

const char checksumTrailer[] =
"};"                                                                        "\n"
""                                                                          "\n"
"static const int whitelistCount = (int) SK_ARRAY_COUNT(whitelist);"        "\n";


#include "SkOSFile.h"

bool GenerateChecksums() {
    FILE* file = sk_fopen(checksumFileName, kWrite_SkFILE_Flag);
    if (!file) {
        SkDebugf("Can't open %s for writing.\n", checksumFileName);
        return false;
    }
    SkString line;
    line.printf(checksumHeader, __FUNCTION__, __FILE__, checksumFileName);
    sk_fwrite(line.c_str(), line.size(), file);
    for (int i = 0; i < whitelistCount; ++i) {
        const char* fontName = whitelist[i].fFontName;
        SkTypeface* tf = SkTypeface::CreateFromName(fontName, SkTypeface::kNormal);
        uint32_t checksum = compute_checksum(tf);
        line.printf(checksumEntry, fontName, checksum);
        sk_fwrite(line.c_str(), line.size(), file);
    }
    sk_fwrite(checksumTrailer, sizeof(checksumTrailer) - 1, file);
    sk_fclose(file);
    return true;
}
