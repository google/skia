SkColor Reference
===


<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
typedef uint8_t <a href='SkColor_Reference#SkAlpha'>SkAlpha</a>;
<a href='SkColor_Reference#SkAlpha'>typedef</a> <a href='SkColor_Reference#SkAlpha'>uint32_t</a> <a href='SkColor_Reference#SkColor'>SkColor</a>;

<a href='SkColor_Reference#SkColor'>static</a> <a href='SkColor_Reference#SkColor'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>a</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>r</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>g</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>b</a>);
#<a href='undocumented#U8CPU'>define</a> <a href='SkColor_Reference#SkColorSetRGB'>SkColorSetRGB</a>(<a href='SkColor_Reference#SkColorSetRGB'>r</a>, <a href='SkColor_Reference#SkColorSetRGB'>g</a>, <a href='SkColor_Reference#SkColorSetRGB'>b</a>) <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, <a href='SkColor_Reference#SkColorSetARGB'>r</a>, <a href='SkColor_Reference#SkColorSetARGB'>g</a>, <a href='SkColor_Reference#SkColorSetARGB'>b</a>)
#<a href='SkColor_Reference#SkColorSetARGB'>define</a> <a href='SkColor_Reference#SkColorGetA'>SkColorGetA</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 24) & 0<a href='SkColor_Reference#Color'>xFF</a>)
#<a href='SkColor_Reference#Color'>define</a> <a href='SkColor_Reference#SkColorGetR'>SkColorGetR</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 16) & 0<a href='SkColor_Reference#Color'>xFF</a>)
#<a href='SkColor_Reference#Color'>define</a> <a href='SkColor_Reference#SkColorGetG'>SkColorGetG</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 8) & 0<a href='SkColor_Reference#Color'>xFF</a>)
#<a href='SkColor_Reference#Color'>define</a> <a href='SkColor_Reference#SkColorGetB'>SkColorGetB</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 0) & 0<a href='SkColor_Reference#Color'>xFF</a>)

<a href='SkColor_Reference#Color'>static</a> <a href='SkColor_Reference#Color'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColorSetA'>SkColorSetA</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>c</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>a</a>);
<a href='undocumented#U8CPU'>constexpr</a> <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SK_AlphaTRANSPARENT'>SK_AlphaTRANSPARENT</a> = 0<a href='SkColor_Reference#SK_AlphaTRANSPARENT'>x00</a>;
<a href='SkColor_Reference#SK_AlphaTRANSPARENT'>constexpr</a> <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SK_AlphaOPAQUE'>SK_AlphaOPAQUE</a> = 0<a href='SkColor_Reference#SK_AlphaOPAQUE'>xFF</a>;
<a href='SkColor_Reference#SK_AlphaOPAQUE'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>);
<a href='SkColor_Reference#SkColorSetARGB'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorBLACK'>SK_ColorBLACK</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>);
<a href='SkColor_Reference#SkColorSetARGB'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorDKGRAY'>SK_ColorDKGRAY</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x44</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x44</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x44</a>);
<a href='SkColor_Reference#SkColorSetARGB'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorGRAY'>SK_ColorGRAY</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x88</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x88</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x88</a>);
<a href='SkColor_Reference#SkColorSetARGB'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorLTGRAY'>SK_ColorLTGRAY</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xCC</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xCC</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xCC</a>);
<a href='SkColor_Reference#SkColorSetARGB'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorWHITE'>SK_ColorWHITE</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>);
<a href='SkColor_Reference#SkColorSetARGB'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorRED'>SK_ColorRED</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>);
<a href='SkColor_Reference#SkColorSetARGB'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorGREEN'>SK_ColorGREEN</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>);
<a href='SkColor_Reference#SkColorSetARGB'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorBLUE'>SK_ColorBLUE</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>);
<a href='SkColor_Reference#SkColorSetARGB'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorYELLOW'>SK_ColorYELLOW</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>);
<a href='SkColor_Reference#SkColorSetARGB'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorCYAN'>SK_ColorCYAN</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>);
<a href='SkColor_Reference#SkColorSetARGB'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorMAGENTA'>SK_ColorMAGENTA</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>x00</a>, 0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>);

<a href='SkColor_Reference#SkColorSetARGB'>void</a> <a href='SkColor_Reference#SkRGBToHSV'>SkRGBToHSV</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>red</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>green</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>blue</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>hsv</a>[3]);

<a href='undocumented#SkScalar'>static</a> <a href='undocumented#SkScalar'>void</a> <a href='SkColor_Reference#SkColorToHSV'>SkColorToHSV</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>hsv</a>[3]);

<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkHSVToColor'>SkHSVToColor</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>hsv</a>[3]);

<a href='undocumented#SkScalar'>static</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkHSVToColor'>SkHSVToColor</a>(<a href='SkColor_Reference#SkHSVToColor'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>hsv</a>[3]);
<a href='undocumented#SkScalar'>typedef</a> <a href='undocumented#SkScalar'>uint32_t</a> <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>;

<a href='SkColor_Reference#SkPMColor'>SkPMColor</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>a</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>r</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>g</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>b</a>);

<a href='SkColor_Reference#SkPMColor'>SkPMColor</a> <a href='SkColor_Reference#SkPreMultiplyColor'>SkPreMultiplyColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>c</a>);
<a href='SkColor_Reference#SkColor'>template</a> <<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>kAT</a>>
<a href='SkImageInfo_Reference#SkAlphaType'>struct</a> <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> {
    // <<a href='SkColor4f_Reference#SkRGBA4f'>i</a>><a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>interface</a></<a href='SkColor4f_Reference#SkRGBA4f'>i</a>>
};
<a href='SkColor4f_Reference#SkRGBA4f'>template</a> <> <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a> <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>::<a href='SkColor4f_Reference#SkColor4f'>FromColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a>);
<a href='SkColor_Reference#SkColor'>template</a> <> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>::<a href='SkColor4f_Reference#SkColor4f'>toSkColor</a>() <a href='SkColor4f_Reference#SkColor4f'>const</a>;
</pre>

<a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>constants</a> <a href='SkColor_Reference#Color'>can</a> <a href='SkColor_Reference#Color'>be</a> <a href='SkColor_Reference#Color'>helpful</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>write</a> <a href='SkColor_Reference#Color'>code</a>, <a href='SkColor_Reference#Color'>documenting</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>meaning</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>values</a>
<a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>represent</a> <a href='SkColor_Reference#Color'>transparency</a> <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>values</a>. <a href='SkColor_Reference#Color'>The</a> <a href='SkColor_Reference#Color'>use</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>constants</a> <a href='SkColor_Reference#Color'>is</a> <a href='SkColor_Reference#Color'>not</a>
<a href='SkColor_Reference#Color'>required</a>.

<a name='Functions'></a>

<a name='Alpha'></a>

