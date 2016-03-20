Using Skia's PDF Backend
========================

Here is an example of using Skia's PDF backend in the recommended way:
via the SkDocument and SkCanvas APIs.

<!--?prettify lang=cc?-->

    #include "SkDocument.h"

    bool WritePDF(SkWStream* outputStream) {
        sk_sp<SkDocument> pdfDocument(SkDocument::CreatePDF(outputStream));
        typedef SkDocument::Attribute Attr;
        Attr info[] = {
            Attr(SkString("Title"),    SkString("....")),
            Attr(SkString("Author"),   SkString("....")),
            Attr(SkString("Subject"),  SkString("....")),
            Attr(SkString("Keywords"), SkString("....")),
            Attr(SkString("Creator"),  SkString("....")),
        };
        int infoCount = sizeof(info) / sizeof(info[0]);
        SkTime::DateTime now;
        SkTime::GetDateTime(&now);
        pdfDocument->setMetadata(info, infoCount, &now, &now);

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
