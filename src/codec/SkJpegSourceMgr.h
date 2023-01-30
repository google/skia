/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegSourceMgr_codec_DEFINED
#define SkJpegSourceMgr_codec_DEFINED

#include <cstddef>
#include <cstdint>
#include <memory>

class SkStream;

/*
 * Interface to adapt an SkStream to the jpeg_source_mgr interface. This interface has different
 * implementations for SkStreams with different capabilities.
 */
class SkJpegSourceMgr {
public:
    static std::unique_ptr<SkJpegSourceMgr> Make(SkStream* stream);
    virtual ~SkJpegSourceMgr() {}

    // Interface called by libjpeg via its jpeg_source_mgr interface.
    virtual void initSource(const uint8_t*& nextInputByte, size_t& bytesInBuffer) = 0;
    virtual bool fillInputBuffer(const uint8_t*& nextInputByte, size_t& bytesInBuffer) = 0;
    virtual bool skipInputBytes(size_t bytes,
                                const uint8_t*& nextInputByte,
                                size_t& bytesInBuffer) = 0;
};

#endif
