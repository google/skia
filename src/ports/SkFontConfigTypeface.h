/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigInterface.h"
#include "SkFontHost_FreeType_common.h"
#include "SkStream.h"
#include "SkTypefaceCache.h"

class SkFontDescriptor;

class SkTypeface_FCI : public SkTypeface_FreeType {
    SkAutoTUnref<SkFontConfigInterface> fFCI;
    SkFontConfigInterface::FontIdentity fIdentity;
    SkString fFamilyName;
    SkAutoTDelete<SkStreamAsset> fLocalStream;

public:
    static SkTypeface_FCI* Create(SkFontConfigInterface* fci,
                                  const SkFontConfigInterface::FontIdentity& fi,
                                  const SkString& familyName,
                                  const SkFontStyle& style)
    {
        return new SkTypeface_FCI(fci, fi, familyName, style);
    }

    static SkTypeface_FCI* Create(const SkFontStyle& style, bool fixedWidth,
                                  SkStreamAsset* localStream, int index)
    {
        return new SkTypeface_FCI(style, fixedWidth, localStream, index);
    }

    const SkFontConfigInterface::FontIdentity& getIdentity() const {
        return fIdentity;
    }

    SkStreamAsset* getLocalStream() const {
        return fLocalStream.get();
    }

    bool isFamilyName(const char* name) const {
        return fFamilyName.equals(name);
    }

protected:
    SkTypeface_FCI(SkFontConfigInterface* fci,
                   const SkFontConfigInterface::FontIdentity& fi,
                   const SkString& familyName,
                   const SkFontStyle& style)
            : INHERITED(style, SkTypefaceCache::NewFontID(), false)
            , fFCI(SkRef(fci))
            , fIdentity(fi)
            , fFamilyName(familyName)
            , fLocalStream(nullptr) {}

    SkTypeface_FCI(const SkFontStyle& style, bool fixedWidth, SkStreamAsset* localStream, int index)
            : INHERITED(style, SkTypefaceCache::NewFontID(), fixedWidth)
            , fLocalStream(localStream)
    {
        fIdentity.fTTCIndex = index;
    }

    void onGetFamilyName(SkString* familyName) const override { *familyName = fFamilyName; }
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override;
    SkStreamAsset* onOpenStream(int* ttcIndex) const override;

private:
    typedef SkTypeface_FreeType INHERITED;
};
