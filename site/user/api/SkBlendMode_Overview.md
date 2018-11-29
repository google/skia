SkBlendMode Overview
===
Describes how destination <a href='undocumented#Pixel'>pixel</a> is replaced with a combination of itself and
source <a href='undocumented#Pixel'>pixel</a>. <a href='#Blend_Mode'>Blend_Mode</a> may use source, destination, or both. <a href='#Blend_Mode'>Blend_Mode</a> may
operate on each <a href='SkColor_Reference#Color'>Color</a> component independently, or may allow all source <a href='undocumented#Pixel'>pixel</a>
components to contribute to one destination <a href='undocumented#Pixel'>pixel</a> component.

<a href='#Blend_Mode'>Blend_Mode</a> does not use adjacent pixels to determine the outcome.

<a href='#Blend_Mode'>Blend_Mode</a> uses source and read destination <a href='SkColor_Reference#Alpha'>Alpha</a> to determine written
destination <a href='SkColor_Reference#Alpha'>Alpha</a>; both source and destination <a href='SkColor_Reference#Alpha'>Alpha</a> may also affect written
destination <a href='SkColor_Reference#Color'>Color</a> components.

Regardless of how <a href='SkColor_Reference#Alpha'>Alpha</a> is encoded in source and destination <a href='undocumented#Pixel'>pixel</a>, nearly all
<a href='#Image_Info_Color_Type'>Color_Types</a> treat it as ranging from zero to one. And, nearly all <a href='#Blend_Mode'>Blend_Mode</a>
algorithms limit the output so that all results are also zero to one.

Two exceptions are <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kPlus'>kPlus</a> and <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>.

<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kPlus'>kPlus</a> permits computing <a href='SkColor_Reference#Alpha'>Alpha</a> and <a href='SkColor_Reference#Color'>Color</a> component values larger
than one. For <a href='#Image_Info_Color_Type'>Color_Types</a> other than <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>, resulting <a href='SkColor_Reference#Alpha'>Alpha</a>
and component values are clamped to one.

<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> permits values outside the zero to one range. It is up
to the client to ensure that the result is within the range of zero to one,
and therefore well-defined.

<a name='Porter_Duff'></a>

<a href='https://graphics.pixar.com/library/Compositing/paper.pdf'>Compositing Digital Images</a></a> describes <a href='#Blend_Mode_Overview_Porter_Duff'>Porter_Duff</a> modes <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kClear'>kClear</a> through <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kXor'>kXor</a>.

Drawing a <a href='SkBitmap_Reference#Bitmap'>bitmap</a> with transparency using <a href='#Blend_Mode_Overview_Porter_Duff'>Porter_Duff</a> compositing is free to clear
the destination.

![Porter_Duff](https://fiddle.skia.org/i/819903e0bb125385269948474b6c8a84_raster.png "")

Draw geometry with transparency using <a href='#Blend_Mode_Overview_Porter_Duff'>Porter_Duff</a> compositing does not combine
transparent source pixels, leaving the destination outside the geometry untouched.

![Porter_Duff](https://fiddle.skia.org/i/8f320c1e94e77046e00f7e9e843caa27_raster.png "")

<a name='Lighten_Darken'></a>

Modes <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kPlus'>kPlus</a> and <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kScreen'>kScreen</a> use
simple arithmetic to lighten or darken the destination. Modes
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kOverlay'>kOverlay</a> through <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kMultiply'>kMultiply</a> use more complicated
algorithms to lighten or darken; sometimes one mode does both, as described by
<a href='https://en.wikipedia.org/wiki/Blend_modes'>Blend Modes</a></a> .

![Lighten_Darken](https://fiddle.skia.org/i/23a33fa04cdd0204b2490d05e340f87c_raster.png "")

<a name='Modulate_Blend'></a>

<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kModulate'>kModulate</a> is a mashup of <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcATop'>kSrcATop</a> and <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kMultiply'>kMultiply</a>.
It multiplies all components, including <a href='SkColor_Reference#Alpha'>Alpha</a>; unlike <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kMultiply'>kMultiply</a>, if either
source or destination is transparent, result is transparent. <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kModulate'>kModulate</a>
uses <a href='undocumented#Premultiply'>Premultiplied</a> values to compute the product; <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kMultiply'>kMultiply</a> uses <a href='undocumented#Unpremultiply'>Unpremultiplied</a>
values to compute the product.

![Modulate_Blend](https://fiddle.skia.org/i/877f96610ab7638a310432674b04f837_raster.png "")

<a name='Color_Blends'></a>

Modes <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kHue'>kHue</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSaturation'>kSaturation</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kColor'>kColor</a>, and
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kLuminosity'>kLuminosity</a> convert source and destination pixels using all
components <a href='SkColor_Reference#Color'>color</a> information, using
<a href='https://www.w3.org/TR/compositing-1/#blendingnonseparable'>non-separable blend modes</a></a> .

![Color_Blends](https://fiddle.skia.org/i/630fe21aea8369b307231f5bcf8b2d50_raster.png "")

