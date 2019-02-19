undocumented
===


<a name='SkBBHFactory'></a>

---

<a name='GrBackendSemaphore'></a>

---

<a name='SkClipOp'></a>

---

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkClipOp_kDifference'><code>SkClipOp::kDifference</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkClipOp_kIntersect'><code>SkClipOp::kIntersect</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

<a name='SkColorFilter'></a>

---

<a name='SkColorSpace'></a>

---

<a name='SkColorSpace_MakeSRGBLinear'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#sk_sp'>sk sp</a>&lt;<a href='#SkColorSpace'>SkColorSpace</a>&gt; <a href='#SkColorSpace_MakeSRGBLinear'>MakeSRGBLinear</a>()
</pre>

<a name='SkColorSpace_gammaCloseToSRGB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkColorSpace_gammaCloseToSRGB'>gammaCloseToSRGB</a>() const
</pre>

<a name='SkColorSpace_Equals'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkColorSpace_Equals'>Equals</a>(const <a href='#SkColorSpace'>SkColorSpace</a>* src, const <a href='#SkColorSpace'>SkColorSpace</a>* dst)
</pre>

<a name='SkCreateColorSpaceXformCanvas'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
std::unique_ptr&lt;<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>&gt; SK_API <a href='#SkCreateColorSpaceXformCanvas'>SkCreateColorSpaceXformCanvas</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* target,
                                                     <a href='#sk_sp'>sk sp</a>&lt;<a href='#SkColorSpace'>SkColorSpace</a>&gt; targetCS)
</pre>

<a name='SkData'></a>

---

<a name='SkDebugCanvas'></a>

---

<a name='SkDebugf'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SK_API void <a href='#SkDebugf'>SkDebugf</a>(const char format[], ...)
</pre>

<a name='SkDeferredDisplayList'></a>

---

<a name='Recorder'></a>

<a name='SkDeferredDisplayListRecorder'></a>

---

<a name='SkDeserialProcs'></a>

---<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkDeserialPictureProc</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkDeserialProcs_fPictureProc'><code>fPictureProc</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>void*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkDeserialProcs_fPictureCtx'><code>fPictureCtx</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkDeserialTypefaceProc</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkDeserialProcs_fTypefaceProc'><code>fTypefaceProc</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>void*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkDeserialProcs_fTypefaceCtx'><code>fTypefaceCtx</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

<a name='SkBaseDevice'></a>

---

<a name='SkDocument'></a>

---

<a name='SkDocument_beginPage'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='#SkDocument_beginPage'>beginPage</a>(<a href='#SkScalar'>SkScalar</a> width, <a href='#SkScalar'>SkScalar</a> height, const <a href='SkRect_Reference#SkRect'>SkRect</a>* content = NULL)
</pre>

<a name='PDF'></a>

<a name='SkDrawLooper'></a>

---

<a name='SkDrawable'></a>

---

<a name='SkDrawable_draw'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkDrawable_draw'>draw</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>*, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* = NULL)
</pre>

<a name='SkFilterQuality'></a>

---

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kNone_SkFilterQuality'><code>kNone_SkFilterQuality</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kLow_SkFilterQuality'><code>kLow_SkFilterQuality</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kMedium_SkFilterQuality'><code>kMedium_SkFilterQuality</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kHigh_SkFilterQuality'><code>kHigh_SkFilterQuality</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

<a name='Nearest_Neighbor'></a>

<a name='Bilerp'></a>

<a name='MipMap'></a>

<a name='BiCubic'></a>

<a name='Advance'></a>

<a name='Engine'></a>

<a name='SkTextEncoding'></a>

---

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kUTF8_SkTextEncoding'><code>kUTF8_SkTextEncoding</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kUTF16_SkTextEncoding'><code>kUTF16_SkTextEncoding</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kUTF32_SkTextEncoding'><code>kUTF32_SkTextEncoding</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kGlyphID_SkTextEncoding'><code>kGlyphID_SkTextEncoding</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

<a name='SkFont'></a>

---

<a name='SkFontHinting'></a>

---

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kNo_SkFontHinting'><code>kNo_SkFontHinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kSlight_SkFontHinting'><code>kSlight_SkFontHinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kNormal_SkFontHinting'><code>kNormal_SkFontHinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kFull_SkFontHinting'><code>kFull_SkFontHinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

<a name='SkFontMetrics'></a>

---

