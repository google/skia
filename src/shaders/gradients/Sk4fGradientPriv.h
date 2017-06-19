/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4fGradientPriv_DEFINED
#define Sk4fGradientPriv_DEFINED

#include "SkColor.h"
#include "SkHalf.h"
#include "SkImageInfo.h"
#include "SkNx.h"
#include "SkPM4f.h"
#include "SkPM4fPriv.h"
#include "SkUtils.h"

// Templates shared by various 4f gradient flavors.

namespace {

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
        const float alpha = c[SkPM4f::A];
        // FIXME: portable swizzle?
        return c * Sk4f(alpha, alpha, alpha, 1);
    }
};

// Struct encapsulating various dest-dependent ops:
//
//   - load()       Load a SkPM4f value into Sk4f.  Normally called once per interval
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

    static Sk4f load(const SkPM4f& c) {
        return c.to4f();
    }

    static void store(const Sk4f& c, SkPM4f* dst) {
        PM::apply(c).store(dst->fVec);
    }

    static void store(const Sk4f& c, SkPM4f* dst, int n) {
        const Sk4f pmc = PM::apply(c);
        for (int i = 0; i < n; ++i) {
            pmc.store(dst[i].fVec);
        }
    }

    static void store4x(const Sk4f& c0, const Sk4f& c1,
                        const Sk4f& c2, const Sk4f& c3,
                        SkPM4f* dst) {
        store(c0, dst + 0);
        store(c1, dst + 1);
        store(c2, dst + 2);
        store(c3, dst + 3);
    }
};

} // anonymous namespace

#endif // Sk4fGradientPriv_DEFINED
