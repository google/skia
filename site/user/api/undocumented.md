undocumented
===

# <a name="Alias"></a> Alias

# <a name="Anti-alias"></a> Anti-alias

# <a name="Arc"></a> Arc

# <a name="BBH_Factory"></a> BBH Factory

# <a name="SkBBHFactory"></a> Class SkBBHFactory

# <a name="Backend_Semaphore"></a> Backend Semaphore

# <a name="GrBackendSemaphore"></a> Class GrBackendSemaphore

# <a name="Blend_Mode"></a> Blend Mode

## <a name="SkBlendMode"></a> Enum SkBlendMode

### Constants

<table>
  <tr>
    <td><a name="SkBlendMode_kSrc"> <code><strong>SkBlendMode::kSrc </strong></code> </a></td><td>1</td><td></td>
  </tr>
  <tr>
    <td><a name="SkBlendMode_kSrcOver"> <code><strong>SkBlendMode::kSrcOver </strong></code> </a></td><td>3</td><td></td>
  </tr>
  <tr>
    <td><a name="SkBlendMode_kPlus"> <code><strong>SkBlendMode::kPlus </strong></code> </a></td><td>12</td><td></td>
  </tr>

</table>

# <a name="Circle"></a> Circle

# <a name="Clip_Op"></a> Clip Op

## <a name="SkClipOp"></a> Enum SkClipOp

### Constants

<table>
  <tr>
    <td><a name="SkClipOp_kDifference"> <code><strong>SkClipOp::kDifference </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="SkClipOp_kIntersect"> <code><strong>SkClipOp::kIntersect </strong></code> </a></td><td>1</td><td></td>
  </tr>

</table>

# <a name="Color"></a> Color

<a name="SkColorGetA"></a>
## SkColorGetA

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int SkColorGetA(color)
</pre>

---

<a name="SkColorGetR"></a>
## SkColorGetR

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int SkColorGetR(color)
</pre>

---

<a name="SkColorGetG"></a>
## SkColorGetG

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int SkColorGetG(color)
</pre>

---

<a name="SkColorGetB"></a>
## SkColorGetB

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int SkColorGetB(color)
</pre>

---

<a name="SkColorSetARGB"></a>
## SkColorSetARGB

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int SkColorSetARGB(a, r, g, b)
</pre>

---

### Constants

<table>
  <tr>
    <td><a name="SK_ColorBLACK"> <code><strong>SK_ColorBLACK </strong></code> </a></td><td>0xFF000000 </td><td></td>
  </tr>
  <tr>
    <td><a name="SK_ColorBLUE"> <code><strong>SK_ColorBLUE </strong></code> </a></td><td>0xFF0000FF </td><td></td>
  </tr>
  <tr>
    <td><a name="SK_ColorGREEN"> <code><strong>SK_ColorGREEN </strong></code> </a></td><td>0xFF00FF00 </td><td></td>
  </tr>
  <tr>
    <td><a name="SK_ColorRED"> <code><strong>SK_ColorRED </strong></code> </a></td><td>0xFFFF0000 </td><td></td>
  </tr>
  <tr>
    <td><a name="SK_ColorWHITE"> <code><strong>SK_ColorWHITE </strong></code> </a></td><td>0xFFFFFFFF </td><td></td>
  </tr>
</table>

## <a name="Alpha"></a> Alpha

## <a name="RGB"></a> RGB

## <a name="Red"></a> Red

## <a name="Blue"></a> Blue

## <a name="Green"></a> Green

## <a name="ARGB"></a> ARGB

## <a name="RBG"></a> RBG

## <a name="RGB-565"></a> RGB-565

# <a name="Color_Filter"></a> Color Filter

# <a name="SkColorFilter"></a> Class SkColorFilter

<a name="SkColorFilter_toString"></a>
## toString

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void toString(SkString* str) const
</pre>

---

# <a name="Color_Space"></a> Color Space