<a href='#SkFontMetrics'>SkFontMetrics</a> is filled out by <a href='SkPaint_Reference#SkPaint_getFontMetrics'>SkPaint::getFontMetrics</a>. <a href='#SkFontMetrics'>SkFontMetrics</a> contents
reflect the values
computed by <a href='#Font_Manager'>Font Manager</a> using <a href='#Typeface'>Typeface</a>. Values are set to zero if they are
not available.

All vertical values are relative to the baseline, on a y-axis pointing down.
Zero is on the baseline, negative values are above the baseline, and positive
values are below the baseline.

<a href='#SkFontMetrics_fUnderlineThickness'>fUnderlineThickness</a> and <a href='#SkFontMetrics_fUnderlinePosition'>fUnderlinePosition</a> have a bit set in <a href='#SkFontMetrics_fFlags'>fFlags</a> if their values
are valid, since their value may be zero.

<a href='#SkFontMetrics_fStrikeoutThickness'>fStrikeoutThickness</a> and <a href='#SkFontMetrics_fStrikeoutPosition'>fStrikeoutPosition</a> have a bit set in <a href='#SkFontMetrics_fFlags'>fFlags</a> if their values
are valid, since their value may be zero.

<a name='SkFontMetrics_FontMetricsFlags'></a>

---

<a href='#SkFontMetrics_FontMetricsFlags'>FontMetricsFlags</a> are set in <a href='#SkFontMetrics_fFlags'>fFlags</a> when underline and strikeout metrics are valid;
the underline or strikeout metric may be valid and zero.
Fonts with embedded bitmaps may not have valid underline or strikeout metrics.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_kUnderlineThicknessIsValid_Flag'><code>SkFontMetrics::kUnderlineThicknessIsValid_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0001</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if fUnderlineThickness is valid</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_kUnderlinePositionIsValid_Flag'><code>SkFontMetrics::kUnderlinePositionIsValid_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0002</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if fUnderlinePosition is valid</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_kStrikeoutThicknessIsValid_Flag'><code>SkFontMetrics::kStrikeoutThicknessIsValid_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0004</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if fStrikeoutThickness is valid</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_kStrikeoutPositionIsValid_Flag'><code>SkFontMetrics::kStrikeoutPositionIsValid_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0008</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if fStrikeoutPosition is valid</td>
  </tr>
</table><table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>uint32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fFlags'><code>fFlags</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
is set to FontMetricsFlags when metrics are valid</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fTop'><code>fTop</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Greatest extent above the baseline for any glyph.
Typically less than zero.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fAscent'><code>fAscent</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Recommended distance above the baseline to reserve for a line of text.
Typically less than zero.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fDescent'><code>fDescent</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Recommended distance below the baseline to reserve for a line of text.
Typically greater than zero.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fBottom'><code>fBottom</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Greatest extent below the baseline for any glyph.
Typically greater than zero.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fLeading'><code>fLeading</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Recommended distance to add between lines of text.
Typically greater than or equal to zero.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fAvgCharWidth'><code>fAvgCharWidth</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Average character width, if it is available.
Zero if no average width is stored in the font.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fMaxCharWidth'><code>fMaxCharWidth</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
maximum character width</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fXMin'><code>fXMin</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Minimum bounding box x-axis value for all <a href='#Glyph'>Glyphs</a>.
Typically less than zero.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fXMax'><code>fXMax</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Maximum bounding box x-axis value for all <a href='#Glyph'>Glyphs</a>.
Typically greater than zero.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fXHeight'><code>fXHeight</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be zero if no lower-case height is stored in the font.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fCapHeight'><code>fCapHeight</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be zero if no upper-case height is stored in the font.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fUnderlineThickness'><code>fUnderlineThickness</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
If the metric is valid, the <a href='#SkFontMetrics_kUnderlineThicknessIsValid_Flag'>kUnderlineThicknessIsValid Flag</a> is set in <a href='#SkFontMetrics_fFlags'>fFlags</a>.
If <a href='#SkFontMetrics_kUnderlineThicknessIsValid_Flag'>kUnderlineThicknessIsValid Flag</a> is clear, <a href='#SkFontMetrics_fUnderlineThickness'>fUnderlineThickness</a> is zero.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fUnderlinePosition'><code>fUnderlinePosition</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Position of the top of the underline stroke relative to the baseline.
Typically positive when valid.

