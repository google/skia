/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "VisualLightweightBenchModule.h"

#include "ProcStats.h"
#include "SkApplication.h"
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkGr.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "Stats.h"
#include "gl/GrGLInterface.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

// Between samples we reset context
// Between frames we swap buffers

DEFINE_int32(maxWarmupFrames, 100, "maxmium frames to try and tune for sane timings");
DEFINE_int32(gpuFrameLag, 5, "Overestimate of maximum number of frames GPU allows to lag.");
DEFINE_int32(samples, 10, "Number of times to time each skp.");
DEFINE_int32(frames, 5, "Number of frames of each skp to render per sample.");
DEFINE_double(loopMs, 5, "Target loop time in millseconds.");
DEFINE_bool2(verbose, v, false, "enable verbose output from the test driver.");
DEFINE_string(outResultsFile, "", "If given, write results here as JSON.");
DEFINE_string(key, "",
              "Space-separated key/value pairs to add to JSON identifying this builder.");
DEFINE_string(properties, "",
              "Space-separated key/value pairs to add to JSON identifying this run.");

static SkString humanize(double ms) {
    if (FLAGS_verbose) {
        return SkStringPrintf("%llu", (uint64_t)(ms*1e6));
    }
    return HumanizeMs(ms);
}

#define HUMANIZE(time) humanize(time).c_str()

// A trivial bench to warm up the gpu
class WarmupBench : public Benchmark {
public:
private:
    const char* onGetName() override { return "warmupbench"; }
    void onDraw(const int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            sk_tool_utils::draw_checkerboard(canvas, 0xffffffff, 0xffc6c3c6, 10);
        }
    }
};

VisualLightweightBenchModule::VisualLightweightBenchModule(VisualBench* owner)
    : fCurrentSample(0)
    , fCurrentFrame(0)
    , fLoops(1)
    , fState(kWarmup_State)
    , fBenchmark(nullptr)
    , fOwner(SkRef(owner))
    , fResults(new ResultsWriter) {
    fBenchmarkStream.reset(new VisualBenchmarkStream);

    // Print header
    SkDebugf("curr/maxrss\tloops\tmin\tmedian\tmean\tmax\tstddev\t%-*s\tbench\n", FLAGS_samples,
             "samples");

    // setup json logging if required
    if (!FLAGS_outResultsFile.isEmpty()) {
        fResults.reset(new NanoJSONResultsWriter(FLAGS_outResultsFile[0]));
    }

    if (1 == FLAGS_key.count() % 2) {
        SkDebugf("ERROR: --key must be passed with an even number of arguments.\n");
    } else {
        for (int i = 1; i < FLAGS_key.count(); i += 2) {
            fResults->key(FLAGS_key[i - 1], FLAGS_key[i]);
        }
    }

    if (1 == FLAGS_properties.count() % 2) {
        SkDebugf("ERROR: --properties must be passed with an even number of arguments.\n");
    } else {
        for (int i = 1; i < FLAGS_properties.count(); i += 2) {
            fResults->property(FLAGS_properties[i - 1], FLAGS_properties[i]);
        }
    }
}

inline void VisualLightweightBenchModule::renderFrame(SkCanvas* canvas) {
    fBenchmark->draw(fLoops, canvas);
    canvas->flush();
    fOwner->present();
}

void VisualLightweightBenchModule::printStats() {
    const SkTArray<double>& measurements = fRecords.back().fMeasurements;
    const char* shortName = fBenchmark->getUniqueName();

    // update log
    // Note: We currently log only the minimum.  It would be interesting to log more information
    SkString configName;
    if (FLAGS_msaa > 0) {
        configName.appendf("msaa_%d", FLAGS_msaa);
    } else {
        configName.appendf("gpu");
    }
    fResults->config(configName.c_str());
    fResults->configOption("name", shortName);
    SkASSERT(measurements.count());
    Stats stats(measurements);
    fResults->metric("min_ms", stats.min);

    // Print output
    if (FLAGS_verbose) {
        for (int i = 0; i < measurements.count(); i++) {
            SkDebugf("%s  ", HUMANIZE(measurements[i]));
        }
        SkDebugf("%s\n", shortName);
    } else {
        const double stdDevPercent = 100 * sqrt(stats.var) / stats.mean;
        SkDebugf("%4d/%-4dMB\t%d\t%s\t%s\t%s\t%s\t%.0f%%\t%s\t%s\n",
                 sk_tools::getCurrResidentSetSizeMB(),
                 sk_tools::getMaxResidentSetSizeMB(),
                 fLoops,
                 HUMANIZE(stats.min),
                 HUMANIZE(stats.median),
                 HUMANIZE(stats.mean),
                 HUMANIZE(stats.max),
                 stdDevPercent,
                 stats.plot.c_str(),
                 shortName);
    }
}

