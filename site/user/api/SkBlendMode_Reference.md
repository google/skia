SkBlendMode Reference
===

# <a name='Blend_Mode'>Blend Mode</a>

## Overview

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constant'>Constants</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>enum and enum class, and their const values</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member_Function'>Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>global and class member functions</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Related_Function'>Related Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>similar member functions grouped together</td>
  </tr>
</table>


## <a name='Member_Function'>Member Function</a>


SkBlendMode member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_Name'>SkBlendMode Name</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns mode as C string</td>
  </tr>
</table>

## <a name='Constant'>Constant</a>


SkBlendMode related constants are defined by <code>enum</code>, <code>enum class</code>,  <code>#define</code>, <code>const</code>, and <code>constexpr</code>.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode'>SkBlendMode</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>algorithm combining source and destination pixels</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kClear'>SkBlendMode::kClear</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>replaces destination with zero: fully transparent</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kColor'>SkBlendMode::kColor</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>hue and saturation of source with luminosity of destination</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kColorBurn'>SkBlendMode::kColorBurn</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>darken destination to reflect source</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kColorDodge'>SkBlendMode::kColorDodge</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>brighten destination to reflect source</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kDarken'>SkBlendMode::kDarken</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>darker of source and destination</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kDifference'>SkBlendMode::kDifference</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>subtract darker from lighter with higher contrast</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kDst'>SkBlendMode::kDst</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>preserves destination</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kDstATop'>SkBlendMode::kDstATop</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>destination inside source blended with source</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kDstIn'>SkBlendMode::kDstIn</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>destination trimmed by source</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kDstOut'>SkBlendMode::kDstOut</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>destination trimmed outside source</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kDstOver'>SkBlendMode::kDstOver</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>destination over source</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kExclusion'>SkBlendMode::kExclusion</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>subtract darker from lighter with lower contrast</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kHardLight'>SkBlendMode::kHardLight</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>multiply or screen, depending on source</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kHue'>SkBlendMode::kHue</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>hue of source with saturation and luminosity of destination</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kLastCoeffMode'>SkBlendMode::kLastCoeffMode</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>last <a href='#Porter_Duff'>Porter Duff</a> blend mode</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kLastMode'>SkBlendMode::kLastMode</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>last valid value</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kLastSeparableMode'>SkBlendMode::kLastSeparableMode</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>last blend mode operating separately on components</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kLighten'>SkBlendMode::kLighten</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>lighter of source and destination</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kLuminosity'>SkBlendMode::kLuminosity</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>luminosity of source with hue and saturation of destination</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kModulate'>SkBlendMode::kModulate</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>product of <a href='undocumented#Premultiply'>Premultiplied</a> colors; darkens destination</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>multiply source with destination, darkening image</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kOverlay'>SkBlendMode::kOverlay</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>multiply or screen, depending on destination</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kPlus'>SkBlendMode::kPlus</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sum of colors</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kSaturation'>SkBlendMode::kSaturation</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>saturation of source with hue and luminosity of destination</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kScreen'>SkBlendMode::kScreen</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>multiply inverse of pixels, inverting result; brightens destination</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kSoftLight'>SkBlendMode::kSoftLight</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>lighten or darken, depending on source</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kSrc'>SkBlendMode::kSrc</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>replaces destination</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kSrcATop'>SkBlendMode::kSrcATop</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>source inside destination blended with destination</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kSrcIn'>SkBlendMode::kSrcIn</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>source trimmed inside destination</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kSrcOut'>SkBlendMode::kSrcOut</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>source trimmed outside destination</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>source over destination</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkBlendMode_kXor'>SkBlendMode::kXor</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>each of source and destination trimmed outside the other</td>
  </tr>
</table>

Describes how destination pixel is replaced with a combination of itself and
source pixel. <a href='#Blend_Mode'>Blend Mode</a> may use source, destination, or both. <a href='#Blend_Mode'>Blend Mode</a> may
operate on each <a href='#Color'>Color</a> component independently, or may allow all source pixel
components to contribute to one destination pixel component.

<a href='#Blend_Mode'>Blend Mode</a> does not use adjacent pixels to determine the outcome.

<a href='#Blend_Mode'>Blend Mode</a> uses source and read destination <a href='SkColor_Reference#Alpha'>Alpha</a> to determine written
destination <a href='SkColor_Reference#Alpha'>Alpha</a>; both source and destination <a href='SkColor_Reference#Alpha'>Alpha</a> may also affect written
destination <a href='#Color'>Color</a> components.

