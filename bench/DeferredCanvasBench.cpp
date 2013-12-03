/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkDeferredCanvas.h"
#include "SkDevice.h"
#include "SkString.h"

class DeferredCanvasBench : public SkBenchmark {
public:
    DeferredCanvasBench(const char name[])  {
        fName.printf("deferred_canvas_%s", name);
    }

    enum {
        CANVAS_WIDTH = 200,
        CANVAS_HEIGHT = 200,
    };
protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) {
        SkBaseDevice *device = canvas->getDevice()->createCompatibleDevice(
            SkBitmap::kARGB_8888_Config, CANVAS_WIDTH, CANVAS_HEIGHT, false);

        SkAutoTUnref<SkDeferredCanvas> deferredCanvas(SkDeferredCanvas::Create(device));
        device->unref();

        initDeferredCanvas(deferredCanvas);
        drawInDeferredCanvas(loops, deferredCanvas);
        finalizeDeferredCanvas(deferredCanvas);
        deferredCanvas->flush();
    }

    virtual void initDeferredCanvas(SkDeferredCanvas* canvas) = 0;
    virtual void drawInDeferredCanvas(const int loops, SkDeferredCanvas* canvas) = 0;
    virtual void finalizeDeferredCanvas(SkDeferredCanvas* canvas) = 0;

    SkString fName;

private:
    typedef SkBenchmark INHERITED;
};

class SimpleNotificationClient : public SkDeferredCanvas::NotificationClient {
public:
    SimpleNotificationClient() : fDummy(false) {}

    //bogus virtual implementations that just do something small
    virtual void prepareForDraw() SK_OVERRIDE {fDummy = true;}
    virtual void storageAllocatedForRecordingChanged(size_t) SK_OVERRIDE {fDummy = false;}
    virtual void flushedDrawCommands() SK_OVERRIDE {fDummy = !fDummy;}
private:
    bool fDummy;

    typedef SkDeferredCanvas::NotificationClient INHERITED;
};

// Test that records very simple draw operations.
// This benchmark aims to capture performance fluctuations in the recording
// overhead of SkDeferredCanvas
class DeferredRecordBench : public DeferredCanvasBench {
public:
    DeferredRecordBench()
        : INHERITED("record") {
    }

protected:

    virtual void initDeferredCanvas(SkDeferredCanvas* canvas) SK_OVERRIDE {
        canvas->setNotificationClient(&fNotificationClient);
    }

    virtual void drawInDeferredCanvas(const int loops, SkDeferredCanvas* canvas) SK_OVERRIDE {
        SkRect rect;
        rect.setXYWH(0, 0, 10, 10);
        SkPaint paint;
        for (int i = 0; i < loops; i++) {
            canvas->save(SkCanvas::kMatrixClip_SaveFlag);
            canvas->translate(SkIntToScalar(i * 27 % CANVAS_WIDTH), SkIntToScalar(i * 13 % CANVAS_HEIGHT));
            canvas->drawRect(rect, paint);
            canvas->restore();
        }
    }

    virtual void finalizeDeferredCanvas(SkDeferredCanvas* canvas) SK_OVERRIDE {
        canvas->clear(0x0);
        canvas->setNotificationClient(NULL);
    }

private:
    typedef DeferredCanvasBench INHERITED;
    SimpleNotificationClient fNotificationClient;
};


///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new DeferredRecordBench(); )
