/*
 * Copyright 2008 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigInterface.h"
#include "SkFontConfigTypeface.h"
#include "SkFontDescriptor.h"
#include "SkFontHost.h"
#include "SkFontHost_FreeType_common.h"
#include "SkFontStream.h"
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

struct FindRec {
    FindRec(const char* name, const SkFontStyle& style)
        : fFamilyName(name)  // don't need to make a deep copy
        , fStyle(style) {}

    const char* fFamilyName;
    SkFontStyle fStyle;
};

static bool find_proc(SkTypeface* cachedTypeface, const SkFontStyle& cachedStyle, void* ctx) {
    FontConfigTypeface* cachedFCTypeface = (FontConfigTypeface*)cachedTypeface;
    const FindRec* rec = static_cast<const FindRec*>(ctx);

    return rec->fStyle == cachedStyle &&
           cachedFCTypeface->isFamilyName(rec->fFamilyName);
}

SkTypeface* FontConfigTypeface::LegacyCreateTypeface(
                const SkTypeface* familyFace,
                const char familyName[],
                SkTypeface::Style style) {
    SkAutoTUnref<SkFontConfigInterface> fci(RefFCI());
    if (NULL == fci.get()) {
        return NULL;
    }

    if (familyFace) {
        FontConfigTypeface* fct = (FontConfigTypeface*)familyFace;
        familyName = fct->getFamilyName();
    }

    SkFontStyle requestedStyle(style);
    FindRec rec(familyName, requestedStyle);
    SkTypeface* face = SkTypefaceCache::FindByProcAndRef(find_proc, &rec);
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
        return NULL;
    }

    // check if we, in fact, already have this. perhaps fontconfig aliased the
    // requested name to some other name we actually have...
    rec.fFamilyName = outFamilyName.c_str();
    rec.fStyle = SkFontStyle(outStyle);
    face = SkTypefaceCache::FindByProcAndRef(find_proc, &rec);
    if (face) {
        return face;
    }

    face = FontConfigTypeface::Create(SkFontStyle(outStyle), indentity, outFamilyName);
    SkTypefaceCache::Add(face, requestedStyle);
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
    if (NULL == fci.get()) {
        return NULL;
    }

    *ttcIndex = this->getIdentity().fTTCIndex;
    return fci->openStream(this->getIdentity());
}

void FontConfigTypeface::onGetFamilyName(SkString* familyName) const {
    *familyName = this->getFamilyName();
}

void FontConfigTypeface::onGetFontDescriptor(SkFontDescriptor* desc,
                                             bool* isLocalStream) const {
    desc->setFamilyName(this->getFamilyName());
    desc->setFontIndex(this->getIdentity().fTTCIndex);
    *isLocalStream = SkToBool(this->getLocalStream());
}
