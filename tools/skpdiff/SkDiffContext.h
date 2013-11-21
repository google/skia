/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDiffContext_DEFINED
#define SkDiffContext_DEFINED

#include "SkImageDiffer.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "SkTLList.h"
#include "SkThread.h"

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
     * Creates the directory if it does not exist and uses it to store differences
     * between images.
     */
    void setDifferenceDir(const SkString& directory);

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
     *    "commonName"     : string containing the common prefix of the baselinePath
     *                       and testPath filenames
     *    "baselinePath"   : string containing the path to the baseline image
     *    "testPath"       : string containing the path to the test image
     *    "differencePath" : (optional) string containing the path to an alpha
     *                       mask of the pixel difference between the baseline
     *                       and test images
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
        SkString           fCommonName;
        SkString           fDifferencePath;
        SkString           fBaselinePath;
        SkString               fTestPath;
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

    SkString fDifferenceDir;
};

#endif
