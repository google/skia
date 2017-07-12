/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFDocument.h"

#include "SkCanvas.h"
#include "SkMakeUnique.h"
#include "SkPDFCanon.h"
#include "SkPDFDevice.h"
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
    fObjNumMap.addObjectRecursively(object.get());
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
        wStream->writeDecAsText(index);
        wStream->writeText(" 0 obj\n");  // Generation number is always 0.
        object->emitObject(wStream, fObjNumMap);
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
    trailerDict.emitObject(wStream, fObjNumMap);
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
    fCanon.fPixelSerializer = std::move(jpegEncoder);
}

SkPDFDocument::~SkPDFDocument() {
    // subclasses of SkDocument must call close() in their destructors.
    this->close();
}

void SkPDFDocument::serialize(const sk_sp<SkPDFObject>& object) {
    fObjectSerializer.addObjectRecursively(object);
    fObjectSerializer.serializeObjects(this->getStream());
}

SkCanvas* SkPDFDocument::onBeginPage(SkScalar width, SkScalar height) {
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
    fPageDevice = sk_make_sp<SkPDFDevice>(pageSize, this);
    fPageDevice->setFlip();  // Only the top-level device needs to be flipped.
    fCanvas.reset(new SkCanvas(fPageDevice.get()));
    return fCanvas.get();
}

void SkPDFDocument::onEndPage() {
    SkASSERT(fCanvas.get());
    fCanvas->flush();
    fCanvas.reset(nullptr);
    SkASSERT(fPageDevice);
    auto page = sk_make_sp<SkPDFDict>("Page");
    page->insertObject("Resources", fPageDevice->makeResourceDict());
    page->insertObject("MediaBox", fPageDevice->copyMediaBox());
    auto annotations = sk_make_sp<SkPDFArray>();
    fPageDevice->appendAnnotations(annotations.get());
    if (annotations->size() > 0) {
        page->insertObject("Annots", std::move(annotations));
    }
    auto contentObject = sk_make_sp<SkPDFStream>(fPageDevice->content());
    this->serialize(contentObject);
    page->insertObjRef("Contents", std::move(contentObject));
    fPageDevice->appendDestinations(fDests.get(), page.get());
    fPages.emplace_back(std::move(page));
    fPageDevice.reset(nullptr);
}

void SkPDFDocument::onAbort() {
    this->reset();
}

void SkPDFDocument::reset() {
    fCanvas.reset(nullptr);
    fPages.reset();
    renew(&fCanon);
    renew(&fObjectSerializer);
    fFonts.reset();
}

