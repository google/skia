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
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTypes.h"

/**
 * Base class for writing out the bench results.
 *
 * TODO(jcgregorio) Add info if tests fail to converge?
 */
class ResultsWriter : SkNoncopyable {
public:
    virtual ~ResultsWriter() {};

    // Records one key value pair that makes up a unique identifier for this run.
    // All keys must be set before calling bench().
    virtual void key(const char name[], const char value[]) = 0;

    // Records one option set for this run. All options must be set before
    // calling bench().
    virtual void option(const char name[], const char value[]) = 0;

    // Denotes the start of a specific benchmark. Once bench is called,
    // then config and timer can be called multiple times to record runs.
    virtual void bench(const char name[], int32_t x, int32_t y) = 0;

    // Records the specific configuration a bench is run under, such as "8888".
    virtual void config(const char name[]) = 0;

    // Records the options for a configuration, such as "GL_RENDERER".
    virtual void configOption(const char name[], const char* value) = 0;

    // Records a single test metric.
    virtual void timer(const char name[], double ms) = 0;

    // Call when all results are finished.
    virtual void end() = 0;
};

/**
 * This ResultsWriter handles writing out the human readable format of the
 * bench results.
 */
class LoggerResultsWriter : public ResultsWriter {
public:
    explicit LoggerResultsWriter(BenchLogger& logger, const char* timeFormat)
        : fLogger(logger)
        , fTimeFormat(timeFormat) {
        fLogger.logProgress("skia bench:");
    }
    virtual void key(const char name[], const char value[]) {
        // Don't log keys to keep microbench output unchanged.
    }
    virtual void option(const char name[], const char value[]) {
        fLogger.logProgress(SkStringPrintf(" %s=%s", name, value));
    }
    virtual void bench(const char name[], int32_t x, int32_t y) {
        fLogger.logProgress(SkStringPrintf(
            "\nrunning bench [%3d %3d] %40s", x, y, name));
    }
    virtual void config(const char name[]) {
        fLogger.logProgress(SkStringPrintf("   %s:", name));
    }
    virtual void configOption(const char name[], const char* value) {
        // Don't log configOptions to keep microbench output unchanged.
    }
    virtual void timer(const char name[], double ms) {
        fLogger.logProgress(SkStringPrintf("  %s = ", name));
        fLogger.logProgress(SkStringPrintf(fTimeFormat, ms));
    }
    virtual void end() {
        fLogger.logProgress("\n");
    }
private:
    BenchLogger& fLogger;
    const char* fTimeFormat;
};

/**
 * This ResultsWriter handles writing out the results in JSON.
 *
 * The output looks like (except compressed to a single line):
 *
 *  {
 *   "options" : {
 *      "alpha" : "0xFF",
 *      "scale" : "0",
 *      ...
 *      "system" : "UNIX"
 *   },
 *   "results" : [
 *      {
 *      "name" : "Xfermode_Luminosity_640_480",
 *      "results" : [
 *         {
 *            "name": "565",
 *            "cmsecs" : 143.188128906250,
 *            "msecs" : 143.835957031250
 *         },
 *         ...
 */

Json::Value* SkFindNamedNode(Json::Value* root, const char name[]);
Json::Value SkMakeBuilderJSON(const SkString &buildername);

class JSONResultsWriter : public ResultsWriter {
public:
    explicit JSONResultsWriter(const char filename[])
        : fFilename(filename)
        , fRoot()
        , fResults(fRoot["results"])
        , fBench(NULL)
        , fConfig(NULL) {
    }
    virtual void key(const char name[], const char value[]) {
    }
    virtual void option(const char name[], const char value[]) {
        fRoot["options"][name] = value;
    }
    virtual void bench(const char name[], int32_t x, int32_t y) {
        SkString sk_name(name);
        sk_name.append("_");
        sk_name.appendS32(x);
        sk_name.append("_");
        sk_name.appendS32(y);
        Json::Value* bench_node = SkFindNamedNode(&fResults, sk_name.c_str());
        fBench = &(*bench_node)["results"];
    }
    virtual void config(const char name[]) {
        SkASSERT(NULL != fBench);
        fConfig = SkFindNamedNode(fBench, name);
    }
    virtual void configOption(const char name[], const char* value) {
    }
    virtual void timer(const char name[], double ms) {
        SkASSERT(NULL != fConfig);
        (*fConfig)[name] = ms;
    }
    virtual void end() {
        SkFILEWStream stream(fFilename.c_str());
        stream.writeText(Json::FastWriter().write(fRoot).c_str());
        stream.flush();
    }
private:

