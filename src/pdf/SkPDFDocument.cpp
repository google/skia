/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFCanon.h"
#include "SkPDFDevice.h"
#include "SkPDFDocument.h"
#include "SkPDFFont.h"
#include "SkPDFMetadata.h"
#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkPDFUtils.h"
#include "SkStream.h"

static void emit_pdf_header(SkWStream* stream) {
    stream->writeText("%PDF-1.4\n%");
    // The PDF spec recommends including a comment with four bytes, all
    // with their high bits set.  This is "Skia" with the high bits set.
    stream->write32(0xD3EBE9E1);
    stream->writeText("\n");
}

static void emit_pdf_footer(SkWStream* stream,
                            const SkPDFObjNumMap& objNumMap,
                            const SkPDFSubstituteMap& substitutes,
                            SkPDFObject* docCatalog,
                            int64_t objCount,
                            int32_t xRefFileOffset,
                            sk_sp<SkPDFObject> info,
                            sk_sp<SkPDFObject> id) {
    SkPDFDict trailerDict;
    // TODO(http://crbug.com/80908): Linearized format will take a
    //                               Prev entry too.
    trailerDict.insertInt("Size", int(objCount));
    trailerDict.insertObjRef("Root", sk_ref_sp(docCatalog));
    SkASSERT(info);
    trailerDict.insertObjRef("Info", std::move(info));
    if (id) {
        trailerDict.insertObject("ID", std::move(id));
    }
    stream->writeText("trailer\n");
    trailerDict.emitObject(stream, objNumMap, substitutes);
    stream->writeText("\nstartxref\n");
    stream->writeBigDecAsText(xRefFileOffset);
    stream->writeText("\n%%EOF");
}

static void perform_font_subsetting(
        const SkTArray<sk_sp<const SkPDFDevice>>& pageDevices,
        SkPDFSubstituteMap* substituteMap) {
    SkASSERT(substituteMap);

    SkPDFGlyphSetMap usage;
    for (const sk_sp<const SkPDFDevice>& pageDevice : pageDevices) {
        usage.merge(pageDevice->getFontGlyphUsage());
    }
    SkPDFGlyphSetMap::F2BIter iterator(usage);
    const SkPDFGlyphSetMap::FontGlyphSetPair* entry = iterator.next();
    while (entry) {
        sk_sp<SkPDFFont> subsetFont(
                entry->fFont->getFontSubset(entry->fGlyphSet));
        if (subsetFont) {
            substituteMap->setSubstitute(entry->fFont, subsetFont.get());
        }
        entry = iterator.next();
    }
}

static sk_sp<SkPDFDict> create_pdf_page(const SkPDFDevice* pageDevice) {
    auto page = sk_make_sp<SkPDFDict>("Page");
    page->insertObject("Resources", pageDevice->makeResourceDict());
    page->insertObject("MediaBox", pageDevice->copyMediaBox());
    auto annotations = sk_make_sp<SkPDFArray>();
    pageDevice->appendAnnotations(annotations.get());
    if (annotations->size() > 0) {
        page->insertObject("Annots", std::move(annotations));
    }
    auto content = pageDevice->content();
    page->insertObjRef("Contents", sk_make_sp<SkPDFStream>(content.get()));
    return page;
}

// return root node.
static sk_sp<SkPDFDict> generate_page_tree(
        const SkTDArray<SkPDFDict*>& pages,
        SkTDArray<SkPDFDict*>* pageTree) {
    // PDF wants a tree describing all the pages in the document.  We arbitrary
    // choose 8 (kNodeSize) as the number of allowed children.  The internal
    // nodes have type "Pages" with an array of children, a parent pointer, and
    // the number of leaves below the node as "Count."  The leaves are passed
    // into the method, have type "Page" and need a parent pointer. This method
    // builds the tree bottom up, skipping internal nodes that would have only
    // one child.
    static const int kNodeSize = 8;

    // curNodes takes a reference to its items, which it passes to pageTree.
    SkTDArray<SkPDFDict*> curNodes;
    curNodes.setReserve(pages.count());
    for (int i = 0; i < pages.count(); i++) {
        SkSafeRef(pages[i]);
        curNodes.push(pages[i]);
    }

    // nextRoundNodes passes its references to nodes on to curNodes.
    SkTDArray<SkPDFDict*> nextRoundNodes;
    nextRoundNodes.setReserve((pages.count() + kNodeSize - 1)/kNodeSize);

    int treeCapacity = kNodeSize;
    do {
        for (int i = 0; i < curNodes.count(); ) {
            if (i > 0 && i + 1 == curNodes.count()) {
                nextRoundNodes.push(curNodes[i]);
                break;
            }

            auto newNode = sk_make_sp<SkPDFDict>("Pages");
            auto kids = sk_make_sp<SkPDFArray>();
            kids->reserve(kNodeSize);

            int count = 0;
            for (; i < curNodes.count() && count < kNodeSize; i++, count++) {
                curNodes[i]->insertObjRef("Parent", newNode);
                kids->appendObjRef(sk_ref_sp(curNodes[i]));

                // TODO(vandebo): put the objects in strict access order.
                // Probably doesn't matter because they are so small.
                if (curNodes[i] != pages[0]) {
                    pageTree->push(curNodes[i]);  // Transfer reference.
                } else {
                    SkSafeUnref(curNodes[i]);
                }
            }

            // treeCapacity is the number of leaf nodes possible for the
            // current set of subtrees being generated. (i.e. 8, 64, 512, ...).
            // It is hard to count the number of leaf nodes in the current
            // subtree. However, by construction, we know that unless it's the
            // last subtree for the current depth, the leaf count will be
            // treeCapacity, otherwise it's what ever is left over after
            // consuming treeCapacity chunks.
            int pageCount = treeCapacity;
            if (i == curNodes.count()) {
                pageCount = ((pages.count() - 1) % treeCapacity) + 1;
            }
            newNode->insertInt("Count", pageCount);
            newNode->insertObject("Kids", std::move(kids));
            nextRoundNodes.push(newNode.release());  // Transfer reference.
        }

        curNodes = nextRoundNodes;
        nextRoundNodes.rewind();
        treeCapacity *= kNodeSize;
    } while (curNodes.count() > 1);

    pageTree->push(curNodes[0]);  // Transfer reference.
    return sk_ref_sp(curNodes[0]);
}

