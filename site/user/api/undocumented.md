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
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaintDefaults_Flags'><code>SkPaintDefaults_Flags</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
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

