
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrContext.h"

#include "SkGpuCanvas.h"
#include "SkGpuDevice.h"

///////////////////////////////////////////////////////////////////////////////

SkGpuCanvas::SkGpuCanvas(GrContext* context, GrRenderTarget* renderTarget) {
    SkASSERT(context);
    fContext = context;
    fContext->ref();

    this->setDevice(new SkGpuDevice(context, renderTarget))->unref();
}

SkGpuCanvas::~SkGpuCanvas() {
    // call this now, while our override of restore() is in effect
    this->restoreToCount(1);
    fContext->flush(false);
    fContext->unref();
}

///////////////////////////////////////////////////////////////////////////////

bool SkGpuCanvas::getViewport(SkIPoint* size) const {
    if (size) {
        SkDevice* device = this->getDevice();
        if (device) {
            size->set(device->width(), device->height());
        } else {
            size->set(0, 0);
        }
    }
    return true;
}

