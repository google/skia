SkPaint Reference
===


<a name='SkPaint'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkPaint_Reference#SkPaint'>SkPaint</a> {

    <a href='#SkPaint_empty_constructor'>SkPaint()</a>;
    <a href='#SkPaint_copy_const_SkPaint'>SkPaint</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='#SkPaint_move_SkPaint'>SkPaint</a>(<a href='SkPaint_Reference#SkPaint'>SkPaint</a>&& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='#SkPaint_destructor'>~SkPaint()</a>;
    <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='#SkPaint_copy_operator'>operator=</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='#SkPaint_move_operator'>operator=</a>(<a href='SkPaint_Reference#SkPaint'>SkPaint</a>&& <a href='SkPaint_Reference#Paint'>paint</a>);
    friend bool <a href='#SkPaint_equal_operator'>operator==</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& a, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& b);
    friend bool <a href='#SkPaint_notequal_operator'>operator!=</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& a, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& b);
    uint32_t <a href='#SkPaint_getHash'>getHash</a>() const;
    void <a href='#SkPaint_reset'>reset()</a>;
    void <a href='#SkPaint_setHinting'>setHinting</a>(<a href='undocumented#SkFontHinting'>SkFontHinting</a> hintingLevel);
    <a href='undocumented#SkFontHinting'>SkFontHinting</a> <a href='#SkPaint_getHinting'>getHinting</a>() const;

    enum <a href='#SkPaint_Flags'>Flags</a> {
        <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> = 0x01,
        <a href='#SkPaint_kDither_Flag'>kDither_Flag</a> = 0x04,
        <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a> = 0x20,
        <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> = 0x40,
        <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> = 0x80,
        <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> = 0x200,
        <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> = 0x400,
        <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a> = 0x800,
        <a href='#SkPaint_kAllFlags'>kAllFlags</a> = 0xFFFF,
    };

    uint32_t <a href='#SkPaint_getFlags'>getFlags</a>() const;
    void <a href='#SkPaint_setFlags'>setFlags</a>(uint32_t flags);
    bool <a href='#SkPaint_isAntiAlias'>isAntiAlias</a>() const;
    void <a href='#SkPaint_setAntiAlias'>setAntiAlias</a>(bool aa);
    bool <a href='#SkPaint_isDither'>isDither</a>() const;
    void <a href='#SkPaint_setDither'>setDither</a>(bool dither);
    bool <a href='#SkPaint_isLinearText'>isLinearText</a>() const;
    void <a href='#SkPaint_setLinearText'>setLinearText</a>(bool linearText);
    bool <a href='#SkPaint_isSubpixelText'>isSubpixelText</a>() const;
    void <a href='#SkPaint_setSubpixelText'>setSubpixelText</a>(bool subpixelText);
    bool <a href='#SkPaint_isLCDRenderText'>isLCDRenderText</a>() const;
    void <a href='#SkPaint_setLCDRenderText'>setLCDRenderText</a>(bool lcdText);
    bool <a href='#SkPaint_isEmbeddedBitmapText'>isEmbeddedBitmapText</a>() const;
    void <a href='#SkPaint_setEmbeddedBitmapText'>setEmbeddedBitmapText</a>(bool useEmbeddedBitmapText);
    bool <a href='#SkPaint_isAutohinted'>isAutohinted</a>() const;
    void <a href='#SkPaint_setAutohinted'>setAutohinted</a>(bool useAutohinter);
    bool <a href='#SkPaint_isFakeBoldText'>isFakeBoldText</a>() const;
    void <a href='#SkPaint_setFakeBoldText'>setFakeBoldText</a>(bool fakeBoldText);
    <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='#SkPaint_getFilterQuality'>getFilterQuality</a>() const;
    void <a href='#SkPaint_setFilterQuality'>setFilterQuality</a>(<a href='undocumented#SkFilterQuality'>SkFilterQuality</a> quality);

    enum <a href='#SkPaint_Style'>Style</a> : uint8_t {
        <a href='#SkPaint_kFill_Style'>kFill_Style</a>,
        <a href='#SkPaint_kStroke_Style'>kStroke_Style</a>,
        <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>,
    };

    static constexpr int <a href='#SkPaint_kStyleCount'>kStyleCount</a> = <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a> + 1
    <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_getStyle'>getStyle</a>() const;
    void <a href='#SkPaint_setStyle'>setStyle</a>(<a href='#SkPaint_Style'>Style</a> style);
    <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPaint_getColor'>getColor</a>() const;
    <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a> <a href='#SkPaint_getColor4f'>getColor4f</a>() const;
    void <a href='#SkPaint_setColor'>setColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>);
    void <a href='#SkPaint_setColor4f'>setColor4f</a>(const <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>& <a href='SkColor_Reference#Color'>color</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* colorSpace);
    uint8_t <a href='#SkPaint_getAlpha'>getAlpha</a>() const;
    void <a href='#SkPaint_setAlpha'>setAlpha</a>(<a href='undocumented#U8CPU'>U8CPU</a> a);
    void <a href='#SkPaint_setARGB'>setARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> a, <a href='undocumented#U8CPU'>U8CPU</a> r, <a href='undocumented#U8CPU'>U8CPU</a> g, <a href='undocumented#U8CPU'>U8CPU</a> b);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getStrokeWidth'>getStrokeWidth</a>() const;
    void <a href='#SkPaint_setStrokeWidth'>setStrokeWidth</a>(<a href='undocumented#SkScalar'>SkScalar</a> width);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getStrokeMiter'>getStrokeMiter</a>() const;
    void <a href='#SkPaint_setStrokeMiter'>setStrokeMiter</a>(<a href='undocumented#SkScalar'>SkScalar</a> miter);

    enum <a href='#SkPaint_Cap'>Cap</a> {
        <a href='#SkPaint_kButt_Cap'>kButt_Cap</a>,
        <a href='#SkPaint_kRound_Cap'>kRound_Cap</a>,
        <a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>,
        <a href='#SkPaint_kLast_Cap'>kLast_Cap</a> = <a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>,
        <a href='#SkPaint_kDefault_Cap'>kDefault_Cap</a> = <a href='#SkPaint_kButt_Cap'>kButt_Cap</a>,
    };

    static constexpr int <a href='#SkPaint_kCapCount'>kCapCount</a> = <a href='#SkPaint_kLast_Cap'>kLast_Cap</a> + 1
    enum <a href='#SkPaint_Join'>Join</a> : uint8_t {
        <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>,
        <a href='#SkPaint_kRound_Join'>kRound_Join</a>,
        <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>,
        <a href='#SkPaint_kLast_Join'>kLast_Join</a> = <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>,
        <a href='#SkPaint_kDefault_Join'>kDefault_Join</a> = <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>,
    };

    static constexpr int <a href='#SkPaint_kJoinCount'>kJoinCount</a> = <a href='#SkPaint_kLast_Join'>kLast_Join</a> + 1
    <a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_getStrokeCap'>getStrokeCap</a>() const;
    void <a href='#SkPaint_setStrokeCap'>setStrokeCap</a>(<a href='#SkPaint_Cap'>Cap</a> cap);
    <a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a>() const;
    void <a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a>(<a href='#SkPaint_Join'>Join</a> join);
    bool <a href='#SkPaint_getFillPath'>getFillPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& src, <a href='SkPath_Reference#SkPath'>SkPath</a>* dst, const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect,
                     <a href='undocumented#SkScalar'>SkScalar</a> resScale = 1) const;
    bool <a href='#SkPaint_getFillPath'>getFillPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& src, <a href='SkPath_Reference#SkPath'>SkPath</a>* dst) const;
    <a href='undocumented#SkShader'>SkShader</a>* <a href='#SkPaint_getShader'>getShader</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkShader'>SkShader</a>> <a href='#SkPaint_refShader'>refShader</a>() const;
    void <a href='#SkPaint_setShader'>setShader</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkShader'>SkShader</a>> <a href='undocumented#Shader'>shader</a>);
    <a href='undocumented#SkColorFilter'>SkColorFilter</a>* <a href='#SkPaint_getColorFilter'>getColorFilter</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorFilter'>SkColorFilter</a>> <a href='#SkPaint_refColorFilter'>refColorFilter</a>() const;
    void <a href='#SkPaint_setColorFilter'>setColorFilter</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorFilter'>SkColorFilter</a>> colorFilter);
    <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkPaint_getBlendMode'>getBlendMode</a>() const;
    bool <a href='#SkPaint_isSrcOver'>isSrcOver</a>() const;
    void <a href='#SkPaint_setBlendMode'>setBlendMode</a>(<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode);
    <a href='undocumented#SkPathEffect'>SkPathEffect</a>* <a href='#SkPaint_getPathEffect'>getPathEffect</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkPathEffect'>SkPathEffect</a>> <a href='#SkPaint_refPathEffect'>refPathEffect</a>() const;
    void <a href='#SkPaint_setPathEffect'>setPathEffect</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkPathEffect'>SkPathEffect</a>> pathEffect);
    <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>* <a href='#SkPaint_getMaskFilter'>getMaskFilter</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>> <a href='#SkPaint_refMaskFilter'>refMaskFilter</a>() const;
    void <a href='#SkPaint_setMaskFilter'>setMaskFilter</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>> maskFilter);
    <a href='undocumented#SkTypeface'>SkTypeface</a>* <a href='#SkPaint_getTypeface'>getTypeface</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkTypeface'>SkTypeface</a>> <a href='#SkPaint_refTypeface'>refTypeface</a>() const;
    void <a href='#SkPaint_setTypeface'>setTypeface</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkTypeface'>SkTypeface</a>> <a href='undocumented#Typeface'>typeface</a>);
    <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='#SkPaint_getImageFilter'>getImageFilter</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkImageFilter'>SkImageFilter</a>> <a href='#SkPaint_refImageFilter'>refImageFilter</a>() const;
    void <a href='#SkPaint_setImageFilter'>setImageFilter</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkImageFilter'>SkImageFilter</a>> imageFilter);
    <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>* <a href='#SkPaint_getDrawLooper'>getDrawLooper</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>> <a href='#SkPaint_refDrawLooper'>refDrawLooper</a>() const;
    void <a href='#SkPaint_setDrawLooper'>setDrawLooper</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>> drawLooper);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSize'>getTextSize</a>() const;
    void <a href='#SkPaint_setTextSize'>setTextSize</a>(<a href='undocumented#SkScalar'>SkScalar</a> textSize);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextScaleX'>getTextScaleX</a>() const;
    void <a href='#SkPaint_setTextScaleX'>setTextScaleX</a>(<a href='undocumented#SkScalar'>SkScalar</a> scaleX);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSkewX'>getTextSkewX</a>() const;
    void <a href='#SkPaint_setTextSkewX'>setTextSkewX</a>(<a href='undocumented#SkScalar'>SkScalar</a> skewX);
    <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> <a href='#SkPaint_getTextEncoding'>getTextEncoding</a>() const;
    void <a href='#SkPaint_setTextEncoding'>setTextEncoding</a>(<a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding);

    typedef <a href='undocumented#SkFontMetrics'>SkFontMetrics</a> <a href='#SkPaint_FontMetrics'>FontMetrics</a>;

    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getFontMetrics'>getFontMetrics</a>(<a href='undocumented#SkFontMetrics'>SkFontMetrics</a>* metrics) const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getFontSpacing'>getFontSpacing</a>() const;
    int <a href='#SkPaint_textToGlyphs'>textToGlyphs</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength,
                     <a href='undocumented#SkGlyphID'>SkGlyphID</a> <a href='undocumented#Glyph'>glyphs</a>[]) const;
    bool <a href='#SkPaint_containsText'>containsText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength) const;
    void <a href='#SkPaint_glyphsToUnichars'>glyphsToUnichars</a>(const <a href='undocumented#SkGlyphID'>SkGlyphID</a> <a href='undocumented#Glyph'>glyphs</a>[], int count, <a href='undocumented#SkUnichar'>SkUnichar</a> <a href='undocumented#Text'>text</a>[]) const;
    int <a href='#SkPaint_countText'>countText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength) const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_measureText'>measureText</a>(const void* <a href='undocumented#Text'>text</a>, size_t length, <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds) const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_measureText'>measureText</a>(const void* <a href='undocumented#Text'>text</a>, size_t length) const;
    int <a href='#SkPaint_getTextWidths'>getTextWidths</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, <a href='undocumented#SkScalar'>SkScalar</a> widths[],
                      <a href='SkRect_Reference#SkRect'>SkRect</a> bounds[] = nullptr) const;
    void <a href='#SkPaint_getTextPath'>getTextPath</a>(const void* <a href='undocumented#Text'>text</a>, size_t length, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y,
                     <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#Path'>path</a>) const;
    void <a href='#SkPaint_getPosTextPath'>getPosTextPath</a>(const void* <a href='undocumented#Text'>text</a>, size_t length,
                        const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pos[], <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#Path'>path</a>) const;
    bool <a href='#SkPaint_nothingToDraw'>nothingToDraw</a>() const;
};

</pre>

<a href='SkPaint_Reference#Paint'>Paint</a> controls options applied when drawing and measuring. <a href='SkPaint_Reference#Paint'>Paint</a> collects all
options outside of the <a href='#Canvas_Clip'>Canvas_Clip</a> and <a href='#Canvas_Matrix'>Canvas_Matrix</a>.

Various options apply to <a href='undocumented#Text'>text</a>, strokes and fills, and images.

Some options may not be implemented on all platforms; in these cases, setting
the option has no effect. Some options are conveniences that duplicate <a href='SkCanvas_Reference#Canvas'>Canvas</a>
functionality; for instance,  <a href='#Text_Size'>text size</a> is identical to <a href='SkMatrix_Reference#Matrix'>matrix</a> scale.

<a href='SkPaint_Reference#Paint'>Paint</a> options are rarely exclusive; each option modifies a stage of the drawing
pipeline and multiple pipeline stages may be affected by a single <a href='SkPaint_Reference#Paint'>Paint</a>.

<a href='SkPaint_Reference#Paint'>Paint</a> collects effects and filters that describe single-pass and multiple-pass
algorithms that alter the drawing geometry, <a href='SkColor_Reference#Color'>color</a>, and transparency. For instance,
<a href='SkPaint_Reference#Paint'>Paint</a> does not directly implement dashing or blur, but contains the objects that do so.

The objects contained by <a href='SkPaint_Reference#Paint'>Paint</a> are opaque, and cannot be edited outside of the <a href='SkPaint_Reference#Paint'>Paint</a>
to affect it. The implementation is free to defer computations associated with the
<a href='SkPaint_Reference#Paint'>Paint</a>, or ignore them altogether. For instance, some GPU implementations draw all
<a href='SkPath_Reference#Path'>Path</a> geometries with <a href='#Paint_Anti_Alias'>Anti_Aliasing</a>, regardless of how <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a>
is set in <a href='SkPaint_Reference#Paint'>Paint</a>.

<a href='SkPaint_Reference#Paint'>Paint</a> describes a single <a href='SkColor_Reference#Color'>color</a>, a single <a href='SkFont_Reference#Font'>font</a>, a single <a href='SkImage_Reference#Image'>image</a> quality, and so on.
Multiple colors are drawn either by using multiple paints or with objects like
<a href='undocumented#Shader'>Shader</a> attached to <a href='SkPaint_Reference#Paint'>Paint</a>.

<a name='SkPaint_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_empty_constructor'>SkPaint()</a>
</pre>

Constructs <a href='SkPaint_Reference#Paint'>Paint</a> with default values.

| attribute | default value |
| --- | ---  |
| <a href='#Paint_Anti_Alias'>Anti_Alias</a> | false |
| <a href='#Blend_Mode'>Blend_Mode</a> | <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a> |
| <a href='SkColor_Reference#Color'>Color</a> | <a href='SkColor_Reference#SK_ColorBLACK'>SK_ColorBLACK</a> |
| <a href='#Color_Alpha'>Color_Alpha</a> | 255 |
| <a href='#Color_Filter'>Color_Filter</a> | nullptr |
| Dither | false |
| <a href='#Draw_Looper'>Draw_Looper</a> | nullptr |
| <a href='#Filter_Quality'>Filter_Quality</a> | <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a> |
| <a href='#Font_Force_Hinting'>Font_Force_Hinting</a> | false |
| <a href='#Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a> | false |
| <a href='#Font_Embolden'>Font_Embolden</a> | false |
| <a href='#Font_Hinting'>Font_Hinting</a> | <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNormal'>kNormal</a> |
| <a href='#Font_Hinting_Spacing'>Font_Hinting_Spacing</a> | false |
| <a href='#Font_Anti_Alias'>Font_Anti_Alias</a> | false |
| <a href='#Font_Linear'>Font_Linear</a> | false |
| <a href='#Font_Scale_X'>Font_Scale_X</a> | 1 |
| <a href='#Font_Size'>Font_Size</a> | 12 |
| <a href='#Font_Skew_X'>Font_Skew_X</a> | 0 |
| <a href='#Font_Subpixel'>Font_Subpixel</a> | false |
| <a href='#Image_Filter'>Image_Filter</a> | nullptr |
| <a href='#Paint_Miter_Limit'>Miter_Limit</a> | 4 |
| <a href='#Mask_Filter'>Mask_Filter</a> | nullptr |
| <a href='#Path_Effect'>Path_Effect</a> | nullptr |
| <a href='undocumented#Shader'>Shader</a> | nullptr |
| <a href='#SkPaint_Style'>Style</a> | <a href='#SkPaint_kFill_Style'>kFill_Style</a> |
| <a href='#Text_Encoding'>Text_Encoding</a> | <a href='undocumented#SkTextEncoding::kUTF8'>SkTextEncoding::kUTF8</a> |
| <a href='undocumented#Typeface'>Typeface</a> | nullptr |
| <a href='#Paint_Stroke_Cap'>Stroke_Cap</a> | <a href='#SkPaint_kButt_Cap'>kButt_Cap</a> |
| <a href='#Paint_Stroke_Join'>Stroke_Join</a> | <a href='#SkPaint_kMiter_Join'>kMiter_Join</a> |
| <a href='#Paint_Stroke_Width'>Stroke_Width</a> | 0 |

The flags, <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, hinting, and miter limit may be overridden at compile time by defining
<a href='SkPaint_Reference#Paint'>paint</a> default values. The overrides may be included in "SkUserConfig.h" or predefined by the
build system.

### Return Value

default initialized <a href='SkPaint_Reference#Paint'>Paint</a>

### Example

<div><fiddle-embed name="@Paint_empty_constructor"></fiddle-embed></div>

<a name='SkPaint_copy_const_SkPaint'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_copy_const_SkPaint'>SkPaint</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Makes a shallow copy of <a href='SkPaint_Reference#SkPaint'>SkPaint</a>. <a href='undocumented#SkTypeface'>SkTypeface</a>, <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>, and <a href='undocumented#SkImageFilter'>SkImageFilter</a> are shared
between the original <a href='#SkPaint_copy_const_SkPaint_paint'>paint</a> and the copy. Objects containing <a href='undocumented#SkRefCnt'>SkRefCnt</a> increment
their references by one.

The referenced objects <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>, and <a href='undocumented#SkImageFilter'>SkImageFilter</a> cannot be modified after they are created.
This prevents objects with <a href='undocumented#SkRefCnt'>SkRefCnt</a> from being modified once <a href='SkPaint_Reference#SkPaint'>SkPaint</a> refers to them.

### Parameters

<table>  <tr>    <td><a name='SkPaint_copy_const_SkPaint_paint'><code><strong>paint</strong></code></a></td>
    <td>original to copy</td>
  </tr>
</table>

### Return Value

shallow copy of <a href='#SkPaint_copy_const_SkPaint_paint'>paint</a>

### Example

<div><fiddle-embed name="@Paint_copy_const_SkPaint">

#### Example Output

~~~~
SK_ColorRED == paint1.getColor()
SK_ColorBLUE == paint2.getColor()
~~~~

</fiddle-embed></div>

<a name='SkPaint_move_SkPaint'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_move_SkPaint'>SkPaint</a>(<a href='SkPaint_Reference#SkPaint'>SkPaint</a>&& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Implements a move constructor to avoid increasing the reference counts
of objects referenced by the <a href='#SkPaint_move_SkPaint_paint'>paint</a>.

After the call, <a href='#SkPaint_move_SkPaint_paint'>paint</a> is undefined, and can be safely destructed.

### Parameters

<table>  <tr>    <td><a name='SkPaint_move_SkPaint_paint'><code><strong>paint</strong></code></a></td>
    <td>original to move</td>
  </tr>
</table>

### Return Value

content of <a href='#SkPaint_move_SkPaint_paint'>paint</a>

### Example

<div><fiddle-embed name="@Paint_move_SkPaint">

#### Example Output

~~~~
path effect unique: true
~~~~

</fiddle-embed></div>

<a name='SkPaint_reset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_reset'>reset()</a>
</pre>

Sets all <a href='SkPaint_Reference#SkPaint'>SkPaint</a> contents to their initial values. This is equivalent to replacing
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> with the result of <a href='#SkPaint_empty_constructor'>SkPaint()</a>.

### Example

<div><fiddle-embed name="@Paint_reset">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

<a name='SkPaint_destructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_destructor'>~SkPaint()</a>
</pre>

Decreases <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> of owned objects: <a href='undocumented#SkTypeface'>SkTypeface</a>, <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>, and <a href='undocumented#SkImageFilter'>SkImageFilter</a>. If the
objects containing <a href='undocumented#SkRefCnt'>SkRefCnt</a> go to zero, they are deleted.

<a name='Management'></a>

<a name='SkPaint_copy_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='#SkPaint_copy_operator'>operator=</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Makes a shallow copy of <a href='SkPaint_Reference#SkPaint'>SkPaint</a>. <a href='undocumented#SkTypeface'>SkTypeface</a>, <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>, and <a href='undocumented#SkImageFilter'>SkImageFilter</a> are shared
between the original <a href='#SkPaint_copy_operator_paint'>paint</a> and the copy. Objects containing <a href='undocumented#SkRefCnt'>SkRefCnt</a> in the
prior destination are decreased by one, and the referenced objects are deleted if the
resulting count is zero. Objects containing <a href='undocumented#SkRefCnt'>SkRefCnt</a> in the parameter <a href='#SkPaint_copy_operator_paint'>paint</a>
are increased by one. <a href='#SkPaint_copy_operator_paint'>paint</a> is unmodified.

### Parameters

<table>  <tr>    <td><a name='SkPaint_copy_operator_paint'><code><strong>paint</strong></code></a></td>
    <td>original to copy</td>
  </tr>
</table>

### Return Value

content of <a href='#SkPaint_copy_operator_paint'>paint</a>

### Example

<div><fiddle-embed name="@Paint_copy_operator">

#### Example Output

~~~~
SK_ColorRED == paint1.getColor()
SK_ColorRED == paint2.getColor()
~~~~

</fiddle-embed></div>

<a name='SkPaint_move_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='#SkPaint_move_operator'>operator=</a>(<a href='SkPaint_Reference#SkPaint'>SkPaint</a>&& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Moves the <a href='#SkPaint_move_operator_paint'>paint</a> to avoid increasing the reference counts
of objects referenced by the <a href='#SkPaint_move_operator_paint'>paint</a> parameter. Objects containing <a href='undocumented#SkRefCnt'>SkRefCnt</a> in the
prior destination are decreased by one; those objects are deleted if the resulting count
is zero.

After the call, <a href='#SkPaint_move_operator_paint'>paint</a> is undefined, and can be safely destructed.

### Parameters

<table>  <tr>    <td><a name='SkPaint_move_operator_paint'><code><strong>paint</strong></code></a></td>
    <td>original to move</td>
  </tr>
</table>

### Return Value

content of <a href='#SkPaint_move_operator_paint'>paint</a>

### Example

<div><fiddle-embed name="@Paint_move_operator">

#### Example Output

~~~~
SK_ColorRED == paint2.getColor()
~~~~

</fiddle-embed></div>

<a name='SkPaint_equal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_equal_operator'>operator==</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& a, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& b)
</pre>

