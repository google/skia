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
#include "skpdiff_util.h"

SkDiffContext::SkDiffContext() {
    fDiffers = NULL;
    fDifferCount = 0;
    fThreadCount = SkThreadPool::kThreadPerCore;
}

SkDiffContext::~SkDiffContext() {
    if (NULL != fDiffers) {
        SkDELETE_ARRAY(fDiffers);
    }
}

void SkDiffContext::setDifferenceDir(const SkString& path) {
    if (!path.isEmpty() && sk_mkdir(path.c_str())) {
        fDifferenceDir = path;
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

static SkString get_common_prefix(const SkString& a, const SkString& b) {
    const size_t maxPrefixLength = SkTMin(a.size(), b.size());
    SkASSERT(maxPrefixLength > 0);
    for (size_t x = 0; x < maxPrefixLength; ++x) {
        if (a[x] != b[x]) {
            SkString result;
            result.set(a.c_str(), x);
            return result;
        }
    }
    if (a.size() > b.size()) {
        return b;
    } else {
        return a;
    }
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
    fRecordMutex.acquire();
    DiffRecord* newRecord = fRecords.addToHead(DiffRecord());
    fRecordMutex.release();

    // compute the common name
    SkString baseName = SkOSPath::SkBasename(baselinePath);
    SkString testName = SkOSPath::SkBasename(testPath);
    newRecord->fCommonName = get_common_prefix(baseName, testName);

    newRecord->fBaselinePath = baselinePath;
    newRecord->fTestPath = testPath;

    bool alphaMaskPending = false;

    // only enable alpha masks if a difference dir has been provided
    if (!fDifferenceDir.isEmpty()) {
        alphaMaskPending = true;
    }

    // Perform each diff
    for (int differIndex = 0; differIndex < fDifferCount; differIndex++) {
        SkImageDiffer* differ = fDiffers[differIndex];

        // Copy the results into data for this record
        DiffData& diffData = newRecord->fDiffs.push_back();
        diffData.fDiffName = differ->getName();

        if (!differ->diff(&baselineBitmap, &testBitmap, alphaMaskPending, &diffData.fResult)) {
            // if the diff failed the remove its entry from the list
            newRecord->fDiffs.pop_back();
            continue;
        }

        if (alphaMaskPending
                && SkImageDiffer::RESULT_CORRECT != diffData.fResult.result
                && !diffData.fResult.poiAlphaMask.empty()
                && !newRecord->fCommonName.isEmpty()) {

            newRecord->fDifferencePath = SkOSPath::SkPathJoin(fDifferenceDir.c_str(),
                                                              newRecord->fCommonName.c_str());

            // compute the image diff and output it
            SkBitmap copy;
            diffData.fResult.poiAlphaMask.copyTo(&copy, SkBitmap::kARGB_8888_Config);
            SkImageEncoder::EncodeFile(newRecord->fDifferencePath.c_str(), copy,
                                       SkImageEncoder::kPNG_Type, 100);

            // cleanup the existing bitmap to free up resources;
            diffData.fResult.poiAlphaMask.reset();

            alphaMaskPending = false;
        }
    }
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
    SkTLList<DiffRecord>::Iter iter(fRecords, SkTLList<DiffRecord>::Iter::kHead_IterStart);
    DiffRecord* currentRecord = iter.get();

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

            stream.writeText("            \"commonName\": \"");
            stream.writeText(currentRecord->fCommonName.c_str());
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
                    stream.writeScalarAsText((SkScalar)data.fResult.result);
                    stream.writeText(",\n");

                    stream.writeText("                    \"pointsOfInterest\": ");
                    stream.writeDecAsText(data.fResult.poiCount);
                    stream.writeText("\n");

                stream.writeText("                }");

                // JSON does not allow trailing commas
                if (diffIndex + 1 < currentRecord->fDiffs.count()) {
                    stream.writeText(",");
                }
                stream.writeText("                \n");
            }
            stream.writeText("            ]\n");

        stream.writeText("        }");

        currentRecord = iter.next();

        // JSON does not allow trailing commas
        if (NULL != currentRecord) {
            stream.writeText(",");
        }
        stream.writeText("\n");
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

    SkTLList<DiffRecord>::Iter iter(fRecords, SkTLList<DiffRecord>::Iter::kHead_IterStart);
    DiffRecord* currentRecord = iter.get();

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
        currentRecord = iter.next();
    }
    stream.writeText("\n");

    double values[100];
    SkASSERT(cntColumns < 100);  // Make the array larger, if we ever have so many diff types.

    SkTLList<DiffRecord>::Iter iter2(fRecords, SkTLList<DiffRecord>::Iter::kHead_IterStart);
    currentRecord = iter2.get();
    while (NULL != currentRecord) {
        for (int i = 0; i < cntColumns; i++) {
            values[i] = -1;
        }

        for (int diffIndex = 0; diffIndex < currentRecord->fDiffs.count(); diffIndex++) {
            DiffData& data = currentRecord->fDiffs[diffIndex];
            int index = -1;
            SkAssertResult(columns.find(data.fDiffName, &index));
            SkASSERT(index >= 0 && index < cntColumns);
            values[index] = data.fResult.result;
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

        currentRecord = iter2.next();
    }
}
