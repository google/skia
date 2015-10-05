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
DEFINE_bool2(verbose, v, false, "enable verbose output from the test driver.");
DEFINE_string(outResultsFile, "", "If given, write results here as JSON.");
DEFINE_string(key, "",
              "Space-separated key/value pairs to add to JSON identifying this builder.");
DEFINE_string(properties, "",
              "Space-separated key/value pairs to add to JSON identifying this run.");
DEFINE_int32(samples, 10, "Number of times to time each skp.");

static SkString humanize(double ms) {
    if (FLAGS_verbose) {
        return SkStringPrintf("%llu", (uint64_t)(ms*1e6));
    }
    return HumanizeMs(ms);
}

#define HUMANIZE(time) humanize(time).c_str()

VisualLightweightBenchModule::VisualLightweightBenchModule(VisualBench* owner)
    : fCurrentSample(0)
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
    fBenchmark->draw(fTSM.loops(), canvas);
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
                 fTSM.loops(),
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
    if (fBenchmark) {
        return true;
    }

    fBenchmark.reset(fBenchmarkStream->next());
    if (!fBenchmark) {
        return false;
    }

    fOwner->clear(canvas, SK_ColorWHITE, 2);

    fBenchmark->delayedSetup();
    fRecords.push_back();

    // Log bench name
    fResults->bench(fBenchmark->getUniqueName(), fBenchmark->getSize().fX,
                    fBenchmark->getSize().fY);
    return true;
}

void VisualLightweightBenchModule::draw(SkCanvas* canvas) {
    if (!this->advanceRecordIfNecessary(canvas)) {
        SkDebugf("Exiting VisualBench successfully\n");
        fOwner->closeWindow();
        return;
    }
    this->renderFrame(canvas);
    TimingStateMachine::ParentEvents event = fTSM.nextFrame(canvas, fBenchmark);
    switch (event) {
        case TimingStateMachine::kReset_ParentEvents:
            fOwner->reset();
            break;
        case TimingStateMachine::kTiming_ParentEvents:
            break;
        case TimingStateMachine::kTimingFinished_ParentEvents:
            fOwner->reset();
            fRecords.back().fMeasurements.push_back(fTSM.lastMeasurement());
            if (++fCurrentSample > FLAGS_samples) {
                this->printStats();
                fTSM.nextBenchmark(canvas, fBenchmark);
                fCurrentSample = 0;
                fBenchmark.reset(nullptr);
            } else {
                fTSM.nextSampleWithPrewarm();
            }
            break;
    }
}

bool VisualLightweightBenchModule::onHandleChar(SkUnichar c) {
    return true;
}
