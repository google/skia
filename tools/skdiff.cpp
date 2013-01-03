/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "skdiff.h"
#include "SkBitmap.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkTypes.h"

/*static*/ char const * const DiffRecord::ResultNames[DiffRecord::kResultCount] = {
    "EqualBits",
    "EqualPixels",
    "DifferentPixels",
    "DifferentSizes",
    "CouldNotCompare",
    "Unknown",
};

DiffRecord::Result DiffRecord::getResultByName(const char *name) {
    for (int result = 0; result < DiffRecord::kResultCount; ++result) {
        if (0 == strcmp(DiffRecord::ResultNames[result], name)) {
            return static_cast<DiffRecord::Result>(result);
        }
    }
    return DiffRecord::kResultCount;
}

static char const * const ResultDescriptions[DiffRecord::kResultCount] = {
    "contain exactly the same bits",
    "contain the same pixel values, but not the same bits",
    "have identical dimensions but some differing pixels",
    "have differing dimensions",
    "could not be compared",
    "not compared yet",
};

const char* DiffRecord::getResultDescription(DiffRecord::Result result) {
    return ResultDescriptions[result];
}

/*static*/ char const * const DiffResource::StatusNames[DiffResource::kStatusCount] = {
    "Decoded",
    "CouldNotDecode",

    "Read",
    "CouldNotRead",

    "Exists",
    "DoesNotExist",

    "Specified",
    "Unspecified",

    "Unknown",
};

DiffResource::Status DiffResource::getStatusByName(const char *name) {
    for (int status = 0; status < DiffResource::kStatusCount; ++status) {
        if (0 == strcmp(DiffResource::StatusNames[status], name)) {
            return static_cast<DiffResource::Status>(status);
        }
    }
    return DiffResource::kStatusCount;
}

static char const * const StatusDescriptions[DiffResource::kStatusCount] = {
    "decoded",
    "could not be decoded",

    "read",
    "could not be read",

    "found",
    "not found",

    "specified",
    "unspecified",

    "unknown",
};

const char* DiffResource::getStatusDescription(DiffResource::Status status) {
    return StatusDescriptions[status];
}

bool DiffResource::isStatusFailed(DiffResource::Status status) {
    return DiffResource::kCouldNotDecode_Status == status ||
           DiffResource::kCouldNotRead_Status == status ||
           DiffResource::kDoesNotExist_Status == status ||
           DiffResource::kUnspecified_Status == status ||
           DiffResource::kUnknown_Status == status;
}

bool DiffResource::getMatchingStatuses(char* selector, bool statuses[kStatusCount]) {
    if (!strcmp(selector, "any")) {
        for (int statusIndex = 0; statusIndex < kStatusCount; ++statusIndex) {
            statuses[statusIndex] = true;
        }
        return true;
    }

    for (int statusIndex = 0; statusIndex < kStatusCount; ++statusIndex) {
        statuses[statusIndex] = false;
    }

    static const char kDelimiterChar = ',';
    bool understood = true;
    while (true) {
        char* delimiterPtr = strchr(selector, kDelimiterChar);

        if (delimiterPtr) {
            *delimiterPtr = '\0';
        }

        if (!strcmp(selector, "failed")) {
            for (int statusIndex = 0; statusIndex < kStatusCount; ++statusIndex) {
                Status status = static_cast<Status>(statusIndex);
                statuses[statusIndex] |= isStatusFailed(status);
            }
        } else {
            Status status = getStatusByName(selector);
            if (status == kStatusCount) {
                understood = false;
            } else {
                statuses[status] = true;
            }
        }

        if (!delimiterPtr) {
            break;
        }

        *delimiterPtr = kDelimiterChar;
        selector = delimiterPtr + 1;
    }
    return understood;
}

static inline bool colors_match_thresholded(SkPMColor c0, SkPMColor c1, const int threshold) {
    int da = SkGetPackedA32(c0) - SkGetPackedA32(c1);
    int dr = SkGetPackedR32(c0) - SkGetPackedR32(c1);
    int dg = SkGetPackedG32(c0) - SkGetPackedG32(c1);
    int db = SkGetPackedB32(c0) - SkGetPackedB32(c1);

    return ((SkAbs32(da) <= threshold) &&
            (SkAbs32(dr) <= threshold) &&
            (SkAbs32(dg) <= threshold) &&
            (SkAbs32(db) <= threshold));
}

