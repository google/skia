/* include/graphics/SkFontHost.h
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

#ifndef SkFontHost_DEFINED
#define SkFontHost_DEFINED

#include "SkScalerContext.h"
#include "SkTypeface.h"

class SkDescriptor;

/** \class SkFontHost

    This class is ported to each environment. It is responsible for bridging the gap
    between SkTypeface and the resulting platform-specific instance of SkScalerContext.
*/
class SkFontHost {
public:
    /** Return a subclass of SkTypeface, one that can be used by your scalaracontext
        (returned by SkFontHost::CreateScalarContext).
        1) If family is null, use name.
        2) If name is null, use family.
        3) If both are null, use default family.
    */
    static SkTypeface* CreateTypeface(const SkTypeface* family, const char name[], SkTypeface::Style);
    /** Given a typeface (or null), return the number of bytes needed to flatten it
        into a buffer, for the purpose of communicating information to the
        scalercontext. If buffer is null, then ignore it but still return the number
        of bytes that would be written.
    */
    static uint32_t FlattenTypeface(const SkTypeface* face, void* buffer);
    /** Return a subclass of SkScalarContext
    */
    static SkScalerContext* CreateScalerContext(const SkDescriptor* desc);
    /** Return a scalercontext using the "fallback" font. If there is no designated
        fallback, return null.
    */
    static SkScalerContext* CreateFallbackScalerContext(const SkScalerContext::Rec&);
};

#endif