<a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>represents</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>transparency</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkColor_Reference#Color'>Color</a>. <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>with</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkColor_Reference#Alpha'>zero</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>fully</a>
<a href='SkColor_Reference#Alpha'>transparent</a>. <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>with</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>of</a> 255 <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a>. <a href='SkColor_Reference#Alpha'>Some</a>, <a href='SkColor_Reference#Alpha'>but</a> <a href='SkColor_Reference#Alpha'>not</a> <a href='SkColor_Reference#Alpha'>all</a> <a href='undocumented#Pixel'>pixel</a>
<a href='undocumented#Pixel'>formats</a> <a href='undocumented#Pixel'>contain</a> <a href='SkColor_Reference#Alpha'>Alpha</a>. <a href='SkColor_Reference#Alpha'>Pixels</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>store</a> <a href='SkColor_Reference#Alpha'>it</a> <a href='SkColor_Reference#Alpha'>as</a> <a href='SkColor_Reference#Alpha'>unsigned</a> <a href='SkColor_Reference#Alpha'>integers</a> <a href='SkColor_Reference#Alpha'>or</a>
<a href='SkColor_Reference#Alpha'>floating</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>values</a>. <a href='SkPoint_Reference#Point'>Unsigned</a> <a href='SkPoint_Reference#Point'>integer</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>ranges</a> <a href='SkColor_Reference#Alpha'>from</a> <a href='SkColor_Reference#Alpha'>zero</a>, <a href='SkColor_Reference#Alpha'>fully</a>
<a href='SkColor_Reference#Alpha'>transparent</a>, <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>all</a> <a href='SkColor_Reference#Alpha'>bits</a> <a href='SkColor_Reference#Alpha'>set</a>, <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a>. <a href='SkColor_Reference#Alpha'>Floating</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>ranges</a> <a href='SkColor_Reference#Alpha'>from</a>
<a href='SkColor_Reference#Alpha'>zero</a>, <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>transparent</a>, <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>one</a>, <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a>.

<a name='SkAlpha'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
typedef uint8_t <a href='SkColor_Reference#SkAlpha'>SkAlpha</a>;
</pre>

8-bit type for an <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>value</a>. 255 <a href='SkColor_Reference#Alpha'>is</a> 100% <a href='SkColor_Reference#Alpha'>opaque</a>, <a href='SkColor_Reference#Alpha'>zero</a> <a href='SkColor_Reference#Alpha'>is</a> 100% <a href='SkColor_Reference#Alpha'>transparent</a>.

<a name='SkColor'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
typedef uint32_t <a href='SkColor_Reference#SkColor'>SkColor</a>;
</pre>

32-bit ARGB <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>value</a>, <a href='undocumented#Unpremultiply'>Unpremultiplied</a>. <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>components</a> <a href='SkColor_Reference#Color'>are</a> <a href='SkColor_Reference#Color'>always</a> <a href='SkColor_Reference#Color'>in</a>
<a href='SkColor_Reference#Color'>a</a> <a href='SkColor_Reference#Color'>known</a> <a href='SkColor_Reference#Color'>order</a>. <a href='SkColor_Reference#Color'>This</a> <a href='SkColor_Reference#Color'>is</a> <a href='SkColor_Reference#Color'>different</a> <a href='SkColor_Reference#Color'>from</a> <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>, <a href='SkColor_Reference#SkPMColor'>which</a> <a href='SkColor_Reference#SkPMColor'>has</a> <a href='SkColor_Reference#SkPMColor'>its</a> <a href='SkColor_Reference#SkPMColor'>bytes</a> <a href='SkColor_Reference#SkPMColor'>in</a> <a href='SkColor_Reference#SkPMColor'>a</a> <a href='SkColor_Reference#SkPMColor'>configuration</a>
<a href='SkColor_Reference#SkPMColor'>dependent</a> <a href='SkColor_Reference#SkPMColor'>order</a>, <a href='SkColor_Reference#SkPMColor'>to</a> <a href='SkColor_Reference#SkPMColor'>match</a> <a href='SkColor_Reference#SkPMColor'>the</a> <a href='SkColor_Reference#SkPMColor'>format</a> <a href='SkColor_Reference#SkPMColor'>of</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a>. <a href='SkColor_Reference#SkColor'>SkColor</a>
<a href='SkColor_Reference#SkColor'>is</a> <a href='SkColor_Reference#SkColor'>the</a> <a href='SkColor_Reference#SkColor'>type</a> <a href='SkColor_Reference#SkColor'>used</a> <a href='SkColor_Reference#SkColor'>to</a> <a href='SkColor_Reference#SkColor'>specify</a> <a href='SkColor_Reference#SkColor'>colors</a> <a href='SkColor_Reference#SkColor'>in</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>and</a> <a href='SkPaint_Reference#SkPaint'>in</a> <a href='SkPaint_Reference#SkPaint'>gradients</a>.

<a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>that</a> <a href='SkColor_Reference#Color'>is</a> <a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>has</a> <a href='undocumented#Premultiply'>the</a> <a href='undocumented#Premultiply'>same</a> <a href='undocumented#Premultiply'>component</a> <a href='undocumented#Premultiply'>values</a> <a href='undocumented#Premultiply'>as</a> <a href='SkColor_Reference#Color'>Color</a>
<a href='SkColor_Reference#Color'>that</a> <a href='SkColor_Reference#Color'>is</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='undocumented#Unpremultiply'>if</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>is</a> 255, <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a>, <a href='SkColor_Reference#Alpha'>although</a> <a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>have</a> <a href='SkColor_Reference#Alpha'>the</a>
<a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>in</a> <a href='SkColor_Reference#Alpha'>a</a> <a href='SkColor_Reference#Alpha'>different</a> <a href='SkColor_Reference#Alpha'>order</a>.

### See Also

<a href='SkColor_Reference#SkPMColor'>SkPMColor</a>

<a name='SkColorSetARGB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr inline <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>a</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>r</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>g</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>b</a>)
</pre>

