// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPDFDocument_DEFINED
#define SkPDFDocument_DEFINED

#include "include/core/SkDocument.h"
#include "include/core/SkMilestone.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkNoncopyable.h"

#include <cstdint>
#include <memory>
#include <vector>

class SkCanvas;
class SkCodec;
class SkData;
class SkExecutor;
class SkPDFArray;
class SkPDFStructTree;
class SkPixmap;
class SkWStream;

#define SKPDF_STRING(X) SKPDF_STRING_IMPL(X)
#define SKPDF_STRING_IMPL(X) #X

namespace SkPDF {

/** Attributes for nodes in the PDF tree. */
class SK_API AttributeList : SkNoncopyable {
public:
    AttributeList();
    ~AttributeList();

    // Each attribute must have an owner (e.g. "Layout", "List", "Table", etc)
    // and an attribute name (e.g. "BBox", "RowSpan", etc.) from PDF32000_2008 14.8.5,
    // and then a value of the proper type according to the spec.
    void appendInt(const char* owner, const char* name, int value);
    void appendFloat(const char* owner, const char* name, float value);
    void appendName(const char* owner, const char* attrName, const char* value);
    void appendFloatArray(const char* owner,
                          const char* name,
                          const std::vector<float>& value);
    void appendNodeIdArray(const char* owner,
                           const char* attrName,
                           const std::vector<int>& nodeIds);

private:
    friend class ::SkPDFStructTree;

    std::unique_ptr<SkPDFArray> fAttrs;
    std::vector<int> fElemIds; // element identifiers referenced by fAttrs
};

/** A node in a PDF structure tree, giving a semantic representation
    of the content.  Each node ID is associated with content
    by passing the SkCanvas and node ID to SkPDF::SetNodeId() when drawing.
    NodeIDs should be unique within each tree.
*/
struct StructureElementNode {
    SkString fTypeString;
    std::vector<std::unique_ptr<StructureElementNode>> fChildVector;
    int fNodeId = 0;
    AttributeList fAttributes;
    SkString fAlt;
    SkString fLang;
};

struct DateTime {
    int16_t  fTimeZoneMinutes;  // The number of minutes that this
                                // is ahead of or behind UTC.
    uint16_t fYear;          //!< e.g. 2005
    uint8_t  fMonth;         //!< 1..12
    uint8_t  fDayOfWeek;     //!< 0..6, 0==Sunday
    uint8_t  fDay;           //!< 1..31
    uint8_t  fHour;          //!< 0..23
    uint8_t  fMinute;        //!< 0..59
    uint8_t  fSecond;        //!< 0..59

    void toISO8601(SkString* dst) const;
};

using DecodeJpegCallback = std::unique_ptr<SkCodec> (*)(sk_sp<SkData>);
using EncodeJpegCallback = bool (*)(SkWStream* dst, const SkPixmap& src, int quality);

/** Optional metadata to be passed into the PDF factory function.
*/
struct Metadata {
    /** The document's title.
    */
    SkString fTitle;

    /** The name of the person who created the document.
    */
    SkString fAuthor;

    /** The subject of the document.
    */
    SkString fSubject;

    /** Keywords associated with the document.  Commas may be used to delineate
        keywords within the string.
    */
    SkString fKeywords;

    /** If the document was converted to PDF from another format,
        the name of the conforming product that created the
        original document from which it was converted.
    */
    SkString fCreator;

    /** The product that is converting this document to PDF.
    */
    SkString fProducer = SkString("Skia/PDF m" SKPDF_STRING(SK_MILESTONE));

    /** The date and time the document was created.
        The zero default value represents an unknown/unset time.
    */
    DateTime fCreation = {0, 0, 0, 0, 0, 0, 0, 0};

    /** The date and time the document was most recently modified.
        The zero default value represents an unknown/unset time.
    */
    DateTime fModified = {0, 0, 0, 0, 0, 0, 0, 0};

    /** The natural language of the text in the PDF. If fLang is empty, the root
        StructureElementNode::fLang will be used (if not empty). Text not in
        this language should be marked with StructureElementNode::fLang.
    */
    SkString fLang;

    /** The DPI (pixels-per-inch) at which features without native PDF support
        will be rasterized (e.g. draw image with perspective, draw text with
        perspective, ...)  A larger DPI would create a PDF that reflects the
        original intent with better fidelity, but it can make for larger PDF
        files too, which would use more memory while rendering, and it would be
        slower to be processed or sent online or to printer.
    */
    SkScalar fRasterDPI = SK_ScalarDefaultRasterDPI;

    /** If true, include XMP metadata, a document UUID, and sRGB output intent
        information.  This adds length to the document and makes it
        non-reproducable, but are necessary features for PDF/A-2b conformance
    */
    bool fPDFA = false;

