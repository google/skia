SkBlendMode Reference
===


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

const char* <a href='#SkBlendMode_Name'>SkBlendMode_Name</a>(<a href='#SkBlendMode'>SkBlendMode</a> blendMode);
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
Replaces destination with <a href='SkColor_Reference#Alpha'>Alpha</a> and <a href='SkColor_Reference#Color'>Color</a> components set to zero;
a fully transparent <a href='undocumented#Pixel'>pixel</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBlendMode_kSrc'><code>SkBlendMode::kSrc</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Src'>Src</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces destination with source. Destination <a href='SkColor_Reference#Alpha'>alpha</a> and <a href='SkColor_Reference#Color'>color</a> component values
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
replaces destination with source. Used as the default <a href='#Blend_Mode'>Blend_Mode</a> for <a href='SkPaint_Reference#SkPaint'>SkPaint</a>.
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

<a href='SkCanvas_Reference#SkCanvas_drawColor'>SkCanvas::drawColor</a> <a href='SkCanvas_Reference#SkCanvas_drawVertices'>SkCanvas::drawVertices</a><sup><a href='SkCanvas_Reference#SkCanvas_drawVertices_2'>[2]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_drawVertices_3'>[3]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_drawVertices_4'>[4]</a></sup> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkShader_MakeCompose'>SkShader::MakeCompose</a> <a href='undocumented#SkXfermodeImageFilter'>SkXfermodeImageFilter</a>

<a name='Clear'></a>

---

<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::kClear sets destination to: <code>\[0, 0\]</code>.
Use SkBlendMode::kClear to initialize a buffer to fully transparent pixels when
creating a mask with irregular edges.

### Example

<div><fiddle-embed name="a9b56a26ca469bab9ab10e16f62fb2e2"><div><a href='SkColor_Reference#SK_ColorYELLOW'>SK_ColorYELLOW</a> is ignored because SkBlendMode::kClear ignores the source pixel
value and the destination pixel value, always setting the destination to zero.
</div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_clear'>SkCanvas::clear</a>

<a name='Src'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component;
SkBlendMode::kSrc sets destination to: <code>\[Sa, Sc\]</code>.
Use SkBlendMode::kSrc to copy one buffer to another. All pixels are copied,
regardless of source and destination Alpha values. As a parameter to
SkCanvas::drawAtlas, selects sprites and ignores colors.

### Example

<div><fiddle-embed name="0fc85dd916cc1a5896d36c80b9847391"><div>SkBlendMode::kSrc does not blend transparent pixels with existing background;
it punches a transparent hole in the existing image.
</div></fiddle-embed></div>

### See Also

<a href='SkSurface_Reference#SkSurface_draw'>SkSurface::draw</a><sup><a href='SkSurface_Reference#SkSurface_draw_2'>[2]</a></sup> <a href='SkSurface_Reference#SkSurface_readPixels'>SkSurface::readPixels</a><sup><a href='SkSurface_Reference#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='SkSurface_Reference#SkSurface_readPixels_3'>[3]</a></sup>

<a name='Dst'></a>

---

Given: <code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kDst preserves destination set to: <code>\[Da, Dc\]</code>.
Setting Paint Blend_Mode to SkBlendMode::kDst causes drawing with
Paint to have no effect. As a parameter to SkCanvas::drawAtlas,
selects colors and ignores sprites.

### Example

<div><fiddle-embed name="35915a2273be1076f00f2e47998ce808"></fiddle-embed></div>

<a name='Src_Over'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kSrcOver replaces destination with: <code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Sa\), Sc&nbsp;\+&nbsp;Dc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Sa\)\]</code>,
drawing source over destination. SkBlendMode::kSrcOver is the default for Paint.

SkBlendMode::kSrcOver cannot make destination more transparent; the result will
be at least as opaque as the less transparent of source and original destination.

### Example

<div><fiddle-embed name="2ea9c149964a06cdb4929158cb4f15f8"></fiddle-embed></div>

