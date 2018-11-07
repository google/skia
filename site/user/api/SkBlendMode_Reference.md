SkBlendMode Reference
===


<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum class <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> {
    <a href='SkBlendMode_Reference#SkBlendMode'>kClear</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kSrc</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kDst</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kSrcOver</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kDstOver</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kSrcIn</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kDstIn</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kSrcOut</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kDstOut</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kSrcATop</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kDstATop</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kXor</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kPlus</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kModulate</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kScreen</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kLastCoeffMode</a> = <a href='SkBlendMode_Reference#SkBlendMode'>kScreen</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kOverlay</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kDarken</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kLighten</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kColorDodge</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kColorBurn</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kHardLight</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kSoftLight</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kDifference</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kExclusion</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kMultiply</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kLastSeparableMode</a> = <a href='SkBlendMode_Reference#SkBlendMode'>kMultiply</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kHue</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kSaturation</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kColor</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kLuminosity</a>,
    <a href='SkBlendMode_Reference#SkBlendMode'>kLastMode</a> = <a href='SkBlendMode_Reference#SkBlendMode'>kLuminosity</a>,
};

<a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkBlendMode_Reference#SkBlendMode'>char</a>* <a href='SkBlendMode_Reference#SkBlendMode_Name'>SkBlendMode_Name</a>(<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>blendMode</a>);
</pre>

<a name='SkBlendMode'></a>

---

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
Replaces destination with <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>components</a> <a href='SkColor_Reference#Color'>set</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>zero</a>;
<a href='SkColor_Reference#Color'>a</a> <a href='SkColor_Reference#Color'>fully</a> <a href='SkColor_Reference#Color'>transparent</a> <a href='undocumented#Pixel'>pixel</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrc'><code>SkBlendMode::kSrc</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Src'>Src</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with source. Destination <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>component</a> <a href='SkColor_Reference#Color'>values</a>
<a href='SkColor_Reference#Color'>are</a> <a href='SkColor_Reference#Color'>ignored</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kDst'><code>SkBlendMode::kDst</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Dst'>Dst</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Preserves destination, ignoring source. Drawing with <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>set</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='#SkBlendMode_kDst'>kDst</a> <a href='#SkBlendMode_kDst'>has</a>
<a href='#SkBlendMode_kDst'>no</a> <a href='#SkBlendMode_kDst'>effect</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrcOver'><code>SkBlendMode::kSrcOver</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Src_Over'>Src&nbsp;Over</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with source blended with destination. If source is opaque,
replaces destination with source. Used as the default <a href='#Blend_Mode'>Blend_Mode</a> <a href='#Blend_Mode'>for</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>.
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
Multiplies source with destination, darkening <a href='SkImage_Reference#Image'>image</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kLastSeparableMode'><code>SkBlendMode::kLastSeparableMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>24</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Last  blend mode operating separately on components.
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

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawColor'>drawColor</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawVertices'>drawVertices</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeCompose'>MakeCompose</a> <a href='undocumented#SkXfermodeImageFilter'>SkXfermodeImageFilter</a>

<a name='Clear'></a>

---

<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kClear'>kClear</a> <a href='#SkBlendMode_kClear'>sets</a> <a href='#SkBlendMode_kClear'>destination</a> <a href='#SkBlendMode_kClear'>to</a>: <code>[0, 0]</code>.
Use <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kClear'>kClear</a> <a href='#SkBlendMode_kClear'>to</a> <a href='#SkBlendMode_kClear'>initialize</a> <a href='#SkBlendMode_kClear'>a</a> <a href='#SkBlendMode_kClear'>buffer</a> <a href='#SkBlendMode_kClear'>to</a> <a href='#SkBlendMode_kClear'>fully</a> <a href='#SkBlendMode_kClear'>transparent</a> <a href='#SkBlendMode_kClear'>pixels</a> <a href='#SkBlendMode_kClear'>when</a>
<a href='#SkBlendMode_kClear'>creating</a> <a href='#SkBlendMode_kClear'>a</a> <a href='#SkBlendMode_kClear'>mask</a> <a href='#SkBlendMode_kClear'>with</a> <a href='#SkBlendMode_kClear'>irregular</a> <a href='#SkBlendMode_kClear'>edges</a>.

### Example

<div><fiddle-embed name="a9b56a26ca469bab9ab10e16f62fb2e2"><div><a href='SkColor_Reference#SK_ColorYELLOW'>SK_ColorYELLOW</a> <a href='SkColor_Reference#SK_ColorYELLOW'>is</a> <a href='SkColor_Reference#SK_ColorYELLOW'>ignored</a> <a href='SkColor_Reference#SK_ColorYELLOW'>because</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kClear'>kClear</a> <a href='#SkBlendMode_kClear'>ignores</a> <a href='#SkBlendMode_kClear'>the</a> <a href='#SkBlendMode_kClear'>source</a> <a href='undocumented#Pixel'>pixel</a>
<a href='undocumented#Pixel'>value</a> <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>destination</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>value</a>, <a href='undocumented#Pixel'>always</a> <a href='undocumented#Pixel'>setting</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>destination</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>zero</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_clear'>clear</a>

