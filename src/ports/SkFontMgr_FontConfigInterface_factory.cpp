/*
 * Copyright 2008 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/ports/SkFontConfigInterface.h"
#include "include/ports/SkFontMgr_FontConfigInterface.h"

#if !defined(SK_DISABLE_LEGACY_FONTMGR_FACTORY)
sk_sp<SkFontMgr> SkFontMgr::Factory() {
    sk_sp<SkFontConfigInterface> fci(SkFontConfigInterface::RefGlobal());
    if (!fci) {
        return nullptr;
    }
    return SkFontMgr_New_FCI(std::move(fci));
}
#endif
