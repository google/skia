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
 * Creates an index.html in the current working directory to compare each
 * pair that does not match exactly.
 * Does *not* recursively descend directories.
 */

struct DiffRecord {
    DiffRecord (const SkString filename)
        : fFilename (filename)
        , fFractionDifference (0)
        , fWeightedFraction (0)
        , fAverageMismatchR (0)
        , fAverageMismatchG (0)
        , fAverageMismatchB (0)
        , fMaxMismatchR (0)
        , fMaxMismatchG (0)
        , fMaxMismatchB (0) { };

    SkString fFilename;

    SkBitmap fBaseBitmap;
    SkBitmap fComparisonBitmap;
    SkBitmap fDifferenceBitmap;

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
};

#define MAX2(a,b) (((b) < (a)) ? (a) : (b))
#define MAX3(a,b,c) (((b) < (a)) ? MAX2((a), (c)) : MAX2((b), (c)))

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
    float leftValue = MAX3((*lhs)->fMaxMismatchR,
                           (*lhs)->fMaxMismatchG,
                           (*lhs)->fMaxMismatchB);
    float rightValue = MAX3((*rhs)->fMaxMismatchR,
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

static bool get_bitmaps (DiffRecord* diffRecord,
                         const SkString& baseDir,
                         const SkString& comparisonDir) {
    SkString comparePath (comparisonDir);
    comparePath.append(diffRecord->fFilename);
    SkFILEStream compareStream(comparePath.c_str());
    if (!compareStream.isValid()) {
        SkDebugf("WARNING: couldn't open comparison file <%s>\n",
                 comparePath.c_str());
        return false;
    }

    SkString basePath (baseDir);
    basePath.append(diffRecord->fFilename);
    SkFILEStream baseStream(basePath.c_str());
    if (!baseStream.isValid()) {
        SkDebugf("ERROR: couldn't open base file <%s>\n",
                 basePath.c_str());
        return false;
    }

    SkImageDecoder* codec = SkImageDecoder::Factory(&baseStream);
    if (NULL == codec) {
        SkDebugf("ERROR: no codec found for <%s>\n",
                 basePath.c_str());
        return false;
    }

    SkAutoTDelete<SkImageDecoder> ad(codec);

    baseStream.rewind();
    if (!codec->decode(&baseStream, &diffRecord->fBaseBitmap,
                       SkBitmap::kARGB_8888_Config,
                       SkImageDecoder::kDecodePixels_Mode)) {
        SkDebugf("ERROR: codec failed for <%s>\n",
                 basePath.c_str());
        return false;
    }

    if (!codec->decode(&compareStream, &diffRecord->fComparisonBitmap,
                       SkBitmap::kARGB_8888_Config,
                       SkImageDecoder::kDecodePixels_Mode)) {
        SkDebugf("ERROR: codec failed for <%s>\n",
                 comparePath.c_str());
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
static bool write_bitmap(const SkString& path, const SkBitmap& bitmap) {
    SkBitmap copy;
    bitmap.copyTo(&copy, SkBitmap::kARGB_8888_Config);
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

/// Returns white on every pixel so that differences jump out at you;
/// makes it easy to spot areas of difference that are in the least-significant
/// bits.
static inline SkPMColor compute_diff_white(SkPMColor c0, SkPMColor c1) {
    return SkPackARGB32(0xFF, 0xFF, 0xFF, 0xFF);
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
    SkAutoLockPixels alp(dr->fDifferenceBitmap);

    const int w = dr->fComparisonBitmap.width();
    const int h = dr->fComparisonBitmap.height();
    int mismatchedPixels = 0;
    int totalMismatchR = 0;
    int totalMismatchG = 0;
    int totalMismatchB = 0;
    // Accumulate fractionally different pixels, then divide out
    // # of pixels at the end.
    dr->fWeightedFraction = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            SkPMColor c0 = *dr->fBaseBitmap.getAddr32(x, y);
            SkPMColor c1 = *dr->fComparisonBitmap.getAddr32(x, y);
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
                *dr->fDifferenceBitmap.getAddr32(x, y) = outputDifference;
            } else {
                *dr->fDifferenceBitmap.getAddr32(x, y) = 0;
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

/// Given a image filename, returns the name of the file containing the
/// associated difference image.
static SkString filename_to_diff_filename (const SkString& filename) {
    SkString diffName (filename);
    const char* cstring = diffName.c_str();
    int dotOffset = strrchr(cstring, '.') - cstring;
    diffName.remove(dotOffset, diffName.size() - dotOffset);
    diffName.append("-diff.png");
    return diffName;
}

/// Creates difference images, returns the number that have a 0 metric.
static void create_diff_images (DiffMetricProc dmp,
                                SkQSortCompareProc scp,
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
        DiffRecord * drp = new DiffRecord (filename);
        if (!get_bitmaps(drp, baseDir, comparisonDir)) {
            continue;
        }

        const int w = drp->fBaseBitmap.width();
        const int h = drp->fBaseBitmap.height();
        drp->fDifferenceBitmap.setConfig(SkBitmap::kARGB_8888_Config, w, h);
        drp->fDifferenceBitmap.allocPixels();
        compute_diff(drp, dmp, colorThreshold);

        SkString outPath (outputDir);
        outPath.append(filename_to_diff_filename(filename));
        write_bitmap(outPath, drp->fDifferenceBitmap);

        differences->push(drp);
        summary->add(drp);
    }

    SkQSort(differences->begin(), differences->count(), sizeof(DiffRecord*),
            scp);
}

/// Make layout more consistent by scaling image to 240 height, 360 width,
/// or natural size, whichever is smallest.
static int compute_image_height (const SkBitmap& bmp) {
    int retval = 240;
    if (bmp.height() < retval) {
        retval = bmp.height();
    }
    float scale = (float) retval / bmp.height();
    if (bmp.width() * scale > 360) {
        scale = (float) 360 / bmp.width();
        retval = bmp.height() * scale;
    }
    return retval;
}

static void print_page_header (SkFILEWStream* stream,
                               const int matchCount,
                               const int colorThreshold,
                               const RecordArray& differences) {
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

}

static void print_pixel_count (SkFILEWStream* stream,
                               const DiffRecord& diff) {
    stream->writeText("<br>(");
    stream->writeDecAsText(diff.fFractionDifference *
                           diff.fBaseBitmap.width() *
                           diff.fBaseBitmap.height());
    stream->writeText(" pixels)");
/*
    stream->writeDecAsText(diff.fWeightedFraction *
                           diff.fBaseBitmap.width() *
                           diff.fBaseBitmap.height());
    stream->writeText(" weighted pixels)");
*/
}

static void print_label_cell (SkFILEWStream* stream,
                              const DiffRecord& diff) {
    stream->writeText("<td>");
    stream->writeText(diff.fFilename.c_str());
    stream->writeText("<br>");
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
    stream->writeDecAsText(MAX3(diff.fAverageMismatchR,
                                diff.fAverageMismatchG,
                                diff.fAverageMismatchB));
    stream->writeText("<br>Max color mismatch ");
    stream->writeDecAsText(MAX3(diff.fMaxMismatchR,
                                diff.fMaxMismatchG,
                                diff.fMaxMismatchB));
    stream->writeText("</td>");
}

static void print_image_cell (SkFILEWStream* stream,
                              const SkString& directory,
                              const SkString& filename,
                              int height) {
    stream->writeText("<td><a href=\"");
    stream->writeText(directory.c_str());
    stream->writeText(filename.c_str());
    stream->writeText("\"><img src=\"");
    stream->writeText(directory.c_str());
    stream->writeText(filename.c_str());
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

    const SkString localDir ("");
    SkString outputPath (outputDir);
    outputPath.append("index.html");
    //SkFILEWStream outputStream ("index.html");
    SkFILEWStream outputStream (outputPath.c_str());

    // FIXME this doesn't work if there are '..' inside the outputDir
    unsigned int ui;
    SkString relativePath;
    for (ui = 0; ui < outputDir.size(); ui++) {
        if (outputDir[ui] == '/') {
            relativePath.append("../");
        }
    }
    SkString relativeBaseDir (relativePath);
    SkString relativeComparisonDir (relativePath);
    relativeBaseDir.append(baseDir);
    relativeComparisonDir.append(comparisonDir);

    outputStream.writeText("<html>\n<body>\n");
    print_page_header(&outputStream, matchCount, colorThreshold, differences);

    outputStream.writeText("<table>\n");
    int i;
    for (i = 0; i < differences.count(); i++) {
        DiffRecord* diff = differences[i];
        if (0 == diff->fFractionDifference) {
            continue;
        }
        int height = compute_image_height(diff->fBaseBitmap);
        outputStream.writeText("<tr>\n");
        print_label_cell(&outputStream, *diff);
        print_image_cell(&outputStream, relativeBaseDir,
                         diff->fFilename, height);
        print_image_cell(&outputStream, localDir,
                         filename_to_diff_filename(diff->fFilename), height);
        print_image_cell(&outputStream, relativeComparisonDir,
                         diff->fFilename, height);
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
"    -white: force all difference pixels to white\n"
"    -threshold n: only report differences > n (in one channel) [default 0]\n"
"    -sortbymismatch: sort by average color channel mismatch\n");
    SkDebugf(
"    -sortbymaxmismatch: sort by worst color channel mismatch,\n"
"                        break ties with -sortbymismatch,\n"
"        [default by fraction of pixels mismatching]\n");
    SkDebugf(
"    -weighted: sort by # pixels different weighted by color difference\n");
    SkDebugf("    baseDir: directory to read baseline images from\n");
    SkDebugf("    comparisonDir: directory to read comparison images from\n");
    SkDebugf(
"    outputDir: directory to write difference images to; defaults to\n"
"               comparisonDir\n");
    SkDebugf("Also creates an \"index.html\" file in the current directory.\n");
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

    RecordArray differences;
    DiffSummary summary;

    int i, j;
    for (i = 1, j = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-help")) {
            usage(argv[0]);
            return 0;
        }
        if (!strcmp(argv[i], "-white")) {
            diffProc = compute_diff_white;
            continue;
        }
        if (!strcmp(argv[i], "-sortbymismatch")) {
            sortProc = (SkQSortCompareProc) compare_diff_mean_mismatches;
            continue;
        }
        if (!strcmp(argv[i], "-sortbymaxmismatch")) {
            sortProc = (SkQSortCompareProc) compare_diff_max_mismatches;
            continue;
        }
        if (!strcmp(argv[i], "-weighted")) {
            sortProc = (SkQSortCompareProc) compare_diff_weighted;
            continue;
        }
        if (!strcmp(argv[i], "-threshold")) {
            colorThreshold = atoi(argv[++i]);
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
    if (j == 2) {
        outputDir = comparisonDir;
    } else if (j != 3) {
        usage(argv[0]);
        return 0;
    }

    if (!baseDir.endsWith("/")) {
        baseDir.append("/");
    }
    if (!comparisonDir.endsWith("/")) {
        comparisonDir.append("/");
    }
    if (!outputDir.endsWith("/")) {
        outputDir.append("/");
    }

    create_diff_images(diffProc, sortProc, colorThreshold, &differences,
                       baseDir, comparisonDir, outputDir, &summary);
    summary.print();
    print_diff_page(summary.fNumMatches, colorThreshold, differences,
                    baseDir, comparisonDir, outputDir);

    for (i = 0; i < differences.count(); i++) {
        delete differences[i];
    }
}