<a name='Src'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrc'>kSrc</a> <a href='#SkBlendMode_kSrc'>sets</a> <a href='#SkBlendMode_kSrc'>destination</a> <a href='#SkBlendMode_kSrc'>to</a>: <code>[Sa, Sc]</code>.
Use <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrc'>kSrc</a> <a href='#SkBlendMode_kSrc'>to</a> <a href='#SkBlendMode_kSrc'>copy</a> <a href='#SkBlendMode_kSrc'>one</a> <a href='#SkBlendMode_kSrc'>buffer</a> <a href='#SkBlendMode_kSrc'>to</a> <a href='#SkBlendMode_kSrc'>another</a>. <a href='#SkBlendMode_kSrc'>All</a> <a href='#SkBlendMode_kSrc'>pixels</a> <a href='#SkBlendMode_kSrc'>are</a> <a href='#SkBlendMode_kSrc'>copied</a>,
<a href='#SkBlendMode_kSrc'>regardless</a> <a href='#SkBlendMode_kSrc'>of</a> <a href='#SkBlendMode_kSrc'>source</a> <a href='#SkBlendMode_kSrc'>and</a> <a href='#SkBlendMode_kSrc'>destination</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>values</a>. <a href='SkColor_Reference#Alpha'>As</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='SkColor_Reference#Alpha'>parameter</a> <a href='SkColor_Reference#Alpha'>to</a>
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawAtlas'>drawAtlas</a>, <a href='#SkCanvas_drawAtlas'>selects</a> <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>and</a> <a href='undocumented#Sprite'>ignores</a> <a href='undocumented#Sprite'>colors</a>.

### Example

<div><fiddle-embed name="0fc85dd916cc1a5896d36c80b9847391"><div><a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrc'>kSrc</a> <a href='#SkBlendMode_kSrc'>does</a> <a href='#SkBlendMode_kSrc'>not</a> <a href='#SkBlendMode_kSrc'>blend</a> <a href='#SkBlendMode_kSrc'>transparent</a> <a href='#SkBlendMode_kSrc'>pixels</a> <a href='#SkBlendMode_kSrc'>with</a> <a href='#SkBlendMode_kSrc'>existing</a> <a href='#SkBlendMode_kSrc'>background</a>;
<a href='#SkBlendMode_kSrc'>it</a> <a href='#SkBlendMode_kSrc'>punches</a> <a href='#SkBlendMode_kSrc'>a</a> <a href='#SkBlendMode_kSrc'>transparent</a> <a href='#SkBlendMode_kSrc'>hole</a> <a href='#SkBlendMode_kSrc'>in</a> <a href='#SkBlendMode_kSrc'>the</a> <a href='#SkBlendMode_kSrc'>existing</a> <a href='SkImage_Reference#Image'>image</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_draw'>draw</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_readPixels'>readPixels</a>

<a name='Dst'></a>

---

Given: <code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDst'>kDst</a> <a href='#SkBlendMode_kDst'>preserves</a> <a href='#SkBlendMode_kDst'>destination</a> <a href='#SkBlendMode_kDst'>set</a> <a href='#SkBlendMode_kDst'>to</a>: <code>[Da, Dc]</code>.
Setting <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#Blend_Mode'>Blend_Mode</a> <a href='#Blend_Mode'>to</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDst'>kDst</a> <a href='#SkBlendMode_kDst'>causes</a> <a href='#SkBlendMode_kDst'>drawing</a> <a href='#SkBlendMode_kDst'>with</a>
<a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='SkPaint_Reference#Paint'>have</a> <a href='SkPaint_Reference#Paint'>no</a> <a href='SkPaint_Reference#Paint'>effect</a>. <a href='SkPaint_Reference#Paint'>As</a> <a href='SkPaint_Reference#Paint'>a</a> <a href='SkPaint_Reference#Paint'>parameter</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawAtlas'>drawAtlas</a>,
<a href='#SkCanvas_drawAtlas'>selects</a> <a href='#SkCanvas_drawAtlas'>colors</a> <a href='#SkCanvas_drawAtlas'>and</a> <a href='#SkCanvas_drawAtlas'>ignores</a> <a href='undocumented#Sprite'>sprites</a>.

### Example

<div><fiddle-embed name="35915a2273be1076f00f2e47998ce808"></fiddle-embed></div>

