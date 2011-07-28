
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkGlobals.h"
#include "SkThread.h"

static SkGlobals::BootStrap gBootStrap;

SkGlobals::BootStrap& SkGlobals::GetBootStrap()
{
    return gBootStrap;
}


