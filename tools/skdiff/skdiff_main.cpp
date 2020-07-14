/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkStream.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTSearch.h"
#include "src/utils/SkOSPath.h"
#include "tools/skdiff/skdiff.h"
#include "tools/skdiff/skdiff_html.h"
#include "tools/skdiff/skdiff_utils.h"

#include <stdlib.h>

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

typedef SkTDArray<SkString*> StringArray;
typedef StringArray FileArray;

static void add_unique_basename(StringArray* array, const SkString& filename) {
    // trim off dirs
    const char* src = filename.c_str();
    const char* trimmed = strrchr(src, SkOSPath::SEPARATOR);
    if (trimmed) {
        trimmed += 1;   // skip the separator
    } else {
        trimmed = src;
    }
    const char* end = strrchr(trimmed, '.');
    if (!end) {
        end = trimmed + strlen(trimmed);
    }
    SkString result(trimmed, end - trimmed);

    // only add unique entries
    for (int i = 0; i < array->count(); ++i) {
        if (*array->getAt(i) == result) {
            return;
        }
    }
    *array->append() = new SkString(result);
}

struct DiffSummary {
    DiffSummary ()
        : fNumMatches(0)
        , fNumMismatches(0)
        , fMaxMismatchV(0)
        , fMaxMismatchPercent(0) { }

    ~DiffSummary() {
        for (int i = 0; i < DiffRecord::kResultCount; ++i) {
            fResultsOfType[i].deleteAll();
        }
        for (int base = 0; base < DiffResource::kStatusCount; ++base) {
            for (int comparison = 0; comparison < DiffResource::kStatusCount; ++comparison) {
                fStatusOfType[base][comparison].deleteAll();
            }
        }
    }

    uint32_t fNumMatches;
    uint32_t fNumMismatches;
    uint32_t fMaxMismatchV;
    float fMaxMismatchPercent;

    FileArray fResultsOfType[DiffRecord::kResultCount];
    FileArray fStatusOfType[DiffResource::kStatusCount][DiffResource::kStatusCount];

    StringArray fFailedBaseNames[DiffRecord::kResultCount];

    void printContents(const FileArray& fileArray,
                       const char* baseStatus, const char* comparisonStatus,
                       bool listFilenames) {
        int n = fileArray.count();
        printf("%d file pairs %s in baseDir and %s in comparisonDir",
                n,            baseStatus,       comparisonStatus);
        if (listFilenames) {
            printf(": ");
            for (int i = 0; i < n; ++i) {
                printf("%s ", fileArray[i]->c_str());
            }
        }
        printf("\n");
    }

    void printStatus(bool listFilenames,
                     bool failOnStatusType[DiffResource::kStatusCount]
                                          [DiffResource::kStatusCount]) {
        typedef DiffResource::Status Status;

        for (int base = 0; base < DiffResource::kStatusCount; ++base) {
            Status baseStatus = static_cast<Status>(base);
            for (int comparison = 0; comparison < DiffResource::kStatusCount; ++comparison) {
                Status comparisonStatus = static_cast<Status>(comparison);
                const FileArray& fileArray = fStatusOfType[base][comparison];
                if (fileArray.count() > 0) {
                    if (failOnStatusType[base][comparison]) {
                        printf("   [*] ");
                    } else {
                        printf("   [_] ");
                    }
                    printContents(fileArray,
                                  DiffResource::getStatusDescription(baseStatus),
                                  DiffResource::getStatusDescription(comparisonStatus),
                                  listFilenames);
                }
            }
        }
    }

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

    void print(bool listFilenames, bool failOnResultType[DiffRecord::kResultCount],
               bool failOnStatusType[DiffResource::kStatusCount]
                                    [DiffResource::kStatusCount]) {
        printf("\ncompared %d file pairs:\n", fNumMatches + fNumMismatches);
        for (int resultInt = 0; resultInt < DiffRecord::kResultCount; ++resultInt) {
            DiffRecord::Result result = static_cast<DiffRecord::Result>(resultInt);
            if (failOnResultType[result]) {
                printf("[*] ");
            } else {
                printf("[_] ");
            }
            printContents(fResultsOfType[result], DiffRecord::getResultDescription(result),
                          listFilenames);
            if (DiffRecord::kCouldNotCompare_Result == result) {
                printStatus(listFilenames, failOnStatusType);
            }
        }
        printf("(results marked with [*] will cause nonzero return value)\n");
        printf("\nnumber of mismatching file pairs: %d\n", fNumMismatches);
        if (fNumMismatches > 0) {
            printf("Maximum pixel intensity mismatch %d\n", fMaxMismatchV);
            printf("Largest area mismatch was %.2f%% of pixels\n",fMaxMismatchPercent);
        }
    }

