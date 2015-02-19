/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPath.h"

template<int NumBits> static uint64_t get_top_n_float_bits(float f) {
    char* floatData = reinterpret_cast<char*>(&f);
    uint32_t floatBits = *reinterpret_cast<uint32_t*>(floatData);
    return floatBits >> (32 - NumBits);
}

void GrPath::ComputeKey(const SkPath& path, const SkStrokeRec& stroke, GrUniqueKey* key) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kDomain, 3);
    *reinterpret_cast<uint64_t*>(&builder[0]) = ComputeStrokeKey(stroke);
    builder[2] = path.getGenerationID();
}

uint64_t GrPath::ComputeStrokeKey(const SkStrokeRec& stroke) {
    enum {
        kStyleBits = 2,
        kJoinBits = 2,
        kCapBits = 2,
        kWidthBits = 29,
        kMiterBits = 29,

        kJoinShift = kStyleBits,
        kCapShift = kJoinShift + kJoinBits,
        kWidthShift = kCapShift + kCapBits,
        kMiterShift = kWidthShift + kWidthBits,

        kBitCount = kMiterShift + kMiterBits
    };

    SK_COMPILE_ASSERT(SkStrokeRec::kStyleCount <= (1 << kStyleBits), style_shift_will_be_wrong);
    SK_COMPILE_ASSERT(SkPaint::kJoinCount <= (1 << kJoinBits), cap_shift_will_be_wrong);
    SK_COMPILE_ASSERT(SkPaint::kCapCount <= (1 << kCapBits), miter_shift_will_be_wrong);
    SK_COMPILE_ASSERT(kBitCount == 64, wrong_stroke_key_size);

    if (!stroke.needToApply()) {
        return SkStrokeRec::kFill_Style;
    }

    uint64_t key = stroke.getStyle();
    key |= stroke.getJoin() << kJoinShift;
    key |= stroke.getCap() << kCapShift;
    key |= get_top_n_float_bits<kWidthBits>(stroke.getWidth()) << kWidthShift;
    key |= get_top_n_float_bits<kMiterBits>(stroke.getMiter()) << kMiterShift;

    return key;
}
