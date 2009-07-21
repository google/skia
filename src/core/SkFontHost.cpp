/* libs/graphics/sgl/SkFontHost.cpp
**
** Copyright 2009, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
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