If the metric is valid, the <a href='#SkFontMetrics_kUnderlinePositionIsValid_Flag'>kUnderlinePositionIsValid Flag</a> is set in <a href='#SkFontMetrics_fFlags'>fFlags</a>.
If <a href='#SkFontMetrics_kUnderlinePositionIsValid_Flag'>kUnderlinePositionIsValid Flag</a> is clear, <a href='#SkFontMetrics_fUnderlinePosition'>fUnderlinePosition</a> is zero.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fStrikeoutThickness'><code>fStrikeoutThickness</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
If the metric is valid, the <a href='#SkFontMetrics_kStrikeoutThicknessIsValid_Flag'>kStrikeoutThicknessIsValid Flag</a> is set in <a href='#SkFontMetrics_fFlags'>fFlags</a>.
If <a href='#SkFontMetrics_kStrikeoutThicknessIsValid_Flag'>kStrikeoutThicknessIsValid Flag</a> is clear, <a href='#SkFontMetrics_fStrikeoutThickness'>fStrikeoutThickness</a> is zero.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFontMetrics_fStrikeoutPosition'><code>fStrikeoutPosition</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Position of the bottom of the strikeout stroke relative to the baseline.
Typically negative when valid.

If the metric is valid, the <a href='#SkFontMetrics_kStrikeoutPositionIsValid_Flag'>kStrikeoutPositionIsValid Flag</a> is set in <a href='#SkFontMetrics_fFlags'>fFlags</a>.
If <a href='#SkFontMetrics_kStrikeoutPositionIsValid_Flag'>kStrikeoutPositionIsValid Flag</a> is clear, <a href='#SkFontMetrics_fStrikeoutPosition'>fStrikeoutPosition</a> is zero.
</td>
  </tr>
</table>

<a name='SkFontMetrics_hasUnderlineThickness'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkFontMetrics_hasUnderlineThickness'>hasUnderlineThickness</a>(<a href='#SkScalar'>SkScalar</a>* thickness) const
</pre>

<a name='SkFontMetrics_hasUnderlinePosition'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkFontMetrics_hasUnderlinePosition'>hasUnderlinePosition</a>(<a href='#SkScalar'>SkScalar</a>* position) const
</pre>

<a name='SkFontMetrics_hasStrikeoutThickness'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkFontMetrics_hasStrikeoutThickness'>hasStrikeoutThickness</a>(<a href='#SkScalar'>SkScalar</a>* thickness) const
</pre>

<a name='SkFontMetrics_hasStrikeoutPosition'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkFontMetrics_hasStrikeoutPosition'>hasStrikeoutPosition</a>(<a href='#SkScalar'>SkScalar</a>* position) const
</pre>

<a name='GrContext'></a>

---

<a name='GrContext_flush'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#GrContext_flush'>flush</a>()
</pre>

<a name='ID'></a>

<a name='ArcTo'></a>

<a name='SkISize'></a>

---

<a name='SkISize_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkISize_width'>width</a>() const
</pre>

<a name='SkISize_height'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkISize_height'>height</a>() const
</pre>

<a name='SkImageFilter'></a>

---

<a name='SkMaskFilter'></a>

---

<a name='SkMetaData'></a>

---

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaintDefaults_Hinting'><code>SkPaintDefaults_Hinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaintDefaults_TextSize'><code>SkPaintDefaults_TextSize</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>12</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaintDefaults_MiterLimit'><code>SkPaintDefaults_MiterLimit</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>


<a name='SkPathOp'></a>

---

</table>

<a name='Op'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool SK_API <a href='#Op'>Op</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& one, const <a href='SkPath_Reference#SkPath'>SkPath</a>& two, <a href='#SkPathOp'>SkPathOp</a> op, <a href='SkPath_Reference#SkPath'>SkPath</a>* result)
</pre>

<a name='SkPathEffect'></a>

---

<a name='SkPictureRecorder'></a>

---

<a name='SkPictureRecorder_beginRecording'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='#SkPictureRecorder_beginRecording'>beginRecording</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& bounds, <a href='#SkBBHFactory'>SkBBHFactory</a>* bbhFactory = NULL,
                         uint32_t recordFlags = 0)
</pre>

<a name='Storage'></a>

<a name='SkPixelRef'></a>

---

<a name='SkPixelRef_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPixelRef_width'>width</a>() const
</pre>

<a name='SkPixelRef_height'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPixelRef_height'>height</a>() const
</pre>

<a name='SkPixelRef_isImmutable'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixelRef_isImmutable'>isImmutable</a>() const
</pre>

<a name='SkPixelRef_setImmutable'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPixelRef_setImmutable'>setImmutable</a>()
</pre>

