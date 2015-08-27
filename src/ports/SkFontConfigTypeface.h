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

class FontConfigTypeface : public SkTypeface_FreeType {
    SkFontConfigInterface::FontIdentity fIdentity;
    SkString fFamilyName;
    SkAutoTDelete<SkStreamAsset> fLocalStream;

public:
    static FontConfigTypeface* Create(const SkFontStyle& style,
                                      const SkFontConfigInterface::FontIdentity& fi,
                                      const SkString& familyName) {
        return new FontConfigTypeface(style, fi, familyName);
    }

    static FontConfigTypeface* Create(const SkFontStyle& style, bool fixedWidth,
                                      SkStreamAsset* localStream) {
        return new FontConfigTypeface(style, fixedWidth, localStream);
    }

    const SkFontConfigInterface::FontIdentity& getIdentity() const {
        return fIdentity;
    }

    SkStreamAsset* getLocalStream() const { return fLocalStream.get(); }

    bool isFamilyName(const char* name) const {
        return fFamilyName.equals(name);
    }

    static SkTypeface* LegacyCreateTypeface(const char familyName[], SkTypeface::Style);

protected:
    FontConfigTypeface(const SkFontStyle& style,
                       const SkFontConfigInterface::FontIdentity& fi,
                       const SkString& familyName)
            : INHERITED(style, SkTypefaceCache::NewFontID(), false)
            , fIdentity(fi)
            , fFamilyName(familyName)
            , fLocalStream(nullptr) {}

    FontConfigTypeface(const SkFontStyle& style, bool fixedWidth, SkStreamAsset* localStream)
            : INHERITED(style, SkTypefaceCache::NewFontID(), fixedWidth)
            , fLocalStream(localStream) {
        // we default to empty fFamilyName and fIdentity
    }

    void onGetFamilyName(SkString* familyName) const override;
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override;
    SkStreamAsset* onOpenStream(int* ttcIndex) const override;

private:
    typedef SkTypeface_FreeType INHERITED;
};