Compares <a href='#SkPaint_equal_operator_a'>a</a> and <a href='#SkPaint_equal_operator_b'>b</a>, and returns true if <a href='#SkPaint_equal_operator_a'>a</a> and <a href='#SkPaint_equal_operator_b'>b</a> are equivalent. May return false
if <a href='undocumented#SkTypeface'>SkTypeface</a>, <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>, or <a href='undocumented#SkImageFilter'>SkImageFilter</a> have identical contents but different pointers.

### Parameters

<table>  <tr>    <td><a name='SkPaint_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkPaint_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='SkPaint_Reference#SkPaint'>SkPaint</a> pair are equivalent

### Example

<div><fiddle-embed name="@Paint_equal_operator">

#### Example Output

~~~~
paint1 == paint2
paint1 != paint2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_notequal_operator'>operator!=</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='#SkPaint_equal_operator_a'>a</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='#SkPaint_equal_operator_b'>b</a>)

<a name='SkPaint_notequal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_notequal_operator'>operator!=</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& a, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& b)
</pre>

Compares <a href='#SkPaint_notequal_operator_a'>a</a> and <a href='#SkPaint_notequal_operator_b'>b</a>, and returns true if <a href='#SkPaint_notequal_operator_a'>a</a> and <a href='#SkPaint_notequal_operator_b'>b</a> are not equivalent. May return true
if <a href='undocumented#SkTypeface'>SkTypeface</a>, <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>, or <a href='undocumented#SkImageFilter'>SkImageFilter</a> have identical contents but different pointers.

### Parameters

<table>  <tr>    <td><a name='SkPaint_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkPaint_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='SkPaint_Reference#SkPaint'>SkPaint</a> pair are not equivalent

### Example

<div><fiddle-embed name="@Paint_notequal_operator">

#### Example Output

~~~~
paint1 == paint2
paint1 == paint2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_equal_operator'>operator==</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='#SkPaint_notequal_operator_a'>a</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='#SkPaint_notequal_operator_b'>b</a>)

<a name='SkPaint_getHash'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPaint_getHash'>getHash</a>()const
</pre>

Returns a hash generated from <a href='SkPaint_Reference#SkPaint'>SkPaint</a> values and pointers.
Identical hashes guarantee that the paints are
equivalent, but differing hashes do not guarantee that the paints have differing
contents.

If <a href='#SkPaint_equal_operator'>operator==</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& a, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& b) returns true for two paints,
their hashes are also equal.

The hash returned is platform and implementation specific.

### Return Value

a shallow hash

### Example

<div><fiddle-embed name="@Paint_getHash">

#### Example Output

~~~~
paint1 == paint2
paint1.getHash() == paint2.getHash()
~~~~

</fiddle-embed></div>

<a name='Hinting'></a>

<a name='SkPaint_setHinting'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setHinting'>setHinting</a>(<a href='undocumented#SkFontHinting'>SkFontHinting</a> hintingLevel)
</pre>

Sets level of <a href='undocumented#Glyph'>glyph</a> outline adjustment.
Does not check for valid values of <a href='#SkPaint_setHinting_hintingLevel'>hintingLevel</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setHinting_hintingLevel'><code><strong>hintingLevel</strong></code></a></td>
    <td>one of: <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNone'>kNone</a>, <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kSlight'>kSlight</a>,</td>
  </tr>
</table>

<a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNormal'>kNormal</a>, <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kFull'>kFull</a>

### Example

<div><fiddle-embed name="bb179ec5698ec1398ff18f3657ab73f7">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

<a name='SkPaint_getHinting'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkFontHinting'>SkFontHinting</a> <a href='#SkPaint_getHinting'>getHinting</a>()const
</pre>

Returns level of <a href='undocumented#Glyph'>glyph</a> outline adjustment.

### Return Value

one of: <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNone'>kNone</a>, <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kSlight'>kSlight</a>, <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNormal'>kNormal</a>,

<a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kFull'>kFull</a>

### Example

<div><fiddle-embed name="b56b70c7ea2453c41bfa58b626953bed">

#### Example Output

~~~~
SkFontHinting::kNormal == paint.getHinting()
~~~~

</fiddle-embed></div>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kAntiAlias_Flag'><code>SkPaint::kAntiAlias_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0001</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask for setting Anti_Alias</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kDither_Flag'><code>SkPaint::kDither_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0004</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask for setting Dither</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kFakeBoldText_Flag'><code>SkPaint::kFakeBoldText_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0020</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask for setting Font_Embolden</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kLinearText_Flag'><code>SkPaint::kLinearText_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0040</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask for setting Font_Linear</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kSubpixelText_Flag'><code>SkPaint::kSubpixelText_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0080</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask for setting Font_Subpixel</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kLCDRenderText_Flag'><code>SkPaint::kLCDRenderText_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0200</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask for setting Font_Anti_Alias</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kEmbeddedBitmapText_Flag'><code>SkPaint::kEmbeddedBitmapText_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0400</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask for setting Font_Embedded_Bitmaps</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kAutoHinting_Flag'><code>SkPaint::kAutoHinting_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0800</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask for setting Font_Force_Hinting</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kAllFlags'><code>SkPaint::kAllFlags</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask of all <a href='#SkPaint_Flags'>Flags</a>, including private flags and flags reserved for future use
</td>
  </tr>
<a href='#SkPaint_Flags'>Flags</a> default to all flags clear, disabling the associated feature.
</table>

<a name='SkPaint_getFlags'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPaint_getFlags'>getFlags</a>()const
</pre>

Returns <a href='SkPaint_Reference#Paint'>paint</a> settings described by <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a>. Each setting uses one
bit, and can be tested with <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a> members.

### Return Value

zero, one, or more bits described by <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a>

### Example

<div><fiddle-embed name="8a3f8c309533388b01aa66e1267f322d">

#### Example Output

~~~~
(SkPaint::kAntiAlias_Flag & paint.getFlags()) != 0
~~~~

</fiddle-embed></div>

<a name='SkPaint_setFlags'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setFlags'>setFlags</a>(uint32_t flags)
</pre>

Replaces <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a> with <a href='#SkPaint_setFlags_flags'>flags</a>, the union of the <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a> members.
All <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a> members may be cleared, or one or more may be set.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setFlags_flags'><code><strong>flags</strong></code></a></td>
    <td>union of <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a> for <a href='SkPaint_Reference#SkPaint'>SkPaint</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="54baed3f6bc4b9c31ba664e27767fdc7">

#### Example Output

~~~~
paint.isAntiAlias()
paint.isDither()
~~~~

</fiddle-embed></div>

<a name='Anti_Alias'></a>

---

<a href='#Paint_Anti_Alias'>Anti_Alias</a> drawing approximates partial <a href='undocumented#Pixel'>pixel</a> coverage with transparency.
If <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> is clear, <a href='undocumented#Pixel'>pixel</a> centers contained by the shape edge are drawn opaque.
If <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> is set, pixels are drawn with <a href='#Color_Alpha'>Color_Alpha</a> equal to their coverage.

The rule for <a href='undocumented#Alias'>Aliased</a> pixels is inconsistent across platforms. A shape edge
passing through the <a href='undocumented#Pixel'>pixel</a> center may, but is not required to, draw the <a href='undocumented#Pixel'>pixel</a>.

<a href='#Raster_Engine'>Raster_Engine</a> draws <a href='undocumented#Alias'>Aliased</a> pixels whose centers are on or to the right of the start of an
active <a href='SkPath_Reference#Path'>Path</a> edge, and whose center is to the left of the end of the active <a href='SkPath_Reference#Path'>Path</a> edge.

A platform may only support <a href='#Paint_Anti_Alias'>Anti_Aliased</a> drawing. Some GPU-backed platforms use
<a href='undocumented#Supersampling'>Supersampling</a> to <a href='#Paint_Anti_Alias'>Anti_Alias</a> all drawing, and have no mechanism to selectively
<a href='undocumented#Alias'>Alias</a>.

The amount of coverage computed for <a href='#Paint_Anti_Alias'>Anti_Aliased</a> pixels also varies across platforms.

<a href='#Paint_Anti_Alias'>Anti_Alias</a> is disabled by default.
<a href='#Paint_Anti_Alias'>Anti_Alias</a> can be enabled by default by setting <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults_Flags</a> to <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a>
at compile time.

### Example

<div><fiddle-embed name="@Anti_Alias"><div>A red <a href='undocumented#Line'>line</a> is drawn with transparency on the edges to make it look smoother.
A blue <a href='undocumented#Line'>line</a> draws only where the <a href='undocumented#Pixel'>pixel</a> centers are contained.
The <a href='undocumented#Line'>lines</a> are drawn into <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>, then drawn magnified to make the
<a href='undocumented#Alias'>Aliasing</a> easier to see.
</div></fiddle-embed></div>

<a name='SkPaint_isAntiAlias'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isAntiAlias'>isAntiAlias</a>()const
</pre>

Returns true if pixels on the active edges of <a href='SkPath_Reference#SkPath'>SkPath</a> may be drawn with partial transparency.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() masked with <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a>.

### Return Value

<a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> state

### Example

<div><fiddle-embed name="d7d5f4f7da7acd5104a652f490c6f7b8">

#### Example Output

~~~~
paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)
paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)
~~~~

</fiddle-embed></div>

<a name='SkPaint_setAntiAlias'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setAntiAlias'>setAntiAlias</a>(bool aa)
</pre>

Requests, but does not require, that edge pixels draw opaque or with
partial transparency.

Sets <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> if <a href='#SkPaint_setAntiAlias_aa'>aa</a> is true.
Clears <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> if <a href='#SkPaint_setAntiAlias_aa'>aa</a> is false.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setAntiAlias_aa'><code><strong>aa</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c2ff148374d01cbef845b223e725905c">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

<a name='Dither'></a>

---

Dither increases fidelity by adjusting the <a href='SkColor_Reference#Color'>color</a> of adjacent pixels.
This can help to smooth <a href='SkColor_Reference#Color'>color</a> transitions and reducing banding in gradients.
Dithering lessens visible banding from <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>
and <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> gradients,
and improves rendering into a <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

Dithering is always enabled for linear gradients drawing into
<a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a> and <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.
Dither cannot be enabled for <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a> and
<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

Dither is disabled by default.
Dither can be enabled by default by setting <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults_Flags</a> to <a href='#SkPaint_kDither_Flag'>kDither_Flag</a>
at compile time.

Some platform implementations may ignore dithering. Set <code>SK_IGNORE_GPU_DITHER</code>to ignore Dither on <a href='#GPU_Surface'>GPU_Surface</a>.

### Example

<div><fiddle-embed name="@Dither_a"><div>Dithering in the bottom half more closely approximates the requested <a href='SkColor_Reference#Color'>color</a> by
alternating nearby colors from <a href='undocumented#Pixel'>pixel</a> to <a href='undocumented#Pixel'>pixel</a>.
</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="@Dither_b"><div>Dithering introduces subtle adjustments to <a href='SkColor_Reference#Color'>color</a> to smooth gradients.
Drawing the gradient repeatedly with <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kPlus'>kPlus</a> exaggerates the
dither, making it easier to see.
</div></fiddle-embed></div>

### See Also

Gradient <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>

<a name='SkPaint_isDither'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isDither'>isDither</a>()const
</pre>

Returns true if <a href='SkColor_Reference#Color'>color</a> error may be distributed to smooth <a href='SkColor_Reference#Color'>color</a> transition.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() masked with <a href='#SkPaint_kDither_Flag'>kDither_Flag</a>.

### Return Value

<a href='#SkPaint_kDither_Flag'>kDither_Flag</a> state

### Example

<div><fiddle-embed name="f4ce93f6c5e7335436a985377fd980c0">

