/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDocument.h"
#include "SkCanvas.h"
#include "SkImageGenerator.h"
#include "SkData.h"
#include "SkStream.h"

#include "Resources.h"
#include "Test.h"

// Returned bitmap is lazy.  Only lazy bitmaps hold onto the original data.
static SkBitmap bitmap_from_data(SkData* data) {
    SkASSERT(data);
    SkBitmap bm;
    SkDEPRECATED_InstallDiscardablePixelRef(data, &bm);
    return bm;
}

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


static SkData* load_resource(
        skiatest::Reporter* r, const char* test, const char* filename) {
    SkString path(GetResourcePath(filename));
    SkData* data = SkData::NewFromFileName(path.c_str());
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
DEF_TEST(PDFJpegEmbedTest, r) {
    const char test[] = "PDFJpegEmbedTest";
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

    SkBitmap bm1(bitmap_from_data(mandrillData.get()));
    canvas->drawBitmap(bm1, 65.0, 0.0, nullptr);
    SkBitmap bm2(bitmap_from_data(cmykData.get()));
    canvas->drawBitmap(bm2, 0.0, 512.0, nullptr);

    canvas->flush();
    document->endPage();
    document->close();
    SkAutoTUnref<SkData> pdfData(pdf.copyToData());
    SkASSERT(pdfData);
    pdf.reset();

    REPORTER_ASSERT(r, is_subset_of(mandrillData.get(), pdfData.get()));

    // This JPEG uses a nonstandard colorspace - it can not be
    // embedded into the PDF directly.
    REPORTER_ASSERT(r, !is_subset_of(cmykData.get(), pdfData.get()));
    ////////////////////////////////////////////////////////////////////////////
    pdf.reset();
    document = SkDocument::MakePDF(&pdf);
    canvas = document->beginPage(642, 1028);

    canvas->clear(SK_ColorLTGRAY);

    sk_sp<SkImage> im1(SkImage::MakeFromEncoded(mandrillData));
    canvas->drawImage(im1.get(), 65.0, 0.0, nullptr);
    sk_sp<SkImage> im2(SkImage::MakeFromEncoded(cmykData));
    canvas->drawImage(im2.get(), 0.0, 512.0, nullptr);

    canvas->flush();
    document->endPage();
    document->close();
    pdfData.reset(pdf.copyToData());
    SkASSERT(pdfData);
    pdf.reset();

    REPORTER_ASSERT(r, is_subset_of(mandrillData.get(), pdfData.get()));

    // This JPEG uses a nonstandard colorspace - it can not be
    // embedded into the PDF directly.
    REPORTER_ASSERT(r, !is_subset_of(cmykData.get(), pdfData.get()));
}

#include "SkJpegInfo.h"

DEF_TEST(JpegIdentification, r) {
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
        SkAutoTUnref<SkData> data(
                load_resource(r, "JpegIdentification", kTests[i].path));
        if (!data) {
            continue;
        }
        SkJFIFInfo info;
        bool isJfif = SkIsJFIF(data, &info);
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
}
