/* libs/graphics/sgl/SkMask.cpp
**
** Copyright 2007, The Android Open Source Project
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

#include "SkMask.h"

size_t SkMask::computeImageSize() const
{
    return fBounds.height() * fRowBytes;
}

size_t SkMask::computeTotalImageSize() const
{
    size_t size = this->computeImageSize();

    if (fFormat == SkMask::k3D_Format)
        size *= 3;
    return size;
}

/** We explicitly use this allocator for SkBimap pixels, so that we can
    freely assign memory allocated by one class to the other.
*/
uint8_t* SkMask::AllocImage(size_t size)
{
    return (uint8_t*)sk_malloc_throw(SkAlign4(size));
}

/** We explicitly use this allocator for SkBimap pixels, so that we can
    freely assign memory allocated by one class to the other.
*/
void SkMask::FreeImage(void* image)
{
    sk_free(image);
}

