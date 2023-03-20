/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkFontDescriptor.h"
#include "src/ports/SkFontHost_FreeType_common.h"
#include "src/ports/SkFontMgr_custom.h"

#include <limits>
#include <memory>

using namespace skia_private;

class SkData;

SkTypeface_Custom::SkTypeface_Custom(const SkFontStyle& style, bool isFixedPitch,
                                     bool sysFont, const SkString familyName, int index)
    : INHERITED(style, isFixedPitch)
    , fIsSysFont(sysFont), fFamilyName(familyName), fIndex(index)
{ }

bool SkTypeface_Custom::isSysFont() const { return fIsSysFont; }

void SkTypeface_Custom::onGetFamilyName(SkString* familyName) const {
    *familyName = fFamilyName;
}

void SkTypeface_Custom::onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const {
    desc->setFamilyName(fFamilyName.c_str());
    desc->setStyle(this->fontStyle());
    desc->setFactoryId(SkTypeface_FreeType::FactoryId);
    *isLocal = !this->isSysFont();
}

int SkTypeface_Custom::getIndex() const { return fIndex; }


SkTypeface_Empty::SkTypeface_Empty() : INHERITED(SkFontStyle(), false, true, SkString(), 0) {}

std::unique_ptr<SkStreamAsset> SkTypeface_Empty::onOpenStream(int*) const { return nullptr; }

sk_sp<SkTypeface> SkTypeface_Empty::onMakeClone(const SkFontArguments& args) const {
    return sk_ref_sp(this);
}

std::unique_ptr<SkFontData> SkTypeface_Empty::onMakeFontData() const { return nullptr; }

SkTypeface_File::SkTypeface_File(const SkFontStyle& style, bool isFixedPitch, bool sysFont,
                                 const SkString familyName, const char path[], int index)
    : INHERITED(style, isFixedPitch, sysFont, familyName, index)
    , fPath(path)
{ }

std::unique_ptr<SkStreamAsset> SkTypeface_File::onOpenStream(int* ttcIndex) const {
    *ttcIndex = this->getIndex();
    return SkStream::MakeFromFile(fPath.c_str());
}

sk_sp<SkTypeface> SkTypeface_File::onMakeClone(const SkFontArguments& args) const {
    std::unique_ptr<SkFontData> data = this->cloneFontData(args);
    if (!data) {
        return nullptr;
    }

    SkString familyName;
    this->getFamilyName(&familyName);

    return sk_make_sp<SkTypeface_FreeTypeStream>(std::move(data),
                                                 familyName,
                                                 this->fontStyle(),
                                                 this->isFixedPitch());
}

std::unique_ptr<SkFontData> SkTypeface_File::onMakeFontData() const {
    int index;
    std::unique_ptr<SkStreamAsset> stream(this->onOpenStream(&index));
    if (!stream) {
        return nullptr;
    }
    return std::make_unique<SkFontData>(std::move(stream), index, 0, nullptr, 0, nullptr, 0);
}

///////////////////////////////////////////////////////////////////////////////

SkFontStyleSet_Custom::SkFontStyleSet_Custom(const SkString familyName) : fFamilyName(familyName) {}

void SkFontStyleSet_Custom::appendTypeface(sk_sp<SkTypeface> typeface) {
    fStyles.emplace_back(std::move(typeface));
}

int SkFontStyleSet_Custom::count() {
    return fStyles.size();
}

void SkFontStyleSet_Custom::getStyle(int index, SkFontStyle* style, SkString* name) {
    SkASSERT(index < fStyles.size());
    if (style) {
        *style = fStyles[index]->fontStyle();
    }
    if (name) {
        name->reset();
    }
}

SkTypeface* SkFontStyleSet_Custom::createTypeface(int index) {
    SkASSERT(index < fStyles.size());
    return SkRef(fStyles[index].get());
}

SkTypeface* SkFontStyleSet_Custom::matchStyle(const SkFontStyle& pattern) {
    return this->matchStyleCSS3(pattern);
}