    SkString fFilename;
    Json::Value fRoot;
    Json::Value& fResults;
    Json::Value* fBench;
    Json::Value* fConfig;
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
    "options": {
       "GL_Version": "3.1",
       ...
    },
    "gitHash": "d1830323662ae8ae06908b97f15180fd25808894",
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
    explicit NanoJSONResultsWriter(const char filename[], const char gitHash[])
        : fFilename(filename)
        , fRoot()
        , fResults(fRoot["results"])
        , fBench(NULL)
        , fConfig(NULL) {
        fRoot["gitHash"] = gitHash;
    }
    virtual void key(const char name[], const char value[]) {
        fRoot["key"][name] = value;
    }
    virtual void option(const char name[], const char value[]) {
        fRoot["options"][name] = value;
    }
    virtual void bench(const char name[], int32_t x, int32_t y) {
        SkString id = SkStringPrintf( "%s_%d_%d", name, x, y);
        fResults[id.c_str()] = Json::Value(Json::objectValue);
        fBench = &fResults[id.c_str()];
    }
    virtual void config(const char name[]) {
        SkASSERT(NULL != fBench);
        fConfig = &(*fBench)[name];
    }
    virtual void configOption(const char name[], const char* value) {
        (*fConfig)["options"][name] = value;
    }
    virtual void timer(const char name[], double ms) {
        // Don't record if nan, or -nan.
        if (sk_double_isnan(ms)) {
            return;
        }
        SkASSERT(NULL != fConfig);
        (*fConfig)[name] = ms;
    }
    virtual void end() {
        SkFILEWStream stream(fFilename.c_str());
        stream.writeText(Json::FastWriter().write(fRoot).c_str());
        stream.flush();
    }
private:

    SkString fFilename;
    Json::Value fRoot;
    Json::Value& fResults;
    Json::Value* fBench;
    Json::Value* fConfig;
};


/**
 * This ResultsWriter writes out to multiple ResultsWriters.
 */
class MultiResultsWriter : public ResultsWriter {
public:
    MultiResultsWriter() : writers() {
    };
    void add(ResultsWriter* writer) {
      writers.push_back(writer);
    }
    virtual void key(const char name[], const char value[]) {
        for (int i = 0; i < writers.count(); ++i) {
            writers[i]->key(name, value);
        }
    }
    virtual void option(const char name[], const char value[]) {
        for (int i = 0; i < writers.count(); ++i) {
            writers[i]->option(name, value);
        }
    }
    virtual void bench(const char name[], int32_t x, int32_t y) {
        for (int i = 0; i < writers.count(); ++i) {
            writers[i]->bench(name, x, y);
        }
    }
    virtual void config(const char name[]) {
        for (int i = 0; i < writers.count(); ++i) {
            writers[i]->config(name);
        }
    }
    virtual void configOption(const char name[], const char* value) {
        for (int i = 0; i < writers.count(); ++i) {
            writers[i]->configOption(name, value);
        }
    }
    virtual void timer(const char name[], double ms) {
        for (int i = 0; i < writers.count(); ++i) {
            writers[i]->timer(name, ms);
        }
    }
    virtual void end() {
        for (int i = 0; i < writers.count(); ++i) {
            writers[i]->end();
        }
    }
private:
    SkTArray<ResultsWriter *> writers;
};

/**
 * Calls the end() method of T on destruction.
 */
template <typename T> class CallEnd : SkNoncopyable {
public:
    CallEnd(T& obj) : fObj(obj) {}
    ~CallEnd() { fObj.end(); }
private:
    T&  fObj;
};

#endif
