/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Request_DEFINED
#define Request_DEFINED

#include "GrContextFactory.h"

#include "SkDebugCanvas.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkSurface.h"

#include "UrlDataManager.h"

struct MHD_Connection;
struct MHD_PostProcessor;

struct UploadContext {
    SkDynamicMemoryWStream fStream;
    MHD_PostProcessor* fPostProcessor;
    MHD_Connection* connection;
};

struct Request {
    Request(SkString rootUrl); 

    // draws to skia draw op N, highlighting the Mth batch(-1 means no highlight)
    SkData* drawToPng(int n, int m = -1);
    SkData* writeOutSkp();
    SkCanvas* getCanvas();
    SkBitmap* getBitmapFromCanvas(SkCanvas* canvas);
    bool enableGPU(bool enable);
    bool hasPicture() const { return SkToBool(fPicture.get()); }
    int getLastOp() const { return fDebugCanvas->getSize() - 1; }

    bool initPictureFromStream(SkStream*);

    // Returns the json list of ops as an SkData
    SkData* getJsonOps(int n);

    // Returns a json list of batches as an SkData
    SkData* getJsonBatchList(int n);

    // Returns json with the viewMatrix and clipRect
    SkData* getJsonInfo(int n);

    // returns the color of the pixel at (x,y) in the canvas
    SkColor getPixel(int x, int y);

    UploadContext* fUploadContext;
    SkAutoTUnref<SkDebugCanvas> fDebugCanvas;
    UrlDataManager fUrlDataManager;
    
private:
    SkData* writeCanvasToPng(SkCanvas* canvas);
    void drawToCanvas(int n, int m = -1);
    SkSurface* createCPUSurface();
    SkSurface* createGPUSurface();
    GrAuditTrail* getAuditTrail(SkCanvas*);
    void cleanupAuditTrail(SkCanvas*);
    
    SkAutoTUnref<SkPicture> fPicture;
    SkAutoTDelete<GrContextFactory> fContextFactory;
    SkAutoTUnref<SkSurface> fSurface;
    bool fGPUEnabled;
};

#endif

