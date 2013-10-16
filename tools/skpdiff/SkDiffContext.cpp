/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkRunnable.h"
#include "SkStream.h"
#include "SkTDict.h"
#include "SkThreadPool.h"

#include "SkDiffContext.h"
#include "SkImageDiffer.h"
#include "skpdiff_util.h"

// Truncates the number of points of interests in JSON output to not freeze the parser
static const int kMaxPOI = 100;

SkDiffContext::SkDiffContext() {
    fRecords = NULL;
    fDiffers = NULL;
    fDifferCount = 0;
    fThreadCount = SkThreadPool::kThreadPerCore;
}

SkDiffContext::~SkDiffContext() {
    // Delete the record linked list
    DiffRecord* currentRecord = fRecords;
    while (NULL != currentRecord) {
        DiffRecord* nextRecord = currentRecord->fNext;
        SkDELETE(currentRecord);
        currentRecord = nextRecord;
    }

    if (NULL != fDiffers) {
        SkDELETE_ARRAY(fDiffers);
    }
}

void SkDiffContext::setDiffers(const SkTDArray<SkImageDiffer*>& differs) {
    // Delete whatever the last array of differs was
    if (NULL != fDiffers) {
        SkDELETE_ARRAY(fDiffers);
        fDiffers = NULL;
        fDifferCount = 0;
    }

    // Copy over the new differs
    fDifferCount = differs.count();
    fDiffers = SkNEW_ARRAY(SkImageDiffer*, fDifferCount);
    differs.copy(fDiffers);
}

void SkDiffContext::addDiff(const char* baselinePath, const char* testPath) {
    // Load the images at the paths
    SkBitmap baselineBitmap;
    SkBitmap testBitmap;
    if (!SkImageDecoder::DecodeFile(baselinePath, &baselineBitmap)) {
        SkDebugf("Failed to load bitmap \"%s\"\n", baselinePath);
        return;
    }
    if (!SkImageDecoder::DecodeFile(testPath, &testBitmap)) {
        SkDebugf("Failed to load bitmap \"%s\"\n", testPath);
        return;
    }

    // Setup a record for this diff
    DiffRecord* newRecord = SkNEW(DiffRecord);
    newRecord->fBaselinePath = baselinePath;
    newRecord->fTestPath = testPath;
    newRecord->fNext = fRecords;
    fRecords = newRecord;

    // Perform each diff
    for (int differIndex = 0; differIndex < fDifferCount; differIndex++) {
        SkImageDiffer* differ = fDiffers[differIndex];
        int diffID = differ->queueDiff(&baselineBitmap, &testBitmap);
        if (diffID >= 0) {

            // Copy the results into data for this record
            DiffData& diffData = newRecord->fDiffs.push_back();

            diffData.fDiffName = differ->getName();
            diffData.fResult = differ->getResult(diffID);

            int poiCount = differ->getPointsOfInterestCount(diffID);
            SkIPoint* poi = differ->getPointsOfInterest(diffID);
            diffData.fPointsOfInterest.append(poiCount, poi);

            // Because we are doing everything synchronously for now, we are done with the diff
            // after reading it.
            differ->deleteDiff(diffID);
        }
    }

    // if we get a difference and we want the alpha mask then compute it here.
}

class SkThreadedDiff : public SkRunnable {
public:
    SkThreadedDiff() : fDiffContext(NULL) { }

    void setup(SkDiffContext* diffContext, const SkString& baselinePath, const SkString& testPath) {
        fDiffContext = diffContext;
        fBaselinePath = baselinePath;
        fTestPath = testPath;
    }

    virtual void run() SK_OVERRIDE {
        fDiffContext->addDiff(fBaselinePath.c_str(), fTestPath.c_str());
    }

private:
    SkDiffContext* fDiffContext;
    SkString fBaselinePath;
    SkString fTestPath;
};

