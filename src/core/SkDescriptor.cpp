/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDescriptor.h"
#include "SkMaskFilter.h"
#include "SkOpts.h"
#include "SkPathEffect.h"
#include "SkWriteBuffer.h"

SkDescriptor::SkDescriptor(const SkScalerContextRec& rec,
                           const SkPathEffect* pe,
                           const SkMaskFilter* mf) : fHash(0), fRec(rec) {
    if (pe || mf) {
        SkBinaryWriteBuffer buf;
        if (pe) { buf.writeFlattenable(pe); }
        if (mf) { buf.writeFlattenable(mf); }

        const size_t len = buf.bytesWritten();
        SkAutoTMalloc<uint8_t> flat{len};
        buf.writeToMemory(flat.get());

        fHash = SkOpts::hash_fn(flat.get(), len, fHash);
    }
    fHash = SkOpts::hash_fn(&fRec, sizeof(fRec), fHash);
}

bool SkDescriptor::operator==(const SkDescriptor& other) const {
    return fHash == other.fHash
        && 0 == memcmp(&fRec, &other.fRec, sizeof(fRec));
}