Regardless of how <a href='SkColor_Reference#Alpha'>Alpha</a> is encoded in source and destination pixel, nearly all
<a href='SkImageInfo_Reference#Color_Type'>Color Types</a> treat it as ranging from zero to one. And, nearly all <a href='#Blend_Mode'>Blend Mode</a>
algorithms limit the output so that all results are also zero to one.

Two exceptions are <a href='#SkBlendMode_kPlus'>SkBlendMode::kPlus</a> and <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>.

<a href='#SkBlendMode_kPlus'>SkBlendMode::kPlus</a> permits computing <a href='SkColor_Reference#Alpha'>Alpha</a> and <a href='#Color'>Color</a> component values larger
than one. For <a href='SkImageInfo_Reference#Color_Type'>Color Types</a> other than <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>, resulting <a href='SkColor_Reference#Alpha'>Alpha</a>
and component values are clamped to one.

<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> permits values outside the zero to one range. It is up
to the client to ensure that the result is within the range of zero to one,
and therefore well-defined.

## <a name='Porter_Duff'>Porter Duff</a>

<a href='https://graphics.pixar.com/library/Compositing/paper.pdf'>Compositing Digital Images</a></a> describes <a href='#Porter_Duff'>Porter Duff</a> modes <a href='#SkBlendMode_kClear'>SkBlendMode::kClear</a> through <a href='#SkBlendMode_kXor'>SkBlendMode::kXor</a>.

Drawing a bitmap with transparency using <a href='#Porter_Duff'>Porter Duff</a> compositing is free to clear
the destination.

