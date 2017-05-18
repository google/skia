/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPath.h"
#include "GrShape.h"

static inline void write_style_key(uint32_t* key, const GrStyle& style)  {
    // Pass 1 for the scale since the GPU will apply the style not GrStyle::applyToPath().
    GrStyle::WriteKey(key, style, GrStyle::Apply::kPathEffectAndStrokeRec, SK_Scalar1);
}


void GrPath::ComputeKey(const GrShape& shape, GrUniqueKey* key, bool* outIsVolatile) {
    int geoCnt = shape.unstyledKeySize();
    int styleCnt = GrStyle::KeySize(shape.style(), GrStyle::Apply::kPathEffectAndStrokeRec);
    // This should only fail for an arbitrary path effect, and we should not have gotten
    // here with anything other than a dash path effect.
    SkASSERT(styleCnt >= 0);
    if (geoCnt < 0) {
        *outIsVolatile = true;
        return;
    }
    static const GrUniqueKey::Domain kGeneralPathDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kGeneralPathDomain, geoCnt + styleCnt);
    shape.writeUnstyledKey(&builder[0]);
    if (styleCnt) {
        write_style_key(&builder[geoCnt], shape.style());
    }
    *outIsVolatile = false;
}

#ifdef SK_DEBUG
bool GrPath::isEqualTo(const SkPath& path, const GrStyle& style) const {
    // Since this is only called in debug we don't care about performance.
    int cnt0 = GrStyle::KeySize(fStyle, GrStyle::Apply::kPathEffectAndStrokeRec);
    int cnt1 = GrStyle::KeySize(style, GrStyle::Apply::kPathEffectAndStrokeRec);
    if (cnt0 < 0 || cnt1 < 0 || cnt0 != cnt1) {
        return false;
    }
    if (cnt0) {
        SkAutoTArray<uint32_t> key0(cnt0);
        SkAutoTArray<uint32_t> key1(cnt0);
        write_style_key(key0.get(), fStyle);
        write_style_key(key1.get(), style);
        if (0 != memcmp(key0.get(), key1.get(), cnt0)) {
            return false;
        }
    }
    return fSkPath == path;
}
#endif
