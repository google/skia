undocumented
===

# <a name='Alias'>Alias</a>

# <a name='Arc'>Arc</a>

# <a name='BBH_Factory'>BBH Factory</a>

# <a name='SkBBHFactory'>Class SkBBHFactory</a>

# <a name='Backend_Semaphore'>Backend Semaphore</a>

# <a name='GrBackendSemaphore'>Class GrBackendSemaphore</a>

# <a name='Bezier_Curve'>Bezier Curve</a>

# <a name='Big_Endian'>Big Endian</a>

# <a name='Cartesian_Coordinate'>Cartesian Coordinate</a>

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

# <a name='Color_Space'>Color Space</a>

# <a name='SkColorSpace'>Class SkColorSpace</a>

<a name='SkColorSpace_MakeSRGBLinear'></a>
## MakeSRGBLinear

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#sk_sp'>sk sp</a>&lt;<a href='#SkColorSpace'>SkColorSpace</a>&gt; <a href='#SkColorSpace_MakeSRGBLinear'>MakeSRGBLinear</a>()
</pre>

---

<a name='SkColorSpace_gammaCloseToSRGB'></a>
## gammaCloseToSRGB

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkColorSpace_gammaCloseToSRGB'>gammaCloseToSRGB</a>() const
</pre>

---

<a name='SkColorSpace_Equals'></a>
## Equals

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkColorSpace_Equals'>Equals</a>(const <a href='#SkColorSpace'>SkColorSpace</a>* src, const <a href='#SkColorSpace'>SkColorSpace</a>* dst)
</pre>

---

# <a name='Coons_Patch'>Coons Patch</a>

# <a name='Core_Graphics'>Core Graphics</a>

# <a name='Core_Text'>Core Text</a>

# <a name='Create_Color_Space_Xform_Canvas'>Create Color Space Xform Canvas</a>

<a name='SkCreateColorSpaceXformCanvas'></a>
## SkCreateColorSpaceXformCanvas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
std::unique_ptr&lt;<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>&gt; SK_API <a href='#SkCreateColorSpaceXformCanvas'>SkCreateColorSpaceXformCanvas</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* target,
                                                     <a href='#sk_sp'>sk sp</a>&lt;<a href='#SkColorSpace'>SkColorSpace</a>&gt; targetCS)
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
SK_API void <a href='#SkDebugf'>SkDebugf</a>(const char format[], ...)
</pre>

---

# <a name='Deferred_Display_List'>Deferred Display List</a>

# <a name='SkDeferredDisplayList'>Class SkDeferredDisplayList</a>

## <a name='Recorder'>Recorder</a>

# <a name='SkDeferredDisplayListRecorder'>Class SkDeferredDisplayListRecorder</a>

# <a name='Descenders'>Descenders</a>

# <a name='Deserial_Procs'>Deserial Procs</a>

# <a name='SkDeserialProcs'>Struct SkDeserialProcs</a>

# <a name='Destructor'>Destructor</a>

# <a name='Device'>Device</a>

# <a name='SkBaseDevice'>Class SkBaseDevice</a>

# <a name='Document'>Document</a>

# <a name='SkDocument'>Class SkDocument</a>

<a name='SkDocument_beginPage'></a>
## beginPage

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='#SkDocument_beginPage'>beginPage</a>(<a href='#SkScalar'>SkScalar</a> width, <a href='#SkScalar'>SkScalar</a> height, const <a href='SkRect_Reference#SkRect'>SkRect</a>* content = NULL)
</pre>

---

## <a name='PDF'>PDF</a>

# <a name='Draw_Layer'>Draw Layer</a>

# <a name='Draw_Looper'>Draw Looper</a>

# <a name='SkDrawLooper'>Class SkDrawLooper</a>

# <a name='Drawable'>Drawable</a>

# <a name='SkDrawable'>Class SkDrawable</a>

<a name='SkDrawable_draw'></a>
## draw

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkDrawable_draw'>draw</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>*, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* = NULL)
</pre>

---

# <a name='Euclidean_Distance'>Euclidean Distance</a>

# <a name='Euclidean_Space'>Euclidean Space</a>

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

# <a name='GrContext'>Class GrContext</a>

<a name='GrContext_flush'></a>
## flush

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#GrContext_flush'>flush</a>()
</pre>

---

# <a name='GPU_Share_Group'>GPU Share Group</a>

# <a name='GPU_Surface'>GPU Surface</a>

# <a name='GPU_Texture'>GPU Texture</a>

# <a name='Glyph'>Glyph</a>

# <a name='Grayscale'>Grayscale</a>

# <a name='HTML_Aqua'>HTML Aqua</a>

# <a name='HTML_Canvas'>HTML Canvas</a>

## <a name='ArcTo'>ArcTo</a>

# <a name='HTML_Fuchsia'>HTML Fuchsia</a>

# <a name='HTML_Gray'>HTML Gray</a>

# <a name='HTML_Green'>HTML Green</a>

# <a name='HTML_Lime'>HTML Lime</a>

# <a name='HTML_Silver'>HTML Silver</a>

# <a name='ISize'>ISize</a>

# <a name='SkISize'>Struct SkISize</a>

# <a name='Image_Filter'>Image Filter</a>

# <a name='SkImageFilter'>Class SkImageFilter</a>

# <a name='Image_Scaling'>Image Scaling</a>

# <a name='Kerning'>Kerning</a>

# <a name='Left_Side_Bearing'>Left Side Bearing</a>

# <a name='Line'>Line</a>

# <a name='Little_Endian'>Little Endian</a>

# <a name='Mask_Alpha'>Mask Alpha</a>

# <a name='Mask_Filter'>Mask Filter</a>

# <a name='SkMaskFilter'>Class SkMaskFilter</a>

