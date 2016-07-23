/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skdiff_DEFINED
#define skdiff_DEFINED

#include "SkBitmap.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkString.h"
#include "../private/SkTDArray.h"

#if defined(SK_BUILD_FOR_WIN32)
    #define PATH_DIV_STR "\\"
    #define PATH_DIV_CHAR '\\'
#else
    #define PATH_DIV_STR "/"
    #define PATH_DIV_CHAR '/'
#endif

#define MAX2(a,b) (((b) < (a)) ? (a) : (b))
#define MAX3(a,b,c) (((b) < (a)) ? MAX2((a), (c)) : MAX2((b), (c)))


struct DiffResource {
    enum Status {
        /** The resource was specified, exists, read, and decoded. */
        kDecoded_Status,
        /** The resource was specified, exists, read, but could not be decoded. */
        kCouldNotDecode_Status,

        /** The resource was specified, exists, and read. */
        kRead_Status,
        /** The resource was specified, exists, but could not be read. */
        kCouldNotRead_Status,

        /** The resource was specified and exists. */
        kExists_Status,
        /** The resource was specified, but does not exist. */
        kDoesNotExist_Status,

        /** The resource was specified. */
        kSpecified_Status,
        /** The resource was not specified. */
        kUnspecified_Status,

        /** Nothing is yet known about the resource. */
        kUnknown_Status,

        /** NOT A VALID VALUE -- used to set up arrays and to represent an unknown value. */
        kStatusCount
    };
    static char const * const StatusNames[DiffResource::kStatusCount];

    /** Returns the Status with this name.
     *  If there is no Status with this name, returns kStatusCount.
     */
    static Status getStatusByName(const char *name);

    /** Returns a text description of the given Status type. */
    static const char *getStatusDescription(Status status);

    /** Returns true if the Status indicates some kind of failure. */
    static bool isStatusFailed(Status status);

    /** Sets statuses[i] if it is implied by selector, unsets it if not.
     *  Selector may be a comma delimited list of status names, "any", or "failed".
     *  Returns true if the selector was entirely understood, false otherwise.
     */
    static bool getMatchingStatuses(char* selector, bool statuses[kStatusCount]);

    DiffResource() : fFilename(), fFullPath(), fBitmap(), fStatus(kUnknown_Status) { };

    /** If isEmpty() indicates no filename available. */
    SkString fFilename;
    /** If isEmpty() indicates no path available. */
    SkString fFullPath;
    /** If empty() indicates the bitmap could not be created. */
    SkBitmap fBitmap;
    Status fStatus;
};

struct DiffRecord {

    // Result of comparison for each pair of files.
    // Listed from "better" to "worse", for sorting of results.
    enum Result {
        kEqualBits_Result,
        kEqualPixels_Result,
        kDifferentPixels_Result,
        kDifferentSizes_Result,
        kCouldNotCompare_Result,
        kUnknown_Result,

        kResultCount  // NOT A VALID VALUE--used to set up arrays. Must be last.
    };
    static char const * const ResultNames[DiffRecord::kResultCount];

    /** Returns the Result with this name.
     *  If there is no Result with this name, returns kResultCount.
     */
    static Result getResultByName(const char *name);

    /** Returns a text description of the given Result type. */
    static const char *getResultDescription(Result result);

    DiffRecord()
        : fBase()
        , fComparison()
        , fDifference()
        , fWhite()
        , fFractionDifference(0)
        , fWeightedFraction(0)
        , fAverageMismatchA(0)
        , fAverageMismatchR(0)
        , fAverageMismatchG(0)
        , fAverageMismatchB(0)
        , fTotalMismatchA(0)
        , fMaxMismatchA(0)
        , fMaxMismatchR(0)
        , fMaxMismatchG(0)
        , fMaxMismatchB(0)
        , fResult(kUnknown_Result) {
    };

    DiffResource fBase;
    DiffResource fComparison;
    DiffResource fDifference;
    DiffResource fWhite;

    /// Arbitrary floating-point metric to be used to sort images from most
    /// to least different from baseline; values of 0 will be omitted from the
    /// summary webpage.
    float fFractionDifference;
    float fWeightedFraction;

    float fAverageMismatchA;
    float fAverageMismatchR;
    float fAverageMismatchG;
    float fAverageMismatchB;

    uint32_t fTotalMismatchA;

    uint32_t fMaxMismatchA;
    uint32_t fMaxMismatchR;
    uint32_t fMaxMismatchG;
    uint32_t fMaxMismatchB;

