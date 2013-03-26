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

#include "SkFontMgr.h"

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
    fUnion.fR.fSlant = kUpright_Slant;
}

int SkFontMgr::countFamilies() {
    return this->onCountFamilies();
}

void SkFontMgr::getFamilyName(int index, SkString* familyName) {
    this->onGetFamilyName(index, familyName);
}

SkFontStyleSet* SkFontMgr::createStyleSet(int index) {
    return this->onCreateStyleSet(index);
}

SkTypeface* SkFontMgr::matchFamilyStyle(const char familyName[],
                                        const SkFontStyle& fs) {
    return this->onMatchFamilyStyle(familyName, fs);
}

SkTypeface* SkFontMgr::matchFaceStyle(const SkTypeface* face, 
                                      const SkFontStyle& fs) {
    return this->matchFaceStyle(face, fs);
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
#if 1
    return NULL;
#else
    static SkFontMgr* gFM;
    if (NULL == gFM) {
        gFM = SkFontMgr::Factory();
    }
    return SkRef(gFM);
#endif
}

