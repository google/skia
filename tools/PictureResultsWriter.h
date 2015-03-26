/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Classes for writing out bench results in various formats.
 */

#ifndef SkPictureResultsWriter_DEFINED
#define SkPictureResultsWriter_DEFINED


#include "PictureRenderer.h"
#include "BenchLogger.h"
#include "ResultsWriter.h"
#include "SkJSONCPP.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "TimerData.h"

/**
 * Base class for writing picture bench results.
 */
class PictureResultsWriter : SkNoncopyable {
public:
    enum TileFlags {kPurging, kAvg};

    PictureResultsWriter() {}
    virtual ~PictureResultsWriter() {}

    virtual void bench(const char name[], int32_t x, int32_t y) = 0;
    virtual void logRenderer(sk_tools::PictureRenderer *pr) = 0;
    virtual void tileMeta(int x, int y, int tx, int ty) = 0;
    virtual void addTileFlag(PictureResultsWriter::TileFlags flag) = 0;
    virtual void tileData(
            TimerData* data,
            const char format[],
            const TimerData::Result result,
            uint32_t timerTypes,
            int numInnerLoops = 1) = 0;
   virtual void end() = 0;
};

/**
 * This class allows bench data to be piped into multiple
 * PictureResultWriter classes. It does not own any classes
 * passed to it, so the owner is required to manage any classes
 * passed to PictureResultsMultiWriter */
class PictureResultsMultiWriter : public PictureResultsWriter {
public:
    PictureResultsMultiWriter()
        : fWriters() {}
    void add(PictureResultsWriter* newWriter) {
        fWriters.push_back(newWriter);
    }
    virtual ~PictureResultsMultiWriter() {}
    void bench(const char name[], int32_t x, int32_t y) override {
        for(int i=0; i<fWriters.count(); ++i) {
            fWriters[i]->bench(name, x, y);
        }
    }
    void logRenderer(sk_tools::PictureRenderer *pr) override {
        for(int i=0; i<fWriters.count(); ++i) {
            fWriters[i]->logRenderer(pr);
        }
    }
    void tileMeta(int x, int y, int tx, int ty) override {
        for(int i=0; i<fWriters.count(); ++i) {
            fWriters[i]->tileMeta(x, y, tx, ty);
        }
    }
    void addTileFlag(PictureResultsWriter::TileFlags flag) override {
        for(int i=0; i<fWriters.count(); ++i) {
            fWriters[i]->addTileFlag(flag);
        }
    }
    virtual void tileData(
            TimerData* data,
            const char format[],
            const TimerData::Result result,
            uint32_t timerTypes,
            int numInnerLoops = 1) override {
        for(int i=0; i<fWriters.count(); ++i) {
            fWriters[i]->tileData(data, format, result, timerTypes,
                                 numInnerLoops);
        }
    }
   void end() override {
        for(int i=0; i<fWriters.count(); ++i) {
            fWriters[i]->end();
        }
   }
private:
    SkTArray<PictureResultsWriter*> fWriters;
};

/**
 * Writes to BenchLogger to mimic original behavior
 */
class PictureResultsLoggerWriter : public PictureResultsWriter {
private:
    void logProgress(const char str[]) {
        if(fLogger != NULL) {
            fLogger->logProgress(str);
        }
    }
public:
    PictureResultsLoggerWriter(BenchLogger* log)
          : fLogger(log), fCurrentLine() {}
    void bench(const char name[], int32_t x, int32_t y) override {
        SkString result;
        result.printf("running bench [%i %i] %s ", x, y, name);
        this->logProgress(result.c_str());
    }
    void logRenderer(sk_tools::PictureRenderer* renderer) override {
        fCurrentLine = renderer->getConfigName();
    }
    void tileMeta(int x, int y, int tx, int ty) override {
        fCurrentLine.appendf(": tile [%i,%i] out of [%i,%i]", x, y, tx, ty);
    }
    void addTileFlag(PictureResultsWriter::TileFlags flag) override {
        if(flag == PictureResultsWriter::kPurging) {
            fCurrentLine.append(" <withPurging>");
        } else if(flag == PictureResultsWriter::kAvg) {
            fCurrentLine.append(" <averaged>");
        }
    }
    virtual void tileData(
            TimerData* data,
            const char format[],
            const TimerData::Result result,
            uint32_t timerTypes,
            int numInnerLoops = 1) override {
        SkString results = data->getResult(format, result,
                fCurrentLine.c_str(), timerTypes, numInnerLoops);
        results.append("\n");
        this->logProgress(results.c_str());
    }
    void end() override {}
private:
    BenchLogger* fLogger;
    SkString fCurrentLine;
};

/**
 * This PictureResultsWriter collects data in a JSON node
 *
 * The format is something like
 * {
 *      benches: [
 *          {
 *              name: "Name_of_test"
 *              tilesets: [
 *                  {
 *                      name: "Name of the configuration"
 *                      tiles: [
 *                          {
 *                              flags: {
 *                                  purging: true //Flags for the current tile
 *                                              // are put here
 *                              }
 *                              data: {
 *                                  wsecs: [....] //Actual data ends up here
 *                              }
 *                          }
 *                      ]
 *                  }
 *              ]
 *          }
 *      ]
 * }*/

