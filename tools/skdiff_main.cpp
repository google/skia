
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkColorPriv.h"
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
 * Does *not* recursively descend directories.
 *
 * With the --chromium flag, *does* recursively descend the first directory
 * named, comparing *-expected.png with *-actual.png and writing diff
 * images into the second directory, also writing index.html there.
 */

#if SK_BUILD_FOR_WIN32
    #define PATH_DIV_STR "\\"
    #define PATH_DIV_CHAR '\\'
#else
    #define PATH_DIV_STR "/"
    #define PATH_DIV_CHAR '/'
#endif

struct DiffRecord {
    DiffRecord (const SkString filename,
                const SkString basePath,
                const SkString comparisonPath)
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
        , fDoImageSizesMismatch (false) {
        // These asserts are valid for GM, but not for --chromium
        //SkASSERT(basePath.endsWith(filename.c_str()));
        //SkASSERT(comparisonPath.endsWith(filename.c_str()));
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

    /// By the time we need to report image size mismatch, we've already
    /// released the bitmaps, so we need to remember it when we detect it.
    bool fDoImageSizesMismatch;
};

#define MAX2(a,b) (((b) < (a)) ? (a) : (b))
#define MAX3(a,b,c) (((b) < (a)) ? MAX2((a), (c)) : MAX2((b), (c)))

const SkPMColor PMCOLOR_WHITE = SkPreMultiplyColor(SK_ColorWHITE);
const SkPMColor PMCOLOR_BLACK = SkPreMultiplyColor(SK_ColorBLACK);

struct DiffSummary {
    DiffSummary ()
        : fNumMatches (0)
        , fNumMismatches (0)
        , fMaxMismatchV (0)
        , fMaxMismatchPercent (0) { };

    uint32_t fNumMatches;
    uint32_t fNumMismatches;
    uint32_t fMaxMismatchV;
    float fMaxMismatchPercent;

    void print () {
        printf("%d of %d images matched.\n", fNumMatches,
               fNumMatches + fNumMismatches);
        if (fNumMismatches > 0) {
            printf("Maximum pixel intensity mismatch %d\n", fMaxMismatchV);
            printf("Largest area mismatch was %.2f%% of pixels\n",
                   fMaxMismatchPercent);
        }

    }

