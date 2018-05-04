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

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th><th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkBlendMode_kSrc'> <code>SkBlendMode::kSrc</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkBlendMode_kSrcOver'> <code>SkBlendMode::kSrcOver</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkBlendMode_kPlus'> <code>SkBlendMode::kPlus</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>12</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>




# <a name="Circle"></a> Circle

# <a name="Clip_Op"></a> Clip Op

## <a name="SkClipOp"></a> Enum SkClipOp

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th><th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkClipOp_kDifference'> <code>SkClipOp::kDifference</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkClipOp_kIntersect'> <code>SkClipOp::kIntersect</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>




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

<a name="SkColorSpace_gammaCloseToSRGB"></a>
## gammaCloseToSRGB

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool gammaCloseToSRGB() const
</pre>

---

<a name="SkColorSpace_Equals"></a>
## Equals

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static bool Equals(const SkColorSpace* src, const SkColorSpace* dst)
</pre>

---

## <a name="SkTransferFunctionBehavior"></a> Enum SkTransferFunctionBehavior

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th><th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkTransferFunctionBehavior_kRespect'> <code>SkTransferFunctionBehavior::kRespect</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkTransferFunctionBehavior_kIgnore'> <code>SkTransferFunctionBehavior::kIgnore</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>




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

# <a name="Debug_Canvas"></a> Debug Canvas

# <a name="SkDebugCanvas"></a> Class SkDebugCanvas

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

# <a name="Encoded_Image_Format"></a> Encoded Image Format

## <a name="SkEncodedImageFormat"></a> Enum SkEncodedImageFormat

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th><th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkEncodedImageFormat_kUnknown'> <code>SkEncodedImageFormat::kUnknown</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkEncodedImageFormat_kBMP'> <code>SkEncodedImageFormat::kBMP</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkEncodedImageFormat_kGIF'> <code>SkEncodedImageFormat::kGIF</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkEncodedImageFormat_kICO'> <code>SkEncodedImageFormat::kICO</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkEncodedImageFormat_kJPEG'> <code>SkEncodedImageFormat::kJPEG</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkEncodedImageFormat_kPNG'> <code>SkEncodedImageFormat::kPNG</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkEncodedImageFormat_kWBMP'> <code>SkEncodedImageFormat::kWBMP</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>6</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkEncodedImageFormat_kWEBP'> <code>SkEncodedImageFormat::kWEBP</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>7</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkEncodedImageFormat_kPKM'> <code>SkEncodedImageFormat::kPKM</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkEncodedImageFormat_kKTX'> <code>SkEncodedImageFormat::kKTX</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>9</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkEncodedImageFormat_kASTC'> <code>SkEncodedImageFormat::kASTC</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>10</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkEncodedImageFormat_kDNG'> <code>SkEncodedImageFormat::kDNG</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>11</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkEncodedImageFormat_kHEIF'> <code>SkEncodedImageFormat::kHEIF</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>12</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>




# <a name="Filter_Quality"></a> Filter Quality

## <a name="SkFilterQuality"></a> Enum SkFilterQuality

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th><th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='kNone_SkFilterQuality'> <code>kNone_SkFilterQuality</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='kLow_SkFilterQuality'> <code>kLow_SkFilterQuality</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='kMedium_SkFilterQuality'> <code>kMedium_SkFilterQuality</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='kHigh_SkFilterQuality'> <code>kHigh_SkFilterQuality</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>




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

# <a name="Image_Filter"></a> Image Filter

## <a name="Scaling"></a> Scaling

# <a name="SkImageFilter"></a> Class SkImageFilter

<a name="SkImageFilter_toString"></a>
## toString

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void toString(SkString* str) const
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

# <a name="Mask_Alpha"></a> Mask Alpha

# <a name="Mask_Filter"></a> Mask Filter

# <a name="SkMaskFilter"></a> Class SkMaskFilter

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

