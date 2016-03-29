/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtilsArm_DEFINED
#define SkUtilsArm_DEFINED

#include "SkUtils.h"

// Define SK_ARM_NEON_MODE to one of the following values
// corresponding respectively to:
// - No ARM Neon support at all  (not targetting ARMv7-A, or don't have NEON)
// - Full ARM Neon support (i.e. assume the CPU always supports it)
// - Optional ARM Neon support (i.e. probe CPU at runtime)
//
#define SK_ARM_NEON_MODE_NONE     0
#define SK_ARM_NEON_MODE_ALWAYS   1
#define SK_ARM_NEON_MODE_DYNAMIC  2

#if defined(SK_ARM_HAS_OPTIONAL_NEON)
#  define SK_ARM_NEON_MODE  SK_ARM_NEON_MODE_DYNAMIC
#elif defined(SK_ARM_HAS_NEON)
#  define SK_ARM_NEON_MODE  SK_ARM_NEON_MODE_ALWAYS
#else
#  define SK_ARM_NEON_MODE  SK_ARM_NEON_MODE_NONE
#endif

// Convenience test macros, always defined as 0 or 1
#define SK_ARM_NEON_IS_NONE    (SK_ARM_NEON_MODE == SK_ARM_NEON_MODE_NONE)
#define SK_ARM_NEON_IS_ALWAYS  (SK_ARM_NEON_MODE == SK_ARM_NEON_MODE_ALWAYS)
#define SK_ARM_NEON_IS_DYNAMIC (SK_ARM_NEON_MODE == SK_ARM_NEON_MODE_DYNAMIC)

// The sk_cpu_arm_has_neon() function returns true iff the target device
// is ARMv7-A and supports Neon instructions. In DYNAMIC mode, this actually
// probes the CPU at runtime (and caches the result).

#if SK_ARM_NEON_IS_NONE
static inline bool sk_cpu_arm_has_neon(void) {
    return false;
}
#elif SK_ARM_NEON_IS_ALWAYS
static inline bool sk_cpu_arm_has_neon(void) {
    return true;
}
#else // SK_ARM_NEON_IS_DYNAMIC

extern bool sk_cpu_arm_has_neon(void) SK_PURE_FUNC;
#endif

// Use SK_ARM_NEON_WRAP(symbol) to map 'symbol' to a NEON-specific symbol
// when applicable. This will transform 'symbol' differently depending on
// the current NEON configuration, i.e.:
//
//    NONE           -> 'symbol'
//    ALWAYS         -> 'symbol_neon'
//    DYNAMIC        -> 'symbol' or 'symbol_neon' depending on runtime check.
//
// The goal is to simplify user code, for example:
//
//      return SK_ARM_NEON_WRAP(do_something)(params);
//
// Replaces the equivalent:
//
//     #if SK_ARM_NEON_IS_NONE
//       return do_something(params);
//     #elif SK_ARM_NEON_IS_ALWAYS
//       return do_something_neon(params);
//     #elif SK_ARM_NEON_IS_DYNAMIC
//       if (sk_cpu_arm_has_neon())
//         return do_something_neon(params);
//       else
//         return do_something(params);
//     #endif
//
#if SK_ARM_NEON_IS_NONE
#  define SK_ARM_NEON_WRAP(x)   (x)
#elif SK_ARM_NEON_IS_ALWAYS
#  define SK_ARM_NEON_WRAP(x)   (x ## _neon)
#elif SK_ARM_NEON_IS_DYNAMIC
#  define SK_ARM_NEON_WRAP(x)   (sk_cpu_arm_has_neon() ? x ## _neon : x)
#endif

#endif // SkUtilsArm_DEFINED
