/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkMakeUnique.h"
#include "src/ports/SkFontMgr_custom.h"

struct SkEmbeddedResource { const uint8_t* data; size_t size; };
struct SkEmbeddedResourceHeader { const SkEmbeddedResource* entries; int count; };

class EmbeddedSystemFontLoader : public SkFontMgr_Custom::SystemFontLoader {
public:
    EmbeddedSystemFontLoader(const SkEmbeddedResourceHeader* header) : fHeader(header) { }

    void loadSystemFonts(const SkTypeface_FreeType::Scanner& scanner,
                         SkFontMgr_Custom::Families* families) const override
    {
        for (int i = 0; i < fHeader->count; ++i) {
            const SkEmbeddedResource& fontEntry = fHeader->entries[i];
            load_embedded_font(scanner, fontEntry.data, fontEntry.size, i, families);
        }

        if (families->empty()) {
            SkFontStyleSet_Custom* family = new SkFontStyleSet_Custom(SkString());
            families->push_back().reset(family);
            family->appendTypeface(sk_make_sp<SkTypeface_Empty>());
        }
    }

private:
    static SkFontStyleSet_Custom* find_family(SkFontMgr_Custom::Families& families,
                                              const char familyName[])
    {
       for (int i = 0; i < families.count(); ++i) {
            if (families[i]->getFamilyName().equals(familyName)) {
                return families[i].get();
            }
        }
        return nullptr;
    }

    static void load_embedded_font(const SkTypeface_FreeType::Scanner& scanner,
                                   const uint8_t* data, size_t size, int index,
                                   SkFontMgr_Custom::Families* families)
    {
        auto stream = skstd::make_unique<SkMemoryStream>(data, size, false);

        int numFaces;
        if (!scanner.recognizedFont(stream.get(), &numFaces)) {
            SkDebugf("---- failed to open <%d> as a font\n", index);
            return;
        }

        for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex) {
            bool isFixedPitch;
            SkString realname;
            SkFontStyle style = SkFontStyle(); // avoid uninitialized warning
            if (!scanner.scanFont(stream.get(), faceIndex,
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
            auto data = skstd::make_unique<SkFontData>(std::move(stream), faceIndex, nullptr, 0);
            addTo->appendTypeface(sk_make_sp<SkTypeface_Stream>(std::move(data),
                                                                style, isFixedPitch,
                                                                true, realname));
        }
    }

    const SkEmbeddedResourceHeader* fHeader;
};

sk_sp<SkFontMgr> SkFontMgr_New_Custom_Embedded(const SkEmbeddedResourceHeader* header) {
    return sk_make_sp<SkFontMgr_Custom>(EmbeddedSystemFontLoader(header));
}
