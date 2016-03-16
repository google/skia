/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDataTable.h"
#include "SkFontMgr.h"
#include "SkFontMgr_indirect.h"
#include "SkFontStyle.h"
#include "SkMutex.h"
#include "SkOnce.h"
#include "SkRefCnt.h"
#include "SkRemotableFontMgr.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTypeface.h"
#include "SkTypes.h"
#include "SkTemplates.h"

class SkData;

class SkStyleSet_Indirect : public SkFontStyleSet {
public:
    /** Takes ownership of the SkRemotableFontIdentitySet. */
    SkStyleSet_Indirect(const SkFontMgr_Indirect* owner, int familyIndex,
                        SkRemotableFontIdentitySet* data)
        : fOwner(SkRef(owner)), fFamilyIndex(familyIndex), fData(data)
    { }

    int count() override { return fData->count(); }

    void getStyle(int index, SkFontStyle* fs, SkString* style) override {
        if (fs) {
            *fs = fData->at(index).fFontStyle;
        }
        if (style) {
            // TODO: is this useful? Current locale?
            style->reset();
        }
    }

    SkTypeface* createTypeface(int index) override {
        return fOwner->createTypefaceFromFontId(fData->at(index));
    }

    SkTypeface* matchStyle(const SkFontStyle& pattern) override {
        if (fFamilyIndex >= 0) {
            SkFontIdentity id = fOwner->fProxy->matchIndexStyle(fFamilyIndex, pattern);
            return fOwner->createTypefaceFromFontId(id);
        }

        return this->matchStyleCSS3(pattern);
    }
private:
    SkAutoTUnref<const SkFontMgr_Indirect> fOwner;
    int fFamilyIndex;
    SkAutoTUnref<SkRemotableFontIdentitySet> fData;
};

void SkFontMgr_Indirect::set_up_family_names(const SkFontMgr_Indirect* self) {
    self->fFamilyNames.reset(self->fProxy->getFamilyNames());
}

int SkFontMgr_Indirect::onCountFamilies() const {
    SkOnce(&fFamilyNamesInited, &fFamilyNamesMutex, SkFontMgr_Indirect::set_up_family_names, this);
    return fFamilyNames->count();
}

void SkFontMgr_Indirect::onGetFamilyName(int index, SkString* familyName) const {
    SkOnce(&fFamilyNamesInited, &fFamilyNamesMutex, SkFontMgr_Indirect::set_up_family_names, this);
    if (index >= fFamilyNames->count()) {
        familyName->reset();
        return;
    }
    familyName->set(fFamilyNames->atStr(index));
}

SkFontStyleSet* SkFontMgr_Indirect::onCreateStyleSet(int index) const {
    SkRemotableFontIdentitySet* set = fProxy->getIndex(index);
    if (nullptr == set) {
        return nullptr;
    }
    return new SkStyleSet_Indirect(this, index, set);
}

SkFontStyleSet* SkFontMgr_Indirect::onMatchFamily(const char familyName[]) const {
    return new SkStyleSet_Indirect(this, -1, fProxy->matchName(familyName));
}

