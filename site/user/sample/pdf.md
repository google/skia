Using Skia's PDF Backend
========================

Here is an example of using Skia's PDF backend in the recommended way:
via the SkDocument and SkCanvas APIs.

<!--?prettify?-->

    #include "SkDocument.h"

    bool WritePDF() {
        SkWStream* output = ....;

        SkAutoTUnref<SkDocument> pdfDocument(
                SkDocument::CreatePDF(outputStream));

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