    /// Which category of diff result.
    Result fResult;
};

typedef SkTDArray<DiffRecord*> RecordArray;

/// A wrapper for any sortProc (comparison routine) which applies a first-order
/// sort beforehand, and a tiebreaker if the sortProc returns 0.
template<typename T> static int compare(const void* untyped_lhs, const void* untyped_rhs) {
    const DiffRecord* lhs = *reinterpret_cast<DiffRecord* const *>(untyped_lhs);
    const DiffRecord* rhs = *reinterpret_cast<DiffRecord* const *>(untyped_rhs);

    // First-order sort... these comparisons should be applied before comparing
    // pixel values, no matter what.
    if (lhs->fResult != rhs->fResult) {
        return (lhs->fResult < rhs->fResult) ? 1 : -1;
    }

    // Passed first-order sort, so call the pixel comparison routine.
    int result = T::comparePixels(lhs, rhs);
    if (result != 0) {
        return result;
    }

    // Tiebreaker... if we got to this point, we don't really care
    // which order they are sorted in, but let's at least be consistent.
    return strcmp(lhs->fBase.fFilename.c_str(), rhs->fBase.fFilename.c_str());
}

/// Comparison routine for qsort; sorts by fFractionDifference
/// from largest to smallest.
class CompareDiffMetrics {
public:
    static int comparePixels(const DiffRecord* lhs, const DiffRecord* rhs) {
        if (lhs->fFractionDifference < rhs->fFractionDifference) {
          return 1;
        }
        if (rhs->fFractionDifference < lhs->fFractionDifference) {
          return -1;
        }
        return 0;
    }
};

class CompareDiffWeighted {
public:
    static int comparePixels(const DiffRecord* lhs, const DiffRecord* rhs) {
        if (lhs->fWeightedFraction < rhs->fWeightedFraction) {
            return 1;
        }
        if (lhs->fWeightedFraction > rhs->fWeightedFraction) {
            return -1;
        }
        return 0;
    }
};

/// Comparison routine for qsort;  sorts by max(fAverageMismatch{RGB})
/// from largest to smallest.
class CompareDiffMeanMismatches {
public:
    static int comparePixels(const DiffRecord* lhs, const DiffRecord* rhs) {
        float leftValue = MAX3(lhs->fAverageMismatchR,
                               lhs->fAverageMismatchG,
                               lhs->fAverageMismatchB);
        float rightValue = MAX3(rhs->fAverageMismatchR,
                                rhs->fAverageMismatchG,
                                rhs->fAverageMismatchB);
        if (leftValue < rightValue) {
            return 1;
        }
        if (rightValue < leftValue) {
            return -1;
        }
        return 0;
    }
};

/// Comparison routine for qsort;  sorts by max(fMaxMismatch{RGB})
/// from largest to smallest.
class CompareDiffMaxMismatches {
public:
    static int comparePixels(const DiffRecord* lhs, const DiffRecord* rhs) {
        uint32_t leftValue = MAX3(lhs->fMaxMismatchR,
                                  lhs->fMaxMismatchG,
                                  lhs->fMaxMismatchB);
        uint32_t rightValue = MAX3(rhs->fMaxMismatchR,
                                   rhs->fMaxMismatchG,
                                   rhs->fMaxMismatchB);
        if (leftValue < rightValue) {
            return 1;
        }
        if (rightValue < leftValue) {
            return -1;
        }

        return CompareDiffMeanMismatches::comparePixels(lhs, rhs);
    }
};


/// Parameterized routine to compute the color of a pixel in a difference image.
typedef SkPMColor (*DiffMetricProc)(SkPMColor, SkPMColor);

// from gm
static inline SkPMColor compute_diff_pmcolor(SkPMColor c0, SkPMColor c1) {
    int dr = SkGetPackedR32(c0) - SkGetPackedR32(c1);
    int dg = SkGetPackedG32(c0) - SkGetPackedG32(c1);
    int db = SkGetPackedB32(c0) - SkGetPackedB32(c1);

    return SkPackARGB32(0xFF, SkAbs32(dr), SkAbs32(dg), SkAbs32(db));
}

/** When finished, dr->fResult should have some value other than kUnknown_Result.
 *  Expects dr->fWhite.fBitmap and dr->fDifference.fBitmap to have the same bounds as
 *  dr->fBase.fBitmap and have a valid pixelref.
 */
void compute_diff(DiffRecord* dr, DiffMetricProc diffFunction, const int colorThreshold);

#endif