static sk_sp<SkData> SkSrgbIcm() {
    // Source: http://www.argyllcms.com/icclibsrc.html
    static const char kProfile[] =
        "\0\0\14\214argl\2 \0\0mntrRGB XYZ \7\336\0\1\0\6\0\26\0\17\0:acspM"
        "SFT\0\0\0\0IEC sRGB\0\0\0\0\0\0\0\0\0\0\0\0\0\0\366\326\0\1\0\0\0\0"
        "\323-argl\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\21desc\0\0\1P\0\0\0\231cprt\0"
        "\0\1\354\0\0\0gdmnd\0\0\2T\0\0\0pdmdd\0\0\2\304\0\0\0\210tech\0\0\3"
        "L\0\0\0\14vued\0\0\3X\0\0\0gview\0\0\3\300\0\0\0$lumi\0\0\3\344\0\0"
        "\0\24meas\0\0\3\370\0\0\0$wtpt\0\0\4\34\0\0\0\24bkpt\0\0\0040\0\0\0"
        "\24rXYZ\0\0\4D\0\0\0\24gXYZ\0\0\4X\0\0\0\24bXYZ\0\0\4l\0\0\0\24rTR"
        "C\0\0\4\200\0\0\10\14gTRC\0\0\4\200\0\0\10\14bTRC\0\0\4\200\0\0\10"
        "\14desc\0\0\0\0\0\0\0?sRGB IEC61966-2.1 (Equivalent to www.srgb.co"
        "m 1998 HP profile)\0\0\0\0\0\0\0\0\0\0\0?sRGB IEC61966-2.1 (Equiva"
        "lent to www.srgb.com 1998 HP profile)\0\0\0\0\0\0\0\0text\0\0\0\0C"
        "reated by Graeme W. Gill. Released into the public domain. No Warr"
        "anty, Use at your own risk.\0\0desc\0\0\0\0\0\0\0\26IEC http://www"
        ".iec.ch\0\0\0\0\0\0\0\0\0\0\0\26IEC http://www.iec.ch\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0desc\0\0\0\0\0\0\0.IEC 61966-2.1 Default RGB colour sp"
        "ace - sRGB\0\0\0\0\0\0\0\0\0\0\0.IEC 61966-2.1 Default RGB colour "
        "space - sRGB\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0sig \0\0\0"
        "\0CRT desc\0\0\0\0\0\0\0\rIEC61966-2.1\0\0\0\0\0\0\0\0\0\0\0\rIEC6"
        "1966-2.1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0view\0\0\0\0"
        "\0\23\244|\0\24_0\0\20\316\2\0\3\355\262\0\4\23\n\0\3\\g\0\0\0\1XY"
        "Z \0\0\0\0\0L\n=\0P\0\0\0W\36\270meas\0\0\0\0\0\0\0\1\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\2\217\0\0\0\2XYZ \0\0\0\0\0\0\363Q\0\1\0\0\0"
        "\1\26\314XYZ \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0XYZ \0\0\0\0\0\0o\240"
        "\0\0008\365\0\0\3\220XYZ \0\0\0\0\0\0b\227\0\0\267\207\0\0\30\331X"
        "YZ \0\0\0\0\0\0$\237\0\0\17\204\0\0\266\304curv\0\0\0\0\0\0\4\0\0\0"
        "\0\5\0\n\0\17\0\24\0\31\0\36\0#\0(\0-\0002\0007\0;\0@\0E\0J\0O\0T\0"
        "Y\0^\0c\0h\0m\0r\0w\0|\0\201\0\206\0\213\0\220\0\225\0\232\0\237\0"
        "\244\0\251\0\256\0\262\0\267\0\274\0\301\0\306\0\313\0\320\0\325\0"
        "\333\0\340\0\345\0\353\0\360\0\366\0\373\1\1\1\7\1\r\1\23\1\31\1\37"
        "\1%\1+\0012\0018\1>\1E\1L\1R\1Y\1`\1g\1n\1u\1|\1\203\1\213\1\222\1"
        "\232\1\241\1\251\1\261\1\271\1\301\1\311\1\321\1\331\1\341\1\351\1"
        "\362\1\372\2\3\2\14\2\24\2\35\2&\2/\0028\2A\2K\2T\2]\2g\2q\2z\2\204"
        "\2\216\2\230\2\242\2\254\2\266\2\301\2\313\2\325\2\340\2\353\2\365"
        "\3\0\3\13\3\26\3!\3-\0038\3C\3O\3Z\3f\3r\3~\3\212\3\226\3\242\3\256"
        "\3\272\3\307\3\323\3\340\3\354\3\371\4\6\4\23\4 \4-\4;\4H\4U\4c\4q"
        "\4~\4\214\4\232\4\250\4\266\4\304\4\323\4\341\4\360\4\376\5\r\5\34"
        "\5+\5:\5I\5X\5g\5w\5\206\5\226\5\246\5\265\5\305\5\325\5\345\5\366"
        "\6\6\6\26\6'\0067\6H\6Y\6j\6{\6\214\6\235\6\257\6\300\6\321\6\343\6"
        "\365\7\7\7\31\7+\7=\7O\7a\7t\7\206\7\231\7\254\7\277\7\322\7\345\7"
        "\370\10\13\10\37\0102\10F\10Z\10n\10\202\10\226\10\252\10\276\10\322"
        "\10\347\10\373\t\20\t%\t:\tO\td\ty\t\217\t\244\t\272\t\317\t\345\t"
        "\373\n\21\n'\n=\nT\nj\n\201\n\230\n\256\n\305\n\334\n\363\13\13\13"
        "\"\0139\13Q\13i\13\200\13\230\13\260\13\310\13\341\13\371\14\22\14"
        "*\14C\14\\\14u\14\216\14\247\14\300\14\331\14\363\r\r\r&\r@\rZ\rt\r"
        "\216\r\251\r\303\r\336\r\370\16\23\16.\16I\16d\16\177\16\233\16\266"
        "\16\322\16\356\17\t\17%\17A\17^\17z\17\226\17\263\17\317\17\354\20"
        "\t\20&\20C\20a\20~\20\233\20\271\20\327\20\365\21\23\0211\21O\21m\21"
        "\214\21\252\21\311\21\350\22\7\22&\22E\22d\22\204\22\243\22\303\22"
        "\343\23\3\23#\23C\23c\23\203\23\244\23\305\23\345\24\6\24'\24I\24j"
        "\24\213\24\255\24\316\24\360\25\22\0254\25V\25x\25\233\25\275\25\340"
        "\26\3\26&\26I\26l\26\217\26\262\26\326\26\372\27\35\27A\27e\27\211"
        "\27\256\27\322\27\367\30\33\30@\30e\30\212\30\257\30\325\30\372\31"
        " \31E\31k\31\221\31\267\31\335\32\4\32*\32Q\32w\32\236\32\305\32\354"
        "\33\24\33;\33c\33\212\33\262\33\332\34\2\34*\34R\34{\34\243\34\314"
        "\34\365\35\36\35G\35p\35\231\35\303\35\354\36\26\36@\36j\36\224\36"
        "\276\36\351\37\23\37>\37i\37\224\37\277\37\352 \25 A l \230 \304 \360"
        "!\34!H!u!\241!\316!\373\"'\"U\"\202\"\257\"\335#\n#8#f#\224#\302#\360"
        "$\37$M$|$\253$\332%\t%8%h%\227%\307%\367&'&W&\207&\267&\350'\30'I'"
        "z'\253'\334(\r(?(q(\242(\324)\6)8)k)\235)\320*\2*5*h*\233*\317+\2+"
        "6+i+\235+\321,\5,9,n,\242,\327-\14-A-v-\253-\341.\26.L.\202.\267.\356"
        "/$/Z/\221/\307/\376050l0\2440\3331\0221J1\2021\2721\3622*2c2\2332\324"
        "3\r3F3\1773\2703\3614+4e4\2364\3305\0235M5\2075\3025\375676r6\2566"
        "\3517$7`7\2347\3278\0248P8\2148\3109\0059B9\1779\2749\371:6:t:\262"
        ":\357;-;k;\252;\350<'<e<\244<\343=\"=a=\241=\340> >`>\240>\340?!?a"
        "?\242?\342@#@d@\246@\347A)AjA\254A\356B0BrB\265B\367C:C}C\300D\3DG"
        "D\212D\316E\22EUE\232E\336F\"FgF\253F\360G5G{G\300H\5HKH\221H\327I"
        "\35IcI\251I\360J7J}J\304K\14KSK\232K\342L*LrL\272M\2MJM\223M\334N%"
        "NnN\267O\0OIO\223O\335P'PqP\273Q\6QPQ\233Q\346R1R|R\307S\23S_S\252"
        "S\366TBT\217T\333U(UuU\302V\17V\\V\251V\367WDW\222W\340X/X}X\313Y\32"
        "YiY\270Z\7ZVZ\246Z\365[E[\225[\345\\5\\\206\\\326]']x]\311^\32^l^\275"
        "_\17_a_\263`\5`W`\252`\374aOa\242a\365bIb\234b\360cCc\227c\353d@d\224"
        "d\351e=e\222e\347f=f\222f\350g=g\223g\351h?h\226h\354iCi\232i\361j"
        "Hj\237j\367kOk\247k\377lWl\257m\10m`m\271n\22nkn\304o\36oxo\321p+p"
        "\206p\340q:q\225q\360rKr\246s\1s]s\270t\24tpt\314u(u\205u\341v>v\233"
        "v\370wVw\263x\21xnx\314y*y\211y\347zFz\245{\4{c{\302|!|\201|\341}A"
        "}\241~\1~b~\302\177#\177\204\177\345\200G\200\250\201\n\201k\201\315"
        "\2020\202\222\202\364\203W\203\272\204\35\204\200\204\343\205G\205"
        "\253\206\16\206r\206\327\207;\207\237\210\4\210i\210\316\2113\211\231"
        "\211\376\212d\212\312\2130\213\226\213\374\214c\214\312\2151\215\230"
        "\215\377\216f\216\316\2176\217\236\220\6\220n\220\326\221?\221\250"
        "\222\21\222z\222\343\223M\223\266\224 \224\212\224\364\225_\225\311"
        "\2264\226\237\227\n\227u\227\340\230L\230\270\231$\231\220\231\374"
        "\232h\232\325\233B\233\257\234\34\234\211\234\367\235d\235\322\236"
        "@\236\256\237\35\237\213\237\372\240i\240\330\241G\241\266\242&\242"
        "\226\243\6\243v\243\346\244V\244\307\2458\245\251\246\32\246\213\246"
        "\375\247n\247\340\250R\250\304\2517\251\251\252\34\252\217\253\2\253"
        "u\253\351\254\\\254\320\255D\255\270\256-\256\241\257\26\257\213\260"
        "\0\260u\260\352\261`\261\326\262K\262\302\2638\263\256\264%\264\234"
        "\265\23\265\212\266\1\266y\266\360\267h\267\340\270Y\270\321\271J\271"
        "\302\272;\272\265\273.\273\247\274!\274\233\275\25\275\217\276\n\276"
        "\204\276\377\277z\277\365\300p\300\354\301g\301\343\302_\302\333\303"
        "X\303\324\304Q\304\316\305K\305\310\306F\306\303\307A\307\277\310="
        "\310\274\311:\311\271\3128\312\267\3136\313\266\3145\314\265\3155\315"
        "\265\3166\316\266\3177\317\270\3209\320\272\321<\321\276\322?\322\301"
        "\323D\323\306\324I\324\313\325N\325\321\326U\326\330\327\\\327\340"
        "\330d\330\350\331l\331\361\332v\332\373\333\200\334\5\334\212\335\20"
        "\335\226\336\34\336\242\337)\337\257\3406\340\275\341D\341\314\342"
        "S\342\333\343c\343\353\344s\344\374\345\204\346\r\346\226\347\37\347"
        "\251\3502\350\274\351F\351\320\352[\352\345\353p\353\373\354\206\355"
        "\21\355\234\356(\356\264\357@\357\314\360X\360\345\361r\361\377\362"
        "\214\363\31\363\247\3644\364\302\365P\365\336\366m\366\373\367\212"
        "\370\31\370\250\3718\371\307\372W\372\347\373w\374\7\374\230\375)\375"
        "\272\376K\376\334\377m\377\377";
    const size_t kProfileLength = 3212;
    static_assert(kProfileLength == sizeof(kProfile) - 1, "");
    return SkData::MakeWithoutCopy(kProfile, kProfileLength);
}

