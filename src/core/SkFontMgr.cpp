/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/SkOnce.h"
#include "src/core/SkFontDescriptor.h"

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

    SkTypeface* onMatchFamilyStyle(const char[], const SkFontStyle&) const override {
        return nullptr;
    }
    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                            const SkFontStyle& style,
                                            const char* bcp47[],
                                            int bcp47Count,
                                            SkUnichar character) const override {
        return nullptr;
    }
    SkTypeface* onMatchFaceStyle(const SkTypeface*, const SkFontStyle&) const override {
        return nullptr;
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>, int) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                           const SkFontArguments&) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromFontData(std::unique_ptr<SkFontData>) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromFile(const char[], int) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char [], SkFontStyle) const override {
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

sk_sp<SkTypeface> SkFontMgr::makeFromData(sk_sp<SkData> data, int ttcIndex) const {
    if (nullptr == data) {
        return nullptr;
    }
    return this->onMakeFromData(std::move(data), ttcIndex);
}

sk_sp<SkTypeface> SkFontMgr::makeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const {
    if (nullptr == stream) {
        return nullptr;
    }
    return this->onMakeFromStreamIndex(std::move(stream), ttcIndex);
}

sk_sp<SkTypeface> SkFontMgr::makeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                            const SkFontArguments& args) const {
    if (nullptr == stream) {
        return nullptr;
    }
    return this->onMakeFromStreamArgs(std::move(stream), args);
}

sk_sp<SkTypeface> SkFontMgr::makeFromFontData(std::unique_ptr<SkFontData> data) const {
    if (nullptr == data) {
        return nullptr;
    }
    return this->onMakeFromFontData(std::move(data));
}

sk_sp<SkTypeface> SkFontMgr::makeFromFile(const char path[], int ttcIndex) const {
    if (nullptr == path) {
        return nullptr;
    }
    return this->onMakeFromFile(path, ttcIndex);
}

sk_sp<SkTypeface> SkFontMgr::legacyMakeTypeface(const char familyName[], SkFontStyle style) const {
    return this->onLegacyMakeTypeface(familyName, style);
}

sk_sp<SkTypeface> SkFontMgr::onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                                  const SkFontArguments& args) const {
    return this->makeFromStream(std::move(stream), args.getCollectionIndex());
}
sk_sp<SkTypeface> SkFontMgr::onMakeFromFontData(std::unique_ptr<SkFontData> data) const {
    return this->makeFromStream(data->detachStream(), data->getIndex());
}

// A global function pointer that's not declared, but can be overriden at startup by test tools.
sk_sp<SkFontMgr> (*gSkFontMgr_DefaultFactory)() = nullptr;

sk_sp<SkFontMgr> SkFontMgr::RefDefault() {
    static SkOnce once;
    static sk_sp<SkFontMgr> singleton;

    once([]{
        sk_sp<SkFontMgr> fm = gSkFontMgr_DefaultFactory ? gSkFontMgr_DefaultFactory()
                                                        : SkFontMgr::Factory();
        singleton = fm ? std::move(fm) : sk_make_sp<SkEmptyFontMgr>();
    });
    return singleton;
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
        Score& operator +=(int rhs) { this->score += rhs; return *this; }
        Score& operator <<=(int rhs) { this->score <<= rhs; return *this; }
        bool operator <(const Score& that) { return this->score < that.score; }
    };

    Score maxScore = { 0, 0 };
    for (int i = 0; i < count; ++i) {
        SkFontStyle current;
        this->getStyle(i, &current, nullptr);
        Score currentScore = { 0, i };

        // CSS stretch / SkFontStyle::Width
        // Takes priority over everything else.
        if (pattern.width() <= SkFontStyle::kNormal_Width) {
            if (current.width() <= pattern.width()) {
                currentScore += 10 - pattern.width() + current.width();
            } else {
                currentScore += 10 - current.width();
            }
        } else {
            if (current.width() > pattern.width()) {
                currentScore += 10 + pattern.width() - current.width();
            } else {
                currentScore += current.width();
            }
        }
        currentScore <<= 8;

        // CSS style (normal, italic, oblique) / SkFontStyle::Slant (upright, italic, oblique)
        // Takes priority over all valid weights.
        static_assert(SkFontStyle::kUpright_Slant == 0 &&
                      SkFontStyle::kItalic_Slant  == 1 &&
                      SkFontStyle::kOblique_Slant == 2,
                      "SkFontStyle::Slant values not as required.");
        SkASSERT(0 <= pattern.slant() && pattern.slant() <= 2 &&
                 0 <= current.slant() && current.slant() <= 2);
        static const int score[3][3] = {
            /*               Upright Italic Oblique  [current]*/
            /*   Upright */ {   3   ,  1   ,   2   },
            /*   Italic  */ {   1   ,  3   ,   2   },
            /*   Oblique */ {   1   ,  2   ,   3   },
            /* [pattern] */
        };
        currentScore += score[pattern.slant()][current.slant()];
        currentScore <<= 8;

        // Synthetics (weight, style) [no stretch synthetic?]

        // CSS weight / SkFontStyle::Weight
        // The 'closer' to the target weight, the higher the score.
        // 1000 is the 'heaviest' recognized weight
        if (pattern.weight() == current.weight()) {
            currentScore += 1000;
        // less than 400 prefer lighter weights
        } else if (pattern.weight() < 400) {
            if (current.weight() <= pattern.weight()) {
                currentScore += 1000 - pattern.weight() + current.weight();
            } else {
                currentScore += 1000 - current.weight();
            }
        // between 400 and 500 prefer heavier up to 500, then lighter weights
        } else if (pattern.weight() <= 500) {
            if (current.weight() >= pattern.weight() && current.weight() <= 500) {
                currentScore += 1000 + pattern.weight() - current.weight();
            } else if (current.weight() <= pattern.weight()) {
                currentScore += 500 + current.weight();
            } else {
                currentScore += 1000 - current.weight();
            }
        // greater than 500 prefer heavier weights
        } else if (pattern.weight() > 500) {
            if (current.weight() > pattern.weight()) {
                currentScore += 1000 + pattern.weight() - current.weight();
            } else {
                currentScore += current.weight();
            }
        }

        if (maxScore < currentScore) {
            maxScore = currentScore;
        }
    }

    return this->createTypeface(maxScore.index);
}
