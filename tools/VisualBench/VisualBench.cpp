/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "VisualBench.h"

#include "GrContext.h"
#include "ProcStats.h"
#include "SkApplication.h"
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkGr.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "Stats.h"
#include "VisualDebugModule.h"
#include "VisualLightweightBenchModule.h"
#include "VisualInteractiveModule.h"
#include "gl/GrGLInterface.h"

#include <stdlib.h>

DEFINE_bool2(fullscreen, f, true, "Run fullscreen.");
DEFINE_string(mode, "classic", "one of: classic interactive debugger");
DEFINE_bool2(dif, d, false, "Use device-independent fonts.");

VisualBench::VisualBench(void* hwnd, int argc, char** argv)
    : INHERITED(hwnd) {
    SkDebugf("Command line arguments: ");
    for (int i = 1; i < argc; ++i) {
        SkDebugf("%s ", argv[i]);
    }
    SkDebugf("\n");

    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_nvpr && !FLAGS_msaa) {
        SkDebugf("Got nvpr without msaa. Exiting.\n");
        exit(-1);
    }

    // these have to happen after commandline parsing
    if (FLAGS_dif) {
        const SkSurfaceProps& props(INHERITED::getSurfaceProps());
        uint32_t flags = SkSurfaceProps::kUseDeviceIndependentFonts_Flag | props.flags();
        INHERITED::setSurfaceProps(SkSurfaceProps(flags, props.pixelGeometry()));
    }
    fModule.reset(new VisualLightweightBenchModule(this));

    if (FLAGS_mode.count()) {
        SkASSERT(FLAGS_mode.count() == 1);
        SkString mode(FLAGS_mode[0]);
        if (mode == SkString("interactive")) {
            fModule.reset(new VisualInteractiveModule(this));
        } else if (mode == SkString("debugger")) {
            fModule.reset(new VisualDebugModule(this));
        }
    }

    this->setTitle();
    this->setupBackend();
}

VisualBench::~VisualBench() {
    this->tearDownContext();
}

void VisualBench::setTitle() {
    SkString title("VisualBench");
    INHERITED::setTitle(title.c_str());
}

SkSurface* VisualBench::createSurface() {
    if (!fSurface) {
        SkSurfaceProps props(INHERITED::getSurfaceProps());
        fSurface.reset(SkSurface::NewRenderTargetDirect(fRenderTarget, &props));
    }

    // The caller will wrap the SkSurface in an SkAutoTUnref
    return SkRef(fSurface.get());
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

    this->resetContext();
    return true;
}

void VisualBench::resetContext() {
    this->tearDownContext();
    this->setupContext();
}

void VisualBench::setupContext() {
    int screenSamples = FLAGS_offscreen ? 0 : FLAGS_msaa;
    if (!this->attach(kNativeGL_BackEndType, screenSamples, &fAttachmentInfo)) {
        SkDebugf("Not possible to create backend.\n");
        INHERITED::detach();
        SkFAIL("Could not create backend\n");
    }

    this->setVsync(false);

    fSurface.reset(nullptr);

    fInterface.reset(GrGLCreateNativeInterface());

    // TODO use the GLContext creation factories and also set this all up in configs
    if (!FLAGS_nvpr) {
        fInterface.reset(GrGLInterfaceRemoveNVPR(fInterface));
    }
    SkASSERT(fInterface);

    // setup contexts
    fContext.reset(GrContext::Create(kOpenGL_GrBackend, (GrBackendContext)fInterface.get()));
    SkASSERT(fContext);

    // setup rendertargets
    this->setupRenderTarget();
}

void VisualBench::tearDownContext() {
    if (fContext) {
        // We abandon the context in case SkWindow has kept a ref to the surface
        fContext->abandonContext();
        fContext.reset();
        fSurface.reset();
        fInterface.reset();
        this->detach();
    }
}

void VisualBench::setupRenderTarget() {
    if (fContext) {
        fRenderTarget.reset(this->renderTarget(fAttachmentInfo, fInterface, fContext));
    }
}

void VisualBench::draw(SkCanvas* canvas) {
    fModule->draw(canvas);

    // Invalidate the window to force a redraw. Poor man's animation mechanism.
    this->inval(nullptr);
}

void VisualBench::clear(SkCanvas* canvas, SkColor color, int frames) {
    canvas->clear(color);
    for (int i = 0; i < frames - 1; ++i) {
        canvas->flush();
        this->present();
        canvas->clear(color);
    }
}

void VisualBench::onSizeChange() {
    this->setupRenderTarget();
}

bool VisualBench::onHandleChar(SkUnichar unichar) {
    static const auto kEscKey = 27;
    if (kEscKey == unichar) {
        this->closeWindow();
        return true;
    }

    return fModule->onHandleChar(unichar);
}

// Externally declared entry points
void application_init() {
    SkGraphics::Init();
    SkEvent::Init();
}

void application_term() {
    SkEvent::Term();
}

SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv) {
    return new VisualBench(hwnd, argc, argv);
}

