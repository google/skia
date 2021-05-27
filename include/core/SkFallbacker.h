/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFallbacker_DEFINED
#define SkFallbacker_DEFINED

#include "include/core/SkFontTypes.h"
#include "include/core/SkTypeface.h"

class SK_API SkFallbacker : public SkRefCnt {
public:
    ~SkFallbacker() override {}

    struct Rec {
        sk_sp<SkTypeface> typeface;
        size_t            textBytes;
    };

    std::vector<Rec> resolve(const void* text, size_t bytes, SkTextEncoding) const = 0;
};

class SkSimpleFallbacker {
public:
    static sk_sp<SkFallbacker> Make(SkSpan<SkTypeface*> inPreferredOrder);
};

#endif
