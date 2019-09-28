/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_xml_DEFINED
#define sk_xml_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API sk_xmlstreamwriter_t* sk_xmlstreamwriter_new(sk_wstream_t* stream);
SK_C_API void sk_xmlstreamwriter_delete(sk_xmlstreamwriter_t* writer);

SK_C_PLUS_PLUS_END_GUARD

#endif
