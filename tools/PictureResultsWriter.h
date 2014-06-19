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
    virtual void tileConfig(SkString configName) = 0;
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
    virtual void bench(const char name[], int32_t x, int32_t y) {
        for(int i=0; i<fWriters.count(); ++i) {
            fWriters[i]->bench(name, x, y);
        }
    }
    virtual void tileConfig(SkString configName) {
        for(int i=0; i<fWriters.count(); ++i) {
            fWriters[i]->tileConfig(configName);
        }
    }
    virtual void tileMeta(int x, int y, int tx, int ty) {
        for(int i=0; i<fWriters.count(); ++i) {
            fWriters[i]->tileMeta(x, y, tx, ty);
        }
    }
    virtual void addTileFlag(PictureResultsWriter::TileFlags flag) {
        for(int i=0; i<fWriters.count(); ++i) {
            fWriters[i]->addTileFlag(flag);
        }
    }
    virtual void tileData(
            TimerData* data,
            const char format[],
            const TimerData::Result result,
            uint32_t timerTypes,
            int numInnerLoops = 1) {
        for(int i=0; i<fWriters.count(); ++i) {
            fWriters[i]->tileData(data, format, result, timerTypes,
                                 numInnerLoops);
        }
    }
   virtual void end() {
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
          : fLogger(log), currentLine() {}
    virtual void bench(const char name[], int32_t x, int32_t y) {
        SkString result;
        result.printf("running bench [%i %i] %s ", x, y, name);
        this->logProgress(result.c_str());
    }
    virtual void tileConfig(SkString configName) {
        currentLine = configName;
    }
    virtual void tileMeta(int x, int y, int tx, int ty) {
        currentLine.appendf(": tile [%i,%i] out of [%i,%i]", x, y, tx, ty);
    }
    virtual void addTileFlag(PictureResultsWriter::TileFlags flag) {
        if(flag == PictureResultsWriter::kPurging) {
            currentLine.append(" <withPurging>");
        } else if(flag == PictureResultsWriter::kAvg) {
            currentLine.append(" <averaged>");
        }
    }
    virtual void tileData(
            TimerData* data,
            const char format[],
            const TimerData::Result result,
            uint32_t timerTypes,
            int numInnerLoops = 1) {
        SkString results = data->getResult(format, result,
                currentLine.c_str(), timerTypes, numInnerLoops);
        results.append("\n");
        this->logProgress(results.c_str());
    }
    virtual void end() {}
private:
    BenchLogger* fLogger;
    SkString currentLine;
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
    PictureJSONResultsWriter(const char filename[])
        : fFilename(filename),
          fRoot(),
          fCurrentBench(NULL),
          fCurrentTileSet(NULL),
          fCurrentTile(NULL) {}

    virtual void bench(const char name[], int32_t x, int32_t y) {
        SkString sk_name(name);
        sk_name.append("_");
        sk_name.appendS32(x);
        sk_name.append("_");
        sk_name.appendS32(y);
        Json::Value* bench_node = SkFindNamedNode(&fRoot["benches"], sk_name.c_str());
        fCurrentBench = &(*bench_node)["tileSets"];
    }
    virtual void tileConfig(SkString configName) {
        SkASSERT(fCurrentBench != NULL);
        fCurrentTileSet = SkFindNamedNode(fCurrentBench, configName.c_str());
        fCurrentTile = &(*fCurrentTileSet)["tiles"][0];
    }
    virtual void tileMeta(int x, int y, int tx, int ty) {
        SkASSERT(fCurrentTileSet != NULL);
        (*fCurrentTileSet)["tx"] = tx;
        (*fCurrentTileSet)["ty"] = ty;
        fCurrentTile = &(*fCurrentTileSet)["tiles"][x+tx*y];
    }
    virtual void addTileFlag(PictureResultsWriter::TileFlags flag) {
        SkASSERT(fCurrentTile != NULL);
        if(flag == PictureResultsWriter::kPurging) {
            (*fCurrentTile)["flags"]["purging"] = true;
        } else if(flag == PictureResultsWriter::kAvg) {
            (*fCurrentTile)["flags"]["averaged"] = true;
        }
    }
    virtual void tileData(
            TimerData* data,
            const char format[],
            const TimerData::Result result,
            uint32_t timerTypes,
            int numInnerLoops = 1) {
        SkASSERT(fCurrentTile != NULL);
        (*fCurrentTile)["data"] = data->getJSON(timerTypes, result, numInnerLoops);
    }
    virtual void end() {
       SkFILEWStream stream(fFilename.c_str());
       stream.writeText(Json::FastWriter().write(fRoot).c_str());
       stream.flush();
    }
private:
    SkString fFilename;
    Json::Value fRoot;
    Json::Value *fCurrentBench;
    Json::Value *fCurrentTileSet;
    Json::Value *fCurrentTile;
};

#endif
