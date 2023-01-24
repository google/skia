/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegMultiPicture_codec_DEFINED
#define SkJpegMultiPicture_codec_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"

#include <cstdint>
#include <memory>
#include <vector>

class SkData;
class SkJpegSeekableScan;

/*
 * Parsed Jpeg Multi-Picture Format structure as specified in CIPA DC-x007-2009. An introduction to
 * the format can be found in Figure 1 (Basic MP File format data structure)  and Figure 6 (Internal
 * Structure of the MP Index IFD) in that document. This parsing will extract only the size and
 * offset parameters from the images in the Index Image File Directory.
 */
struct SkJpegMultiPictureParameters {
    // An individual image.
    struct Image {
        // The size of the image in bytes.
        uint32_t size;
        // The offset of the image in bytes. This offset is specified relative to the address of
        // the MP Endian field in the MP Header, unless the image is a First Individual Image, in
        // which case the value of the offest [sic] shall be NULL (from section 5.2.3.3).
        uint32_t dataOffset;
    };

    // The images listed in the Index Image File Directory.
    std::vector<Image> images;
};

/*
 * Parse Jpeg Multi-Picture Format parameters. The specified data should start with the MP Header.
 * Returns nullptr on error.
 */
std::unique_ptr<SkJpegMultiPictureParameters> SkJpegParseMultiPicture(
        const sk_sp<const SkData>& data);

/*
 * Create SkStreams for all MultiPicture images, given a SkJpegSeekableScan of the image. This will
 * return nullptr if there is not MultiPicture segment, or if the MultiPicture parameters fail to
 * parse.
 */
struct SkJpegMultiPictureStreams {
    // An individual image.
    struct Image {
        // An SkStream from which the image's data may be read. This is nullptr for the First
        // Individual Image and for any images which encounter errors (e.g, they are outside of
        // the range of the stream).
        std::unique_ptr<SkStream> stream;
    };

    // The images as listed in the Index Image File Directory.
    std::vector<Image> images;
};
std::unique_ptr<SkJpegMultiPictureStreams> SkJpegExtractMultiPictureStreams(
        SkJpegSeekableScan* scan);

#endif
