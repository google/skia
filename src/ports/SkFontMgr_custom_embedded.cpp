/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "src/core/SkFontDescriptor.h"
#include "src/ports/SkFontMgr_custom.h"

struct SkEmbeddedResource { const uint8_t* data; size_t size; };
struct SkEmbeddedResourceHeader { const SkEmbeddedResource* entries; int count; };

static int load_font_from_data(const SkTypeface_FreeType::Scanner& scanner,
                                const uint8_t* data, size_t size, int index,
                                SkFontMgr_Custom_Families* families);

class EmbeddedSystemFontLoader : public SkFontMgr_Custom_SystemFontLoader {
public:
    EmbeddedSystemFontLoader(const SkEmbeddedResourceHeader* header) : fHeader(header) { }

    void loadSystemFonts(const SkTypeface_FreeType::Scanner& scanner,
                         SkFontMgr_Custom_Families* families) const override
    {
        int loaded = 0;
        if (fHeader) {
            for (int i = 0; i < fHeader->count; ++i) {
                const SkEmbeddedResource& fontEntry = fHeader->entries[i];
                loaded += load_font_from_data(scanner, fontEntry.data, fontEntry.size, i, families);
            }
        }

        if (families->empty() && loaded) {
            SkFontStyleSet_Custom* family = new SkFontStyleSet_Custom(SkString());
            families->push_back().reset(family);
            family->appendTypeface(sk_make_sp<SkTypeface_Empty>());
        }
    }

private:
    const SkEmbeddedResourceHeader* fHeader;
};

DataFontLoader::DataFontLoader(const uint8_t** datas, const size_t* sizes, int n)
    : fDatas(datas)
    , fSizes(sizes)
    , fNum(n)
{
    printf("Here51 %zu %d\n", *sizes, n);
}

void DataFontLoader::loadSystemFonts(const SkTypeface_FreeType::Scanner& scanner,
                                     SkFontMgr_Custom_Families* families) const
{
    int loaded = 0;
    for (int i = 0; i < fNum; ++i) {
        loaded += load_font_from_data(scanner, fDatas[i], fSizes[i], i, families);
    }

    if (families->empty() && loaded) {
        SkFontStyleSet_Custom* family = new SkFontStyleSet_Custom(SkString());
        families->push_back().reset(family);
        family->appendTypeface(sk_make_sp<SkTypeface_Empty>());
    }
}

static SkFontStyleSet_Custom* find_family(SkFontMgr_Custom_Families& families,
                                          const char familyName[])
{
   for (int i = 0; i < families.count(); ++i) {
        if (families[i]->getFamilyName().equals(familyName)) {
            return families[i].get();
        }
    }
    return nullptr;
}

static int load_font_from_data(const SkTypeface_FreeType::Scanner& scanner,
                                const uint8_t* data, size_t size, int index,
                                SkFontMgr_Custom_Families* families)
{
    if (!data || !size) {
        SkDebugf("---- either because of data=%p or size=%d fail to open <%d> as a font\n", data, size, index);
        return 0;
    }

    auto stream = std::make_unique<SkMemoryStream>(data, size, false);

    int numFaces;
    if (!scanner.recognizedFont(stream.get(), &numFaces)) {
        SkDebugf("---- failed to open <%d> as a font\n", index);
        return 0;
    }

    for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex) {
        bool isFixedPitch;
        SkString realname;
        SkFontStyle style = SkFontStyle(); // avoid uninitialized warning
        if (!scanner.scanFont(stream.get(), faceIndex,
                              &realname, &style, &isFixedPitch, nullptr))
        {
            SkDebugf("---- failed to open <%d> <%d> as a font\n", index, faceIndex);
            return 0;
        }

        SkFontStyleSet_Custom* addTo = find_family(*families, realname.c_str());
        if (nullptr == addTo) {
            addTo = new SkFontStyleSet_Custom(realname);
            families->push_back().reset(addTo);
        }
        auto data = std::make_unique<SkFontData>(stream->duplicate(), faceIndex, nullptr, 0);
        addTo->appendTypeface(sk_make_sp<SkTypeface_Stream>(std::move(data),
                                                            style, isFixedPitch,
                                                            true, realname));
    }
    return (0 < numFaces) ? 1 : 0; // At least one is loaded
}

sk_sp<SkFontMgr> SkFontMgr_New_Custom_Embedded(const SkEmbeddedResourceHeader* header) {
    return sk_make_sp<SkFontMgr_Custom>(EmbeddedSystemFontLoader(header));
}

// SkFontMgr_New_Custom_Data expects to be called with the data for n font files. datas and sizes
// are parallel arrays of bytes and byte lengths.
sk_sp<SkFontMgr> SkFontMgr_New_Custom_Data(const uint8_t** datas, const size_t* sizes, int n) {
    SkASSERT(datas != nullptr);
    SkASSERT(sizes != nullptr);
    SkASSERT(n > 0);
    return sk_make_sp<SkFontMgr_Custom>(DataFontLoader(datas, sizes, n));
}