#### Example Output

~~~~
paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag)
paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag)
~~~~

</fiddle-embed></div>

<a name='SkPaint_setDither'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setDither'>setDither</a>(bool dither)
</pre>

Requests, but does not require, to distribute <a href='SkColor_Reference#Color'>color</a> error.

Sets <a href='#SkPaint_kDither_Flag'>kDither_Flag</a> if <a href='#SkPaint_setDither_dither'>dither</a> is true.
Clears <a href='#SkPaint_kDither_Flag'>kDither_Flag</a> if <a href='#SkPaint_setDither_dither'>dither</a> is false.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setDither_dither'><code><strong>dither</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kDither_Flag'>kDither_Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="69b7162e8324d9239dd02dd9ada2bdff">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>

<a name='Device_Text'></a>

---

<a href='#Font_Anti_Alias'>Font_Anti_Alias</a> and <a href='#Font_Subpixel'>Font_Subpixel</a> increase the precision of <a href='undocumented#Glyph'>glyph</a> position.

When set, <a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> takes advantage of the organization of RGB stripes that
create a <a href='SkColor_Reference#Color'>color</a>, and relies
on the small <a href='undocumented#Size'>size</a> of the stripe and visual perception to make the <a href='SkColor_Reference#Color'>color</a> fringing imperceptible.
<a href='#Font_Anti_Alias'>Font_Anti_Alias</a> can be enabled on devices that orient stripes horizontally or vertically, and that order
the <a href='SkColor_Reference#Color'>color</a> components as RGB or BGR.

<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> uses the <a href='undocumented#Pixel'>pixel</a> transparency to represent a fractional offset.
As the opaqueness
of the <a href='SkColor_Reference#Color'>color</a> increases, the edge of the <a href='undocumented#Glyph'>glyph</a> appears to move towards the outside of the <a href='undocumented#Pixel'>pixel</a>.

Either or both techniques can be enabled.
<a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> and <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> are clear by default.
<a href='#Font_Anti_Alias'>Font_Anti_Alias</a> or <a href='#Font_Subpixel'>Font_Subpixel</a> can be enabled by default by setting <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults_Flags</a> to
<a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> or <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> (or both) at compile time.

### Example

<div><fiddle-embed name="4606ae1be792d6bc46d496432f050ee9"><div>Four commas are drawn normally and with combinations of <a href='#Font_Anti_Alias'>Font_Anti_Alias</a> and <a href='#Font_Subpixel'>Font_Subpixel</a>.
When <a href='#Font_Subpixel'>Font_Subpixel</a> is disabled, the comma <a href='undocumented#Glyph'>Glyphs</a> are identical, but not evenly spaced.
When <a href='#Font_Subpixel'>Font_Subpixel</a> is enabled, the comma <a href='undocumented#Glyph'>Glyphs</a> are unique, but appear evenly spaced.
</div></fiddle-embed></div>

<a name='Linear_Text'></a>

<a href='#Font_Linear'>Font_Linear</a> selects whether <a href='undocumented#Text'>text</a> is rendered as a <a href='undocumented#Glyph'>Glyph</a> or as a <a href='SkPath_Reference#Path'>Path</a>.
If <a href='#Font_Linear'>Font_Linear</a> is set, it has the same effect as setting Hinting to <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNormal'>kNormal</a>.
If <a href='#Font_Linear'>Font_Linear</a> is clear, it is the same as setting Hinting to <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNone'>kNone</a>.

<a name='SkPaint_isLinearText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isLinearText'>isLinearText</a>()const
</pre>

Returns true if <a href='undocumented#Text'>text</a> is converted to <a href='SkPath_Reference#SkPath'>SkPath</a> before drawing and measuring.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() masked with <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a>.

### Return Value

<a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> state

### Example

<div><fiddle-embed name="2890ad644f980637837e6fcb386fb462"></fiddle-embed></div>

### See Also

<a href='#SkPaint_setLinearText'>setLinearText</a> Hinting

<a name='SkPaint_setLinearText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setLinearText'>setLinearText</a>(bool linearText)
</pre>

Requests, but does not require, that <a href='undocumented#Glyph'>glyphs</a> are converted to <a href='SkPath_Reference#SkPath'>SkPath</a>
before drawing and measuring.
By default, <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> is clear.

Sets <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> if <a href='#SkPaint_setLinearText_linearText'>linearText</a> is true.
Clears <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> if <a href='#SkPaint_setLinearText_linearText'>linearText</a> is false.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setLinearText_linearText'><code><strong>linearText</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c93bb912f3bddfb4d96d3ad70ada552b"></fiddle-embed></div>

### See Also

<a href='#SkPaint_isLinearText'>isLinearText</a> Hinting

<a name='Subpixel_Text'></a>

<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> uses the <a href='undocumented#Pixel'>pixel</a> transparency to represent a fractional offset.
As the opaqueness
of the <a href='SkColor_Reference#Color'>color</a> increases, the edge of the <a href='undocumented#Glyph'>glyph</a> appears to move towards the outside of the <a href='undocumented#Pixel'>pixel</a>.

<a name='SkPaint_isSubpixelText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isSubpixelText'>isSubpixelText</a>()const
</pre>

Returns true if <a href='undocumented#Glyph'>glyphs</a> at different <a href='SkFont_Reference#Subpixel'>sub-pixel</a> positions may differ on <a href='undocumented#Pixel'>pixel</a> edge coverage.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() masked with <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a>.

### Return Value

<a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> state

### Example

<div><fiddle-embed name="abe9afc0932e2199324ae6cbb396e67c">

#### Example Output

~~~~
paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)
paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)
~~~~

</fiddle-embed></div>

<a name='SkPaint_setSubpixelText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setSubpixelText'>setSubpixelText</a>(bool subpixelText)
</pre>

Requests, but does not require, that <a href='undocumented#Glyph'>glyphs</a> respect <a href='SkFont_Reference#Subpixel'>sub-pixel</a> positioning.

Sets <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> if <a href='#SkPaint_setSubpixelText_subpixelText'>subpixelText</a> is true.
Clears <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> if <a href='#SkPaint_setSubpixelText_subpixelText'>subpixelText</a> is false.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setSubpixelText_subpixelText'><code><strong>subpixelText</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a77bbc1a4e3be9a8ab0f842f877c5ee4">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

<a name='LCD_Text'></a>

When set, <a href='#Font_Anti_Alias'>Font_Anti_Alias</a> takes advantage of the organization of RGB stripes that
create a <a href='SkColor_Reference#Color'>color</a>, and relies
on the small <a href='undocumented#Size'>size</a> of the stripe and visual perception to make the <a href='SkColor_Reference#Color'>color</a> fringing imperceptible.
<a href='#Font_Anti_Alias'>Font_Anti_Alias</a> can be enabled on devices that orient stripes horizontally or vertically, and that order
the <a href='SkColor_Reference#Color'>color</a> components as RGB or BGR.

<a name='SkPaint_isLCDRenderText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isLCDRenderText'>isLCDRenderText</a>()const
</pre>

Returns true if <a href='undocumented#Glyph'>glyphs</a> may use LCD striping to improve <a href='undocumented#Glyph'>glyph</a> edges.

Returns true if <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> is set.

### Return Value

<a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> state

### Example

<div><fiddle-embed name="68e1fd95dd2fd06a333899d2bd2396b9">

#### Example Output

~~~~
paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)
paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)
~~~~

</fiddle-embed></div>

<a name='SkPaint_setLCDRenderText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setLCDRenderText'>setLCDRenderText</a>(bool lcdText)
</pre>

Requests, but does not require, that <a href='undocumented#Glyph'>glyphs</a> use LCD striping for <a href='undocumented#Glyph'>glyph</a> edges.

Sets <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> if <a href='#SkPaint_setLCDRenderText_lcdText'>lcdText</a> is true.
Clears <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> if <a href='#SkPaint_setLCDRenderText_lcdText'>lcdText</a> is false.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setLCDRenderText_lcdText'><code><strong>lcdText</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="50dedf8450159571a3edaf4f0050defe">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

<a name='Embedded_Bitmaps'></a>

---

<a href='#Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a> allows selecting custom sized <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='undocumented#Glyph'>Glyphs</a>.
<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> when set chooses an embedded <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='undocumented#Glyph'>glyph</a> over an outline contained
in a <a href='SkFont_Reference#Font'>font</a> if the platform supports this option.

FreeType selects the <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='undocumented#Glyph'>glyph</a> if available when <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> is set, and selects
the outline <a href='undocumented#Glyph'>glyph</a> if <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> is clear.
Windows may select the <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='undocumented#Glyph'>glyph</a> but is not required to do so.
<a href='#OS_X'>OS_X</a> and iOS do not support this option.

<a href='#Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a> is disabled by default.
<a href='#Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a> can be enabled by default by setting <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults_Flags</a> to
<a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> at compile time.

### Example

<pre style="padding: 1em 1em 1em 1em; font-size: 13px width: 62.5em; background-color: #f0f0f0">
<div>The "hintgasp" TrueType font in the Skia resources/fonts directory
        includes an embedded bitmap Glyph at odd font sizes. This example works
        on platforms that use FreeType as their Font_Engine.
        Windows may, but is not required to, return a bitmap glyph if
        kEmbeddedBitmapText_Flag is set.</div>SkBitmap bitmap;
    bitmap.allocN32Pixels(30, 15);
    bitmap.eraseColor(0);
    SkCanvas offscreen(bitmap);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(13);
    paint.setTypeface(MakeResourceAsTypeface("fonts/hintgasp.ttf"));
    for (bool embedded : { false, true}) {
        paint.setEmbeddedBitmapText(embedded);
        offscreen.drawString("A", embedded ? 5 : 15, 15, paint);
    }
    canvas->drawBitmap(bitmap, 0, 0);
    canvas->scale(10, 10);
    canvas->drawBitmap(bitmap, -2, 1);

</pre>

<a name='SkPaint_isEmbeddedBitmapText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isEmbeddedBitmapText'>isEmbeddedBitmapText</a>()const
</pre>

Returns true if <a href='SkFont_Reference#Font'>font</a> engine may return <a href='undocumented#Glyph'>glyphs</a> from <a href='SkFont_Reference#Font'>font</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> instead of from outlines.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() masked with <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a>.

### Return Value

<a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> state

### Example

<div><fiddle-embed name="eba10b27b790e87183ae451b3fc5c4b1">

#### Example Output

~~~~
paint.isEmbeddedBitmapText() == !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)
paint.isEmbeddedBitmapText() == !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)
~~~~

</fiddle-embed></div>

<a name='SkPaint_setEmbeddedBitmapText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setEmbeddedBitmapText'>setEmbeddedBitmapText</a>(bool useEmbeddedBitmapText)
</pre>

Requests, but does not require, to use <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> in fonts instead of outlines.

Sets <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> if <a href='#SkPaint_setEmbeddedBitmapText_useEmbeddedBitmapText'>useEmbeddedBitmapText</a> is true.
Clears <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> if <a href='#SkPaint_setEmbeddedBitmapText_useEmbeddedBitmapText'>useEmbeddedBitmapText</a> is false.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setEmbeddedBitmapText_useEmbeddedBitmapText'><code><strong>useEmbeddedBitmapText</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="246dffdd93a484ba4ad7ecf71198a5d4">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

<a name='Automatic_Hinting'></a>

If Hinting is set to <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNormal'>kNormal</a> or <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kFull'>kFull</a>, <a href='#Font_Force_Hinting'>Font_Force_Hinting</a>
instructs the <a href='#Font_Manager'>Font_Manager</a> to always hint <a href='undocumented#Glyph'>Glyphs</a>.
<a href='#Font_Force_Hinting'>Font_Force_Hinting</a> has no effect if Hinting is set to <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNone'>kNone</a> or
<a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kSlight'>kSlight</a>.

<a href='#Font_Force_Hinting'>Font_Force_Hinting</a> only affects platforms that use FreeType as the <a href='#Font_Manager'>Font_Manager</a>.

<a name='SkPaint_isAutohinted'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isAutohinted'>isAutohinted</a>()const
</pre>

Returns true if <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::Hinting is set to <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNormal'>kNormal</a> or
<a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kFull'>kFull</a>, and if platform uses FreeType as the <a href='SkFont_Reference#Font'>font</a> manager.
If true, instructs the <a href='SkFont_Reference#Font'>font</a> manager to always hint <a href='undocumented#Glyph'>glyphs</a>.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() masked with <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a>.

### Return Value

<a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a> state

### Example

<div><fiddle-embed name="aa4781afbe3b90e7ef56a287e5b9ce1e">

#### Example Output

~~~~
paint.isAutohinted() == !!(paint.getFlags() & SkPaint::kAutoHinting_Flag)
paint.isAutohinted() == !!(paint.getFlags() & SkPaint::kAutoHinting_Flag)
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_setAutohinted'>setAutohinted</a> Hinting

<a name='SkPaint_setAutohinted'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setAutohinted'>setAutohinted</a>(bool useAutohinter)
</pre>

Sets whether to always hint <a href='undocumented#Glyph'>glyphs</a>.
If <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::Hinting is set to <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNormal'>kNormal</a> or <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kFull'>kFull</a>
and <a href='#SkPaint_setAutohinted_useAutohinter'>useAutohinter</a> is set, instructs the  <a href='undocumented#Font_Manager'>font manager</a> to always hint <a href='undocumented#Glyph'>glyphs</a>.
<a href='#SkPaint_setAutohinted_useAutohinter'>useAutohinter</a> has no effect if <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::Hinting is set to <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNone'>kNone</a> or
<a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kSlight'>kSlight</a>.

Only affects platforms that use FreeType as the  <a href='undocumented#Font_Manager'>font manager</a>.

Sets <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a> if <a href='#SkPaint_setAutohinted_useAutohinter'>useAutohinter</a> is true.
Clears <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a> if <a href='#SkPaint_setAutohinted_useAutohinter'>useAutohinter</a> is false.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setAutohinted_useAutohinter'><code><strong>useAutohinter</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4e185306d7de9390fe8445eed0139309"></fiddle-embed></div>

### See Also

<a href='#SkPaint_isAutohinted'>isAutohinted</a> Hinting

<a name='Fake_Bold'></a>

---

<a href='#Font_Embolden'>Font_Embolden</a> approximates the bold <a href='SkFont_Reference#Font'>font</a> style accompanying a normal <a href='SkFont_Reference#Font'>font</a> when a bold <a href='SkFont_Reference#Font'>font</a> face
is not available. Skia does not provide <a href='SkFont_Reference#Font'>font</a> substitution; it is up to the client to find the
bold <a href='SkFont_Reference#Font'>font</a> face using the platform <a href='#Font_Manager'>Font_Manager</a>.

Use <a href='#Paint_Text_Skew_X'>Text_Skew_X</a> to approximate an italic <a href='SkFont_Reference#Font'>font</a> style when the italic <a href='SkFont_Reference#Font'>font</a> face
is not available.

A FreeType based port may define SK_USE_FREETYPE_EMBOLDEN at compile time to direct
the  <a href='SkFont_Reference#Font_Engine'>font engine</a> to create the bold <a href='undocumented#Glyph'>Glyphs</a>. Otherwise, the extra bold is computed
by increasing the  <a href='#Stroke_Width'>stroke width</a> and setting the <a href='#SkPaint_Style'>Style</a> to <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a> as needed.

<a href='#Font_Embolden'>Font_Embolden</a> is disabled by default.

### Example

<div><fiddle-embed name="e811f4829a2daaaeaad3795504a7e02a"></fiddle-embed></div>

<a name='SkPaint_isFakeBoldText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isFakeBoldText'>isFakeBoldText</a>()const
</pre>

Returns true if approximate bold by increasing the stroke width when creating <a href='undocumented#Glyph'>glyph</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a>
from outlines.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() masked with <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a>.

### Return Value

<a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a> state

### Example

<div><fiddle-embed name="f54d1f85b16073b80b9eef2e1a1d151d">

#### Example Output

~~~~
paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)
paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)
~~~~

</fiddle-embed></div>

<a name='SkPaint_setFakeBoldText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setFakeBoldText'>setFakeBoldText</a>(bool fakeBoldText)
</pre>

Increases  <a href='#Stroke_Width'>stroke width</a> when creating <a href='undocumented#Glyph'>glyph</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> to approximate a bold <a href='undocumented#Typeface'>typeface</a>.

Sets <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a> if <a href='#SkPaint_setFakeBoldText_fakeBoldText'>fakeBoldText</a> is true.
Clears <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a> if <a href='#SkPaint_setFakeBoldText_fakeBoldText'>fakeBoldText</a> is false.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setFakeBoldText_fakeBoldText'><code><strong>fakeBoldText</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="594d47858eb11028cb626515a520910a">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

<a name='Filter_Quality_Methods'></a>

---

<a href='#Filter_Quality'>Filter_Quality</a> trades speed for <a href='SkImage_Reference#Image'>image</a> filtering when the <a href='SkImage_Reference#Image'>image</a> is scaled.
A lower <a href='#Filter_Quality'>Filter_Quality</a> draws faster, but has less fidelity.
A higher <a href='#Filter_Quality'>Filter_Quality</a> draws slower, but looks better.
If the <a href='SkImage_Reference#Image'>image</a> is drawn without scaling, the <a href='#Filter_Quality'>Filter_Quality</a> choice will not result
in a noticeable difference.

<a href='#Filter_Quality'>Filter_Quality</a> is used in <a href='SkPaint_Reference#Paint'>Paint</a> passed as a parameter to

<table>  <tr>
    <td><a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawBitmap'>drawBitmap</a></td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a></td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawImage'>drawImage</a></td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawImageRect'>drawImageRect</a></td>
  </tr>
</table>

and when <a href='SkPaint_Reference#Paint'>Paint</a> has a <a href='undocumented#Shader'>Shader</a> specialization that uses <a href='SkImage_Reference#Image'>Image</a> or <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>.

