/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/fonts/GlobalFontMgr.h"
#include "include/core/SkTypes.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkNativeFontMgrFactory.h"
#include "include/private/SkOnce.h"
#include "src/core/SkFontDescriptor.h"

namespace ToolUtils {

namespace {

class SkEmptyFontMgr : public SkFontMgr {
protected:
    int onCountFamilies() const override {
        return 0;
    }
    void onGetFamilyName(int index, SkString* familyName) const override {
        SkDEBUGFAIL("onGetFamilyName called with bad index");
    }
    SkFontStyleSet* onCreateStyleSet(int index) const override {
        SkDEBUGFAIL("onCreateStyleSet called with bad index");
        return nullptr;
    }
    SkFontStyleSet* onMatchFamily(const char[]) const override {
        return SkFontStyleSet::CreateEmpty();
    }

    SkTypeface* onMatchFamilyStyle(const char[], const SkFontStyle&) const override {
        return nullptr;
    }
    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                            const SkFontStyle& style,
                                            const char* bcp47[],
                                            int bcp47Count,
                                            SkUnichar character) const override {
        return nullptr;
    }
    SkTypeface* onMatchFaceStyle(const SkTypeface*, const SkFontStyle&) const override {
        return nullptr;
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>, int) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                           const SkFontArguments&) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromFontData(std::unique_ptr<SkFontData>) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onMakeFromFile(const char[], int) const override {
        return nullptr;
    }
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char [], SkFontStyle) const override {
        return nullptr;
    }
};

static sk_sp<SkFontMgr> gFontMgr = nullptr;
static SkOnce gFontMgrOnce;

}  // namespace


void SetGlobalNativeFontMgr() {
    SetGlobalFontMgr(SkNativeFontMgrFactory());
}

void SetGlobalFontMgr(sk_sp<SkFontMgr> fm) {
    if (fm) {
        gFontMgr = std::move(fm);
    } else {
        gFontMgr = sk_make_sp<SkEmptyFontMgr>();
    }
}

sk_sp<SkFontMgr> GlobalFontMgr() {
    gFontMgrOnce([]{
        if (!gFontMgr) {
            gFontMgr = sk_make_sp<SkEmptyFontMgr>();
        }
    });
    return gFontMgr;
}

sk_sp<SkTypeface> DefaultTypeface() {
    static SkOnce once;
    static sk_sp<SkTypeface> defaultTypeface;

    once([] {
        sk_sp<SkFontMgr> fm(GlobalFontMgr());
        auto t = fm->legacyMakeTypeface(nullptr, SkFontStyle::Normal());
        defaultTypeface = t ? t : SkTypeface::MakeEmpty();
    });
    return defaultTypeface;
}

sk_sp<SkTypeface> TypefaceFromName(const char name[], SkFontStyle fontStyle) {
    return GlobalFontMgr()->legacyMakeTypeface(name, fontStyle);
}

}  // namespace ToolUtils
