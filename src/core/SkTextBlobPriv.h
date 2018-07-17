/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextBlobPriv_DEFINED
#define SkTextBlobPriv_DEFINED

#include "SkTextBlob.h"

class SkReadBuffer;
class SkWriteBuffer;

class SkTextBlobPriv {
public:
    /**
     *  Serialize to a buffer.
     */
    static void Flatten(const SkTextBlob& , SkWriteBuffer&);

    /**
     *  Recreate an SkTextBlob that was serialized into a buffer.
     *
     *  @param  SkReadBuffer Serialized blob data.
     *  @return A new SkTextBlob representing the serialized data, or NULL if the buffer is
     *          invalid.
     */
    static sk_sp<SkTextBlob> MakeFromBuffer(SkReadBuffer&);
};

#endif // SkTextBlobPriv_DEFINED
