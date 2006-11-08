/* libs/graphics/sgl/SkRasterizer.cpp
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

#include "SkRasterizer.h"
#include "SkDraw.h"
#include "SkMaskFilter.h"
#include "SkPath.h"

// do nothing for now, since we don't store anything at flatten time
SkRasterizer::SkRasterizer(SkRBuffer&) {}

bool SkRasterizer::rasterize(const SkPath& fillPath, const SkMatrix& matrix,
                             const SkRect16* clipBounds, SkMaskFilter* filter,
                             SkMask* mask, SkMask::CreateMode mode)
{
    SkRect16 storage;
    
    if (clipBounds && filter && SkMask::kJustRenderImage_CreateMode != mode)
    {        
        SkPoint16   margin;
        SkMask      srcM, dstM;
        
        srcM.fFormat = SkMask::kA8_Format;
        srcM.fBounds.set(0, 0, 1, 1);
        srcM.fImage = NULL;
        if (!filter->filterMask(&dstM, srcM, matrix, &margin))
            return false;
        
        storage = *clipBounds;
        storage.inset(-margin.fX, -margin.fY);
        clipBounds = &storage;
    }
    
    return this->onRasterize(fillPath, matrix, clipBounds, mask, mode);
}

/*  Our default implementation of the virtual method just scan converts
*/
bool SkRasterizer::onRasterize(const SkPath& fillPath, const SkMatrix& matrix,
                             const SkRect16* clipBounds,
                             SkMask* mask, SkMask::CreateMode mode)
{
    SkPath  devPath;
    
    fillPath.transform(matrix, &devPath);
    return SkDraw::DrawToMask(devPath, clipBounds, NULL, NULL, mask, mode);
}