Returns <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>value</a> <a href='SkColor_Reference#Color'>from</a> 8-<a href='SkColor_Reference#Color'>bit</a> <a href='SkColor_Reference#Color'>component</a> <a href='SkColor_Reference#Color'>values</a>. <a href='SkColor_Reference#Color'>Asserts</a> <a href='SkColor_Reference#Color'>if</a> <a href='SkColor_Reference#Color'>SK_DEBUG</a> <a href='SkColor_Reference#Color'>is</a> <a href='SkColor_Reference#Color'>defined</a>
<a href='SkColor_Reference#Color'>if</a> <a href='#SkColorSetARGB_a'>a</a>, <a href='#SkColorSetARGB_r'>r</a>, <a href='#SkColorSetARGB_g'>g</a>, <a href='#SkColorSetARGB_g'>or</a> <a href='#SkColorSetARGB_b'>b</a> <a href='#SkColorSetARGB_b'>exceed</a> 255. <a href='#SkColorSetARGB_b'>Since</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>is</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a>, <a href='#SkColorSetARGB_a'>a</a> <a href='#SkColorSetARGB_a'>may</a> <a href='#SkColorSetARGB_a'>be</a> <a href='#SkColorSetARGB_a'>smaller</a>
<a href='#SkColorSetARGB_a'>than</a> <a href='#SkColorSetARGB_a'>the</a> <a href='#SkColorSetARGB_a'>largest</a> <a href='#SkColorSetARGB_a'>of</a> <a href='#SkColorSetARGB_r'>r</a>, <a href='#SkColorSetARGB_g'>g</a>, <a href='#SkColorSetARGB_g'>and</a> <a href='#SkColorSetARGB_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkColorSetARGB_a'><code><strong>a</strong></code></a></td>
    <td>amount of <a href='SkColor_Reference#Alpha'>Alpha</a>, <a href='SkColor_Reference#Alpha'>from</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>transparent</a> (0) <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a> (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetARGB_r'><code><strong>r</strong></code></a></td>
    <td>amount of red, from no red (0) to full red (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetARGB_g'><code><strong>g</strong></code></a></td>
    <td>amount of green, from no green (0) to full green (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetARGB_b'><code><strong>b</strong></code></a></td>
    <td>amount of blue, from no blue (0) to full blue (255)</td>
  </tr>
</table>

### Return Value

<a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#Unpremultiply'>Unpremultiplied</a>

### Example

<div><fiddle-embed name="35888f0869e01a6e03b5b93bba563734"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorSetRGB'>SkColorSetRGB</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_setARGB'>setARGB</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_setColor'>setColor</a> <a href='SkColor_Reference#SkColorSetA'>SkColorSetA</a>

<a name='SkColorSetRGB'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#define <a href='SkColor_Reference#SkColorSetRGB'>SkColorSetRGB</a>(<a href='SkColor_Reference#SkColorSetRGB'>r</a>, <a href='SkColor_Reference#SkColorSetRGB'>g</a>, <a href='SkColor_Reference#SkColorSetRGB'>b</a>) <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0<a href='SkColor_Reference#SkColorSetARGB'>xFF</a>, <a href='SkColor_Reference#SkColorSetARGB'>r</a>, <a href='SkColor_Reference#SkColorSetARGB'>g</a>, <a href='SkColor_Reference#SkColorSetARGB'>b</a>);
</pre>

Returns <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>value</a> <a href='SkColor_Reference#Color'>from</a> 8-<a href='SkColor_Reference#Color'>bit</a> <a href='SkColor_Reference#Color'>component</a> <a href='SkColor_Reference#Color'>values</a>, <a href='SkColor_Reference#Color'>with</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>set</a>
<a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a> <a href='SkColor_Reference#Alpha'>to</a> 255.

### Parameters

<table>  <tr>    <td><a name='SkColorSetRGB_r'><code><strong>r</strong></code></a></td>
    <td>amount of red, from no red (0) to full red (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetRGB_g'><code><strong>g</strong></code></a></td>
    <td>amount of green, from no green (0) to full green (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetRGB_b'><code><strong>b</strong></code></a></td>
    <td>amount of blue, from no blue (0) to full blue (255)</td>
  </tr>
</table>

### Return Value

<a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>with</a> <a href='SkColor_Reference#Color'>opaque</a> <a href='SkColor_Reference#Alpha'>alpha</a>

### Example

<div><fiddle-embed name="dad12dd912197cd5edd789ac0801bf8a"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>

<a name='SkColorGetA'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#define <a href='SkColor_Reference#SkColorGetA'>SkColorGetA</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 24) & 0<a href='SkColor_Reference#Color'>xFF</a>);
</pre>

Returns <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>byte</a> <a href='SkColor_Reference#Alpha'>from</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>value</a>.

### Parameters

<table>  <tr>    <td><a name='SkColorGetA_color'><code><strong>color</strong></code></a></td>
    <td><a href='SkColor_Reference#SkColor'>SkColor</a>, <a href='SkColor_Reference#SkColor'>a</a> 32-<a href='SkColor_Reference#SkColor'>bit</a> <a href='SkColor_Reference#SkColor'>unsigned</a> <a href='SkColor_Reference#SkColor'>int</a>, <a href='SkColor_Reference#SkColor'>in</a> 0<a href='SkColor_Reference#SkColor'>xAARRGGBB</a> <a href='SkColor_Reference#SkColor'>format</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="896ce0316b489608a95af5439ca2aab1"></fiddle-embed></div>

### See Also

<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_getAlpha'>getAlpha</a>

<a name='SkColorGetR'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#define <a href='SkColor_Reference#SkColorGetR'>SkColorGetR</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 16) & 0<a href='SkColor_Reference#Color'>xFF</a>);
</pre>

Returns red component of <a href='SkColor_Reference#Color'>Color</a>, <a href='SkColor_Reference#Color'>from</a> <a href='SkColor_Reference#Color'>zero</a> <a href='SkColor_Reference#Color'>to</a> 255.

### Parameters

<table>  <tr>    <td><a name='SkColorGetR_color'><code><strong>color</strong></code></a></td>
    <td><a href='SkColor_Reference#SkColor'>SkColor</a>, <a href='SkColor_Reference#SkColor'>a</a> 32-<a href='SkColor_Reference#SkColor'>bit</a> <a href='SkColor_Reference#SkColor'>unsigned</a> <a href='SkColor_Reference#SkColor'>int</a>, <a href='SkColor_Reference#SkColor'>in</a> 0<a href='SkColor_Reference#SkColor'>xAARRGGBB</a> <a href='SkColor_Reference#SkColor'>format</a></td>
  </tr>
</table>

### Return Value

red byte

### Example

<div><fiddle-embed name="d6da38577f189eaa6d9df75f6c3ed252"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorGetG'>SkColorGetG</a> <a href='SkColor_Reference#SkColorGetB'>SkColorGetB</a>

<a name='SkColorGetG'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#define <a href='SkColor_Reference#SkColorGetG'>SkColorGetG</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 8) & 0<a href='SkColor_Reference#Color'>xFF</a>);
</pre>

Returns green component of <a href='SkColor_Reference#Color'>Color</a>, <a href='SkColor_Reference#Color'>from</a> <a href='SkColor_Reference#Color'>zero</a> <a href='SkColor_Reference#Color'>to</a> 255.

### Parameters

<table>  <tr>    <td><a name='SkColorGetG_color'><code><strong>color</strong></code></a></td>
    <td><a href='SkColor_Reference#SkColor'>SkColor</a>, <a href='SkColor_Reference#SkColor'>a</a> 32-<a href='SkColor_Reference#SkColor'>bit</a> <a href='SkColor_Reference#SkColor'>unsigned</a> <a href='SkColor_Reference#SkColor'>int</a>, <a href='SkColor_Reference#SkColor'>in</a> 0<a href='SkColor_Reference#SkColor'>xAARRGGBB</a> <a href='SkColor_Reference#SkColor'>format</a></td>
  </tr>
</table>

### Return Value

green byte

### Example

<div><fiddle-embed name="535d38b2c019299d915170f7b03d5fea"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorGetR'>SkColorGetR</a> <a href='SkColor_Reference#SkColorGetB'>SkColorGetB</a>

<a name='SkColorGetB'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#define <a href='SkColor_Reference#SkColorGetB'>SkColorGetB</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 0) & 0<a href='SkColor_Reference#Color'>xFF</a>);
</pre>

Returns blue component of <a href='SkColor_Reference#Color'>Color</a>, <a href='SkColor_Reference#Color'>from</a> <a href='SkColor_Reference#Color'>zero</a> <a href='SkColor_Reference#Color'>to</a> 255.

### Parameters

<table>  <tr>    <td><a name='SkColorGetB_color'><code><strong>color</strong></code></a></td>
    <td><a href='SkColor_Reference#SkColor'>SkColor</a>, <a href='SkColor_Reference#SkColor'>a</a> 32-<a href='SkColor_Reference#SkColor'>bit</a> <a href='SkColor_Reference#SkColor'>unsigned</a> <a href='SkColor_Reference#SkColor'>int</a>, <a href='SkColor_Reference#SkColor'>in</a> 0<a href='SkColor_Reference#SkColor'>xAARRGGBB</a> <a href='SkColor_Reference#SkColor'>format</a></td>
  </tr>
</table>

### Return Value

blue byte

### Example

<div><fiddle-embed name="9ee27675284faea375611dc88123a2c5"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorGetR'>SkColorGetR</a> <a href='SkColor_Reference#SkColorGetG'>SkColorGetG</a>

<a name='SkColorSetA'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr inline <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColorSetA'>SkColorSetA</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>c</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>a</a>)
</pre>

Returns <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>with</a> <a href='SkColor_Reference#Color'>red</a>, <a href='SkColor_Reference#Color'>blue</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>green</a> <a href='SkColor_Reference#Color'>set</a> <a href='SkColor_Reference#Color'>from</a> <a href='#SkColorSetA_c'>c</a>; <a href='#SkColorSetA_c'>and</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>set</a>
<a href='SkColor_Reference#Alpha'>from</a> <a href='#SkColorSetA_a'>a</a>. <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='#SkColorSetA_c'>c</a> <a href='#SkColorSetA_c'>is</a> <a href='#SkColorSetA_c'>ignored</a> <a href='#SkColorSetA_c'>and</a> <a href='#SkColorSetA_c'>is</a> <a href='#SkColorSetA_c'>replaced</a> <a href='#SkColorSetA_c'>by</a> <a href='#SkColorSetA_a'>a</a> <a href='#SkColorSetA_a'>in</a> <a href='#SkColorSetA_a'>result</a>.

### Parameters

<table>  <tr>    <td><a name='SkColorSetA_c'><code><strong>c</strong></code></a></td>
    <td>packed RGB, eight bits per component</td>
  </tr>
  <tr>    <td><a name='SkColorSetA_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkColor_Reference#Alpha'>Alpha</a>: <a href='SkColor_Reference#Alpha'>transparent</a> <a href='SkColor_Reference#Alpha'>at</a> <a href='SkColor_Reference#Alpha'>zero</a>, <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a> <a href='SkColor_Reference#Alpha'>at</a> 255</td>
  </tr>
</table>

### Return Value

<a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>with</a> <a href='SkColor_Reference#Color'>transparency</a>

### Example

<div><fiddle-embed name="18f6f376f771f5ffa56d5e5b2ebd20fb"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>

<a name='Alpha_Constants'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
constexpr <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SK_AlphaTRANSPARENT'>SK_AlphaTRANSPARENT</a> = 0<a href='SkColor_Reference#SK_AlphaTRANSPARENT'>x00</a>;
<a href='SkColor_Reference#SK_AlphaTRANSPARENT'>constexpr</a> <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SK_AlphaOPAQUE'>SK_AlphaOPAQUE</a> = 0<a href='SkColor_Reference#SK_AlphaOPAQUE'>xFF</a>;
</pre>

<a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>constants</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>conveniences</a> <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>represent</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>transparent</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>fully</a>
<a href='SkColor_Reference#Alpha'>opaque</a> <a href='SkColor_Reference#Alpha'>colors</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>masks</a>. <a href='SkColor_Reference#Alpha'>Their</a> <a href='SkColor_Reference#Alpha'>use</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>not</a> <a href='SkColor_Reference#Alpha'>required</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_AlphaTRANSPARENT'><code>SK_AlphaTRANSPARENT</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x00</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully transparent <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SkAlpha'>value</a>. <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SkAlpha'>ranges</a> <a href='SkColor_Reference#SkAlpha'>from</a> <a href='SkColor_Reference#SkAlpha'>zero</a>,
<a href='SkColor_Reference#SkAlpha'>fully</a> <a href='SkColor_Reference#SkAlpha'>transparent</a>; <a href='SkColor_Reference#SkAlpha'>to</a> 255, <a href='SkColor_Reference#SkAlpha'>fully</a> <a href='SkColor_Reference#SkAlpha'>opaque</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_AlphaOPAQUE'><code>SK_AlphaOPAQUE</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SkAlpha'>value</a>. <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SkAlpha'>ranges</a> <a href='SkColor_Reference#SkAlpha'>from</a> <a href='SkColor_Reference#SkAlpha'>zero</a>,
<a href='SkColor_Reference#SkAlpha'>fully</a> <a href='SkColor_Reference#SkAlpha'>transparent</a>; <a href='SkColor_Reference#SkAlpha'>to</a> 255, <a href='SkColor_Reference#SkAlpha'>fully</a> <a href='SkColor_Reference#SkAlpha'>opaque</a>.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bc9c7ea424d10bbcd1e5a88770d4794e"><div><a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>parts</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>red</a> <a href='SkBitmap_Reference#Bitmap'>if</a> <a href='SkBitmap_Reference#Bitmap'>they</a> <a href='SkBitmap_Reference#Bitmap'>mostly</a> <a href='SkBitmap_Reference#Bitmap'>contain</a> <a href='SkBitmap_Reference#Bitmap'>transparent</a> <a href='SkBitmap_Reference#Bitmap'>pixels</a>.
</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="0424f67ebc2858e8fd04ae3367b115ff"><div><a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>parts</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>green</a> <a href='SkBitmap_Reference#Bitmap'>if</a> <a href='SkBitmap_Reference#Bitmap'>they</a> <a href='SkBitmap_Reference#Bitmap'>contain</a> <a href='SkBitmap_Reference#Bitmap'>fully</a> <a href='SkBitmap_Reference#Bitmap'>opaque</a> <a href='SkBitmap_Reference#Bitmap'>pixels</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a> <a href='SkColor_Reference#SK_ColorBLACK'>SK_ColorBLACK</a>

<a name='Color_Constants'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a>;
<a href='SkColor_Reference#SK_ColorTRANSPARENT'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorBLACK'>SK_ColorBLACK</a>;
<a href='SkColor_Reference#SK_ColorBLACK'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorDKGRAY'>SK_ColorDKGRAY</a>;
<a href='SkColor_Reference#SK_ColorDKGRAY'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorGRAY'>SK_ColorGRAY</a>;
<a href='SkColor_Reference#SK_ColorGRAY'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorLTGRAY'>SK_ColorLTGRAY</a>;
<a href='SkColor_Reference#SK_ColorLTGRAY'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorWHITE'>SK_ColorWHITE</a>;
<a href='SkColor_Reference#SK_ColorWHITE'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorRED'>SK_ColorRED</a>;
<a href='SkColor_Reference#SK_ColorRED'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorGREEN'>SK_ColorGREEN</a>;
<a href='SkColor_Reference#SK_ColorGREEN'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorBLUE'>SK_ColorBLUE</a>;
<a href='SkColor_Reference#SK_ColorBLUE'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorYELLOW'>SK_ColorYELLOW</a>;
<a href='SkColor_Reference#SK_ColorYELLOW'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorCYAN'>SK_ColorCYAN</a>;
<a href='SkColor_Reference#SK_ColorCYAN'>constexpr</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorMAGENTA'>SK_ColorMAGENTA</a>;
</pre>

<a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>names</a> <a href='SkColor_Reference#Color'>are</a> <a href='SkColor_Reference#Color'>provided</a> <a href='SkColor_Reference#Color'>as</a> <a href='SkColor_Reference#Color'>conveniences</a>, <a href='SkColor_Reference#Color'>but</a> <a href='SkColor_Reference#Color'>are</a> <a href='SkColor_Reference#Color'>not</a> <a href='SkColor_Reference#Color'>otherwise</a> <a href='SkColor_Reference#Color'>special</a>.
<a href='SkColor_Reference#Color'>The</a> <a href='SkColor_Reference#Color'>values</a> <a href='SkColor_Reference#Color'>chosen</a> <a href='SkColor_Reference#Color'>for</a> <a href='SkColor_Reference#Color'>names</a> <a href='SkColor_Reference#Color'>may</a> <a href='SkColor_Reference#Color'>not</a> <a href='SkColor_Reference#Color'>be</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>same</a> <a href='SkColor_Reference#Color'>as</a> <a href='SkColor_Reference#Color'>values</a> <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>by</a>
<a href='undocumented#SVG'>SVG</a>, <a href='undocumented#SVG'>HTML</a>, <a href='undocumented#SVG'>CSS</a>, <a href='undocumented#SVG'>or</a> <a href='undocumented#SVG'>colors</a> <a href='undocumented#SVG'>named</a> <a href='undocumented#SVG'>by</a> <a href='undocumented#SVG'>a</a> <a href='undocumented#SVG'>platform</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorTRANSPARENT'><code>SK_ColorTRANSPARENT</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x00000000</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully transparent <a href='SkColor_Reference#SkColor'>SkColor</a>. <a href='SkColor_Reference#SkColor'>May</a> <a href='SkColor_Reference#SkColor'>be</a> <a href='SkColor_Reference#SkColor'>used</a> <a href='SkColor_Reference#SkColor'>to</a> <a href='SkColor_Reference#SkColor'>initialize</a> <a href='SkColor_Reference#SkColor'>a</a> <a href='SkColor_Reference#SkColor'>destination</a>
<a href='SkColor_Reference#SkColor'>containing</a> <a href='SkColor_Reference#SkColor'>a</a> <a href='SkColor_Reference#SkColor'>mask</a> <a href='SkColor_Reference#SkColor'>or</a> <a href='SkColor_Reference#SkColor'>a</a> <a href='SkColor_Reference#SkColor'>non-rectangular</a> <a href='SkImage_Reference#Image'>image</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorBLACK'><code>SK_ColorBLACK</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF000000</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque black.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorDKGRAY'><code>SK_ColorDKGRAY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF444444</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque dark gray.
Note that <a href='#SVG_darkgray'>SVG_darkgray</a> <a href='#SVG_darkgray'>is</a> <a href='#SVG_darkgray'>equivalent</a> <a href='#SVG_darkgray'>to</a> 0<a href='#SVG_darkgray'>xFFA9A9A9</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorGRAY'><code>SK_ColorGRAY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF888888</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque gray.
Note that <a href='#HTML_Gray'>HTML_Gray</a> <a href='#HTML_Gray'>is</a> <a href='#HTML_Gray'>equivalent</a> <a href='#HTML_Gray'>to</a> 0<a href='#HTML_Gray'>xFF808080</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorLTGRAY'><code>SK_ColorLTGRAY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFCCCCCC</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque light gray. <a href='#HTML_Silver'>HTML_Silver</a> <a href='#HTML_Silver'>is</a> <a href='#HTML_Silver'>equivalent</a> <a href='#HTML_Silver'>to</a> 0<a href='#HTML_Silver'>xFFC0C0C0</a>.
<a href='#HTML_Silver'>Note</a> <a href='#HTML_Silver'>that</a> <a href='#SVG_lightgray'>SVG_lightgray</a> <a href='#SVG_lightgray'>is</a> <a href='#SVG_lightgray'>equivalent</a> <a href='#SVG_lightgray'>to</a> 0<a href='#SVG_lightgray'>xFFD3D3D3</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorWHITE'><code>SK_ColorWHITE</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFFFFFF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque white.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorRED'><code>SK_ColorRED</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFF0000</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque red.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorGREEN'><code>SK_ColorGREEN</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF00FF00</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque green. <a href='#HTML_Lime'>HTML_Lime</a> <a href='#HTML_Lime'>is</a> <a href='#HTML_Lime'>equivalent</a>.
<a href='#HTML_Lime'>Note</a> <a href='#HTML_Lime'>that</a> <a href='#HTML_Green'>HTML_Green</a> <a href='#HTML_Green'>is</a> <a href='#HTML_Green'>equivalent</a> <a href='#HTML_Green'>to</a> 0<a href='#HTML_Green'>xFF008000</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorBLUE'><code>SK_ColorBLUE</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF0000FF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque blue.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorYELLOW'><code>SK_ColorYELLOW</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFFFF00</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque yellow.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorCYAN'><code>SK_ColorCYAN</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF00FFFF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque cyan. <a href='#HTML_Aqua'>HTML_Aqua</a> <a href='#HTML_Aqua'>is</a> <a href='#HTML_Aqua'>equivalent</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorMAGENTA'><code>SK_ColorMAGENTA</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFF00FF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque magenta. <a href='#HTML_Fuchsia'>HTML_Fuchsia</a> <a href='#HTML_Fuchsia'>is</a> <a href='#HTML_Fuchsia'>equivalent</a>.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1c2e38321464818847f953ddd45cb5a1"></fiddle-embed></div>

### Example

<div><fiddle-embed name="9ca1e2a5b9b4c92ecf4409d0813867d6"><div><a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a> <a href='SkColor_Reference#SK_ColorTRANSPARENT'>sets</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>components</a> <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>zero</a>.
</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="6971489f28291f08e429cc6ccc73b09b"><div><a href='SkColor_Reference#SK_ColorBLACK'>SK_ColorBLACK</a> <a href='SkColor_Reference#SK_ColorBLACK'>sets</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>one</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>components</a> <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>zero</a>.
</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="fce650f997e802d4e55edf62b8437a2d"><div><a href='SkColor_Reference#SK_ColorWHITE'>SK_ColorWHITE</a> <a href='SkColor_Reference#SK_ColorWHITE'>sets</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>components</a> <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>one</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_clear'>clear</a> <a href='SkColor_Reference#SK_AlphaOPAQUE'>SK_AlphaOPAQUE</a>

<a name='HSV'></a>

<a name='HSV_Hue'></a>

Hue represents an angle, in degrees, on a <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>wheel</a>. <a href='SkColor_Reference#Color'>Hue</a> <a href='SkColor_Reference#Color'>has</a> <a href='SkColor_Reference#Color'>a</a> <a href='SkColor_Reference#Color'>positive</a> <a href='SkColor_Reference#Color'>value</a>
<a href='SkColor_Reference#Color'>modulo</a> 360, <a href='SkColor_Reference#Color'>where</a> <a href='SkColor_Reference#Color'>zero</a> <a href='SkColor_Reference#Color'>degrees</a> <a href='SkColor_Reference#Color'>is</a> <a href='SkColor_Reference#Color'>red</a>.

<a name='HSV_Saturation'></a>

<a href='undocumented#Saturation'>Saturation</a> <a href='undocumented#Saturation'>represents</a> <a href='undocumented#Saturation'>the</a> <a href='undocumented#Saturation'>intensity</a> <a href='undocumented#Saturation'>of</a> <a href='undocumented#Saturation'>the</a> <a href='SkColor_Reference#Color'>color</a>. <a href='undocumented#Saturation'>Saturation</a> <a href='undocumented#Saturation'>varies</a> <a href='undocumented#Saturation'>from</a> <a href='undocumented#Saturation'>zero</a>,
<a href='undocumented#Saturation'>with</a> <a href='undocumented#Saturation'>no</a> <a href='undocumented#Saturation'>Hue</a> <a href='undocumented#Saturation'>contribution</a>; <a href='undocumented#Saturation'>to</a> <a href='undocumented#Saturation'>one</a>, <a href='undocumented#Saturation'>with</a> <a href='undocumented#Saturation'>full</a> <a href='undocumented#Saturation'>Hue</a> <a href='undocumented#Saturation'>contribution</a>.

<a name='HSV_Value'></a>

Value represents the lightness of the <a href='SkColor_Reference#Color'>color</a>. <a href='SkColor_Reference#Color'>Value</a> <a href='SkColor_Reference#Color'>varies</a> <a href='SkColor_Reference#Color'>from</a> <a href='SkColor_Reference#Color'>zero</a>, <a href='SkColor_Reference#Color'>black</a>; <a href='SkColor_Reference#Color'>to</a>
<a href='SkColor_Reference#Color'>one</a>, <a href='SkColor_Reference#Color'>full</a> <a href='SkColor_Reference#Color'>brightness</a>.

<a name='SkRGBToHSV'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='SkColor_Reference#SkRGBToHSV'>SkRGBToHSV</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>red</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>green</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>blue</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>hsv</a>[3])
</pre>

Converts RGB to its HSV components.
<a href='#SkRGBToHSV_hsv'>hsv</a>[0] <a href='#SkRGBToHSV_hsv'>contains</a> <a href='#Color_HSV_Hue'>HSV_Hue</a>, <a href='#Color_HSV_Hue'>a</a> <a href='#Color_HSV_Hue'>value</a> <a href='#Color_HSV_Hue'>from</a> <a href='#Color_HSV_Hue'>zero</a> <a href='#Color_HSV_Hue'>to</a> <a href='#Color_HSV_Hue'>less</a> <a href='#Color_HSV_Hue'>than</a> 360.
<a href='#SkRGBToHSV_hsv'>hsv</a>[1] <a href='#SkRGBToHSV_hsv'>contains</a> <a href='#Color_HSV_Saturation'>HSV_Saturation</a>, <a href='#Color_HSV_Saturation'>a</a> <a href='#Color_HSV_Saturation'>value</a> <a href='#Color_HSV_Saturation'>from</a> <a href='#Color_HSV_Saturation'>zero</a> <a href='#Color_HSV_Saturation'>to</a> <a href='#Color_HSV_Saturation'>one</a>.
<a href='#SkRGBToHSV_hsv'>hsv</a>[2] <a href='#SkRGBToHSV_hsv'>contains</a> <a href='#Color_HSV_Value'>HSV_Value</a>, <a href='#Color_HSV_Value'>a</a> <a href='#Color_HSV_Value'>value</a> <a href='#Color_HSV_Value'>from</a> <a href='#Color_HSV_Value'>zero</a> <a href='#Color_HSV_Value'>to</a> <a href='#Color_HSV_Value'>one</a>.

### Parameters

<table>  <tr>    <td><a name='SkRGBToHSV_red'><code><strong>red</strong></code></a></td>
    <td><a href='#SkRGBToHSV_red'>red</a> <a href='#SkRGBToHSV_red'>component</a> <a href='#SkRGBToHSV_red'>value</a> <a href='#SkRGBToHSV_red'>from</a> <a href='#SkRGBToHSV_red'>zero</a> <a href='#SkRGBToHSV_red'>to</a> 255</td>
  </tr>
  <tr>    <td><a name='SkRGBToHSV_green'><code><strong>green</strong></code></a></td>
    <td><a href='#SkRGBToHSV_green'>green</a> <a href='#SkRGBToHSV_green'>component</a> <a href='#SkRGBToHSV_green'>value</a> <a href='#SkRGBToHSV_green'>from</a> <a href='#SkRGBToHSV_green'>zero</a> <a href='#SkRGBToHSV_green'>to</a> 255</td>
  </tr>
  <tr>    <td><a name='SkRGBToHSV_blue'><code><strong>blue</strong></code></a></td>
    <td><a href='#SkRGBToHSV_blue'>blue</a> <a href='#SkRGBToHSV_blue'>component</a> <a href='#SkRGBToHSV_blue'>value</a> <a href='#SkRGBToHSV_blue'>from</a> <a href='#SkRGBToHSV_blue'>zero</a> <a href='#SkRGBToHSV_blue'>to</a> 255</td>
  </tr>
  <tr>    <td><a name='SkRGBToHSV_hsv'><code><strong>hsv</strong></code></a></td>
    <td>three element array which holds the resulting HSV components
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4fb2da4a3d9b14ca4ac24eefb0f5126a"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorToHSV'>SkColorToHSV</a> <a href='SkColor_Reference#SkHSVToColor'>SkHSVToColor</a>

<a name='SkColorToHSV'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='SkColor_Reference#SkColorToHSV'>SkColorToHSV</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>hsv</a>[3])
</pre>

Converts ARGB to its HSV components. <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>in</a> <a href='SkColor_Reference#Alpha'>ARGB</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>ignored</a>.
<a href='#SkColorToHSV_hsv'>hsv</a>[0] <a href='#SkColorToHSV_hsv'>contains</a> <a href='#Color_HSV_Hue'>HSV_Hue</a>, <a href='#Color_HSV_Hue'>and</a> <a href='#Color_HSV_Hue'>is</a> <a href='#Color_HSV_Hue'>assigned</a> <a href='#Color_HSV_Hue'>a</a> <a href='#Color_HSV_Hue'>value</a> <a href='#Color_HSV_Hue'>from</a> <a href='#Color_HSV_Hue'>zero</a> <a href='#Color_HSV_Hue'>to</a> <a href='#Color_HSV_Hue'>less</a> <a href='#Color_HSV_Hue'>than</a> 360.
<a href='#SkColorToHSV_hsv'>hsv</a>[1] <a href='#SkColorToHSV_hsv'>contains</a> <a href='#Color_HSV_Saturation'>HSV_Saturation</a>, <a href='#Color_HSV_Saturation'>a</a> <a href='#Color_HSV_Saturation'>value</a> <a href='#Color_HSV_Saturation'>from</a> <a href='#Color_HSV_Saturation'>zero</a> <a href='#Color_HSV_Saturation'>to</a> <a href='#Color_HSV_Saturation'>one</a>.
<a href='#SkColorToHSV_hsv'>hsv</a>[2] <a href='#SkColorToHSV_hsv'>contains</a> <a href='#Color_HSV_Value'>HSV_Value</a>, <a href='#Color_HSV_Value'>a</a> <a href='#Color_HSV_Value'>value</a> <a href='#Color_HSV_Value'>from</a> <a href='#Color_HSV_Value'>zero</a> <a href='#Color_HSV_Value'>to</a> <a href='#Color_HSV_Value'>one</a>.

### Parameters

<table>  <tr>    <td><a name='SkColorToHSV_color'><code><strong>color</strong></code></a></td>
    <td>ARGB <a href='#SkColorToHSV_color'>color</a> <a href='#SkColorToHSV_color'>to</a> <a href='#SkColorToHSV_color'>convert</a>
</td>
  </tr>
  <tr>    <td><a name='SkColorToHSV_hsv'><code><strong>hsv</strong></code></a></td>
    <td>three element array which holds the resulting HSV components
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1e0370f12c8aab5b84f9e824074f1e5a"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkRGBToHSV'>SkRGBToHSV</a> <a href='SkColor_Reference#SkHSVToColor'>SkHSVToColor</a>

<a name='SkHSVToColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkHSVToColor'>SkHSVToColor</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>hsv</a>[3])
</pre>

Converts HSV components to an ARGB <a href='SkColor_Reference#Color'>color</a>. <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>passed</a> <a href='SkColor_Reference#Alpha'>through</a> <a href='SkColor_Reference#Alpha'>unchanged</a>.
<a href='#SkHSVToColor_hsv'>hsv</a>[0] <a href='#SkHSVToColor_hsv'>represents</a> <a href='#Color_HSV_Hue'>HSV_Hue</a>, <a href='#Color_HSV_Hue'>an</a> <a href='#Color_HSV_Hue'>angle</a> <a href='#Color_HSV_Hue'>from</a> <a href='#Color_HSV_Hue'>zero</a> <a href='#Color_HSV_Hue'>to</a> <a href='#Color_HSV_Hue'>less</a> <a href='#Color_HSV_Hue'>than</a> 360.
<a href='#SkHSVToColor_hsv'>hsv</a>[1] <a href='#SkHSVToColor_hsv'>represents</a> <a href='#Color_HSV_Saturation'>HSV_Saturation</a>, <a href='#Color_HSV_Saturation'>and</a> <a href='#Color_HSV_Saturation'>varies</a> <a href='#Color_HSV_Saturation'>from</a> <a href='#Color_HSV_Saturation'>zero</a> <a href='#Color_HSV_Saturation'>to</a> <a href='#Color_HSV_Saturation'>one</a>.
<a href='#SkHSVToColor_hsv'>hsv</a>[2] <a href='#SkHSVToColor_hsv'>represents</a> <a href='#Color_HSV_Value'>HSV_Value</a>, <a href='#Color_HSV_Value'>and</a> <a href='#Color_HSV_Value'>varies</a> <a href='#Color_HSV_Value'>from</a> <a href='#Color_HSV_Value'>zero</a> <a href='#Color_HSV_Value'>to</a> <a href='#Color_HSV_Value'>one</a>.

<a href='#Color_HSV_Value'>Out</a> <a href='#Color_HSV_Value'>of</a> <a href='#Color_HSV_Value'>range</a> <a href='#SkHSVToColor_hsv'>hsv</a> <a href='#SkHSVToColor_hsv'>values</a> <a href='#SkHSVToColor_hsv'>are</a> <a href='#SkHSVToColor_hsv'>pinned</a>.

### Parameters

<table>  <tr>    <td><a name='SkHSVToColor_alpha'><code><strong>alpha</strong></code></a></td>
    <td><a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>returned</a> <a href='SkColor_Reference#Alpha'>ARGB</a> <a href='SkColor_Reference#Color'>color </a>
</td>
  </tr>
  <tr>    <td><a name='SkHSVToColor_hsv'><code><strong>hsv</strong></code></a></td>
    <td>three element array which holds the input HSV components
</td>
  </tr>
</table>

### Return Value

ARGB equivalent to HSV

### Example

<div><fiddle-embed name="311a59931ac340b90f202cd6ac399a0a"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorToHSV'>SkColorToHSV</a> <a href='SkColor_Reference#SkRGBToHSV'>SkRGBToHSV</a>

<a name='SkHSVToColor_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkHSVToColor'>SkHSVToColor</a>(<a href='SkColor_Reference#SkHSVToColor'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>hsv</a>[3])
</pre>

Converts HSV components to an ARGB <a href='SkColor_Reference#Color'>color</a>. <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>set</a> <a href='SkColor_Reference#Alpha'>to</a> 255.
<a href='#SkHSVToColor_2_hsv'>hsv</a>[0] <a href='#SkHSVToColor_2_hsv'>represents</a> <a href='#Color_HSV_Hue'>HSV_Hue</a>, <a href='#Color_HSV_Hue'>an</a> <a href='#Color_HSV_Hue'>angle</a> <a href='#Color_HSV_Hue'>from</a> <a href='#Color_HSV_Hue'>zero</a> <a href='#Color_HSV_Hue'>to</a> <a href='#Color_HSV_Hue'>less</a> <a href='#Color_HSV_Hue'>than</a> 360.
<a href='#SkHSVToColor_2_hsv'>hsv</a>[1] <a href='#SkHSVToColor_2_hsv'>represents</a> <a href='#Color_HSV_Saturation'>HSV_Saturation</a>, <a href='#Color_HSV_Saturation'>and</a> <a href='#Color_HSV_Saturation'>varies</a> <a href='#Color_HSV_Saturation'>from</a> <a href='#Color_HSV_Saturation'>zero</a> <a href='#Color_HSV_Saturation'>to</a> <a href='#Color_HSV_Saturation'>one</a>.
<a href='#SkHSVToColor_2_hsv'>hsv</a>[2] <a href='#SkHSVToColor_2_hsv'>represents</a> <a href='#Color_HSV_Value'>HSV_Value</a>, <a href='#Color_HSV_Value'>and</a> <a href='#Color_HSV_Value'>varies</a> <a href='#Color_HSV_Value'>from</a> <a href='#Color_HSV_Value'>zero</a> <a href='#Color_HSV_Value'>to</a> <a href='#Color_HSV_Value'>one</a>.

<a href='#Color_HSV_Value'>Out</a> <a href='#Color_HSV_Value'>of</a> <a href='#Color_HSV_Value'>range</a> <a href='#SkHSVToColor_2_hsv'>hsv</a> <a href='#SkHSVToColor_2_hsv'>values</a> <a href='#SkHSVToColor_2_hsv'>are</a> <a href='#SkHSVToColor_2_hsv'>pinned</a>.

### Parameters

<table>  <tr>    <td><a name='SkHSVToColor_2_hsv'><code><strong>hsv</strong></code></a></td>
    <td>three element array which holds the input HSV components
</td>
  </tr>
</table>

### Return Value

RGB equivalent to HSV

### Example

<div><fiddle-embed name="d355a17547908cdbc2c38720974b5d11"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorToHSV'>SkColorToHSV</a> <a href='SkColor_Reference#SkRGBToHSV'>SkRGBToHSV</a>

<a name='PM_Color'></a>

<a name='SkPMColor'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
typedef uint32_t <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>;
</pre>

32-bit ARGB <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>value</a>, <a href='undocumented#Premultiply'>Premultiplied</a>. <a href='undocumented#Premultiply'>The</a> <a href='undocumented#Premultiply'>byte</a> <a href='undocumented#Premultiply'>order</a> <a href='undocumented#Premultiply'>for</a> <a href='undocumented#Premultiply'>this</a> <a href='undocumented#Premultiply'>value</a> <a href='undocumented#Premultiply'>is</a>
<a href='undocumented#Premultiply'>configuration</a> <a href='undocumented#Premultiply'>dependent</a>, <a href='undocumented#Premultiply'>matching</a> <a href='undocumented#Premultiply'>the</a> <a href='undocumented#Premultiply'>format</a> <a href='undocumented#Premultiply'>of</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a>.
<a href='SkBitmap_Reference#Bitmap'>This</a> <a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>different</a> <a href='SkBitmap_Reference#Bitmap'>from</a> <a href='SkColor_Reference#SkColor'>SkColor</a>, <a href='SkColor_Reference#SkColor'>which</a> <a href='SkColor_Reference#SkColor'>is</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a>, <a href='undocumented#Unpremultiply'>and</a> <a href='undocumented#Unpremultiply'>is</a> <a href='undocumented#Unpremultiply'>always</a> <a href='undocumented#Unpremultiply'>in</a> <a href='undocumented#Unpremultiply'>the</a>
<a href='undocumented#Unpremultiply'>same</a> <a href='undocumented#Unpremultiply'>byte</a> <a href='undocumented#Unpremultiply'>order</a>.

<a name='SkPreMultiplyARGB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkPMColor'>SkPMColor</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>a</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>r</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>g</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>b</a>)
</pre>

Returns <a href='#SkPreMultiplyARGB_a'>a</a> <a href='SkColor_Reference#SkPMColor'>SkPMColor</a> <a href='SkColor_Reference#SkPMColor'>value</a> <a href='SkColor_Reference#SkPMColor'>from</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a> 8-<a href='undocumented#Unpremultiply'>bit</a> <a href='undocumented#Unpremultiply'>component</a> <a href='undocumented#Unpremultiply'>values</a>.

### Parameters

<table>  <tr>    <td><a name='SkPreMultiplyARGB_a'><code><strong>a</strong></code></a></td>
    <td>amount of <a href='SkColor_Reference#Alpha'>Alpha</a>, <a href='SkColor_Reference#Alpha'>from</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>transparent</a> (0) <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a> (255)</td>
  </tr>
  <tr>    <td><a name='SkPreMultiplyARGB_r'><code><strong>r</strong></code></a></td>
    <td>amount of red, from no red (0) to full red (255)</td>
  </tr>
  <tr>    <td><a name='SkPreMultiplyARGB_g'><code><strong>g</strong></code></a></td>
    <td>amount of green, from no green (0) to full green (255)</td>
  </tr>
  <tr>    <td><a name='SkPreMultiplyARGB_b'><code><strong>b</strong></code></a></td>
    <td>amount of blue, from no blue (0) to full blue (255)</td>
  </tr>
</table>

### Return Value

<a href='undocumented#Premultiply'>Premultiplied</a> <a href='SkColor_Reference#Color'>Color</a>

### Example

<div><fiddle-embed name="756345484fd48ca0ea7b6cec350f73b8"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkPreMultiplyColor'>SkPreMultiplyColor</a>

<a name='SkPreMultiplyColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkPMColor'>SkPMColor</a> <a href='SkColor_Reference#SkPreMultiplyColor'>SkPreMultiplyColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>c</a>)
</pre>