<a name='Src_Over'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a> <a href='#SkBlendMode_kSrcOver'>replaces</a> <a href='#SkBlendMode_kSrcOver'>destination</a> <a href='#SkBlendMode_kSrcOver'>with</a>: <code>[Sa + Da * (1 - Sa), Sc + Dc * (1 - Sa)]</code>,
drawing source over destination. <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a> <a href='#SkBlendMode_kSrcOver'>is</a> <a href='#SkBlendMode_kSrcOver'>the</a> <a href='#SkBlendMode_kSrcOver'>default</a> <a href='#SkBlendMode_kSrcOver'>for</a> <a href='SkPaint_Reference#Paint'>Paint</a>.

<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a> <a href='#SkBlendMode_kSrcOver'>cannot</a> <a href='#SkBlendMode_kSrcOver'>make</a> <a href='#SkBlendMode_kSrcOver'>destination</a> <a href='#SkBlendMode_kSrcOver'>more</a> <a href='#SkBlendMode_kSrcOver'>transparent</a>; <a href='#SkBlendMode_kSrcOver'>the</a> <a href='#SkBlendMode_kSrcOver'>result</a> <a href='#SkBlendMode_kSrcOver'>will</a>
<a href='#SkBlendMode_kSrcOver'>be</a> <a href='#SkBlendMode_kSrcOver'>at</a> <a href='#SkBlendMode_kSrcOver'>least</a> <a href='#SkBlendMode_kSrcOver'>as</a> <a href='#SkBlendMode_kSrcOver'>opaque</a> <a href='#SkBlendMode_kSrcOver'>as</a> <a href='#SkBlendMode_kSrcOver'>the</a> <a href='#SkBlendMode_kSrcOver'>less</a> <a href='#SkBlendMode_kSrcOver'>transparent</a> <a href='#SkBlendMode_kSrcOver'>of</a> <a href='#SkBlendMode_kSrcOver'>source</a> <a href='#SkBlendMode_kSrcOver'>and</a> <a href='#SkBlendMode_kSrcOver'>original</a> <a href='#SkBlendMode_kSrcOver'>destination</a>.

### Example

<div><fiddle-embed name="2ea9c149964a06cdb4929158cb4f15f8"></fiddle-embed></div>

<a name='Dst_Over'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDstOver'>kDstOver</a> <a href='#SkBlendMode_kDstOver'>replaces</a> <a href='#SkBlendMode_kDstOver'>destination</a> <a href='#SkBlendMode_kDstOver'>with</a>: <code>[Da + Sa * (1 - Da), Dc + Sc * (1 - Da)]</code>,
drawing destination over source. Has no effect destination if is opaque.

### Example

<div><fiddle-embed name="10dbb4d97902956ef5f5f8562f65119e"></fiddle-embed></div>

<a name='Src_In'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcIn'>kSrcIn</a> <a href='#SkBlendMode_kSrcIn'>replaces</a> <a href='#SkBlendMode_kSrcIn'>destination</a> <a href='#SkBlendMode_kSrcIn'>with</a>: <code>[Sa * Da, Sc * Da]</code>,
drawing source with destination opacity.

### Example

<div><fiddle-embed name="b0833c18fe8b0eeaab9bd6d2160d272f"></fiddle-embed></div>

<a name='Dst_In'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDstIn'>kDstIn</a> <a href='#SkBlendMode_kDstIn'>replaces</a> <a href='#SkBlendMode_kDstIn'>destination</a> <a href='#SkBlendMode_kDstIn'>with</a>: <code>[Da * Sa, Dc * Sa]</code>,
scaling destination <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>by</a> <a href='SkColor_Reference#Alpha'>source</a> <a href='SkColor_Reference#Alpha'>Alpha</a>. <a href='SkColor_Reference#Alpha'>Resulting</a>
<a href='SkColor_Reference#Alpha'>destination</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>visible</a> <a href='SkColor_Reference#Alpha'>where</a> <a href='SkColor_Reference#Alpha'>source</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>visible</a>.

### Example

<div><fiddle-embed name="a5eeba05ccf6097a5d110a9d64f97c25"></fiddle-embed></div>

<a name='Src_Out'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOut'>kSrcOut</a> <a href='#SkBlendMode_kSrcOut'>replaces</a> <a href='#SkBlendMode_kSrcOut'>destination</a> <a href='#SkBlendMode_kSrcOut'>with</a>: <code>[Sa * (1 - Da), Sc * (1 - Da)]</code>,
drawing source fully where destination <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>zero</a>. <a href='SkColor_Reference#Alpha'>Is</a> <a href='SkColor_Reference#Alpha'>destination</a>
<a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>opaque</a>, <a href='SkColor_Reference#Alpha'>has</a> <a href='SkColor_Reference#Alpha'>no</a> <a href='SkColor_Reference#Alpha'>effect</a>.

