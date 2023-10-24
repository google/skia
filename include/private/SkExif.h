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
#include "include/private/base/SkAPI.h"

#include <cstdint>

class SK_API SkExifMetadata {
public:
    /*
     * Parse the metadata specified in |data|.
     */
    SkExifMetadata(const sk_sp<SkData> data);

    /*
     * If the image encoded origin is specified, populate |out| and return true. Otherwise return
     * false.
     */
    bool getOrigin(SkEncodedOrigin* out) const {
        if (fOriginPresent && out) *out = fOriginValue;
        return fOriginPresent;
    }

    /*
     * If the HDR headroom is specified, populate |out| and return true. Otherwise return false.
     */
    bool getHdrHeadroom(float* out) const {
        if (fHdrHeadroomPresent && out) *out = fHdrHeadroomValue;
        return fHdrHeadroomPresent;
    }

    /*
     * If resolution unit, x, or y is specified, populate |out| and return true. Otherwise return
     * false.
     */
    bool getResolutionUnit(uint16_t* out) const {
        if (fResolutionUnitPresent && out) *out = fResolutionUnitValue;
        return fResolutionUnitPresent;
    }
    bool getXResolution(float* out) const {
        if (fXResolutionPresent && out) *out = fXResolutionValue;
        return fXResolutionPresent;
    }
    bool getYResolution(float* out) const {
        if (fYResolutionPresent && out) *out = fYResolutionValue;
        return fYResolutionPresent;
    }

    /*
     * If pixel dimension x or y is specified, populate |out| and return true. Otherwise return
     * false.
     */
    bool getPixelXDimension(uint32_t* out) const {
        if (fPixelXDimensionPresent && out) *out = fPixelXDimensionValue;
        return fPixelXDimensionPresent;
    }
    bool getPixelYDimension(uint32_t* out) const {
        if (fPixelYDimensionPresent && out) *out = fPixelYDimensionValue;
        return fPixelYDimensionPresent;
    }

private:
    // Helper functions and constants for parsing the data.
    void parseIfd(uint32_t ifdOffset, bool littleEndian, bool isRoot);

    // The input data.
    const sk_sp<SkData> fData;

    // The origin property.
    bool fOriginPresent = false;
    SkEncodedOrigin fOriginValue = kTopLeft_SkEncodedOrigin;

    // The HDR headroom property.
    bool fHdrHeadroomPresent = false;
    float fHdrHeadroomValue = 1.f;

    // Resolution.
    bool fResolutionUnitPresent = false;
    uint16_t fResolutionUnitValue = 0;
    bool fXResolutionPresent = false;
    float fXResolutionValue = 0;
    bool fYResolutionPresent = false;
    float fYResolutionValue = 0;

    // Size in pixels.
    bool fPixelXDimensionPresent = false;
    uint32_t fPixelXDimensionValue = 0;
    bool fPixelYDimensionPresent = false;
    uint32_t fPixelYDimensionValue = 0;
};

#endif
