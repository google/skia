/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFCanon.h"
#include "SkPDFCanvas.h"
#include "SkPDFDevice.h"
#include "SkPDFDocument.h"
#include "SkPDFFont.h"
#include "SkPDFStream.h"
#include "SkPDFUtils.h"
#include "SkStream.h"


SkPDFObjectSerializer::SkPDFObjectSerializer() : fBaseOffset(0), fNextToBeSerialized(0) {}

template <class T> static void renew(T* t) { t->~T(); new (t) T; }

void SkPDFObjectSerializer::addObjectRecursively(const sk_sp<SkPDFObject>& object) {
    fObjNumMap.addObjectRecursively(object.get(), fSubstituteMap);
}

void SkPDFObjectSerializer::serializeHeader(SkWStream* wStream, const SkPDFMetadata& md) {
    fBaseOffset = wStream->bytesWritten();
    static const char kHeader[] = "%PDF-1.4\n%\xE1\xE9\xEB\xD3\n";
    wStream->write(kHeader, strlen(kHeader));
    // The PDF spec recommends including a comment with four
    // bytes, all with their high bits set.  "\xD3\xEB\xE9\xE1" is
    // "Skia" with the high bits set.
    fInfoDict.reset(md.createDocumentInformationDict());
    this->addObjectRecursively(fInfoDict);
    this->serializeObjects(wStream);
}

// Serialize all objects in the fObjNumMap that have not yet been serialized;
void SkPDFObjectSerializer::serializeObjects(SkWStream* wStream) {
    const SkTArray<sk_sp<SkPDFObject>>& objects = fObjNumMap.objects();
    while (fNextToBeSerialized < objects.count()) {
        SkPDFObject* object = objects[fNextToBeSerialized].get();
        int32_t index = fNextToBeSerialized + 1;  // Skip object 0.
        // "The first entry in the [XREF] table (object number 0) is
        // always free and has a generation number of 65,535; it is
        // the head of the linked list of free objects."
        SkASSERT(fOffsets.count() == fNextToBeSerialized);
        fOffsets.push(this->offset(wStream));
        SkASSERT(object == fSubstituteMap.getSubstitute(object));
        wStream->writeDecAsText(index);
        wStream->writeText(" 0 obj\n");  // Generation number is always 0.
        object->emitObject(wStream, fObjNumMap, fSubstituteMap);
        wStream->writeText("\nendobj\n");
        object->drop();
        ++fNextToBeSerialized;
    }
}

// Xref table and footer
void SkPDFObjectSerializer::serializeFooter(SkWStream* wStream,
                                            const sk_sp<SkPDFObject> docCatalog,
                                            sk_sp<SkPDFObject> id) {
    this->serializeObjects(wStream);
    int32_t xRefFileOffset = this->offset(wStream);
    // Include the special zeroth object in the count.
    int32_t objCount = SkToS32(fOffsets.count() + 1);
    wStream->writeText("xref\n0 ");
    wStream->writeDecAsText(objCount);
    wStream->writeText("\n0000000000 65535 f \n");
    for (int i = 0; i < fOffsets.count(); i++) {
        wStream->writeBigDecAsText(fOffsets[i], 10);
        wStream->writeText(" 00000 n \n");
    }
    SkPDFDict trailerDict;
    trailerDict.insertInt("Size", objCount);
    SkASSERT(docCatalog);
    trailerDict.insertObjRef("Root", docCatalog);
    SkASSERT(fInfoDict);
    trailerDict.insertObjRef("Info", std::move(fInfoDict));
    if (id) {
        trailerDict.insertObject("ID", std::move(id));
    }
    wStream->writeText("trailer\n");
    trailerDict.emitObject(wStream, fObjNumMap, fSubstituteMap);
    wStream->writeText("\nstartxref\n");
    wStream->writeBigDecAsText(xRefFileOffset);
    wStream->writeText("\n%%EOF");
}