### Example

<div><fiddle-embed name="ccc1e74226e0c9eacbc21f1eed017b84"></fiddle-embed></div>

<a name='Dst_Out'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDstOut'>kDstOut</a> <a href='#SkBlendMode_kDstOut'>replaces</a> <a href='#SkBlendMode_kDstOut'>destination</a> <a href='#SkBlendMode_kDstOut'>with</a>: <code>[Da * (1 - Sa), Dc * (1 - Sa)]</code>,
scaling destination <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>by</a> <a href='SkColor_Reference#Alpha'>source</a> <a href='SkColor_Reference#Alpha'>transparency</a>. <a href='SkColor_Reference#Alpha'>Resulting</a>
<a href='SkColor_Reference#Alpha'>destination</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>visible</a> <a href='SkColor_Reference#Alpha'>where</a> <a href='SkColor_Reference#Alpha'>source</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>transparent</a>. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>source</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>transparent</a>,
<a href='SkColor_Reference#Alpha'>has</a> <a href='SkColor_Reference#Alpha'>no</a> <a href='SkColor_Reference#Alpha'>effect</a>.

### Example

<div><fiddle-embed name="b9a894c9accfc5d94081bbd77d5d790a"></fiddle-embed></div>

<a name='Src_Atop'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcATop'>kSrcATop</a> <a href='#SkBlendMode_kSrcATop'>replaces</a> <a href='#SkBlendMode_kSrcATop'>destination</a> <a href='#SkBlendMode_kSrcATop'>with</a>: <code>[Da, Sc * Da + Dc * (1 - Sa)]</code>,
replacing opaque destination with opaque source. If source or destination
is transparent, has no effect.

### Example

<div><fiddle-embed name="a13148977bfc985934a92752c83a2041"></fiddle-embed></div>

<a name='Dst_Atop'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDstATop'>kDstATop</a> <a href='#SkBlendMode_kDstATop'>replaces</a> <a href='#SkBlendMode_kDstATop'>destination</a> <a href='#SkBlendMode_kDstATop'>with</a>: <code>[Sa, Dc * Sa + Sc * (1 - Da)]</code>,
making destination transparent where source is transparent.

### Example

<div><fiddle-embed name="1955856d45773a4fd914fcc1f813222f"></fiddle-embed></div>

<a name='Xor'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kXor'>kXor</a> <a href='#SkBlendMode_kXor'>replaces</a> <a href='#SkBlendMode_kXor'>destination</a> <a href='#SkBlendMode_kXor'>with</a>:
<code>[Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa)]</code>,
exchanging the transparency of the source and destination.

### Example

<div><fiddle-embed name="29db2c7493d9098b8a086ddbe30dd6d6"></fiddle-embed></div>

<a name='Plus'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kPlus'>kPlus</a> <a href='#SkBlendMode_kPlus'>replaces</a> <a href='#SkBlendMode_kPlus'>destination</a> <a href='#SkBlendMode_kPlus'>with</a>: <code>[Sa + Da, Sc + Dc]</code>,
summing the <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>components</a>.

### Example

<div><fiddle-embed name="05383441e510d54008402e128fc8ad2b"></fiddle-embed></div>

