/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This test confirms that a multi skp can be serialize and deserailzied without error.
 */

#include "include/core/SkDocument.h"
#include "include/core/SkFont.h"
#include "include/core/SkPicture.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "src/core/SkRecord.h"
#include "src/core/SkRecorder.h"
#include "src/utils/SkMultiPictureDocument.h"
#include "tests/Test.h"
#include "tools/SkSharingProc.h"

namespace {

class RecordVisitor {
// An SkRecord visitor that remembers the name of the last visited command.
public:
    SkString name;

    explicit RecordVisitor() {}

    template <typename T>
    void operator()(const T& command) {
        name = SkString(NameOf(command));
    }

    SkString lastCommandName() {
        return name;
    }
private:
    template <typename T>
    static const char* NameOf(const T&) {
        #define CASE(U) case SkRecords::U##_Type: return #U;
        switch (T::kType) { SK_RECORD_TYPES(CASE) }
        #undef CASE
        return "Unknown T";
    }
};
} // namespace

// Compare record tested with record expected. Assert op sequence is the same (comparing types)
// frame_num is only used for error message.
static void compareRecords(const SkRecord& tested, const SkRecord& expected,
    int frame_num, skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, tested.count() == expected.count(),
        "Found %d commands in frame %d, expected %d", tested.count(), frame_num, expected.count());

    RecordVisitor rv;
    for (int i = 0; i < tested.count(); i++) {
        tested.visit(i, rv);
        const SkString testCommandName = rv.lastCommandName();
        expected.visit(i, rv);
        const SkString expectedCommandName = rv.lastCommandName();
        REPORTER_ASSERT(reporter, testCommandName == expectedCommandName,
            "Unexpected command type '%s' in frame %d, op %d. Expected '%s'",
            testCommandName.c_str(), frame_num, i, expectedCommandName.c_str());
    }
}

static void draw_something(SkCanvas* canvas, int seed, sk_sp<SkImage> image) {
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

// Test serialization and deserialization of multi skp.
DEF_TEST(Serialize_and_deserialize_multi_skp, reporter) {
    // Create the stream we will serialize into.
    SkDynamicMemoryWStream stream;

    // Create the image sharing proc.
    SkSharingSerialContext ctx;
    SkSerialProcs procs;
    procs.fImageProc = SkSharingSerialContext::serializeImage;
    procs.fImageCtx = &ctx;

    // Create the mulit picture document used for recording frames.
    sk_sp<SkDocument> multipic = SkMakeMultiPictureDocument(&stream, &procs);

    static const int NUM_FRAMES = 12;
    static const int WIDTH = 256;
    static const int HEIGHT = 256;

    // Make an image to be used in a later step.
    auto surface(SkSurface::MakeRasterN32Premul(100, 100));
    surface->getCanvas()->clear(SK_ColorGREEN);
    sk_sp<SkImage> image(surface->makeImageSnapshot());
    REPORTER_ASSERT(reporter, image);

    // Create frames, recording them to multipic.
    SkRecord expectedRecords[NUM_FRAMES];
    for (int i=0; i<NUM_FRAMES; i++) {
        SkCanvas* pictureCanvas = multipic->beginPage(WIDTH, HEIGHT);
        draw_something(pictureCanvas, i, image);
        multipic->endPage();
        // Also record the same commands to separate SkRecords for later comparison
        SkRecorder canvas(&expectedRecords[i], WIDTH, HEIGHT);
        draw_something(&canvas, i, image);
    }
    // Finalize
    multipic->close();

    // Confirm written data is at least as large as the magic word
    std::unique_ptr<SkStreamAsset> writtenStream = stream.detachAsStream();
    REPORTER_ASSERT(reporter, writtenStream->getLength() > 24,
        "Written data length too short (%d)", writtenStream->getLength());
    SkDebugf("Multi Frame file size = %d\n", writtenStream->getLength());

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
    SkRecorder resultRecorder(nullptr, 1, 1);
    int i=0;
    for (const auto& frame : frames) {
        SkRect bounds = frame.fPicture->cullRect();
        REPORTER_ASSERT(reporter, bounds.width() == WIDTH,
            "Page width: expected (%d) got (%d)", WIDTH, bounds.width());
        REPORTER_ASSERT(reporter, bounds.height() == HEIGHT,
            "Page height: expected (%d) got (%d)", HEIGHT, bounds.height());
        // confirm contents of picture match what we drew.
        // There are several ways of doing this, an ideal comparison would not break in the same
        // way at the same time as the code under test (no serialization), and would involve only
        // minimal transformation of frame.fPicture, minimizing the chance that a detected fault lies
        // in the test itself. The comparions also would not be an overly sensitive change detector,
        // so that it doesn't break every time someone submits code (no golden file)

        // Extract the SkRecord from the deserialized picture using playback (instead of a mess of
        // friend classes to grab the private record inside frame.fPicture
        SkRecord record;
        // This picture mode is necessary so that we record the command contents of frame.fPicture
        // not just a 'DrawPicture' command.
        resultRecorder.reset(&record, bounds, SkRecorder::Playback_DrawPictureMode, nullptr);
        frame.fPicture->playback(&resultRecorder);
        // Compare the record to the expected one
        compareRecords(record, expectedRecords[i], i, reporter);
        i++;
    }
}
