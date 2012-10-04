
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSerializationHelpers_DEFINED
#define SkSerializationHelpers_DEFINED

class SkBitmap;
class SkStream;
class SkWStream;

namespace SkSerializationHelpers {
    /**
     *  Function to encode an SkBitmap to an SkWStream. A function with this signature can be passed
     *  to SkPicture::serialize() and SkOrderedWriteBuffer. The function should return true if it
     *  succeeds. Otherwise it should return false so that SkOrderedWriteBuffer can switch to
     *  another method of storing SkBitmaps.
     */
    typedef bool (*EncodeBitmap)(SkWStream*, const SkBitmap&);

    /**
     *  Function to decode an SkBitmap from an SkStream. A function with this signature can be
     *  passed to the SkStream constructor for SkPicture and SkOrderedReadBuffer to decode SkBitmaps
     *  which were previously encoded. The function should return true if it succeeds. Otherwise it
     *  should return false so that SkOrderedReadBuffer can skip the data and provide a dummy
     *  SkBitmap.
     */
    typedef bool (*DecodeBitmap)(SkStream*, SkBitmap*);
}

#endif // SkSerializationHelpers_DEFINED