    /** Encoding quality controls the trade-off between size and quality. By
        default this is set to 101 percent, which corresponds to lossless
        encoding. If this value is set to a value <= 100, and the image is
        opaque, it will be encoded (using JPEG) with that quality setting.
    */
    int fEncodingQuality = 101;

    /** An optional tree of structured document tags that provide
        a semantic representation of the content. The caller
        should retain ownership.
    */
    StructureElementNode* fStructureElementTreeRoot = nullptr;

    enum class Outline : int {
        None = 0,
        StructureElementHeaders = 1,
        StructureElements = 2,
    } fOutline = Outline::None;

    /** Executor to handle threaded work within PDF Backend. If this is nullptr,
        then all work will be done serially on the main thread. To have worker
        threads assist with various tasks, set this to a valid SkExecutor
        instance. Currently used for executing Deflate algorithm in parallel.

        If set, the PDF output will be non-reproducible in the order and
        internal numbering of objects, but should render the same.

        Experimental.
    */
    SkExecutor* fExecutor = nullptr;

    /** PDF streams may be compressed to save space.
        Use this to specify the desired compression vs time tradeoff.
    */
    enum class CompressionLevel : int {
        Default = -1,
        None = 0,
        LowButFast = 1,
        Average = 6,
        HighButSlow = 9,
    } fCompressionLevel = CompressionLevel::Default;

    /** Preferred Subsetter. */
    enum Subsetter {
        kHarfbuzz_Subsetter,
    } fSubsetter = kHarfbuzz_Subsetter;

    /** Clients can provide a way to decode jpeg. To use Skia's JPEG decoder, pass in
        SkJpegDecoder::Decode. If not supplied, all images will need to be re-encoded
        as jpegs or deflated images before embedding. If supplied, Skia may be able to
        skip the re-encoding step.
        Skia's JPEG decoder can be used here.
    */
    SkPDF::DecodeJpegCallback jpegDecoder = nullptr;

    /** Clients can provide a way to encode jpeg. If not supplied, images will be embedded
        as a deflated image, potentially making them much larger. If clients provide
        their own implementation, JPEGs should be encoded to RGB (not YUV) otherwise they
        will have the wrong surrounding metadata provided by Skia.
        Skia's JPEG encoder can be used here.
    */
    SkPDF::EncodeJpegCallback jpegEncoder = nullptr;

    // Skia's PDF support depends on having both a jpeg encoder and decoder for writing
    // compact PDFs. It will technically work, but produce larger than optimal PDFs
    // if either the decoder or encoder are left as nullptr. If clients will be creating
    // PDFs that don't use images or otherwise want to incur this cost (with the upside
    // of not having a jpeg library), they should set this to true to avoid an internal
    // assert from firing.
    bool allowNoJpegs = false;
};

namespace NodeID {
static const constexpr int Nothing = 0;
static const constexpr int OtherArtifact = -1;
static const constexpr int PaginationArtifact = -2;
static const constexpr int PaginationHeaderArtifact = -3;
static const constexpr int PaginationFooterArtifact = -4;
static const constexpr int PaginationWatermarkArtifact = -5;
static const constexpr int LayoutArtifact = -6;
static const constexpr int PageArtifact = -7;
static const constexpr int BackgroundArtifact = -8;
}  // namespace NodeID

/** Associate a node ID with subsequent drawing commands in an
    SkCanvas.  The same node ID can appear in a StructureElementNode
    in order to associate a document's structure element tree with
    its content.

    A node ID of zero indicates no node ID. Negative node IDs are reserved.

    @param canvas  The canvas used to draw to the PDF.
    @param nodeId  The node ID for subsequent drawing commands.
*/
SK_API void SetNodeId(SkCanvas* dst, int nodeID);

/** Create a PDF-backed document, writing the results into a SkWStream.

    PDF pages are sized in point units. 1 pt == 1/72 inch == 127/360 mm.

    @param stream A PDF document will be written to this stream.  The document may write
           to the stream at anytime during its lifetime, until either close() is
           called or the document is deleted.
    @param metadata a PDFmetadata object. Some fields may be left empty.

    @returns NULL if there is an error, otherwise a newly created PDF-backed SkDocument.
*/
SK_API sk_sp<SkDocument> MakeDocument(SkWStream* stream, const Metadata& metadata);

#if !defined(SK_DISABLE_LEGACY_PDF_JPEG)
static inline sk_sp<SkDocument> MakeDocument(SkWStream* stream) {
    return MakeDocument(stream, Metadata());
}
#endif

}  // namespace SkPDF

#undef SKPDF_STRING
#undef SKPDF_STRING_IMPL
#endif  // SkPDFDocument_DEFINED
