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
#include "include/private/SkNx.h"
#include "src/core/SkOpts.h"

// Templates shared by various 4f gradient flavors.

namespace {  // NOLINT(google-build-namespaces)

enum class ApplyPremul { True, False };

template <ApplyPremul>
struct PremulTraits;

template <>
struct PremulTraits<ApplyPremul::False> {
    static Sk4f apply(const Sk4f& c) { return c; }
};

template <>
struct PremulTraits<ApplyPremul::True> {
    static Sk4f apply(const Sk4f& c) {
        const float alpha = c[3];
        // FIXME: portable swizzle?
        return c * Sk4f(alpha, alpha, alpha, 1);
    }
};

// Struct encapsulating various dest-dependent ops:
//
//   - load()       Load a SkPMColor4f value into Sk4f.  Normally called once per interval
//                  advance.  Also applies a scale and swizzle suitable for DstType.
//
//   - store()      Store one Sk4f to dest.  Optionally handles premul, color space
//                  conversion, etc.
//
//   - store(count) Store the Sk4f value repeatedly to dest, count times.
//
//   - store4x()    Store 4 Sk4f values to dest (opportunistic optimization).
//

template <ApplyPremul premul>
struct DstTraits {
    using PM   = PremulTraits<premul>;

    // For L32, prescaling by 255 saves a per-pixel multiplication when premul is not needed.
    static Sk4f load(const SkPMColor4f& c) {
        Sk4f c4f = swizzle_rb_if_bgra(Sk4f::Load(c.vec()));
        return premul == ApplyPremul::False
            ? c4f * Sk4f(255)
            : c4f;
    }

    static void store(const Sk4f& c, SkPMColor* dst, const Sk4f& bias) {
        if (premul == ApplyPremul::False) {
            // c is pre-scaled by 255 and pre-biased, just store.
            SkNx_cast<uint8_t>(c).store(dst);
        } else {
            *dst = Sk4f_toL32(PM::apply(c) + bias);
        }
    }

    static void store(const Sk4f& c, SkPMColor* dst, int n) {
        SkPMColor pmc;
        store(c, &pmc, Sk4f(0));
        sk_memset32(dst, pmc, n);
    }

    static void store4x(const Sk4f& c0, const Sk4f& c1,
                        const Sk4f& c2, const Sk4f& c3,
                        SkPMColor* dst,
                        const Sk4f& bias0,
                        const Sk4f& bias1) {
        if (premul == ApplyPremul::False) {
            // colors are pre-scaled and pre-biased.
            Sk4f_ToBytes((uint8_t*)dst, c0, c1, c2, c3);
        } else {
            store(c0, dst + 0, bias0);
            store(c1, dst + 1, bias1);
            store(c2, dst + 2, bias0);
            store(c3, dst + 3, bias1);
        }
    }

    static Sk4f pre_lerp_bias(const Sk4f& bias) {
        // We can apply the bias before interpolation when the colors are premultiplied.
        return premul == ApplyPremul::False ? bias : 0;
    }
};

} // anonymous namespace

#endif // Sk4fGradientPriv_DEFINED
