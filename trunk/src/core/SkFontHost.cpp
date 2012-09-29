
/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkFontHost.h"

static SkFontHost::LCDOrientation gLCDOrientation = SkFontHost::kHorizontal_LCDOrientation;
static SkFontHost::LCDOrder gLCDOrder = SkFontHost::kRGB_LCDOrder;

// static
SkFontHost::LCDOrientation SkFontHost::GetSubpixelOrientation()
{
    return gLCDOrientation;
}

// static
void SkFontHost::SetSubpixelOrientation(LCDOrientation orientation)
{
    gLCDOrientation = orientation;
}

// static
SkFontHost::LCDOrder SkFontHost::GetSubpixelOrder()
{
    return gLCDOrder;
}

// static
void SkFontHost::SetSubpixelOrder(LCDOrder order)
{
    gLCDOrder = order;
}
