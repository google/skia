/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigInterface.h"
#include "SkFontDescriptor.h"
#include "SkFontHost_FreeType_common.h"
#include "SkRefCnt.h"
#include "SkStream.h"

class SkFontDescriptor;

class SkTypeface_FCI : public SkTypeface_FreeType {
    sk_sp<SkFontConfigInterface> fFCI;
    SkFontConfigInterface::FontIdentity fIdentity;
    SkString fFamilyName;
    std::unique_ptr<SkFontData> fFontData;

public:
    static SkTypeface_FCI* Create(sk_sp<SkFontConfigInterface> fci,
                                  const SkFontConfigInterface::FontIdentity& fi,
                                  SkString familyName,
                                  const SkFontStyle& style)
    {
        return new SkTypeface_FCI(std::move(fci), fi, std::move(familyName), style);
    }

    static SkTypeface_FCI* Create(std::unique_ptr<SkFontData> data,
                                  SkString familyName, SkFontStyle style, bool isFixedPitch)
    {
        return new SkTypeface_FCI(std::move(data), std::move(familyName), style, isFixedPitch);
    }

    const SkFontConfigInterface::FontIdentity& getIdentity() const {
        return fIdentity;
    }

protected:
    SkTypeface_FCI(sk_sp<SkFontConfigInterface> fci,
                   const SkFontConfigInterface::FontIdentity& fi,
                   SkString familyName,
                   const SkFontStyle& style)
            : INHERITED(style, false)
            , fFCI(std::move(fci))
            , fIdentity(fi)
            , fFamilyName(std::move(familyName))
            , fFontData(nullptr) {}

    SkTypeface_FCI(std::unique_ptr<SkFontData> data,
                   SkString familyName, SkFontStyle style, bool isFixedPitch)
            : INHERITED(style, isFixedPitch)
            , fFamilyName(std::move(familyName))
            , fFontData(std::move(data))
    {
        SkASSERT(fFontData);
        fIdentity.fTTCIndex = fFontData->getIndex();
    }

    void onGetFamilyName(SkString* familyName) const override { *familyName = fFamilyName; }
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override;
    SkStreamAsset* onOpenStream(int* ttcIndex) const override;
    std::unique_ptr<SkFontData> onMakeFontData() const override;

private:
    typedef SkTypeface_FreeType INHERITED;
};