SkTypeface* SkFontMgr_Indirect::createTypefaceFromFontId(const SkFontIdentity& id) const {
    if (id.fDataId == SkFontIdentity::kInvalidDataId) {
        return nullptr;
    }

    SkAutoMutexAcquire ama(fDataCacheMutex);

    SkAutoTUnref<SkTypeface> dataTypeface;
    int dataTypefaceIndex = 0;
    for (int i = 0; i < fDataCache.count(); ++i) {
        const DataEntry& entry = fDataCache[i];
        if (entry.fDataId == id.fDataId) {
            if (entry.fTtcIndex == id.fTtcIndex &&
                !entry.fTypeface->weak_expired() && entry.fTypeface->try_ref())
            {
                return entry.fTypeface;
            }
            if (dataTypeface.get() == nullptr &&
                !entry.fTypeface->weak_expired() && entry.fTypeface->try_ref())
            {
                dataTypeface.reset(entry.fTypeface);
                dataTypefaceIndex = entry.fTtcIndex;
            }
        }

        if (entry.fTypeface->weak_expired()) {
            fDataCache.removeShuffle(i);
            --i;
        }
    }

    // No exact match, but did find a data match.
    if (dataTypeface.get() != nullptr) {
        SkAutoTDelete<SkStreamAsset> stream(dataTypeface->openStream(nullptr));
        if (stream.get() != nullptr) {
            return fImpl->createFromStream(stream.release(), dataTypefaceIndex);
        }
    }

    // No data match, request data and add entry.
    SkAutoTDelete<SkStreamAsset> stream(fProxy->getData(id.fDataId));
    if (stream.get() == nullptr) {
        return nullptr;
    }

    SkAutoTUnref<SkTypeface> typeface(fImpl->createFromStream(stream.release(), id.fTtcIndex));
    if (typeface.get() == nullptr) {
        return nullptr;
    }

    DataEntry& newEntry = fDataCache.push_back();
    typeface->weak_ref();
    newEntry.fDataId = id.fDataId;
    newEntry.fTtcIndex = id.fTtcIndex;
    newEntry.fTypeface = typeface.get();  // weak reference passed to new entry.

    return typeface.release();
}

SkTypeface* SkFontMgr_Indirect::onMatchFamilyStyle(const char familyName[],
                                                   const SkFontStyle& fontStyle) const {
    SkFontIdentity id = fProxy->matchNameStyle(familyName, fontStyle);
    return this->createTypefaceFromFontId(id);
}

SkTypeface* SkFontMgr_Indirect::onMatchFamilyStyleCharacter(const char familyName[],
                                                            const SkFontStyle& style,
                                                            const char* bcp47[],
                                                            int bcp47Count,
                                                            SkUnichar character) const {
    SkFontIdentity id = fProxy->matchNameStyleCharacter(familyName, style, bcp47,
                                                        bcp47Count, character);
    return this->createTypefaceFromFontId(id);
}

SkTypeface* SkFontMgr_Indirect::onMatchFaceStyle(const SkTypeface* familyMember,
                                                 const SkFontStyle& fontStyle) const {
    SkString familyName;
    familyMember->getFamilyName(&familyName);
    return this->matchFamilyStyle(familyName.c_str(), fontStyle);
}

SkTypeface* SkFontMgr_Indirect::onCreateFromStream(SkStreamAsset* stream, int ttcIndex) const {
    return fImpl->createFromStream(stream, ttcIndex);
}

SkTypeface* SkFontMgr_Indirect::onCreateFromFile(const char path[], int ttcIndex) const {
    return fImpl->createFromFile(path, ttcIndex);
}

SkTypeface* SkFontMgr_Indirect::onCreateFromData(SkData* data, int ttcIndex) const {
    return fImpl->createFromData(data, ttcIndex);
}

SkTypeface* SkFontMgr_Indirect::onLegacyCreateTypeface(const char familyName[],
                                                       unsigned styleBits) const {
    bool bold = SkToBool(styleBits & SkTypeface::kBold);
    bool italic = SkToBool(styleBits & SkTypeface::kItalic);
    SkFontStyle style = SkFontStyle(bold ? SkFontStyle::kBold_Weight
                                         : SkFontStyle::kNormal_Weight,
                                    SkFontStyle::kNormal_Width,
                                    italic ? SkFontStyle::kItalic_Slant
                                           : SkFontStyle::kUpright_Slant);

    SkAutoTUnref<SkTypeface> face(this->matchFamilyStyle(familyName, style));

    if (nullptr == face.get()) {
        face.reset(this->matchFamilyStyle(nullptr, style));
    }

    if (nullptr == face.get()) {
        SkFontIdentity fontId = this->fProxy->matchIndexStyle(0, style);
        face.reset(this->createTypefaceFromFontId(fontId));
    }

    return face.release();
}
