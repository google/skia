/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontDescriptor.h"
#include "SkFontMgr.h"
#include "SkOncePtr.h"
#include "SkStream.h"
#include "SkTypes.h"

class SkFontStyle;
class SkTypeface;

class SkEmptyFontStyleSet : public SkFontStyleSet {
public:
    int count() override { return 0; }
    void getStyle(int, SkFontStyle*, SkString*) override {
        SkDEBUGFAIL("SkFontStyleSet::getStyle called on empty set");
    }
    SkTypeface* createTypeface(int index) override {
        SkDEBUGFAIL("SkFontStyleSet::createTypeface called on empty set");
        return nullptr;
    }
    SkTypeface* matchStyle(const SkFontStyle&) override {
        return nullptr;
    }
};

SkFontStyleSet* SkFontStyleSet::CreateEmpty() { return new SkEmptyFontStyleSet; }

///////////////////////////////////////////////////////////////////////////////

class SkEmptyFontMgr : public SkFontMgr {
protected:
    int onCountFamilies() const override {
        return 0;
    }
    void onGetFamilyName(int index, SkString* familyName) const override {
        SkDEBUGFAIL("onGetFamilyName called with bad index");
    }
    SkFontStyleSet* onCreateStyleSet(int index) const override {
        SkDEBUGFAIL("onCreateStyleSet called with bad index");
        return nullptr;
    }
    SkFontStyleSet* onMatchFamily(const char[]) const override {
        return SkFontStyleSet::CreateEmpty();
    }

    virtual SkTypeface* onMatchFamilyStyle(const char[],
                                           const SkFontStyle&) const override {
        return nullptr;
    }
    virtual SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                                    const SkFontStyle& style,
                                                    const char* bcp47[],
                                                    int bcp47Count,
                                                    SkUnichar character) const override {
        return nullptr;
    }
    virtual SkTypeface* onMatchFaceStyle(const SkTypeface*,
                                         const SkFontStyle&) const override {
        return nullptr;
    }
    SkTypeface* onCreateFromData(SkData*, int) const override {
        return nullptr;
    }
    SkTypeface* onCreateFromStream(SkStreamAsset* stream, int) const override {
        delete stream;
        return nullptr;
    }
    SkTypeface* onCreateFromFile(const char[], int) const override {
        return nullptr;
    }
    SkTypeface* onLegacyCreateTypeface(const char [], unsigned) const override {
        return nullptr;
    }
};

static SkFontStyleSet* emptyOnNull(SkFontStyleSet* fsset) {
    if (nullptr == fsset) {
        fsset = SkFontStyleSet::CreateEmpty();
    }
    return fsset;
}

int SkFontMgr::countFamilies() const {
    return this->onCountFamilies();
}

void SkFontMgr::getFamilyName(int index, SkString* familyName) const {
    this->onGetFamilyName(index, familyName);
}

SkFontStyleSet* SkFontMgr::createStyleSet(int index) const {
    return emptyOnNull(this->onCreateStyleSet(index));
}

SkFontStyleSet* SkFontMgr::matchFamily(const char familyName[]) const {
    return emptyOnNull(this->onMatchFamily(familyName));
}

SkTypeface* SkFontMgr::matchFamilyStyle(const char familyName[],
                                        const SkFontStyle& fs) const {
    return this->onMatchFamilyStyle(familyName, fs);
}

SkTypeface* SkFontMgr::matchFamilyStyleCharacter(const char familyName[], const SkFontStyle& style,
                                                 const char* bcp47[], int bcp47Count,
                                                 SkUnichar character) const {
    return this->onMatchFamilyStyleCharacter(familyName, style, bcp47, bcp47Count, character);
}

SkTypeface* SkFontMgr::matchFaceStyle(const SkTypeface* face,
                                      const SkFontStyle& fs) const {
    return this->onMatchFaceStyle(face, fs);
}

SkTypeface* SkFontMgr::createFromData(SkData* data, int ttcIndex) const {
    if (nullptr == data) {
        return nullptr;
    }
    return this->onCreateFromData(data, ttcIndex);
}

SkTypeface* SkFontMgr::createFromStream(SkStreamAsset* stream, int ttcIndex) const {
    if (nullptr == stream) {
        return nullptr;
    }
    return this->onCreateFromStream(stream, ttcIndex);
}

SkTypeface* SkFontMgr::createFromFontData(SkFontData* data) const {
    if (nullptr == data) {
        return nullptr;
    }
    return this->onCreateFromFontData(data);
}

// This implementation is temporary until it can be made pure virtual.
SkTypeface* SkFontMgr::onCreateFromFontData(SkFontData* data) const {
    SkTypeface* ret = this->createFromStream(data->detachStream(), data->getIndex());
    delete data;
    return ret;
}

SkTypeface* SkFontMgr::createFromFile(const char path[], int ttcIndex) const {
    if (nullptr == path) {
        return nullptr;
    }
    return this->onCreateFromFile(path, ttcIndex);
}

SkTypeface* SkFontMgr::legacyCreateTypeface(const char familyName[],
                                            unsigned styleBits) const {
    return this->onLegacyCreateTypeface(familyName, styleBits);
}

SK_DECLARE_STATIC_ONCE_PTR(SkFontMgr, singleton);
SkFontMgr* SkFontMgr::RefDefault() {
    return SkRef(singleton.get([]{
        SkFontMgr* fm = SkFontMgr::Factory();
        return fm ? fm : new SkEmptyFontMgr;
    }));
}

/**
* Width has the greatest priority.
* If the value of pattern.width is 5 (normal) or less,
*    narrower width values are checked first, then wider values.
* If the value of pattern.width is greater than 5 (normal),
*    wider values are checked first, followed by narrower values.
*
* Italic/Oblique has the next highest priority.
* If italic requested and there is some italic font, use it.
* If oblique requested and there is some oblique font, use it.
* If italic requested and there is some oblique font, use it.
* If oblique requested and there is some italic font, use it.
*
* Exact match.
* If pattern.weight < 400, weights below pattern.weight are checked
*   in descending order followed by weights above pattern.weight
*   in ascending order until a match is found.
* If pattern.weight > 500, weights above pattern.weight are checked
*   in ascending order followed by weights below pattern.weight
*   in descending order until a match is found.
* If pattern.weight is 400, 500 is checked first
*   and then the rule for pattern.weight < 400 is used.
* If pattern.weight is 500, 400 is checked first
*   and then the rule for pattern.weight < 400 is used.
*/
SkTypeface* SkFontStyleSet::matchStyleCSS3(const SkFontStyle& pattern) {
    int count = this->count();
    if (0 == count) {
        return nullptr;
    }

    struct Score {
        int score;
        int index;
    };

    Score maxScore = { 0, 0 };
    for (int i = 0; i < count; ++i) {
        SkFontStyle current;
        this->getStyle(i, &current, nullptr);
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
        if (pattern.isItalic() == current.isItalic()) {
            currentScore.score += 1001;
        }

        // Synthetics (weight/style) [no stretch synthetic?]

        // The 'closer' to the target weight, the higher the score.
        // 1000 is the 'heaviest' recognized weight
        if (pattern.weight() == current.weight()) {
            currentScore.score += 1000;
        } else if (pattern.weight() <= 500) {
            if (400 <= pattern.weight() && pattern.weight() < 450) {
                if (450 <= current.weight() && current.weight() <= 500) {
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