class PictureJSONResultsWriter : public PictureResultsWriter {
public:
    PictureJSONResultsWriter(const char filename[],
                             const char builderName[],
                             int buildNumber,
                             int timestamp,
                             const char gitHash[],
                             int gitNumber)
        : fStream(filename) {
        fBuilderName = SkString(builderName);
        fBuildNumber = buildNumber;
        fTimestamp = timestamp;
        fGitHash = SkString(gitHash);
        fGitNumber = gitNumber;
        fBuilderData = this->makeBuilderJson();
    }

    void bench(const char name[], int32_t x, int32_t y) override {
        fBenchName = SkString(name);
    }
    void logRenderer(sk_tools::PictureRenderer* pr) override {
        fParams = pr->getJSONConfig();
        fConfigString = pr->getConfigName();
    }
    // Apparently tiles aren't used, so tileMeta is empty
    void tileMeta(int x, int y, int tx, int ty) override {}
    // Flags aren't used, so addTileFlag is empty
    void addTileFlag(PictureResultsWriter::TileFlags flag) override {}
    virtual void tileData(
            TimerData* data,
            const char format[],
            const TimerData::Result result,
            uint32_t timerTypes,
            int numInnerLoops = 1) override {
        Json::Value newData = data->getJSON(timerTypes, result, numInnerLoops);
        Json::Value combinedParams(fBuilderData);
        for(Json::ValueIterator iter = fParams.begin(); iter != fParams.end();
                iter++) {
            combinedParams[iter.key().asString()]= *iter;
        }
        // For each set of timer data
        for(Json::ValueIterator iter = newData.begin(); iter != newData.end();
                iter++) {
            Json::Value data;
            data["buildNumber"] = fBuildNumber;
            data["timestamp"] = fTimestamp;
            data["gitHash"] = fGitHash.c_str();
            data["gitNumber"] = fGitNumber;
            data["isTrybot"] = fBuilderName.endsWith("Trybot");

            data["params"] = combinedParams;
            data["params"]["benchName"] = fBenchName.c_str();

            // Not including skpSize because that's deprecated?
            data["key"] = this->makeKey(iter.key().asString().c_str()).c_str();
            // Get the data
            SkTArray<double> times;
            Json::Value val = *iter;
            for(Json::ValueIterator vals = val.begin(); vals != val.end();
                    vals++) {
                times.push_back((*vals).asDouble());
            }
            qsort(static_cast<void*>(times.begin()), times.count(),
                    sizeof(double), PictureJSONResultsWriter::CompareDoubles);
            data["value"] = times[static_cast<int>(times.count() * 0.25f)];
            data["params"]["measurementType"] = iter.key().asString();
            fStream.writeText(Json::FastWriter().write(data).c_str());
        }
    }
    void end() override {
       fStream.flush();
    }
private:
    Json::Value makeBuilderJson() const {
        static const int kNumKeys = 6;
        static const char* kKeys[kNumKeys] = {
            "role", "os", "model", "gpu", "arch", "configuration"};
        Json::Value builderData;

        if (!fBuilderName.isEmpty()) {
            SkTArray<SkString> splitBuilder;
            SkStrSplit(fBuilderName.c_str(), "-", &splitBuilder);
            SkASSERT(splitBuilder.count() >= kNumKeys);
            for (int i = 0; i < kNumKeys && i < splitBuilder.count(); ++i) {
                builderData[kKeys[i]] = splitBuilder[i].c_str();
            }
            builderData["builderName"] = fBuilderName.c_str();
            if (kNumKeys < splitBuilder.count()) {
                SkString extras;
                for (int i = kNumKeys; i < splitBuilder.count(); ++i) {
                    extras.append(splitBuilder[i]);
                    if (i != splitBuilder.count() - 1) {
                        extras.append("-");
                    }
                }
                builderData["badParams"] = extras.c_str();
            }
        }
        return builderData;
    }

    static int CompareDoubles(const void* p1, const void* p2) {
        if(*static_cast<const double*>(p1) < *static_cast<const double*>(p2)) {
            return -1;
        } else if(*static_cast<const double*>(p1) ==
                *static_cast<const double*>(p2)) {
            return 0;
        } else {
            return 1;
        }
    }
    SkString makeKey(const char measurementType[]) const {
        SkString tmp(fBuilderName);
        tmp.append("_");
        tmp.append(fBenchName);
        tmp.append("_");
        tmp.append(fConfigString);
        tmp.append("_");
        tmp.append(measurementType);
        return tmp;
    }

    SkFILEWStream   fStream;
    Json::Value     fBuilderData;
    SkString        fBenchName;
    Json::Value     fParams;

    SkString        fConfigString;
    SkString        fBuilderName;
    int             fBuildNumber;
    int             fTimestamp;
    SkString        fGitHash;
    int             fGitNumber;
};

#endif
