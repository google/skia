
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkColorPriv.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "SkTemplates.h"
#include "SkTime.h"
#include "SkTSearch.h"
#include "SkTypes.h"

/**
 * skdiff
 *
 * Given three directory names, expects to find identically-named files in
 * each of the first two; the first are treated as a set of baseline,
 * the second a set of variant images, and a diff image is written into the
 * third directory for each pair.
 * Creates an index.html in the current third directory to compare each
 * pair that does not match exactly.
 * Recursively descends directories, unless run with --norecurse.
 *
 * Returns zero exit code if all images match across baseDir and comparisonDir.
 */

#if SK_BUILD_FOR_WIN32
    #define PATH_DIV_STR "\\"
    #define PATH_DIV_CHAR '\\'
#else
    #define PATH_DIV_STR "/"
    #define PATH_DIV_CHAR '/'
#endif

// Result of comparison for each pair of files.
// Listed from "better" to "worse", for sorting of results.
enum Result {
    kEqualBits,
    kEqualPixels,
    kDifferentPixels,
    kDifferentSizes,
    kDifferentOther,
    kComparisonMissing,
    kBaseMissing,
    kUnknown,
    //
    kNumResultTypes  // NOT A VALID VALUE--used to set up arrays. Must be last.
};

// Returns the Result with this name.
// If there is no Result with this name, returns kNumResultTypes.
// TODO: Is there a better return value for the fall-through case?
static Result getResultByName(const char *name) {
    if (0 == strcmp("EqualBits", name)) {
        return kEqualBits;
    }
    if (0 == strcmp("EqualPixels", name)) {
        return kEqualPixels;
    }
    if (0 == strcmp("DifferentPixels", name)) {
        return kDifferentPixels;
    }
    if (0 == strcmp("DifferentSizes", name)) {
        return kDifferentSizes;
    }
    if (0 == strcmp("DifferentOther", name)) {
        return kDifferentOther;
    }
    if (0 == strcmp("ComparisonMissing", name)) {
        return kComparisonMissing;
    }
    if (0 == strcmp("BaseMissing", name)) {
        return kBaseMissing;
    }
    if (0 == strcmp("Unknown", name)) {
        return kUnknown;
    }
    return kNumResultTypes;
}

// Returns a text description of the given Result type.
static const char *getResultDescription(Result result) {
    switch (result) {
      case kEqualBits:
        return "contain exactly the same bits";
      case kEqualPixels:
        return "contain the same pixel values, but not the same bits";
      case kDifferentPixels:
        return "have identical dimensions but some differing pixels";
      case kDifferentSizes:
        return "have differing dimensions";
      case kDifferentOther:
        return "contain different bits and are not parsable images";
      case kBaseMissing:
        return "missing from baseDir";
      case kComparisonMissing:
        return "missing from comparisonDir";
      case kUnknown:
        return "not compared yet";
      default:
        return NULL;
    }
}

struct DiffRecord {
    DiffRecord (const SkString filename,
                const SkString basePath,
                const SkString comparisonPath,
                const Result result = kUnknown)
        : fFilename (filename)
        , fBasePath (basePath)
        , fComparisonPath (comparisonPath)
        , fBaseBitmap (new SkBitmap ())
        , fComparisonBitmap (new SkBitmap ())
        , fDifferenceBitmap (new SkBitmap ())
        , fWhiteBitmap (new SkBitmap ())
        , fBaseHeight (0)
        , fBaseWidth (0)
        , fFractionDifference (0)
        , fWeightedFraction (0)
        , fAverageMismatchR (0)
        , fAverageMismatchG (0)
        , fAverageMismatchB (0)
        , fMaxMismatchR (0)
        , fMaxMismatchG (0)
        , fMaxMismatchB (0)
        , fResult(result) {
    };

    SkString fFilename;
    SkString fBasePath;
    SkString fComparisonPath;

    SkBitmap* fBaseBitmap;
    SkBitmap* fComparisonBitmap;
    SkBitmap* fDifferenceBitmap;
    SkBitmap* fWhiteBitmap;

    int fBaseHeight;
    int fBaseWidth;

    /// Arbitrary floating-point metric to be used to sort images from most
    /// to least different from baseline; values of 0 will be omitted from the
    /// summary webpage.
    float fFractionDifference;
    float fWeightedFraction;

    float fAverageMismatchR;
    float fAverageMismatchG;
    float fAverageMismatchB;

    uint32_t fMaxMismatchR;
    uint32_t fMaxMismatchG;
    uint32_t fMaxMismatchB;

    /// Which category of diff result.
    Result fResult;
};

#define MAX2(a,b) (((b) < (a)) ? (a) : (b))
#define MAX3(a,b,c) (((b) < (a)) ? MAX2((a), (c)) : MAX2((b), (c)))

const SkPMColor PMCOLOR_WHITE = SkPreMultiplyColor(SK_ColorWHITE);
const SkPMColor PMCOLOR_BLACK = SkPreMultiplyColor(SK_ColorBLACK);

typedef SkTDArray<SkString*> StringArray;
typedef StringArray FileArray;

struct DiffSummary {
    DiffSummary ()
        : fNumMatches (0)
        , fNumMismatches (0)
        , fMaxMismatchV (0)
        , fMaxMismatchPercent (0) { };

    ~DiffSummary() {
        for (int i = 0; i < kNumResultTypes; i++) {
            fResultsOfType[i].deleteAll();
        }
    }

    uint32_t fNumMatches;
    uint32_t fNumMismatches;
    uint32_t fMaxMismatchV;
    float fMaxMismatchPercent;

    FileArray fResultsOfType[kNumResultTypes];

    // Print a line about the contents of this FileArray to stdout.
    void printContents(const FileArray& fileArray, const char* headerText, bool listFilenames) {
        int n = fileArray.count();
        printf("%d file pairs %s", n, headerText);
        if (listFilenames) {
            printf(": ");
            for (int i = 0; i < n; ++i) {
                printf("%s ", fileArray[i]->c_str());
            }
        }
        printf("\n");
    }