Returns <a href='#Color_PM_Color'>PM_Color</a> <a href='#Color_PM_Color'>closest</a> <a href='#Color_PM_Color'>to</a> <a href='SkColor_Reference#Color'>Color</a> <a href='#SkPreMultiplyColor_c'>c</a>. <a href='#SkPreMultiplyColor_c'>Multiplies</a> <a href='#SkPreMultiplyColor_c'>c</a> <a href='#SkPreMultiplyColor_c'>RGB</a> <a href='#SkPreMultiplyColor_c'>components</a> <a href='#SkPreMultiplyColor_c'>by</a> <a href='#SkPreMultiplyColor_c'>the</a> <a href='#SkPreMultiplyColor_c'>c</a> <a href='SkColor_Reference#Alpha'>Alpha</a>,
<a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>arranges</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>bytes</a> <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>match</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>format</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>.

### Parameters

<table>  <tr>    <td><a name='SkPreMultiplyColor_c'><code><strong>c</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='undocumented#Unpremultiply'>ARGB</a> <a href='SkColor_Reference#Color'>Color</a></td>
  </tr>
</table>

### Return Value

<a href='undocumented#Premultiply'>Premultiplied</a> <a href='SkColor_Reference#Color'>Color</a>

### Example

<div><fiddle-embed name="0bcc0f86a2aefc899f3500503dce6968"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a>