<a name='Dst_Over'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kDstOver replaces destination with: <code>\[Da&nbsp;\+&nbsp;Sa&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Da\), Dc&nbsp;\+&nbsp;Sc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Da\)\]</code>,
drawing destination over source. Has no effect destination if is opaque.

### Example

<div><fiddle-embed name="10dbb4d97902956ef5f5f8562f65119e"></fiddle-embed></div>

<a name='Src_In'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha;
SkBlendMode::kSrcIn replaces destination with: <code>\[Sa&nbsp;\*&nbsp;Da, Sc&nbsp;\*&nbsp;Da\]</code>,
drawing source with destination opacity.

### Example

<div><fiddle-embed name="b0833c18fe8b0eeaab9bd6d2160d272f"></fiddle-embed></div>

<a name='Dst_In'></a>

---

Given: <code>Sa</code> as source Alpha,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kDstIn replaces destination with: <code>\[Da&nbsp;\*&nbsp;Sa, Dc&nbsp;\*&nbsp;Sa\]</code>,
scaling destination Alpha by source Alpha. Resulting
destination is visible where source is visible.

### Example

<div><fiddle-embed name="a5eeba05ccf6097a5d110a9d64f97c25"></fiddle-embed></div>

<a name='Src_Out'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha;
SkBlendMode::kSrcOut replaces destination with: <code>\[Sa&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Da\), Sc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Da\)\]</code>,
drawing source fully where destination Alpha is zero. Is destination
is opaque, has no effect.

### Example

<div><fiddle-embed name="ccc1e74226e0c9eacbc21f1eed017b84"></fiddle-embed></div>

<a name='Dst_Out'></a>

---

Given: <code>Sa</code> as source Alpha,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kDstOut replaces destination with: <code>\[Da&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Sa\), Dc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Sa\)\]</code>,
scaling destination Alpha by source transparency. Resulting
destination is visible where source is transparent. If source is transparent,
has no effect.

### Example

<div><fiddle-embed name="b9a894c9accfc5d94081bbd77d5d790a"></fiddle-embed></div>

<a name='Src_Atop'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kSrcATop replaces destination with: <code>\[Da, Sc&nbsp;\*&nbsp;Da&nbsp;\+&nbsp;Dc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Sa\)\]</code>,
replacing opaque destination with opaque source. If source or destination
is transparent, has no effect.

### Example

<div><fiddle-embed name="a13148977bfc985934a92752c83a2041"></fiddle-embed></div>

<a name='Dst_Atop'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kDstATop replaces destination with: <code>\[Sa, Dc&nbsp;\*&nbsp;Sa&nbsp;\+&nbsp;Sc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Da\)\]</code>,
making destination transparent where source is transparent.

### Example

<div><fiddle-embed name="1955856d45773a4fd914fcc1f813222f"></fiddle-embed></div>

<a name='Xor'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kXor replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;2&nbsp;\*&nbsp;Sa&nbsp;\*&nbsp;Da, Sc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Da\)&nbsp;\+&nbsp;Dc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Sa\)\]</code>,
exchanging the transparency of the source and destination.

### Example

<div><fiddle-embed name="29db2c7493d9098b8a086ddbe30dd6d6"></fiddle-embed></div>

<a name='Plus'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kPlus replaces destination with: <code>\[Sa&nbsp;\+&nbsp;Da, Sc&nbsp;\+&nbsp;Dc\]</code>,
summing the Alpha and Color components.

### Example

<div><fiddle-embed name="05383441e510d54008402e128fc8ad2b"></fiddle-embed></div>

