Using Skia's PDF Backend
========================

Here is an example of using Skia's PDF backend in the recommended way:
via the SkDocument and SkCanvas APIs.

<!--?prettify lang=cc?-->

    #include "SkDocument.h"

    bool WritePDF(SkWStream* outputStream) {
        SkDocument::PDFMetadata metadata;
        metadata.fCreator  = "creator....";
        metadata.fTitle    = "title...";
        metadata.fAuthor   = "author...";
        metadata.fSubject  = "subject...";
        metadata.fKeywords = "keywords...";
        metadata.fCreator  = "creator...";
        SkTime::DateTime now = get_current_date_and_time();
        metadata.fCreation.fEnabled = true;
        metadata.fCreation.fDateTime = now;
        metadata.fModified.fEnabled = true;
        metadata.fModified.fDateTime = now;
        sk_sp<SkDocument> pdfDocument(SkDocument::MakePDF(
                outputStream, SK_ScalarDefaultRasterDPI, metadata,
                nullptr, true);
        assert(pdfDocument);

        int numberOfPages = ....;
        for (int page = 0; page < numberOfPages; ++page) {
            SkScalar pageWidth = ....;
            SkScalar pageHeight = ....;
            SkCanvas* pageCanvas =
                    pdfDocument->beginPage(pageWidth, pageHeight);

            // ....insert canvas draw commands here....

            pdfDocument->endPage();
        }
        return pdfDocument->close();
    }
