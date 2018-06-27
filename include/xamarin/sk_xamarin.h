/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
