/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4fGradientPriv_DEFINED
#define Sk4fGradientPriv_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/private/SkColorData.h"
#include "include/private/SkHalf.h"
#include "include/private/SkVx.h"
#include "src/core/SkOpts.h"

// Templates shared by various 4f gradient flavors.

namespace {  // NOLINT(google-build-namespaces)

enum class ApplyPremul { True, False };

template <ApplyPremul>
struct PremulTraits;

template <>
struct PremulTraits<ApplyPremul::False> {
    static skvx::float4 apply(const skvx::float4& c) { return c; }
};

template <>
struct PremulTraits<ApplyPremul::True> {
    static skvx::float4 apply(const skvx::float4& c) {
        const float alpha = c[3];
        // FIXME: portable swizzle?
        return c * skvx::float4(alpha, alpha, alpha, 1);
    }
};

// Struct encapsulating various dest-dependent ops:
//
//   - load()       Load a SkPMColor4f value into skvx::float4.  Normally called once per interval
//                  advance.  Also applies a scale and swizzle suitable for DstType.
//
//   - store()      Store one skvx::float4 to dest.  Optionally handles premul, color space
//                  conversion, etc.
//
//   - store(count) Store the skvx::float4 value repeatedly to dest, count times.
//
//   - store4x()    Store 4 skvx::float4 values to dest (opportunistic optimization).
//

template <ApplyPremul premul>
struct DstTraits {
    using PM   = PremulTraits<premul>;

    // For L32, prescaling by 255 saves a per-pixel multiplication when premul is not needed.
    static skvx::float4 load(const SkPMColor4f& c) {
        skvx::float4 c4f = swizzle_rb_if_bgra(skvx::float4::Load(c.vec()));
        return premul == ApplyPremul::False
            ? c4f * skvx::float4(255)
            : c4f;
    }

    static void store(const skvx::float4& c, SkPMColor* dst, const skvx::float4& bias) {
        if (premul == ApplyPremul::False) {
            // c is pre-scaled by 255 and pre-biased, just store.
            skvx::cast<uint8_t>(c).store(dst);
        } else {
            *dst = Sk4f_toL32(PM::apply(c) + bias);
        }
    }

    static void store(const skvx::float4& c, SkPMColor* dst, int n) {
        SkPMColor pmc;
        store(c, &pmc, skvx::float4(0));
        sk_memset32(dst, pmc, n);
    }

    static void store4x(const skvx::float4& c0, const skvx::float4& c1,
                        const skvx::float4& c2, const skvx::float4& c3,
                        SkPMColor* dst,
                        const skvx::float4& bias0,
                        const skvx::float4& bias1) {
        if (premul == ApplyPremul::False) {
            // colors are pre-scaled and pre-biased.
            skvx::cast<uint8_t>(c0).store(dst + 0);
            skvx::cast<uint8_t>(c1).store(dst + 1);
            skvx::cast<uint8_t>(c2).store(dst + 2);
            skvx::cast<uint8_t>(c3).store(dst + 3);
        } else {
            store(c0, dst + 0, bias0);
            store(c1, dst + 1, bias1);
            store(c2, dst + 2, bias0);
            store(c3, dst + 3, bias1);
        }
    }

    static skvx::float4 pre_lerp_bias(const skvx::float4& bias) {
        // We can apply the bias before interpolation when the colors are premultiplied.
        return premul == ApplyPremul::False ? bias : 0;
    }
};

} // anonymous namespace

#endif // Sk4fGradientPriv_DEFINED