<a href='#Filter_Quality'>Filter_Quality</a> is <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a> by default.

### Example

<div><fiddle-embed name="@Filter_Quality_Methods"></fiddle-embed></div>

<a name='SkPaint_getFilterQuality'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='#SkPaint_getFilterQuality'>getFilterQuality</a>()const
</pre>

Returns <a href='undocumented#SkFilterQuality'>SkFilterQuality</a>, the <a href='SkImage_Reference#Image'>image</a> filtering level. A lower setting
draws faster; a higher setting looks better when the <a href='SkImage_Reference#Image'>image</a> is scaled.

### Return Value

one of: <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a>,

<a href='undocumented#kMedium_SkFilterQuality'>kMedium_SkFilterQuality</a>, <a href='undocumented#kHigh_SkFilterQuality'>kHigh_SkFilterQuality</a>

### Example

<div><fiddle-embed name="@Paint_getFilterQuality">

#### Example Output

~~~~
kNone_SkFilterQuality == paint.getFilterQuality()
~~~~

</fiddle-embed></div>

<a name='SkPaint_setFilterQuality'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setFilterQuality'>setFilterQuality</a>(<a href='undocumented#SkFilterQuality'>SkFilterQuality</a> quality)
</pre>

Sets <a href='undocumented#SkFilterQuality'>SkFilterQuality</a>, the <a href='SkImage_Reference#Image'>image</a> filtering level. A lower setting
draws faster; a higher setting looks better when the <a href='SkImage_Reference#Image'>image</a> is scaled.
Does not check to see if <a href='#SkPaint_setFilterQuality_quality'>quality</a> is valid.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setFilterQuality_quality'><code><strong>quality</strong></code></a></td>
    <td>one of: <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a>,</td>
  </tr>
</table>

<a href='undocumented#kMedium_SkFilterQuality'>kMedium_SkFilterQuality</a>, <a href='undocumented#kHigh_SkFilterQuality'>kHigh_SkFilterQuality</a>

### Example

<div><fiddle-embed name="@Paint_setFilterQuality">

#### Example Output

~~~~
kHigh_SkFilterQuality == paint.getFilterQuality()
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='#Image_Scaling'>Image_Scaling</a>

<a name='Color_Methods'></a>

---

| name | description |
| --- | ---  |
| <a href='#SkPaint_getColor'>getColor</a> | returns <a href='#Color_Alpha'>Color_Alpha</a> and RGB, one drawing <a href='SkColor_Reference#Color'>color</a> |
| <a href='#SkPaint_setColor'>setColor</a> | sets <a href='#Color_Alpha'>Color_Alpha</a> and RGB, one drawing <a href='SkColor_Reference#Color'>color</a> |

<a href='SkColor_Reference#Color'>Color</a> specifies the red, blue, green, and <a href='#Color_Alpha'>Color_Alpha</a>
values used to draw a filled or stroked shape in a 32-bit value. Each component
occupies 8-bits, ranging from zero: no contribution; to 255: full intensity.
All values in any combination are valid.

<a href='SkColor_Reference#Color'>Color</a> is not <a href='undocumented#Premultiply'>Premultiplied</a>; <a href='#Color_Alpha'>Color_Alpha</a> sets the transparency independent of
RGB: red, blue, and green.

The bit positions of <a href='#Color_Alpha'>Color_Alpha</a> and RGB are independent of the bit
positions on the output <a href='undocumented#Device'>device</a>, which may have more or fewer bits, and may have
a different arrangement.

| bit positions | <a href='#Color_Alpha'>Color_Alpha</a> | red | blue | green |
| --- | --- | --- | --- | ---  |
|  | 31 - 24 | 23 - 16 | 15 - 8 | 7 - 0 |

### Example

<div><fiddle-embed name="@Color_Methods"></fiddle-embed></div>

<a name='SkPaint_getColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPaint_getColor'>getColor</a>()const
</pre>

Retrieves <a href='SkColor_Reference#Alpha'>alpha</a> and RGB, <a href='undocumented#Unpremultiply'>unpremultiplied</a>, packed into 32 bits.
Use helpers <a href='SkColor_Reference#SkColorGetA'>SkColorGetA</a>(), <a href='SkColor_Reference#SkColorGetR'>SkColorGetR</a>(), <a href='SkColor_Reference#SkColorGetG'>SkColorGetG</a>(), and <a href='SkColor_Reference#SkColorGetB'>SkColorGetB</a>() to extract
a <a href='SkColor_Reference#Color'>color</a> component.

### Return Value

<a href='undocumented#Unpremultiply'>unpremultiplied</a> ARGB

### Example

<div><fiddle-embed name="@Paint_getColor">

#### Example Output

~~~~
Yellow is 100% red, 100% green, and 0% blue.
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_getColor4f'>getColor4f</a> <a href='SkColor_Reference#SkColor'>SkColor</a>

<a name='SkPaint_getColor4f'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a> <a href='#SkPaint_getColor4f'>getColor4f</a>()const
</pre>

Retrieves <a href='SkColor_Reference#Alpha'>alpha</a> and RGB, <a href='undocumented#Unpremultiply'>unpremultiplied</a>, as four floating <a href='SkPoint_Reference#Point'>point</a> values. RGB are
are extended sRGB values (sRGB gamut, and encoded with the sRGB transfer function).

### Return Value

<a href='undocumented#Unpremultiply'>unpremultiplied</a> RGBA

### Example

<div><fiddle-embed name="@Paint_getColor4f">

#### Example Output

~~~~
Yellow is 100% red, 100% green, and 0% blue.
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_getColor'>getColor</a> <a href='SkColor_Reference#SkColor'>SkColor</a>

<a name='SkPaint_setColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setColor'>setColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>)
</pre>

Sets <a href='SkColor_Reference#Alpha'>alpha</a> and RGB used when stroking and filling. The <a href='#SkPaint_setColor_color'>color</a> is a 32-bit value,
<a href='undocumented#Unpremultiply'>unpremultiplied</a>, packing 8-bit components for <a href='SkColor_Reference#Alpha'>alpha</a>, red, blue, and green.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setColor_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> ARGB</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_setColor">

#### Example Output

~~~~
green1 == green2
~~~~

</fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPaint_setColor4f'>setColor4f</a> <a href='#SkPaint_setARGB'>setARGB</a> <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>

<a name='SkPaint_setColor4f'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setColor4f'>setColor4f</a>(const <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>& <a href='SkColor_Reference#Color'>color</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* colorSpace)
</pre>

Sets <a href='SkColor_Reference#Alpha'>alpha</a> and RGB used when stroking and filling. The <a href='#SkPaint_setColor4f_color'>color</a> is four floating
<a href='SkPoint_Reference#Point'>point</a> values, <a href='undocumented#Unpremultiply'>unpremultiplied</a>. The <a href='#SkPaint_setColor4f_color'>color</a> values are interpreted as being in
the <a href='#SkPaint_setColor4f_colorSpace'>colorSpace</a>. If <a href='#SkPaint_setColor4f_colorSpace'>colorSpace</a> is nullptr, then <a href='#SkPaint_setColor4f_color'>color</a> is assumed to be in the
sRGB  <a href='undocumented#Color_Space'>color space</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setColor4f_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> RGBA</td>
  </tr>
  <tr>    <td><a name='SkPaint_setColor4f_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td><a href='undocumented#SkColorSpace'>SkColorSpace</a> describing the encoding of <a href='#SkPaint_setColor4f_color'>color</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_setColor4f">

#### Example Output

~~~~
green1 == green2
~~~~

</fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPaint_setColor'>setColor</a> <a href='#SkPaint_setARGB'>setARGB</a> <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>

<a name='Alpha_Methods'></a>

<a href='#Color_Alpha'>Color_Alpha</a> sets the transparency independent of RGB: red, blue, and green.

<a name='SkPaint_getAlpha'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint8_t <a href='#SkPaint_getAlpha'>getAlpha</a>()const
</pre>

Retrieves <a href='SkColor_Reference#Alpha'>alpha</a> from the <a href='SkColor_Reference#Color'>color</a> used when stroking and filling.

### Return Value

<a href='SkColor_Reference#Alpha'>alpha</a> ranging from zero, fully transparent, to 255, fully opaque

### Example

<div><fiddle-embed name="@Paint_getAlpha">

#### Example Output

~~~~
255 == paint.getAlpha()
~~~~

</fiddle-embed></div>

<a name='SkPaint_setAlpha'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setAlpha'>setAlpha</a>(<a href='undocumented#U8CPU'>U8CPU</a> a)
</pre>

Replaces <a href='SkColor_Reference#Alpha'>alpha</a>, leaving RGB
unchanged. An out of range value triggers an assert in the debug
build. <a href='#SkPaint_setAlpha_a'>a</a> is <a href='#SkPaint_setAlpha_a'>a</a> value from zero to 255.
<a href='#SkPaint_setAlpha_a'>a</a> set to zero makes <a href='SkColor_Reference#Color'>color</a> fully transparent; <a href='#SkPaint_setAlpha_a'>a</a> set to 255 makes <a href='SkColor_Reference#Color'>color</a>
fully opaque.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setAlpha_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkColor_Reference#Alpha'>alpha</a> component of <a href='SkColor_Reference#Color'>color</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_setAlpha">

#### Example Output

~~~~
0x44112233 == paint.getColor()
~~~~

</fiddle-embed></div>

<a name='SkPaint_setARGB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setARGB'>setARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> a, <a href='undocumented#U8CPU'>U8CPU</a> r, <a href='undocumented#U8CPU'>U8CPU</a> g, <a href='undocumented#U8CPU'>U8CPU</a> b)
</pre>

Sets <a href='SkColor_Reference#Color'>color</a> used when drawing solid fills. The <a href='SkColor_Reference#Color'>color</a> components range from 0 to 255.
The <a href='SkColor_Reference#Color'>color</a> is <a href='undocumented#Unpremultiply'>unpremultiplied</a>; <a href='SkColor_Reference#Alpha'>alpha</a> sets the transparency independent of RGB.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setARGB_a'><code><strong>a</strong></code></a></td>
    <td>amount of <a href='SkColor_Reference#Alpha'>alpha</a>, from fully transparent (0) to fully opaque (255)</td>
  </tr>
  <tr>    <td><a name='SkPaint_setARGB_r'><code><strong>r</strong></code></a></td>
    <td>amount of red, from no red (0) to full red (255)</td>
  </tr>
  <tr>    <td><a name='SkPaint_setARGB_g'><code><strong>g</strong></code></a></td>
    <td>amount of green, from no green (0) to full green (255)</td>
  </tr>
  <tr>    <td><a name='SkPaint_setARGB_b'><code><strong>b</strong></code></a></td>
    <td>amount of blue, from no blue (0) to full blue (255)</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_setARGB">

#### Example Output

~~~~
transRed1 == transRed2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_setColor'>setColor</a> <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>

<a name='Style'></a>

---

<a href='#SkPaint_Style'>Style</a> specifies if the geometry is filled, stroked, or both filled and stroked.
Some shapes ignore <a href='#SkPaint_Style'>Style</a> and are always drawn filled or stroked.

<a name='Style_Fill'></a>

Set <a href='#SkPaint_Style'>Style</a> to <a href='#SkPaint_kFill_Style'>kFill_Style</a> to fill the shape.
The fill covers the area inside the geometry for most shapes.

<a name='Style_Stroke'></a>

Set <a href='#SkPaint_Style'>Style</a> to <a href='#SkPaint_kStroke_Style'>kStroke_Style</a> to stroke the shape.

The stroke covers the area described by following the shape edge with a pen or brush of
<a href='#Paint_Stroke_Width'>Stroke_Width</a>. The area covered where the shape starts and stops is described by <a href='#Paint_Stroke_Cap'>Stroke_Cap</a>.
The area covered where the shape turns a corner is described by <a href='#Paint_Stroke_Join'>Stroke_Join</a>.
The stroke is centered on the shape; it extends equally on either side of the shape edge.As <a href='#Paint_Stroke_Width'>Stroke_Width</a> gets smaller, the drawn <a href='SkPath_Reference#Path'>path</a> frame is thinner. <a href='#Paint_Stroke_Width'>Stroke_Width</a> less than one
may have gaps, and if <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> is set, <a href='#Color_Alpha'>Color_Alpha</a> will increase to visually decrease coverage.

### See Also

<a href='#Path_Fill_Type'>Path_Fill_Type</a> <a href='#Path_Effect'>Path_Effect</a> <a href='#Paint_Style_Fill'>Style_Fill</a> <a href='#Paint_Style_Stroke'>Style_Stroke</a>

<a name='Hairline'></a>

---

<a href='#Paint_Stroke_Width'>Stroke_Width</a> of zero has a special meaning and switches drawing to use Hairline.
Hairline draws the thinnest continuous frame. If <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> is clear, adjacent pixels
flow horizontally, vertically,or diagonally.

<a href='SkPath_Reference#Path'>Path</a> drawing with Hairline may hit the same <a href='undocumented#Pixel'>pixel</a> more than once. For instance, <a href='SkPath_Reference#Path'>Path</a> containing
two <a href='undocumented#Line'>lines</a> in one <a href='#Path_Overview_Contour'>Path_Contour</a> will draw the corner <a href='SkPoint_Reference#Point'>point</a> once, but may both <a href='undocumented#Line'>lines</a> may draw the adjacent
<a href='undocumented#Pixel'>pixel</a>. If <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> is set, transparency is applied twice, resulting in a darker <a href='undocumented#Pixel'>pixel</a>. Some
GPU-backed implementations apply transparency at a later drawing stage, avoiding double hit pixels
while stroking.

### See Also

<a href='#Path_Fill_Type'>Path_Fill_Type</a> <a href='#Path_Effect'>Path_Effect</a> <a href='#Paint_Style_Fill'>Style_Fill</a> <a href='#Paint_Style_Stroke'>Style_Stroke</a>

<a name='SkPaint_Style'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Style'>Style</a> : uint8_t {
        <a href='#SkPaint_kFill_Style'>kFill_Style</a>,
        <a href='#SkPaint_kStroke_Style'>kStroke_Style</a>,
        <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>,
    };

</pre>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    static constexpr int <a href='#SkPaint_kStyleCount'>kStyleCount</a> = <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a> + 1;
</pre>

Set <a href='#SkPaint_Style'>Style</a> to fill, stroke, or both fill and stroke geometry.
The stroke and fill
share all <a href='SkPaint_Reference#Paint'>paint</a> attributes; for instance, they are drawn with the same <a href='SkColor_Reference#Color'>color</a>.

Use <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a> to avoid hitting the same pixels twice with a stroke draw and
a fill draw.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kFill_Style'><code>SkPaint::kFill_Style</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Applies to <a href='SkRect_Reference#Rect'>Rect</a>, <a href='SkRegion_Reference#Region'>Region</a>, <a href='#RRect'>Round_Rect</a>, <a href='undocumented#Circle'>Circles</a>, <a href='undocumented#Oval'>Ovals</a>, <a href='SkPath_Reference#Path'>Path</a>, and <a href='undocumented#Text'>Text</a>.
<a href='SkBitmap_Reference#Bitmap'>Bitmap</a>, <a href='SkImage_Reference#Image'>Image</a>, <a href='undocumented#Patch'>Patches</a>, <a href='SkRegion_Reference#Region'>Region</a>, <a href='undocumented#Sprite'>Sprites</a>, and <a href='undocumented#Vertices'>Vertices</a> are painted as if
<a href='#SkPaint_kFill_Style'>kFill_Style</a> is set, and ignore the set <a href='#SkPaint_Style'>Style</a>.
The <a href='#Path_Fill_Type'>Path_Fill_Type</a> specifies additional rules to fill the area outside the <a href='SkPath_Reference#Path'>path</a> edge,
and to create an unfilled hole inside the shape.
<a href='#SkPaint_Style'>Style</a> is set to <a href='#SkPaint_kFill_Style'>kFill_Style</a> by default.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kStroke_Style'><code>SkPaint::kStroke_Style</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Applies to <a href='SkRect_Reference#Rect'>Rect</a>, <a href='SkRegion_Reference#Region'>Region</a>, <a href='#RRect'>Round_Rect</a>, <a href='undocumented#Arc'>Arcs</a>, <a href='undocumented#Circle'>Circles</a>, <a href='undocumented#Oval'>Ovals</a>, <a href='SkPath_Reference#Path'>Path</a>, and <a href='undocumented#Text'>Text</a>.
<a href='undocumented#Arc'>Arcs</a>, <a href='undocumented#Line'>Lines</a>, and <a href='SkPoint_Reference#Point'>points</a>, are always drawn as if <a href='#SkPaint_kStroke_Style'>kStroke_Style</a> is set,
and ignore the set <a href='#SkPaint_Style'>Style</a>.
The stroke construction is unaffected by the <a href='#Path_Fill_Type'>Path_Fill_Type</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kStrokeAndFill_Style'><code>SkPaint::kStrokeAndFill_Style</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Applies to <a href='SkRect_Reference#Rect'>Rect</a>, <a href='SkRegion_Reference#Region'>Region</a>, <a href='#RRect'>Round_Rect</a>, <a href='undocumented#Circle'>Circles</a>, <a href='undocumented#Oval'>Ovals</a>, <a href='SkPath_Reference#Path'>Path</a>, and <a href='undocumented#Text'>Text</a>.
<a href='SkPath_Reference#Path'>Path</a> is treated as if it is set to <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>,
and the set <a href='#Path_Fill_Type'>Path_Fill_Type</a> is ignored.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kStyleCount'><code>SkPaint::kStyleCount</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#SkPaint_Style'>Style</a> is a legal value.
</td>
  </tr>
</table>

<a name='SkPaint_getStyle'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_getStyle'>getStyle</a>()const
</pre>

Returns whether the geometry is filled, stroked, or filled and stroked.

