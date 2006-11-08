/* libs/graphics/ports/SkFontHost_none.cpp
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

#include "SkFontHost.h"

SkTypeface* SkFontHost::CreateTypeface( const SkTypeface* familyFace,
                                        const char familyName[],
                                        SkTypeface::Style style)
{
    SkASSERT(!"SkFontHost::CreateTypeface unimplemented");
    return nil;
}

uint32_t SkFontHost::FlattenTypeface(const SkTypeface* tface, void* buffer)
{
    SkASSERT(!"SkFontHost::FlattenTypeface unimplemented");
    return 0;
}

SkScalerContext* SkFontHost::CreateScalerContext(const SkDescriptor* desc)
{
    SkASSERT(!"SkFontHost::CreateScalarContext unimplemented");
    return nil;
}

SkFontHost::ScalerContextID SkFontHost::FindScalerContextIDForUnichar(int32_t unichar)
{
    SkASSERT(!"SkFontHost::FindScalerContextIDForUnichar unimplemented");
    return kMissing_ScalerContextID;
}

SkScalerContext* SkFontHost::CreateScalerContextFromID(SkFontHost::ScalerContextID id, const SkScalerContext::Rec& rec)
{
    SkASSERT(!"SkFontHost::CreateScalerContextFromID unimplemented");
    return nil;
}
