/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SkFontHost_DEFINED
#define SkFontHost_DEFINED

#include "SkScalerContext.h"
#include "SkTypeface.h"

class SkDescriptor;
class SkStream;
class SkWStream;

/** \class SkFontHost

    This class is ported to each environment. It is responsible for bridging the gap
    between SkTypeface and the resulting platform-specific instance of SkScalerContext.
*/
class SkFontHost {
public:
    /** Return the closest matching typeface given either an existing family
        (specified by a typeface in that family) or by a familyName, and a
        requested style.
        1) If familyFace is null, use famillyName.
        2) If famillyName is null, use familyFace.
        3) If both are null, return the default font that best matches style

        NOTE: this does not return a new typeface, nor does it affect the
        owner count of an existing one, so the caller is free to ignore the
        return result, or just compare it against null.
     */
    static SkTypeface* FindTypeface(const SkTypeface* familyFace,
                                    const char famillyName[],
                                    SkTypeface::Style style);

    /** Return the typeface associated with the uniqueID, or null if that ID
        does not match any faces.

        NOTE: this does not return a new typeface, nor does it affect the
        owner count of an existing one, so the caller is free to ignore the
        return result, or just compare it against null.
    */
    static SkTypeface* ResolveTypeface(uint32_t uniqueID);
    
    /** Return a new stream to read the font data, or null if the uniqueID does
        not match an existing typeface. The caller must call CloseStream() when
        it is finished reading the stream.
    */
    static SkStream* OpenStream(uint32_t uniqueID);
    
    /** Call this when finished reading from the stream returned by OpenStream.
        The caller should NOT try to delete the stream.
     */
    static void CloseStream(uint32_t uniqueID, SkStream*);

    /** Return a new typeface given the data buffer (owned by the caller).
        If the data does not represent a valid font, return null. The caller is
        responsible for unref-ing the returned typeface (if it is not null).
    */
    static SkTypeface* CreateTypeface(SkStream*);

    ///////////////////////////////////////////////////////////////////////////

    /** Write a unique identifier to the stream, so that the same typeface can
        be retrieved with Deserialize().
    */
    static void Serialize(const SkTypeface*, SkWStream*);

    /** Given a stream created by Serialize(), return the corresponding typeface
        or null if no match is found.

        NOTE: this does not return a new typeface, nor does it affect the
        owner count of an existing one, so the caller is free to ignore the
        return result, or just compare it against null.
     */
    static SkTypeface* Deserialize(SkStream*);

    ///////////////////////////////////////////////////////////////////////////
    
    /** Return a subclass of SkScalarContext
    */
    static SkScalerContext* CreateScalerContext(const SkDescriptor* desc);

    /** Return a scalercontext using the "fallback" font. If there is no designated
        fallback, return null.
    */
    static SkScalerContext* CreateFallbackScalerContext(const SkScalerContext::Rec&);

    /** Return the number of bytes (approx) that should be purged from the font
        cache. The input parameter is the cache's estimate of how much as been
        allocated by the cache so far.
        To purge (basically) everything, return the input parameter.
        To purge nothing, return 0
    */
    static size_t ShouldPurgeFontCache(size_t sizeAllocatedSoFar);

    /** Return SkScalerContext gamma flag, or 0, based on the paint that will be
        used to draw something with antialiasing.
    */
    static int ComputeGammaFlag(const SkPaint& paint);

    /** Return NULL or a pointer to 256 bytes for the black (table[0]) and
        white (table[1]) gamma tables.
    */
    static void GetGammaTables(const uint8_t* tables[2]);
};

#endif