![Porter_Duff](https://fiddle.skia.org/i/8c27fb2a58f63505cffa74c1c79e16ba_raster.png "")

Draw geometry with transparency using <a href='#Porter_Duff'>Porter Duff</a> compositing does not combine
transparent source pixels, leaving the destination outside the geometry untouched.

![Porter_Duff](https://fiddle.skia.org/i/50ebbb0162bbf60524a196236d66c915_raster.png "")

## <a name='Lighten_Darken'>Lighten Darken</a>

Modes <a href='#SkBlendMode_kPlus'>SkBlendMode::kPlus</a> and <a href='#SkBlendMode_kScreen'>SkBlendMode::kScreen</a> use
simple arithmetic to lighten or darken the destination. Modes
<a href='#SkBlendMode_kOverlay'>SkBlendMode::kOverlay</a> through <a href='#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a> use more complicated
algorithms to lighten or darken; sometimes one mode does both, as described by <a href='https://en.wikipedia.org/wiki/Blend_modes'>Blend Modes</a></a> .

![Lighten_Darken](https://fiddle.skia.org/i/8e04f89252632da0fffe713f07f2296f_raster.png "")

## <a name='Modulate_Blend'>Modulate Blend</a>

<a href='#SkBlendMode_kModulate'>SkBlendMode::kModulate</a> is a mashup of <a href='#SkBlendMode_kSrcATop'>SkBlendMode::kSrcATop</a> and <a href='#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a>.
It multiplies all components, including <a href='SkColor_Reference#Alpha'>Alpha</a>; unlike <a href='#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a>, if either
source or destination is transparent, result is transparent. <a href='#SkBlendMode_kModulate'>SkBlendMode::kModulate</a>
uses <a href='undocumented#Premultiply'>Premultiplied</a> values to compute the product; <a href='#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a> uses <a href='undocumented#Unpremultiply'>Unpremultiplied</a>
values to compute the product.

![Modulate_Blend](https://fiddle.skia.org/i/d8abdd8fb56f9e69342d745d425c4a17_raster.png "")

## <a name='Color_Blends'>Color Blends</a>

Modes <a href='#SkBlendMode_kHue'>SkBlendMode::kHue</a>, <a href='#SkBlendMode_kSaturation'>SkBlendMode::kSaturation</a>, <a href='#SkBlendMode_kColor'>SkBlendMode::kColor</a>, and
<a href='#SkBlendMode_kLuminosity'>SkBlendMode::kLuminosity</a> convert source and destination pixels using all
components color information, using <a href='https://www.w3.org/TR/compositing-1/#blendingnonseparable'>non-separable blend modes</a></a> .

![Color_Blends](https://fiddle.skia.org/i/03710c1770728da885fa4ac24a19d5d1_raster.png "")

## <a name='SkBlendMode'>Enum SkBlendMode</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum class <a href='#SkBlendMode'>SkBlendMode</a> {
        <a href='#SkBlendMode_kClear'>kClear</a>,
        <a href='#SkBlendMode_kSrc'>kSrc</a>,
        <a href='#SkBlendMode_kDst'>kDst</a>,
        <a href='#SkBlendMode_kSrcOver'>kSrcOver</a>,
        <a href='#SkBlendMode_kDstOver'>kDstOver</a>,
        <a href='#SkBlendMode_kSrcIn'>kSrcIn</a>,
        <a href='#SkBlendMode_kDstIn'>kDstIn</a>,
        <a href='#SkBlendMode_kSrcOut'>kSrcOut</a>,
        <a href='#SkBlendMode_kDstOut'>kDstOut</a>,
        <a href='#SkBlendMode_kSrcATop'>kSrcATop</a>,
        <a href='#SkBlendMode_kDstATop'>kDstATop</a>,
        <a href='#SkBlendMode_kXor'>kXor</a>,
        <a href='#SkBlendMode_kPlus'>kPlus</a>,
        <a href='#SkBlendMode_kModulate'>kModulate</a>,
        <a href='#SkBlendMode_kScreen'>kScreen</a>,
        <a href='#SkBlendMode_kLastCoeffMode'>kLastCoeffMode</a> = <a href='#SkBlendMode_kScreen'>kScreen</a>,
        <a href='#SkBlendMode_kOverlay'>kOverlay</a>,
        <a href='#SkBlendMode_kDarken'>kDarken</a>,
        <a href='#SkBlendMode_kLighten'>kLighten</a>,
        <a href='#SkBlendMode_kColorDodge'>kColorDodge</a>,
        <a href='#SkBlendMode_kColorBurn'>kColorBurn</a>,
        <a href='#SkBlendMode_kHardLight'>kHardLight</a>,
        <a href='#SkBlendMode_kSoftLight'>kSoftLight</a>,
        <a href='#SkBlendMode_kDifference'>kDifference</a>,
        <a href='#SkBlendMode_kExclusion'>kExclusion</a>,
        <a href='#SkBlendMode_kMultiply'>kMultiply</a>,
        <a href='#SkBlendMode_kLastSeparableMode'>kLastSeparableMode</a> = <a href='#SkBlendMode_kMultiply'>kMultiply</a>,
        <a href='#SkBlendMode_kHue'>kHue</a>,
        <a href='#SkBlendMode_kSaturation'>kSaturation</a>,
        <a href='#SkBlendMode_kColor'>kColor</a>,
        <a href='#SkBlendMode_kLuminosity'>kLuminosity</a>,
        <a href='#SkBlendMode_kLastMode'>kLastMode</a> = <a href='#SkBlendMode_kLuminosity'>kLuminosity</a>,
    };
</pre>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kClear'><code>SkBlendMode::kClear</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Clear'>Clear</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with <a href='SkColor_Reference#Alpha'>Alpha</a> and <a href='#Color'>Color</a> components set to zero;
a fully transparent pixel.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrc'><code>SkBlendMode::kSrc</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Src'>Src</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with source. Destination alpha and color component values
are ignored.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDst'><code>SkBlendMode::kDst</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Dst'>Dst</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Preserves destination, ignoring source. Drawing with <a href='SkPaint_Reference#Paint'>Paint</a> set to <a href='#SkBlendMode_kDst'>kDst</a> has
no effect.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrcOver'><code>SkBlendMode::kSrcOver</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Src_Over'>Src&nbsp;Over</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with source blended with destination. If source is opaque,
replaces destination with source. Used as the default <a href='#Blend_Mode'>Blend Mode</a> for <a href='SkPaint_Reference#SkPaint'>SkPaint</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDstOver'><code>SkBlendMode::kDstOver</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Dst_Over'>Dst&nbsp;Over</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with destination blended with source. If destination is opaque,
has no effect.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrcIn'><code>SkBlendMode::kSrcIn</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Src_In'>Src&nbsp;In</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with source using destination opacity.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDstIn'><code>SkBlendMode::kDstIn</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>6</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Dst_In'>Dst&nbsp;In</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Scales destination opacity by source opacity.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrcOut'><code>SkBlendMode::kSrcOut</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>7</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Src_Out'>Src&nbsp;Out</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with source using the inverse of destination opacity,
drawing source fully where destination opacity is zero.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDstOut'><code>SkBlendMode::kDstOut</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Dst_Out'>Dst&nbsp;Out</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination opacity with inverse of source opacity. If source is
transparent, has no effect.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrcATop'><code>SkBlendMode::kSrcATop</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>9</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Src_Atop'>Src&nbsp;Atop</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Blends destination with source using read destination opacity.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDstATop'><code>SkBlendMode::kDstATop</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>10</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Dst_Atop'>Dst&nbsp;Atop</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Blends destination with source using source opacity.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kXor'><code>SkBlendMode::kXor</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>11</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Xor'>Xor</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Blends destination by exchanging transparency of the source and destination.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kPlus'><code>SkBlendMode::kPlus</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>12</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Plus'>Plus</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with source and destination added together.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kModulate'><code>SkBlendMode::kModulate</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>13</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Modulate'>Modulate</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with source and destination multiplied together.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kScreen'><code>SkBlendMode::kScreen</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>14</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Screen'>Screen</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with inverted source and destination multiplied together.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kLastCoeffMode'><code>SkBlendMode::kLastCoeffMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>14</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
last Porter_Duff blend mode</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kOverlay'><code>SkBlendMode::kOverlay</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>15</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Overlay'>Overlay</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with multiply or screen, depending on destination.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDarken'><code>SkBlendMode::kDarken</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>16</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Darken'>Darken</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with darker of source and destination.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kLighten'><code>SkBlendMode::kLighten</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>17</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Lighten'>Lighten</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with lighter of source and destination.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kColorDodge'><code>SkBlendMode::kColorDodge</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>18</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Dodge'>Color&nbsp;Dodge</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Makes destination brighter to reflect source.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kColorBurn'><code>SkBlendMode::kColorBurn</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>19</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Burn'>Color&nbsp;Burn</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Makes destination darker to reflect source.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kHardLight'><code>SkBlendMode::kHardLight</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>20</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Hard_Light'>Hard&nbsp;Light</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Makes destination lighter or darker, depending on source.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSoftLight'><code>SkBlendMode::kSoftLight</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>21</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Soft_Light'>Soft&nbsp;Light</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Makes destination lighter or darker, depending on source.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDifference'><code>SkBlendMode::kDifference</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>22</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Difference'>Difference</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Subtracts darker from lighter with higher contrast.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kExclusion'><code>SkBlendMode::kExclusion</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>23</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Exclusion'>Exclusion</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Subtracts darker from lighter with lower contrast.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kMultiply'><code>SkBlendMode::kMultiply</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>24</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Multiply'>Multiply</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Multiplies source with destination, darkening image.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kLastSeparableMode'><code>SkBlendMode::kLastSeparableMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>24</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Last blend mode operating separately on components.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kHue'><code>SkBlendMode::kHue</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>25</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Hue'>Hue</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces hue of destination with hue of source, leaving saturation and luminosity
unchanged.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSaturation'><code>SkBlendMode::kSaturation</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>26</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Saturation'>Saturation</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces saturation of destination saturation hue of source, leaving hue and
luminosity unchanged.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kColor'><code>SkBlendMode::kColor</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>27</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color'>Color</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces hue and saturation of destination with hue and saturation of source,
leaving luminosity unchanged.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kLuminosity'><code>SkBlendMode::kLuminosity</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>28</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Luminosity'>Luminosity</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces luminosity of destination with luminosity of source, leaving hue and
saturation unchanged.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kLastMode'><code>SkBlendMode::kLastMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>28</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Used by tests to iterate through all valid values.
</td>
  </tr>
</table>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawColor'>SkCanvas::drawColor</a> <a href='SkCanvas_Reference#SkCanvas_drawVertices'>SkCanvas::drawVertices</a><sup><a href='SkCanvas_Reference#SkCanvas_drawVertices_2'>[2]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_drawVertices_3'>[3]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_drawVertices_4'>[4]</a></sup> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkShader_MakeCompose'>SkShader::MakeCompose</a> <a href='undocumented#SkXfermodeImageFilter'>SkXfermodeImageFilter</a>

## <a name='Clear'>Clear</a>

<a href='#SkBlendMode_kClear'>SkBlendMode::kClear</a> sets destination to:
[0, 0]
.
Use <a href='#SkBlendMode_kClear'>SkBlendMode::kClear</a> to initialize a buffer to fully transparent pixels when
creating a mask with irregular edges.

### Example

<div><fiddle-embed name="a9b56a26ca469bab9ab10e16f62fb2e2"><div><a href='SkColor_Reference#SK_ColorYELLOW'>SK ColorYELLOW</a> is ignored because <a href='#SkBlendMode_kClear'>SkBlendMode::kClear</a> ignores the source pixel
value and the destination pixel value, always setting the destination to zero.
</div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_clear'>SkCanvas::clear</a>

## <a name='Src'>Src</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kSrc'>SkBlendMode::kSrc</a> sets destination to:
[Sa, Sc]
.
Use <a href='#SkBlendMode_kSrc'>SkBlendMode::kSrc</a> to copy one buffer to another. All pixels are copied,
regardless of source and destination <a href='SkColor_Reference#Alpha'>Alpha</a> values. As a parameter to
<a href='SkCanvas_Reference#SkCanvas_drawAtlas'>SkCanvas::drawAtlas</a>, selects sprites and ignores colors.

### Example

<div><fiddle-embed name="0fc85dd916cc1a5896d36c80b9847391"><div><a href='#SkBlendMode_kSrc'>SkBlendMode::kSrc</a> does not blend transparent pixels with existing background;
it punches a transparent hole in the existing image.
</div></fiddle-embed></div>

### See Also

<a href='SkSurface_Reference#SkSurface_draw'>SkSurface::draw</a><sup><a href='SkSurface_Reference#SkSurface_draw_2'>[2]</a></sup> <a href='SkSurface_Reference#SkSurface_readPixels'>SkSurface::readPixels</a><sup><a href='SkSurface_Reference#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='SkSurface_Reference#SkSurface_readPixels_3'>[3]</a></sup>

## <a name='Dst'>Dst</a>

Given:
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kDst'>SkBlendMode::kDst</a> preserves destination set to:
[Da, Dc]
.
Setting <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#Blend_Mode'>Blend Mode</a> to <a href='#SkBlendMode_kDst'>SkBlendMode::kDst</a> causes drawing with
<a href='SkPaint_Reference#Paint'>Paint</a> to have no effect. As a parameter to <a href='SkCanvas_Reference#SkCanvas_drawAtlas'>SkCanvas::drawAtlas</a>,
selects colors and ignores sprites.

### Example

<div><fiddle-embed name="35915a2273be1076f00f2e47998ce808"></fiddle-embed></div>

## <a name='Src_Over'>Src Over</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a> replaces destination with:
[Sa + Da * (1 - Sa), Sc + Dc * (1 - Sa)]
,
drawing source over destination. <a href='#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a> is the default for <a href='SkPaint_Reference#Paint'>Paint</a>.

<a href='#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a> cannot make destination more transparent; the result will
be at least as opaque as the less transparent of source and original destination.

### Example

<div><fiddle-embed name="2ea9c149964a06cdb4929158cb4f15f8"></fiddle-embed></div>

## <a name='Dst_Over'>Dst Over</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kDstOver'>SkBlendMode::kDstOver</a> replaces destination with:
[Da + Sa * (1 - Da), Dc + Sc * (1 - Da)]
,
drawing destination over source. Has no effect destination if is opaque.

### Example

<div><fiddle-embed name="10dbb4d97902956ef5f5f8562f65119e"></fiddle-embed></div>

## <a name='Src_In'>Src In</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component, Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>;
<a href='#SkBlendMode_kSrcIn'>SkBlendMode::kSrcIn</a> replaces destination with:
[Sa * Da, Sc * Da]
,
drawing source with destination opacity.

### Example

<div><fiddle-embed name="b0833c18fe8b0eeaab9bd6d2160d272f"></fiddle-embed></div>

## <a name='Dst_In'>Dst In</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kDstIn'>SkBlendMode::kDstIn</a> replaces destination with:
[Da * Sa, Dc * Sa]
,
scaling destination <a href='SkColor_Reference#Alpha'>Alpha</a> by source <a href='SkColor_Reference#Alpha'>Alpha</a>. Resulting
destination is visible where source is visible.

### Example

<div><fiddle-embed name="a5eeba05ccf6097a5d110a9d64f97c25"></fiddle-embed></div>

## <a name='Src_Out'>Src Out</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component, Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>;
<a href='#SkBlendMode_kSrcOut'>SkBlendMode::kSrcOut</a> replaces destination with:
[Sa * (1 - Da), Sc * (1 - Da)]
,
drawing source fully where destination <a href='SkColor_Reference#Alpha'>Alpha</a> is zero. Is destination
is opaque, has no effect.

### Example

<div><fiddle-embed name="ccc1e74226e0c9eacbc21f1eed017b84"></fiddle-embed></div>

## <a name='Dst_Out'>Dst Out</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kDstOut'>SkBlendMode::kDstOut</a> replaces destination with:
[Da * (1 - Sa), Dc * (1 - Sa)]
,
scaling destination <a href='SkColor_Reference#Alpha'>Alpha</a> by source transparency. Resulting
destination is visible where source is transparent. If source is transparent,
has no effect.

### Example

<div><fiddle-embed name="b9a894c9accfc5d94081bbd77d5d790a"></fiddle-embed></div>

## <a name='Src_Atop'>Src Atop</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kSrcATop'>SkBlendMode::kSrcATop</a> replaces destination with:
[Da, Sc * Da + Dc * (1 - Sa)]
,
replacing opaque destination with opaque source. If source or destination
is transparent, has no effect.

### Example

<div><fiddle-embed name="a13148977bfc985934a92752c83a2041"></fiddle-embed></div>

## <a name='Dst_Atop'>Dst Atop</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kDstATop'>SkBlendMode::kDstATop</a> replaces destination with:
[Sa, Dc * Sa + Sc * (1 - Da)]
,
making destination transparent where source is transparent.

### Example

<div><fiddle-embed name="1955856d45773a4fd914fcc1f813222f"></fiddle-embed></div>

## <a name='Xor'>Xor</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kXor'>SkBlendMode::kXor</a> replaces destination with:
[Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa)]
,
exchanging the transparency of the source and destination.

### Example

<div><fiddle-embed name="29db2c7493d9098b8a086ddbe30dd6d6"></fiddle-embed></div>

## <a name='Plus'>Plus</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kPlus'>SkBlendMode::kPlus</a> replaces destination with:
[Sa + Da, Sc + Dc]
,
summing the <a href='SkColor_Reference#Alpha'>Alpha</a> and <a href='#Color'>Color</a> components.

### Example

<div><fiddle-embed name="05383441e510d54008402e128fc8ad2b"></fiddle-embed></div>

## <a name='Modulate'>Modulate</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kModulate'>SkBlendMode::kModulate</a> replaces destination with:
[Sa * Da, Sc * Dc]
,
scaling <a href='SkColor_Reference#Alpha'>Alpha</a> and <a href='#Color'>Color</a> components by the lesser of the values.
<a href='#SkBlendMode_kModulate'>SkBlendMode::kModulate</a> differs from <a href='#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a> in two ways.
<a href='#SkBlendMode_kModulate'>SkBlendMode::kModulate</a> like <a href='#SkBlendMode_kSrcATop'>SkBlendMode::kSrcATop</a> alters the destination inside
the destination area, as if the destination <a href='SkColor_Reference#Alpha'>Alpha</a> defined the boundaries of a
soft clip. <a href='#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a> like <a href='#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a> can alter the
destination where the destination is transparent.
<a href='#SkBlendMode_kModulate'>SkBlendMode::kModulate</a> computes the product of the source and destination using
<a href='undocumented#Premultiply'>Premultiplied</a> component values. <a href='#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a> the product of the source
and destination using <a href='undocumented#Unpremultiply'>Unpremultiplied</a> component values.

### Example

<div><fiddle-embed name="3fdac2b2f48bd227d2e74234c260bc8e"><div>If source and destination are opaque, <a href='#SkBlendMode_kModulate'>SkBlendMode::kModulate</a> and
<a href='#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a> produce the same results.
</div></fiddle-embed></div>

## <a name='Screen'>Screen</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kScreen'>SkBlendMode::kScreen</a> replaces destination with:
[Sa + Da - Sa * Da, Sc + Dc - Sc * Dc]
.

### Example

<div><fiddle-embed name="b7b42965927788d853f449f08ddf46de"></fiddle-embed></div>

## <a name='Overlay'>Overlay</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kOverlay'>SkBlendMode::kOverlay</a> replaces destination with:
[Sa + Da - Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa) +
(2 * Dc <= Da ? 2 * Sc * Dc : Sa * Da - 2 * (Da - Dc) * (Sa - Sc))]
.

### Example

<div><fiddle-embed name="03bf042201de02d6d131938ccd3172eb"></fiddle-embed></div>

## <a name='Darken'>Darken</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kDarken'>SkBlendMode::kDarken</a> replaces destination with:
[Sa + Da - Sa * Da,  Sc + Dc - max(Sc * Da, Dc * Sa)]
.
<a href='#SkBlendMode_kDarken'>SkBlendMode::kDarken</a> does not make an image darker; it replaces the destination
component with source if source is darker.

### Example

<div><fiddle-embed name="23c974d2759f523ca2f4a78ae86855c3"></fiddle-embed></div>

## <a name='Lighten'>Lighten</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kLighten'>SkBlendMode::kLighten</a> replaces destination with:
[Sa + Da - Sa * Da,  Sc + Dc - min(Sc * Da, Dc * Sa)]
.
<a href='#SkBlendMode_kDarken'>SkBlendMode::kDarken</a> does not make an image lighter; it replaces the destination
component with source if source is lighter.

### Example

<div><fiddle-embed name="95cb08b8c8db3af3b2c9ad56ae7d6bc1"></fiddle-embed></div>

## <a name='Color_Dodge'>Color Dodge</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kColorDodge'>SkBlendMode::kColorDodge</a> replaces destination with:
[Sa + Da - Sa * Da, Dc == 0 ? Sc * (1 - Da) : Sc == Sa ? Sc + Da * (1 - Sa) :
Sa * min(Da, Dc * Sa / (Sa - Sc)) + Sc * (1 - Da) + Da * (1 - Sa)]
,
making destination brighter to reflect source.

### Example

<div><fiddle-embed name="280ad6267a7d2d77b6d2c4531c6fc0bf"></fiddle-embed></div>

## <a name='Color_Burn'>Color Burn</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kColorBurn'>SkBlendMode::kColorBurn</a> replaces destination with:
[Sa + Da - Sa * Da, Dc == Da ? Dc + Sc * (1 - Da) : Sc == 0 ? Da * (1 - Sa) :
Sa * (Da - min(Da, (Da - Dc) * Sa / Sc)) + Sc * (1 - Da) + Da * (1 - Sa)]
,
making destination darker to reflect source.

### Example

<div><fiddle-embed name="3eeef529375d8083ae0d615789d55e89"></fiddle-embed></div>

## <a name='Hard_Light'>Hard Light</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kHardLight'>SkBlendMode::kHardLight</a> replaces destination with:
[Sa + Da - Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa) +
2 * Sc <= Sa ? 2 * Sc * Dc : Sa * Da - 2 * (Da - Dc) * (Sa - Sc)]
,
making destination lighter or darker, depending on source.

### Example

<div><fiddle-embed name="ac2fe555e2196e15863ea4ce74db3d54"></fiddle-embed></div>

## <a name='Soft_Light'>Soft Light</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component
where m = Da > 0 ? Dc / Da : 0;
<a href='#SkBlendMode_kSoftLight'>SkBlendMode::kSoftLight</a> replaces destination with:
[Sa + Da - Sa * Da, Sc / Da + Dc / Sa +
(2 * Sc <= Sa ? Dc * (Sa + (2 * Sc - Sa) * (1 - m)) : Dc * Sa + Da * (2 * Sc - Sa) *
(4 * Dc <= Da ? (16 * m * m  + 4 * m) * (m - 1) + 7 * m : sqrt(m) - m))]\
,
making destination lighter or darker, depending on source.

### Example

<div><fiddle-embed name="ac93f30dff13f8a8bb31398de370863b"></fiddle-embed></div>

## <a name='Difference'>Difference</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kDifference'>SkBlendMode::kDifference</a> replaces destination with:
[Sa + Da - Sa * Da, Sc + Dc - 2 * min(Sc * Da, Dc * Sa)]
,
replacing destination with lighter less darker.

### Example

<div><fiddle-embed name="52d2c8d1b9b428de4477b4caa1543a3d"></fiddle-embed></div>

## <a name='Exclusion'>Exclusion</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a>,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kExclusion'>SkBlendMode::kExclusion</a> replaces destination with:
[Sa + Da - Sa * Da, Sc + Dc - 2 * Sc * Dc]
,
replacing destination with lighter less darker, ignoring <a href='SkColor_Reference#Alpha'>Alpha</a>.

### Example

<div><fiddle-embed name="a544ee1c67c7c557a9e54d5e99f94bb6"></fiddle-embed></div>

## <a name='Multiply'>Multiply</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='#Color'>Color</a> component,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='#Color'>Color</a> component;
<a href='#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a> replaces destination with:
[Sa + Da - Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa) + Sc * Dc]
,
the product of <a href='undocumented#Unpremultiply'>Unpremultiplied</a> source and destination.
<a href='#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a> makes the image darker.

