/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPackBits_DEFINED
#define SkPackBits_DEFINED

#include "SkTypes.h"

class SkPackBits {
public:
    /** Given the number of 8bit values that will be passed to Pack8,
        returns the worst-case size needed for the dst[] buffer.
    */
    static size_t ComputeMaxSize8(size_t srcSize);

    /** Write the src array into a packed format. The packing process may end
        up writing more bytes than it read, so dst[] must be large enough.
        @param src      Input array of 8bit values
        @param srcSize  Number of entries in src[]
        @param dst      Buffer (allocated by caller) to write the packed data
                        into
        @param dstSize  Number of bytes in the output buffer.
        @return the number of bytes written to dst[]
    */
    static size_t Pack8(const uint8_t src[], size_t srcSize, uint8_t dst[],
                        size_t dstSize);

    /** Unpack the data in src[], and expand it into dst[]. The src[] data was
        written by a previous call to Pack8.
        @param src      Input data to unpack, previously created by Pack8.
        @param srcSize  Number of bytes of src to unpack
        @param dst      Buffer (allocated by caller) to expand the src[] into.
        @param dstSize  Number of bytes in the output buffer.
        @return the number of bytes written into dst.
    */
    static int Unpack8(const uint8_t src[], size_t srcSize, uint8_t dst[],
                       size_t dstSize);
};

#endif