### Return Value

one of:<a href='#SkPaint_kFill_Style'>kFill_Style</a>, <a href='#SkPaint_kStroke_Style'>kStroke_Style</a>, <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>

### Example

<div><fiddle-embed name="@Paint_getStyle">

#### Example Output

~~~~
SkPaint::kFill_Style == paint.getStyle()
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_setStyle'>setStyle</a>

<a name='SkPaint_setStyle'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setStyle'>setStyle</a>(<a href='#SkPaint_Style'>Style</a> style)
</pre>

Sets whether the geometry is filled, stroked, or filled and stroked.
Has no effect if <a href='#SkPaint_setStyle_style'>style</a> is not a legal <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> value.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStyle_style'><code><strong>style</strong></code></a></td>
    <td>one of: <a href='#SkPaint_kFill_Style'>kFill_Style</a>, <a href='#SkPaint_kStroke_Style'>kStroke_Style</a>, <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_setStyle"></fiddle-embed></div>

### See Also

<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_getStyle'>getStyle</a>

<a name='Stroke_Width'></a>

---

<a href='#Paint_Stroke_Width'>Stroke_Width</a> sets the width for stroking. The width is the thickness
of the stroke perpendicular to the  <a href='SkPath_Reference#Path_Direction'>path direction</a> when the  <a href='SkPaint_Reference#Paint'>paint style</a> is
set to <a href='#SkPaint_kStroke_Style'>kStroke_Style</a> or <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>.

When width is greater than zero, the stroke encompasses as many pixels partially
or fully as needed. When the width equals zero, the <a href='SkPaint_Reference#Paint'>paint</a> enables hairlines;
the stroke is always one <a href='undocumented#Pixel'>pixel</a> wide.

The stroke dimensions are scaled by the  <a href='SkCanvas_Reference#Canvas_Matrix'>canvas matrix</a>, but Hairline stroke
remains one <a href='undocumented#Pixel'>pixel</a> wide regardless of scaling.

The default width for the <a href='SkPaint_Reference#Paint'>paint</a> is zero.

### Example

<div><fiddle-embed name="@Stroke_Width" gpu="true"><div>The pixels hit to represent thin <a href='undocumented#Line'>lines</a> vary with the angle of the
<a href='undocumented#Line'>line</a> and the platform implementation.
</div></fiddle-embed></div>

<a name='SkPaint_getStrokeWidth'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getStrokeWidth'>getStrokeWidth</a>()const
</pre>

Returns the thickness of the pen used by <a href='SkPaint_Reference#SkPaint'>SkPaint</a> to
outline the shape.

### Return Value

zero for hairline, greater than zero for pen thickness

### Example

<div><fiddle-embed name="@Paint_getStrokeWidth">

#### Example Output

~~~~
0 == paint.getStrokeWidth()
~~~~

</fiddle-embed></div>

<a name='SkPaint_setStrokeWidth'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setStrokeWidth'>setStrokeWidth</a>(<a href='undocumented#SkScalar'>SkScalar</a> width)
</pre>

Sets the thickness of the pen used by the <a href='SkPaint_Reference#Paint'>paint</a> to
outline the shape.
Has no effect if <a href='#SkPaint_setStrokeWidth_width'>width</a> is less than zero.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeWidth_width'><code><strong>width</strong></code></a></td>
    <td>zero thickness for hairline; greater than zero for pen thickness</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_setStrokeWidth">

#### Example Output

~~~~
5 == paint.getStrokeWidth()
~~~~

</fiddle-embed></div>

<a name='Miter_Limit'></a>

---

<a href='#Paint_Miter_Limit'>Miter_Limit</a> specifies the maximum miter length,
relative to the  <a href='#Stroke_Width'>stroke width</a>.

<a href='#Paint_Miter_Limit'>Miter_Limit</a> is used when the <a href='#Paint_Stroke_Join'>Stroke_Join</a>
is set to <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>, and the <a href='#SkPaint_Style'>Style</a> is either <a href='#SkPaint_kStroke_Style'>kStroke_Style</a>
or <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>.

If the miter at a corner exceeds this limit, <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>
is replaced with <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>.

<a href='#Paint_Miter_Limit'>Miter_Limit</a> can be computed from the corner angle using:
<code><a href='#Miter_Limit'>miter limit</a> = 1 / sin ( angle / 2 )</code>.

<a href='#Paint_Miter_Limit'>Miter_Limit</a> default value is 4.
The default may be changed at compile time by setting <a href='undocumented#SkPaintDefaults_MiterLimit'>SkPaintDefaults_MiterLimit</a>
in "SkUserConfig.h" or as a define supplied by the build environment.

Here are some miter limits and the angles that triggers them.

| <a href='#Miter_Limit'>miter limit</a> | angle in degrees |
| --- | ---  |
| 10 | 11.48 |
| 9 | 12.76 |
| 8 | 14.36 |
| 7 | 16.43 |
| 6 | 19.19 |
| 5 | 23.07 |
| 4 | 28.96 |
| 3 | 38.94 |
| 2 | 60 |
| 1 | 180 |

### Example

<div><fiddle-embed name="@Miter_Limit"><div>This example draws a stroked corner and the miter length beneath.
When the  <a href='#Miter_Limit'>miter limit</a> is decreased slightly, the miter join is replaced
by a bevel join.
</div></fiddle-embed></div>

<a name='SkPaint_getStrokeMiter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getStrokeMiter'>getStrokeMiter</a>()const
</pre>

Returns the limit at which a sharp corner is drawn beveled.

### Return Value

zero and greater miter limit

### Example

<div><fiddle-embed name="@Paint_getStrokeMiter">

#### Example Output

~~~~
default miter limit == 4
~~~~

</fiddle-embed></div>

### See Also

<a href='#Paint_Miter_Limit'>Miter_Limit</a> <a href='#SkPaint_setStrokeMiter'>setStrokeMiter</a> <a href='#SkPaint_Join'>Join</a>

<a name='SkPaint_setStrokeMiter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setStrokeMiter'>setStrokeMiter</a>(<a href='undocumented#SkScalar'>SkScalar</a> miter)
</pre>

Sets the limit at which a sharp corner is drawn beveled.
Valid values are zero and greater.
Has no effect if <a href='#SkPaint_setStrokeMiter_miter'>miter</a> is less than zero.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeMiter_miter'><code><strong>miter</strong></code></a></td>
    <td>zero and greater  <a href='#Miter_Limit'>miter limit</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_setStrokeMiter">

#### Example Output

~~~~
default miter limit == 8
~~~~

</fiddle-embed></div>

### See Also

<a href='#Paint_Miter_Limit'>Miter_Limit</a> <a href='#SkPaint_getStrokeMiter'>getStrokeMiter</a> <a href='#SkPaint_Join'>Join</a>

<a name='Stroke_Cap'></a>

<a name='SkPaint_Cap'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Cap'>Cap</a> {
        <a href='#SkPaint_kButt_Cap'>kButt_Cap</a>,
        <a href='#SkPaint_kRound_Cap'>kRound_Cap</a>,
        <a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>,
        <a href='#SkPaint_kLast_Cap'>kLast_Cap</a> = <a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>,
        <a href='#SkPaint_kDefault_Cap'>kDefault_Cap</a> = <a href='#SkPaint_kButt_Cap'>kButt_Cap</a>,
    };

</pre>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    static constexpr int <a href='#SkPaint_kCapCount'>kCapCount</a> = <a href='#SkPaint_kLast_Cap'>kLast_Cap</a> + 1;
</pre>

<a href='#Paint_Stroke_Cap'>Stroke_Cap</a> draws at the beginning and end of an open <a href='#Path_Overview_Contour'>Path_Contour</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kButt_Cap'><code>SkPaint::kButt_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Does not extend the stroke past the beginning or the end.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kRound_Cap'><code>SkPaint::kRound_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds a <a href='undocumented#Circle'>circle</a> with a diameter equal to <a href='#Paint_Stroke_Width'>Stroke_Width</a> at the beginning
and end.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kSquare_Cap'><code>SkPaint::kSquare_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds a square with sides equal to <a href='#Paint_Stroke_Width'>Stroke_Width</a> at the beginning
and end. The square sides are parallel to the initial and final direction
of the stroke.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kLast_Cap'><code>SkPaint::kLast_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Equivalent to the largest value for <a href='#Paint_Stroke_Cap'>Stroke_Cap</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kDefault_Cap'><code>SkPaint::kDefault_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#Paint_Stroke_Cap'>Stroke_Cap</a> is set to <a href='#SkPaint_kButt_Cap'>kButt_Cap</a> by default.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kCapCount'><code>SkPaint::kCapCount</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#Paint_Stroke_Cap'>Stroke_Cap</a> is a legal value.
</td>
  </tr>
</table>

Stroke describes the area covered by a pen of <a href='#Paint_Stroke_Width'>Stroke_Width</a> as it
follows the <a href='#Path_Overview_Contour'>Path_Contour</a>, moving parallel to the <a href='SkPath_Overview#Contour'>contour</a> direction.

If the <a href='#Path_Overview_Contour'>Path_Contour</a> is not terminated by <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kClose_Verb'>kClose_Verb</a>, the <a href='SkPath_Overview#Contour'>contour</a> has a
visible beginning and end.

<a href='#Path_Overview_Contour'>Path_Contour</a> may start and end at the same <a href='SkPoint_Reference#Point'>point</a>; defining <a href='#Path_Overview_Contour_Zero_Length'>Zero_Length_Contour</a>.

<a href='#SkPaint_kButt_Cap'>kButt_Cap</a> and <a href='#Path_Overview_Contour_Zero_Length'>Zero_Length_Contour</a> is not drawn.
<a href='#SkPaint_kRound_Cap'>kRound_Cap</a> and <a href='#Path_Overview_Contour_Zero_Length'>Zero_Length_Contour</a> draws a <a href='undocumented#Circle'>circle</a> of diameter <a href='#Paint_Stroke_Width'>Stroke_Width</a>
at the <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPoint_Reference#Point'>point</a>.
<a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a> and <a href='#Path_Overview_Contour_Zero_Length'>Zero_Length_Contour</a> draws an upright square with a side of
<a href='#Paint_Stroke_Width'>Stroke_Width</a> at the <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPoint_Reference#Point'>point</a>.

<a href='#Paint_Stroke_Cap'>Stroke_Cap</a> is <a href='#SkPaint_kButt_Cap'>kButt_Cap</a> by default.

### Example

<div><fiddle-embed name="@Paint_053"></fiddle-embed></div>

<a name='SkPaint_getStrokeCap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_getStrokeCap'>getStrokeCap</a>()const
</pre>

Returns the geometry drawn at the beginning and end of strokes.

### Return Value

one of: <a href='#SkPaint_kButt_Cap'>kButt_Cap</a>, <a href='#SkPaint_kRound_Cap'>kRound_Cap</a>, <a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>

### Example

<div><fiddle-embed name="@Paint_getStrokeCap">

#### Example Output

~~~~
kButt_Cap == default stroke cap
~~~~

</fiddle-embed></div>

### See Also

<a href='#Paint_Stroke_Cap'>Stroke_Cap</a> <a href='#SkPaint_setStrokeCap'>setStrokeCap</a>

<a name='SkPaint_setStrokeCap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setStrokeCap'>setStrokeCap</a>(<a href='#SkPaint_Cap'>Cap</a> cap)
</pre>

Sets the geometry drawn at the beginning and end of strokes.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeCap_cap'><code><strong>cap</strong></code></a></td>
    <td>one of: <a href='#SkPaint_kButt_Cap'>kButt_Cap</a>, <a href='#SkPaint_kRound_Cap'>kRound_Cap</a>, <a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>;</td>
  </tr>
</table>

has no effect if <a href='#SkPaint_setStrokeCap_cap'>cap</a> is not valid

### Example

<div><fiddle-embed name="@Paint_setStrokeCap_a">

#### Example Output

~~~~
kRound_Cap == paint.getStrokeCap()
~~~~

</fiddle-embed></div>

### See Also

<a href='#Paint_Stroke_Cap'>Stroke_Cap</a> <a href='#SkPaint_getStrokeCap'>getStrokeCap</a>

<a name='Stroke_Join'></a>

<a href='#Paint_Stroke_Join'>Stroke_Join</a> draws at the sharp corners of an open or closed <a href='#Path_Overview_Contour'>Path_Contour</a>.

Stroke describes the area covered by a pen of <a href='#Paint_Stroke_Width'>Stroke_Width</a> as it
follows the <a href='#Path_Overview_Contour'>Path_Contour</a>, moving parallel to the <a href='SkPath_Overview#Contour'>contour</a> direction.

If the <a href='SkPath_Overview#Contour'>contour</a> direction changes abruptly, because the tangent direction leading
to the end of a <a href='undocumented#Curve'>curve</a> within the <a href='SkPath_Overview#Contour'>contour</a> does not match the tangent direction of
the following <a href='undocumented#Curve'>curve</a>, the pair of <a href='undocumented#Curve'>curves</a> meet at <a href='#Paint_Stroke_Join'>Stroke_Join</a>.

### Example

<div><fiddle-embed name="@Paint_setStrokeCap_b"></fiddle-embed></div>

<a name='SkPaint_Join'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Join'>Join</a> : uint8_t {
        <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>,
        <a href='#SkPaint_kRound_Join'>kRound_Join</a>,
        <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>,
        <a href='#SkPaint_kLast_Join'>kLast_Join</a> = <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>,
        <a href='#SkPaint_kDefault_Join'>kDefault_Join</a> = <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>,
    };

</pre>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    static constexpr int <a href='#SkPaint_kJoinCount'>kJoinCount</a> = <a href='#SkPaint_kLast_Join'>kLast_Join</a> + 1;
</pre>

<a href='#SkPaint_Join'>Join</a> specifies how corners are drawn when a shape is stroked. <a href='#SkPaint_Join'>Join</a>
affects the four corners of a stroked rectangle, and the connected segments in a
stroked <a href='SkPath_Reference#Path'>path</a>.

Choose miter join to draw sharp corners. Choose round join to draw a <a href='undocumented#Circle'>circle</a> with a
radius equal to the  <a href='#Stroke_Width'>stroke width</a> on top of the corner. Choose bevel join to minimally
connect the thick strokes.

The  <a href='#Fill_Path'>fill path</a> constructed to describe the stroked <a href='SkPath_Reference#Path'>path</a> respects the join setting but may
not contain the actual join. For instance, a  <a href='#Fill_Path'>fill path</a> constructed with round joins does
not necessarily include <a href='undocumented#Circle'>circles</a> at each connected segment.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kMiter_Join'><code>SkPaint::kMiter_Join</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Extends the outside corner to the extent allowed by <a href='#Paint_Miter_Limit'>Miter_Limit</a>.
If the extension exceeds <a href='#Paint_Miter_Limit'>Miter_Limit</a>, <a href='#SkPaint_kBevel_Join'>kBevel_Join</a> is used instead.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kRound_Join'><code>SkPaint::kRound_Join</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds a <a href='undocumented#Circle'>circle</a> with a diameter of <a href='#Paint_Stroke_Width'>Stroke_Width</a> at the sharp corner.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kBevel_Join'><code>SkPaint::kBevel_Join</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Connects the outside edges of the sharp corner.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kLast_Join'><code>SkPaint::kLast_Join</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
equivalent to the largest value for Stroke_Join</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kDefault_Join'><code>SkPaint::kDefault_Join</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#Paint_Stroke_Join'>Stroke_Join</a> is set to <a href='#SkPaint_kMiter_Join'>kMiter_Join</a> by default.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kJoinCount'><code>SkPaint::kJoinCount</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#Paint_Stroke_Join'>Stroke_Join</a> is a legal value.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_057"></fiddle-embed></div>

### See Also

<a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a> <a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a> <a href='#SkPaint_setStrokeMiter'>setStrokeMiter</a> <a href='#SkPaint_getStrokeMiter'>getStrokeMiter</a>

<a name='SkPaint_getStrokeJoin'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a>()const
</pre>

Returns the geometry drawn at the corners of strokes.

### Return Value

one of: <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>, <a href='#SkPaint_kRound_Join'>kRound_Join</a>, <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>

### Example

<div><fiddle-embed name="@Paint_getStrokeJoin">

#### Example Output

~~~~
kMiter_Join == default stroke join
~~~~

</fiddle-embed></div>

### See Also

<a href='#Paint_Stroke_Join'>Stroke_Join</a> <a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a>

<a name='SkPaint_setStrokeJoin'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a>(<a href='#SkPaint_Join'>Join</a> join)
</pre>

Sets the geometry drawn at the corners of strokes.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeJoin_join'><code><strong>join</strong></code></a></td>
    <td>one of: <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>, <a href='#SkPaint_kRound_Join'>kRound_Join</a>, <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>;</td>
  </tr>
</table>

otherwise, has no effect

### Example

<div><fiddle-embed name="@Paint_setStrokeJoin">

#### Example Output

~~~~
kMiter_Join == paint.getStrokeJoin()
~~~~

</fiddle-embed></div>

### See Also

<a href='#Paint_Stroke_Join'>Stroke_Join</a> <a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a>

### See Also

<a href='#Paint_Miter_Limit'>Miter_Limit</a>

<a name='Fill_Path'></a>

---

<a href='#Paint_Fill_Path'>Fill_Path</a> creates a <a href='SkPath_Reference#Path'>Path</a> by applying the <a href='#Path_Effect'>Path_Effect</a>, followed by the <a href='#Paint_Style_Stroke'>Style_Stroke</a>.

If <a href='SkPaint_Reference#Paint'>Paint</a> contains <a href='#Path_Effect'>Path_Effect</a>, <a href='#Path_Effect'>Path_Effect</a> operates on the source <a href='SkPath_Reference#Path'>Path</a>; the result
replaces the destination <a href='SkPath_Reference#Path'>Path</a>. Otherwise, the source <a href='SkPath_Reference#Path'>Path</a> is replaces the
destination <a href='SkPath_Reference#Path'>Path</a>.

