/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkExif_DEFINED
#define SkExif_DEFINED

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"

#include <cstdint>

class SkExifMetadata {
public:
    /*
     * Parse the metadata specified in |data|.
     */
    SkExifMetadata(const sk_sp<SkData> data);

    /*
     * If the image encoded origin is specified, populate |origin| and return true. Otherwise
     * return false.
     */
    bool getOrigin(SkEncodedOrigin* origin) const {
        if (fOriginPresent) *origin = fOriginValue;
        return fOriginPresent;
    }

private:
    // Helper functions and constants for parsing the data.
    static constexpr uint16_t kSubIFDOffsetTag = 0x8769;
    void parseIfd(uint32_t ifdOffset, bool littleEndian, bool isRoot);

    // The input data.
    const sk_sp<SkData> fData;

    // The origin property.
    static constexpr uint16_t kOriginTag = 0x112;
    bool fOriginPresent = false;
    SkEncodedOrigin fOriginValue = kTopLeft_SkEncodedOrigin;
};

#endif
