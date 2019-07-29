/*
 * Copyright 2019 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_general_DEFINED
#define sk_general_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

// ref counting

SK_C_API bool sk_refcnt_unique(const sk_refcnt_t* refcnt);
SK_C_API int sk_refcnt_get_ref_count(const sk_refcnt_t* refcnt);
SK_C_API void sk_refcnt_safe_ref(sk_refcnt_t* refcnt);
SK_C_API void sk_refcnt_safe_unref(sk_refcnt_t* refcnt);

SK_C_API bool sk_nvrefcnt_unique(const sk_nvrefcnt_t* refcnt);
SK_C_API int sk_nvrefcnt_get_ref_count(const sk_nvrefcnt_t* refcnt);
SK_C_API void sk_nvrefcnt_safe_ref(sk_nvrefcnt_t* refcnt);
SK_C_API void sk_nvrefcnt_safe_unref(sk_nvrefcnt_t* refcnt);

// color type

SK_C_API sk_colortype_t sk_colortype_get_default_8888(void);

SK_C_PLUS_PLUS_END_GUARD

#endif
