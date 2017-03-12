/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkData.h"
#include "SkDocument.h"
#include "SkImageGenerator.h"
#include "SkStream.h"

#include "Resources.h"
#include "Test.h"

static bool is_subset_of(SkData* smaller, SkData* larger) {
    SkASSERT(smaller && larger);
    if (smaller->size() > larger->size()) {
        return false;
    }
    size_t size = smaller->size();
    size_t size_diff = larger->size() - size;
    for (size_t i = 0; i <= size_diff; ++i) {
        if (0 == memcmp(larger->bytes() + i, smaller->bytes(), size)) {
            return true;
        }
    }
    return false;
}


static sk_sp<SkData> load_resource(
        skiatest::Reporter* r, const char* test, const char* filename) {
    SkString path(GetResourcePath(filename));
    sk_sp<SkData> data(SkData::MakeFromFileName(path.c_str()));
    if (!data) {
        INFOF(r, "\n%s: Resource '%s' can not be found.\n",
              test, filename);
    }
    return data;  // May return nullptr.
}

/**
 *  Test that for Jpeg files that use the JFIF colorspace, they are
 *  directly embedded into the PDF (without re-encoding) when that
 *  makes sense.
 */
DEF_TEST(SkPDF_JpegEmbedTest, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_JpegEmbedTest, r);
    const char test[] = "SkPDF_JpegEmbedTest";
    sk_sp<SkData> mandrillData(load_resource(r, test, "mandrill_512_q075.jpg"));
    sk_sp<SkData> cmykData(load_resource(r, test, "CMYK.jpg"));
    if (!mandrillData || !cmykData) {
        return;
    }
    ////////////////////////////////////////////////////////////////////////////
    SkDynamicMemoryWStream pdf;
    sk_sp<SkDocument> document(SkDocument::MakePDF(&pdf));
    SkCanvas* canvas = document->beginPage(642, 1028);

    canvas->clear(SK_ColorLTGRAY);

    sk_sp<SkImage> im1(SkImage::MakeFromEncoded(mandrillData));
    canvas->drawImage(im1.get(), 65.0, 0.0, nullptr);
    sk_sp<SkImage> im2(SkImage::MakeFromEncoded(cmykData));
    canvas->drawImage(im2.get(), 0.0, 512.0, nullptr);

    canvas->flush();
    document->endPage();
    document->close();
    sk_sp<SkData> pdfData = pdf.detachAsData();
    SkASSERT(pdfData);

    REPORTER_ASSERT(r, is_subset_of(mandrillData.get(), pdfData.get()));

    // This JPEG uses a nonstandard colorspace - it can not be
    // embedded into the PDF directly.
    REPORTER_ASSERT(r, !is_subset_of(cmykData.get(), pdfData.get()));
}

#ifdef SK_SUPPORT_PDF

#include "SkJpegInfo.h"

