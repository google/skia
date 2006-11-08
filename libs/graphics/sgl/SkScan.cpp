/* libs/graphics/sgl/SkScan.cpp
**
** Copyright 2006, Google Inc.
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

#include "SkScan.h"
#include "SkBlitter.h"
#include "SkRegion.h"

void SkScan::FillRect(const SkRect& rect, const SkRegion* clip, SkBlitter* blitter)
{
    SkRect16 r;

    rect.round(&r);
    SkScan::FillDevRect(r, clip, blitter);
}

void SkScan::FillDevRect(const SkRect16& r, const SkRegion* clip, SkBlitter* blitter)
{
    if (!r.isEmpty())
    {
        if (clip)
        {
            SkRegion::Cliperator    cliper(*clip, r);
            const SkRect16&         rr = cliper.rect();

            while (!cliper.done())
            {
                blitter->blitRect(rr.fLeft, rr.fTop, rr.width(), rr.height());
                cliper.next();
            }
        }
        else
            blitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
    }
}

