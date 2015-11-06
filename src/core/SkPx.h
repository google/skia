/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPx_DEFINED
#define SkPx_DEFINED

#include "SkTypes.h"
#include "SkColorPriv.h"

// We'll include one of src/opts/SkPx_{sse,neon,none}.h to define a type SkPx.
//
// SkPx represents up to SkPx::N 8888 pixels.  It's agnostic to whether these
// are SkColors or SkPMColors; it only assumes that alpha is the high byte.
static_assert(SK_A32_SHIFT == 24, "For both SkColor and SkPMColor, alpha is always the high byte.");
//
// SkPx::Alpha represents up to SkPx::N 8-bit values, usually coverage or alpha.
// SkPx::Wide represents up to SkPx::N pixels with 16 bits per component.
//
// SkPx supports the following methods:
//    static SkPx Dup(uint32_t);
//    static SkPx Load(const uint32_t*);
//    static SkPx Load(const uint32_t*, int n);  // where 0<n<SkPx::N
//    void store(uint32_t*) const;
//    void store(uint32_t*, int n) const;        // where 0<n<SkPx::N
//
//    Alpha alpha() const;    // argb -> a
//    Wide widenLo() const;   // argb -> 0a0r0g0b
//    Wide widenHi() const;   // argb -> a0r0g0b0
//    Wide widenLoHi() const; // argb -> aarrggbb
//
//    SkPx    operator+(const SkPx&) const;
//    SkPx    operator-(const SkPx&) const;
//    SkPx saturatedAdd(const SkPx&) const;
//
//    Wide operator*(const Alpha&) const;  // argb * A -> (a*A)(r*A)(g*A)(b*A)
//
//    // Fast approximate (px*a+127)/255.
//    // Never off by more than 1, and always correct when px or a is 0 or 255.
//    // We use the approximation (px*a+px)/256.
//    SkPx approxMulDiv255(const Alpha&) const;
//
//    SkPx addAlpha(const Alpha&) const;  // argb + A -> (a+A)rgb
//
// SkPx::Alpha supports the following methods:
//    static Alpha Dup(uint8_t);
//    static Alpha Load(const uint8_t*);
//    static Alpha Load(const uint8_t*, int n);  // where 0<n<SkPx::N
//
//    Alpha inv() const;  // a -> 255-a
//
// SkPx::Wide supports the following methods:
//    Wide operator+(const Wide&);
//    Wide operator-(const Wide&);
//    Wide shl<int bits>();
//    Wide shr<int bits>();
//
//    // Return the high byte of each component of (*this + o.widenLo()).
//    SkPx addNarrowHi(const SkPx& o);
//
// Methods left unwritten, but certainly to come:
//    SkPx SkPx::operator<(const SkPx&) const;
//    SkPx SkPx::thenElse(const SkPx& then, const SkPx& else) const;
//    Wide Wide::operator<(const Wide&) const;
//    Wide Wide::thenElse(const Wide& then, const Wide& else) const;
//
//    SkPx Wide::div255() const;  // Rounds, think (*this + 127) / 255.
//
//  The different implementations of SkPx have complete freedom to choose
//  SkPx::N and how they represent SkPx, SkPx::Alpha, and SkPx::Wide.
//
//  All observable math must remain identical.

#if defined(SKNX_NO_SIMD)
    #include "../opts/SkPx_none.h"
#else
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
        #include "../opts/SkPx_sse.h"
    #elif defined(SK_ARM_HAS_NEON)
        #include "../opts/SkPx_neon.h"
    #else
        #include "../opts/SkPx_none.h"
    #endif
#endif

#endif//SkPx_DEFINED
