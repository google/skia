/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDocument_DEFINED
#define SkDocument_DEFINED

#include "SkBitmap.h"
#include "SkPicture.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTime.h"

class SkCanvas;
class SkWStream;

#ifdef SK_BUILD_FOR_WIN
struct IXpsOMObjectFactory;
#endif

/** SK_ScalarDefaultDPI is 72 DPI.
*/
#define SK_ScalarDefaultRasterDPI           72.0f

/**
 *  High-level API for creating a document-based canvas. To use..
 *
 *  1. Create a document, specifying a stream to store the output.
 *  2. For each "page" of content:
 *      a. canvas = doc->beginPage(...)
 *      b. draw_my_content(canvas);
 *      c. doc->endPage();
 *  3. Close the document with doc->close().
 */
class SK_API SkDocument : public SkRefCnt {
public:
    struct OptionalTimestamp {
        SkTime::DateTime fDateTime;
        bool fEnabled;
        OptionalTimestamp() : fEnabled(false) {}
    };

    /**
     *  Optional metadata to be passed into the PDF factory function.
     */
    struct PDFMetadata {
        /**
         * The document's title.
         */
        SkString fTitle;
        /**
         * The name of the person who created the document.
         */
        SkString fAuthor;
        /**
         * The subject of the document.
         */
        SkString fSubject;
        /**
         * Keywords associated with the document.  Commas may be used
         * to delineate keywords within the string.
         */
        SkString fKeywords;
        /**
         * If the document was converted to PDF from another format,
         * the name of the conforming product that created the
         * original document from which it was converted.
         */
        SkString fCreator;
        /**
         * The product that is converting this document to PDF.
         *
         * Leave fProducer empty to get the default, correct value.
         */
        SkString fProducer;
        /**
         * The date and time the document was created.
         */
        OptionalTimestamp fCreation;
        /**
         * The date and time the document was most recently modified.
         */
        OptionalTimestamp fModified;

        /** The DPI (pixels-per-inch) at which features without
         *  native PDF support will be rasterized (e.g. draw image
         *  with perspective, draw text with perspective, ...)  A
         *  larger DPI would create a PDF that reflects the
         *  original intent with better fidelity, but it can make
         *  for larger PDF files too, which would use more memory
         *  while rendering, and it would be slower to be processed
         *  or sent online or to printer.
         */
        SkScalar fRasterDPI = SK_ScalarDefaultRasterDPI;

        /** If true, include XMP metadata, a document UUID, and sRGB output intent information.
         *  This adds length to the document and makes it non-reproducable, but are necessary
         *  features for PDF/A-2b conformance
         */
        bool fPDFA = false;

        /**
         *  Encoding quality controls the trade-off between size and quality. By default this is
         *  set to 101 percent, which corresponds to lossless encoding. If this value is set to
         *  a value <= 100, and the image is opaque, it will be encoded (using JPEG) with that
         *  quality setting.
         */
        int fEncodingQuality = 101;
    };

    /**
     *  Create a PDF-backed document, writing the results into a
     *  SkWStream.
     *
     *  PDF pages are sized in point units. 1 pt == 1/72 inch == 127/360 mm.
     *
     *  @param stream A PDF document will be written to this stream.  The document may write
     *         to the stream at anytime during its lifetime, until either close() is
     *         called or the document is deleted.
     *  @param metadata a PDFmetadata object.  Any fields may be left empty.
     *
     *  @returns NULL if there is an error, otherwise a newly created PDF-backed SkDocument.
     */
    static sk_sp<SkDocument> MakePDF(SkWStream* stream, const PDFMetadata& metadata);
    static sk_sp<SkDocument> MakePDF(SkWStream* stream);

#ifdef SK_BUILD_FOR_WIN
    /**
     *  Create a XPS-backed document, writing the results into the stream.
     *
     *  @param stream A XPS document will be written to this stream.  The
     *                document may write to the stream at anytime during its
     *                lifetime, until either close() or abort() are called or
     *                the document is deleted.
     *  @param xpsFactory A pointer to a COM XPS factory.  Must be non-null.
     *                    The document will take a ref to the factory. See
     *                    dm/DMSrcSink.cpp for an example.
     *  @param dpi The DPI (pixels-per-inch) at which features without
     *             native XPS support will be rasterized (e.g. draw image
     *             with perspective, draw text with perspective, ...)  A
     *             larger DPI would create a XPS that reflects the
     *             original intent with better fidelity, but it can make
     *             for larger XPS files too, which would use more memory
     *             while rendering, and it would be slower to be processed
     *             or sent online or to printer.
     *
     *  @returns nullptr if XPS is not supported.
     */
    static sk_sp<SkDocument> MakeXPS(SkWStream* stream,
                                     IXpsOMObjectFactory* xpsFactory,
                                     SkScalar dpi = SK_ScalarDefaultRasterDPI);
#endif

    /**
     *  Begin a new page for the document, returning the canvas that will draw
     *  into the page. The document owns this canvas, and it will go out of
     *  scope when endPage() or close() is called, or the document is deleted.
     */
    SkCanvas* beginPage(SkScalar width, SkScalar height, const SkRect* content = nullptr);

    /**
     *  Call endPage() when the content for the current page has been drawn
     *  (into the canvas returned by beginPage()). After this call the canvas
     *  returned by beginPage() will be out-of-scope.
     */
    void endPage();

    /**
     *  Call close() when all pages have been drawn. This will close the file
     *  or stream holding the document's contents. After close() the document
     *  can no longer add new pages. Deleting the document will automatically
     *  call close() if need be.
     */
    void close();

    /**
     *  Call abort() to stop producing the document immediately.
     *  The stream output must be ignored, and should not be trusted.
     */
    void abort();

protected:
    SkDocument(SkWStream*);

    // note: subclasses must call close() in their destructor, as the base class
    // cannot do this for them.
    virtual ~SkDocument();

    virtual SkCanvas* onBeginPage(SkScalar width, SkScalar height) = 0;
    virtual void onEndPage() = 0;
    virtual void onClose(SkWStream*) = 0;
    virtual void onAbort() = 0;

    // Allows subclasses to write to the stream as pages are written.
    SkWStream* getStream() { return fStream; }

    enum State {
        kBetweenPages_State,
        kInPage_State,
        kClosed_State
    };
    State getState() const { return fState; }

private:
    SkWStream* fStream;
    State      fState;

    typedef SkRefCnt INHERITED;
};

#endif