<a name='Modulate'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kModulate'>kModulate</a> <a href='#SkBlendMode_kModulate'>replaces</a> <a href='#SkBlendMode_kModulate'>destination</a> <a href='#SkBlendMode_kModulate'>with</a>: <code>[Sa * Da, Sc * Dc]</code>,
scaling <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>components</a> <a href='SkColor_Reference#Color'>by</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>lesser</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>values</a>.
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kModulate'>kModulate</a> <a href='#SkBlendMode_kModulate'>differs</a> <a href='#SkBlendMode_kModulate'>from</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kMultiply'>kMultiply</a> <a href='#SkBlendMode_kMultiply'>in</a> <a href='#SkBlendMode_kMultiply'>two</a> <a href='#SkBlendMode_kMultiply'>ways</a>.
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kModulate'>kModulate</a> <a href='#SkBlendMode_kModulate'>like</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcATop'>kSrcATop</a> <a href='#SkBlendMode_kSrcATop'>alters</a> <a href='#SkBlendMode_kSrcATop'>the</a> <a href='#SkBlendMode_kSrcATop'>destination</a> <a href='#SkBlendMode_kSrcATop'>inside</a>
<a href='#SkBlendMode_kSrcATop'>the</a> <a href='#SkBlendMode_kSrcATop'>destination</a> <a href='#SkBlendMode_kSrcATop'>area</a>, <a href='#SkBlendMode_kSrcATop'>as</a> <a href='#SkBlendMode_kSrcATop'>if</a> <a href='#SkBlendMode_kSrcATop'>the</a> <a href='#SkBlendMode_kSrcATop'>destination</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>defined</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>boundaries</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkColor_Reference#Alpha'>a</a>
<a href='SkColor_Reference#Alpha'>soft</a> <a href='SkColor_Reference#Alpha'>clip</a>. <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kMultiply'>kMultiply</a> <a href='#SkBlendMode_kMultiply'>like</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a> <a href='#SkBlendMode_kSrcOver'>can</a> <a href='#SkBlendMode_kSrcOver'>alter</a> <a href='#SkBlendMode_kSrcOver'>the</a>
<a href='#SkBlendMode_kSrcOver'>destination</a> <a href='#SkBlendMode_kSrcOver'>where</a> <a href='#SkBlendMode_kSrcOver'>the</a> <a href='#SkBlendMode_kSrcOver'>destination</a> <a href='#SkBlendMode_kSrcOver'>is</a> <a href='#SkBlendMode_kSrcOver'>transparent</a>.
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kModulate'>kModulate</a> <a href='#SkBlendMode_kModulate'>computes</a> <a href='#SkBlendMode_kModulate'>the</a> <a href='#SkBlendMode_kModulate'>product</a> <a href='#SkBlendMode_kModulate'>of</a> <a href='#SkBlendMode_kModulate'>the</a> <a href='#SkBlendMode_kModulate'>source</a> <a href='#SkBlendMode_kModulate'>and</a> <a href='#SkBlendMode_kModulate'>destination</a> <a href='#SkBlendMode_kModulate'>using</a>
<a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>component</a> <a href='undocumented#Premultiply'>values</a>. <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kMultiply'>kMultiply</a> <a href='#SkBlendMode_kMultiply'>the</a> <a href='#SkBlendMode_kMultiply'>product</a> <a href='#SkBlendMode_kMultiply'>of</a> <a href='#SkBlendMode_kMultiply'>the</a> <a href='#SkBlendMode_kMultiply'>source</a>
<a href='#SkBlendMode_kMultiply'>and</a> <a href='#SkBlendMode_kMultiply'>destination</a> <a href='#SkBlendMode_kMultiply'>using</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='undocumented#Unpremultiply'>component</a> <a href='undocumented#Unpremultiply'>values</a>.

### Example

<div><fiddle-embed name="3fdac2b2f48bd227d2e74234c260bc8e"><div>If source and destination are opaque, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kModulate'>kModulate</a> <a href='#SkBlendMode_kModulate'>and</a>
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kMultiply'>kMultiply</a> <a href='#SkBlendMode_kMultiply'>produce</a> <a href='#SkBlendMode_kMultiply'>the</a> <a href='#SkBlendMode_kMultiply'>same</a> <a href='#SkBlendMode_kMultiply'>results</a>.
</div></fiddle-embed></div>

<a name='Screen'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kScreen'>kScreen</a> <a href='#SkBlendMode_kScreen'>replaces</a> <a href='#SkBlendMode_kScreen'>destination</a> <a href='#SkBlendMode_kScreen'>with</a>: <code>[Sa + Da - Sa * Da, Sc + Dc - Sc * Dc]</code>.

### Example

<div><fiddle-embed name="b7b42965927788d853f449f08ddf46de"></fiddle-embed></div>

<a name='Overlay'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kOverlay'>kOverlay</a> <a href='#SkBlendMode_kOverlay'>replaces</a> <a href='#SkBlendMode_kOverlay'>destination</a> <a href='#SkBlendMode_kOverlay'>with</a>:
<code>[Sa + Da - Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa) +
(2 * Dc <= Da ? 2 * Sc * Dc : Sa * Da - 2 * (Da - Dc) * (Sa - Sc))]</code>.

### Example

<div><fiddle-embed name="03bf042201de02d6d131938ccd3172eb"></fiddle-embed></div>

<a name='Darken'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDarken'>kDarken</a> <a href='#SkBlendMode_kDarken'>replaces</a> <a href='#SkBlendMode_kDarken'>destination</a> <a href='#SkBlendMode_kDarken'>with</a>:
<code>[Sa + Da - Sa * Da,  Sc + Dc - <a href='undocumented#max()'>max</a>(<a href='undocumented#max()'>Sc</a> * <a href='undocumented#max()'>Da</a>, <a href='undocumented#max()'>Dc</a> * <a href='undocumented#max()'>Sa</a>)]</code>.
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDarken'>kDarken</a> <a href='#SkBlendMode_kDarken'>does</a> <a href='#SkBlendMode_kDarken'>not</a> <a href='#SkBlendMode_kDarken'>make</a> <a href='#SkBlendMode_kDarken'>an</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>darker</a>; <a href='SkImage_Reference#Image'>it</a> <a href='SkImage_Reference#Image'>replaces</a> <a href='SkImage_Reference#Image'>the</a> <a href='SkImage_Reference#Image'>destination</a>
<a href='SkImage_Reference#Image'>component</a> <a href='SkImage_Reference#Image'>with</a> <a href='SkImage_Reference#Image'>source</a> <a href='SkImage_Reference#Image'>if</a> <a href='SkImage_Reference#Image'>source</a> <a href='SkImage_Reference#Image'>is</a> <a href='SkImage_Reference#Image'>darker</a>.

