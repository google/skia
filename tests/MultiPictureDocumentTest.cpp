/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This test confirms that a MultiPictureDocument can be serialized and deserialized without error.
 * And that the pictures within it are re-created accurately
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkDocument.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/docs/SkMultiPictureDocument.h"
#include "tests/Test.h"
#include "tools/SkSharingProc.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <memory>
#include <vector>

// Covers rects, ovals, paths, images, text
static void draw_basic(SkCanvas* canvas, int seed, sk_sp<SkImage> image) {
    canvas->drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(seed);
    paint.setColor(SK_ColorRED);

    SkRect rect = SkRect::MakeXYWH(50+seed, 50+seed, 4*seed, 60);
    canvas->drawRect(rect, paint);

    SkRRect oval;
    oval.setOval(rect);
    oval.offset(40, 60+seed);
    paint.setColor(SK_ColorBLUE);
    canvas->drawRRect(oval, paint);

    paint.setColor(SK_ColorCYAN);
    canvas->drawCircle(180, 50, 5*seed, paint);

    rect.offset(80, 0);
    paint.setColor(SK_ColorYELLOW);
    canvas->drawRoundRect(rect, 10, 10, paint);

    SkPath path;
    path.cubicTo(768, 0, -512, 256, 256, 256);
    paint.setColor(SK_ColorGREEN);
    canvas->drawPath(path, paint);

    canvas->drawImage(image, 128-seed, 128, SkSamplingOptions(), &paint);

    if (seed % 2 == 0) {
        SkRect rect2 = SkRect::MakeXYWH(0, 0, 40, 60);
        canvas->drawImageRect(image, rect2, SkSamplingOptions(), &paint);
    }

    SkPaint paint2;
    SkFont font = ToolUtils::DefaultFont();
    font.setSize(2 + seed);
    auto text = SkTextBlob::MakeFromString(SkStringPrintf("Frame %d", seed).c_str(), font);
    canvas->drawTextBlob(text.get(), 50, 25, paint2);
}

// Covers all of the above and drawing nested sub-pictures.
static void draw_advanced(SkCanvas* canvas, int seed, sk_sp<SkImage> image, sk_sp<SkPicture> sub) {
    draw_basic(canvas, seed, image);

    // Use subpicture twice in different places
    canvas->drawPicture(sub);
    canvas->save();
    canvas->translate(seed, seed);
    canvas->drawPicture(sub);
    canvas->restore();
}

// Test serialization and deserialization of multi picture document
DEF_TEST(SkMultiPictureDocument_Serialize_and_deserialize, reporter) {
    // Create the stream we will serialize into.
    SkDynamicMemoryWStream stream;

    // Create the image sharing proc.
    SkSharingSerialContext ctx;
    SkSerialProcs procs;
    procs.fImageProc = SkSharingSerialContext::serializeImage;
    procs.fImageCtx = &ctx;

    // Create the multi picture document used for recording frames.
    sk_sp<SkDocument> multipic = SkMultiPictureDocument::Make(&stream, &procs);

    static const int NUM_FRAMES = 12;
    static const int WIDTH = 256;
    static const int HEIGHT = 256;

    // Make an image to be used in a later step.
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100)));
    surface->getCanvas()->clear(SK_ColorGREEN);
    sk_sp<SkImage> image(surface->makeImageSnapshot());
    REPORTER_ASSERT(reporter, image);

    // Make a subpicture to be used in a later step
    SkPictureRecorder pr;
    SkCanvas* subCanvas = pr.beginRecording(100, 100);
    draw_basic(subCanvas, 42, image);
    sk_sp<SkPicture> sub = pr.finishRecordingAsPicture();

    const SkImageInfo info = SkImageInfo::MakeN32Premul(WIDTH, HEIGHT);
    std::vector<sk_sp<SkImage>> expectedImages;

    for (int i=0; i<NUM_FRAMES; i++) {
        SkCanvas* pictureCanvas = multipic->beginPage(WIDTH, HEIGHT);
        draw_advanced(pictureCanvas, i, image, sub);
        multipic->endPage();
        // Also draw the picture to an image for later comparison
        auto surf = SkSurfaces::Raster(info);
        draw_advanced(surf->getCanvas(), i, image, sub);
        expectedImages.push_back(surf->makeImageSnapshot());
    }
    // Finalize
    multipic->close();

    // Confirm written data is at least as large as the magic word
    std::unique_ptr<SkStreamAsset> writtenStream = stream.detachAsStream();
    REPORTER_ASSERT(reporter, writtenStream->getLength() > 24,
        "Written data length too short (%zu)", writtenStream->getLength());
    // SkDebugf("Multi Frame file size = %zu\n", writtenStream->getLength());

    // Set up deserialization
    SkSharingDeserialContext deserialContext;
    SkDeserialProcs dprocs;
    dprocs.fImageProc = SkSharingDeserialContext::deserializeImage;
    dprocs.fImageCtx = &deserialContext;

    // Confirm data is a MultiPictureDocument
    int frame_count = SkMultiPictureDocument::ReadPageCount(writtenStream.get());
    REPORTER_ASSERT(reporter, frame_count == NUM_FRAMES,
        "Expected %d frames, got %d. \n 0 frames may indicate the written file was not a "
        "MultiPictureDocument.", NUM_FRAMES, frame_count);

    // Deserialize
    std::vector<SkDocumentPage> frames(frame_count);
    REPORTER_ASSERT(reporter,
        SkMultiPictureDocument::Read(writtenStream.get(), frames.data(), frame_count, &dprocs),
        "Failed while reading MultiPictureDocument");

    // Examine each frame.
    int i=0;
    for (const auto& frame : frames) {
        SkRect bounds = frame.fPicture->cullRect();
        REPORTER_ASSERT(reporter, bounds.width() == WIDTH,
            "Page width: expected (%d) got (%d)", WIDTH, (int)bounds.width());
        REPORTER_ASSERT(reporter, bounds.height() == HEIGHT,
            "Page height: expected (%d) got (%d)", HEIGHT, (int)bounds.height());

        auto surf = SkSurfaces::Raster(info);
        surf->getCanvas()->drawPicture(frame.fPicture);
        auto img = surf->makeImageSnapshot();
        REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(img.get(), expectedImages[i].get()),
                        "Frame %d is wrong", i);

        i++;
    }
}