SkString SkFontStyleSet_Custom::getFamilyName() { return fFamilyName; }


SkFontMgr_Custom::SkFontMgr_Custom(const SystemFontLoader& loader) : fDefaultFamily(nullptr) {
    loader.loadSystemFonts(fScanner, &fFamilies);

    // Try to pick a default font.
    static const char* defaultNames[] = {
        "Arial", "Verdana", "Times New Roman", "Droid Sans", "DejaVu Serif", nullptr
    };
    for (size_t i = 0; i < std::size(defaultNames); ++i) {
        sk_sp<SkFontStyleSet_Custom> set(this->onMatchFamily(defaultNames[i]));
        if (nullptr == set) {
            continue;
        }

        sk_sp<SkTypeface> tf(set->matchStyle(SkFontStyle(SkFontStyle::kNormal_Weight,
                                                         SkFontStyle::kNormal_Width,
                                                         SkFontStyle::kUpright_Slant)));
        if (nullptr == tf) {
            continue;
        }

        fDefaultFamily = set.get();
        break;
    }
    if (nullptr == fDefaultFamily) {
        fDefaultFamily = fFamilies[0].get();
    }
}

int SkFontMgr_Custom::onCountFamilies() const {
    return fFamilies.size();
}

void SkFontMgr_Custom::onGetFamilyName(int index, SkString* familyName) const {
    SkASSERT(index < fFamilies.size());
    familyName->set(fFamilies[index]->getFamilyName());
}

SkFontStyleSet_Custom* SkFontMgr_Custom::onCreateStyleSet(int index) const {
    SkASSERT(index < fFamilies.size());
    return SkRef(fFamilies[index].get());
}

SkFontStyleSet_Custom* SkFontMgr_Custom::onMatchFamily(const char familyName[]) const {
    for (int i = 0; i < fFamilies.size(); ++i) {
        if (fFamilies[i]->getFamilyName().equals(familyName)) {
            return SkRef(fFamilies[i].get());
        }
    }
    return nullptr;
}

SkTypeface* SkFontMgr_Custom::onMatchFamilyStyle(const char familyName[],
                                const SkFontStyle& fontStyle) const
{
    sk_sp<SkFontStyleSet> sset(this->matchFamily(familyName));
    return sset->matchStyle(fontStyle);
}

SkTypeface* SkFontMgr_Custom::onMatchFamilyStyleCharacter(const char familyName[],
                                                          const SkFontStyle&,
                                                          const char* bcp47[], int bcp47Count,
                                                          SkUnichar character) const
{
    return nullptr;
}

sk_sp<SkTypeface> SkFontMgr_Custom::onMakeFromData(sk_sp<SkData> data, int ttcIndex) const {
    return this->makeFromStream(std::make_unique<SkMemoryStream>(std::move(data)), ttcIndex);
}

sk_sp<SkTypeface> SkFontMgr_Custom::onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                                          int ttcIndex) const {
    return this->makeFromStream(std::move(stream), SkFontArguments().setCollectionIndex(ttcIndex));
}

sk_sp<SkTypeface> SkFontMgr_Custom::onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                                         const SkFontArguments& args) const {
    return SkTypeface_FreeType::MakeFromStream(std::move(stream), args);
}

sk_sp<SkTypeface> SkFontMgr_Custom::onMakeFromFile(const char path[], int ttcIndex) const {
    std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path);
    return stream ? this->makeFromStream(std::move(stream), ttcIndex) : nullptr;
}

sk_sp<SkTypeface> SkFontMgr_Custom::onLegacyMakeTypeface(const char familyName[],
                                                         SkFontStyle style) const {
    sk_sp<SkTypeface> tf;

    if (familyName) {
        tf.reset(this->onMatchFamilyStyle(familyName, style));
    }

    if (nullptr == tf) {
        tf.reset(fDefaultFamily->matchStyle(style));
    }

    return tf;
}