    void print(bool listFilenames, bool failOnResultType[kNumResultTypes]) {
        printf("\ncompared %d file pairs:\n", fNumMatches + fNumMismatches);
        for (int resultInt = 0; resultInt < kNumResultTypes; resultInt++) {
            Result result = static_cast<Result>(resultInt);
            if (failOnResultType[result]) {
                printf("[*] ");
            } else {
                printf("[_] ");
            }
            printContents(fResultsOfType[result], getResultDescription(result), listFilenames);
        }
        printf("(results marked with [*] will cause nonzero return value)\n");
        printf("\nnumber of mismatching file pairs: %d\n", fNumMismatches);
        if (fNumMismatches > 0) {
            printf("Maximum pixel intensity mismatch %d\n", fMaxMismatchV);
            printf("Largest area mismatch was %.2f%% of pixels\n",fMaxMismatchPercent);
        }
    }

    void add (DiffRecord* drp) {
        uint32_t mismatchValue;

        fResultsOfType[drp->fResult].push(new SkString(drp->fFilename));
        switch (drp->fResult) {
          case kEqualBits:
            fNumMatches++;
            break;
          case kEqualPixels:
            fNumMatches++;
            break;
          case kDifferentSizes:
            fNumMismatches++;
            break;
          case kDifferentPixels:
            fNumMismatches++;
            if (drp->fFractionDifference * 100 > fMaxMismatchPercent) {
                fMaxMismatchPercent = drp->fFractionDifference * 100;
            }
            mismatchValue = MAX3(drp->fMaxMismatchR, drp->fMaxMismatchG,
                                 drp->fMaxMismatchB);
            if (mismatchValue > fMaxMismatchV) {
                fMaxMismatchV = mismatchValue;
            }
            break;
          case kDifferentOther:
            fNumMismatches++;
            break;
          case kBaseMissing:
            fNumMismatches++;
            break;
          case kComparisonMissing:
            fNumMismatches++;
            break;
          case kUnknown:
            SkDEBUGFAIL("adding uncategorized DiffRecord");
            break;
          default:
            SkDEBUGFAIL("adding DiffRecord with unhandled fResult value");
            break;
        }
    }
};

typedef SkTDArray<DiffRecord*> RecordArray;

/// A wrapper for any sortProc (comparison routine) which applies a first-order
/// sort beforehand, and a tiebreaker if the sortProc returns 0.
template<typename T>
static int compare(const void* untyped_lhs, const void* untyped_rhs) {
    const DiffRecord* lhs = *reinterpret_cast<DiffRecord* const*>(untyped_lhs);
    const DiffRecord* rhs = *reinterpret_cast<DiffRecord* const*>(untyped_rhs);

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
    return strcmp(lhs->fFilename.c_str(), rhs->fFilename.c_str());
}

/// Comparison routine for qsort;  sorts by fFractionDifference
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

#if 0 // UNUSED
static void expand_and_copy (int width, int height, SkBitmap** dest) {
    SkBitmap* temp = new SkBitmap ();
    temp->reset();
    temp->setConfig((*dest)->config(), width, height);
    temp->allocPixels();
    (*dest)->copyPixelsTo(temp->getPixels(), temp->getSize(),
                          temp->rowBytes());
    *dest = temp;
}
#endif

/// Returns true if the two buffers passed in are both non-NULL, and include
/// exactly the same byte values (and identical lengths).
static bool are_buffers_equal(SkData* skdata1, SkData* skdata2) {
    if ((NULL == skdata1) || (NULL == skdata2)) {
        return false;
    }
    if (skdata1->size() != skdata2->size()) {
        return false;
    }
    return (0 == memcmp(skdata1->data(), skdata2->data(), skdata1->size()));
}

/// Reads the file at the given path and returns its complete contents as an
/// SkData object (or returns NULL on error).
static SkData* read_file(const char* file_path) {
    SkFILEStream fileStream(file_path);
    if (!fileStream.isValid()) {
        SkDebugf("WARNING: could not open file <%s> for reading\n", file_path);
        return NULL;
    }
    size_t bytesInFile = fileStream.getLength();
    size_t bytesLeftToRead = bytesInFile;

    void* bufferStart = sk_malloc_throw(bytesInFile);
    char* bufferPointer = (char*)bufferStart;
    while (bytesLeftToRead > 0) {
        size_t bytesReadThisTime = fileStream.read(
            bufferPointer, bytesLeftToRead);
        if (0 == bytesReadThisTime) {
            SkDebugf("WARNING: error reading from <%s>\n", file_path);
            sk_free(bufferStart);
            return NULL;
        }
        bytesLeftToRead -= bytesReadThisTime;
        bufferPointer += bytesReadThisTime;
    }
    return SkData::NewFromMalloc(bufferStart, bytesInFile);
}

/// Decodes binary contents of baseFile and comparisonFile into
/// diffRecord->fBaseBitmap and diffRecord->fComparisonBitmap.
/// Returns true if that succeeds.
static bool get_bitmaps (SkData* baseFileContents,
                         SkData* comparisonFileContents,
                         DiffRecord* diffRecord) {
    SkMemoryStream compareStream(comparisonFileContents->data(),
                                 comparisonFileContents->size());
    SkMemoryStream baseStream(baseFileContents->data(),
                              baseFileContents->size());

    SkImageDecoder* codec = SkImageDecoder::Factory(&baseStream);
    if (NULL == codec) {
        SkDebugf("ERROR: no codec found for basePath <%s>\n",
                 diffRecord->fBasePath.c_str());
        return false;
    }

    // In debug, the DLL will automatically be unloaded when this is deleted,
    // but that shouldn't be a problem in release mode.
    SkAutoTDelete<SkImageDecoder> ad(codec);

    baseStream.rewind();
    if (!codec->decode(&baseStream, diffRecord->fBaseBitmap,
                       SkBitmap::kARGB_8888_Config,
                       SkImageDecoder::kDecodePixels_Mode)) {
        SkDebugf("ERROR: codec failed for basePath <%s>\n",
                 diffRecord->fBasePath.c_str());
        return false;
    }

    diffRecord->fBaseWidth = diffRecord->fBaseBitmap->width();
    diffRecord->fBaseHeight = diffRecord->fBaseBitmap->height();

    if (!codec->decode(&compareStream, diffRecord->fComparisonBitmap,
                       SkBitmap::kARGB_8888_Config,
                       SkImageDecoder::kDecodePixels_Mode)) {
        SkDebugf("ERROR: codec failed for comparisonPath <%s>\n",
                 diffRecord->fComparisonPath.c_str());
        return false;
    }

    return true;
}