# <a name="SkColorSpace"></a> Class SkColorSpace

<a name="SkColorSpace_MakeSRGBLinear"></a>
## MakeSRGBLinear

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkColorSpace&gt; MakeSRGBLinear()
</pre>

---

## <a name="SkTransferFunctionBehavior"></a> Enum SkTransferFunctionBehavior

### Constants

<table>
  <tr>
    <td><a name="SkTransferFunctionBehavior_kRespect"> <code><strong>SkTransferFunctionBehavior::kRespect </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="SkTransferFunctionBehavior_kIgnore"> <code><strong>SkTransferFunctionBehavior::kIgnore </strong></code> </a></td><td>1</td><td></td>
  </tr>

</table>

# <a name="Core_Graphics"></a> Core Graphics

# <a name="Core_Text"></a> Core Text

# <a name="Create_Color_Space_Xform_Canvas"></a> Create Color Space Xform Canvas

<a name="SkCreateColorSpaceXformCanvas"></a>
## SkCreateColorSpaceXformCanvas

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
std::unique_ptr&lt;SkCanvas&gt; SK_API SkCreateColorSpaceXformCanvas(SkCanvas* target,
                                                     sk_sp&lt;SkColorSpace&gt; targetCS)
</pre>

---

# <a name="Curve"></a> Curve

# <a name="Data"></a> Data

# <a name="SkData"></a> Class SkData

# <a name="Debugging"></a> Debugging

<a name="SkDebugf"></a>
## SkDebugf

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SK_API void SkDebugf(const char format[], ...)
</pre>

---

# <a name="Deferred_Display_List"></a> Deferred Display List

# <a name="SkDeferredDisplayList"></a> Class SkDeferredDisplayList

## <a name="Recorder"></a> Recorder

# <a name="SkDeferredDisplayListRecorder"></a> Class SkDeferredDisplayListRecorder

# <a name="Device"></a> Device

# <a name="SkBaseDevice"></a> Class SkBaseDevice

# <a name="Document"></a> Document

# <a name="SkDocument"></a> Class SkDocument

<a name="SkDocument_beginPage"></a>
## beginPage

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkCanvas* beginPage(SkScalar width, SkScalar height, const SkRect* content = NULL)
</pre>

---

## <a name="PDF"></a> PDF

# <a name="Draw_Filter"></a> Draw Filter

# <a name="SkDrawFilter"></a> Class SkDrawFilter

# <a name="Draw_Layer"></a> Draw Layer

# <a name="Draw_Looper"></a> Draw Looper

# <a name="SkDrawLooper"></a> Class SkDrawLooper

# <a name="Drawable"></a> Drawable

# <a name="SkDrawable"></a> Class SkDrawable

<a name="SkDrawable_draw"></a>
## draw

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void draw(SkCanvas*, const SkMatrix* = NULL)
</pre>

---

# <a name="Dump_Canvas"></a> Dump Canvas

# <a name="SkDumpCanvas"></a> Class SkDumpCanvas

# <a name="Encoded_Image_Format"></a> Encoded Image Format

## <a name="SkEncodedImageFormat"></a> Enum SkEncodedImageFormat

### Constants

