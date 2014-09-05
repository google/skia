/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "skdiff.h"
#include "skdiff_utils.h"
#include "SkBitmap.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkTDArray.h"
#include "SkTemplates.h"
#include "SkTypes.h"

#include <stdio.h>

/// If outputDir.isEmpty(), don't write out diff files.
static void create_diff_images (DiffMetricProc dmp,
                                const int colorThreshold,
                                const SkString& baseFile,
                                const SkString& comparisonFile,
                                const SkString& outputDir,
                                const SkString& outputFilename,
                                DiffRecord* drp) {
    SkASSERT(!baseFile.isEmpty());
    SkASSERT(!comparisonFile.isEmpty());

    drp->fBase.fFilename = baseFile;
    drp->fBase.fFullPath = baseFile;
    drp->fBase.fStatus = DiffResource::kSpecified_Status;

    drp->fComparison.fFilename = comparisonFile;
    drp->fComparison.fFullPath = comparisonFile;
    drp->fComparison.fStatus = DiffResource::kSpecified_Status;

    SkAutoDataUnref baseFileBits(read_file(drp->fBase.fFullPath.c_str()));
    if (baseFileBits) {
        drp->fBase.fStatus = DiffResource::kRead_Status;
    }
    SkAutoDataUnref comparisonFileBits(read_file(drp->fComparison.fFullPath.c_str()));
    if (comparisonFileBits) {
        drp->fComparison.fStatus = DiffResource::kRead_Status;
    }
    if (NULL == baseFileBits || NULL == comparisonFileBits) {
        if (NULL == baseFileBits) {
            drp->fBase.fStatus = DiffResource::kCouldNotRead_Status;
        }
        if (NULL == comparisonFileBits) {
            drp->fComparison.fStatus = DiffResource::kCouldNotRead_Status;
        }
        drp->fResult = DiffRecord::kCouldNotCompare_Result;
        return;
    }

    if (are_buffers_equal(baseFileBits, comparisonFileBits)) {
        drp->fResult = DiffRecord::kEqualBits_Result;
        return;
    }

    get_bitmap(baseFileBits, drp->fBase, SkImageDecoder::kDecodePixels_Mode);
    get_bitmap(comparisonFileBits, drp->fComparison, SkImageDecoder::kDecodePixels_Mode);
    if (DiffResource::kDecoded_Status != drp->fBase.fStatus ||
        DiffResource::kDecoded_Status != drp->fComparison.fStatus)
    {
        drp->fResult = DiffRecord::kCouldNotCompare_Result;
        return;
    }

    create_and_write_diff_image(drp, dmp, colorThreshold, outputDir, outputFilename);
    //TODO: copy fBase.fFilename and fComparison.fFilename to outputDir
    //      svn and git often present tmp files to diff tools which are promptly deleted

    //TODO: serialize drp to outputDir
    //      write a tool to deserialize them and call print_diff_page

    SkASSERT(DiffRecord::kUnknown_Result != drp->fResult);
}

static void usage (char * argv0) {
    SkDebugf("Skia image diff tool\n");
    SkDebugf("\n"
"Usage: \n"
"    %s <baseFile> <comparisonFile>\n" , argv0);
    SkDebugf(
"\nArguments:"
"\n    --failonresult <result>: After comparing all file pairs, exit with nonzero"
"\n                             return code (number of file pairs yielding this"
"\n                             result) if any file pairs yielded this result."
"\n                             This flag may be repeated, in which case the"
"\n                             return code will be the number of fail pairs"
"\n                             yielding ANY of these results."
"\n    --failonstatus <baseStatus> <comparisonStatus>: exit with nonzero return"
"\n                             code if any file pairs yeilded this status."
"\n    --help: display this info"
"\n    --listfilenames: list all filenames for each result type in stdout"
"\n    --nodiffs: don't write out image diffs, just generate report on stdout"
"\n    --outputdir: directory to write difference images"
"\n    --threshold <n>: only report differences > n (per color channel) [default 0]"
"\n    -u: ignored. Recognized for compatibility with svn diff."
"\n    -L: first occurrence label for base, second occurrence label for comparison."
"\n        Labels must be of the form \"<filename>(\t<specifier>)?\"."
"\n        The base <filename> will be used to create files in outputdir."
"\n"
"\n    baseFile: baseline image file."
"\n    comparisonFile: comparison image file"
"\n"
"\nIf no sort is specified, it will sort by fraction of pixels mismatching."
"\n");
}

