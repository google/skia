/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "VisualBench.h"

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
#include "Timer.h"
#include "gl/GrGLInterface.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

VisualBench::VisualBench(void* hwnd, int argc, char** argv)
    : INHERITED(hwnd)
    , fCurrentLoops(1)
    , fCurrentPicture(0)
    , fCurrentFrame(0) {
    SkCommandLineFlags::Parse(argc, argv);

    SkTArray<SkString> skps;
    for (int i = 0; i < FLAGS_skps.count(); i++) {
        if (SkStrEndsWith(FLAGS_skps[i], ".skp")) {
            skps.push_back() = FLAGS_skps[i];
        } else {
            SkOSFile::Iter it(FLAGS_skps[i], ".skp");
            SkString path;
            while (it.next(&path)) {
                skps.push_back() = SkOSPath::Join(FLAGS_skps[0], path.c_str());
            }
        }
    }

    this->setTitle();
    this->setupBackend();

    // Load picture for playback
    for (int i = 0; i < skps.count(); i++) {
        SkFILEStream stream(skps[i].c_str());
        if (stream.isValid()) {
            fPictures.push_back(SkPicture::CreateFromStream(&stream));
        } else {
            SkDebugf("couldn't load picture at \"path\"\n", skps[i].c_str());
        }
    }
}

VisualBench::~VisualBench() {
    for (int i = 0; i < fPictures.count(); i++) {
        fPictures[i]->~SkPicture();
    }
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

    if (!this->attach(kNativeGL_BackEndType, 0 /*msaa*/, &fAttachmentInfo)) {
        SkDebugf("Not possible to create backend.\n");
        INHERITED::detach();
        return false;
    }

    this->setFullscreen(true);
    this->setVsync(false);

    fInterface.reset(GrGLCreateNativeInterface());
    SkASSERT(fInterface);

    // setup contexts
    fContext.reset(GrContext::Create(kOpenGL_GrBackend, (GrBackendContext)fInterface.get()));
    SkASSERT(fContext);

    // setup rendertargets
    this->setupRenderTarget();
    return true;
}

void VisualBench::setupRenderTarget() {
    fRenderTarget.reset(this->renderTarget(fAttachmentInfo, fInterface, fContext));
}

void VisualBench::draw(SkCanvas* canvas) {
    fCurrentFrame++;
    WallTimer timer;
    timer.start();
    for (int i = 0; i < fCurrentLoops; i++) {
        canvas->drawPicture(fPictures[fCurrentPicture]);
    }
    // in case we have queued drawing calls
    fContext->flush();
    INHERITED::present();
    timer.end();

    SkDebugf("%s\n", HumanizeMs(timer.fWall).c_str());

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

