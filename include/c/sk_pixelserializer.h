/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_pixelserializer_DEFINED
#define sk_pixelserializer_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API void sk_pixelserializer_unref(sk_pixelserializer_t* cserializer);
SK_C_API bool sk_pixelserializer_use_encoded_data(sk_pixelserializer_t* cserializer, const void* data, size_t len);
SK_C_API sk_data_t* sk_pixelserializer_encode(sk_pixelserializer_t* cserializer, const sk_pixmap_t* cpixmap);

SK_C_PLUS_PLUS_END_GUARD

#endif
