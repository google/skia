undocumented
===

# <a name='Alias'>Alias</a>

# <a name='Anti-alias'>Anti-alias</a>

# <a name='Arc'>Arc</a>

# <a name='BBH_Factory'>BBH Factory</a>

# <a name='SkBBHFactory'>Class SkBBHFactory</a>

# <a name='Backend_Semaphore'>Backend Semaphore</a>

# <a name='GrBackendSemaphore'>Class GrBackendSemaphore</a>

# <a name='Blend_Mode'>Blend Mode</a>

## <a name='SkBlendMode'>Enum SkBlendMode</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrc'><code>SkBlendMode::kSrc</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrcOver'><code>SkBlendMode::kSrcOver</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kPlus'><code>SkBlendMode::kPlus</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>12</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

# <a name='Circle'>Circle</a>

# <a name='Clip_Op'>Clip Op</a>

## <a name='SkClipOp'>Enum SkClipOp</a>

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

# <a name='Color_Filter'>Color Filter</a>

# <a name='SkColorFilter'>Class SkColorFilter</a>

<a name='SkColorFilter_toString'></a>
## toString

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void toString(SkString* str) const
</pre>

---

# <a name='Color_Space'>Color Space</a>

# <a name='SkColorSpace'>Class SkColorSpace</a>

<a name='SkColorSpace_MakeSRGBLinear'></a>
## MakeSRGBLinear

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static sk_sp&lt;SkColorSpace&gt; MakeSRGBLinear()
</pre>

---

<a name='SkColorSpace_gammaCloseToSRGB'></a>
## gammaCloseToSRGB

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool gammaCloseToSRGB() const
</pre>

---

<a name='SkColorSpace_Equals'></a>
## Equals

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool Equals(const SkColorSpace* src, const SkColorSpace* dst)
</pre>

---

## <a name='SkTransferFunctionBehavior'>Enum SkTransferFunctionBehavior</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTransferFunctionBehavior_kRespect'><code>SkTransferFunctionBehavior::kRespect</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTransferFunctionBehavior_kIgnore'><code>SkTransferFunctionBehavior::kIgnore</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

# <a name='Core_Graphics'>Core Graphics</a>

# <a name='Core_Text'>Core Text</a>

# <a name='Create_Color_Space_Xform_Canvas'>Create Color Space Xform Canvas</a>

<a name='SkCreateColorSpaceXformCanvas'></a>
## SkCreateColorSpaceXformCanvas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
std::unique_ptr&lt;SkCanvas&gt; SK_API SkCreateColorSpaceXformCanvas(SkCanvas* target,
                                                     sk_sp&lt;SkColorSpace&gt; targetCS)
</pre>

---

# <a name='Curve'>Curve</a>

# <a name='Data'>Data</a>

# <a name='SkData'>Class SkData</a>

# <a name='Debug_Canvas'>Debug Canvas</a>

# <a name='SkDebugCanvas'>Class SkDebugCanvas</a>

# <a name='Debugging'>Debugging</a>

<a name='SkDebugf'></a>
## SkDebugf

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SK_API void SkDebugf(const char format[], ...)
</pre>

---

# <a name='Deferred_Display_List'>Deferred Display List</a>

# <a name='SkDeferredDisplayList'>Class SkDeferredDisplayList</a>

## <a name='Recorder'>Recorder</a>

# <a name='SkDeferredDisplayListRecorder'>Class SkDeferredDisplayListRecorder</a>

# <a name='Device'>Device</a>

# <a name='SkBaseDevice'>Class SkBaseDevice</a>

# <a name='Document'>Document</a>

# <a name='SkDocument'>Class SkDocument</a>

<a name='SkDocument_beginPage'></a>
## beginPage

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SkCanvas* beginPage(SkScalar width, SkScalar height, const SkRect* content = NULL)
</pre>

---

## <a name='PDF'>PDF</a>

# <a name='Draw_Filter'>Draw Filter</a>

# <a name='SkDrawFilter'>Class SkDrawFilter</a>

# <a name='Draw_Layer'>Draw Layer</a>

# <a name='Draw_Looper'>Draw Looper</a>

# <a name='SkDrawLooper'>Class SkDrawLooper</a>

# <a name='Drawable'>Drawable</a>

# <a name='SkDrawable'>Class SkDrawable</a>

<a name='SkDrawable_draw'></a>
## draw

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void draw(SkCanvas*, const SkMatrix* = NULL)
</pre>

---

# <a name='Encoded_Image_Format'>Encoded Image Format</a>

