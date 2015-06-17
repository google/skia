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
#include "SkSize.h"
#include "SkStream.h"
#include "SkTDict.h"
#include "SkTaskGroup.h"

// from the tools directory for replace_char(...)
#include "picture_utils.h"

#include "SkDiffContext.h"
#include "SkImageDiffer.h"
#include "skpdiff_util.h"

SkDiffContext::SkDiffContext() {
    fDiffers = NULL;
    fDifferCount = 0;
}

SkDiffContext::~SkDiffContext() {
    if (fDiffers) {
        SkDELETE_ARRAY(fDiffers);
    }
}

void SkDiffContext::setAlphaMaskDir(const SkString& path) {
    if (!path.isEmpty() && sk_mkdir(path.c_str())) {
        fAlphaMaskDir = path;
    }
}

void SkDiffContext::setRgbDiffDir(const SkString& path) {
    if (!path.isEmpty() && sk_mkdir(path.c_str())) {
        fRgbDiffDir = path;
    }
}

void SkDiffContext::setWhiteDiffDir(const SkString& path) {
    if (!path.isEmpty() && sk_mkdir(path.c_str())) {
        fWhiteDiffDir = path;
    }
}

void SkDiffContext::setLongNames(const bool useLongNames) {
    longNames = useLongNames;
}