    void add (DiffRecord* drp) {
        if (0 == drp->fFractionDifference) {
            fNumMatches++;
        } else {
            fNumMismatches++;
            if (drp->fFractionDifference * 100 > fMaxMismatchPercent) {
                fMaxMismatchPercent = drp->fFractionDifference * 100;
            }
            uint32_t value = MAX3(drp->fMaxMismatchR, drp->fMaxMismatchG,
                                  drp->fMaxMismatchB);
            if (value > fMaxMismatchV) {
                fMaxMismatchV = value;
            }
        }
    }
};

typedef SkTDArray<DiffRecord*> RecordArray;

/// Comparison routine for qsort;  sorts by fFractionDifference
/// from largest to smallest.
static int compare_diff_metrics (DiffRecord** lhs, DiffRecord** rhs) {
    if ((*lhs)->fFractionDifference < (*rhs)->fFractionDifference) {
        return 1;
    }
    if ((*rhs)->fFractionDifference < (*lhs)->fFractionDifference) {
        return -1;
    }
    return 0;
}

static int compare_diff_weighted (DiffRecord** lhs, DiffRecord** rhs) {
    if ((*lhs)->fWeightedFraction < (*rhs)->fWeightedFraction) {
        return 1;
    }
    if ((*lhs)->fWeightedFraction > (*rhs)->fWeightedFraction) {
        return -1;
    }
    return 0;
}

/// Comparison routine for qsort;  sorts by max(fAverageMismatch{RGB})
/// from largest to smallest.
static int compare_diff_mean_mismatches (DiffRecord** lhs, DiffRecord** rhs) {
    float leftValue = MAX3((*lhs)->fAverageMismatchR,
                           (*lhs)->fAverageMismatchG,
                           (*lhs)->fAverageMismatchB);
    float rightValue = MAX3((*rhs)->fAverageMismatchR,
                            (*rhs)->fAverageMismatchG,
                            (*rhs)->fAverageMismatchB);
    if (leftValue < rightValue) {
        return 1;
    }
    if (rightValue < leftValue) {
        return -1;
    }
    return 0;
}

/// Comparison routine for qsort;  sorts by max(fMaxMismatch{RGB})
/// from largest to smallest.
static int compare_diff_max_mismatches (DiffRecord** lhs, DiffRecord** rhs) {
    uint32_t leftValue = MAX3((*lhs)->fMaxMismatchR,
                              (*lhs)->fMaxMismatchG,
                              (*lhs)->fMaxMismatchB);
    uint32_t rightValue = MAX3((*rhs)->fMaxMismatchR,
                               (*rhs)->fMaxMismatchG,
                               (*rhs)->fMaxMismatchB);
    if (leftValue < rightValue) {
        return 1;
    }
    if (rightValue < leftValue) {
        return -1;
    }
    return compare_diff_mean_mismatches(lhs, rhs);
}



/// Parameterized routine to compute the color of a pixel in a difference image.
typedef SkPMColor (*DiffMetricProc)(SkPMColor, SkPMColor);

static void expand_and_copy (int width, int height, SkBitmap** dest) {
    SkBitmap* temp = new SkBitmap ();
    temp->reset();
    temp->setConfig((*dest)->config(), width, height);
    temp->allocPixels();
    (*dest)->copyPixelsTo(temp->getPixels(), temp->getSize(),
                          temp->rowBytes());
    *dest = temp;
}

static bool get_bitmaps (DiffRecord* diffRecord) {
    SkFILEStream compareStream(diffRecord->fComparisonPath.c_str());
    if (!compareStream.isValid()) {
        SkDebugf("WARNING: couldn't open comparison file <%s>\n",
                 diffRecord->fComparisonPath.c_str());
        return false;
    }

    SkFILEStream baseStream(diffRecord->fBasePath.c_str());
    if (!baseStream.isValid()) {
        SkDebugf("ERROR: couldn't open base file <%s>\n",
                 diffRecord->fBasePath.c_str());
        return false;
    }

    SkImageDecoder* codec = SkImageDecoder::Factory(&baseStream);
    if (NULL == codec) {
        SkDebugf("ERROR: no codec found for <%s>\n",
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
        SkDebugf("ERROR: codec failed for <%s>\n",
                 diffRecord->fBasePath.c_str());
        return false;
    }

    diffRecord->fBaseWidth = diffRecord->fBaseBitmap->width();
    diffRecord->fBaseHeight = diffRecord->fBaseBitmap->height();

    if (!codec->decode(&compareStream, diffRecord->fComparisonBitmap,
                       SkBitmap::kARGB_8888_Config,
                       SkImageDecoder::kDecodePixels_Mode)) {
        SkDebugf("ERROR: codec failed for <%s>\n",
                 diffRecord->fComparisonPath.c_str());
        return false;
    }

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
        dr->fDoImageSizesMismatch = true;
        dr->fFractionDifference = 1;
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
    int pixelCount = w * h;
    dr->fFractionDifference = ((float) mismatchedPixels) / pixelCount;
    dr->fWeightedFraction /= pixelCount;
    dr->fAverageMismatchR = ((float) totalMismatchR) / pixelCount;
    dr->fAverageMismatchG = ((float) totalMismatchG) / pixelCount;
    dr->fAverageMismatchB = ((float) totalMismatchB) / pixelCount;
}

static SkString filename_to_derived_filename (const SkString& filename,
                                              const char *suffix) {
    SkString diffName (filename);
    const char* cstring = diffName.c_str();
    int dotOffset = strrchr(cstring, '.') - cstring;
    diffName.remove(dotOffset, diffName.size() - dotOffset);
    diffName.append(suffix);
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

/// Convert a chromium/WebKit LayoutTest "foo-expected.png" to "foo-actual.png"
static SkString chrome_expected_path_to_actual (const SkString& expected) {
    SkString actualPath (expected);
    actualPath.remove(actualPath.size() - 13, 13);
    actualPath.append("-actual.png");
    return actualPath;
}

/// Convert a chromium/WebKit LayoutTest "foo-expected.png" to "foo.png"
static SkString chrome_expected_name_to_short (const SkString& expected) {
    SkString shortName (expected);
    shortName.remove(shortName.size() - 13, 13);
    shortName.append(".png");
    return shortName;
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
    compute_diff(drp, dmp, colorThreshold);

    SkString differencePath (outputDir);
    differencePath.append(filename_to_diff_filename(filename));
    write_bitmap(differencePath, drp->fDifferenceBitmap);
    SkString whitePath (outputDir);
    whitePath.append(filename_to_white_filename(filename));
    write_bitmap(whitePath, drp->fWhiteBitmap);
    release_bitmaps(drp);
}

/// Creates difference images, returns the number that have a 0 metric.
static void create_diff_images (DiffMetricProc dmp,
                                const int colorThreshold,
                                RecordArray* differences,
                                const SkString& baseDir,
                                const SkString& comparisonDir,
                                const SkString& outputDir,
                                DiffSummary* summary) {

    //@todo thudson 19 Apr 2011
    // this lets us know about files in baseDir not in compareDir, but it
    // doesn't detect files in compareDir not in baseDir. Doing that
    // efficiently seems to imply iterating through both directories to
    // create a merged list, and then attempting to process every entry
    // in that list?

    SkOSFile::Iter baseIterator (baseDir.c_str());
    SkString filename;
    while (baseIterator.next(&filename)) {
        if (filename.endsWith(".pdf")) {
            continue;
        }
        SkString basePath (baseDir);
        basePath.append(filename);
        SkString comparisonPath (comparisonDir);
        comparisonPath.append(filename);
        DiffRecord * drp = new DiffRecord (filename, basePath, comparisonPath);
        if (!get_bitmaps(drp)) {
            continue;
        }

        create_and_write_diff_image(drp, dmp, colorThreshold,
                                    outputDir, filename);

        differences->push(drp);
        summary->add(drp);
    }
}

static void create_diff_images_chromium (DiffMetricProc dmp,
                                         const int colorThreshold,
                                         RecordArray* differences,
                                         const SkString& dirname,
                                         const SkString& outputDir,
                                         DiffSummary* summary) {
    SkOSFile::Iter baseIterator (dirname.c_str());
    SkString filename;
    while (baseIterator.next(&filename)) {
        if (filename.endsWith(".pdf")) {
            continue;
        }
        if (filename.endsWith("-expected.png")) {
            SkString expectedPath (dirname);
            expectedPath.append(filename);
            SkString shortName (chrome_expected_name_to_short(filename));
            SkString actualPath (chrome_expected_path_to_actual(expectedPath));
            DiffRecord * drp =
                new DiffRecord (shortName, expectedPath, actualPath);
            if (!get_bitmaps(drp)) {
                continue;
            }
            create_and_write_diff_image(drp, dmp, colorThreshold,
                                        outputDir, shortName);

            differences->push(drp);
            summary->add(drp);
        }
    }
}

static void analyze_chromium(DiffMetricProc dmp,
                             const int colorThreshold,
                             RecordArray* differences,
                             const SkString& dirname,
                             const SkString& outputDir,
                             DiffSummary* summary) {
    create_diff_images_chromium(dmp, colorThreshold, differences,
                                dirname, outputDir, summary);
    SkOSFile::Iter dirIterator(dirname.c_str());
    SkString newdirname;
    while (dirIterator.next(&newdirname, true)) {
        if (newdirname.startsWith(".")) {
            continue;
        }
        SkString fullname (dirname);
        fullname.append(newdirname);
        if (!fullname.endsWith(PATH_DIV_STR)) {
            fullname.append(PATH_DIV_STR);
        }
        analyze_chromium(dmp, colorThreshold, differences,
                         fullname, outputDir, summary);
    }
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
                                const SkString &comparisonDir) {
    SkTime::DateTime dt;
    SkTime::GetDateTime(&dt);
    stream->writeText("<table>\n");
    stream->writeText("<tr><th>");
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
    stream->writeText("</th>\n<th>");
    stream->writeText(baseDir.c_str());
    stream->writeText("</th>\n<th>");
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

static void print_label_cell (SkFILEWStream* stream,
                              const DiffRecord& diff) {
    stream->writeText("<td>");
    stream->writeText(diff.fFilename.c_str());
    stream->writeText("<br>");
    if (diff.fDoImageSizesMismatch) {
        stream->writeText("Image sizes differ");
        stream->writeText("</td>");
        return;
    }
    char metricBuf [20];
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

static void print_diff_page (const int matchCount,
                             const int colorThreshold,
                             const RecordArray& differences,
                             const SkString& baseDir,
                             const SkString& comparisonDir,
                             const SkString& outputDir) {

    SkString outputPath (outputDir);
    outputPath.append("index.html");
    //SkFILEWStream outputStream ("index.html");
    SkFILEWStream outputStream (outputPath.c_str());

    // Need to convert paths from relative-to-cwd to relative-to-outputDir
    // FIXME this doesn't work if there are '..' inside the outputDir
    unsigned int ui;
    SkString relativePath;
    for (ui = 0; ui < outputDir.size(); ui++) {
        if (outputDir[ui] == PATH_DIV_CHAR) {
            relativePath.append(".." PATH_DIV_STR);
        }
    }

    outputStream.writeText("<html>\n<body>\n");
    print_table_header(&outputStream, matchCount, colorThreshold, differences,
                       baseDir, comparisonDir);
    int i;
    for (i = 0; i < differences.count(); i++) {
        DiffRecord* diff = differences[i];
        if (0 == diff->fFractionDifference) {
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
    outputStream.writeText("</table>\n");
    outputStream.writeText("</body>\n</html>\n");
    outputStream.flush();
}

static void usage (char * argv0) {
    SkDebugf("Skia baseline image diff tool\n");
    SkDebugf("Usage: %s baseDir comparisonDir [outputDir]\n", argv0);
    SkDebugf(
"       %s --chromium --release|--debug baseDir outputDir\n", argv0);
    SkDebugf(
"    --threshold n: only report differences > n (in one channel) [default 0]\n"
"    --sortbymismatch: sort by average color channel mismatch\n");
    SkDebugf(
"    --sortbymaxmismatch: sort by worst color channel mismatch,\n"
"                        break ties with -sortbymismatch,\n"
"        [default by fraction of pixels mismatching]\n");
    SkDebugf(
"    --weighted: sort by # pixels different weighted by color difference\n");
    SkDebugf(
"    --chromium-release: process Webkit LayoutTests results instead of gm\n"
"    --chromium-debug: process Webkit LayoutTests results instead of gm\n");
    SkDebugf(
"    baseDir: directory to read baseline images from,\n"
"             or chromium/src directory for --chromium.\n");
    SkDebugf("    comparisonDir: directory to read comparison images from\n");
    SkDebugf(
"    outputDir: directory to write difference images to; defaults to\n"
"               comparisonDir when not running --chromium\n");
    SkDebugf("Also creates an \"index.html\" file in the output directory.\n");
}

int main (int argc, char ** argv) {
    DiffMetricProc diffProc = compute_diff_pmcolor;
    SkQSortCompareProc sortProc = (SkQSortCompareProc) compare_diff_metrics;

    // Maximum error tolerated in any one color channel in any one pixel before
    // a difference is reported.
    int colorThreshold = 0;
    SkString baseDir;
    SkString comparisonDir;
    SkString outputDir;

    bool analyzeChromium = false;
    bool chromiumDebug = false;
    bool chromiumRelease = false;

    RecordArray differences;
    DiffSummary summary;

    int i, j;
    for (i = 1, j = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--help")) {
            usage(argv[0]);
            return 0;
        }
        if (!strcmp(argv[i], "--sortbymismatch")) {
            sortProc = (SkQSortCompareProc) compare_diff_mean_mismatches;
            continue;
        }
        if (!strcmp(argv[i], "--sortbymaxmismatch")) {
            sortProc = (SkQSortCompareProc) compare_diff_max_mismatches;
            continue;
        }
        if (!strcmp(argv[i], "--weighted")) {
            sortProc = (SkQSortCompareProc) compare_diff_weighted;
            continue;
        }
        if (!strcmp(argv[i], "--chromium-release")) {
            analyzeChromium = true;
            chromiumRelease = true;
            continue;
        }
        if (!strcmp(argv[i], "--chromium-debug")) {
            analyzeChromium = true;
            chromiumDebug = true;
            continue;
        }
        if (argv[i][0] != '-') {
            switch (j++) {
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
                    usage(argv[0]);
                    return 0;
            }
        }

        SkDebugf("Unrecognized argument <%s>\n", argv[i]);
        usage(argv[0]);
        return 0;
    }
    if (analyzeChromium) {
        if (j != 2) {
            usage(argv[0]);
            return 0;
        }
        if (chromiumRelease && chromiumDebug) {
            SkDebugf(
"--chromium must be either -release or -debug, not both!\n");
            return 0;
        }
    }

    if (j == 2) {
        outputDir = comparisonDir;
    } else if (j != 3) {
        usage(argv[0]);
        return 0;
    }

    if (!baseDir.endsWith(PATH_DIV_STR)) {
        baseDir.append(PATH_DIV_STR);
    }
    if (!comparisonDir.endsWith(PATH_DIV_STR)) {
        comparisonDir.append(PATH_DIV_STR);
    }
    if (!outputDir.endsWith(PATH_DIV_STR)) {
        outputDir.append(PATH_DIV_STR);
    }

    if (analyzeChromium) {
        baseDir.append("webkit" PATH_DIV_STR);
        if (chromiumRelease) {
            baseDir.append("Release" PATH_DIV_STR);
        }
        if (chromiumDebug) {
            baseDir.append("Debug" PATH_DIV_STR);
        }
        baseDir.append("layout-test-results" PATH_DIV_STR);
        analyze_chromium(diffProc, colorThreshold, &differences,
                         baseDir, outputDir, &summary);
    } else {
        create_diff_images(diffProc, colorThreshold, &differences,
                           baseDir, comparisonDir, outputDir, &summary);
    }
    summary.print();

    if (differences.count()) {
        SkQSort(differences.begin(), differences.count(),
            sizeof(DiffRecord*), sortProc);
    }
    print_diff_page(summary.fNumMatches, colorThreshold, differences,
                    baseDir, comparisonDir, outputDir);

    for (i = 0; i < differences.count(); i++) {
        delete differences[i];
    }
}