bool VisualLightweightBenchModule::advanceRecordIfNecessary(SkCanvas* canvas) {
    if (!fBenchmark && fState == kWarmup_State) {
        fBenchmark.reset(new WarmupBench);
        return true;
    }

    if (fBenchmark) {
        return true;
    }

    fBenchmark.reset(fBenchmarkStream->next());
    if (!fBenchmark) {
        return false;
    }

    fOwner->clear(canvas, SK_ColorWHITE, 2);

    fBenchmark->preDraw();
    fRecords.push_back();

    // Log bench name
    fResults->bench(fBenchmark->getUniqueName(), fBenchmark->getSize().fX,
                    fBenchmark->getSize().fY);
    return true;
}

inline void VisualLightweightBenchModule::nextState(State nextState) {
    fState = nextState;
}

void VisualLightweightBenchModule::perCanvasPreDraw(SkCanvas* canvas, State nextState) {
    fBenchmark->perCanvasPreDraw(canvas);
    fCurrentFrame = 0;
    this->nextState(nextState);
}

void VisualLightweightBenchModule::warmup(SkCanvas* canvas) {
    if (fCurrentFrame >= FLAGS_maxWarmupFrames) {
        this->nextState(kPreWarmLoopsPerCanvasPreDraw_State);
        fBenchmark.reset(nullptr);
        this->resetTimingState();
        fLoops = 1;
    } else {
        bool isEven = (fCurrentFrame++ % 2) == 0;
        if (isEven) {
            fTimer.start();
        } else {
            double elapsedMs = this->elapsed();
            if (elapsedMs < FLAGS_loopMs) {
                fLoops *= 2;
            }
            fTimer = WallTimer();
            fOwner->reset();
        }
    }
}

void VisualLightweightBenchModule::preWarm(State nextState) {
    if (fCurrentFrame >= FLAGS_gpuFrameLag) {
        // we currently time across all frames to make sure we capture all GPU work
        this->nextState(nextState);
        fCurrentFrame = 0;
        fTimer.start();
    } else {
        fCurrentFrame++;
    }
}

void VisualLightweightBenchModule::draw(SkCanvas* canvas) {
    if (!this->advanceRecordIfNecessary(canvas)) {
        SkDebugf("Exiting VisualBench successfully\n");
        fOwner->closeWindow();
        return;
    }
    this->renderFrame(canvas);
    switch (fState) {
        case kWarmup_State: {
            this->warmup(canvas);
            break;
        }
        case kPreWarmLoopsPerCanvasPreDraw_State: {
            this->perCanvasPreDraw(canvas, kPreWarmLoops_State);
            break;
        }
        case kPreWarmLoops_State: {
            this->preWarm(kTuneLoops_State);
            break;
        }
        case kTuneLoops_State: {
            this->tuneLoops();
            break;
        }
        case kPreWarmTimingPerCanvasPreDraw_State: {
            this->perCanvasPreDraw(canvas, kPreWarmTiming_State);
            break;
        }
        case kPreWarmTiming_State: {
            this->preWarm(kTiming_State);
            break;
        }
        case kTiming_State: {
            this->timing(canvas);
            break;
        }
    }
}

inline double VisualLightweightBenchModule::elapsed() {
    fTimer.end();
    return fTimer.fWall;
}

void VisualLightweightBenchModule::resetTimingState() {
    fCurrentFrame = 0;
    fTimer = WallTimer();
    fOwner->reset();
}

void VisualLightweightBenchModule::scaleLoops(double elapsedMs) {
    // Scale back the number of loops
    fLoops = (int)ceil(fLoops * FLAGS_loopMs / elapsedMs);
}

inline void VisualLightweightBenchModule::tuneLoops() {
    if (1 << 30 == fLoops) {
        // We're about to wrap.  Something's wrong with the bench.
        SkDebugf("InnerLoops wrapped\n");
        fLoops = 1;
    } else {
        double elapsedMs = this->elapsed();
        if (elapsedMs > FLAGS_loopMs) {
            this->scaleLoops(elapsedMs);
            this->nextState(kPreWarmTimingPerCanvasPreDraw_State);
        } else {
            fLoops *= 2;
            this->nextState(kPreWarmLoops_State);
        }
        this->resetTimingState();
    }
}

void VisualLightweightBenchModule::recordMeasurement() {
    double measurement = this->elapsed() / (FLAGS_frames * fLoops);
    fRecords.back().fMeasurements.push_back(measurement);
}

void VisualLightweightBenchModule::postDraw(SkCanvas* canvas) {
    fBenchmark->perCanvasPostDraw(canvas);
    fBenchmark.reset(nullptr);
    fCurrentSample = 0;
    fLoops = 1;
}

inline void VisualLightweightBenchModule::timing(SkCanvas* canvas) {
    if (fCurrentFrame >= FLAGS_frames) {
        this->recordMeasurement();
        if (fCurrentSample++ >= FLAGS_samples) {
            this->printStats();
            this->postDraw(canvas);
            this->nextState(kPreWarmLoopsPerCanvasPreDraw_State);
        } else {
            this->nextState(kPreWarmTimingPerCanvasPreDraw_State);
        }
        this->resetTimingState();
    } else {
        fCurrentFrame++;
    }
}

bool VisualLightweightBenchModule::onHandleChar(SkUnichar c) {
    return true;
}