<table>
  <tr>
    <td><a name="SkEncodedImageFormat_kUnknown"> <code><strong>SkEncodedImageFormat::kUnknown </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="SkEncodedImageFormat_kBMP"> <code><strong>SkEncodedImageFormat::kBMP </strong></code> </a></td><td>1</td><td></td>
  </tr>
  <tr>
    <td><a name="SkEncodedImageFormat_kGIF"> <code><strong>SkEncodedImageFormat::kGIF </strong></code> </a></td><td>2</td><td></td>
  </tr>
  <tr>
    <td><a name="SkEncodedImageFormat_kICO"> <code><strong>SkEncodedImageFormat::kICO </strong></code> </a></td><td>3</td><td></td>
  </tr>
  <tr>
    <td><a name="SkEncodedImageFormat_kJPEG"> <code><strong>SkEncodedImageFormat::kJPEG </strong></code> </a></td><td>4</td><td></td>
  </tr>
  <tr>
    <td><a name="SkEncodedImageFormat_kPNG"> <code><strong>SkEncodedImageFormat::kPNG </strong></code> </a></td><td>5</td><td></td>
  </tr>
  <tr>
    <td><a name="SkEncodedImageFormat_kWBMP"> <code><strong>SkEncodedImageFormat::kWBMP </strong></code> </a></td><td>6</td><td></td>
  </tr>
  <tr>
    <td><a name="SkEncodedImageFormat_kWEBP"> <code><strong>SkEncodedImageFormat::kWEBP </strong></code> </a></td><td>7</td><td></td>
  </tr>
  <tr>
    <td><a name="SkEncodedImageFormat_kPKM"> <code><strong>SkEncodedImageFormat::kPKM </strong></code> </a></td><td>8</td><td></td>
  </tr>
  <tr>
    <td><a name="SkEncodedImageFormat_kKTX"> <code><strong>SkEncodedImageFormat::kKTX </strong></code> </a></td><td>9</td><td></td>
  </tr>
  <tr>
    <td><a name="SkEncodedImageFormat_kASTC"> <code><strong>SkEncodedImageFormat::kASTC </strong></code> </a></td><td>10</td><td></td>
  </tr>
  <tr>
    <td><a name="SkEncodedImageFormat_kDNG"> <code><strong>SkEncodedImageFormat::kDNG </strong></code> </a></td><td>11</td><td></td>
  </tr>
  <tr>
    <td><a name="SkEncodedImageFormat_kHEIF"> <code><strong>SkEncodedImageFormat::kHEIF </strong></code> </a></td><td>12</td><td></td>
  </tr>

</table>

# <a name="Filter_Quality"></a> Filter Quality

## <a name="SkFilterQuality"></a> Enum SkFilterQuality

### Constants

<table>
  <tr>
    <td><a name="kNone_SkFilterQuality"> <code><strong>kNone_SkFilterQuality </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="kLow_SkFilterQuality"> <code><strong>kLow_SkFilterQuality </strong></code> </a></td><td>1</td><td></td>
  </tr>
  <tr>
    <td><a name="kMedium_SkFilterQuality"> <code><strong>kMedium_SkFilterQuality </strong></code> </a></td><td>2</td><td></td>
  </tr>
  <tr>
    <td><a name="kHigh_SkFilterQuality"> <code><strong>kHigh_SkFilterQuality </strong></code> </a></td><td>3</td><td></td>
  </tr>

</table>

## <a name="Nearest_Neighbor"></a> Nearest Neighbor

## <a name="Bilerp"></a> Bilerp

## <a name="MipMap"></a> MipMap

## <a name="BiCubic"></a> BiCubic

# <a name="Font"></a> Font

## <a name="Advance"></a> Advance

## <a name="Engine"></a> Engine

# <a name="Font_Manager"></a> Font Manager

# <a name="GPU_Context"></a> GPU Context

## <a name="Resource_Cache_Limits"></a> Resource Cache Limits

# <a name="GrContext"></a> Class GrContext

<a name="GrContext_flush"></a>
## flush

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void flush()
</pre>

---

# <a name="GPU_Surface"></a> GPU Surface

# <a name="GPU_Texture"></a> GPU Texture

# <a name="Glyph"></a> Glyph

# <a name="HTML_Canvas"></a> HTML Canvas

## <a name="ArcTo"></a> ArcTo

# <a name="ISize"></a> ISize

# <a name="SkISize"></a> Struct SkISize

# <a name="Image_Alpha_Type"></a> Image Alpha Type

## <a name="SkAlphaType"></a> Enum SkAlphaType

### Constants

