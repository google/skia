// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkTextBlobTrace_DEFINED
#define SkTextBlobTrace_DEFINED

#include "include/core/SkTypes.h"

#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkStream.h"
#include "include/core/SkTextBlob.h"
#include "src/core/SkGlyphRun.h"
#include "src/core/SkWriteBuffer.h"

#include <cstdint>
#include <vector>

namespace SkTextBlobTrace {

struct Record {
    uint32_t origUniqueID;
    SkPaint paint;
    SkPoint offset;
    sk_sp<SkTextBlob> blob;
};

std::vector<SkTextBlobTrace::Record> CreateBlobTrace(SkStream* stream);

void DumpTrace(const std::vector<SkTextBlobTrace::Record>&);

class Capture {
public:
    Capture();
    ~Capture();
    void capture(const SkGlyphRunList&);
    // If `dst` is nullptr, write to a file.
    void dump(SkWStream* dst = nullptr) const;

private:
    size_t fBlobCount = 0;
    sk_sp<SkRefCntSet> fTypefaceSet;
    SkBinaryWriteBuffer fWriteBuffer;

    Capture(const Capture&) = delete;
    Capture& operator=(const Capture&) = delete;
};

}  // namespace
#endif  // SkTextBlobTrace_DEFINED
