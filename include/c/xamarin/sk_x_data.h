/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_x_data_DEFINED
#define sk_x_data_DEFINED

#include "sk_types.h"
#include "xamarin\sk_x_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API sk_data_t* sk_data_new_from_file(const char* path);
SK_API sk_data_t* sk_data_new_from_stream(sk_stream_t* stream, size_t length);
SK_API const uint8_t* sk_data_get_bytes(const sk_data_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
