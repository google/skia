/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontConfigTypeface_DEFINED
#define SkFontConfigTypeface_DEFINED

#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/ports/SkFontConfigInterface.h"
#include "src/core/SkFontDescriptor.h"
#include "src/ports/SkTypeface_proxy.h"

class SkFontDescriptor;

class SkTypeface_FCI : public SkTypeface_proxy {
public:
    static SkTypeface_FCI* Create(sk_sp<SkTypeface> realTypeface,
                                  sk_sp<SkFontConfigInterface> fci,
                                  const SkFontConfigInterface::FontIdentity& fi,
                                  SkString familyName,
                                  const SkFontStyle& style,
                                  const bool isFixedPitch)
    {
        if (!realTypeface || !fci) {
            return nullptr;
        }
        return new SkTypeface_FCI(std::move(realTypeface), std::move(fci), fi,
                                  std::move(familyName), style, isFixedPitch);
    }

    const SkFontConfigInterface::FontIdentity& getIdentity() const {
        return fIdentity;
    }

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        sk_sp<SkTypeface> realTypeface = SkTypeface_proxy::onMakeClone(args);
        if (!realTypeface) {
            return nullptr;
        }
        return sk_sp<SkTypeface>(SkTypeface_FCI::Create(
                std::move(realTypeface),
                fFCI,
                fIdentity,
                fFamilyName,
                this->fontStyle(),
                this->isFixedPitch()));
    }

protected:
    SkTypeface_FCI(sk_sp<SkTypeface> realTypeface,
                   sk_sp<SkFontConfigInterface> fci,
                   const SkFontConfigInterface::FontIdentity& fi,
                   SkString familyName,
                   const SkFontStyle& style,
                   bool isFixedPitch)
            : SkTypeface_proxy(std::move(realTypeface), style, isFixedPitch)
            , fFCI(std::move(fci))
            , fIdentity(fi)
            , fFamilyName(std::move(familyName)) {
    }

    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override;
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;

    void onGetFamilyName(SkString* familyName) const override {
        *familyName = fFamilyName;
    }

    SkFontStyle onGetFontStyle() const override {
        return SkTypeface::onGetFontStyle();
    }

    bool onGetFixedPitch() const override {
        return SkTypeface::onGetFixedPitch();
    }
private:
    sk_sp<SkFontConfigInterface> fFCI;
    SkFontConfigInterface::FontIdentity fIdentity;
    SkString fFamilyName;
};

#endif  // SkFontConfigTypeface_DEFINED