<table>
  <tr>
    <td><a name="kUnknown_SkAlphaType"> <code><strong>kUnknown_SkAlphaType </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="kOpaque_SkAlphaType"> <code><strong>kOpaque_SkAlphaType </strong></code> </a></td><td>1</td><td></td>
  </tr>
  <tr>
    <td><a name="kPremul_SkAlphaType"> <code><strong>kPremul_SkAlphaType </strong></code> </a></td><td>2</td><td></td>
  </tr>
  <tr>
    <td><a name="kUnpremul_SkAlphaType"> <code><strong>kUnpremul_SkAlphaType </strong></code> </a></td><td>3</td><td></td>
  </tr>

</table>

# <a name="Image_Color_Type"></a> Image Color Type

## <a name="Native_Color_Type"></a> Native Color Type

## <a name="SkColorType"></a> Enum SkColorType

### Constants

<table>
  <tr>
    <td><a name="kUnknown_SkColorType"> <code><strong>kUnknown_SkColorType </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="kAlpha_8_SkColorType"> <code><strong>kAlpha_8_SkColorType </strong></code> </a></td><td>1</td><td></td>
  </tr>
  <tr>
    <td><a name="kRGB_565_SkColorType"> <code><strong>kRGB_565_SkColorType </strong></code> </a></td><td>2</td><td></td>
  </tr>
  <tr>
    <td><a name="kARGB_4444_SkColorType"> <code><strong>kARGB_4444_SkColorType </strong></code> </a></td><td>3</td><td></td>
  </tr>
  <tr>
    <td><a name="kRGBA_8888_SkColorType"> <code><strong>kRGBA_8888_SkColorType </strong></code> </a></td><td>4</td><td></td>
  </tr>
  <tr>
    <td><a name="kBGRA_8888_SkColorType"> <code><strong>kBGRA_8888_SkColorType </strong></code> </a></td><td>5</td><td></td>
  </tr>
  <tr>
    <td><a name="kIndex_8_SkColorType"> <code><strong>kIndex_8_SkColorType </strong></code> </a></td><td>6</td><td></td>
  </tr>
  <tr>
    <td><a name="kGray_8_SkColorType"> <code><strong>kGray_8_SkColorType </strong></code> </a></td><td>7</td><td></td>
  </tr>
  <tr>
    <td><a name="kRGBA_F16_SkColorType"> <code><strong>kRGBA_F16_SkColorType </strong></code> </a></td><td>8</td><td></td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="kN32_SkColorType"> <code><strong>kN32_SkColorType </strong></code> </a></td><td>4</td><td></td>
  </tr>

</table>

# <a name="Image_Filter"></a> Image Filter

## <a name="Scaling"></a> Scaling

# <a name="SkImageFilter"></a> Class SkImageFilter

<a name="SkImageFilter_toString"></a>
## toString

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void toString(SkString* str) const
</pre>

---

# <a name="Image_Info"></a> Image Info

# <a name="SkImageInfo"></a> Struct SkImageInfo

<a name="SkImageInfo_empty_constructor"></a>
## SkImageInfo

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkImageInfo()
</pre>

---

<a name="SkImageInfo_MakeN32Premul"></a>
## MakeN32Premul

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeN32Premul(int width, int height, sk_sp&lt;SkColorSpace&gt; cs = nullptr)
</pre>

---

<a name="SkImageInfo_makeColorSpace"></a>
## makeColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkImageInfo makeColorSpace(sk_sp&lt;SkColorSpace&gt; cs) const
</pre>

---

<a name="SkImageInfo_minRowBytes"></a>
## minRowBytes

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t minRowBytes() const
</pre>

---

<a name="SkImageInfo_isOpaque"></a>
## isOpaque

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isOpaque() const
</pre>

---

<a name="SkImageInfo_bytesPerPixel"></a>
## bytesPerPixel

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int bytesPerPixel() const
</pre>

---

<a name="SkImageInfo_height"></a>
## height

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int height() const
</pre>

---

