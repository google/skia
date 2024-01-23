/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/docs/SkPDFDocument.h"
#include "src/pdf/SkPDFDocumentPriv.h"

#include "include/core/SkStream.h"
#include "include/docs/SkPDFDocument.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkUTF.h"
#include "src/pdf/SkPDFDevice.h"
#include "src/pdf/SkPDFFont.h"
#include "src/pdf/SkPDFGradientShader.h"
#include "src/pdf/SkPDFGraphicState.h"
#include "src/pdf/SkPDFShader.h"
#include "src/pdf/SkPDFTag.h"
#include "src/pdf/SkPDFUtils.h"

#include <utility>

// For use in SkCanvas::drawAnnotation
const char* SkPDFGetNodeIdKey() {
    static constexpr char key[] = "PDF_Node_Key";
    return key;
}

static SkString ToValidUtf8String(const SkData& d) {
    if (d.size() == 0) {
        SkDEBUGFAIL("Not a valid string, data length is zero.");
        return SkString();
    }

    const char* c_str = static_cast<const char*>(d.data());
    if (c_str[d.size() - 1] != 0) {
        SkDEBUGFAIL("Not a valid string, not null-terminated.");
        return SkString();
    }

    // CountUTF8 returns -1 if there's an invalid UTF-8 byte sequence.
    int valid_utf8_chars_count = SkUTF::CountUTF8(c_str, d.size() - 1);
    if (valid_utf8_chars_count == -1) {
        SkDEBUGFAIL("Not a valid UTF-8 string.");
        return SkString();
    }

    return SkString(c_str, d.size() - 1);
}

////////////////////////////////////////////////////////////////////////////////

void SkPDFOffsetMap::markStartOfDocument(const SkWStream* s) { fBaseOffset = s->bytesWritten(); }

static size_t difference(size_t minuend, size_t subtrahend) {
    return SkASSERT(minuend >= subtrahend), minuend - subtrahend;
}

void SkPDFOffsetMap::markStartOfObject(int referenceNumber, const SkWStream* s) {
    SkASSERT(referenceNumber > 0);
    size_t index = SkToSizeT(referenceNumber - 1);
    if (index >= fOffsets.size()) {
        fOffsets.resize(index + 1);
    }
    fOffsets[index] = SkToInt(difference(s->bytesWritten(), fBaseOffset));
}

int SkPDFOffsetMap::objectCount() const {
    return SkToInt(fOffsets.size() + 1); // Include the special zeroth object in the count.
}

int SkPDFOffsetMap::emitCrossReferenceTable(SkWStream* s) const {
    int xRefFileOffset = SkToInt(difference(s->bytesWritten(), fBaseOffset));
    s->writeText("xref\n0 ");
    s->writeDecAsText(this->objectCount());
    s->writeText("\n0000000000 65535 f \n");
    for (int offset : fOffsets) {
        SkASSERT(offset > 0);  // Offset was set.
        s->writeBigDecAsText(offset, 10);
        s->writeText(" 00000 n \n");
    }
    return xRefFileOffset;
}
//
////////////////////////////////////////////////////////////////////////////////

#define SKPDF_MAGIC "\xD3\xEB\xE9\xE1"
#ifndef SK_BUILD_FOR_WIN
static_assert((SKPDF_MAGIC[0] & 0x7F) == "Skia"[0], "");
static_assert((SKPDF_MAGIC[1] & 0x7F) == "Skia"[1], "");
static_assert((SKPDF_MAGIC[2] & 0x7F) == "Skia"[2], "");
static_assert((SKPDF_MAGIC[3] & 0x7F) == "Skia"[3], "");
#endif
static void serializeHeader(SkPDFOffsetMap* offsetMap, SkWStream* wStream) {
    offsetMap->markStartOfDocument(wStream);
    wStream->writeText("%PDF-1.4\n%" SKPDF_MAGIC "\n");
    // The PDF spec recommends including a comment with four
    // bytes, all with their high bits set.  "\xD3\xEB\xE9\xE1" is
    // "Skia" with the high bits set.
}
#undef SKPDF_MAGIC

