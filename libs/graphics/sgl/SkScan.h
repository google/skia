/* libs/graphics/sgl/SkScan.h
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

#ifndef SkScan_DEFINED
#define SkScan_DEFINED

#include "SkRect.h"

class SkRegion;
class SkBlitter;
class SkPath;

class SkScan {
public:
    static void FillDevRect(const SkRect16&, const SkRegion* clip, SkBlitter*);
    static void FillRect(const SkRect&, const SkRegion* clip, SkBlitter*);
    static void FillPath(const SkPath&, const SkRegion* clip, SkBlitter*);

    static void HairLine(const SkPoint&, const SkPoint&, const SkRegion* clip, SkBlitter*);
    static void HairRect(const SkRect&, const SkRegion* clip, SkBlitter*);
    static void HairPath(const SkPath&, const SkRegion* clip, SkBlitter*);

    static void FrameRect(const SkRect&, SkScalar width, const SkRegion* clip, SkBlitter*);

    static void AntiFillRect(const SkRect&, const SkRegion* clip, SkBlitter*);
    static void AntiFillPath(const SkPath&, const SkRegion* clip, SkBlitter*);

    static void AntiHairLine(const SkPoint&, const SkPoint&, const SkRegion* clip, SkBlitter*);
    static void AntiHairRect(const SkRect&, const SkRegion* clip, SkBlitter*);
    static void AntiHairPath(const SkPath&, const SkRegion* clip, SkBlitter*);
};

#endif
