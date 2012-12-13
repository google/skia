/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ANDROID_CUTILS_COMPILER_H
#define ANDROID_CUTILS_COMPILER_H

/*
 * helps the compiler's optimizer predicting branches
 */

#ifdef __cplusplus
#   define CC_LIKELY( exp )    (__builtin_expect( !!(exp), true ))
#   define CC_UNLIKELY( exp )  (__builtin_expect( !!(exp), false ))
#else
#   define CC_LIKELY( exp )    (__builtin_expect( !!(exp), 1 ))
#   define CC_UNLIKELY( exp )  (__builtin_expect( !!(exp), 0 ))
#endif

/**
 * exports marked symbols
 *
 * if used on a C++ class declaration, this macro must be inserted
 * after the "class" keyword. For instance:
 *
 * template <typename TYPE>
 * class ANDROID_API Singleton { }
 */

#define ANDROID_API __attribute__((visibility("default")))

#endif // ANDROID_CUTILS_COMPILER_H