Fill <a href='SkPath_Reference#Path'>Path</a> can request the <a href='#Path_Effect'>Path_Effect</a> to restrict to a culling rectangle, but
the <a href='#Path_Effect'>Path_Effect</a> is not required to do so.

If <a href='#SkPaint_Style'>Style</a> is <a href='#SkPaint_kStroke_Style'>kStroke_Style</a> or <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>,
and <a href='#Paint_Stroke_Width'>Stroke_Width</a> is greater than zero, the <a href='#Paint_Stroke_Width'>Stroke_Width</a>, <a href='#Paint_Stroke_Cap'>Stroke_Cap</a>, <a href='#Paint_Stroke_Join'>Stroke_Join</a>,
and <a href='#Paint_Miter_Limit'>Miter_Limit</a> operate on the destination <a href='SkPath_Reference#Path'>Path</a>, replacing it.

Fill <a href='SkPath_Reference#Path'>Path</a> can specify the precision used by <a href='#Paint_Stroke_Width'>Stroke_Width</a> to approximate the stroke geometry.

If the <a href='#SkPaint_Style'>Style</a> is <a href='#SkPaint_kStroke_Style'>kStroke_Style</a> and the <a href='#Paint_Stroke_Width'>Stroke_Width</a> is zero, <a href='#SkPaint_getFillPath'>getFillPath</a>
returns false since Hairline has no filled equivalent.

### See Also

<a href='#Paint_Style_Stroke'>Style_Stroke</a> <a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Path_Effect'>Path_Effect</a>

<a name='SkPaint_getFillPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_getFillPath'>getFillPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& src, <a href='SkPath_Reference#SkPath'>SkPath</a>* dst, const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect, <a href='undocumented#SkScalar'>SkScalar</a> resScale = 1)const
</pre>

Returns the filled equivalent of the stroked <a href='SkPath_Reference#Path'>path</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getFillPath_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> read to create a filled version</td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_dst'><code><strong>dst</strong></code></a></td>
    <td>resulting <a href='SkPath_Reference#SkPath'>SkPath</a>; may be the same as <a href='#SkPaint_getFillPath_src'>src</a>, but may not be nullptr</td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>optional limit passed to <a href='undocumented#SkPathEffect'>SkPathEffect</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_resScale'><code><strong>resScale</strong></code></a></td>
    <td>if > 1, increase precision, else if (0 < <a href='#SkPaint_getFillPath_resScale'>resScale</a> < 1) reduce precision</td>
  </tr>
</table>

to favor speed and <a href='undocumented#Size'>size</a>

### Return Value

true if the <a href='SkPath_Reference#Path'>path</a> represents  <a href='#Style_Fill'>style fill</a>, or false if it represents hairline

### Example

<div><fiddle-embed name="@Paint_getFillPath"><div>A very small <a href='SkPath_Reference#Quad'>Quad</a> stroke is turned into a filled <a href='SkPath_Reference#Path'>path</a> with increasing levels of precision.
At the lowest precision, the <a href='SkPath_Reference#Quad'>Quad</a> stroke is approximated by a rectangle.
At the highest precision, the filled <a href='SkPath_Reference#Path'>path</a> has high fidelity compared to the original stroke.
</div></fiddle-embed></div>

<a name='SkPaint_getFillPath_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_getFillPath'>getFillPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& src, <a href='SkPath_Reference#SkPath'>SkPath</a>* dst)const
</pre>

Returns the filled equivalent of the stroked <a href='SkPath_Reference#Path'>path</a>.

Replaces <a href='#SkPaint_getFillPath_2_dst'>dst</a> with the <a href='#SkPaint_getFillPath_2_src'>src</a> <a href='SkPath_Reference#Path'>path</a> modified by <a href='undocumented#SkPathEffect'>SkPathEffect</a> and  <a href='#Style_Stroke'>style stroke</a>.
<a href='undocumented#SkPathEffect'>SkPathEffect</a>, if any, is not culled.  <a href='#Stroke_Width'>stroke width</a> is created with default precision.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getFillPath_2_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> read to create a filled version</td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_2_dst'><code><strong>dst</strong></code></a></td>
    <td>resulting <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='#SkPaint_getFillPath_2_dst'>dst</a> may be the same as <a href='#SkPaint_getFillPath_2_src'>src</a>, but may not be nullptr</td>
  </tr>
</table>

### Return Value

true if the <a href='SkPath_Reference#Path'>path</a> represents  <a href='#Style_Fill'>style fill</a>, or false if it represents hairline

### Example

<div><fiddle-embed name="@Paint_getFillPath_2"></fiddle-embed></div>

<a name='Shader_Methods'></a>

---

<a href='undocumented#Shader'>Shader</a> defines the colors used when drawing a shape.
<a href='undocumented#Shader'>Shader</a> may be an <a href='SkImage_Reference#Image'>image</a>, a gradient, or a computed fill.
If <a href='SkPaint_Reference#Paint'>Paint</a> has no <a href='undocumented#Shader'>Shader</a>, then <a href='SkColor_Reference#Color'>Color</a> fills the shape.

<a href='undocumented#Shader'>Shader</a> is modulated by <a href='#Color_Alpha'>Color_Alpha</a> component of <a href='SkColor_Reference#Color'>Color</a>.
If <a href='undocumented#Shader'>Shader</a> object defines only <a href='#Color_Alpha'>Color_Alpha</a>, then <a href='SkColor_Reference#Color'>Color</a> modulated by <a href='#Color_Alpha'>Color_Alpha</a> describes
the fill.

The drawn transparency can be modified without altering <a href='undocumented#Shader'>Shader</a>, by changing <a href='#Color_Alpha'>Color_Alpha</a>.

### Example

<div><fiddle-embed name="@Shader_Methods_a"></fiddle-embed></div>

If <a href='undocumented#Shader'>Shader</a> generates only <a href='#Color_Alpha'>Color_Alpha</a> then all components of <a href='SkColor_Reference#Color'>Color</a> modulate the output.

### Example

<div><fiddle-embed name="@Shader_Methods_b"></fiddle-embed></div>

<a name='SkPaint_getShader'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkShader'>SkShader</a>* <a href='#SkPaint_getShader'>getShader</a>()const
</pre>

Returns optional colors used when filling a <a href='SkPath_Reference#Path'>path</a>, such as a gradient.

Does not alter <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkShader'>SkShader</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="@Paint_getShader">

#### Example Output

~~~~
nullptr == shader
nullptr != shader
~~~~

</fiddle-embed></div>

<a name='SkPaint_refShader'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkShader'>SkShader</a>&gt; <a href='#SkPaint_refShader'>refShader</a>()const
</pre>

Returns optional colors used when filling a <a href='SkPath_Reference#Path'>path</a>, such as a gradient.

Increases <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Return Value

<a href='undocumented#SkShader'>SkShader</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="@Paint_refShader">

#### Example Output

~~~~
shader unique: true
shader unique: false
~~~~

</fiddle-embed></div>

<a name='SkPaint_setShader'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setShader'>setShader</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkShader'>SkShader</a>&gt; <a href='undocumented#Shader'>shader</a>)
</pre>

Sets optional colors used when filling a <a href='SkPath_Reference#Path'>path</a>, such as a gradient.

Sets <a href='undocumented#SkShader'>SkShader</a> to <a href='#SkPaint_setShader_shader'>shader</a>, decreasing <a href='undocumented#SkRefCnt'>SkRefCnt</a> of the previous <a href='undocumented#SkShader'>SkShader</a>.
Increments <a href='#SkPaint_setShader_shader'>shader</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setShader_shader'><code><strong>shader</strong></code></a></td>
    <td>how geometry is filled with <a href='SkColor_Reference#Color'>color</a>; if nullptr, <a href='SkColor_Reference#Color'>color</a> is used instead</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_setShader"></fiddle-embed></div>

<a name='Color_Filter_Methods'></a>

---

<a href='#Color_Filter'>Color_Filter</a> alters the <a href='SkColor_Reference#Color'>color</a> used when drawing a shape.
<a href='#Color_Filter'>Color_Filter</a> may apply <a href='#Blend_Mode'>Blend_Mode</a>, transform the <a href='SkColor_Reference#Color'>color</a> through a <a href='SkMatrix_Reference#Matrix'>matrix</a>, or composite multiple filters.
If <a href='SkPaint_Reference#Paint'>Paint</a> has no <a href='#Color_Filter'>Color_Filter</a>, the <a href='SkColor_Reference#Color'>color</a> is unaltered.

The drawn transparency can be modified without altering <a href='#Color_Filter'>Color_Filter</a>, by changing <a href='#Color_Alpha'>Color_Alpha</a>.

### Example

<div><fiddle-embed name="@Color_Filter_Methods"></fiddle-embed></div>

<a name='SkPaint_getColorFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkColorFilter'>SkColorFilter</a>* <a href='#SkPaint_getColorFilter'>getColorFilter</a>()const
</pre>

Returns <a href='undocumented#SkColorFilter'>SkColorFilter</a> if set, or nullptr.
Does not alter <a href='undocumented#SkColorFilter'>SkColorFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkColorFilter'>SkColorFilter</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="@Paint_getColorFilter">

#### Example Output

~~~~
nullptr == color filter
nullptr != color filter
~~~~

</fiddle-embed></div>

<a name='SkPaint_refColorFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorFilter'>SkColorFilter</a>&gt; <a href='#SkPaint_refColorFilter'>refColorFilter</a>()const
</pre>

Returns <a href='undocumented#SkColorFilter'>SkColorFilter</a> if set, or nullptr.
Increases <a href='undocumented#SkColorFilter'>SkColorFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Return Value

<a href='undocumented#SkColorFilter'>SkColorFilter</a> if set, or nullptr

### Example

<div><fiddle-embed name="@Paint_refColorFilter">

#### Example Output

~~~~
color filter unique: true
color filter unique: false
~~~~

</fiddle-embed></div>

<a name='SkPaint_setColorFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setColorFilter'>setColorFilter</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorFilter'>SkColorFilter</a>&gt; colorFilter)
</pre>

Sets <a href='undocumented#SkColorFilter'>SkColorFilter</a> to filter, decreasing <a href='undocumented#SkRefCnt'>SkRefCnt</a> of the previous
<a href='undocumented#SkColorFilter'>SkColorFilter</a>. Pass nullptr to clear <a href='undocumented#SkColorFilter'>SkColorFilter</a>.

Increments filter <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setColorFilter_colorFilter'><code><strong>colorFilter</strong></code></a></td>
    <td><a href='undocumented#SkColorFilter'>SkColorFilter</a> to apply to subsequent draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_setColorFilter"></fiddle-embed></div>

<a name='Blend_Mode_Methods'></a>

---

<a href='#Blend_Mode'>Blend_Mode</a> describes how <a href='SkColor_Reference#Color'>Color</a> combines with the destination <a href='SkColor_Reference#Color'>color</a>.
The default setting, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>, draws the source <a href='SkColor_Reference#Color'>color</a>
over the destination <a href='SkColor_Reference#Color'>color</a>.

### Example

<div><fiddle-embed name="@Blend_Mode_Methods"></fiddle-embed></div>

### See Also

<a href='#Blend_Mode'>Blend_Mode</a>

<a name='SkPaint_getBlendMode'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkPaint_getBlendMode'>getBlendMode</a>()const
</pre>

Returns <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>.
By default, returns <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>.

### Return Value

mode used to combine source <a href='SkColor_Reference#Color'>color</a> with destination <a href='SkColor_Reference#Color'>color</a>

### Example

<div><fiddle-embed name="@Paint_getBlendMode">

#### Example Output

~~~~
kSrcOver == getBlendMode
kSrcOver != getBlendMode
~~~~

</fiddle-embed></div>

<a name='SkPaint_isSrcOver'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isSrcOver'>isSrcOver</a>()const
</pre>

Returns true if <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> is <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>, the default.

### Return Value

true if <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> is <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>

### Example

<div><fiddle-embed name="@Paint_setBlendMode">

#### Example Output

~~~~
isSrcOver == true
isSrcOver != true
~~~~

</fiddle-embed></div>

<a name='SkPaint_setBlendMode'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setBlendMode'>setBlendMode</a>(<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode)
</pre>

Sets <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> to <a href='#SkPaint_setBlendMode_mode'>mode</a>.
Does not check for valid input.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setBlendMode_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> used to combine source <a href='SkColor_Reference#Color'>color</a> and destination</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_setBlendMode">

#### Example Output

~~~~
isSrcOver == true
isSrcOver != true
~~~~

</fiddle-embed></div>

<a name='Path_Effect_Methods'></a>

---

<a href='#Path_Effect'>Path_Effect</a> modifies the <a href='SkPath_Reference#Path'>path</a> geometry before drawing it.
<a href='#Path_Effect'>Path_Effect</a> may implement dashing, custom fill effects and custom stroke effects.
If <a href='SkPaint_Reference#Paint'>Paint</a> has no <a href='#Path_Effect'>Path_Effect</a>, the <a href='SkPath_Reference#Path'>path</a> geometry is unaltered when filled or stroked.

### Example

<div><fiddle-embed name="@Path_Effect_Methods"></fiddle-embed></div>

### See Also

<a href='#Path_Effect'>Path_Effect</a>

<a name='SkPaint_getPathEffect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkPathEffect'>SkPathEffect</a>* <a href='#SkPaint_getPathEffect'>getPathEffect</a>()const
</pre>

Returns <a href='undocumented#SkPathEffect'>SkPathEffect</a> if set, or nullptr.
Does not alter <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkPathEffect'>SkPathEffect</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="@Paint_getPathEffect">

#### Example Output

~~~~
nullptr == path effect
nullptr != path effect
~~~~

</fiddle-embed></div>

<a name='SkPaint_refPathEffect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkPathEffect'>SkPathEffect</a>&gt; <a href='#SkPaint_refPathEffect'>refPathEffect</a>()const
</pre>

Returns <a href='undocumented#SkPathEffect'>SkPathEffect</a> if set, or nullptr.
Increases <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Return Value

<a href='undocumented#SkPathEffect'>SkPathEffect</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="@Paint_refPathEffect">

#### Example Output

~~~~
path effect unique: true
path effect unique: false
~~~~

</fiddle-embed></div>

<a name='SkPaint_setPathEffect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setPathEffect'>setPathEffect</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkPathEffect'>SkPathEffect</a>&gt; pathEffect)
</pre>

Sets <a href='undocumented#SkPathEffect'>SkPathEffect</a> to <a href='#SkPaint_setPathEffect_pathEffect'>pathEffect</a>, decreasing <a href='undocumented#SkRefCnt'>SkRefCnt</a> of the previous
<a href='undocumented#SkPathEffect'>SkPathEffect</a>. Pass nullptr to leave the <a href='SkPath_Reference#Path'>path</a> geometry unaltered.

Increments <a href='#SkPaint_setPathEffect_pathEffect'>pathEffect</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setPathEffect_pathEffect'><code><strong>pathEffect</strong></code></a></td>
    <td>replace <a href='SkPath_Reference#SkPath'>SkPath</a> with a modification when drawn</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_setPathEffect"></fiddle-embed></div>

<a name='Mask_Filter_Methods'></a>

---

<a href='#Mask_Filter'>Mask_Filter</a> uses coverage of the shape drawn to create <a href='#Mask_Alpha'>Mask_Alpha</a>.
<a href='#Mask_Filter'>Mask_Filter</a> takes a Mask, and returns a Mask.

<a href='#Mask_Filter'>Mask_Filter</a> may change the geometry and transparency of the shape, such as
creating a blur effect. Set <a href='#Mask_Filter'>Mask_Filter</a> to nullptr to prevent <a href='#Mask_Filter'>Mask_Filter</a> from
modifying the draw.

### Example

<div><fiddle-embed name="@Mask_Filter_Methods"></fiddle-embed></div>

<a name='SkPaint_getMaskFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>* <a href='#SkPaint_getMaskFilter'>getMaskFilter</a>()const
</pre>

Returns <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> if set, or nullptr.
Does not alter <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkMaskFilter'>SkMaskFilter</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="@Paint_getMaskFilter">

#### Example Output

~~~~
nullptr == mask filter
nullptr != mask filter
~~~~

</fiddle-embed></div>

<a name='SkPaint_refMaskFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>&gt; <a href='#SkPaint_refMaskFilter'>refMaskFilter</a>()const
</pre>

Returns <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> if set, or nullptr.

Increases <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Return Value

<a href='undocumented#SkMaskFilter'>SkMaskFilter</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="@Paint_refMaskFilter">

#### Example Output

~~~~
mask filter unique: true
mask filter unique: false
~~~~

</fiddle-embed></div>

<a name='SkPaint_setMaskFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setMaskFilter'>setMaskFilter</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>&gt; maskFilter)
</pre>

Sets <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> to <a href='#SkPaint_setMaskFilter_maskFilter'>maskFilter</a>, decreasing <a href='undocumented#SkRefCnt'>SkRefCnt</a> of the previous
<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>. Pass nullptr to clear <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> and leave <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> effect on
<a href='undocumented#Mask_Alpha'>mask alpha</a> unaltered.

Increments <a href='#SkPaint_setMaskFilter_maskFilter'>maskFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setMaskFilter_maskFilter'><code><strong>maskFilter</strong></code></a></td>
    <td>modifies clipping mask generated from drawn geometry</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_setMaskFilter"></fiddle-embed></div>

<a name='Typeface_Methods'></a>

---

<a href='undocumented#Typeface'>Typeface</a> identifies the <a href='SkFont_Reference#Font'>font</a> used when drawing and measuring <a href='undocumented#Text'>text</a>.
<a href='undocumented#Typeface'>Typeface</a> may be specified by name, from a file, or from a <a href='undocumented#Data'>data</a> <a href='SkStream_Reference#Stream'>stream</a>.
The default <a href='undocumented#Typeface'>Typeface</a> defers to the platform-specific default <a href='SkFont_Reference#Font'>font</a>
implementation.

