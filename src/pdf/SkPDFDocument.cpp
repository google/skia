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
void SkPDFObjectSerializer::serializeHeader(SkWStream* wStream, const SkPDFMetadata& md) {
    fBaseOffset = wStream->bytesWritten();
    static const char kHeader[] = "%PDF-1.4\n%" SKPDF_MAGIC "\n";
    wStream->write(kHeader, strlen(kHeader));
    // The PDF spec recommends including a comment with four
    // bytes, all with their high bits set.  "\xD3\xEB\xE9\xE1" is
    // "Skia" with the high bits set.
    fInfoDict.reset(md.createDocumentInformationDict());
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
    if (fPages.empty()) {
        // if this is the first page if the document.
        fObjectSerializer.serializeHeader(this->getStream(), fMetadata);
        fDests = sk_make_sp<SkPDFDict>();
        #ifdef SK_PDF_GENERATE_PDFA
            SkPDFMetadata::UUID uuid = fMetadata.uuid();
            // We use the same UUID for Document ID and Instance ID since this
            // is the first revision of this document (and Skia does not
            // support revising existing PDF documents).
            // If we are not in PDF/A mode, don't use a UUID since testing
            // works best with reproducible outputs.
            fID.reset(SkPDFMetadata::CreatePdfId(uuid, uuid));
            fXMP.reset(fMetadata.createXMPObject(uuid, uuid));
            fObjectSerializer.addObjectRecursively(fXMP);
            fObjectSerializer.serializeObjects(this->getStream());
        #endif
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

void SkPDFDocument::setMetadata(const SkDocument::Attribute info[],
                                int infoCount,
                                const SkTime::DateTime* creationDate,
                                const SkTime::DateTime* modifiedDate) {
    fMetadata.fInfo.reset(info, infoCount);
    fMetadata.fCreation.reset(clone(creationDate));
    fMetadata.fModified.reset(clone(modifiedDate));
}

#ifdef SK_PDF_GENERATE_PDFA
static sk_sp<SkData> SkSrgbIcm() {
    // Source: http://www.argyllcms.com/icclibsrc.html
    static const char kProfile[] =
        "\000\000\014\214argl\002 \000\000mntrRGB XYZ \007\335\000\007\000\037"
        "\000\023\000\020\000'acspMSFT\000\000\000\000IEC sRGB\000\000\000\000"
        "\000\000\000\000\000\000\000\000\000\000\366\326\000\001\000\000\000"
        "\000\323-argl\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
        "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
        "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\021"
        "desc\000\000\001P\000\000\000\231cprt\000\000\001\354\000\000\000g"
        "dmnd\000\000\002T\000\000\000pdmdd\000\000\002\304\000\000\000\210"
        "tech\000\000\003L\000\000\000\014vued\000\000\003X\000\000\000gvie"
        "w\000\000\003\300\000\000\000$lumi\000\000\003\344\000\000\000\024"
        "meas\000\000\003\370\000\000\000$wtpt\000\000\004\034\000\000\000\024"
        "bkpt\000\000\0040\000\000\000\024rXYZ\000\000\004D\000\000\000\024"
        "gXYZ\000\000\004X\000\000\000\024bXYZ\000\000\004l\000\000\000\024"
        "rTRC\000\000\004\200\000\000\010\014gTRC\000\000\004\200\000\000\010"
        "\014bTRC\000\000\004\200\000\000\010\014desc\000\000\000\000\000\000"
        "\000?sRGB IEC61966-2.1 (Equivalent to www.srgb.com 1998 HP profile"
        ")\000\000\000\000\000\000\000\000\000\000\000?sRGB IEC61966-2.1 (E"
        "quivalent to www.srgb.com 1998 HP profile)\000\000\000\000\000\000"
        "\000\000text\000\000\000\000Created by Graeme W. Gill. Released in"
        "to the public domain. No Warranty, Use at your own risk.\000\000de"
        "sc\000\000\000\000\000\000\000\026IEC http://www.iec.ch\000\000\000"
        "\000\000\000\000\000\000\000\000\026IEC http://www.iec.ch\000\000\000"
        "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
        "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
        "\000\000\000\000\000\000\000\000\000desc\000\000\000\000\000\000\000"
        ".IEC 61966-2.1 Default RGB colour space - sRGB\000\000\000\000\000"
        "\000\000\000\000\000\000.IEC 61966-2.1 Default RGB colour space - "
        "sRGB\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
        "\000\000\000\000\000\000sig \000\000\000\000CRT desc\000\000\000\000"
        "\000\000\000\rIEC61966-2.1\000\000\000\000\000\000\000\000\000\000"
        "\000\rIEC61966-2.1\000\000\000\000\000\000\000\000\000\000\000\000"
        "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
        "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
        "\000\000\000\000\000\000\000\000\000\000view\000\000\000\000\000\023"
        "\244|\000\024_0\000\020\316\002\000\003\355\262\000\004\023\n\000\003"
        "\\g\000\000\000\001XYZ \000\000\000\000\000L\n=\000P\000\000\000W\036"
        "\270meas\000\000\000\000\000\000\000\001\000\000\000\000\000\000\000"
        "\000\000\000\000\000\000\000\000\000\000\000\002\217\000\000\000\002"
        "XYZ \000\000\000\000\000\000\363Q\000\001\000\000\000\001\026\314X"
        "YZ \000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
        "XYZ \000\000\000\000\000\000o\240\000\0008\365\000\000\003\220XYZ "
        "\000\000\000\000\000\000b\227\000\000\267\207\000\000\030\331XYZ \000"
        "\000\000\000\000\000$\237\000\000\017\204\000\000\266\303curv\000\000"
        "\000\000\000\000\004\000\000\000\000\005\000\n\000\017\000\024\000"
        "\031\000\036\000#\000(\000-\0002\0007\000;\000@\000E\000J\000O\000"
        "T\000Y\000^\000c\000h\000m\000r\000w\000|\000\201\000\206\000\213\000"
        "\220\000\225\000\232\000\237\000\244\000\251\000\256\000\262\000\267"
        "\000\274\000\301\000\306\000\313\000\320\000\325\000\333\000\340\000"
        "\345\000\353\000\360\000\366\000\373\001\001\001\007\001\r\001\023"
        "\001\031\001\037\001%\001+\0012\0018\001>\001E\001L\001R\001Y\001`"
        "\001g\001n\001u\001|\001\203\001\213\001\222\001\232\001\241\001\251"
        "\001\261\001\271\001\301\001\311\001\321\001\331\001\341\001\351\001"
        "\362\001\372\002\003\002\014\002\024\002\035\002&\002/\0028\002A\002"
        "K\002T\002]\002g\002q\002z\002\204\002\216\002\230\002\242\002\254"
        "\002\266\002\301\002\313\002\325\002\340\002\353\002\365\003\000\003"
        "\013\003\026\003!\003-\0038\003C\003O\003Z\003f\003r\003~\003\212\003"
        "\226\003\242\003\256\003\272\003\307\003\323\003\340\003\354\003\371"
        "\004\006\004\023\004 \004-\004;\004H\004U\004c\004q\004~\004\214\004"
        "\232\004\250\004\266\004\304\004\323\004\341\004\360\004\376\005\r"
        "\005\034\005+\005:\005I\005X\005g\005w\005\206\005\226\005\246\005"
        "\265\005\305\005\325\005\345\005\366\006\006\006\026\006'\0067\006"
        "H\006Y\006j\006{\006\214\006\235\006\257\006\300\006\321\006\343\006"
        "\365\007\007\007\031\007+\007=\007O\007a\007t\007\206\007\231\007\254"
        "\007\277\007\322\007\345\007\370\010\013\010\037\0102\010F\010Z\010"
        "n\010\202\010\226\010\252\010\276\010\322\010\347\010\373\t\020\t%"
        "\t:\tO\td\ty\t\217\t\244\t\272\t\317\t\345\t\373\n\021\n'\n=\nT\nj"
        "\n\201\n\230\n\256\n\305\n\334\n\363\013\013\013\"\0139\013Q\013i\013"
        "\200\013\230\013\260\013\310\013\341\013\371\014\022\014*\014C\014"
        "\\\014u\014\216\014\247\014\300\014\331\014\363\r\r\r&\r@\rZ\rt\r\216"
        "\r\251\r\303\r\336\r\370\016\023\016.\016I\016d\016\177\016\233\016"
        "\266\016\322\016\356\017\t\017%\017A\017^\017z\017\226\017\263\017"
        "\317\017\354\020\t\020&\020C\020a\020~\020\233\020\271\020\327\020"
        "\365\021\023\0211\021O\021m\021\214\021\252\021\311\021\350\022\007"
        "\022&\022E\022d\022\204\022\243\022\303\022\343\023\003\023#\023C\023"
        "c\023\203\023\244\023\305\023\345\024\006\024'\024I\024j\024\213\024"
        "\255\024\316\024\360\025\022\0254\025V\025x\025\233\025\275\025\340"
        "\026\003\026&\026I\026l\026\217\026\262\026\326\026\372\027\035\027"
        "A\027e\027\211\027\256\027\322\027\367\030\033\030@\030e\030\212\030"
        "\257\030\325\030\372\031 \031E\031k\031\221\031\267\031\335\032\004"
        "\032*\032Q\032w\032\236\032\305\032\354\033\024\033;\033c\033\212\033"
        "\262\033\332\034\002\034*\034R\034{\034\243\034\314\034\365\035\036"
        "\035G\035p\035\231\035\303\035\354\036\026\036@\036j\036\224\036\276"
        "\036\351\037\023\037>\037i\037\224\037\277\037\352 \025 A l \230 \304"
        " \360!\034!H!u!\241!\316!\373\"'\"U\"\202\"\257\"\335#\n#8#f#\224#"
        "\302#\360$\037$M$|$\253$\332%\t%8%h%\227%\307%\367&'&W&\207&\267&\350"
        "'\030'I'z'\253'\334(\r(?(q(\242(\324)\006)8)k)\235)\320*\002*5*h*\233"
        "*\317+\002+6+i+\235+\321,\005,9,n,\242,\327-\014-A-v-\253-\341.\026"
        ".L.\202.\267.\356/$/Z/\221/\307/\376050l0\2440\3331\0221J1\2021\272"
        "1\3622*2c2\2332\3243\r3F3\1773\2703\3614+4e4\2364\3305\0235M5\2075"
        "\3025\375676r6\2566\3517$7`7\2347\3278\0248P8\2148\3109\0059B9\177"
        "9\2749\371:6:t:\262:\357;-;k;\252;\350<'<e<\244<\343=\"=a=\241=\340"
        "> >`>\240>\340?!?a?\242?\342@#@d@\246@\347A)AjA\254A\356B0BrB\265B"
        "\367C:C}C\300D\003DGD\212D\316E\022EUE\232E\336F\"FgF\253F\360G5G{"
        "G\300H\005HKH\221H\327I\035IcI\251I\360J7J}J\304K\014KSK\232K\342L"
        "*LrL\272M\002MJM\223M\334N%NnN\267O\000OIO\223O\335P'PqP\273Q\006Q"
        "PQ\233Q\346R1R|R\307S\023S_S\252S\366TBT\217T\333U(UuU\302V\017V\\"
        "V\251V\367WDW\222W\340X/X}X\313Y\032YiY\270Z\007ZVZ\246Z\365[E[\225"
        "[\345\\5\\\206\\\326]']x]\311^\032^l^\275_\017_a_\263`\005`W`\252`"
        "\374aOa\242a\365bIb\234b\360cCc\227c\353d@d\224d\351e=e\222e\347f="
        "f\222f\350g=g\223g\351h?h\226h\354iCi\232i\361jHj\237j\367kOk\247k"
        "\377lWl\257m\010m`m\271n\022nkn\304o\036oxo\321p+p\206p\340q:q\225"
        "q\360rKr\246s\001s]s\270t\024tpt\314u(u\205u\341v>v\233v\370wVw\263"
        "x\021xnx\314y*y\211y\347zFz\245{\004{c{\302|!|\201|\341}A}\241~\001"
        "~b~\302\177#\177\204\177\345\200G\200\250\201\n\201k\201\315\2020\202"
        "\222\202\364\203W\203\272\204\035\204\200\204\343\205G\205\253\206"
        "\016\206r\206\327\207;\207\237\210\004\210i\210\316\2113\211\231\211"
        "\376\212d\212\312\2130\213\226\213\374\214c\214\312\2151\215\230\215"
        "\377\216f\216\316\2176\217\236\220\006\220n\220\326\221?\221\250\222"
        "\021\222z\222\343\223M\223\266\224 \224\212\224\364\225_\225\311\226"
        "4\226\237\227\n\227u\227\340\230L\230\270\231$\231\220\231\374\232"
        "h\232\325\233B\233\257\234\034\234\211\234\367\235d\235\322\236@\236"
        "\256\237\035\237\213\237\372\240i\240\330\241G\241\266\242&\242\226"
        "\243\006\243v\243\346\244V\244\307\2458\245\251\246\032\246\213\246"
        "\375\247n\247\340\250R\250\304\2517\251\251\252\034\252\217\253\002"
        "\253u\253\351\254\\\254\320\255D\255\270\256-\256\241\257\026\257\213"
        "\260\000\260u\260\352\261`\261\326\262K\262\302\2638\263\256\264%\264"
        "\234\265\023\265\212\266\001\266y\266\360\267h\267\340\270Y\270\321"
        "\271J\271\302\272;\272\265\273.\273\247\274!\274\233\275\025\275\217"
        "\276\n\276\204\276\377\277z\277\365\300p\300\354\301g\301\343\302_"
        "\302\333\303X\303\324\304Q\304\316\305K\305\310\306F\306\303\307A\307"
        "\277\310=\310\274\311:\311\271\3128\312\267\3136\313\266\3145\314\265"
        "\3155\315\265\3166\316\266\3177\317\270\3209\320\272\321<\321\276\322"
        "?\322\301\323D\323\306\324I\324\313\325N\325\321\326U\326\330\327\\"
        "\327\340\330d\330\350\331l\331\361\332v\332\373\333\200\334\005\334"
        "\212\335\020\335\226\336\034\336\242\337)\337\257\3406\340\275\341"
        "D\341\314\342S\342\333\343c\343\353\344s\344\374\345\204\346\r\346"
        "\226\347\037\347\251\3502\350\274\351F\351\320\352[\352\345\353p\353"
        "\373\354\206\355\021\355\234\356(\356\264\357@\357\314\360X\360\345"
        "\361r\361\377\362\214\363\031\363\247\3644\364\302\365P\365\336\366"
        "m\366\373\367\212\370\031\370\250\3718\371\307\372W\372\347\373w\374"
        "\007\374\230\375)\375\272\376K\376\334\377m\377\377";
    return SkData::MakeWithoutCopy(kProfile, sizeof(kProfile) - 1);
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
#endif  // SK_PDF_GENERATE_PDFA

bool SkPDFDocument::onClose(SkWStream* stream) {
    SkASSERT(!fCanvas.get());
    if (fPages.empty()) {
        fPages.reset();
        fCanon.reset();
        renew(&fObjectSerializer);
        return false;
    }
    auto docCatalog = sk_make_sp<SkPDFDict>("Catalog");
    #ifdef SK_PDF_GENERATE_PDFA
        SkASSERT(fXMP);
        docCatalog->insertObjRef("Metadata", fXMP);
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
        // Don't specify OutputIntents if we are not in PDF/A mode since
        // no one has ever asked for this feature.
        docCatalog->insertObject("OutputIntents", std::move(intentArray));
    #endif
    docCatalog->insertObjRef("Pages", generate_page_tree(&fPages));

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
    #ifdef SK_PDF_GENERATE_PDFA
        fObjectSerializer.serializeFooter(this->getStream(), docCatalog, fID);
    #else
        fObjectSerializer.serializeFooter(
                this->getStream(), docCatalog, nullptr);
    #endif
    SkASSERT(fPages.count() == 0);
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
    std::unique_ptr<SkFILEWStream> stream(new SkFILEWStream(path));
    return stream->isValid()
        ? SkPDFMakeDocument(stream.release(), delete_wstream, dpi, nullptr).release()
        : nullptr;
}