### Example

<div><fiddle-embed name="eb29c896f008dfbef09e16b85114fc3a"></fiddle-embed></div>

## <a name='Hue'>Hue</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, S as source <a href='#Color'>Color</a>,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, D as destination <a href='#Color'>Color</a>;
<a href='#SkBlendMode_kHue'>SkBlendMode::kHue</a> replaces destination with:
[Sa + Da - Sa * Da, SetLuminosity(SetSaturation(S, Saturation(D)), <a href='#Luminosity'>Luminosity(D)</a>)]
,
source hue, leaving destination luminosity and saturation unchanged.

### Example

<div><fiddle-embed name="41e45570d682397d3b8ff2f51bd9c574"></fiddle-embed></div>

## <a name='Saturation'>Saturation</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, S as source <a href='#Color'>Color</a>,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, D as destination <a href='#Color'>Color</a>;
<a href='#SkBlendMode_kHue'>SkBlendMode::kHue</a> replaces destination with:
[Sa + Da - Sa * Da, SetLuminosity(SetSaturation(D, Saturation(S)), <a href='#Luminosity'>Luminosity(D)</a>)]
,
source hue, leaving destination luminosity and saturation unchanged.

### Example

<div><fiddle-embed name="a48698975d236573cef512f94a7e360b"></fiddle-embed></div>

