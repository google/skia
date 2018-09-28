/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_imageinfo_DEFINED
#define sk_imageinfo_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

sk_colorspace_t* sk_colorspace_new_srgb();

void sk_colorspace_ref(sk_colorspace_t*);
void sk_colorspace_unref(sk_colorspace_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
