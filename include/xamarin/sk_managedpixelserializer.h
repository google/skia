/*
 * Copyright 2017 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_managedpixelserializer_DEFINED
#define sk_managedpixelserializer_DEFINED

#include "sk_xamarin.h"

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD


typedef struct sk_managedpixelserializer_t sk_managedpixelserializer_t;


typedef bool       (*sk_managedpixelserializer_use_delegate)    (sk_managedpixelserializer_t* serializer, const void* data, size_t len);
typedef sk_data_t* (*sk_managedpixelserializer_encode_delegate) (sk_managedpixelserializer_t* serializer, const sk_pixmap_t* pixmap);


SK_X_API sk_managedpixelserializer_t* sk_managedpixelserializer_new ();
SK_X_API void sk_managedpixelserializer_set_delegates (
    const sk_managedpixelserializer_use_delegate pUse,
    const sk_managedpixelserializer_encode_delegate pEncode);


SK_C_PLUS_PLUS_END_GUARD

#endif