### Example

<div><fiddle-embed name="1a7a5062725139760962582f599f1b97"></fiddle-embed></div>

<a name='SkPaint_getTypeface'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkTypeface'>SkTypeface</a>* <a href='#SkPaint_getTypeface'>getTypeface</a>()const
</pre>

Returns <a href='undocumented#SkTypeface'>SkTypeface</a> if set, or nullptr.
Does not alter <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkTypeface'>SkTypeface</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="5ce718e5a184baaac80e7098d7dad67b">

#### Example Output

~~~~
nullptr == typeface
nullptr != typeface
~~~~

</fiddle-embed></div>

<a name='SkPaint_refTypeface'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkTypeface'>SkTypeface</a>&gt; <a href='#SkPaint_refTypeface'>refTypeface</a>()const
</pre>

Increases <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Return Value

<a href='undocumented#SkTypeface'>SkTypeface</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="8b5aa7e555a0dc31be69db7cadf471a1">

#### Example Output

~~~~
typeface1 != typeface2
typeface1 == typeface2
~~~~

</fiddle-embed></div>

<a name='SkPaint_setTypeface'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTypeface'>setTypeface</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkTypeface'>SkTypeface</a>&gt; <a href='undocumented#Typeface'>typeface</a>)
</pre>

Sets <a href='undocumented#SkTypeface'>SkTypeface</a> to <a href='#SkPaint_setTypeface_typeface'>typeface</a>, decreasing <a href='undocumented#SkRefCnt'>SkRefCnt</a> of the previous <a href='undocumented#SkTypeface'>SkTypeface</a>.
Pass nullptr to clear <a href='undocumented#SkTypeface'>SkTypeface</a> and use the default <a href='#SkPaint_setTypeface_typeface'>typeface</a>. Increments
<a href='#SkPaint_setTypeface_typeface'>typeface</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTypeface_typeface'><code><strong>typeface</strong></code></a></td>
    <td><a href='SkFont_Reference#Font'>font</a> and style used to draw <a href='undocumented#Text'>text</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0e6fbb7773cd925b274552f4cd1abef2"></fiddle-embed></div>

<a name='Image_Filter_Methods'></a>

---

<a href='#Image_Filter'>Image_Filter</a> operates on the <a href='undocumented#Pixel'>pixel</a> representation of the shape, as modified by <a href='SkPaint_Reference#Paint'>Paint</a>
with <a href='#Blend_Mode'>Blend_Mode</a> set to <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>. <a href='#Image_Filter'>Image_Filter</a> creates a new <a href='SkBitmap_Reference#Bitmap'>bitmap</a>,
which is drawn to the <a href='undocumented#Device'>device</a> using the set <a href='#Blend_Mode'>Blend_Mode</a>.

<a href='#Image_Filter'>Image_Filter</a> is higher level than <a href='#Mask_Filter'>Mask_Filter</a>; for instance, an <a href='#Image_Filter'>Image_Filter</a>
can operate on all channels of <a href='SkColor_Reference#Color'>Color</a>, while <a href='#Mask_Filter'>Mask_Filter</a> generates <a href='SkColor_Reference#Alpha'>Alpha</a> only.
<a href='#Image_Filter'>Image_Filter</a> operates independently of and can be used in combination with
<a href='#Mask_Filter'>Mask_Filter</a>.

### Example

<div><fiddle-embed name="@Image_Filter_Methods"></fiddle-embed></div>

<a name='SkPaint_getImageFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='#SkPaint_getImageFilter'>getImageFilter</a>()const
</pre>

Returns <a href='undocumented#SkImageFilter'>SkImageFilter</a> if set, or nullptr.
Does not alter <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkImageFilter'>SkImageFilter</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="@Paint_getImageFilter">

#### Example Output

~~~~
nullptr == image filter
nullptr != image filter
~~~~

</fiddle-embed></div>

<a name='SkPaint_refImageFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkImageFilter'>SkImageFilter</a>&gt; <a href='#SkPaint_refImageFilter'>refImageFilter</a>()const
</pre>

Returns <a href='undocumented#SkImageFilter'>SkImageFilter</a> if set, or nullptr.
Increases <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Return Value

<a href='undocumented#SkImageFilter'>SkImageFilter</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="@Paint_refImageFilter">

#### Example Output

~~~~
image filter unique: true
image filter unique: false
~~~~

</fiddle-embed></div>

<a name='SkPaint_setImageFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setImageFilter'>setImageFilter</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkImageFilter'>SkImageFilter</a>&gt; imageFilter)
</pre>

Sets <a href='undocumented#SkImageFilter'>SkImageFilter</a> to <a href='#SkPaint_setImageFilter_imageFilter'>imageFilter</a>, decreasing <a href='undocumented#SkRefCnt'>SkRefCnt</a> of the previous
<a href='undocumented#SkImageFilter'>SkImageFilter</a>. Pass nullptr to clear <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and remove <a href='undocumented#SkImageFilter'>SkImageFilter</a> effect
on drawing.

Increments <a href='#SkPaint_setImageFilter_imageFilter'>imageFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setImageFilter_imageFilter'><code><strong>imageFilter</strong></code></a></td>
    <td>how <a href='SkImage_Reference#SkImage'>SkImage</a> is sampled when transformed</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6679d6e4ec632715ee03e68391bd7f9a"></fiddle-embed></div>

<a name='Draw_Looper_Methods'></a>

---

<a href='#Draw_Looper'>Draw_Looper</a> sets a modifier that communicates state from one <a href='#Draw_Layer'>Draw_Layer</a>
to another to construct the draw.

<a href='#Draw_Looper'>Draw_Looper</a> draws one or more times, modifying the <a href='SkCanvas_Reference#Canvas'>canvas</a> and <a href='SkPaint_Reference#Paint'>paint</a> each time.
<a href='#Draw_Looper'>Draw_Looper</a> may be used to draw multiple colors or create a colored shadow.
Set <a href='#Draw_Looper'>Draw_Looper</a> to nullptr to prevent <a href='#Draw_Looper'>Draw_Looper</a> from modifying the draw.

### Example

<div><fiddle-embed name="@Draw_Looper_Methods"></fiddle-embed></div>

<a name='SkPaint_getDrawLooper'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>* <a href='#SkPaint_getDrawLooper'>getDrawLooper</a>()const
</pre>

Returns <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> if set, or nullptr.
Does not alter <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkDrawLooper'>SkDrawLooper</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="@Paint_getDrawLooper">

#### Example Output

~~~~
nullptr == draw looper
nullptr != draw looper
~~~~

</fiddle-embed></div>

<a name='SkPaint_refDrawLooper'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>&gt; <a href='#SkPaint_refDrawLooper'>refDrawLooper</a>()const
</pre>

Returns <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> if set, or nullptr.
Increases <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Return Value

<a href='undocumented#SkDrawLooper'>SkDrawLooper</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="@Paint_refDrawLooper">

#### Example Output

~~~~
draw looper unique: true
draw looper unique: false
~~~~

</fiddle-embed></div>

<a name='SkPaint_setDrawLooper'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setDrawLooper'>setDrawLooper</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>&gt; drawLooper)
</pre>

Sets <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> to <a href='#SkPaint_setDrawLooper_drawLooper'>drawLooper</a>, decreasing <a href='undocumented#SkRefCnt'>SkRefCnt</a> of the previous
<a href='#SkPaint_setDrawLooper_drawLooper'>drawLooper</a>.  Pass nullptr to clear <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> and leave <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> effect on
drawing unaltered.

Increments <a href='#SkPaint_setDrawLooper_drawLooper'>drawLooper</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setDrawLooper_drawLooper'><code><strong>drawLooper</strong></code></a></td>
    <td>iterates through drawing one or more time, altering <a href='SkPaint_Reference#SkPaint'>SkPaint</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Paint_setDrawLooper"></fiddle-embed></div>

<a name='Text_Size'></a>

---

<a href='#Paint_Text_Size'>Text_Size</a> adjusts the overall  <a href='#Text_Size'>text size</a> in <a href='SkPoint_Reference#Point'>points</a>.
<a href='#Paint_Text_Size'>Text_Size</a> can be set to any positive value or zero.
<a href='#Paint_Text_Size'>Text_Size</a> defaults to 12.
Set <a href='undocumented#SkPaintDefaults_TextSize'>SkPaintDefaults_TextSize</a> at compile time to change the default setting.

### Example

<div><fiddle-embed name="91c9a3e498bb9412e4522a95d076ed5f"></fiddle-embed></div>

<a name='SkPaint_getTextSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSize'>getTextSize</a>()const
</pre>

Returns <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a> in <a href='SkPoint_Reference#Point'>points</a>.

### Return Value

typographic height of <a href='undocumented#Text'>text</a>

### Example

<div><fiddle-embed name="983e2a71ba72d4ba8c945420040b8f1c"></fiddle-embed></div>

<a name='SkPaint_setTextSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextSize'>setTextSize</a>(<a href='undocumented#SkScalar'>SkScalar</a> textSize)
</pre>

Sets  <a href='#Text_Size'>text size</a> in <a href='SkPoint_Reference#Point'>points</a>.
Has no effect if <a href='#SkPaint_setTextSize_textSize'>textSize</a> is not greater than or equal to zero.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextSize_textSize'><code><strong>textSize</strong></code></a></td>
    <td>typographic height of <a href='undocumented#Text'>text</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6510c9e2f57b83c47e67829e7a68d493"></fiddle-embed></div>

<a name='Text_Scale_X'></a>

---

<a href='#Paint_Text_Scale_X'>Text_Scale_X</a> adjusts the <a href='undocumented#Text'>text</a> horizontal scale.
<a href='undocumented#Text'>Text</a> scaling approximates condensed and expanded type faces when the actual face
is not available.
<a href='#Paint_Text_Scale_X'>Text_Scale_X</a> can be set to any value.
<a href='#Paint_Text_Scale_X'>Text_Scale_X</a> defaults to 1.

### Example

<div><fiddle-embed name="d13d787c1e36f515319fc998411c1d91"></fiddle-embed></div>

<a name='SkPaint_getTextScaleX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextScaleX'>getTextScaleX</a>()const
</pre>

Returns <a href='undocumented#Text'>text</a> scale on x-axis.
Default value is 1.

### Return Value

<a href='undocumented#Text'>text</a> horizontal scale

### Example

<div><fiddle-embed name="5dc8e58f6910cb8e4de9ed60f888188b"></fiddle-embed></div>

<a name='SkPaint_setTextScaleX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextScaleX'>setTextScaleX</a>(<a href='undocumented#SkScalar'>SkScalar</a> scaleX)
</pre>

Sets  <a href='undocumented#Text'>text scale</a> on x-axis.
Default value is 1.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextScaleX_scaleX'><code><strong>scaleX</strong></code></a></td>
    <td><a href='undocumented#Text'>text</a> horizontal scale</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a75bbdb8bb866b125c4c1dd5e967d470"></fiddle-embed></div>

<a name='Text_Skew_X'></a>

---

<a href='#Paint_Text_Skew_X'>Text_Skew_X</a> adjusts the <a href='undocumented#Text'>text</a> horizontal slant.
<a href='undocumented#Text'>Text</a> skewing approximates italic and oblique type faces when the actual face
is not available.
<a href='#Paint_Text_Skew_X'>Text_Skew_X</a> can be set to any value.
<a href='#Paint_Text_Skew_X'>Text_Skew_X</a> defaults to 0.

### Example

<div><fiddle-embed name="aff208b0aab265f273045b27e683c17c"></fiddle-embed></div>

<a name='SkPaint_getTextSkewX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSkewX'>getTextSkewX</a>()const
</pre>

Returns <a href='undocumented#Text'>text</a> skew on x-axis.
Default value is zero.

### Return Value

additional shear on x-axis relative to y-axis

### Example

<div><fiddle-embed name="11c10f466dae0d1639dbb9f6a0040218"></fiddle-embed></div>

<a name='SkPaint_setTextSkewX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextSkewX'>setTextSkewX</a>(<a href='undocumented#SkScalar'>SkScalar</a> skewX)
</pre>

Sets  <a href='undocumented#Text'>text skew</a> on x-axis.
Default value is zero.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextSkewX_skewX'><code><strong>skewX</strong></code></a></td>
    <td>additional shear on x-axis relative to y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6bd705a6e0c5f8ee24f302fe531bfabc"></fiddle-embed></div>

<a name='Text_Encoding'></a>

---

### Example

<div><fiddle-embed name="767fa4e7b6300e16a419f9881f0f9d3d"><div>First <a href='undocumented#Line'>line</a> is encoded in UTF-8.
Second <a href='undocumented#Line'>line</a> is encoded in UTF-16.
Third <a href='undocumented#Line'>line</a> is encoded in UTF-32.
Fourth <a href='undocumented#Line'>line</a> has 16-bit <a href='undocumented#Glyph'>glyph</a> indices.
</div></fiddle-embed></div>

<a name='SkPaint_getTextEncoding'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkTextEncoding'>SkTextEncoding</a> <a href='#SkPaint_getTextEncoding'>getTextEncoding</a>()const
</pre>

Returns the <a href='undocumented#Text'>text</a> encoding. <a href='undocumented#Text'>Text</a> encoding describes how to interpret the <a href='undocumented#Text'>text</a> bytes pass
to methods like <a href='#SkPaint_measureText'>measureText</a>() and <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawText'>drawText</a>().

### Return Value

the <a href='undocumented#Text'>text</a> encoding

### Example

<div><fiddle-embed name="0d21e968e9a4c78c902ae3ef494941a0">

#### Example Output

~~~~
SkTextEncoding::kUTF8 == text encoding
SkTextEncoding::kGlyphID == text encoding
~~~~

</fiddle-embed></div>

<a name='SkPaint_setTextEncoding'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextEncoding'>setTextEncoding</a>(<a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding)
</pre>

Sets the  <a href='#Text_Encoding'>text encoding</a>. <a href='undocumented#Text'>Text</a> <a href='#SkPaint_setTextEncoding_encoding'>encoding</a> describes how to interpret the <a href='undocumented#Text'>text</a> bytes pass
to methods like <a href='#SkPaint_measureText'>measureText</a>() and <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawText'>drawText</a>().

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextEncoding_encoding'><code><strong>encoding</strong></code></a></td>
    <td>the new  <a href='#Text_Encoding'>text encoding</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a5d1ba0dbf42afb797ffdb07647b5cb9">

#### Example Output

~~~~
4 != text encoding
~~~~

</fiddle-embed></div>

<a name='SkPaint_FontMetrics'></a>

---

<a name='Font_Metrics'></a>

<a name='SkPaint_getFontMetrics'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getFontMetrics'>getFontMetrics</a>(<a href='undocumented#SkFontMetrics'>SkFontMetrics</a>* metrics)const
</pre>

Returns <a href='undocumented#SkFontMetrics'>SkFontMetrics</a> associated with <a href='undocumented#SkTypeface'>SkTypeface</a>.
The return value is the recommended spacing between <a href='undocumented#Line'>lines</a>: the sum of <a href='#SkPaint_getFontMetrics_metrics'>metrics</a>
descent, ascent, and leading.
If <a href='#SkPaint_getFontMetrics_metrics'>metrics</a> is not nullptr, <a href='undocumented#SkFontMetrics'>SkFontMetrics</a> is copied to <a href='#SkPaint_getFontMetrics_metrics'>metrics</a>.
Results are scaled by  <a href='#Text_Size'>text size</a> but does not take into account
dimensions required by   <a href='#Text_Scale_X'>text scale x</a>,   <a href='#Text_Skew_X'>text skew x</a>,  <a href='#Fake_Bold'>fake bold</a>,
<a href='#Style_Stroke'>style stroke</a>, and <a href='undocumented#SkPathEffect'>SkPathEffect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getFontMetrics_metrics'><code><strong>metrics</strong></code></a></td>
    <td>storage for <a href='undocumented#SkFontMetrics'>SkFontMetrics</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

recommended spacing between <a href='undocumented#Line'>lines</a>

### Example

<div><fiddle-embed name="59d9b8249afa1c2af6186711250ce240"></fiddle-embed></div>

### See Also

<a href='#Font_Size'>Font_Size</a> <a href='undocumented#Typeface'>Typeface</a> <a href='#Paint_Typeface_Methods'>Typeface_Methods</a>

<a name='SkPaint_getFontSpacing'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getFontSpacing'>getFontSpacing</a>()const
</pre>

Returns the recommended spacing between <a href='undocumented#Line'>lines</a>: the sum of metrics
descent, ascent, and leading.
Result is scaled by <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a> but does not take into account
dimensions required by stroking and <a href='undocumented#SkPathEffect'>SkPathEffect</a>.
Returns the same result as <a href='#SkPaint_getFontMetrics'>getFontMetrics</a>().

### Return Value

recommended spacing between <a href='undocumented#Line'>lines</a>

### Example

<div><fiddle-embed name="424741e26e1b174e43087d67422ce14f">

#### Example Output

~~~~
textSize: 12 fontSpacing: 13.9688
textSize: 18 fontSpacing: 20.9531
textSize: 24 fontSpacing: 27.9375
textSize: 32 fontSpacing: 37.25
~~~~

</fiddle-embed></div>

<a name='SkPaint_textToGlyphs'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_textToGlyphs'>textToGlyphs</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, <a href='undocumented#SkGlyphID'>SkGlyphID</a> <a href='undocumented#Glyph'>glyphs</a>[])const
</pre>