### Example

<div><fiddle-embed name="23c974d2759f523ca2f4a78ae86855c3"></fiddle-embed></div>

<a name='Lighten'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kLighten'>kLighten</a> <a href='#SkBlendMode_kLighten'>replaces</a> <a href='#SkBlendMode_kLighten'>destination</a> <a href='#SkBlendMode_kLighten'>with</a>:
<code>[Sa + Da - Sa * Da,  Sc + Dc - <a href='undocumented#min()'>min</a>(<a href='undocumented#min()'>Sc</a> * <a href='undocumented#min()'>Da</a>, <a href='undocumented#min()'>Dc</a> * <a href='undocumented#min()'>Sa</a>)]</code>.
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDarken'>kDarken</a> <a href='#SkBlendMode_kDarken'>does</a> <a href='#SkBlendMode_kDarken'>not</a> <a href='#SkBlendMode_kDarken'>make</a> <a href='#SkBlendMode_kDarken'>an</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>lighter</a>; <a href='SkImage_Reference#Image'>it</a> <a href='SkImage_Reference#Image'>replaces</a> <a href='SkImage_Reference#Image'>the</a> <a href='SkImage_Reference#Image'>destination</a>
<a href='SkImage_Reference#Image'>component</a> <a href='SkImage_Reference#Image'>with</a> <a href='SkImage_Reference#Image'>source</a> <a href='SkImage_Reference#Image'>if</a> <a href='SkImage_Reference#Image'>source</a> <a href='SkImage_Reference#Image'>is</a> <a href='SkImage_Reference#Image'>lighter</a>.

### Example

<div><fiddle-embed name="95cb08b8c8db3af3b2c9ad56ae7d6bc1"></fiddle-embed></div>

<a name='Color_Dodge'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kColorDodge'>kColorDodge</a> <a href='#SkBlendMode_kColorDodge'>replaces</a> <a href='#SkBlendMode_kColorDodge'>destination</a> <a href='#SkBlendMode_kColorDodge'>with</a>:
<code>[Sa + Da - Sa * Da, Dc == 0 ? Sc * (1 - Da) : Sc == Sa ? Sc + Da * (1 - Sa) :
Sa * <a href='undocumented#min()'>min</a>(<a href='undocumented#min()'>Da</a>, <a href='undocumented#min()'>Dc</a> * <a href='undocumented#min()'>Sa</a> / (<a href='undocumented#min()'>Sa</a> - <a href='undocumented#min()'>Sc</a>)) + <a href='undocumented#min()'>Sc</a> * (1 - <a href='undocumented#min()'>Da</a>) + <a href='undocumented#min()'>Da</a> * (1 - <a href='undocumented#min()'>Sa</a>)]</code>,
making destination brighter to reflect source.

### Example

<div><fiddle-embed name="280ad6267a7d2d77b6d2c4531c6fc0bf"></fiddle-embed></div>

<a name='Color_Burn'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kColorBurn'>kColorBurn</a> <a href='#SkBlendMode_kColorBurn'>replaces</a> <a href='#SkBlendMode_kColorBurn'>destination</a> <a href='#SkBlendMode_kColorBurn'>with</a>:
<code>[Sa + Da - Sa * Da, Dc == Da ? Dc + Sc * (1 - Da) : Sc == 0 ? Da * (1 - Sa) :
Sa * (Da - <a href='undocumented#min()'>min</a>(<a href='undocumented#min()'>Da</a>, (<a href='undocumented#min()'>Da</a> - <a href='undocumented#min()'>Dc</a>) * <a href='undocumented#min()'>Sa</a> / <a href='undocumented#min()'>Sc</a>)) + <a href='undocumented#min()'>Sc</a> * (1 - <a href='undocumented#min()'>Da</a>) + <a href='undocumented#min()'>Da</a> * (1 - <a href='undocumented#min()'>Sa</a>)]</code>,
making destination darker to reflect source.

### Example

<div><fiddle-embed name="3eeef529375d8083ae0d615789d55e89"></fiddle-embed></div>

<a name='Hard_Light'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kHardLight'>kHardLight</a> <a href='#SkBlendMode_kHardLight'>replaces</a> <a href='#SkBlendMode_kHardLight'>destination</a> <a href='#SkBlendMode_kHardLight'>with</a>:
<code>[Sa + Da - Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa) +
2 * Sc <= Sa ? 2 * Sc * Dc : Sa * Da - 2 * (Da - Dc) * (Sa - Sc)]</code>,
making destination lighter or darker, depending on source.

