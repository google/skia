/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontLCDConfig.h"
#include "SkLazyPtr.h"

static SkFontLCDConfig::LCDOrientation gLCDOrientation = SkFontLCDConfig::kHorizontal_LCDOrientation;
static SkFontLCDConfig::LCDOrder gLCDOrder = SkFontLCDConfig::kRGB_LCDOrder;

SkFontLCDConfig::LCDOrientation SkFontLCDConfig::GetSubpixelOrientation() {
    return gLCDOrientation;
}

void SkFontLCDConfig::SetSubpixelOrientation(LCDOrientation orientation) {
    gLCDOrientation = orientation;
}

SkFontLCDConfig::LCDOrder SkFontLCDConfig::GetSubpixelOrder() {
    return gLCDOrder;
}

void SkFontLCDConfig::SetSubpixelOrder(LCDOrder order) {
    gLCDOrder = order;
}

///////////////////////////////////////////////////////////////////////////////
// Legacy wrappers : remove from SkFontHost when webkit switches to new API

#include "SkFontHost.h"

SkFontHost::LCDOrientation SkFontHost::GetSubpixelOrientation() {
    return (SkFontHost::LCDOrientation)SkFontLCDConfig::GetSubpixelOrientation();
}

void SkFontHost::SetSubpixelOrientation(LCDOrientation orientation) {
    SkFontLCDConfig::SetSubpixelOrientation((SkFontLCDConfig::LCDOrientation)orientation);
}

SkFontHost::LCDOrder SkFontHost::GetSubpixelOrder() {
    return (SkFontHost::LCDOrder)SkFontLCDConfig::GetSubpixelOrder();
}

void SkFontHost::SetSubpixelOrder(LCDOrder order) {
    SkFontLCDConfig::SetSubpixelOrder((SkFontLCDConfig::LCDOrder)order);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "SkFontStyle.h"

SkFontStyle::SkFontStyle() {
    fUnion.fU32 = 0;
    fUnion.fR.fWeight = kNormal_Weight;
    fUnion.fR.fWidth = kNormal_Width;
    fUnion.fR.fSlant = kUpright_Slant;
}

SkFontStyle::SkFontStyle(int weight, int width, Slant slant) {
    fUnion.fU32 = 0;
    fUnion.fR.fWeight = SkPin32(weight, kThin_Weight, kBlack_Weight);
    fUnion.fR.fWidth = SkPin32(width, kUltraCondensed_Width, kUltaExpanded_Width);
    fUnion.fR.fSlant = SkPin32(slant, kUpright_Slant, kItalic_Slant);
}

#include "SkFontMgr.h"

class SkEmptyFontStyleSet : public SkFontStyleSet {
public:
    virtual int count() SK_OVERRIDE { return 0; }
    virtual void getStyle(int, SkFontStyle*, SkString*) SK_OVERRIDE {
        SkDEBUGFAIL("SkFontStyleSet::getStyle called on empty set");
    }
    virtual SkTypeface* createTypeface(int index) SK_OVERRIDE {
        SkDEBUGFAIL("SkFontStyleSet::createTypeface called on empty set");
        return NULL;
    }
    virtual SkTypeface* matchStyle(const SkFontStyle&) SK_OVERRIDE {
        return NULL;
    }
};

SkFontStyleSet* SkFontStyleSet::CreateEmpty() {
    return SkNEW(SkEmptyFontStyleSet);
}

///////////////////////////////////////////////////////////////////////////////

class SkEmptyFontMgr : public SkFontMgr {
protected:
    virtual int onCountFamilies() const SK_OVERRIDE {
        return 0;
    }
    virtual void onGetFamilyName(int index, SkString* familyName) const SK_OVERRIDE {
        SkDEBUGFAIL("onGetFamilyName called with bad index");
    }
    virtual SkFontStyleSet* onCreateStyleSet(int index) const SK_OVERRIDE {
        SkDEBUGFAIL("onCreateStyleSet called with bad index");
        return NULL;
    }
    virtual SkFontStyleSet* onMatchFamily(const char[]) const SK_OVERRIDE {
        return SkFontStyleSet::CreateEmpty();
    }

    virtual SkTypeface* onMatchFamilyStyle(const char[],
                                           const SkFontStyle&) const SK_OVERRIDE {
        return NULL;
    }
    virtual SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                                    const SkFontStyle& style,
                                                    const char bpc47[],
                                                    uint32_t character) const SK_OVERRIDE {
        return NULL;
    }
    virtual SkTypeface* onMatchFaceStyle(const SkTypeface*,
                                         const SkFontStyle&) const SK_OVERRIDE {
        return NULL;
    }
    virtual SkTypeface* onCreateFromData(SkData*, int) const SK_OVERRIDE {
        return NULL;
    }
    virtual SkTypeface* onCreateFromStream(SkStream*, int) const SK_OVERRIDE {
        return NULL;
    }
    virtual SkTypeface* onCreateFromFile(const char[], int) const SK_OVERRIDE {
        return NULL;
    }
    virtual SkTypeface* onLegacyCreateTypeface(const char [], unsigned) const SK_OVERRIDE {
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
                                                 const char bpc47[], uint32_t character) const {
    return this->onMatchFamilyStyleCharacter(familyName, style, bpc47, character);
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

SkTypeface* SkFontMgr::createFromStream(SkStream* stream, int ttcIndex) const {
    if (NULL == stream) {
        return NULL;
    }
    return this->onCreateFromStream(stream, ttcIndex);
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

SkFontMgr* SkFontMgr::CreateDefault() {
    SkFontMgr* fm = SkFontMgr::Factory();
    return fm ? fm : SkNEW(SkEmptyFontMgr);
}

SkFontMgr* SkFontMgr::RefDefault() {
    SK_DECLARE_STATIC_LAZY_PTR(SkFontMgr, singleton, CreateDefault);
    return SkRef(singleton.get());
}

//////////////////////////////////////////////////////////////////////////

#ifndef SK_FONTHOST_DOES_NOT_USE_FONTMGR

#if 0
static SkFontStyle TypefaceStyleBitsToFontStyle(SkTypeface::Style styleBits) {
    SkFontStyle::Weight weight = (styleBits & SkTypeface::kBold) ?
                                     SkFontStyle::kBold_Weight :
                                     SkFontStyle::kNormal_Weight;
    SkFontStyle::Width width = SkFontStyle::kNormal_Width;
    SkFontStyle::Slant slant = (styleBits & SkTypeface::kItalic) ?
                                     SkFontStyle::kUpright_Slant :
                                     SkFontStyle::kItalic_Slant;
    return SkFontStyle(weight, width, slant);
}
#endif

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                                       const char familyName[],
                                       SkTypeface::Style style) {
    SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());
    if (familyFace) {
        bool bold = style & SkTypeface::kBold;
        bool italic = style & SkTypeface::kItalic;
        SkFontStyle newStyle = SkFontStyle(bold ? SkFontStyle::kBold_Weight
                                                : SkFontStyle::kNormal_Weight,
                                           SkFontStyle::kNormal_Width,
                                           italic ? SkFontStyle::kItalic_Slant
                                                  : SkFontStyle::kUpright_Slant);
        return fm->matchFaceStyle(familyFace, newStyle);
    } else {
        return fm->legacyCreateTypeface(familyName, style);
    }
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(const char path[]) {
    SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());
    return fm->createFromFile(path);
}

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream) {
    SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());
    return fm->createFromStream(stream);
}

#endif
