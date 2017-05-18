/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontMgr_custom.h"
#include "SkFontMgr_empty.h"

class EmptyFontLoader : public SkFontMgr_Custom::SystemFontLoader {
public:
    EmptyFontLoader() { }

    void loadSystemFonts(const SkTypeface_FreeType::Scanner& scanner,
                         SkFontMgr_Custom::Families* families) const override
    {
        SkFontStyleSet_Custom* family = new SkFontStyleSet_Custom(SkString());
        families->push_back().reset(family);
        family->appendTypeface(sk_make_sp<SkTypeface_Empty>());
    }

};

SK_API sk_sp<SkFontMgr> SkFontMgr_New_Custom_Empty() {
    return sk_make_sp<SkFontMgr_Custom>(EmptyFontLoader());
}
