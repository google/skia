/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"

#if !defined(SK_DISABLE_LEGACY_FONTMGR_FACTORY)
struct SkEmbeddedResource { const uint8_t* data; size_t size; };
struct SkEmbeddedResourceHeader { const SkEmbeddedResource* entries; int count; };
sk_sp<SkFontMgr> SkFontMgr_New_Custom_Embedded(const SkEmbeddedResourceHeader* header);

extern "C" const SkEmbeddedResourceHeader SK_EMBEDDED_FONTS;
sk_sp<SkFontMgr> SkFontMgr::Factory() {
    return SkFontMgr_New_Custom_Embedded(&SK_EMBEDDED_FONTS);
}
#endif