static void begin_indirect_object(SkPDFOffsetMap* offsetMap,
                                  SkPDFIndirectReference ref,
                                  SkWStream* s) {
    offsetMap->markStartOfObject(ref.fValue, s);
    s->writeDecAsText(ref.fValue);
    s->writeText(" 0 obj\n");  // Generation number is always 0.
}

static void end_indirect_object(SkWStream* s) { s->writeText("\nendobj\n"); }

// Xref table and footer
static void serialize_footer(const SkPDFOffsetMap& offsetMap,
                             SkWStream* wStream,
                             SkPDFIndirectReference infoDict,
                             SkPDFIndirectReference docCatalog,
                             SkUUID uuid) {
    int xRefFileOffset = offsetMap.emitCrossReferenceTable(wStream);
    SkPDFDict trailerDict;
    trailerDict.insertInt("Size", offsetMap.objectCount());
    SkASSERT(docCatalog != SkPDFIndirectReference());
    trailerDict.insertRef("Root", docCatalog);
    SkASSERT(infoDict != SkPDFIndirectReference());
    trailerDict.insertRef("Info", infoDict);
    if (SkUUID() != uuid) {
        trailerDict.insertObject("ID", SkPDFMetadata::MakePdfId(uuid, uuid));
    }
    wStream->writeText("trailer\n");
    trailerDict.emitObject(wStream);
    wStream->writeText("\nstartxref\n");
    wStream->writeBigDecAsText(xRefFileOffset);
    wStream->writeText("\n%%EOF\n");
}

static SkPDFIndirectReference generate_page_tree(
        SkPDFDocument* doc,
        std::vector<std::unique_ptr<SkPDFDict>> pages,
        const std::vector<SkPDFIndirectReference>& pageRefs) {
    // PDF wants a tree describing all the pages in the document.  We arbitrary
    // choose 8 (kNodeSize) as the number of allowed children.  The internal
    // nodes have type "Pages" with an array of children, a parent pointer, and
    // the number of leaves below the node as "Count."  The leaves are passed
    // into the method, have type "Page" and need a parent pointer. This method
    // builds the tree bottom up, skipping internal nodes that would have only
    // one child.
    SkASSERT(pages.size() > 0);
    struct PageTreeNode {
        std::unique_ptr<SkPDFDict> fNode;
        SkPDFIndirectReference fReservedRef;
        int fPageObjectDescendantCount;

        static std::vector<PageTreeNode> Layer(std::vector<PageTreeNode> vec, SkPDFDocument* doc) {
            std::vector<PageTreeNode> result;
            static constexpr size_t kMaxNodeSize = 8;
            const size_t n = vec.size();
            SkASSERT(n >= 1);
            const size_t result_len = (n - 1) / kMaxNodeSize + 1;
            SkASSERT(result_len >= 1);
            SkASSERT(n == 1 || result_len < n);
            result.reserve(result_len);
            size_t index = 0;
            for (size_t i = 0; i < result_len; ++i) {
                if (n != 1 && index + 1 == n) {  // No need to create a new node.
                    result.push_back(std::move(vec[index++]));
                    continue;
                }
                SkPDFIndirectReference parent = doc->reserveRef();
                auto kids_list = SkPDFMakeArray();
                int descendantCount = 0;
                for (size_t j = 0; j < kMaxNodeSize && index < n; ++j) {
                    PageTreeNode& node = vec[index++];
                    node.fNode->insertRef("Parent", parent);
                    kids_list->appendRef(doc->emit(*node.fNode, node.fReservedRef));
                    descendantCount += node.fPageObjectDescendantCount;
                }
                auto next = SkPDFMakeDict("Pages");
                next->insertInt("Count", descendantCount);
                next->insertObject("Kids", std::move(kids_list));
                result.push_back(PageTreeNode{std::move(next), parent, descendantCount});
            }
            return result;
        }
    };
    std::vector<PageTreeNode> currentLayer;
    currentLayer.reserve(pages.size());
    SkASSERT(pages.size() == pageRefs.size());
    for (size_t i = 0; i < pages.size(); ++i) {
        currentLayer.push_back(PageTreeNode{std::move(pages[i]), pageRefs[i], 1});
    }
    currentLayer = PageTreeNode::Layer(std::move(currentLayer), doc);
    while (currentLayer.size() > 1) {
        currentLayer = PageTreeNode::Layer(std::move(currentLayer), doc);
    }
    SkASSERT(currentLayer.size() == 1);
    const PageTreeNode& root = currentLayer[0];
    return doc->emit(*root.fNode, root.fReservedRef);
}

