
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkFontHost.h"
#include "SkScalerContext.h"

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                                     const char famillyName[],
                                     SkTypeface::Style style) {
    SkDEBUGFAIL("SkFontHost::FindTypeface unimplemented");
    return NULL;
}

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream*) {
    SkDEBUGFAIL("SkFontHost::CreateTypeface unimplemented");
    return NULL;
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(char const*) {
    SkDEBUGFAIL("SkFontHost::CreateTypefaceFromFile unimplemented");
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

void SkFontHost::Serialize(const SkTypeface* face, SkWStream* stream) {
    SkDEBUGFAIL("SkFontHost::Serialize unimplemented");
}

SkTypeface* SkFontHost::Deserialize(SkStream* stream) {
    SkDEBUGFAIL("SkFontHost::Deserialize unimplemented");
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

SkTypeface* SkFontHost::NextLogicalTypeface(SkFontID currFontID,
                                            SkFontID origFontID) {
    return NULL;
}
