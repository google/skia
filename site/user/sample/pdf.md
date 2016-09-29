Using Skia's PDF Backend
========================

Here is an example of using Skia's PDF backend (SkPDF) via the
SkDocument and SkCanvas APIs.

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

* * *

<span id="limits">SkPDF Limitations</span>
------------------------------------------

There are several corners of Skia's public API that SkPDF currently
does not handle because either no known client uses the feature or
there is no simple PDF-ish way to handle it.

In this document:

  + **drop** means to draw nothing.

  + **ignore** mean to draw without the effect

  + **expand** means to implement something in a non-PDF-ish way.
    This may mean to rasterize vector graphics, to expand paths with
    path effects into many individual paths, or to convert text to
    paths.

<style scoped><!--
#pdftable {border-collapse:collapse;}
#pdftable tr th, #pdftable tr td {border:#888888 2px solid;padding: 5px;}
--></style>
<table id="pdftable">
<tr><th>Effect</th>                  <th>text</th>   <th>images</th> <th>everything
                                                                         else</th></tr>
<tr><th>SkMaskFilter</th>            <td>drop</td>   <td>ignore</td> <td>ignore</td></tr>
<tr><th>SkPathEffect</th>            <td>ignore</td> <td>n/a</td>    <td>expand</td></tr>
<tr><th>SkColorFilter</th>           <td>ignore</td> <td>expand</td> <td>ignore</td></tr>
<tr><th>SkImageFilter</th>           <td>expand</td> <td>expand</td> <td>expand</td></tr>
<tr><th>unsupported SkXferModes</th> <td>ignore</td> <td>ignore</td> <td>ignore</td></tr>
<tr><th>non-gradient SkShader</th>   <td>expand</td> <td>n/a</td>    <td>expand</td></tr>
</table>

Notes:

  - *SkImageFilter*: When SkImageFilter is expanded, text-as-text is lost.

  - *SkXferMode*: The following transfer modes are not natively
    supported by PDF: DstOver, SrcIn, DstIn, SrcOut, DstOut, SrcATop,
    DstATop, and Modulate.

Other limitations:

  - *drawText with VerticalText* — drop. No known clients seem to make use
    of the VerticalText flag.

  - *drawTextOnPath* — expand. (Text-as-text is lost.)

  - *drawVertices* — drop.

  - *drawPatch* — drop.

* * *
