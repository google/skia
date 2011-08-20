
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPDFCatalog.h"
#include "SkPDFDevice.h"
#include "SkPDFDocument.h"
#include "SkPDFPage.h"
#include "SkPDFFont.h"
#include "SkStream.h"

// Add the resources, starting at firstIndex to the catalog, removing any dupes.
// A hash table would be really nice here.
void addResourcesToCatalog(int firstIndex, bool firstPage,
                          SkTDArray<SkPDFObject*>* resourceList,
                          SkPDFCatalog* catalog) {
    for (int i = firstIndex; i < resourceList->count(); i++) {
        int index = resourceList->find((*resourceList)[i]);
        if (index != i) {
            (*resourceList)[i]->unref();
            resourceList->removeShuffle(i);
            i--;
        } else {
            catalog->addObject((*resourceList)[i], firstPage);
        }
    }
}

static void perform_font_subsetting(SkPDFCatalog* catalog,
                                    const SkTDArray<SkPDFPage*>& pages,
                                    SkTDArray<SkPDFObject*>* substitutes) {
    SkASSERT(catalog);
    SkASSERT(substitutes);

    SkPDFGlyphSetMap usage;
    for (int i = 0; i < pages.count(); ++i) {
        usage.merge(pages[i]->getFontGlyphUsage());
    }
    SkPDFGlyphSetMap::F2BIter iterator(usage);
    SkPDFGlyphSetMap::FontGlyphSetPair* entry = iterator.next();
    while (entry) {
        SkPDFFont* subsetFont =
            entry->fFont->getFontSubset(entry->fGlyphSet);
        if (subsetFont) {
            catalog->setSubstitute(entry->fFont, subsetFont);
            substitutes->push(subsetFont);  // Transfer ownership to substitutes
        }
        entry = iterator.next();
    }
}

SkPDFDocument::SkPDFDocument(Flags flags)
        : fXRefFileOffset(0),
          fSecondPageFirstResourceIndex(0) {
    fCatalog.reset(new SkPDFCatalog(flags));
    fDocCatalog = new SkPDFDict("Catalog");
    fDocCatalog->unref();  // SkRefPtr and new both took a reference.
    fCatalog->addObject(fDocCatalog.get(), true);
}

SkPDFDocument::~SkPDFDocument() {
    fPages.safeUnrefAll();

    // The page tree has both child and parent pointers, so it creates a
    // reference cycle.  We must clear that cycle to properly reclaim memory.
    for (int i = 0; i < fPageTree.count(); i++) {
        fPageTree[i]->clear();
    }
    fPageTree.safeUnrefAll();
    fPageResources.safeUnrefAll();
    fSubstitutes.safeUnrefAll();
}

