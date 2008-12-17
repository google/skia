/* libs/graphics/sgl/SkScan.cpp
**
** Copyright 2006, The Android Open Source Project
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

static inline void blitrect(SkBlitter* blitter, const SkIRect& r) {
    blitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
}

void SkScan::FillIRect(const SkIRect& r, const SkRegion* clip,
                       SkBlitter* blitter) {
    if (!r.isEmpty()) {
        if (clip) {
            if (clip->isRect()) {
                const SkIRect& clipBounds = clip->getBounds();
                
                if (clipBounds.contains(r)) {
                    blitrect(blitter, r);
                } else {
                    SkIRect rr = r;
                    if (rr.intersect(clipBounds)) {
                        blitrect(blitter, rr);
                    }
                }
            } else {
                SkRegion::Cliperator    cliper(*clip, r);
                const SkIRect&          rr = cliper.rect();
                
                while (!cliper.done()) {
                    blitrect(blitter, rr);
                    cliper.next();
                }
            }
        } else {
            blitrect(blitter, r);
        }
    }
}

void SkScan::FillXRect(const SkXRect& xr, const SkRegion* clip,
                       SkBlitter* blitter) {
    SkIRect r;
    
    XRect_round(xr, &r);
    SkScan::FillIRect(r, clip, blitter);
}

#ifdef SK_SCALAR_IS_FLOAT

void SkScan::FillRect(const SkRect& r, const SkRegion* clip,
                       SkBlitter* blitter) {
    SkIRect ir;
    
    r.round(&ir);
    SkScan::FillIRect(ir, clip, blitter);
}

#endif

