/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace_Base.h"
#include "SkPDFCanvas.h"
#include "SkPDFDevice.h"
#include "SkPDFDocument.h"
#include "SkPDFStream.h"
#include "SkPDFUtils.h"
#include "SkStream.h"

SkPDFObjectSerializer::SkPDFObjectSerializer() : fBaseOffset(0), fNextToBeSerialized(0) {}

template <class T> static void renew(T* t) { t->~T(); new (t) T; }

SkPDFObjectSerializer::~SkPDFObjectSerializer() {
    for (int i = 0; i < fObjNumMap.objects().count(); ++i) {
        fObjNumMap.objects()[i]->drop();
    }
}

void SkPDFObjectSerializer::addObjectRecursively(const sk_sp<SkPDFObject>& object) {
    fObjNumMap.addObjectRecursively(object.get(), fSubstituteMap);
}

#define SKPDF_MAGIC "\xD3\xEB\xE9\xE1"
#ifndef SK_BUILD_FOR_WIN32
static_assert((SKPDF_MAGIC[0] & 0x7F) == "Skia"[0], "");
static_assert((SKPDF_MAGIC[1] & 0x7F) == "Skia"[1], "");
static_assert((SKPDF_MAGIC[2] & 0x7F) == "Skia"[2], "");
static_assert((SKPDF_MAGIC[3] & 0x7F) == "Skia"[3], "");
#endif
void SkPDFObjectSerializer::serializeHeader(SkWStream* wStream,
                                            const SkDocument::PDFMetadata& md) {
    fBaseOffset = wStream->bytesWritten();
    static const char kHeader[] = "%PDF-1.4\n%" SKPDF_MAGIC "\n";
    wStream->write(kHeader, strlen(kHeader));
    // The PDF spec recommends including a comment with four
    // bytes, all with their high bits set.  "\xD3\xEB\xE9\xE1" is
    // "Skia" with the high bits set.
    fInfoDict = SkPDFMetadata::MakeDocumentInformationDict(md);
    this->addObjectRecursively(fInfoDict);
    this->serializeObjects(wStream);
}
#undef SKPDF_MAGIC

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


// return root node.
static sk_sp<SkPDFDict> generate_page_tree(SkTArray<sk_sp<SkPDFDict>>* pages) {
    // PDF wants a tree describing all the pages in the document.  We arbitrary
    // choose 8 (kNodeSize) as the number of allowed children.  The internal
    // nodes have type "Pages" with an array of children, a parent pointer, and
    // the number of leaves below the node as "Count."  The leaves are passed
    // into the method, have type "Page" and need a parent pointer. This method
    // builds the tree bottom up, skipping internal nodes that would have only
    // one child.
    static const int kNodeSize = 8;

    // curNodes takes a reference to its items, which it passes to pageTree.
    int totalPageCount = pages->count();
    SkTArray<sk_sp<SkPDFDict>> curNodes;
    curNodes.swap(pages);

    // nextRoundNodes passes its references to nodes on to curNodes.
    int treeCapacity = kNodeSize;
    do {
        SkTArray<sk_sp<SkPDFDict>> nextRoundNodes;
        for (int i = 0; i < curNodes.count(); ) {
            if (i > 0 && i + 1 == curNodes.count()) {
                SkASSERT(curNodes[i]);
                nextRoundNodes.emplace_back(std::move(curNodes[i]));
                break;
            }

            auto newNode = sk_make_sp<SkPDFDict>("Pages");
            auto kids = sk_make_sp<SkPDFArray>();
            kids->reserve(kNodeSize);

            int count = 0;
            for (; i < curNodes.count() && count < kNodeSize; i++, count++) {
                SkASSERT(curNodes[i]);
                curNodes[i]->insertObjRef("Parent", newNode);
                kids->appendObjRef(std::move(curNodes[i]));
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
                pageCount = ((totalPageCount - 1) % treeCapacity) + 1;
            }
            newNode->insertInt("Count", pageCount);
            newNode->insertObject("Kids", std::move(kids));
            nextRoundNodes.emplace_back(std::move(newNode));
        }
        SkDEBUGCODE( for (const auto& n : curNodes) { SkASSERT(!n); } );

        curNodes.swap(&nextRoundNodes);
        nextRoundNodes.reset();
        treeCapacity *= kNodeSize;
    } while (curNodes.count() > 1);
    return std::move(curNodes[0]);
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
                             const SkDocument::PDFMetadata& metadata,
                             sk_sp<SkPixelSerializer> jpegEncoder,
                             bool pdfa)
    : SkDocument(stream, doneProc)
    , fRasterDpi(rasterDpi)
    , fMetadata(metadata)
    , fPDFA(pdfa) {
    fCanon.setPixelSerializer(std::move(jpegEncoder));
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
    if (fPages.empty()) {
        // if this is the first page if the document.
        fObjectSerializer.serializeHeader(this->getStream(), fMetadata);
        fDests = sk_make_sp<SkPDFDict>();
        if (fPDFA) {
            SkPDFMetadata::UUID uuid = SkPDFMetadata::CreateUUID(fMetadata);
            // We use the same UUID for Document ID and Instance ID since this
            // is the first revision of this document (and Skia does not
            // support revising existing PDF documents).
            // If we are not in PDF/A mode, don't use a UUID since testing
            // works best with reproducible outputs.
            fID = SkPDFMetadata::MakePdfId(uuid, uuid);
            fXMP = SkPDFMetadata::MakeXMPObject(fMetadata, uuid, uuid);
            fObjectSerializer.addObjectRecursively(fXMP);
            fObjectSerializer.serializeObjects(this->getStream());
        }
    }
    SkISize pageSize = SkISize::Make(
            SkScalarRoundToInt(width), SkScalarRoundToInt(height));
    fPageDevice.reset(
            SkPDFDevice::Create(pageSize, fRasterDpi, this));
    fCanvas = sk_make_sp<SkPDFCanvas>(fPageDevice);
    fCanvas->clipRect(trimBox);
    fCanvas->translate(trimBox.x(), trimBox.y());
    return fCanvas.get();
}

