/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontConfigTypeface_DEFINED
#define SkFontConfigTypeface_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/ports/SkFontConfigInterface.h"
#include "src/core/SkFontDescriptor.h"
#include "src/ports/SkTypeface_FreeType.h"

class SkFontDescriptor;

class SkTypeface_FCI : public SkTypeface_FreeType {
    sk_sp<SkFontConfigInterface> fFCI;
    SkFontConfigInterface::FontIdentity fIdentity;
    SkString fFamilyName;

public:
    static SkTypeface_FCI* Create(sk_sp<SkFontConfigInterface> fci,
                                  const SkFontConfigInterface::FontIdentity& fi,
                                  SkString familyName,
                                  const SkFontStyle& style)
    {
        return new SkTypeface_FCI(std::move(fci), fi, std::move(familyName), style);
    }

    const SkFontConfigInterface::FontIdentity& getIdentity() const {
        return fIdentity;
    }

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        std::unique_ptr<SkFontData> data = this->cloneFontData(args);
        if (!data) {
            return nullptr;
        }
        return sk_sp<SkTypeface>(
            new SkTypeface_FreeTypeStream(std::move(data), fFamilyName,
                                          this->fontStyle(), this->isFixedPitch()));
    }

protected:
    SkTypeface_FCI(sk_sp<SkFontConfigInterface> fci,
                   const SkFontConfigInterface::FontIdentity& fi,
                   SkString familyName,
                   const SkFontStyle& style)
            : INHERITED(style, false)
            , fFCI(std::move(fci))
            , fIdentity(fi)
            , fFamilyName(std::move(familyName)) {}

    void onGetFamilyName(SkString* familyName) const override { *familyName = fFamilyName; }
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override;
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    std::unique_ptr<SkFontData> onMakeFontData() const override;

private:
    using INHERITED = SkTypeface_FreeType;
};

#endif  // SkFontConfigTypeface_DEFINED
