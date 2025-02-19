/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tests/Test.h"

#ifdef SK_SUPPORT_PDF

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkDocument.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/docs/SkPDFDocument.h"
#include "src/pdf/SkPDFUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <memory>
#include <utility>
#include <vector>

using PDFTag = SkPDF::StructureElementNode;

// Test building a tagged PDF containing a table.
// Add this to args.gn to output the PDF to a file:
//   extra_cflags = [ "-DSK_PDF_TEST_TAGS_OUTPUT_PATH=\"/tmp/table.pdf\"" ]
DEF_TEST(SkPDF_tagged_table, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_tagged, r);
#ifdef SK_PDF_TEST_TAGS_OUTPUT_PATH
    SkFILEWStream outputStream(SK_PDF_TEST_TAGS_OUTPUT_PATH);
#else
    SkDynamicMemoryWStream outputStream;
#endif

    SkSize pageSize = SkSize::Make(612, 792);  // U.S. Letter

    SkPDF::Metadata metadata;
    metadata.fTitle = "Example Tagged Table PDF";
    metadata.fCreator = "Skia";
    SkPDF::DateTime now;
    SkPDFUtils::GetDateTime(&now);
    metadata.fCreation = now;
    metadata.fModified = now;

    constexpr int kRowCount = 5;
    constexpr int kColCount = 4;
    const char* cellData[kRowCount * kColCount] = {
        "Car",                  "Engine",   "City MPG", "Highway MPG",
        "Mitsubishi Mirage ES", "Gas",      "28",       "47",
        "Toyota Prius Three",   "Hybrid",   "43",       "59",
        "Nissan Leaf SL",       "Electric", "N/A",      nullptr,
        "Tesla Model 3",        nullptr,    "N/A",      nullptr
    };

    // The document tag.
    auto root = std::make_unique<PDFTag>();
    root->fNodeId = 1;
    root->fTypeString = "Document";
    root->fLang = "en-US";

    // Heading.
    auto h1 = std::make_unique<PDFTag>();
    h1->fNodeId = 2;
    h1->fTypeString = "H1";
    h1->fAlt = "Tagged PDF Table Alt Text";
    root->fChildVector.push_back(std::move(h1));

    // Table.
    auto table = std::make_unique<PDFTag>();
    table->fNodeId = 3;
    table->fTypeString = "Table";
    auto& rows = table->fChildVector;
    table->fAttributes.appendFloatArray("Layout", "BBox", {72, 72, 360, 360});

    for (int rowIndex = 0; rowIndex < kRowCount; rowIndex++) {
        auto row = std::make_unique<PDFTag>();
        row->fNodeId = 4 + rowIndex;
        row->fTypeString = "TR";
        auto& cells = row->fChildVector;
        for (int colIndex = 0; colIndex < kColCount; colIndex++) {
            auto cell = std::make_unique<PDFTag>();
            int cellIndex = rowIndex * kColCount + colIndex;
            cell->fNodeId = 10 + cellIndex;
            if (!cellData[cellIndex]) {
                cell->fTypeString = "NonStruct";
            } else if (rowIndex == 0 || colIndex == 0) {
                cell->fTypeString = "TH";
            } else {
                cell->fTypeString = "TD";
                std::vector<int> headerIds;
                headerIds.push_back(10 + rowIndex * kColCount);  // Row header
                headerIds.push_back(10 + colIndex);  // Col header.
                cell->fAttributes.appendNodeIdArray(
                    "Table", "Headers", headerIds);
            }

            if (cellIndex == 13) {
                cell->fAttributes.appendInt("Table", "RowSpan", 2);
            } else if (cellIndex == 14 || cellIndex == 18) {
                cell->fAttributes.appendInt("Table", "ColSpan", 2);
            } else if (rowIndex == 0 || colIndex == 0) {
                cell->fAttributes.appendName(
                    "Table", "Scope", rowIndex == 0 ? "Column" : "Row");
            }
            cells.push_back(std::move(cell));
        }
        rows.push_back(std::move(row));
    }
    root->fChildVector.push_back(std::move(table));

    metadata.fStructureElementTreeRoot = root.get();
    sk_sp<SkDocument> document = SkPDF::MakeDocument(
        &outputStream, metadata);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    SkCanvas* canvas =
            document->beginPage(pageSize.width(),
                                pageSize.height());
    SkPDF::SetNodeId(canvas, 2);
    SkFont font(ToolUtils::DefaultTypeface(), 36);
    canvas->drawString("Tagged PDF Table", 72, 72, font, paint);

    font.setSize(14);
    for (int rowIndex = 0; rowIndex < kRowCount; rowIndex++) {
        for (int colIndex = 0; colIndex < kColCount; colIndex++) {
            int cellIndex = rowIndex * kColCount + colIndex;
            const char* str = cellData[cellIndex];
            if (!str)
                continue;

            int x = 72 + colIndex * 108 + (colIndex > 0 ? 72 : 0);
            int y = 144 + rowIndex * 48;

            SkPDF::SetNodeId(canvas, 10 + cellIndex);
            canvas->drawString(str, x, y, font, paint);
        }
    }

    document->endPage();
    document->close();
    outputStream.flush();
}

#endif