void SkDiffContext::diffDirectories(const char baselinePath[], const char testPath[]) {
    // Get the files in the baseline, we will then look for those inside the test path
    SkTArray<SkString> baselineEntries;
    if (!get_directory(baselinePath, &baselineEntries)) {
        SkDebugf("Unable to open path \"%s\"\n", baselinePath);
        return;
    }

    SkThreadPool threadPool(fThreadCount);
    SkTArray<SkThreadedDiff> runnableDiffs;
    runnableDiffs.reset(baselineEntries.count());

    for (int x = 0; x < baselineEntries.count(); x++) {
        const char* baseFilename = baselineEntries[x].c_str();

        // Find the real location of each file to compare
        SkString baselineFile = SkOSPath::SkPathJoin(baselinePath, baseFilename);
        SkString testFile = SkOSPath::SkPathJoin(testPath, baseFilename);

        // Check that the test file exists and is a file
        if (sk_exists(testFile.c_str()) && !sk_isdir(testFile.c_str())) {
            // Queue up the comparison with the differ
            runnableDiffs[x].setup(this, baselineFile, testFile);
            threadPool.add(&runnableDiffs[x]);
        } else {
            SkDebugf("Baseline file \"%s\" has no corresponding test file\n", baselineFile.c_str());
        }
    }

    threadPool.wait();
}


void SkDiffContext::diffPatterns(const char baselinePattern[], const char testPattern[]) {
    // Get the files in the baseline and test patterns. Because they are in sorted order, it's easy
    // to find corresponding images by matching entry indices.

    SkTArray<SkString> baselineEntries;
    if (!glob_files(baselinePattern, &baselineEntries)) {
        SkDebugf("Unable to get pattern \"%s\"\n", baselinePattern);
        return;
    }

    SkTArray<SkString> testEntries;
    if (!glob_files(testPattern, &testEntries)) {
        SkDebugf("Unable to get pattern \"%s\"\n", testPattern);
        return;
    }

    if (baselineEntries.count() != testEntries.count()) {
        SkDebugf("Baseline and test patterns do not yield corresponding number of files\n");
        return;
    }

    SkThreadPool threadPool(fThreadCount);
    SkTArray<SkThreadedDiff> runnableDiffs;
    runnableDiffs.reset(baselineEntries.count());

    for (int x = 0; x < baselineEntries.count(); x++) {
        runnableDiffs[x].setup(this, baselineEntries[x], testEntries[x]);
        threadPool.add(&runnableDiffs[x]);
    }

    threadPool.wait();
}

