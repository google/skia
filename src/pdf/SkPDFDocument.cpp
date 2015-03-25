
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPDFCatalog.h"
#include "SkPDFDevice.h"
#include "SkPDFDocument.h"
#include "SkPDFFont.h"
#include "SkPDFPage.h"
#include "SkPDFTypes.h"
#include "SkStream.h"


static void perform_font_subsetting(SkPDFCatalog* catalog,
                                    const SkTDArray<SkPDFPage*>& pages) {
    SkASSERT(catalog);

    SkPDFGlyphSetMap usage;
    for (int i = 0; i < pages.count(); ++i) {
        usage.merge(pages[i]->getFontGlyphUsage());
    }
    SkPDFGlyphSetMap::F2BIter iterator(usage);
    const SkPDFGlyphSetMap::FontGlyphSetPair* entry = iterator.next();
    while (entry) {
        SkAutoTUnref<SkPDFFont> subsetFont(
                entry->fFont->getFontSubset(entry->fGlyphSet));
        if (subsetFont) {
            catalog->setSubstitute(entry->fFont, subsetFont.get());
        }
        entry = iterator.next();
    }
}

static void emit_pdf_header(SkWStream* stream) {
    stream->writeText("%PDF-1.4\n%");
    // The PDF spec recommends including a comment with four bytes, all
    // with their high bits set.  This is "Skia" with the high bits set.
    stream->write32(0xD3EBE9E1);
    stream->writeText("\n");
}

static void emit_pdf_footer(SkWStream* stream,
                            SkPDFCatalog* catalog,
                            SkPDFObject* docCatalog,
                            int64_t objCount,
                            int32_t xRefFileOffset) {
    SkPDFDict trailerDict;
    // TODO(vandebo): Linearized format will take a Prev entry too.
    // TODO(vandebo): PDF/A requires an ID entry.
    trailerDict.insertInt("Size", int(objCount));
    trailerDict.insert("Root", new SkPDFObjRef(docCatalog))->unref();

    stream->writeText("trailer\n");
    trailerDict.emitObject(stream, catalog);
    stream->writeText("\nstartxref\n");
    stream->writeBigDecAsText(xRefFileOffset);
    stream->writeText("\n%%EOF");
}