static bool get_bitmap_height_width(const SkString& path,
                                    int *height, int *width) {
    SkFILEStream stream(path.c_str());
    if (!stream.isValid()) {
        SkDebugf("ERROR: couldn't open file <%s>\n",
                 path.c_str());
        return false;
    }

    SkImageDecoder* codec = SkImageDecoder::Factory(&stream);
    if (NULL == codec) {
        SkDebugf("ERROR: no codec found for <%s>\n",
                 path.c_str());
        return false;
    }

    SkAutoTDelete<SkImageDecoder> ad(codec);
    SkBitmap bm;

    stream.rewind();
    if (!codec->decode(&stream, &bm,
                       SkBitmap::kARGB_8888_Config,
                       SkImageDecoder::kDecodePixels_Mode)) {
        SkDebugf("ERROR: codec failed for <%s>\n",
                 path.c_str());
        return false;
    }

    *height = bm.height();
    *width = bm.width();

    return true;
}

// from gm - thanks to PNG, we need to force all pixels 100% opaque
static void force_all_opaque(const SkBitmap& bitmap) {
   SkAutoLockPixels lock(bitmap);
   for (int y = 0; y < bitmap.height(); y++) {
       for (int x = 0; x < bitmap.width(); x++) {
           *bitmap.getAddr32(x, y) |= (SK_A32_MASK << SK_A32_SHIFT);
       }
   }
}

// from gm
static bool write_bitmap(const SkString& path, const SkBitmap* bitmap) {
    SkBitmap copy;
    bitmap->copyTo(&copy, SkBitmap::kARGB_8888_Config);
    force_all_opaque(copy);
    return SkImageEncoder::EncodeFile(path.c_str(), copy,
                                      SkImageEncoder::kPNG_Type, 100);
}

// from gm
static inline SkPMColor compute_diff_pmcolor(SkPMColor c0, SkPMColor c1) {
    int dr = SkGetPackedR32(c0) - SkGetPackedR32(c1);
    int dg = SkGetPackedG32(c0) - SkGetPackedG32(c1);
    int db = SkGetPackedB32(c0) - SkGetPackedB32(c1);

    return SkPackARGB32(0xFF, SkAbs32(dr), SkAbs32(dg), SkAbs32(db));
}

static inline bool colors_match_thresholded(SkPMColor c0, SkPMColor c1,
                                            const int threshold) {
    int da = SkGetPackedA32(c0) - SkGetPackedA32(c1);
    int dr = SkGetPackedR32(c0) - SkGetPackedR32(c1);
    int dg = SkGetPackedG32(c0) - SkGetPackedG32(c1);
    int db = SkGetPackedB32(c0) - SkGetPackedB32(c1);

    return ((SkAbs32(da) <= threshold) &&
            (SkAbs32(dr) <= threshold) &&
            (SkAbs32(dg) <= threshold) &&
            (SkAbs32(db) <= threshold));
}

// based on gm
// Postcondition: when we exit this method, dr->fResult should have some value
// other than kUnknown.
static void compute_diff(DiffRecord* dr,
                         DiffMetricProc diffFunction,
                         const int colorThreshold) {
    SkAutoLockPixels alpDiff(*dr->fDifferenceBitmap);
    SkAutoLockPixels alpWhite(*dr->fWhiteBitmap);

    const int w = dr->fComparisonBitmap->width();
    const int h = dr->fComparisonBitmap->height();
    int mismatchedPixels = 0;
    int totalMismatchR = 0;
    int totalMismatchG = 0;
    int totalMismatchB = 0;

    if (w != dr->fBaseWidth || h != dr->fBaseHeight) {
        dr->fResult = kDifferentSizes;
        return;
    }
    // Accumulate fractionally different pixels, then divide out
    // # of pixels at the end.
    dr->fWeightedFraction = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            SkPMColor c0 = *dr->fBaseBitmap->getAddr32(x, y);
            SkPMColor c1 = *dr->fComparisonBitmap->getAddr32(x, y);
            SkPMColor trueDifference = compute_diff_pmcolor(c0, c1);
            SkPMColor outputDifference = diffFunction(c0, c1);
            uint32_t thisR = SkGetPackedR32(trueDifference);
            uint32_t thisG = SkGetPackedG32(trueDifference);
            uint32_t thisB = SkGetPackedB32(trueDifference);
            totalMismatchR += thisR;
            totalMismatchG += thisG;
            totalMismatchB += thisB;
            // In HSV, value is defined as max RGB component.
            int value = MAX3(thisR, thisG, thisB);
            dr->fWeightedFraction += ((float) value) / 255;
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
                *dr->fDifferenceBitmap->getAddr32(x, y) = outputDifference;
                *dr->fWhiteBitmap->getAddr32(x, y) = PMCOLOR_WHITE;
            } else {
                *dr->fDifferenceBitmap->getAddr32(x, y) = 0;
                *dr->fWhiteBitmap->getAddr32(x, y) = PMCOLOR_BLACK;
            }
        }
    }
    if (0 == mismatchedPixels) {
        dr->fResult = kEqualPixels;
        return;
    }
    dr->fResult = kDifferentPixels;
    int pixelCount = w * h;
    dr->fFractionDifference = ((float) mismatchedPixels) / pixelCount;
    dr->fWeightedFraction /= pixelCount;
    dr->fAverageMismatchR = ((float) totalMismatchR) / pixelCount;
    dr->fAverageMismatchG = ((float) totalMismatchG) / pixelCount;
    dr->fAverageMismatchB = ((float) totalMismatchB) / pixelCount;
}

