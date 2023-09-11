/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "src/core/SkDevice.h"
#include "src/core/SkSpecialImage.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/Device.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

struct GrContextOptions;

class DeviceTestingAccess {
public:
    static sk_sp<SkSpecialImage> MakeSpecial(SkDevice* dev, const SkBitmap& bm) {
        return dev->makeSpecial(bm);
    }

    static sk_sp<SkSpecialImage> MakeSpecial(SkDevice* dev, SkImage* img) {
        return dev->makeSpecial(img);
    }

    static sk_sp<SkSpecialImage> SnapSpecial(SkDevice* dev) {
        return dev->snapSpecial();
    }
};

// TODO: re-enable this when Raster methods are implemented
#if 0
DEF_TEST(SpecialImage_BitmapDevice, reporter) {
    static const int kWidth = 100;
    static const int kHeight = 90;

    SkImageInfo ii = SkImageInfo::MakeN32Premul(2*kWidth, 2*kHeight);

    sk_sp<SkDevice> bmDev = SkBitmapDevice::Create(ii);

    SkBitmap bm;
    bm.tryAllocN32Pixels(kWidth, kHeight);

    // Create a raster-backed special image from a raster-backed SkBitmap
    sk_sp<SkSpecialImage> special = DeviceTestingAccess::MakeSpecial(bmDev.get(), bm);
    SkASSERT(!special->isTextureBacked());
    SkASSERT(kWidth == special->width());
    SkASSERT(kHeight == special->height());
    SkASSERT(bm.getGenerationID() == special->uniqueID());
    SkASSERT(SkIRect::MakeWH(kWidth, kHeight) == special->subset());

    // Create a raster-backed special image from a raster-backed SkImage
    sk_sp<SkImage> image(SkImages::RasterFromBitmap(bm));
    special = DeviceTestingAccess::MakeSpecial(bmDev.get(), image.get());
    SkASSERT(!special->isTextureBacked());
    SkASSERT(kWidth == special->width());
    SkASSERT(kHeight == special->height());
    SkASSERT(bm.getGenerationID() == special->uniqueID());
    SkASSERT(SkIRect::MakeWH(kWidth, kHeight) == special->subset());

    // Snap the device as a raster-backed special image
    special = DeviceTestingAccess::SnapSpecial(bmDev.get());
    SkASSERT(!special->isTextureBacked());
    SkASSERT(2*kWidth == special->width());
    SkASSERT(2*kHeight == special->height());
    SkASSERT(SkIRect::MakeWH(2*kWidth, 2*kHeight) == special->subset());
}
#endif

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SpecialImage_GPUDevice,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();

    static const int kWidth = 100;
    static const int kHeight = 90;

    SkImageInfo ii = SkImageInfo::MakeN32Premul(2*kWidth, 2*kHeight);

    auto device = dContext->priv().createDevice(skgpu::Budgeted::kNo,
                                                ii,
                                                SkBackingFit::kExact,
                                                1,
                                                skgpu::Mipmapped::kNo,
                                                GrProtected::kNo,
                                                kBottomLeft_GrSurfaceOrigin,
                                                SkSurfaceProps(),
                                                skgpu::ganesh::Device::InitContents::kClear);

    SkBitmap bm;
    SkAssertResult(bm.tryAllocN32Pixels(kWidth, kHeight));

    // Create a gpu-backed special image from a raster-backed SkBitmap
    sk_sp<SkSpecialImage> special = DeviceTestingAccess::MakeSpecial(device.get(), bm);
    SkASSERT(special->isGaneshBacked());
    SkASSERT(kWidth == special->width());
    SkASSERT(kHeight == special->height());
    SkASSERT(bm.getGenerationID() == special->uniqueID());
    SkASSERT(SkIRect::MakeWH(kWidth, kHeight) == special->subset());

    // Create a gpu-backed special image from a raster-backed SkImage
    sk_sp<SkImage> image(bm.asImage());
    special = DeviceTestingAccess::MakeSpecial(device.get(), image.get());
    SkASSERT(special->isGaneshBacked());
    SkASSERT(kWidth == special->width());
    SkASSERT(kHeight == special->height());
    // TODO: Hmmm, this is a bit unexpected
    SkASSERT(image->uniqueID() != special->uniqueID());
    SkASSERT(SkIRect::MakeWH(kWidth, kHeight) == special->subset());

    // Create a gpu-backed special image from a gpu-backed SkImage
    image = SkImages::TextureFromImage(dContext, image);
    special = DeviceTestingAccess::MakeSpecial(device.get(), image.get());
    SkASSERT(special->isGaneshBacked());
    SkASSERT(kWidth == special->width());
    SkASSERT(kHeight == special->height());
    SkASSERT(image->uniqueID() == special->uniqueID());
    SkASSERT(SkIRect::MakeWH(kWidth, kHeight) == special->subset());

    // Snap the device as a gpu-backed special image
    special = DeviceTestingAccess::SnapSpecial(device.get());
    SkASSERT(special->isGaneshBacked());
    SkASSERT(2*kWidth == special->width());
    SkASSERT(2*kHeight == special->height());
    SkASSERT(SkIRect::MakeWH(2*kWidth, 2*kHeight) == special->subset());
}