static bool emit_pdf_document(const SkTArray<sk_sp<const SkPDFDevice>>& pageDevices,
                              const SkPDFMetadata& metadata,
                              SkWStream* stream) {
    if (pageDevices.empty()) {
        return false;
    }

    SkTDArray<SkPDFDict*> pages;  // TODO: SkTArray<sk_sp<SkPDFDict>>
    auto dests = sk_make_sp<SkPDFDict>();

    for (const sk_sp<const SkPDFDevice>& pageDevice : pageDevices) {
        SkASSERT(pageDevice);
        SkASSERT(pageDevices[0]->getCanon() == pageDevice->getCanon());
        sk_sp<SkPDFDict> page(create_pdf_page(pageDevice.get()));
        pageDevice->appendDestinations(dests.get(), page.get());
        pages.push(page.release());
    }

    auto docCatalog = sk_make_sp<SkPDFDict>("Catalog");

    sk_sp<SkPDFObject> infoDict(metadata.createDocumentInformationDict());

    sk_sp<SkPDFObject> id, xmp;
#ifdef SK_PDF_GENERATE_PDFA
    SkPDFMetadata::UUID uuid = metadata.uuid();
    // We use the same UUID for Document ID and Instance ID since this
    // is the first revision of this document (and Skia does not
    // support revising existing PDF documents).
    // If we are not in PDF/A mode, don't use a UUID since testing
    // works best with reproducible outputs.
    id.reset(SkPDFMetadata::CreatePdfId(uuid, uuid));
    xmp.reset(metadata.createXMPObject(uuid, uuid));
    docCatalog->insertObjRef("Metadata", std::move(xmp));

    // sRGB is specified by HTML, CSS, and SVG.
    auto outputIntent = sk_make_sp<SkPDFDict>("OutputIntent");
    outputIntent->insertName("S", "GTS_PDFA1");
    outputIntent->insertString("RegistryName", "http://www.color.org");
    outputIntent->insertString("OutputConditionIdentifier",
                               "sRGB IEC61966-2.1");
    auto intentArray = sk_make_sp<SkPDFArray>();
    intentArray->appendObject(std::move(outputIntent));
    // Don't specify OutputIntents if we are not in PDF/A mode since
    // no one has ever asked for this feature.
    docCatalog->insertObject("OutputIntents", std::move(intentArray));
#endif

    SkTDArray<SkPDFDict*> pageTree;
    docCatalog->insertObjRef("Pages", generate_page_tree(pages, &pageTree));

    if (dests->size() > 0) {
        docCatalog->insertObjRef("Dests", std::move(dests));
    }

    // Build font subsetting info before proceeding.
    SkPDFSubstituteMap substitutes;
    perform_font_subsetting(pageDevices, &substitutes);

    SkPDFObjNumMap objNumMap;
    objNumMap.addObjectRecursively(infoDict.get(), substitutes);
    objNumMap.addObjectRecursively(docCatalog.get(), substitutes);
    size_t baseOffset = stream->bytesWritten();
    emit_pdf_header(stream);
    SkTDArray<int32_t> offsets;
    for (int i = 0; i < objNumMap.objects().count(); ++i) {
        SkPDFObject* object = objNumMap.objects()[i].get();
        size_t offset = stream->bytesWritten();
        // This assert checks that size(pdf_header) > 0 and that
        // the output stream correctly reports bytesWritten().
        SkASSERT(offset > baseOffset);
        offsets.push(SkToS32(offset - baseOffset));
        SkASSERT(object == substitutes.getSubstitute(object));
        SkASSERT(objNumMap.getObjectNumber(object) == i + 1);
        stream->writeDecAsText(i + 1);
        stream->writeText(" 0 obj\n");  // Generation number is always 0.
        object->emitObject(stream, objNumMap, substitutes);
        stream->writeText("\nendobj\n");
        object->drop();
    }
    int32_t xRefFileOffset = SkToS32(stream->bytesWritten() - baseOffset);

    // Include the zeroth object in the count.
    int32_t objCount = SkToS32(offsets.count() + 1);

    stream->writeText("xref\n0 ");
    stream->writeDecAsText(objCount);
    stream->writeText("\n0000000000 65535 f \n");
    for (int i = 0; i < offsets.count(); i++) {
        SkASSERT(offsets[i] > 0);
        stream->writeBigDecAsText(offsets[i], 10);
        stream->writeText(" 00000 n \n");
    }
    emit_pdf_footer(stream, objNumMap, substitutes, docCatalog.get(), objCount,
                    xRefFileOffset, std::move(infoDict), std::move(id));

    // The page tree has both child and parent pointers, so it creates a
    // reference cycle.  We must clear that cycle to properly reclaim memory.
    for (int i = 0; i < pageTree.count(); i++) {
        pageTree[i]->drop();
    }
    pageTree.safeUnrefAll();
    pages.unrefAll();
    return true;
}