static sk_sp<SkPDFStream> make_srgb_color_profile() {
    sk_sp<SkPDFStream> stream = sk_make_sp<SkPDFStream>(SkSrgbIcm());
    stream->dict()->insertInt("N", 3);
    sk_sp<SkPDFArray> array = sk_make_sp<SkPDFArray>();
    array->appendScalar(0.0f);
    array->appendScalar(1.0f);
    array->appendScalar(0.0f);
    array->appendScalar(1.0f);
    array->appendScalar(0.0f);
    array->appendScalar(1.0f);
    stream->dict()->insertObject("Range", std::move(array));
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

void SkPDFDocument::onClose(SkWStream* stream) {
    SkASSERT(!fCanvas.get());
    if (fPages.empty()) {
        this->reset();
        return;
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
    SkPDFCanon* canon = &fCanon;
    fFonts.foreach([canon](SkPDFFont* p){ p->getFontSubset(canon); });
    fObjectSerializer.addObjectRecursively(docCatalog);
    fObjectSerializer.serializeObjects(this->getStream());
    fObjectSerializer.serializeFooter(this->getStream(), docCatalog, fID);
    this->reset();
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkDocument> SkPDFMakeDocument(SkWStream* stream,
                                    void (*proc)(SkWStream*, bool),
                                    SkScalar dpi,
                                    const SkDocument::PDFMetadata& metadata,
                                    sk_sp<SkPixelSerializer> jpeg,
                                    bool pdfa) {
    if (dpi <= 0) {
        dpi = 72.0f;
    }
    return stream ? sk_make_sp<SkPDFDocument>(stream, proc, dpi, metadata,
                                              std::move(jpeg), pdfa)
                  : nullptr;
}

sk_sp<SkDocument> SkDocument::MakePDF(const char path[], SkScalar dpi) {
    auto delete_wstream = [](SkWStream* stream, bool) { delete stream; };
    auto stream = skstd::make_unique<SkFILEWStream>(path);
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