## <a name='Color'>Color</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, S as source <a href='#Color'>Color</a>,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, D as destination <a href='#Color'>Color</a>;
<a href='#SkBlendMode_kColor'>SkBlendMode::kColor</a> replaces destination with:
[Sa + Da - Sa * Da, SetLuminosity(S, Luminosity(D))]
,
source hue and saturation, leaving destination luminosity unchanged.

### Example

<div><fiddle-embed name="5d7c6e23a34ca9bf3ba8cda4cdc94cc4"></fiddle-embed></div>

## <a name='Luminosity'>Luminosity</a>

Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, S as source <a href='#Color'>Color</a>,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, D as destination <a href='#Color'>Color</a>;
<a href='#SkBlendMode_kLuminosity'>SkBlendMode::kLuminosity</a> replaces destination with:
[Sa + Da - Sa * Da, SetLuminosity(D, Luminosity(S))]
,
source luminosity, leaving destination hue and saturation unchanged.

### Example

<div><fiddle-embed name="7d42fe34ae20dd9e12c39dc3950e9989"></fiddle-embed></div>

<a name='SkBlendMode_Name'></a>
## SkBlendMode_Name

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SK_API const char* <a href='#SkBlendMode_Name'>SkBlendMode Name</a>(<a href='#SkBlendMode'>SkBlendMode</a> blendMode)
</pre>