<a name='Modulate'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kModulate replaces destination with: <code>\[Sa&nbsp;\*&nbsp;Da, Sc&nbsp;\*&nbsp;Dc\]</code>,
scaling Alpha and Color components by the lesser of the values.
SkBlendMode::kModulate differs from SkBlendMode::kMultiply in two ways.
SkBlendMode::kModulate like SkBlendMode::kSrcATop alters the destination inside
the destination area, as if the destination Alpha defined the boundaries of a
soft clip. SkBlendMode::kMultiply like SkBlendMode::kSrcOver can alter the
destination where the destination is transparent.
SkBlendMode::kModulate computes the product of the source and destination using
Premultiplied component values. SkBlendMode::kMultiply the product of the source
and destination using Unpremultiplied component values.

### Example

<div><fiddle-embed name="3fdac2b2f48bd227d2e74234c260bc8e"><div>If source and destination are opaque, SkBlendMode::kModulate and
SkBlendMode::kMultiply produce the same results.
</div></fiddle-embed></div>

<a name='Screen'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kScreen replaces destination with: <code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, Sc&nbsp;\+&nbsp;Dc&nbsp;\-&nbsp;Sc&nbsp;\*&nbsp;Dc\]</code>.

### Example

<div><fiddle-embed name="b7b42965927788d853f449f08ddf46de"></fiddle-embed></div>

<a name='Overlay'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kOverlay replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, Sc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Da\)&nbsp;\+&nbsp;Dc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Sa\)&nbsp;\+
&nbsp;&nbsp;&nbsp;&nbsp;\(2&nbsp;\*&nbsp;Dc&nbsp;<=&nbsp;Da&nbsp;?&nbsp;2&nbsp;\*&nbsp;Sc&nbsp;\*&nbsp;Dc&nbsp;:&nbsp;Sa&nbsp;\*&nbsp;Da&nbsp;\-&nbsp;2&nbsp;\*&nbsp;\(Da&nbsp;\-&nbsp;Dc\)&nbsp;\*&nbsp;\(Sa&nbsp;\-&nbsp;Sc\)\)\]</code>.

### Example

<div><fiddle-embed name="03bf042201de02d6d131938ccd3172eb"></fiddle-embed></div>

<a name='Darken'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kDarken replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, &nbsp;Sc&nbsp;\+&nbsp;Dc&nbsp;\-&nbsp;max\(Sc&nbsp;\*&nbsp;Da, Dc&nbsp;\*&nbsp;Sa\)\]</code>.
SkBlendMode::kDarken does not make an image darker; it replaces the destination
component with source if source is darker.

### Example

<div><fiddle-embed name="23c974d2759f523ca2f4a78ae86855c3"></fiddle-embed></div>

<a name='Lighten'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kLighten replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, &nbsp;Sc&nbsp;\+&nbsp;Dc&nbsp;\-&nbsp;min\(Sc&nbsp;\*&nbsp;Da, Dc&nbsp;\*&nbsp;Sa\)\]</code>.
SkBlendMode::kDarken does not make an image lighter; it replaces the destination
component with source if source is lighter.

### Example

<div><fiddle-embed name="95cb08b8c8db3af3b2c9ad56ae7d6bc1"></fiddle-embed></div>

<a name='Color_Dodge'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kColorDodge replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, Dc&nbsp;==&nbsp;0&nbsp;?&nbsp;Sc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Da\)&nbsp;:&nbsp;Sc&nbsp;==&nbsp;Sa&nbsp;?&nbsp;Sc&nbsp;\+&nbsp;Da&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Sa\)&nbsp;:
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Sa&nbsp;\*&nbsp;min\(Da, Dc&nbsp;\*&nbsp;Sa&nbsp;/&nbsp;\(Sa&nbsp;\-&nbsp;Sc\)\)&nbsp;\+&nbsp;Sc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Da\)&nbsp;\+&nbsp;Da&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Sa\)\]</code>,
making destination brighter to reflect source.

### Example

<div><fiddle-embed name="280ad6267a7d2d77b6d2c4531c6fc0bf"></fiddle-embed></div>

