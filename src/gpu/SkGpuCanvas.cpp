/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrContext.h"

#include "SkGpuCanvas.h"
#include "SkGpuDevice.h"
#include "SkGpuDeviceFactory.h"

///////////////////////////////////////////////////////////////////////////////

SkGpuCanvas::SkGpuCanvas(GrContext* context, GrRenderTarget* renderTarget) {
    SkDeviceFactory* factory = SkNEW_ARGS(SkGpuDeviceFactory,
                                          (context, renderTarget));
    this->setDeviceFactory(factory)->unref();

    SkASSERT(context);
    fContext = context;
    fContext->ref();
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