<a name="SkImageInfo_width"></a>
## width

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int width() const
</pre>

---

<a name="SkImageInfo_colorType"></a>
## colorType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColorType colorType() const
</pre>

---

<a name="SkImageInfo_alphaType"></a>
## alphaType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkAlphaType alphaType() const
</pre>

---

<a name="SkImageInfo_colorSpace"></a>
## colorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColorSpace* colorSpace() const
</pre>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isOpaque() const
</pre>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t minRowBytes() const
</pre>

---

<a name="SkImageInfo_computeByteSize"></a>
## computeByteSize

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t computeByteSize(size_t rowBytes) const
</pre>

---

<a name="SkImageInfo_validate"></a>
## validate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void validate() const
</pre>

---

# <a name="Image_Scaling"></a> Image Scaling

# <a name="Left_Side_Bearing"></a> Left Side Bearing

# <a name="Line"></a> Line

# <a name="Malloc_Pixel_Ref"></a> Malloc Pixel Ref

# <a name="SkMallocPixelRef"></a> Class SkMallocPixelRef

<a name="SkMallocPixelRef_MakeZeroed"></a>
## MakeZeroed

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkPixelRef&gt; MakeZeroed(const SkImageInfo&, size_t rowBytes)
</pre>

---

<a name="SkMallocPixelRef_MakeAllocate"></a>
## MakeAllocate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkPixelRef&gt; MakeAllocate(const SkImageInfo&, size_t rowBytes)
</pre>

---

# <a name="Mask"></a> Mask

# <a name="SkMask"></a> Class SkMask

## <a name="SkMask_Format"></a> Enum SkMask::Format

### Constants

<table>
  <tr>
    <td><a name="SkMask_kBW_Format"> <code><strong>SkMask::kBW_Format </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="SkMask_kA8_Format"> <code><strong>SkMask::kA8_Format </strong></code> </a></td><td>1</td><td></td>
  </tr>
  <tr>
    <td><a name="SkMask_k3D_Format"> <code><strong>SkMask::k3D_Format </strong></code> </a></td><td>2</td><td></td>
  </tr>
  <tr>
    <td><a name="SkMask_kARGB32_Format"> <code><strong>SkMask::kARGB32_Format </strong></code> </a></td><td>3</td><td></td>
  </tr>
k  <tr>
    <td><a name="SkMask_LCD16_Format"> <code><strong>SkMask::LCD16_Format </strong></code> </a></td><td>4</td><td></td>
  </tr>

</table>

# <a name="Mask_Alpha"></a> Mask Alpha

# <a name="Mask_Filter"></a> Mask Filter

# <a name="SkMaskFilter"></a> Class SkMaskFilter

<a name="SkMaskFilter_filterMask"></a>
## filterMask

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
virtual bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&, SkIPoint* margin) const
</pre>

---

<a name="SkMaskFilter_toString"></a>
## toString

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void toString(SkString* str) const
</pre>

---

# <a name="Math"></a> Math

<a name="sk_64_isS32"></a>
## sk_64_isS32

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static inline bool sk_64_isS32(int64_t value)
</pre>

---

<a name="SkIntToScalar"></a>
## SkIntToScalar

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIntToScalar(x)
</pre>

---

<a name="SkScalarRoundToInt"></a>
## SkScalarRoundToInt

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalarRoundToInt(x)
</pre>

---

<a name="SkScalarFloorToInt"></a>
## SkScalarFloorToInt

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalarFloorToInt(x)
</pre>

---

<a name="SkScalarCeilToInt"></a>
## SkScalarCeilToInt

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalarCeilToInt(x)
</pre>

---

<a name="SkScalarFloorToScalar"></a>
## SkScalarFloorToScalar

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalarFloorToScalar(x)
</pre>

---

<a name="SkScalarCeilToScalar"></a>
## SkScalarCeilToScalar

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalarCeilToScalar(x)
</pre>

---

