/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFrontBufferedStream_DEFINED
#define SkFrontBufferedStream_DEFINED

#include "FrontBufferedStream.h"

// Temporary pass through until Android updates to use the new API.
class SK_API SkFrontBufferedStream {
public:
    static std::unique_ptr<SkStreamRewindable> Make(std::unique_ptr<SkStream> stream,
                                                    size_t minBufferSize) {
        return android::skia::FrontBufferedStream::Make(std::move(stream), minBufferSize);
    }
};
#endif  // SkFrontBufferedStream_DEFINED