<a name='Color_Burn'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kColorBurn replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, Dc&nbsp;==&nbsp;Da&nbsp;?&nbsp;Dc&nbsp;\+&nbsp;Sc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Da\)&nbsp;:&nbsp;Sc&nbsp;==&nbsp;0&nbsp;?&nbsp;Da&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Sa\)&nbsp;:
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Sa&nbsp;\*&nbsp;\(Da&nbsp;\-&nbsp;min\(Da, \(Da&nbsp;\-&nbsp;Dc\)&nbsp;\*&nbsp;Sa&nbsp;/&nbsp;Sc\)\)&nbsp;\+&nbsp;Sc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Da\)&nbsp;\+&nbsp;Da&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Sa\)\]</code>,
making destination darker to reflect source.

### Example

<div><fiddle-embed name="3eeef529375d8083ae0d615789d55e89"></fiddle-embed></div>

<a name='Hard_Light'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kHardLight replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, Sc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Da\)&nbsp;\+&nbsp;Dc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Sa\)&nbsp;\+
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2&nbsp;\*&nbsp;Sc&nbsp;<=&nbsp;Sa&nbsp;?&nbsp;2&nbsp;\*&nbsp;Sc&nbsp;\*&nbsp;Dc&nbsp;:&nbsp;Sa&nbsp;\*&nbsp;Da&nbsp;\-&nbsp;2&nbsp;\*&nbsp;\(Da&nbsp;\-&nbsp;Dc\)&nbsp;\*&nbsp;\(Sa&nbsp;\-&nbsp;Sc\)\]</code>,
making destination lighter or darker, depending on source.

### Example

<div><fiddle-embed name="ac2fe555e2196e15863ea4ce74db3d54"></fiddle-embed></div>

<a name='Soft_Light'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
where <code>m&nbsp;=&nbsp;Da&nbsp;>&nbsp;0&nbsp;?&nbsp;Dc&nbsp;/&nbsp;Da&nbsp;:&nbsp;0</code>;
SkBlendMode::kSoftLight replaces destination with: <code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, Sc&nbsp;/&nbsp;Da&nbsp;\+&nbsp;Dc&nbsp;/&nbsp;Sa&nbsp;\+
&nbsp;&nbsp;&nbsp;&nbsp;\(2&nbsp;\*&nbsp;Sc&nbsp;<=&nbsp;Sa&nbsp;?&nbsp;Dc&nbsp;\*&nbsp;\(Sa&nbsp;\+&nbsp;\(2&nbsp;\*&nbsp;Sc&nbsp;\-&nbsp;Sa\)&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;m\)\)&nbsp;:&nbsp;Dc&nbsp;\*&nbsp;Sa&nbsp;\+&nbsp;Da&nbsp;\*&nbsp;\(2&nbsp;\*&nbsp;Sc&nbsp;\-&nbsp;Sa\)&nbsp;\*
&nbsp;&nbsp;&nbsp;&nbsp;\(4&nbsp;\*&nbsp;Dc&nbsp;<=&nbsp;Da&nbsp;?&nbsp;\(16&nbsp;\*&nbsp;m&nbsp;\*&nbsp;m&nbsp;&nbsp;\+&nbsp;4&nbsp;\*&nbsp;m\)&nbsp;\*&nbsp;\(m&nbsp;\-&nbsp;1\)&nbsp;\+&nbsp;7&nbsp;\*&nbsp;m&nbsp;:&nbsp;sqrt(m)&nbsp;\-&nbsp;m\)\)\]</code>,
making destination lighter or darker, depending on source.

### Example

<div><fiddle-embed name="ac93f30dff13f8a8bb31398de370863b"></fiddle-embed></div>

<a name='Difference'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kDifference replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, Sc&nbsp;\+&nbsp;Dc&nbsp;\-&nbsp;2&nbsp;\*&nbsp;min\(Sc&nbsp;\*&nbsp;Da, Dc&nbsp;\*&nbsp;Sa\)\]</code>,
replacing destination with lighter less darker.

### Example