## <a name='SkEncodedImageFormat'>Enum SkEncodedImageFormat</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkEncodedImageFormat_kUnknown'><code>SkEncodedImageFormat::kUnknown</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkEncodedImageFormat_kBMP'><code>SkEncodedImageFormat::kBMP</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkEncodedImageFormat_kGIF'><code>SkEncodedImageFormat::kGIF</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkEncodedImageFormat_kICO'><code>SkEncodedImageFormat::kICO</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkEncodedImageFormat_kJPEG'><code>SkEncodedImageFormat::kJPEG</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkEncodedImageFormat_kPNG'><code>SkEncodedImageFormat::kPNG</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkEncodedImageFormat_kWBMP'><code>SkEncodedImageFormat::kWBMP</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>6</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkEncodedImageFormat_kWEBP'><code>SkEncodedImageFormat::kWEBP</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>7</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkEncodedImageFormat_kPKM'><code>SkEncodedImageFormat::kPKM</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkEncodedImageFormat_kKTX'><code>SkEncodedImageFormat::kKTX</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>9</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkEncodedImageFormat_kASTC'><code>SkEncodedImageFormat::kASTC</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>10</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkEncodedImageFormat_kDNG'><code>SkEncodedImageFormat::kDNG</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>11</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkEncodedImageFormat_kHEIF'><code>SkEncodedImageFormat::kHEIF</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>12</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

# <a name='Filter_Quality'>Filter Quality</a>

## <a name='SkFilterQuality'>Enum SkFilterQuality</a>

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

## <a name='Nearest_Neighbor'>Nearest Neighbor</a>

## <a name='Bilerp'>Bilerp</a>

## <a name='MipMap'>MipMap</a>

## <a name='BiCubic'>BiCubic</a>

# <a name='Font'>Font</a>

## <a name='Advance'>Advance</a>

## <a name='Engine'>Engine</a>

# <a name='Font_Manager'>Font Manager</a>

# <a name='GPU_Context'>GPU Context</a>

## <a name='Resource_Cache_Limits'>Resource Cache Limits</a>

# <a name='GrContext'>Class GrContext</a>

<a name='GrContext_flush'></a>
## flush

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void flush()
</pre>

---

# <a name='GPU_Surface'>GPU Surface</a>

# <a name='GPU_Texture'>GPU Texture</a>

# <a name='Glyph'>Glyph</a>

# <a name='HTML_Canvas'>HTML Canvas</a>

## <a name='ArcTo'>ArcTo</a>

# <a name='ISize'>ISize</a>

# <a name='SkISize'>Struct SkISize</a>

# <a name='Image_Filter'>Image Filter</a>

## <a name='Scaling'>Scaling</a>

# <a name='SkImageFilter'>Class SkImageFilter</a>

<a name='SkImageFilter_toString'></a>
## toString

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void toString(SkString* str) const
</pre>

---

# <a name='Image_Scaling'>Image Scaling</a>

# <a name='Left_Side_Bearing'>Left Side Bearing</a>

# <a name='Line'>Line</a>

# <a name='Malloc_Pixel_Ref'>Malloc Pixel Ref</a>

# <a name='SkMallocPixelRef'>Class SkMallocPixelRef</a>

<a name='SkMallocPixelRef_MakeZeroed'></a>
## MakeZeroed

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static sk_sp&lt;SkPixelRef&gt; MakeZeroed(const SkImageInfo&, size_t rowBytes)
</pre>

---

<a name='SkMallocPixelRef_MakeAllocate'></a>
## MakeAllocate

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static sk_sp&lt;SkPixelRef&gt; MakeAllocate(const SkImageInfo&, size_t rowBytes)
</pre>

---

# <a name='Mask_Alpha'>Mask Alpha</a>

# <a name='Mask_Filter'>Mask Filter</a>

# <a name='SkMaskFilter'>Class SkMaskFilter</a>

<a name='SkMaskFilter_toString'></a>
## toString

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void toString(SkString* str) const
</pre>

---

# <a name='Math'>Math</a>

<a name='sk_64_isS32'></a>
## sk_64_isS32

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static inline bool sk_64_isS32(int64_t value)
</pre>

---

<a name='SkIntToScalar'></a>
## SkIntToScalar

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SkIntToScalar(x)
</pre>

---

<a name='SkScalarRoundToInt'></a>
## SkScalarRoundToInt

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SkScalarRoundToInt(x)
</pre>

---

<a name='SkScalarFloorToInt'></a>
## SkScalarFloorToInt

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SkScalarFloorToInt(x)
</pre>

---

<a name='SkScalarCeilToInt'></a>
## SkScalarCeilToInt

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SkScalarCeilToInt(x)
</pre>

---

<a name='SkScalarFloorToScalar'></a>
## SkScalarFloorToScalar

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SkScalarFloorToScalar(x)
</pre>

---

<a name='SkScalarCeilToScalar'></a>
## SkScalarCeilToScalar

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SkScalarCeilToScalar(x)
</pre>

---

