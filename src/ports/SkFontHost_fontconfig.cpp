/*
 * Copyright 2008 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigInterface.h"
#include "SkFontConfigTypeface.h"
#include "SkFontDescriptor.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "SkTypefaceCache.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SK_DECLARE_STATIC_MUTEX(gFontConfigInterfaceMutex);
static SkFontConfigInterface* gFontConfigInterface;

SkFontConfigInterface* SkFontConfigInterface::RefGlobal() {
    SkAutoMutexAcquire ac(gFontConfigInterfaceMutex);

    return SkSafeRef(gFontConfigInterface);
}

SkFontConfigInterface* SkFontConfigInterface::SetGlobal(SkFontConfigInterface* fc) {
    SkAutoMutexAcquire ac(gFontConfigInterfaceMutex);

    SkRefCnt_SafeAssign(gFontConfigInterface, fc);
    return fc;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// convenience function to create the direct interface if none is installed.
extern SkFontConfigInterface* SkCreateDirectFontConfigInterface();

static SkFontConfigInterface* RefFCI() {
    for (;;) {
        SkFontConfigInterface* fci = SkFontConfigInterface::RefGlobal();
        if (fci) {
            return fci;
        }
        fci = SkFontConfigInterface::GetSingletonDirectInterface(&gFontConfigInterfaceMutex);
        SkFontConfigInterface::SetGlobal(fci);
    }
}

// export this to SkFontMgr_fontconfig.cpp until this file just goes away.
SkFontConfigInterface* SkFontHost_fontconfig_ref_global();
SkFontConfigInterface* SkFontHost_fontconfig_ref_global() {
    return RefFCI();
}

///////////////////////////////////////////////////////////////////////////////

struct NameStyle {
    NameStyle(const char* name, const SkFontStyle& style)
        : fFamilyName(name)  // don't need to make a deep copy
        , fStyle(style) {}

    const char* fFamilyName;
    SkFontStyle fStyle;
};

static bool find_by_NameStyle(SkTypeface* cachedTypeface,
                              const SkFontStyle& cachedStyle,
                              void* ctx)
{
    FontConfigTypeface* cachedFCTypeface = static_cast<FontConfigTypeface*>(cachedTypeface);
    const NameStyle* nameStyle = static_cast<const NameStyle*>(ctx);

    return nameStyle->fStyle == cachedStyle &&
           cachedFCTypeface->isFamilyName(nameStyle->fFamilyName);
}

static bool find_by_FontIdentity(SkTypeface* cachedTypeface, const SkFontStyle&, void* ctx) {
    typedef SkFontConfigInterface::FontIdentity FontIdentity;
    FontConfigTypeface* cachedFCTypeface = static_cast<FontConfigTypeface*>(cachedTypeface);
    FontIdentity* indentity = static_cast<FontIdentity*>(ctx);

    return cachedFCTypeface->getIdentity() == *indentity;
}

SkTypeface* FontConfigTypeface::LegacyCreateTypeface(const char familyName[],
                                                     SkTypeface::Style style)
{
    SkAutoTUnref<SkFontConfigInterface> fci(RefFCI());
    if (nullptr == fci.get()) {
        return nullptr;
    }

    // Check if requested NameStyle is in the NameStyle cache.
    SkFontStyle requestedStyle(style);
    NameStyle nameStyle(familyName, requestedStyle);
    SkTypeface* face = SkTypefaceCache::FindByProcAndRef(find_by_NameStyle, &nameStyle);
    if (face) {
        //SkDebugf("found cached face <%s> <%s> %p [%d]\n",
        //         familyName, ((FontConfigTypeface*)face)->getFamilyName(),
        //         face, face->getRefCnt());
        return face;
    }

    SkFontConfigInterface::FontIdentity indentity;
    SkString outFamilyName;
    SkTypeface::Style outStyle;
    if (!fci->matchFamilyName(familyName, style, &indentity, &outFamilyName, &outStyle)) {
        return nullptr;
    }

    // Check if a typeface with this FontIdentity is already in the FontIdentity cache.
    face = SkTypefaceCache::FindByProcAndRef(find_by_FontIdentity, &indentity);
    if (!face) {
        face = FontConfigTypeface::Create(SkFontStyle(outStyle), indentity, outFamilyName);
        // Add this FontIdentity to the FontIdentity cache.
        SkTypefaceCache::Add(face, requestedStyle);
    }
    // TODO: Ensure requested NameStyle and resolved NameStyle are both in the NameStyle cache.

    //SkDebugf("add face <%s> <%s> %p [%d]\n",
    //         familyName, outFamilyName.c_str(),
    //         face, face->getRefCnt());
    return face;
}

///////////////////////////////////////////////////////////////////////////////

SkStreamAsset* FontConfigTypeface::onOpenStream(int* ttcIndex) const {
    SkStreamAsset* stream = this->getLocalStream();
    if (stream) {
        // TODO: should have been provided by CreateFromStream()
        *ttcIndex = 0;
        return stream->duplicate();
    }

    SkAutoTUnref<SkFontConfigInterface> fci(RefFCI());
    if (nullptr == fci.get()) {
        return nullptr;
    }

    *ttcIndex = this->getIdentity().fTTCIndex;
    return fci->openStream(this->getIdentity());
}

void FontConfigTypeface::onGetFamilyName(SkString* familyName) const {
    *familyName = fFamilyName;
}

void FontConfigTypeface::onGetFontDescriptor(SkFontDescriptor* desc,
                                             bool* isLocalStream) const {
    SkString name;
    this->getFamilyName(&name);
    desc->setFamilyName(name.c_str());
    *isLocalStream = SkToBool(this->getLocalStream());
}
