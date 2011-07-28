
/*
 * Copyright 2008 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


// -----------------------------------------------------------------------------
// This is a noop gamma implementation for systems where gamma is already
// corrected, or dealt with in a system wide fashion. For example, on X windows
// one uses the xgamma utility to set the server-wide gamma correction value.
// -----------------------------------------------------------------------------

#include "SkFontHost.h"

void SkFontHost::GetGammaTables(const uint8_t* tables[2])
{
    tables[0] = NULL;
    tables[1] = NULL;
}

int SkFontHost::ComputeGammaFlag(const SkPaint& paint)
{
    return 0;
}