void SkPDFDocument::onEndPage() {
    SkASSERT(fCanvas.get());
    fCanvas->flush();
    fCanvas.reset(nullptr);
    SkASSERT(fPageDevice);
    fGlyphUsage.merge(fPageDevice->getFontGlyphUsage());
    auto page = sk_make_sp<SkPDFDict>("Page");
    page->insertObject("Resources", fPageDevice->makeResourceDict());
    page->insertObject("MediaBox", fPageDevice->copyMediaBox());
    auto annotations = sk_make_sp<SkPDFArray>();
    fPageDevice->appendAnnotations(annotations.get());
    if (annotations->size() > 0) {
        page->insertObject("Annots", std::move(annotations));
    }
    auto contentData = fPageDevice->content();
    auto contentObject = sk_make_sp<SkPDFStream>(contentData.get());
    this->serialize(contentObject);
    page->insertObjRef("Contents", std::move(contentObject));
    fPageDevice->appendDestinations(fDests.get(), page.get());
    fPages.emplace_back(std::move(page));
    fPageDevice.reset(nullptr);
}

void SkPDFDocument::onAbort() {
    fCanvas.reset(nullptr);
    fPages.reset();
    fCanon.reset();
    renew(&fObjectSerializer);
}

#ifdef SK_SUPPORT_LEGACY_DOCUMENT_API
void SkPDFDocument::setMetadata(const SkDocument::Attribute info[],
                                int infoCount,
                                const SkTime::DateTime* creationDate,
                                const SkTime::DateTime* modifiedDate) {
    for (int i = 0; i < infoCount; ++i) {
        const SkDocument::Attribute& kv = info[i];
        SkPDFMetadata::SetMetadataByKey(kv.fKey, kv.fValue, &fMetadata);
    }
    if (creationDate) {
        fMetadata.fCreation.fEnabled = true;
        fMetadata.fCreation.fDateTime = *creationDate;
    }
    if (modifiedDate) {
        fMetadata.fModified.fEnabled = true;
        fMetadata.fModified.fDateTime = *modifiedDate;
    }
}
#endif  // SK_SUPPORT_LEGACY_DOCUMENT_API

static sk_sp<SkData> SkSrgbIcm() {
    auto srgb = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
    return as_CSB(srgb)->writeToICC();
}