Converts <a href='#SkPaint_textToGlyphs_text'>text</a> into <a href='undocumented#Glyph'>glyph</a> indices.
Returns the number of <a href='undocumented#Glyph'>glyph</a> indices represented by <a href='#SkPaint_textToGlyphs_text'>text</a>.
<a href='undocumented#SkTextEncoding'>SkTextEncoding</a> specifies how <a href='#SkPaint_textToGlyphs_text'>text</a> represents characters or <a href='#SkPaint_textToGlyphs_glyphs'>glyphs</a>.
<a href='#SkPaint_textToGlyphs_glyphs'>glyphs</a> may be nullptr, to compute the <a href='undocumented#Glyph'>glyph</a> count.

Does not check <a href='#SkPaint_textToGlyphs_text'>text</a> for valid character codes or valid <a href='undocumented#Glyph'>glyph</a> indices.

If <a href='#SkPaint_textToGlyphs_byteLength'>byteLength</a> equals zero, returns zero.
If <a href='#SkPaint_textToGlyphs_byteLength'>byteLength</a> includes a partial character, the partial character is ignored.

If <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> is <a href='undocumented#SkTextEncoding::kUTF8'>SkTextEncoding::kUTF8</a> and
<a href='#SkPaint_textToGlyphs_text'>text</a> contains an invalid UTF-8 sequence, zero is returned.

### Parameters

<table>  <tr>    <td><a name='SkPaint_textToGlyphs_text'><code><strong>text</strong></code></a></td>
    <td>character storage encoded with <a href='undocumented#SkTextEncoding'>SkTextEncoding</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_textToGlyphs_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>length of character storage in bytes</td>
  </tr>
  <tr>    <td><a name='SkPaint_textToGlyphs_glyphs'><code><strong>glyphs</strong></code></a></td>
    <td>storage for <a href='undocumented#Glyph'>glyph</a> indices; may be nullptr</td>
  </tr>
</table>

### Return Value

number of <a href='#SkPaint_textToGlyphs_glyphs'>glyphs</a> represented by <a href='#SkPaint_textToGlyphs_text'>text</a> of length <a href='#SkPaint_textToGlyphs_byteLength'>byteLength</a>

### Example

<div><fiddle-embed name="d11136d8a74f63009da2a7f550710823"></fiddle-embed></div>

<a name='SkPaint_countText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_countText'>countText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength)const
</pre>

Returns the number of <a href='undocumented#Glyph'>glyphs</a> in <a href='#SkPaint_countText_text'>text</a>.
Uses <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> to count the <a href='undocumented#Glyph'>glyphs</a>.
Returns the same result as <a href='#SkPaint_textToGlyphs'>textToGlyphs</a>().

### Parameters

<table>  <tr>    <td><a name='SkPaint_countText_text'><code><strong>text</strong></code></a></td>
    <td>character storage encoded with <a href='undocumented#SkTextEncoding'>SkTextEncoding</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_countText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>length of character storage in bytes</td>
  </tr>
</table>

### Return Value

number of <a href='undocumented#Glyph'>glyphs</a> represented by <a href='#SkPaint_countText_text'>text</a> of length <a href='#SkPaint_countText_byteLength'>byteLength</a>

### Example

<div><fiddle-embed name="85436c71aab5410767fc688ab0573e09">

#### Example Output

~~~~
count = 5
~~~~

</fiddle-embed></div>

<a name='SkPaint_containsText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_containsText'>containsText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength)const
</pre>

Returns true if all <a href='#SkPaint_containsText_text'>text</a> corresponds to a non-zero <a href='undocumented#Glyph'>glyph</a> index.
Returns false if any characters in <a href='#SkPaint_containsText_text'>text</a> are not supported in
<a href='undocumented#SkTypeface'>SkTypeface</a>.

If <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> is <a href='undocumented#SkTextEncoding::kGlyphID'>SkTextEncoding::kGlyphID</a>,
returns true if all <a href='undocumented#Glyph'>glyph</a> indices in <a href='#SkPaint_containsText_text'>text</a> are non-zero;
does not check to see if <a href='#SkPaint_containsText_text'>text</a> contains valid <a href='undocumented#Glyph'>glyph</a> indices for <a href='undocumented#SkTypeface'>SkTypeface</a>.

Returns true if <a href='#SkPaint_containsText_byteLength'>byteLength</a> is zero.

### Parameters

<table>  <tr>    <td><a name='SkPaint_containsText_text'><code><strong>text</strong></code></a></td>
    <td>array of characters or <a href='undocumented#Glyph'>glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_containsText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>number of bytes in <a href='#SkPaint_containsText_text'>text</a> array</td>
  </tr>
</table>

### Return Value

true if all <a href='#SkPaint_containsText_text'>text</a> corresponds to a non-zero <a href='undocumented#Glyph'>glyph</a> index

<div><a href='#SkPaint_containsText'>containsText</a> succeeds for degree symbol, but cannot find a <a href='undocumented#Glyph'>glyph</a> index
corresponding to the Unicode surrogate code <a href='SkPoint_Reference#Point'>point</a>.
</div>

#### Example Output

~~~~
0x00b0 == has char
0xd800 != has char
~~~~

### Example

<div><fiddle-embed name="6a68cb3c8b81a5976c81ee004f559247"><div><a href='#SkPaint_containsText'>containsText</a> returns true that <a href='undocumented#Glyph'>glyph</a> index is greater than zero, not
that it corresponds to an entry in <a href='undocumented#Typeface'>Typeface</a>.
</div>

#### Example Output

~~~~
0x01ff == has glyph
0x0000 != has glyph
0xffff == has glyph
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_setTextEncoding'>setTextEncoding</a> <a href='undocumented#Typeface'>Typeface</a>

<a name='SkPaint_glyphsToUnichars'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_glyphsToUnichars'>glyphsToUnichars</a>(const <a href='undocumented#SkGlyphID'>SkGlyphID</a> <a href='undocumented#Glyph'>glyphs</a>[], int count, <a href='undocumented#SkUnichar'>SkUnichar</a> <a href='undocumented#Text'>text</a>[])const
</pre>

Converts <a href='#SkPaint_glyphsToUnichars_glyphs'>glyphs</a> into <a href='#SkPaint_glyphsToUnichars_text'>text</a> if possible.
<a href='undocumented#Glyph'>Glyph</a> values without direct Unicode equivalents are mapped to zero.
Uses the <a href='undocumented#SkTypeface'>SkTypeface</a>, but is unaffected
by <a href='undocumented#SkTextEncoding'>SkTextEncoding</a>; the <a href='#SkPaint_glyphsToUnichars_text'>text</a> values returned are equivalent to <a href='undocumented#SkTextEncoding::kUTF32'>SkTextEncoding::kUTF32</a>.

Only supported on platforms that use FreeType as the  <a href='SkFont_Reference#Font_Engine'>font engine</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_glyphsToUnichars_glyphs'><code><strong>glyphs</strong></code></a></td>
    <td>array of indices into <a href='SkFont_Reference#Font'>font</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_glyphsToUnichars_count'><code><strong>count</strong></code></a></td>
    <td>length of <a href='undocumented#Glyph'>glyph</a> array</td>
  </tr>
  <tr>    <td><a name='SkPaint_glyphsToUnichars_text'><code><strong>text</strong></code></a></td>
    <td>storage for character codes, one per <a href='undocumented#Glyph'>glyph</a></td>
  </tr>
</table>

<div>Convert UTF-8 <a href='#SkPaint_glyphsToUnichars_text'>text</a> to <a href='#SkPaint_glyphsToUnichars_glyphs'>glyphs</a>; then convert <a href='#SkPaint_glyphsToUnichars_glyphs'>glyphs</a> to Unichar code <a href='SkPoint_Reference#Point'>points</a>.
</div>

<a name='Measure_Text'></a>

<a name='SkPaint_measureText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_measureText'>measureText</a>(const void* <a href='undocumented#Text'>text</a>, size_t length, <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds)const
</pre>

Returns the advance width of <a href='#SkPaint_measureText_text'>text</a>.
The advance is the normal distance to move before drawing additional <a href='#SkPaint_measureText_text'>text</a>.
Uses <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> to decode <a href='#SkPaint_measureText_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> to get the  <a href='#Font_Metrics'>font metrics</a>,
and  <a href='#Text_Size'>text size</a>,   <a href='#Text_Scale_X'>text scale x</a>,   <a href='#Text_Skew_X'>text skew x</a>,  <a href='#Stroke_Width'>stroke width</a>, and
<a href='undocumented#SkPathEffect'>SkPathEffect</a> to scale the metrics and <a href='#SkPaint_measureText_bounds'>bounds</a>.
Returns the bounding box of <a href='#SkPaint_measureText_text'>text</a> if <a href='#SkPaint_measureText_bounds'>bounds</a> is not nullptr.
The bounding box is computed as if the <a href='#SkPaint_measureText_text'>text</a> was drawn at the origin.

### Parameters

<table>  <tr>    <td><a name='SkPaint_measureText_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> indices to be measured</td>
  </tr>
  <tr>    <td><a name='SkPaint_measureText_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_measureText_text'>text</a> to measure</td>
  </tr>
  <tr>    <td><a name='SkPaint_measureText_bounds'><code><strong>bounds</strong></code></a></td>
    <td>returns bounding box relative to (0, 0) if not nullptr</td>
  </tr>
</table>

### Return Value

advance width or height

### Example

<div><fiddle-embed name="06084f609184470135a9cd9ebc5af149"></fiddle-embed></div>

<a name='SkPaint_measureText_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_measureText'>measureText</a>(const void* <a href='undocumented#Text'>text</a>, size_t length)const
</pre>

Returns the advance width of <a href='#SkPaint_measureText_2_text'>text</a>.
The advance is the normal distance to move before drawing additional <a href='#SkPaint_measureText_2_text'>text</a>.
Uses <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> to decode <a href='#SkPaint_measureText_2_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> to get the  <a href='#Font_Metrics'>font metrics</a>,
and  <a href='#Text_Size'>text size</a> to scale the metrics.
Does not scale the advance or bounds by  <a href='#Fake_Bold'>fake bold</a> or <a href='undocumented#SkPathEffect'>SkPathEffect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_measureText_2_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> indices to be measured</td>
  </tr>
  <tr>    <td><a name='SkPaint_measureText_2_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_measureText_2_text'>text</a> to measure</td>
  </tr>
</table>

### Return Value

advance width or height

### Example

<div><fiddle-embed name="f1139a5ddd17fd47c2f45f6e642cac76">

#### Example Output

~~~~
default width = 5
double width = 10
~~~~

</fiddle-embed></div>

<a name='SkPaint_getTextWidths'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getTextWidths'>getTextWidths</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, <a href='undocumented#SkScalar'>SkScalar</a> widths[], <a href='SkRect_Reference#SkRect'>SkRect</a> bounds[] = nullptr)const
</pre>

Retrieves the advance and <a href='#SkPaint_getTextWidths_bounds'>bounds</a> for each <a href='undocumented#Glyph'>glyph</a> in <a href='#SkPaint_getTextWidths_text'>text</a>, and returns
the <a href='undocumented#Glyph'>glyph</a> count in <a href='#SkPaint_getTextWidths_text'>text</a>.
Both <a href='#SkPaint_getTextWidths_widths'>widths</a> and <a href='#SkPaint_getTextWidths_bounds'>bounds</a> may be nullptr.
If <a href='#SkPaint_getTextWidths_widths'>widths</a> is not nullptr, <a href='#SkPaint_getTextWidths_widths'>widths</a> must be an array of <a href='undocumented#Glyph'>glyph</a> count entries.
if <a href='#SkPaint_getTextWidths_bounds'>bounds</a> is not nullptr, <a href='#SkPaint_getTextWidths_bounds'>bounds</a> must be an array of <a href='undocumented#Glyph'>glyph</a> count entries.
Uses <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> to decode <a href='#SkPaint_getTextWidths_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> to get the  <a href='#Font_Metrics'>font metrics</a>,
and  <a href='#Text_Size'>text size</a> to scale the <a href='#SkPaint_getTextWidths_widths'>widths</a> and <a href='#SkPaint_getTextWidths_bounds'>bounds</a>.
Does not scale the advance by  <a href='#Fake_Bold'>fake bold</a> or <a href='undocumented#SkPathEffect'>SkPathEffect</a>.
Does include  <a href='#Fake_Bold'>fake bold</a> and <a href='undocumented#SkPathEffect'>SkPathEffect</a> in the <a href='#SkPaint_getTextWidths_bounds'>bounds</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextWidths_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> indices to be measured</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextWidths_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getTextWidths_text'>text</a> to measure</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextWidths_widths'><code><strong>widths</strong></code></a></td>
    <td>returns <a href='#SkPaint_getTextWidths_text'>text</a> advances for each <a href='undocumented#Glyph'>glyph</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextWidths_bounds'><code><strong>bounds</strong></code></a></td>
    <td>returns <a href='#SkPaint_getTextWidths_bounds'>bounds</a> for each <a href='undocumented#Glyph'>glyph</a> relative to (0, 0); may be nullptr</td>
  </tr>
</table>

### Return Value

<a href='undocumented#Glyph'>glyph</a> count in <a href='#SkPaint_getTextWidths_text'>text</a>

### Example

<div><fiddle-embed name="6b9e101f49e9c2c28755c5bdcef64dfb"><div>Bounds of <a href='undocumented#Glyph'>Glyphs</a> increase for stroked <a href='#SkPaint_getTextWidths_text'>text</a>, but <a href='#SkPaint_getTextWidths_text'>text</a> advance remains the same.
The underlines show the <a href='#SkPaint_getTextWidths_text'>text</a> advance, spaced to keep them distinct.
</div></fiddle-embed></div>

<a name='Text_Path'></a>

<a href='#Paint_Text_Path'>Text_Path</a> describes the geometry of <a href='undocumented#Glyph'>Glyphs</a> used to draw <a href='undocumented#Text'>text</a>.

<a name='SkPaint_getTextPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_getTextPath'>getTextPath</a>(const void* <a href='undocumented#Text'>text</a>, size_t length, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#Path'>path</a>)const
</pre>

Returns the geometry as <a href='SkPath_Reference#SkPath'>SkPath</a> equivalent to the drawn <a href='#SkPaint_getTextPath_text'>text</a>.
Uses <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> to decode <a href='#SkPaint_getTextPath_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> to get the <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>,
and  <a href='#Text_Size'>text size</a>,  <a href='#Fake_Bold'>fake bold</a>, and <a href='undocumented#SkPathEffect'>SkPathEffect</a> to scale and modify the <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>.
All of the <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a> are stored in <a href='#SkPaint_getTextPath_path'>path</a>.
Uses <a href='#SkPaint_getTextPath_x'>x</a>, <a href='#SkPaint_getTextPath_y'>y</a>, to position <a href='#SkPaint_getTextPath_path'>path</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextPath_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> indices</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextPath_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getTextPath_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextPath_x'><code><strong>x</strong></code></a></td>
    <td>x-axis value of the origin of the <a href='#SkPaint_getTextPath_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextPath_y'><code><strong>y</strong></code></a></td>
    <td>y-axis value of the origin of the <a href='#SkPaint_getTextPath_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextPath_path'><code><strong>path</strong></code></a></td>
    <td>geometry of the <a href='undocumented#Glyph'>glyphs</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7c9e6a399f898d68026c1f0865e6f73e"><div><a href='undocumented#Text'>Text</a> is added to <a href='SkPath_Reference#Path'>Path</a>, offset, and subtracted from <a href='SkPath_Reference#Path'>Path</a>, then added at
the offset location. The result is rendered with one draw call.
</div></fiddle-embed></div>

<a name='SkPaint_getPosTextPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_getPosTextPath'>getPosTextPath</a>(const void* <a href='undocumented#Text'>text</a>, size_t length, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pos[], <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#Path'>path</a>)const
</pre>

Returns the geometry as <a href='SkPath_Reference#SkPath'>SkPath</a> equivalent to the drawn <a href='#SkPaint_getPosTextPath_text'>text</a>.
Uses <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> to decode <a href='#SkPaint_getPosTextPath_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> to get the <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>,
and  <a href='#Text_Size'>text size</a>,  <a href='#Fake_Bold'>fake bold</a>, and <a href='undocumented#SkPathEffect'>SkPathEffect</a> to scale and modify the <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>.
All of the <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a> are stored in <a href='#SkPaint_getPosTextPath_path'>path</a>.
Uses <a href='#SkPaint_getPosTextPath_pos'>pos</a> array to position <a href='#SkPaint_getPosTextPath_path'>path</a>.
<a href='#SkPaint_getPosTextPath_pos'>pos</a> contains a position for each <a href='undocumented#Glyph'>glyph</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getPosTextPath_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> indices</td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextPath_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getPosTextPath_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextPath_pos'><code><strong>pos</strong></code></a></td>
    <td>positions of each <a href='undocumented#Glyph'>glyph</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextPath_path'><code><strong>path</strong></code></a></td>
    <td>geometry of the <a href='undocumented#Glyph'>glyphs</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7f27c93472aa99a7542fb3493076f072"><div>Simplifies three <a href='undocumented#Glyph'>Glyphs</a> to eliminate overlaps, and strokes the result.
</div></fiddle-embed></div>

<a name='SkPaint_nothingToDraw'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_nothingToDraw'>nothingToDraw</a>()const
</pre>

Returns true if <a href='SkPaint_Reference#SkPaint'>SkPaint</a> prevents all drawing;
otherwise, the <a href='SkPaint_Reference#SkPaint'>SkPaint</a> may or may not allow drawing.

Returns true if, for example, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> combined with <a href='SkColor_Reference#Alpha'>alpha</a> computes a
new <a href='SkColor_Reference#Alpha'>alpha</a> of zero.

### Return Value

true if <a href='SkPaint_Reference#SkPaint'>SkPaint</a> prevents all drawing

### Example

<div><fiddle-embed name="@Paint_nothingToDraw">

#### Example Output

~~~~
initial nothing to draw: false
blend dst nothing to draw: true
blend src over nothing to draw: false
alpha 0 nothing to draw: true
~~~~

</fiddle-embed></div>

<a name='Utility'></a>

