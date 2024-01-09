/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if __ANDROID_API__ >= 26

#include "include/android/graphite/SurfaceAndroid.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "tests/Test.h"

#include <android/hardware_buffer.h>

using namespace skgpu::graphite;

static const int DEV_W = 16, DEV_H = 16;

namespace {

SkPMColor get_src_color(int x, int y) {
    SkASSERT(x >= 0 && x < DEV_W);
    SkASSERT(y >= 0 && y < DEV_H);

    U8CPU r = x;
    U8CPU g = y;
    U8CPU b = 0xc;

    U8CPU a = 0xff;
    switch ((x+y) % 5) {
        case 0:
            a = 0xff;
            break;
        case 1:
            a = 0x80;
            break;
        case 2:
            a = 0xCC;
            break;
        case 4:
            a = 0x01;
            break;
        case 3:
            a = 0x00;
            break;
    }
    a = 0xff;
    return SkPremultiplyARGBInline(a, r, g, b);
}

SkBitmap make_src_bitmap() {
    static SkBitmap bmp;
    if (bmp.isNull()) {
        bmp.allocN32Pixels(DEV_W, DEV_H);
        intptr_t pixels = reinterpret_cast<intptr_t>(bmp.getPixels());
        for (int y = 0; y < DEV_H; ++y) {
            for (int x = 0; x < DEV_W; ++x) {
                SkPMColor* pixel = reinterpret_cast<SkPMColor*>(
                        pixels + y * bmp.rowBytes() + x * bmp.bytesPerPixel());
                *pixel = get_src_color(x, y);
            }
        }
    }
    return bmp;
}

bool check_read(skiatest::Reporter* reporter, const SkBitmap& expectedBitmap,
                const SkBitmap& actualBitmap) {
    bool result = true;
    for (int y = 0; y < DEV_H && result; ++y) {
        for (int x = 0; x < DEV_W && result; ++x) {
            const uint32_t srcPixel = *expectedBitmap.getAddr32(x, y);
            const uint32_t dstPixel = *actualBitmap.getAddr32(x, y);
            if (srcPixel != dstPixel) {
                ERRORF(reporter, "Expected readback pixel (%d, %d) value 0x%08x, got 0x%08x.",
                       x, y,  srcPixel, dstPixel);
                result = false;
            }/* else {
                SkDebugf("Got good pixel (%d, %d) value 0x%08x, got 0x%08x.\n",
                       x, y,  srcPixel, dstPixel);
            }*/
        }
    }
    return result;
}

AHardwareBuffer* create_AHB(skiatest::Reporter* reporter,
                            int width, int height,
                            bool forSurface, bool isProtected,
                            const SkBitmap* data) {

    AHardwareBuffer_Desc hwbDesc;
    hwbDesc.width = width;
    hwbDesc.height = height;
    hwbDesc.layers = 1;
    hwbDesc.usage = AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
                    AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                    (isProtected ? AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT : 0);

    if (forSurface) {
        SkASSERT(!data);
        hwbDesc.usage |= AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER |
                         AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT;
    } else {
        hwbDesc.usage |= AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
    }

    hwbDesc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    // The following three are not used by AHardwareBuffer_allocate
    hwbDesc.stride = 0;
    hwbDesc.rfu0= 0;
    hwbDesc.rfu1= 0;

    AHardwareBuffer* buffer = nullptr;
    if (int error = AHardwareBuffer_allocate(&hwbDesc, &buffer)) {
        ERRORF(reporter, "Failed to allocated hardware buffer, error: %d", error);
        return nullptr;
    }

    if (data) {
        SkASSERT(data->width() == width && data->height() == height);
        // Get actual desc for allocated buffer so we know the stride for uploading cpu data.
        AHardwareBuffer_describe(buffer, &hwbDesc);

        uint32_t* bufferAddr;
        if (AHardwareBuffer_lock(buffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN, -1, nullptr,
                                 reinterpret_cast<void**>(&bufferAddr))) {
            ERRORF(reporter, "Failed to lock hardware buffer");
            AHardwareBuffer_release(buffer);
            return nullptr;
        }

        int bbp = data->bytesPerPixel();
        uint32_t* src = (uint32_t*)data->getPixels();
        int nextLineStep = width;
        uint32_t* dst = bufferAddr;
        for (int y = 0; y < height; ++y) {
            memcpy(dst, src, width * bbp);
            src += nextLineStep;
            dst += hwbDesc.stride;
        }
        AHardwareBuffer_unlock(buffer, nullptr);
    }

    return buffer;
}

void delete_buffer(void* context) {
    AHardwareBuffer* buffer = static_cast<AHardwareBuffer*>(context);
    if (buffer) {
        AHardwareBuffer_release(buffer);
    }
}

} // anonymous namespace

// Test to make sure we can import an AHardwareBuffer into an SkSurface and draw into it.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(Graphite_AHardwareBuffer_ImportAsSurface,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    if (!context->priv().caps()->supportsAHardwareBufferImages()) {
        return;
    }

    bool isProtected = context->priv().caps()->protectedSupport();

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    ///////////////////////////////////////////////////////////////////////////
    // Setup SkBitmaps
    ///////////////////////////////////////////////////////////////////////////

    const SkBitmap srcBitmap = make_src_bitmap();

    AHardwareBuffer* buffer = create_AHB(reporter,
                                         DEV_W, DEV_H,
                                         /* writeable= */ true,
                                         isProtected,
                                         /* data= */ nullptr);
    if (!buffer) {
        return;
    }

    sk_sp<SkSurface> surface = SkSurfaces::WrapAndroidHardwareBuffer(recorder.get(),
                                                                     buffer,
                                                                     /* colorSpace= */ nullptr,
                                                                     /* surfaceProps= */ nullptr,
                                                                     delete_buffer,
                                                                     buffer);
    if (!surface) {
        ERRORF(reporter, "Failed to make SkSurface.");
        return;
    }

    sk_sp<SkImage> grBacked = SkImages::TextureFromImage(recorder.get(), srcBitmap.asImage().get());

    surface->getCanvas()->drawImage(grBacked, 0, 0);

    if (!isProtected) {
        // In Protected mode we can't readback so we just test that we can wrap the AHB and
        // draw it w/o errors
        SkBitmap readbackBitmap;
        readbackBitmap.allocN32Pixels(DEV_W, DEV_H);

        REPORTER_ASSERT(reporter, surface->readPixels(readbackBitmap, 0, 0));
        REPORTER_ASSERT(reporter, check_read(reporter, srcBitmap, readbackBitmap));
    }

    surface.reset();
}

#endif // __ANDROID_API__ >= 26