#if defined(SK_GANESH) && defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "include/android/AHardwareBufferUtils.h"
#include "include/android/GrAHardwareBufferUtils.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"

#include <android/hardware_buffer.h>

static const int DEV_W = 16, DEV_H = 16;

static SkPMColor get_src_color(int x, int y) {
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

static SkBitmap make_src_bitmap() {
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

static void cleanup_resources(AHardwareBuffer* buffer) {
    if (buffer) {
        AHardwareBuffer_release(buffer);
    }
}

static sk_sp<SkImage> makeAHardwareBufferTestImage(
    skiatest::Reporter* reporter, GrDirectContext* context, AHardwareBuffer* buffer) {

    const SkBitmap srcBitmap = make_src_bitmap();

    AHardwareBuffer_Desc hwbDesc;
    hwbDesc.width = DEV_W;
    hwbDesc.height = DEV_H;
    hwbDesc.layers = 1;
    hwbDesc.usage = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN |
                    AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN |
                    AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    hwbDesc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    // The following three are not used in the allocate
    hwbDesc.stride = 0;
    hwbDesc.rfu0= 0;
    hwbDesc.rfu1= 0;

    if (int error = AHardwareBuffer_allocate(&hwbDesc, &buffer)) {
        ERRORF(reporter, "Failed to allocated hardware buffer, error: %d", error);
        cleanup_resources(buffer);
        return nullptr;
    }

    // Get actual desc for allocated buffer so we know the stride for uploading cpu data.
    AHardwareBuffer_describe(buffer, &hwbDesc);

    void* bufferAddr;
    if (AHardwareBuffer_lock(buffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN, -1, nullptr,
                             &bufferAddr)) {
        ERRORF(reporter, "Failed to lock hardware buffer");
        cleanup_resources(buffer);
        return nullptr;
    }

    // fill buffer
    int bbp = srcBitmap.bytesPerPixel();
    uint32_t* src = (uint32_t*)srcBitmap.getPixels();
    int nextLineStep = DEV_W;
    uint32_t* dst = static_cast<uint32_t*>(bufferAddr);
    for (int y = 0; y < DEV_H; ++y) {
        memcpy(dst, src, DEV_W * bbp);
        src += nextLineStep;
        dst += hwbDesc.stride;
    }
    AHardwareBuffer_unlock(buffer, nullptr);

    // Make SkImage from buffer in a way that mimics libs/hwui/AutoBackendTextureRelease
    GrBackendFormat backendFormat =
            GrAHardwareBufferUtils::GetBackendFormat(context, buffer, hwbDesc.format, false);
    GrAHardwareBufferUtils::DeleteImageProc deleteProc;
    GrAHardwareBufferUtils::UpdateImageProc updateProc;
    GrAHardwareBufferUtils::TexImageCtx imageCtx;
    GrBackendTexture texture = GrAHardwareBufferUtils::MakeBackendTexture(
        context, buffer, hwbDesc.width, hwbDesc.height,
        &deleteProc, // set by MakeBackendTexture
        &updateProc, // set by MakeBackendTexture
        &imageCtx, // set by MakeBackendTexture
        false,   // don't make protected image
        backendFormat,
        false   // isRenderable
    );
    SkColorType colorType = AHardwareBufferUtils::GetSkColorTypeFromBufferFormat(hwbDesc.format);
    sk_sp<SkImage> image = SkImages::BorrowTextureFrom(context,
                                                       texture,
                                                       kTopLeft_GrSurfaceOrigin,
                                                       colorType,
                                                       kPremul_SkAlphaType,
                                                       SkColorSpace::MakeSRGB(),
                                                       deleteProc,
                                                       imageCtx);

    REPORTER_ASSERT(reporter, image);
    REPORTER_ASSERT(reporter, image->isTextureBacked());
    return image;
}

// Test the onEndPage callback's intended use by processing an mskp containing AHardwareBuffer-backed SkImages
// Expected behavior is that the callback is called while the AHardwareBuffer is still valid and the
// images are copied so .close() can still access them.
// Confirm deserialized file contains images with correct data.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkMultiPictureDocument_AHardwarebuffer,
                                       reporter,
                                       ctx_info,
                                       CtsEnforcement::kApiLevel_T) {
    auto context = ctx_info.directContext();
    if (!context->priv().caps()->supportsAHardwareBufferImages()) {
        return;
    }

    // Create the stream we will serialize into.
    SkDynamicMemoryWStream stream;

    // Create the image sharing proc.
    SkSharingSerialContext ctx;
    SkSerialProcs procs;
    procs.fImageProc = SkSharingSerialContext::serializeImage;
    procs.fImageCtx = &ctx;

    // Create the multi picture document used for recording frames.
    // Pass a lambda as the onEndPage callback that captures our sharing context
    sk_sp<SkDocument> multipic = SkMultiPictureDocument::Make(&stream, &procs,
        [sharingCtx = &ctx](const SkPicture* pic) {
            SkSharingSerialContext::collectNonTextureImagesFromPicture(pic, sharingCtx);
        });

    static const int WIDTH = 256;
    static const int HEIGHT = 256;

    // Make an image to be used in a later step.
    AHardwareBuffer* ahbuffer = nullptr;
    sk_sp<SkImage> image = makeAHardwareBufferTestImage(reporter, context, ahbuffer);

    const SkImageInfo info = SkImageInfo::MakeN32Premul(WIDTH, HEIGHT);
    std::vector<sk_sp<SkImage>> expectedImages;

    // Record single frame
    SkCanvas* pictureCanvas = multipic->beginPage(WIDTH, HEIGHT);
    draw_basic(pictureCanvas, 0, image);
    multipic->endPage();
    // Also draw the picture to an image for later comparison
    auto surf = SkSurfaces::Raster(info);
    draw_basic(surf->getCanvas(), 0, image);
    expectedImages.push_back(surf->makeImageSnapshot());

    // Release Ahardwarebuffer. If the code under test has not copied it already,
    // close() will fail.
    // Note that this only works because we're doing one frame only. If this test were recording
    // two or more frames, it would have change the buffer contents instead.
    cleanup_resources(ahbuffer);

    // Finalize
    multipic->close();

    // Confirm written data is at least as large as the magic word
    std::unique_ptr<SkStreamAsset> writtenStream = stream.detachAsStream();
    REPORTER_ASSERT(reporter, writtenStream->getLength() > 24,
        "Written data length too short (%zu)", writtenStream->getLength());

    // Set up deserialization
    SkSharingDeserialContext deserialContext;
    SkDeserialProcs dprocs;
    dprocs.fImageProc = SkSharingDeserialContext::deserializeImage;
    dprocs.fImageCtx = &deserialContext;

    // Confirm data is a MultiPictureDocument
    int frame_count = SkMultiPictureDocument::ReadPageCount(writtenStream.get());
    REPORTER_ASSERT(reporter, frame_count == 1,
        "Expected 1 frame, got %d. \n 0 frames may indicate the written file was not a "
        "MultiPictureDocument.", frame_count);

    // Deserialize
    std::vector<SkDocumentPage> frames(frame_count);
    REPORTER_ASSERT(reporter,
        SkMultiPictureDocument::Read(writtenStream.get(), frames.data(), frame_count, &dprocs),
        "Failed while reading MultiPictureDocument");

    // Examine frame.
    SkRect bounds = frames[0].fPicture->cullRect();
    REPORTER_ASSERT(reporter, bounds.width() == WIDTH,
        "Page width: expected (%d) got (%d)", WIDTH, (int)bounds.width());
    REPORTER_ASSERT(reporter, bounds.height() == HEIGHT,
        "Page height: expected (%d) got (%d)", HEIGHT, (int)bounds.height());

    auto surf2 = SkSurfaces::Raster(info);
    surf2->getCanvas()->drawPicture(frames[0].fPicture);
    auto img = surf2->makeImageSnapshot();
    REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(img.get(), expectedImages[0].get()));
}

#endif // android compilation
