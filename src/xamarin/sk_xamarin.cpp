/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"

#include "sk_xamarin.h"

#include "sk_types_priv.h"

#ifdef NEED_INIT_NEON

namespace SkOpts {
    void Init_neon() {}
}

#endif