<a name='SkPoint3'></a>

---

<a name='Arct'></a>

<a name='SkRSXform'></a>

---<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRSXform_fSCos'><code>fSCos</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRSXform_fSSin'><code>fSSin</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRSXform_fTx'><code>fTx</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRSXform_fTy'><code>fTy</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

<a name='SkRasterHandleAllocator'></a>

---

<a name='SkRasterHandleAllocator_Handle'></a>

---

<a name='SkRasterHandleAllocator_Rec'></a>

---

<a name='SkRasterHandleAllocator_MakeCanvas'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static std::unique_ptr&lt;<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>&gt; <a href='#SkRasterHandleAllocator_MakeCanvas'>MakeCanvas</a>(std::unique_ptr&lt;<a href='#SkRasterHandleAllocator'>SkRasterHandleAllocator</a>&gt;,
                                            const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>&, const <a href='#SkRasterHandleAllocator_Rec'>Rec</a>* rec = nullptr)
</pre>

<a name='SkRefCnt'></a>

---

<a name='GrBackendRenderTarget'></a>

---

<a name='GrBackendRenderTarget_isValid'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#GrBackendRenderTarget_isValid'>isValid</a>() const
</pre>

<a name='GrRenderTarget'></a>

---

<a name='Canvas'></a>

<a name='Arc'></a>

<a name='Sweep_Flag'></a>

<a name='SkScalar'></a>

---

<a name='SkSerialProcs'></a>

---<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkSerialPictureProc</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSerialProcs_fPictureProc'><code>fPictureProc</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>void*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSerialProcs_fPictureCtx'><code>fPictureCtx</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkSerialTypefaceProc</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSerialProcs_fTypefaceProc'><code>fTypefaceProc</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>void*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSerialProcs_fTypefaceCtx'><code>fTypefaceCtx</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

<a name='SkShader'></a>

---

<a name='SkShader_TileMode'></a>

---

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkShader_kClamp_TileMode'><code>SkShader::kClamp_TileMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkShader_kRepeat_TileMode'><code>SkShader::kRepeat_TileMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkShader_kMirror_TileMode'><code>SkShader::kMirror_TileMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

<a name='SkShader_MakeBitmapShader'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#sk_sp'>sk sp</a>&lt;<a href='#SkShader'>SkShader</a>&gt; <a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& src, <a href='#SkShader_TileMode'>TileMode</a> tmx, <a href='#SkShader_TileMode'>TileMode</a> tmy,
                                        const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* localMatrix = nullptr)
</pre>

<a name='SkShader_MakeCompose'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#sk_sp'>sk sp</a>&lt;<a href='#SkShader'>SkShader</a>&gt; <a href='#SkShader_MakeCompose'>MakeCompose</a>(<a href='#sk_sp'>sk sp</a>&lt;<a href='#SkShader'>SkShader</a>&gt; dst, <a href='#sk_sp'>sk sp</a>&lt;<a href='#SkShader'>SkShader</a>&gt; src, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode,
                                   float lerp = 1)
</pre>

<a name='SkSize'></a>

---

<a name='SkSize_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkSize_width'>width</a>() const
</pre>

<a name='SkSize_height'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkSize_height'>height</a>() const
</pre>

<a name='sk_sp'></a>

---

<a name='SkStreamAsset'></a>

---

<a name='SkString'></a>

---

<a name='SkSurfaceCharacterization'></a>

---

<a name='SkSurfaceProps'></a>

---

<a name='Legacy_Font_Host'></a>

<a name='SkSurfaceProps_InitType'></a>

---

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurfaceProps_kLegacyFontHost_InitType'><code>SkSurfaceProps::kLegacyFontHost_InitType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

#
Topic

<a name='GrBackendTexture'></a>

---

<a name='GrBackendTexture_isValid'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#GrBackendTexture_isValid'>isValid</a>() const
</pre>

<a name='ID'></a>

<a name='SkFontID'></a>

---

<a name='SkTypeface'></a>

---

<a name='SkTypeface_uniqueID'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkFontID'>SkFontID</a> <a href='#SkTypeface_uniqueID'>uniqueID</a>() const
</pre>

<a name='SkVertices'></a>

---

<a name='SkVertices_Bone'></a>

---

<a name='Colors'></a>

<a name='Texs'></a>

<a name='SkXfermodeImageFilter'></a>

---

<a name='SkYUVAIndex'></a>

---

