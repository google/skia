
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkFlate_DEFINED
#define SkFlate_DEFINED

#include "SkTypes.h"

class SkData;
class SkWStream;
class SkStream;

/** \class SkFlate
    A class to provide access to the flate compression algorithm.
*/
class SkFlate {
public:
    /** Indicates if the flate algorithm is available.
     */
    static bool HaveFlate();

    /**
     *  Use the flate compression algorithm to compress the data in src,
     *  putting the result into dst.  Returns false if an error occurs.
     */
    static bool Deflate(SkStream* src, SkWStream* dst);
    
    /**
     *  Use the flate compression algorithm to compress the data in src,
     *  putting the result into dst.  Returns false if an error occurs.
     */
    static bool Deflate(const void* src, size_t len, SkWStream* dst);
    
    /**
     *  Use the flate compression algorithm to compress the data,
     *  putting the result into dst.  Returns false if an error occurs.
     */
    static bool Deflate(const SkData*, SkWStream* dst);
    
    /** Use the flate compression algorithm to decompress the data in src,
        putting the result into dst.  Returns false if an error occurs.
     */
    static bool Inflate(SkStream* src, SkWStream* dst);
};

#endif