bool SkPDFDocument::emitPDF(SkWStream* stream) {
    if (fPages.isEmpty()) {
        return false;
    }
    for (int i = 0; i < fPages.count(); i++) {
        if (fPages[i] == NULL) {
            return false;
        }
    }

    // We haven't emitted the document before if fPageTree is empty.
    if (fPageTree.isEmpty()) {
        SkPDFDict* pageTreeRoot;
        SkPDFPage::GeneratePageTree(fPages, fCatalog.get(), &fPageTree,
                                    &pageTreeRoot);
        fDocCatalog->insert("Pages", new SkPDFObjRef(pageTreeRoot))->unref();

        /* TODO(vandebo): output intent
        SkRefPtr<SkPDFDict> outputIntent = new SkPDFDict("OutputIntent");
        outputIntent->unref();  // SkRefPtr and new both took a reference.
        outputIntent->insert("S", new SkPDFName("GTS_PDFA1"))->unref();
        outputIntent->insert("OutputConditionIdentifier",
                             new SkPDFString("sRGB"))->unref();
        SkRefPtr<SkPDFArray> intentArray = new SkPDFArray;
        intentArray->unref();  // SkRefPtr and new both took a reference.
        intentArray->append(outputIntent.get());
        fDocCatalog->insert("OutputIntent", intentArray.get());
        */

        bool firstPage = true;
        for (int i = 0; i < fPages.count(); i++) {
            int resourceCount = fPageResources.count();
            fPages[i]->finalizePage(fCatalog.get(), firstPage, &fPageResources);
            addResourcesToCatalog(resourceCount, firstPage, &fPageResources,
                                  fCatalog.get());
            if (i == 0) {
                firstPage = false;
                fSecondPageFirstResourceIndex = fPageResources.count();
            }
        }

        // Build font subsetting info before proceeding.
        perform_font_subsetting(fCatalog.get(), fPages, &fSubstitutes);

        // Figure out the size of things and inform the catalog of file offsets.
        off_t fileOffset = headerSize();
        fileOffset += fCatalog->setFileOffset(fDocCatalog.get(), fileOffset);
        fileOffset += fCatalog->setFileOffset(fPages[0], fileOffset);
        fileOffset += fPages[0]->getPageSize(fCatalog.get(), fileOffset);
        for (int i = 0; i < fSecondPageFirstResourceIndex; i++) {
            fileOffset += fCatalog->setFileOffset(fPageResources[i],
                                                  fileOffset);
        }
        // Add the size of resources of substitute objects used on page 1.
        fileOffset += fCatalog->setSubstituteResourcesOffsets(fileOffset, true);
        if (fPages.count() > 1) {
            // TODO(vandebo): For linearized format, save the start of the
            // first page xref table and calculate the size.
        }

        for (int i = 0; i < fPageTree.count(); i++) {
            fileOffset += fCatalog->setFileOffset(fPageTree[i], fileOffset);
        }

        for (int i = 1; i < fPages.count(); i++) {
            fileOffset += fPages[i]->getPageSize(fCatalog.get(), fileOffset);
        }

        for (int i = fSecondPageFirstResourceIndex;
                 i < fPageResources.count();
                 i++) {
            fileOffset += fCatalog->setFileOffset(fPageResources[i],
                                                  fileOffset);
        }

        fileOffset += fCatalog->setSubstituteResourcesOffsets(fileOffset,
                                                              false);
        fXRefFileOffset = fileOffset;
    }

    emitHeader(stream);
    fDocCatalog->emitObject(stream, fCatalog.get(), true);
    fPages[0]->emitObject(stream, fCatalog.get(), true);
    fPages[0]->emitPage(stream, fCatalog.get());
    for (int i = 0; i < fSecondPageFirstResourceIndex; i++) {
        fPageResources[i]->emit(stream, fCatalog.get(), true);
    }
    fCatalog->emitSubstituteResources(stream, true);
    // TODO(vandebo): Support linearized format
    // if (fPages.size() > 1) {
    //     // TODO(vandebo): Save the file offset for the first page xref table.
    //     fCatalog->emitXrefTable(stream, true);
    // }

    for (int i = 0; i < fPageTree.count(); i++) {
        fPageTree[i]->emitObject(stream, fCatalog.get(), true);
    }

    for (int i = 1; i < fPages.count(); i++) {
        fPages[i]->emitPage(stream, fCatalog.get());
    }

    for (int i = fSecondPageFirstResourceIndex;
            i < fPageResources.count();
            i++) {
        fPageResources[i]->emit(stream, fCatalog.get(), true);
    }

    fCatalog->emitSubstituteResources(stream, false);
    int64_t objCount = fCatalog->emitXrefTable(stream, fPages.count() > 1);
    emitFooter(stream, objCount);
    return true;
}

bool SkPDFDocument::setPage(int pageNumber, SkPDFDevice* pdfDevice) {
    if (!fPageTree.isEmpty()) {
        return false;
    }

    pageNumber--;
    SkASSERT(pageNumber >= 0);

    if (pageNumber >= fPages.count()) {
        int oldSize = fPages.count();
        fPages.setCount(pageNumber + 1);
        for (int i = oldSize; i <= pageNumber; i++) {
            fPages[i] = NULL;
        }
    }

    SkPDFPage* page = new SkPDFPage(pdfDevice);
    SkSafeUnref(fPages[pageNumber]);
    fPages[pageNumber] = page;  // Reference from new passed to fPages.
    return true;
}

bool SkPDFDocument::appendPage(SkPDFDevice* pdfDevice) {
    if (!fPageTree.isEmpty()) {
        return false;
    }

    SkPDFPage* page = new SkPDFPage(pdfDevice);
    fPages.push(page);  // Reference from new passed to fPages.
    return true;
}

const SkTDArray<SkPDFPage*>& SkPDFDocument::getPages() {
    return fPages;
}

void SkPDFDocument::emitHeader(SkWStream* stream) {
    stream->writeText("%PDF-1.4\n%");
    // The PDF spec recommends including a comment with four bytes, all
    // with their high bits set.  This is "Skia" with the high bits set.
    stream->write32(0xD3EBE9E1);
    stream->writeText("\n");
}

size_t SkPDFDocument::headerSize() {
    SkDynamicMemoryWStream buffer;
    emitHeader(&buffer);
    return buffer.getOffset();
}

void SkPDFDocument::emitFooter(SkWStream* stream, int64_t objCount) {
    if (fTrailerDict.get() == NULL) {
        fTrailerDict = new SkPDFDict();
        fTrailerDict->unref();  // SkRefPtr and new both took a reference.

        // TODO(vandebo): Linearized format will take a Prev entry too.
        // TODO(vandebo): PDF/A requires an ID entry.
        fTrailerDict->insertInt("Size", objCount);
        fTrailerDict->insert("Root",
                             new SkPDFObjRef(fDocCatalog.get()))->unref();
    }

    stream->writeText("trailer\n");
    fTrailerDict->emitObject(stream, fCatalog.get(), false);
    stream->writeText("\nstartxref\n");
    stream->writeBigDecAsText(fXRefFileOffset);
    stream->writeText("\n%%EOF");
}
