
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
#include "SkTSet.h"

static void addResourcesToCatalog(bool firstPage,
                                  SkTSet<SkPDFObject*>* resourceSet,
                                  SkPDFCatalog* catalog) {
    for (int i = 0; i < resourceSet->count(); i++) {
        catalog->addObject((*resourceSet)[i], firstPage);
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
    const SkPDFGlyphSetMap::FontGlyphSetPair* entry = iterator.next();
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

SkPDFDocument::SkPDFDocument()
        : fXRefFileOffset(0),
          fTrailerDict(NULL) {
    fCatalog.reset(SkNEW(SkPDFCatalog));
    fDocCatalog = SkNEW_ARGS(SkPDFDict, ("Catalog"));
    fCatalog->addObject(fDocCatalog, true);
    fFirstPageResources = NULL;
    fOtherPageResources = NULL;
}

SkPDFDocument::~SkPDFDocument() {
    fPages.safeUnrefAll();

    // The page tree has both child and parent pointers, so it creates a
    // reference cycle.  We must clear that cycle to properly reclaim memory.
    for (int i = 0; i < fPageTree.count(); i++) {
        fPageTree[i]->clear();
    }
    fPageTree.safeUnrefAll();

    if (fFirstPageResources) {
        fFirstPageResources->safeUnrefAll();
    }
    if (fOtherPageResources) {
        fOtherPageResources->safeUnrefAll();
    }

    fSubstitutes.safeUnrefAll();

    fDocCatalog->unref();
    SkSafeUnref(fTrailerDict);
    SkDELETE(fFirstPageResources);
    SkDELETE(fOtherPageResources);
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

    fFirstPageResources = SkNEW(SkTSet<SkPDFObject*>);
    fOtherPageResources = SkNEW(SkTSet<SkPDFObject*>);

    // We haven't emitted the document before if fPageTree is empty.
    if (fPageTree.isEmpty()) {
        SkPDFDict* pageTreeRoot;
        SkPDFPage::GeneratePageTree(fPages, fCatalog.get(), &fPageTree,
                                    &pageTreeRoot);
        fDocCatalog->insert("Pages", new SkPDFObjRef(pageTreeRoot))->unref();

        /* TODO(vandebo): output intent
        SkAutoTUnref<SkPDFDict> outputIntent = new SkPDFDict("OutputIntent");
        outputIntent->insert("S", new SkPDFName("GTS_PDFA1"))->unref();
        outputIntent->insert("OutputConditionIdentifier",
                             new SkPDFString("sRGB"))->unref();
        SkAutoTUnref<SkPDFArray> intentArray = new SkPDFArray;
        intentArray->append(outputIntent.get());
        fDocCatalog->insert("OutputIntent", intentArray.get());
        */

        SkAutoTUnref<SkPDFDict> dests(SkNEW(SkPDFDict));

        bool firstPage = true;
        /* The references returned in newResources are transfered to
         * fFirstPageResources or fOtherPageResources depending on firstPage and
         * knownResources doesn't have a reference but just relies on the other
         * two sets to maintain a reference.
         */
        SkTSet<SkPDFObject*> knownResources;

        // mergeInto returns the number of duplicates.
        // If there are duplicates, there is a bug and we mess ref counting.
        SkDEBUGCODE(int duplicates =) knownResources.mergeInto(*fFirstPageResources);
        SkASSERT(duplicates == 0);

        for (int i = 0; i < fPages.count(); i++) {
            if (i == 1) {
                firstPage = false;
                SkDEBUGCODE(duplicates =) knownResources.mergeInto(*fOtherPageResources);
            }
            SkTSet<SkPDFObject*> newResources;
            fPages[i]->finalizePage(
                fCatalog.get(), firstPage, knownResources, &newResources);
            addResourcesToCatalog(firstPage, &newResources, fCatalog.get());
            if (firstPage) {
                SkDEBUGCODE(duplicates =) fFirstPageResources->mergeInto(newResources);
            } else {
                SkDEBUGCODE(duplicates =) fOtherPageResources->mergeInto(newResources);
            }
            SkASSERT(duplicates == 0);

            SkDEBUGCODE(duplicates =) knownResources.mergeInto(newResources);
            SkASSERT(duplicates == 0);

            fPages[i]->appendDestinations(dests);
        }

        if (dests->size() > 0) {
            SkPDFDict* raw_dests = dests.get();
            fFirstPageResources->add(dests.detach());  // Transfer ownership.
            fCatalog->addObject(raw_dests, true /* onFirstPage */);
            fDocCatalog->insert("Dests", SkNEW_ARGS(SkPDFObjRef, (raw_dests)))->unref();
        }

        // Build font subsetting info before proceeding.
        perform_font_subsetting(fCatalog.get(), fPages, &fSubstitutes);
    }

    SkTSet<SkPDFObject*> resourceSet;
    if (resourceSet.add(fDocCatalog)) {
        fDocCatalog->addResources(&resourceSet, fCatalog);
    }
    off_t baseOffset = SkToOffT(stream->bytesWritten());
    emitHeader(stream);
    for (int i = 0; i < resourceSet.count(); ++i) {
        SkPDFObject* object = resourceSet[i];
        fCatalog->setFileOffset(object,
                                SkToOffT(stream->bytesWritten()) - baseOffset);
        SkASSERT(object == fCatalog->getSubstituteObject(object));
        stream->writeDecAsText(fCatalog->getObjectNumber(object));
        stream->writeText(" 0 obj\n");  // Generation number is always 0.
        object->emitObject(stream, fCatalog);
        stream->writeText("\nendobj\n");
    }
    fXRefFileOffset = SkToOffT(stream->bytesWritten()) - baseOffset;
    int64_t objCount = fCatalog->emitXrefTable(stream, fPages.count() > 1);
    emitFooter(stream, objCount);
    return true;
}

// TODO(halcanary): remove this method, since it is unused.
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

// Deprecated.
// TODO(halcanary): remove
void SkPDFDocument::getCountOfFontTypes(
        int counts[SkAdvancedTypefaceMetrics::kOther_Font + 2]) const {
    sk_bzero(counts, sizeof(int) *
                     (SkAdvancedTypefaceMetrics::kOther_Font + 2));
    SkTDArray<SkFontID> seenFonts;
    int notEmbeddable = 0;

    for (int pageNumber = 0; pageNumber < fPages.count(); pageNumber++) {
        const SkTDArray<SkPDFFont*>& fontResources =
                fPages[pageNumber]->getFontResources();
        for (int font = 0; font < fontResources.count(); font++) {
            SkFontID fontID = fontResources[font]->typeface()->uniqueID();
            if (seenFonts.find(fontID) == -1) {
                counts[fontResources[font]->getType()]++;
                seenFonts.push(fontID);
                if (!fontResources[font]->canEmbed()) {
                    notEmbeddable++;
                }
            }
        }
    }
    counts[SkAdvancedTypefaceMetrics::kOther_Font + 1] = notEmbeddable;
}

// TODO(halcanary): expose notEmbeddableCount in SkDocument
void SkPDFDocument::getCountOfFontTypes(
        int counts[SkAdvancedTypefaceMetrics::kOther_Font + 1],
        int* notSubsettableCount,
        int* notEmbeddableCount) const {
    sk_bzero(counts, sizeof(int) *
                     (SkAdvancedTypefaceMetrics::kOther_Font + 1));
    SkTDArray<SkFontID> seenFonts;
    int notSubsettable = 0;
    int notEmbeddable = 0;

    for (int pageNumber = 0; pageNumber < fPages.count(); pageNumber++) {
        const SkTDArray<SkPDFFont*>& fontResources =
                fPages[pageNumber]->getFontResources();
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

void SkPDFDocument::emitHeader(SkWStream* stream) {
    stream->writeText("%PDF-1.4\n%");
    // The PDF spec recommends including a comment with four bytes, all
    // with their high bits set.  This is "Skia" with the high bits set.
    stream->write32(0xD3EBE9E1);
    stream->writeText("\n");
}

//TODO(halcanary): remove this function
size_t SkPDFDocument::headerSize() {
    SkDynamicMemoryWStream buffer;
    emitHeader(&buffer);
    return buffer.getOffset();
}

void SkPDFDocument::emitFooter(SkWStream* stream, int64_t objCount) {
    if (NULL == fTrailerDict) {
        fTrailerDict = SkNEW(SkPDFDict);

        // TODO(vandebo): Linearized format will take a Prev entry too.
        // TODO(vandebo): PDF/A requires an ID entry.
        fTrailerDict->insertInt("Size", int(objCount));
        fTrailerDict->insert("Root", new SkPDFObjRef(fDocCatalog))->unref();
    }

    stream->writeText("trailer\n");
    fTrailerDict->emitObject(stream, fCatalog.get());
    stream->writeText("\nstartxref\n");
    stream->writeBigDecAsText(fXRefFileOffset);
    stream->writeText("\n%%EOF");
}
