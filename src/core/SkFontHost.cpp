/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontLCDConfig.h"

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
        SkASSERT(!"SkFontStyleSet::getStyle called on empty set");
    }
    virtual SkTypeface* createTypeface(int index) SK_OVERRIDE {
        SkASSERT(!"SkFontStyleSet::createTypeface called on empty set");
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
    virtual int onCountFamilies() SK_OVERRIDE {
        return 0;
    }
    virtual void onGetFamilyName(int index, SkString* familyName) SK_OVERRIDE {
        SkASSERT(!"onGetFamilyName called with bad index");
    }
    virtual SkFontStyleSet* onCreateStyleSet(int index) SK_OVERRIDE {
        SkASSERT(!"onCreateStyleSet called with bad index");
        return NULL;
    }
    virtual SkFontStyleSet* onMatchFamily(const char[]) SK_OVERRIDE {
        return SkFontStyleSet::CreateEmpty();
    }

    virtual SkTypeface* onMatchFamilyStyle(const char[],
                                           const SkFontStyle&) SK_OVERRIDE {
        return NULL;
    }
    virtual SkTypeface* onMatchFaceStyle(const SkTypeface*,
                                         const SkFontStyle&) SK_OVERRIDE {
        return NULL;
    }
    virtual SkTypeface* onCreateFromData(SkData*, int) SK_OVERRIDE {
        return NULL;
    }
    virtual SkTypeface* onCreateFromStream(SkStream*, int) SK_OVERRIDE {
        return NULL;
    }
    virtual SkTypeface* onCreateFromFile(const char[], int) SK_OVERRIDE {
        return NULL;
    }
};

int SkFontMgr::countFamilies() {
    return this->onCountFamilies();
}

void SkFontMgr::getFamilyName(int index, SkString* familyName) {
    this->onGetFamilyName(index, familyName);
}

SkFontStyleSet* SkFontMgr::createStyleSet(int index) {
    return this->onCreateStyleSet(index);
}

SkFontStyleSet* SkFontMgr::matchFamily(const char familyName[]) {
    return this->onMatchFamily(familyName);
}

SkTypeface* SkFontMgr::matchFamilyStyle(const char familyName[],
                                        const SkFontStyle& fs) {
    return this->onMatchFamilyStyle(familyName, fs);
}

SkTypeface* SkFontMgr::matchFaceStyle(const SkTypeface* face,
                                      const SkFontStyle& fs) {
    return this->onMatchFaceStyle(face, fs);
}

SkTypeface* SkFontMgr::createFromData(SkData* data, int ttcIndex) {
    return this->onCreateFromData(data, ttcIndex);
}

SkTypeface* SkFontMgr::createFromStream(SkStream* stream, int ttcIndex) {
    return this->onCreateFromStream(stream, ttcIndex);
}

SkTypeface* SkFontMgr::createFromFile(const char path[], int ttcIndex) {
    return this->onCreateFromFile(path, ttcIndex);
}

SkFontMgr* SkFontMgr::RefDefault() {
    static SkFontMgr* gFM;
    if (NULL == gFM) {
        gFM = SkFontMgr::Factory();
        // we never want to return NULL
        if (NULL == gFM) {
            gFM = SkNEW(SkEmptyFontMgr);
        }
    }
    return SkRef(gFM);
}