const int kNoError = 0;
const int kGenericError = -1;

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    DiffMetricProc diffProc = compute_diff_pmcolor;

    // Maximum error tolerated in any one color channel in any one pixel before
    // a difference is reported.
    int colorThreshold = 0;
    SkString baseFile;
    SkString baseLabel;
    SkString comparisonFile;
    SkString comparisonLabel;
    SkString outputDir;

    bool listFilenames = false;

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
    int numLabelArguments = 0;
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
        if (!strcmp(argv[i], "--outputdir")) {
            if (argc == ++i) {
                SkDebugf("outputdir expects one argument.\n");
                continue;
            }
            outputDir.set(argv[i]);
            continue;
        }
        if (!strcmp(argv[i], "--threshold")) {
            colorThreshold = atoi(argv[++i]);
            continue;
        }
        if (!strcmp(argv[i], "-u")) {
            //we don't produce unified diffs, ignore parameter to work with svn diff
            continue;
        }
        if (!strcmp(argv[i], "-L")) {
            if (argc == ++i) {
                SkDebugf("label expects one argument.\n");
                continue;
            }
            switch (numLabelArguments++) {
                case 0:
                    baseLabel.set(argv[i]);
                    continue;
                case 1:
                    comparisonLabel.set(argv[i]);
                    continue;
                default:
                    SkDebugf("extra label argument <%s>\n", argv[i]);
                    usage(argv[0]);
                    return kGenericError;
            }
            continue;
        }
        if (argv[i][0] != '-') {
            switch (numUnflaggedArguments++) {
                case 0:
                    baseFile.set(argv[i]);
                    continue;
                case 1:
                    comparisonFile.set(argv[i]);
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

    if (numUnflaggedArguments != 2) {
        usage(argv[0]);
        return kGenericError;
    }

    if (listFilenames) {
        printf("Base file is [%s]\n", baseFile.c_str());
    }

    if (listFilenames) {
        printf("Comparison file is [%s]\n", comparisonFile.c_str());
    }

    if (outputDir.isEmpty()) {
        if (listFilenames) {
            printf("Not writing any diffs. No output dir specified.\n");
        }
    } else {
        if (!outputDir.endsWith(PATH_DIV_STR)) {
            outputDir.append(PATH_DIV_STR);
        }
        if (listFilenames) {
            printf("Writing diffs. Output dir is [%s]\n", outputDir.c_str());
        }
    }

    // Some obscure documentation about diff/patch labels:
    //
    // Posix says the format is: <filename><tab><date>
    //     It also states that if a filename contains <tab> or <newline>
    //     the result is implementation defined
    //
    // Svn diff --diff-cmd provides labels of the form: <filename><tab><revision>
    //
    // Git diff --ext-diff does not supply arguments compatible with diff.
    //     However, it does provide the filename directly.
    //     skimagediff_git.sh: skimagediff %2 %5 -L "%1\t(%3)" -L "%1\t(%6)"
    //
    // Git difftool sets $LOCAL, $REMOTE, $MERGED, and $BASE instead of command line parameters.
    //     difftool.<>.cmd: skimagediff $LOCAL $REMOTE -L "$MERGED\t(local)" -L "$MERGED\t(remote)"
    //
    // Diff will write any specified label verbatim. Without a specified label diff will write
    //     <filename><tab><date>
    //     However, diff will encode the filename as a cstring if the filename contains
    //         Any of <space> or <double quote>
    //         A char less than 32
    //         Any escapable character \\, \a, \b, \t, \n, \v, \f, \r
    //
    // Patch decodes:
    //     If first <non-white-space> is <double quote>, parse filename from cstring.
    //     If there is a <tab> after the first <non-white-space>, filename is
    //         [first <non-white-space>, the next run of <white-space> with an embedded <tab>).
    //     Otherwise the filename is [first <non-space>, the next <white-space>).
    //
    // The filename /dev/null means the file does not exist (used in adds and deletes).

    // Considering the above, skimagediff will consider the contents of a -L parameter as
    //     <filename>(\t<specifier>)?
    SkString outputFile;

    if (baseLabel.isEmpty()) {
        baseLabel.set(baseFile);
        outputFile = baseLabel;
    } else {
        const char* baseLabelCstr = baseLabel.c_str();
        const char* tab = strchr(baseLabelCstr, '\t');
        if (NULL == tab) {
            outputFile = baseLabel;
        } else {
            outputFile.set(baseLabelCstr, tab - baseLabelCstr);
        }
    }
    if (comparisonLabel.isEmpty()) {
        comparisonLabel.set(comparisonFile);
    }
    printf("Base:       %s\n", baseLabel.c_str());
    printf("Comparison: %s\n", comparisonLabel.c_str());

    DiffRecord dr;
    create_diff_images(diffProc, colorThreshold, baseFile, comparisonFile, outputDir, outputFile,
                       &dr);

    if (DiffResource::isStatusFailed(dr.fBase.fStatus)) {
        printf("Base %s.\n", DiffResource::getStatusDescription(dr.fBase.fStatus));
    }
    if (DiffResource::isStatusFailed(dr.fComparison.fStatus)) {
        printf("Comparison %s.\n", DiffResource::getStatusDescription(dr.fComparison.fStatus));
    }
    printf("Base and Comparison %s.\n", DiffRecord::getResultDescription(dr.fResult));

    if (DiffRecord::kDifferentPixels_Result == dr.fResult) {
        printf("%.4f%% of pixels differ", 100 * dr.fFractionDifference);
        printf(" (%.4f%%  weighted)", 100 * dr.fWeightedFraction);
        if (dr.fFractionDifference < 0.01) {
            printf(" %d pixels", static_cast<int>(dr.fFractionDifference *
                                                  dr.fBase.fBitmap.width() *
                                                  dr.fBase.fBitmap.height()));
        }

        printf("\nAverage color mismatch: ");
        printf("%d", static_cast<int>(MAX3(dr.fAverageMismatchR,
                                           dr.fAverageMismatchG,
                                           dr.fAverageMismatchB)));
        printf("\nMax color mismatch: ");
        printf("%d", MAX3(dr.fMaxMismatchR,
                          dr.fMaxMismatchG,
                          dr.fMaxMismatchB));
        printf("\n");
    }
    printf("\n");

    int num_failing_results = 0;
    if (failOnResultType[dr.fResult]) {
        ++num_failing_results;
    }
    if (failOnStatusType[dr.fBase.fStatus][dr.fComparison.fStatus]) {
        ++num_failing_results;
    }

    return num_failing_results;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
