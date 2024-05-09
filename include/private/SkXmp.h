/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXmp_DEFINED
#define SkXmp_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class SkData;
struct SkGainmapInfo;

#include <cstddef>
#include <memory>

/*
 * An interface to extract information from XMP metadata.
 */
class SK_API SkXmp {
public:
    SkXmp() = default;
    virtual ~SkXmp() = default;
    // Make noncopyable
    SkXmp(const SkXmp&) = delete;
    SkXmp& operator= (const SkXmp&) = delete;

    // Create from XMP data.
    static std::unique_ptr<SkXmp> Make(sk_sp<SkData> xmpData);
    // Create from standard XMP + extended XMP data, see XMP Specification Part 3: Storage in files,
    // Section 1.1.3.1: Extended XMP in JPEG
    static std::unique_ptr<SkXmp> Make(sk_sp<SkData> xmpStandard, sk_sp<SkData> xmpExtended);

    // Extract HDRGM gainmap parameters.
    // TODO(b/338342146): Remove this once all callers are removed.
    bool getGainmapInfoHDRGM(SkGainmapInfo* info) const { return getGainmapInfoAdobe(info); }

    // Extract gainmap parameters from http://ns.adobe.com/hdr-gain-map/1.0/.
    virtual bool getGainmapInfoAdobe(SkGainmapInfo* info) const = 0;

    // If the image specifies http://ns.apple.com/pixeldatainfo/1.0/ AuxiliaryImageType of
    // urn:com:apple:photo:2020:aux:hdrgainmap, and includes a http://ns.apple.com/HDRGainMap/1.0/
    // HDRGainMapVersion, then populate |info| with gainmap parameters that will approximate the
    // math specified at [0] and return true.
    // [0] https://developer.apple.com/documentation/appkit/images_and_pdf/
    //     applying_apple_hdr_effect_to_your_photos
    virtual bool getGainmapInfoApple(float exifHdrHeadroom, SkGainmapInfo* info) const = 0;

    // If this includes GContainer metadata and the GContainer contains an item with semantic
    // GainMap and Mime of image/jpeg, then return true, and populate |offset| and |size| with
    // that item's offset (from the end of the primary JPEG image's EndOfImage), and the size of
    // the gainmap.
    virtual bool getContainerGainmapLocation(size_t* offset, size_t* size) const = 0;

    // Return the GUID of an Extended XMP if present, or null otherwise.
    virtual const char* getExtendedXmpGuid() const = 0;
};

#endif
