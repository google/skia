---
title: "Using Skia's PDF Backend"
linkTitle: "Using Skia's PDF Backend"
---

Here is an example of using Skia's PDF backend (SkPDF) via the SkDocument and
SkCanvas APIs.

<fiddle-embed-sk name='@PDF'></fiddle-embed-sk>

<!-- https://fiddle.skia.org/c/@PDF docs/examples/PDF.cpp -->

---

## SkPDF Limitations

There are several corners of Skia's public API that SkPDF currently does not
handle because either no known client uses the feature or there is no simple
PDF-ish way to handle it.

In this document:

- **drop** means to draw nothing.

- **ignore** means to draw without the effect

- **expand** means to implement something in a non-PDF-ish way. This may mean to
  rasterize vector graphics, to expand paths with path effects into many
  individual paths, or to convert text to paths.

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

- _SkImageFilter_: When SkImageFilter is expanded, text-as-text is lost.

- _SkXferMode_: The following transfer modes are not natively supported by PDF:
  DstOver, SrcIn, DstIn, SrcOut, DstOut, SrcATop, DstATop, and Modulate.

Other limitations:

- _drawText with VerticalText_ — drop. No known clients seem to make use of the
  VerticalText flag.

- _drawTextOnPath_ — expand. (Text-as-text is lost.)

- _drawVertices_ — drop.

- _drawPatch_ — drop.

---