template<typename T, typename... Args>
static void reset_object(T* dst, Args&&... args) {
    dst->~T();
    new (dst) T(std::forward<Args>(args)...);
}

////////////////////////////////////////////////////////////////////////////////

SkPDFDocument::SkPDFDocument(SkWStream* stream,
                             SkPDF::Metadata metadata)
    : SkDocument(stream)
    , fMetadata(std::move(metadata)) {
    constexpr float kDpiForRasterScaleOne = 72.0f;
    if (fMetadata.fRasterDPI != kDpiForRasterScaleOne) {
        fInverseRasterScale = kDpiForRasterScaleOne / fMetadata.fRasterDPI;
        fRasterScale        = fMetadata.fRasterDPI / kDpiForRasterScaleOne;
    }
    if (fMetadata.fStructureElementTreeRoot) {
        fTagTree.init(fMetadata.fStructureElementTreeRoot, fMetadata.fOutline);
    }
    fExecutor = fMetadata.fExecutor;
}

SkPDFDocument::~SkPDFDocument() {
    // subclasses of SkDocument must call close() in their destructors.
    this->close();
}

SkPDFIndirectReference SkPDFDocument::emit(const SkPDFObject& object, SkPDFIndirectReference ref){
    SkAutoMutexExclusive lock(fMutex);
    object.emitObject(this->beginObject(ref));
    this->endObject();
    return ref;
}

SkWStream* SkPDFDocument::beginObject(SkPDFIndirectReference ref) SK_REQUIRES(fMutex) {
    begin_indirect_object(&fOffsetMap, ref, this->getStream());
    return this->getStream();
}

void SkPDFDocument::endObject() SK_REQUIRES(fMutex) {
    end_indirect_object(this->getStream());
}

static SkSize operator*(SkISize u, SkScalar s) { return SkSize{u.width() * s, u.height() * s}; }
static SkSize operator*(SkSize u, SkScalar s) { return SkSize{u.width() * s, u.height() * s}; }

SkCanvas* SkPDFDocument::onBeginPage(SkScalar width, SkScalar height) {
    SkASSERT(fCanvas.imageInfo().dimensions().isZero());
    if (fPages.empty()) {
        // if this is the first page if the document.
        {
            SkAutoMutexExclusive autoMutexAcquire(fMutex);
            serializeHeader(&fOffsetMap, this->getStream());

        }

        fInfoDict = this->emit(*SkPDFMetadata::MakeDocumentInformationDict(fMetadata));
        if (fMetadata.fPDFA) {
            fUUID = SkPDFMetadata::CreateUUID(fMetadata);
            // We use the same UUID for Document ID and Instance ID since this
            // is the first revision of this document (and Skia does not
            // support revising existing PDF documents).
            // If we are not in PDF/A mode, don't use a UUID since testing
            // works best with reproducible outputs.
            fXMP = SkPDFMetadata::MakeXMPObject(fMetadata, fUUID, fUUID, this);
        }
    }
    // By scaling the page at the device level, we will create bitmap layer
    // devices at the rasterized scale, not the 72dpi scale.  Bitmap layer
    // devices are created when saveLayer is called with an ImageFilter;  see
    // SkPDFDevice::onCreateDevice().
    SkISize pageSize = (SkSize{width, height} * fRasterScale).toRound();
    SkMatrix initialTransform;
    // Skia uses the top left as the origin but PDF natively has the origin at the
    // bottom left. This matrix corrects for that, as well as the raster scale.
    initialTransform.setScaleTranslate(fInverseRasterScale, -fInverseRasterScale,
                                       0, fInverseRasterScale * pageSize.height());
    fPageDevice = sk_make_sp<SkPDFDevice>(pageSize, this, initialTransform);
    reset_object(&fCanvas, fPageDevice);
    fCanvas.scale(fRasterScale, fRasterScale);
    fPageRefs.push_back(this->reserveRef());
    return &fCanvas;
}