<a name="SkScalarIsFinite"></a>
## SkScalarIsFinite

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalarIsFinite(x)
</pre>

---

<a name="SkScalarIsNaN"></a>
## SkScalarIsNaN

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalarIsNaN(x)
</pre>

---

<a name="SkTFitsIn"></a>
## SkTFitsIn

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
template &lt;typename D, typename S&gt; inline bool SkTFitsIn(S s)
</pre>

---

# <a name="Meta_Data"></a> Meta Data

# <a name="SkMetaData"></a> Class SkMetaData

# <a name="Mip_Map"></a> Mip Map

# <a name="Nine_Patch"></a> Nine Patch

# <a name="Number_Types"></a> Number Types

# <a name="Scalar"></a> Scalar

### Constants

<table>
  <tr>
    <td><a name="SK_MinS32FitsInFloat"> <code><strong>SK_MinS32FitsInFloat </strong></code> </a></td><td>to be written</td><td></td>
  </tr>
  <tr>
    <td><a name="SK_MaxS32FitsInFloat"> <code><strong>SK_MaxS32FitsInFloat </strong></code> </a></td><td>to be written</td><td></td>
  </tr>
  <tr>
    <td><a name="SK_ScalarMin"> <code><strong>SK_ScalarMin </strong></code> </a></td><td>to be written</td><td></td>
  </tr>
  <tr>
    <td><a name="SK_ScalarMax"> <code><strong>SK_ScalarMax </strong></code> </a></td><td>to be written</td><td></td>
  </tr>
  <tr>
    <td><a name="SK_ScalarInfinity"> <code><strong>SK_ScalarInfinity </strong></code> </a></td><td>to be written</td><td></td>
  </tr>
  <tr>
    <td><a name="SK_ScalarNegativeInfinity"> <code><strong>SK_ScalarNegativeInfinity </strong></code> </a></td><td>to be written</td><td></td>
  </tr>
  <tr>
    <td><a name="SK_ScalarNaN"> <code><strong>SK_ScalarNaN </strong></code> </a></td><td>to be written</td><td></td>
  </tr>
  <tr>
    <td><a name="SK_MinS32"> <code><strong>SK_MinS32 </strong></code> </a></td><td>to be written</td><td></td>
  </tr>
  <tr>
    <td><a name="SK_MaxS32"> <code><strong>SK_MaxS32 </strong></code> </a></td><td>to be written</td><td></td>
  </tr>
</table>

# <a name="OS_X"></a> OS X

# <a name="Oval"></a> Oval

# <a name="Paint_Defaults"></a> Paint Defaults

### Constants

<table>
  <tr>
    <td><a name="SkPaintDefaults_Flags"> <code><strong>SkPaintDefaults_Flags </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPaintDefaults_Hinting"> <code><strong>SkPaintDefaults_Hinting </strong></code> </a></td><td>2</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPaintDefaults_TextSize"> <code><strong>SkPaintDefaults_TextSize </strong></code> </a></td><td>12</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPaintDefaults_MiterLimit"> <code><strong>SkPaintDefaults_MiterLimit </strong></code> </a></td><td>4</td><td></td>
  </tr>
</table>

# <a name="Patch"></a> Patch

# <a name="PathOps"></a> PathOps

<a name="Op"></a>
## Op

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_API Op(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result)
</pre>

---

# <a name="Path_Effect"></a> Path Effect

# <a name="SkPathEffect"></a> Class SkPathEffect

<a name="SkPathEffect_toString"></a>
## toString

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void toString(SkString* str) const
</pre>

---

# <a name="Path_Measure"></a> Path Measure

# <a name="SkPathMeasure"></a> Class SkPathMeasure

<a name="SkPathMeasure_dump"></a>
## dump

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void dump() const
</pre>

---

# <a name="Picture"></a> Picture

# <a name="SkPicture"></a> Class SkPicture

