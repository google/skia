/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/ports/SkFontMgr_data.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontScanner.h"
#include "src/ports/SkFontMgr_custom.h"

struct SkEmbeddedResource { const uint8_t* data; size_t size; };
struct SkEmbeddedResourceHeader { const SkEmbeddedResource* entries; int count; };

static void load_font_from_data(const SkFontScanner* scanner,
                                std::unique_ptr<SkMemoryStream> stream, int index,
                                SkFontMgr_Custom::Families* families);

class EmbeddedSystemFontLoader : public SkFontMgr_Custom::SystemFontLoader {
public:
    EmbeddedSystemFontLoader(const SkEmbeddedResourceHeader* header) : fHeader(header) { }

    void loadSystemFonts(const SkFontScanner* scanner,
                         SkFontMgr_Custom::Families* families) const override
    {
        for (int i = 0; i < fHeader->count; ++i) {
            const SkEmbeddedResource& fontEntry = fHeader->entries[i];
            auto stream = std::make_unique<SkMemoryStream>(fontEntry.data, fontEntry.size, false);
            load_font_from_data(scanner, std::move(stream), i, families);
        }

        if (families->empty()) {
            SkFontStyleSet_Custom* family = new SkFontStyleSet_Custom(SkString());
            families->push_back().reset(family);
            family->appendTypeface(sk_make_sp<SkTypeface_Empty>());
        }
    }

    const SkEmbeddedResourceHeader* fHeader;
};

class DataFontLoader : public SkFontMgr_Custom::SystemFontLoader {
public:
    DataFontLoader(sk_sp<SkData>* datas, int n) : fDatas(datas), fNum(n) { }

    void loadSystemFonts(const SkFontScanner* scanner,
                         SkFontMgr_Custom::Families* families) const override
    {
        for (int i = 0; i < fNum; ++i) {
            auto stream = std::make_unique<SkMemoryStream>(fDatas[i]);
            load_font_from_data(scanner, std::move(stream), i, families);
        }

        if (families->empty()) {
            SkFontStyleSet_Custom* family = new SkFontStyleSet_Custom(SkString());
            families->push_back().reset(family);
            family->appendTypeface(sk_make_sp<SkTypeface_Empty>());
        }
    }

    const sk_sp<SkData>* fDatas;
    const int fNum;
};

static SkFontStyleSet_Custom* find_family(SkFontMgr_Custom::Families& families,
                                          const char familyName[])
{
   for (int i = 0; i < families.size(); ++i) {
        if (families[i]->getFamilyName().equals(familyName)) {
            return families[i].get();
        }
    }
    return nullptr;
}

static void load_font_from_data(const SkFontScanner* scanner,
                                std::unique_ptr<SkMemoryStream> stream, int index,
                                SkFontMgr_Custom::Families* families)
{
    int numFaces;
    if (!scanner->recognizedFont(stream.get(), &numFaces)) {
        SkDebugf("---- failed to open <%d> as a font\n", index);
        return;
    }

    for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex) {
        bool isFixedPitch;
        SkString realname;
        SkFontStyle style = SkFontStyle(); // avoid uninitialized warning
        if (!scanner->scanFont(stream.get(), faceIndex,
                              &realname, &style, &isFixedPitch, nullptr))
        {
            SkDebugf("---- failed to open <%d> <%d> as a font\n", index, faceIndex);
            return;
        }

        SkFontStyleSet_Custom* addTo = find_family(*families, realname.c_str());
        if (nullptr == addTo) {
            addTo = new SkFontStyleSet_Custom(realname);
            families->push_back().reset(addTo);
        }
        auto data = std::make_unique<SkFontData>(stream->duplicate(), faceIndex, 0,
                                                 nullptr, 0, nullptr, 0);
        addTo->appendTypeface(sk_make_sp<SkTypeface_FreeTypeStream>(
            std::move(data), realname, style, isFixedPitch));
    }
}

sk_sp<SkFontMgr> SkFontMgr_New_Custom_Embedded(const SkEmbeddedResourceHeader* header) {
    return sk_make_sp<SkFontMgr_Custom>(EmbeddedSystemFontLoader(header));
}

sk_sp<SkFontMgr> SkFontMgr_New_Custom_Data(SkSpan<sk_sp<SkData>> datas) {
    SkASSERT(!datas.empty());
    return sk_make_sp<SkFontMgr_Custom>(DataFontLoader(datas.data(), datas.size()));
}