static void populate_link_annotation(SkPDFDict* annotation, const SkRect& r) {
    annotation->insertName("Subtype", "Link");
    annotation->insertInt("F", 4);  // required by ISO 19005
    // Border: 0 = Horizontal corner radius.
    //         0 = Vertical corner radius.
    //         0 = Width, 0 = no border.
    annotation->insertObject("Border", SkPDFMakeArray(0, 0, 0));
    annotation->insertObject("Rect", SkPDFMakeArray(r.fLeft, r.fTop, r.fRight, r.fBottom));
}

static SkPDFIndirectReference append_destinations(
        SkPDFDocument* doc,
        const std::vector<SkPDFNamedDestination>& namedDestinations)
{
    SkPDFDict destinations;
    for (const SkPDFNamedDestination& dest : namedDestinations) {
        auto pdfDest = SkPDFMakeArray();
        pdfDest->reserve(5);
        pdfDest->appendRef(dest.fPage);
        pdfDest->appendName("XYZ");
        pdfDest->appendScalar(dest.fPoint.x());
        pdfDest->appendScalar(dest.fPoint.y());
        pdfDest->appendInt(0);  // Leave zoom unchanged
        destinations.insertObject(ToValidUtf8String(*dest.fName), std::move(pdfDest));
    }
    return doc->emit(destinations);
}

std::unique_ptr<SkPDFArray> SkPDFDocument::getAnnotations() {
    std::unique_ptr<SkPDFArray> array;
    size_t count = fCurrentPageLinks.size();
    if (0 == count) {
        return array;  // is nullptr
    }
    array = SkPDFMakeArray();
    array->reserve(count);
    for (const auto& link : fCurrentPageLinks) {
        SkPDFDict annotation("Annot");
        populate_link_annotation(&annotation, link->fRect);
        if (link->fType == SkPDFLink::Type::kUrl) {
            std::unique_ptr<SkPDFDict> action = SkPDFMakeDict("Action");
            action->insertName("S", "URI");
            // This is documented to be a 7 bit ASCII (byte) string.
            action->insertByteString("URI", ToValidUtf8String(*link->fData));
            annotation.insertObject("A", std::move(action));
        } else if (link->fType == SkPDFLink::Type::kNamedDestination) {
            annotation.insertName("Dest", ToValidUtf8String(*link->fData));
        } else {
            SkDEBUGFAIL("Unknown link type.");
        }

        if (link->fNodeId) {
            int structParentKey = createStructParentKeyForNodeId(link->fNodeId);
            if (structParentKey != -1) {
                annotation.insertInt("StructParent", structParentKey);
            }
        }

        SkPDFIndirectReference annotationRef = emit(annotation);
        array->appendRef(annotationRef);
        if (link->fNodeId) {
            fTagTree.addNodeAnnotation(link->fNodeId, annotationRef, SkToUInt(this->currentPageIndex()));
        }
    }
    return array;
}

void SkPDFDocument::onEndPage() {
    SkASSERT(!fCanvas.imageInfo().dimensions().isZero());
    reset_object(&fCanvas);
    SkASSERT(fPageDevice);

    auto page = SkPDFMakeDict("Page");

    SkSize mediaSize = fPageDevice->imageInfo().dimensions() * fInverseRasterScale;
    std::unique_ptr<SkStreamAsset> pageContent = fPageDevice->content();
    auto resourceDict = fPageDevice->makeResourceDict();
    SkASSERT(fPageRefs.size() > 0);
    fPageDevice = nullptr;

    page->insertObject("Resources", std::move(resourceDict));
    page->insertObject("MediaBox", SkPDFUtils::RectToArray(SkRect::MakeSize(mediaSize)));

    if (std::unique_ptr<SkPDFArray> annotations = getAnnotations()) {
        page->insertObject("Annots", std::move(annotations));
        fCurrentPageLinks.clear();
    }

    page->insertRef("Contents", SkPDFStreamOut(nullptr, std::move(pageContent), this));
    // The StructParents unique identifier for each page is just its
    // 0-based page index.
    page->insertInt("StructParents", SkToInt(this->currentPageIndex()));
    fPages.emplace_back(std::move(page));
}