<a name="SkPicture_playback"></a>
## playback

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
virtual void playback(SkCanvas*, AbortCallback* = nullptr) const = 0
</pre>

---

## <a name="Recorder"></a> Recorder

# <a name="SkPictureRecorder"></a> Class SkPictureRecorder

<a name="SkPictureRecorder_beginRecording"></a>
## beginRecording

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkCanvas* beginRecording(const SkRect& bounds, SkBBHFactory* bbhFactory = NULL,
                         uint32_t recordFlags = 0)
</pre>

---

# <a name="Pixel"></a> Pixel

## <a name="Storage"></a> Storage

# <a name="Pixel_Ref"></a> Pixel Ref

# <a name="SkPixelRef"></a> Class SkPixelRef

<a name="SkPixelRef_width"></a>
## width

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int width() const
</pre>

---

<a name="SkPixelRef_height"></a>
## height

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int height() const
</pre>

---

<a name="SkPixelRef_isImmutable"></a>
## isImmutable

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isImmutable() const
</pre>

---

<a name="SkPixelRef_setImmutable"></a>
## setImmutable

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setImmutable()
</pre>

---

# <a name="Pixel_Serializer"></a> Pixel Serializer

# <a name="SkPixelSerializer"></a> Class SkPixelSerializer

# <a name="Point3"></a> Point3

# <a name="SkPoint3"></a> Struct SkPoint3

# <a name="PostScript"></a> PostScript

## <a name="Arct"></a> Arct

# <a name="Premultiply"></a> Premultiply

# <a name="RSXform"></a> RSXform

# <a name="SkRSXform"></a> Struct SkRSXform

# <a name="Raster_Bitmap"></a> Raster Bitmap

# <a name="Raster_Engine"></a> Raster Engine

# <a name="Raster_Handle_Allocator"></a> Raster Handle Allocator

# <a name="SkRasterHandleAllocator"></a> Class SkRasterHandleAllocator

# <a name="SkRasterHandleAllocator_Rec"></a> Struct SkRasterHandleAllocator::Rec

<a name="SkRasterHandleAllocator_MakeCanvas"></a>
## MakeCanvas

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static std::unique_ptr&lt;SkCanvas&gt; MakeCanvas(std::unique_ptr&lt;SkRasterHandleAllocator&gt;,
                                            const SkImageInfo&, const Rec* rec = nullptr)
</pre>

---

# <a name="Raster_Surface"></a> Raster Surface

# <a name="Rasterizer"></a> Rasterizer

# <a name="SkRasterizer"></a> Class SkRasterizer

## <a name="Layer"></a> Layer

# <a name="Read_Buffer"></a> Read Buffer

# <a name="SkReadBuffer"></a> Struct SkReadBuffer

# <a name="Reference_Count"></a> Reference Count

# <a name="SkRefCnt"></a> Class SkRefCnt

# <a name="sk_sp"></a> Class sk_sp

# <a name="Region"></a> Region

# <a name="SkRegion"></a> Class SkRegion

# <a name="Render_Target"></a> Render Target

# <a name="GrRenderTarget"></a> Class GrRenderTarget

# <a name="Right_Side_Bearing"></a> Right Side Bearing

# <a name="Round_Rect"></a> Round Rect

# <a name="SkRRect"></a> Class SkRRect

<a name="SkRRect_dump"></a>
## dump

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void dump() const
</pre>

---

<a name="SkRRect_dumpHex"></a>
## dumpHex

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void dumpHex() const
</pre>

---

# <a name="SVG"></a> SVG

## <a name="Canvas"></a> Canvas

## <a name="Arc"></a> Arc

# <a name="Shader"></a> Shader

# <a name="SkShader"></a> Class SkShader

## <a name="SkShader_TileMode"></a> Enum SkShader::TileMode

### Constants

