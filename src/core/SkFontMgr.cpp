/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontDescriptor.h"
#include "SkFontMgr.h"
#include "SkLazyPtr.h"
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
        return NULL;
    }
    SkTypeface* matchStyle(const SkFontStyle&) override {
        return NULL;
    }
};

SkFontStyleSet* SkFontStyleSet::CreateEmpty() {
    return SkNEW(SkEmptyFontStyleSet);
}

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
        return NULL;
    }
    SkFontStyleSet* onMatchFamily(const char[]) const override {
        return SkFontStyleSet::CreateEmpty();
    }

    virtual SkTypeface* onMatchFamilyStyle(const char[],
                                           const SkFontStyle&) const override {
        return NULL;
    }
    virtual SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                                    const SkFontStyle& style,
                                                    const char* bcp47[],
                                                    int bcp47Count,
                                                    SkUnichar character) const override {
        return NULL;
    }
    virtual SkTypeface* onMatchFaceStyle(const SkTypeface*,
                                         const SkFontStyle&) const override {
        return NULL;
    }
    SkTypeface* onCreateFromData(SkData*, int) const override {
        return NULL;
    }
    SkTypeface* onCreateFromStream(SkStreamAsset* stream, int) const override {
        SkDELETE(stream);
        return NULL;
    }
    SkTypeface* onCreateFromFile(const char[], int) const override {
        return NULL;
    }
    SkTypeface* onLegacyCreateTypeface(const char [], unsigned) const override {
        return NULL;
    }
};

static SkFontStyleSet* emptyOnNull(SkFontStyleSet* fsset) {
    if (NULL == fsset) {
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
    if (NULL == data) {
        return NULL;
    }
    return this->onCreateFromData(data, ttcIndex);
}

SkTypeface* SkFontMgr::createFromStream(SkStreamAsset* stream, int ttcIndex) const {
    if (NULL == stream) {
        return NULL;
    }
    return this->onCreateFromStream(stream, ttcIndex);
}

SkTypeface* SkFontMgr::createFromFontData(SkFontData* data) const {
    if (NULL == data) {
        return NULL;
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
    if (NULL == path) {
        return NULL;
    }
    return this->onCreateFromFile(path, ttcIndex);
}

SkTypeface* SkFontMgr::legacyCreateTypeface(const char familyName[],
                                            unsigned styleBits) const {
    return this->onLegacyCreateTypeface(familyName, styleBits);
}

// As a template argument this must have external linkage.
SkFontMgr* sk_fontmgr_create_default() {
    SkFontMgr* fm = SkFontMgr::Factory();
    return fm ? fm : SkNEW(SkEmptyFontMgr);
}

SK_DECLARE_STATIC_LAZY_PTR(SkFontMgr, singleton, sk_fontmgr_create_default);

SkFontMgr* SkFontMgr::RefDefault() {
    return SkRef(singleton.get());
}
