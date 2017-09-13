undocumented
===

# <a name="Glyph"></a> Glyph

# <a name="Curve"></a> Curve

# <a name="Document"></a> Document

# <a name="SkDocument"></a> Class SkDocument

<a name="SkDocument_beginPage"></a>
## beginPage

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkCanvas* beginPage(SkScalar width, SkScalar height,
                    const SkRect* content = NULL)
</pre>

---

## <a name="PDF"></a> PDF

# <a name="Arc"></a> Arc

# <a name="Rect"></a> Rect

# <a name="SkRect"></a> Struct SkRect

<a name="SkRect_MakeEmpty"></a>
## MakeEmpty

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static constexpr SkRect SK_WARN_UNUSED_RESULT MakeEmpty()
</pre>

---

<a name="SkRect_dump"></a>
## dump

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void dump() const
</pre>

---

<a name="SkRect_dumpHex"></a>
## dumpHex

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void dumpHex() const
</pre>

---

# <a name="Line"></a> Line

# <a name="Region"></a> Region

# <a name="SkRegion"></a> Class SkRegion

# <a name="Device"></a> Device

# <a name="SkBaseDevice"></a> Class SkBaseDevice

# <a name="Vector"></a> Vector

# <a name="SkVector"></a> Struct SkVector

# <a name="Point"></a> Point

# <a name="SkPoint"></a> Struct SkPoint

<a name="SkPoint_equalsWithinTolerance"></a>
## equalsWithinTolerance

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool equalsWithinTolerance(const SkPoint& p) const
</pre>

---

## <a name="Array"></a> Array

# <a name="Patch"></a> Patch

# <a name="Typeface"></a> Typeface

# <a name="SkTypeface"></a> Class SkTypeface

# <a name="Dump_Canvas"></a> Dump Canvas

# <a name="SkDumpCanvas"></a> Class SkDumpCanvas

# <a name="HTML_Canvas"></a> HTML Canvas

## <a name="ArcTo"></a> ArcTo

# <a name="Alias"></a> Alias

# <a name="Anti-alias"></a> Anti-alias

# <a name="BBH_Factory"></a> BBH Factory

# <a name="SkBBHFactory"></a> Class SkBBHFactory

# <a name="Bitmap"></a> Bitmap

# <a name="SkBitmap"></a> Class SkBitmap

## <a name="Row_Bytes"></a> Row Bytes

<a name="SkBitmap_erase"></a>
## erase

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void erase(SkColor c, const SkIRect& area) const
</pre>

---

<a name="SkBitmap_installPixels"></a>
## installPixels

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool installPixels(const SkImageInfo& info, void* pixels, size_t rowBytes)
</pre>

---

<a name="SkBitmap_readPixels"></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                int srcX, int srcY, SkTransferFunctionBehavior behavior) const
</pre>

---

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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int SkColorGetA(color)
</pre>

---

<a name="SkColorGetR"></a>
## SkColorGetR

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int SkColorGetR(color)
</pre>

---

<a name="SkColorGetG"></a>
## SkColorGetG

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int SkColorGetG(color)
</pre>

---

<a name="SkColorGetB"></a>
## SkColorGetB

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int SkColorGetB(color)
</pre>

---

<a name="SkColorSetARGB"></a>
## SkColorSetARGB

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

# <a name="Color_Space"></a> Color Space

# <a name="SkColorSpace"></a> Class SkColorSpace

<a name="SkColorSpace_MakeSRGBLinear"></a>
## MakeSRGBLinear

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp<SkColorSpace> MakeSRGBLinear()
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

# <a name="Data"></a> Data

# <a name="Draw_Filter"></a> Draw Filter

# <a name="SkDrawFilter"></a> Class SkDrawFilter

# <a name="Draw_Layer"></a> Draw Layer

# <a name="Draw_Looper"></a> Draw Looper

# <a name="SkDrawLooper"></a> Class SkDrawLooper

# <a name="Drawable"></a> Drawable

# <a name="SkDrawable"></a> Class SkDrawable

<a name="SkDrawable_draw"></a>
## draw

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void draw(SkCanvas*, const SkMatrix* = NULL)
</pre>

---

# <a name="Raster_Handle_Allocator"></a> Raster Handle Allocator

