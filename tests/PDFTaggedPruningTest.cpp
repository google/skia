/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"

#ifdef SK_SUPPORT_PDF
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkDocument.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/docs/SkPDFDocument.h"
#include "src/pdf/SkPDFUtils.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include <memory>
#include <utility>
#include <vector>

using PDFTag = SkPDF::StructureElementNode;

// Test building a tagged PDF where nodes are pruned.
// Add this to args.gn to output the PDF to a file:
//   extra_cflags = [ "-DSK_PDF_TEST_TAGS_OUTPUT_PATH=\"/tmp/pruning.pdf\"" ]
DEF_TEST(SkPDF_tagged_pruning, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_tagged, r);
#ifdef SK_PDF_TEST_TAGS_OUTPUT_PATH
    SkFILEWStream outputStream(SK_PDF_TEST_TAGS_OUTPUT_PATH);
#else
    SkDynamicMemoryWStream outputStream;
#endif

    SkSize pageSize = SkSize::Make(612, 792);  // U.S. Letter

    SkPDF::Metadata metadata;
    metadata.fTitle = "Example Tagged PDF";
    metadata.fCreator = "Skia";
    SkPDF::DateTime now;
    SkPDFUtils::GetDateTime(&now);
    metadata.fCreation = now;
    metadata.fModified = now;

    // The document tag.
    auto root = std::make_unique<PDFTag>();
    root->fNodeId = 1;
    root->fTypeString = "Document";
    root->fLang = "en-US";

    // First paragraph.
    auto p1 = std::make_unique<PDFTag>();
    p1->fNodeId = 2;
    p1->fAdditionalNodeIds = {3, 4};
    p1->fTypeString = "P";
    root->fChildVector.push_back(std::move(p1));

    // Second paragraph.
    auto p2 = std::make_unique<PDFTag>();
    p2->fNodeId = 5;
    p2->fAdditionalNodeIds = {6, 7};
    p2->fTypeString = "P";
    root->fChildVector.push_back(std::move(p2));

    metadata.fStructureElementTreeRoot = root.get();
    sk_sp<SkDocument> document = SkPDF::MakeDocument(
        &outputStream, metadata);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    SkCanvas* canvas =
            document->beginPage(pageSize.width(),
                                pageSize.height());
    SkFont font(ToolUtils::DefaultTypeface(), 20);
    SkPDF::SetNodeId(canvas, 3);
    canvas->drawString("First paragraph line 1", 72, 72, font, paint);
    SkPDF::SetNodeId(canvas, 4);
    canvas->drawString("First paragraph line 2", 72, 108, font, paint);
    SkPDF::SetNodeId(canvas, 6);
    canvas->drawString("Second paragraph line 1", 72, 180, font, paint);
    SkPDF::SetNodeId(canvas, 7);
    canvas->drawString("Second paragraph line 2", 72, 216, font, paint);

    document->endPage();
    document->close();
    outputStream.flush();
}

// Similar to SkPDF_tagged_pruning but never actually writes out anything annotated.
// Ensures that nothing goes wring when there are no annotations on an annotated PDF.
DEF_TEST(SkPDF_tagged_pruning_empty, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_tagged, r);
#ifdef SK_PDF_TEST_TAGS_OUTPUT_PATH
    SkFILEWStream outputStream(SK_PDF_TEST_TAGS_OUTPUT_PATH);
#else
    SkDynamicMemoryWStream outputStream;
#endif

    SkSize pageSize = SkSize::Make(612, 792);  // U.S. Letter

    SkPDF::Metadata metadata;
    metadata.fTitle = "Example Tagged PDF";
    metadata.fCreator = "Skia";
    SkPDF::DateTime now;
    SkPDFUtils::GetDateTime(&now);
    metadata.fCreation = now;
    metadata.fModified = now;

    // The document tag.
    auto root = std::make_unique<PDFTag>();
    root->fNodeId = 1;
    root->fTypeString = "Document";
    root->fLang = "en-US";

    // First paragraph.
    auto p1 = std::make_unique<PDFTag>();
    p1->fNodeId = 2;
    p1->fAdditionalNodeIds = {3, 4};
    p1->fTypeString = "P";
    root->fChildVector.push_back(std::move(p1));

    // Second paragraph.
    auto p2 = std::make_unique<PDFTag>();
    p2->fNodeId = 5;
    p2->fAdditionalNodeIds = {6, 7};
    p2->fTypeString = "P";
    root->fChildVector.push_back(std::move(p2));

    metadata.fStructureElementTreeRoot = root.get();
    sk_sp<SkDocument> document = SkPDF::MakeDocument(
        &outputStream, metadata);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    SkCanvas* canvas =
            document->beginPage(pageSize.width(),
                                pageSize.height());

    canvas->drawRect(SkRect::MakeXYWH(10, 10, 100, 100), paint);

    document->endPage();
    document->close();
    outputStream.flush();
}

#endif