<a name='SkScalarIsFinite'></a>
## SkScalarIsFinite

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SkScalarIsFinite(x)
</pre>

---

<a name='SkScalarIsNaN'></a>
## SkScalarIsNaN

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SkScalarIsNaN(x)
</pre>

---

<a name='SkTFitsIn'></a>
## SkTFitsIn

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
template &lt;typename D, typename S&gt; inline bool SkTFitsIn(S s)
</pre>

---

# <a name='Meta_Data'>Meta Data</a>

# <a name='SkMetaData'>Class SkMetaData</a>

# <a name='Mip_Map'>Mip Map</a>

# <a name='Nine_Patch'>Nine Patch</a>

# <a name='Number_Types'>Number Types</a>

# <a name='SkGlyphID'>Typedef SkGlyphID</a>

## <a name='Scalar'>Scalar</a>

# <a name='SkScalar'>Typedef SkScalar</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_MinS32FitsInFloat'><code>SK_MinS32FitsInFloat</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_MaxS32FitsInFloat'><code>SK_MaxS32FitsInFloat</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ScalarMin'><code>SK_ScalarMin</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ScalarMax'><code>SK_ScalarMax</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ScalarInfinity'><code>SK_ScalarInfinity</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ScalarNegativeInfinity'><code>SK_ScalarNegativeInfinity</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ScalarNaN'><code>SK_ScalarNaN</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_MinS32'><code>SK_MinS32</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_MaxS32'><code>SK_MaxS32</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_MaxSizeT'><code>SK_MaxSizeT</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

# <a name='SkUnichar'>Typedef SkUnichar</a>

# <a name='U8CPU'>Typedef U8CPU</a>

# <a name='OS_X'>OS X</a>

# <a name='Oval'>Oval</a>

# <a name='Paint_Defaults'>Paint Defaults</a>

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


# <a name='Patch'>Patch</a>

# <a name='PathOps'>PathOps</a>

<a name='Op'></a>
## Op

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool SK_API Op(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result)
</pre>

---

# <a name='Path_Effect'>Path Effect</a>

# <a name='SkPathEffect'>Class SkPathEffect</a>

<a name='SkPathEffect_toString'></a>
## toString

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void toString(SkString* str) const
</pre>

---

# <a name='Path_Measure'>Path Measure</a>

# <a name='SkPathMeasure'>Class SkPathMeasure</a>

<a name='SkPathMeasure_dump'></a>
## dump

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void dump() const
</pre>

---

# <a name='Picture'>Picture</a>

# <a name='SkPicture'>Class SkPicture</a>

<a name='SkPicture_playback'></a>
## playback

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual void playback(SkCanvas*, AbortCallback* = nullptr) const = 0
</pre>

---

## <a name='Recorder'>Recorder</a>

# <a name='SkPictureRecorder'>Class SkPictureRecorder</a>

<a name='SkPictureRecorder_beginRecording'></a>
## beginRecording

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SkCanvas* beginRecording(const SkRect& bounds, SkBBHFactory* bbhFactory = NULL,
                         uint32_t recordFlags = 0)
</pre>

---

# <a name='Pixel'>Pixel</a>

## <a name='Storage'>Storage</a>

# <a name='Pixel_Ref'>Pixel Ref</a>

# <a name='SkPixelRef'>Class SkPixelRef</a>

<a name='SkPixelRef_width'></a>
## width

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int width() const
</pre>

---

<a name='SkPixelRef_height'></a>
## height

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int height() const
</pre>

---

<a name='SkPixelRef_isImmutable'></a>
## isImmutable

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool isImmutable() const
</pre>

---

<a name='SkPixelRef_setImmutable'></a>
## setImmutable

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void setImmutable()
</pre>

---

# <a name='Point3'>Point3</a>

# <a name='SkPoint3'>Struct SkPoint3</a>

# <a name='PostScript'>PostScript</a>

## <a name='Arct'>Arct</a>

# <a name='Premultiply'>Premultiply</a>

# <a name='RSXform'>RSXform</a>

# <a name='SkRSXform'>Struct SkRSXform</a>

# <a name='Raster_Bitmap'>Raster Bitmap</a>

# <a name='Raster_Engine'>Raster Engine</a>

# <a name='Raster_Handle_Allocator'>Raster Handle Allocator</a>

# <a name='SkRasterHandleAllocator'>Class SkRasterHandleAllocator</a>

# <a name='SkRasterHandleAllocator_Rec'>Struct SkRasterHandleAllocator::Rec</a>

<a name='SkRasterHandleAllocator_MakeCanvas'></a>
## MakeCanvas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static std::unique_ptr&lt;SkCanvas&gt; MakeCanvas(std::unique_ptr&lt;SkRasterHandleAllocator&gt;,
                                            const SkImageInfo&, const Rec* rec = nullptr)