void SkDiffContext::outputRecords(SkWStream& stream, bool useJSONP) {
    DiffRecord* currentRecord = fRecords;
    if (useJSONP) {
        stream.writeText("var SkPDiffRecords = {\n");
    } else {
        stream.writeText("{\n");
    }
    stream.writeText("    \"records\": [\n");
    while (NULL != currentRecord) {
        stream.writeText("        {\n");

            SkString differenceAbsPath = get_absolute_path(currentRecord->fDifferencePath);
            SkString baselineAbsPath = get_absolute_path(currentRecord->fBaselinePath);
            SkString testAbsPath = get_absolute_path(currentRecord->fTestPath);

            // strip off directory structure and find the common part of the filename
            SkString baseName = SkOSPath::SkBasename(baselineAbsPath.c_str());
            SkString testName = SkOSPath::SkBasename(testAbsPath.c_str());
            for (size_t x = 0; x < baseName.size(); ++x) {
                if (baseName[x] != testName[x]) {
                    baseName.insertUnichar(x, '\n');
                    break;
                }
            }

            stream.writeText("            \"commonName\": \"");
            stream.writeText(baseName.c_str());
            stream.writeText("\",\n");

            stream.writeText("            \"differencePath\": \"");
            stream.writeText(differenceAbsPath.c_str());
            stream.writeText("\",\n");

            stream.writeText("            \"baselinePath\": \"");
            stream.writeText(baselineAbsPath.c_str());
            stream.writeText("\",\n");

            stream.writeText("            \"testPath\": \"");
            stream.writeText(testAbsPath.c_str());
            stream.writeText("\",\n");

            stream.writeText("            \"diffs\": [\n");
            for (int diffIndex = 0; diffIndex < currentRecord->fDiffs.count(); diffIndex++) {
                DiffData& data = currentRecord->fDiffs[diffIndex];
                stream.writeText("                {\n");

                    stream.writeText("                    \"differName\": \"");
                    stream.writeText(data.fDiffName);
                    stream.writeText("\",\n");

                    stream.writeText("                    \"result\": ");
                    stream.writeScalarAsText((SkScalar)data.fResult);
                    stream.writeText(",\n");

                    stream.writeText("                    \"pointsOfInterest\": [\n");
                    for (int poiIndex = 0; poiIndex < data.fPointsOfInterest.count() &&
                                           poiIndex < kMaxPOI; poiIndex++) {
                        SkIPoint poi = data.fPointsOfInterest[poiIndex];
                        stream.writeText("                        [");
                        stream.writeDecAsText(poi.x());
                        stream.writeText(",");
                        stream.writeDecAsText(poi.y());
                        stream.writeText("]");

                        // JSON does not allow trailing commas
                        if (poiIndex + 1 < data.fPointsOfInterest.count() &&
                            poiIndex + 1 < kMaxPOI) {
                            stream.writeText(",");
                        }
                        stream.writeText("\n");
                    }
                    stream.writeText("                    ]\n");
                stream.writeText("                }");

                // JSON does not allow trailing commas
                if (diffIndex + 1 < currentRecord->fDiffs.count()) {
                    stream.writeText(",");
                }
                stream.writeText("                \n");
            }
            stream.writeText("            ]\n");

        stream.writeText("        }");

        // JSON does not allow trailing commas
        if (NULL != currentRecord->fNext) {
            stream.writeText(",");
        }
        stream.writeText("\n");
        currentRecord = currentRecord->fNext;
    }
    stream.writeText("    ]\n");
    if (useJSONP) {
        stream.writeText("};\n");
    } else {
        stream.writeText("}\n");
    }
}

void SkDiffContext::outputCsv(SkWStream& stream) {
    SkTDict<int> columns(2);
    int cntColumns = 0;

    stream.writeText("key");

    DiffRecord* currentRecord = fRecords;

    // Write CSV header and create a dictionary of all columns.
    while (NULL != currentRecord) {
        for (int diffIndex = 0; diffIndex < currentRecord->fDiffs.count(); diffIndex++) {
            DiffData& data = currentRecord->fDiffs[diffIndex];
            if (!columns.find(data.fDiffName)) {
                columns.set(data.fDiffName, cntColumns);
                stream.writeText(", ");
                stream.writeText(data.fDiffName);
                cntColumns++;
            }
        }
        currentRecord = currentRecord->fNext;
    }
    stream.writeText("\n");

    double values[100];
    SkASSERT(cntColumns < 100);  // Make the array larger, if we ever have so many diff types.

    currentRecord = fRecords;
    while (NULL != currentRecord) {
        for (int i = 0; i < cntColumns; i++) {
            values[i] = -1;
        }

        for (int diffIndex = 0; diffIndex < currentRecord->fDiffs.count(); diffIndex++) {
            DiffData& data = currentRecord->fDiffs[diffIndex];
            int index = -1;
            SkAssertResult(columns.find(data.fDiffName, &index));
            SkASSERT(index >= 0 && index < cntColumns);
            values[index] = data.fResult;
        }

        const char* filename = currentRecord->fBaselinePath.c_str() +
                strlen(currentRecord->fBaselinePath.c_str()) - 1;
        while (filename > currentRecord->fBaselinePath.c_str() && *(filename - 1) != '/') {
            filename--;
        }

        stream.writeText(filename);

        for (int i = 0; i < cntColumns; i++) {
            SkString str;
            str.printf(", %f", values[i]);
            stream.writeText(str.c_str());
        }
        stream.writeText("\n");

        currentRecord = currentRecord->fNext;
    }
}