# <a name="SkRasterHandleAllocator"></a> Class SkRasterHandleAllocator

# <a name="SkRasterHandleAllocator_Rec"></a> Struct SkRasterHandleAllocator::Rec

<a name="SkRasterHandleAllocator_MakeCanvas"></a>
## MakeCanvas

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static std::unique_ptr<SkCanvas> 
                                            MakeCanvas(std::unique_ptr<SkRasterHandleAllocator>,
                                            const SkImageInfo&,
                                            const Rec* rec = nullptr)
</pre>

---

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

# <a name="Left_Side_Bearing"></a> Left Side Bearing

# <a name="Font"></a> Font

## <a name="Advance"></a> Advance

## <a name="Engine"></a> Engine

# <a name="Oval"></a> Oval

# <a name="Font_Manager"></a> Font Manager

# <a name="GPU_Context"></a> GPU Context

# <a name="GPU_Surface"></a> GPU Surface

# <a name="Image"></a> Image

## <a name="Alpha_Type"></a> Alpha Type

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

## <a name="Color_Type"></a> Color Type

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

## <a name="Info"></a> Info

# <a name="SkImageInfo"></a> Struct SkImageInfo

<a name="SkImageInfo_empty_constructor"></a>
## SkImageInfo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkImageInfo()
</pre>

---

<a name="SkImageInfo_makeColorSpace"></a>
## makeColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkImageInfo makeColorSpace(sk_sp<SkColorSpace> cs) const
</pre>

---

<a name="SkImageInfo_minRowBytes"></a>
## minRowBytes

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
size_t minRowBytes() const
</pre>

---

<a name="SkImageInfo_isOpaque"></a>
## isOpaque

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isOpaque() const
</pre>

---

<a name="SkImageInfo_bytesPerPixel"></a>
## bytesPerPixel

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int bytesPerPixel() const
</pre>

---

# <a name="SkImage"></a> Class SkImage

<a name="SkImage_makeShader"></a>
## makeShader

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
sk_sp<SkShader> makeShader(SkShader::TileMode, SkShader::TileMode,
                           const SkMatrix* localMatrix = nullptr) const
</pre>

---

<a name="SkImage_MakeRasterCopy"></a>
## MakeRasterCopy

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp<SkImage> MakeRasterCopy(const SkPixmap&)
</pre>

---

<a name="SkImage_readPixels"></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool readPixels(const SkPixmap& dst, int srcX, int srcY,
                CachingHint = kAllow_CachingHint) const
</pre>

---

<a name="SkImage_scalePixels"></a>
## scalePixels

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool scalePixels(const SkPixmap& dst, SkFilterQuality,
                 CachingHint = kAllow_CachingHint) const
</pre>

---

# <a name="Image_Filter"></a> Image Filter

## <a name="Scaling"></a> Scaling

# <a name="SkImageFilter"></a> Class SkImageFilter

# <a name="Image_Scaling"></a> Image Scaling

# <a name="IRect"></a> IRect

# <a name="SkIRect"></a> Struct SkIRect

<a name="SkIRect_intersect"></a>
## intersect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool intersect(const SkIRect& r)
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

# <a name="Math"></a> Math

<a name="sk_64_isS32"></a>
## sk_64_isS32

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static inline bool sk_64_isS32(int64_t value)
</pre>

---

# <a name="Matrix"></a> Matrix

# <a name="SkMatrix"></a> Struct SkMatrix

# <a name="Nine_Patch"></a> Nine Patch

# <a name="Number_Types"></a> Number Types

### Constants

<table>
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
</table>

# <a name="OS_X"></a> OS X

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

# <a name="Path_Effect"></a> Path Effect

# <a name="SkPathEffect"></a> Class SkPathEffect

# <a name="Path_Measure"></a> Path Measure

# <a name="SkPathMeasure"></a> Class SkPathMeasure

<a name="SkPathMeasure_dump"></a>
## dump

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void dump() const
</pre>

---

# <a name="PathOps"></a> PathOps

<a name="Op"></a>
## Op

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool SK_API Op(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result)
</pre>

---

# <a name="Picture"></a> Picture

## <a name="Recorder"></a> Recorder

# <a name="SkPictureRecorder"></a> Class SkPictureRecorder

