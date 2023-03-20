/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegMultiPicture_codec_DEFINED
#define SkJpegMultiPicture_codec_DEFINED

#include "include/core/SkRefCnt.h"

#include <cstdint>
#include <memory>
#include <vector>

class SkData;

/*
 * Parsed Jpeg Multi-Picture Format structure as specified in CIPA DC-x007-2009. An introduction to
 * the format can be found in Figure 1 (Basic MP File format data structure) and Figure 6 (Internal
 * Structure of the MP Index IFD) in that document. This parsing will extract only the size and
 * offset parameters from the images in the Index Image File Directory.
 */
struct SkJpegMultiPictureParameters {
    // An individual image.
    struct Image {
        // The size of the image in bytes.
        uint32_t size = 0;
        // The offset of the image in bytes. This offset is specified relative to the address of
        // the MP Endian field in the MP Header, unless the image is a First Individual Image, in
        // which case the value of the offest [sic] shall be NULL (from section 5.2.3.3).
        uint32_t dataOffset = 0;
    };

    // The images listed in the Index Image File Directory.
    std::vector<Image> images;

    /*
     * Parse Jpeg Multi-Picture Format parameters. The specified data should be APP2 segment
     * parameters, which, if they are MPF parameter, should start with the {'M', 'P', 'F', 0}
     * signature. Returns nullptr the parameters do not start with the MPF signature, or if there
     * is an error in parsing the parameters.
     */
    static std::unique_ptr<SkJpegMultiPictureParameters> Make(
            const sk_sp<const SkData>& segmentParameters);

    /*
     * Serialize Jpeg Multi-Picture Format parameters into a segment. This segment will start with
     * the {'M', 'P', 'F', 0} signature (it will not include the segment marker or parameter
     * length).
     */
    sk_sp<SkData> serialize() const;

    /*
     * Compute the absolute offset (from the start of the image) for the offset in the multi-picture
     * parameters, given the absolute offset of the MPF segment (the offset of the {0xFF, 0xE2}
     * marker from the start of the image.
     */
    static size_t GetAbsoluteOffset(uint32_t dataOffset, size_t mpSegmentOffset);
};

#endif
