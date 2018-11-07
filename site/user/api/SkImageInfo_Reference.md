SkImageInfo Reference
===


<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> {
    <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>,
    <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>,
    <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
    <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>,
    <a href='SkImageInfo_Reference#kLastEnum_SkAlphaType'>kLastEnum_SkAlphaType</a> = <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>,
};

<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>static</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>bool</a> <a href='SkImageInfo_Reference#SkAlphaTypeIsOpaque'>SkAlphaTypeIsOpaque</a>(<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>at</a>);

<a href='SkImageInfo_Reference#SkAlphaType'>enum</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> {
    <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>,
    <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
    <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
    <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
    <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>kRGBA_F32_SkColorType</a>,
    <a href='SkImageInfo_Reference#kLastEnum_SkColorType'>kLastEnum_SkColorType</a> = <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>kRGBA_F32_SkColorType</a>,
    <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a> = <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
    <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a> = <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
};

<a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>int</a> <a href='SkImageInfo_Reference#SkColorTypeBytesPerPixel'>SkColorTypeBytesPerPixel</a>(<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>ct</a>);

<a href='SkImageInfo_Reference#SkColorType'>bool</a> <a href='SkImageInfo_Reference#SkColorTypeIsAlwaysOpaque'>SkColorTypeIsAlwaysOpaque</a>(<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>ct</a>);

<a href='SkImageInfo_Reference#SkColorType'>bool</a> <a href='SkImageInfo_Reference#SkColorTypeValidateAlphaType'>SkColorTypeValidateAlphaType</a>(<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>,
                                  <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>* <a href='SkImageInfo_Reference#SkAlphaType'>canonical</a> = <a href='SkImageInfo_Reference#SkAlphaType'>nullptr</a>);

<a href='SkImageInfo_Reference#SkAlphaType'>enum</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> {
    <a href='SkImageInfo_Reference#kJPEG_SkYUVColorSpace'>kJPEG_SkYUVColorSpace</a>,
    <a href='SkImageInfo_Reference#kRec601_SkYUVColorSpace'>kRec601_SkYUVColorSpace</a>,
    <a href='SkImageInfo_Reference#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>,
    <a href='SkImageInfo_Reference#kLastEnum_SkYUVColorSpace'>kLastEnum_SkYUVColorSpace</a> = <a href='SkImageInfo_Reference#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>,
};

<a href='SkImageInfo_Reference#kRec709_SkYUVColorSpace'>struct</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> {
    // <<a href='SkImageInfo_Reference#SkImageInfo'>i</a>><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>interface</a></<a href='SkImageInfo_Reference#SkImageInfo'>i</a>>
};
</pre>

<a href='#Image_Info'>Image_Info</a> <a href='#Image_Info'>specifies</a> <a href='#Image_Info'>the</a> <a href='#Image_Info'>dimensions</a> <a href='#Image_Info'>and</a> <a href='#Image_Info'>encoding</a> <a href='#Image_Info'>of</a> <a href='#Image_Info'>the</a> <a href='#Image_Info'>pixels</a> <a href='#Image_Info'>in</a> <a href='#Image_Info'>a</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>.
<a href='SkBitmap_Reference#Bitmap'>The</a> <a href='SkBitmap_Reference#Bitmap'>dimensions</a> <a href='SkBitmap_Reference#Bitmap'>are</a> <a href='SkBitmap_Reference#Bitmap'>integral</a> <a href='SkBitmap_Reference#Bitmap'>width</a> <a href='SkBitmap_Reference#Bitmap'>and</a> <a href='SkBitmap_Reference#Bitmap'>height</a>. <a href='SkBitmap_Reference#Bitmap'>The</a> <a href='SkBitmap_Reference#Bitmap'>encoding</a> <a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>how</a> <a href='undocumented#Pixel'>pixel</a>
<a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>describe</a> <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Color_Alpha'>transparency</a>; <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>components</a> <a href='SkColor_Reference#Color'>red</a>, <a href='SkColor_Reference#Color'>blue</a>,
<a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>green</a>; <a href='SkColor_Reference#Color'>and</a> <a href='#Color_Space'>Color_Space</a>, <a href='#Color_Space'>the</a> <a href='#Color_Space'>range</a> <a href='#Color_Space'>and</a> <a href='#Color_Space'>linearity</a> <a href='#Color_Space'>of</a> <a href='#Color_Space'>colors</a>.

<a href='#Image_Info'>Image_Info</a> <a href='#Image_Info'>describes</a> <a href='#Image_Info'>an</a> <a href='#Image_Info'>uncompressed</a> <a href='#Image_Info'>raster</a> <a href='#Image_Info'>pixels</a>. <a href='#Image_Info'>In</a> <a href='#Image_Info'>contrast</a>, <a href='SkImage_Reference#Image'>Image</a>
<a href='SkImage_Reference#Image'>additionally</a> <a href='SkImage_Reference#Image'>describes</a> <a href='SkImage_Reference#Image'>compressed</a> <a href='SkImage_Reference#Image'>pixels</a> <a href='SkImage_Reference#Image'>like</a> <a href='SkImage_Reference#Image'>PNG</a>, <a href='SkImage_Reference#Image'>and</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>describes</a>
<a href='SkSurface_Reference#Surface'>destinations</a> <a href='SkSurface_Reference#Surface'>on</a> <a href='SkSurface_Reference#Surface'>the</a> <a href='SkSurface_Reference#Surface'>GPU</a>. <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>and</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>may</a> <a href='SkSurface_Reference#Surface'>be</a> <a href='SkSurface_Reference#Surface'>specified</a> <a href='SkSurface_Reference#Surface'>by</a> <a href='#Image_Info'>Image_Info</a>,
<a href='#Image_Info'>but</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>and</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>may</a> <a href='SkSurface_Reference#Surface'>not</a> <a href='SkSurface_Reference#Surface'>contain</a> <a href='#Image_Info'>Image_Info</a>.

<a name='Alpha_Type'></a>

<a name='SkAlphaType'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> {
    <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>,
    <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>,
    <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
    <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>,
    <a href='SkImageInfo_Reference#kLastEnum_SkAlphaType'>kLastEnum_SkAlphaType</a> = <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>,
};
</pre>

Describes how to interpret the <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='undocumented#Pixel'>pixel</a>. <a href='undocumented#Pixel'>A</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>may</a>
<a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>opaque</a>, <a href='undocumented#Pixel'>or</a> <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Color_Alpha'>describing</a> <a href='#Color_Alpha'>multiple</a> <a href='#Color_Alpha'>levels</a> <a href='#Color_Alpha'>of</a> <a href='#Color_Alpha'>transparency</a>.

<a href='#Color_Alpha'>In</a> <a href='#Color_Alpha'>simple</a> <a href='#Color_Alpha'>blending</a>, <a href='#Color_Alpha'>Color_Alpha</a> <a href='SkPath_Reference#Conic_Weight'>weights</a> <a href='SkPath_Reference#Conic_Weight'>the</a> <a href='SkPath_Reference#Conic_Weight'>draw</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>destination</a>
<a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>create</a> <a href='SkColor_Reference#Color'>a</a> <a href='SkColor_Reference#Color'>new</a> <a href='SkColor_Reference#Color'>color</a>. <a href='SkColor_Reference#Color'>If</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>describes</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='SkColor_Reference#Alpha'>weight</a> <a href='SkColor_Reference#Alpha'>from</a> <a href='SkColor_Reference#Alpha'>zero</a> <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>one</a>,
<a href='SkColor_Reference#Alpha'>new</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>is</a> <a href='SkColor_Reference#Color'>set</a> <a href='SkColor_Reference#Color'>to</a>: <code>draw <a href='SkColor_Reference#Color'>color</a> * <a href='SkColor_Reference#Alpha'>alpha</a> + <a href='SkColor_Reference#Alpha'>destination</a> <a href='SkColor_Reference#Color'>color</a> * (1 - <a href='SkColor_Reference#Alpha'>alpha</a>)</code>.

In practice <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>encoded</a> <a href='SkColor_Reference#Alpha'>in</a> <a href='SkColor_Reference#Alpha'>two</a> <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>more</a> <a href='SkColor_Reference#Alpha'>bits</a>, <a href='SkColor_Reference#Alpha'>where</a> 1.0 <a href='SkColor_Reference#Alpha'>equals</a> <a href='SkColor_Reference#Alpha'>all</a> <a href='SkColor_Reference#Alpha'>bits</a> <a href='SkColor_Reference#Alpha'>set</a>.

<a href='SkColor_Reference#Alpha'>RGB</a> <a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>have</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>included</a> <a href='#Color_Alpha'>in</a> <a href='#Color_Alpha'>each</a> <a href='#Color_Alpha'>component</a> <a href='#Color_Alpha'>value</a>; <a href='#Color_Alpha'>the</a> <a href='#Color_Alpha'>stored</a>
<a href='#Color_Alpha'>value</a> <a href='#Color_Alpha'>is</a> <a href='#Color_Alpha'>the</a> <a href='#Color_Alpha'>original</a> <a href='#Color_Alpha'>RGB</a> <a href='#Color_Alpha'>multiplied</a> <a href='#Color_Alpha'>by</a> <a href='#Color_Alpha'>Color_Alpha</a>. <a href='undocumented#Premultiply'>Premultiplied</a> <a href='SkColor_Reference#Color'>color</a>
<a href='SkColor_Reference#Color'>components</a> <a href='SkColor_Reference#Color'>improve</a> <a href='SkColor_Reference#Color'>performance</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kUnknown_SkAlphaType'><code>kUnknown_SkAlphaType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Alpha_Type'>is</a> <a href='#Image_Info_Alpha_Type'>uninitialized</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kOpaque_SkAlphaType'><code>kOpaque_SkAlphaType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Opaque'>Opaque</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Pixels are opaque. The <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>must</a> <a href='#Image_Info_Color_Type'>have</a> <a href='#Image_Info_Color_Type'>no</a> <a href='#Image_Info_Color_Type'>explicit</a> <a href='SkColor_Reference#Alpha'>alpha</a>
<a href='SkColor_Reference#Alpha'>component</a>, <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>all</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>components</a> <a href='SkColor_Reference#Alpha'>must</a> <a href='SkColor_Reference#Alpha'>be</a> <a href='SkColor_Reference#Alpha'>set</a> <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>their</a> <a href='SkColor_Reference#Alpha'>maximum</a> <a href='SkColor_Reference#Alpha'>value</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kPremul_SkAlphaType'><code>kPremul_SkAlphaType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Premul'>Premul</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Pixels have <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>into</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>components</a>.
<a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>pixels</a> <a href='SkSurface_Reference#Surface'>must</a> <a href='SkSurface_Reference#Surface'>be</a> <a href='undocumented#Premultiply'>Premultiplied</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kUnpremul_SkAlphaType'><code>kUnpremul_SkAlphaType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Unpremul'>Unpremul</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='undocumented#Pixel'>Pixel</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>component</a> <a href='SkColor_Reference#Color'>values</a> <a href='SkColor_Reference#Color'>are</a> <a href='SkColor_Reference#Color'>independent</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>value</a>.
<a href='SkColor_Reference#Alpha'>Images</a> <a href='SkColor_Reference#Alpha'>generated</a> <a href='SkColor_Reference#Alpha'>from</a> <a href='SkColor_Reference#Alpha'>encoded</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>like</a> <a href='undocumented#Data'>PNG</a> <a href='undocumented#Data'>do</a> <a href='undocumented#Data'>not</a> <a href='undocumented#Premultiply'>Premultiply</a> <a href='undocumented#Pixel'>pixel</a> <a href='SkColor_Reference#Color'>color</a>
<a href='SkColor_Reference#Color'>components</a>. <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>is</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>supported</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>for</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>pixels</a>, <a href='SkImage_Reference#Image'>but</a> <a href='SkImage_Reference#Image'>not</a> <a href='SkImage_Reference#Image'>for</a>
<a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>pixels</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kLastEnum_SkAlphaType'><code>kLastEnum_SkAlphaType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Used by tests to iterate through all valid values.
</td>
  </tr>
</table>

### See Also

<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>

<a name='Alpha_Type_Opaque'></a>

---

Use <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>as</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>a</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>hint</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>to</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>optimize</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>drawing</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>when</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>component</a>
<a href='SkColor_Reference#Alpha'>of</a> <a href='SkColor_Reference#Alpha'>all</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>set</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>its</a> <a href='undocumented#Pixel'>maximum</a> <a href='undocumented#Pixel'>value</a> <a href='undocumented#Pixel'>of</a> 1.0; <a href='undocumented#Pixel'>all</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>bits</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>set</a>.
<a href='SkColor_Reference#Alpha'>If</a> <a href='#Image_Info'>Image_Info</a> <a href='#Image_Info'>is</a> <a href='#Image_Info'>set</a> <a href='#Image_Info'>to</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>but</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>all</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>not</a> 1.0,
<a href='SkColor_Reference#Alpha'>results</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

### Example

<div><fiddle-embed name="79146a1a41d58d22582fdc567c6ffe4e"><div><a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>parameter</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>a</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>is</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>set</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>to</a> 255, <a href='SkColor_Reference#SkPreMultiplyARGB'>its</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>maximum</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>value</a>, <a href='SkColor_Reference#SkPreMultiplyARGB'>and</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>is</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>interpreted</a>
<a href='SkColor_Reference#SkPreMultiplyARGB'>as</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>of</a> 1.0. <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>may</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>be</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>set</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>to</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>improve</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>performance</a>.
<a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>If</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>parameter</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>a</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>is</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>set</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>to</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>a</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>value</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>smaller</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>than</a> 255,
<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>must</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>be</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>used</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>instead</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>to</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>avoid</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>undefined</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>results</a>.
<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>The</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>four</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>displayed</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>values</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>are</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>the</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>original</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>component</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>values</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>though</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>not</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>necessarily</a>
<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>in</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>the</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>same</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>order</a>.
</div></fiddle-embed></div>

<a name='Alpha_Type_Premul'></a>

---

Use <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>when</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>stored</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>components</a> <a href='SkColor_Reference#Color'>are</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>original</a> <a href='SkColor_Reference#Color'>color</a>
<a href='SkColor_Reference#Color'>multiplied</a> <a href='SkColor_Reference#Color'>by</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a>. <a href='SkColor_Reference#Alpha'>The</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>range</a> <a href='SkColor_Reference#Alpha'>of</a> 0.0 <a href='SkColor_Reference#Alpha'>to</a> 1.0 <a href='SkColor_Reference#Alpha'>is</a>
<a href='SkColor_Reference#Alpha'>achieved</a> <a href='SkColor_Reference#Alpha'>by</a> <a href='SkColor_Reference#Alpha'>dividing</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>integer</a> <a href='SkColor_Reference#Alpha'>bit</a> <a href='SkColor_Reference#Alpha'>value</a> <a href='SkColor_Reference#Alpha'>by</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>maximum</a> <a href='SkColor_Reference#Alpha'>bit</a> <a href='SkColor_Reference#Alpha'>value</a>.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
stored <a href='SkColor_Reference#Color'>color</a> = <a href='SkColor_Reference#Color'>original</a> <a href='SkColor_Reference#Color'>color</a> * <a href='SkColor_Reference#Alpha'>alpha</a> / <a href='SkColor_Reference#Alpha'>max</a> <a href='SkColor_Reference#Alpha'>alpha </a>
</pre>

The <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>component</a> <a href='SkColor_Reference#Color'>must</a> <a href='SkColor_Reference#Color'>be</a> <a href='SkColor_Reference#Color'>equal</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>or</a> <a href='SkColor_Reference#Color'>smaller</a> <a href='SkColor_Reference#Color'>than</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a>,
<a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>results</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

### Example

<div><fiddle-embed name="ad696b39c915803d566e96896ec3a36c"><div><a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>parameter</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>a</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>is</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>set</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>to</a> 150, <a href='SkColor_Reference#SkPreMultiplyARGB'>less</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>than</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>its</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>maximum</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>value</a>, <a href='SkColor_Reference#SkPreMultiplyARGB'>and</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>is</a>
<a href='SkColor_Reference#SkPreMultiplyARGB'>interpreted</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>as</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>of</a> <a href='#Color_Alpha'>about</a> 0.6. <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>must</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>be</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>set</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>since</a>
<a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>parameter</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>a</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>is</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>set</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>to</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>a</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>value</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>smaller</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>than</a> 255,
<a href='SkColor_Reference#SkPreMultiplyARGB'>to</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>avoid</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>undefined</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>results</a>.
<a href='SkColor_Reference#SkPreMultiplyARGB'>The</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>four</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>displayed</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>values</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>reflect</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>that</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>the</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>has</a> <a href='SkColor_Reference#Alpha'>been</a> <a href='SkColor_Reference#Alpha'>multiplied</a>
<a href='SkColor_Reference#Alpha'>by</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>original</a> <a href='SkColor_Reference#Color'>color</a>.
</div></fiddle-embed></div>