<div><fiddle-embed name="52d2c8d1b9b428de4477b4caa1543a3d"></fiddle-embed></div>

<a name='Exclusion'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kExclusion replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, Sc&nbsp;\+&nbsp;Dc&nbsp;\-&nbsp;2&nbsp;\*&nbsp;Sc&nbsp;\*&nbsp;Dc\]</code>,
replacing destination with lighter less darker, ignoring Alpha.

### Example

<div><fiddle-embed name="a544ee1c67c7c557a9e54d5e99f94bb6"></fiddle-embed></div>

<a name='Multiply'></a>

---

Given: <code>Sa</code> as source Alpha, <code>Sc</code> as source Color component,
<code>Da</code> as destination Alpha, <code>Dc</code> as destination Color component;
SkBlendMode::kMultiply replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, Sc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Da\)&nbsp;\+&nbsp;Dc&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;Sa\)&nbsp;\+&nbsp;Sc&nbsp;\*&nbsp;Dc\]</code>,
the product of Unpremultiplied source and destination.
SkBlendMode::kMultiply makes the image darker.

### Example

<div><fiddle-embed name="eb29c896f008dfbef09e16b85114fc3a"></fiddle-embed></div>

<a name='Hue'></a>

---

Given: <code>Sa</code> as source Alpha, <code>S</code> as source Color,
<code>Da</code> as destination Alpha, <code>D</code> as destination Color;
SkBlendMode::kHue replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, SetLuminosity(SetSaturation(S, Saturation(D)\), <a href='#Luminosity'>Luminosity</a>(D\)\)\]</code>,
source hue, leaving destination luminosity and saturation unchanged.

### Example

<div><fiddle-embed name="41e45570d682397d3b8ff2f51bd9c574"></fiddle-embed></div>

<a name='Saturation'></a>

---

Given: <code>Sa</code> as source Alpha, <code>S</code> as source Color,
<code>Da</code> as destination Alpha, <code>D</code> as destination Color;
SkBlendMode::kHue replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, SetLuminosity(SetSaturation(D, Saturation(S)\), <a href='#Luminosity'>Luminosity</a>(D\)\)\]</code>,
source hue, leaving destination luminosity and saturation unchanged.

### Example

<div><fiddle-embed name="a48698975d236573cef512f94a7e360b"></fiddle-embed></div>

<a name='Color'></a>

---

Given: <code>Sa</code> as source Alpha, <code>S</code> as source Color,
<code>Da</code> as destination Alpha, <code>D</code> as destination Color;
SkBlendMode::kColor replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, SetLuminosity(S, Luminosity(D)\)\]</code>,
source hue and saturation, leaving destination luminosity unchanged.

### Example

<div><fiddle-embed name="5d7c6e23a34ca9bf3ba8cda4cdc94cc4"></fiddle-embed></div>

<a name='Luminosity'></a>

---

Given: <code>Sa</code> as source Alpha, <code>S</code> as source Color,
<code>Da</code> as destination Alpha, <code>D</code> as destination Color;
SkBlendMode::kLuminosity replaces destination with:
<code>\[Sa&nbsp;\+&nbsp;Da&nbsp;\-&nbsp;Sa&nbsp;\*&nbsp;Da, SetLuminosity(D, Luminosity(S)\)\]</code>,
source luminosity, leaving destination hue and saturation unchanged.

### Example

<div><fiddle-embed name="7d42fe34ae20dd9e12c39dc3950e9989"></fiddle-embed></div>

<a name='SkBlendMode_Name'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const char* <a href='#SkBlendMode_Name'>SkBlendMode Name</a>(<a href='#SkBlendMode'>SkBlendMode</a> blendMode)
</pre>

Returns name of <a href='#SkBlendMode_Name_blendMode'>blendMode</a> as null-terminated C <a href='undocumented#String'>string</a>.

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

<a href='#SkBlendMode'>SkBlendMode</a>