<table>
  <tr>
    <td><a name="SkShader_kClamp_TileMode"> <code><strong>SkShader::kClamp_TileMode </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="SkShader_kRepeat_TileMode"> <code><strong>SkShader::kRepeat_TileMode </strong></code> </a></td><td>1</td><td></td>
  </tr>
  <tr>
    <td><a name="SkShader_kMirror_TileMode"> <code><strong>SkShader::kMirror_TileMode </strong></code> </a></td><td>2</td><td></td>
  </tr>

</table>

<a name="SkShader_MakeBitmapShader"></a>
## MakeBitmapShader

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkShader&gt; MakeBitmapShader(const SkBitmap& src, TileMode tmx, TileMode tmy,
                                        const SkMatrix* localMatrix = nullptr)
</pre>

---

## <a name="Gradient"></a> Gradient

# <a name="Size"></a> Size

# <a name="SkSize"></a> Struct SkSize

# <a name="Sprite"></a> Sprite

# <a name="Stream"></a> Stream

# <a name="SkStream"></a> Class SkStream

# <a name="String"></a> String

# <a name="SkString"></a> Class SkString

# <a name="Supersampling"></a> Supersampling

# <a name="Surface_Characterization"></a> Surface Characterization

# <a name="SkSurfaceCharacterization"></a> Class SkSurfaceCharacterization

# <a name="Surface_Properties"></a> Surface Properties

# <a name="SkSurfaceProps"></a> Class SkSurfaceProps

# <a name="Legacy_Font_Host"></a> Legacy Font Host

## <a name="SkSurfaceProps_InitType"></a> Enum SkSurfaceProps::InitType

### Constants

<table>
  <tr>
    <td><a name="SkSurfaceProps_kLegacyFontHost_InitType"> <code><strong>SkSurfaceProps::kLegacyFontHost_InitType </strong></code> </a></td><td>0</td><td></td>
  </tr>

</table>

# <a name="Text"></a> Text

# <a name="Text_Blob"></a> Text Blob

# <a name="SkTextBlob"></a> Class SkTextBlob

# <a name="Texture"></a> Texture

# <a name="GrBackendTexture"></a> Class GrBackendTexture

# <a name="Typeface"></a> Typeface

# <a name="SkTypeface"></a> Class SkTypeface

# <a name="Types"></a> Types

## <a name="GrSurfaceOrigin"></a> Enum GrSurfaceOrigin

### Constants

<table>
  <tr>
    <td><a name="kBottomLeft_GrSurfaceOrigin"> <code><strong>kBottomLeft_GrSurfaceOrigin </strong></code> </a></td><td>0 </td><td></td>
  </tr>
  <tr>
    <td><a name="kTopLeft_GrSurfaceOrigin"> <code><strong>kTopLeft_GrSurfaceOrigin </strong></code> </a></td><td>1</td><td></td>
  </tr>

</table>

## <a name="Budgeted"></a> Budgeted

## <a name="SkBudgeted"></a> Enum SkBudgeted

### Constants

<table>
  <tr>
    <td><a name="SkBudgeted_kNo"> <code><strong>SkBudgeted::kNo </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="SkBudgeted_kYes"> <code><strong>SkBudgeted::kYes </strong></code> </a></td><td>1</td><td></td>
  </tr>

</table>

## <a name="GrSemaphoresSubmitted"></a> Enum GrSemaphoresSubmitted

### Constants

<table>
  <tr>
    <td><a name="GrSemaphoresSubmitted_kNo"> <code><strong>GrSemaphoresSubmitted::kNo </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="GrSemaphoresSubmitted_kYes"> <code><strong>GrSemaphoresSubmitted::kYes </strong></code> </a></td><td>1</td><td></td>
  </tr>

</table>

# <a name="Unpremultiply"></a> Unpremultiply

# <a name="Vertices"></a> Vertices

## <a name="Colors"></a> Colors

## <a name="Texs"></a> Texs

# <a name="Write_Buffer"></a> Write Buffer

# <a name="SkWriteBuffer"></a> Struct SkWriteBuffer