/// Return a copy of the "input" string, within which we have replaced all instances
/// of oldSubstring with newSubstring.
///
/// TODO: If we like this, we should move it into the core SkString implementation,
/// adding more checks and ample test cases, and paying more attention to efficiency.
static SkString replace_all(const SkString &input,
                            const char oldSubstring[], const char newSubstring[]) {
    SkString output;
    const char *input_cstr = input.c_str();
    const char *first_char = input_cstr;
    const char *match_char;
    int oldSubstringLen = strlen(oldSubstring);
    while (NULL != (match_char = strstr(first_char, oldSubstring))) {
        output.append(first_char, (match_char - first_char));
        output.append(newSubstring);
        first_char = match_char + oldSubstringLen;
    }
    output.append(first_char);
    return output;
}

static SkString filename_to_derived_filename (const SkString& filename,
                                              const char *suffix) {
    SkString diffName (filename);
    const char* cstring = diffName.c_str();
    int dotOffset = strrchr(cstring, '.') - cstring;
    diffName.remove(dotOffset, diffName.size() - dotOffset);
    diffName.append(suffix);

    // In case we recursed into subdirectories, replace slashes with something else
    // so the diffs will all be written into a single flat directory.
    diffName = replace_all(diffName, PATH_DIV_STR, "_");
    return diffName;
}

/// Given a image filename, returns the name of the file containing the
/// associated difference image.
static SkString filename_to_diff_filename (const SkString& filename) {
    return filename_to_derived_filename(filename, "-diff.png");
}

/// Given a image filename, returns the name of the file containing the
/// "white" difference image.
static SkString filename_to_white_filename (const SkString& filename) {
    return filename_to_derived_filename(filename, "-white.png");
}

static void release_bitmaps(DiffRecord* drp) {
    delete drp->fBaseBitmap;
    drp->fBaseBitmap = NULL;
    delete drp->fComparisonBitmap;
    drp->fComparisonBitmap = NULL;
    delete drp->fDifferenceBitmap;
    drp->fDifferenceBitmap = NULL;
    delete drp->fWhiteBitmap;
    drp->fWhiteBitmap = NULL;
}


/// If outputDir.isEmpty(), don't write out diff files.
static void create_and_write_diff_image(DiffRecord* drp,
                                        DiffMetricProc dmp,
                                        const int colorThreshold,
                                        const SkString& outputDir,
                                        const SkString& filename) {
    const int w = drp->fBaseWidth;
    const int h = drp->fBaseHeight;
    drp->fDifferenceBitmap->setConfig(SkBitmap::kARGB_8888_Config, w, h);
    drp->fDifferenceBitmap->allocPixels();
    drp->fWhiteBitmap->setConfig(SkBitmap::kARGB_8888_Config, w, h);
    drp->fWhiteBitmap->allocPixels();

    SkASSERT(kUnknown == drp->fResult);
    compute_diff(drp, dmp, colorThreshold);
    SkASSERT(kUnknown != drp->fResult);

    if ((kDifferentPixels == drp->fResult) && !outputDir.isEmpty()) {
        SkString differencePath (outputDir);
        differencePath.append(filename_to_diff_filename(filename));
        write_bitmap(differencePath, drp->fDifferenceBitmap);
        SkString whitePath (outputDir);
        whitePath.append(filename_to_white_filename(filename));
        write_bitmap(whitePath, drp->fWhiteBitmap);
    }

    release_bitmaps(drp);
}

/// Returns true if string contains any of these substrings.
static bool string_contains_any_of(const SkString& string,
                                   const StringArray& substrings) {
    for (int i = 0; i < substrings.count(); i++) {
        if (string.contains(substrings[i]->c_str())) {
            return true;
        }
    }
    return false;
}

/// Internal (potentially recursive) implementation of get_file_list.
static void get_file_list_subdir(const SkString& rootDir, const SkString& subDir,
                                 const StringArray& matchSubstrings,
                                 const StringArray& nomatchSubstrings,
                                 bool recurseIntoSubdirs, FileArray *files) {
    bool isSubDirEmpty = subDir.isEmpty();
    SkString dir(rootDir);
    if (!isSubDirEmpty) {
        dir.append(PATH_DIV_STR);
        dir.append(subDir);
    }

    // Iterate over files (not directories) within dir.
    SkOSFile::Iter fileIterator(dir.c_str());
    SkString fileName;
    while (fileIterator.next(&fileName, false)) {
        if (fileName.startsWith(".")) {
            continue;
        }
        SkString pathRelativeToRootDir(subDir);
        if (!isSubDirEmpty) {
            pathRelativeToRootDir.append(PATH_DIV_STR);
        }
        pathRelativeToRootDir.append(fileName);
        if (string_contains_any_of(pathRelativeToRootDir, matchSubstrings) &&
            !string_contains_any_of(pathRelativeToRootDir, nomatchSubstrings)) {
            files->push(new SkString(pathRelativeToRootDir));
        }
    }

    // Recurse into any non-ignored subdirectories.
    if (recurseIntoSubdirs) {
        SkOSFile::Iter dirIterator(dir.c_str());
        SkString dirName;
        while (dirIterator.next(&dirName, true)) {
            if (dirName.startsWith(".")) {
                continue;
            }
            SkString pathRelativeToRootDir(subDir);
            if (!isSubDirEmpty) {
                pathRelativeToRootDir.append(PATH_DIV_STR);
            }
            pathRelativeToRootDir.append(dirName);
            if (!string_contains_any_of(pathRelativeToRootDir, nomatchSubstrings)) {
                get_file_list_subdir(rootDir, pathRelativeToRootDir,
                                     matchSubstrings, nomatchSubstrings, recurseIntoSubdirs,
                                     files);
            }
        }
    }
}