DEF_TEST(SkPDF_JpegIdentification, r) {
    static struct {
        const char* path;
        bool isJfif;
        SkJFIFInfo::Type type;
    } kTests[] = {{"CMYK.jpg", false, SkJFIFInfo::kGrayscale},
                  {"color_wheel.jpg", true, SkJFIFInfo::kYCbCr},
                  {"grayscale.jpg", true, SkJFIFInfo::kGrayscale},
                  {"mandrill_512_q075.jpg", true, SkJFIFInfo::kYCbCr},
                  {"randPixels.jpg", true, SkJFIFInfo::kYCbCr}};
    for (size_t i = 0; i < SK_ARRAY_COUNT(kTests); ++i) {
        sk_sp<SkData> data(load_resource(r, "JpegIdentification", kTests[i].path));
        if (!data) {
            continue;
        }
        SkJFIFInfo info;
        bool isJfif = SkIsJFIF(data.get(), &info);
        if (isJfif != kTests[i].isJfif) {
            ERRORF(r, "%s failed isJfif test", kTests[i].path);
            continue;
        }
        if (!isJfif) {
            continue;  // not applicable
        }
        if (kTests[i].type != info.fType) {
            ERRORF(r, "%s failed jfif type test", kTests[i].path);
            continue;
        }
        INFOF(r, "\nJpegIdentification: %s [%d x %d]\n", kTests[i].path,
              info.fSize.width(), info.fSize.height());
    }

    // Test several malformed jpegs.
    SkJFIFInfo info;
    {
        static const char goodJpeg[] =
            "\377\330\377\340\0\20JFIF\0\1\1\0\0\1\0\1\0\0\377\333\0C\0\10\6\6\7"
            "\6\5\10\7\7\7\t\t\10\n\14\24\r\14\13\13\14\31\22\23\17\24\35\32\37"
            "\36\35\32\34\34 $.' \",#\34\34(7),01444\37'9=82<.342\377\333\0C\1\t"
            "\t\t\14\13\14\30\r\r\0302!\34!222222222222222222222222222222222222"
            "22222222222222\377\300\0\21\10\2\0\2\0\3\1\"\0\2\21\1\3\21\001";
        size_t goodJpegLength = 177;
        auto data = SkData::MakeWithoutCopy(goodJpeg, goodJpegLength);
        REPORTER_ASSERT(r, SkIsJFIF(data.get(), &info));
        REPORTER_ASSERT(r, info.fSize == SkISize::Make(512, 512));
        REPORTER_ASSERT(r, info.fType == SkJFIFInfo::kYCbCr);

        // Not long enough to read first (SOI) segment marker.
        data = SkData::MakeWithoutCopy(goodJpeg, 1);
        REPORTER_ASSERT(r, !SkIsJFIF(data.get(), &info));

        // Not long enough to read second segment (APP0) marker.
        data = SkData::MakeWithoutCopy(goodJpeg, 3);
        REPORTER_ASSERT(r, !SkIsJFIF(data.get(), &info));

        // Not long enough to read second segment's length.
        data = SkData::MakeWithoutCopy(goodJpeg, 5);
        REPORTER_ASSERT(r, !SkIsJFIF(data.get(), &info));

        // APP0 segment is truncated.
        data = SkData::MakeWithoutCopy(goodJpeg, 7);
        REPORTER_ASSERT(r, !SkIsJFIF(data.get(), &info));

        // Missing SOF segment.
        data = SkData::MakeWithoutCopy(goodJpeg, 89);
        REPORTER_ASSERT(r, !SkIsJFIF(data.get(), &info));
    }
    {
        // JFIF tag missing.
        static const char jpeg[] =
            "\377\330\377\340\0\20JFIX\0\1\1\0\0\1\0\1\0\0\377\333\0C\0\10\6\6\7"
            "\6\5\10\7\7\7\t\t\10\n\14\24\r\14\13\13\14\31\22\23\17\24\35\32\37"
            "\36\35\32\34\34 $.' \",#\34\34(7),01444\37'9=82<.342\377\333\0C\1\t"
            "\t\t\14\13\14\30\r\r\0302!\34!222222222222222222222222222222222222"
            "22222222222222\377\300\0\21\10\2\0\2\0\3\1\"\0\2\21\1\3\21\001";
        size_t jpegLength = 177;
        auto data = SkData::MakeWithoutCopy(jpeg, jpegLength);
        REPORTER_ASSERT(r, !SkIsJFIF(data.get(), &info));
    }
    {
        // APP0 segment short (byte 6 changed).
        static const char jpeg[] =
            "\377\330\377\340\0\5JFIF\0\1\1\0\0\1\0\1\0\0\377\333\0C\0\10\6\6\7"
            "\6\5\10\7\7\7\t\t\10\n\14\24\r\14\13\13\14\31\22\23\17\24\35\32\37"
            "\36\35\32\34\34 $.' \",#\34\34(7),01444\37'9=82<.342\377\333\0C\1\t"
            "\t\t\14\13\14\30\r\r\0302!\34!222222222222222222222222222222222222"
            "22222222222222\377\300\0\21\10\2\0\2\0\3\1\"\0\2\21\1\3\21\001";
        size_t jpegLength = 177;
        auto data = SkData::MakeWithoutCopy(jpeg, jpegLength);
        REPORTER_ASSERT(r, !SkIsJFIF(data.get(), &info));
    }
    {
        // SOF segment short. ('\21' replaced with '\5')
        static const char jpeg[] =
            "\377\330\377\340\0\20JFIF\0\1\1\0\0\1\0\1\0\0\377\333\0C\0\10\6\6\7"
            "\6\5\10\7\7\7\t\t\10\n\14\24\r\14\13\13\14\31\22\23\17\24\35\32\37"
            "\36\35\32\34\34 $.' \",#\34\34(7),01444\37'9=82<.342\377\333\0C\1\t"
            "\t\t\14\13\14\30\r\r\0302!\34!222222222222222222222222222222222222"
            "22222222222222\377\300\0\5\10\2\0\2\0\3\1\"\0\2\21\1\3\21\001";
        size_t jpegLength = 177;
        auto data = SkData::MakeWithoutCopy(jpeg, jpegLength);
        REPORTER_ASSERT(r, !SkIsJFIF(data.get(), &info));
    }
    {
        // Unsupported 12-bit components. ('\10' replaced with '\14')
        static const char jpeg[] =
            "\377\330\377\340\0\20JFIF\0\1\1\0\0\1\0\1\0\0\377\333\0C\0\10\6\6\7"
            "\6\5\10\7\7\7\t\t\10\n\14\24\r\14\13\13\14\31\22\23\17\24\35\32\37"
            "\36\35\32\34\34 $.' \",#\34\34(7),01444\37'9=82<.342\377\333\0C\1\t"
            "\t\t\14\13\14\30\r\r\0302!\34!222222222222222222222222222222222222"
            "22222222222222\377\300\0\21\14\2\0\2\0\3\1\"\0\2\21\1\3\21\001";
        size_t jpegLength = 177;
        auto data = SkData::MakeWithoutCopy(jpeg, jpegLength);
        REPORTER_ASSERT(r, !SkIsJFIF(data.get(), &info));
    }
    {
        // Two color channels.  ('\3' replaced with '\2')
        static const char jpeg[] =
            "\377\330\377\340\0\20JFIF\0\1\1\0\0\1\0\1\0\0\377\333\0C\0\10\6\6\7"
            "\6\5\10\7\7\7\t\t\10\n\14\24\r\14\13\13\14\31\22\23\17\24\35\32\37"
            "\36\35\32\34\34 $.' \",#\34\34(7),01444\37'9=82<.342\377\333\0C\1\t"
            "\t\t\14\13\14\30\r\r\0302!\34!222222222222222222222222222222222222"
            "22222222222222\377\300\0\21\10\2\0\2\0\2\1\"\0\2\21\1\3\21\001";
        size_t jpegLength = 177;
        auto data = SkData::MakeWithoutCopy(jpeg, jpegLength);
        REPORTER_ASSERT(r, !SkIsJFIF(data.get(), &info));
    }
}
#endif
