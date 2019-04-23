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
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkMakeUnique.h"
#include "src/ports/SkFontHost_FreeType_common.h"
#include "src/ports/SkFontMgr_custom.h"

#include <limits>
#include <memory>

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
    *isLocal = !this->isSysFont();
}

int SkTypeface_Custom::getIndex() const { return fIndex; }


SkTypeface_Empty::SkTypeface_Empty() : INHERITED(SkFontStyle(), false, true, SkString(), 0) {}

std::unique_ptr<SkStreamAsset> SkTypeface_Empty::onOpenStream(int*) const { return nullptr; }

sk_sp<SkTypeface> SkTypeface_Empty::onMakeClone(const SkFontArguments& args) const {
    return sk_ref_sp(this);
}

SkTypeface_Stream::SkTypeface_Stream(std::unique_ptr<SkFontData> fontData,
                                     const SkFontStyle& style, bool isFixedPitch, bool sysFont,
                                     const SkString familyName)
    : INHERITED(style, isFixedPitch, sysFont, familyName, fontData->getIndex())
    , fData(std::move(fontData))
{ }

std::unique_ptr<SkStreamAsset> SkTypeface_Stream::onOpenStream(int* ttcIndex) const {
    *ttcIndex = fData->getIndex();
    return fData->getStream()->duplicate();
}

std::unique_ptr<SkFontData> SkTypeface_Stream::onMakeFontData() const {
    return skstd::make_unique<SkFontData>(*fData);
}

sk_sp<SkTypeface> SkTypeface_Stream::onMakeClone(const SkFontArguments& args) const {
    std::unique_ptr<SkFontData> data = this->cloneFontData(args);
    if (!data) {
        return nullptr;
    }

    SkString familyName;
    this->getFamilyName(&familyName);

    return sk_make_sp<SkTypeface_Stream>(std::move(data),
                                         this->fontStyle(),
                                         this->isFixedPitch(),
                                         this->isSysFont(),
                                         familyName);
}

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

    return sk_make_sp<SkTypeface_Stream>(std::move(data),
                                         this->fontStyle(),
                                         this->isFixedPitch(),
                                         this->isSysFont(),
                                         familyName);
}

///////////////////////////////////////////////////////////////////////////////

SkFontStyleSet_Custom::SkFontStyleSet_Custom(const SkString familyName) : fFamilyName(familyName) {}

void SkFontStyleSet_Custom::appendTypeface(sk_sp<SkTypeface_Custom> typeface) {
    fStyles.emplace_back(std::move(typeface));
}

int SkFontStyleSet_Custom::count() {
    return fStyles.count();
}

void SkFontStyleSet_Custom::getStyle(int index, SkFontStyle* style, SkString* name) {
    SkASSERT(index < fStyles.count());
    if (style) {
        *style = fStyles[index]->fontStyle();
    }
    if (name) {
        name->reset();
    }
}

SkTypeface* SkFontStyleSet_Custom::createTypeface(int index) {
    SkASSERT(index < fStyles.count());
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
        "Arial", "Verdana", "Times New Roman", "Droid Sans", nullptr
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(defaultNames); ++i) {
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
    return fFamilies.count();
}

void SkFontMgr_Custom::onGetFamilyName(int index, SkString* familyName) const {
    SkASSERT(index < fFamilies.count());
    familyName->set(fFamilies[index]->getFamilyName());
}

SkFontStyleSet_Custom* SkFontMgr_Custom::onCreateStyleSet(int index) const {
    SkASSERT(index < fFamilies.count());
    return SkRef(fFamilies[index].get());
}

SkFontStyleSet_Custom* SkFontMgr_Custom::onMatchFamily(const char familyName[]) const {
    for (int i = 0; i < fFamilies.count(); ++i) {
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

SkTypeface* SkFontMgr_Custom::onMatchFaceStyle(const SkTypeface* familyMember,
                                               const SkFontStyle& fontStyle) const
{
    for (int i = 0; i < fFamilies.count(); ++i) {
        for (int j = 0; j < fFamilies[i]->fStyles.count(); ++j) {
            if (fFamilies[i]->fStyles[j].get() == familyMember) {
                return fFamilies[i]->matchStyle(fontStyle);
            }
        }
    }
    return nullptr;
}

sk_sp<SkTypeface> SkFontMgr_Custom::onMakeFromData(sk_sp<SkData> data, int ttcIndex) const {
    return this->makeFromStream(skstd::make_unique<SkMemoryStream>(std::move(data)), ttcIndex);
}

sk_sp<SkTypeface> SkFontMgr_Custom::onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                                          int ttcIndex) const {
    return this->makeFromStream(std::move(stream), SkFontArguments().setCollectionIndex(ttcIndex));
}

sk_sp<SkTypeface> SkFontMgr_Custom::onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                                         const SkFontArguments& args) const {
    using Scanner = SkTypeface_FreeType::Scanner;
    bool isFixedPitch;
    SkFontStyle style;
    SkString name;
    Scanner::AxisDefinitions axisDefinitions;
    if (!fScanner.scanFont(stream.get(), args.getCollectionIndex(),
                            &name, &style, &isFixedPitch, &axisDefinitions))
    {
        return nullptr;
    }

    const SkFontArguments::VariationPosition position = args.getVariationDesignPosition();
    SkAutoSTMalloc<4, SkFixed> axisValues(axisDefinitions.count());
    Scanner::computeAxisValues(axisDefinitions, position, axisValues, name);

    auto data = skstd::make_unique<SkFontData>(std::move(stream), args.getCollectionIndex(),
                                               axisValues.get(), axisDefinitions.count());
    return sk_sp<SkTypeface>(new SkTypeface_Stream(std::move(data), style, isFixedPitch, false, name));
}

sk_sp<SkTypeface> SkFontMgr_Custom::onMakeFromFontData(std::unique_ptr<SkFontData> data) const {
    bool isFixedPitch;
    SkFontStyle style;
    SkString name;
    if (!fScanner.scanFont(data->getStream(), data->getIndex(),
                            &name, &style, &isFixedPitch, nullptr)) {
        return nullptr;
    }
    return sk_sp<SkTypeface>(new SkTypeface_Stream(std::move(data), style, isFixedPitch, false, name));
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