int32_t SkPDFObjectSerializer::offset(SkWStream* wStream) {
    size_t offset = wStream->bytesWritten();
    SkASSERT(offset > fBaseOffset);
    return SkToS32(offset - fBaseOffset);
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
        SkTDArray<SkPDFDict*>& pages,
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

SkPDFDocument::SkPDFDocument(SkWStream* stream,
                             void (*doneProc)(SkWStream*, bool),
                             SkScalar rasterDpi,
                             SkPixelSerializer* jpegEncoder)
    : SkDocument(stream, doneProc)
    , fRasterDpi(rasterDpi) {
    fCanon.setPixelSerializer(SkSafeRef(jpegEncoder));
}

SkPDFDocument::~SkPDFDocument() {
    // subclasses of SkDocument must call close() in their destructors.
    this->close();
}

void SkPDFDocument::serialize(const sk_sp<SkPDFObject>& object) {
    fObjectSerializer.addObjectRecursively(object);
    fObjectSerializer.serializeObjects(this->getStream());
}

SkCanvas* SkPDFDocument::onBeginPage(SkScalar width, SkScalar height,
                                     const SkRect& trimBox) {
    SkASSERT(!fCanvas.get());  // endPage() was called before this.
    if (fPageDevices.empty()) {
        // if this is the first page if the document.
        fObjectSerializer.serializeHeader(this->getStream(), fMetadata);
    }
    SkISize pageSize = SkISize::Make(
            SkScalarRoundToInt(width), SkScalarRoundToInt(height));
    sk_sp<SkPDFDevice> device(
            SkPDFDevice::Create(pageSize, fRasterDpi, &fCanon));
    fCanvas = sk_make_sp<SkPDFCanvas>(device);
    fPageDevices.push_back(std::move(device));
    fCanvas->clipRect(trimBox);
    fCanvas->translate(trimBox.x(), trimBox.y());
    return fCanvas.get();
}

void SkPDFDocument::onEndPage() {
    SkASSERT(fCanvas.get());
    fCanvas->flush();
    fCanvas.reset(nullptr);
}

void SkPDFDocument::onAbort() {
    fCanvas.reset(nullptr);
    fPageDevices.reset();
    fCanon.reset();
    renew(&fObjectSerializer);
}

void SkPDFDocument::setMetadata(const SkDocument::Attribute info[],
                                int infoCount,
                                const SkTime::DateTime* creationDate,
                                const SkTime::DateTime* modifiedDate) {
    fMetadata.fInfo.reset(info, infoCount);
    fMetadata.fCreation.reset(clone(creationDate));
    fMetadata.fModified.reset(clone(modifiedDate));
}

bool SkPDFDocument::onClose(SkWStream* stream) {
    SkASSERT(!fCanvas.get());
    if (fPageDevices.empty()) {
        fPageDevices.reset();
        fCanon.reset();
        renew(&fObjectSerializer);
        return false;
    }
    SkTDArray<SkPDFDict*> pages;  // TODO: SkTArray<sk_sp<SkPDFDict>>
    auto dests = sk_make_sp<SkPDFDict>();

    for (const sk_sp<const SkPDFDevice>& pageDevice : fPageDevices) {
        SkASSERT(pageDevice);
        SkASSERT(fPageDevices[0]->getCanon() == pageDevice->getCanon());
        sk_sp<SkPDFDict> page(create_pdf_page(pageDevice.get()));
        pageDevice->appendDestinations(dests.get(), page.get());
        pages.push(page.release());
    }

    auto docCatalog = sk_make_sp<SkPDFDict>("Catalog");

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

    // Build font subsetting info before calling addObjectRecursively().
    perform_font_subsetting(fPageDevices, &fObjectSerializer.fSubstituteMap);

    fObjectSerializer.addObjectRecursively(docCatalog);
    fObjectSerializer.serializeObjects(this->getStream());
    fObjectSerializer.serializeFooter(
            this->getStream(), docCatalog, std::move(id));
    pageTree.unrefAll();  // TODO(halcanary): make this unnecesary by
                          // refactoring generate_page_tree().
    pages.unrefAll();
    fPageDevices.reset();
    fCanon.reset();
    renew(&fObjectSerializer);
    return true;
}

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
