/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This test confirms that a MultiPictureDocument can be serialized and deserailzied without error.
 * And that the pictures within it are re-created accurately
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkDocument.h"
#include "include/core/SkFont.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "src/utils/SkMultiPictureDocument.h"
#include "tests/Test.h"
#include "tools/SkSharingProc.h"
#include "tools/ToolUtils.h"

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

    canvas->drawImage(image, 128-seed, 128, &paint);

    if (seed % 2 == 0) {
        SkRect rect2 = SkRect::MakeXYWH(0, 0, 40, 60);
        canvas->drawImageRect(image, rect2, &paint);
    }

    SkPaint paint2;
    auto text = SkTextBlob::MakeFromString(
        SkStringPrintf("Frame %d", seed).c_str(), SkFont(nullptr, 2+seed));
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
    sk_sp<SkDocument> multipic = SkMakeMultiPictureDocument(&stream, &procs);

    static const int NUM_FRAMES = 12;
    static const int WIDTH = 256;
    static const int HEIGHT = 256;

    // Make an image to be used in a later step.
    auto surface(SkSurface::MakeRasterN32Premul(100, 100));
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
        auto surf = SkSurface::MakeRaster(info);
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
    int frame_count = SkMultiPictureDocumentReadPageCount(writtenStream.get());
    REPORTER_ASSERT(reporter, frame_count == NUM_FRAMES,
        "Expected %d frames, got %d. \n 0 frames may indicate the written file was not a "
        "MultiPictureDocument.", NUM_FRAMES, frame_count);

    // Deserailize
    std::vector<SkDocumentPage> frames(frame_count);
    REPORTER_ASSERT(reporter,
        SkMultiPictureDocumentRead(writtenStream.get(), frames.data(), frame_count, &dprocs),
        "Failed while reading MultiPictureDocument");

    // Examine each frame.
    int i=0;
    for (const auto& frame : frames) {
        SkRect bounds = frame.fPicture->cullRect();
        REPORTER_ASSERT(reporter, bounds.width() == WIDTH,
            "Page width: expected (%d) got (%d)", WIDTH, (int)bounds.width());
        REPORTER_ASSERT(reporter, bounds.height() == HEIGHT,
            "Page height: expected (%d) got (%d)", HEIGHT, (int)bounds.height());

        auto surf = SkSurface::MakeRaster(info);
        surf->getCanvas()->drawPicture(frame.fPicture);
        auto img = surf->makeImageSnapshot();
        REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(img.get(), expectedImages[i].get()));

        i++;
    }
}


#if SK_SUPPORT_GPU && defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include <android/hardware_buffer.h>

static sk_sp<SkImage> makeAHardwareBufferTestImage() {
    const SkBitmap srcBitmap = make_src_bitmap();
    AHardwareBuffer* buffer = nullptr;

    AHardwareBuffer_Desc hwbDesc;
    hwbDesc.width = DEV_W;
    hwbDesc.height = DEV_H;
    hwbDesc.layers = 1;
    hwbDesc.usage = AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
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
        return;
    }

    // Get actual desc for allocated buffer so we know the stride for uploading cpu data.
    AHardwareBuffer_describe(buffer, &hwbDesc);

    uint32_t* bufferAddr;
    if (AHardwareBuffer_lock(buffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN, -1, nullptr,
                             reinterpret_cast<void**>(&bufferAddr))) {
        ERRORF(reporter, "Failed to lock hardware buffer");
        cleanup_resources(buffer);
        return;
    }

    int bbp = srcBitmap.bytesPerPixel();
    uint32_t* src = (uint32_t*)srcBitmap.getPixels();
    int nextLineStep = DEV_W;
    if (surfaceOrigin == kBottomLeft_GrSurfaceOrigin) {
        nextLineStep = -nextLineStep;
        src += (DEV_H-1)*DEV_W;
    }
    uint32_t* dst = bufferAddr;
    for (int y = 0; y < DEV_H; ++y) {
        memcpy(dst, src, DEV_W * bbp);
        src += nextLineStep;
        dst += hwbDesc.stride;
    }
    AHardwareBuffer_unlock(buffer, nullptr);

    sk_sp<SkImage> image = SkImage::MakeFromAHardwareBuffer(buffer, kPremul_SkAlphaType,
                                                            nullptr, surfaceOrigin);
    REPORTER_ASSERT(reporter, image);
    return image;
}

#endif // android compilation

// Test the onEndPage callback's intended use by processing an mskp containing AHardwareBuffer-backed SkImages
// Expected behavior is that the callback is called while the AHardwareBuffer is still valid and the
// images are copied so .close() can still access them.
// Confirm deserialized file containes images with correct data.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkMultiPictureDocument_AHardwarebuffer,
                                   reporter, info) {
    auto context = info.directContext();
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
    sk_sp<SkDocument> multipic = SkMakeMultiPictureDocument(&stream, &procs);

    static const int NUM_FRAMES = 1;
    static const int WIDTH = 256;
    static const int HEIGHT = 256;

    // Make an image to be used in a later step.
    auto surface(SkSurface::MakeRasterN32Premul(100, 100));
    surface->getCanvas()->clear(SK_ColorGREEN);
    sk_sp<SkImage> image = makeAHardwareBufferTestImage();

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
        auto surf = SkSurface::MakeRaster(info);
        draw_advanced(surf->getCanvas(), i, image, sub);
        expectedImages.push_back(surf->makeImageSnapshot());

        // release buffer to confirm code does not access it during close()
        AHardwareBuffer_release(buffer);
    }
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
    int frame_count = SkMultiPictureDocumentReadPageCount(writtenStream.get());
    REPORTER_ASSERT(reporter, frame_count == NUM_FRAMES,
        "Expected %d frames, got %d. \n 0 frames may indicate the written file was not a "
        "MultiPictureDocument.", NUM_FRAMES, frame_count);

    // Deserailize
    std::vector<SkDocumentPage> frames(frame_count);
    REPORTER_ASSERT(reporter,
        SkMultiPictureDocumentRead(writtenStream.get(), frames.data(), frame_count, &dprocs),
        "Failed while reading MultiPictureDocument");

    // Examine each frame.
    int i=0;
    for (const auto& frame : frames) {
        SkRect bounds = frame.fPicture->cullRect();
        REPORTER_ASSERT(reporter, bounds.width() == WIDTH,
            "Page width: expected (%d) got (%d)", WIDTH, (int)bounds.width());
        REPORTER_ASSERT(reporter, bounds.height() == HEIGHT,
            "Page height: expected (%d) got (%d)", HEIGHT, (int)bounds.height());

        auto surf = SkSurface::MakeRaster(info);
        surf->getCanvas()->drawPicture(frame.fPicture);
        auto img = surf->makeImageSnapshot();
        REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(img.get(), expectedImages[i].get()));

        i++;
    }
}