    void printfFailingBaseNames(const char separator[]) {
        for (int resultInt = 0; resultInt < DiffRecord::kResultCount; ++resultInt) {
            const StringArray& array = fFailedBaseNames[resultInt];
            if (array.count()) {
                printf("%s [%d]%s", DiffRecord::ResultNames[resultInt], array.count(), separator);
                for (int j = 0; j < array.count(); ++j) {
                    printf("%s%s", array[j]->c_str(), separator);
                }
                printf("\n");
            }
        }
    }

    void add (DiffRecord* drp) {
        uint32_t mismatchValue;

        if (drp->fBase.fFilename.equals(drp->fComparison.fFilename)) {
            fResultsOfType[drp->fResult].push_back(new SkString(drp->fBase.fFilename));
        } else {
            SkString* blame = new SkString("(");
            blame->append(drp->fBase.fFilename);
            blame->append(", ");
            blame->append(drp->fComparison.fFilename);
            blame->append(")");
            fResultsOfType[drp->fResult].push_back(blame);
        }
        switch (drp->fResult) {
          case DiffRecord::kEqualBits_Result:
            fNumMatches++;
            break;
          case DiffRecord::kEqualPixels_Result:
            fNumMatches++;
            break;
          case DiffRecord::kDifferentSizes_Result:
            fNumMismatches++;
            break;
          case DiffRecord::kDifferentPixels_Result:
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
          case DiffRecord::kCouldNotCompare_Result:
            fNumMismatches++;
            fStatusOfType[drp->fBase.fStatus][drp->fComparison.fStatus].push_back(
                    new SkString(drp->fBase.fFilename));
            break;
          case DiffRecord::kUnknown_Result:
            SkDEBUGFAIL("adding uncategorized DiffRecord");
            break;
          default:
            SkDEBUGFAIL("adding DiffRecord with unhandled fResult value");
            break;
        }

        switch (drp->fResult) {
            case DiffRecord::kEqualBits_Result:
            case DiffRecord::kEqualPixels_Result:
                break;
            default:
                add_unique_basename(&fFailedBaseNames[drp->fResult], drp->fBase.fFilename);
                break;
        }
    }
};

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
            files->push_back(new SkString(pathRelativeToRootDir));
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

class AutoReleasePixels {
public:
    AutoReleasePixels(DiffRecord* drp)
    : fDrp(drp) {
        SkASSERT(drp != nullptr);
    }
    ~AutoReleasePixels() {
        fDrp->fBase.fBitmap.setPixelRef(nullptr, 0, 0);
        fDrp->fComparison.fBitmap.setPixelRef(nullptr, 0, 0);
        fDrp->fDifference.fBitmap.setPixelRef(nullptr, 0, 0);
        fDrp->fWhite.fBitmap.setPixelRef(nullptr, 0, 0);
    }

private:
    DiffRecord* fDrp;
};

static void get_bounds(DiffResource& resource, const char* name) {
    if (resource.fBitmap.empty() && !DiffResource::isStatusFailed(resource.fStatus)) {
        sk_sp<SkData> fileBits(read_file(resource.fFullPath.c_str()));
        if (fileBits) {
            get_bitmap(fileBits, resource, true, true);
        } else {
            SkDebugf("WARNING: couldn't read %s file <%s>\n", name, resource.fFullPath.c_str());
            resource.fStatus = DiffResource::kCouldNotRead_Status;
        }
    }
}

static void get_bounds(DiffRecord& drp) {
    get_bounds(drp.fBase, "base");
    get_bounds(drp.fComparison, "comparison");
}

#ifdef SK_OS_WIN
#define ANSI_COLOR_RED     ""
#define ANSI_COLOR_GREEN   ""
#define ANSI_COLOR_YELLOW  ""
#define ANSI_COLOR_RESET   ""
#else
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#endif