/// Iterate over dir and get all files whose filename:
///  - matches any of the substrings in matchSubstrings, but...
///  - DOES NOT match any of the substrings in nomatchSubstrings
///  - DOES NOT start with a dot (.)
/// Adds the matching files to the list in *files.
static void get_file_list(const SkString& dir,
                          const StringArray& matchSubstrings,
                          const StringArray& nomatchSubstrings,
                          bool recurseIntoSubdirs, FileArray *files) {
    get_file_list_subdir(dir, SkString(""),
                         matchSubstrings, nomatchSubstrings, recurseIntoSubdirs,
                         files);
}

static void release_file_list(FileArray *files) {
    files->deleteAll();
}

/// Comparison routines for qsort, sort by file names.
static int compare_file_name_metrics(SkString **lhs, SkString **rhs) {
    return strcmp((*lhs)->c_str(), (*rhs)->c_str());
}

/// Creates difference images, returns the number that have a 0 metric.
/// If outputDir.isEmpty(), don't write out diff files.
static void create_diff_images (DiffMetricProc dmp,
                                const int colorThreshold,
                                RecordArray* differences,
                                const SkString& baseDir,
                                const SkString& comparisonDir,
                                const SkString& outputDir,
                                const StringArray& matchSubstrings,
                                const StringArray& nomatchSubstrings,
                                bool recurseIntoSubdirs,
                                DiffSummary* summary) {
    SkASSERT(!baseDir.isEmpty());
    SkASSERT(!comparisonDir.isEmpty());

    FileArray baseFiles;
    FileArray comparisonFiles;

    get_file_list(baseDir, matchSubstrings, nomatchSubstrings, recurseIntoSubdirs, &baseFiles);
    get_file_list(comparisonDir, matchSubstrings, nomatchSubstrings, recurseIntoSubdirs,
                  &comparisonFiles);

    if (!baseFiles.isEmpty()) {
        qsort(baseFiles.begin(), baseFiles.count(), sizeof(SkString*),
              SkCastForQSort(compare_file_name_metrics));
    }
    if (!comparisonFiles.isEmpty()) {
        qsort(comparisonFiles.begin(), comparisonFiles.count(),
              sizeof(SkString*), SkCastForQSort(compare_file_name_metrics));
    }

    int i = 0;
    int j = 0;

    while (i < baseFiles.count() &&
           j < comparisonFiles.count()) {

        SkString basePath (baseDir);
        basePath.append(*baseFiles[i]);
        SkString comparisonPath (comparisonDir);
        comparisonPath.append(*comparisonFiles[j]);

        DiffRecord *drp = NULL;
        int v = strcmp(baseFiles[i]->c_str(),
                       comparisonFiles[j]->c_str());

        if (v < 0) {
            // in baseDir, but not in comparisonDir
            drp = new DiffRecord(*baseFiles[i], basePath, comparisonPath,
                                 kComparisonMissing);
            ++i;
        } else if (v > 0) {
            // in comparisonDir, but not in baseDir
            drp = new DiffRecord(*comparisonFiles[j], basePath, comparisonPath,
                                 kBaseMissing);
            ++j;
        } else {
            // Found the same filename in both baseDir and comparisonDir.
            drp = new DiffRecord(*baseFiles[i], basePath, comparisonPath);
            SkASSERT(kUnknown == drp->fResult);

            SkData* baseFileBits = NULL;
            SkData* comparisonFileBits = NULL;
            if (NULL == (baseFileBits = read_file(basePath.c_str()))) {
                SkDebugf("WARNING: couldn't read base file <%s>\n",
                         basePath.c_str());
                drp->fResult = kBaseMissing;
            } else if (NULL == (comparisonFileBits = read_file(comparisonPath.c_str()))) {
                SkDebugf("WARNING: couldn't read comparison file <%s>\n",
                         comparisonPath.c_str());
                drp->fResult = kComparisonMissing;
            } else {
                if (are_buffers_equal(baseFileBits, comparisonFileBits)) {
                    drp->fResult = kEqualBits;
                } else if (get_bitmaps(baseFileBits, comparisonFileBits, drp)) {
                    create_and_write_diff_image(drp, dmp, colorThreshold,
                                                outputDir, *baseFiles[i]);
                } else {
                    drp->fResult = kDifferentOther;
                }
            }
            if (baseFileBits) {
                baseFileBits->unref();
            }
            if (comparisonFileBits) {
                comparisonFileBits->unref();
            }
            ++i;
            ++j;
        }
        SkASSERT(kUnknown != drp->fResult);
        differences->push(drp);
        summary->add(drp);
    }

    for (; i < baseFiles.count(); ++i) {
        // files only in baseDir
        SkString basePath (baseDir);
        basePath.append(*baseFiles[i]);
        SkString comparisonPath;
        DiffRecord *drp = new DiffRecord(*baseFiles[i], basePath,
                                         comparisonPath, kComparisonMissing);
        differences->push(drp);
        summary->add(drp);
    }

    for (; j < comparisonFiles.count(); ++j) {
        // files only in comparisonDir
        SkString basePath;
        SkString comparisonPath(comparisonDir);
        comparisonPath.append(*comparisonFiles[j]);
        DiffRecord *drp = new DiffRecord(*comparisonFiles[j], basePath,
                                         comparisonPath, kBaseMissing);
        differences->push(drp);
        summary->add(drp);
    }

    release_file_list(&baseFiles);
    release_file_list(&comparisonFiles);
}

/// Make layout more consistent by scaling image to 240 height, 360 width,
/// or natural size, whichever is smallest.
static int compute_image_height (int height, int width) {
    int retval = 240;
    if (height < retval) {
        retval = height;
    }
    float scale = (float) retval / height;
    if (width * scale > 360) {
        scale = (float) 360 / width;
        retval = static_cast<int>(height * scale);
    }
    return retval;
}