</pre>

---

# <a name='Raster_Surface'>Raster Surface</a>

# <a name='Rasterizer'>Rasterizer</a>

# <a name='SkRasterizer'>Class SkRasterizer</a>

## <a name='Layer'>Layer</a>

# <a name='Read_Buffer'>Read Buffer</a>

# <a name='SkReadBuffer'>Struct SkReadBuffer</a>

# <a name='Reference_Count'>Reference Count</a>

# <a name='SkRefCnt'>Class SkRefCnt</a>

# <a name='sk_sp'>Class sk_sp</a>

# <a name='Region'>Region</a>

# <a name='SkRegion'>Class SkRegion</a>

# <a name='RenderTarget'>RenderTarget</a>

# <a name='GrBackendRenderTarget'>Class GrBackendRenderTarget</a>

<a name='GrBackendRenderTarget_isValid'></a>
## isValid

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool isValid() const
</pre>

---

# <a name='Render_Target'>Render Target</a>

# <a name='GrRenderTarget'>Class GrRenderTarget</a>

# <a name='Right_Side_Bearing'>Right Side Bearing</a>

# <a name='Round_Rect'>Round Rect</a>

# <a name='SkRRect'>Class SkRRect</a>

<a name='SkRRect_dump'></a>
## dump

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void dump() const
</pre>

---

<a name='SkRRect_dumpHex'></a>
## dumpHex

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void dumpHex() const
</pre>

---

# <a name='SVG'>SVG</a>

## <a name='Canvas'>Canvas</a>

## <a name='Arc'>Arc</a>

# <a name='Shader'>Shader</a>

# <a name='SkShader'>Class SkShader</a>

## <a name='SkShader_TileMode'>Enum SkShader::TileMode</a>

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
## MakeBitmapShader

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static sk_sp&lt;SkShader&gt; MakeBitmapShader(const SkBitmap& src, TileMode tmx, TileMode tmy,
                                        const SkMatrix* localMatrix = nullptr)
</pre>

---

## <a name='Gradient'>Gradient</a>

# <a name='Size'>Size</a>

# <a name='SkSize'>Struct SkSize</a>

# <a name='Sprite'>Sprite</a>

# <a name='Stream'>Stream</a>

# <a name='SkStream'>Class SkStream</a>

# <a name='String'>String</a>

# <a name='SkString'>Class SkString</a>

# <a name='Supersampling'>Supersampling</a>

# <a name='Surface_Characterization'>Surface Characterization</a>

# <a name='SkSurfaceCharacterization'>Class SkSurfaceCharacterization</a>

# <a name='Surface_Properties'>Surface Properties</a>

# <a name='SkSurfaceProps'>Class SkSurfaceProps</a>

## <a name='Legacy_Font_Host'>Legacy Font Host</a>

## <a name='SkSurfaceProps_InitType'>Enum SkSurfaceProps::InitType</a>

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

# <a name='Text'>Text</a>

# <a name='Text_Blob'>Text Blob</a>

# <a name='SkTextBlob'>Class SkTextBlob</a>

# <a name='Texture'>Texture</a>

# <a name='GrBackendTexture'>Class GrBackendTexture</a>

<a name='GrBackendTexture_isValid'></a>
## isValid

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool isValid() const
</pre>

---

# <a name='Transfer_Mode'>Transfer Mode</a>

# <a name='Typeface'>Typeface</a>

# <a name='SkTypeface'>Class SkTypeface</a>

# <a name='Types'>Types</a>

# <a name='GrBackendObject'>Typedef GrBackendObject</a>

## <a name='GrSurfaceOrigin'>Enum GrSurfaceOrigin</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kBottomLeft_GrSurfaceOrigin'><code>kBottomLeft_GrSurfaceOrigin</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kTopLeft_GrSurfaceOrigin'><code>kTopLeft_GrSurfaceOrigin</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

## <a name='Budgeted'>Budgeted</a>

## <a name='SkBudgeted'>Enum SkBudgeted</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBudgeted_kNo'><code>SkBudgeted::kNo</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBudgeted_kYes'><code>SkBudgeted::kYes</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

## <a name='GrSemaphoresSubmitted'>Enum GrSemaphoresSubmitted</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='GrSemaphoresSubmitted_kNo'><code>GrSemaphoresSubmitted::kNo</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='GrSemaphoresSubmitted_kYes'><code>GrSemaphoresSubmitted::kYes</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

# <a name='Unpremultiply'>Unpremultiply</a>

# <a name='Vertices'>Vertices</a>

## <a name='Colors'>Colors</a>

## <a name='Texs'>Texs</a>

# <a name='Write_Buffer'>Write Buffer</a>

# <a name='SkWriteBuffer'>Struct SkWriteBuffer</a>
