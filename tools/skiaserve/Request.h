/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Request_DEFINED
#define Request_DEFINED

#include "SkTypes.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#endif

#include "SkDebugCanvas.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkSurface.h"

#include "UrlDataManager.h"

namespace sk_gpu_test {
class GrContextFactory;
}
struct MHD_Connection;
struct MHD_PostProcessor;

struct UploadContext {
    SkDynamicMemoryWStream fStream;
    MHD_PostProcessor* fPostProcessor;
    MHD_Connection* connection;
};

struct Request {
    Request(SkString rootUrl);
    ~Request();

    // draws to canvas operation N, highlighting the Mth GrOp. m = -1 means no highlight.
    sk_sp<SkData> drawToPng(int n, int m = -1);
    sk_sp<SkData> writeOutSkp();
    SkCanvas* getCanvas();
    SkBitmap* getBitmapFromCanvas(SkCanvas* canvas);
    bool enableGPU(bool enable);
    bool setOverdraw(bool enable);
    bool setColorMode(int mode);
    bool hasPicture() const { return SkToBool(fPicture.get()); }
    int getLastOp() const { return fDebugCanvas->getSize() - 1; }

    bool initPictureFromStream(SkStream*);

    // Returns the json list of ops as an SkData
    sk_sp<SkData> getJsonOps(int n);

    // Returns a json list of ops as an SkData
    sk_sp<SkData> getJsonOpList(int n);

    // Returns json with the viewMatrix and clipRect
    sk_sp<SkData> getJsonInfo(int n);

    // returns the color of the pixel at (x,y) in the canvas
    SkColor getPixel(int x, int y);

    UploadContext* fUploadContext;
    std::unique_ptr<SkDebugCanvas> fDebugCanvas;
    UrlDataManager fUrlDataManager;

private:
    sk_sp<SkData> writeCanvasToPng(SkCanvas* canvas);
    void drawToCanvas(int n, int m = -1);
    SkSurface* createCPUSurface();
    SkSurface* createGPUSurface();
    SkIRect getBounds();
    GrContext* getContext();

    sk_sp<SkPicture> fPicture;
    sk_gpu_test::GrContextFactory* fContextFactory;
    sk_sp<SkSurface> fSurface;
    bool fGPUEnabled;
    bool fOverdraw;
    int fColorMode;
};

#endif