static void print_table_header (SkFILEWStream* stream,
                                const int matchCount,
                                const int colorThreshold,
                                const RecordArray& differences,
                                const SkString &baseDir,
                                const SkString &comparisonDir,
                                bool doOutputDate=false) {
    stream->writeText("<table>\n");
    stream->writeText("<tr><th>");
    stream->writeText("select image</th>\n<th>");
    if (doOutputDate) {
        SkTime::DateTime dt;
        SkTime::GetDateTime(&dt);
        stream->writeText("SkDiff run at ");
        stream->writeDecAsText(dt.fHour);
        stream->writeText(":");
        if (dt.fMinute < 10) {
            stream->writeText("0");
        }
        stream->writeDecAsText(dt.fMinute);
        stream->writeText(":");
        if (dt.fSecond < 10) {
            stream->writeText("0");
        }
        stream->writeDecAsText(dt.fSecond);
        stream->writeText("<br>");
    }
    stream->writeDecAsText(matchCount);
    stream->writeText(" of ");
    stream->writeDecAsText(differences.count());
    stream->writeText(" images matched ");
    if (colorThreshold == 0) {
        stream->writeText("exactly");
    } else {
        stream->writeText("within ");
        stream->writeDecAsText(colorThreshold);
        stream->writeText(" color units per component");
    }
    stream->writeText(".<br>");
    stream->writeText("</th>\n<th>");
    stream->writeText("every different pixel shown in white");
    stream->writeText("</th>\n<th>");
    stream->writeText("color difference at each pixel");
    stream->writeText("</th>\n<th>baseDir: ");
    stream->writeText(baseDir.c_str());
    stream->writeText("</th>\n<th>comparisonDir: ");
    stream->writeText(comparisonDir.c_str());
    stream->writeText("</th>\n");
    stream->writeText("</tr>\n");
}

static void print_pixel_count (SkFILEWStream* stream,
                               const DiffRecord& diff) {
    stream->writeText("<br>(");
    stream->writeDecAsText(static_cast<int>(diff.fFractionDifference *
                                            diff.fBaseWidth *
                                            diff.fBaseHeight));
    stream->writeText(" pixels)");
/*
    stream->writeDecAsText(diff.fWeightedFraction *
                           diff.fBaseWidth *
                           diff.fBaseHeight);
    stream->writeText(" weighted pixels)");
*/
}

static void print_checkbox_cell (SkFILEWStream* stream,
                                 const DiffRecord& diff) {
    stream->writeText("<td><input type=\"checkbox\" name=\"");
    stream->writeText(diff.fFilename.c_str());
    stream->writeText("\" checked=\"yes\"></td>");
}

static void print_label_cell (SkFILEWStream* stream,
                              const DiffRecord& diff) {
    char metricBuf [20];

    stream->writeText("<td><b>");
    stream->writeText(diff.fFilename.c_str());
    stream->writeText("</b><br>");
    switch (diff.fResult) {
      case kEqualBits:
        SkDEBUGFAIL("should not encounter DiffRecord with kEqualBits here");
        return;
      case kEqualPixels:
        SkDEBUGFAIL("should not encounter DiffRecord with kEqualPixels here");
        return;
      case kDifferentSizes:
        stream->writeText("Image sizes differ</td>");
        return;
      case kDifferentPixels:
        sprintf(metricBuf, "%12.4f%%", 100 * diff.fFractionDifference);
        stream->writeText(metricBuf);
        stream->writeText(" of pixels differ");
        stream->writeText("\n  (");
        sprintf(metricBuf, "%12.4f%%", 100 * diff.fWeightedFraction);
        stream->writeText(metricBuf);
        stream->writeText(" weighted)");
        // Write the actual number of pixels that differ if it's < 1%
        if (diff.fFractionDifference < 0.01) {
            print_pixel_count(stream, diff);
        }
        stream->writeText("<br>Average color mismatch ");
        stream->writeDecAsText(static_cast<int>(MAX3(diff.fAverageMismatchR,
                                                     diff.fAverageMismatchG,
                                                     diff.fAverageMismatchB)));
        stream->writeText("<br>Max color mismatch ");
        stream->writeDecAsText(MAX3(diff.fMaxMismatchR,
                                    diff.fMaxMismatchG,
                                    diff.fMaxMismatchB));
        stream->writeText("</td>");
        break;
      case kDifferentOther:
        stream->writeText("Files differ; unable to parse one or both files</td>");
        return;
      case kBaseMissing:
        stream->writeText("Missing from baseDir</td>");
        return;
      case kComparisonMissing:
        stream->writeText("Missing from comparisonDir</td>");
        return;
      default:
        SkDEBUGFAIL("encountered DiffRecord with unknown result type");
        return;
    }
}

static void print_image_cell (SkFILEWStream* stream,
                              const SkString& path,
                              int height) {
    stream->writeText("<td><a href=\"");
    stream->writeText(path.c_str());
    stream->writeText("\"><img src=\"");
    stream->writeText(path.c_str());
    stream->writeText("\" height=\"");
    stream->writeDecAsText(height);
    stream->writeText("px\"></a></td>");
}

#if 0 // UNUSED
static void print_text_cell (SkFILEWStream* stream, const char* text) {
    stream->writeText("<td align=center>");
    if (NULL != text) {
        stream->writeText(text);
    }
    stream->writeText("</td>");
}
#endif

static void print_diff_with_missing_file(SkFILEWStream* stream,
                                         DiffRecord& diff,
                                         const SkString& relativePath) {
    stream->writeText("<tr>\n");
    print_checkbox_cell(stream, diff);
    print_label_cell(stream, diff);
    stream->writeText("<td>N/A</td>");
    stream->writeText("<td>N/A</td>");
    if (kBaseMissing != diff.fResult) {
        int h, w;
        if (!get_bitmap_height_width(diff.fBasePath, &h, &w)) {
            stream->writeText("<td>N/A</td>");
        } else {
            int height = compute_image_height(h, w);
            if (!diff.fBasePath.startsWith(PATH_DIV_STR)) {
                diff.fBasePath.prepend(relativePath);
            }
            print_image_cell(stream, diff.fBasePath, height);
        }
    } else {
        stream->writeText("<td>N/A</td>");
    }
    if (kComparisonMissing != diff.fResult) {
        int h, w;
        if (!get_bitmap_height_width(diff.fComparisonPath, &h, &w)) {
            stream->writeText("<td>N/A</td>");
        } else {
            int height = compute_image_height(h, w);
            if (!diff.fComparisonPath.startsWith(PATH_DIV_STR)) {
                diff.fComparisonPath.prepend(relativePath);
            }
            print_image_cell(stream, diff.fComparisonPath, height);
        }
    } else {
        stream->writeText("<td>N/A</td>");
    }
    stream->writeText("</tr>\n");
    stream->flush();
}