## <a name="Scalar"></a> Scalar

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th><th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SK_MinS32FitsInFloat'> <code>SK_MinS32FitsInFloat</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SK_MaxS32FitsInFloat'> <code>SK_MaxS32FitsInFloat</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SK_ScalarMin'> <code>SK_ScalarMin</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SK_ScalarMax'> <code>SK_ScalarMax</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SK_ScalarInfinity'> <code>SK_ScalarInfinity</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SK_ScalarNegativeInfinity'> <code>SK_ScalarNegativeInfinity</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SK_ScalarNaN'> <code>SK_ScalarNaN</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SK_MinS32'> <code>SK_MinS32</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SK_MaxS32'> <code>SK_MaxS32</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SK_MaxSizeT'> <code>SK_MaxSizeT</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>to be written</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
</table>

# <a name="OS_X"></a> OS X

# <a name="Oval"></a> Oval

# <a name="Paint_Defaults"></a> Paint Defaults

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th><th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkPaintDefaults_Flags'> <code>SkPaintDefaults_Flags</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkPaintDefaults_Hinting'> <code>SkPaintDefaults_Hinting</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkPaintDefaults_TextSize'> <code>SkPaintDefaults_TextSize</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>12</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkPaintDefaults_MiterLimit'> <code>SkPaintDefaults_MiterLimit</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>


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

# <a name="RenderTarget"></a> RenderTarget

# <a name="GrBackendRenderTarget"></a> Class GrBackendRenderTarget

<a name="GrBackendRenderTarget_isValid"></a>
## isValid

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isValid() const
</pre>

---

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

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th><th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkShader_kClamp_TileMode'> <code>SkShader::kClamp_TileMode</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkShader_kRepeat_TileMode'> <code>SkShader::kRepeat_TileMode</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkShader_kMirror_TileMode'> <code>SkShader::kMirror_TileMode</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>




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

## <a name="Legacy_Font_Host"></a> Legacy Font Host

## <a name="SkSurfaceProps_InitType"></a> Enum SkSurfaceProps::InitType

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th><th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkSurfaceProps_kLegacyFontHost_InitType'> <code>SkSurfaceProps::kLegacyFontHost_InitType</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>


#
Topic

# <a name="Text"></a> Text

# <a name="Text_Blob"></a> Text Blob

# <a name="SkTextBlob"></a> Class SkTextBlob

# <a name="Texture"></a> Texture

# <a name="GrBackendTexture"></a> Class GrBackendTexture

<a name="GrBackendTexture_isValid"></a>
## isValid

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isValid() const
</pre>

---

# <a name="Transfer_Mode"></a> Transfer Mode

# <a name="Typeface"></a> Typeface

# <a name="SkTypeface"></a> Class SkTypeface

# <a name="Types"></a> Types

## <a name="GrSurfaceOrigin"></a> Enum GrSurfaceOrigin

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th><th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='kBottomLeft_GrSurfaceOrigin'> <code>kBottomLeft_GrSurfaceOrigin</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='kTopLeft_GrSurfaceOrigin'> <code>kTopLeft_GrSurfaceOrigin</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>




## <a name="Budgeted"></a> Budgeted

## <a name="SkBudgeted"></a> Enum SkBudgeted

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th><th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkBudgeted_kNo'> <code>SkBudgeted::kNo</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='SkBudgeted_kYes'> <code>SkBudgeted::kYes</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>




## <a name="GrSemaphoresSubmitted"></a> Enum GrSemaphoresSubmitted

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th><th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='GrSemaphoresSubmitted_kNo'> <code>GrSemaphoresSubmitted::kNo</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='GrSemaphoresSubmitted_kYes'> <code>GrSemaphoresSubmitted::kYes</code> </a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
</td>
  </tr>




# <a name="Unpremultiply"></a> Unpremultiply

# <a name="Vertices"></a> Vertices

## <a name="Colors"></a> Colors

## <a name="Texs"></a> Texs

# <a name="Write_Buffer"></a> Write Buffer

# <a name="SkWriteBuffer"></a> Struct SkWriteBuffer
