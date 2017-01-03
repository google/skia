/*
 * Copyright 2008 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigInterface.h"
#include "SkFontMgr.h"
#include "SkFontMgr_FontConfigInterface.h"

#ifdef SK_LEGACY_FONTMGR_FACTORY
SkFontMgr* SkFontMgr::Factory() {
#else
sk_sp<SkFontMgr> SkFontMgr::Factory() {
#endif
    sk_sp<SkFontConfigInterface> fci(SkFontConfigInterface::RefGlobal());
    if (!fci) {
        return nullptr;
    }
    return SkFontMgr_New_FCI(std::move(fci));
}