void SkDiffContext::setDiffers(const SkTDArray<SkImageDiffer*>& differs) {
    // Delete whatever the last array of differs was
    if (fDiffers) {
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

static SkString get_combined_name(const SkString& a, const SkString& b) {
    // Note (stephana): We must keep this function in sync with
    // getImageDiffRelativeUrl() in static/loader.js (under rebaseline_server).
    SkString result = a;
    result.append("-vs-");
    result.append(b);
    sk_tools::replace_char(&result, '.', '_');
    return result;
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
    SkString baseName = SkOSPath::Basename(baselinePath);
    SkString testName = SkOSPath::Basename(testPath);

    if (longNames) {
        newRecord->fCommonName = get_combined_name(baseName, testName);
    } else {
        newRecord->fCommonName = get_common_prefix(baseName, testName);
    }
    newRecord->fCommonName.append(".png");

    newRecord->fBaselinePath = baselinePath;
    newRecord->fTestPath = testPath;
    newRecord->fSize = SkISize::Make(baselineBitmap.width(), baselineBitmap.height());

    // only generate diff images if we have a place to store them
    SkImageDiffer::BitmapsToCreate bitmapsToCreate;
    bitmapsToCreate.alphaMask = !fAlphaMaskDir.isEmpty();
    bitmapsToCreate.rgbDiff = !fRgbDiffDir.isEmpty();
    bitmapsToCreate.whiteDiff = !fWhiteDiffDir.isEmpty();

    // Perform each diff
    for (int differIndex = 0; differIndex < fDifferCount; differIndex++) {
        SkImageDiffer* differ = fDiffers[differIndex];

        // Copy the results into data for this record
        DiffData& diffData = newRecord->fDiffs.push_back();
        diffData.fDiffName = differ->getName();

        if (!differ->diff(&baselineBitmap, &testBitmap, bitmapsToCreate, &diffData.fResult)) {
            // if the diff failed, record -1 as the result
            // TODO(djsollen): Record more detailed information about exactly what failed.
            // (Image dimension mismatch? etc.)  See http://skbug.com/2710 ('make skpdiff
            // report more detail when it fails to compare two images')
            diffData.fResult.result = -1;
            continue;
        }

        if (bitmapsToCreate.alphaMask
                && SkImageDiffer::RESULT_CORRECT != diffData.fResult.result
                && !diffData.fResult.poiAlphaMask.empty()
                && !newRecord->fCommonName.isEmpty()) {

            newRecord->fAlphaMaskPath = SkOSPath::Join(fAlphaMaskDir.c_str(),
                                                       newRecord->fCommonName.c_str());

            // compute the image diff and output it
            SkBitmap copy;
            diffData.fResult.poiAlphaMask.copyTo(&copy, kN32_SkColorType);
            SkImageEncoder::EncodeFile(newRecord->fAlphaMaskPath.c_str(), copy,
                                       SkImageEncoder::kPNG_Type, 100);

            // cleanup the existing bitmap to free up resources;
            diffData.fResult.poiAlphaMask.reset();

            bitmapsToCreate.alphaMask = false;
        }

        if (bitmapsToCreate.rgbDiff
                && SkImageDiffer::RESULT_CORRECT != diffData.fResult.result
                && !diffData.fResult.rgbDiffBitmap.empty()
                && !newRecord->fCommonName.isEmpty()) {
            // TODO(djsollen): Rather than taking the max r/g/b diffs that come back from
            // a particular differ and storing them as toplevel fields within
            // newRecord, we should extend outputRecords() to report optional
            // fields for each differ (not just "result" and "pointsOfInterest").
            // See http://skbug.com/2712 ('allow skpdiff to report different sets
            // of result fields for different comparison algorithms')
            newRecord->fMaxRedDiff = diffData.fResult.maxRedDiff;
            newRecord->fMaxGreenDiff = diffData.fResult.maxGreenDiff;
            newRecord->fMaxBlueDiff = diffData.fResult.maxBlueDiff;

            newRecord->fRgbDiffPath = SkOSPath::Join(fRgbDiffDir.c_str(),
                                                     newRecord->fCommonName.c_str());
            SkImageEncoder::EncodeFile(newRecord->fRgbDiffPath.c_str(),
                                       diffData.fResult.rgbDiffBitmap,
                                       SkImageEncoder::kPNG_Type, 100);
            diffData.fResult.rgbDiffBitmap.reset();
            bitmapsToCreate.rgbDiff = false;
        }

        if (bitmapsToCreate.whiteDiff
                && SkImageDiffer::RESULT_CORRECT != diffData.fResult.result
                && !diffData.fResult.whiteDiffBitmap.empty()
                && !newRecord->fCommonName.isEmpty()) {
            newRecord->fWhiteDiffPath = SkOSPath::Join(fWhiteDiffDir.c_str(),
                                                       newRecord->fCommonName.c_str());
            SkImageEncoder::EncodeFile(newRecord->fWhiteDiffPath.c_str(),
                                       diffData.fResult.whiteDiffBitmap,
                                       SkImageEncoder::kPNG_Type, 100);
            diffData.fResult.whiteDiffBitmap.reset();
            bitmapsToCreate.whiteDiff = false;
        }
    }
}

void SkDiffContext::diffDirectories(const char baselinePath[], const char testPath[]) {
    // Get the files in the baseline, we will then look for those inside the test path
    SkTArray<SkString> baselineEntries;
    if (!get_directory(baselinePath, &baselineEntries)) {
        SkDebugf("Unable to open path \"%s\"\n", baselinePath);
        return;
    }

    sk_parallel_for(baselineEntries.count(), [&](int i) {
        const char* baseFilename = baselineEntries[i].c_str();

        // Find the real location of each file to compare
        SkString baselineFile = SkOSPath::Join(baselinePath, baseFilename);
        SkString testFile = SkOSPath::Join(testPath, baseFilename);

        // Check that the test file exists and is a file
        if (sk_exists(testFile.c_str()) && !sk_isdir(testFile.c_str())) {
            this->addDiff(baselineFile.c_str(), testFile.c_str());
        } else {
            SkDebugf("Baseline file \"%s\" has no corresponding test file\n", baselineFile.c_str());
        }
    });
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

    sk_parallel_for(baselineEntries.count(), [&](int i) {
        this->addDiff(baselineEntries[i].c_str(), testEntries[i].c_str());
    });
}

void SkDiffContext::outputRecords(SkWStream& stream, bool useJSONP) {
    SkTLList<DiffRecord>::Iter iter(fRecords, SkTLList<DiffRecord>::Iter::kHead_IterStart);
    DiffRecord* currentRecord = iter.get();

    if (useJSONP) {
        stream.writeText("var SkPDiffRecords = {\n");
    } else {
        stream.writeText("{\n");
    }

    // TODO(djsollen): Would it be better to use the jsoncpp library to write out the JSON?
    // This manual approach is probably more efficient, but it sure is ugly.
    // See http://skbug.com/2713 ('make skpdiff use jsoncpp library to write out
    // JSON output, instead of manual writeText() calls?')
    stream.writeText("    \"records\": [\n");
    while (currentRecord) {
        stream.writeText("        {\n");

            SkString baselineAbsPath = get_absolute_path(currentRecord->fBaselinePath);
            SkString testAbsPath = get_absolute_path(currentRecord->fTestPath);

            stream.writeText("            \"commonName\": \"");
            stream.writeText(currentRecord->fCommonName.c_str());
            stream.writeText("\",\n");

            stream.writeText("            \"differencePath\": \"");
            stream.writeText(get_absolute_path(currentRecord->fAlphaMaskPath).c_str());
            stream.writeText("\",\n");

            stream.writeText("            \"rgbDiffPath\": \"");
            stream.writeText(get_absolute_path(currentRecord->fRgbDiffPath).c_str());
            stream.writeText("\",\n");

            stream.writeText("            \"whiteDiffPath\": \"");
            stream.writeText(get_absolute_path(currentRecord->fWhiteDiffPath).c_str());
            stream.writeText("\",\n");

            stream.writeText("            \"baselinePath\": \"");
            stream.writeText(baselineAbsPath.c_str());
            stream.writeText("\",\n");

            stream.writeText("            \"testPath\": \"");
            stream.writeText(testAbsPath.c_str());
            stream.writeText("\",\n");

            stream.writeText("            \"width\": ");
            stream.writeDecAsText(currentRecord->fSize.width());
            stream.writeText(",\n");
            stream.writeText("            \"height\": ");
            stream.writeDecAsText(currentRecord->fSize.height());
            stream.writeText(",\n");

            stream.writeText("            \"maxRedDiff\": ");
            stream.writeDecAsText(currentRecord->fMaxRedDiff);
            stream.writeText(",\n");
            stream.writeText("            \"maxGreenDiff\": ");
            stream.writeDecAsText(currentRecord->fMaxGreenDiff);
            stream.writeText(",\n");
            stream.writeText("            \"maxBlueDiff\": ");
            stream.writeDecAsText(currentRecord->fMaxBlueDiff);
            stream.writeText(",\n");

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
        if (currentRecord) {
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
    while (currentRecord) {
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
    while (currentRecord) {
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
