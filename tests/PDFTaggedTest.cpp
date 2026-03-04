/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"

#ifdef SK_SUPPORT_PDF
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkDocument.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h" // IWYU pragma: keep
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/docs/SkPDFDocument.h"
#include "include/docs/SkPDFJpegHelpers.h"
#include "src/pdf/SkPDFUtils.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include <memory>
#include <utility>
#include <vector>

namespace {
using PDFTag = SkPDF::StructureElementNode;

enum class EmitHeader : bool { No = false, Yes = true };

void write_structured_document(SkWStream& outputStream, SkPDF::Metadata::Outline outline,
                               EmitHeader emitHeader) {
    SkSize pageSize = SkSize::Make(612, 792);  // U.S. Letter

    SkPDF::Metadata metadata;
    metadata.fTitle = "Example Tagged PDF";
    metadata.fCreator = "Skia";
    metadata.fOutline = outline;
    SkPDF::DateTime now;
    SkPDFUtils::GetDateTime(&now);
    metadata.fCreation = now;
    metadata.fModified = now;
    metadata.jpegDecoder = SkPDF::JPEG::Decode;
    metadata.jpegEncoder = SkPDF::JPEG::Encode;

    // The document tag.
    auto root = std::make_unique<PDFTag>();
    root->fNodeId = 1;
    root->fTypeString = "Document";

    // Heading.
    if (emitHeader == EmitHeader::Yes) {
        auto h1 = std::make_unique<PDFTag>();
        h1->fNodeId = 2;
        h1->fTypeString = "H1";
        h1->fAlt = "A Header";
        root->fChildVector.push_back(std::move(h1));
    }

    // Initial paragraph.
    auto p = std::make_unique<PDFTag>();
    p->fNodeId = 3;
    p->fTypeString = "P";
    root->fChildVector.push_back(std::move(p));

    // Hidden div. This is never referenced by marked content
    // so it should not appear in the resulting PDF.
    auto div = std::make_unique<PDFTag>();
    div->fNodeId = 4;
    div->fTypeString = "Div";
    root->fChildVector.push_back(std::move(div));

    // A bulleted list of two items.
    auto l = std::make_unique<PDFTag>();
    l->fNodeId = 5;
    l->fTypeString = "L";

    auto lm1 = std::make_unique<PDFTag>();
    lm1->fNodeId = 6;
    lm1->fTypeString = "Lbl";
    l->fChildVector.push_back(std::move(lm1));

    auto li1 = std::make_unique<PDFTag>();
    li1->fNodeId = 7;
    li1->fTypeString = "LI";
    l->fChildVector.push_back(std::move(li1));

    auto lm2 = std::make_unique<PDFTag>();
    lm2->fNodeId = 8;
    lm2->fTypeString = "Lbl";
    l->fChildVector.push_back(std::move(lm2));
    auto li2 = std::make_unique<PDFTag>();
    li2->fNodeId = 9;
    li2->fTypeString = "LI";
    l->fChildVector.push_back(std::move(li2));

    root->fChildVector.push_back(std::move(l));

    // Paragraph spanning two pages.
    auto p2 = std::make_unique<PDFTag>();
    p2->fNodeId = 10;
    p2->fTypeString = "P";
    root->fChildVector.push_back(std::move(p2));

    // Image with alt text.
    auto img = std::make_unique<PDFTag>();
    img->fNodeId = 11;
    img->fTypeString = "Figure";
    img->fAlt = "Red box";
    root->fChildVector.push_back(std::move(img));

    metadata.fStructureElementTreeRoot = root.get();
    sk_sp<SkDocument> document = SkPDF::MakeDocument(&outputStream, metadata);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    // First page.
    SkCanvas* canvas = document->beginPage(pageSize.width(), pageSize.height());
    SkPDF::SetNodeId(canvas, 2);
    SkFont font(ToolUtils::DefaultTypeface(), 36);
    const char* message = "This is the title";
    canvas->translate(72, 72);
    canvas->drawString(message, 0, 0, font, paint);

    SkPDF::SetNodeId(canvas, 3);
    font.setSize(14);
    message = "This is a simple paragraph.";
    canvas->translate(0, 72);
    canvas->drawString(message, 0, 0, font, paint);

    SkPDF::SetNodeId(canvas, 6);
    font.setSize(14);
    message = "*";
    canvas->translate(0, 72);
    canvas->drawString(message, 0, 0, font, paint);

    SkPDF::SetNodeId(canvas, 7);
    message = "List item 1";
    canvas->translate(36, 0);
    canvas->drawString(message, 0, 0, font, paint);

    SkPDF::SetNodeId(canvas, 8);
    message = "*";
    canvas->translate(-36, 36);
    canvas->drawString(message, 0, 0, font, paint);

    SkPDF::SetNodeId(canvas, 9);
    message = "List item 2";
    canvas->translate(36, 0);
    canvas->drawString(message, 0, 0, font, paint);

    SkPDF::SetNodeId(canvas, 10);
    message = "This is a paragraph that starts on one page";
    canvas->translate(-36, 6 * 72);
    canvas->drawString(message, 0, 0, font, paint);

    document->endPage();

    // Second page.
    canvas = document->beginPage(pageSize.width(), pageSize.height());
    SkPaint bgPaint;
    bgPaint.setColor(SK_ColorLTGRAY);
    SkPDF::SetNodeId(canvas, SkPDF::NodeID::BackgroundArtifact);
    canvas->drawPaint(bgPaint);

    SkPDF::SetNodeId(canvas, 10);
    message = "and finishes on the second page.";
    canvas->drawString(message, 72, 72, font, paint);

    // Test a tagged image with alt text.
    SkPDF::SetNodeId(canvas, 11);
    SkBitmap testBitmap;
    testBitmap.allocN32Pixels(72, 72);
    testBitmap.eraseColor(SK_ColorRED);
    canvas->drawImage(testBitmap.asImage(), 72, 144);

    // This has a node ID but never shows up in the tag tree so it
    // won't be tagged.
    SkPDF::SetNodeId(canvas, 999);
    canvas->drawString("Page", pageSize.width() - 100, pageSize.height() - 30, font, paint);

    SkPDF::SetNodeId(canvas, SkPDF::NodeID::PaginationFooterArtifact);
    canvas->drawString("2", pageSize.width() - 30, pageSize.height() - 30, font, paint);

    document->endPage();

    document->close();

    outputStream.flush();
}
} // namespace