#define VERBOSE_STATUS(status,color,filename) if (verbose) printf( "[ " color " %10s " ANSI_COLOR_RESET " ] %s\n", status, filename->c_str())

/// Creates difference images, returns the number that have a 0 metric.
/// If outputDir.isEmpty(), don't write out diff files.
static void create_diff_images (DiffMetricProc dmp,
                                const int colorThreshold,
                                bool ignoreColorSpace,
                                RecordArray* differences,
                                const SkString& baseDir,
                                const SkString& comparisonDir,
                                const SkString& outputDir,
                                const StringArray& matchSubstrings,
                                const StringArray& nomatchSubstrings,
                                bool recurseIntoSubdirs,
                                bool getBounds,
                                bool verbose,
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

    if (!outputDir.isEmpty()) {
        sk_mkdir(outputDir.c_str());
    }

    int i = 0;
    int j = 0;

    while (i < baseFiles.count() &&
           j < comparisonFiles.count()) {

        SkString basePath(baseDir);
        SkString comparisonPath(comparisonDir);

        DiffRecord *drp = new DiffRecord;
        int v = strcmp(baseFiles[i]->c_str(), comparisonFiles[j]->c_str());

        if (v < 0) {
            // in baseDir, but not in comparisonDir
            drp->fResult = DiffRecord::kCouldNotCompare_Result;

            basePath.append(*baseFiles[i]);
            comparisonPath.append(*baseFiles[i]);

            drp->fBase.fFilename = *baseFiles[i];
            drp->fBase.fFullPath = basePath;
            drp->fBase.fStatus = DiffResource::kExists_Status;

            drp->fComparison.fFilename = *baseFiles[i];
            drp->fComparison.fFullPath = comparisonPath;
            drp->fComparison.fStatus = DiffResource::kDoesNotExist_Status;

            VERBOSE_STATUS("MISSING", ANSI_COLOR_YELLOW, baseFiles[i]);

            ++i;
        } else if (v > 0) {
            // in comparisonDir, but not in baseDir
            drp->fResult = DiffRecord::kCouldNotCompare_Result;

            basePath.append(*comparisonFiles[j]);
            comparisonPath.append(*comparisonFiles[j]);

            drp->fBase.fFilename = *comparisonFiles[j];
            drp->fBase.fFullPath = basePath;
            drp->fBase.fStatus = DiffResource::kDoesNotExist_Status;

            drp->fComparison.fFilename = *comparisonFiles[j];
            drp->fComparison.fFullPath = comparisonPath;
            drp->fComparison.fStatus = DiffResource::kExists_Status;

            VERBOSE_STATUS("MISSING", ANSI_COLOR_YELLOW, comparisonFiles[j]);

            ++j;
        } else {
            // Found the same filename in both baseDir and comparisonDir.
            SkASSERT(DiffRecord::kUnknown_Result == drp->fResult);

            basePath.append(*baseFiles[i]);
            comparisonPath.append(*comparisonFiles[j]);

            drp->fBase.fFilename = *baseFiles[i];
            drp->fBase.fFullPath = basePath;
            drp->fBase.fStatus = DiffResource::kExists_Status;

            drp->fComparison.fFilename = *comparisonFiles[j];
            drp->fComparison.fFullPath = comparisonPath;
            drp->fComparison.fStatus = DiffResource::kExists_Status;

            sk_sp<SkData> baseFileBits(read_file(drp->fBase.fFullPath.c_str()));
            if (baseFileBits) {
                drp->fBase.fStatus = DiffResource::kRead_Status;
            }
            sk_sp<SkData> comparisonFileBits(read_file(drp->fComparison.fFullPath.c_str()));
            if (comparisonFileBits) {
                drp->fComparison.fStatus = DiffResource::kRead_Status;
            }
            if (nullptr == baseFileBits || nullptr == comparisonFileBits) {
                if (nullptr == baseFileBits) {
                    drp->fBase.fStatus = DiffResource::kCouldNotRead_Status;
                    VERBOSE_STATUS("READ FAIL", ANSI_COLOR_RED, baseFiles[i]);
                }
                if (nullptr == comparisonFileBits) {
                    drp->fComparison.fStatus = DiffResource::kCouldNotRead_Status;
                    VERBOSE_STATUS("READ FAIL", ANSI_COLOR_RED, comparisonFiles[j]);
                }
                drp->fResult = DiffRecord::kCouldNotCompare_Result;

            } else if (are_buffers_equal(baseFileBits.get(), comparisonFileBits.get())) {
                drp->fResult = DiffRecord::kEqualBits_Result;
                VERBOSE_STATUS("MATCH", ANSI_COLOR_GREEN, baseFiles[i]);
            } else {
                AutoReleasePixels arp(drp);
                get_bitmap(baseFileBits, drp->fBase, false, ignoreColorSpace);
                get_bitmap(comparisonFileBits, drp->fComparison, false, ignoreColorSpace);
                VERBOSE_STATUS("DIFFERENT", ANSI_COLOR_RED, baseFiles[i]);
                if (DiffResource::kDecoded_Status == drp->fBase.fStatus &&
                    DiffResource::kDecoded_Status == drp->fComparison.fStatus) {
                    create_and_write_diff_image(drp, dmp, colorThreshold,
                                                outputDir, drp->fBase.fFilename);
                } else {
                    drp->fResult = DiffRecord::kCouldNotCompare_Result;
                }
            }

            ++i;
            ++j;
        }

        if (getBounds) {
            get_bounds(*drp);
        }
        SkASSERT(DiffRecord::kUnknown_Result != drp->fResult);
        differences->push_back(drp);
        summary->add(drp);
    }

    for (; i < baseFiles.count(); ++i) {
        // files only in baseDir
        DiffRecord *drp = new DiffRecord();
        drp->fBase.fFilename = *baseFiles[i];
        drp->fBase.fFullPath = baseDir;
        drp->fBase.fFullPath.append(drp->fBase.fFilename);
        drp->fBase.fStatus = DiffResource::kExists_Status;

        drp->fComparison.fFilename = *baseFiles[i];
        drp->fComparison.fFullPath = comparisonDir;
        drp->fComparison.fFullPath.append(drp->fComparison.fFilename);
        drp->fComparison.fStatus = DiffResource::kDoesNotExist_Status;

        drp->fResult = DiffRecord::kCouldNotCompare_Result;
        if (getBounds) {
            get_bounds(*drp);
        }
        differences->push_back(drp);
        summary->add(drp);
    }

    for (; j < comparisonFiles.count(); ++j) {
        // files only in comparisonDir
        DiffRecord *drp = new DiffRecord();
        drp->fBase.fFilename = *comparisonFiles[j];
        drp->fBase.fFullPath = baseDir;
        drp->fBase.fFullPath.append(drp->fBase.fFilename);
        drp->fBase.fStatus = DiffResource::kDoesNotExist_Status;

        drp->fComparison.fFilename = *comparisonFiles[j];
        drp->fComparison.fFullPath = comparisonDir;
        drp->fComparison.fFullPath.append(drp->fComparison.fFilename);
        drp->fComparison.fStatus = DiffResource::kExists_Status;

        drp->fResult = DiffRecord::kCouldNotCompare_Result;
        if (getBounds) {
            get_bounds(*drp);
        }
        differences->push_back(drp);
        summary->add(drp);
    }

    release_file_list(&baseFiles);
    release_file_list(&comparisonFiles);
}

