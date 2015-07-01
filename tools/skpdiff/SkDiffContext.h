/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDiffContext_DEFINED
#define SkDiffContext_DEFINED

#include "SkImageDiffer.h"
#include "SkMutex.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "SkTLList.h"

class SkWStream;

/**
 * Collects records of diffs and outputs them as JSON.
 */
class SkDiffContext {
public:
    SkDiffContext();
    ~SkDiffContext();

    void setThreadCount(int threadCount) { fThreadCount = threadCount; }

    /**
     * Sets the directory within which to store alphaMasks (images that
     * are transparent for each pixel that differs between baseline and test).
     *
     * If the directory does not exist yet, it will be created.
     */
    void setAlphaMaskDir(const SkString& directory);

    /**
     * Sets the directory within which to store rgbDiffs (images showing the
     * per-channel difference between baseline and test at each pixel).
     *
     * If the directory does not exist yet, it will be created.
     */
    void setRgbDiffDir(const SkString& directory);

    /**
     * Sets the directory within which to store whiteDiffs (images showing white
     * for each pixel that differs between baseline and test).
     *
     * If the directory does not exist yet, it will be created.
     */
    void setWhiteDiffDir(const SkString& directory);

    /**
     * Modify the pattern used to generate commonName (= the
     * basename of rgb/white diff files).
     *
     * - true: basename is a combination of the input file names.
     * - false: basename is the common prefix of the input file names.
     *
     * For example, for:
     *   baselinePath=/tmp/dir/image-before.png
     *   testPath=/tmp/dir/image-after.png
     *
     * If setLongNames(true), commonName would be:
     *    image-before-png-vs-image-after-png.png
     *
     * If setLongNames(false), commonName would be:
     *   image-.png
     */
    void setLongNames(const bool useLongNames);

    /**
     * Sets the differs to be used in each diff. Already started diffs will not retroactively use
     * these.
     * @param differs An array of differs to use. The array is copied, but not the differs
     *                themselves.
     */
    void setDiffers(const SkTDArray<SkImageDiffer*>& differs);

    /**
     * Compares two directories of images with the given differ
     * @param baselinePath The baseline directory's path
     * @param testPath     The test directory's path
     */
    void diffDirectories(const char baselinePath[], const char testPath[]);

    /**
     * Compares two sets of images identified by glob style patterns with the given differ
     * @param baselinePattern A pattern for baseline files
     * @param testPattern     A pattern for test files that matches each file of the baseline file
     */
    void diffPatterns(const char baselinePattern[], const char testPattern[]);

    /**
     * Compares the images at the given paths
     * @param baselinePath The baseline file path
     * @param testPath     The matching test file path
     */
    void addDiff(const char* baselinePath, const char* testPath);

    /**
     * Output the records of each diff in JSON.
     *
     * The format of the JSON document is one top level array named "records".
     * Each record in the array is an object with the following values:
     *    "commonName"     : string containing the output filename (basename)
     *                       depending on the value of 'longNames'.
     *                       (see 'setLongNames' for an explanation and example).
     *    "baselinePath"   : string containing the path to the baseline image
     *    "testPath"       : string containing the path to the test image
     *    "differencePath" : (optional) string containing the path to an alpha
     *                       mask of the pixel difference between the baseline
     *                       and test images
     *                       TODO(epoger): consider renaming this "alphaMaskPath"
     *                       to distinguish from other difference types?
     *    "rgbDiffPath"    : (optional) string containing the path to a bitmap
     *                       showing per-channel differences between the
     *                       baseline and test images at each pixel
     *    "whiteDiffPath"  : (optional) string containing the path to a bitmap
     *                       showing every pixel that differs between the
     *                       baseline and test images as white
     *
     * They also have an array named "diffs" with each element being one diff record for the two
     * images indicated in the above field.
     * A diff record includes:
     *    "differName"       : string name of the diff metric used
     *    "result"           : numerical result of the diff
     *
     * Here is an example:
     *
     * {
     *     "records": [
     *         {
     *             "commonName": "queue.png",
     *             "baselinePath": "/a/queue.png",
     *             "testPath": "/b/queue.png",
     *             "diffs": [
     *                 {
     *                     "differName": "different_pixels",
     *                     "result": 1,
     *                 }
     *             ]
     *         }
     *     ]
     * }
     *
     * @param stream   The stream to output the diff to
     * @param useJSONP True to adding padding to the JSON output to make it cross-site requestable.
     */
    void outputRecords(SkWStream& stream, bool useJSONP);

    /**
     * Output the records score in csv format.
     */
    void outputCsv(SkWStream& stream);


private:
    struct DiffData {
        const char* fDiffName;
        SkImageDiffer::Result fResult;
    };

    struct DiffRecord {
        // TODO(djsollen): Some of these fields are required, while others are optional
        // (e.g., fRgbDiffPath is only filled in if SkDifferentPixelsMetric
        // was run).  Figure out a way to note that.  See http://skbug.com/2712
        // ('allow skpdiff to report different sets of result fields for
        // different comparison algorithms')
        SkString           fCommonName;
        SkString           fAlphaMaskPath;
        SkString           fRgbDiffPath;
        SkString           fWhiteDiffPath;
        SkString           fBaselinePath;
        SkString               fTestPath;
        SkISize                    fSize;
        int                  fMaxRedDiff;
        int                fMaxGreenDiff;
        int                 fMaxBlueDiff;
        SkTArray<DiffData>        fDiffs;
    };

    // Used to protect access to fRecords and ensure only one thread is
    // adding new entries at a time.
    SkMutex fRecordMutex;

    // We use linked list for the records so that their pointers remain stable. A resizable array
    // might change its pointers, which would make it harder for async diffs to record their
    // results.
    SkTLList<DiffRecord> fRecords;

    SkImageDiffer** fDiffers;
    int fDifferCount;
    int fThreadCount;

    SkString fAlphaMaskDir;
    SkString fRgbDiffDir;
    SkString fWhiteDiffDir;
    bool longNames;
};

#endif
