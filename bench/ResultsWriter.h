/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Classes for writing out bench results in various formats.
 */

#ifndef SkResultsWriter_DEFINED
#define SkResultsWriter_DEFINED

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkJSONWriter.h"

/**
 NanoJSONResultsWriter helps nanobench writes the test results out in the following format:

 {
    "key": {
      "arch": "Arm7",
      "gpu": "SGX540",
      "os": "Android",
      "model": "GalaxyNexus",
    }
    "gitHash": "d1830323662ae8ae06908b97f15180fd25808894",
    "build_number": "1234",
    "results" : {
        "Xfermode_Luminosity_640_480" : {
           "8888" : {
                 "median_ms" : 143.188128906250,
                 "min_ms" : 143.835957031250,
                 ...
              },
          ...
*/
class NanoJSONResultsWriter : public SkJSONWriter {
public:
    NanoJSONResultsWriter(SkWStream* stream, Mode mode) : SkJSONWriter(stream, mode) {}

    void beginBench(const char* name, int32_t x, int32_t y) {
        SkString id = SkStringPrintf("%s_%d_%d", name, x, y);
        this->beginObject(id.c_str());
    }

    void endBench() { this->endObject(); }

    void appendMetric(const char* name, double value) {
        // Don't record if nan, or -nan.
        if (!sk_double_isnan(value)) {
            this->appendDoubleDigits(name, value, 16);
        }
    }
};

/**
 NanoFILEAppendAndCloseStream: re-open the file, append the data, then close on every write() call.

 The purpose of this class is to not keep the file handle open between JSON flushes. SkJSONWriter
 uses a 32k in-memory cache already, so it only flushes occasionally and is well equipped for a
 steam like this.

 See: https://b.corp.google.com/issues/143074513
*/
class NanoFILEAppendAndCloseStream : public SkWStream {
public:
    NanoFILEAppendAndCloseStream(const char* filePath) : fFilePath(filePath) {
        // Open the file as "write" to ensure it exists and clear any contents before we begin
        // appending.
        FILE* file = sk_fopen(fFilePath.c_str(), kWrite_SkFILE_Flag);
        if (!file) {
            SkDebugf("Failed to open file %s for write.\n", fFilePath.c_str());
            fFilePath.reset();
            return;
        }
        sk_fclose(file);
    }

    size_t bytesWritten() const override { return fBytesWritten; }

    bool write(const void* buffer, size_t size) override {
        if (fFilePath.isEmpty()) {
            return false;
        }

        FILE* file = sk_fopen(fFilePath.c_str(), kAppend_SkFILE_Flag);
        if (!file) {
            SkDebugf("Failed to open file %s for append.\n", fFilePath.c_str());
            return false;
        }

        size_t bytesWritten = sk_fwrite(buffer, size, file);
        fBytesWritten += bytesWritten;
        sk_fclose(file);

        if (bytesWritten != size) {
            SkDebugf("NanoFILEAppendAndCloseStream failed writing %d bytes (wrote %d instead)\n",
                     size, bytesWritten);
            return false;
        }

        return true;
    }

    void flush() override {}

private:
    SkString fFilePath;
    size_t fBytesWritten = 0;
};

#endif
