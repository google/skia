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

#include "BenchLogger.h"
#include "SkJSONCPP.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTypes.h"

/**
 * Base class for writing out the bench results.
 *
 * Default implementation does nothing.
 */
class ResultsWriter : SkNoncopyable {
public:
    virtual ~ResultsWriter() {}

    // Record one key value pair that makes up a unique key for this type of run, e.g.
    // builder name, machine type, Debug/Release, etc.
    virtual void key(const char name[], const char value[]) {}

    // Record one key value pair that describes the run instance, e.g. git hash, build number.
    virtual void property(const char name[], const char value[]) {}

    // Denote the start of a specific benchmark. Once bench is called,
    // then config and metric can be called multiple times to record runs.
    virtual void bench(const char name[], int32_t x, int32_t y) {}

    // Record the specific configuration a bench is run under, such as "8888".
    virtual void config(const char name[]) {}

    // Record the options for a configuration, such as "GL_RENDERER".
    virtual void configOption(const char name[], const char* value) {}

    // Record a single test metric.
    virtual void metric(const char name[], double ms) {}

    // Flush to storage now please.
    virtual void flush() {}
};

/**
 NanoJSONResultsWriter writes the test results out in the following
 format:

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
class NanoJSONResultsWriter : public ResultsWriter {
public:
    explicit NanoJSONResultsWriter(const char filename[])
        : fFilename(filename)
        , fRoot()
        , fResults(fRoot["results"])
        , fBench(nullptr)
        , fConfig(nullptr) {}

    ~NanoJSONResultsWriter() {
        this->flush();
    }

    // Added under "key".
    void key(const char name[], const char value[]) override {
        fRoot["key"][name] = value;
    }
    // Inserted directly into the root.
    void property(const char name[], const char value[]) override {
        fRoot[name] = value;
    }
    void bench(const char name[], int32_t x, int32_t y) override {
        SkString id = SkStringPrintf( "%s_%d_%d", name, x, y);
        fResults[id.c_str()] = Json::Value(Json::objectValue);
        fBench = &fResults[id.c_str()];
    }
    void config(const char name[]) override {
        SkASSERT(fBench);
        fConfig = &(*fBench)[name];
    }
    void configOption(const char name[], const char* value) override {
        (*fConfig)["options"][name] = value;
    }
    void metric(const char name[], double ms) override {
        // Don't record if nan, or -nan.
        if (sk_double_isnan(ms)) {
            return;
        }
        SkASSERT(fConfig);
        (*fConfig)[name] = ms;
    }

    // Flush to storage now please.
    void flush() override {
        SkString dirname = SkOSPath::Dirname(fFilename.c_str());
        if (!sk_exists(dirname.c_str(), kWrite_SkFILE_Flag)) {
            if (!sk_mkdir(dirname.c_str())) {
                SkDebugf("Failed to create directory.");
            }
        }
        SkFILEWStream stream(fFilename.c_str());
        stream.writeText(Json::StyledWriter().write(fRoot).c_str());
        stream.flush();
    }

private:
    SkString fFilename;
    Json::Value fRoot;
    Json::Value& fResults;
    Json::Value* fBench;
    Json::Value* fConfig;
};


#endif