Returns name of <a href='#SkBlendMode_Name_blendMode'>blendMode</a> as null-terminated C string.

### Parameters

<table>  <tr>    <td><a name='SkBlendMode_Name_blendMode'><code><strong>blendMode</strong></code></a></td>
    <td>one of: <a href='#SkBlendMode_kClear'>SkBlendMode::kClear</a>, <a href='#SkBlendMode_kSrc'>SkBlendMode::kSrc</a>, <a href='#SkBlendMode_kDst'>SkBlendMode::kDst</a>, <a href='#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a>,
<a href='#SkBlendMode_kDstOver'>SkBlendMode::kDstOver</a>, <a href='#SkBlendMode_kSrcIn'>SkBlendMode::kSrcIn</a>, <a href='#SkBlendMode_kDstIn'>SkBlendMode::kDstIn</a>,
<a href='#SkBlendMode_kSrcOut'>SkBlendMode::kSrcOut</a>, <a href='#SkBlendMode_kDstOut'>SkBlendMode::kDstOut</a>, <a href='#SkBlendMode_kSrcATop'>SkBlendMode::kSrcATop</a>,
<a href='#SkBlendMode_kDstATop'>SkBlendMode::kDstATop</a>, <a href='#SkBlendMode_kXor'>SkBlendMode::kXor</a>, <a href='#SkBlendMode_kPlus'>SkBlendMode::kPlus</a>,
<a href='#SkBlendMode_kModulate'>SkBlendMode::kModulate</a>, <a href='#SkBlendMode_kScreen'>SkBlendMode::kScreen</a>, <a href='#SkBlendMode_kOverlay'>SkBlendMode::kOverlay</a>,
<a href='#SkBlendMode_kDarken'>SkBlendMode::kDarken</a>, <a href='#SkBlendMode_kLighten'>SkBlendMode::kLighten</a>, <a href='#SkBlendMode_kColorDodge'>SkBlendMode::kColorDodge</a>,
<a href='#SkBlendMode_kColorBurn'>SkBlendMode::kColorBurn</a>, <a href='#SkBlendMode_kHardLight'>SkBlendMode::kHardLight</a>, <a href='#SkBlendMode_kSoftLight'>SkBlendMode::kSoftLight</a>,
<a href='#SkBlendMode_kDifference'>SkBlendMode::kDifference</a>, <a href='#SkBlendMode_kExclusion'>SkBlendMode::kExclusion</a>, <a href='#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a>,
<a href='#SkBlendMode_kHue'>SkBlendMode::kHue</a>, <a href='#SkBlendMode_kSaturation'>SkBlendMode::kSaturation</a>, <a href='#SkBlendMode_kColor'>SkBlendMode::kColor</a>,
<a href='#SkBlendMode_kLuminosity'>SkBlendMode::kLuminosity</a> </td>
  </tr>
</table>

### Return Value

C string

### Example

<div><fiddle-embed name="3996f4994bf4e90b4cd86524c1f9f1a6">

#### Example Output

~~~~
default blend: SkBlendMode::kSrcOver
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBlendMode'>SkBlendMode</a>

---

