/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "VisualBench.h"

#include "ProcStats.h"
#include "SkApplication.h"
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkCommonFlags.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkGr.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "Stats.h"
#include "gl/GrGLInterface.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_int32(gpuFrameLag, 5, "Overestimate of maximum number of frames GPU allows to lag.");
DEFINE_int32(samples, 10, "Number of times to render each skp.");
DEFINE_int32(loops, 5, "Number of times to time.");
DEFINE_int32(msaa, 0, "Number of msaa samples.");
DEFINE_bool2(fullscreen, f, true, "Run fullscreen.");

static SkString humanize(double ms) {
    if (FLAGS_verbose) {
        return SkStringPrintf("%llu", (uint64_t)(ms*1e6));
    }
    return HumanizeMs(ms);
}

#define HUMANIZE(time) humanize(time).c_str()

VisualBench::VisualBench(void* hwnd, int argc, char** argv)
    : INHERITED(hwnd)
    , fLoop(0)
    , fCurrentPictureIdx(-1)
    , fCurrentSample(0)
    , fState(kPreWarm_State) {
    SkCommandLineFlags::Parse(argc, argv);

    // read all the skp file names.
    for (int i = 0; i < FLAGS_skps.count(); i++) {
        if (SkStrEndsWith(FLAGS_skps[i], ".skp")) {
            fRecords.push_back().fFilename = FLAGS_skps[i];
        } else {
            SkOSFile::Iter it(FLAGS_skps[i], ".skp");
            SkString path;
            while (it.next(&path)) {
                fRecords.push_back().fFilename = SkOSPath::Join(FLAGS_skps[i], path.c_str());;
            }
        }
    }

    if (fRecords.empty()) {
        SkDebugf("no valid skps found\n");
    }

    this->setTitle();
    this->setupBackend();
}

VisualBench::~VisualBench() {
    INHERITED::detach();
}

void VisualBench::setTitle() {
    SkString title("VisualBench");
    INHERITED::setTitle(title.c_str());
}

SkSurface* VisualBench::createSurface() {
    SkSurfaceProps props(INHERITED::getSurfaceProps());
    return SkSurface::NewRenderTargetDirect(fRenderTarget, &props);
}

bool VisualBench::setupBackend() {
    this->setColorType(kRGBA_8888_SkColorType);
    this->setVisibleP(true);
    this->setClipToBounds(false);

    if (FLAGS_fullscreen) {
        if (!this->makeFullscreen()) {
            SkDebugf("Could not go fullscreen!");
        }
    }
    if (!this->attach(kNativeGL_BackEndType, FLAGS_msaa, &fAttachmentInfo)) {
        SkDebugf("Not possible to create backend.\n");
        INHERITED::detach();
        return false;
    }

    this->setVsync(false);
    this->resetContext();
    return true;
}

void VisualBench::resetContext() {
    fInterface.reset(GrGLCreateNativeInterface());
    SkASSERT(fInterface);

    // setup contexts
    fContext.reset(GrContext::Create(kOpenGL_GrBackend, (GrBackendContext)fInterface.get()));
    SkASSERT(fContext);

    // setup rendertargets
    this->setupRenderTarget();
}

void VisualBench::setupRenderTarget() {
    if (fContext) {
        fRenderTarget.reset(this->renderTarget(fAttachmentInfo, fInterface, fContext));
    }
}

inline void VisualBench::renderFrame(SkCanvas* canvas) {
    canvas->drawPicture(fPicture);
    canvas->flush();
    INHERITED::present();
}

void VisualBench::printStats() {
    const SkTArray<double>& measurements = fRecords[fCurrentPictureIdx].fMeasurements;
    SkString shortName = SkOSPath::Basename(fRecords[fCurrentPictureIdx].fFilename.c_str());
    if (FLAGS_verbose) {
        for (int i = 0; i < measurements.count(); i++) {
            SkDebugf("%s  ", HUMANIZE(measurements[i]));
        }
        SkDebugf("%s\n", shortName.c_str());
    } else {
        SkASSERT(measurements.count());
        Stats stats(measurements.begin(), measurements.count());
        const double stdDevPercent = 100 * sqrt(stats.var) / stats.mean;
        SkDebugf("%4d/%-4dMB\t%s\t%s\t%s\t%s\t%.0f%%\t%s\n",
                 sk_tools::getCurrResidentSetSizeMB(),
                 sk_tools::getMaxResidentSetSizeMB(),
                 HUMANIZE(stats.min),
                 HUMANIZE(stats.median),
                 HUMANIZE(stats.mean),
                 HUMANIZE(stats.max),
                 stdDevPercent,
                 shortName.c_str());
    }
}

bool VisualBench::advanceRecordIfNecessary() {
    if (fPicture) {
        return true;
    }
    ++fCurrentPictureIdx;
    while (true) {
        if (fCurrentPictureIdx >= fRecords.count()) {
            return false;
        }
        if (this->loadPicture()) {
            return true;
        }
        fRecords.removeShuffle(fCurrentPictureIdx);
    }
}

bool VisualBench::loadPicture() {
    const char* fileName = fRecords[fCurrentPictureIdx].fFilename.c_str();
    SkFILEStream stream(fileName);
    if (stream.isValid()) {
        fPicture.reset(SkPicture::CreateFromStream(&stream));
        if (SkToBool(fPicture)) {
            return true;
        }
    }
    SkDebugf("couldn't load picture at \"%s\"\n", fileName);
    return false;
}

void VisualBench::draw(SkCanvas* canvas) {
    if (!this->advanceRecordIfNecessary()) {
        this->closeWindow();
        return;
    }
    this->renderFrame(canvas);
    switch (fState) {
        case kPreWarm_State: {
            if (fCurrentSample >= FLAGS_gpuFrameLag) {
                // TODO we currently time across all frames to make sure we capture all GPU work
                // We should also rendering an empty SKP to get a baseline to subtract from
                // our timing
                fState = kTiming_State;
                fCurrentSample -= FLAGS_gpuFrameLag;
                fTimer.start();
            } else {
                fCurrentSample++;
            }
            break;
        }
        case kTiming_State: {
            if (fCurrentSample >= FLAGS_samples) {
                fTimer.end();
                fRecords[fCurrentPictureIdx].fMeasurements.push_back(fTimer.fWall / FLAGS_samples);
                this->resetContext();
                fTimer = WallTimer();
                fState = kPreWarm_State;
                fCurrentSample = 0;
                if (fLoop++ > FLAGS_loops) {
                    this->printStats();
                    fPicture.reset(NULL);
                    fLoop = 0;
                }
            } else {
                fCurrentSample++;
            }
            break;
        }
    }

    // Invalidate the window to force a redraw. Poor man's animation mechanism.
    this->inval(NULL);
}

void VisualBench::onSizeChange() {
    this->setupRenderTarget();
}

bool VisualBench::onHandleChar(SkUnichar unichar) {
    return true;
}

// Externally declared entry points
void application_init() {
    SkGraphics::Init();
    SkEvent::Init();
}

void application_term() {
    SkEvent::Term();
    SkGraphics::Term();
}

SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv) {
    return new VisualBench(hwnd, argc, argv);
}

