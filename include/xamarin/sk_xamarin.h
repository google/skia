/*
 * Copyright 2015 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY


#ifndef sk_xamarin_DEFINED
#define sk_xamarin_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

#if defined(_WIN32)
#  define SK_X_API __declspec(dllexport)
#else
#  define SK_X_API __attribute__((visibility("default")))
#endif

SK_C_PLUS_PLUS_END_GUARD

#endif
