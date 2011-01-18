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

#ifndef SkGpuDeviceFactory_DEFINED
#define SkGpuDeviceFactory_DEFINED

#include "SkDevice.h"

class GrContext;

class SkGpuDeviceFactory : public SkDeviceFactory {
public:
    /**
     *  The constructor will ref() the context, passing it to each device
     *  that it creates. It will be unref()'d in the destructor
     *  Non-layered devices created by the factory will draw to the
     *  rootRenderTarget. rootRenderTarget is ref-counted by the factory.
     *  SkGpuDevice::Current3DApiRenderTarget() can be passed as a special
     *  value that will cause the factory to create a render target object
     *  that reflects the state of the underlying 3D API at the time of
     *  construction.
     */
    SkGpuDeviceFactory(GrContext*, GrRenderTarget* rootRenderTarget);

    virtual ~SkGpuDeviceFactory();

    virtual SkDevice* newDevice(SkCanvas*, SkBitmap::Config, int width,
                                int height, bool isOpaque, bool isLayer);

private:
    GrContext* fContext;
    GrRenderTarget* fRootRenderTarget;
};

#endif