# <a name='Meta_Data'>Meta Data</a>

# <a name='SkMetaData'>Class SkMetaData</a>

# <a name='Mip_Map'>Mip Map</a>

# <a name='Multi_Sample_Anti_Aliasing'>Multi Sample Anti Aliasing</a>

# <a name='Nine_Patch'>Nine Patch</a>

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

## <a name='SkPathOp'>Enum SkPathOp</a>

</table>

<a name='Op'></a>
## Op

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool SK_API <a href='#Op'>Op</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& one, const <a href='SkPath_Reference#SkPath'>SkPath</a>& two, <a href='#SkPathOp'>SkPathOp</a> op, <a href='SkPath_Reference#SkPath'>SkPath</a>* result)
</pre>

---

# <a name='Path_Effect'>Path Effect</a>

# <a name='SkPathEffect'>Class SkPathEffect</a>

# <a name='Picture_Recorder'>Picture Recorder</a>

# <a name='SkPictureRecorder'>Class SkPictureRecorder</a>

<a name='SkPictureRecorder_beginRecording'></a>
## beginRecording

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='#SkPictureRecorder_beginRecording'>beginRecording</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& bounds, <a href='#SkBBHFactory'>SkBBHFactory</a>* bbhFactory = NULL,
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
int <a href='#SkPixelRef_width'>width</a>() const
</pre>

---

<a name='SkPixelRef_height'></a>
## height

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPixelRef_height'>height</a>() const
</pre>

---

<a name='SkPixelRef_isImmutable'></a>
## isImmutable

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixelRef_isImmutable'>isImmutable</a>() const
</pre>

---

<a name='SkPixelRef_setImmutable'></a>
## setImmutable

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPixelRef_setImmutable'>setImmutable</a>()
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

## <a name='SkRasterHandleAllocator_Handle'>Typedef SkRasterHandleAllocator::Handle</a>

# <a name='SkRasterHandleAllocator_Rec'>Struct SkRasterHandleAllocator::Rec</a>

<a name='SkRasterHandleAllocator_MakeCanvas'></a>
## MakeCanvas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static std::unique_ptr&lt;<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>&gt; <a href='#SkRasterHandleAllocator_MakeCanvas'>MakeCanvas</a>(std::unique_ptr&lt;<a href='#SkRasterHandleAllocator'>SkRasterHandleAllocator</a>&gt;,
                                            const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>&, const <a href='#SkRasterHandleAllocator_Rec'>Rec</a>* rec = nullptr)
</pre>

---

# <a name='Raster_Surface'>Raster Surface</a>

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
bool <a href='#GrBackendRenderTarget_isValid'>isValid</a>() const
</pre>

---

# <a name='Render_Target'>Render Target</a>

# <a name='GrRenderTarget'>Class GrRenderTarget</a>

# <a name='Right_Side_Bearing'>Right Side Bearing</a>

# <a name='SVG'>SVG</a>

## <a name='Canvas'>Canvas</a>

## <a name='Arc'>Arc</a>

# <a name='SVG_darkgray'>SVG darkgray</a>

# <a name='SVG_lightgray'>SVG lightgray</a>

# <a name='Scalar'>Scalar</a>

## <a name='SkScalar'>Typedef SkScalar</a>

# <a name='Serial_Procs'>Serial Procs</a>

# <a name='SkSerialProcs'>Struct SkSerialProcs</a>

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
static <a href='#sk_sp'>sk sp</a>&lt;<a href='#SkShader'>SkShader</a>&gt; <a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& src, <a href='#SkShader_TileMode'>TileMode</a> tmx, <a href='#SkShader_TileMode'>TileMode</a> tmy,
                                        const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* localMatrix = nullptr)
</pre>

---

<a name='SkShader_MakeCompose'></a>
## MakeCompose

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#sk_sp'>sk sp</a>&lt;<a href='#SkShader'>SkShader</a>&gt; <a href='#SkShader_MakeCompose'>MakeCompose</a>(<a href='#sk_sp'>sk sp</a>&lt;<a href='#SkShader'>SkShader</a>&gt; dst, <a href='#sk_sp'>sk sp</a>&lt;<a href='#SkShader'>SkShader</a>&gt; src, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode,
                                   float lerp = 1)
</pre>

---

# <a name='Size'>Size</a>

# <a name='SkSize'>Struct SkSize</a>

# <a name='Sprite'>Sprite</a>

# <a name='Stream'>Stream</a>

# <a name='SkStream'>Class SkStream</a>

# <a name='String'>String</a>

# <a name='SkString'>Class SkString</a>

# <a name='Subclasses'>Subclasses</a>

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
bool <a href='#GrBackendTexture_isValid'>isValid</a>() const
</pre>

---

# <a name='Transfer_Mode'>Transfer Mode</a>

# <a name='Typeface'>Typeface</a>

# <a name='SkTypeface'>Class SkTypeface</a>

# <a name='UV_Mapping'>UV Mapping</a>

# <a name='Unhinted'>Unhinted</a>

# <a name='Unpremultiply'>Unpremultiply</a>

# <a name='Vertices'>Vertices</a>

# <a name='SkVertices'>Class SkVertices</a>

## <a name='Colors'>Colors</a>

## <a name='Texs'>Texs</a>

# <a name='WStream'>WStream</a>

# <a name='SkWStream'>Class SkWStream</a>

# <a name='Xfermode_Image_Filter'>Xfermode Image Filter</a>

# <a name='SkXfermodeImageFilter'>Class SkXfermodeImageFilter</a>

# <a name='YUV_Component_U'>YUV Component U</a>

# <a name='YUV_Component_V'>YUV Component V</a>

# <a name='YUV_Component_Y'>YUV Component Y</a>