static sk_sp<SkPDFStream> make_srgb_color_profile() {
    sk_sp<SkData> profile = SkSrgbIcm();
    sk_sp<SkPDFStream> stream = sk_make_sp<SkPDFStream>(profile.get());
    stream->insertInt("N", 3);
    sk_sp<SkPDFArray> array = sk_make_sp<SkPDFArray>();
    array->appendScalar(0.0f);
    array->appendScalar(1.0f);
    array->appendScalar(0.0f);
    array->appendScalar(1.0f);
    array->appendScalar(0.0f);
    array->appendScalar(1.0f);
    stream->insertObject("Range", std::move(array));
    return stream;
}

static sk_sp<SkPDFArray> make_srgb_output_intents() {
    // sRGB is specified by HTML, CSS, and SVG.
    auto outputIntent = sk_make_sp<SkPDFDict>("OutputIntent");
    outputIntent->insertName("S", "GTS_PDFA1");
    outputIntent->insertString("RegistryName", "http://www.color.org");
    outputIntent->insertString("OutputConditionIdentifier",
                               "Custom");
    outputIntent->insertString("Info","sRGB IEC61966-2.1");
    outputIntent->insertObjRef("DestOutputProfile",
                               make_srgb_color_profile());
    auto intentArray = sk_make_sp<SkPDFArray>();
    intentArray->appendObject(std::move(outputIntent));
    return intentArray;
}

bool SkPDFDocument::onClose(SkWStream* stream) {
    SkASSERT(!fCanvas.get());
    if (fPages.empty()) {
        fPages.reset();
        fCanon.reset();
        renew(&fObjectSerializer);
        return false;
    }
    auto docCatalog = sk_make_sp<SkPDFDict>("Catalog");
    if (fPDFA) {
        SkASSERT(fXMP);
        docCatalog->insertObjRef("Metadata", fXMP);
        // Don't specify OutputIntents if we are not in PDF/A mode since
        // no one has ever asked for this feature.
        docCatalog->insertObject("OutputIntents", make_srgb_output_intents());
    }
    SkASSERT(!fPages.empty());
    docCatalog->insertObjRef("Pages", generate_page_tree(&fPages));
    SkASSERT(fPages.empty());

    if (fDests->size() > 0) {
        docCatalog->insertObjRef("Dests", std::move(fDests));
    }

    // Build font subsetting info before calling addObjectRecursively().
    for (const auto& entry : fGlyphUsage) {
        sk_sp<SkPDFFont> subsetFont(
                entry.fFont->getFontSubset(entry.fGlyphSet));
        if (subsetFont) {
            fObjectSerializer.fSubstituteMap.setSubstitute(
                    entry.fFont, subsetFont.get());
        }
    }

    fObjectSerializer.addObjectRecursively(docCatalog);
    fObjectSerializer.serializeObjects(this->getStream());
    fObjectSerializer.serializeFooter(this->getStream(), docCatalog, fID);
    fCanon.reset();
    renew(&fObjectSerializer);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkDocument> SkPDFMakeDocument(SkWStream* stream,
                                    void (*proc)(SkWStream*, bool),
                                    SkScalar dpi,
                                    const SkDocument::PDFMetadata& metadata,
                                    sk_sp<SkPixelSerializer> jpeg,
                                    bool pdfa) {
    return stream ? sk_make_sp<SkPDFDocument>(stream, proc, dpi, metadata,
                                              std::move(jpeg), pdfa)
                  : nullptr;
}

sk_sp<SkDocument> SkDocument::MakePDF(const char path[], SkScalar dpi) {
    auto delete_wstream = [](SkWStream* stream, bool) { delete stream; };
    std::unique_ptr<SkFILEWStream> stream(new SkFILEWStream(path));
    return stream->isValid()
                   ? SkPDFMakeDocument(stream.release(), delete_wstream, dpi,
                                       SkDocument::PDFMetadata(), nullptr,
                                       false)
                   : nullptr;
}

sk_sp<SkDocument> SkDocument::MakePDF(SkWStream* stream,
                                      SkScalar dpi,
                                      const SkDocument::PDFMetadata& metadata,
                                      sk_sp<SkPixelSerializer> jpegEncoder,
                                      bool pdfa) {
    return SkPDFMakeDocument(stream, nullptr, dpi, metadata,
                             std::move(jpegEncoder), pdfa);
}
