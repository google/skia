/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOverdrawMode_DEFINED
#define SkOverdrawMode_DEFINED

#include "SkFlattenable.h"

class SkXfermode;

class SkOverdrawMode {
public:
    static sk_sp<SkXfermode> Make();

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP();

private:
    SkOverdrawMode(); // can't be instantiated
};

#endif
