/* libs/graphics/sgl/SkScanPriv.h
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

#ifndef SkScanPriv_DEFINED
#define SkScanPriv_DEFINED

#include "SkScan.h"
#include "SkBlitter.h"

class SkScanClipper {
public:
    SkScanClipper(SkBlitter* blitter, const SkRegion* clip, const SkRect16& bounds);

    SkBlitter*      getBlitter() const { return fBlitter; }
    const SkRect16* getClipRect() const { return fClipRect; }

private:
    SkRectClipBlitter   fRectBlitter;
    SkRgnClipBlitter    fRgnBlitter;
    SkBlitter*          fBlitter;
    const SkRect16*     fClipRect;
};

void sk_fill_path(const SkPath& path, const SkRect16* clipRect, SkBlitter* blitter,
                  const SkRect16& ir, int shiftEdgesUp);

#endif

