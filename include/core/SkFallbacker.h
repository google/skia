/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFallbacker_DEFINED
#define SkFallbacker_DEFINED

#include "include/core/SkFontTypes.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypeface.h"

#include <vector>

class SK_API SkFallbacker : public SkRefCnt {
public:
    ~SkFallbacker() override {}

    struct Rec {
        sk_sp<SkTypeface> typeface;
        size_t            textBytes;
    };

    virtual std::vector<Rec> resolve(const void* text, size_t textBytes, SkTextEncoding) const = 0;

    static sk_sp<SkFallbacker> MakeSimpleOrdered(SkSpan<SkTypeface*>);
};

#endif