bool SkPDFDocument::EmitPDF(const SkTDArray<const SkPDFDevice*>& pageDevices,
                            SkWStream* stream) {
    if (pageDevices.isEmpty()) {
        return false;
    }

    SkTDArray<SkPDFPage*> pages;
    SkAutoTUnref<SkPDFDict> dests(SkNEW(SkPDFDict));

    for (int i = 0; i < pageDevices.count(); i++) {
        SkASSERT(pageDevices[i]);
        SkASSERT(i == 0 ||
                 pageDevices[i - 1]->getCanon() == pageDevices[i]->getCanon());
        // Reference from new passed to pages.
        SkAutoTUnref<SkPDFPage> page(SkNEW_ARGS(SkPDFPage, (pageDevices[i])));
        page->finalizePage();
        page->appendDestinations(dests);
        pages.push(page.detach());
    }
    SkPDFCatalog catalog;

    SkTDArray<SkPDFDict*> pageTree;
    SkAutoTUnref<SkPDFDict> docCatalog(SkNEW_ARGS(SkPDFDict, ("Catalog")));

    SkPDFDict* pageTreeRoot;
    SkPDFPage::GeneratePageTree(pages, &pageTree, &pageTreeRoot);

    docCatalog->insert("Pages", new SkPDFObjRef(pageTreeRoot))->unref();

    /* TODO(vandebo): output intent
    SkAutoTUnref<SkPDFDict> outputIntent = new SkPDFDict("OutputIntent");
    outputIntent->insert("S", new SkPDFName("GTS_PDFA1"))->unref();
    outputIntent->insert("OutputConditionIdentifier",
                         new SkPDFString("sRGB"))->unref();
    SkAutoTUnref<SkPDFArray> intentArray = new SkPDFArray;
    intentArray->append(outputIntent.get());
    docCatalog->insert("OutputIntent", intentArray.get());
    */

    if (dests->size() > 0) {
        docCatalog->insert("Dests", SkNEW_ARGS(SkPDFObjRef, (dests.get())))
                ->unref();
    }

    // Build font subsetting info before proceeding.
    perform_font_subsetting(&catalog, pages);

    SkTSet<SkPDFObject*> resourceSet;
    if (resourceSet.add(docCatalog.get())) {
        docCatalog->addResources(&resourceSet, &catalog);
    }
    for (int i = 0; i < resourceSet.count(); ++i) {
        SkAssertResult(catalog.addObject(resourceSet[i]));
    }

    size_t baseOffset = SkToOffT(stream->bytesWritten());
    emit_pdf_header(stream);
    SkTDArray<int32_t> offsets;
    for (int i = 0; i < resourceSet.count(); ++i) {
        SkPDFObject* object = resourceSet[i];
        offsets.push(SkToS32(stream->bytesWritten() - baseOffset));
        SkASSERT(object == catalog.getSubstituteObject(object));
        SkASSERT(catalog.getObjectNumber(object) == i + 1);
        stream->writeDecAsText(i + 1);
        stream->writeText(" 0 obj\n");  // Generation number is always 0.
        object->emitObject(stream, &catalog);
        stream->writeText("\nendobj\n");
    }
    int32_t xRefFileOffset = SkToS32(stream->bytesWritten() - baseOffset);

    int32_t objCount = SkToS32(offsets.count() + 1);

    stream->writeText("xref\n0 ");
    stream->writeDecAsText(objCount + 1);
    stream->writeText("\n0000000000 65535 f \n");
    for (int i = 0; i < offsets.count(); i++) {
        SkASSERT(offsets[i] > 0);
        stream->writeBigDecAsText(offsets[i], 10);
        stream->writeText(" 00000 n \n");
    }
    emit_pdf_footer(stream, &catalog, docCatalog.get(), objCount,
                    xRefFileOffset);

    // The page tree has both child and parent pointers, so it creates a
    // reference cycle.  We must clear that cycle to properly reclaim memory.
    for (int i = 0; i < pageTree.count(); i++) {
        pageTree[i]->clear();
    }
    pageTree.safeUnrefAll();
    pages.unrefAll();
    return true;
}

// TODO(halcanary): expose notEmbeddableCount in SkDocument
void SkPDFDocument::GetCountOfFontTypes(
        const SkTDArray<SkPDFDevice*>& pageDevices,
        int counts[SkAdvancedTypefaceMetrics::kOther_Font + 1],
        int* notSubsettableCount,
        int* notEmbeddableCount) {
    sk_bzero(counts, sizeof(int) *
                     (SkAdvancedTypefaceMetrics::kOther_Font + 1));
    SkTDArray<SkFontID> seenFonts;
    int notSubsettable = 0;
    int notEmbeddable = 0;

    for (int pageNumber = 0; pageNumber < pageDevices.count(); pageNumber++) {
        const SkTDArray<SkPDFFont*>& fontResources =
                pageDevices[pageNumber]->getFontResources();
        for (int font = 0; font < fontResources.count(); font++) {
            SkFontID fontID = fontResources[font]->typeface()->uniqueID();
            if (seenFonts.find(fontID) == -1) {
                counts[fontResources[font]->getType()]++;
                seenFonts.push(fontID);
                if (!fontResources[font]->canSubset()) {
                    notSubsettable++;
                }
                if (!fontResources[font]->canEmbed()) {
                    notEmbeddable++;
                }
            }
        }
    }
    if (notSubsettableCount) {
        *notSubsettableCount = notSubsettable;

    }
    if (notEmbeddableCount) {
        *notEmbeddableCount = notEmbeddable;
    }
}