void SkPDFDocument::onAbort() {
    this->waitForJobs();
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

static SkPDFIndirectReference make_srgb_color_profile(SkPDFDocument* doc) {
    std::unique_ptr<SkPDFDict> dict = SkPDFMakeDict();
    dict->insertInt("N", 3);
    dict->insertObject("Range", SkPDFMakeArray(0, 1, 0, 1, 0, 1));
    return SkPDFStreamOut(std::move(dict), SkMemoryStream::Make(SkSrgbIcm()),
                          doc, SkPDFSteamCompressionEnabled::Yes);
}

static std::unique_ptr<SkPDFArray> make_srgb_output_intents(SkPDFDocument* doc) {
    // sRGB is specified by HTML, CSS, and SVG.
    auto outputIntent = SkPDFMakeDict("OutputIntent");
    outputIntent->insertName("S", "GTS_PDFA1");
    outputIntent->insertTextString("RegistryName", "http://www.color.org");
    outputIntent->insertTextString("OutputConditionIdentifier", "Custom");
    outputIntent->insertTextString("Info", "sRGB IEC61966-2.1");
    outputIntent->insertRef("DestOutputProfile", make_srgb_color_profile(doc));
    auto intentArray = SkPDFMakeArray();
    intentArray->appendObject(std::move(outputIntent));
    return intentArray;
}

SkPDFIndirectReference SkPDFDocument::getPage(size_t pageIndex) const {
    SkASSERT(pageIndex < fPageRefs.size());
    return fPageRefs[pageIndex];
}

const SkMatrix& SkPDFDocument::currentPageTransform() const {
    return fPageDevice->initialTransform();
}

SkPDFTagTree::Mark SkPDFDocument::createMarkIdForNodeId(int nodeId, SkPoint p) {
    return fTagTree.createMarkIdForNodeId(nodeId, SkToUInt(this->currentPageIndex()), p);
}

void SkPDFDocument::addNodeTitle(int nodeId, SkSpan<const char> title) {
    fTagTree.addNodeTitle(nodeId, std::move(title));
}

int SkPDFDocument::createStructParentKeyForNodeId(int nodeId) {
    return fTagTree.createStructParentKeyForNodeId(nodeId, SkToUInt(this->currentPageIndex()));
}

static std::vector<const SkPDFFont*> get_fonts(const SkPDFDocument& canon) {
    std::vector<const SkPDFFont*> fonts;
    fonts.reserve(canon.fFontMap.count());
    // Sort so the output PDF is reproducible.
    for (const auto& [unused, font] : canon.fFontMap) {
        fonts.push_back(&font);
    }
    std::sort(fonts.begin(), fonts.end(), [](const SkPDFFont* u, const SkPDFFont* v) {
        return u->indirectReference().fValue < v->indirectReference().fValue;
    });
    return fonts;
}

SkString SkPDFDocument::nextFontSubsetTag() {
    // PDF 32000-1:2008 Section 9.6.4 FontSubsets "The tag shall consist of six uppercase letters"
    // "followed by a plus sign" "different subsets in the same PDF file shall have different tags."
    // There are 26^6 or 308,915,776 possible values. So start in range then increment and mod.
    uint32_t thisFontSubsetTag = fNextFontSubsetTag;
    fNextFontSubsetTag = (fNextFontSubsetTag + 1u) % 308915776u;

    SkString subsetTag(7);
    char* subsetTagData = subsetTag.data();
    for (size_t i = 0; i < 6; ++i) {
        subsetTagData[i] = 'A' + (thisFontSubsetTag % 26);
        thisFontSubsetTag /= 26;
    }
    subsetTagData[6] = '+';
    return subsetTag;
}

void SkPDFDocument::onClose(SkWStream* stream) {
    SkASSERT(fCanvas.imageInfo().dimensions().isZero());
    if (fPages.empty()) {
        this->waitForJobs();
        return;
    }
    auto docCatalog = SkPDFMakeDict("Catalog");
    if (fMetadata.fPDFA) {
        SkASSERT(fXMP != SkPDFIndirectReference());
        docCatalog->insertRef("Metadata", fXMP);
        // Don't specify OutputIntents if we are not in PDF/A mode since
        // no one has ever asked for this feature.
        docCatalog->insertObject("OutputIntents", make_srgb_output_intents(this));
    }

    docCatalog->insertRef("Pages", generate_page_tree(this, std::move(fPages), fPageRefs));

    if (!fNamedDestinations.empty()) {
        docCatalog->insertRef("Dests", append_destinations(this, fNamedDestinations));
        fNamedDestinations.clear();
    }

    // Handle tagged PDFs.
    if (SkPDFIndirectReference root = fTagTree.makeStructTreeRoot(this)) {
        // In the document catalog, indicate that this PDF is tagged.
        auto markInfo = SkPDFMakeDict("MarkInfo");
        markInfo->insertBool("Marked", true);
        docCatalog->insertObject("MarkInfo", std::move(markInfo));
        docCatalog->insertRef("StructTreeRoot", root);

        if (SkPDFIndirectReference outline = fTagTree.makeOutline(this)) {
            docCatalog->insertRef("Outlines", outline);
        }
    }

    // If ViewerPreferences DisplayDocTitle isn't set to true, accessibility checks will fail.
    if (!fMetadata.fTitle.isEmpty()) {
        auto viewerPrefs = SkPDFMakeDict("ViewerPreferences");
        viewerPrefs->insertBool("DisplayDocTitle", true);
        docCatalog->insertObject("ViewerPreferences", std::move(viewerPrefs));
    }

    SkString lang = fMetadata.fLang;
    if (lang.isEmpty()) {
        lang = fTagTree.getRootLanguage();
    }
    if (!lang.isEmpty()) {
        docCatalog->insertTextString("Lang", lang);
    }

    auto docCatalogRef = this->emit(*docCatalog);

    for (const SkPDFFont* f : get_fonts(*this)) {
        f->emitSubset(this);
    }

    this->waitForJobs();
    {
        SkAutoMutexExclusive autoMutexAcquire(fMutex);
        serialize_footer(fOffsetMap, this->getStream(), fInfoDict, docCatalogRef, fUUID);
    }
}

void SkPDFDocument::incrementJobCount() { fJobCount++; }

void SkPDFDocument::signalJobComplete() { fSemaphore.signal(); }

void SkPDFDocument::waitForJobs() {
     // fJobCount can increase while we wait.
     while (fJobCount > 0) {
         fSemaphore.wait();
         --fJobCount;
     }
}

///////////////////////////////////////////////////////////////////////////////

void SkPDF::SetNodeId(SkCanvas* canvas, int nodeID) {
    sk_sp<SkData> payload = SkData::MakeWithCopy(&nodeID, sizeof(nodeID));
    const char* key = SkPDFGetNodeIdKey();
    canvas->drawAnnotation({0, 0, 0, 0}, key, payload.get());
}

sk_sp<SkDocument> SkPDF::MakeDocument(SkWStream* stream, const SkPDF::Metadata& metadata) {
    SkPDF::Metadata meta = metadata;
    if (meta.fRasterDPI <= 0) {
        meta.fRasterDPI = 72.0f;
    }
    if (meta.fEncodingQuality < 0) {
        meta.fEncodingQuality = 0;
    }
    return stream ? sk_make_sp<SkPDFDocument>(stream, std::move(meta)) : nullptr;
}

///////////////////////////////////////////////////////////////////////////////
void SkPDF::DateTime::toISO8601(SkString* dst) const {
    if (dst) {
        int timeZoneMinutes = SkToInt(fTimeZoneMinutes);
        char timezoneSign = timeZoneMinutes >= 0 ? '+' : '-';
        int timeZoneHours = SkTAbs(timeZoneMinutes) / 60;
        timeZoneMinutes = SkTAbs(timeZoneMinutes) % 60;
        dst->printf("%04u-%02u-%02uT%02u:%02u:%02u%c%02d:%02d",
                    static_cast<unsigned>(fYear), static_cast<unsigned>(fMonth),
                    static_cast<unsigned>(fDay), static_cast<unsigned>(fHour),
                    static_cast<unsigned>(fMinute),
                    static_cast<unsigned>(fSecond), timezoneSign, timeZoneHours,
                    timeZoneMinutes);
    }
}