static void print_diff_page (const int matchCount,
                             const int colorThreshold,
                             const RecordArray& differences,
                             const SkString& baseDir,
                             const SkString& comparisonDir,
                             const SkString& outputDir) {

    SkASSERT(!baseDir.isEmpty());
    SkASSERT(!comparisonDir.isEmpty());
    SkASSERT(!outputDir.isEmpty());

    SkString outputPath (outputDir);
    outputPath.append("index.html");
    //SkFILEWStream outputStream ("index.html");
    SkFILEWStream outputStream (outputPath.c_str());

    // Need to convert paths from relative-to-cwd to relative-to-outputDir
    // FIXME this doesn't work if there are '..' inside the outputDir

    bool isPathAbsolute = false;
    // On Windows or Linux, a path starting with PATH_DIV_CHAR is absolute.
    if (outputDir.size() > 0 && PATH_DIV_CHAR == outputDir[0]) {
        isPathAbsolute = true;
    }
#ifdef SK_BUILD_FOR_WIN32
    // On Windows, absolute paths can also start with "x:", where x is any
    // drive letter.
    if (outputDir.size() > 1 && ':' == outputDir[1]) {
        isPathAbsolute = true;
    }
#endif

    SkString relativePath;
    if (!isPathAbsolute) {
        unsigned int ui;
        for (ui = 0; ui < outputDir.size(); ui++) {
            if (outputDir[ui] == PATH_DIV_CHAR) {
                relativePath.append(".." PATH_DIV_STR);
            }
        }
    }

    outputStream.writeText(
        "<html>\n<head>\n"
        "<script src=\"https://ajax.googleapis.com/ajax/"
        "libs/jquery/1.7.2/jquery.min.js\"></script>\n"
        "<script type=\"text/javascript\">\n"
        "function generateCheckedList() {\n"
        "var boxes = $(\":checkbox:checked\");\n"
        "var fileCmdLineString = '';\n"
        "var fileMultiLineString = '';\n"
        "for (var i = 0; i < boxes.length; i++) {\n"
        "fileMultiLineString += boxes[i].name + '<br>';\n"
        "fileCmdLineString += boxes[i].name + '&nbsp;';\n"
        "}\n"
        "$(\"#checkedList\").html(fileCmdLineString + "
        "'<br><br>' + fileMultiLineString);\n"
        "}\n"
        "</script>\n</head>\n<body>\n");
    print_table_header(&outputStream, matchCount, colorThreshold, differences,
                       baseDir, comparisonDir);
    int i;
    for (i = 0; i < differences.count(); i++) {
        DiffRecord* diff = differences[i];

        switch (diff->fResult) {
          // Cases in which there is no diff to report.
          case kEqualBits:
          case kEqualPixels:
            continue;
          // Cases in which we want a detailed pixel diff.
          case kDifferentPixels:
            break;
          // Cases in which the files differed, but we can't display the diff.
          case kDifferentSizes:
          case kDifferentOther:
          case kBaseMissing:
          case kComparisonMissing:
            print_diff_with_missing_file(&outputStream, *diff, relativePath);
            continue;
          default:
            SkDEBUGFAIL("encountered DiffRecord with unknown result type");
            continue;
        }

        if (!diff->fBasePath.startsWith(PATH_DIV_STR)) {
            diff->fBasePath.prepend(relativePath);
        }
        if (!diff->fComparisonPath.startsWith(PATH_DIV_STR)) {
            diff->fComparisonPath.prepend(relativePath);
        }

        int height = compute_image_height(diff->fBaseHeight, diff->fBaseWidth);
        outputStream.writeText("<tr>\n");
        print_checkbox_cell(&outputStream, *diff);
        print_label_cell(&outputStream, *diff);
        print_image_cell(&outputStream,
                         filename_to_white_filename(diff->fFilename), height);
        print_image_cell(&outputStream,
                         filename_to_diff_filename(diff->fFilename), height);
        print_image_cell(&outputStream, diff->fBasePath, height);
        print_image_cell(&outputStream, diff->fComparisonPath, height);
        outputStream.writeText("</tr>\n");
        outputStream.flush();
    }
    outputStream.writeText(
        "</table>\n"
        "<input type=\"button\" "
        "onclick=\"generateCheckedList()\" "
        "value=\"Create Rebaseline List\">\n"
        "<div id=\"checkedList\"></div>\n"
        "</body>\n</html>\n");
    outputStream.flush();
}

static void usage (char * argv0) {
    SkDebugf("Skia baseline image diff tool\n");
    SkDebugf("\n"
"Usage: \n"
"    %s <baseDir> <comparisonDir> [outputDir] \n"
, argv0, argv0);
    SkDebugf(
"\nArguments:"
"\n    --failonresult <result>: After comparing all file pairs, exit with nonzero"
"\n                             return code (number of file pairs yielding this"
"\n                             result) if any file pairs yielded this result."
"\n                             This flag may be repeated, in which case the"
"\n                             return code will be the number of fail pairs"
"\n                             yielding ANY of these results."
"\n    --help: display this info"
"\n    --listfilenames: list all filenames for each result type in stdout"
"\n    --match <substring>: compare files whose filenames contain this substring;"
"\n                         if unspecified, compare ALL files."
"\n                         this flag may be repeated."
"\n    --nodiffs: don't write out image diffs or index.html, just generate"
"\n               report on stdout"
"\n    --nomatch <substring>: regardless of --match, DO NOT compare files whose"
"\n                           filenames contain this substring."
"\n                           this flag may be repeated."
"\n    --noprintdirs: do not print the directories used."
"\n    --norecurse: do not recurse into subdirectories."
"\n    --sortbymaxmismatch: sort by worst color channel mismatch;"
"\n                         break ties with -sortbymismatch"
"\n    --sortbymismatch: sort by average color channel mismatch"
"\n    --threshold <n>: only report differences > n (per color channel) [default 0]"
"\n    --weighted: sort by # pixels different weighted by color difference"
"\n"
"\n    baseDir: directory to read baseline images from."
"\n    comparisonDir: directory to read comparison images from"
"\n    outputDir: directory to write difference images and index.html to;"
"\n               defaults to comparisonDir"
"\n"
"\nIf no sort is specified, it will sort by fraction of pixels mismatching."
"\n");
}