<a name='Alpha_Type_Unpremul'></a>

---

Use <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>if</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>stored</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>components</a> <a href='SkColor_Reference#Color'>are</a> <a href='SkColor_Reference#Color'>not</a> <a href='SkColor_Reference#Color'>divided</a> <a href='SkColor_Reference#Color'>by</a> <a href='SkColor_Reference#Color'>the</a>
<a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a>. <a href='SkColor_Reference#Alpha'>Some</a> <a href='SkColor_Reference#Alpha'>drawing</a> <a href='SkColor_Reference#Alpha'>destinations</a> <a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>not</a> <a href='SkColor_Reference#Alpha'>support</a>
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>.

### Example

<div><fiddle-embed name="b8216a9e5ff5bc61a0e46eba7d36307b"><div><a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a> <a href='SkColor_Reference#SkColorSetARGB'>parameter</a> <a href='SkColor_Reference#SkColorSetARGB'>a</a> <a href='SkColor_Reference#SkColorSetARGB'>is</a> <a href='SkColor_Reference#SkColorSetARGB'>set</a> <a href='SkColor_Reference#SkColorSetARGB'>to</a> 150, <a href='SkColor_Reference#SkColorSetARGB'>less</a> <a href='SkColor_Reference#SkColorSetARGB'>than</a> <a href='SkColor_Reference#SkColorSetARGB'>its</a> <a href='SkColor_Reference#SkColorSetARGB'>maximum</a> <a href='SkColor_Reference#SkColorSetARGB'>value</a>, <a href='SkColor_Reference#SkColorSetARGB'>and</a> <a href='SkColor_Reference#SkColorSetARGB'>is</a>
<a href='SkColor_Reference#SkColorSetARGB'>interpreted</a> <a href='SkColor_Reference#SkColorSetARGB'>as</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>of</a> <a href='#Color_Alpha'>about</a> 0.6. <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>is</a> <a href='SkColor_Reference#Color'>not</a> <a href='undocumented#Premultiply'>Premultiplied</a>;
<a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>components</a> <a href='SkColor_Reference#Color'>may</a> <a href='SkColor_Reference#Color'>have</a> <a href='SkColor_Reference#Color'>values</a> <a href='SkColor_Reference#Color'>greater</a> <a href='SkColor_Reference#Color'>than</a>  <a href='SkColor_Reference#Color'>color alpha</a>.
<a href='SkColor_Reference#Color'>The</a> <a href='SkColor_Reference#Color'>four</a> <a href='SkColor_Reference#Color'>displayed</a> <a href='SkColor_Reference#Color'>values</a> <a href='SkColor_Reference#Color'>are</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>original</a> <a href='SkColor_Reference#Color'>component</a> <a href='SkColor_Reference#Color'>values</a>, <a href='SkColor_Reference#Color'>though</a> <a href='SkColor_Reference#Color'>not</a> <a href='SkColor_Reference#Color'>necessarily</a>
<a href='SkColor_Reference#Color'>in</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>same</a> <a href='SkColor_Reference#Color'>order</a>.
</div></fiddle-embed></div>

<a name='SkAlphaTypeIsOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static inline bool <a href='SkImageInfo_Reference#SkAlphaTypeIsOpaque'>SkAlphaTypeIsOpaque</a>(<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>at</a>)
</pre>

Returns true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>equals</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>. <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>a</a>
hint that the <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>opaque</a>, <a href='SkImageInfo_Reference#SkColorType'>or</a> <a href='SkImageInfo_Reference#SkColorType'>that</a> <a href='SkImageInfo_Reference#SkColorType'>all</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>set</a> <a href='SkColor_Reference#Alpha'>to</a>
their 1.0 equivalent. If <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a>
opaque, then the result of drawing any <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>with</a> <a href='undocumented#Pixel'>a</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>value</a> <a href='SkColor_Reference#Alpha'>less</a> <a href='SkColor_Reference#Alpha'>than</a>
1.0 is undefined.

### Parameters

<table>  <tr>    <td><a name='SkAlphaTypeIsOpaque_at'><code><strong>at</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Return Value

true if <a href='#SkAlphaTypeIsOpaque_at'>at</a> <a href='#SkAlphaTypeIsOpaque_at'>equals</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>

<a name='Color_Type'></a>

<a name='SkColorType'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> {
    <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>,
    <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
    <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
    <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
    <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>,
    <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>kRGBA_F32_SkColorType</a>,
    <a href='SkImageInfo_Reference#kLastEnum_SkColorType'>kLastEnum_SkColorType</a> = <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>kRGBA_F32_SkColorType</a>,
    <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a> = <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
    <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a> = <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
};
</pre>

Describes how <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>encode</a> <a href='SkColor_Reference#Color'>color</a>. <a href='SkColor_Reference#Color'>A</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>an</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>mask</a>, <a href='SkColor_Reference#Alpha'>a</a>
<a href='SkColor_Reference#Alpha'>grayscale</a>, <a href='SkColor_Reference#Alpha'>RGB</a>, <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>ARGB</a>.

<a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>selects</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>the</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>native</a> 32-<a href='SkImageInfo_Reference#kN32_SkColorType'>bit</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>ARGB</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>format</a>. <a href='SkImageInfo_Reference#kN32_SkColorType'>On</a> <a href='#Little_Endian'>Little_Endian</a>
<a href='#Little_Endian'>processors</a>, <a href='#Little_Endian'>pixels</a> <a href='#Little_Endian'>containing</a> 8-<a href='#Little_Endian'>bit</a> <a href='#Little_Endian'>ARGB</a> <a href='#Little_Endian'>components</a> <a href='#Little_Endian'>pack</a> <a href='#Little_Endian'>into</a> 32-<a href='#Little_Endian'>bit</a>
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>. <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>On</a> <a href='#Big_Endian'>Big_Endian</a> <a href='#Big_Endian'>processors</a>, <a href='#Big_Endian'>pixels</a> <a href='#Big_Endian'>pack</a> <a href='#Big_Endian'>into</a> 32-<a href='#Big_Endian'>bit</a>
<a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kUnknown_SkColorType'><code>kUnknown_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>is</a> <a href='#Image_Info_Color_Type'>set</a> <a href='#Image_Info_Color_Type'>to</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>by</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>default</a>. <a href='SkImageInfo_Reference#kUnknown_SkColorType'>If</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>set</a>,
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>encoding</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>format</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>and</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>unknown</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kAlpha_8_SkColorType'><code>kAlpha_8_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Alpha_8'>Alpha&nbsp;8</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 8-bit byte <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>encoding</a> <a href='undocumented#Pixel'>that</a> <a href='undocumented#Pixel'>represents</a> <a href='undocumented#Pixel'>transparency</a>. <a href='undocumented#Pixel'>Value</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>zero</a>
<a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>completely</a> <a href='undocumented#Pixel'>transparent</a>; <a href='undocumented#Pixel'>a</a> <a href='undocumented#Pixel'>value</a> <a href='undocumented#Pixel'>of</a> 255 <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>completely</a> <a href='undocumented#Pixel'>opaque</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGB_565_SkColorType'><code>kRGB_565_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGB_565'>RGB&nbsp;565</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 16-bit word <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>encoding</a> <a href='undocumented#Pixel'>that</a> <a href='undocumented#Pixel'>contains</a> <a href='undocumented#Pixel'>five</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>blue</a>,
<a href='undocumented#Pixel'>six</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>green</a>, <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>five</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>red</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kARGB_4444_SkColorType'><code>kARGB_4444_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#ARGB_4444'>ARGB&nbsp;4444</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 16-bit word <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>encoding</a> <a href='undocumented#Pixel'>that</a> <a href='undocumented#Pixel'>contains</a> <a href='undocumented#Pixel'>four</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='SkColor_Reference#Alpha'>alpha</a>,
<a href='SkColor_Reference#Alpha'>four</a> <a href='SkColor_Reference#Alpha'>bits</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkColor_Reference#Alpha'>blue</a>, <a href='SkColor_Reference#Alpha'>four</a> <a href='SkColor_Reference#Alpha'>bits</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>four</a> <a href='SkColor_Reference#Alpha'>bits</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkColor_Reference#Alpha'>red</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGBA_8888_SkColorType'><code>kRGBA_8888_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGBA_8888'>RGBA&nbsp;8888</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 32-bit word <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>encoding</a> <a href='undocumented#Pixel'>that</a> <a href='undocumented#Pixel'>contains</a> <a href='undocumented#Pixel'>eight</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>red</a>,
<a href='undocumented#Pixel'>eight</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>green</a>, <a href='undocumented#Pixel'>eight</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>blue</a>, <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>eight</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='SkColor_Reference#Alpha'>alpha</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGB_888x_SkColorType'><code>kRGB_888x_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGB_888'>RGB&nbsp;888</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 32-bit word <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>encoding</a> <a href='undocumented#Pixel'>that</a> <a href='undocumented#Pixel'>contains</a> <a href='undocumented#Pixel'>eight</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>red</a>,
<a href='undocumented#Pixel'>eight</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>green</a>, <a href='undocumented#Pixel'>eight</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>blue</a>, <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>eight</a> <a href='undocumented#Pixel'>unused</a> <a href='undocumented#Pixel'>bits</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kBGRA_8888_SkColorType'><code>kBGRA_8888_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>6</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#BGRA_8888'>BGRA&nbsp;8888</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 32-bit word <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>encoding</a> <a href='undocumented#Pixel'>that</a> <a href='undocumented#Pixel'>contains</a> <a href='undocumented#Pixel'>eight</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>blue</a>,
<a href='undocumented#Pixel'>eight</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>green</a>, <a href='undocumented#Pixel'>eight</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>red</a>, <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>eight</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='SkColor_Reference#Alpha'>alpha</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGBA_1010102_SkColorType'><code>kRGBA_1010102_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>7</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGBA_1010102'>RGBA&nbsp;1010102</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 32-bit word <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>encoding</a> <a href='undocumented#Pixel'>that</a> <a href='undocumented#Pixel'>contains</a> <a href='undocumented#Pixel'>ten</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>red</a>,
<a href='undocumented#Pixel'>ten</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>green</a>, <a href='undocumented#Pixel'>ten</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>blue</a>, <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>two</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='SkColor_Reference#Alpha'>alpha</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGB_101010x_SkColorType'><code>kRGB_101010x_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGB_101010'>RGB&nbsp;101010</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 32-bit word <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>encoding</a> <a href='undocumented#Pixel'>that</a> <a href='undocumented#Pixel'>contains</a> <a href='undocumented#Pixel'>ten</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>red</a>,
<a href='undocumented#Pixel'>ten</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>green</a>, <a href='undocumented#Pixel'>ten</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>blue</a>, <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>two</a> <a href='undocumented#Pixel'>unused</a> <a href='undocumented#Pixel'>bits</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kGray_8_SkColorType'><code>kGray_8_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>9</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Gray_8'>Gray&nbsp;8</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 8-bit byte <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>encoding</a> <a href='undocumented#Pixel'>that</a> <a href='undocumented#Pixel'>equivalent</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>equal</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>for</a> <a href='undocumented#Pixel'>red</a>,
<a href='undocumented#Pixel'>blue</a>, <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>green</a>, <a href='undocumented#Pixel'>representing</a> <a href='undocumented#Pixel'>colors</a> <a href='undocumented#Pixel'>from</a> <a href='undocumented#Pixel'>black</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>white</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGBA_F16_SkColorType'><code>kRGBA_F16_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>10</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGBA_F16'>RGBA&nbsp;F16</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 64-bit word <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>encoding</a> <a href='undocumented#Pixel'>that</a> <a href='undocumented#Pixel'>contains</a> 16 <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>blue</a>,
16 <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>green</a>, 16 <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>red</a>, <a href='undocumented#Pixel'>and</a> 16 <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='SkColor_Reference#Alpha'>alpha</a>. <a href='SkColor_Reference#Alpha'>Each</a> <a href='SkColor_Reference#Alpha'>component</a>
<a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>encoded</a> <a href='SkColor_Reference#Alpha'>as</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='SkColor_Reference#Alpha'>half</a> <a href='SkColor_Reference#Alpha'>float</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGBA_F32_SkColorType'><code>kRGBA_F32_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>11</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGBA_F32'>RGBA&nbsp;F32</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 128-bit word <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>encoding</a> <a href='undocumented#Pixel'>that</a> <a href='undocumented#Pixel'>contains</a> 32 <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>blue</a>,
32 <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>green</a>, 32 <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>red</a>, <a href='undocumented#Pixel'>and</a> 32 <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>of</a> <a href='SkColor_Reference#Alpha'>alpha</a>. <a href='SkColor_Reference#Alpha'>Each</a> <a href='SkColor_Reference#Alpha'>component</a>
<a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>encoded</a> <a href='SkColor_Reference#Alpha'>as</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='SkColor_Reference#Alpha'>single</a> <a href='SkColor_Reference#Alpha'>precision</a> <a href='SkColor_Reference#Alpha'>float</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kLastEnum_SkColorType'><code>kLastEnum_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>11</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Used by tests to iterate through all valid values.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kN32_SkColorType'><code>kN32_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4 or 6</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Encodes ARGB as either <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>or</a>
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>whichever</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>is</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>native</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>to</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>the</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>platform</a>.
</td>
  </tr>
</table>

### See Also

<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>

<a name='Color_Type_Alpha_8'></a>

---

<a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>pixels</a> <a href='SkColor_Reference#Alpha'>encode</a> <a href='SkColor_Reference#Alpha'>transparency</a> <a href='SkColor_Reference#Alpha'>without</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>information</a>. <a href='SkColor_Reference#Color'>Value</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>zero</a> <a href='SkColor_Reference#Color'>is</a>
<a href='SkColor_Reference#Color'>completely</a> <a href='SkColor_Reference#Color'>transparent</a>; <a href='SkColor_Reference#Color'>a</a> <a href='SkColor_Reference#Color'>value</a> <a href='SkColor_Reference#Color'>of</a> 255 <a href='SkColor_Reference#Color'>is</a> <a href='SkColor_Reference#Color'>completely</a> <a href='SkColor_Reference#Color'>opaque</a>. <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>
<a href='SkBitmap_Reference#Bitmap'>pixels</a> <a href='SkBitmap_Reference#Bitmap'>do</a> <a href='SkBitmap_Reference#Bitmap'>not</a> <a href='SkBitmap_Reference#Bitmap'>visibly</a> <a href='SkBitmap_Reference#Bitmap'>draw</a>, <a href='SkBitmap_Reference#Bitmap'>because</a> <a href='SkBitmap_Reference#Bitmap'>its</a> <a href='SkBitmap_Reference#Bitmap'>pixels</a> <a href='SkBitmap_Reference#Bitmap'>have</a> <a href='SkBitmap_Reference#Bitmap'>no</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>information</a>.
<a href='SkColor_Reference#Color'>When</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>set</a> <a href='SkImageInfo_Reference#SkColorType'>to</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>the</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>paired</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a>
<a href='SkImageInfo_Reference#SkAlphaType'>ignored</a>.

### Example

<div><fiddle-embed name="21ae21e4ce53d2018e042dd457997300"><div><a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>pixels</a> <a href='SkColor_Reference#Alpha'>can</a> <a href='SkColor_Reference#Alpha'>modify</a> <a href='SkColor_Reference#Alpha'>another</a> <a href='SkColor_Reference#Alpha'>draw</a>. <a href='SkColor_Reference#Alpha'>orangePaint</a> <a href='SkColor_Reference#Alpha'>fills</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>bounds</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a>,
<a href='SkBitmap_Reference#Bitmap'>with</a> <a href='SkBitmap_Reference#Bitmap'>its</a> <a href='SkBitmap_Reference#Bitmap'>transparency</a> <a href='SkBitmap_Reference#Bitmap'>set</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>alpha8</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>value</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#Alpha'>Alpha</a> <a href='#Image_Info_Color_Type_Gray_8'>Gray_8</a>

<a name='Color_Type_RGB_565'></a>

---

<a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>encodes</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>RGB</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>to</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>fit</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>in</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>a</a> 16-<a href='SkImageInfo_Reference#kRGB_565_SkColorType'>bit</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>word</a>. <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>Red</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>and</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>blue</a>
<a href='SkImageInfo_Reference#kRGB_565_SkColorType'>components</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>use</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>five</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>bits</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>describing</a> 32 <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>levels</a>. <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>Green</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>components</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>more</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>sensitive</a>
<a href='SkImageInfo_Reference#kRGB_565_SkColorType'>to</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>the</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>eye</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>use</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>six</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>bits</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>describing</a> 64 <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>levels</a>. <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>has</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>no</a>
<a href='SkImageInfo_Reference#kRGB_565_SkColorType'>bits</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>for</a> <a href='SkColor_Reference#Alpha'>Alpha</a>.
<a href='SkColor_Reference#Alpha'>Pixels</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a> <a href='SkColor_Reference#Alpha'>as</a> <a href='SkColor_Reference#Alpha'>if</a> <a href='SkColor_Reference#Alpha'>its</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>was</a> <a href='#Color_Alpha'>set</a> <a href='#Color_Alpha'>to</a> <a href='#Color_Alpha'>one</a>, <a href='#Color_Alpha'>and</a> <a href='#Color_Alpha'>should</a>
<a href='#Color_Alpha'>always</a> <a href='#Color_Alpha'>be</a> <a href='#Color_Alpha'>paired</a> <a href='#Color_Alpha'>with</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.

