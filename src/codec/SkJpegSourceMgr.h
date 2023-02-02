/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegSourceMgr_codec_DEFINED
#define SkJpegSourceMgr_codec_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
#include "include/core/SkRefCnt.h"
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

#include <cstddef>
#include <cstdint>
#include <memory>

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
#include <vector>
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

class SkStream;

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
class SkData;
class SkJpegSegmentScanner;
struct SkJpegSegment;
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

/*
 * Interface to adapt an SkStream to the jpeg_source_mgr interface. This interface has different
 * implementations for SkStreams with different capabilities.
 */
class SkJpegSourceMgr {
public:
    // Create a source manager. If the source manager will buffer data, |bufferSize| specifies
    // the size of that buffer.
    static std::unique_ptr<SkJpegSourceMgr> Make(SkStream* stream, size_t bufferSize = 1024);
    virtual ~SkJpegSourceMgr();

    // Interface called by libjpeg via its jpeg_source_mgr interface.
    virtual void initSource(const uint8_t*& nextInputByte, size_t& bytesInBuffer) = 0;
    virtual bool fillInputBuffer(const uint8_t*& nextInputByte, size_t& bytesInBuffer) = 0;
    virtual bool skipInputBytes(size_t bytes,
                                const uint8_t*& nextInputByte,
                                size_t& bytesInBuffer) = 0;

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    // Parse this stream all the way through its EndOfImage marker and return the list of segments.
    // Return false if there is an error or if no EndOfImage marker is found.
    virtual const std::vector<SkJpegSegment>& getAllSegments() = 0;

    // Return a stream for the subset of this source's stream with the specified offset and size.
    virtual std::unique_ptr<SkStream> getSubsetStream(size_t offset, size_t size) = 0;

    // Copy the parameters from a segment. Return nullptr if the initial bytes of the parameters
    // section do not match the specified signature. Return the parameters starting from end of the
    // signature (so, kParameterLengthSize + signatureLength bytes into the parameter data). This
    // function only works for streams that are memory-backed.
    sk_sp<SkData> copyParameters(const SkJpegSegment& segment,
                                 const void* signature,
                                 const size_t signatureLength);
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

protected:
    SkJpegSourceMgr(SkStream* stream);
    SkStream* const fStream;  // unowned

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    // The segment scanner is lazily creatd only when needed.
    std::unique_ptr<SkJpegSegmentScanner> fScanner;
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS
};

#endif