<a name="SkPictureRecorder_beginRecording"></a>
## beginRecording

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkCanvas* beginRecording(const SkRect& bounds, SkBBHFactory* bbhFactory = NULL,
                         uint32_t recordFlags = 0)
</pre>

---

# <a name="Pixel"></a> Pixel

## <a name="Storage"></a> Storage

# <a name="PixelRef"></a> PixelRef

# <a name="SkPixelRef"></a> Class SkPixelRef

# <a name="PostScript"></a> PostScript

## <a name="Arct"></a> Arct

# <a name="Premultiply"></a> Premultiply

# <a name="Raster_Engine"></a> Raster Engine

# <a name="Raster_Surface"></a> Raster Surface

# <a name="Rasterizer"></a> Rasterizer

# <a name="SkRasterizer"></a> Class SkRasterizer

## <a name="Layer"></a> Layer

# <a name="Reference_Count"></a> Reference Count

# <a name="sk_sp"></a> Class sk_sp

# <a name="Right_Side_Bearing"></a> Right Side Bearing

# <a name="Round_Rect"></a> Round Rect

# <a name="SkRRect"></a> Class SkRRect

<a name="SkRRect_dump"></a>
## dump

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void dump() const
</pre>

---

<a name="SkRRect_dumpHex"></a>
## dumpHex

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void dumpHex() const
</pre>

---

# <a name="RSXform"></a> RSXform

# <a name="SkRSXform"></a> Struct SkRSXform

# <a name="Shader"></a> Shader

# <a name="SkShader"></a> Class SkShader

## <a name="SkShader_TileMode"></a> Enum SkShader::TileMode

### Constants

<table>
  <tr>
    <td><a name="SkShader_kClamp_TileMode"> <code><strong>SkShader::kClamp_TileMode </strong></code> </a></td><td>0</td><td></td>
  </tr>

</table>

<a name="SkShader_MakeBitmapShader"></a>
## MakeBitmapShader

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp<SkShader> MakeBitmapShader(const SkBitmap& src, TileMode tmx,
                                        TileMode tmy,
                                        const SkMatrix* localMatrix = nullptr)
</pre>

---

## <a name="Gradient"></a> Gradient

# <a name="Sprite"></a> Sprite

# <a name="Stream"></a> Stream

# <a name="SkFlattenable"></a> Class SkFlattenable

# <a name="String"></a> String

# <a name="SkString"></a> Class SkString

# <a name="Supersampling"></a> Supersampling

# <a name="Surface"></a> Surface

# <a name="SkSurface"></a> Class SkSurface

<a name="SkSurface_MakeRasterDirect"></a>
## MakeRasterDirect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp<SkSurface> MakeRasterDirect(const SkImageInfo&, void* pixels,
                                         size_t rowBytes,
                                         const SkSurfaceProps* = nullptr)
</pre>

---

<a name="SkSurface_readPixels"></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool readPixels(const SkPixmap& dst, int srcX, int srcY) ;
</pre>

---

## <a name="Properties"></a> Properties

# <a name="SkSurfaceProps"></a> Class SkSurfaceProps

# <a name="Legacy_Font_Host"></a> Legacy Font Host

## <a name="SkSurfaceProps_InitType"></a> Enum SkSurfaceProps::InitType

### Constants

<table>
  <tr>
    <td><a name="SkSurfaceProps_kLegacyFontHost_InitType"> <code><strong>SkSurfaceProps::kLegacyFontHost_InitType </strong></code> </a></td><td>0</td><td></td>
  </tr>

</table>

# <a name="SVG"></a> SVG

## <a name="Canvas"></a> Canvas

## <a name="Arc"></a> Arc

# <a name="Text"></a> Text

# <a name="Text_Blob"></a> Text Blob

# <a name="SkTextBlob"></a> Class SkTextBlob

# <a name="Unpremultiply"></a> Unpremultiply

# <a name="Vertices"></a> Vertices

## <a name="Colors"></a> Colors

## <a name="Texs"></a> Texs

# <a name="Read_Buffer"></a> Read Buffer

# <a name="SkReadBuffer"></a> Struct SkReadBuffer

# <a name="Write_Buffer"></a> Write Buffer

# <a name="SkWriteBuffer"></a> Struct SkWriteBuffer