![Color_Type_RGB_565](https://fiddle.skia.org/i/6dec0226490a4ac1977dc87a31564147_raster.png "")

### Example

<div><fiddle-embed name="7e7c46bb4572e21e13529ff364eb0a9c"></fiddle-embed></div>

### See Also

<a href='#Image_Info_Color_Type_ARGB_4444'>ARGB_4444</a> <a href='#Image_Info_Color_Type_RGBA_8888'>RGBA_8888</a>

<a name='Color_Type_ARGB_4444'></a>

---

<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>encodes</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>ARGB</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>to</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>fit</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>in</a> 16-<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>bit</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>word</a>. <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>Each</a>
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>component</a>: <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>blue</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>red</a>; <a href='SkColor_Reference#Alpha'>use</a> <a href='SkColor_Reference#Alpha'>four</a> <a href='SkColor_Reference#Alpha'>bits</a>, <a href='SkColor_Reference#Alpha'>describing</a> 16 <a href='SkColor_Reference#Alpha'>levels</a>.
<a href='SkColor_Reference#Alpha'>Note</a> <a href='SkColor_Reference#Alpha'>that</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>is</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>misnamed</a>; <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>the</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>acronym</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>does</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>not</a>
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>describe</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>the</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>actual</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>component</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>order</a>.

![Color_Type_ARGB_4444](https://fiddle.skia.org/i/e8008512f0d197051e3f26faa67bafc2_raster.png "")

If paired with <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>: <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>blue</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>green</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>red</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>components</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>are</a>
<a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>by</a> <a href='undocumented#Premultiply'>the</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>value</a>. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>blue</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>red</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>greater</a> <a href='SkColor_Reference#Alpha'>than</a> <a href='SkColor_Reference#Alpha'>alpha</a>,
<a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>drawn</a> <a href='SkColor_Reference#Alpha'>result</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

<a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>paired</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>: <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>blue</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>red</a> <a href='SkColor_Reference#Alpha'>components</a>
<a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>have</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='SkColor_Reference#Alpha'>value</a>. <a href='SkColor_Reference#Alpha'>There</a> <a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>be</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='SkColor_Reference#Alpha'>performance</a> <a href='SkColor_Reference#Alpha'>penalty</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a>
<a href='undocumented#Unpremultiply'>pixels</a>.

<a href='undocumented#Unpremultiply'>If</a> <a href='undocumented#Unpremultiply'>paired</a> <a href='undocumented#Unpremultiply'>with</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>: <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>all</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>at</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>maximum</a>;
<a href='SkColor_Reference#Alpha'>blue</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>red</a> <a href='SkColor_Reference#Alpha'>components</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a>. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>is</a>
<a href='SkColor_Reference#Alpha'>less</a> <a href='SkColor_Reference#Alpha'>than</a> 15, <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>drawn</a> <a href='SkColor_Reference#Alpha'>result</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

### Example

<div><fiddle-embed name="33a360c3404ac21db801943336843d8e"></fiddle-embed></div>

### See Also

<a href='#Image_Info_Color_Type_RGBA_8888'>RGBA_8888</a>

<a name='Color_Type_RGBA_8888'></a>

---

<a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>encodes</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>ARGB</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>into</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>a</a> 32-<a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>bit</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>word</a>. <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>Each</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>component</a>:
<a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>red</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>green</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>blue</a>, <a href='SkColor_Reference#Alpha'>alpha</a>; <a href='SkColor_Reference#Alpha'>use</a> <a href='SkColor_Reference#Alpha'>eight</a> <a href='SkColor_Reference#Alpha'>bits</a>, <a href='SkColor_Reference#Alpha'>describing</a> 256 <a href='SkColor_Reference#Alpha'>levels</a>.

![Color_Type_RGBA_8888](https://fiddle.skia.org/i/9abc324f670e6468f09385551aae5a1c_raster.png "")

If paired with <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>: <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>red</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>green</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>blue</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>components</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>are</a>
<a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>by</a> <a href='undocumented#Premultiply'>the</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>value</a>. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>red</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>blue</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>greater</a> <a href='SkColor_Reference#Alpha'>than</a> <a href='SkColor_Reference#Alpha'>alpha</a>,
<a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>drawn</a> <a href='SkColor_Reference#Alpha'>result</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

<a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>paired</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>: <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>red</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>blue</a> <a href='SkColor_Reference#Alpha'>components</a>
<a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>have</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='SkColor_Reference#Alpha'>value</a>. <a href='SkColor_Reference#Alpha'>There</a> <a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>be</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='SkColor_Reference#Alpha'>performance</a> <a href='SkColor_Reference#Alpha'>penalty</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a>
<a href='undocumented#Unpremultiply'>pixels</a>.

<a href='undocumented#Unpremultiply'>If</a> <a href='undocumented#Unpremultiply'>paired</a> <a href='undocumented#Unpremultiply'>with</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>: <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>all</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>at</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>maximum</a>;
<a href='SkColor_Reference#Alpha'>red</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>blue</a> <a href='SkColor_Reference#Alpha'>components</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a>. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>is</a>
<a href='SkColor_Reference#Alpha'>less</a> <a href='SkColor_Reference#Alpha'>than</a> 255, <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>drawn</a> <a href='SkColor_Reference#Alpha'>result</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

<a href='SkColor_Reference#Alpha'>On</a> <a href='#Big_Endian'>Big_Endian</a> <a href='#Big_Endian'>platforms</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>is</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>the</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>native</a> <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Color_Type'>and</a>
<a href='#Image_Info_Color_Type'>will</a> <a href='#Image_Info_Color_Type'>have</a> <a href='#Image_Info_Color_Type'>the</a> <a href='#Image_Info_Color_Type'>best</a> <a href='#Image_Info_Color_Type'>performance</a>. <a href='#Image_Info_Color_Type'>Use</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>to</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>choose</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>the</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>best</a>
<a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>for</a> <a href='#Image_Info_Color_Type'>the</a> <a href='#Image_Info_Color_Type'>platform</a> <a href='#Image_Info_Color_Type'>at</a> <a href='#Image_Info_Color_Type'>compile</a> <a href='#Image_Info_Color_Type'>time</a>.

### Example

<div><fiddle-embed name="947922a19d59893fe7f9d9ee1954379b"></fiddle-embed></div>

### See Also

<a href='#Image_Info_Color_Type_RGB_888'>RGB_888</a> <a href='#Image_Info_Color_Type_BGRA_8888'>BGRA_8888</a>

<a name='Color_Type_RGB_888'></a>

---

<a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>encodes</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>RGB</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>into</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>a</a> 32-<a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>bit</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>word</a>. <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>Each</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>component</a>:
<a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>red</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>green</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>blue</a>; <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>use</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>eight</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>bits</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>describing</a> 256 <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>levels</a>. <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>Eight</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>bits</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>are</a>
<a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>unused</a>. <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>Pixels</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>described</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>by</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>are</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>fully</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>opaque</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>as</a> <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>if</a>
<a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>their</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>was</a> <a href='#Color_Alpha'>set</a> <a href='#Color_Alpha'>to</a> <a href='#Color_Alpha'>one</a>, <a href='#Color_Alpha'>and</a> <a href='#Color_Alpha'>should</a> <a href='#Color_Alpha'>always</a> <a href='#Color_Alpha'>be</a> <a href='#Color_Alpha'>paired</a> <a href='#Color_Alpha'>with</a>
<a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.

![Color_Type_RGB_888](https://fiddle.skia.org/i/7527d7ade4764302818e250cd4e03962_raster.png "")

### Example

<div><fiddle-embed name="4260d6cc15db2c60c07f6fdc8d9ae425"></fiddle-embed></div>

### See Also

<a href='#Image_Info_Color_Type_RGBA_8888'>RGBA_8888</a> <a href='#Image_Info_Color_Type_BGRA_8888'>BGRA_8888</a>

<a name='Color_Type_BGRA_8888'></a>

---

<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>encodes</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>ARGB</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>into</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>a</a> 32-<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>bit</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>word</a>. <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>Each</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>component</a>:
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>blue</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>green</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>red</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>and</a> <a href='SkColor_Reference#Alpha'>alpha</a>; <a href='SkColor_Reference#Alpha'>use</a> <a href='SkColor_Reference#Alpha'>eight</a> <a href='SkColor_Reference#Alpha'>bits</a>, <a href='SkColor_Reference#Alpha'>describing</a> 256 <a href='SkColor_Reference#Alpha'>levels</a>.

![Color_Type_BGRA_8888](https://fiddle.skia.org/i/6c35ca14d88b0de200ba7f897f889ad7_raster.png "")

If paired with <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>: <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>blue</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>green</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>red</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>components</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>are</a>
<a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>by</a> <a href='undocumented#Premultiply'>the</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>value</a>. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>blue</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>red</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>greater</a> <a href='SkColor_Reference#Alpha'>than</a> <a href='SkColor_Reference#Alpha'>alpha</a>,
<a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>drawn</a> <a href='SkColor_Reference#Alpha'>result</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

<a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>paired</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>: <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>blue</a>, <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>green</a>, <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>red</a>, <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>and</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>components</a>
<a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>have</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='SkColor_Reference#Alpha'>value</a>. <a href='SkColor_Reference#Alpha'>There</a> <a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>be</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='SkColor_Reference#Alpha'>performance</a> <a href='SkColor_Reference#Alpha'>penalty</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a>
<a href='undocumented#Unpremultiply'>pixels</a>.

<a href='undocumented#Unpremultiply'>If</a> <a href='undocumented#Unpremultiply'>paired</a> <a href='undocumented#Unpremultiply'>with</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>: <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>all</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>at</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>maximum</a>;
<a href='SkColor_Reference#Alpha'>blue</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>red</a> <a href='SkColor_Reference#Alpha'>components</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a>. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>is</a>
<a href='SkColor_Reference#Alpha'>less</a> <a href='SkColor_Reference#Alpha'>than</a> 255, <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>drawn</a> <a href='SkColor_Reference#Alpha'>result</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

<a href='SkColor_Reference#Alpha'>On</a> <a href='#Little_Endian'>Little_Endian</a> <a href='#Little_Endian'>platforms</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>is</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>the</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>native</a> <a href='#Image_Info_Color_Type'>Color_Type</a>,
<a href='#Image_Info_Color_Type'>and</a> <a href='#Image_Info_Color_Type'>will</a> <a href='#Image_Info_Color_Type'>have</a> <a href='#Image_Info_Color_Type'>the</a> <a href='#Image_Info_Color_Type'>best</a> <a href='#Image_Info_Color_Type'>performance</a>. <a href='#Image_Info_Color_Type'>Use</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>to</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>choose</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>the</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>best</a>
<a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>for</a> <a href='#Image_Info_Color_Type'>the</a> <a href='#Image_Info_Color_Type'>platform</a> <a href='#Image_Info_Color_Type'>at</a> <a href='#Image_Info_Color_Type'>compile</a> <a href='#Image_Info_Color_Type'>time</a>.

### Example

<div><fiddle-embed name="945ce5344fce5470f8604b2e06e9f9ae"></fiddle-embed></div>

### See Also

<a href='#Image_Info_Color_Type_RGBA_8888'>RGBA_8888</a>

<a name='Color_Type_RGBA_1010102'></a>

---

<a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>encodes</a> <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>ARGB</a> <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>into</a> <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>a</a> 32-<a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>bit</a> <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>word</a>. <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>Each</a>
<a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>: <a href='SkColor_Reference#Color'>red</a>, <a href='SkColor_Reference#Color'>green</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>blue</a>; <a href='SkColor_Reference#Color'>use</a> <a href='SkColor_Reference#Color'>ten</a> <a href='SkColor_Reference#Color'>bits</a>, <a href='SkColor_Reference#Color'>describing</a> 1024 <a href='SkColor_Reference#Color'>levels</a>.
<a href='SkColor_Reference#Color'>Two</a> <a href='SkColor_Reference#Color'>bits</a> <a href='SkColor_Reference#Color'>contain</a> <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>describing</a> <a href='SkColor_Reference#Alpha'>four</a> <a href='SkColor_Reference#Alpha'>levels</a>. <a href='SkColor_Reference#Alpha'>Possible</a> <a href='SkColor_Reference#Alpha'>alpha</a>
<a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>zero</a>: <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>transparent</a>; <a href='SkColor_Reference#Alpha'>one</a>: 33% <a href='SkColor_Reference#Alpha'>opaque</a>; <a href='SkColor_Reference#Alpha'>two</a>: 67% <a href='SkColor_Reference#Alpha'>opaque</a>;
<a href='SkColor_Reference#Alpha'>three</a>: <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a>.

<a href='SkColor_Reference#Alpha'>At</a> <a href='SkColor_Reference#Alpha'>present</a>, <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>in</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>does</a> <a href='SkPaint_Reference#Paint'>not</a> <a href='SkPaint_Reference#Paint'>provide</a> <a href='SkPaint_Reference#Paint'>enough</a> <a href='SkPaint_Reference#Paint'>precision</a> <a href='SkPaint_Reference#Paint'>to</a>
<a href='SkPaint_Reference#Paint'>draw</a> <a href='SkPaint_Reference#Paint'>all</a> <a href='SkPaint_Reference#Paint'>colors</a> <a href='SkPaint_Reference#Paint'>possible</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='SkPaint_Reference#Paint'>a</a> <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

![Color_Type_RGBA_1010102](https://fiddle.skia.org/i/8d78daf69145f611054f289a7443a670_raster.png "")

If paired with <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>: <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>red</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>green</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>blue</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>components</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>are</a>
<a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>by</a> <a href='undocumented#Premultiply'>the</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>value</a>. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>red</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>blue</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>greater</a> <a href='SkColor_Reference#Alpha'>than</a> <a href='SkColor_Reference#Alpha'>the</a>
<a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>replicated</a> <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>ten</a> <a href='SkColor_Reference#Alpha'>bits</a>, <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>drawn</a> <a href='SkColor_Reference#Alpha'>result</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

<a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>paired</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>: <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>red</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>blue</a> <a href='SkColor_Reference#Alpha'>components</a>
<a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>have</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='SkColor_Reference#Alpha'>value</a>. <a href='SkColor_Reference#Alpha'>There</a> <a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>be</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='SkColor_Reference#Alpha'>performance</a> <a href='SkColor_Reference#Alpha'>penalty</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a>
<a href='undocumented#Unpremultiply'>pixels</a>.

<a href='undocumented#Unpremultiply'>If</a> <a href='undocumented#Unpremultiply'>paired</a> <a href='undocumented#Unpremultiply'>with</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>: <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>all</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>at</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>maximum</a>;
<a href='SkColor_Reference#Alpha'>red</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>blue</a> <a href='SkColor_Reference#Alpha'>components</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a>. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>is</a>
<a href='SkColor_Reference#Alpha'>less</a> <a href='SkColor_Reference#Alpha'>than</a> <a href='SkColor_Reference#Alpha'>three</a>, <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>drawn</a> <a href='SkColor_Reference#Alpha'>result</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

### Example

<div><fiddle-embed name="1282dc1127ce1b0061544619ae4de0f0"></fiddle-embed></div>

### See Also

<a href='#Image_Info_Color_Type_RGB_101010'>RGB_101010</a> <a href='#Image_Info_Color_Type_RGBA_8888'>RGBA_8888</a>

<a name='Color_Type_RGB_101010'></a>

---

<a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a> <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>encodes</a> <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>RGB</a> <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>into</a> <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>a</a> 32-<a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>bit</a> <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>word</a>. <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>Each</a>
<a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>component</a>: <a href='SkColor_Reference#Color'>red</a>, <a href='SkColor_Reference#Color'>green</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>blue</a>; <a href='SkColor_Reference#Color'>use</a> <a href='SkColor_Reference#Color'>ten</a> <a href='SkColor_Reference#Color'>bits</a>, <a href='SkColor_Reference#Color'>describing</a> 1024 <a href='SkColor_Reference#Color'>levels</a>.
<a href='SkColor_Reference#Color'>Two</a> <a href='SkColor_Reference#Color'>bits</a> <a href='SkColor_Reference#Color'>are</a> <a href='SkColor_Reference#Color'>unused</a>. <a href='SkColor_Reference#Color'>Pixels</a> <a href='SkColor_Reference#Color'>described</a> <a href='SkColor_Reference#Color'>by</a> <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a> <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>are</a> <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>fully</a>
<a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>opaque</a> <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>as</a> <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>if</a> <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>its</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>was</a> <a href='#Color_Alpha'>set</a> <a href='#Color_Alpha'>to</a> <a href='#Color_Alpha'>one</a>, <a href='#Color_Alpha'>and</a> <a href='#Color_Alpha'>should</a> <a href='#Color_Alpha'>always</a> <a href='#Color_Alpha'>be</a> <a href='#Color_Alpha'>paired</a>
<a href='#Color_Alpha'>with</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.

<a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>At</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>present</a>, <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>in</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>does</a> <a href='SkPaint_Reference#Paint'>not</a> <a href='SkPaint_Reference#Paint'>provide</a> <a href='SkPaint_Reference#Paint'>enough</a> <a href='SkPaint_Reference#Paint'>precision</a> <a href='SkPaint_Reference#Paint'>to</a>
<a href='SkPaint_Reference#Paint'>draw</a> <a href='SkPaint_Reference#Paint'>all</a> <a href='SkPaint_Reference#Paint'>colors</a> <a href='SkPaint_Reference#Paint'>possible</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='SkPaint_Reference#Paint'>a</a> <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

![Color_Type_RGB_101010](https://fiddle.skia.org/i/4c9f4d939e2047269d73fa3507caf01f_raster.png "")

### Example

<div><fiddle-embed name="92f81aa0459230459600a01e79ccff29"></fiddle-embed></div>

### See Also

<a href='#Image_Info_Color_Type_RGBA_1010102'>RGBA_1010102</a>

<a name='Color_Type_Gray_8'></a>

---

<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>encodes</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>grayscale</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>level</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>in</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>eight</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>bits</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>that</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>equivalent</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>to</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>equal</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>values</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>for</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>red</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>blue</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>and</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>green</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>representing</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>colors</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>from</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>black</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>to</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>white</a>.  <a href='SkImageInfo_Reference#kGray_8_SkColorType'>Pixels</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>described</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>by</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>are</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>fully</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>opaque</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>as</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>if</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>its</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>was</a> <a href='#Color_Alpha'>set</a> <a href='#Color_Alpha'>to</a> <a href='#Color_Alpha'>one</a>, <a href='#Color_Alpha'>and</a> <a href='#Color_Alpha'>should</a> <a href='#Color_Alpha'>always</a> <a href='#Color_Alpha'>be</a> <a href='#Color_Alpha'>paired</a> <a href='#Color_Alpha'>with</a>
<a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.

### Example

<div><fiddle-embed name="93da0eb0b6722a4f33dc7dae094abf0b"></fiddle-embed></div>

### See Also

<a href='#Image_Info_Color_Type_Alpha_8'>Alpha_8</a>

<a name='Color_Type_RGBA_F16'></a>

---

<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>encodes</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>ARGB</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>into</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>a</a> 64-<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>bit</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>word</a>. <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>Each</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>component</a>:
<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>blue</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>green</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>red</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>and</a> <a href='SkColor_Reference#Alpha'>alpha</a>; <a href='SkColor_Reference#Alpha'>use</a> 16 <a href='SkColor_Reference#Alpha'>bits</a>, <a href='SkColor_Reference#Alpha'>describing</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='SkColor_Reference#Alpha'>floating</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>value</a>,
<a href='SkPoint_Reference#Point'>from</a> -65500 <a href='SkPoint_Reference#Point'>to</a> 65000 <a href='SkPoint_Reference#Point'>with</a> 3.31 <a href='SkPoint_Reference#Point'>decimal</a> <a href='SkPoint_Reference#Point'>digits</a> <a href='SkPoint_Reference#Point'>of</a> <a href='SkPoint_Reference#Point'>precision</a>.

<a href='SkPoint_Reference#Point'>At</a> <a href='SkPoint_Reference#Point'>present</a>, <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>in</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>does</a> <a href='SkPaint_Reference#Paint'>not</a> <a href='SkPaint_Reference#Paint'>provide</a> <a href='SkPaint_Reference#Paint'>enough</a> <a href='SkPaint_Reference#Paint'>precision</a> <a href='SkPaint_Reference#Paint'>or</a> <a href='SkPaint_Reference#Paint'>range</a> <a href='SkPaint_Reference#Paint'>to</a>
<a href='SkPaint_Reference#Paint'>draw</a> <a href='SkPaint_Reference#Paint'>all</a> <a href='SkPaint_Reference#Paint'>colors</a> <a href='SkPaint_Reference#Paint'>possible</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='SkPaint_Reference#Paint'>a</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

<a href='SkSurface_Reference#Surface'>Each</a> <a href='SkSurface_Reference#Surface'>component</a> <a href='SkSurface_Reference#Surface'>encodes</a> <a href='SkSurface_Reference#Surface'>a</a> <a href='SkSurface_Reference#Surface'>floating</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>value</a> <a href='SkPoint_Reference#Point'>using</a>
<a href='https://www.khronos.org/opengl/wiki/Small_Float_Formats'>Half floats</a></a>. Meaningful colors are represented by the range 0.0 to 1.0, although smaller
and larger values may be useful when used in combination with <a href='#Transfer_Mode'>Transfer_Mode</a>.

![Color_Type_RGBA_F16](https://fiddle.skia.org/i/1bb35ae52173e0fef874022ca8138adc_raster.png "")

If paired with <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>: <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>blue</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>green</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>red</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>components</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>are</a>
<a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>by</a> <a href='undocumented#Premultiply'>the</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>value</a>. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>blue</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>red</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>greater</a> <a href='SkColor_Reference#Alpha'>than</a> <a href='SkColor_Reference#Alpha'>alpha</a>,
<a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>drawn</a> <a href='SkColor_Reference#Alpha'>result</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

<a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>paired</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>: <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>blue</a>, <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>green</a>, <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>red</a>, <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>and</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>components</a>
<a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>have</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='SkColor_Reference#Alpha'>value</a>. <a href='SkColor_Reference#Alpha'>There</a> <a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>be</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='SkColor_Reference#Alpha'>performance</a> <a href='SkColor_Reference#Alpha'>penalty</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a>
<a href='undocumented#Unpremultiply'>pixels</a>.

<a href='undocumented#Unpremultiply'>If</a> <a href='undocumented#Unpremultiply'>paired</a> <a href='undocumented#Unpremultiply'>with</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>: <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>all</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>at</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>maximum</a>;
<a href='SkColor_Reference#Alpha'>blue</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>red</a> <a href='SkColor_Reference#Alpha'>components</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a>. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>is</a>
<a href='SkColor_Reference#Alpha'>less</a> <a href='SkColor_Reference#Alpha'>than</a> <a href='SkColor_Reference#Alpha'>one</a>, <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>drawn</a> <a href='SkColor_Reference#Alpha'>result</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

### Example

<div><fiddle-embed name="dd81527bbdf5eaae7dd21ac04ab84f9e"></fiddle-embed></div>

### See Also

<a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>

<a name='Color_Type_RGBA_F32'></a>

---

<a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>kRGBA_F32_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>encodes</a> <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>ARGB</a> <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>into</a> <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>a</a> 128-<a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>bit</a> <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>word</a>. <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>Each</a> <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>component</a>:
<a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>blue</a>, <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>green</a>, <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>red</a>, <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>and</a> <a href='SkColor_Reference#Alpha'>alpha</a>; <a href='SkColor_Reference#Alpha'>use</a> 32 <a href='SkColor_Reference#Alpha'>bits</a>, <a href='SkColor_Reference#Alpha'>describing</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='SkColor_Reference#Alpha'>floating</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>value</a>,
<a href='SkPoint_Reference#Point'>from</a> -3.402823<a href='SkPoint_Reference#Point'>e</a>+38 <a href='SkPoint_Reference#Point'>to</a> 3.402823<a href='SkPoint_Reference#Point'>e</a>+38 <a href='SkPoint_Reference#Point'>with</a> 7.225 <a href='SkPoint_Reference#Point'>decimal</a> <a href='SkPoint_Reference#Point'>digits</a> <a href='SkPoint_Reference#Point'>of</a> <a href='SkPoint_Reference#Point'>precision</a>.

<a href='SkPoint_Reference#Point'>At</a> <a href='SkPoint_Reference#Point'>present</a>, <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>in</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>does</a> <a href='SkPaint_Reference#Paint'>not</a> <a href='SkPaint_Reference#Paint'>provide</a> <a href='SkPaint_Reference#Paint'>enough</a> <a href='SkPaint_Reference#Paint'>precision</a> <a href='SkPaint_Reference#Paint'>or</a> <a href='SkPaint_Reference#Paint'>range</a> <a href='SkPaint_Reference#Paint'>to</a>
<a href='SkPaint_Reference#Paint'>draw</a> <a href='SkPaint_Reference#Paint'>all</a> <a href='SkPaint_Reference#Paint'>colors</a> <a href='SkPaint_Reference#Paint'>possible</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='SkPaint_Reference#Paint'>a</a> <a href='SkImageInfo_Reference#kRGBA_F32_SkColorType'>kRGBA_F32_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

<a href='SkSurface_Reference#Surface'>Each</a> <a href='SkSurface_Reference#Surface'>component</a> <a href='SkSurface_Reference#Surface'>encodes</a> <a href='SkSurface_Reference#Surface'>a</a> <a href='SkSurface_Reference#Surface'>floating</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>value</a> <a href='SkPoint_Reference#Point'>using</a>
<a href='https://en.wikipedia.org/wiki/Single-precision_floating-point_format'>single-precision floats</a></a>. Meaningful colors are represented by the range 0.0 to 1.0, although smaller
and larger values may be useful when used in combination with <a href='#Transfer_Mode'>Transfer_Mode</a>.

![Color_Type_RGBA_F32](https://fiddle.skia.org/i/4ba31a8f9bc94a996f34da81ef541a9c_raster.png "")

If paired with <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>: <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>blue</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>green</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>red</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>components</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>are</a>
<a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>by</a> <a href='undocumented#Premultiply'>the</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>value</a>. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>blue</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>red</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>greater</a> <a href='SkColor_Reference#Alpha'>than</a> <a href='SkColor_Reference#Alpha'>alpha</a>,
<a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>drawn</a> <a href='SkColor_Reference#Alpha'>result</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

<a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>paired</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>: <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>blue</a>, <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>green</a>, <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>red</a>, <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>and</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>components</a>
<a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>have</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='SkColor_Reference#Alpha'>value</a>. <a href='SkColor_Reference#Alpha'>There</a> <a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>be</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='SkColor_Reference#Alpha'>performance</a> <a href='SkColor_Reference#Alpha'>penalty</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a>
<a href='undocumented#Unpremultiply'>pixels</a>.

<a href='undocumented#Unpremultiply'>If</a> <a href='undocumented#Unpremultiply'>paired</a> <a href='undocumented#Unpremultiply'>with</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>: <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>all</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>at</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>maximum</a>;
<a href='SkColor_Reference#Alpha'>blue</a>, <a href='SkColor_Reference#Alpha'>green</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>red</a> <a href='SkColor_Reference#Alpha'>components</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a>. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>is</a>
<a href='SkColor_Reference#Alpha'>less</a> <a href='SkColor_Reference#Alpha'>than</a> <a href='SkColor_Reference#Alpha'>one</a>, <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>drawn</a> <a href='SkColor_Reference#Alpha'>result</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>undefined</a>.

### See Also

<a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>

<a name='SkColorTypeBytesPerPixel'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='SkImageInfo_Reference#SkColorTypeBytesPerPixel'>SkColorTypeBytesPerPixel</a>(<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>ct</a>)
</pre>

Returns the number of bytes required to store a <a href='undocumented#Pixel'>pixel</a>, <a href='undocumented#Pixel'>including</a> <a href='undocumented#Pixel'>unused</a> <a href='undocumented#Pixel'>padding</a>.
Returns zero if <a href='#SkColorTypeBytesPerPixel_ct'>ct</a> <a href='#SkColorTypeBytesPerPixel_ct'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>or</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>invalid</a>.

### Parameters

<table>  <tr>    <td><a name='SkColorTypeBytesPerPixel_ct'><code><strong>ct</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Return Value

bytes per <a href='undocumented#Pixel'>pixel</a>

### Example

<div><fiddle-embed name="09ef49d07cb7005ba3e34d5ea53896f5"><a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>
</fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>

<a name='SkColorTypeIsAlwaysOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='SkImageInfo_Reference#SkColorTypeIsAlwaysOpaque'>SkColorTypeIsAlwaysOpaque</a>(<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>ct</a>)
</pre>

Returns true if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>always</a> <a href='SkImageInfo_Reference#SkColorType'>decodes</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>to</a> 1.0, <a href='SkColor_Reference#Alpha'>making</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='undocumented#Pixel'>pixel</a>
fully opaque. If true, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>does</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#SkColorType'>reserve</a> <a href='SkImageInfo_Reference#SkColorType'>bits</a> <a href='SkImageInfo_Reference#SkColorType'>to</a> <a href='SkImageInfo_Reference#SkColorType'>encode</a> <a href='SkColor_Reference#Alpha'>alpha</a>.

### Parameters

<table>  <tr>    <td><a name='SkColorTypeIsAlwaysOpaque_ct'><code><strong>ct</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Return Value

true if <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>always</a> <a href='SkColor_Reference#Alpha'>set</a> <a href='SkColor_Reference#Alpha'>to</a> 1.0

### Example

<div><fiddle-embed name="9b3eb5aaa0dfea9feee54e7650fa5446"><a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>
</fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkColorTypeValidateAlphaType'>SkColorTypeValidateAlphaType</a>

<a name='SkColorTypeValidateAlphaType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='SkImageInfo_Reference#SkColorTypeValidateAlphaType'>SkColorTypeValidateAlphaType</a>(<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>colorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>alphaType</a>,
                                  <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>* <a href='SkImageInfo_Reference#SkAlphaType'>canonical</a> = <a href='SkImageInfo_Reference#SkAlphaType'>nullptr</a>)
</pre>

Returns true if <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> <a href='#SkColorTypeValidateAlphaType_canonical'>can</a> <a href='#SkColorTypeValidateAlphaType_canonical'>be</a> <a href='#SkColorTypeValidateAlphaType_canonical'>set</a> <a href='#SkColorTypeValidateAlphaType_canonical'>to</a> <a href='#SkColorTypeValidateAlphaType_canonical'>a</a> <a href='#SkColorTypeValidateAlphaType_canonical'>valid</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>for</a> <a href='#SkColorTypeValidateAlphaType_colorType'>colorType</a>. <a href='#SkColorTypeValidateAlphaType_colorType'>If</a>
there is more than one valid <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>set</a> <a href='SkImageInfo_Reference#SkAlphaType'>to</a> <a href='#SkColorTypeValidateAlphaType_alphaType'>alphaType</a>, <a href='#SkColorTypeValidateAlphaType_alphaType'>if</a> <a href='#SkColorTypeValidateAlphaType_alphaType'>valid</a>.
If true is returned and <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> <a href='#SkColorTypeValidateAlphaType_canonical'>is</a> <a href='#SkColorTypeValidateAlphaType_canonical'>not</a> <a href='#SkColorTypeValidateAlphaType_canonical'>nullptr</a>, <a href='#SkColorTypeValidateAlphaType_canonical'>store</a> <a href='#SkColorTypeValidateAlphaType_canonical'>valid</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>.

Returns false only if <a href='#SkColorTypeValidateAlphaType_alphaType'>alphaType</a> <a href='#SkColorTypeValidateAlphaType_alphaType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>,  <a href='SkImageInfo_Reference#Color_Type'>color type</a> <a href='SkColor_Reference#Color'>is</a> <a href='SkColor_Reference#Color'>not</a>
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkColorType'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#SkColorType'>always</a> <a href='SkImageInfo_Reference#SkColorType'>opaque</a>. <a href='SkImageInfo_Reference#SkColorType'>If</a> <a href='SkImageInfo_Reference#SkColorType'>false</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>returned</a>,
<a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> <a href='#SkColorTypeValidateAlphaType_canonical'>is</a> <a href='#SkColorTypeValidateAlphaType_canonical'>ignored</a>.

For <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>: <a href='SkImageInfo_Reference#kUnknown_SkColorType'>set</a> <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> <a href='#SkColorTypeValidateAlphaType_canonical'>to</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>return</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>true</a>.
For <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>: <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>set</a> <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> <a href='#SkColorTypeValidateAlphaType_canonical'>to</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>or</a>
<a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>return</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>true</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>if</a> <a href='#SkColorTypeValidateAlphaType_alphaType'>alphaType</a> <a href='#SkColorTypeValidateAlphaType_alphaType'>is</a> <a href='#SkColorTypeValidateAlphaType_alphaType'>not</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.
For <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>and</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>: <a href='SkImageInfo_Reference#kGray_8_SkColorType'>set</a> <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> <a href='#SkColorTypeValidateAlphaType_canonical'>to</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>return</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>true</a>.
For <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>and</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>: <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>set</a> <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> <a href='#SkColorTypeValidateAlphaType_canonical'>to</a> <a href='#SkColorTypeValidateAlphaType_alphaType'>alphaType</a>
and return true if <a href='#SkColorTypeValidateAlphaType_alphaType'>alphaType</a> <a href='#SkColorTypeValidateAlphaType_alphaType'>is</a> <a href='#SkColorTypeValidateAlphaType_alphaType'>not</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.

### Parameters

<table>  <tr>    <td><a name='SkColorTypeValidateAlphaType_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Parameters

<table>  <tr>    <td><a name='SkColorTypeValidateAlphaType_alphaType'><code><strong>alphaType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Parameters

<table>  <tr>    <td><a name='SkColorTypeValidateAlphaType_canonical'><code><strong>canonical</strong></code></a></td>
    <td>storage for <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a></td>
  </tr>
</table>

### Return Value

true if valid <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>can</a> <a href='SkImageInfo_Reference#SkAlphaType'>be</a> <a href='SkImageInfo_Reference#SkAlphaType'>associated</a> <a href='SkImageInfo_Reference#SkAlphaType'>with</a> <a href='#SkColorTypeValidateAlphaType_colorType'>colorType</a>

### Example

<div><fiddle-embed name="befac1c29ed21507d367e4d824383a04"><a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>
</fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkColorTypeIsAlwaysOpaque'>SkColorTypeIsAlwaysOpaque</a>

<a name='YUV_ColorSpace'></a>

<a name='SkYUVColorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> {
    <a href='SkImageInfo_Reference#kJPEG_SkYUVColorSpace'>kJPEG_SkYUVColorSpace</a>,
    <a href='SkImageInfo_Reference#kRec601_SkYUVColorSpace'>kRec601_SkYUVColorSpace</a>,
    <a href='SkImageInfo_Reference#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>,
    <a href='SkImageInfo_Reference#kLastEnum_SkYUVColorSpace'>kLastEnum_SkYUVColorSpace</a> = <a href='SkImageInfo_Reference#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>,
};
</pre>

Describes <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>range</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>YUV</a> <a href='SkColor_Reference#Color'>pixels</a>. <a href='SkColor_Reference#Color'>The</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>mapping</a> <a href='SkColor_Reference#Color'>from</a> <a href='SkColor_Reference#Color'>YUV</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>RGB</a> <a href='SkColor_Reference#Color'>varies</a>
<a href='SkColor_Reference#Color'>depending</a> <a href='SkColor_Reference#Color'>on</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>source</a>. <a href='SkColor_Reference#Color'>YUV</a> <a href='SkColor_Reference#Color'>pixels</a> <a href='SkColor_Reference#Color'>may</a> <a href='SkColor_Reference#Color'>be</a> <a href='SkColor_Reference#Color'>generated</a> <a href='SkColor_Reference#Color'>by</a> <a href='SkColor_Reference#Color'>JPEG</a> <a href='SkColor_Reference#Color'>images</a>, <a href='SkColor_Reference#Color'>standard</a>
<a href='SkColor_Reference#Color'>video</a> <a href='SkColor_Reference#Color'>streams</a>, <a href='SkColor_Reference#Color'>or</a> <a href='SkColor_Reference#Color'>high</a> <a href='SkColor_Reference#Color'>definition</a> <a href='SkColor_Reference#Color'>video</a> <a href='SkColor_Reference#Color'>streams</a>. <a href='SkColor_Reference#Color'>Each</a> <a href='SkColor_Reference#Color'>has</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>own</a> <a href='SkColor_Reference#Color'>mapping</a> <a href='SkColor_Reference#Color'>from</a>
<a href='SkColor_Reference#Color'>YUV</a> <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>RGB</a>.

<a href='SkColor_Reference#Color'>JPEG</a> <a href='SkColor_Reference#Color'>YUV</a> <a href='SkColor_Reference#Color'>values</a> <a href='SkColor_Reference#Color'>encode</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>full</a> <a href='SkColor_Reference#Color'>range</a> <a href='SkColor_Reference#Color'>of</a> 0 <a href='SkColor_Reference#Color'>to</a> 255 <a href='SkColor_Reference#Color'>for</a> <a href='SkColor_Reference#Color'>all</a> <a href='SkColor_Reference#Color'>three</a> <a href='SkColor_Reference#Color'>components</a>.
<a href='SkColor_Reference#Color'>Video</a> <a href='SkColor_Reference#Color'>YUV</a> <a href='SkColor_Reference#Color'>values</a> <a href='SkColor_Reference#Color'>range</a> <a href='SkColor_Reference#Color'>from</a> 16 <a href='SkColor_Reference#Color'>to</a> 235 <a href='SkColor_Reference#Color'>for</a> <a href='SkColor_Reference#Color'>all</a> <a href='SkColor_Reference#Color'>three</a> <a href='SkColor_Reference#Color'>components</a>. <a href='SkColor_Reference#Color'>Details</a> <a href='SkColor_Reference#Color'>of</a>
<a href='SkColor_Reference#Color'>encoding</a> <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>conversion</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>RGB</a> <a href='SkColor_Reference#Color'>are</a> <a href='SkColor_Reference#Color'>described</a> <a href='SkColor_Reference#Color'>in</a>
<a href='https://en.wikipedia.org/wiki/YCbCr'>YCbCr color space</a></a> .

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kJPEG_SkYUVColorSpace'><code>kJPEG_SkYUVColorSpace</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Describes standard JPEG  <a href='undocumented#Color_Space'>color space</a>;
<a href='https://en.wikipedia.org/wiki/Rec._601'>CCIR 601</a></a> with full range of 0 to 255 for components.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRec601_SkYUVColorSpace'><code>kRec601_SkYUVColorSpace</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Describes standard used by standard definition television;
<a href='https://en.wikipedia.org/wiki/Rec._601'>CCIR 601</a></a> with studio range of 16 to 235 range for components.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRec709_SkYUVColorSpace'><code>kRec709_SkYUVColorSpace</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Describes standard used by high definition television;
<a href='https://en.wikipedia.org/wiki/Rec._709'>Rec. 709</a></a> with studio range of 16 to 235 range for components.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kLastEnum_SkYUVColorSpace'><code>kLastEnum_SkYUVColorSpace</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Used by tests to iterate through all valid values.
</td>
  </tr>
</table>

### See Also

<a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a>

<a name='SkImageInfo'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
struct <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> {
<a href='SkImageInfo_Reference#SkImageInfo'>public</a>:
    <a href='#SkImageInfo_empty_constructor'>SkImageInfo()</a>;
    <a href='SkImageInfo_Reference#SkImageInfo'>static</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_Make'>Make</a>(<a href='#SkImageInfo_Make'>int</a> <a href='#SkImageInfo_Make'>width</a>, <a href='#SkImageInfo_Make'>int</a> <a href='#SkImageInfo_Make'>height</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>ct</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>at</a>,
                     <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>cs</a> = <a href='undocumented#SkColorSpace'>nullptr</a>);
    <a href='undocumented#SkColorSpace'>static</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeN32'>MakeN32</a>(<a href='#SkImageInfo_MakeN32'>int</a> <a href='#SkImageInfo_MakeN32'>width</a>, <a href='#SkImageInfo_MakeN32'>int</a> <a href='#SkImageInfo_MakeN32'>height</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>at</a>,
                        <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>cs</a> = <a href='undocumented#SkColorSpace'>nullptr</a>);
    <a href='undocumented#SkColorSpace'>static</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a>(<a href='#SkImageInfo_MakeS32'>int</a> <a href='#SkImageInfo_MakeS32'>width</a>, <a href='#SkImageInfo_MakeS32'>int</a> <a href='#SkImageInfo_MakeS32'>height</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>at</a>);
    <a href='SkImageInfo_Reference#SkAlphaType'>static</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a>(<a href='#SkImageInfo_MakeN32Premul'>int</a> <a href='#SkImageInfo_MakeN32Premul'>width</a>, <a href='#SkImageInfo_MakeN32Premul'>int</a> <a href='#SkImageInfo_MakeN32Premul'>height</a>, <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>cs</a> = <a href='undocumented#SkColorSpace'>nullptr</a>);
    <a href='undocumented#SkColorSpace'>static</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a>(<a href='#SkImageInfo_MakeN32Premul'>const</a> <a href='undocumented#SkISize'>SkISize</a>& <a href='undocumented#Size'>size</a>);
    <a href='undocumented#Size'>static</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a>(<a href='#SkImageInfo_MakeA8'>int</a> <a href='#SkImageInfo_MakeA8'>width</a>, <a href='#SkImageInfo_MakeA8'>int</a> <a href='#SkImageInfo_MakeA8'>height</a>);
    <a href='#SkImageInfo_MakeA8'>static</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeUnknown'>MakeUnknown</a>(<a href='#SkImageInfo_MakeUnknown'>int</a> <a href='#SkImageInfo_MakeUnknown'>width</a>, <a href='#SkImageInfo_MakeUnknown'>int</a> <a href='#SkImageInfo_MakeUnknown'>height</a>);
    <a href='#SkImageInfo_MakeUnknown'>static</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeUnknown'>MakeUnknown</a>();
    <a href='#SkImageInfo_MakeUnknown'>int</a> <a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>const</a>;
    <a href='#SkImageInfo_width'>int</a> <a href='#SkImageInfo_height'>height()</a> <a href='#SkImageInfo_height'>const</a>;
    <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>const</a>;
    <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>const</a>;
    <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkImageInfo_colorSpace'>colorSpace</a>() <a href='#SkImageInfo_colorSpace'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImageInfo_refColorSpace'>refColorSpace</a>() <a href='#SkImageInfo_refColorSpace'>const</a>;
    <a href='#SkImageInfo_refColorSpace'>bool</a> <a href='#SkImageInfo_isEmpty'>isEmpty</a>() <a href='#SkImageInfo_isEmpty'>const</a>;
    <a href='#SkImageInfo_isEmpty'>bool</a> <a href='#SkImageInfo_isOpaque'>isOpaque</a>() <a href='#SkImageInfo_isOpaque'>const</a>;
    <a href='undocumented#SkISize'>SkISize</a> <a href='#SkImageInfo_dimensions'>dimensions()</a> <a href='#SkImageInfo_dimensions'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkImageInfo_bounds'>bounds()</a> <a href='#SkImageInfo_bounds'>const</a>;
    <a href='#SkImageInfo_bounds'>bool</a> <a href='#SkImageInfo_gammaCloseToSRGB'>gammaCloseToSRGB</a>() <a href='#SkImageInfo_gammaCloseToSRGB'>const</a>;
    <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeWH'>makeWH</a>(<a href='#SkImageInfo_makeWH'>int</a> <a href='#SkImageInfo_makeWH'>newWidth</a>, <a href='#SkImageInfo_makeWH'>int</a> <a href='#SkImageInfo_makeWH'>newHeight</a>) <a href='#SkImageInfo_makeWH'>const</a>;
    <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeAlphaType'>makeAlphaType</a>(<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>newAlphaType</a>) <a href='SkImageInfo_Reference#SkAlphaType'>const</a>;
    <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeColorType'>makeColorType</a>(<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>newColorType</a>) <a href='SkImageInfo_Reference#SkColorType'>const</a>;
    <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeColorSpace'>makeColorSpace</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>cs</a>) <a href='undocumented#SkColorSpace'>const</a>;
    <a href='undocumented#SkColorSpace'>int</a> <a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>() <a href='#SkImageInfo_bytesPerPixel'>const</a>;
    <a href='#SkImageInfo_bytesPerPixel'>int</a> <a href='#SkImageInfo_shiftPerPixel'>shiftPerPixel</a>() <a href='#SkImageInfo_shiftPerPixel'>const</a>;
    <a href='#SkImageInfo_shiftPerPixel'>uint64_t</a> <a href='#SkImageInfo_minRowBytes64'>minRowBytes64</a>() <a href='#SkImageInfo_minRowBytes64'>const</a>;
    <a href='#SkImageInfo_minRowBytes64'>size_t</a> <a href='#SkImageInfo_minRowBytes'>minRowBytes</a>() <a href='#SkImageInfo_minRowBytes'>const</a>;
    <a href='#SkImageInfo_minRowBytes'>size_t</a> <a href='#SkImageInfo_computeOffset'>computeOffset</a>(<a href='#SkImageInfo_computeOffset'>int</a> <a href='#SkImageInfo_computeOffset'>x</a>, <a href='#SkImageInfo_computeOffset'>int</a> <a href='#SkImageInfo_computeOffset'>y</a>, <a href='#SkImageInfo_computeOffset'>size_t</a> <a href='#SkImageInfo_computeOffset'>rowBytes</a>) <a href='#SkImageInfo_computeOffset'>const</a>;
    <a href='#SkImageInfo_computeOffset'>bool</a> <a href='#SkImageInfo_computeOffset'>operator</a>==(<a href='#SkImageInfo_computeOffset'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>other</a>) <a href='SkImageInfo_Reference#SkImageInfo'>const</a>;
    <a href='SkImageInfo_Reference#SkImageInfo'>bool</a> <a href='SkImageInfo_Reference#SkImageInfo'>operator</a>!=(<a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>other</a>) <a href='SkImageInfo_Reference#SkImageInfo'>const</a>;
    <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkImageInfo_computeByteSize'>computeByteSize</a>(<a href='#SkImageInfo_computeByteSize'>size_t</a> <a href='#SkImageInfo_computeByteSize'>rowBytes</a>) <a href='#SkImageInfo_computeByteSize'>const</a>;
    <a href='#SkImageInfo_computeByteSize'>size_t</a> <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a>() <a href='#SkImageInfo_computeMinByteSize'>const</a>;
    <a href='#SkImageInfo_computeMinByteSize'>static</a> <a href='#SkImageInfo_computeMinByteSize'>bool</a> <a href='#SkImageInfo_ByteSizeOverflowed'>ByteSizeOverflowed</a>(<a href='#SkImageInfo_ByteSizeOverflowed'>size_t</a> <a href='#SkImageInfo_ByteSizeOverflowed'>byteSize</a>);
    <a href='#SkImageInfo_ByteSizeOverflowed'>bool</a> <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>(<a href='#SkImageInfo_validRowBytes'>size_t</a> <a href='#SkImageInfo_validRowBytes'>rowBytes</a>) <a href='#SkImageInfo_validRowBytes'>const</a>;
    <a href='#SkImageInfo_validRowBytes'>void</a> <a href='#SkImageInfo_reset'>reset()</a>;
    <a href='#SkImageInfo_reset'>void</a> <a href='#SkImageInfo_validate'>validate()</a> <a href='#SkImageInfo_validate'>const</a>;
};
</pre>

Describes <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>dimensions</a> <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>encoding</a>. <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>, <a href='SkImage_Reference#Image'>Image</a>, <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>, <a href='SkPixmap_Reference#Pixmap'>and</a> <a href='SkSurface_Reference#Surface'>Surface</a>
<a href='SkSurface_Reference#Surface'>can</a> <a href='SkSurface_Reference#Surface'>be</a> <a href='SkSurface_Reference#Surface'>created</a> <a href='SkSurface_Reference#Surface'>from</a> <a href='#Image_Info'>Image_Info</a>. <a href='#Image_Info'>Image_Info</a> <a href='#Image_Info'>can</a> <a href='#Image_Info'>be</a> <a href='#Image_Info'>retrieved</a> <a href='#Image_Info'>from</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>and</a>
<a href='SkPixmap_Reference#Pixmap'>Pixmap</a>, <a href='SkPixmap_Reference#Pixmap'>but</a> <a href='SkPixmap_Reference#Pixmap'>not</a> <a href='SkPixmap_Reference#Pixmap'>from</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>and</a> <a href='SkSurface_Reference#Surface'>Surface</a>. <a href='SkSurface_Reference#Surface'>For</a> <a href='SkSurface_Reference#Surface'>example</a>, <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>and</a> <a href='SkSurface_Reference#Surface'>Surface</a>
<a href='SkSurface_Reference#Surface'>implementations</a> <a href='SkSurface_Reference#Surface'>may</a> <a href='SkSurface_Reference#Surface'>defer</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>depth</a>, <a href='undocumented#Pixel'>so</a> <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>completely</a> <a href='undocumented#Pixel'>specify</a> <a href='#Image_Info'>Image_Info</a>.

<a href='#Image_Info'>Image_Info</a> <a href='#Image_Info'>contains</a> <a href='#Image_Info'>dimensions</a>, <a href='#Image_Info'>the</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>integral</a> <a href='undocumented#Pixel'>width</a> <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>height</a>. <a href='undocumented#Pixel'>It</a> <a href='undocumented#Pixel'>encodes</a>
<a href='undocumented#Pixel'>how</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>bits</a> <a href='undocumented#Pixel'>describe</a> <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Color_Alpha'>transparency</a>; <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>components</a> <a href='SkColor_Reference#Color'>red</a>, <a href='SkColor_Reference#Color'>blue</a>,
<a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>green</a>; <a href='SkColor_Reference#Color'>and</a> <a href='#Color_Space'>Color_Space</a>, <a href='#Color_Space'>the</a> <a href='#Color_Space'>range</a> <a href='#Color_Space'>and</a> <a href='#Color_Space'>linearity</a> <a href='#Color_Space'>of</a> <a href='#Color_Space'>colors</a>.

<a name='SkImageInfo_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkImageInfo_empty_constructor'>SkImageInfo()</a>
</pre>

Creates an empty <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>with</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>,
a width and height of zero, and no <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

### Return Value

empty <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="f206f698e7a8db3d84334c26b1a702dc"><div>An empty <a href='#Image_Info'>Image_Info</a> <a href='#Image_Info'>may</a> <a href='#Image_Info'>be</a> <a href='#Image_Info'>passed</a> <a href='#Image_Info'>to</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_accessTopLayerPixels'>accessTopLayerPixels</a> <a href='#SkCanvas_accessTopLayerPixels'>as</a> <a href='#SkCanvas_accessTopLayerPixels'>storage</a>
<a href='#SkCanvas_accessTopLayerPixels'>for</a> <a href='#SkCanvas_accessTopLayerPixels'>the</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>actual</a> <a href='#Image_Info'>Image_Info</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a>

<a name='SkImageInfo_Make'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_Make'>Make</a>(<a href='#SkImageInfo_Make'>int</a> <a href='#SkImageInfo_Make'>width</a>, <a href='#SkImageInfo_Make'>int</a> <a href='#SkImageInfo_Make'>height</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>ct</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>at</a>,
                        <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>cs</a> = <a href='undocumented#SkColorSpace'>nullptr</a>)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>from</a> <a href='SkImageInfo_Reference#SkImageInfo'>integral</a> <a href='SkImageInfo_Reference#SkImageInfo'>dimensions</a> <a href='#SkImageInfo_Make_width'>width</a> <a href='#SkImageInfo_Make_width'>and</a> <a href='#SkImageInfo_Make_height'>height</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImageInfo_Make_ct'>ct</a>,
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImageInfo_Make_at'>at</a>, <a href='#SkImageInfo_Make_at'>and</a> <a href='#SkImageInfo_Make_at'>optionally</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='#SkImageInfo_Make_cs'>cs</a>.

If <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='#SkImageInfo_Make_cs'>cs</a> <a href='#SkImageInfo_Make_cs'>is</a> <a href='#SkImageInfo_Make_cs'>nullptr</a> <a href='#SkImageInfo_Make_cs'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>is</a> <a href='SkImageInfo_Reference#SkImageInfo'>part</a> <a href='SkImageInfo_Reference#SkImageInfo'>of</a> <a href='SkImageInfo_Reference#SkImageInfo'>drawing</a> <a href='SkImageInfo_Reference#SkImageInfo'>source</a>: <a href='undocumented#SkColorSpace'>SkColorSpace</a>
defaults to sRGB, mapping into <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_Make_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>column</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_Make_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_Make_ct'><code><strong>ct</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_Make_at'><code><strong>at</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_Make_cs'><code><strong>cs</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="9f47f9c2a99473f5b1113db48096d586"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a>

<a name='SkImageInfo_MakeN32'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeN32'>MakeN32</a>(<a href='#SkImageInfo_MakeN32'>int</a> <a href='#SkImageInfo_MakeN32'>width</a>, <a href='#SkImageInfo_MakeN32'>int</a> <a href='#SkImageInfo_MakeN32'>height</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>at</a>, <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>cs</a> = <a href='undocumented#SkColorSpace'>nullptr</a>)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>from</a> <a href='SkImageInfo_Reference#SkImageInfo'>integral</a> <a href='SkImageInfo_Reference#SkImageInfo'>dimensions</a> <a href='#SkImageInfo_MakeN32_width'>width</a> <a href='#SkImageInfo_MakeN32_width'>and</a> <a href='#SkImageInfo_MakeN32_height'>height</a>, <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>,
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImageInfo_MakeN32_at'>at</a>, <a href='#SkImageInfo_MakeN32_at'>and</a> <a href='#SkImageInfo_MakeN32_at'>optionally</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='#SkImageInfo_MakeN32_cs'>cs</a>. <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>will</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>equal</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>either</a>
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>or</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>whichever</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>is</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>optimal</a>.

If <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='#SkImageInfo_MakeN32_cs'>cs</a> <a href='#SkImageInfo_MakeN32_cs'>is</a> <a href='#SkImageInfo_MakeN32_cs'>nullptr</a> <a href='#SkImageInfo_MakeN32_cs'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>is</a> <a href='SkImageInfo_Reference#SkImageInfo'>part</a> <a href='SkImageInfo_Reference#SkImageInfo'>of</a> <a href='SkImageInfo_Reference#SkImageInfo'>drawing</a> <a href='SkImageInfo_Reference#SkImageInfo'>source</a>: <a href='undocumented#SkColorSpace'>SkColorSpace</a>
defaults to sRGB, mapping into <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeN32_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>column</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeN32_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeN32_at'><code><strong>at</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeN32_cs'><code><strong>cs</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="78cea0c4cac205b61ad6f6c982cbd888"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a>

<a name='SkImageInfo_MakeS32'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a>(<a href='#SkImageInfo_MakeS32'>int</a> <a href='#SkImageInfo_MakeS32'>width</a>, <a href='#SkImageInfo_MakeS32'>int</a> <a href='#SkImageInfo_MakeS32'>height</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>at</a>)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>from</a> <a href='SkImageInfo_Reference#SkImageInfo'>integral</a> <a href='SkImageInfo_Reference#SkImageInfo'>dimensions</a> <a href='#SkImageInfo_MakeS32_width'>width</a> <a href='#SkImageInfo_MakeS32_width'>and</a> <a href='#SkImageInfo_MakeS32_height'>height</a>, <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>,
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImageInfo_MakeS32_at'>at</a>, <a href='#SkImageInfo_MakeS32_at'>with</a> <a href='#SkImageInfo_MakeS32_at'>sRGB</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeS32_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>column</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeS32_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeS32_at'><code><strong>at</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="de418ccb42471d1589508ef3955f8c53"><div>Top gradient is drawn to offScreen without <a href='#Color_Space'>Color_Space</a>. <a href='#Color_Space'>It</a> <a href='#Color_Space'>is</a> <a href='#Color_Space'>darker</a> <a href='#Color_Space'>than</a> <a href='#Color_Space'>middle</a>
<a href='#Color_Space'>gradient</a>, <a href='#Color_Space'>drawn</a> <a href='#Color_Space'>to</a> <a href='#Color_Space'>offScreen</a> <a href='#Color_Space'>with</a> <a href='#Color_Space'>sRGB</a> <a href='#Color_Space'>Color_Space</a>. <a href='#Color_Space'>Bottom</a> <a href='#Color_Space'>gradient</a> <a href='#Color_Space'>shares</a> <a href='#Color_Space'>bits</a>
<a href='#Color_Space'>with</a> <a href='#Color_Space'>middle</a>, <a href='#Color_Space'>but</a> <a href='#Color_Space'>does</a> <a href='#Color_Space'>not</a> <a href='#Color_Space'>specify</a> <a href='#Color_Space'>the</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>in</a> <a href='#Color_Space'>noColorSpaceBitmap</a>. <a href='#Color_Space'>A</a> <a href='#Color_Space'>source</a>
<a href='#Color_Space'>without</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>is</a> <a href='#Color_Space'>treated</a> <a href='#Color_Space'>as</a> <a href='#Color_Space'>sRGB</a>; <a href='#Color_Space'>the</a> <a href='#Color_Space'>bottom</a> <a href='#Color_Space'>gradient</a> <a href='#Color_Space'>is</a> <a href='#Color_Space'>identical</a> <a href='#Color_Space'>to</a> <a href='#Color_Space'>the</a>
<a href='#Color_Space'>middle</a> <a href='#Color_Space'>gradient</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a>

<a name='SkImageInfo_MakeN32Premul'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a>(<a href='#SkImageInfo_MakeN32Premul'>int</a> <a href='#SkImageInfo_MakeN32Premul'>width</a>, <a href='#SkImageInfo_MakeN32Premul'>int</a> <a href='#SkImageInfo_MakeN32Premul'>height</a>, <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>cs</a> = <a href='undocumented#SkColorSpace'>nullptr</a>)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>from</a> <a href='SkImageInfo_Reference#SkImageInfo'>integral</a> <a href='SkImageInfo_Reference#SkImageInfo'>dimensions</a> <a href='#SkImageInfo_MakeN32Premul_width'>width</a> <a href='#SkImageInfo_MakeN32Premul_width'>and</a> <a href='#SkImageInfo_MakeN32Premul_height'>height</a>, <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>,
<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>with</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>optional</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

If <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='#SkImageInfo_MakeN32Premul_cs'>cs</a> <a href='#SkImageInfo_MakeN32Premul_cs'>is</a> <a href='#SkImageInfo_MakeN32Premul_cs'>nullptr</a> <a href='#SkImageInfo_MakeN32Premul_cs'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>is</a> <a href='SkImageInfo_Reference#SkImageInfo'>part</a> <a href='SkImageInfo_Reference#SkImageInfo'>of</a> <a href='SkImageInfo_Reference#SkImageInfo'>drawing</a> <a href='SkImageInfo_Reference#SkImageInfo'>source</a>: <a href='undocumented#SkColorSpace'>SkColorSpace</a>
defaults to sRGB, mapping into <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeN32Premul_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>column</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeN32Premul_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeN32Premul_cs'><code><strong>cs</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="525650a67e19fdd8ca9f72b7eda65174"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a> <a href='#SkImageInfo_Make'>Make</a>

<a name='SkImageInfo_MakeN32Premul_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a>(<a href='#SkImageInfo_MakeN32Premul'>const</a> <a href='undocumented#SkISize'>SkISize</a>& <a href='undocumented#Size'>size</a>)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>from</a> <a href='SkImageInfo_Reference#SkImageInfo'>integral</a> <a href='SkImageInfo_Reference#SkImageInfo'>dimensions</a> <a href='SkImageInfo_Reference#SkImageInfo'>width</a> <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>height</a>, <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>,
<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>with</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>set</a> <a href='undocumented#SkColorSpace'>to</a> <a href='undocumented#SkColorSpace'>nullptr</a>.

If <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>is</a> <a href='SkImageInfo_Reference#SkImageInfo'>part</a> <a href='SkImageInfo_Reference#SkImageInfo'>of</a> <a href='SkImageInfo_Reference#SkImageInfo'>drawing</a> <a href='SkImageInfo_Reference#SkImageInfo'>source</a>: <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>defaults</a> <a href='undocumented#SkColorSpace'>to</a> <a href='undocumented#SkColorSpace'>sRGB</a>, <a href='undocumented#SkColorSpace'>mapping</a>
into <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeN32Premul_2_size'><code><strong>size</strong></code></a></td>
    <td>width and height, each must be zero or greater</td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="b9026d7f39029756bd7cab9542c64f4e"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a> <a href='#SkImageInfo_Make'>Make</a>

<a name='SkImageInfo_MakeA8'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a>(<a href='#SkImageInfo_MakeA8'>int</a> <a href='#SkImageInfo_MakeA8'>width</a>, <a href='#SkImageInfo_MakeA8'>int</a> <a href='#SkImageInfo_MakeA8'>height</a>)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>from</a> <a href='SkImageInfo_Reference#SkImageInfo'>integral</a> <a href='SkImageInfo_Reference#SkImageInfo'>dimensions</a> <a href='#SkImageInfo_MakeA8_width'>width</a> <a href='#SkImageInfo_MakeA8_width'>and</a> <a href='#SkImageInfo_MakeA8_height'>height</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>with</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>set</a> <a href='undocumented#SkColorSpace'>to</a> <a href='undocumented#SkColorSpace'>nullptr</a>.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeA8_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>column</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeA8_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="547388991687b8e10d482d8b1c82777d"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_Make'>Make</a>

<a name='SkImageInfo_MakeUnknown'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeUnknown'>MakeUnknown</a>(<a href='#SkImageInfo_MakeUnknown'>int</a> <a href='#SkImageInfo_MakeUnknown'>width</a>, <a href='#SkImageInfo_MakeUnknown'>int</a> <a href='#SkImageInfo_MakeUnknown'>height</a>)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>from</a> <a href='SkImageInfo_Reference#SkImageInfo'>integral</a> <a href='SkImageInfo_Reference#SkImageInfo'>dimensions</a> <a href='#SkImageInfo_MakeUnknown_width'>width</a> <a href='#SkImageInfo_MakeUnknown_width'>and</a> <a href='#SkImageInfo_MakeUnknown_height'>height</a>, <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>,
<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>with</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>set</a> <a href='undocumented#SkColorSpace'>to</a> <a href='undocumented#SkColorSpace'>nullptr</a>.

Returned <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>as</a> <a href='SkImageInfo_Reference#SkImageInfo'>part</a> <a href='SkImageInfo_Reference#SkImageInfo'>of</a> <a href='SkImageInfo_Reference#SkImageInfo'>source</a> <a href='SkImageInfo_Reference#SkImageInfo'>does</a> <a href='SkImageInfo_Reference#SkImageInfo'>not</a> <a href='SkImageInfo_Reference#SkImageInfo'>draw</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>as</a> <a href='SkImageInfo_Reference#SkImageInfo'>part</a> <a href='SkImageInfo_Reference#SkImageInfo'>of</a> <a href='SkImageInfo_Reference#SkImageInfo'>destination</a>
can not be drawn to.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeUnknown_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>column</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeUnknown_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="75f13a78b28b08c72baf32b7d868de1c"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_empty_constructor'>SkImageInfo()</a> <a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_Make'>Make</a>

<a name='SkImageInfo_MakeUnknown_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeUnknown'>MakeUnknown</a>()
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>from</a> <a href='SkImageInfo_Reference#SkImageInfo'>integral</a> <a href='SkImageInfo_Reference#SkImageInfo'>dimensions</a> <a href='SkImageInfo_Reference#SkImageInfo'>width</a> <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>height</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='SkImageInfo_Reference#SkImageInfo'>zero</a>,
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>with</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>set</a> <a href='undocumented#SkColorSpace'>to</a> <a href='undocumented#SkColorSpace'>nullptr</a>.

Returned <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>as</a> <a href='SkImageInfo_Reference#SkImageInfo'>part</a> <a href='SkImageInfo_Reference#SkImageInfo'>of</a> <a href='SkImageInfo_Reference#SkImageInfo'>source</a> <a href='SkImageInfo_Reference#SkImageInfo'>does</a> <a href='SkImageInfo_Reference#SkImageInfo'>not</a> <a href='SkImageInfo_Reference#SkImageInfo'>draw</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>as</a> <a href='SkImageInfo_Reference#SkImageInfo'>part</a> <a href='SkImageInfo_Reference#SkImageInfo'>of</a> <a href='SkImageInfo_Reference#SkImageInfo'>destination</a>
can not be drawn to.

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="a1af7696ae0cdd6f379546dd1f211b7a"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_empty_constructor'>SkImageInfo()</a> <a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_Make'>Make</a>

<a name='Property'></a>

<a name='SkImageInfo_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>count</a> <a href='undocumented#Pixel'>in</a> <a href='undocumented#Pixel'>each</a> <a href='undocumented#Pixel'>row</a>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>width</a>

### Example

<div><fiddle-embed name="e2491817695290d0218be77f091b8460"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_height'>height</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_width'>width</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>::<a href='#SkPixelRef_width'>width</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_width'>width</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_width'>width</a>

<a name='SkImageInfo_height'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImageInfo_height'>height()</a> <a href='#SkImageInfo_height'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>height</a>

### Example

<div><fiddle-embed name="72c35baaeddca1d912edf93d19429c8e"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_width'>width</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_height'>height</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>::<a href='#SkPixelRef_height'>height</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_height'>height</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_height'>height</a>

<a name='SkImageInfo_colorType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>const</a>
</pre>

Returns <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Color_Type'>one</a> <a href='#Image_Info_Color_Type'>of</a>: <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>
.

### Return Value

<a href='#Image_Info_Color_Type'>Color_Type</a>

### Example

<div><fiddle-embed name="06ecc3ce7f35cc7f930cbc2a662e3105">

#### Example Output

~~~~
color type: kAlpha_8_SkColorType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_alphaType'>alphaType</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_colorType'>colorType</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_colorType'>colorType</a>

<a name='SkImageInfo_alphaType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>const</a>
</pre>

Returns <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Image_Info_Alpha_Type'>one</a> <a href='#Image_Info_Alpha_Type'>of</a>: <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>
.

### Return Value

<a href='#Image_Info_Alpha_Type'>Alpha_Type</a>

### Example

<div><fiddle-embed name="5c1d2499a4056b6cff38c1cf924158a1">

#### Example Output

~~~~
alpha type: kPremul_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_colorType'>colorType</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_alphaType'>alphaType</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_alphaType'>alphaType</a>

<a name='SkImageInfo_colorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkImageInfo_colorSpace'>colorSpace</a>() <a href='#SkImageInfo_colorSpace'>const</a>
</pre>

Returns <a href='undocumented#SkColorSpace'>SkColorSpace</a>, <a href='undocumented#SkColorSpace'>the</a> <a href='undocumented#SkColorSpace'>range</a> <a href='undocumented#SkColorSpace'>of</a> <a href='undocumented#SkColorSpace'>colors</a>. <a href='undocumented#SkColorSpace'>The</a> <a href='undocumented#SkColorSpace'>reference</a> <a href='undocumented#SkColorSpace'>count</a> <a href='undocumented#SkColorSpace'>of</a>
<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>is</a> <a href='undocumented#SkColorSpace'>unchanged</a>. <a href='undocumented#SkColorSpace'>The</a> <a href='undocumented#SkColorSpace'>returned</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>is</a> <a href='undocumented#SkColorSpace'>immutable</a>.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a>, <a href='undocumented#SkColorSpace'>or</a> <a href='undocumented#SkColorSpace'>nullptr</a>

### Example

<div><fiddle-embed name="5602b816d7cf75e3851274ef36a4c10f"><div><a href='undocumented#SkColorSpace'>SkColorSpace</a>::<a href='#SkColorSpace_MakeSRGBLinear'>MakeSRGBLinear</a> <a href='#SkColorSpace_MakeSRGBLinear'>creates</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>with</a> <a href='#Color_Space'>linear</a> <a href='#Color_Space'>gamma</a>
<a href='#Color_Space'>and</a> <a href='#Color_Space'>an</a> <a href='#Color_Space'>sRGB</a> <a href='#Color_Space'>gamut</a>. <a href='#Color_Space'>This</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>gamma</a> <a href='#Color_Space'>is</a> <a href='#Color_Space'>not</a> <a href='#Color_Space'>close</a> <a href='#Color_Space'>to</a> <a href='#Color_Space'>sRGB</a> <a href='#Color_Space'>gamma</a>.
</div>

#### Example Output

~~~~
gammaCloseToSRGB: false  gammaIsLinear: true  isSRGB: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#Color_Space'>Color_Space</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_colorSpace'>colorSpace</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_colorSpace'>colorSpace</a>

<a name='SkImageInfo_refColorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='#SkImageInfo_refColorSpace'>refColorSpace</a>() <a href='#SkImageInfo_refColorSpace'>const</a>
</pre>

Returns smart pointer to <a href='undocumented#SkColorSpace'>SkColorSpace</a>, <a href='undocumented#SkColorSpace'>the</a> <a href='undocumented#SkColorSpace'>range</a> <a href='undocumented#SkColorSpace'>of</a> <a href='undocumented#SkColorSpace'>colors</a>. <a href='undocumented#SkColorSpace'>The</a> <a href='undocumented#SkColorSpace'>smart</a> <a href='undocumented#SkColorSpace'>pointer</a>
tracks the number of objects sharing this <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>reference</a> <a href='undocumented#SkColorSpace'>so</a> <a href='undocumented#SkColorSpace'>the</a> <a href='undocumented#SkColorSpace'>memory</a>
is released when the owners destruct.

The returned <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>is</a> <a href='undocumented#SkColorSpace'>immutable</a>.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>wrapped</a> <a href='undocumented#SkColorSpace'>in</a> <a href='undocumented#SkColorSpace'>a</a> <a href='undocumented#SkColorSpace'>smart</a> <a href='undocumented#SkColorSpace'>pointer</a>

### Example

<div><fiddle-embed name="33f65524736736fd91802b4198ba6fa8"></fiddle-embed></div>

### See Also

<a href='#Color_Space'>Color_Space</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_refColorSpace'>refColorSpace</a>

<a name='SkImageInfo_isEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImageInfo_isEmpty'>isEmpty</a>() <a href='#SkImageInfo_isEmpty'>const</a>
</pre>

Returns if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>describes</a> <a href='SkImageInfo_Reference#SkImageInfo'>an</a> <a href='SkImageInfo_Reference#SkImageInfo'>empty</a> <a href='SkImageInfo_Reference#SkImageInfo'>area</a> <a href='SkImageInfo_Reference#SkImageInfo'>of</a> <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a> <a href='SkImageInfo_Reference#SkImageInfo'>by</a> <a href='SkImageInfo_Reference#SkImageInfo'>checking</a> <a href='SkImageInfo_Reference#SkImageInfo'>if</a> <a href='SkImageInfo_Reference#SkImageInfo'>either</a>
width or height is zero or smaller.

### Return Value

true if either dimension is zero or smaller

### Example

<div><fiddle-embed name="b8757200da5be0b43763cf79feb681a7">

#### Example Output

~~~~
width: 0 height: 0 empty: true
width: 0 height: 2 empty: true
width: 2 height: 0 empty: true
width: 2 height: 2 empty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_dimensions'>dimensions</a> <a href='#SkImageInfo_bounds'>bounds</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_empty'>empty</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_bounds'>bounds</a>

<a name='SkImageInfo_isOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImageInfo_isOpaque'>isOpaque</a>() <a href='#SkImageInfo_isOpaque'>const</a>
</pre>

Returns true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#SkAlphaType'>set</a> <a href='SkImageInfo_Reference#SkAlphaType'>to</a> <a href='SkImageInfo_Reference#SkAlphaType'>hint</a> <a href='SkImageInfo_Reference#SkAlphaType'>that</a> <a href='SkImageInfo_Reference#SkAlphaType'>all</a> <a href='SkImageInfo_Reference#SkAlphaType'>pixels</a> <a href='SkImageInfo_Reference#SkAlphaType'>are</a> <a href='SkImageInfo_Reference#SkAlphaType'>opaque</a>; <a href='SkImageInfo_Reference#SkAlphaType'>their</a>
<a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>value</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>implicitly</a> <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>explicitly</a> 1.0. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>true</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>all</a> <a href='SkColor_Reference#Alpha'>pixels</a> <a href='SkColor_Reference#Alpha'>are</a>
not opaque, Skia may draw incorrectly.

Does not check if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>allows</a> <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>if</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>value</a> <a href='undocumented#Pixel'>has</a>
transparency.

### Return Value

true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>

### Example

<div><fiddle-embed name="e9bd4f02b6cfb3ac864cb7fee7d7299c">

#### Example Output

~~~~
isOpaque: false
isOpaque: false
isOpaque: true
isOpaque: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#Color_Alpha'>Color_Alpha</a> <a href='SkImageInfo_Reference#SkColorTypeValidateAlphaType'>SkColorTypeValidateAlphaType</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_isOpaque'>isOpaque</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_isOpaque'>isOpaque</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_isOpaque'>isOpaque</a>

<a name='SkImageInfo_dimensions'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkISize'>SkISize</a> <a href='#SkImageInfo_dimensions'>dimensions()</a> <a href='#SkImageInfo_dimensions'>const</a>
</pre>

Returns <a href='undocumented#SkISize'>SkISize</a> { <a href='#SkImageInfo_width'>width()</a>, <a href='#SkImageInfo_height'>height()</a> }.

### Return Value

integral <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>and</a> <a href='#SkImageInfo_height'>height()</a>

### Example

<div><fiddle-embed name="d5547cd2b302822aa85b7b0ae3f48458">

#### Example Output

~~~~
dimensionsAsBounds == bounds
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_width'>width</a> <a href='#SkImageInfo_height'>height</a> <a href='#SkImageInfo_bounds'>bounds</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_dimensions'>dimensions</a>

<a name='SkImageInfo_bounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkImageInfo_bounds'>bounds()</a> <a href='#SkImageInfo_bounds'>const</a>
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> { 0, 0, <a href='#SkImageInfo_width'>width()</a>, <a href='#SkImageInfo_height'>height()</a> }.

### Return Value

integral rectangle from origin to <a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>and</a> <a href='#SkImageInfo_height'>height()</a>

### Example

<div><fiddle-embed name="a818be8945cd0c18f99ffe53e90afa48"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_width'>width</a> <a href='#SkImageInfo_height'>height</a> <a href='#SkImageInfo_dimensions'>dimensions</a>

<a name='SkImageInfo_gammaCloseToSRGB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImageInfo_gammaCloseToSRGB'>gammaCloseToSRGB</a>() <a href='#SkImageInfo_gammaCloseToSRGB'>const</a>
</pre>

Returns true if associated <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>is</a> <a href='#Color_Space'>not</a> <a href='#Color_Space'>nullptr</a>, <a href='#Color_Space'>and</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>gamma</a>
<a href='#Color_Space'>is</a> <a href='#Color_Space'>approximately</a> <a href='#Color_Space'>the</a> <a href='#Color_Space'>same</a> <a href='#Color_Space'>as</a> <a href='#Color_Space'>sRGB</a>.
<a href='#Color_Space'>This</a> <a href='#Color_Space'>includes</a> <a href='#Color_Space'>the</a>
<a href='https://en.wikipedia.org/wiki/SRGB#The_sRGB_transfer_function_(%22gamma%22)'>sRGB transfer function</a></a> as well as a gamma <a href='undocumented#Curve'>curve</a> <a href='undocumented#Curve'>described</a> <a href='undocumented#Curve'>by</a> <a href='undocumented#Curve'>a</a> 2.2 <a href='undocumented#Curve'>exponent</a>.

### Return Value

true if <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>gamma</a> <a href='#Color_Space'>is</a> <a href='#Color_Space'>approximately</a> <a href='#Color_Space'>the</a> <a href='#Color_Space'>same</a> <a href='#Color_Space'>as</a> <a href='#Color_Space'>sRGB</a>

### Example

<div><fiddle-embed name="22df72732e898a11773fbfe07388a546"></fiddle-embed></div>

### See Also

<a href='undocumented#SkColorSpace'>SkColorSpace</a>::<a href='#SkColorSpace_gammaCloseToSRGB'>gammaCloseToSRGB</a>

<a name='SkImageInfo_makeWH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeWH'>makeWH</a>(<a href='#SkImageInfo_makeWH'>int</a> <a href='#SkImageInfo_makeWH'>newWidth</a>, <a href='#SkImageInfo_makeWH'>int</a> <a href='#SkImageInfo_makeWH'>newHeight</a>) <a href='#SkImageInfo_makeWH'>const</a>
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>with</a> <a href='SkImageInfo_Reference#SkImageInfo'>the</a> <a href='SkImageInfo_Reference#SkImageInfo'>same</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>, <a href='undocumented#SkColorSpace'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>,
with dimensions set to width and height.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_makeWH_newWidth'><code><strong>newWidth</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>column</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_makeWH_newHeight'><code><strong>newHeight</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="cd203a3f9c5fb68272f21f302dd54fbc"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_makeAlphaType'>makeAlphaType</a> <a href='#SkImageInfo_makeColorSpace'>makeColorSpace</a> <a href='#SkImageInfo_makeColorType'>makeColorType</a>

<a name='SkImageInfo_makeAlphaType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeAlphaType'>makeAlphaType</a>(<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>newAlphaType</a>) <a href='SkImageInfo_Reference#SkAlphaType'>const</a>
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>with</a> <a href='SkImageInfo_Reference#SkImageInfo'>same</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>, <a href='undocumented#SkColorSpace'>width</a>, <a href='undocumented#SkColorSpace'>and</a> <a href='undocumented#SkColorSpace'>height</a>,
with <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>set</a> <a href='SkImageInfo_Reference#SkAlphaType'>to</a> <a href='#SkImageInfo_makeAlphaType_newAlphaType'>newAlphaType</a>.

Created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>contains</a> <a href='#SkImageInfo_makeAlphaType_newAlphaType'>newAlphaType</a> <a href='#SkImageInfo_makeAlphaType_newAlphaType'>even</a> <a href='#SkImageInfo_makeAlphaType_newAlphaType'>if</a> <a href='#SkImageInfo_makeAlphaType_newAlphaType'>it</a> <a href='#SkImageInfo_makeAlphaType_newAlphaType'>is</a> <a href='#SkImageInfo_makeAlphaType_newAlphaType'>incompatible</a> <a href='#SkImageInfo_makeAlphaType_newAlphaType'>with</a>
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkColorType'>in</a> <a href='SkImageInfo_Reference#SkColorType'>which</a> <a href='SkImageInfo_Reference#SkColorType'>case</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>is</a> <a href='SkImageInfo_Reference#SkImageInfo'>ignored</a>.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_makeAlphaType_newAlphaType'><code><strong>newAlphaType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="e72db006f1bea26feceaef8727ff9818"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a> <a href='#SkImageInfo_makeColorType'>makeColorType</a> <a href='#SkImageInfo_makeColorSpace'>makeColorSpace</a>

<a name='SkImageInfo_makeColorType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeColorType'>makeColorType</a>(<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>newColorType</a>) <a href='SkImageInfo_Reference#SkColorType'>const</a>
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>with</a> <a href='SkImageInfo_Reference#SkImageInfo'>same</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>, <a href='undocumented#SkColorSpace'>width</a>, <a href='undocumented#SkColorSpace'>and</a> <a href='undocumented#SkColorSpace'>height</a>,
with <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>set</a> <a href='SkImageInfo_Reference#SkColorType'>to</a> <a href='#SkImageInfo_makeColorType_newColorType'>newColorType</a>.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_makeColorType_newColorType'><code><strong>newColorType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="3ac267b08b12dc83c95f91d8dd5d70ee"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_makeAlphaType'>makeAlphaType</a> <a href='#SkImageInfo_makeColorSpace'>makeColorSpace</a>

<a name='SkImageInfo_makeColorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeColorSpace'>makeColorSpace</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>cs</a>) <a href='undocumented#SkColorSpace'>const</a>
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>with</a> <a href='SkImageInfo_Reference#SkImageInfo'>same</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkColorType'>width</a>, <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkColorType'>height</a>,
with <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>set</a> <a href='undocumented#SkColorSpace'>to</a> <a href='#SkImageInfo_makeColorSpace_cs'>cs</a>.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_makeColorSpace_cs'><code><strong>cs</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="fe3c5a755d3dde29bba058a583f18901"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_makeAlphaType'>makeAlphaType</a> <a href='#SkImageInfo_makeColorType'>makeColorType</a>

<a name='SkImageInfo_bytesPerPixel'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>() <a href='#SkImageInfo_bytesPerPixel'>const</a>
</pre>

Returns number of bytes per <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>required</a> <a href='undocumented#Pixel'>by</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.
Returns zero if <a href='#SkImageInfo_colorType'>colorType</a>( <a href='#SkImageInfo_colorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

bytes in <a href='undocumented#Pixel'>pixel</a>

### Example

<div><fiddle-embed name="9b6de4a07b2316228e9340e5a3b82134"><a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

#### Example Output

~~~~
color: kUnknown_SkColorType      bytesPerPixel: 0
color: kAlpha_8_SkColorType      bytesPerPixel: 1
color: kRGB_565_SkColorType      bytesPerPixel: 2
color: kARGB_4444_SkColorType    bytesPerPixel: 2
color: kRGBA_8888_SkColorType    bytesPerPixel: 4
color: kRGB_888x_SkColorType     bytesPerPixel: 4
color: kBGRA_8888_SkColorType    bytesPerPixel: 4
color: kRGBA_1010102_SkColorType bytesPerPixel: 4
color: kRGB_101010x_SkColorType  bytesPerPixel: 4
color: kGray_8_SkColorType       bytesPerPixel: 1
color: kRGBA_F16_SkColorType     bytesPerPixel: 8
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_width'>width</a> <a href='#SkImageInfo_shiftPerPixel'>shiftPerPixel</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_bytesPerPixel'>bytesPerPixel</a>

<a name='SkImageInfo_shiftPerPixel'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImageInfo_shiftPerPixel'>shiftPerPixel</a>() <a href='#SkImageInfo_shiftPerPixel'>const</a>
</pre>

Returns bit shift converting row bytes to row pixels.
Returns zero for <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

one of: 0, 1, 2, 3; left shift to convert pixels to bytes

### Example

<div><fiddle-embed name="e47b911f94fc629f756a829e523a2a89"><a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

#### Example Output

~~~~
color: kUnknown_SkColorType       shiftPerPixel: 0
color: kAlpha_8_SkColorType       shiftPerPixel: 0
color: kRGB_565_SkColorType       shiftPerPixel: 1
color: kARGB_4444_SkColorType     shiftPerPixel: 1
color: kRGBA_8888_SkColorType     shiftPerPixel: 2
color: kRGB_888x_SkColorType      shiftPerPixel: 2
color: kBGRA_8888_SkColorType     shiftPerPixel: 2
color: kRGBA_1010102_SkColorType  shiftPerPixel: 2
color: kRGB_101010x_SkColorType   shiftPerPixel: 2
color: kGray_8_SkColorType        shiftPerPixel: 0
color: kRGBA_F16_SkColorType      shiftPerPixel: 3
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a> <a href='#SkImageInfo_minRowBytes'>minRowBytes</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_shiftPerPixel'>shiftPerPixel</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_shiftPerPixel'>shiftPerPixel</a>

<a name='SkImageInfo_minRowBytes64'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint64_t <a href='#SkImageInfo_minRowBytes64'>minRowBytes64</a>() <a href='#SkImageInfo_minRowBytes64'>const</a>
</pre>

Returns minimum bytes per row, computed from <a href='undocumented#Pixel'>pixel</a> <a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkColorType'>which</a>
specifies <a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>(). <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>maximum</a> <a href='SkBitmap_Reference#SkBitmap'>value</a> <a href='SkBitmap_Reference#SkBitmap'>for</a> <a href='SkBitmap_Reference#SkBitmap'>row</a> <a href='SkBitmap_Reference#SkBitmap'>bytes</a> <a href='SkBitmap_Reference#SkBitmap'>must</a> <a href='SkBitmap_Reference#SkBitmap'>fit</a>
in 31 bits.

### Return Value

<a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>times</a> <a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>() <a href='#SkImageInfo_bytesPerPixel'>as</a> <a href='#SkImageInfo_bytesPerPixel'>unsigned</a> 64-<a href='#SkImageInfo_bytesPerPixel'>bit</a> <a href='#SkImageInfo_bytesPerPixel'>integer</a>

### Example

<div><fiddle-embed name="4b5d3904476726a39f1c3e276d6b6ba7">

#### Example Output

~~~~
RGBA_F16 width 16777216 (0x01000000) OK
RGBA_F16 width 33554432 (0x02000000) OK
RGBA_F16 width 67108864 (0x04000000) OK
RGBA_F16 width 134217728 (0x08000000) OK
RGBA_F16 width 268435456 (0x10000000) too large
RGBA_F16 width 536870912 (0x20000000) too large
RGBA_F16 width 1073741824 (0x40000000) too large
RGBA_F16 width -2147483648 (0x80000000) too large
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_minRowBytes'>minRowBytes</a> <a href='#SkImageInfo_computeByteSize'>computeByteSize</a> <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a> <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>

<a name='SkImageInfo_minRowBytes'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkImageInfo_minRowBytes'>minRowBytes</a>() <a href='#SkImageInfo_minRowBytes'>const</a>
</pre>

Returns minimum bytes per row, computed from <a href='undocumented#Pixel'>pixel</a> <a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkColorType'>which</a>
specifies <a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>(). <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>maximum</a> <a href='SkBitmap_Reference#SkBitmap'>value</a> <a href='SkBitmap_Reference#SkBitmap'>for</a> <a href='SkBitmap_Reference#SkBitmap'>row</a> <a href='SkBitmap_Reference#SkBitmap'>bytes</a> <a href='SkBitmap_Reference#SkBitmap'>must</a> <a href='SkBitmap_Reference#SkBitmap'>fit</a>
in 31 bits.

### Return Value

<a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>times</a> <a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>() <a href='#SkImageInfo_bytesPerPixel'>as</a> <a href='#SkImageInfo_bytesPerPixel'>signed</a> 32-<a href='#SkImageInfo_bytesPerPixel'>bit</a> <a href='#SkImageInfo_bytesPerPixel'>integer</a>

### Example

<div><fiddle-embed name="897230ecfb36095486beca324fd369f9">

#### Example Output

~~~~
RGBA_F16 width 16777216 (0x01000000) OK
RGBA_F16 width 33554432 (0x02000000) OK
RGBA_F16 width 67108864 (0x04000000) OK
RGBA_F16 width 134217728 (0x08000000) OK
RGBA_F16 width 268435456 (0x10000000) too large
RGBA_F16 width 536870912 (0x20000000) too large
RGBA_F16 width 1073741824 (0x40000000) too large
RGBA_F16 width -2147483648 (0x80000000) too large
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_minRowBytes64'>minRowBytes64</a> <a href='#SkImageInfo_computeByteSize'>computeByteSize</a> <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a> <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>

<a name='SkImageInfo_computeOffset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkImageInfo_computeOffset'>computeOffset</a>(<a href='#SkImageInfo_computeOffset'>int</a> <a href='#SkImageInfo_computeOffset'>x</a>, <a href='#SkImageInfo_computeOffset'>int</a> <a href='#SkImageInfo_computeOffset'>y</a>, <a href='#SkImageInfo_computeOffset'>size_t</a> <a href='#SkImageInfo_computeOffset'>rowBytes</a>) <a href='#SkImageInfo_computeOffset'>const</a>
</pre>

Returns byte offset of <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>from</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>base</a> <a href='undocumented#Pixel'>address</a>.

Asserts in debug build if <a href='#SkImageInfo_computeOffset_x'>x</a> <a href='#SkImageInfo_computeOffset_x'>or</a> <a href='#SkImageInfo_computeOffset_y'>y</a> <a href='#SkImageInfo_computeOffset_y'>is</a> <a href='#SkImageInfo_computeOffset_y'>outside</a> <a href='#SkImageInfo_computeOffset_y'>of</a> <a href='#SkImageInfo_computeOffset_y'>bounds</a>. <a href='#SkImageInfo_computeOffset_y'>Does</a> <a href='#SkImageInfo_computeOffset_y'>not</a> <a href='#SkImageInfo_computeOffset_y'>assert</a> <a href='#SkImageInfo_computeOffset_y'>if</a>
<a href='#SkImageInfo_computeOffset_rowBytes'>rowBytes</a> <a href='#SkImageInfo_computeOffset_rowBytes'>is</a> <a href='#SkImageInfo_computeOffset_rowBytes'>smaller</a> <a href='#SkImageInfo_computeOffset_rowBytes'>than</a> <a href='#SkImageInfo_minRowBytes'>minRowBytes</a>(), <a href='#SkImageInfo_minRowBytes'>even</a> <a href='#SkImageInfo_minRowBytes'>though</a> <a href='#SkImageInfo_minRowBytes'>result</a> <a href='#SkImageInfo_minRowBytes'>may</a> <a href='#SkImageInfo_minRowBytes'>be</a> <a href='#SkImageInfo_minRowBytes'>incorrect</a>.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_computeOffset_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkImageInfo_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_computeOffset_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkImageInfo_height'>height()</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_computeOffset_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>larger</a></td>
  </tr>
</table>

### Return Value

offset within <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>array</a>

### Example

<div><fiddle-embed name="818e4e1191e39d2a642902cbf253b399"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_height'>height</a> <a href='#SkImageInfo_width'>width</a> <a href='#SkImageInfo_minRowBytes'>minRowBytes</a> <a href='#SkImageInfo_computeByteSize'>computeByteSize</a>

<a name='SkImageInfo_equal1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>other</a>) <a href='SkImageInfo_Reference#SkImageInfo'>const</a>
</pre>

Compares <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>with</a> <a href='#SkImageInfo_equal1_operator_other'>other</a>, <a href='#SkImageInfo_equal1_operator_other'>and</a> <a href='#SkImageInfo_equal1_operator_other'>returns</a> <a href='#SkImageInfo_equal1_operator_other'>true</a> <a href='#SkImageInfo_equal1_operator_other'>if</a> <a href='#SkImageInfo_equal1_operator_other'>width</a>, <a href='#SkImageInfo_equal1_operator_other'>height</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>,
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>are</a> <a href='undocumented#SkColorSpace'>equivalent</a>.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_equal1_operator_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='SkImageInfo_Reference#SkImageInfo'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>equals</a> <a href='#SkImageInfo_equal1_operator_other'>other</a>

### Example

<div><fiddle-embed name="53c212c4f2449df0b0eedbc6227b6ab7">

#### Example Output

~~~~
info1 != info2
info1 != info2
info1 != info2
info1 == info2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_notequal1_operator'>operator!=(const SkImageInfo& other)_const</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>::<a href='#SkColorSpace_Equals'>Equals</a>

<a name='SkImageInfo_notequal1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>other</a>) <a href='SkImageInfo_Reference#SkImageInfo'>const</a>
</pre>

Compares <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>with</a> <a href='#SkImageInfo_notequal1_operator_other'>other</a>, <a href='#SkImageInfo_notequal1_operator_other'>and</a> <a href='#SkImageInfo_notequal1_operator_other'>returns</a> <a href='#SkImageInfo_notequal1_operator_other'>true</a> <a href='#SkImageInfo_notequal1_operator_other'>if</a> <a href='#SkImageInfo_notequal1_operator_other'>width</a>, <a href='#SkImageInfo_notequal1_operator_other'>height</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>,
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>are</a> <a href='undocumented#SkColorSpace'>not</a> <a href='undocumented#SkColorSpace'>equivalent</a>.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_notequal1_operator_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='SkImageInfo_Reference#SkImageInfo'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>is</a> <a href='SkImageInfo_Reference#SkImageInfo'>not</a> <a href='SkImageInfo_Reference#SkImageInfo'>equal</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkImageInfo_notequal1_operator_other'>other</a>

### Example

<div><fiddle-embed name="8c039fde0a476ac1aa62bf9de5d61c77">

#### Example Output

~~~~
info1 != info2
info1 != info2
info1 != info2
info1 == info2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_equal1_operator'>operator==(const SkImageInfo& other)_const</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>::<a href='#SkColorSpace_Equals'>Equals</a>

<a name='SkImageInfo_computeByteSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkImageInfo_computeByteSize'>computeByteSize</a>(<a href='#SkImageInfo_computeByteSize'>size_t</a> <a href='#SkImageInfo_computeByteSize'>rowBytes</a>) <a href='#SkImageInfo_computeByteSize'>const</a>
</pre>

Returns storage required by <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>array</a>, <a href='undocumented#Pixel'>given</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>dimensions</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>,
and <a href='#SkImageInfo_computeByteSize_rowBytes'>rowBytes</a>. <a href='#SkImageInfo_computeByteSize_rowBytes'>rowBytes</a> <a href='#SkImageInfo_computeByteSize_rowBytes'>is</a> <a href='#SkImageInfo_computeByteSize_rowBytes'>assumed</a> <a href='#SkImageInfo_computeByteSize_rowBytes'>to</a> <a href='#SkImageInfo_computeByteSize_rowBytes'>be</a> <a href='#SkImageInfo_computeByteSize_rowBytes'>at</a> <a href='#SkImageInfo_computeByteSize_rowBytes'>least</a> <a href='#SkImageInfo_computeByteSize_rowBytes'>as</a> <a href='#SkImageInfo_computeByteSize_rowBytes'>large</a> <a href='#SkImageInfo_computeByteSize_rowBytes'>as</a> <a href='#SkImageInfo_minRowBytes'>minRowBytes</a>().

Returns zero if height is zero.
Returns SIZE_MAX if answer exceeds the range of size_t.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_computeByteSize_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>larger</a></td>
  </tr>
</table>

### Return Value

memory required by <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>buffer</a>

### Example

<div><fiddle-embed name="9def507d2295f7051effd0c83bb04436"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a> <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>

<a name='SkImageInfo_computeMinByteSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a>() <a href='#SkImageInfo_computeMinByteSize'>const</a>
</pre>

Returns storage required by <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>array</a>, <a href='undocumented#Pixel'>given</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>dimensions</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a>
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>. <a href='SkImageInfo_Reference#SkColorType'>Uses</a> <a href='#SkImageInfo_minRowBytes'>minRowBytes</a>() <a href='#SkImageInfo_minRowBytes'>to</a> <a href='#SkImageInfo_minRowBytes'>compute</a> <a href='#SkImageInfo_minRowBytes'>bytes</a> <a href='#SkImageInfo_minRowBytes'>for</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a>.

Returns zero if height is zero.
Returns SIZE_MAX if answer exceeds the range of size_t.

### Return Value

least memory required by <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>buffer</a>

### Example

<div><fiddle-embed name="fc18640fdde437cb35338aed7c68d399"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_computeByteSize'>computeByteSize</a> <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>

<a name='SkImageInfo_ByteSizeOverflowed'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkImageInfo_ByteSizeOverflowed'>ByteSizeOverflowed</a>(<a href='#SkImageInfo_ByteSizeOverflowed'>size_t</a> <a href='#SkImageInfo_ByteSizeOverflowed'>byteSize</a>)
</pre>

Returns true if <a href='#SkImageInfo_ByteSizeOverflowed_byteSize'>byteSize</a> <a href='#SkImageInfo_ByteSizeOverflowed_byteSize'>equals</a> <a href='#SkImageInfo_ByteSizeOverflowed_byteSize'>SIZE_MAX</a>. <a href='#SkImageInfo_computeByteSize'>computeByteSize</a>() <a href='#SkImageInfo_computeByteSize'>and</a>
<a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a>() <a href='#SkImageInfo_computeMinByteSize'>return</a> <a href='#SkImageInfo_computeMinByteSize'>SIZE_MAX</a> <a href='#SkImageInfo_computeMinByteSize'>if</a> <a href='#SkImageInfo_computeMinByteSize'>size_t</a> <a href='#SkImageInfo_computeMinByteSize'>can</a> <a href='#SkImageInfo_computeMinByteSize'>not</a> <a href='#SkImageInfo_computeMinByteSize'>hold</a> <a href='#SkImageInfo_computeMinByteSize'>buffer</a> <a href='undocumented#Size'>size</a>.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_ByteSizeOverflowed_byteSize'><code><strong>byteSize</strong></code></a></td>
    <td>result of <a href='#SkImageInfo_computeByteSize'>computeByteSize</a>() <a href='#SkImageInfo_computeByteSize'>or</a> <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a>()</td>
  </tr>
</table>

### Return Value

true if <a href='#SkImageInfo_computeByteSize'>computeByteSize</a>() <a href='#SkImageInfo_computeByteSize'>or</a> <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a>() <a href='#SkImageInfo_computeMinByteSize'>result</a> <a href='#SkImageInfo_computeMinByteSize'>exceeds</a> <a href='#SkImageInfo_computeMinByteSize'>size_t</a>

### Example

<div><fiddle-embed name="6a63dfdd62ab77ff57783af8c33d7b78">

#### Example Output

~~~~
rowBytes:100000000 size:99999999900000008 overflowed:false
rowBytes:1000000000 size:999999999000000008 overflowed:false
rowBytes:10000000000 size:9999999990000000008 overflowed:false
rowBytes:100000000000 size:18446744073709551615 overflowed:true
rowBytes:1000000000000 size:18446744073709551615 overflowed:true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_computeByteSize'>computeByteSize</a> <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a> <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>

<a name='SkImageInfo_validRowBytes'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>(<a href='#SkImageInfo_validRowBytes'>size_t</a> <a href='#SkImageInfo_validRowBytes'>rowBytes</a>) <a href='#SkImageInfo_validRowBytes'>const</a>
</pre>

Returns true if <a href='#SkImageInfo_validRowBytes_rowBytes'>rowBytes</a> <a href='#SkImageInfo_validRowBytes_rowBytes'>is</a> <a href='#SkImageInfo_validRowBytes_rowBytes'>smaller</a> <a href='#SkImageInfo_validRowBytes_rowBytes'>than</a> <a href='#SkImageInfo_validRowBytes_rowBytes'>width</a> <a href='#SkImageInfo_validRowBytes_rowBytes'>times</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_validRowBytes_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>larger</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkImageInfo_validRowBytes_rowBytes'>rowBytes</a> <a href='#SkImageInfo_validRowBytes_rowBytes'>is</a> <a href='#SkImageInfo_validRowBytes_rowBytes'>large</a> <a href='#SkImageInfo_validRowBytes_rowBytes'>enough</a> <a href='#SkImageInfo_validRowBytes_rowBytes'>to</a> <a href='#SkImageInfo_validRowBytes_rowBytes'>contain</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a>

### Example

<div><fiddle-embed name="c6b0f6a3f493cb08d9abcdefe12de245">

#### Example Output

~~~~
validRowBytes(60): false
validRowBytes(64): true
validRowBytes(68): true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_ByteSizeOverflowed'>ByteSizeOverflowed</a> <a href='#SkImageInfo_computeByteSize'>computeByteSize</a> <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a>

<a name='SkImageInfo_reset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkImageInfo_reset'>reset()</a>
</pre>

Creates an empty <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>with</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>,
a width and height of zero, and no <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

### Example

<div><fiddle-embed name="ab7e73786805c936de386b6c1ebe1f13">

#### Example Output

~~~~
info == copy
info != reset copy
SkImageInfo() == reset copy
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_empty_constructor'>SkImageInfo()</a>

<a name='Utility'></a>

<a name='SkImageInfo_validate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkImageInfo_validate'>validate()</a> <a href='#SkImageInfo_validate'>const</a>
</pre>

### See Also

<a href='#SkImageInfo_validRowBytes'>validRowBytes</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_validate'>validate</a>