### Example

<div><fiddle-embed name="ac2fe555e2196e15863ea4ce74db3d54"></fiddle-embed></div>

<a name='Soft_Light'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkColor_Reference#Color'>where</a> <code>m = Da > 0 ? Dc / Da : 0</code>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSoftLight'>kSoftLight</a> <a href='#SkBlendMode_kSoftLight'>replaces</a> <a href='#SkBlendMode_kSoftLight'>destination</a> <a href='#SkBlendMode_kSoftLight'>with</a>: <code>[Sa + Da - Sa * Da, Sc / Da + Dc / Sa +
(2 * Sc <= Sa ? Dc * (Sa + (2 * Sc - Sa) * (1 - m)) : Dc * Sa + Da * (2 * Sc - Sa) *
(4 * Dc <= Da ? (16 * m * m  + 4 * m) * (m - 1) + 7 * m : <a href='undocumented#sqrt()'>sqrt</a>(<a href='undocumented#sqrt()'>m</a>) - <a href='undocumented#sqrt()'>m</a>))]</code>,
making destination lighter or darker, depending on source.

### Example

<div><fiddle-embed name="ac93f30dff13f8a8bb31398de370863b"></fiddle-embed></div>

<a name='Difference'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDifference'>kDifference</a> <a href='#SkBlendMode_kDifference'>replaces</a> <a href='#SkBlendMode_kDifference'>destination</a> <a href='#SkBlendMode_kDifference'>with</a>:
<code>[Sa + Da - Sa * Da, Sc + Dc - 2 * <a href='undocumented#min()'>min</a>(<a href='undocumented#min()'>Sc</a> * <a href='undocumented#min()'>Da</a>, <a href='undocumented#min()'>Dc</a> * <a href='undocumented#min()'>Sa</a>)]</code>,
replacing destination with lighter less darker.

### Example

<div><fiddle-embed name="52d2c8d1b9b428de4477b4caa1543a3d"></fiddle-embed></div>

<a name='Exclusion'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kExclusion'>kExclusion</a> <a href='#SkBlendMode_kExclusion'>replaces</a> <a href='#SkBlendMode_kExclusion'>destination</a> <a href='#SkBlendMode_kExclusion'>with</a>:
<code>[Sa + Da - Sa * Da, Sc + Dc - 2 * Sc * Dc]</code>,
replacing destination with lighter less darker, ignoring <a href='SkColor_Reference#Alpha'>Alpha</a>.

### Example

<div><fiddle-embed name="a544ee1c67c7c557a9e54d5e99f94bb6"></fiddle-embed></div>

<a name='Multiply'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Sc</code> as source <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>Dc</code> as destination <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kMultiply'>kMultiply</a> <a href='#SkBlendMode_kMultiply'>replaces</a> <a href='#SkBlendMode_kMultiply'>destination</a> <a href='#SkBlendMode_kMultiply'>with</a>:
<code>[Sa + Da - Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa) + Sc * Dc]</code>,
the product of <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='undocumented#Unpremultiply'>source</a> <a href='undocumented#Unpremultiply'>and</a> <a href='undocumented#Unpremultiply'>destination</a>.
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kMultiply'>kMultiply</a> <a href='#SkBlendMode_kMultiply'>makes</a> <a href='#SkBlendMode_kMultiply'>the</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>darker</a>.

### Example

<div><fiddle-embed name="eb29c896f008dfbef09e16b85114fc3a"></fiddle-embed></div>

<a name='Hue'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>S</code> as source <a href='SkColor_Reference#Color'>Color</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>D</code> as destination <a href='SkColor_Reference#Color'>Color</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kHue'>kHue</a> <a href='#SkBlendMode_kHue'>replaces</a> <a href='#SkBlendMode_kHue'>destination</a> <a href='#SkBlendMode_kHue'>with</a>:
<code>[Sa + Da - Sa * Da, <a href='undocumented#SetLuminosity'>SetLuminosity</a>(<a href='undocumented#SetSaturation'>SetSaturation</a>(<a href='undocumented#SetSaturation'>S</a>, <a href='undocumented#Saturation'>Saturation</a>(<a href='undocumented#Saturation'>D</a>)), <a href='undocumented#Luminosity'>Luminosity</a>(<a href='undocumented#Luminosity'>D</a>))]</code>,
source hue, leaving destination luminosity and saturation unchanged.

### Example

<div><fiddle-embed name="41e45570d682397d3b8ff2f51bd9c574"></fiddle-embed></div>