#if 0
// TODO(halcanary): expose notEmbeddableCount in SkDocument
void GetCountOfFontTypes(
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
#endif

template <typename T> static T* clone(const T* o) { return o ? new T(*o) : nullptr; }
////////////////////////////////////////////////////////////////////////////////

namespace {
class SkPDFDocument : public SkDocument {
public:
    SkPDFDocument(SkWStream* stream,
                   void (*doneProc)(SkWStream*, bool),
                   SkScalar rasterDpi,
                   SkPixelSerializer* jpegEncoder)
        : SkDocument(stream, doneProc)
        , fRasterDpi(rasterDpi) {
        fCanon.setPixelSerializer(SkSafeRef(jpegEncoder));
    }

    virtual ~SkPDFDocument() {
        // subclasses must call close() in their destructors
        this->close();
    }

protected:
    SkCanvas* onBeginPage(SkScalar width, SkScalar height,
                          const SkRect& trimBox) override {
        SkASSERT(!fCanvas.get());

        SkISize pageSize = SkISize::Make(
                SkScalarRoundToInt(width), SkScalarRoundToInt(height));
        sk_sp<SkPDFDevice> device(
                SkPDFDevice::Create(pageSize, fRasterDpi, &fCanon));
        fCanvas.reset(new SkCanvas(device.get()));
        fPageDevices.push_back(std::move(device));
        fCanvas->clipRect(trimBox);
        fCanvas->translate(trimBox.x(), trimBox.y());
        return fCanvas.get();
    }

    void onEndPage() override {
        SkASSERT(fCanvas.get());
        fCanvas->flush();
        fCanvas.reset(nullptr);
    }

    bool onClose(SkWStream* stream) override {
        SkASSERT(!fCanvas.get());

        bool success = emit_pdf_document(fPageDevices, fMetadata, stream);
        fPageDevices.reset();
        fCanon.reset();
        return success;
    }

    void onAbort() override {
        fPageDevices.reset();
        fCanon.reset();
    }

    void setMetadata(const SkDocument::Attribute info[],
                     int infoCount,
                     const SkTime::DateTime* creationDate,
                     const SkTime::DateTime* modifiedDate) override {
        fMetadata.fInfo.reset(info, infoCount);
        fMetadata.fCreation.reset(clone(creationDate));
        fMetadata.fModified.reset(clone(modifiedDate));
    }

private:
    SkPDFCanon fCanon;
    SkTArray<sk_sp<const SkPDFDevice>> fPageDevices;
    sk_sp<SkCanvas> fCanvas;
    SkScalar fRasterDpi;
    SkPDFMetadata fMetadata;
};
}  // namespace
///////////////////////////////////////////////////////////////////////////////

sk_sp<SkDocument> SkPDFMakeDocument(SkWStream* stream,
                                    void (*proc)(SkWStream*, bool),
                                    SkScalar dpi,
                                    SkPixelSerializer* jpeg) {
    return stream ? sk_make_sp<SkPDFDocument>(stream, proc, dpi, jpeg) : nullptr;
}

SkDocument* SkDocument::CreatePDF(SkWStream* stream, SkScalar dpi) {
    return SkPDFMakeDocument(stream, nullptr, dpi, nullptr).release();
}

SkDocument* SkDocument::CreatePDF(SkWStream* stream,
                                  SkScalar dpi,
                                  SkPixelSerializer* jpegEncoder) {
    return SkPDFMakeDocument(stream, nullptr, dpi, jpegEncoder).release();
}

SkDocument* SkDocument::CreatePDF(const char path[], SkScalar dpi) {
    auto delete_wstream = [](SkWStream* stream, bool) { delete stream; };
    SkAutoTDelete<SkFILEWStream> stream(new SkFILEWStream(path));
    return stream->isValid()
        ? SkPDFMakeDocument(stream.release(), delete_wstream, dpi, nullptr).release()
        : nullptr;
}
