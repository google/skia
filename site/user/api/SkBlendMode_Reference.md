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
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constructor'>Constructors</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>functions that construct <a href='#Blend_Mode'>Blend Mode</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member_Function'>Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>global and class member functions</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Related_Function'>Related Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>similar member functions grouped together</td>
  </tr>
</table>


## <a name='Function'>Function</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
</table>


## <a name='Enum'>Enum</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
</table>


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

Describes <a href='https://dvcs.w3.org/hg/FXTF/rawfile/tip/compositing/index.html#blending'>screen through luminosity</a></a> .

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kClear'><code>SkBlendMode::kClear</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with <a href='SkColor_Reference#Alpha'>Alpha</a> and <a href='SkColor_Reference#Color'>Color</a> components set to:
[0, 0]
.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrc'><code>SkBlendMode::kSrc</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='SkColor_Reference#Color'>Color</a>;
replaces destination set to:
[Sa, Sc]
.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDst'><code>SkBlendMode::kDst</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Given:
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='SkColor_Reference#Color'>Color</a>;
preserves destination set to:
[Da, Dc]
.
Setting <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#Blend_Mode'>Blend Mode</a> to <a href='#SkBlendMode_kDst'>kDst</a> causes drawing with
<a href='SkPaint_Reference#Paint'>Paint</a> to have no effect. As a parameter to <a href='SkCanvas_Reference#SkCanvas_drawAtlas'>SkCanvas::drawAtlas</a>,
selects sprites and ignores colors.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrcOver'><code>SkBlendMode::kSrcOver</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='SkColor_Reference#Color'>Color</a>,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='SkColor_Reference#Color'>Color</a>;
replaces destination with:
[Sa + Da * (1 - Sa), Sc + Dc * (1 - Sa)]
,
drawing source over destination. <a href='#SkBlendMode_kSrcOver'>kSrcOver</a> is the default
for <a href='SkPaint_Reference#Paint'>Paint</a>.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDstOver'><code>SkBlendMode::kDstOver</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='SkColor_Reference#Color'>Color</a>,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='SkColor_Reference#Color'>Color</a>;
replaces destination with:
[Da + Sa * (1 - Da), Dc + Sc * (1 - Da)]
,
drawing destination over source. If destination is opaque,
has no effect.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrcIn'><code>SkBlendMode::kSrcIn</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='SkColor_Reference#Color'>Color</a>, Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>;
replaces destination with:
[Sa * Da, Sc * Da]
,
drawing source with destination opacity. If destination
<a href='SkColor_Reference#Alpha'>Alpha</a> is zero, has no effect.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDstIn'><code>SkBlendMode::kDstIn</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>6</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='SkColor_Reference#Color'>Color</a>;
replaces destination with:
[Da * Sa, Dc * Sa]
,
scaling destination <a href='SkColor_Reference#Alpha'>Alpha</a> by source <a href='SkColor_Reference#Alpha'>Alpha</a>. Resulting
destination is visible where source is visible. If source
is opaque, has no effect.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrcOut'><code>SkBlendMode::kSrcOut</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>7</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='SkColor_Reference#Color'>Color</a>, Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>;
replaces destination with:
[Sa * (1 - Da), Sc * (1 - Da)]
,
drawing source fully where destination <a href='SkColor_Reference#Alpha'>Alpha</a> is zero. Is destination
is opaque, has no effect.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDstOut'><code>SkBlendMode::kDstOut</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='SkColor_Reference#Color'>Color</a>;
replaces destination with:
[Da * (1 - Sa), Dc * (1 - Sa)]
,
scaling destination <a href='SkColor_Reference#Alpha'>Alpha</a> by source transparency. Resulting
destination is visible where source is transparent. If source is transparent,
has no effect.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrcATop'><code>SkBlendMode::kSrcATop</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>9</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='SkColor_Reference#Color'>Color</a>,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='SkColor_Reference#Color'>Color</a>;
replaces destination with:
[Da, Sc * Da + Dc * (1 - Sa)]
,
replacing opaque destination with opaque source. If source or destination
is transparent, has no effect.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDstATop'><code>SkBlendMode::kDstATop</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>10</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Given:
Sa as source <a href='SkColor_Reference#Alpha'>Alpha</a>, Sc as source <a href='SkColor_Reference#Color'>Color</a>,
Da as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, Dc as destination <a href='SkColor_Reference#Color'>Color</a>;
replaces destination with:
[Sa, Dc * Sa + Sc * (1 - Da)]
,
making destination transparent where source is transparent.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kXor'><code>SkBlendMode::kXor</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>11</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa)]
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kPlus'><code>SkBlendMode::kPlus</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>12</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da, Sc + Dc]
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kModulate'><code>SkBlendMode::kModulate</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>13</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa * Da, Sc * Dc]
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kScreen'><code>SkBlendMode::kScreen</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>14</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da - Sa * Da, Sc + Dc - Sc * Dc]
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kLastCoeffMode'><code>SkBlendMode::kLastCoeffMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>14</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
last Porter Duff blend mode</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kOverlay'><code>SkBlendMode::kOverlay</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>15</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da * (1 - Sa),  2 * Dc <= Da ? 2 * Sc * Dc, Sa * Da - 2 * (Da - Dc) * (Sa - Sc)]
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDarken'><code>SkBlendMode::kDarken</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>16</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da * (1 - Sa),  Sc + Dc - max(Sc * Da, Dc * Sa)]
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kLighten'><code>SkBlendMode::kLighten</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>17</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da * (1 - Sa),  Sc + Dc - min(Sc * Da, Dc * Sa)]
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kColorDodge'><code>SkBlendMode::kColorDodge</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>18</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da * (1 - Sa), Dc == 0 ? Sc * (1 - Da) : Sc == Sa ? Sc + Da * (1 - Sa) :
Sa * min(Da, Dc * Sa / (Sa - Sc)) + Sc * (1 - Da) + Da * (1 - Sa)]
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kColorBurn'><code>SkBlendMode::kColorBurn</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>19</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da * (1 - Sa), Dc == Da ? Dc + Sc * (1 - Da) : Sc == 0 ? Da * (1 - Sa) :
Sa * (Da - min(Da, (Da - Dc) * Sa / Sc)) + Sc * (1 - Da) + Da * (1 - Sa)]
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kHardLight'><code>SkBlendMode::kHardLight</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>20</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da * (1 - Sa),  2 * Sc <= Sa ? 2 * Sc * Dc, Sa * Da - 2 * (Da - Dc) * (Sa - Sc)]
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSoftLight'><code>SkBlendMode::kSoftLight</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>21</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
where m = Da > 0 ? Dc / Da : 0
[Sa + Da * (1 - Sa), Sc / Da + Dc / Sa +
(2 * Sc <= Sa ? Dc * (Sa + (2 * Sc - Sa) * (1 - m)) : Dc * Sa + Da * (2 * Sc - Sa) *
(4 * Dc <= Da ? (16 * m * m  + 4 * m) * (m - 1) + 7 * m : sqrt(m) - m))]
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDifference'><code>SkBlendMode::kDifference</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>22</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da * (1 - Sa),  Sc + Dc - 2 * min(Sc * Da, Dc * Sa)]
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kExclusion'><code>SkBlendMode::kExclusion</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>23</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da * (1 - Sa),  Sc + Dc - 2 * Sc * Dc]
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kMultiply'><code>SkBlendMode::kMultiply</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>24</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa * (1 - Da) + Da * (1 - Sa) - Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa) - Sc * Dc]
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kLastSeparableMode'><code>SkBlendMode::kLastSeparableMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>24</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
last blend mode operating separately on components</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kHue'><code>SkBlendMode::kHue</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>25</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da * (1 - Sa), SetLum(SetSat(S, Sat(D)), Lum(D))]
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSaturation'><code>SkBlendMode::kSaturation</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>26</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da * (1 - Sa), SetLum(SetSat(D, Sat(S)), Lum(D))]
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kColor'><code>SkBlendMode::kColor</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>27</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da * (1 - Sa), SetLum(S, Lum(D))]
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kLuminosity'><code>SkBlendMode::kLuminosity</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>28</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
[
Sa + Da * (1 - Sa), SetLum(D, Lum(S))]
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kLastMode'><code>SkBlendMode::kLastMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>28</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkBlendMode_Name'></a>
## SkBlendMode_Name

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SK_API const char* <a href='#SkBlendMode_Name'>SkBlendMode Name</a>(<a href='#SkBlendMode'>SkBlendMode</a>)
</pre>

Return the (c-string) name of the blendmode.

### Parameters

<table>  <tr>    <td><a name='SkBlendMode_Name_SkBlendMode'><code><strong>SkBlendMode</strong></code></a></td>
    <td>incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