// To log the output, replace the SkDynamicMemoryWStream with something like
// SkFILEWStream outputStream("test.pdf");

// Test building a tagged PDF with headers and a header outline.
DEF_TEST(SkPDF_structelem_header_outline_doc, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_structelem_header_outline_doc, r);
    SkDynamicMemoryWStream outputStream;
    write_structured_document(outputStream, SkPDF::Metadata::Outline::StructureElementHeaders,
                              EmitHeader::Yes);
}

// Test building a tagged PDF with a structure element outline.
DEF_TEST(SkPDF_structelem_outline_doc, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_structelem_outline_doc, r);
    SkDynamicMemoryWStream outputStream;
    write_structured_document(outputStream, SkPDF::Metadata::Outline::StructureElements,
                              EmitHeader::Yes);
}

// Test building a tagged PDF with no headers with a header outline.
DEF_TEST(SkPDF_structelem_header_outline_doc_noheader, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_structelem_header_outline_doc, r);
    SkDynamicMemoryWStream outputStream;
    write_structured_document(outputStream, SkPDF::Metadata::Outline::StructureElementHeaders,
                              EmitHeader::No);
}


// Test that structure tagging is preserved inside saveLayer (FormXObject).
DEF_TEST(SkPDF_tagged_saveLayer, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_tagged_saveLayer, r);

    SkPDF::Metadata metadata;
    metadata.fTitle = "Tagged SaveLayer Test";
    metadata.fCreator = "Skia";
    metadata.allowNoJpegs = true;
    metadata.fCompressionLevel = SkPDF::Metadata::CompressionLevel::None;

    auto makeElement = [](int elemId, SkString type) -> std::unique_ptr<PDFTag> {
        auto element = std::make_unique<PDFTag>();
        element->fNodeId = elemId;
        element->fTypeString = type;
        return element;
    };

    int elemId = 1;
    auto root = makeElement(elemId++, SkString("Document"));
    root->fChildVector.push_back(makeElement(elemId++, SkString("P")));
    root->fChildVector.push_back(makeElement(elemId++, SkString("P")));

    metadata.fStructureElementTreeRoot = root.get();

    SkDynamicMemoryWStream outputStream;
    sk_sp<SkDocument> document = SkPDF::MakeDocument(&outputStream, metadata);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    SkFont font(ToolUtils::DefaultTypeface(), 14);
    SkCanvas* canvas =
            document->beginPage(SkSize::Make(612, 792).width(), SkSize::Make(612, 792).height());

    SkPDF::SetNodeId(canvas, root->fChildVector[0]->fNodeId);
    canvas->drawString("Before saveLayer", 72, 72, font, paint);

    SkPaint layerPaint;
    layerPaint.setAlphaf(0.8f);
    canvas->saveLayer(nullptr, &layerPaint);
    SkPDF::SetNodeId(canvas, root->fChildVector[1]->fNodeId);
    canvas->drawString("Inside saveLayer", 72, 144, font, paint);
    canvas->restore();

    document->endPage();
    document->close();

    sk_sp<SkData> pdfData = outputStream.detachAsData();
    SkSpan data(static_cast<const char*>(pdfData->data()), pdfData->size());

    auto contains = [](SkSpan<const char> haystack, SkSpan<const char> needle) -> bool {
        return std::search(haystack.begin(), haystack.end(),
                           needle.begin(), needle.end() - 1)
               != haystack.end();
    };
    auto count = [](SkSpan<const char> haystack, SkSpan<const char> needle) -> int {
        int found = 0;
        auto haystackBegin = haystack.begin();
        while ((haystackBegin = std::search(haystackBegin, haystack.end(),
                                            needle.begin(), needle.end() - 1))
               != haystack.end())
        {
            ++haystackBegin;
            ++found;
        }
        return found;
    };

    // Expect both a page stream and a FormXObject stream carrying marked content.
    REPORTER_ASSERT(r, count(data, SkSpan("/StructParents ")) >= 2);

    // Marks in the FormXObject stream should be referenced from the structure tree with /Stm.
    REPORTER_ASSERT(r, contains(data, SkSpan("/Type /MCR")));
    REPORTER_ASSERT(r, contains(data, SkSpan("/Stm ")));
}

#endif