const SkPMColor PMCOLOR_WHITE = SkPreMultiplyColor(SK_ColorWHITE);
const SkPMColor PMCOLOR_BLACK = SkPreMultiplyColor(SK_ColorBLACK);

void compute_diff(DiffRecord* dr, DiffMetricProc diffFunction, const int colorThreshold) {
    const int w = dr->fComparison.fBitmap.width();
    const int h = dr->fComparison.fBitmap.height();
    if (w != dr->fBase.fBitmap.width() || h != dr->fBase.fBitmap.height()) {
        dr->fResult = DiffRecord::kDifferentSizes_Result;
        return;
    }

    SkAutoLockPixels alpDiff(dr->fDifference.fBitmap);
    SkAutoLockPixels alpWhite(dr->fWhite.fBitmap);
    int mismatchedPixels = 0;
    int totalMismatchA = 0;
    int totalMismatchR = 0;
    int totalMismatchG = 0;
    int totalMismatchB = 0;

    // Accumulate fractionally different pixels, then divide out
    // # of pixels at the end.
    dr->fWeightedFraction = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            SkPMColor c0 = *dr->fBase.fBitmap.getAddr32(x, y);
            SkPMColor c1 = *dr->fComparison.fBitmap.getAddr32(x, y);
            SkPMColor outputDifference = diffFunction(c0, c1);
            uint32_t thisA = SkAbs32(SkGetPackedA32(c0) - SkGetPackedA32(c1));
            uint32_t thisR = SkAbs32(SkGetPackedR32(c0) - SkGetPackedR32(c1));
            uint32_t thisG = SkAbs32(SkGetPackedG32(c0) - SkGetPackedG32(c1));
            uint32_t thisB = SkAbs32(SkGetPackedB32(c0) - SkGetPackedB32(c1));
            totalMismatchA += thisA;
            totalMismatchR += thisR;
            totalMismatchG += thisG;
            totalMismatchB += thisB;
            // In HSV, value is defined as max RGB component.
            int value = MAX3(thisR, thisG, thisB);
            dr->fWeightedFraction += ((float) value) / 255;
            if (thisA > dr->fMaxMismatchA) {
                dr->fMaxMismatchA = thisA;
            }
            if (thisR > dr->fMaxMismatchR) {
                dr->fMaxMismatchR = thisR;
            }
            if (thisG > dr->fMaxMismatchG) {
                dr->fMaxMismatchG = thisG;
            }
            if (thisB > dr->fMaxMismatchB) {
                dr->fMaxMismatchB = thisB;
            }
            if (!colors_match_thresholded(c0, c1, colorThreshold)) {
                mismatchedPixels++;
                *dr->fDifference.fBitmap.getAddr32(x, y) = outputDifference;
                *dr->fWhite.fBitmap.getAddr32(x, y) = PMCOLOR_WHITE;
            } else {
                *dr->fDifference.fBitmap.getAddr32(x, y) = 0;
                *dr->fWhite.fBitmap.getAddr32(x, y) = PMCOLOR_BLACK;
            }
        }
    }
    if (0 == mismatchedPixels) {
        dr->fResult = DiffRecord::kEqualPixels_Result;
        return;
    }
    dr->fResult = DiffRecord::kDifferentPixels_Result;
    int pixelCount = w * h;
    dr->fFractionDifference = ((float) mismatchedPixels) / pixelCount;
    dr->fWeightedFraction /= pixelCount;
    dr->fTotalMismatchA = totalMismatchA;
    dr->fAverageMismatchA = ((float) totalMismatchA) / pixelCount;
    dr->fAverageMismatchR = ((float) totalMismatchR) / pixelCount;
    dr->fAverageMismatchG = ((float) totalMismatchG) / pixelCount;
    dr->fAverageMismatchB = ((float) totalMismatchB) / pixelCount;
}