static void usage (char * argv0) {
    SkDebugf("Skia baseline image diff tool\n");
    SkDebugf("\n"
"Usage: \n"
"    %s <baseDir> <comparisonDir> [outputDir] \n", argv0);
    SkDebugf(
"\nArguments:"
"\n    --failonresult <result>: After comparing all file pairs, exit with nonzero"
"\n                             return code (number of file pairs yielding this"
"\n                             result) if any file pairs yielded this result."
"\n                             This flag may be repeated, in which case the"
"\n                             return code will be the number of fail pairs"
"\n                             yielding ANY of these results."
"\n    --failonstatus <baseStatus> <comparisonStatus>: exit with nonzero return"
"\n                             code if any file pairs yielded this status."
"\n    --help: display this info"
"\n    --listfilenames: list all filenames for each result type in stdout"
"\n    --match <substring>: compare files whose filenames contain this substring;"
"\n                         if unspecified, compare ALL files."
"\n                         this flag may be repeated."
"\n    --nocolorspace: Ignore color space of images."
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

int main(int argc, char** argv) {
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
    bool verbose = false;
    bool listFailingBase = false;
    bool ignoreColorSpace = false;

    RecordArray differences;
    DiffSummary summary;

    bool failOnResultType[DiffRecord::kResultCount];
    for (int i = 0; i < DiffRecord::kResultCount; i++) {
        failOnResultType[i] = false;
    }

    bool failOnStatusType[DiffResource::kStatusCount][DiffResource::kStatusCount];
    for (int base = 0; base < DiffResource::kStatusCount; ++base) {
        for (int comparison = 0; comparison < DiffResource::kStatusCount; ++comparison) {
            failOnStatusType[base][comparison] = false;
        }
    }

    int i;
    int numUnflaggedArguments = 0;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--failonresult")) {
            if (argc == ++i) {
                SkDebugf("failonresult expects one argument.\n");
                continue;
            }
            DiffRecord::Result type = DiffRecord::getResultByName(argv[i]);
            if (type != DiffRecord::kResultCount) {
                failOnResultType[type] = true;
            } else {
                SkDebugf("ignoring unrecognized result <%s>\n", argv[i]);
            }
            continue;
        }
        if (!strcmp(argv[i], "--failonstatus")) {
            if (argc == ++i) {
                SkDebugf("failonstatus missing base status.\n");
                continue;
            }
            bool baseStatuses[DiffResource::kStatusCount];
            if (!DiffResource::getMatchingStatuses(argv[i], baseStatuses)) {
                SkDebugf("unrecognized base status <%s>\n", argv[i]);
            }

            if (argc == ++i) {
                SkDebugf("failonstatus missing comparison status.\n");
                continue;
            }
            bool comparisonStatuses[DiffResource::kStatusCount];
            if (!DiffResource::getMatchingStatuses(argv[i], comparisonStatuses)) {
                SkDebugf("unrecognized comarison status <%s>\n", argv[i]);
            }

            for (int base = 0; base < DiffResource::kStatusCount; ++base) {
                for (int comparison = 0; comparison < DiffResource::kStatusCount; ++comparison) {
                    failOnStatusType[base][comparison] |=
                        baseStatuses[base] && comparisonStatuses[comparison];
                }
            }
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
        if (!strcmp(argv[i], "--verbose")) {
            verbose = true;
            continue;
        }
        if (!strcmp(argv[i], "--match")) {
            matchSubstrings.push_back(new SkString(argv[++i]));
            continue;
        }
        if (!strcmp(argv[i], "--nocolorspace")) {
            ignoreColorSpace = true;
            continue;
        }
        if (!strcmp(argv[i], "--nodiffs")) {
            generateDiffs = false;
            continue;
        }
        if (!strcmp(argv[i], "--nomatch")) {
            nomatchSubstrings.push_back(new SkString(argv[++i]));
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
        if (!strcmp(argv[i], "--listFailingBase")) {
            listFailingBase = true;
            continue;
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
        matchSubstrings.push_back(new SkString(""));
    }

    create_diff_images(diffProc, colorThreshold, ignoreColorSpace, &differences,
                       baseDir, comparisonDir, outputDir,
                       matchSubstrings, nomatchSubstrings, recurseIntoSubdirs, generateDiffs,
                       verbose, &summary);
    summary.print(listFilenames, failOnResultType, failOnStatusType);

    if (listFailingBase) {
        summary.printfFailingBaseNames("\n");
    }

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
    for (int i = 0; i < DiffRecord::kResultCount; i++) {
        if (failOnResultType[i]) {
            num_failing_results += summary.fResultsOfType[i].count();
        }
    }
    if (!failOnResultType[DiffRecord::kCouldNotCompare_Result]) {
        for (int base = 0; base < DiffResource::kStatusCount; ++base) {
            for (int comparison = 0; comparison < DiffResource::kStatusCount; ++comparison) {
                if (failOnStatusType[base][comparison]) {
                    num_failing_results += summary.fStatusOfType[base][comparison].count();
                }
            }
        }
    }

    // On Linux (and maybe other platforms too), any results outside of the
    // range [0...255] are wrapped (mod 256).  Do the conversion ourselves, to
    // make sure that we only return 0 when there were no failures.
    return (num_failing_results > 255) ? 255 : num_failing_results;
}