const int kNoError = 0;
const int kGenericError = -1;

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    DiffMetricProc diffProc = compute_diff_pmcolor;
    int (*sortProc)(const void*, const void*) = compare<CompareDiffMetrics>;

    // Maximum error tolerated in any one color channel in any one pixel before
    // a difference is reported.
    int colorThreshold = 0;
    SkString baseDir;
    SkString comparisonDir;
    SkString outputDir;

    StringArray matchSubstrings;
    StringArray nomatchSubstrings;

    bool generateDiffs = true;
    bool listFilenames = false;
    bool printDirNames = true;
    bool recurseIntoSubdirs = true;

    RecordArray differences;
    DiffSummary summary;

    bool failOnResultType[kNumResultTypes];
    for (int i = 0; i < kNumResultTypes; i++) {
        failOnResultType[i] = false;
    }

    int i;
    int numUnflaggedArguments = 0;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--failonresult")) {
            Result type = getResultByName(argv[++i]);
            failOnResultType[type] = true;
            continue;
        }
        if (!strcmp(argv[i], "--help")) {
            usage(argv[0]);
            return kNoError;
        }
        if (!strcmp(argv[i], "--listfilenames")) {
            listFilenames = true;
            continue;
        }
        if (!strcmp(argv[i], "--match")) {
            matchSubstrings.push(new SkString(argv[++i]));
            continue;
        }
        if (!strcmp(argv[i], "--nodiffs")) {
            generateDiffs = false;
            continue;
        }
        if (!strcmp(argv[i], "--nomatch")) {
            nomatchSubstrings.push(new SkString(argv[++i]));
            continue;
        }
        if (!strcmp(argv[i], "--noprintdirs")) {
            printDirNames = false;
            continue;
        }
        if (!strcmp(argv[i], "--norecurse")) {
            recurseIntoSubdirs = false;
            continue;
        }
        if (!strcmp(argv[i], "--sortbymaxmismatch")) {
            sortProc = compare<CompareDiffMaxMismatches>;
            continue;
        }
        if (!strcmp(argv[i], "--sortbymismatch")) {
            sortProc = compare<CompareDiffMeanMismatches>;
            continue;
        }
        if (!strcmp(argv[i], "--threshold")) {
            colorThreshold = atoi(argv[++i]);
            continue;
        }
        if (!strcmp(argv[i], "--weighted")) {
            sortProc = compare<CompareDiffWeighted>;
            continue;
        }
        if (argv[i][0] != '-') {
            switch (numUnflaggedArguments++) {
                case 0:
                    baseDir.set(argv[i]);
                    continue;
                case 1:
                    comparisonDir.set(argv[i]);
                    continue;
                case 2:
                    outputDir.set(argv[i]);
                    continue;
                default:
                    SkDebugf("extra unflagged argument <%s>\n", argv[i]);
                    usage(argv[0]);
                    return kGenericError;
            }
        }

        SkDebugf("Unrecognized argument <%s>\n", argv[i]);
        usage(argv[0]);
        return kGenericError;
    }

    if (numUnflaggedArguments == 2) {
        outputDir = comparisonDir;
    } else if (numUnflaggedArguments != 3) {
        usage(argv[0]);
        return kGenericError;
    }

    if (!baseDir.endsWith(PATH_DIV_STR)) {
        baseDir.append(PATH_DIV_STR);
    }
    if (printDirNames) {
        printf("baseDir is [%s]\n", baseDir.c_str());
    }

    if (!comparisonDir.endsWith(PATH_DIV_STR)) {
        comparisonDir.append(PATH_DIV_STR);
    }
    if (printDirNames) {
        printf("comparisonDir is [%s]\n", comparisonDir.c_str());
    }

    if (!outputDir.endsWith(PATH_DIV_STR)) {
        outputDir.append(PATH_DIV_STR);
    }
    if (generateDiffs) {
        if (printDirNames) {
            printf("writing diffs to outputDir is [%s]\n", outputDir.c_str());
        }
    } else {
        if (printDirNames) {
            printf("not writing any diffs to outputDir [%s]\n", outputDir.c_str());
        }
        outputDir.set("");
    }

    // If no matchSubstrings were specified, match ALL strings
    // (except for whatever nomatchSubstrings were specified, if any).
    if (matchSubstrings.isEmpty()) {
        matchSubstrings.push(new SkString(""));
    }

    create_diff_images(diffProc, colorThreshold, &differences,
                       baseDir, comparisonDir, outputDir,
                       matchSubstrings, nomatchSubstrings, recurseIntoSubdirs, &summary);
    summary.print(listFilenames, failOnResultType);

    if (differences.count()) {
        qsort(differences.begin(), differences.count(),
              sizeof(DiffRecord*), sortProc);
    }

    if (generateDiffs) {
        print_diff_page(summary.fNumMatches, colorThreshold, differences,
                        baseDir, comparisonDir, outputDir);
    }

    for (i = 0; i < differences.count(); i++) {
        delete differences[i];
    }
    matchSubstrings.deleteAll();
    nomatchSubstrings.deleteAll();

    int num_failing_results = 0;
    for (int i = 0; i < kNumResultTypes; i++) {
        if (failOnResultType[i]) {
            num_failing_results += summary.fResultsOfType[i].count();
        }
    }

    // On Linux (and maybe other platforms too), any results outside of the
    // range [0...255] are wrapped (mod 256).  Do the conversion ourselves, to
    // make sure that we only return 0 when there were no failures.
    return (num_failing_results > 255) ? 255 : num_failing_results;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
