/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontMgr_indirect.h"

#include "SkDataTable.h"
#include "SkFontStyle.h"
#include "SkOnce.h"
#include "SkStream.h"
#include "SkTSearch.h"
#include "SkTypeface.h"

class SkData;
class SkString;

class SkStyleSet_Indirect : public SkFontStyleSet {
public:
    /** Takes ownership of the SkRemotableFontIdentitySet. */
    SkStyleSet_Indirect(const SkFontMgr_Indirect* owner, int familyIndex,
                        SkRemotableFontIdentitySet* data)
        : fOwner(SkRef(owner)), fFamilyIndex(familyIndex), fData(data)
    { }

    virtual int count() SK_OVERRIDE { return fData->count(); }

    virtual void getStyle(int index, SkFontStyle* fs, SkString* style) SK_OVERRIDE {
        if (fs) {
            *fs = fData->at(index).fFontStyle;
        }
        if (style) {
            // TODO: is this useful? Current locale?
            style->reset();
        }
    }

    virtual SkTypeface* createTypeface(int index) SK_OVERRIDE {
        return fOwner->createTypefaceFromFontId(fData->at(index));
    }

    virtual SkTypeface* matchStyle(const SkFontStyle& pattern) SK_OVERRIDE {
        if (fFamilyIndex >= 0) {
            SkFontIdentity id = fOwner->fProxy->matchIndexStyle(fFamilyIndex, pattern);
            return fOwner->createTypefaceFromFontId(id);
        }

        // If this SkStyleSet was created via onMatchFamily we would need a call like
        // fOwner->fProxy->matchNameStyle(fFamilyName, pattern);
        // but would not activate fonts (only consider fonts which would come back from matchName).

        // CSS policy sounds good.
        struct Score {
            int score;
            int index;
        };

        // Width has the greatest priority.
        // If the value of pattern.width is 5 (normal) or less,
        //    narrower width values are checked first, then wider values.
        // If the value of pattern.width is greater than 5 (normal),
        //    wider values are checked first, followed by narrower values.

        // Italic/Oblique has the next highest priority.
        // If italic requested and there is some italic font, use it.
        // If oblique requested and there is some oblique font, use it.
        // If italic requested and there is some oblique font, use it.
        // If oblique requested and there is some italic font, use it.

        // Exact match.
        // If pattern.weight < 400, weights below pattern.weight are checked
        //   in descending order followed by weights above pattern.weight
        //   in ascending order until a match is found.
        // If pattern.weight > 500, weights above pattern.weight are checked
        //   in ascending order followed by weights below pattern.weight
        //   in descending order until a match is found.
        // If pattern.weight is 400, 500 is checked first
        //   and then the rule for pattern.weight < 400 is used.
        // If pattern.weight is 500, 400 is checked first
        //   and then the rule for pattern.weight < 400 is used

        Score maxScore = { 0, 0 };
        for (int i = 0; i < fData->count(); ++i) {
            const SkFontStyle& current = fData->at(i).fFontStyle;
            Score currentScore = { 0, i };

            // CSS stretch. (This is the width.)
            // This has the highest priority.
            if (pattern.width() <= SkFontStyle::kNormal_Width) {
                if (current.width() <= pattern.width()) {
                    currentScore.score += 10 - pattern.width() + current.width();
                } else {
                    currentScore.score += 10 - current.width();
                }
            } else {
                if (current.width() > pattern.width()) {
                    currentScore.score += 10 + pattern.width() - current.width();
                } else {
                    currentScore.score += current.width();
                }
            }
            currentScore.score *= 1002;

            // CSS style (italic/oblique)
            // Being italic trumps all valid weights which are not italic.
            // Note that newer specs differentiate between italic and oblique.
            if (pattern.isItalic() && current.isItalic()) {
                currentScore.score += 1001;
            }

            // Synthetics (weight/style) [no stretch synthetic?]

            // The 'closer' to the target weight, the higher the score.
            // 1000 is the 'heaviest' recognized weight
            if (pattern.weight() == current.weight()) {
                currentScore.score += 1000;
            } else if (pattern.weight() <= 500) {
                if (pattern.weight() >= 400 && pattern.weight() < 450) {
                    if (current.weight() >= 450 && current.weight() <= 500) {
                        // Artificially boost the 500 weight.
                        // TODO: determine correct number to use.
                        currentScore.score += 500;
                    }
                }
                if (current.weight() <= pattern.weight()) {
                    currentScore.score += 1000 - pattern.weight() + current.weight();
                } else {
                    currentScore.score += 1000 - current.weight();
                }
            } else if (pattern.weight() > 500) {
                if (current.weight() > pattern.weight()) {
                    currentScore.score += 1000 + pattern.weight() - current.weight();
                } else {
                    currentScore.score += current.weight();
                }
            }

            if (currentScore.score > maxScore.score) {
                maxScore = currentScore;
            }
        }

        return this->createTypeface(maxScore.index);
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
    if (NULL == set) {
        return NULL;
    }
    return SkNEW_ARGS(SkStyleSet_Indirect, (this, index, set));
}

SkFontStyleSet* SkFontMgr_Indirect::onMatchFamily(const char familyName[]) const {
    return SkNEW_ARGS(SkStyleSet_Indirect, (this, -1, fProxy->matchName(familyName)));
}

SkTypeface* SkFontMgr_Indirect::createTypefaceFromFontId(const SkFontIdentity& id) const {
    if (id.fDataId == SkFontIdentity::kInvalidDataId) {
        return NULL;
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
            if (dataTypeface.get() == NULL &&
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
    if (dataTypeface.get() != NULL) {
        SkAutoTUnref<SkStream> stream(dataTypeface->openStream(NULL));
        if (stream.get() != NULL) {
            return fImpl->createFromStream(stream.get(), dataTypefaceIndex);
        }
    }

    // No data match, request data and add entry.
    SkAutoTUnref<SkStreamAsset> stream(fProxy->getData(id.fDataId));
    if (stream.get() == NULL) {
        return NULL;
    }

    SkAutoTUnref<SkTypeface> typeface(fImpl->createFromStream(stream, id.fTtcIndex));
    if (typeface.get() == NULL) {
        return NULL;
    }

    DataEntry& newEntry = fDataCache.push_back();
    typeface->weak_ref();
    newEntry.fDataId = id.fDataId;
    newEntry.fTtcIndex = id.fTtcIndex;
    newEntry.fTypeface = typeface.get();  // weak reference passed to new entry.

    return typeface.detach();
}

SkTypeface* SkFontMgr_Indirect::onMatchFamilyStyle(const char familyName[],
                                                   const SkFontStyle& fontStyle) const {
    SkFontIdentity id = fProxy->matchNameStyle(familyName, fontStyle);
    return this->createTypefaceFromFontId(id);
}

#ifdef SK_FM_NEW_MATCH_FAMILY_STYLE_CHARACTER
SkTypeface* SkFontMgr_Indirect::onMatchFamilyStyleCharacter(const char familyName[],
                                                            const SkFontStyle& style,
                                                            const char* bcp47[],
                                                            int bcp47Count,
                                                            SkUnichar character) const {
    SkFontIdentity id = fProxy->matchNameStyleCharacter(familyName, style, bcp47,
                                                        bcp47Count, character);
    return this->createTypefaceFromFontId(id);
}
#else
SkTypeface* SkFontMgr_Indirect::onMatchFamilyStyleCharacter(const char familyName[],
                                                            const SkFontStyle& style,
                                                            const char bcp47[],
                                                            SkUnichar character) const {
    SkFontIdentity id = fProxy->matchNameStyleCharacter(familyName, style, bcp47, character);
    return this->createTypefaceFromFontId(id);
}
#endif

SkTypeface* SkFontMgr_Indirect::onMatchFaceStyle(const SkTypeface* familyMember,
                                                 const SkFontStyle& fontStyle) const {
    SkString familyName;
    familyMember->getFamilyName(&familyName);
    return this->matchFamilyStyle(familyName.c_str(), fontStyle);
}

SkTypeface* SkFontMgr_Indirect::onCreateFromStream(SkStream* stream, int ttcIndex) const {
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

    if (NULL == face.get()) {
        face.reset(this->matchFamilyStyle(NULL, style));
    }

    if (NULL == face.get()) {
        SkFontIdentity fontId = this->fProxy->matchIndexStyle(0, style);
        face.reset(this->createTypefaceFromFontId(fontId));
    }

    return face.detach();
}