<a name='Saturation'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>S</code> as source <a href='SkColor_Reference#Color'>Color</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>D</code> as destination <a href='SkColor_Reference#Color'>Color</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kHue'>kHue</a> <a href='#SkBlendMode_kHue'>replaces</a> <a href='#SkBlendMode_kHue'>destination</a> <a href='#SkBlendMode_kHue'>with</a>:
<code>[Sa + Da - Sa * Da, <a href='undocumented#SetLuminosity'>SetLuminosity</a>(<a href='undocumented#SetSaturation'>SetSaturation</a>(<a href='undocumented#SetSaturation'>D</a>, <a href='undocumented#Saturation'>Saturation</a>(<a href='undocumented#Saturation'>S</a>)), <a href='undocumented#Luminosity'>Luminosity</a>(<a href='undocumented#Luminosity'>D</a>))]</code>,
source hue, leaving destination luminosity and saturation unchanged.

### Example

<div><fiddle-embed name="a48698975d236573cef512f94a7e360b"></fiddle-embed></div>

<a name='Color'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>S</code> as source <a href='SkColor_Reference#Color'>Color</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>D</code> as destination <a href='SkColor_Reference#Color'>Color</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kColor'>kColor</a> <a href='#SkBlendMode_kColor'>replaces</a> <a href='#SkBlendMode_kColor'>destination</a> <a href='#SkBlendMode_kColor'>with</a>:
<code>[Sa + Da - Sa * Da, <a href='undocumented#SetLuminosity'>SetLuminosity</a>(<a href='undocumented#SetLuminosity'>S</a>, <a href='undocumented#Luminosity'>Luminosity</a>(<a href='undocumented#Luminosity'>D</a>))]</code>,
source hue and saturation, leaving destination luminosity unchanged.

### Example

<div><fiddle-embed name="5d7c6e23a34ca9bf3ba8cda4cdc94cc4"></fiddle-embed></div>

<a name='Luminosity'></a>

---

Given: <code>Sa</code> as source <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>S</code> as source <a href='SkColor_Reference#Color'>Color</a>,
<code>Da</code> as destination <a href='SkColor_Reference#Alpha'>Alpha</a>, <code>D</code> as destination <a href='SkColor_Reference#Color'>Color</a>;
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kLuminosity'>kLuminosity</a> <a href='#SkBlendMode_kLuminosity'>replaces</a> <a href='#SkBlendMode_kLuminosity'>destination</a> <a href='#SkBlendMode_kLuminosity'>with</a>:
<code>[Sa + Da - Sa * Da, <a href='undocumented#SetLuminosity'>SetLuminosity</a>(<a href='undocumented#SetLuminosity'>D</a>, <a href='undocumented#Luminosity'>Luminosity</a>(<a href='undocumented#Luminosity'>S</a>))]</code>,
source luminosity, leaving destination hue and saturation unchanged.

### Example

<div><fiddle-embed name="7d42fe34ae20dd9e12c39dc3950e9989"></fiddle-embed></div>

<a name='SkBlendMode_Name'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const char* <a href='SkBlendMode_Reference#SkBlendMode_Name'>SkBlendMode_Name</a>(<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>blendMode</a>)
</pre>

Returns name of <a href='#SkBlendMode_Name_blendMode'>blendMode</a> <a href='#SkBlendMode_Name_blendMode'>as</a> <a href='#SkBlendMode_Name_blendMode'>null-terminated</a> <a href='#SkBlendMode_Name_blendMode'>C</a> <a href='undocumented#String'>string</a>.

### Parameters

<table>  <tr>    <td><a name='SkBlendMode_Name_blendMode'><code><strong>blendMode</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kClear'>kClear</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrc'>kSrc</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDst'>kDst</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDstOver'>kDstOver</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcIn'>kSrcIn</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDstIn'>kDstIn</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOut'>kSrcOut</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDstOut'>kDstOut</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcATop'>kSrcATop</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDstATop'>kDstATop</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kXor'>kXor</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kPlus'>kPlus</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kModulate'>kModulate</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kScreen'>kScreen</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kOverlay'>kOverlay</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDarken'>kDarken</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kLighten'>kLighten</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kColorDodge'>kColorDodge</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kColorBurn'>kColorBurn</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kHardLight'>kHardLight</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSoftLight'>kSoftLight</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kDifference'>kDifference</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kExclusion'>kExclusion</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kMultiply'>kMultiply</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kHue'>kHue</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSaturation'>kSaturation</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kColor'>kColor</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kLuminosity'>kLuminosity</a>

### Return Value

C <a href='undocumented#String'>string</a>

### Example

<div><fiddle-embed name="3996f4994bf4e90b4cd86524c1f9f1a6">

#### Example Output

~~~~
default blend: SkBlendMode::kSrcOver
~~~~

</fiddle-embed></div>

### See Also

<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>

