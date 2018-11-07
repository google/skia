SkPaint Reference
===


<a name='SkPaint'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkPaint_Reference#SkPaint'>SkPaint</a> {
<a href='SkPaint_Reference#SkPaint'>public</a>:
    <a href='#SkPaint_empty_constructor'>SkPaint()</a>;
    <a href='SkPaint_Reference#SkPaint'>SkPaint</a>(<a href='SkPaint_Reference#SkPaint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#SkPaint'>SkPaint</a>(<a href='SkPaint_Reference#SkPaint'>SkPaint</a>&& <a href='SkPaint_Reference#Paint'>paint</a>);
    ~<a href='#SkPaint_empty_constructor'>SkPaint()</a>;
    <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#SkPaint'>operator</a>=(<a href='SkPaint_Reference#SkPaint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#SkPaint'>operator</a>=(<a href='SkPaint_Reference#SkPaint'>SkPaint</a>&& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>friend</a> <a href='SkPaint_Reference#Paint'>bool</a> <a href='SkPaint_Reference#Paint'>operator</a>==(<a href='SkPaint_Reference#Paint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#SkPaint'>a</a>, <a href='SkPaint_Reference#SkPaint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#SkPaint'>b</a>);
    <a href='SkPaint_Reference#SkPaint'>friend</a> <a href='SkPaint_Reference#SkPaint'>bool</a> <a href='SkPaint_Reference#SkPaint'>operator</a>!=(<a href='SkPaint_Reference#SkPaint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#SkPaint'>a</a>, <a href='SkPaint_Reference#SkPaint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#SkPaint'>b</a>);
    <a href='SkPaint_Reference#SkPaint'>uint32_t</a> <a href='#SkPaint_getHash'>getHash</a>() <a href='#SkPaint_getHash'>const</a>;
    <a href='#SkPaint_getHash'>void</a> <a href='#SkPaint_reset'>reset()</a>;

    <a href='#SkPaint_reset'>enum</a> <a href='#SkPaint_Hinting'>Hinting</a> : <a href='#SkPaint_Hinting'>uint8_t</a> {
        <a href='#SkPaint_kNo_Hinting'>kNo_Hinting</a> = 0,
        <a href='#SkPaint_kSlight_Hinting'>kSlight_Hinting</a> = 1,
        <a href='#SkPaint_kNormal_Hinting'>kNormal_Hinting</a> = 2,
        <a href='#SkPaint_kFull_Hinting'>kFull_Hinting</a> = 3,
    };

    <a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_getHinting'>getHinting</a>() <a href='#SkPaint_getHinting'>const</a>;
    <a href='#SkPaint_getHinting'>void</a> <a href='#SkPaint_setHinting'>setHinting</a>(<a href='undocumented#SkFontHinting'>SkFontHinting</a> <a href='undocumented#SkFontHinting'>hintingLevel</a>);
   <a href='undocumented#SkFontHinting'>void</a> <a href='#SkPaint_setHinting'>setHinting</a>(<a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>hintingLevel</a>);

    <a href='#SkPaint_Hinting'>enum</a> <a href='#SkPaint_Flags'>Flags</a> {
        <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> = 0<a href='#SkPaint_kAntiAlias_Flag'>x01</a>,
        <a href='#SkPaint_kDither_Flag'>kDither_Flag</a> = 0<a href='#SkPaint_kDither_Flag'>x04</a>,
        <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a> = 0<a href='#SkPaint_kFakeBoldText_Flag'>x20</a>,
        <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> = 0<a href='#SkPaint_kLinearText_Flag'>x40</a>,
        <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> = 0<a href='#SkPaint_kSubpixelText_Flag'>x80</a>,
        <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> = 0<a href='#SkPaint_kLCDRenderText_Flag'>x200</a>,
        <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> = 0<a href='#SkPaint_kEmbeddedBitmapText_Flag'>x400</a>,
        <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a> = 0<a href='#SkPaint_kAutoHinting_Flag'>x800</a>,
        <a href='#SkPaint_kAllFlags'>kAllFlags</a> = 0<a href='#SkPaint_kAllFlags'>xFFFF</a>,
    };

    <a href='#SkPaint_kAllFlags'>enum</a> <a href='#SkPaint_ReserveFlags'>ReserveFlags</a> {
        <a href='#SkPaint_kUnderlineText_ReserveFlag'>kUnderlineText_ReserveFlag</a> = 0<a href='#SkPaint_kUnderlineText_ReserveFlag'>x08</a>,
        <a href='#SkPaint_kStrikeThruText_ReserveFlag'>kStrikeThruText_ReserveFlag</a> = 0<a href='#SkPaint_kStrikeThruText_ReserveFlag'>x10</a>,
    };

    <a href='#SkPaint_kStrikeThruText_ReserveFlag'>uint32_t</a> <a href='#SkPaint_getFlags'>getFlags</a>() <a href='#SkPaint_getFlags'>const</a>;
    <a href='#SkPaint_getFlags'>void</a> <a href='#SkPaint_setFlags'>setFlags</a>(<a href='#SkPaint_setFlags'>uint32_t</a> <a href='#SkPaint_setFlags'>flags</a>);
    <a href='#SkPaint_setFlags'>bool</a> <a href='#SkPaint_isAntiAlias'>isAntiAlias</a>() <a href='#SkPaint_isAntiAlias'>const</a>;
    <a href='#SkPaint_isAntiAlias'>void</a> <a href='#SkPaint_setAntiAlias'>setAntiAlias</a>(<a href='#SkPaint_setAntiAlias'>bool</a> <a href='#SkPaint_setAntiAlias'>aa</a>);
    <a href='#SkPaint_setAntiAlias'>bool</a> <a href='#SkPaint_isDither'>isDither</a>() <a href='#SkPaint_isDither'>const</a>;
    <a href='#SkPaint_isDither'>void</a> <a href='#SkPaint_setDither'>setDither</a>(<a href='#SkPaint_setDither'>bool</a> <a href='#SkPaint_setDither'>dither</a>);
    <a href='#SkPaint_setDither'>bool</a> <a href='#SkPaint_isLinearText'>isLinearText</a>() <a href='#SkPaint_isLinearText'>const</a>;
    <a href='#SkPaint_isLinearText'>void</a> <a href='#SkPaint_setLinearText'>setLinearText</a>(<a href='#SkPaint_setLinearText'>bool</a> <a href='#SkPaint_setLinearText'>linearText</a>);
    <a href='#SkPaint_setLinearText'>bool</a> <a href='#SkPaint_isSubpixelText'>isSubpixelText</a>() <a href='#SkPaint_isSubpixelText'>const</a>;
    <a href='#SkPaint_isSubpixelText'>void</a> <a href='#SkPaint_setSubpixelText'>setSubpixelText</a>(<a href='#SkPaint_setSubpixelText'>bool</a> <a href='#SkPaint_setSubpixelText'>subpixelText</a>);
    <a href='#SkPaint_setSubpixelText'>bool</a> <a href='#SkPaint_isLCDRenderText'>isLCDRenderText</a>() <a href='#SkPaint_isLCDRenderText'>const</a>;
    <a href='#SkPaint_isLCDRenderText'>void</a> <a href='#SkPaint_setLCDRenderText'>setLCDRenderText</a>(<a href='#SkPaint_setLCDRenderText'>bool</a> <a href='#SkPaint_setLCDRenderText'>lcdText</a>);
    <a href='#SkPaint_setLCDRenderText'>bool</a> <a href='#SkPaint_isEmbeddedBitmapText'>isEmbeddedBitmapText</a>() <a href='#SkPaint_isEmbeddedBitmapText'>const</a>;
    <a href='#SkPaint_isEmbeddedBitmapText'>void</a> <a href='#SkPaint_setEmbeddedBitmapText'>setEmbeddedBitmapText</a>(<a href='#SkPaint_setEmbeddedBitmapText'>bool</a> <a href='#SkPaint_setEmbeddedBitmapText'>useEmbeddedBitmapText</a>);
    <a href='#SkPaint_setEmbeddedBitmapText'>bool</a> <a href='#SkPaint_isAutohinted'>isAutohinted</a>() <a href='#SkPaint_isAutohinted'>const</a>;
    <a href='#SkPaint_isAutohinted'>void</a> <a href='#SkPaint_setAutohinted'>setAutohinted</a>(<a href='#SkPaint_setAutohinted'>bool</a> <a href='#SkPaint_setAutohinted'>useAutohinter</a>);
    <a href='#SkPaint_setAutohinted'>bool</a> <a href='#SkPaint_isFakeBoldText'>isFakeBoldText</a>() <a href='#SkPaint_isFakeBoldText'>const</a>;
    <a href='#SkPaint_isFakeBoldText'>void</a> <a href='#SkPaint_setFakeBoldText'>setFakeBoldText</a>(<a href='#SkPaint_setFakeBoldText'>bool</a> <a href='#SkPaint_setFakeBoldText'>fakeBoldText</a>);
    <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='#SkPaint_getFilterQuality'>getFilterQuality</a>() <a href='#SkPaint_getFilterQuality'>const</a>;
    <a href='#SkPaint_getFilterQuality'>void</a> <a href='#SkPaint_setFilterQuality'>setFilterQuality</a>(<a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>quality</a>);

    <a href='undocumented#SkFilterQuality'>enum</a> <a href='#SkPaint_Style'>Style</a> : <a href='#SkPaint_Style'>uint8_t</a> {
        <a href='#SkPaint_kFill_Style'>kFill_Style</a>,
        <a href='#SkPaint_kStroke_Style'>kStroke_Style</a>,
        <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>,
    };

    <a href='#SkPaint_kStrokeAndFill_Style'>static</a> <a href='#SkPaint_kStrokeAndFill_Style'>constexpr</a> <a href='#SkPaint_kStrokeAndFill_Style'>int</a> <a href='#SkPaint_kStyleCount'>kStyleCount</a> = <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a> + 1;

    <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_getStyle'>getStyle</a>() <a href='#SkPaint_getStyle'>const</a>;
    <a href='#SkPaint_getStyle'>void</a> <a href='#SkPaint_setStyle'>setStyle</a>(<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>style</a>);
    <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPaint_getColor'>getColor</a>() <a href='#SkPaint_getColor'>const</a>;
    <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a> <a href='#SkPaint_getColor4f'>getColor4f</a>() <a href='#SkPaint_getColor4f'>const</a>;
    <a href='#SkPaint_getColor4f'>void</a> <a href='#SkPaint_setColor'>setColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>);
    <a href='SkColor_Reference#Color'>void</a> <a href='#SkPaint_setColor4f'>setColor4f</a>(<a href='#SkPaint_setColor4f'>const</a> <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>& <a href='SkColor_Reference#Color'>color</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='undocumented#SkColorSpace'>colorSpace</a>);
    <a href='undocumented#SkColorSpace'>uint8_t</a> <a href='#SkPaint_getAlpha'>getAlpha</a>() <a href='#SkPaint_getAlpha'>const</a>;
    <a href='#SkPaint_getAlpha'>void</a> <a href='#SkPaint_setAlpha'>setAlpha</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>a</a>);
    <a href='undocumented#U8CPU'>void</a> <a href='#SkPaint_setARGB'>setARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>a</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>r</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>g</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>b</a>);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getStrokeWidth'>getStrokeWidth</a>() <a href='#SkPaint_getStrokeWidth'>const</a>;
    <a href='#SkPaint_getStrokeWidth'>void</a> <a href='#SkPaint_setStrokeWidth'>setStrokeWidth</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>width</a>);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getStrokeMiter'>getStrokeMiter</a>() <a href='#SkPaint_getStrokeMiter'>const</a>;
    <a href='#SkPaint_getStrokeMiter'>void</a> <a href='#SkPaint_setStrokeMiter'>setStrokeMiter</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>miter</a>);

    <a href='undocumented#SkScalar'>enum</a> <a href='#SkPaint_Cap'>Cap</a> {
        <a href='#SkPaint_kButt_Cap'>kButt_Cap</a>,
        <a href='#SkPaint_kRound_Cap'>kRound_Cap</a>,
        <a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>,
        <a href='#SkPaint_kLast_Cap'>kLast_Cap</a> = <a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>,
        <a href='#SkPaint_kDefault_Cap'>kDefault_Cap</a> = <a href='#SkPaint_kButt_Cap'>kButt_Cap</a>,
    };

    <a href='#SkPaint_kButt_Cap'>static</a> <a href='#SkPaint_kButt_Cap'>constexpr</a> <a href='#SkPaint_kButt_Cap'>int</a> <a href='#SkPaint_kCapCount'>kCapCount</a> = <a href='#SkPaint_kLast_Cap'>kLast_Cap</a> + 1;

    <a href='#SkPaint_kLast_Cap'>enum</a> <a href='#SkPaint_Join'>Join</a> : <a href='#SkPaint_Join'>uint8_t</a> {
        <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>,
        <a href='#SkPaint_kRound_Join'>kRound_Join</a>,
        <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>,
        <a href='#SkPaint_kLast_Join'>kLast_Join</a> = <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>,
        <a href='#SkPaint_kDefault_Join'>kDefault_Join</a> = <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>,
    };

    <a href='#SkPaint_kMiter_Join'>static</a> <a href='#SkPaint_kMiter_Join'>constexpr</a> <a href='#SkPaint_kMiter_Join'>int</a> <a href='#SkPaint_kJoinCount'>kJoinCount</a> = <a href='#SkPaint_kLast_Join'>kLast_Join</a> + 1;

    <a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_getStrokeCap'>getStrokeCap</a>() <a href='#SkPaint_getStrokeCap'>const</a>;
    <a href='#SkPaint_getStrokeCap'>void</a> <a href='#SkPaint_setStrokeCap'>setStrokeCap</a>(<a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_Cap'>cap</a>);
    <a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a>() <a href='#SkPaint_getStrokeJoin'>const</a>;
    <a href='#SkPaint_getStrokeJoin'>void</a> <a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a>(<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_Join'>join</a>);
    <a href='#SkPaint_Join'>bool</a> <a href='#SkPaint_getFillPath'>getFillPath</a>(<a href='#SkPaint_getFillPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>src</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#SkPath'>dst</a>, <a href='SkPath_Reference#SkPath'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>cullRect</a>,
                     <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>resScale</a> = 1) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>bool</a> <a href='#SkPaint_getFillPath'>getFillPath</a>(<a href='#SkPaint_getFillPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>src</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#SkPath'>dst</a>) <a href='SkPath_Reference#SkPath'>const</a>;
    <a href='undocumented#SkShader'>SkShader</a>* <a href='#SkPaint_getShader'>getShader</a>() <a href='#SkPaint_getShader'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkShader'>SkShader</a>> <a href='#SkPaint_refShader'>refShader</a>() <a href='#SkPaint_refShader'>const</a>;
    <a href='#SkPaint_refShader'>void</a> <a href='#SkPaint_setShader'>setShader</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkShader'>SkShader</a>> <a href='undocumented#Shader'>shader</a>);
    <a href='undocumented#SkColorFilter'>SkColorFilter</a>* <a href='#SkPaint_getColorFilter'>getColorFilter</a>() <a href='#SkPaint_getColorFilter'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorFilter'>SkColorFilter</a>> <a href='#SkPaint_refColorFilter'>refColorFilter</a>() <a href='#SkPaint_refColorFilter'>const</a>;
    <a href='#SkPaint_refColorFilter'>void</a> <a href='#SkPaint_setColorFilter'>setColorFilter</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorFilter'>SkColorFilter</a>> <a href='undocumented#SkColorFilter'>colorFilter</a>);
    <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkPaint_getBlendMode'>getBlendMode</a>() <a href='#SkPaint_getBlendMode'>const</a>;
    <a href='#SkPaint_getBlendMode'>bool</a> <a href='#SkPaint_isSrcOver'>isSrcOver</a>() <a href='#SkPaint_isSrcOver'>const</a>;
    <a href='#SkPaint_isSrcOver'>void</a> <a href='#SkPaint_setBlendMode'>setBlendMode</a>(<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>);
    <a href='undocumented#SkPathEffect'>SkPathEffect</a>* <a href='#SkPaint_getPathEffect'>getPathEffect</a>() <a href='#SkPaint_getPathEffect'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkPathEffect'>SkPathEffect</a>> <a href='#SkPaint_refPathEffect'>refPathEffect</a>() <a href='#SkPaint_refPathEffect'>const</a>;
    <a href='#SkPaint_refPathEffect'>void</a> <a href='#SkPaint_setPathEffect'>setPathEffect</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkPathEffect'>SkPathEffect</a>> <a href='undocumented#SkPathEffect'>pathEffect</a>);
    <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>* <a href='#SkPaint_getMaskFilter'>getMaskFilter</a>() <a href='#SkPaint_getMaskFilter'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>> <a href='#SkPaint_refMaskFilter'>refMaskFilter</a>() <a href='#SkPaint_refMaskFilter'>const</a>;
    <a href='#SkPaint_refMaskFilter'>void</a> <a href='#SkPaint_setMaskFilter'>setMaskFilter</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>> <a href='undocumented#SkMaskFilter'>maskFilter</a>);
    <a href='undocumented#SkTypeface'>SkTypeface</a>* <a href='#SkPaint_getTypeface'>getTypeface</a>() <a href='#SkPaint_getTypeface'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkTypeface'>SkTypeface</a>> <a href='#SkPaint_refTypeface'>refTypeface</a>() <a href='#SkPaint_refTypeface'>const</a>;
    <a href='#SkPaint_refTypeface'>void</a> <a href='#SkPaint_setTypeface'>setTypeface</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkTypeface'>SkTypeface</a>> <a href='undocumented#Typeface'>typeface</a>);
    <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='#SkPaint_getImageFilter'>getImageFilter</a>() <a href='#SkPaint_getImageFilter'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkImageFilter'>SkImageFilter</a>> <a href='#SkPaint_refImageFilter'>refImageFilter</a>() <a href='#SkPaint_refImageFilter'>const</a>;
    <a href='#SkPaint_refImageFilter'>void</a> <a href='#SkPaint_setImageFilter'>setImageFilter</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkImageFilter'>SkImageFilter</a>> <a href='undocumented#SkImageFilter'>imageFilter</a>);
    <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>* <a href='#SkPaint_getDrawLooper'>getDrawLooper</a>() <a href='#SkPaint_getDrawLooper'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>> <a href='#SkPaint_refDrawLooper'>refDrawLooper</a>() <a href='#SkPaint_refDrawLooper'>const</a>;
    <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>* <a href='#SkPaint_getLooper'>getLooper</a>() <a href='#SkPaint_getLooper'>const</a>;
    <a href='#SkPaint_getLooper'>void</a> <a href='#SkPaint_setDrawLooper'>setDrawLooper</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>> <a href='undocumented#SkDrawLooper'>drawLooper</a>);
    <a href='undocumented#SkDrawLooper'>void</a> <a href='#SkPaint_setLooper'>setLooper</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>> <a href='undocumented#SkDrawLooper'>drawLooper</a>);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSize'>getTextSize</a>() <a href='#SkPaint_getTextSize'>const</a>;
    <a href='#SkPaint_getTextSize'>void</a> <a href='#SkPaint_setTextSize'>setTextSize</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>textSize</a>);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextScaleX'>getTextScaleX</a>() <a href='#SkPaint_getTextScaleX'>const</a>;
    <a href='#SkPaint_getTextScaleX'>void</a> <a href='#SkPaint_setTextScaleX'>setTextScaleX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scaleX</a>);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSkewX'>getTextSkewX</a>() <a href='#SkPaint_getTextSkewX'>const</a>;
    <a href='#SkPaint_getTextSkewX'>void</a> <a href='#SkPaint_setTextSkewX'>setTextSkewX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>skewX</a>);

    <a href='undocumented#SkScalar'>enum</a> <a href='#SkPaint_TextEncoding'>TextEncoding</a> : <a href='#SkPaint_TextEncoding'>uint8_t</a> {
        <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>,
        <a href='#SkPaint_kUTF16_TextEncoding'>kUTF16_TextEncoding</a>,
        <a href='#SkPaint_kUTF32_TextEncoding'>kUTF32_TextEncoding</a>,
        <a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID_TextEncoding</a>,
    };

    <a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_getTextEncoding'>getTextEncoding</a>() <a href='#SkPaint_getTextEncoding'>const</a>;
    <a href='#SkPaint_getTextEncoding'>void</a> <a href='#SkPaint_setTextEncoding'>setTextEncoding</a>(<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>encoding</a>);

    <a href='#SkPaint_TextEncoding'>typedef</a> <a href='undocumented#SkFontMetrics'>SkFontMetrics</a> <a href='#SkPaint_FontMetrics'>FontMetrics</a>;

    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getFontMetrics'>getFontMetrics</a>(<a href='undocumented#SkFontMetrics'>SkFontMetrics</a>* <a href='undocumented#SkFontMetrics'>metrics</a>) <a href='undocumented#SkFontMetrics'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getFontSpacing'>getFontSpacing</a>() <a href='#SkPaint_getFontSpacing'>const</a>;
    <a href='#SkPaint_getFontSpacing'>int</a> <a href='#SkPaint_textToGlyphs'>textToGlyphs</a>(<a href='#SkPaint_textToGlyphs'>const</a> <a href='#SkPaint_textToGlyphs'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>,
                     <a href='undocumented#SkGlyphID'>SkGlyphID</a> <a href='undocumented#Glyph'>glyphs</a>[]) <a href='undocumented#Glyph'>const</a>;
    <a href='undocumented#Glyph'>bool</a> <a href='#SkPaint_containsText'>containsText</a>(<a href='#SkPaint_containsText'>const</a> <a href='#SkPaint_containsText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>) <a href='undocumented#Text'>const</a>;
    <a href='undocumented#Text'>void</a> <a href='#SkPaint_glyphsToUnichars'>glyphsToUnichars</a>(<a href='#SkPaint_glyphsToUnichars'>const</a> <a href='undocumented#SkGlyphID'>SkGlyphID</a> <a href='undocumented#Glyph'>glyphs</a>[], <a href='undocumented#Glyph'>int</a> <a href='undocumented#Glyph'>count</a>, <a href='undocumented#SkUnichar'>SkUnichar</a> <a href='undocumented#Text'>text</a>[]) <a href='undocumented#Text'>const</a>;
    <a href='undocumented#Text'>int</a> <a href='#SkPaint_countText'>countText</a>(<a href='#SkPaint_countText'>const</a> <a href='#SkPaint_countText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>) <a href='undocumented#Text'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_measureText'>measureText</a>(<a href='#SkPaint_measureText'>const</a> <a href='#SkPaint_measureText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>, <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_measureText'>measureText</a>(<a href='#SkPaint_measureText'>const</a> <a href='#SkPaint_measureText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>) <a href='undocumented#Text'>const</a>;
    <a href='undocumented#Text'>size_t</a> <a href='#SkPaint_breakText'>breakText</a>(<a href='#SkPaint_breakText'>const</a> <a href='#SkPaint_breakText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>maxWidth</a>,
                      <a href='undocumented#SkScalar'>SkScalar</a>* <a href='undocumented#SkScalar'>measuredWidth</a> = <a href='undocumented#SkScalar'>nullptr</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>int</a> <a href='#SkPaint_getTextWidths'>getTextWidths</a>(<a href='#SkPaint_getTextWidths'>const</a> <a href='#SkPaint_getTextWidths'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>widths</a>[],
                      <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>bounds</a>[] = <a href='SkRect_Reference#SkRect'>nullptr</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='SkRect_Reference#SkRect'>void</a> <a href='#SkPaint_getTextPath'>getTextPath</a>(<a href='#SkPaint_getTextPath'>const</a> <a href='#SkPaint_getTextPath'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>,
                     <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#Path'>path</a>) <a href='SkPath_Reference#Path'>const</a>;
    <a href='SkPath_Reference#Path'>void</a> <a href='#SkPaint_getPosTextPath'>getPosTextPath</a>(<a href='#SkPaint_getPosTextPath'>const</a> <a href='#SkPaint_getPosTextPath'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>,
                        <a href='undocumented#Text'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pos</a>[], <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#Path'>path</a>) <a href='SkPath_Reference#Path'>const</a>;
    <a href='SkPath_Reference#Path'>int</a> <a href='#SkPaint_getTextIntercepts'>getTextIntercepts</a>(<a href='#SkPaint_getTextIntercepts'>const</a> <a href='#SkPaint_getTextIntercepts'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>,
                          <a href='undocumented#SkScalar'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bounds</a>[2], <a href='undocumented#SkScalar'>SkScalar</a>* <a href='undocumented#SkScalar'>intervals</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>int</a> <a href='#SkPaint_getPosTextIntercepts'>getPosTextIntercepts</a>(<a href='#SkPaint_getPosTextIntercepts'>const</a> <a href='#SkPaint_getPosTextIntercepts'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>, <a href='undocumented#Text'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pos</a>[],
                             <a href='SkPoint_Reference#SkPoint'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bounds</a>[2], <a href='undocumented#SkScalar'>SkScalar</a>* <a href='undocumented#SkScalar'>intervals</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>int</a> <a href='#SkPaint_getPosTextHIntercepts'>getPosTextHIntercepts</a>(<a href='#SkPaint_getPosTextHIntercepts'>const</a> <a href='#SkPaint_getPosTextHIntercepts'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>, <a href='undocumented#Text'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xpos</a>[],
                              <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>constY</a>, <a href='undocumented#SkScalar'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bounds</a>[2], <a href='undocumented#SkScalar'>SkScalar</a>* <a href='undocumented#SkScalar'>intervals</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>int</a> <a href='#SkPaint_getTextBlobIntercepts'>getTextBlobIntercepts</a>(<a href='#SkPaint_getTextBlobIntercepts'>const</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>* <a href='SkTextBlob_Reference#SkTextBlob'>blob</a>, <a href='SkTextBlob_Reference#SkTextBlob'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bounds</a>[2],
                              <a href='undocumented#SkScalar'>SkScalar</a>* <a href='undocumented#SkScalar'>intervals</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPaint_getFontBounds'>getFontBounds</a>() <a href='#SkPaint_getFontBounds'>const</a>;
    <a href='#SkPaint_getFontBounds'>bool</a> <a href='#SkPaint_nothingToDraw'>nothingToDraw</a>() <a href='#SkPaint_nothingToDraw'>const</a>;
    <a href='#SkPaint_nothingToDraw'>bool</a> <a href='#SkPaint_canComputeFastBounds'>canComputeFastBounds</a>() <a href='#SkPaint_canComputeFastBounds'>const</a>;
    <a href='#SkPaint_canComputeFastBounds'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkPaint_computeFastBounds'>computeFastBounds</a>(<a href='#SkPaint_computeFastBounds'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>orig</a>, <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>storage</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkPaint_computeFastStrokeBounds'>computeFastStrokeBounds</a>(<a href='#SkPaint_computeFastStrokeBounds'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>orig</a>,
                                          <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>storage</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkPaint_doComputeFastBounds'>doComputeFastBounds</a>(<a href='#SkPaint_doComputeFastBounds'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>orig</a>, <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>storage</a>,
                                      <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>style</a>) <a href='#SkPaint_Style'>const</a>;
};
</pre>

<a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>controls</a> <a href='SkPaint_Reference#Paint'>options</a> <a href='SkPaint_Reference#Paint'>applied</a> <a href='SkPaint_Reference#Paint'>when</a> <a href='SkPaint_Reference#Paint'>drawing</a> <a href='SkPaint_Reference#Paint'>and</a> <a href='SkPaint_Reference#Paint'>measuring</a>. <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>collects</a> <a href='SkPaint_Reference#Paint'>all</a>
<a href='SkPaint_Reference#Paint'>options</a> <a href='SkPaint_Reference#Paint'>outside</a> <a href='SkPaint_Reference#Paint'>of</a> <a href='SkPaint_Reference#Paint'>the</a> <a href='#Canvas_Clip'>Canvas_Clip</a> <a href='#Canvas_Clip'>and</a> <a href='#Canvas_Matrix'>Canvas_Matrix</a>.

<a href='#Canvas_Matrix'>Various</a> <a href='#Canvas_Matrix'>options</a> <a href='#Canvas_Matrix'>apply</a> <a href='#Canvas_Matrix'>to</a> <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>strokes</a> <a href='undocumented#Text'>and</a> <a href='undocumented#Text'>fills</a>, <a href='undocumented#Text'>and</a> <a href='undocumented#Text'>images</a>.

<a href='undocumented#Text'>Some</a> <a href='undocumented#Text'>options</a> <a href='undocumented#Text'>may</a> <a href='undocumented#Text'>not</a> <a href='undocumented#Text'>be</a> <a href='undocumented#Text'>implemented</a> <a href='undocumented#Text'>on</a> <a href='undocumented#Text'>all</a> <a href='undocumented#Text'>platforms</a>; <a href='undocumented#Text'>in</a> <a href='undocumented#Text'>these</a> <a href='undocumented#Text'>cases</a>, <a href='undocumented#Text'>setting</a>
<a href='undocumented#Text'>the</a> <a href='undocumented#Text'>option</a> <a href='undocumented#Text'>has</a> <a href='undocumented#Text'>no</a> <a href='undocumented#Text'>effect</a>. <a href='undocumented#Text'>Some</a> <a href='undocumented#Text'>options</a> <a href='undocumented#Text'>are</a> <a href='undocumented#Text'>conveniences</a> <a href='undocumented#Text'>that</a> <a href='undocumented#Text'>duplicate</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a>
<a href='SkCanvas_Reference#Canvas'>functionality</a>; <a href='SkCanvas_Reference#Canvas'>for</a> <a href='SkCanvas_Reference#Canvas'>instance</a>,  <a href='#Text_Size'>text size</a> <a href='undocumented#Text'>is</a> <a href='undocumented#Text'>identical</a> <a href='undocumented#Text'>to</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>scale</a>.

<a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>options</a> <a href='SkPaint_Reference#Paint'>are</a> <a href='SkPaint_Reference#Paint'>rarely</a> <a href='SkPaint_Reference#Paint'>exclusive</a>; <a href='SkPaint_Reference#Paint'>each</a> <a href='SkPaint_Reference#Paint'>option</a> <a href='SkPaint_Reference#Paint'>modifies</a> <a href='SkPaint_Reference#Paint'>a</a> <a href='SkPaint_Reference#Paint'>stage</a> <a href='SkPaint_Reference#Paint'>of</a> <a href='SkPaint_Reference#Paint'>the</a> <a href='SkPaint_Reference#Paint'>drawing</a>
<a href='SkPaint_Reference#Paint'>pipeline</a> <a href='SkPaint_Reference#Paint'>and</a> <a href='SkPaint_Reference#Paint'>multiple</a> <a href='SkPaint_Reference#Paint'>pipeline</a> <a href='SkPaint_Reference#Paint'>stages</a> <a href='SkPaint_Reference#Paint'>may</a> <a href='SkPaint_Reference#Paint'>be</a> <a href='SkPaint_Reference#Paint'>affected</a> <a href='SkPaint_Reference#Paint'>by</a> <a href='SkPaint_Reference#Paint'>a</a> <a href='SkPaint_Reference#Paint'>single</a> <a href='SkPaint_Reference#Paint'>Paint</a>.

<a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>collects</a> <a href='SkPaint_Reference#Paint'>effects</a> <a href='SkPaint_Reference#Paint'>and</a> <a href='SkPaint_Reference#Paint'>filters</a> <a href='SkPaint_Reference#Paint'>that</a> <a href='SkPaint_Reference#Paint'>describe</a> <a href='SkPaint_Reference#Paint'>single-pass</a> <a href='SkPaint_Reference#Paint'>and</a> <a href='SkPaint_Reference#Paint'>multiple-pass</a>
<a href='SkPaint_Reference#Paint'>algorithms</a> <a href='SkPaint_Reference#Paint'>that</a> <a href='SkPaint_Reference#Paint'>alter</a> <a href='SkPaint_Reference#Paint'>the</a> <a href='SkPaint_Reference#Paint'>drawing</a> <a href='SkPaint_Reference#Paint'>geometry</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>transparency</a>. <a href='SkColor_Reference#Color'>For</a> <a href='SkColor_Reference#Color'>instance</a>,
<a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>does</a> <a href='SkPaint_Reference#Paint'>not</a> <a href='SkPaint_Reference#Paint'>directly</a> <a href='SkPaint_Reference#Paint'>implement</a> <a href='SkPaint_Reference#Paint'>dashing</a> <a href='SkPaint_Reference#Paint'>or</a> <a href='SkPaint_Reference#Paint'>blur</a>, <a href='SkPaint_Reference#Paint'>but</a> <a href='SkPaint_Reference#Paint'>contains</a> <a href='SkPaint_Reference#Paint'>the</a> <a href='SkPaint_Reference#Paint'>objects</a> <a href='SkPaint_Reference#Paint'>that</a> <a href='SkPaint_Reference#Paint'>do</a> <a href='SkPaint_Reference#Paint'>so</a>.

<a href='SkPaint_Reference#Paint'>The</a> <a href='SkPaint_Reference#Paint'>objects</a> <a href='SkPaint_Reference#Paint'>contained</a> <a href='SkPaint_Reference#Paint'>by</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>are</a> <a href='SkPaint_Reference#Paint'>opaque</a>, <a href='SkPaint_Reference#Paint'>and</a> <a href='SkPaint_Reference#Paint'>cannot</a> <a href='SkPaint_Reference#Paint'>be</a> <a href='SkPaint_Reference#Paint'>edited</a> <a href='SkPaint_Reference#Paint'>outside</a> <a href='SkPaint_Reference#Paint'>of</a> <a href='SkPaint_Reference#Paint'>the</a> <a href='SkPaint_Reference#Paint'>Paint</a>
<a href='SkPaint_Reference#Paint'>to</a> <a href='SkPaint_Reference#Paint'>affect</a> <a href='SkPaint_Reference#Paint'>it</a>. <a href='SkPaint_Reference#Paint'>The</a> <a href='SkPaint_Reference#Paint'>implementation</a> <a href='SkPaint_Reference#Paint'>is</a> <a href='SkPaint_Reference#Paint'>free</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='SkPaint_Reference#Paint'>defer</a> <a href='SkPaint_Reference#Paint'>computations</a> <a href='SkPaint_Reference#Paint'>associated</a> <a href='SkPaint_Reference#Paint'>with</a> <a href='SkPaint_Reference#Paint'>the</a>
<a href='SkPaint_Reference#Paint'>Paint</a>, <a href='SkPaint_Reference#Paint'>or</a> <a href='SkPaint_Reference#Paint'>ignore</a> <a href='SkPaint_Reference#Paint'>them</a> <a href='SkPaint_Reference#Paint'>altogether</a>. <a href='SkPaint_Reference#Paint'>For</a> <a href='SkPaint_Reference#Paint'>instance</a>, <a href='SkPaint_Reference#Paint'>some</a> <a href='SkPaint_Reference#Paint'>GPU</a> <a href='SkPaint_Reference#Paint'>implementations</a> <a href='SkPaint_Reference#Paint'>draw</a> <a href='SkPaint_Reference#Paint'>all</a>
<a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>geometries</a> <a href='SkPath_Reference#Path'>with</a> <a href='#Paint_Anti_Alias'>Anti_Aliasing</a>, <a href='#Paint_Anti_Alias'>regardless</a> <a href='#Paint_Anti_Alias'>of</a> <a href='#Paint_Anti_Alias'>how</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a>
<a href='#SkPaint_kAntiAlias_Flag'>is</a> <a href='#SkPaint_kAntiAlias_Flag'>set</a> <a href='#SkPaint_kAntiAlias_Flag'>in</a> <a href='SkPaint_Reference#Paint'>Paint</a>.

<a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>describes</a> <a href='SkPaint_Reference#Paint'>a</a> <a href='SkPaint_Reference#Paint'>single</a> <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>a</a> <a href='SkColor_Reference#Color'>single</a> <a href='undocumented#Font'>font</a>, <a href='undocumented#Font'>a</a> <a href='undocumented#Font'>single</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>quality</a>, <a href='SkImage_Reference#Image'>and</a> <a href='SkImage_Reference#Image'>so</a> <a href='SkImage_Reference#Image'>on</a>.
<a href='SkImage_Reference#Image'>Multiple</a> <a href='SkImage_Reference#Image'>colors</a> <a href='SkImage_Reference#Image'>are</a> <a href='SkImage_Reference#Image'>drawn</a> <a href='SkImage_Reference#Image'>either</a> <a href='SkImage_Reference#Image'>by</a> <a href='SkImage_Reference#Image'>using</a> <a href='SkImage_Reference#Image'>multiple</a> <a href='SkImage_Reference#Image'>paints</a> <a href='SkImage_Reference#Image'>or</a> <a href='SkImage_Reference#Image'>with</a> <a href='SkImage_Reference#Image'>objects</a> <a href='SkImage_Reference#Image'>like</a>
<a href='undocumented#Shader'>Shader</a> <a href='undocumented#Shader'>attached</a> <a href='undocumented#Shader'>to</a> <a href='SkPaint_Reference#Paint'>Paint</a>.

<a name='SkPaint_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_empty_constructor'>SkPaint()</a>
</pre>

Constructs <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>with</a> <a href='SkPaint_Reference#Paint'>default</a> <a href='SkPaint_Reference#Paint'>values</a>.

| attribute | default value |
| --- | ---  |
| <a href='#Paint_Anti_Alias'>Anti_Alias</a> | false |
| <a href='#Blend_Mode'>Blend_Mode</a> | <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a> |
| <a href='SkColor_Reference#Color'>Color</a> | <a href='SkColor_Reference#SK_ColorBLACK'>SK_ColorBLACK</a> |
| <a href='#Color_Alpha'>Color_Alpha</a> | 255 |
| <a href='#Color_Filter'>Color_Filter</a> | nullptr |
| Dither | false |
| <a href='#Draw_Looper'>Draw_Looper</a> | nullptr |
| <a href='#Paint_Fake_Bold'>Fake_Bold</a> | false |
| <a href='#Filter_Quality'>Filter_Quality</a> | <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a> |
| <a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a> | false |
| <a href='#Paint_Automatic_Hinting'>Automatic_Hinting</a> | false |
| <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a> | false |
| <a href='#SkPaint_Hinting'>Hinting</a> | <a href='#SkPaint_kNormal_Hinting'>kNormal_Hinting</a> |
| <a href='#Image_Filter'>Image_Filter</a> | nullptr |
| <a href='#Paint_LCD_Text'>LCD_Text</a> | false |
| <a href='#Paint_Linear_Text'>Linear_Text</a> | false |
| <a href='#Paint_Miter_Limit'>Miter_Limit</a> | 4 |
| <a href='#Mask_Filter'>Mask_Filter</a> | nullptr |
| <a href='#Path_Effect'>Path_Effect</a> | nullptr |
| <a href='undocumented#Shader'>Shader</a> | nullptr |
| <a href='#SkPaint_Style'>Style</a> | <a href='#SkPaint_kFill_Style'>kFill_Style</a> |
| <a href='#Paint_Text_Encoding'>Text_Encoding</a> | <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a> |
| <a href='#Paint_Text_Scale_X'>Text_Scale_X</a> | 1 |
| <a href='#Paint_Text_Size'>Text_Size</a> | 12 |
| <a href='#Paint_Text_Skew_X'>Text_Skew_X</a> | 0 |
| <a href='undocumented#Typeface'>Typeface</a> | nullptr |
| <a href='#Paint_Stroke_Cap'>Stroke_Cap</a> | <a href='#SkPaint_kButt_Cap'>kButt_Cap</a> |
| <a href='#Paint_Stroke_Join'>Stroke_Join</a> | <a href='#SkPaint_kMiter_Join'>kMiter_Join</a> |
| <a href='#Paint_Stroke_Width'>Stroke_Width</a> | 0 |
| <a href='#Paint_Subpixel_Text'>Subpixel_Text</a> | false |

The flags, <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>hinting</a>, <a href='undocumented#Size'>and</a> <a href='undocumented#Size'>miter</a> <a href='undocumented#Size'>limit</a> <a href='undocumented#Size'>may</a> <a href='undocumented#Size'>be</a> <a href='undocumented#Size'>overridden</a> <a href='undocumented#Size'>at</a> <a href='undocumented#Size'>compile</a> <a href='undocumented#Size'>time</a> <a href='undocumented#Size'>by</a> <a href='undocumented#Size'>defining</a>
<a href='SkPaint_Reference#Paint'>paint</a> <a href='SkPaint_Reference#Paint'>default</a> <a href='SkPaint_Reference#Paint'>values</a>. <a href='SkPaint_Reference#Paint'>The</a> <a href='SkPaint_Reference#Paint'>overrides</a> <a href='SkPaint_Reference#Paint'>may</a> <a href='SkPaint_Reference#Paint'>be</a> <a href='SkPaint_Reference#Paint'>included</a> <a href='SkPaint_Reference#Paint'>in</a> "<a href='SkPaint_Reference#Paint'>SkUserConfig</a>.<a href='SkPaint_Reference#Paint'>h</a>" <a href='SkPaint_Reference#Paint'>or</a> <a href='SkPaint_Reference#Paint'>predefined</a> <a href='SkPaint_Reference#Paint'>by</a> <a href='SkPaint_Reference#Paint'>the</a>
<a href='SkPaint_Reference#Paint'>build</a> <a href='SkPaint_Reference#Paint'>system</a>.

### Return Value

default initialized <a href='SkPaint_Reference#Paint'>Paint</a>

### Example

<div><fiddle-embed name="c4b2186d85c142a481298f7144295ffd"></fiddle-embed></div>

<a name='SkPaint_copy_const_SkPaint'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>(<a href='SkPaint_Reference#SkPaint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Makes a shallow copy of <a href='SkPaint_Reference#SkPaint'>SkPaint</a>. <a href='undocumented#SkTypeface'>SkTypeface</a>, <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>, <a href='undocumented#SkDrawLooper'>and</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkImageFilter'>are</a> <a href='undocumented#SkImageFilter'>shared</a>
between the original <a href='#SkPaint_copy_const_SkPaint_paint'>paint</a> <a href='#SkPaint_copy_const_SkPaint_paint'>and</a> <a href='#SkPaint_copy_const_SkPaint_paint'>the</a> <a href='#SkPaint_copy_const_SkPaint_paint'>copy</a>. <a href='#SkPaint_copy_const_SkPaint_paint'>Objects</a> <a href='#SkPaint_copy_const_SkPaint_paint'>containing</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>increment</a>
their references by one.

The referenced objects <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>, <a href='undocumented#SkDrawLooper'>and</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkImageFilter'>cannot</a> <a href='undocumented#SkImageFilter'>be</a> <a href='undocumented#SkImageFilter'>modified</a> <a href='undocumented#SkImageFilter'>after</a> <a href='undocumented#SkImageFilter'>they</a> <a href='undocumented#SkImageFilter'>are</a> <a href='undocumented#SkImageFilter'>created</a>.
This prevents objects with <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>from</a> <a href='undocumented#SkRefCnt'>being</a> <a href='undocumented#SkRefCnt'>modified</a> <a href='undocumented#SkRefCnt'>once</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>refers</a> <a href='SkPaint_Reference#SkPaint'>to</a> <a href='SkPaint_Reference#SkPaint'>them</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_copy_const_SkPaint_paint'><code><strong>paint</strong></code></a></td>
    <td>original to copy</td>
  </tr>
</table>

### Return Value

shallow copy of <a href='#SkPaint_copy_const_SkPaint_paint'>paint</a>

### Example

<div><fiddle-embed name="b99971ad0ef243d617925289d963b62d">

#### Example Output

~~~~
SK_ColorRED == paint1.getColor()
SK_ColorBLUE == paint2.getColor()
~~~~

</fiddle-embed></div>

<a name='SkPaint_move_SkPaint'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>(<a href='SkPaint_Reference#SkPaint'>SkPaint</a>&& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Implements a move constructor to avoid increasing the reference counts
of objects referenced by the <a href='#SkPaint_move_SkPaint_paint'>paint</a>.

After the call, <a href='#SkPaint_move_SkPaint_paint'>paint</a> <a href='#SkPaint_move_SkPaint_paint'>is</a> <a href='#SkPaint_move_SkPaint_paint'>undefined</a>, <a href='#SkPaint_move_SkPaint_paint'>and</a> <a href='#SkPaint_move_SkPaint_paint'>can</a> <a href='#SkPaint_move_SkPaint_paint'>be</a> <a href='#SkPaint_move_SkPaint_paint'>safely</a> <a href='#SkPaint_move_SkPaint_paint'>destructed</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_move_SkPaint_paint'><code><strong>paint</strong></code></a></td>
    <td>original to move</td>
  </tr>
</table>

### Return Value

content of <a href='#SkPaint_move_SkPaint_paint'>paint</a>

### Example

<div><fiddle-embed name="8ed1488a503cd5282b86a51614aa90b1">

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

Sets all <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>contents</a> <a href='SkPaint_Reference#SkPaint'>to</a> <a href='SkPaint_Reference#SkPaint'>their</a> <a href='SkPaint_Reference#SkPaint'>initial</a> <a href='SkPaint_Reference#SkPaint'>values</a>. <a href='SkPaint_Reference#SkPaint'>This</a> <a href='SkPaint_Reference#SkPaint'>is</a> <a href='SkPaint_Reference#SkPaint'>equivalent</a> <a href='SkPaint_Reference#SkPaint'>to</a> <a href='SkPaint_Reference#SkPaint'>replacing</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>with</a> <a href='SkPaint_Reference#SkPaint'>the</a> <a href='SkPaint_Reference#SkPaint'>result</a> <a href='SkPaint_Reference#SkPaint'>of</a> <a href='#SkPaint_empty_constructor'>SkPaint()</a>.

### Example

<div><fiddle-embed name="ef269937ade7e7353635121d9a64f9f7">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

<a name='SkPaint_destructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
~<a href='#SkPaint_empty_constructor'>SkPaint()</a>
</pre>

Decreases <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>of</a> <a href='undocumented#SkRefCnt'>owned</a> <a href='undocumented#SkRefCnt'>objects</a>: <a href='undocumented#SkTypeface'>SkTypeface</a>, <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>, <a href='undocumented#SkDrawLooper'>and</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a>. <a href='undocumented#SkImageFilter'>If</a> <a href='undocumented#SkImageFilter'>the</a>
objects containing <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>go</a> <a href='undocumented#SkRefCnt'>to</a> <a href='undocumented#SkRefCnt'>zero</a>, <a href='undocumented#SkRefCnt'>they</a> <a href='undocumented#SkRefCnt'>are</a> <a href='undocumented#SkRefCnt'>deleted</a>.

<a name='Management'></a>

<a name='SkPaint_copy_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#SkPaint'>operator</a>=(<a href='SkPaint_Reference#SkPaint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Makes a shallow copy of <a href='SkPaint_Reference#SkPaint'>SkPaint</a>. <a href='undocumented#SkTypeface'>SkTypeface</a>, <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>, <a href='undocumented#SkDrawLooper'>and</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkImageFilter'>are</a> <a href='undocumented#SkImageFilter'>shared</a>
between the original <a href='#SkPaint_copy_operator_paint'>paint</a> <a href='#SkPaint_copy_operator_paint'>and</a> <a href='#SkPaint_copy_operator_paint'>the</a> <a href='#SkPaint_copy_operator_paint'>copy</a>. <a href='#SkPaint_copy_operator_paint'>Objects</a> <a href='#SkPaint_copy_operator_paint'>containing</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>in</a> <a href='undocumented#SkRefCnt'>the</a>
prior destination are decreased by one, and the referenced objects are deleted if the
resulting count is zero. Objects containing <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>in</a> <a href='undocumented#SkRefCnt'>the</a> <a href='undocumented#SkRefCnt'>parameter</a> <a href='#SkPaint_copy_operator_paint'>paint</a>
are increased by one. <a href='#SkPaint_copy_operator_paint'>paint</a> <a href='#SkPaint_copy_operator_paint'>is</a> <a href='#SkPaint_copy_operator_paint'>unmodified</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_copy_operator_paint'><code><strong>paint</strong></code></a></td>
    <td>original to copy</td>
  </tr>
</table>

### Return Value

content of <a href='#SkPaint_copy_operator_paint'>paint</a>

### Example

<div><fiddle-embed name="b476a9088f80dece176ed577807d3992">

#### Example Output

~~~~
SK_ColorRED == paint1.getColor()
SK_ColorRED == paint2.getColor()
~~~~

</fiddle-embed></div>

<a name='SkPaint_move_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#SkPaint'>operator</a>=(<a href='SkPaint_Reference#SkPaint'>SkPaint</a>&& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Moves the <a href='#SkPaint_move_operator_paint'>paint</a> <a href='#SkPaint_move_operator_paint'>to</a> <a href='#SkPaint_move_operator_paint'>avoid</a> <a href='#SkPaint_move_operator_paint'>increasing</a> <a href='#SkPaint_move_operator_paint'>the</a> <a href='#SkPaint_move_operator_paint'>reference</a> <a href='#SkPaint_move_operator_paint'>counts</a>
of objects referenced by the <a href='#SkPaint_move_operator_paint'>paint</a> <a href='#SkPaint_move_operator_paint'>parameter</a>. <a href='#SkPaint_move_operator_paint'>Objects</a> <a href='#SkPaint_move_operator_paint'>containing</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>in</a> <a href='undocumented#SkRefCnt'>the</a>
prior destination are decreased by one; those objects are deleted if the resulting count
is zero.

After the call, <a href='#SkPaint_move_operator_paint'>paint</a> <a href='#SkPaint_move_operator_paint'>is</a> <a href='#SkPaint_move_operator_paint'>undefined</a>, <a href='#SkPaint_move_operator_paint'>and</a> <a href='#SkPaint_move_operator_paint'>can</a> <a href='#SkPaint_move_operator_paint'>be</a> <a href='#SkPaint_move_operator_paint'>safely</a> <a href='#SkPaint_move_operator_paint'>destructed</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_move_operator_paint'><code><strong>paint</strong></code></a></td>
    <td>original to move</td>
  </tr>
</table>

### Return Value

content of <a href='#SkPaint_move_operator_paint'>paint</a>

### Example

<div><fiddle-embed name="9fb7459b097d713f5f1fe5675afe14f5">

#### Example Output

~~~~
SK_ColorRED == paint2.getColor()
~~~~

</fiddle-embed></div>

<a name='SkPaint_equal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#SkPaint'>a</a>, <a href='SkPaint_Reference#SkPaint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#SkPaint'>b</a>)
</pre>

Compares <a href='#SkPaint_equal_operator_a'>a</a> <a href='#SkPaint_equal_operator_a'>and</a> <a href='#SkPaint_equal_operator_b'>b</a>, <a href='#SkPaint_equal_operator_b'>and</a> <a href='#SkPaint_equal_operator_b'>returns</a> <a href='#SkPaint_equal_operator_b'>true</a> <a href='#SkPaint_equal_operator_b'>if</a> <a href='#SkPaint_equal_operator_a'>a</a> <a href='#SkPaint_equal_operator_a'>and</a> <a href='#SkPaint_equal_operator_b'>b</a> <a href='#SkPaint_equal_operator_b'>are</a> <a href='#SkPaint_equal_operator_b'>equivalent</a>. <a href='#SkPaint_equal_operator_b'>May</a> <a href='#SkPaint_equal_operator_b'>return</a> <a href='#SkPaint_equal_operator_b'>false</a>
if <a href='undocumented#SkTypeface'>SkTypeface</a>, <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>, <a href='undocumented#SkDrawLooper'>or</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkImageFilter'>have</a> <a href='undocumented#SkImageFilter'>identical</a> <a href='undocumented#SkImageFilter'>contents</a> <a href='undocumented#SkImageFilter'>but</a> <a href='undocumented#SkImageFilter'>different</a> <a href='undocumented#SkImageFilter'>pointers</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>to</a> <a href='SkPaint_Reference#SkPaint'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>to</a> <a href='SkPaint_Reference#SkPaint'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>pair</a> <a href='SkPaint_Reference#SkPaint'>are</a> <a href='SkPaint_Reference#SkPaint'>equivalent</a>

### Example

<div><fiddle-embed name="7481a948e34672720337a631830586dd">

#### Example Output

~~~~
paint1 == paint2
paint1 != paint2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_notequal_operator'>operator!=(const SkPaint& a, const SkPaint& b)</a>

<a name='SkPaint_notequal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#SkPaint'>a</a>, <a href='SkPaint_Reference#SkPaint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#SkPaint'>b</a>)
</pre>

Compares <a href='#SkPaint_notequal_operator_a'>a</a> <a href='#SkPaint_notequal_operator_a'>and</a> <a href='#SkPaint_notequal_operator_b'>b</a>, <a href='#SkPaint_notequal_operator_b'>and</a> <a href='#SkPaint_notequal_operator_b'>returns</a> <a href='#SkPaint_notequal_operator_b'>true</a> <a href='#SkPaint_notequal_operator_b'>if</a> <a href='#SkPaint_notequal_operator_a'>a</a> <a href='#SkPaint_notequal_operator_a'>and</a> <a href='#SkPaint_notequal_operator_b'>b</a> <a href='#SkPaint_notequal_operator_b'>are</a> <a href='#SkPaint_notequal_operator_b'>not</a> <a href='#SkPaint_notequal_operator_b'>equivalent</a>. <a href='#SkPaint_notequal_operator_b'>May</a> <a href='#SkPaint_notequal_operator_b'>return</a> <a href='#SkPaint_notequal_operator_b'>true</a>
if <a href='undocumented#SkTypeface'>SkTypeface</a>, <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>, <a href='undocumented#SkDrawLooper'>or</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkImageFilter'>have</a> <a href='undocumented#SkImageFilter'>identical</a> <a href='undocumented#SkImageFilter'>contents</a> <a href='undocumented#SkImageFilter'>but</a> <a href='undocumented#SkImageFilter'>different</a> <a href='undocumented#SkImageFilter'>pointers</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>to</a> <a href='SkPaint_Reference#SkPaint'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>to</a> <a href='SkPaint_Reference#SkPaint'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>pair</a> <a href='SkPaint_Reference#SkPaint'>are</a> <a href='SkPaint_Reference#SkPaint'>not</a> <a href='SkPaint_Reference#SkPaint'>equivalent</a>

### Example

<div><fiddle-embed name="b6c8484b1187f555b435ad5369833be4">

#### Example Output

~~~~
paint1 == paint2
paint1 == paint2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_equal_operator'>operator==(const SkPaint& a, const SkPaint& b)</a>

<a name='SkPaint_getHash'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPaint_getHash'>getHash</a>() <a href='#SkPaint_getHash'>const</a>
</pre>

Returns a hash generated from <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>values</a> <a href='SkPaint_Reference#SkPaint'>and</a> <a href='SkPaint_Reference#SkPaint'>pointers</a>.
Identical hashes guarantee that the paints are
equivalent, but differing hashes do not guarantee that the paints have differing
contents.

If If<a href='#SkPaint_equal_operator'>operator==(const SkPaint& a, const SkPaint& b)</a> <a href='#SkPaint_equal_operator'>returns</a> <a href='#SkPaint_equal_operator'>true</a> <a href='#SkPaint_equal_operator'>for</a> <a href='#SkPaint_equal_operator'>two</a> <a href='#SkPaint_equal_operator'>paints</a>,
their hashes are also equal.

The hash returned is platform and implementation specific.

### Return Value

a shallow hash

### Example

<div><fiddle-embed name="7f7e1b701361912b344f90ae6b530393">

#### Example Output

~~~~
paint1 == paint2
paint1.getHash() == paint2.getHash()
~~~~

</fiddle-embed></div>

<a name='Hinting'></a>

<a name='SkPaint_Hinting'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Hinting'>Hinting</a> : <a href='#SkPaint_Hinting'>uint8_t</a> {
        <a href='#SkPaint_kNo_Hinting'>kNo_Hinting</a> = 0,
        <a href='#SkPaint_kSlight_Hinting'>kSlight_Hinting</a> = 1,
        <a href='#SkPaint_kNormal_Hinting'>kNormal_Hinting</a> = 2,
        <a href='#SkPaint_kFull_Hinting'>kFull_Hinting</a> = 3,
    };
</pre>

<a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>adjusts</a> <a href='#SkPaint_Hinting'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>outlines</a> <a href='undocumented#Glyph'>so</a> <a href='undocumented#Glyph'>that</a> <a href='undocumented#Glyph'>the</a> <a href='undocumented#Glyph'>shape</a> <a href='undocumented#Glyph'>provides</a> <a href='undocumented#Glyph'>a</a> <a href='undocumented#Glyph'>uniform</a>
<a href='undocumented#Glyph'>look</a> <a href='undocumented#Glyph'>at</a> <a href='undocumented#Glyph'>a</a> <a href='undocumented#Glyph'>given</a> <a href='SkPoint_Reference#Point'>point</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>on</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>engines</a> <a href='undocumented#Font'>that</a> <a href='undocumented#Font'>support</a> <a href='undocumented#Font'>it</a>. <a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>may</a> <a href='#SkPaint_Hinting'>have</a> <a href='#SkPaint_Hinting'>a</a>
<a href='#SkPaint_Hinting'>muted</a> <a href='#SkPaint_Hinting'>effect</a> <a href='#SkPaint_Hinting'>or</a> <a href='#SkPaint_Hinting'>no</a> <a href='#SkPaint_Hinting'>effect</a> <a href='#SkPaint_Hinting'>at</a> <a href='#SkPaint_Hinting'>all</a> <a href='#SkPaint_Hinting'>depending</a> <a href='#SkPaint_Hinting'>on</a> <a href='#SkPaint_Hinting'>the</a> <a href='#SkPaint_Hinting'>platform</a>.

<a href='#SkPaint_Hinting'>The</a> <a href='#SkPaint_Hinting'>four</a> <a href='#SkPaint_Hinting'>levels</a> <a href='#SkPaint_Hinting'>roughly</a> <a href='#SkPaint_Hinting'>control</a> <a href='#SkPaint_Hinting'>corresponding</a> <a href='#SkPaint_Hinting'>features</a> <a href='#SkPaint_Hinting'>on</a> <a href='#SkPaint_Hinting'>platforms</a> <a href='#SkPaint_Hinting'>that</a> <a href='#SkPaint_Hinting'>use</a> <a href='#SkPaint_Hinting'>FreeType</a>
<a href='#SkPaint_Hinting'>as</a> <a href='#SkPaint_Hinting'>the</a> <a href='#Font_Engine'>Font_Engine</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kNo_Hinting'><code>SkPaint::kNo_Hinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Leaves <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>outlines</a> <a href='undocumented#Glyph'>unchanged</a> <a href='undocumented#Glyph'>from</a> <a href='undocumented#Glyph'>their</a> <a href='undocumented#Glyph'>native</a> <a href='undocumented#Glyph'>representation</a>.
<a href='undocumented#Glyph'>With</a> <a href='undocumented#Glyph'>FreeType</a>, <a href='undocumented#Glyph'>this</a> <a href='undocumented#Glyph'>is</a> <a href='undocumented#Glyph'>equivalent</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>the</a> <a href='undocumented#Glyph'>FT_LOAD_NO_HINTING</a>
<a href='undocumented#Glyph'>bit-field</a> <a href='undocumented#Glyph'>constant</a> <a href='undocumented#Glyph'>supplied</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>FT_Load_Glyph</a>, <a href='undocumented#Glyph'>which</a> <a href='undocumented#Glyph'>indicates</a> <a href='undocumented#Glyph'>that</a> <a href='undocumented#Glyph'>the</a> <a href='SkPoint_Reference#Vector'>vector</a>
<a href='SkPoint_Reference#Vector'>outline</a> <a href='SkPoint_Reference#Vector'>being</a> <a href='SkPoint_Reference#Vector'>loaded</a> <a href='SkPoint_Reference#Vector'>should</a> <a href='SkPoint_Reference#Vector'>not</a> <a href='SkPoint_Reference#Vector'>be</a> <a href='SkPoint_Reference#Vector'>fitted</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>the</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>grid</a> <a href='undocumented#Pixel'>but</a> <a href='undocumented#Pixel'>simply</a> <a href='undocumented#Pixel'>scaled</a>
<a href='undocumented#Pixel'>to</a> 26.6 <a href='undocumented#Pixel'>fractional</a> <a href='undocumented#Pixel'>pixels</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kSlight_Hinting'><code>SkPaint::kSlight_Hinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Modifies <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>outlines</a> <a href='undocumented#Glyph'>minimally</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>improve</a> <a href='undocumented#Glyph'>contrast</a>.
<a href='undocumented#Glyph'>With</a> <a href='undocumented#Glyph'>FreeType</a>, <a href='undocumented#Glyph'>this</a> <a href='undocumented#Glyph'>is</a> <a href='undocumented#Glyph'>equivalent</a> <a href='undocumented#Glyph'>in</a> <a href='undocumented#Glyph'>spirit</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>the</a>
<a href='undocumented#Glyph'>FT_LOAD_TARGET_LIGHT</a> <a href='undocumented#Glyph'>value</a> <a href='undocumented#Glyph'>supplied</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>FT_Load_Glyph</a>. <a href='undocumented#Glyph'>It</a> <a href='undocumented#Glyph'>chooses</a> <a href='undocumented#Glyph'>a</a>
<a href='undocumented#Glyph'>lighter</a> <a href='undocumented#Glyph'>hinting</a> <a href='undocumented#Glyph'>algorithm</a> <a href='undocumented#Glyph'>for</a> <a href='undocumented#Glyph'>non-monochrome</a> <a href='undocumented#Glyph'>modes</a>.
<a href='undocumented#Glyph'>Generated</a> <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>may</a> <a href='undocumented#Glyph'>be</a> <a href='undocumented#Glyph'>fuzzy</a> <a href='undocumented#Glyph'>but</a> <a href='undocumented#Glyph'>better</a> <a href='undocumented#Glyph'>resemble</a> <a href='undocumented#Glyph'>their</a> <a href='undocumented#Glyph'>original</a> <a href='undocumented#Glyph'>shape</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kNormal_Hinting'><code>SkPaint::kNormal_Hinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Modifies <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>outlines</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>improve</a> <a href='undocumented#Glyph'>contrast</a>. <a href='undocumented#Glyph'>This</a> <a href='undocumented#Glyph'>is</a> <a href='undocumented#Glyph'>the</a> <a href='undocumented#Glyph'>default</a>.
<a href='undocumented#Glyph'>With</a> <a href='undocumented#Glyph'>FreeType</a>, <a href='undocumented#Glyph'>this</a> <a href='undocumented#Glyph'>supplies</a> <a href='undocumented#Glyph'>FT_LOAD_TARGET_NORMAL</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>FT_Load_Glyph</a>,
<a href='undocumented#Glyph'>choosing</a> <a href='undocumented#Glyph'>the</a> <a href='undocumented#Glyph'>default</a> <a href='undocumented#Glyph'>hinting</a> <a href='undocumented#Glyph'>algorithm</a>, <a href='undocumented#Glyph'>which</a> <a href='undocumented#Glyph'>is</a> <a href='undocumented#Glyph'>optimized</a> <a href='undocumented#Glyph'>for</a> <a href='undocumented#Glyph'>standard</a>
<a href='undocumented#Glyph'>gray-level</a> <a href='undocumented#Glyph'>rendering</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kFull_Hinting'><code>SkPaint::kFull_Hinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Modifies <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>outlines</a> <a href='undocumented#Glyph'>for</a> <a href='undocumented#Glyph'>maximum</a> <a href='undocumented#Glyph'>contrast</a>. <a href='undocumented#Glyph'>With</a> <a href='undocumented#Glyph'>FreeType</a>, <a href='undocumented#Glyph'>this</a> <a href='undocumented#Glyph'>selects</a>
<a href='undocumented#Glyph'>FT_LOAD_TARGET_LCD</a> <a href='undocumented#Glyph'>or</a> <a href='undocumented#Glyph'>FT_LOAD_TARGET_LCD_V</a> <a href='undocumented#Glyph'>if</a> <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> <a href='#SkPaint_kLCDRenderText_Flag'>is</a> <a href='#SkPaint_kLCDRenderText_Flag'>set</a>.
<a href='#SkPaint_kLCDRenderText_Flag'>FT_LOAD_TARGET_LCD</a> <a href='#SkPaint_kLCDRenderText_Flag'>is</a> <a href='#SkPaint_kLCDRenderText_Flag'>a</a> <a href='#SkPaint_kLCDRenderText_Flag'>variant</a> <a href='#SkPaint_kLCDRenderText_Flag'>of</a> <a href='#SkPaint_kLCDRenderText_Flag'>FT_LOAD_TARGET_NORMAL</a> <a href='#SkPaint_kLCDRenderText_Flag'>optimized</a> <a href='#SkPaint_kLCDRenderText_Flag'>for</a>
<a href='#SkPaint_kLCDRenderText_Flag'>horizontally</a> <a href='#SkPaint_kLCDRenderText_Flag'>decimated</a> <a href='#SkPaint_kLCDRenderText_Flag'>LCD</a> <a href='#SkPaint_kLCDRenderText_Flag'>displays</a>; <a href='#SkPaint_kLCDRenderText_Flag'>FT_LOAD_TARGET_LCD_V</a> <a href='#SkPaint_kLCDRenderText_Flag'>is</a> <a href='#SkPaint_kLCDRenderText_Flag'>a</a>
<a href='#SkPaint_kLCDRenderText_Flag'>variant</a> <a href='#SkPaint_kLCDRenderText_Flag'>of</a> <a href='#SkPaint_kLCDRenderText_Flag'>FT_LOAD_TARGET_NORMAL</a> <a href='#SkPaint_kLCDRenderText_Flag'>optimized</a> <a href='#SkPaint_kLCDRenderText_Flag'>for</a> <a href='#SkPaint_kLCDRenderText_Flag'>vertically</a> <a href='#SkPaint_kLCDRenderText_Flag'>decimated</a> <a href='#SkPaint_kLCDRenderText_Flag'>LCD</a> <a href='#SkPaint_kLCDRenderText_Flag'>displays</a>.
</td>
  </tr>
</table>

On <a href='#OS_X'>OS_X</a> <a href='#OS_X'>and</a> <a href='#OS_X'>iOS</a>, <a href='#OS_X'>hinting</a> <a href='#OS_X'>controls</a> <a href='#OS_X'>whether</a> <a href='#Core_Graphics'>Core_Graphics</a> <a href='#Core_Graphics'>dilates</a> <a href='#Core_Graphics'>the</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>outlines</a>
<a href='undocumented#Font'>to</a> <a href='undocumented#Font'>account</a> <a href='undocumented#Font'>for</a>  <a href='#LCD_Text'>LCD text</a>. <a href='undocumented#Font'>No</a> <a href='undocumented#Font'>hinting</a> <a href='undocumented#Font'>uses</a> <a href='#Core_Text'>Core_Text</a> <a href='#Core_Text'>grayscale</a> <a href='#Core_Text'>output</a>.
<a href='#Core_Text'>Normal</a> <a href='#Core_Text'>hinting</a> <a href='#Core_Text'>uses</a> <a href='#Core_Text'>Core_Text</a> <a href='#Core_Text'>LCD</a> <a href='#Core_Text'>output</a>. <a href='#Core_Text'>If</a> <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> <a href='#SkPaint_kLCDRenderText_Flag'>is</a> <a href='#SkPaint_kLCDRenderText_Flag'>clear</a>,
<a href='#SkPaint_kLCDRenderText_Flag'>the</a> <a href='#SkPaint_kLCDRenderText_Flag'>LCD</a> <a href='#SkPaint_kLCDRenderText_Flag'>output</a> <a href='#SkPaint_kLCDRenderText_Flag'>is</a> <a href='#SkPaint_kLCDRenderText_Flag'>reduced</a> <a href='#SkPaint_kLCDRenderText_Flag'>to</a> <a href='#SkPaint_kLCDRenderText_Flag'>a</a> <a href='#SkPaint_kLCDRenderText_Flag'>single</a> <a href='#SkPaint_kLCDRenderText_Flag'>grayscale</a> <a href='#SkPaint_kLCDRenderText_Flag'>channel</a>.

<a href='#SkPaint_kLCDRenderText_Flag'>On</a> <a href='#SkPaint_kLCDRenderText_Flag'>Windows</a> <a href='#SkPaint_kLCDRenderText_Flag'>with</a> <a href='#SkPaint_kLCDRenderText_Flag'>DirectWrite</a>, <a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>has</a> <a href='#SkPaint_Hinting'>no</a> <a href='#SkPaint_Hinting'>effect</a>.

<a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>defaults</a> <a href='#SkPaint_Hinting'>to</a> <a href='#SkPaint_kNormal_Hinting'>kNormal_Hinting</a>.
<a href='#SkPaint_kNormal_Hinting'>Set</a> <a href='undocumented#SkPaintDefaults_Hinting'>SkPaintDefaults_Hinting</a> <a href='undocumented#SkPaintDefaults_Hinting'>at</a> <a href='undocumented#SkPaintDefaults_Hinting'>compile</a> <a href='undocumented#SkPaintDefaults_Hinting'>time</a> <a href='undocumented#SkPaintDefaults_Hinting'>to</a> <a href='undocumented#SkPaintDefaults_Hinting'>change</a> <a href='undocumented#SkPaintDefaults_Hinting'>the</a> <a href='undocumented#SkPaintDefaults_Hinting'>default</a> <a href='undocumented#SkPaintDefaults_Hinting'>setting</a>.

<a name='SkPaint_getHinting'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_getHinting'>getHinting</a>() <a href='#SkPaint_getHinting'>const</a>
</pre>

Returns level of <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>outline</a> <a href='undocumented#Glyph'>adjustment</a>.

### Return Value

one of: <a href='#SkPaint_kNo_Hinting'>kNo_Hinting</a>, <a href='#SkPaint_kSlight_Hinting'>kSlight_Hinting</a>, <a href='#SkPaint_kNormal_Hinting'>kNormal_Hinting</a>, <a href='#SkPaint_kFull_Hinting'>kFull_Hinting</a>

### Example

<div><fiddle-embed name="329e2e5a5919ac431e1c58878a5b99e0">

#### Example Output

~~~~
SkPaint::kNormal_Hinting == paint.getHinting()
~~~~

</fiddle-embed></div>

<a name='SkPaint_setHinting'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setHinting'>setHinting</a>(<a href='undocumented#SkFontHinting'>SkFontHinting</a> <a href='undocumented#SkFontHinting'>hintingLevel</a>)
</pre>

Sets level of <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>outline</a> <a href='undocumented#Glyph'>adjustment</a>.
Does not check for valid values of <a href='#SkPaint_setHinting_hintingLevel'>hintingLevel</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setHinting_hintingLevel'><code><strong>hintingLevel</strong></code></a></td>
    <td>one of: <a href='undocumented#kNo_SkFontHinting'>kNo_SkFontHinting</a>, <a href='undocumented#kSlight_SkFontHinting'>kSlight_SkFontHinting</a>,</td>
  </tr>
</table>

<a href='undocumented#kNormal_SkFontHinting'>kNormal_SkFontHinting</a>, <a href='undocumented#kFull_SkFontHinting'>kFull_SkFontHinting</a>

### Example

<div><fiddle-embed name="197268a89c3343f600b9bade61c513ae">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

<a name='SkPaint_setHinting_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setHinting'>setHinting</a>(<a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>hintingLevel</a>)
</pre>

Sets level of <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>outline</a> <a href='undocumented#Glyph'>adjustment</a>.
Does not check for valid values of <a href='#SkPaint_setHinting_2_hintingLevel'>hintingLevel</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setHinting_2_hintingLevel'><code><strong>hintingLevel</strong></code></a></td>
    <td>one of: <a href='#SkPaint_kNo_Hinting'>kNo_Hinting</a>, <a href='#SkPaint_kSlight_Hinting'>kSlight_Hinting</a>, <a href='#SkPaint_kNormal_Hinting'>kNormal_Hinting</a>, <a href='#SkPaint_kFull_Hinting'>kFull_Hinting</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="78153fbd3f1000cb33b97bbe831ed34e">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

<a name='Flags'></a>

<a name='SkPaint_Flags'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Flags'>Flags</a> {
        <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> = 0<a href='#SkPaint_kAntiAlias_Flag'>x01</a>,
        <a href='#SkPaint_kDither_Flag'>kDither_Flag</a> = 0<a href='#SkPaint_kDither_Flag'>x04</a>,
        <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a> = 0<a href='#SkPaint_kFakeBoldText_Flag'>x20</a>,
        <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> = 0<a href='#SkPaint_kLinearText_Flag'>x40</a>,
        <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> = 0<a href='#SkPaint_kSubpixelText_Flag'>x80</a>,
        <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> = 0<a href='#SkPaint_kLCDRenderText_Flag'>x200</a>,
        <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> = 0<a href='#SkPaint_kEmbeddedBitmapText_Flag'>x400</a>,
        <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a> = 0<a href='#SkPaint_kAutoHinting_Flag'>x800</a>,
        <a href='#SkPaint_kAllFlags'>kAllFlags</a> = 0<a href='#SkPaint_kAllFlags'>xFFFF</a>,
    };
</pre>

The bit values stored in <a href='#SkPaint_Flags'>Flags</a>.
<a href='#SkPaint_Flags'>The</a> <a href='#SkPaint_Flags'>default</a> <a href='#SkPaint_Flags'>value</a> <a href='#SkPaint_Flags'>for</a> <a href='#SkPaint_Flags'>Flags</a>, <a href='#SkPaint_Flags'>normally</a> <a href='#SkPaint_Flags'>zero</a>, <a href='#SkPaint_Flags'>can</a> <a href='#SkPaint_Flags'>be</a> <a href='#SkPaint_Flags'>changed</a> <a href='#SkPaint_Flags'>at</a> <a href='#SkPaint_Flags'>compile</a> <a href='#SkPaint_Flags'>time</a>
<a href='#SkPaint_Flags'>with</a> <a href='#SkPaint_Flags'>a</a> <a href='#SkPaint_Flags'>custom</a> <a href='#SkPaint_Flags'>definition</a> <a href='#SkPaint_Flags'>of</a> <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults_Flags</a>.
<a href='undocumented#SkPaintDefaults_Flags'>All</a> <a href='undocumented#SkPaintDefaults_Flags'>flags</a> <a href='undocumented#SkPaintDefaults_Flags'>can</a> <a href='undocumented#SkPaintDefaults_Flags'>be</a> <a href='undocumented#SkPaintDefaults_Flags'>read</a> <a href='undocumented#SkPaintDefaults_Flags'>and</a> <a href='undocumented#SkPaintDefaults_Flags'>written</a> <a href='undocumented#SkPaintDefaults_Flags'>explicitly</a>; <a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_Flags'>allows</a> <a href='#SkPaint_Flags'>manipulating</a>
<a href='#SkPaint_Flags'>multiple</a> <a href='#SkPaint_Flags'>settings</a> <a href='#SkPaint_Flags'>at</a> <a href='#SkPaint_Flags'>once</a>.

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
mask for setting Fake_Bold</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kLinearText_Flag'><code>SkPaint::kLinearText_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0040</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask for setting Linear_Text</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kSubpixelText_Flag'><code>SkPaint::kSubpixelText_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0080</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask for setting Subpixel_Text</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kLCDRenderText_Flag'><code>SkPaint::kLCDRenderText_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0200</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask for setting LCD_Text</td>
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
mask for setting Automatic_Hinting</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kAllFlags'><code>SkPaint::kAllFlags</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask of all <a href='#SkPaint_Flags'>Flags</a>, <a href='#SkPaint_Flags'>including</a> <a href='#SkPaint_Flags'>private</a> <a href='#SkPaint_Flags'>flags</a> <a href='#SkPaint_Flags'>and</a> <a href='#SkPaint_Flags'>flags</a> <a href='#SkPaint_Flags'>reserved</a> <a href='#SkPaint_Flags'>for</a> <a href='#SkPaint_Flags'>future</a> <a href='#SkPaint_Flags'>use</a>
</td>
  </tr>
<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_Flags'>default</a> <a href='#SkPaint_Flags'>to</a> <a href='#SkPaint_Flags'>all</a> <a href='#SkPaint_Flags'>flags</a> <a href='#SkPaint_Flags'>clear</a>, <a href='#SkPaint_Flags'>disabling</a> <a href='#SkPaint_Flags'>the</a> <a href='#SkPaint_Flags'>associated</a> <a href='#SkPaint_Flags'>feature</a>.
</table>

<a name='SkPaint_ReserveFlags'></a>

---

To be deprecated soon.

Only valid for Android framework.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_ReserveFlags'>ReserveFlags</a> {
        <a href='#SkPaint_kUnderlineText_ReserveFlag'>kUnderlineText_ReserveFlag</a> = 0<a href='#SkPaint_kUnderlineText_ReserveFlag'>x08</a>,
        <a href='#SkPaint_kStrikeThruText_ReserveFlag'>kStrikeThruText_ReserveFlag</a> = 0<a href='#SkPaint_kStrikeThruText_ReserveFlag'>x10</a>,
    };
</pre>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kUnderlineText_ReserveFlag'><code>SkPaint::kUnderlineText_ReserveFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0008</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
To be deprecated soon.

</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kStrikeThruText_ReserveFlag'><code>SkPaint::kStrikeThruText_ReserveFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0010</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
To be deprecated soon.

</td>
  </tr>
</table>

<a name='SkPaint_getFlags'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPaint_getFlags'>getFlags</a>() <a href='#SkPaint_getFlags'>const</a>
</pre>

Returns <a href='SkPaint_Reference#Paint'>paint</a> <a href='SkPaint_Reference#Paint'>settings</a> <a href='SkPaint_Reference#Paint'>described</a> <a href='SkPaint_Reference#Paint'>by</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a>. <a href='#SkPaint_Flags'>Each</a> <a href='#SkPaint_Flags'>setting</a> <a href='#SkPaint_Flags'>uses</a> <a href='#SkPaint_Flags'>one</a>
bit, and can be tested with <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_Flags'>members</a>.

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
void <a href='#SkPaint_setFlags'>setFlags</a>(<a href='#SkPaint_setFlags'>uint32_t</a> <a href='#SkPaint_setFlags'>flags</a>)
</pre>

Replaces <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_Flags'>with</a> <a href='#SkPaint_setFlags_flags'>flags</a>, <a href='#SkPaint_setFlags_flags'>the</a> <a href='#SkPaint_setFlags_flags'>union</a> <a href='#SkPaint_setFlags_flags'>of</a> <a href='#SkPaint_setFlags_flags'>the</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_Flags'>members</a>.
All <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_Flags'>members</a> <a href='#SkPaint_Flags'>may</a> <a href='#SkPaint_Flags'>be</a> <a href='#SkPaint_Flags'>cleared</a>, <a href='#SkPaint_Flags'>or</a> <a href='#SkPaint_Flags'>one</a> <a href='#SkPaint_Flags'>or</a> <a href='#SkPaint_Flags'>more</a> <a href='#SkPaint_Flags'>may</a> <a href='#SkPaint_Flags'>be</a> <a href='#SkPaint_Flags'>set</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setFlags_flags'><code><strong>flags</strong></code></a></td>
    <td>union of <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_Flags'>for</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a></td>
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

<a href='#Paint_Anti_Alias'>Anti_Alias</a> <a href='#Paint_Anti_Alias'>drawing</a> <a href='#Paint_Anti_Alias'>approximates</a> <a href='#Paint_Anti_Alias'>partial</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>coverage</a> <a href='undocumented#Pixel'>with</a> <a href='undocumented#Pixel'>transparency</a>.
<a href='undocumented#Pixel'>If</a> <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> <a href='#SkPaint_kAntiAlias_Flag'>is</a> <a href='#SkPaint_kAntiAlias_Flag'>clear</a>, <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>centers</a> <a href='undocumented#Pixel'>contained</a> <a href='undocumented#Pixel'>by</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>shape</a> <a href='undocumented#Pixel'>edge</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>drawn</a> <a href='undocumented#Pixel'>opaque</a>.
<a href='undocumented#Pixel'>If</a> <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> <a href='#SkPaint_kAntiAlias_Flag'>is</a> <a href='#SkPaint_kAntiAlias_Flag'>set</a>, <a href='#SkPaint_kAntiAlias_Flag'>pixels</a> <a href='#SkPaint_kAntiAlias_Flag'>are</a> <a href='#SkPaint_kAntiAlias_Flag'>drawn</a> <a href='#SkPaint_kAntiAlias_Flag'>with</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>equal</a> <a href='#Color_Alpha'>to</a> <a href='#Color_Alpha'>their</a> <a href='#Color_Alpha'>coverage</a>.

<a href='#Color_Alpha'>The</a> <a href='#Color_Alpha'>rule</a> <a href='#Color_Alpha'>for</a> <a href='undocumented#Alias'>Aliased</a> <a href='undocumented#Alias'>pixels</a> <a href='undocumented#Alias'>is</a> <a href='undocumented#Alias'>inconsistent</a> <a href='undocumented#Alias'>across</a> <a href='undocumented#Alias'>platforms</a>. <a href='undocumented#Alias'>A</a> <a href='undocumented#Alias'>shape</a> <a href='undocumented#Alias'>edge</a>
<a href='undocumented#Alias'>passing</a> <a href='undocumented#Alias'>through</a> <a href='undocumented#Alias'>the</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>center</a> <a href='undocumented#Pixel'>may</a>, <a href='undocumented#Pixel'>but</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>required</a> <a href='undocumented#Pixel'>to</a>, <a href='undocumented#Pixel'>draw</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>pixel</a>.

<a href='#Raster_Engine'>Raster_Engine</a> <a href='#Raster_Engine'>draws</a> <a href='undocumented#Alias'>Aliased</a> <a href='undocumented#Alias'>pixels</a> <a href='undocumented#Alias'>whose</a> <a href='undocumented#Alias'>centers</a> <a href='undocumented#Alias'>are</a> <a href='undocumented#Alias'>on</a> <a href='undocumented#Alias'>or</a> <a href='undocumented#Alias'>to</a> <a href='undocumented#Alias'>the</a> <a href='undocumented#Alias'>right</a> <a href='undocumented#Alias'>of</a> <a href='undocumented#Alias'>the</a> <a href='undocumented#Alias'>start</a> <a href='undocumented#Alias'>of</a> <a href='undocumented#Alias'>an</a>
<a href='undocumented#Alias'>active</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>edge</a>, <a href='SkPath_Reference#Path'>and</a> <a href='SkPath_Reference#Path'>whose</a> <a href='SkPath_Reference#Path'>center</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>left</a> <a href='SkPath_Reference#Path'>of</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>end</a> <a href='SkPath_Reference#Path'>of</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>active</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>edge</a>.

A platform may only support <a href='#Paint_Anti_Alias'>Anti_Aliased</a> <a href='#Paint_Anti_Alias'>drawing</a>. <a href='#Paint_Anti_Alias'>Some</a> <a href='#Paint_Anti_Alias'>GPU-backed</a> <a href='#Paint_Anti_Alias'>platforms</a> <a href='#Paint_Anti_Alias'>use</a>
<a href='undocumented#Supersampling'>Supersampling</a> <a href='undocumented#Supersampling'>to</a> <a href='#Paint_Anti_Alias'>Anti_Alias</a> <a href='#Paint_Anti_Alias'>all</a> <a href='#Paint_Anti_Alias'>drawing</a>, <a href='#Paint_Anti_Alias'>and</a> <a href='#Paint_Anti_Alias'>have</a> <a href='#Paint_Anti_Alias'>no</a> <a href='#Paint_Anti_Alias'>mechanism</a> <a href='#Paint_Anti_Alias'>to</a> <a href='#Paint_Anti_Alias'>selectively</a>
<a href='undocumented#Alias'>Alias</a>.

<a href='undocumented#Alias'>The</a> <a href='undocumented#Alias'>amount</a> <a href='undocumented#Alias'>of</a> <a href='undocumented#Alias'>coverage</a> <a href='undocumented#Alias'>computed</a> <a href='undocumented#Alias'>for</a> <a href='#Paint_Anti_Alias'>Anti_Aliased</a> <a href='#Paint_Anti_Alias'>pixels</a> <a href='#Paint_Anti_Alias'>also</a> <a href='#Paint_Anti_Alias'>varies</a> <a href='#Paint_Anti_Alias'>across</a> <a href='#Paint_Anti_Alias'>platforms</a>.

<a href='#Paint_Anti_Alias'>Anti_Alias</a> <a href='#Paint_Anti_Alias'>is</a> <a href='#Paint_Anti_Alias'>disabled</a> <a href='#Paint_Anti_Alias'>by</a> <a href='#Paint_Anti_Alias'>default</a>.
<a href='#Paint_Anti_Alias'>Anti_Alias</a> <a href='#Paint_Anti_Alias'>can</a> <a href='#Paint_Anti_Alias'>be</a> <a href='#Paint_Anti_Alias'>enabled</a> <a href='#Paint_Anti_Alias'>by</a> <a href='#Paint_Anti_Alias'>default</a> <a href='#Paint_Anti_Alias'>by</a> <a href='#Paint_Anti_Alias'>setting</a> <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults_Flags</a> <a href='undocumented#SkPaintDefaults_Flags'>to</a> <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a>
<a href='#SkPaint_kAntiAlias_Flag'>at</a> <a href='#SkPaint_kAntiAlias_Flag'>compile</a> <a href='#SkPaint_kAntiAlias_Flag'>time</a>.

### Example

<div><fiddle-embed name="a6575a49467ce8d28bb01cc7638fa04d"><div>A red <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>is</a> <a href='undocumented#Line'>drawn</a> <a href='undocumented#Line'>with</a> <a href='undocumented#Line'>transparency</a> <a href='undocumented#Line'>on</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>edges</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Line'>make</a> <a href='undocumented#Line'>it</a> <a href='undocumented#Line'>look</a> <a href='undocumented#Line'>smoother</a>.
<a href='undocumented#Line'>A</a> <a href='undocumented#Line'>blue</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>draws</a> <a href='undocumented#Line'>only</a> <a href='undocumented#Line'>where</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>centers</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>contained</a>.
<a href='undocumented#Pixel'>The</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>are</a> <a href='undocumented#Line'>drawn</a> <a href='undocumented#Line'>into</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>then</a> <a href='SkBitmap_Reference#Bitmap'>drawn</a> <a href='SkBitmap_Reference#Bitmap'>magnified</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>make</a> <a href='SkBitmap_Reference#Bitmap'>the</a>
<a href='undocumented#Alias'>Aliasing</a> <a href='undocumented#Alias'>easier</a> <a href='undocumented#Alias'>to</a> <a href='undocumented#Alias'>see</a>.
</div></fiddle-embed></div>

<a name='SkPaint_isAntiAlias'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isAntiAlias'>isAntiAlias</a>() <a href='#SkPaint_isAntiAlias'>const</a>
</pre>

Returns true if pixels on the active edges of <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>be</a> <a href='SkPath_Reference#SkPath'>drawn</a> <a href='SkPath_Reference#SkPath'>with</a> <a href='SkPath_Reference#SkPath'>partial</a> <a href='SkPath_Reference#SkPath'>transparency</a>.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() <a href='#SkPaint_getFlags'>masked</a> <a href='#SkPaint_getFlags'>with</a> <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a>.

### Return Value

<a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> <a href='#SkPaint_kAntiAlias_Flag'>state</a>

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
void <a href='#SkPaint_setAntiAlias'>setAntiAlias</a>(<a href='#SkPaint_setAntiAlias'>bool</a> <a href='#SkPaint_setAntiAlias'>aa</a>)
</pre>

Requests, but does not require, that <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>edge</a> <a href='SkPath_Reference#SkPath'>pixels</a> <a href='SkPath_Reference#SkPath'>draw</a> <a href='SkPath_Reference#SkPath'>opaque</a> <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>with</a>
partial transparency.

Sets <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> <a href='#SkPaint_kAntiAlias_Flag'>if</a> <a href='#SkPaint_setAntiAlias_aa'>aa</a> <a href='#SkPaint_setAntiAlias_aa'>is</a> <a href='#SkPaint_setAntiAlias_aa'>true</a>.
Clears <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> <a href='#SkPaint_kAntiAlias_Flag'>if</a> <a href='#SkPaint_setAntiAlias_aa'>aa</a> <a href='#SkPaint_setAntiAlias_aa'>is</a> <a href='#SkPaint_setAntiAlias_aa'>false</a>.

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

Dither increases fidelity by adjusting the <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>adjacent</a> <a href='SkColor_Reference#Color'>pixels</a>.
<a href='SkColor_Reference#Color'>This</a> <a href='SkColor_Reference#Color'>can</a> <a href='SkColor_Reference#Color'>help</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>smooth</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>transitions</a> <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>reducing</a> <a href='SkColor_Reference#Color'>banding</a> <a href='SkColor_Reference#Color'>in</a> <a href='SkColor_Reference#Color'>gradients</a>.
<a href='SkColor_Reference#Color'>Dithering</a> <a href='SkColor_Reference#Color'>lessens</a> <a href='SkColor_Reference#Color'>visible</a> <a href='SkColor_Reference#Color'>banding</a> <a href='SkColor_Reference#Color'>from</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>
<a href='SkImageInfo_Reference#kRGB_565_SkColorType'>and</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>gradients</a>,
<a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>and</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>improves</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>rendering</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>into</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>a</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

<a href='SkSurface_Reference#Surface'>Dithering</a> <a href='SkSurface_Reference#Surface'>is</a> <a href='SkSurface_Reference#Surface'>always</a> <a href='SkSurface_Reference#Surface'>enabled</a> <a href='SkSurface_Reference#Surface'>for</a> <a href='SkSurface_Reference#Surface'>linear</a> <a href='SkSurface_Reference#Surface'>gradients</a> <a href='SkSurface_Reference#Surface'>drawing</a> <a href='SkSurface_Reference#Surface'>into</a>
<a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>and</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.
<a href='SkSurface_Reference#Surface'>Dither</a> <a href='SkSurface_Reference#Surface'>cannot</a> <a href='SkSurface_Reference#Surface'>be</a> <a href='SkSurface_Reference#Surface'>enabled</a> <a href='SkSurface_Reference#Surface'>for</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>and</a>
<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

<a href='SkSurface_Reference#Surface'>Dither</a> <a href='SkSurface_Reference#Surface'>is</a> <a href='SkSurface_Reference#Surface'>disabled</a> <a href='SkSurface_Reference#Surface'>by</a> <a href='SkSurface_Reference#Surface'>default</a>.
<a href='SkSurface_Reference#Surface'>Dither</a> <a href='SkSurface_Reference#Surface'>can</a> <a href='SkSurface_Reference#Surface'>be</a> <a href='SkSurface_Reference#Surface'>enabled</a> <a href='SkSurface_Reference#Surface'>by</a> <a href='SkSurface_Reference#Surface'>default</a> <a href='SkSurface_Reference#Surface'>by</a> <a href='SkSurface_Reference#Surface'>setting</a> <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults_Flags</a> <a href='undocumented#SkPaintDefaults_Flags'>to</a> <a href='#SkPaint_kDither_Flag'>kDither_Flag</a>
<a href='#SkPaint_kDither_Flag'>at</a> <a href='#SkPaint_kDither_Flag'>compile</a> <a href='#SkPaint_kDither_Flag'>time</a>.

<a href='#SkPaint_kDither_Flag'>Some</a> <a href='#SkPaint_kDither_Flag'>platform</a> <a href='#SkPaint_kDither_Flag'>implementations</a> <a href='#SkPaint_kDither_Flag'>may</a> <a href='#SkPaint_kDither_Flag'>ignore</a> <a href='#SkPaint_kDither_Flag'>dithering</a>. <a href='#SkPaint_kDither_Flag'>Set</a> <code>SK_IGNORE_GPU_DITHER</code>to ignore Dither on <a href='#GPU_Surface'>GPU_Surface</a>.

### Example

<div><fiddle-embed name="8b26507690b71462f44642b911890bbf"><div>Dithering in the bottom half more closely approximates the requested <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>by</a>
<a href='SkColor_Reference#Color'>alternating</a> <a href='SkColor_Reference#Color'>nearby</a> <a href='SkColor_Reference#Color'>colors</a> <a href='SkColor_Reference#Color'>from</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>pixel</a>.
</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="76d4d4a7931a48495e4d5f54e073be53"><div>Dithering introduces subtle adjustments to <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>smooth</a> <a href='SkColor_Reference#Color'>gradients</a>.
<a href='SkColor_Reference#Color'>Drawing</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>gradient</a> <a href='SkColor_Reference#Color'>repeatedly</a> <a href='SkColor_Reference#Color'>with</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kPlus'>kPlus</a> <a href='#SkBlendMode_kPlus'>exaggerates</a> <a href='#SkBlendMode_kPlus'>the</a>
<a href='#SkBlendMode_kPlus'>dither</a>, <a href='#SkBlendMode_kPlus'>making</a> <a href='#SkBlendMode_kPlus'>it</a> <a href='#SkBlendMode_kPlus'>easier</a> <a href='#SkBlendMode_kPlus'>to</a> <a href='#SkBlendMode_kPlus'>see</a>.
</div></fiddle-embed></div>

### See Also

Gradient <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>

<a name='SkPaint_isDither'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isDither'>isDither</a>() <a href='#SkPaint_isDither'>const</a>
</pre>

Returns true if <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>error</a> <a href='SkColor_Reference#Color'>may</a> <a href='SkColor_Reference#Color'>be</a> <a href='SkColor_Reference#Color'>distributed</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>smooth</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>transition</a>.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() <a href='#SkPaint_getFlags'>masked</a> <a href='#SkPaint_getFlags'>with</a> <a href='#SkPaint_kDither_Flag'>kDither_Flag</a>.

### Return Value

<a href='#SkPaint_kDither_Flag'>kDither_Flag</a> <a href='#SkPaint_kDither_Flag'>state</a>

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
void <a href='#SkPaint_setDither'>setDither</a>(<a href='#SkPaint_setDither'>bool</a> <a href='#SkPaint_setDither'>dither</a>)
</pre>

Requests, but does not require, to distribute <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>error</a>.

Sets <a href='#SkPaint_kDither_Flag'>kDither_Flag</a> <a href='#SkPaint_kDither_Flag'>if</a> <a href='#SkPaint_setDither_dither'>dither</a> <a href='#SkPaint_setDither_dither'>is</a> <a href='#SkPaint_setDither_dither'>true</a>.
Clears <a href='#SkPaint_kDither_Flag'>kDither_Flag</a> <a href='#SkPaint_kDither_Flag'>if</a> <a href='#SkPaint_setDither_dither'>dither</a> <a href='#SkPaint_setDither_dither'>is</a> <a href='#SkPaint_setDither_dither'>false</a>.

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

<a href='#Paint_LCD_Text'>LCD_Text</a> <a href='#Paint_LCD_Text'>and</a> <a href='#Paint_Subpixel_Text'>Subpixel_Text</a> <a href='#Paint_Subpixel_Text'>increase</a> <a href='#Paint_Subpixel_Text'>the</a> <a href='#Paint_Subpixel_Text'>precision</a> <a href='#Paint_Subpixel_Text'>of</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>position</a>.

<a href='undocumented#Glyph'>When</a> <a href='undocumented#Glyph'>set</a>, <a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> <a href='#SkPaint_kLCDRenderText_Flag'>takes</a> <a href='#SkPaint_kLCDRenderText_Flag'>advantage</a> <a href='#SkPaint_kLCDRenderText_Flag'>of</a> <a href='#SkPaint_kLCDRenderText_Flag'>the</a> <a href='#SkPaint_kLCDRenderText_Flag'>organization</a> <a href='#SkPaint_kLCDRenderText_Flag'>of</a> <a href='#SkPaint_kLCDRenderText_Flag'>RGB</a> <a href='#SkPaint_kLCDRenderText_Flag'>stripes</a> <a href='#SkPaint_kLCDRenderText_Flag'>that</a>
<a href='#SkPaint_kLCDRenderText_Flag'>create</a> <a href='#SkPaint_kLCDRenderText_Flag'>a</a> <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>relies</a>
<a href='SkColor_Reference#Color'>on</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>small</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>stripe</a> <a href='undocumented#Size'>and</a> <a href='undocumented#Size'>visual</a> <a href='undocumented#Size'>perception</a> <a href='undocumented#Size'>to</a> <a href='undocumented#Size'>make</a> <a href='undocumented#Size'>the</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>fringing</a> <a href='SkColor_Reference#Color'>imperceptible</a>.
<a href='#Paint_LCD_Text'>LCD_Text</a> <a href='#Paint_LCD_Text'>can</a> <a href='#Paint_LCD_Text'>be</a> <a href='#Paint_LCD_Text'>enabled</a> <a href='#Paint_LCD_Text'>on</a> <a href='#Paint_LCD_Text'>devices</a> <a href='#Paint_LCD_Text'>that</a> <a href='#Paint_LCD_Text'>orient</a> <a href='#Paint_LCD_Text'>stripes</a> <a href='#Paint_LCD_Text'>horizontally</a> <a href='#Paint_LCD_Text'>or</a> <a href='#Paint_LCD_Text'>vertically</a>, <a href='#Paint_LCD_Text'>and</a> <a href='#Paint_LCD_Text'>that</a> <a href='#Paint_LCD_Text'>order</a>
<a href='#Paint_LCD_Text'>the</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>components</a> <a href='SkColor_Reference#Color'>as</a> <a href='SkColor_Reference#Color'>RGB</a> <a href='SkColor_Reference#Color'>or</a> <a href='SkColor_Reference#Color'>BGR</a>.

<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> <a href='#SkPaint_kSubpixelText_Flag'>uses</a> <a href='#SkPaint_kSubpixelText_Flag'>the</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>transparency</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>represent</a> <a href='undocumented#Pixel'>a</a> <a href='undocumented#Pixel'>fractional</a> <a href='undocumented#Pixel'>offset</a>.
<a href='undocumented#Pixel'>As</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>opaqueness</a>
<a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>the</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>increases</a>, <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>edge</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>appears</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>move</a> <a href='undocumented#Glyph'>towards</a> <a href='undocumented#Glyph'>the</a> <a href='undocumented#Glyph'>outside</a> <a href='undocumented#Glyph'>of</a> <a href='undocumented#Glyph'>the</a> <a href='undocumented#Pixel'>pixel</a>.

<a href='undocumented#Pixel'>Either</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>both</a> <a href='undocumented#Pixel'>techniques</a> <a href='undocumented#Pixel'>can</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>enabled</a>.
<a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> <a href='#SkPaint_kLCDRenderText_Flag'>and</a> <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> <a href='#SkPaint_kSubpixelText_Flag'>are</a> <a href='#SkPaint_kSubpixelText_Flag'>clear</a> <a href='#SkPaint_kSubpixelText_Flag'>by</a> <a href='#SkPaint_kSubpixelText_Flag'>default</a>.
<a href='#Paint_LCD_Text'>LCD_Text</a> <a href='#Paint_LCD_Text'>or</a> <a href='#Paint_Subpixel_Text'>Subpixel_Text</a> <a href='#Paint_Subpixel_Text'>can</a> <a href='#Paint_Subpixel_Text'>be</a> <a href='#Paint_Subpixel_Text'>enabled</a> <a href='#Paint_Subpixel_Text'>by</a> <a href='#Paint_Subpixel_Text'>default</a> <a href='#Paint_Subpixel_Text'>by</a> <a href='#Paint_Subpixel_Text'>setting</a> <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults_Flags</a> <a href='undocumented#SkPaintDefaults_Flags'>to</a>
<a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> <a href='#SkPaint_kLCDRenderText_Flag'>or</a> <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> (<a href='#SkPaint_kSubpixelText_Flag'>or</a> <a href='#SkPaint_kSubpixelText_Flag'>both</a>) <a href='#SkPaint_kSubpixelText_Flag'>at</a> <a href='#SkPaint_kSubpixelText_Flag'>compile</a> <a href='#SkPaint_kSubpixelText_Flag'>time</a>.

### Example

<div><fiddle-embed name="4606ae1be792d6bc46d496432f050ee9"><div>Four commas are drawn normally and with combinations of <a href='#Paint_LCD_Text'>LCD_Text</a> <a href='#Paint_LCD_Text'>and</a> <a href='#Paint_Subpixel_Text'>Subpixel_Text</a>.
<a href='#Paint_Subpixel_Text'>When</a> <a href='#Paint_Subpixel_Text'>Subpixel_Text</a> <a href='#Paint_Subpixel_Text'>is</a> <a href='#Paint_Subpixel_Text'>disabled</a>, <a href='#Paint_Subpixel_Text'>the</a> <a href='#Paint_Subpixel_Text'>comma</a> <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>are</a> <a href='undocumented#Glyph'>identical</a>, <a href='undocumented#Glyph'>but</a> <a href='undocumented#Glyph'>not</a> <a href='undocumented#Glyph'>evenly</a> <a href='undocumented#Glyph'>spaced</a>.
<a href='undocumented#Glyph'>When</a> <a href='#Paint_Subpixel_Text'>Subpixel_Text</a> <a href='#Paint_Subpixel_Text'>is</a> <a href='#Paint_Subpixel_Text'>enabled</a>, <a href='#Paint_Subpixel_Text'>the</a> <a href='#Paint_Subpixel_Text'>comma</a> <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>are</a> <a href='undocumented#Glyph'>unique</a>, <a href='undocumented#Glyph'>but</a> <a href='undocumented#Glyph'>appear</a> <a href='undocumented#Glyph'>evenly</a> <a href='undocumented#Glyph'>spaced</a>.
</div></fiddle-embed></div>

<a name='Linear_Text'></a>

<a href='#Paint_Linear_Text'>Linear_Text</a> <a href='#Paint_Linear_Text'>selects</a> <a href='#Paint_Linear_Text'>whether</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>is</a> <a href='undocumented#Text'>rendered</a> <a href='undocumented#Text'>as</a> <a href='undocumented#Text'>a</a> <a href='undocumented#Glyph'>Glyph</a> <a href='undocumented#Glyph'>or</a> <a href='undocumented#Glyph'>as</a> <a href='undocumented#Glyph'>a</a> <a href='SkPath_Reference#Path'>Path</a>.
<a href='SkPath_Reference#Path'>If</a> <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> <a href='#SkPaint_kLinearText_Flag'>is</a> <a href='#SkPaint_kLinearText_Flag'>set</a>, <a href='#SkPaint_kLinearText_Flag'>it</a> <a href='#SkPaint_kLinearText_Flag'>has</a> <a href='#SkPaint_kLinearText_Flag'>the</a> <a href='#SkPaint_kLinearText_Flag'>same</a> <a href='#SkPaint_kLinearText_Flag'>effect</a> <a href='#SkPaint_kLinearText_Flag'>as</a> <a href='#SkPaint_kLinearText_Flag'>setting</a> <a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>to</a> <a href='#SkPaint_kNormal_Hinting'>kNormal_Hinting</a>.
<a href='#SkPaint_kNormal_Hinting'>If</a> <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> <a href='#SkPaint_kLinearText_Flag'>is</a> <a href='#SkPaint_kLinearText_Flag'>clear</a>, <a href='#SkPaint_kLinearText_Flag'>it</a> <a href='#SkPaint_kLinearText_Flag'>is</a> <a href='#SkPaint_kLinearText_Flag'>the</a> <a href='#SkPaint_kLinearText_Flag'>same</a> <a href='#SkPaint_kLinearText_Flag'>as</a> <a href='#SkPaint_kLinearText_Flag'>setting</a> <a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>to</a> <a href='#SkPaint_kNo_Hinting'>kNo_Hinting</a>.

<a name='SkPaint_isLinearText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isLinearText'>isLinearText</a>() <a href='#SkPaint_isLinearText'>const</a>
</pre>

Returns true if <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>is</a> <a href='undocumented#Text'>converted</a> <a href='undocumented#Text'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>before</a> <a href='SkPath_Reference#SkPath'>drawing</a> <a href='SkPath_Reference#SkPath'>and</a> <a href='SkPath_Reference#SkPath'>measuring</a>.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() <a href='#SkPaint_getFlags'>masked</a> <a href='#SkPaint_getFlags'>with</a> <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a>.

### Return Value

<a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> <a href='#SkPaint_kLinearText_Flag'>state</a>

### Example

<div><fiddle-embed name="2890ad644f980637837e6fcb386fb462"></fiddle-embed></div>

### See Also

<a href='#SkPaint_setLinearText'>setLinearText</a> <a href='#SkPaint_Hinting'>Hinting</a>

<a name='SkPaint_setLinearText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setLinearText'>setLinearText</a>(<a href='#SkPaint_setLinearText'>bool</a> <a href='#SkPaint_setLinearText'>linearText</a>)
</pre>

Returns true if <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>is</a> <a href='undocumented#Text'>converted</a> <a href='undocumented#Text'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>before</a> <a href='SkPath_Reference#SkPath'>drawing</a> <a href='SkPath_Reference#SkPath'>and</a> <a href='SkPath_Reference#SkPath'>measuring</a>.
By default, <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> <a href='#SkPaint_kLinearText_Flag'>is</a> <a href='#SkPaint_kLinearText_Flag'>clear</a>.

Sets <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> <a href='#SkPaint_kLinearText_Flag'>if</a> <a href='#SkPaint_setLinearText_linearText'>linearText</a> <a href='#SkPaint_setLinearText_linearText'>is</a> <a href='#SkPaint_setLinearText_linearText'>true</a>.
Clears <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> <a href='#SkPaint_kLinearText_Flag'>if</a> <a href='#SkPaint_setLinearText_linearText'>linearText</a> <a href='#SkPaint_setLinearText_linearText'>is</a> <a href='#SkPaint_setLinearText_linearText'>false</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setLinearText_linearText'><code><strong>linearText</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c93bb912f3bddfb4d96d3ad70ada552b"></fiddle-embed></div>

### See Also

<a href='#SkPaint_isLinearText'>isLinearText</a> <a href='#SkPaint_Hinting'>Hinting</a>

<a name='Subpixel_Text'></a>

<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> <a href='#SkPaint_kSubpixelText_Flag'>uses</a> <a href='#SkPaint_kSubpixelText_Flag'>the</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>transparency</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>represent</a> <a href='undocumented#Pixel'>a</a> <a href='undocumented#Pixel'>fractional</a> <a href='undocumented#Pixel'>offset</a>.
<a href='undocumented#Pixel'>As</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>opaqueness</a>
<a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>the</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>increases</a>, <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>edge</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>appears</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>move</a> <a href='undocumented#Glyph'>towards</a> <a href='undocumented#Glyph'>the</a> <a href='undocumented#Glyph'>outside</a> <a href='undocumented#Glyph'>of</a> <a href='undocumented#Glyph'>the</a> <a href='undocumented#Pixel'>pixel</a>.

<a name='SkPaint_isSubpixelText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isSubpixelText'>isSubpixelText</a>() <a href='#SkPaint_isSubpixelText'>const</a>
</pre>

Returns true if <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>at</a> <a href='undocumented#Glyph'>different</a> <a href='undocumented#Glyph'>sub-pixel</a> <a href='undocumented#Glyph'>positions</a> <a href='undocumented#Glyph'>may</a> <a href='undocumented#Glyph'>differ</a> <a href='undocumented#Glyph'>on</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>edge</a> <a href='undocumented#Pixel'>coverage</a>.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() <a href='#SkPaint_getFlags'>masked</a> <a href='#SkPaint_getFlags'>with</a> <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a>.

### Return Value

<a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> <a href='#SkPaint_kSubpixelText_Flag'>state</a>

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
void <a href='#SkPaint_setSubpixelText'>setSubpixelText</a>(<a href='#SkPaint_setSubpixelText'>bool</a> <a href='#SkPaint_setSubpixelText'>subpixelText</a>)
</pre>

Requests, but does not require, that <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>respect</a> <a href='undocumented#Glyph'>sub-pixel</a> <a href='undocumented#Glyph'>positioning</a>.

Sets <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> <a href='#SkPaint_kSubpixelText_Flag'>if</a> <a href='#SkPaint_setSubpixelText_subpixelText'>subpixelText</a> <a href='#SkPaint_setSubpixelText_subpixelText'>is</a> <a href='#SkPaint_setSubpixelText_subpixelText'>true</a>.
Clears <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> <a href='#SkPaint_kSubpixelText_Flag'>if</a> <a href='#SkPaint_setSubpixelText_subpixelText'>subpixelText</a> <a href='#SkPaint_setSubpixelText_subpixelText'>is</a> <a href='#SkPaint_setSubpixelText_subpixelText'>false</a>.

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

When set, <a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> <a href='#SkPaint_kLCDRenderText_Flag'>takes</a> <a href='#SkPaint_kLCDRenderText_Flag'>advantage</a> <a href='#SkPaint_kLCDRenderText_Flag'>of</a> <a href='#SkPaint_kLCDRenderText_Flag'>the</a> <a href='#SkPaint_kLCDRenderText_Flag'>organization</a> <a href='#SkPaint_kLCDRenderText_Flag'>of</a> <a href='#SkPaint_kLCDRenderText_Flag'>RGB</a> <a href='#SkPaint_kLCDRenderText_Flag'>stripes</a> <a href='#SkPaint_kLCDRenderText_Flag'>that</a>
<a href='#SkPaint_kLCDRenderText_Flag'>create</a> <a href='#SkPaint_kLCDRenderText_Flag'>a</a> <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>relies</a>
<a href='SkColor_Reference#Color'>on</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>small</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>stripe</a> <a href='undocumented#Size'>and</a> <a href='undocumented#Size'>visual</a> <a href='undocumented#Size'>perception</a> <a href='undocumented#Size'>to</a> <a href='undocumented#Size'>make</a> <a href='undocumented#Size'>the</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>fringing</a> <a href='SkColor_Reference#Color'>imperceptible</a>.
<a href='#Paint_LCD_Text'>LCD_Text</a> <a href='#Paint_LCD_Text'>can</a> <a href='#Paint_LCD_Text'>be</a> <a href='#Paint_LCD_Text'>enabled</a> <a href='#Paint_LCD_Text'>on</a> <a href='#Paint_LCD_Text'>devices</a> <a href='#Paint_LCD_Text'>that</a> <a href='#Paint_LCD_Text'>orient</a> <a href='#Paint_LCD_Text'>stripes</a> <a href='#Paint_LCD_Text'>horizontally</a> <a href='#Paint_LCD_Text'>or</a> <a href='#Paint_LCD_Text'>vertically</a>, <a href='#Paint_LCD_Text'>and</a> <a href='#Paint_LCD_Text'>that</a> <a href='#Paint_LCD_Text'>order</a>
<a href='#Paint_LCD_Text'>the</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>components</a> <a href='SkColor_Reference#Color'>as</a> <a href='SkColor_Reference#Color'>RGB</a> <a href='SkColor_Reference#Color'>or</a> <a href='SkColor_Reference#Color'>BGR</a>.

<a name='SkPaint_isLCDRenderText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isLCDRenderText'>isLCDRenderText</a>() <a href='#SkPaint_isLCDRenderText'>const</a>
</pre>

Returns true if <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>may</a> <a href='undocumented#Glyph'>use</a> <a href='undocumented#Glyph'>LCD</a> <a href='undocumented#Glyph'>striping</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>improve</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>edges</a>.

Returns true if <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> <a href='#SkPaint_kLCDRenderText_Flag'>is</a> <a href='#SkPaint_kLCDRenderText_Flag'>set</a>.

### Return Value

<a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> <a href='#SkPaint_kLCDRenderText_Flag'>state</a>

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
void <a href='#SkPaint_setLCDRenderText'>setLCDRenderText</a>(<a href='#SkPaint_setLCDRenderText'>bool</a> <a href='#SkPaint_setLCDRenderText'>lcdText</a>)
</pre>

Requests, but does not require, that <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>use</a> <a href='undocumented#Glyph'>LCD</a> <a href='undocumented#Glyph'>striping</a> <a href='undocumented#Glyph'>for</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>edges</a>.

Sets <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> <a href='#SkPaint_kLCDRenderText_Flag'>if</a> <a href='#SkPaint_setLCDRenderText_lcdText'>lcdText</a> <a href='#SkPaint_setLCDRenderText_lcdText'>is</a> <a href='#SkPaint_setLCDRenderText_lcdText'>true</a>.
Clears <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> <a href='#SkPaint_kLCDRenderText_Flag'>if</a> <a href='#SkPaint_setLCDRenderText_lcdText'>lcdText</a> <a href='#SkPaint_setLCDRenderText_lcdText'>is</a> <a href='#SkPaint_setLCDRenderText_lcdText'>false</a>.

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

<a name='Font_Embedded_Bitmaps'></a>

---

<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a> <a href='#Paint_Font_Embedded_Bitmaps'>allows</a> <a href='#Paint_Font_Embedded_Bitmaps'>selecting</a> <a href='#Paint_Font_Embedded_Bitmaps'>custom</a> <a href='#Paint_Font_Embedded_Bitmaps'>sized</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='undocumented#Glyph'>Glyphs</a>.
<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>when</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>set</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>chooses</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>an</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>embedded</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>over</a> <a href='undocumented#Glyph'>an</a> <a href='undocumented#Glyph'>outline</a> <a href='undocumented#Glyph'>contained</a>
<a href='undocumented#Glyph'>in</a> <a href='undocumented#Glyph'>a</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>if</a> <a href='undocumented#Font'>the</a> <a href='undocumented#Font'>platform</a> <a href='undocumented#Font'>supports</a> <a href='undocumented#Font'>this</a> <a href='undocumented#Font'>option</a>.

<a href='undocumented#Font'>FreeType</a> <a href='undocumented#Font'>selects</a> <a href='undocumented#Font'>the</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>if</a> <a href='undocumented#Glyph'>available</a> <a href='undocumented#Glyph'>when</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>is</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>set</a>, <a href='#SkPaint_kEmbeddedBitmapText_Flag'>and</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>selects</a>
<a href='#SkPaint_kEmbeddedBitmapText_Flag'>the</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>outline</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>if</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>is</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>clear</a>.
<a href='#SkPaint_kEmbeddedBitmapText_Flag'>Windows</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>may</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>select</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>the</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>but</a> <a href='undocumented#Glyph'>is</a> <a href='undocumented#Glyph'>not</a> <a href='undocumented#Glyph'>required</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>do</a> <a href='undocumented#Glyph'>so</a>.
<a href='#OS_X'>OS_X</a> <a href='#OS_X'>and</a> <a href='#OS_X'>iOS</a> <a href='#OS_X'>do</a> <a href='#OS_X'>not</a> <a href='#OS_X'>support</a> <a href='#OS_X'>this</a> <a href='#OS_X'>option</a>.

<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a> <a href='#Paint_Font_Embedded_Bitmaps'>is</a> <a href='#Paint_Font_Embedded_Bitmaps'>disabled</a> <a href='#Paint_Font_Embedded_Bitmaps'>by</a> <a href='#Paint_Font_Embedded_Bitmaps'>default</a>.
<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a> <a href='#Paint_Font_Embedded_Bitmaps'>can</a> <a href='#Paint_Font_Embedded_Bitmaps'>be</a> <a href='#Paint_Font_Embedded_Bitmaps'>enabled</a> <a href='#Paint_Font_Embedded_Bitmaps'>by</a> <a href='#Paint_Font_Embedded_Bitmaps'>default</a> <a href='#Paint_Font_Embedded_Bitmaps'>by</a> <a href='#Paint_Font_Embedded_Bitmaps'>setting</a> <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults_Flags</a> <a href='undocumented#SkPaintDefaults_Flags'>to</a>
<a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>at</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>compile</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>time</a>.

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
bool <a href='#SkPaint_isEmbeddedBitmapText'>isEmbeddedBitmapText</a>() <a href='#SkPaint_isEmbeddedBitmapText'>const</a>
</pre>

Returns true if <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>engine</a> <a href='undocumented#Font'>may</a> <a href='undocumented#Font'>return</a> <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>from</a> <a href='undocumented#Font'>font</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> <a href='SkBitmap_Reference#Bitmap'>instead</a> <a href='SkBitmap_Reference#Bitmap'>of</a> <a href='SkBitmap_Reference#Bitmap'>from</a> <a href='SkBitmap_Reference#Bitmap'>outlines</a>.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() <a href='#SkPaint_getFlags'>masked</a> <a href='#SkPaint_getFlags'>with</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a>.

### Return Value

<a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>state</a>

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
void <a href='#SkPaint_setEmbeddedBitmapText'>setEmbeddedBitmapText</a>(<a href='#SkPaint_setEmbeddedBitmapText'>bool</a> <a href='#SkPaint_setEmbeddedBitmapText'>useEmbeddedBitmapText</a>)
</pre>

Requests, but does not require, to use <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> <a href='SkBitmap_Reference#Bitmap'>in</a> <a href='SkBitmap_Reference#Bitmap'>fonts</a> <a href='SkBitmap_Reference#Bitmap'>instead</a> <a href='SkBitmap_Reference#Bitmap'>of</a> <a href='SkBitmap_Reference#Bitmap'>outlines</a>.

Sets <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>if</a> <a href='#SkPaint_setEmbeddedBitmapText_useEmbeddedBitmapText'>useEmbeddedBitmapText</a> <a href='#SkPaint_setEmbeddedBitmapText_useEmbeddedBitmapText'>is</a> <a href='#SkPaint_setEmbeddedBitmapText_useEmbeddedBitmapText'>true</a>.
Clears <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>if</a> <a href='#SkPaint_setEmbeddedBitmapText_useEmbeddedBitmapText'>useEmbeddedBitmapText</a> <a href='#SkPaint_setEmbeddedBitmapText_useEmbeddedBitmapText'>is</a> <a href='#SkPaint_setEmbeddedBitmapText_useEmbeddedBitmapText'>false</a>.

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

If <a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>is</a> <a href='#SkPaint_Hinting'>set</a> <a href='#SkPaint_Hinting'>to</a> <a href='#SkPaint_kNormal_Hinting'>kNormal_Hinting</a> <a href='#SkPaint_kNormal_Hinting'>or</a> <a href='#SkPaint_kFull_Hinting'>kFull_Hinting</a>, <a href='#Paint_Automatic_Hinting'>Automatic_Hinting</a>
<a href='#Paint_Automatic_Hinting'>instructs</a> <a href='#Paint_Automatic_Hinting'>the</a> <a href='#Font_Manager'>Font_Manager</a> <a href='#Font_Manager'>to</a> <a href='#Font_Manager'>always</a> <a href='#Font_Manager'>hint</a> <a href='undocumented#Glyph'>Glyphs</a>.
<a href='#Paint_Automatic_Hinting'>Automatic_Hinting</a> <a href='#Paint_Automatic_Hinting'>has</a> <a href='#Paint_Automatic_Hinting'>no</a> <a href='#Paint_Automatic_Hinting'>effect</a> <a href='#Paint_Automatic_Hinting'>if</a> <a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>is</a> <a href='#SkPaint_Hinting'>set</a> <a href='#SkPaint_Hinting'>to</a> <a href='#SkPaint_kNo_Hinting'>kNo_Hinting</a> <a href='#SkPaint_kNo_Hinting'>or</a>
<a href='#SkPaint_kSlight_Hinting'>kSlight_Hinting</a>.

<a href='#Paint_Automatic_Hinting'>Automatic_Hinting</a> <a href='#Paint_Automatic_Hinting'>only</a> <a href='#Paint_Automatic_Hinting'>affects</a> <a href='#Paint_Automatic_Hinting'>platforms</a> <a href='#Paint_Automatic_Hinting'>that</a> <a href='#Paint_Automatic_Hinting'>use</a> <a href='#Paint_Automatic_Hinting'>FreeType</a> <a href='#Paint_Automatic_Hinting'>as</a> <a href='#Paint_Automatic_Hinting'>the</a> <a href='#Font_Manager'>Font_Manager</a>.

<a name='SkPaint_isAutohinted'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isAutohinted'>isAutohinted</a>() <a href='#SkPaint_isAutohinted'>const</a>
</pre>

Returns true if <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>is</a> <a href='#SkPaint_Hinting'>set</a> <a href='#SkPaint_Hinting'>to</a> <a href='#SkPaint_kNormal_Hinting'>kNormal_Hinting</a> <a href='#SkPaint_kNormal_Hinting'>or</a> <a href='#SkPaint_kFull_Hinting'>kFull_Hinting</a>, <a href='#SkPaint_kFull_Hinting'>and</a> <a href='#SkPaint_kFull_Hinting'>if</a>
platform uses FreeType as the <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>manager</a>. <a href='undocumented#Font'>If</a> <a href='undocumented#Font'>true</a>, <a href='undocumented#Font'>instructs</a>
the <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>manager</a> <a href='undocumented#Font'>to</a> <a href='undocumented#Font'>always</a> <a href='undocumented#Font'>hint</a> <a href='undocumented#Glyph'>glyphs</a>.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() <a href='#SkPaint_getFlags'>masked</a> <a href='#SkPaint_getFlags'>with</a> <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a>.

### Return Value

<a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a> <a href='#SkPaint_kAutoHinting_Flag'>state</a>

### Example

<div><fiddle-embed name="aa4781afbe3b90e7ef56a287e5b9ce1e">

#### Example Output

~~~~
paint.isAutohinted() == !!(paint.getFlags() & SkPaint::kAutoHinting_Flag)
paint.isAutohinted() == !!(paint.getFlags() & SkPaint::kAutoHinting_Flag)
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_setAutohinted'>setAutohinted</a> <a href='#SkPaint_Hinting'>Hinting</a>

<a name='SkPaint_setAutohinted'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setAutohinted'>setAutohinted</a>(<a href='#SkPaint_setAutohinted'>bool</a> <a href='#SkPaint_setAutohinted'>useAutohinter</a>)
</pre>

Sets whether to always hint <a href='undocumented#Glyph'>glyphs</a>.
If <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>is</a> <a href='#SkPaint_Hinting'>set</a> <a href='#SkPaint_Hinting'>to</a> <a href='#SkPaint_kNormal_Hinting'>kNormal_Hinting</a> <a href='#SkPaint_kNormal_Hinting'>or</a> <a href='#SkPaint_kFull_Hinting'>kFull_Hinting</a> <a href='#SkPaint_kFull_Hinting'>and</a> <a href='#SkPaint_setAutohinted_useAutohinter'>useAutohinter</a> <a href='#SkPaint_setAutohinted_useAutohinter'>is</a> <a href='#SkPaint_setAutohinted_useAutohinter'>set</a>,
instructs the  <a href='undocumented#Font_Manager'>font manager</a> <a href='undocumented#Font'>to</a> <a href='undocumented#Font'>always</a> <a href='undocumented#Font'>hint</a> <a href='undocumented#Glyph'>glyphs</a>.
<a href='SkPaint_Reference#Automatic_Hinting'>auto-hinting</a> <a href='SkPaint_Reference#Automatic_Hinting'>has</a> <a href='SkPaint_Reference#Automatic_Hinting'>no</a> <a href='SkPaint_Reference#Automatic_Hinting'>effect</a> <a href='SkPaint_Reference#Automatic_Hinting'>if</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>is</a> <a href='#SkPaint_Hinting'>set</a> <a href='#SkPaint_Hinting'>to</a> <a href='#SkPaint_kNo_Hinting'>kNo_Hinting</a> <a href='#SkPaint_kNo_Hinting'>or</a>
<a href='#SkPaint_kSlight_Hinting'>kSlight_Hinting</a>.

Only affects platforms that use FreeType as the  <a href='undocumented#Font_Manager'>font manager</a>.

Sets <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a> <a href='#SkPaint_kAutoHinting_Flag'>if</a> <a href='#SkPaint_setAutohinted_useAutohinter'>useAutohinter</a> <a href='#SkPaint_setAutohinted_useAutohinter'>is</a> <a href='#SkPaint_setAutohinted_useAutohinter'>true</a>.
Clears <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a> <a href='#SkPaint_kAutoHinting_Flag'>if</a> <a href='#SkPaint_setAutohinted_useAutohinter'>useAutohinter</a> <a href='#SkPaint_setAutohinted_useAutohinter'>is</a> <a href='#SkPaint_setAutohinted_useAutohinter'>false</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setAutohinted_useAutohinter'><code><strong>useAutohinter</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4e185306d7de9390fe8445eed0139309"></fiddle-embed></div>

### See Also

<a href='#SkPaint_isAutohinted'>isAutohinted</a> <a href='#SkPaint_Hinting'>Hinting</a>

<a name='Fake_Bold'></a>

---

<a href='#Paint_Fake_Bold'>Fake_Bold</a> <a href='#Paint_Fake_Bold'>approximates</a> <a href='#Paint_Fake_Bold'>the</a> <a href='#Paint_Fake_Bold'>bold</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>style</a> <a href='undocumented#Font'>accompanying</a> <a href='undocumented#Font'>a</a> <a href='undocumented#Font'>normal</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>when</a> <a href='undocumented#Font'>a</a> <a href='undocumented#Font'>bold</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>face</a>
<a href='undocumented#Font'>is</a> <a href='undocumented#Font'>not</a> <a href='undocumented#Font'>available</a>. <a href='undocumented#Font'>Skia</a> <a href='undocumented#Font'>does</a> <a href='undocumented#Font'>not</a> <a href='undocumented#Font'>provide</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>substitution</a>; <a href='undocumented#Font'>it</a> <a href='undocumented#Font'>is</a> <a href='undocumented#Font'>up</a> <a href='undocumented#Font'>to</a> <a href='undocumented#Font'>the</a> <a href='undocumented#Font'>client</a> <a href='undocumented#Font'>to</a> <a href='undocumented#Font'>find</a> <a href='undocumented#Font'>the</a>
<a href='undocumented#Font'>bold</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>face</a> <a href='undocumented#Font'>using</a> <a href='undocumented#Font'>the</a> <a href='undocumented#Font'>platform</a> <a href='#Font_Manager'>Font_Manager</a>.

<a href='#Font_Manager'>Use</a> <a href='#Paint_Text_Skew_X'>Text_Skew_X</a> <a href='#Paint_Text_Skew_X'>to</a> <a href='#Paint_Text_Skew_X'>approximate</a> <a href='#Paint_Text_Skew_X'>an</a> <a href='#Paint_Text_Skew_X'>italic</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>style</a> <a href='undocumented#Font'>when</a> <a href='undocumented#Font'>the</a> <a href='undocumented#Font'>italic</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>face</a>
<a href='undocumented#Font'>is</a> <a href='undocumented#Font'>not</a> <a href='undocumented#Font'>available</a>.

<a href='undocumented#Font'>A</a> <a href='undocumented#Font'>FreeType</a> <a href='undocumented#Font'>based</a> <a href='undocumented#Font'>port</a> <a href='undocumented#Font'>may</a> <a href='undocumented#Font'>define</a> <a href='undocumented#Font'>SK_USE_FREETYPE_EMBOLDEN</a> <a href='undocumented#Font'>at</a> <a href='undocumented#Font'>compile</a> <a href='undocumented#Font'>time</a> <a href='undocumented#Font'>to</a> <a href='undocumented#Font'>direct</a>
<a href='undocumented#Font'>the</a>  <a href='undocumented#Font_Engine'>font engine</a> <a href='undocumented#Font'>to</a> <a href='undocumented#Font'>create</a> <a href='undocumented#Font'>the</a> <a href='undocumented#Font'>bold</a> <a href='undocumented#Glyph'>Glyphs</a>. <a href='undocumented#Glyph'>Otherwise</a>, <a href='undocumented#Glyph'>the</a> <a href='undocumented#Glyph'>extra</a> <a href='undocumented#Glyph'>bold</a> <a href='undocumented#Glyph'>is</a> <a href='undocumented#Glyph'>computed</a>
<a href='undocumented#Glyph'>by</a> <a href='undocumented#Glyph'>increasing</a> <a href='undocumented#Glyph'>the</a>  <a href='#Stroke_Width'>stroke width</a> <a href='undocumented#Glyph'>and</a> <a href='undocumented#Glyph'>setting</a> <a href='undocumented#Glyph'>the</a> <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>to</a> <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a> <a href='#SkPaint_kStrokeAndFill_Style'>as</a> <a href='#SkPaint_kStrokeAndFill_Style'>needed</a>.

<a href='#Paint_Fake_Bold'>Fake_Bold</a> <a href='#Paint_Fake_Bold'>is</a> <a href='#Paint_Fake_Bold'>disabled</a> <a href='#Paint_Fake_Bold'>by</a> <a href='#Paint_Fake_Bold'>default</a>.

### Example

<div><fiddle-embed name="e811f4829a2daaaeaad3795504a7e02a"></fiddle-embed></div>

<a name='SkPaint_isFakeBoldText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isFakeBoldText'>isFakeBoldText</a>() <a href='#SkPaint_isFakeBoldText'>const</a>
</pre>

Returns true if approximate bold by increasing the stroke width when creating <a href='undocumented#Glyph'>glyph</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a>
from outlines.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a>() <a href='#SkPaint_getFlags'>masked</a> <a href='#SkPaint_getFlags'>with</a> <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a>.

### Return Value

<a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a> <a href='#SkPaint_kFakeBoldText_Flag'>state</a>

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
void <a href='#SkPaint_setFakeBoldText'>setFakeBoldText</a>(<a href='#SkPaint_setFakeBoldText'>bool</a> <a href='#SkPaint_setFakeBoldText'>fakeBoldText</a>)
</pre>

Increases  <a href='#Stroke_Width'>stroke width</a> when creating <a href='undocumented#Glyph'>glyph</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>approximate</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>bold</a> <a href='undocumented#Typeface'>typeface</a>.

Sets <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a> <a href='#SkPaint_kFakeBoldText_Flag'>if</a> <a href='#SkPaint_setFakeBoldText_fakeBoldText'>fakeBoldText</a> <a href='#SkPaint_setFakeBoldText_fakeBoldText'>is</a> <a href='#SkPaint_setFakeBoldText_fakeBoldText'>true</a>.
Clears <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a> <a href='#SkPaint_kFakeBoldText_Flag'>if</a> <a href='#SkPaint_setFakeBoldText_fakeBoldText'>fakeBoldText</a> <a href='#SkPaint_setFakeBoldText_fakeBoldText'>is</a> <a href='#SkPaint_setFakeBoldText_fakeBoldText'>false</a>.

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

<a name='Full_Hinting_Spacing'></a>

if <a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_Hinting'>is</a> <a href='#SkPaint_Hinting'>set</a> <a href='#SkPaint_Hinting'>to</a> <a href='#SkPaint_kFull_Hinting'>kFull_Hinting</a>, <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a> <a href='#Paint_Full_Hinting_Spacing'>adjusts</a> <a href='#Paint_Full_Hinting_Spacing'>the</a> <a href='#Paint_Full_Hinting_Spacing'>character</a>
<a href='#Paint_Full_Hinting_Spacing'>spacing</a> <a href='#Paint_Full_Hinting_Spacing'>by</a> <a href='#Paint_Full_Hinting_Spacing'>the</a> <a href='#Paint_Full_Hinting_Spacing'>difference</a> <a href='#Paint_Full_Hinting_Spacing'>of</a> <a href='#Paint_Full_Hinting_Spacing'>the</a> <a href='#Paint_Full_Hinting_Spacing'>hinted</a> <a href='#Paint_Full_Hinting_Spacing'>and</a> <a href='#Paint_Full_Hinting_Spacing'>unhinted</a> <a href='#Left_Side_Bearing'>Left_Side_Bearing</a> <a href='#Left_Side_Bearing'>and</a>
<a href='#Right_Side_Bearing'>Right_Side_Bearing</a>. <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a> <a href='#Paint_Full_Hinting_Spacing'>only</a> <a href='#Paint_Full_Hinting_Spacing'>applies</a> <a href='#Paint_Full_Hinting_Spacing'>to</a> <a href='#Paint_Full_Hinting_Spacing'>platforms</a> <a href='#Paint_Full_Hinting_Spacing'>that</a> <a href='#Paint_Full_Hinting_Spacing'>use</a>
<a href='#Paint_Full_Hinting_Spacing'>FreeType</a> <a href='#Paint_Full_Hinting_Spacing'>as</a> <a href='#Paint_Full_Hinting_Spacing'>their</a> <a href='#Font_Engine'>Font_Engine</a>.

<a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a> <a href='#Paint_Full_Hinting_Spacing'>is</a> <a href='#Paint_Full_Hinting_Spacing'>not</a> <a href='#Paint_Full_Hinting_Spacing'>related</a> <a href='#Paint_Full_Hinting_Spacing'>to</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>kerning</a>, <a href='undocumented#Text'>where</a> <a href='undocumented#Text'>the</a> <a href='undocumented#Text'>space</a> <a href='undocumented#Text'>between</a>
<a href='undocumented#Text'>a</a> <a href='undocumented#Text'>specific</a> <a href='undocumented#Text'>pair</a> <a href='undocumented#Text'>of</a> <a href='undocumented#Text'>characters</a> <a href='undocumented#Text'>is</a> <a href='undocumented#Text'>adjusted</a> <a href='undocumented#Text'>using</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>in</a> <a href='undocumented#Data'>the</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>kerning</a> <a href='undocumented#Font'>tables</a>.

<a name='Filter_Quality_Methods'></a>

---

<a href='#Filter_Quality'>Filter_Quality</a> <a href='#Filter_Quality'>trades</a> <a href='#Filter_Quality'>speed</a> <a href='#Filter_Quality'>for</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>filtering</a> <a href='SkImage_Reference#Image'>when</a> <a href='SkImage_Reference#Image'>the</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>is</a> <a href='SkImage_Reference#Image'>scaled</a>.
<a href='SkImage_Reference#Image'>A</a> <a href='SkImage_Reference#Image'>lower</a> <a href='#Filter_Quality'>Filter_Quality</a> <a href='#Filter_Quality'>draws</a> <a href='#Filter_Quality'>faster</a>, <a href='#Filter_Quality'>but</a> <a href='#Filter_Quality'>has</a> <a href='#Filter_Quality'>less</a> <a href='#Filter_Quality'>fidelity</a>.
<a href='#Filter_Quality'>A</a> <a href='#Filter_Quality'>higher</a> <a href='#Filter_Quality'>Filter_Quality</a> <a href='#Filter_Quality'>draws</a> <a href='#Filter_Quality'>slower</a>, <a href='#Filter_Quality'>but</a> <a href='#Filter_Quality'>looks</a> <a href='#Filter_Quality'>better</a>.
<a href='#Filter_Quality'>If</a> <a href='#Filter_Quality'>the</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>is</a> <a href='SkImage_Reference#Image'>drawn</a> <a href='SkImage_Reference#Image'>without</a> <a href='SkImage_Reference#Image'>scaling</a>, <a href='SkImage_Reference#Image'>the</a> <a href='#Filter_Quality'>Filter_Quality</a> <a href='#Filter_Quality'>choice</a> <a href='#Filter_Quality'>will</a> <a href='#Filter_Quality'>not</a> <a href='#Filter_Quality'>result</a>
<a href='#Filter_Quality'>in</a> <a href='#Filter_Quality'>a</a> <a href='#Filter_Quality'>noticeable</a> <a href='#Filter_Quality'>difference</a>.

<a href='#Filter_Quality'>Filter_Quality</a> <a href='#Filter_Quality'>is</a> <a href='#Filter_Quality'>used</a> <a href='#Filter_Quality'>in</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>passed</a> <a href='SkPaint_Reference#Paint'>as</a> <a href='SkPaint_Reference#Paint'>a</a> <a href='SkPaint_Reference#Paint'>parameter</a> <a href='SkPaint_Reference#Paint'>to</a>

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

and when <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>has</a> <a href='SkPaint_Reference#Paint'>a</a> <a href='undocumented#Shader'>Shader</a> <a href='undocumented#Shader'>specialization</a> <a href='undocumented#Shader'>that</a> <a href='undocumented#Shader'>uses</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>or</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>.

<a href='#Filter_Quality'>Filter_Quality</a> <a href='#Filter_Quality'>is</a> <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a> <a href='undocumented#kNone_SkFilterQuality'>by</a> <a href='undocumented#kNone_SkFilterQuality'>default</a>.

### Example

<div><fiddle-embed name="69369cff2f5b145a6f616092513266a0"></fiddle-embed></div>

<a name='SkPaint_getFilterQuality'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='#SkPaint_getFilterQuality'>getFilterQuality</a>() <a href='#SkPaint_getFilterQuality'>const</a>
</pre>

Returns <a href='undocumented#SkFilterQuality'>SkFilterQuality</a>, <a href='undocumented#SkFilterQuality'>the</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>filtering</a> <a href='SkImage_Reference#Image'>level</a>. <a href='SkImage_Reference#Image'>A</a> <a href='SkImage_Reference#Image'>lower</a> <a href='SkImage_Reference#Image'>setting</a>
draws faster; a higher setting looks better when the <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>is</a> <a href='SkImage_Reference#Image'>scaled</a>.

### Return Value

one of: <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a>,

<a href='undocumented#kMedium_SkFilterQuality'>kMedium_SkFilterQuality</a>, <a href='undocumented#kHigh_SkFilterQuality'>kHigh_SkFilterQuality</a>

### Example

<div><fiddle-embed name="d4ca1f23809b6835c4ba46ea98a86900">

#### Example Output

~~~~
kNone_SkFilterQuality == paint.getFilterQuality()
~~~~

</fiddle-embed></div>

<a name='SkPaint_setFilterQuality'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setFilterQuality'>setFilterQuality</a>(<a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>quality</a>)
</pre>

Sets <a href='undocumented#SkFilterQuality'>SkFilterQuality</a>, <a href='undocumented#SkFilterQuality'>the</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>filtering</a> <a href='SkImage_Reference#Image'>level</a>. <a href='SkImage_Reference#Image'>A</a> <a href='SkImage_Reference#Image'>lower</a> <a href='SkImage_Reference#Image'>setting</a>
draws faster; a higher setting looks better when the <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>is</a> <a href='SkImage_Reference#Image'>scaled</a>.
Does not check to see if <a href='#SkPaint_setFilterQuality_quality'>quality</a> <a href='#SkPaint_setFilterQuality_quality'>is</a> <a href='#SkPaint_setFilterQuality_quality'>valid</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setFilterQuality_quality'><code><strong>quality</strong></code></a></td>
    <td>one of: <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a>,</td>
  </tr>
</table>

<a href='undocumented#kMedium_SkFilterQuality'>kMedium_SkFilterQuality</a>, <a href='undocumented#kHigh_SkFilterQuality'>kHigh_SkFilterQuality</a>

### Example

<div><fiddle-embed name="e4288fabf24ee60b645e8bb6ea0afadf">

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
| <a href='#SkPaint_getColor'>getColor</a> | returns <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>and</a> <a href='#Color_Alpha'>RGB</a>, <a href='#Color_Alpha'>one</a> <a href='#Color_Alpha'>drawing</a> <a href='SkColor_Reference#Color'>color</a> |
| <a href='#SkPaint_setColor'>setColor</a> | sets <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>and</a> <a href='#Color_Alpha'>RGB</a>, <a href='#Color_Alpha'>one</a> <a href='#Color_Alpha'>drawing</a> <a href='SkColor_Reference#Color'>color</a> |

<a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>specifies</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>red</a>, <a href='SkColor_Reference#Color'>blue</a>, <a href='SkColor_Reference#Color'>green</a>, <a href='SkColor_Reference#Color'>and</a> <a href='#Color_Alpha'>Color_Alpha</a>
<a href='#Color_Alpha'>values</a> <a href='#Color_Alpha'>used</a> <a href='#Color_Alpha'>to</a> <a href='#Color_Alpha'>draw</a> <a href='#Color_Alpha'>a</a> <a href='#Color_Alpha'>filled</a> <a href='#Color_Alpha'>or</a> <a href='#Color_Alpha'>stroked</a> <a href='#Color_Alpha'>shape</a> <a href='#Color_Alpha'>in</a> <a href='#Color_Alpha'>a</a> 32-<a href='#Color_Alpha'>bit</a> <a href='#Color_Alpha'>value</a>. <a href='#Color_Alpha'>Each</a> <a href='#Color_Alpha'>component</a>
<a href='#Color_Alpha'>occupies</a> 8-<a href='#Color_Alpha'>bits</a>, <a href='#Color_Alpha'>ranging</a> <a href='#Color_Alpha'>from</a> <a href='#Color_Alpha'>zero</a>: <a href='#Color_Alpha'>no</a> <a href='#Color_Alpha'>contribution</a>; <a href='#Color_Alpha'>to</a> 255: <a href='#Color_Alpha'>full</a> <a href='#Color_Alpha'>intensity</a>.
<a href='#Color_Alpha'>All</a> <a href='#Color_Alpha'>values</a> <a href='#Color_Alpha'>in</a> <a href='#Color_Alpha'>any</a> <a href='#Color_Alpha'>combination</a> <a href='#Color_Alpha'>are</a> <a href='#Color_Alpha'>valid</a>.

<a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>is</a> <a href='SkColor_Reference#Color'>not</a> <a href='undocumented#Premultiply'>Premultiplied</a>; <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>sets</a> <a href='#Color_Alpha'>the</a> <a href='#Color_Alpha'>transparency</a> <a href='#Color_Alpha'>independent</a> <a href='#Color_Alpha'>of</a>
<a href='#Color_Alpha'>RGB</a>: <a href='#Color_Alpha'>red</a>, <a href='#Color_Alpha'>blue</a>, <a href='#Color_Alpha'>and</a> <a href='#Color_Alpha'>green</a>.

<a href='#Color_Alpha'>The</a> <a href='#Color_Alpha'>bit</a> <a href='#Color_Alpha'>positions</a> <a href='#Color_Alpha'>of</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>and</a> <a href='#Color_Alpha'>RGB</a> <a href='#Color_Alpha'>are</a> <a href='#Color_Alpha'>independent</a> <a href='#Color_Alpha'>of</a> <a href='#Color_Alpha'>the</a> <a href='#Color_Alpha'>bit</a>
<a href='#Color_Alpha'>positions</a> <a href='#Color_Alpha'>on</a> <a href='#Color_Alpha'>the</a> <a href='#Color_Alpha'>output</a> <a href='undocumented#Device'>device</a>, <a href='undocumented#Device'>which</a> <a href='undocumented#Device'>may</a> <a href='undocumented#Device'>have</a> <a href='undocumented#Device'>more</a> <a href='undocumented#Device'>or</a> <a href='undocumented#Device'>fewer</a> <a href='undocumented#Device'>bits</a>, <a href='undocumented#Device'>and</a> <a href='undocumented#Device'>may</a> <a href='undocumented#Device'>have</a>
<a href='undocumented#Device'>a</a> <a href='undocumented#Device'>different</a> <a href='undocumented#Device'>arrangement</a>.

| bit positions | <a href='#Color_Alpha'>Color_Alpha</a> | red | blue | green |
| --- | --- | --- | --- | ---  |
|  | 31 - 24 | 23 - 16 | 15 - 8 | 7 - 0 |

### Example

<div><fiddle-embed name="214b559d75c65a7bef6ef4be1f860053"></fiddle-embed></div>

<a name='SkPaint_getColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPaint_getColor'>getColor</a>() <a href='#SkPaint_getColor'>const</a>
</pre>

Retrieves <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>RGB</a>, <a href='undocumented#Unpremultiply'>unpremultiplied</a>, <a href='undocumented#Unpremultiply'>packed</a> <a href='undocumented#Unpremultiply'>into</a> 32 <a href='undocumented#Unpremultiply'>bits</a>.
Use helpers <a href='SkColor_Reference#SkColorGetA'>SkColorGetA</a>(), <a href='SkColor_Reference#SkColorGetR'>SkColorGetR</a>(), <a href='SkColor_Reference#SkColorGetG'>SkColorGetG</a>(), <a href='SkColor_Reference#SkColorGetG'>and</a> <a href='SkColor_Reference#SkColorGetB'>SkColorGetB</a>() <a href='SkColor_Reference#SkColorGetB'>to</a> <a href='SkColor_Reference#SkColorGetB'>extract</a>
a <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>component</a>.

### Return Value

<a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='undocumented#Unpremultiply'>ARGB</a>

### Example

<div><fiddle-embed name="72d41f890203109a41f589a7403acae9">

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
<a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a> <a href='#SkPaint_getColor4f'>getColor4f</a>() <a href='#SkPaint_getColor4f'>const</a>
</pre>

Retrieves <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>RGB</a>, <a href='undocumented#Unpremultiply'>unpremultiplied</a>, <a href='undocumented#Unpremultiply'>as</a> <a href='undocumented#Unpremultiply'>four</a> <a href='undocumented#Unpremultiply'>floating</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>values</a>. <a href='SkPoint_Reference#Point'>RGB</a> <a href='SkPoint_Reference#Point'>are</a>
are extended sRGB values (sRGB gamut, and encoded with the sRGB transfer function).

### Return Value

<a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='undocumented#Unpremultiply'>RGBA</a>

### Example

<div><fiddle-embed name="8512ea2176f36e8f1aeef311ff228790">

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

Sets <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>RGB</a> <a href='SkColor_Reference#Alpha'>used</a> <a href='SkColor_Reference#Alpha'>when</a> <a href='SkColor_Reference#Alpha'>stroking</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>filling</a>. <a href='SkColor_Reference#Alpha'>The</a> <a href='#SkPaint_setColor_color'>color</a> <a href='#SkPaint_setColor_color'>is</a> <a href='#SkPaint_setColor_color'>a</a> 32-<a href='#SkPaint_setColor_color'>bit</a> <a href='#SkPaint_setColor_color'>value</a>,
<a href='undocumented#Unpremultiply'>unpremultiplied</a>, <a href='undocumented#Unpremultiply'>packing</a> 8-<a href='undocumented#Unpremultiply'>bit</a> <a href='undocumented#Unpremultiply'>components</a> <a href='undocumented#Unpremultiply'>for</a> <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>red</a>, <a href='SkColor_Reference#Alpha'>blue</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>green</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setColor_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='undocumented#Unpremultiply'>ARGB</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6e70f18300bd676a3c056ceb6b62f8df">

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
void <a href='#SkPaint_setColor4f'>setColor4f</a>(<a href='#SkPaint_setColor4f'>const</a> <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>& <a href='SkColor_Reference#Color'>color</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='undocumented#SkColorSpace'>colorSpace</a>)
</pre>

Sets <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>RGB</a> <a href='SkColor_Reference#Alpha'>used</a> <a href='SkColor_Reference#Alpha'>when</a> <a href='SkColor_Reference#Alpha'>stroking</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>filling</a>. <a href='SkColor_Reference#Alpha'>The</a> <a href='#SkPaint_setColor4f_color'>color</a> <a href='#SkPaint_setColor4f_color'>is</a> <a href='#SkPaint_setColor4f_color'>four</a> <a href='#SkPaint_setColor4f_color'>floating</a>
<a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>values</a>, <a href='undocumented#Unpremultiply'>unpremultiplied</a>. <a href='undocumented#Unpremultiply'>The</a> <a href='#SkPaint_setColor4f_color'>color</a> <a href='#SkPaint_setColor4f_color'>values</a> <a href='#SkPaint_setColor4f_color'>are</a> <a href='#SkPaint_setColor4f_color'>interpreted</a> <a href='#SkPaint_setColor4f_color'>as</a> <a href='#SkPaint_setColor4f_color'>being</a> <a href='#SkPaint_setColor4f_color'>in</a>
the <a href='#SkPaint_setColor4f_colorSpace'>colorSpace</a>. <a href='#SkPaint_setColor4f_colorSpace'>If</a> <a href='#SkPaint_setColor4f_colorSpace'>colorSpace</a> <a href='#SkPaint_setColor4f_colorSpace'>is</a> <a href='#SkPaint_setColor4f_colorSpace'>nullptr</a>, <a href='#SkPaint_setColor4f_colorSpace'>then</a> <a href='#SkPaint_setColor4f_color'>color</a> <a href='#SkPaint_setColor4f_color'>is</a> <a href='#SkPaint_setColor4f_color'>assumed</a> <a href='#SkPaint_setColor4f_color'>to</a> <a href='#SkPaint_setColor4f_color'>be</a> <a href='#SkPaint_setColor4f_color'>in</a> <a href='#SkPaint_setColor4f_color'>the</a>
sRGB  <a href='undocumented#Color_Space'>color space</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setColor4f_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='undocumented#Unpremultiply'>RGBA</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_setColor4f_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td><a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>describing</a> <a href='undocumented#SkColorSpace'>the</a> <a href='undocumented#SkColorSpace'>encoding</a> <a href='undocumented#SkColorSpace'>of</a> <a href='#SkPaint_setColor4f_color'>color</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="fa60859e3d03bdc117a05b32e093a8f1">

#### Example Output

~~~~
green1 == green2
~~~~

</fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPaint_setColor'>setColor</a> <a href='#SkPaint_setARGB'>setARGB</a> <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>

<a name='Alpha_Methods'></a>

<a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>sets</a> <a href='#Color_Alpha'>the</a> <a href='#Color_Alpha'>transparency</a> <a href='#Color_Alpha'>independent</a> <a href='#Color_Alpha'>of</a> <a href='#Color_Alpha'>RGB</a>: <a href='#Color_Alpha'>red</a>, <a href='#Color_Alpha'>blue</a>, <a href='#Color_Alpha'>and</a> <a href='#Color_Alpha'>green</a>.

<a name='SkPaint_getAlpha'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint8_t <a href='#SkPaint_getAlpha'>getAlpha</a>() <a href='#SkPaint_getAlpha'>const</a>
</pre>

Retrieves <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>from</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>stroking</a> <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>filling</a>.

### Return Value

<a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>ranging</a> <a href='SkColor_Reference#Alpha'>from</a> <a href='SkColor_Reference#Alpha'>zero</a>, <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>transparent</a>, <a href='SkColor_Reference#Alpha'>to</a> 255, <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a>

### Example

<div><fiddle-embed name="9a85bb62fe3d877b18fb7f952c4fa7f7">

#### Example Output

~~~~
255 == paint.getAlpha()
~~~~

</fiddle-embed></div>

<a name='SkPaint_setAlpha'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setAlpha'>setAlpha</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>a</a>)
</pre>

Replaces <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>leaving</a> <a href='SkColor_Reference#Alpha'>RGB</a>
unchanged. An out of range value triggers an assert in the debug
build. <a href='#SkPaint_setAlpha_a'>a</a> <a href='#SkPaint_setAlpha_a'>is</a> <a href='#SkPaint_setAlpha_a'>a</a> <a href='#SkPaint_setAlpha_a'>value</a> <a href='#SkPaint_setAlpha_a'>from</a> <a href='#SkPaint_setAlpha_a'>zero</a> <a href='#SkPaint_setAlpha_a'>to</a> 255.
<a href='#SkPaint_setAlpha_a'>a</a> <a href='#SkPaint_setAlpha_a'>set</a> <a href='#SkPaint_setAlpha_a'>to</a> <a href='#SkPaint_setAlpha_a'>zero</a> <a href='#SkPaint_setAlpha_a'>makes</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>fully</a> <a href='SkColor_Reference#Color'>transparent</a>; <a href='#SkPaint_setAlpha_a'>a</a> <a href='#SkPaint_setAlpha_a'>set</a> <a href='#SkPaint_setAlpha_a'>to</a> 255 <a href='#SkPaint_setAlpha_a'>makes</a> <a href='SkColor_Reference#Color'>color</a>
fully opaque.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setAlpha_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkColor_Reference#Color'>color</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6ddc0360512dfb9947e75c17e6a8103d">

#### Example Output

~~~~
0x44112233 == paint.getColor()
~~~~

</fiddle-embed></div>

<a name='SkPaint_setARGB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setARGB'>setARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>a</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>r</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>g</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>b</a>)
</pre>

Sets <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>drawing</a> <a href='SkColor_Reference#Color'>solid</a> <a href='SkColor_Reference#Color'>fills</a>. <a href='SkColor_Reference#Color'>The</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>components</a> <a href='SkColor_Reference#Color'>range</a> <a href='SkColor_Reference#Color'>from</a> 0 <a href='SkColor_Reference#Color'>to</a> 255.
The <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>is</a> <a href='undocumented#Unpremultiply'>unpremultiplied</a>; <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>sets</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>transparency</a> <a href='SkColor_Reference#Alpha'>independent</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkColor_Reference#Alpha'>RGB</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setARGB_a'><code><strong>a</strong></code></a></td>
    <td>amount of <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>from</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>transparent</a> (0) <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a> (255)</td>
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

<div><fiddle-embed name="cb62e4755789ed32f7120dc55984959d">

#### Example Output

~~~~
transRed1 == transRed2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_setColor'>setColor</a> <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>

<a name='Style'></a>

---

<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>specifies</a> <a href='#SkPaint_Style'>if</a> <a href='#SkPaint_Style'>the</a> <a href='#SkPaint_Style'>geometry</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_Style'>filled</a>, <a href='#SkPaint_Style'>stroked</a>, <a href='#SkPaint_Style'>or</a> <a href='#SkPaint_Style'>both</a> <a href='#SkPaint_Style'>filled</a> <a href='#SkPaint_Style'>and</a> <a href='#SkPaint_Style'>stroked</a>.
<a href='#SkPaint_Style'>Some</a> <a href='#SkPaint_Style'>shapes</a> <a href='#SkPaint_Style'>ignore</a> <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>and</a> <a href='#SkPaint_Style'>are</a> <a href='#SkPaint_Style'>always</a> <a href='#SkPaint_Style'>drawn</a> <a href='#SkPaint_Style'>filled</a> <a href='#SkPaint_Style'>or</a> <a href='#SkPaint_Style'>stroked</a>.

<a name='Style_Fill'></a>

Set <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>to</a> <a href='#SkPaint_kFill_Style'>kFill_Style</a> <a href='#SkPaint_kFill_Style'>to</a> <a href='#SkPaint_kFill_Style'>fill</a> <a href='#SkPaint_kFill_Style'>the</a> <a href='#SkPaint_kFill_Style'>shape</a>.
<a href='#SkPaint_kFill_Style'>The</a> <a href='#SkPaint_kFill_Style'>fill</a> <a href='#SkPaint_kFill_Style'>covers</a> <a href='#SkPaint_kFill_Style'>the</a> <a href='#SkPaint_kFill_Style'>area</a> <a href='#SkPaint_kFill_Style'>inside</a> <a href='#SkPaint_kFill_Style'>the</a> <a href='#SkPaint_kFill_Style'>geometry</a> <a href='#SkPaint_kFill_Style'>for</a> <a href='#SkPaint_kFill_Style'>most</a> <a href='#SkPaint_kFill_Style'>shapes</a>.

<a name='Style_Stroke'></a>

Set <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>to</a> <a href='#SkPaint_kStroke_Style'>kStroke_Style</a> <a href='#SkPaint_kStroke_Style'>to</a> <a href='#SkPaint_kStroke_Style'>stroke</a> <a href='#SkPaint_kStroke_Style'>the</a> <a href='#SkPaint_kStroke_Style'>shape</a>.

<a href='#SkPaint_kStroke_Style'>The</a> <a href='#SkPaint_kStroke_Style'>stroke</a> <a href='#SkPaint_kStroke_Style'>covers</a> <a href='#SkPaint_kStroke_Style'>the</a> <a href='#SkPaint_kStroke_Style'>area</a> <a href='#SkPaint_kStroke_Style'>described</a> <a href='#SkPaint_kStroke_Style'>by</a> <a href='#SkPaint_kStroke_Style'>following</a> <a href='#SkPaint_kStroke_Style'>the</a> <a href='#SkPaint_kStroke_Style'>shape</a> <a href='#SkPaint_kStroke_Style'>edge</a> <a href='#SkPaint_kStroke_Style'>with</a> <a href='#SkPaint_kStroke_Style'>a</a> <a href='#SkPaint_kStroke_Style'>pen</a> <a href='#SkPaint_kStroke_Style'>or</a> <a href='#SkPaint_kStroke_Style'>brush</a> <a href='#SkPaint_kStroke_Style'>of</a>
<a href='#Paint_Stroke_Width'>Stroke_Width</a>. <a href='#Paint_Stroke_Width'>The</a> <a href='#Paint_Stroke_Width'>area</a> <a href='#Paint_Stroke_Width'>covered</a> <a href='#Paint_Stroke_Width'>where</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#Paint_Stroke_Width'>shape</a> <a href='#Paint_Stroke_Width'>starts</a> <a href='#Paint_Stroke_Width'>and</a> <a href='#Paint_Stroke_Width'>stops</a> <a href='#Paint_Stroke_Width'>is</a> <a href='#Paint_Stroke_Width'>described</a> <a href='#Paint_Stroke_Width'>by</a> <a href='#Paint_Stroke_Cap'>Stroke_Cap</a>.
<a href='#Paint_Stroke_Cap'>The</a> <a href='#Paint_Stroke_Cap'>area</a> <a href='#Paint_Stroke_Cap'>covered</a> <a href='#Paint_Stroke_Cap'>where</a> <a href='#Paint_Stroke_Cap'>the</a> <a href='#Paint_Stroke_Cap'>shape</a> <a href='#Paint_Stroke_Cap'>turns</a> <a href='#Paint_Stroke_Cap'>a</a> <a href='#Paint_Stroke_Cap'>corner</a> <a href='#Paint_Stroke_Cap'>is</a> <a href='#Paint_Stroke_Cap'>described</a> <a href='#Paint_Stroke_Cap'>by</a> <a href='#Paint_Stroke_Join'>Stroke_Join</a>.
<a href='#Paint_Stroke_Join'>The</a> <a href='#Paint_Stroke_Join'>stroke</a> <a href='#Paint_Stroke_Join'>is</a> <a href='#Paint_Stroke_Join'>centered</a> <a href='#Paint_Stroke_Join'>on</a> <a href='#Paint_Stroke_Join'>the</a> <a href='#Paint_Stroke_Join'>shape</a>; <a href='#Paint_Stroke_Join'>it</a> <a href='#Paint_Stroke_Join'>extends</a> <a href='#Paint_Stroke_Join'>equally</a> <a href='#Paint_Stroke_Join'>on</a> <a href='#Paint_Stroke_Join'>either</a> <a href='#Paint_Stroke_Join'>side</a> <a href='#Paint_Stroke_Join'>of</a> <a href='#Paint_Stroke_Join'>the</a> <a href='#Paint_Stroke_Join'>shape</a> <a href='#Paint_Stroke_Join'>edge</a>.As <a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Paint_Stroke_Width'>gets</a> <a href='#Paint_Stroke_Width'>smaller</a>, <a href='#Paint_Stroke_Width'>the</a> <a href='#Paint_Stroke_Width'>drawn</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>frame</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>thinner</a>. <a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Paint_Stroke_Width'>less</a> <a href='#Paint_Stroke_Width'>than</a> <a href='#Paint_Stroke_Width'>one</a>
<a href='#Paint_Stroke_Width'>may</a> <a href='#Paint_Stroke_Width'>have</a> <a href='#Paint_Stroke_Width'>gaps</a>, <a href='#Paint_Stroke_Width'>and</a> <a href='#Paint_Stroke_Width'>if</a> <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> <a href='#SkPaint_kAntiAlias_Flag'>is</a> <a href='#SkPaint_kAntiAlias_Flag'>set</a>, <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>will</a> <a href='#Color_Alpha'>increase</a> <a href='#Color_Alpha'>to</a> <a href='#Color_Alpha'>visually</a> <a href='#Color_Alpha'>decrease</a> <a href='#Color_Alpha'>coverage</a>.

### See Also

<a href='#Path_Fill_Type'>Path_Fill_Type</a> <a href='#Path_Effect'>Path_Effect</a> <a href='#Paint_Style_Fill'>Style_Fill</a> <a href='#Paint_Style_Stroke'>Style_Stroke</a>

<a name='Hairline'></a>

---

<a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Paint_Stroke_Width'>of</a> <a href='#Paint_Stroke_Width'>zero</a> <a href='#Paint_Stroke_Width'>has</a> <a href='#Paint_Stroke_Width'>a</a> <a href='#Paint_Stroke_Width'>special</a> <a href='#Paint_Stroke_Width'>meaning</a> <a href='#Paint_Stroke_Width'>and</a> <a href='#Paint_Stroke_Width'>switches</a> <a href='#Paint_Stroke_Width'>drawing</a> <a href='#Paint_Stroke_Width'>to</a> <a href='#Paint_Stroke_Width'>use</a> <a href='#Paint_Stroke_Width'>Hairline</a>.
<a href='#Paint_Stroke_Width'>Hairline</a> <a href='#Paint_Stroke_Width'>draws</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#Paint_Stroke_Width'>thinnest</a> <a href='#Paint_Stroke_Width'>continuous</a> <a href='#Paint_Stroke_Width'>frame</a>. <a href='#Paint_Stroke_Width'>If</a> <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> <a href='#SkPaint_kAntiAlias_Flag'>is</a> <a href='#SkPaint_kAntiAlias_Flag'>clear</a>, <a href='#SkPaint_kAntiAlias_Flag'>adjacent</a> <a href='#SkPaint_kAntiAlias_Flag'>pixels</a>
<a href='#SkPaint_kAntiAlias_Flag'>flow</a> <a href='#SkPaint_kAntiAlias_Flag'>horizontally</a>, <a href='#SkPaint_kAntiAlias_Flag'>vertically</a>,<a href='#SkPaint_kAntiAlias_Flag'>or</a> <a href='#SkPaint_kAntiAlias_Flag'>diagonally</a>.

<a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>drawing</a> <a href='SkPath_Reference#Path'>with</a> <a href='SkPath_Reference#Path'>Hairline</a> <a href='SkPath_Reference#Path'>may</a> <a href='SkPath_Reference#Path'>hit</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>same</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>more</a> <a href='undocumented#Pixel'>than</a> <a href='undocumented#Pixel'>once</a>. <a href='undocumented#Pixel'>For</a> <a href='undocumented#Pixel'>instance</a>, <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>containing</a>
<a href='SkPath_Reference#Path'>two</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>in</a> <a href='undocumented#Line'>one</a> <a href='#Path_Overview_Contour'>Path_Contour</a> <a href='#Path_Overview_Contour'>will</a> <a href='#Path_Overview_Contour'>draw</a> <a href='#Path_Overview_Contour'>the</a> <a href='#Path_Overview_Contour'>corner</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>once</a>, <a href='SkPoint_Reference#Point'>but</a> <a href='SkPoint_Reference#Point'>may</a> <a href='SkPoint_Reference#Point'>both</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>may</a> <a href='undocumented#Line'>draw</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>adjacent</a>
<a href='undocumented#Pixel'>pixel</a>. <a href='undocumented#Pixel'>If</a> <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> <a href='#SkPaint_kAntiAlias_Flag'>is</a> <a href='#SkPaint_kAntiAlias_Flag'>set</a>, <a href='#SkPaint_kAntiAlias_Flag'>transparency</a> <a href='#SkPaint_kAntiAlias_Flag'>is</a> <a href='#SkPaint_kAntiAlias_Flag'>applied</a> <a href='#SkPaint_kAntiAlias_Flag'>twice</a>, <a href='#SkPaint_kAntiAlias_Flag'>resulting</a> <a href='#SkPaint_kAntiAlias_Flag'>in</a> <a href='#SkPaint_kAntiAlias_Flag'>a</a> <a href='#SkPaint_kAntiAlias_Flag'>darker</a> <a href='undocumented#Pixel'>pixel</a>. <a href='undocumented#Pixel'>Some</a>
<a href='undocumented#Pixel'>GPU-backed</a> <a href='undocumented#Pixel'>implementations</a> <a href='undocumented#Pixel'>apply</a> <a href='undocumented#Pixel'>transparency</a> <a href='undocumented#Pixel'>at</a> <a href='undocumented#Pixel'>a</a> <a href='undocumented#Pixel'>later</a> <a href='undocumented#Pixel'>drawing</a> <a href='undocumented#Pixel'>stage</a>, <a href='undocumented#Pixel'>avoiding</a> <a href='undocumented#Pixel'>double</a> <a href='undocumented#Pixel'>hit</a> <a href='undocumented#Pixel'>pixels</a>
<a href='undocumented#Pixel'>while</a> <a href='undocumented#Pixel'>stroking</a>.

### See Also

<a href='#Path_Fill_Type'>Path_Fill_Type</a> <a href='#Path_Effect'>Path_Effect</a> <a href='#Paint_Style_Fill'>Style_Fill</a> <a href='#Paint_Style_Stroke'>Style_Stroke</a>

<a name='SkPaint_Style'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Style'>Style</a> : <a href='#SkPaint_Style'>uint8_t</a> {
        <a href='#SkPaint_kFill_Style'>kFill_Style</a>,
        <a href='#SkPaint_kStroke_Style'>kStroke_Style</a>,
        <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>,
    };
</pre>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    static constexpr int <a href='#SkPaint_kStyleCount'>kStyleCount</a> = <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a> + 1;
</pre>

Set <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>to</a> <a href='#SkPaint_Style'>fill</a>, <a href='#SkPaint_Style'>stroke</a>, <a href='#SkPaint_Style'>or</a> <a href='#SkPaint_Style'>both</a> <a href='#SkPaint_Style'>fill</a> <a href='#SkPaint_Style'>and</a> <a href='#SkPaint_Style'>stroke</a> <a href='#SkPaint_Style'>geometry</a>.
<a href='#SkPaint_Style'>The</a> <a href='#SkPaint_Style'>stroke</a> <a href='#SkPaint_Style'>and</a> <a href='#SkPaint_Style'>fill</a>
<a href='#SkPaint_Style'>share</a> <a href='#SkPaint_Style'>all</a> <a href='SkPaint_Reference#Paint'>paint</a> <a href='SkPaint_Reference#Paint'>attributes</a>; <a href='SkPaint_Reference#Paint'>for</a> <a href='SkPaint_Reference#Paint'>instance</a>, <a href='SkPaint_Reference#Paint'>they</a> <a href='SkPaint_Reference#Paint'>are</a> <a href='SkPaint_Reference#Paint'>drawn</a> <a href='SkPaint_Reference#Paint'>with</a> <a href='SkPaint_Reference#Paint'>the</a> <a href='SkPaint_Reference#Paint'>same</a> <a href='SkColor_Reference#Color'>color</a>.

<a href='SkColor_Reference#Color'>Use</a> <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a> <a href='#SkPaint_kStrokeAndFill_Style'>to</a> <a href='#SkPaint_kStrokeAndFill_Style'>avoid</a> <a href='#SkPaint_kStrokeAndFill_Style'>hitting</a> <a href='#SkPaint_kStrokeAndFill_Style'>the</a> <a href='#SkPaint_kStrokeAndFill_Style'>same</a> <a href='#SkPaint_kStrokeAndFill_Style'>pixels</a> <a href='#SkPaint_kStrokeAndFill_Style'>twice</a> <a href='#SkPaint_kStrokeAndFill_Style'>with</a> <a href='#SkPaint_kStrokeAndFill_Style'>a</a> <a href='#SkPaint_kStrokeAndFill_Style'>stroke</a> <a href='#SkPaint_kStrokeAndFill_Style'>draw</a> <a href='#SkPaint_kStrokeAndFill_Style'>and</a>
<a href='#SkPaint_kStrokeAndFill_Style'>a</a> <a href='#SkPaint_kStrokeAndFill_Style'>fill</a> <a href='#SkPaint_kStrokeAndFill_Style'>draw</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kFill_Style'><code>SkPaint::kFill_Style</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Applies to <a href='SkRect_Reference#Rect'>Rect</a>, <a href='SkRegion_Reference#Region'>Region</a>, <a href='#RRect'>Round_Rect</a>, <a href='undocumented#Circle'>Circles</a>, <a href='undocumented#Oval'>Ovals</a>, <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>and</a> <a href='undocumented#Text'>Text</a>.
<a href='SkBitmap_Reference#Bitmap'>Bitmap</a>, <a href='SkImage_Reference#Image'>Image</a>, <a href='undocumented#Patch'>Patches</a>, <a href='SkRegion_Reference#Region'>Region</a>, <a href='undocumented#Sprite'>Sprites</a>, <a href='undocumented#Sprite'>and</a> <a href='undocumented#Vertices'>Vertices</a> <a href='undocumented#Vertices'>are</a> <a href='undocumented#Vertices'>painted</a> <a href='undocumented#Vertices'>as</a> <a href='undocumented#Vertices'>if</a>
<a href='#SkPaint_kFill_Style'>kFill_Style</a> <a href='#SkPaint_kFill_Style'>is</a> <a href='#SkPaint_kFill_Style'>set</a>, <a href='#SkPaint_kFill_Style'>and</a> <a href='#SkPaint_kFill_Style'>ignore</a> <a href='#SkPaint_kFill_Style'>the</a> <a href='#SkPaint_kFill_Style'>set</a> <a href='#SkPaint_Style'>Style</a>.
<a href='#SkPaint_Style'>The</a> <a href='#Path_Fill_Type'>Path_Fill_Type</a> <a href='#Path_Fill_Type'>specifies</a> <a href='#Path_Fill_Type'>additional</a> <a href='#Path_Fill_Type'>rules</a> <a href='#Path_Fill_Type'>to</a> <a href='#Path_Fill_Type'>fill</a> <a href='#Path_Fill_Type'>the</a> <a href='#Path_Fill_Type'>area</a> <a href='#Path_Fill_Type'>outside</a> <a href='#Path_Fill_Type'>the</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>edge</a>,
<a href='SkPath_Reference#Path'>and</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>create</a> <a href='SkPath_Reference#Path'>an</a> <a href='SkPath_Reference#Path'>unfilled</a> <a href='SkPath_Reference#Path'>hole</a> <a href='SkPath_Reference#Path'>inside</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>shape</a>.
<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_Style'>set</a> <a href='#SkPaint_Style'>to</a> <a href='#SkPaint_kFill_Style'>kFill_Style</a> <a href='#SkPaint_kFill_Style'>by</a> <a href='#SkPaint_kFill_Style'>default</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kStroke_Style'><code>SkPaint::kStroke_Style</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Applies to <a href='SkRect_Reference#Rect'>Rect</a>, <a href='SkRegion_Reference#Region'>Region</a>, <a href='#RRect'>Round_Rect</a>, <a href='undocumented#Arc'>Arcs</a>, <a href='undocumented#Circle'>Circles</a>, <a href='undocumented#Oval'>Ovals</a>, <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>and</a> <a href='undocumented#Text'>Text</a>.
<a href='undocumented#Arc'>Arcs</a>, <a href='undocumented#Line'>Lines</a>, <a href='undocumented#Line'>and</a> <a href='SkPoint_Reference#Point'>points</a>, <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>always</a> <a href='SkPoint_Reference#Point'>drawn</a> <a href='SkPoint_Reference#Point'>as</a> <a href='SkPoint_Reference#Point'>if</a> <a href='#SkPaint_kStroke_Style'>kStroke_Style</a> <a href='#SkPaint_kStroke_Style'>is</a> <a href='#SkPaint_kStroke_Style'>set</a>,
<a href='#SkPaint_kStroke_Style'>and</a> <a href='#SkPaint_kStroke_Style'>ignore</a> <a href='#SkPaint_kStroke_Style'>the</a> <a href='#SkPaint_kStroke_Style'>set</a> <a href='#SkPaint_Style'>Style</a>.
<a href='#SkPaint_Style'>The</a> <a href='#SkPaint_Style'>stroke</a> <a href='#SkPaint_Style'>construction</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_Style'>unaffected</a> <a href='#SkPaint_Style'>by</a> <a href='#SkPaint_Style'>the</a> <a href='#Path_Fill_Type'>Path_Fill_Type</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kStrokeAndFill_Style'><code>SkPaint::kStrokeAndFill_Style</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Applies to <a href='SkRect_Reference#Rect'>Rect</a>, <a href='SkRegion_Reference#Region'>Region</a>, <a href='#RRect'>Round_Rect</a>, <a href='undocumented#Circle'>Circles</a>, <a href='undocumented#Oval'>Ovals</a>, <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>and</a> <a href='undocumented#Text'>Text</a>.
<a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>treated</a> <a href='SkPath_Reference#Path'>as</a> <a href='SkPath_Reference#Path'>if</a> <a href='SkPath_Reference#Path'>it</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>set</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>,
<a href='#SkPath_kWinding_FillType'>and</a> <a href='#SkPath_kWinding_FillType'>the</a> <a href='#SkPath_kWinding_FillType'>set</a> <a href='#Path_Fill_Type'>Path_Fill_Type</a> <a href='#Path_Fill_Type'>is</a> <a href='#Path_Fill_Type'>ignored</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kStyleCount'><code>SkPaint::kStyleCount</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_Style'>a</a> <a href='#SkPaint_Style'>legal</a> <a href='#SkPaint_Style'>value</a>.
</td>
  </tr>
</table>

<a name='SkPaint_getStyle'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_getStyle'>getStyle</a>() <a href='#SkPaint_getStyle'>const</a>
</pre>

Returns whether the geometry is filled, stroked, or filled and stroked.

### Return Value

one of:<a href='#SkPaint_kFill_Style'>kFill_Style</a>, <a href='#SkPaint_kStroke_Style'>kStroke_Style</a>, <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>

### Example

<div><fiddle-embed name="1c5e18c3c0102d2dac86a78ba8c8ce01">

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
void <a href='#SkPaint_setStyle'>setStyle</a>(<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>style</a>)
</pre>

Sets whether the geometry is filled, stroked, or filled and stroked.
Has no effect if <a href='#SkPaint_setStyle_style'>style</a> <a href='#SkPaint_setStyle_style'>is</a> <a href='#SkPaint_setStyle_style'>not</a> <a href='#SkPaint_setStyle_style'>a</a> <a href='#SkPaint_setStyle_style'>legal</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>value</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStyle_style'><code><strong>style</strong></code></a></td>
    <td>one of: <a href='#SkPaint_kFill_Style'>kFill_Style</a>, <a href='#SkPaint_kStroke_Style'>kStroke_Style</a>, <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c7bb6248e4735b8d1a32d02fba40d344"></fiddle-embed></div>

### See Also

<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_getStyle'>getStyle</a>

<a name='Stroke_Width'></a>

---

<a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Paint_Stroke_Width'>sets</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#Paint_Stroke_Width'>width</a> <a href='#Paint_Stroke_Width'>for</a> <a href='#Paint_Stroke_Width'>stroking</a>. <a href='#Paint_Stroke_Width'>The</a> <a href='#Paint_Stroke_Width'>width</a> <a href='#Paint_Stroke_Width'>is</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#Paint_Stroke_Width'>thickness</a>
<a href='#Paint_Stroke_Width'>of</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#Paint_Stroke_Width'>stroke</a> <a href='#Paint_Stroke_Width'>perpendicular</a> <a href='#Paint_Stroke_Width'>to</a> <a href='#Paint_Stroke_Width'>the</a>  <a href='SkPath_Reference#Path_Direction'>path direction</a> <a href='SkPath_Reference#Path'>when</a> <a href='SkPath_Reference#Path'>the</a>  <a href='SkPaint_Reference#Paint'>paint style</a> <a href='SkPaint_Reference#Paint'>is</a>
<a href='SkPaint_Reference#Paint'>set</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='#SkPaint_kStroke_Style'>kStroke_Style</a> <a href='#SkPaint_kStroke_Style'>or</a> <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>.

<a href='#SkPaint_kStrokeAndFill_Style'>When</a> <a href='#SkPaint_kStrokeAndFill_Style'>width</a> <a href='#SkPaint_kStrokeAndFill_Style'>is</a> <a href='#SkPaint_kStrokeAndFill_Style'>greater</a> <a href='#SkPaint_kStrokeAndFill_Style'>than</a> <a href='#SkPaint_kStrokeAndFill_Style'>zero</a>, <a href='#SkPaint_kStrokeAndFill_Style'>the</a> <a href='#SkPaint_kStrokeAndFill_Style'>stroke</a> <a href='#SkPaint_kStrokeAndFill_Style'>encompasses</a> <a href='#SkPaint_kStrokeAndFill_Style'>as</a> <a href='#SkPaint_kStrokeAndFill_Style'>many</a> <a href='#SkPaint_kStrokeAndFill_Style'>pixels</a> <a href='#SkPaint_kStrokeAndFill_Style'>partially</a>
<a href='#SkPaint_kStrokeAndFill_Style'>or</a> <a href='#SkPaint_kStrokeAndFill_Style'>fully</a> <a href='#SkPaint_kStrokeAndFill_Style'>as</a> <a href='#SkPaint_kStrokeAndFill_Style'>needed</a>. <a href='#SkPaint_kStrokeAndFill_Style'>When</a> <a href='#SkPaint_kStrokeAndFill_Style'>the</a> <a href='#SkPaint_kStrokeAndFill_Style'>width</a> <a href='#SkPaint_kStrokeAndFill_Style'>equals</a> <a href='#SkPaint_kStrokeAndFill_Style'>zero</a>, <a href='#SkPaint_kStrokeAndFill_Style'>the</a> <a href='SkPaint_Reference#Paint'>paint</a> <a href='SkPaint_Reference#Paint'>enables</a> <a href='SkPaint_Reference#Paint'>hairlines</a>;
<a href='SkPaint_Reference#Paint'>the</a> <a href='SkPaint_Reference#Paint'>stroke</a> <a href='SkPaint_Reference#Paint'>is</a> <a href='SkPaint_Reference#Paint'>always</a> <a href='SkPaint_Reference#Paint'>one</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>wide</a>.

<a href='undocumented#Pixel'>The</a> <a href='undocumented#Pixel'>stroke</a> <a href='undocumented#Pixel'>dimensions</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>scaled</a> <a href='undocumented#Pixel'>by</a> <a href='undocumented#Pixel'>the</a>  <a href='SkCanvas_Reference#Canvas_Matrix'>canvas matrix</a>, <a href='SkCanvas_Reference#Canvas'>but</a> <a href='SkCanvas_Reference#Canvas'>Hairline</a> <a href='SkCanvas_Reference#Canvas'>stroke</a>
<a href='SkCanvas_Reference#Canvas'>remains</a> <a href='SkCanvas_Reference#Canvas'>one</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>wide</a> <a href='undocumented#Pixel'>regardless</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>scaling</a>.

<a href='undocumented#Pixel'>The</a> <a href='undocumented#Pixel'>default</a> <a href='undocumented#Pixel'>width</a> <a href='undocumented#Pixel'>for</a> <a href='undocumented#Pixel'>the</a> <a href='SkPaint_Reference#Paint'>paint</a> <a href='SkPaint_Reference#Paint'>is</a> <a href='SkPaint_Reference#Paint'>zero</a>.

### Example

<div><fiddle-embed name="5112c7209a19e035c61cef33a624a652" gpu="true"><div>The pixels hit to represent thin <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>vary</a> <a href='undocumented#Line'>with</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>angle</a> <a href='undocumented#Line'>of</a> <a href='undocumented#Line'>the</a>
<a href='undocumented#Line'>line</a> <a href='undocumented#Line'>and</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>platform</a> <a href='undocumented#Line'>implementation</a>.
</div></fiddle-embed></div>

<a name='SkPaint_getStrokeWidth'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getStrokeWidth'>getStrokeWidth</a>() <a href='#SkPaint_getStrokeWidth'>const</a>
</pre>

Returns the thickness of the pen used by <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>to</a>
outline the shape.

### Return Value

zero for hairline, greater than zero for pen thickness

### Example

<div><fiddle-embed name="99aa73f64df8bbf06e656cd891a81b9e">

#### Example Output

~~~~
0 == paint.getStrokeWidth()
~~~~

</fiddle-embed></div>

<a name='SkPaint_setStrokeWidth'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setStrokeWidth'>setStrokeWidth</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>width</a>)
</pre>

Sets the thickness of the pen used by the <a href='SkPaint_Reference#Paint'>paint</a> <a href='SkPaint_Reference#Paint'>to</a>
outline the shape.
Has no effect if <a href='#SkPaint_setStrokeWidth_width'>width</a> <a href='#SkPaint_setStrokeWidth_width'>is</a> <a href='#SkPaint_setStrokeWidth_width'>less</a> <a href='#SkPaint_setStrokeWidth_width'>than</a> <a href='#SkPaint_setStrokeWidth_width'>zero</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeWidth_width'><code><strong>width</strong></code></a></td>
    <td>zero thickness for hairline; greater than zero for pen thickness</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0c4446c0870b5c7b5a2efe77ff92afb8">

#### Example Output

~~~~
5 == paint.getStrokeWidth()
~~~~

</fiddle-embed></div>

<a name='Miter_Limit'></a>

---

<a href='#Paint_Miter_Limit'>Miter_Limit</a> <a href='#Paint_Miter_Limit'>specifies</a> <a href='#Paint_Miter_Limit'>the</a> <a href='#Paint_Miter_Limit'>maximum</a> <a href='#Paint_Miter_Limit'>miter</a> <a href='#Paint_Miter_Limit'>length</a>,
<a href='#Paint_Miter_Limit'>relative</a> <a href='#Paint_Miter_Limit'>to</a> <a href='#Paint_Miter_Limit'>the</a>  <a href='#Stroke_Width'>stroke width</a>.

<a href='#Paint_Miter_Limit'>Miter_Limit</a> <a href='#Paint_Miter_Limit'>is</a> <a href='#Paint_Miter_Limit'>used</a> <a href='#Paint_Miter_Limit'>when</a> <a href='#Paint_Miter_Limit'>the</a> <a href='#Paint_Stroke_Join'>Stroke_Join</a>
<a href='#Paint_Stroke_Join'>is</a> <a href='#Paint_Stroke_Join'>set</a> <a href='#Paint_Stroke_Join'>to</a> <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>, <a href='#SkPaint_kMiter_Join'>and</a> <a href='#SkPaint_kMiter_Join'>the</a> <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_Style'>either</a> <a href='#SkPaint_kStroke_Style'>kStroke_Style</a>
<a href='#SkPaint_kStroke_Style'>or</a> <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>.

<a href='#SkPaint_kStrokeAndFill_Style'>If</a> <a href='#SkPaint_kStrokeAndFill_Style'>the</a> <a href='#SkPaint_kStrokeAndFill_Style'>miter</a> <a href='#SkPaint_kStrokeAndFill_Style'>at</a> <a href='#SkPaint_kStrokeAndFill_Style'>a</a> <a href='#SkPaint_kStrokeAndFill_Style'>corner</a> <a href='#SkPaint_kStrokeAndFill_Style'>exceeds</a> <a href='#SkPaint_kStrokeAndFill_Style'>this</a> <a href='#SkPaint_kStrokeAndFill_Style'>limit</a>, <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>
<a href='#SkPaint_kMiter_Join'>is</a> <a href='#SkPaint_kMiter_Join'>replaced</a> <a href='#SkPaint_kMiter_Join'>with</a> <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>.

<a href='#Paint_Miter_Limit'>Miter_Limit</a> <a href='#Paint_Miter_Limit'>can</a> <a href='#Paint_Miter_Limit'>be</a> <a href='#Paint_Miter_Limit'>computed</a> <a href='#Paint_Miter_Limit'>from</a> <a href='#Paint_Miter_Limit'>the</a> <a href='#Paint_Miter_Limit'>corner</a> <a href='#Paint_Miter_Limit'>angle</a> <a href='#Paint_Miter_Limit'>using</a>:
<code><a href='#Miter_Limit'>miter limit</a> = 1 / sin ( angle / 2 )</code>.

<a href='#Paint_Miter_Limit'>Miter_Limit</a> <a href='#Paint_Miter_Limit'>default</a> <a href='#Paint_Miter_Limit'>value</a> <a href='#Paint_Miter_Limit'>is</a> 4.
<a href='#Paint_Miter_Limit'>The</a> <a href='#Paint_Miter_Limit'>default</a> <a href='#Paint_Miter_Limit'>may</a> <a href='#Paint_Miter_Limit'>be</a> <a href='#Paint_Miter_Limit'>changed</a> <a href='#Paint_Miter_Limit'>at</a> <a href='#Paint_Miter_Limit'>compile</a> <a href='#Paint_Miter_Limit'>time</a> <a href='#Paint_Miter_Limit'>by</a> <a href='#Paint_Miter_Limit'>setting</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>SkPaintDefaults_MiterLimit</a>
<a href='undocumented#SkPaintDefaults_MiterLimit'>in</a> "<a href='undocumented#SkPaintDefaults_MiterLimit'>SkUserConfig</a>.<a href='undocumented#SkPaintDefaults_MiterLimit'>h</a>" <a href='undocumented#SkPaintDefaults_MiterLimit'>or</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>as</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>a</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>define</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>supplied</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>by</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>the</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>build</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>environment</a>.

<a href='undocumented#SkPaintDefaults_MiterLimit'>Here</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>are</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>some</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>miter</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>limits</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>and</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>the</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>angles</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>that</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>triggers</a> <a href='undocumented#SkPaintDefaults_MiterLimit'>them</a>.

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

<div><fiddle-embed name="5de2de0f00354e59074a9bb1a42d5a63"><div>This example draws a stroked corner and the miter length beneath.
When the  <a href='#Miter_Limit'>miter limit</a> is decreased slightly, the miter join is replaced
by a bevel join.
</div></fiddle-embed></div>

<a name='SkPaint_getStrokeMiter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getStrokeMiter'>getStrokeMiter</a>() <a href='#SkPaint_getStrokeMiter'>const</a>
</pre>

Returns the limit at which a sharp corner is drawn beveled.

### Return Value

zero and greater miter limit

### Example

<div><fiddle-embed name="50da74a43b725f07a914df588c867d36">

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
void <a href='#SkPaint_setStrokeMiter'>setStrokeMiter</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>miter</a>)
</pre>

Sets the limit at which a sharp corner is drawn beveled.
Valid values are zero and greater.
Has no effect if <a href='#SkPaint_setStrokeMiter_miter'>miter</a> <a href='#SkPaint_setStrokeMiter_miter'>is</a> <a href='#SkPaint_setStrokeMiter_miter'>less</a> <a href='#SkPaint_setStrokeMiter_miter'>than</a> <a href='#SkPaint_setStrokeMiter_miter'>zero</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeMiter_miter'><code><strong>miter</strong></code></a></td>
    <td>zero and greater  <a href='#Miter_Limit'>miter limit</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="700b284dbc97785c6a9c9636088713ad">

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

<a href='#Paint_Stroke_Cap'>Stroke_Cap</a> <a href='#Paint_Stroke_Cap'>draws</a> <a href='#Paint_Stroke_Cap'>at</a> <a href='#Paint_Stroke_Cap'>the</a> <a href='#Paint_Stroke_Cap'>beginning</a> <a href='#Paint_Stroke_Cap'>and</a> <a href='#Paint_Stroke_Cap'>end</a> <a href='#Paint_Stroke_Cap'>of</a> <a href='#Paint_Stroke_Cap'>an</a> <a href='#Paint_Stroke_Cap'>open</a> <a href='#Path_Overview_Contour'>Path_Contour</a>.

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
Adds a <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>with</a> <a href='undocumented#Circle'>a</a> <a href='undocumented#Circle'>diameter</a> <a href='undocumented#Circle'>equal</a> <a href='undocumented#Circle'>to</a> <a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Paint_Stroke_Width'>at</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#Paint_Stroke_Width'>beginning</a>
<a href='#Paint_Stroke_Width'>and</a> <a href='#Paint_Stroke_Width'>end</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kSquare_Cap'><code>SkPaint::kSquare_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds a square with sides equal to <a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Paint_Stroke_Width'>at</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#Paint_Stroke_Width'>beginning</a>
<a href='#Paint_Stroke_Width'>and</a> <a href='#Paint_Stroke_Width'>end</a>. <a href='#Paint_Stroke_Width'>The</a> <a href='#Paint_Stroke_Width'>square</a> <a href='#Paint_Stroke_Width'>sides</a> <a href='#Paint_Stroke_Width'>are</a> <a href='#Paint_Stroke_Width'>parallel</a> <a href='#Paint_Stroke_Width'>to</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#Paint_Stroke_Width'>initial</a> <a href='#Paint_Stroke_Width'>and</a> <a href='#Paint_Stroke_Width'>final</a> <a href='#Paint_Stroke_Width'>direction</a>
<a href='#Paint_Stroke_Width'>of</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#Paint_Stroke_Width'>stroke</a>.
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
<a href='#Paint_Stroke_Cap'>Stroke_Cap</a> <a href='#Paint_Stroke_Cap'>is</a> <a href='#Paint_Stroke_Cap'>set</a> <a href='#Paint_Stroke_Cap'>to</a> <a href='#SkPaint_kButt_Cap'>kButt_Cap</a> <a href='#SkPaint_kButt_Cap'>by</a> <a href='#SkPaint_kButt_Cap'>default</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kCapCount'><code>SkPaint::kCapCount</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#Paint_Stroke_Cap'>Stroke_Cap</a> <a href='#Paint_Stroke_Cap'>is</a> <a href='#Paint_Stroke_Cap'>a</a> <a href='#Paint_Stroke_Cap'>legal</a> <a href='#Paint_Stroke_Cap'>value</a>.
</td>
  </tr>
</table>

Stroke describes the area covered by a pen of <a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Paint_Stroke_Width'>as</a> <a href='#Paint_Stroke_Width'>it</a>
<a href='#Paint_Stroke_Width'>follows</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#Path_Overview_Contour'>Path_Contour</a>, <a href='#Path_Overview_Contour'>moving</a> <a href='#Path_Overview_Contour'>parallel</a> <a href='#Path_Overview_Contour'>to</a> <a href='#Path_Overview_Contour'>the</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>direction</a>.

<a href='SkPath_Overview#Contour'>If</a> <a href='SkPath_Overview#Contour'>the</a> <a href='#Path_Overview_Contour'>Path_Contour</a> <a href='#Path_Overview_Contour'>is</a> <a href='#Path_Overview_Contour'>not</a> <a href='#Path_Overview_Contour'>terminated</a> <a href='#Path_Overview_Contour'>by</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kClose_Verb'>kClose_Verb</a>, <a href='#SkPath_kClose_Verb'>the</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>has</a> <a href='SkPath_Overview#Contour'>a</a>
<a href='SkPath_Overview#Contour'>visible</a> <a href='SkPath_Overview#Contour'>beginning</a> <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>end</a>.

<a href='#Path_Overview_Contour'>Path_Contour</a> <a href='#Path_Overview_Contour'>may</a> <a href='#Path_Overview_Contour'>start</a> <a href='#Path_Overview_Contour'>and</a> <a href='#Path_Overview_Contour'>end</a> <a href='#Path_Overview_Contour'>at</a> <a href='#Path_Overview_Contour'>the</a> <a href='#Path_Overview_Contour'>same</a> <a href='SkPoint_Reference#Point'>point</a>; <a href='SkPoint_Reference#Point'>defining</a> <a href='#Path_Overview_Contour_Zero_Length'>Zero_Length_Contour</a>.

<a href='#SkPaint_kButt_Cap'>kButt_Cap</a> <a href='#SkPaint_kButt_Cap'>and</a> <a href='#Path_Overview_Contour_Zero_Length'>Zero_Length_Contour</a> <a href='#Path_Overview_Contour_Zero_Length'>is</a> <a href='#Path_Overview_Contour_Zero_Length'>not</a> <a href='#Path_Overview_Contour_Zero_Length'>drawn</a>.
<a href='#SkPaint_kRound_Cap'>kRound_Cap</a> <a href='#SkPaint_kRound_Cap'>and</a> <a href='#Path_Overview_Contour_Zero_Length'>Zero_Length_Contour</a> <a href='#Path_Overview_Contour_Zero_Length'>draws</a> <a href='#Path_Overview_Contour_Zero_Length'>a</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>of</a> <a href='undocumented#Circle'>diameter</a> <a href='#Paint_Stroke_Width'>Stroke_Width</a>
<a href='#Paint_Stroke_Width'>at</a> <a href='#Paint_Stroke_Width'>the</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPoint_Reference#Point'>point</a>.
<a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a> <a href='#SkPaint_kSquare_Cap'>and</a> <a href='#Path_Overview_Contour_Zero_Length'>Zero_Length_Contour</a> <a href='#Path_Overview_Contour_Zero_Length'>draws</a> <a href='#Path_Overview_Contour_Zero_Length'>an</a> <a href='#Path_Overview_Contour_Zero_Length'>upright</a> <a href='#Path_Overview_Contour_Zero_Length'>square</a> <a href='#Path_Overview_Contour_Zero_Length'>with</a> <a href='#Path_Overview_Contour_Zero_Length'>a</a> <a href='#Path_Overview_Contour_Zero_Length'>side</a> <a href='#Path_Overview_Contour_Zero_Length'>of</a>
<a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Paint_Stroke_Width'>at</a> <a href='#Paint_Stroke_Width'>the</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPoint_Reference#Point'>point</a>.

<a href='#Paint_Stroke_Cap'>Stroke_Cap</a> <a href='#Paint_Stroke_Cap'>is</a> <a href='#SkPaint_kButt_Cap'>kButt_Cap</a> <a href='#SkPaint_kButt_Cap'>by</a> <a href='#SkPaint_kButt_Cap'>default</a>.

### Example

<div><fiddle-embed name="2bffb6384cc20077e632e7d01da045ca"></fiddle-embed></div>

<a name='SkPaint_getStrokeCap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_getStrokeCap'>getStrokeCap</a>() <a href='#SkPaint_getStrokeCap'>const</a>
</pre>

Returns the geometry drawn at the beginning and end of strokes.

### Return Value

one of: <a href='#SkPaint_kButt_Cap'>kButt_Cap</a>, <a href='#SkPaint_kRound_Cap'>kRound_Cap</a>, <a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>

### Example

<div><fiddle-embed name="aabf9baee8e026fae36fca30e955512b">

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
void <a href='#SkPaint_setStrokeCap'>setStrokeCap</a>(<a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_Cap'>cap</a>)
</pre>

Sets the geometry drawn at the beginning and end of strokes.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeCap_cap'><code><strong>cap</strong></code></a></td>
    <td>one of: <a href='#SkPaint_kButt_Cap'>kButt_Cap</a>, <a href='#SkPaint_kRound_Cap'>kRound_Cap</a>, <a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>;</td>
  </tr>
</table>

has no effect if <a href='#SkPaint_setStrokeCap_cap'>cap</a> <a href='#SkPaint_setStrokeCap_cap'>is</a> <a href='#SkPaint_setStrokeCap_cap'>not</a> <a href='#SkPaint_setStrokeCap_cap'>valid</a>

### Example

<div><fiddle-embed name="de83fbd848a4625345b4b87a6e55d98a">

#### Example Output

~~~~
kRound_Cap == paint.getStrokeCap()
~~~~

</fiddle-embed></div>

### See Also

<a href='#Paint_Stroke_Cap'>Stroke_Cap</a> <a href='#SkPaint_getStrokeCap'>getStrokeCap</a>

<a name='Stroke_Join'></a>

<a href='#Paint_Stroke_Join'>Stroke_Join</a> <a href='#Paint_Stroke_Join'>draws</a> <a href='#Paint_Stroke_Join'>at</a> <a href='#Paint_Stroke_Join'>the</a> <a href='#Paint_Stroke_Join'>sharp</a> <a href='#Paint_Stroke_Join'>corners</a> <a href='#Paint_Stroke_Join'>of</a> <a href='#Paint_Stroke_Join'>an</a> <a href='#Paint_Stroke_Join'>open</a> <a href='#Paint_Stroke_Join'>or</a> <a href='#Paint_Stroke_Join'>closed</a> <a href='#Path_Overview_Contour'>Path_Contour</a>.

<a href='#Path_Overview_Contour'>Stroke</a> <a href='#Path_Overview_Contour'>describes</a> <a href='#Path_Overview_Contour'>the</a> <a href='#Path_Overview_Contour'>area</a> <a href='#Path_Overview_Contour'>covered</a> <a href='#Path_Overview_Contour'>by</a> <a href='#Path_Overview_Contour'>a</a> <a href='#Path_Overview_Contour'>pen</a> <a href='#Path_Overview_Contour'>of</a> <a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Paint_Stroke_Width'>as</a> <a href='#Paint_Stroke_Width'>it</a>
<a href='#Paint_Stroke_Width'>follows</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#Path_Overview_Contour'>Path_Contour</a>, <a href='#Path_Overview_Contour'>moving</a> <a href='#Path_Overview_Contour'>parallel</a> <a href='#Path_Overview_Contour'>to</a> <a href='#Path_Overview_Contour'>the</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>direction</a>.

<a href='SkPath_Overview#Contour'>If</a> <a href='SkPath_Overview#Contour'>the</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>direction</a> <a href='SkPath_Overview#Contour'>changes</a> <a href='SkPath_Overview#Contour'>abruptly</a>, <a href='SkPath_Overview#Contour'>because</a> <a href='SkPath_Overview#Contour'>the</a> <a href='SkPath_Overview#Contour'>tangent</a> <a href='SkPath_Overview#Contour'>direction</a> <a href='SkPath_Overview#Contour'>leading</a>
<a href='SkPath_Overview#Contour'>to</a> <a href='SkPath_Overview#Contour'>the</a> <a href='SkPath_Overview#Contour'>end</a> <a href='SkPath_Overview#Contour'>of</a> <a href='SkPath_Overview#Contour'>a</a> <a href='undocumented#Curve'>curve</a> <a href='undocumented#Curve'>within</a> <a href='undocumented#Curve'>the</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>does</a> <a href='SkPath_Overview#Contour'>not</a> <a href='SkPath_Overview#Contour'>match</a> <a href='SkPath_Overview#Contour'>the</a> <a href='SkPath_Overview#Contour'>tangent</a> <a href='SkPath_Overview#Contour'>direction</a> <a href='SkPath_Overview#Contour'>of</a>
<a href='SkPath_Overview#Contour'>the</a> <a href='SkPath_Overview#Contour'>following</a> <a href='undocumented#Curve'>curve</a>, <a href='undocumented#Curve'>the</a> <a href='undocumented#Curve'>pair</a> <a href='undocumented#Curve'>of</a> <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>meet</a> <a href='undocumented#Curve'>at</a> <a href='#Paint_Stroke_Join'>Stroke_Join</a>.

### Example

<div><fiddle-embed name="917c44b504d3f9308571fd3835d90a0d"></fiddle-embed></div>

<a name='SkPaint_Join'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Join'>Join</a> : <a href='#SkPaint_Join'>uint8_t</a> {
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

<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_Join'>specifies</a> <a href='#SkPaint_Join'>how</a> <a href='#SkPaint_Join'>corners</a> <a href='#SkPaint_Join'>are</a> <a href='#SkPaint_Join'>drawn</a> <a href='#SkPaint_Join'>when</a> <a href='#SkPaint_Join'>a</a> <a href='#SkPaint_Join'>shape</a> <a href='#SkPaint_Join'>is</a> <a href='#SkPaint_Join'>stroked</a>. <a href='#SkPaint_Join'>Join</a>
<a href='#SkPaint_Join'>affects</a> <a href='#SkPaint_Join'>the</a> <a href='#SkPaint_Join'>four</a> <a href='#SkPaint_Join'>corners</a> <a href='#SkPaint_Join'>of</a> <a href='#SkPaint_Join'>a</a> <a href='#SkPaint_Join'>stroked</a> <a href='#SkPaint_Join'>rectangle</a>, <a href='#SkPaint_Join'>and</a> <a href='#SkPaint_Join'>the</a> <a href='#SkPaint_Join'>connected</a> <a href='#SkPaint_Join'>segments</a> <a href='#SkPaint_Join'>in</a> <a href='#SkPaint_Join'>a</a>
<a href='#SkPaint_Join'>stroked</a> <a href='SkPath_Reference#Path'>path</a>.

<a href='SkPath_Reference#Path'>Choose</a> <a href='SkPath_Reference#Path'>miter</a> <a href='SkPath_Reference#Path'>join</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>draw</a> <a href='SkPath_Reference#Path'>sharp</a> <a href='SkPath_Reference#Path'>corners</a>. <a href='SkPath_Reference#Path'>Choose</a> <a href='SkPath_Reference#Path'>round</a> <a href='SkPath_Reference#Path'>join</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>draw</a> <a href='SkPath_Reference#Path'>a</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>with</a> <a href='undocumented#Circle'>a</a>
<a href='undocumented#Circle'>radius</a> <a href='undocumented#Circle'>equal</a> <a href='undocumented#Circle'>to</a> <a href='undocumented#Circle'>the</a>  <a href='#Stroke_Width'>stroke width</a> <a href='undocumented#Circle'>on</a> <a href='undocumented#Circle'>top</a> <a href='undocumented#Circle'>of</a> <a href='undocumented#Circle'>the</a> <a href='undocumented#Circle'>corner</a>. <a href='undocumented#Circle'>Choose</a> <a href='undocumented#Circle'>bevel</a> <a href='undocumented#Circle'>join</a> <a href='undocumented#Circle'>to</a> <a href='undocumented#Circle'>minimally</a>
<a href='undocumented#Circle'>connect</a> <a href='undocumented#Circle'>the</a> <a href='undocumented#Circle'>thick</a> <a href='undocumented#Circle'>strokes</a>.

<a href='undocumented#Circle'>The</a>  <a href='#Fill_Path'>fill path</a> <a href='undocumented#Circle'>constructed</a> <a href='undocumented#Circle'>to</a> <a href='undocumented#Circle'>describe</a> <a href='undocumented#Circle'>the</a> <a href='undocumented#Circle'>stroked</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>respects</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>join</a> <a href='SkPath_Reference#Path'>setting</a> <a href='SkPath_Reference#Path'>but</a> <a href='SkPath_Reference#Path'>may</a>
<a href='SkPath_Reference#Path'>not</a> <a href='SkPath_Reference#Path'>contain</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>actual</a> <a href='SkPath_Reference#Path'>join</a>. <a href='SkPath_Reference#Path'>For</a> <a href='SkPath_Reference#Path'>instance</a>, <a href='SkPath_Reference#Path'>a</a>  <a href='#Fill_Path'>fill path</a> <a href='SkPath_Reference#Path'>constructed</a> <a href='SkPath_Reference#Path'>with</a> <a href='SkPath_Reference#Path'>round</a> <a href='SkPath_Reference#Path'>joins</a> <a href='SkPath_Reference#Path'>does</a>
<a href='SkPath_Reference#Path'>not</a> <a href='SkPath_Reference#Path'>necessarily</a> <a href='SkPath_Reference#Path'>include</a> <a href='undocumented#Circle'>circles</a> <a href='undocumented#Circle'>at</a> <a href='undocumented#Circle'>each</a> <a href='undocumented#Circle'>connected</a> <a href='undocumented#Circle'>segment</a>.

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
<a href='#Paint_Miter_Limit'>If</a> <a href='#Paint_Miter_Limit'>the</a> <a href='#Paint_Miter_Limit'>extension</a> <a href='#Paint_Miter_Limit'>exceeds</a> <a href='#Paint_Miter_Limit'>Miter_Limit</a>, <a href='#SkPaint_kBevel_Join'>kBevel_Join</a> <a href='#SkPaint_kBevel_Join'>is</a> <a href='#SkPaint_kBevel_Join'>used</a> <a href='#SkPaint_kBevel_Join'>instead</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kRound_Join'><code>SkPaint::kRound_Join</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds a <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>with</a> <a href='undocumented#Circle'>a</a> <a href='undocumented#Circle'>diameter</a> <a href='undocumented#Circle'>of</a> <a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Paint_Stroke_Width'>at</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#Paint_Stroke_Width'>sharp</a> <a href='#Paint_Stroke_Width'>corner</a>.
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
<a href='#Paint_Stroke_Join'>Stroke_Join</a> <a href='#Paint_Stroke_Join'>is</a> <a href='#Paint_Stroke_Join'>set</a> <a href='#Paint_Stroke_Join'>to</a> <a href='#SkPaint_kMiter_Join'>kMiter_Join</a> <a href='#SkPaint_kMiter_Join'>by</a> <a href='#SkPaint_kMiter_Join'>default</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kJoinCount'><code>SkPaint::kJoinCount</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#Paint_Stroke_Join'>Stroke_Join</a> <a href='#Paint_Stroke_Join'>is</a> <a href='#Paint_Stroke_Join'>a</a> <a href='#Paint_Stroke_Join'>legal</a> <a href='#Paint_Stroke_Join'>value</a>.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3b1aebacc21c1836a52876b9b0b3905e"></fiddle-embed></div>

### See Also

<a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a> <a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a> <a href='#SkPaint_setStrokeMiter'>setStrokeMiter</a> <a href='#SkPaint_getStrokeMiter'>getStrokeMiter</a>

<a name='SkPaint_getStrokeJoin'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a>() <a href='#SkPaint_getStrokeJoin'>const</a>
</pre>

Returns the geometry drawn at the corners of strokes.

### Return Value

one of: <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>, <a href='#SkPaint_kRound_Join'>kRound_Join</a>, <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>

### Example

<div><fiddle-embed name="31bf751d0a8ddf176b871810820d8199">

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
void <a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a>(<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_Join'>join</a>)
</pre>

Sets the geometry drawn at the corners of strokes.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeJoin_join'><code><strong>join</strong></code></a></td>
    <td>one of: <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>, <a href='#SkPaint_kRound_Join'>kRound_Join</a>, <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>;</td>
  </tr>
</table>

otherwise, has no effect

### Example

<div><fiddle-embed name="48d963ad4286eddf680f9c511eb6da91">

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

<a href='#Paint_Fill_Path'>Fill_Path</a> <a href='#Paint_Fill_Path'>creates</a> <a href='#Paint_Fill_Path'>a</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>by</a> <a href='SkPath_Reference#Path'>applying</a> <a href='SkPath_Reference#Path'>the</a> <a href='#Path_Effect'>Path_Effect</a>, <a href='#Path_Effect'>followed</a> <a href='#Path_Effect'>by</a> <a href='#Path_Effect'>the</a> <a href='#Paint_Style_Stroke'>Style_Stroke</a>.

<a href='#Paint_Style_Stroke'>If</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>contains</a> <a href='#Path_Effect'>Path_Effect</a>, <a href='#Path_Effect'>Path_Effect</a> <a href='#Path_Effect'>operates</a> <a href='#Path_Effect'>on</a> <a href='#Path_Effect'>the</a> <a href='#Path_Effect'>source</a> <a href='SkPath_Reference#Path'>Path</a>; <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>result</a>
<a href='SkPath_Reference#Path'>replaces</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>destination</a> <a href='SkPath_Reference#Path'>Path</a>. <a href='SkPath_Reference#Path'>Otherwise</a>, <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>source</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>replaces</a> <a href='SkPath_Reference#Path'>the</a>
<a href='SkPath_Reference#Path'>destination</a> <a href='SkPath_Reference#Path'>Path</a>.

<a href='SkPath_Reference#Path'>Fill</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>can</a> <a href='SkPath_Reference#Path'>request</a> <a href='SkPath_Reference#Path'>the</a> <a href='#Path_Effect'>Path_Effect</a> <a href='#Path_Effect'>to</a> <a href='#Path_Effect'>restrict</a> <a href='#Path_Effect'>to</a> <a href='#Path_Effect'>a</a> <a href='#Path_Effect'>culling</a> <a href='#Path_Effect'>rectangle</a>, <a href='#Path_Effect'>but</a>
<a href='#Path_Effect'>the</a> <a href='#Path_Effect'>Path_Effect</a> <a href='#Path_Effect'>is</a> <a href='#Path_Effect'>not</a> <a href='#Path_Effect'>required</a> <a href='#Path_Effect'>to</a> <a href='#Path_Effect'>do</a> <a href='#Path_Effect'>so</a>.

<a href='#Path_Effect'>If</a> <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_kStroke_Style'>kStroke_Style</a> <a href='#SkPaint_kStroke_Style'>or</a> <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>,
<a href='#SkPaint_kStrokeAndFill_Style'>and</a> <a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Paint_Stroke_Width'>is</a> <a href='#Paint_Stroke_Width'>greater</a> <a href='#Paint_Stroke_Width'>than</a> <a href='#Paint_Stroke_Width'>zero</a>, <a href='#Paint_Stroke_Width'>the</a> <a href='#Paint_Stroke_Width'>Stroke_Width</a>, <a href='#Paint_Stroke_Cap'>Stroke_Cap</a>, <a href='#Paint_Stroke_Join'>Stroke_Join</a>,
<a href='#Paint_Stroke_Join'>and</a> <a href='#Paint_Miter_Limit'>Miter_Limit</a> <a href='#Paint_Miter_Limit'>operate</a> <a href='#Paint_Miter_Limit'>on</a> <a href='#Paint_Miter_Limit'>the</a> <a href='#Paint_Miter_Limit'>destination</a> <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>replacing</a> <a href='SkPath_Reference#Path'>it</a>.

<a href='SkPath_Reference#Path'>Fill</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>can</a> <a href='SkPath_Reference#Path'>specify</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>precision</a> <a href='SkPath_Reference#Path'>used</a> <a href='SkPath_Reference#Path'>by</a> <a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Paint_Stroke_Width'>to</a> <a href='#Paint_Stroke_Width'>approximate</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#Paint_Stroke_Width'>stroke</a> <a href='#Paint_Stroke_Width'>geometry</a>.

<a href='#Paint_Stroke_Width'>If</a> <a href='#Paint_Stroke_Width'>the</a> <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_kStroke_Style'>kStroke_Style</a> <a href='#SkPaint_kStroke_Style'>and</a> <a href='#SkPaint_kStroke_Style'>the</a> <a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Paint_Stroke_Width'>is</a> <a href='#Paint_Stroke_Width'>zero</a>, <a href='#SkPaint_getFillPath'>getFillPath</a>
<a href='#SkPaint_getFillPath'>returns</a> <a href='#SkPaint_getFillPath'>false</a> <a href='#SkPaint_getFillPath'>since</a> <a href='#SkPaint_getFillPath'>Hairline</a> <a href='#SkPaint_getFillPath'>has</a> <a href='#SkPaint_getFillPath'>no</a> <a href='#SkPaint_getFillPath'>filled</a> <a href='#SkPaint_getFillPath'>equivalent</a>.

### See Also

<a href='#Paint_Style_Stroke'>Style_Stroke</a> <a href='#Paint_Stroke_Width'>Stroke_Width</a> <a href='#Path_Effect'>Path_Effect</a>

<a name='SkPaint_getFillPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_getFillPath'>getFillPath</a>(<a href='#SkPaint_getFillPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>src</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#SkPath'>dst</a>, <a href='SkPath_Reference#SkPath'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>cullRect</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>resScale</a> = 1) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns the filled equivalent of the stroked <a href='SkPath_Reference#Path'>path</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getFillPath_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>read</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>create</a> <a href='SkPath_Reference#SkPath'>a</a> <a href='SkPath_Reference#SkPath'>filled</a> <a href='SkPath_Reference#SkPath'>version</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_dst'><code><strong>dst</strong></code></a></td>
    <td>resulting <a href='SkPath_Reference#SkPath'>SkPath</a>; <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>be</a> <a href='SkPath_Reference#SkPath'>the</a> <a href='SkPath_Reference#SkPath'>same</a> <a href='SkPath_Reference#SkPath'>as</a> <a href='#SkPaint_getFillPath_src'>src</a>, <a href='#SkPaint_getFillPath_src'>but</a> <a href='#SkPaint_getFillPath_src'>may</a> <a href='#SkPaint_getFillPath_src'>not</a> <a href='#SkPaint_getFillPath_src'>be</a> <a href='#SkPaint_getFillPath_src'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>optional limit passed to <a href='undocumented#SkPathEffect'>SkPathEffect</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_resScale'><code><strong>resScale</strong></code></a></td>
    <td>if > 1, increase precision, else if (0 < <a href='#SkPaint_getFillPath_resScale'>resScale</a> < 1) <a href='#SkPaint_getFillPath_resScale'>reduce</a> <a href='#SkPaint_getFillPath_resScale'>precision</a></td>
  </tr>
</table>

to favor speed and <a href='undocumented#Size'>size</a>

### Return Value

true if the <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>represents</a>  <a href='#Style_Fill'>style fill</a>, <a href='SkPath_Reference#Path'>or</a> <a href='SkPath_Reference#Path'>false</a> <a href='SkPath_Reference#Path'>if</a> <a href='SkPath_Reference#Path'>it</a> <a href='SkPath_Reference#Path'>represents</a> <a href='SkPath_Reference#Path'>hairline</a>

### Example

<div><fiddle-embed name="cedd6233848198e1fca4d1e14816baaf"><div>A very small <a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>stroke</a> <a href='SkPath_Reference#Quad'>is</a> <a href='SkPath_Reference#Quad'>turned</a> <a href='SkPath_Reference#Quad'>into</a> <a href='SkPath_Reference#Quad'>a</a> <a href='SkPath_Reference#Quad'>filled</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>with</a> <a href='SkPath_Reference#Path'>increasing</a> <a href='SkPath_Reference#Path'>levels</a> <a href='SkPath_Reference#Path'>of</a> <a href='SkPath_Reference#Path'>precision</a>.
<a href='SkPath_Reference#Path'>At</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>lowest</a> <a href='SkPath_Reference#Path'>precision</a>, <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>stroke</a> <a href='SkPath_Reference#Quad'>is</a> <a href='SkPath_Reference#Quad'>approximated</a> <a href='SkPath_Reference#Quad'>by</a> <a href='SkPath_Reference#Quad'>a</a> <a href='SkPath_Reference#Quad'>rectangle</a>.
<a href='SkPath_Reference#Quad'>At</a> <a href='SkPath_Reference#Quad'>the</a> <a href='SkPath_Reference#Quad'>highest</a> <a href='SkPath_Reference#Quad'>precision</a>, <a href='SkPath_Reference#Quad'>the</a> <a href='SkPath_Reference#Quad'>filled</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>has</a> <a href='SkPath_Reference#Path'>high</a> <a href='SkPath_Reference#Path'>fidelity</a> <a href='SkPath_Reference#Path'>compared</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>original</a> <a href='SkPath_Reference#Path'>stroke</a>.
</div></fiddle-embed></div>

<a name='SkPaint_getFillPath_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_getFillPath'>getFillPath</a>(<a href='#SkPaint_getFillPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>src</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#SkPath'>dst</a>) <a href='SkPath_Reference#SkPath'>const</a>
</pre>

Returns the filled equivalent of the stroked <a href='SkPath_Reference#Path'>path</a>.

Replaces <a href='#SkPaint_getFillPath_2_dst'>dst</a> <a href='#SkPaint_getFillPath_2_dst'>with</a> <a href='#SkPaint_getFillPath_2_dst'>the</a> <a href='#SkPaint_getFillPath_2_src'>src</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>modified</a> <a href='SkPath_Reference#Path'>by</a> <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>and</a>  <a href='#Style_Stroke'>style stroke</a>.
<a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkPathEffect'>if</a> <a href='undocumented#SkPathEffect'>any</a>, <a href='undocumented#SkPathEffect'>is</a> <a href='undocumented#SkPathEffect'>not</a> <a href='undocumented#SkPathEffect'>culled</a>.  <a href='#Stroke_Width'>stroke width</a> <a href='undocumented#SkPathEffect'>is</a> <a href='undocumented#SkPathEffect'>created</a> <a href='undocumented#SkPathEffect'>with</a> <a href='undocumented#SkPathEffect'>default</a> <a href='undocumented#SkPathEffect'>precision</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getFillPath_2_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>read</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>create</a> <a href='SkPath_Reference#SkPath'>a</a> <a href='SkPath_Reference#SkPath'>filled</a> <a href='SkPath_Reference#SkPath'>version</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_2_dst'><code><strong>dst</strong></code></a></td>
    <td>resulting <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='#SkPaint_getFillPath_2_dst'>dst</a> <a href='#SkPaint_getFillPath_2_dst'>may</a> <a href='#SkPaint_getFillPath_2_dst'>be</a> <a href='#SkPaint_getFillPath_2_dst'>the</a> <a href='#SkPaint_getFillPath_2_dst'>same</a> <a href='#SkPaint_getFillPath_2_dst'>as</a> <a href='#SkPaint_getFillPath_2_src'>src</a>, <a href='#SkPaint_getFillPath_2_src'>but</a> <a href='#SkPaint_getFillPath_2_src'>may</a> <a href='#SkPaint_getFillPath_2_src'>not</a> <a href='#SkPaint_getFillPath_2_src'>be</a> <a href='#SkPaint_getFillPath_2_src'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if the <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>represents</a>  <a href='#Style_Fill'>style fill</a>, <a href='SkPath_Reference#Path'>or</a> <a href='SkPath_Reference#Path'>false</a> <a href='SkPath_Reference#Path'>if</a> <a href='SkPath_Reference#Path'>it</a> <a href='SkPath_Reference#Path'>represents</a> <a href='SkPath_Reference#Path'>hairline</a>

### Example

<div><fiddle-embed name="e6d8ca0cc17e0b475bd54dd995825468"></fiddle-embed></div>

<a name='Shader_Methods'></a>

---

<a href='undocumented#Shader'>Shader</a> <a href='undocumented#Shader'>defines</a> <a href='undocumented#Shader'>the</a> <a href='undocumented#Shader'>colors</a> <a href='undocumented#Shader'>used</a> <a href='undocumented#Shader'>when</a> <a href='undocumented#Shader'>drawing</a> <a href='undocumented#Shader'>a</a> <a href='undocumented#Shader'>shape</a>.
<a href='undocumented#Shader'>Shader</a> <a href='undocumented#Shader'>may</a> <a href='undocumented#Shader'>be</a> <a href='undocumented#Shader'>an</a> <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>a</a> <a href='SkImage_Reference#Image'>gradient</a>, <a href='SkImage_Reference#Image'>or</a> <a href='SkImage_Reference#Image'>a</a> <a href='SkImage_Reference#Image'>computed</a> <a href='SkImage_Reference#Image'>fill</a>.
<a href='SkImage_Reference#Image'>If</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>has</a> <a href='SkPaint_Reference#Paint'>no</a> <a href='undocumented#Shader'>Shader</a>, <a href='undocumented#Shader'>then</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>fills</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>shape</a>.

<a href='undocumented#Shader'>Shader</a> <a href='undocumented#Shader'>is</a> <a href='undocumented#Shader'>modulated</a> <a href='undocumented#Shader'>by</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>component</a> <a href='#Color_Alpha'>of</a> <a href='SkColor_Reference#Color'>Color</a>.
<a href='SkColor_Reference#Color'>If</a> <a href='undocumented#Shader'>Shader</a> <a href='undocumented#Shader'>object</a> <a href='undocumented#Shader'>defines</a> <a href='undocumented#Shader'>only</a> <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Color_Alpha'>then</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>modulated</a> <a href='SkColor_Reference#Color'>by</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>describes</a>
<a href='#Color_Alpha'>the</a> <a href='#Color_Alpha'>fill</a>.

<a href='#Color_Alpha'>The</a> <a href='#Color_Alpha'>drawn</a> <a href='#Color_Alpha'>transparency</a> <a href='#Color_Alpha'>can</a> <a href='#Color_Alpha'>be</a> <a href='#Color_Alpha'>modified</a> <a href='#Color_Alpha'>without</a> <a href='#Color_Alpha'>altering</a> <a href='undocumented#Shader'>Shader</a>, <a href='undocumented#Shader'>by</a> <a href='undocumented#Shader'>changing</a> <a href='#Color_Alpha'>Color_Alpha</a>.

### Example

<div><fiddle-embed name="c015dc2010c15e1c00b4f7330232b0f7"></fiddle-embed></div>

If <a href='undocumented#Shader'>Shader</a> <a href='undocumented#Shader'>generates</a> <a href='undocumented#Shader'>only</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>then</a> <a href='#Color_Alpha'>all</a> <a href='#Color_Alpha'>components</a> <a href='#Color_Alpha'>of</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>modulate</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>output</a>.

### Example

<div><fiddle-embed name="fe80fd80b98a20823db7fb9a077243c7"></fiddle-embed></div>

<a name='SkPaint_getShader'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkShader'>SkShader</a>* <a href='#SkPaint_getShader'>getShader</a>() <a href='#SkPaint_getShader'>const</a>
</pre>

Returns optional colors used when filling a <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>such</a> <a href='SkPath_Reference#Path'>as</a> <a href='SkPath_Reference#Path'>a</a> <a href='SkPath_Reference#Path'>gradient</a>.

Does not alter <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>if</a> <a href='undocumented#SkShader'>previously</a> <a href='undocumented#SkShader'>set</a>, <a href='undocumented#SkShader'>nullptr</a> <a href='undocumented#SkShader'>otherwise</a>

### Example

<div><fiddle-embed name="09f15b9fd88882850da2d235eb86292f">

#### Example Output

~~~~
nullptr == shader
nullptr != shader
~~~~

</fiddle-embed></div>

<a name='SkPaint_refShader'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkShader'>SkShader</a>&<a href='undocumented#SkShader'>gt</a>; <a href='#SkPaint_refShader'>refShader</a>() <a href='#SkPaint_refShader'>const</a>
</pre>

Returns optional colors used when filling a <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>such</a> <a href='SkPath_Reference#Path'>as</a> <a href='SkPath_Reference#Path'>a</a> <a href='SkPath_Reference#Path'>gradient</a>.

Increases <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Return Value

<a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>if</a> <a href='undocumented#SkShader'>previously</a> <a href='undocumented#SkShader'>set</a>, <a href='undocumented#SkShader'>nullptr</a> <a href='undocumented#SkShader'>otherwise</a>

### Example

<div><fiddle-embed name="53da0295972a418cbc9607bbb17feaa8">

#### Example Output

~~~~
shader unique: true
shader unique: false
~~~~

</fiddle-embed></div>

<a name='SkPaint_setShader'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setShader'>setShader</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkShader'>SkShader</a>&<a href='undocumented#SkShader'>gt</a>; <a href='undocumented#Shader'>shader</a>)
</pre>

Sets optional colors used when filling a <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>such</a> <a href='SkPath_Reference#Path'>as</a> <a href='SkPath_Reference#Path'>a</a> <a href='SkPath_Reference#Path'>gradient</a>.

Sets <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>to</a> <a href='#SkPaint_setShader_shader'>shader</a>, <a href='#SkPaint_setShader_shader'>decreasing</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>of</a> <a href='undocumented#SkRefCnt'>the</a> <a href='undocumented#SkRefCnt'>previous</a> <a href='undocumented#SkShader'>SkShader</a>.
Increments <a href='#SkPaint_setShader_shader'>shader</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setShader_shader'><code><strong>shader</strong></code></a></td>
    <td>how geometry is filled with <a href='SkColor_Reference#Color'>color</a>; <a href='SkColor_Reference#Color'>if</a> <a href='SkColor_Reference#Color'>nullptr</a>, <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>is</a> <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>instead</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="77e64d5bae9b1ba037fd99252bb4aa58"></fiddle-embed></div>

<a name='Color_Filter_Methods'></a>

---

<a href='#Color_Filter'>Color_Filter</a> <a href='#Color_Filter'>alters</a> <a href='#Color_Filter'>the</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>drawing</a> <a href='SkColor_Reference#Color'>a</a> <a href='SkColor_Reference#Color'>shape</a>.
<a href='#Color_Filter'>Color_Filter</a> <a href='#Color_Filter'>may</a> <a href='#Color_Filter'>apply</a> <a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Blend_Mode'>transform</a> <a href='#Blend_Mode'>the</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>through</a> <a href='SkColor_Reference#Color'>a</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='SkMatrix_Reference#Matrix'>or</a> <a href='SkMatrix_Reference#Matrix'>composite</a> <a href='SkMatrix_Reference#Matrix'>multiple</a> <a href='SkMatrix_Reference#Matrix'>filters</a>.
<a href='SkMatrix_Reference#Matrix'>If</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>has</a> <a href='SkPaint_Reference#Paint'>no</a> <a href='#Color_Filter'>Color_Filter</a>, <a href='#Color_Filter'>the</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>is</a> <a href='SkColor_Reference#Color'>unaltered</a>.

<a href='SkColor_Reference#Color'>The</a> <a href='SkColor_Reference#Color'>drawn</a> <a href='SkColor_Reference#Color'>transparency</a> <a href='SkColor_Reference#Color'>can</a> <a href='SkColor_Reference#Color'>be</a> <a href='SkColor_Reference#Color'>modified</a> <a href='SkColor_Reference#Color'>without</a> <a href='SkColor_Reference#Color'>altering</a> <a href='#Color_Filter'>Color_Filter</a>, <a href='#Color_Filter'>by</a> <a href='#Color_Filter'>changing</a> <a href='#Color_Alpha'>Color_Alpha</a>.

### Example

<div><fiddle-embed name="5abde56ca2f89a18b8e231abd1b57c56"></fiddle-embed></div>

<a name='SkPaint_getColorFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkColorFilter'>SkColorFilter</a>* <a href='#SkPaint_getColorFilter'>getColorFilter</a>() <a href='#SkPaint_getColorFilter'>const</a>
</pre>

Returns <a href='undocumented#SkColorFilter'>SkColorFilter</a> <a href='undocumented#SkColorFilter'>if</a> <a href='undocumented#SkColorFilter'>set</a>, <a href='undocumented#SkColorFilter'>or</a> <a href='undocumented#SkColorFilter'>nullptr</a>.
Does not alter <a href='undocumented#SkColorFilter'>SkColorFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkColorFilter'>SkColorFilter</a> <a href='undocumented#SkColorFilter'>if</a> <a href='undocumented#SkColorFilter'>previously</a> <a href='undocumented#SkColorFilter'>set</a>, <a href='undocumented#SkColorFilter'>nullptr</a> <a href='undocumented#SkColorFilter'>otherwise</a>

### Example

<div><fiddle-embed name="093bdc627d6b59002670fd290931f6c9">

#### Example Output

~~~~
nullptr == color filter
nullptr != color filter
~~~~

</fiddle-embed></div>

<a name='SkPaint_refColorFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorFilter'>SkColorFilter</a>&<a href='undocumented#SkColorFilter'>gt</a>; <a href='#SkPaint_refColorFilter'>refColorFilter</a>() <a href='#SkPaint_refColorFilter'>const</a>
</pre>

Returns <a href='undocumented#SkColorFilter'>SkColorFilter</a> <a href='undocumented#SkColorFilter'>if</a> <a href='undocumented#SkColorFilter'>set</a>, <a href='undocumented#SkColorFilter'>or</a> <a href='undocumented#SkColorFilter'>nullptr</a>.
Increases <a href='undocumented#SkColorFilter'>SkColorFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Return Value

<a href='undocumented#SkColorFilter'>SkColorFilter</a> <a href='undocumented#SkColorFilter'>if</a> <a href='undocumented#SkColorFilter'>set</a>, <a href='undocumented#SkColorFilter'>or</a> <a href='undocumented#SkColorFilter'>nullptr</a>

### Example

<div><fiddle-embed name="b588c95fa4c86ddbc4b0546762f08297">

#### Example Output

~~~~
color filter unique: true
color filter unique: false
~~~~

</fiddle-embed></div>

<a name='SkPaint_setColorFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setColorFilter'>setColorFilter</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorFilter'>SkColorFilter</a>&<a href='undocumented#SkColorFilter'>gt</a>; <a href='undocumented#SkColorFilter'>colorFilter</a>)
</pre>

Sets <a href='undocumented#SkColorFilter'>SkColorFilter</a> <a href='undocumented#SkColorFilter'>to</a> <a href='undocumented#SkColorFilter'>filter</a>, <a href='undocumented#SkColorFilter'>decreasing</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>of</a> <a href='undocumented#SkRefCnt'>the</a> <a href='undocumented#SkRefCnt'>previous</a>
<a href='undocumented#SkColorFilter'>SkColorFilter</a>. <a href='undocumented#SkColorFilter'>Pass</a> <a href='undocumented#SkColorFilter'>nullptr</a> <a href='undocumented#SkColorFilter'>to</a> <a href='undocumented#SkColorFilter'>clear</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>.

Increments filter <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setColorFilter_colorFilter'><code><strong>colorFilter</strong></code></a></td>
    <td><a href='undocumented#SkColorFilter'>SkColorFilter</a> <a href='undocumented#SkColorFilter'>to</a> <a href='undocumented#SkColorFilter'>apply</a> <a href='undocumented#SkColorFilter'>to</a> <a href='undocumented#SkColorFilter'>subsequent</a> <a href='undocumented#SkColorFilter'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c7b786dc9b3501cd0eaba47494b6fa31"></fiddle-embed></div>

<a name='Blend_Mode_Methods'></a>

---

<a href='#Blend_Mode'>Blend_Mode</a> <a href='#Blend_Mode'>describes</a> <a href='#Blend_Mode'>how</a> <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>combines</a> <a href='SkColor_Reference#Color'>with</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>destination</a> <a href='SkColor_Reference#Color'>color</a>.
<a href='SkColor_Reference#Color'>The</a> <a href='SkColor_Reference#Color'>default</a> <a href='SkColor_Reference#Color'>setting</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>, <a href='#SkBlendMode_kSrcOver'>draws</a> <a href='#SkBlendMode_kSrcOver'>the</a> <a href='#SkBlendMode_kSrcOver'>source</a> <a href='SkColor_Reference#Color'>color</a>
<a href='SkColor_Reference#Color'>over</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Color'>destination</a> <a href='SkColor_Reference#Color'>color</a>.

### Example

<div><fiddle-embed name="73092d4d06faecea3c204d852a4dd8a8"></fiddle-embed></div>

### See Also

<a href='#Blend_Mode'>Blend_Mode</a>

<a name='SkPaint_getBlendMode'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkPaint_getBlendMode'>getBlendMode</a>() <a href='#SkPaint_getBlendMode'>const</a>
</pre>

Returns <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>.
By default, returns <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>.

### Return Value

mode used to combine source <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>with</a> <a href='SkColor_Reference#Color'>destination</a> <a href='SkColor_Reference#Color'>color</a>

### Example

<div><fiddle-embed name="a1e059c8f6740fa2044cc64152b39dda">

#### Example Output

~~~~
kSrcOver == getBlendMode
kSrcOver != getBlendMode
~~~~

</fiddle-embed></div>

<a name='SkPaint_isSrcOver'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isSrcOver'>isSrcOver</a>() <a href='#SkPaint_isSrcOver'>const</a>
</pre>

Returns true if <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>is</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>, <a href='#SkBlendMode_kSrcOver'>the</a> <a href='#SkBlendMode_kSrcOver'>default</a>.

### Return Value

true if <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>is</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>

### Example

<div><fiddle-embed name="257c9473db7a2b3a0fb2b9e2431e59a6">

#### Example Output

~~~~
isSrcOver == true
isSrcOver != true
~~~~

</fiddle-embed></div>

<a name='SkPaint_setBlendMode'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setBlendMode'>setBlendMode</a>(<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>)
</pre>

Sets <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>to</a> <a href='#SkPaint_setBlendMode_mode'>mode</a>.
Does not check for valid input.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setBlendMode_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>used</a> <a href='SkBlendMode_Reference#SkBlendMode'>to</a> <a href='SkBlendMode_Reference#SkBlendMode'>combine</a> <a href='SkBlendMode_Reference#SkBlendMode'>source</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>destination</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="257c9473db7a2b3a0fb2b9e2431e59a6">

#### Example Output

~~~~
isSrcOver == true
isSrcOver != true
~~~~

</fiddle-embed></div>

<a name='Path_Effect_Methods'></a>

---

<a href='#Path_Effect'>Path_Effect</a> <a href='#Path_Effect'>modifies</a> <a href='#Path_Effect'>the</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>geometry</a> <a href='SkPath_Reference#Path'>before</a> <a href='SkPath_Reference#Path'>drawing</a> <a href='SkPath_Reference#Path'>it</a>.
<a href='#Path_Effect'>Path_Effect</a> <a href='#Path_Effect'>may</a> <a href='#Path_Effect'>implement</a> <a href='#Path_Effect'>dashing</a>, <a href='#Path_Effect'>custom</a> <a href='#Path_Effect'>fill</a> <a href='#Path_Effect'>effects</a> <a href='#Path_Effect'>and</a> <a href='#Path_Effect'>custom</a> <a href='#Path_Effect'>stroke</a> <a href='#Path_Effect'>effects</a>.
<a href='#Path_Effect'>If</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>has</a> <a href='SkPaint_Reference#Paint'>no</a> <a href='#Path_Effect'>Path_Effect</a>, <a href='#Path_Effect'>the</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>geometry</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>unaltered</a> <a href='SkPath_Reference#Path'>when</a> <a href='SkPath_Reference#Path'>filled</a> <a href='SkPath_Reference#Path'>or</a> <a href='SkPath_Reference#Path'>stroked</a>.

### Example

<div><fiddle-embed name="8cf5684b187d60f09e11c4a48993ea39"></fiddle-embed></div>

### See Also

<a href='#Path_Effect'>Path_Effect</a>

<a name='SkPaint_getPathEffect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkPathEffect'>SkPathEffect</a>* <a href='#SkPaint_getPathEffect'>getPathEffect</a>() <a href='#SkPaint_getPathEffect'>const</a>
</pre>

Returns <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>if</a> <a href='undocumented#SkPathEffect'>set</a>, <a href='undocumented#SkPathEffect'>or</a> <a href='undocumented#SkPathEffect'>nullptr</a>.
Does not alter <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>if</a> <a href='undocumented#SkPathEffect'>previously</a> <a href='undocumented#SkPathEffect'>set</a>, <a href='undocumented#SkPathEffect'>nullptr</a> <a href='undocumented#SkPathEffect'>otherwise</a>

### Example

<div><fiddle-embed name="211a1b14bfa6c4332082c8eab4fbc5fd">

#### Example Output

~~~~
nullptr == path effect
nullptr != path effect
~~~~

</fiddle-embed></div>

<a name='SkPaint_refPathEffect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkPathEffect'>SkPathEffect</a>&<a href='undocumented#SkPathEffect'>gt</a>; <a href='#SkPaint_refPathEffect'>refPathEffect</a>() <a href='#SkPaint_refPathEffect'>const</a>
</pre>

Returns <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>if</a> <a href='undocumented#SkPathEffect'>set</a>, <a href='undocumented#SkPathEffect'>or</a> <a href='undocumented#SkPathEffect'>nullptr</a>.
Increases <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Return Value

<a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>if</a> <a href='undocumented#SkPathEffect'>previously</a> <a href='undocumented#SkPathEffect'>set</a>, <a href='undocumented#SkPathEffect'>nullptr</a> <a href='undocumented#SkPathEffect'>otherwise</a>

### Example

<div><fiddle-embed name="f56039b94c702c2704c8c5100e623aca">

#### Example Output

~~~~
path effect unique: true
path effect unique: false
~~~~

</fiddle-embed></div>

<a name='SkPaint_setPathEffect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setPathEffect'>setPathEffect</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkPathEffect'>SkPathEffect</a>&<a href='undocumented#SkPathEffect'>gt</a>; <a href='undocumented#SkPathEffect'>pathEffect</a>)
</pre>

Sets <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>to</a> <a href='#SkPaint_setPathEffect_pathEffect'>pathEffect</a>, <a href='#SkPaint_setPathEffect_pathEffect'>decreasing</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>of</a> <a href='undocumented#SkRefCnt'>the</a> <a href='undocumented#SkRefCnt'>previous</a>
<a href='undocumented#SkPathEffect'>SkPathEffect</a>. <a href='undocumented#SkPathEffect'>Pass</a> <a href='undocumented#SkPathEffect'>nullptr</a> <a href='undocumented#SkPathEffect'>to</a> <a href='undocumented#SkPathEffect'>leave</a> <a href='undocumented#SkPathEffect'>the</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>geometry</a> <a href='SkPath_Reference#Path'>unaltered</a>.

Increments <a href='#SkPaint_setPathEffect_pathEffect'>pathEffect</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setPathEffect_pathEffect'><code><strong>pathEffect</strong></code></a></td>
    <td>replace <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>with</a> <a href='SkPath_Reference#SkPath'>a</a> <a href='SkPath_Reference#SkPath'>modification</a> <a href='SkPath_Reference#SkPath'>when</a> <a href='SkPath_Reference#SkPath'>drawn</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="52dd55074ca0b7d520d04e750ca2a0d7"></fiddle-embed></div>

<a name='Mask_Filter_Methods'></a>

---

<a href='#Mask_Filter'>Mask_Filter</a> <a href='#Mask_Filter'>uses</a> <a href='#Mask_Filter'>coverage</a> <a href='#Mask_Filter'>of</a> <a href='#Mask_Filter'>the</a> <a href='#Mask_Filter'>shape</a> <a href='#Mask_Filter'>drawn</a> <a href='#Mask_Filter'>to</a> <a href='#Mask_Filter'>create</a> <a href='#Mask_Alpha'>Mask_Alpha</a>.
<a href='#Mask_Filter'>Mask_Filter</a> <a href='#Mask_Filter'>takes</a> <a href='#Mask_Filter'>a</a> <a href='#Mask_Filter'>Mask</a>, <a href='#Mask_Filter'>and</a> <a href='#Mask_Filter'>returns</a> <a href='#Mask_Filter'>a</a> <a href='#Mask_Filter'>Mask</a>.

<a href='#Mask_Filter'>Mask_Filter</a> <a href='#Mask_Filter'>may</a> <a href='#Mask_Filter'>change</a> <a href='#Mask_Filter'>the</a> <a href='#Mask_Filter'>geometry</a> <a href='#Mask_Filter'>and</a> <a href='#Mask_Filter'>transparency</a> <a href='#Mask_Filter'>of</a> <a href='#Mask_Filter'>the</a> <a href='#Mask_Filter'>shape</a>, <a href='#Mask_Filter'>such</a> <a href='#Mask_Filter'>as</a>
<a href='#Mask_Filter'>creating</a> <a href='#Mask_Filter'>a</a> <a href='#Mask_Filter'>blur</a> <a href='#Mask_Filter'>effect</a>. <a href='#Mask_Filter'>Set</a> <a href='#Mask_Filter'>Mask_Filter</a> <a href='#Mask_Filter'>to</a> <a href='#Mask_Filter'>nullptr</a> <a href='#Mask_Filter'>to</a> <a href='#Mask_Filter'>prevent</a> <a href='#Mask_Filter'>Mask_Filter</a> <a href='#Mask_Filter'>from</a>
<a href='#Mask_Filter'>modifying</a> <a href='#Mask_Filter'>the</a> <a href='#Mask_Filter'>draw</a>.

### Example

<div><fiddle-embed name="55d7b9d482ac8e17a6153f555a8adb8d"></fiddle-embed></div>

<a name='SkPaint_getMaskFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>* <a href='#SkPaint_getMaskFilter'>getMaskFilter</a>() <a href='#SkPaint_getMaskFilter'>const</a>
</pre>

Returns <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>if</a> <a href='undocumented#SkMaskFilter'>set</a>, <a href='undocumented#SkMaskFilter'>or</a> <a href='undocumented#SkMaskFilter'>nullptr</a>.
Does not alter <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>if</a> <a href='undocumented#SkMaskFilter'>previously</a> <a href='undocumented#SkMaskFilter'>set</a>, <a href='undocumented#SkMaskFilter'>nullptr</a> <a href='undocumented#SkMaskFilter'>otherwise</a>

### Example

<div><fiddle-embed name="5ac4b31371726da87bb7390b385e9fee">

#### Example Output

~~~~
nullptr == mask filter
nullptr != mask filter
~~~~

</fiddle-embed></div>

<a name='SkPaint_refMaskFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>&<a href='undocumented#SkMaskFilter'>gt</a>; <a href='#SkPaint_refMaskFilter'>refMaskFilter</a>() <a href='#SkPaint_refMaskFilter'>const</a>
</pre>

Returns <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>if</a> <a href='undocumented#SkMaskFilter'>set</a>, <a href='undocumented#SkMaskFilter'>or</a> <a href='undocumented#SkMaskFilter'>nullptr</a>.

Increases <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Return Value

<a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>if</a> <a href='undocumented#SkMaskFilter'>previously</a> <a href='undocumented#SkMaskFilter'>set</a>, <a href='undocumented#SkMaskFilter'>nullptr</a> <a href='undocumented#SkMaskFilter'>otherwise</a>

### Example

<div><fiddle-embed name="084b0dc3cebd78718c651d58f257f799">

#### Example Output

~~~~
mask filter unique: true
mask filter unique: false
~~~~

</fiddle-embed></div>

<a name='SkPaint_setMaskFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setMaskFilter'>setMaskFilter</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>&<a href='undocumented#SkMaskFilter'>gt</a>; <a href='undocumented#SkMaskFilter'>maskFilter</a>)
</pre>

Sets <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>to</a> <a href='#SkPaint_setMaskFilter_maskFilter'>maskFilter</a>, <a href='#SkPaint_setMaskFilter_maskFilter'>decreasing</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>of</a> <a href='undocumented#SkRefCnt'>the</a> <a href='undocumented#SkRefCnt'>previous</a>
<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>. <a href='undocumented#SkMaskFilter'>Pass</a> <a href='undocumented#SkMaskFilter'>nullptr</a> <a href='undocumented#SkMaskFilter'>to</a> <a href='undocumented#SkMaskFilter'>clear</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>and</a> <a href='undocumented#SkMaskFilter'>leave</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>effect</a> <a href='undocumented#SkMaskFilter'>on</a>
<a href='undocumented#Mask_Alpha'>mask alpha</a> unaltered.

Increments <a href='#SkPaint_setMaskFilter_maskFilter'>maskFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setMaskFilter_maskFilter'><code><strong>maskFilter</strong></code></a></td>
    <td>modifies clipping mask generated from drawn geometry</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a993831c40f3e134f809134e3b74e4a6"></fiddle-embed></div>

<a name='Typeface_Methods'></a>

---

<a href='undocumented#Typeface'>Typeface</a> <a href='undocumented#Typeface'>identifies</a> <a href='undocumented#Typeface'>the</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>used</a> <a href='undocumented#Font'>when</a> <a href='undocumented#Font'>drawing</a> <a href='undocumented#Font'>and</a> <a href='undocumented#Font'>measuring</a> <a href='undocumented#Text'>text</a>.
<a href='undocumented#Typeface'>Typeface</a> <a href='undocumented#Typeface'>may</a> <a href='undocumented#Typeface'>be</a> <a href='undocumented#Typeface'>specified</a> <a href='undocumented#Typeface'>by</a> <a href='undocumented#Typeface'>name</a>, <a href='undocumented#Typeface'>from</a> <a href='undocumented#Typeface'>a</a> <a href='undocumented#Typeface'>file</a>, <a href='undocumented#Typeface'>or</a> <a href='undocumented#Typeface'>from</a> <a href='undocumented#Typeface'>a</a> <a href='undocumented#Data'>data</a> <a href='SkStream_Reference#Stream'>stream</a>.
<a href='SkStream_Reference#Stream'>The</a> <a href='SkStream_Reference#Stream'>default</a> <a href='undocumented#Typeface'>Typeface</a> <a href='undocumented#Typeface'>defers</a> <a href='undocumented#Typeface'>to</a> <a href='undocumented#Typeface'>the</a> <a href='undocumented#Typeface'>platform-specific</a> <a href='undocumented#Typeface'>default</a> <a href='undocumented#Font'>font</a>
<a href='undocumented#Font'>implementation</a>.

### Example

<div><fiddle-embed name="1a7a5062725139760962582f599f1b97"></fiddle-embed></div>

<a name='SkPaint_getTypeface'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkTypeface'>SkTypeface</a>* <a href='#SkPaint_getTypeface'>getTypeface</a>() <a href='#SkPaint_getTypeface'>const</a>
</pre>

Returns <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>if</a> <a href='undocumented#SkTypeface'>set</a>, <a href='undocumented#SkTypeface'>or</a> <a href='undocumented#SkTypeface'>nullptr</a>.
Does not alter <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>if</a> <a href='undocumented#SkTypeface'>previously</a> <a href='undocumented#SkTypeface'>set</a>, <a href='undocumented#SkTypeface'>nullptr</a> <a href='undocumented#SkTypeface'>otherwise</a>

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
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkTypeface'>SkTypeface</a>&<a href='undocumented#SkTypeface'>gt</a>; <a href='#SkPaint_refTypeface'>refTypeface</a>() <a href='#SkPaint_refTypeface'>const</a>
</pre>

Increases <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Return Value

<a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>if</a> <a href='undocumented#SkTypeface'>previously</a> <a href='undocumented#SkTypeface'>set</a>, <a href='undocumented#SkTypeface'>nullptr</a> <a href='undocumented#SkTypeface'>otherwise</a>

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
void <a href='#SkPaint_setTypeface'>setTypeface</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkTypeface'>SkTypeface</a>&<a href='undocumented#SkTypeface'>gt</a>; <a href='undocumented#Typeface'>typeface</a>)
</pre>

Sets <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>to</a> <a href='#SkPaint_setTypeface_typeface'>typeface</a>, <a href='#SkPaint_setTypeface_typeface'>decreasing</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>of</a> <a href='undocumented#SkRefCnt'>the</a> <a href='undocumented#SkRefCnt'>previous</a> <a href='undocumented#SkTypeface'>SkTypeface</a>.
Pass nullptr to clear <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>and</a> <a href='undocumented#SkTypeface'>use</a> <a href='undocumented#SkTypeface'>the</a> <a href='undocumented#SkTypeface'>default</a> <a href='#SkPaint_setTypeface_typeface'>typeface</a>. <a href='#SkPaint_setTypeface_typeface'>Increments</a>
<a href='#SkPaint_setTypeface_typeface'>typeface</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTypeface_typeface'><code><strong>typeface</strong></code></a></td>
    <td><a href='undocumented#Font'>font</a> <a href='undocumented#Font'>and</a> <a href='undocumented#Font'>style</a> <a href='undocumented#Font'>used</a> <a href='undocumented#Font'>to</a> <a href='undocumented#Font'>draw</a> <a href='undocumented#Text'>text</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0e6fbb7773cd925b274552f4cd1abef2"></fiddle-embed></div>

<a name='Image_Filter_Methods'></a>

---

<a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>operates</a> <a href='#Image_Filter'>on</a> <a href='#Image_Filter'>the</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>representation</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>shape</a>, <a href='undocumented#Pixel'>as</a> <a href='undocumented#Pixel'>modified</a> <a href='undocumented#Pixel'>by</a> <a href='SkPaint_Reference#Paint'>Paint</a>
<a href='SkPaint_Reference#Paint'>with</a> <a href='#Blend_Mode'>Blend_Mode</a> <a href='#Blend_Mode'>set</a> <a href='#Blend_Mode'>to</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>. <a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>creates</a> <a href='#Image_Filter'>a</a> <a href='#Image_Filter'>new</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a>,
<a href='SkBitmap_Reference#Bitmap'>which</a> <a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>drawn</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>using</a> <a href='undocumented#Device'>the</a> <a href='undocumented#Device'>set</a> <a href='#Blend_Mode'>Blend_Mode</a>.

<a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>is</a> <a href='#Image_Filter'>higher</a> <a href='#Image_Filter'>level</a> <a href='#Image_Filter'>than</a> <a href='#Mask_Filter'>Mask_Filter</a>; <a href='#Mask_Filter'>for</a> <a href='#Mask_Filter'>instance</a>, <a href='#Mask_Filter'>an</a> <a href='#Image_Filter'>Image_Filter</a>
<a href='#Image_Filter'>can</a> <a href='#Image_Filter'>operate</a> <a href='#Image_Filter'>on</a> <a href='#Image_Filter'>all</a> <a href='#Image_Filter'>channels</a> <a href='#Image_Filter'>of</a> <a href='SkColor_Reference#Color'>Color</a>, <a href='SkColor_Reference#Color'>while</a> <a href='#Mask_Filter'>Mask_Filter</a> <a href='#Mask_Filter'>generates</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>only</a>.
<a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>operates</a> <a href='#Image_Filter'>independently</a> <a href='#Image_Filter'>of</a> <a href='#Image_Filter'>and</a> <a href='#Image_Filter'>can</a> <a href='#Image_Filter'>be</a> <a href='#Image_Filter'>used</a> <a href='#Image_Filter'>in</a> <a href='#Image_Filter'>combination</a> <a href='#Image_Filter'>with</a>
<a href='#Mask_Filter'>Mask_Filter</a>.

### Example

<div><fiddle-embed name="ece04ee3d3761e3425f37c8f06f054c1"></fiddle-embed></div>

<a name='SkPaint_getImageFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='#SkPaint_getImageFilter'>getImageFilter</a>() <a href='#SkPaint_getImageFilter'>const</a>
</pre>

Returns <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkImageFilter'>if</a> <a href='undocumented#SkImageFilter'>set</a>, <a href='undocumented#SkImageFilter'>or</a> <a href='undocumented#SkImageFilter'>nullptr</a>.
Does not alter <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkImageFilter'>if</a> <a href='undocumented#SkImageFilter'>previously</a> <a href='undocumented#SkImageFilter'>set</a>, <a href='undocumented#SkImageFilter'>nullptr</a> <a href='undocumented#SkImageFilter'>otherwise</a>

### Example

<div><fiddle-embed name="c11f8eaa1dd149bc18db21e23ce26904">

#### Example Output

~~~~
nullptr == image filter
nullptr != image filter
~~~~

</fiddle-embed></div>

<a name='SkPaint_refImageFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkImageFilter'>SkImageFilter</a>&<a href='undocumented#SkImageFilter'>gt</a>; <a href='#SkPaint_refImageFilter'>refImageFilter</a>() <a href='#SkPaint_refImageFilter'>const</a>
</pre>

Returns <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkImageFilter'>if</a> <a href='undocumented#SkImageFilter'>set</a>, <a href='undocumented#SkImageFilter'>or</a> <a href='undocumented#SkImageFilter'>nullptr</a>.
Increases <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Return Value

<a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkImageFilter'>if</a> <a href='undocumented#SkImageFilter'>previously</a> <a href='undocumented#SkImageFilter'>set</a>, <a href='undocumented#SkImageFilter'>nullptr</a> <a href='undocumented#SkImageFilter'>otherwise</a>

### Example

<div><fiddle-embed name="13f09088b569251547107d14ae989dc1">

#### Example Output

~~~~
image filter unique: true
image filter unique: false
~~~~

</fiddle-embed></div>

<a name='SkPaint_setImageFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setImageFilter'>setImageFilter</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkImageFilter'>SkImageFilter</a>&<a href='undocumented#SkImageFilter'>gt</a>; <a href='undocumented#SkImageFilter'>imageFilter</a>)
</pre>

Sets <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkImageFilter'>to</a> <a href='#SkPaint_setImageFilter_imageFilter'>imageFilter</a>, <a href='#SkPaint_setImageFilter_imageFilter'>decreasing</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>of</a> <a href='undocumented#SkRefCnt'>the</a> <a href='undocumented#SkRefCnt'>previous</a>
<a href='undocumented#SkImageFilter'>SkImageFilter</a>. <a href='undocumented#SkImageFilter'>Pass</a> <a href='undocumented#SkImageFilter'>nullptr</a> <a href='undocumented#SkImageFilter'>to</a> <a href='undocumented#SkImageFilter'>clear</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='undocumented#SkImageFilter'>remove</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkImageFilter'>effect</a>
on drawing.

Increments <a href='#SkPaint_setImageFilter_imageFilter'>imageFilter</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setImageFilter_imageFilter'><code><strong>imageFilter</strong></code></a></td>
    <td>how <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>sampled</a> <a href='SkImage_Reference#SkImage'>when</a> <a href='SkImage_Reference#SkImage'>transformed</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6679d6e4ec632715ee03e68391bd7f9a"></fiddle-embed></div>

<a name='Draw_Looper_Methods'></a>

---

<a href='#Draw_Looper'>Draw_Looper</a> <a href='#Draw_Looper'>sets</a> <a href='#Draw_Looper'>a</a> <a href='#Draw_Looper'>modifier</a> <a href='#Draw_Looper'>that</a> <a href='#Draw_Looper'>communicates</a> <a href='#Draw_Looper'>state</a> <a href='#Draw_Looper'>from</a> <a href='#Draw_Looper'>one</a> <a href='#Draw_Layer'>Draw_Layer</a>
<a href='#Draw_Layer'>to</a> <a href='#Draw_Layer'>another</a> <a href='#Draw_Layer'>to</a> <a href='#Draw_Layer'>construct</a> <a href='#Draw_Layer'>the</a> <a href='#Draw_Layer'>draw</a>.

<a href='#Draw_Looper'>Draw_Looper</a> <a href='#Draw_Looper'>draws</a> <a href='#Draw_Looper'>one</a> <a href='#Draw_Looper'>or</a> <a href='#Draw_Looper'>more</a> <a href='#Draw_Looper'>times</a>, <a href='#Draw_Looper'>modifying</a> <a href='#Draw_Looper'>the</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>and</a> <a href='SkPaint_Reference#Paint'>paint</a> <a href='SkPaint_Reference#Paint'>each</a> <a href='SkPaint_Reference#Paint'>time</a>.
<a href='#Draw_Looper'>Draw_Looper</a> <a href='#Draw_Looper'>may</a> <a href='#Draw_Looper'>be</a> <a href='#Draw_Looper'>used</a> <a href='#Draw_Looper'>to</a> <a href='#Draw_Looper'>draw</a> <a href='#Draw_Looper'>multiple</a> <a href='#Draw_Looper'>colors</a> <a href='#Draw_Looper'>or</a> <a href='#Draw_Looper'>create</a> <a href='#Draw_Looper'>a</a> <a href='#Draw_Looper'>colored</a> <a href='#Draw_Looper'>shadow</a>.
<a href='#Draw_Looper'>Set</a> <a href='#Draw_Looper'>Draw_Looper</a> <a href='#Draw_Looper'>to</a> <a href='#Draw_Looper'>nullptr</a> <a href='#Draw_Looper'>to</a> <a href='#Draw_Looper'>prevent</a> <a href='#Draw_Looper'>Draw_Looper</a> <a href='#Draw_Looper'>from</a> <a href='#Draw_Looper'>modifying</a> <a href='#Draw_Looper'>the</a> <a href='#Draw_Looper'>draw</a>.

### Example

<div><fiddle-embed name="84ec12a36e50df5ac565cc7a75ffbe9f"></fiddle-embed></div>

<a name='SkPaint_getDrawLooper'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>* <a href='#SkPaint_getDrawLooper'>getDrawLooper</a>() <a href='#SkPaint_getDrawLooper'>const</a>
</pre>

Returns <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> <a href='undocumented#SkDrawLooper'>if</a> <a href='undocumented#SkDrawLooper'>set</a>, <a href='undocumented#SkDrawLooper'>or</a> <a href='undocumented#SkDrawLooper'>nullptr</a>.
Does not alter <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkDrawLooper'>SkDrawLooper</a> <a href='undocumented#SkDrawLooper'>if</a> <a href='undocumented#SkDrawLooper'>previously</a> <a href='undocumented#SkDrawLooper'>set</a>, <a href='undocumented#SkDrawLooper'>nullptr</a> <a href='undocumented#SkDrawLooper'>otherwise</a>

### Example

<div><fiddle-embed name="af4c5acc7a91e7f23c2af48018903ad4">

#### Example Output

~~~~
nullptr == draw looper
nullptr != draw looper
~~~~

</fiddle-embed></div>

<a name='SkPaint_refDrawLooper'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>&<a href='undocumented#SkDrawLooper'>gt</a>; <a href='#SkPaint_refDrawLooper'>refDrawLooper</a>() <a href='#SkPaint_refDrawLooper'>const</a>
</pre>

Returns <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> <a href='undocumented#SkDrawLooper'>if</a> <a href='undocumented#SkDrawLooper'>set</a>, <a href='undocumented#SkDrawLooper'>or</a> <a href='undocumented#SkDrawLooper'>nullptr</a>.
Increases <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Return Value

<a href='undocumented#SkDrawLooper'>SkDrawLooper</a> <a href='undocumented#SkDrawLooper'>if</a> <a href='undocumented#SkDrawLooper'>previously</a> <a href='undocumented#SkDrawLooper'>set</a>, <a href='undocumented#SkDrawLooper'>nullptr</a> <a href='undocumented#SkDrawLooper'>otherwise</a>

### Example

<div><fiddle-embed name="2a3782c33f04ed17a725d0e449c6f7c3">

#### Example Output

~~~~
draw looper unique: true
draw looper unique: false
~~~~

</fiddle-embed></div>

<a name='SkPaint_getLooper'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>* <a href='#SkPaint_getLooper'>getLooper</a>() <a href='#SkPaint_getLooper'>const</a>
</pre>

Deprecated.

<a name='SkPaint_setDrawLooper'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void setDrawLooper(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>&<a href='undocumented#SkDrawLooper'>gt</a>; <a href='undocumented#SkDrawLooper'>drawLooper</a>)
</pre>

Sets <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> <a href='undocumented#SkDrawLooper'>to</a> <a href='#SkPaint_setDrawLooper_drawLooper'>drawLooper</a>, <a href='#SkPaint_setDrawLooper_drawLooper'>decreasing</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>of</a> <a href='undocumented#SkRefCnt'>the</a> <a href='undocumented#SkRefCnt'>previous</a>
<a href='#SkPaint_setDrawLooper_drawLooper'>drawLooper</a>.  <a href='#SkPaint_setDrawLooper_drawLooper'>Pass</a> <a href='#SkPaint_setDrawLooper_drawLooper'>nullptr</a> <a href='#SkPaint_setDrawLooper_drawLooper'>to</a> <a href='#SkPaint_setDrawLooper_drawLooper'>clear</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> <a href='undocumented#SkDrawLooper'>and</a> <a href='undocumented#SkDrawLooper'>leave</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a> <a href='undocumented#SkDrawLooper'>effect</a> <a href='undocumented#SkDrawLooper'>on</a>
drawing unaltered.

Increments <a href='#SkPaint_setDrawLooper_drawLooper'>drawLooper</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> <a href='undocumented#SkRefCnt'>by</a> <a href='undocumented#SkRefCnt'>one</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setDrawLooper_drawLooper'><code><strong>drawLooper</strong></code></a></td>
    <td>iterates through drawing one or more time, altering <a href='SkPaint_Reference#SkPaint'>SkPaint</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bf10f838b330f0a3a3266d42ea68a638"></fiddle-embed></div>

<a name='SkPaint_setLooper'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setLooper'>setLooper</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>&<a href='undocumented#SkDrawLooper'>gt</a>; <a href='undocumented#SkDrawLooper'>drawLooper</a>)
</pre>

Deprecated.

<a name='Text_Size'></a>

---

<a href='#Paint_Text_Size'>Text_Size</a> <a href='#Paint_Text_Size'>adjusts</a> <a href='#Paint_Text_Size'>the</a> <a href='#Paint_Text_Size'>overall</a>  <a href='#Text_Size'>text size</a> <a href='undocumented#Text'>in</a> <a href='SkPoint_Reference#Point'>points</a>.
<a href='#Paint_Text_Size'>Text_Size</a> <a href='#Paint_Text_Size'>can</a> <a href='#Paint_Text_Size'>be</a> <a href='#Paint_Text_Size'>set</a> <a href='#Paint_Text_Size'>to</a> <a href='#Paint_Text_Size'>any</a> <a href='#Paint_Text_Size'>positive</a> <a href='#Paint_Text_Size'>value</a> <a href='#Paint_Text_Size'>or</a> <a href='#Paint_Text_Size'>zero</a>.
<a href='#Paint_Text_Size'>Text_Size</a> <a href='#Paint_Text_Size'>defaults</a> <a href='#Paint_Text_Size'>to</a> 12.
<a href='#Paint_Text_Size'>Set</a> <a href='undocumented#SkPaintDefaults_TextSize'>SkPaintDefaults_TextSize</a> <a href='undocumented#SkPaintDefaults_TextSize'>at</a> <a href='undocumented#SkPaintDefaults_TextSize'>compile</a> <a href='undocumented#SkPaintDefaults_TextSize'>time</a> <a href='undocumented#SkPaintDefaults_TextSize'>to</a> <a href='undocumented#SkPaintDefaults_TextSize'>change</a> <a href='undocumented#SkPaintDefaults_TextSize'>the</a> <a href='undocumented#SkPaintDefaults_TextSize'>default</a> <a href='undocumented#SkPaintDefaults_TextSize'>setting</a>.

### Example

<div><fiddle-embed name="91c9a3e498bb9412e4522a95d076ed5f"></fiddle-embed></div>

<a name='SkPaint_getTextSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSize'>getTextSize</a>() <a href='#SkPaint_getTextSize'>const</a>
</pre>

Returns <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>in</a> <a href='SkPoint_Reference#Point'>points</a>.

### Return Value

typographic height of <a href='undocumented#Text'>text</a>

### Example

<div><fiddle-embed name="983e2a71ba72d4ba8c945420040b8f1c"></fiddle-embed></div>

<a name='SkPaint_setTextSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextSize'>setTextSize</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>textSize</a>)
</pre>

Sets  <a href='#Text_Size'>text size</a> <a href='undocumented#Text'>in</a> <a href='SkPoint_Reference#Point'>points</a>.
Has no effect if <a href='#SkPaint_setTextSize_textSize'>textSize</a> <a href='#SkPaint_setTextSize_textSize'>is</a> <a href='#SkPaint_setTextSize_textSize'>not</a> <a href='#SkPaint_setTextSize_textSize'>greater</a> <a href='#SkPaint_setTextSize_textSize'>than</a> <a href='#SkPaint_setTextSize_textSize'>or</a> <a href='#SkPaint_setTextSize_textSize'>equal</a> <a href='#SkPaint_setTextSize_textSize'>to</a> <a href='#SkPaint_setTextSize_textSize'>zero</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextSize_textSize'><code><strong>textSize</strong></code></a></td>
    <td>typographic height of <a href='undocumented#Text'>text</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6510c9e2f57b83c47e67829e7a68d493"></fiddle-embed></div>

<a name='Text_Scale_X'></a>

---

<a href='#Paint_Text_Scale_X'>Text_Scale_X</a> <a href='#Paint_Text_Scale_X'>adjusts</a> <a href='#Paint_Text_Scale_X'>the</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>horizontal</a> <a href='undocumented#Text'>scale</a>.
<a href='undocumented#Text'>Text</a> <a href='undocumented#Text'>scaling</a> <a href='undocumented#Text'>approximates</a> <a href='undocumented#Text'>condensed</a> <a href='undocumented#Text'>and</a> <a href='undocumented#Text'>expanded</a> <a href='undocumented#Text'>type</a> <a href='undocumented#Text'>faces</a> <a href='undocumented#Text'>when</a> <a href='undocumented#Text'>the</a> <a href='undocumented#Text'>actual</a> <a href='undocumented#Text'>face</a>
<a href='undocumented#Text'>is</a> <a href='undocumented#Text'>not</a> <a href='undocumented#Text'>available</a>.
<a href='#Paint_Text_Scale_X'>Text_Scale_X</a> <a href='#Paint_Text_Scale_X'>can</a> <a href='#Paint_Text_Scale_X'>be</a> <a href='#Paint_Text_Scale_X'>set</a> <a href='#Paint_Text_Scale_X'>to</a> <a href='#Paint_Text_Scale_X'>any</a> <a href='#Paint_Text_Scale_X'>value</a>.
<a href='#Paint_Text_Scale_X'>Text_Scale_X</a> <a href='#Paint_Text_Scale_X'>defaults</a> <a href='#Paint_Text_Scale_X'>to</a> 1.

### Example

<div><fiddle-embed name="d13d787c1e36f515319fc998411c1d91"></fiddle-embed></div>

<a name='SkPaint_getTextScaleX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextScaleX'>getTextScaleX</a>() <a href='#SkPaint_getTextScaleX'>const</a>
</pre>

Returns <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>scale</a> <a href='undocumented#Text'>on</a> <a href='undocumented#Text'>x-axis</a>.
Default value is 1.

### Return Value

<a href='undocumented#Text'>text</a> <a href='undocumented#Text'>horizontal</a> <a href='undocumented#Text'>scale</a>

### Example

<div><fiddle-embed name="5dc8e58f6910cb8e4de9ed60f888188b"></fiddle-embed></div>

<a name='SkPaint_setTextScaleX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextScaleX'>setTextScaleX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scaleX</a>)
</pre>

Sets  <a href='undocumented#Text'>text scale</a> <a href='undocumented#Text'>on</a> <a href='undocumented#Text'>x-axis</a>.
Default value is 1.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextScaleX_scaleX'><code><strong>scaleX</strong></code></a></td>
    <td><a href='undocumented#Text'>text</a> <a href='undocumented#Text'>horizontal</a> <a href='undocumented#Text'>scale</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a75bbdb8bb866b125c4c1dd5e967d470"></fiddle-embed></div>

<a name='Text_Skew_X'></a>

---

<a href='#Paint_Text_Skew_X'>Text_Skew_X</a> <a href='#Paint_Text_Skew_X'>adjusts</a> <a href='#Paint_Text_Skew_X'>the</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>horizontal</a> <a href='undocumented#Text'>slant</a>.
<a href='undocumented#Text'>Text</a> <a href='undocumented#Text'>skewing</a> <a href='undocumented#Text'>approximates</a> <a href='undocumented#Text'>italic</a> <a href='undocumented#Text'>and</a> <a href='undocumented#Text'>oblique</a> <a href='undocumented#Text'>type</a> <a href='undocumented#Text'>faces</a> <a href='undocumented#Text'>when</a> <a href='undocumented#Text'>the</a> <a href='undocumented#Text'>actual</a> <a href='undocumented#Text'>face</a>
<a href='undocumented#Text'>is</a> <a href='undocumented#Text'>not</a> <a href='undocumented#Text'>available</a>.
<a href='#Paint_Text_Skew_X'>Text_Skew_X</a> <a href='#Paint_Text_Skew_X'>can</a> <a href='#Paint_Text_Skew_X'>be</a> <a href='#Paint_Text_Skew_X'>set</a> <a href='#Paint_Text_Skew_X'>to</a> <a href='#Paint_Text_Skew_X'>any</a> <a href='#Paint_Text_Skew_X'>value</a>.
<a href='#Paint_Text_Skew_X'>Text_Skew_X</a> <a href='#Paint_Text_Skew_X'>defaults</a> <a href='#Paint_Text_Skew_X'>to</a> 0.

### Example

<div><fiddle-embed name="aff208b0aab265f273045b27e683c17c"></fiddle-embed></div>

<a name='SkPaint_getTextSkewX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSkewX'>getTextSkewX</a>() <a href='#SkPaint_getTextSkewX'>const</a>
</pre>

Returns <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>skew</a> <a href='undocumented#Text'>on</a> <a href='undocumented#Text'>x-axis</a>.
Default value is zero.

### Return Value

additional shear on x-axis relative to y-axis

### Example

<div><fiddle-embed name="11c10f466dae0d1639dbb9f6a0040218"></fiddle-embed></div>

<a name='SkPaint_setTextSkewX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextSkewX'>setTextSkewX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>skewX</a>)
</pre>

Sets  <a href='undocumented#Text'>text skew</a> <a href='undocumented#Text'>on</a> <a href='undocumented#Text'>x-axis</a>.
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

<a name='SkPaint_TextEncoding'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_TextEncoding'>TextEncoding</a> : <a href='#SkPaint_TextEncoding'>uint8_t</a> {
        <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>,
        <a href='#SkPaint_kUTF16_TextEncoding'>kUTF16_TextEncoding</a>,
        <a href='#SkPaint_kUTF32_TextEncoding'>kUTF32_TextEncoding</a>,
        <a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID_TextEncoding</a>,
    };
</pre>

<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>determines</a> <a href='#SkPaint_TextEncoding'>whether</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>specifies</a> <a href='undocumented#Text'>character</a> <a href='undocumented#Text'>codes</a> <a href='undocumented#Text'>and</a> <a href='undocumented#Text'>their</a> <a href='undocumented#Text'>encoded</a>
<a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>or</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a>. <a href='undocumented#Glyph'>Characters</a> <a href='undocumented#Glyph'>are</a> <a href='undocumented#Glyph'>encoded</a> <a href='undocumented#Glyph'>as</a> <a href='undocumented#Glyph'>specified</a> <a href='undocumented#Glyph'>by</a> <a href='undocumented#Glyph'>the</a>
<a href='https://unicode.org/standard/standard.html'>Unicode standard</a></a> .

Character codes encoded <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>are</a> <a href='undocumented#Size'>specified</a> <a href='undocumented#Size'>by</a> <a href='undocumented#Size'>UTF-8</a>, <a href='undocumented#Size'>UTF-16</a>, <a href='undocumented#Size'>or</a> <a href='undocumented#Size'>UTF-32</a>.
<a href='undocumented#Size'>All</a> <a href='undocumented#Size'>character</a> <a href='undocumented#Size'>code</a> <a href='undocumented#Size'>formats</a> <a href='undocumented#Size'>are</a> <a href='undocumented#Size'>able</a> <a href='undocumented#Size'>to</a> <a href='undocumented#Size'>represent</a> <a href='undocumented#Size'>all</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>Unicode</a>, <a href='undocumented#Size'>differing</a> <a href='undocumented#Size'>only</a>
<a href='undocumented#Size'>in</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>total</a> <a href='undocumented#Size'>storage</a> <a href='undocumented#Size'>required</a>.

<a href='https://tools.ietf.org/html/rfc3629'>UTF-8 (RFC 3629)</a></a> encodes each character as one or more 8-bit bytes.

<a href='https://tools.ietf.org/html/rfc2781'>UTF-16 (RFC 2781)</a></a> encodes each character as one or two 16-bit words.

<a href='https://www.unicode.org/versions/Unicode5.0.0/ch03.pdf'>UTF-32</a></a> encodes each character as one 32-bit word.

<a href='#Font_Manager'>Font_Manager</a> <a href='#Font_Manager'>uses</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>to</a> <a href='undocumented#Data'>convert</a> <a href='undocumented#Data'>character</a> <a href='undocumented#Data'>code</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>into</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a>.
<a href='undocumented#Glyph'>A</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>index</a> <a href='undocumented#Glyph'>is</a> <a href='undocumented#Glyph'>a</a> 16-<a href='undocumented#Glyph'>bit</a> <a href='undocumented#Glyph'>word</a>.

<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>is</a> <a href='#SkPaint_TextEncoding'>set</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a> <a href='#SkPaint_kUTF8_TextEncoding'>by</a> <a href='#SkPaint_kUTF8_TextEncoding'>default</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kUTF8_TextEncoding'><code>SkPaint::kUTF8_TextEncoding</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
uses bytes to represent UTF-8 or ASCII</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kUTF16_TextEncoding'><code>SkPaint::kUTF16_TextEncoding</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
uses two byte words to represent most of Unicode</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kUTF32_TextEncoding'><code>SkPaint::kUTF32_TextEncoding</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
uses four byte words to represent all of Unicode</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kGlyphID_TextEncoding'><code>SkPaint::kGlyphID_TextEncoding</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
uses two byte words to represent glyph indices</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="b29294e7f29d160a1b46abf2dcec9d2a"><div>First <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>is</a> <a href='undocumented#Line'>encoded</a> <a href='undocumented#Line'>in</a> <a href='undocumented#Line'>UTF-8</a>.
<a href='undocumented#Line'>Second</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>is</a> <a href='undocumented#Line'>encoded</a> <a href='undocumented#Line'>in</a> <a href='undocumented#Line'>UTF-16</a>.
<a href='undocumented#Line'>Third</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>is</a> <a href='undocumented#Line'>encoded</a> <a href='undocumented#Line'>in</a> <a href='undocumented#Line'>UTF-32</a>.
<a href='undocumented#Line'>Fourth</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>has</a> 16-<a href='undocumented#Line'>bit</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a>.
</div></fiddle-embed></div>

<a name='SkPaint_getTextEncoding'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_getTextEncoding'>getTextEncoding</a>() <a href='#SkPaint_getTextEncoding'>const</a>
</pre>

Returns <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a>.
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>determines</a> <a href='#SkPaint_TextEncoding'>how</a> <a href='#SkPaint_TextEncoding'>character</a> <a href='#SkPaint_TextEncoding'>code</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>mapped</a> <a href='SkPoint_Reference#Point'>to</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a>.

### Return Value

one of: <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>, <a href='#SkPaint_kUTF16_TextEncoding'>kUTF16_TextEncoding</a>, <a href='#SkPaint_kUTF32_TextEncoding'>kUTF32_TextEncoding</a>, <a href='#SkPaint_kUTF32_TextEncoding'>or</a>

<a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID_TextEncoding</a>

### Example

<div><fiddle-embed name="c6cc2780a9828b3af8c4621c12b29a1b">

#### Example Output

~~~~
kUTF8_TextEncoding == text encoding
kGlyphID_TextEncoding == text encoding
~~~~

</fiddle-embed></div>

<a name='SkPaint_setTextEncoding'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextEncoding'>setTextEncoding</a>(<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>encoding</a>)
</pre>

Sets <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_setTextEncoding_encoding'>encoding</a>.
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>determines</a> <a href='#SkPaint_TextEncoding'>how</a> <a href='#SkPaint_TextEncoding'>character</a> <a href='#SkPaint_TextEncoding'>code</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>mapped</a> <a href='SkPoint_Reference#Point'>to</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a>.
Invalid values for <a href='#SkPaint_setTextEncoding_encoding'>encoding</a> <a href='#SkPaint_setTextEncoding_encoding'>are</a> <a href='#SkPaint_setTextEncoding_encoding'>ignored</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextEncoding_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of: <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>, <a href='#SkPaint_kUTF16_TextEncoding'>kUTF16_TextEncoding</a>, <a href='#SkPaint_kUTF32_TextEncoding'>kUTF32_TextEncoding</a>, <a href='#SkPaint_kUTF32_TextEncoding'>or</a></td>
  </tr>
</table>

<a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID_TextEncoding</a>

### Example

<div><fiddle-embed name="6d9ffdd3c5543e9f12972a06dd4a0ce5">

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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getFontMetrics'>getFontMetrics</a>(<a href='undocumented#SkFontMetrics'>SkFontMetrics</a>* <a href='undocumented#SkFontMetrics'>metrics</a>) <a href='undocumented#SkFontMetrics'>const</a>
</pre>

Returns <a href='undocumented#SkFontMetrics'>SkFontMetrics</a> <a href='undocumented#SkFontMetrics'>associated</a> <a href='undocumented#SkFontMetrics'>with</a> <a href='undocumented#SkTypeface'>SkTypeface</a>.
The return value is the recommended spacing between <a href='undocumented#Line'>lines</a>: <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>sum</a> <a href='undocumented#Line'>of</a> <a href='#SkPaint_getFontMetrics_metrics'>metrics</a>
descent, ascent, and leading.
If <a href='#SkPaint_getFontMetrics_metrics'>metrics</a> <a href='#SkPaint_getFontMetrics_metrics'>is</a> <a href='#SkPaint_getFontMetrics_metrics'>not</a> <a href='#SkPaint_getFontMetrics_metrics'>nullptr</a>, <a href='undocumented#SkFontMetrics'>SkFontMetrics</a> <a href='undocumented#SkFontMetrics'>is</a> <a href='undocumented#SkFontMetrics'>copied</a> <a href='undocumented#SkFontMetrics'>to</a> <a href='#SkPaint_getFontMetrics_metrics'>metrics</a>.
Results are scaled by  <a href='#Text_Size'>text size</a> <a href='undocumented#Text'>but</a> <a href='undocumented#Text'>does</a> <a href='undocumented#Text'>not</a> <a href='undocumented#Text'>take</a> <a href='undocumented#Text'>into</a> <a href='undocumented#Text'>account</a>
dimensions required by   <a href='#Text_Scale_X'>text scale x</a>,   <a href='#Text_Skew_X'>text skew x</a>,  <a href='#Fake_Bold'>fake bold</a>,
<a href='#Style_Stroke'>style stroke</a>, and <a href='undocumented#SkPathEffect'>SkPathEffect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getFontMetrics_metrics'><code><strong>metrics</strong></code></a></td>
    <td>storage for <a href='undocumented#SkFontMetrics'>SkFontMetrics</a>; <a href='undocumented#SkFontMetrics'>may</a> <a href='undocumented#SkFontMetrics'>be</a> <a href='undocumented#SkFontMetrics'>nullptr</a></td>
  </tr>
</table>

### Return Value

recommended spacing between <a href='undocumented#Line'>lines</a>

### Example

<div><fiddle-embed name="59d9b8249afa1c2af6186711250ce240"></fiddle-embed></div>

### See Also

<a href='#Paint_Text_Size'>Text_Size</a> <a href='undocumented#Typeface'>Typeface</a> <a href='#Paint_Typeface_Methods'>Typeface_Methods</a>

<a name='SkPaint_getFontSpacing'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getFontSpacing'>getFontSpacing</a>() <a href='#SkPaint_getFontSpacing'>const</a>
</pre>

Returns the recommended spacing between <a href='undocumented#Line'>lines</a>: <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>sum</a> <a href='undocumented#Line'>of</a> <a href='undocumented#Line'>metrics</a>
descent, ascent, and leading.
Result is scaled by <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>but</a> <a href='undocumented#Size'>does</a> <a href='undocumented#Size'>not</a> <a href='undocumented#Size'>take</a> <a href='undocumented#Size'>into</a> <a href='undocumented#Size'>account</a>
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
int <a href='#SkPaint_textToGlyphs'>textToGlyphs</a>(<a href='#SkPaint_textToGlyphs'>const</a> <a href='#SkPaint_textToGlyphs'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>, <a href='undocumented#SkGlyphID'>SkGlyphID</a> <a href='undocumented#Glyph'>glyphs</a>[]) <a href='undocumented#Glyph'>const</a>
</pre>

Converts <a href='#SkPaint_textToGlyphs_text'>text</a> <a href='#SkPaint_textToGlyphs_text'>into</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a>.
Returns the number of <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a> <a href='undocumented#Glyph'>represented</a> <a href='undocumented#Glyph'>by</a> <a href='#SkPaint_textToGlyphs_text'>text</a>.
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>specifies</a> <a href='#SkPaint_TextEncoding'>how</a> <a href='#SkPaint_textToGlyphs_text'>text</a> <a href='#SkPaint_textToGlyphs_text'>represents</a> <a href='#SkPaint_textToGlyphs_text'>characters</a> <a href='#SkPaint_textToGlyphs_text'>or</a> <a href='#SkPaint_textToGlyphs_glyphs'>glyphs</a>.
<a href='#SkPaint_textToGlyphs_glyphs'>glyphs</a> <a href='#SkPaint_textToGlyphs_glyphs'>may</a> <a href='#SkPaint_textToGlyphs_glyphs'>be</a> <a href='#SkPaint_textToGlyphs_glyphs'>nullptr</a>, <a href='#SkPaint_textToGlyphs_glyphs'>to</a> <a href='#SkPaint_textToGlyphs_glyphs'>compute</a> <a href='#SkPaint_textToGlyphs_glyphs'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>count</a>.

Does not check <a href='#SkPaint_textToGlyphs_text'>text</a> <a href='#SkPaint_textToGlyphs_text'>for</a> <a href='#SkPaint_textToGlyphs_text'>valid</a> <a href='#SkPaint_textToGlyphs_text'>character</a> <a href='#SkPaint_textToGlyphs_text'>codes</a> <a href='#SkPaint_textToGlyphs_text'>or</a> <a href='#SkPaint_textToGlyphs_text'>valid</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a>.

If <a href='#SkPaint_textToGlyphs_byteLength'>byteLength</a> <a href='#SkPaint_textToGlyphs_byteLength'>equals</a> <a href='#SkPaint_textToGlyphs_byteLength'>zero</a>, <a href='#SkPaint_textToGlyphs_byteLength'>returns</a> <a href='#SkPaint_textToGlyphs_byteLength'>zero</a>.
If <a href='#SkPaint_textToGlyphs_byteLength'>byteLength</a> <a href='#SkPaint_textToGlyphs_byteLength'>includes</a> <a href='#SkPaint_textToGlyphs_byteLength'>a</a> <a href='#SkPaint_textToGlyphs_byteLength'>partial</a> <a href='#SkPaint_textToGlyphs_byteLength'>character</a>, <a href='#SkPaint_textToGlyphs_byteLength'>the</a> <a href='#SkPaint_textToGlyphs_byteLength'>partial</a> <a href='#SkPaint_textToGlyphs_byteLength'>character</a> <a href='#SkPaint_textToGlyphs_byteLength'>is</a> <a href='#SkPaint_textToGlyphs_byteLength'>ignored</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>is</a> <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a> <a href='#SkPaint_kUTF8_TextEncoding'>and</a>
<a href='#SkPaint_textToGlyphs_text'>text</a> <a href='#SkPaint_textToGlyphs_text'>contains</a> <a href='#SkPaint_textToGlyphs_text'>an</a> <a href='#SkPaint_textToGlyphs_text'>invalid</a> <a href='#SkPaint_textToGlyphs_text'>UTF-8</a> <a href='#SkPaint_textToGlyphs_text'>sequence</a>, <a href='#SkPaint_textToGlyphs_text'>zero</a> <a href='#SkPaint_textToGlyphs_text'>is</a> <a href='#SkPaint_textToGlyphs_text'>returned</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_textToGlyphs_text'><code><strong>text</strong></code></a></td>
    <td>character storage encoded with <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_textToGlyphs_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>length of character storage in bytes</td>
  </tr>
  <tr>    <td><a name='SkPaint_textToGlyphs_glyphs'><code><strong>glyphs</strong></code></a></td>
    <td>storage for <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a>; <a href='undocumented#Glyph'>may</a> <a href='undocumented#Glyph'>be</a> <a href='undocumented#Glyph'>nullptr</a></td>
  </tr>
</table>

### Return Value

number of <a href='#SkPaint_textToGlyphs_glyphs'>glyphs</a> <a href='#SkPaint_textToGlyphs_glyphs'>represented</a> <a href='#SkPaint_textToGlyphs_glyphs'>by</a> <a href='#SkPaint_textToGlyphs_text'>text</a> <a href='#SkPaint_textToGlyphs_text'>of</a> <a href='#SkPaint_textToGlyphs_text'>length</a> <a href='#SkPaint_textToGlyphs_byteLength'>byteLength</a>

### Example

<div><fiddle-embed name="343e9471a7f7b5f09abdc3b44983433b"></fiddle-embed></div>

<a name='SkPaint_countText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_countText'>countText</a>(<a href='#SkPaint_countText'>const</a> <a href='#SkPaint_countText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>) <a href='undocumented#Text'>const</a>
</pre>

Returns the number of <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>in</a> <a href='#SkPaint_countText_text'>text</a>.
Uses <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_TextEncoding'>count</a> <a href='#SkPaint_TextEncoding'>the</a> <a href='undocumented#Glyph'>glyphs</a>.
Returns the same result as <a href='#SkPaint_textToGlyphs'>textToGlyphs</a>().

### Parameters

<table>  <tr>    <td><a name='SkPaint_countText_text'><code><strong>text</strong></code></a></td>
    <td>character storage encoded with <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_countText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>length of character storage in bytes</td>
  </tr>
</table>

### Return Value

number of <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>represented</a> <a href='undocumented#Glyph'>by</a> <a href='#SkPaint_countText_text'>text</a> <a href='#SkPaint_countText_text'>of</a> <a href='#SkPaint_countText_text'>length</a> <a href='#SkPaint_countText_byteLength'>byteLength</a>

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
bool <a href='#SkPaint_containsText'>containsText</a>(<a href='#SkPaint_containsText'>const</a> <a href='#SkPaint_containsText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>) <a href='undocumented#Text'>const</a>
</pre>

Returns true if all <a href='#SkPaint_containsText_text'>text</a> <a href='#SkPaint_containsText_text'>corresponds</a> <a href='#SkPaint_containsText_text'>to</a> <a href='#SkPaint_containsText_text'>a</a> <a href='#SkPaint_containsText_text'>non-zero</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>index</a>.
Returns false if any characters in <a href='#SkPaint_containsText_text'>text</a> <a href='#SkPaint_containsText_text'>are</a> <a href='#SkPaint_containsText_text'>not</a> <a href='#SkPaint_containsText_text'>supported</a> <a href='#SkPaint_containsText_text'>in</a>
<a href='undocumented#SkTypeface'>SkTypeface</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>is</a> <a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID_TextEncoding</a>,
returns true if all <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a> <a href='undocumented#Glyph'>in</a> <a href='#SkPaint_containsText_text'>text</a> <a href='#SkPaint_containsText_text'>are</a> <a href='#SkPaint_containsText_text'>non-zero</a>;
does not check to see if <a href='#SkPaint_containsText_text'>text</a> <a href='#SkPaint_containsText_text'>contains</a> <a href='#SkPaint_containsText_text'>valid</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a> <a href='undocumented#Glyph'>for</a> <a href='undocumented#SkTypeface'>SkTypeface</a>.

Returns true if <a href='#SkPaint_containsText_byteLength'>byteLength</a> <a href='#SkPaint_containsText_byteLength'>is</a> <a href='#SkPaint_containsText_byteLength'>zero</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_containsText_text'><code><strong>text</strong></code></a></td>
    <td>array of characters or <a href='undocumented#Glyph'>glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_containsText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>number of bytes in <a href='#SkPaint_containsText_text'>text</a> <a href='#SkPaint_containsText_text'>array</a></td>
  </tr>
</table>

### Return Value

true if all <a href='#SkPaint_containsText_text'>text</a> <a href='#SkPaint_containsText_text'>corresponds</a> <a href='#SkPaint_containsText_text'>to</a> <a href='#SkPaint_containsText_text'>a</a> <a href='#SkPaint_containsText_text'>non-zero</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>index</a>

<div><a href='#SkPaint_containsText'>containsText</a> <a href='#SkPaint_containsText'>succeeds</a> <a href='#SkPaint_containsText'>for</a> <a href='#SkPaint_containsText'>degree</a> <a href='#SkPaint_containsText'>symbol</a>, <a href='#SkPaint_containsText'>but</a> <a href='#SkPaint_containsText'>cannot</a> <a href='#SkPaint_containsText'>find</a> <a href='#SkPaint_containsText'>a</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>index</a>
<a href='undocumented#Glyph'>corresponding</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>the</a> <a href='undocumented#Glyph'>Unicode</a> <a href='undocumented#Glyph'>surrogate</a> <a href='undocumented#Glyph'>code</a> <a href='SkPoint_Reference#Point'>point</a>.
</div>

#### Example Output

~~~~
0x00b0 == has char
0xd800 != has char
~~~~

### Example

<div><fiddle-embed name="083557b6f653d6fc00a34e01f87b74ff"><div><a href='#SkPaint_containsText'>containsText</a> <a href='#SkPaint_containsText'>returns</a> <a href='#SkPaint_containsText'>true</a> <a href='#SkPaint_containsText'>that</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>index</a> <a href='undocumented#Glyph'>is</a> <a href='undocumented#Glyph'>greater</a> <a href='undocumented#Glyph'>than</a> <a href='undocumented#Glyph'>zero</a>, <a href='undocumented#Glyph'>not</a>
<a href='undocumented#Glyph'>that</a> <a href='undocumented#Glyph'>it</a> <a href='undocumented#Glyph'>corresponds</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>an</a> <a href='undocumented#Glyph'>entry</a> <a href='undocumented#Glyph'>in</a> <a href='undocumented#Typeface'>Typeface</a>.
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
void <a href='#SkPaint_glyphsToUnichars'>glyphsToUnichars</a>(<a href='#SkPaint_glyphsToUnichars'>const</a> <a href='undocumented#SkGlyphID'>SkGlyphID</a> <a href='undocumented#Glyph'>glyphs</a>[], <a href='undocumented#Glyph'>int</a> <a href='undocumented#Glyph'>count</a>, <a href='undocumented#SkUnichar'>SkUnichar</a> <a href='undocumented#Text'>text</a>[]) <a href='undocumented#Text'>const</a>
</pre>

Converts <a href='#SkPaint_glyphsToUnichars_glyphs'>glyphs</a> <a href='#SkPaint_glyphsToUnichars_glyphs'>into</a> <a href='#SkPaint_glyphsToUnichars_text'>text</a> <a href='#SkPaint_glyphsToUnichars_text'>if</a> <a href='#SkPaint_glyphsToUnichars_text'>possible</a>.
<a href='undocumented#Glyph'>Glyph</a> <a href='undocumented#Glyph'>values</a> <a href='undocumented#Glyph'>without</a> <a href='undocumented#Glyph'>direct</a> <a href='undocumented#Glyph'>Unicode</a> <a href='undocumented#Glyph'>equivalents</a> <a href='undocumented#Glyph'>are</a> <a href='undocumented#Glyph'>mapped</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>zero</a>.
Uses the <a href='undocumented#SkTypeface'>SkTypeface</a>, <a href='undocumented#SkTypeface'>but</a> <a href='undocumented#SkTypeface'>is</a> <a href='undocumented#SkTypeface'>unaffected</a>
by <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a>; <a href='#SkPaint_TextEncoding'>the</a> <a href='#SkPaint_glyphsToUnichars_text'>text</a> <a href='#SkPaint_glyphsToUnichars_text'>values</a> <a href='#SkPaint_glyphsToUnichars_text'>returned</a> <a href='#SkPaint_glyphsToUnichars_text'>are</a> <a href='#SkPaint_glyphsToUnichars_text'>equivalent</a> <a href='#SkPaint_glyphsToUnichars_text'>to</a> <a href='#SkPaint_kUTF32_TextEncoding'>kUTF32_TextEncoding</a>.

Only supported on platforms that use FreeType as the  <a href='undocumented#Font_Engine'>font engine</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_glyphsToUnichars_glyphs'><code><strong>glyphs</strong></code></a></td>
    <td>array of indices into <a href='undocumented#Font'>font</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_glyphsToUnichars_count'><code><strong>count</strong></code></a></td>
    <td>length of <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>array</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_glyphsToUnichars_text'><code><strong>text</strong></code></a></td>
    <td>storage for character codes, one per <a href='undocumented#Glyph'>glyph</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c12686b0b3e0a87d0a248bbfc57e9492"><div>Convert UTF-8 <a href='#SkPaint_glyphsToUnichars_text'>text</a> <a href='#SkPaint_glyphsToUnichars_text'>to</a> <a href='#SkPaint_glyphsToUnichars_glyphs'>glyphs</a>; <a href='#SkPaint_glyphsToUnichars_glyphs'>then</a> <a href='#SkPaint_glyphsToUnichars_glyphs'>convert</a> <a href='#SkPaint_glyphsToUnichars_glyphs'>glyphs</a> <a href='#SkPaint_glyphsToUnichars_glyphs'>to</a> <a href='#SkPaint_glyphsToUnichars_glyphs'>Unichar</a> <a href='#SkPaint_glyphsToUnichars_glyphs'>code</a> <a href='SkPoint_Reference#Point'>points</a>.
</div></fiddle-embed></div>

<a name='Measure_Text'></a>

<a name='SkPaint_measureText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_measureText'>measureText</a>(<a href='#SkPaint_measureText'>const</a> <a href='#SkPaint_measureText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>, <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Returns the advance width of <a href='#SkPaint_measureText_text'>text</a>.
The advance is the normal distance to move before drawing additional <a href='#SkPaint_measureText_text'>text</a>.
Uses <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_TextEncoding'>decode</a> <a href='#SkPaint_measureText_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>to</a> <a href='undocumented#SkTypeface'>get</a> <a href='undocumented#SkTypeface'>the</a>  <a href='#Font_Metrics'>font metrics</a>,
and  <a href='#Text_Size'>text size</a>,   <a href='#Text_Scale_X'>text scale x</a>,   <a href='#Text_Skew_X'>text skew x</a>,  <a href='#Stroke_Width'>stroke width</a>, <a href='#SkPaint_measureText_text'>and</a>
<a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>to</a> <a href='undocumented#SkPathEffect'>scale</a> <a href='undocumented#SkPathEffect'>the</a> <a href='undocumented#SkPathEffect'>metrics</a> <a href='undocumented#SkPathEffect'>and</a> <a href='#SkPaint_measureText_bounds'>bounds</a>.
Returns the bounding box of <a href='#SkPaint_measureText_text'>text</a> <a href='#SkPaint_measureText_text'>if</a> <a href='#SkPaint_measureText_bounds'>bounds</a> <a href='#SkPaint_measureText_bounds'>is</a> <a href='#SkPaint_measureText_bounds'>not</a> <a href='#SkPaint_measureText_bounds'>nullptr</a>.
The bounding box is computed as if the <a href='#SkPaint_measureText_text'>text</a> <a href='#SkPaint_measureText_text'>was</a> <a href='#SkPaint_measureText_text'>drawn</a> <a href='#SkPaint_measureText_text'>at</a> <a href='#SkPaint_measureText_text'>the</a> <a href='#SkPaint_measureText_text'>origin</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_measureText_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>be</a> <a href='undocumented#Glyph'>measured</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_measureText_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_measureText_text'>text</a> <a href='#SkPaint_measureText_text'>to</a> <a href='#SkPaint_measureText_text'>measure</a></td>
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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_measureText'>measureText</a>(<a href='#SkPaint_measureText'>const</a> <a href='#SkPaint_measureText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>) <a href='undocumented#Text'>const</a>
</pre>

Returns the advance width of <a href='#SkPaint_measureText_2_text'>text</a>.
The advance is the normal distance to move before drawing additional <a href='#SkPaint_measureText_2_text'>text</a>.
Uses <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_TextEncoding'>decode</a> <a href='#SkPaint_measureText_2_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>to</a> <a href='undocumented#SkTypeface'>get</a> <a href='undocumented#SkTypeface'>the</a>  <a href='#Font_Metrics'>font metrics</a>,
and  <a href='#Text_Size'>text size</a> <a href='#SkPaint_measureText_2_text'>to</a> <a href='#SkPaint_measureText_2_text'>scale</a> <a href='#SkPaint_measureText_2_text'>the</a> <a href='#SkPaint_measureText_2_text'>metrics</a>.
Does not scale the advance or bounds by  <a href='#Fake_Bold'>fake bold</a> or <a href='undocumented#SkPathEffect'>SkPathEffect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_measureText_2_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>be</a> <a href='undocumented#Glyph'>measured</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_measureText_2_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_measureText_2_text'>text</a> <a href='#SkPaint_measureText_2_text'>to</a> <a href='#SkPaint_measureText_2_text'>measure</a></td>
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

<a name='SkPaint_breakText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkPaint_breakText'>breakText</a>(<a href='#SkPaint_breakText'>const</a> <a href='#SkPaint_breakText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>maxWidth</a>,
                 <a href='undocumented#SkScalar'>SkScalar</a>* <a href='undocumented#SkScalar'>measuredWidth</a> = <a href='undocumented#SkScalar'>nullptr</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns the bytes of <a href='#SkPaint_breakText_text'>text</a> <a href='#SkPaint_breakText_text'>that</a> <a href='#SkPaint_breakText_text'>fit</a> <a href='#SkPaint_breakText_text'>within</a> <a href='#SkPaint_breakText_maxWidth'>maxWidth</a>.
The <a href='#SkPaint_breakText_text'>text</a> <a href='#SkPaint_breakText_text'>fragment</a> <a href='#SkPaint_breakText_text'>fits</a> <a href='#SkPaint_breakText_text'>if</a> <a href='#SkPaint_breakText_text'>its</a> <a href='#SkPaint_breakText_text'>advance</a> <a href='#SkPaint_breakText_text'>width</a> <a href='#SkPaint_breakText_text'>is</a> <a href='#SkPaint_breakText_text'>less</a> <a href='#SkPaint_breakText_text'>than</a> <a href='#SkPaint_breakText_text'>or</a> <a href='#SkPaint_breakText_text'>equal</a> <a href='#SkPaint_breakText_text'>to</a> <a href='#SkPaint_breakText_maxWidth'>maxWidth</a>.
Measures only while the advance is less than or equal to <a href='#SkPaint_breakText_maxWidth'>maxWidth</a>.
Returns the advance or the <a href='#SkPaint_breakText_text'>text</a> <a href='#SkPaint_breakText_text'>fragment</a> <a href='#SkPaint_breakText_text'>in</a> <a href='#SkPaint_breakText_measuredWidth'>measuredWidth</a> <a href='#SkPaint_breakText_measuredWidth'>if</a> <a href='#SkPaint_breakText_measuredWidth'>it</a> <a href='#SkPaint_breakText_measuredWidth'>not</a> <a href='#SkPaint_breakText_measuredWidth'>nullptr</a>.
Uses <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_TextEncoding'>decode</a> <a href='#SkPaint_breakText_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>to</a> <a href='undocumented#SkTypeface'>get</a> <a href='undocumented#SkTypeface'>the</a>  <a href='#Font_Metrics'>font metrics</a>,
and  <a href='#Text_Size'>text size</a> <a href='#SkPaint_breakText_text'>to</a> <a href='#SkPaint_breakText_text'>scale</a> <a href='#SkPaint_breakText_text'>the</a> <a href='#SkPaint_breakText_text'>metrics</a>.
Does not scale the advance or bounds by  <a href='#Fake_Bold'>fake bold</a> or <a href='undocumented#SkPathEffect'>SkPathEffect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_breakText_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>be</a> <a href='undocumented#Glyph'>measured</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_breakText_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_breakText_text'>text</a> <a href='#SkPaint_breakText_text'>to</a> <a href='#SkPaint_breakText_text'>measure</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_breakText_maxWidth'><code><strong>maxWidth</strong></code></a></td>
    <td>advance limit; <a href='#SkPaint_breakText_text'>text</a> <a href='#SkPaint_breakText_text'>is</a> <a href='#SkPaint_breakText_text'>measured</a> <a href='#SkPaint_breakText_text'>while</a> <a href='#SkPaint_breakText_text'>advance</a> <a href='#SkPaint_breakText_text'>is</a> <a href='#SkPaint_breakText_text'>less</a> <a href='#SkPaint_breakText_text'>than</a> <a href='#SkPaint_breakText_maxWidth'>maxWidth</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_breakText_measuredWidth'><code><strong>measuredWidth</strong></code></a></td>
    <td>returns the width of the <a href='#SkPaint_breakText_text'>text</a> <a href='#SkPaint_breakText_text'>less</a> <a href='#SkPaint_breakText_text'>than</a> <a href='#SkPaint_breakText_text'>or</a> <a href='#SkPaint_breakText_text'>equal</a> <a href='#SkPaint_breakText_text'>to</a> <a href='#SkPaint_breakText_maxWidth'>maxWidth</a></td>
  </tr>
</table>

### Return Value

bytes of <a href='#SkPaint_breakText_text'>text</a> <a href='#SkPaint_breakText_text'>that</a> <a href='#SkPaint_breakText_text'>fit</a>, <a href='#SkPaint_breakText_text'>always</a> <a href='#SkPaint_breakText_text'>less</a> <a href='#SkPaint_breakText_text'>than</a> <a href='#SkPaint_breakText_text'>or</a> <a href='#SkPaint_breakText_text'>equal</a> <a href='#SkPaint_breakText_text'>to</a> <a href='#SkPaint_breakText_length'>length</a>

### Example

<div><fiddle-embed name="fd0033470ccbd5c7059670fdbf96cffc"><div><a href='undocumented#Line'>Line</a> <a href='undocumented#Line'>under</a> "<a href='undocumented#Line'>Breakfast</a>" <a href='undocumented#Line'>shows</a> <a href='undocumented#Line'>desired</a> <a href='undocumented#Line'>width</a>, <a href='undocumented#Line'>shorter</a> <a href='undocumented#Line'>than</a> <a href='undocumented#Line'>available</a> <a href='undocumented#Line'>characters</a>.
<a href='undocumented#Line'>Line</a> <a href='undocumented#Line'>under</a> "<a href='undocumented#Line'>Bre</a>" <a href='undocumented#Line'>shows</a> <a href='undocumented#Line'>measured</a> <a href='undocumented#Line'>width</a> <a href='undocumented#Line'>after</a> <a href='undocumented#Line'>breaking</a> <a href='#SkPaint_breakText_text'>text</a>.
</div></fiddle-embed></div>

<a name='SkPaint_getTextWidths'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getTextWidths'>getTextWidths</a>(<a href='#SkPaint_getTextWidths'>const</a> <a href='#SkPaint_getTextWidths'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>widths</a>[], <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>bounds</a>[] = <a href='SkRect_Reference#SkRect'>nullptr</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Retrieves the advance and <a href='#SkPaint_getTextWidths_bounds'>bounds</a> <a href='#SkPaint_getTextWidths_bounds'>for</a> <a href='#SkPaint_getTextWidths_bounds'>each</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>in</a> <a href='#SkPaint_getTextWidths_text'>text</a>, <a href='#SkPaint_getTextWidths_text'>and</a> <a href='#SkPaint_getTextWidths_text'>returns</a>
the <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>count</a> <a href='undocumented#Glyph'>in</a> <a href='#SkPaint_getTextWidths_text'>text</a>.
Both <a href='#SkPaint_getTextWidths_widths'>widths</a> <a href='#SkPaint_getTextWidths_widths'>and</a> <a href='#SkPaint_getTextWidths_bounds'>bounds</a> <a href='#SkPaint_getTextWidths_bounds'>may</a> <a href='#SkPaint_getTextWidths_bounds'>be</a> <a href='#SkPaint_getTextWidths_bounds'>nullptr</a>.
If <a href='#SkPaint_getTextWidths_widths'>widths</a> <a href='#SkPaint_getTextWidths_widths'>is</a> <a href='#SkPaint_getTextWidths_widths'>not</a> <a href='#SkPaint_getTextWidths_widths'>nullptr</a>, <a href='#SkPaint_getTextWidths_widths'>widths</a> <a href='#SkPaint_getTextWidths_widths'>must</a> <a href='#SkPaint_getTextWidths_widths'>be</a> <a href='#SkPaint_getTextWidths_widths'>an</a> <a href='#SkPaint_getTextWidths_widths'>array</a> <a href='#SkPaint_getTextWidths_widths'>of</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>count</a> <a href='undocumented#Glyph'>entries</a>.
if <a href='#SkPaint_getTextWidths_bounds'>bounds</a> <a href='#SkPaint_getTextWidths_bounds'>is</a> <a href='#SkPaint_getTextWidths_bounds'>not</a> <a href='#SkPaint_getTextWidths_bounds'>nullptr</a>, <a href='#SkPaint_getTextWidths_bounds'>bounds</a> <a href='#SkPaint_getTextWidths_bounds'>must</a> <a href='#SkPaint_getTextWidths_bounds'>be</a> <a href='#SkPaint_getTextWidths_bounds'>an</a> <a href='#SkPaint_getTextWidths_bounds'>array</a> <a href='#SkPaint_getTextWidths_bounds'>of</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>count</a> <a href='undocumented#Glyph'>entries</a>.
Uses <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_TextEncoding'>decode</a> <a href='#SkPaint_getTextWidths_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>to</a> <a href='undocumented#SkTypeface'>get</a> <a href='undocumented#SkTypeface'>the</a>  <a href='#Font_Metrics'>font metrics</a>,
and  <a href='#Text_Size'>text size</a> <a href='#SkPaint_getTextWidths_text'>to</a> <a href='#SkPaint_getTextWidths_text'>scale</a> <a href='#SkPaint_getTextWidths_text'>the</a> <a href='#SkPaint_getTextWidths_widths'>widths</a> <a href='#SkPaint_getTextWidths_widths'>and</a> <a href='#SkPaint_getTextWidths_bounds'>bounds</a>.
Does not scale the advance by  <a href='#Fake_Bold'>fake bold</a> or <a href='undocumented#SkPathEffect'>SkPathEffect</a>.
Does include  <a href='#Fake_Bold'>fake bold</a> and <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>in</a> <a href='undocumented#SkPathEffect'>the</a> <a href='#SkPaint_getTextWidths_bounds'>bounds</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextWidths_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>be</a> <a href='undocumented#Glyph'>measured</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextWidths_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getTextWidths_text'>text</a> <a href='#SkPaint_getTextWidths_text'>to</a> <a href='#SkPaint_getTextWidths_text'>measure</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextWidths_widths'><code><strong>widths</strong></code></a></td>
    <td>returns <a href='#SkPaint_getTextWidths_text'>text</a> <a href='#SkPaint_getTextWidths_text'>advances</a> <a href='#SkPaint_getTextWidths_text'>for</a> <a href='#SkPaint_getTextWidths_text'>each</a> <a href='undocumented#Glyph'>glyph</a>; <a href='undocumented#Glyph'>may</a> <a href='undocumented#Glyph'>be</a> <a href='undocumented#Glyph'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextWidths_bounds'><code><strong>bounds</strong></code></a></td>
    <td>returns <a href='#SkPaint_getTextWidths_bounds'>bounds</a> <a href='#SkPaint_getTextWidths_bounds'>for</a> <a href='#SkPaint_getTextWidths_bounds'>each</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>relative</a> <a href='undocumented#Glyph'>to</a> (0, 0); <a href='undocumented#Glyph'>may</a> <a href='undocumented#Glyph'>be</a> <a href='undocumented#Glyph'>nullptr</a></td>
  </tr>
</table>

### Return Value

<a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>count</a> <a href='undocumented#Glyph'>in</a> <a href='#SkPaint_getTextWidths_text'>text</a>

### Example

<div><fiddle-embed name="6b9e101f49e9c2c28755c5bdcef64dfb"><div>Bounds of <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>increase</a> <a href='undocumented#Glyph'>for</a> <a href='undocumented#Glyph'>stroked</a> <a href='#SkPaint_getTextWidths_text'>text</a>, <a href='#SkPaint_getTextWidths_text'>but</a> <a href='#SkPaint_getTextWidths_text'>text</a> <a href='#SkPaint_getTextWidths_text'>advance</a> <a href='#SkPaint_getTextWidths_text'>remains</a> <a href='#SkPaint_getTextWidths_text'>the</a> <a href='#SkPaint_getTextWidths_text'>same</a>.
<a href='#SkPaint_getTextWidths_text'>The</a> <a href='#SkPaint_getTextWidths_text'>underlines</a> <a href='#SkPaint_getTextWidths_text'>show</a> <a href='#SkPaint_getTextWidths_text'>the</a> <a href='#SkPaint_getTextWidths_text'>text</a> <a href='#SkPaint_getTextWidths_text'>advance</a>, <a href='#SkPaint_getTextWidths_text'>spaced</a> <a href='#SkPaint_getTextWidths_text'>to</a> <a href='#SkPaint_getTextWidths_text'>keep</a> <a href='#SkPaint_getTextWidths_text'>them</a> <a href='#SkPaint_getTextWidths_text'>distinct</a>.
</div></fiddle-embed></div>

<a name='Text_Path'></a>

<a href='#Paint_Text_Path'>Text_Path</a> <a href='#Paint_Text_Path'>describes</a> <a href='#Paint_Text_Path'>the</a> <a href='#Paint_Text_Path'>geometry</a> <a href='#Paint_Text_Path'>of</a> <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>used</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>draw</a> <a href='undocumented#Text'>text</a>.

<a name='SkPaint_getTextPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_getTextPath'>getTextPath</a>(<a href='#SkPaint_getTextPath'>const</a> <a href='#SkPaint_getTextPath'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#Path'>path</a>) <a href='SkPath_Reference#Path'>const</a>
</pre>

Returns the geometry as <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>equivalent</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>the</a> <a href='SkPath_Reference#SkPath'>drawn</a> <a href='#SkPaint_getTextPath_text'>text</a>.
Uses <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_TextEncoding'>decode</a> <a href='#SkPaint_getTextPath_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>to</a> <a href='undocumented#SkTypeface'>get</a> <a href='undocumented#SkTypeface'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>,
and  <a href='#Text_Size'>text size</a>,  <a href='#Fake_Bold'>fake bold</a>, <a href='#SkPaint_getTextPath_text'>and</a> <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>to</a> <a href='undocumented#SkPathEffect'>scale</a> <a href='undocumented#SkPathEffect'>and</a> <a href='undocumented#SkPathEffect'>modify</a> <a href='undocumented#SkPathEffect'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>.
All of the <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a> <a href='SkPath_Reference#Path'>are</a> <a href='SkPath_Reference#Path'>stored</a> <a href='SkPath_Reference#Path'>in</a> <a href='#SkPaint_getTextPath_path'>path</a>.
Uses <a href='#SkPaint_getTextPath_x'>x</a>, <a href='#SkPaint_getTextPath_y'>y</a>, <a href='#SkPaint_getTextPath_y'>to</a> <a href='#SkPaint_getTextPath_y'>position</a> <a href='#SkPaint_getTextPath_path'>path</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextPath_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a></td>
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

<div><fiddle-embed name="7c9e6a399f898d68026c1f0865e6f73e"><div><a href='undocumented#Text'>Text</a> <a href='undocumented#Text'>is</a> <a href='undocumented#Text'>added</a> <a href='undocumented#Text'>to</a> <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>offset</a>, <a href='SkPath_Reference#Path'>and</a> <a href='SkPath_Reference#Path'>subtracted</a> <a href='SkPath_Reference#Path'>from</a> <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>then</a> <a href='SkPath_Reference#Path'>added</a> <a href='SkPath_Reference#Path'>at</a>
<a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>offset</a> <a href='SkPath_Reference#Path'>location</a>. <a href='SkPath_Reference#Path'>The</a> <a href='SkPath_Reference#Path'>result</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>rendered</a> <a href='SkPath_Reference#Path'>with</a> <a href='SkPath_Reference#Path'>one</a> <a href='SkPath_Reference#Path'>draw</a> <a href='SkPath_Reference#Path'>call</a>.
</div></fiddle-embed></div>

<a name='SkPaint_getPosTextPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_getPosTextPath'>getPosTextPath</a>(<a href='#SkPaint_getPosTextPath'>const</a> <a href='#SkPaint_getPosTextPath'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>, <a href='undocumented#Text'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pos</a>[], <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#Path'>path</a>) <a href='SkPath_Reference#Path'>const</a>
</pre>

Returns the geometry as <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>equivalent</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>the</a> <a href='SkPath_Reference#SkPath'>drawn</a> <a href='#SkPaint_getPosTextPath_text'>text</a>.
Uses <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_TextEncoding'>decode</a> <a href='#SkPaint_getPosTextPath_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>to</a> <a href='undocumented#SkTypeface'>get</a> <a href='undocumented#SkTypeface'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>,
and  <a href='#Text_Size'>text size</a>,  <a href='#Fake_Bold'>fake bold</a>, <a href='#SkPaint_getPosTextPath_text'>and</a> <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>to</a> <a href='undocumented#SkPathEffect'>scale</a> <a href='undocumented#SkPathEffect'>and</a> <a href='undocumented#SkPathEffect'>modify</a> <a href='undocumented#SkPathEffect'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>.
All of the <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a> <a href='SkPath_Reference#Path'>are</a> <a href='SkPath_Reference#Path'>stored</a> <a href='SkPath_Reference#Path'>in</a> <a href='#SkPaint_getPosTextPath_path'>path</a>.
Uses <a href='#SkPaint_getPosTextPath_pos'>pos</a> <a href='#SkPaint_getPosTextPath_pos'>array</a> <a href='#SkPaint_getPosTextPath_pos'>to</a> <a href='#SkPaint_getPosTextPath_pos'>position</a> <a href='#SkPaint_getPosTextPath_path'>path</a>.
<a href='#SkPaint_getPosTextPath_pos'>pos</a> <a href='#SkPaint_getPosTextPath_pos'>contains</a> <a href='#SkPaint_getPosTextPath_pos'>a</a> <a href='#SkPaint_getPosTextPath_pos'>position</a> <a href='#SkPaint_getPosTextPath_pos'>for</a> <a href='#SkPaint_getPosTextPath_pos'>each</a> <a href='undocumented#Glyph'>glyph</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getPosTextPath_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a></td>
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

<div><fiddle-embed name="7f27c93472aa99a7542fb3493076f072"><div>Simplifies three <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>to</a> <a href='undocumented#Glyph'>eliminate</a> <a href='undocumented#Glyph'>overlaps</a>, <a href='undocumented#Glyph'>and</a> <a href='undocumented#Glyph'>strokes</a> <a href='undocumented#Glyph'>the</a> <a href='undocumented#Glyph'>result</a>.
</div></fiddle-embed></div>

<a name='Text_Intercepts'></a>

<a href='#Paint_Text_Intercepts'>Text_Intercepts</a> <a href='#Paint_Text_Intercepts'>describe</a> <a href='#Paint_Text_Intercepts'>the</a> <a href='#Paint_Text_Intercepts'>intersection</a> <a href='#Paint_Text_Intercepts'>of</a> <a href='#Paint_Text_Intercepts'>drawn</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>with</a> <a href='undocumented#Glyph'>a</a> <a href='undocumented#Glyph'>pair</a>
<a href='undocumented#Glyph'>of</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>parallel</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>advance</a>. <a href='#Paint_Text_Intercepts'>Text_Intercepts</a> <a href='#Paint_Text_Intercepts'>permits</a> <a href='#Paint_Text_Intercepts'>creating</a> <a href='#Paint_Text_Intercepts'>a</a>
<a href='#Paint_Text_Intercepts'>underline</a> <a href='#Paint_Text_Intercepts'>that</a> <a href='#Paint_Text_Intercepts'>skips</a> <a href='#Paint_Text_Intercepts'>Descenders</a>.

<a name='SkPaint_getTextIntercepts'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getTextIntercepts'>getTextIntercepts</a>(<a href='#SkPaint_getTextIntercepts'>const</a> <a href='#SkPaint_getTextIntercepts'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>,
                      <a href='undocumented#SkScalar'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bounds</a>[2], <a href='undocumented#SkScalar'>SkScalar</a>* <a href='undocumented#SkScalar'>intervals</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns the number of <a href='#SkPaint_getTextIntercepts_intervals'>intervals</a> <a href='#SkPaint_getTextIntercepts_intervals'>that</a> <a href='#SkPaint_getTextIntercepts_intervals'>intersect</a> <a href='#SkPaint_getTextIntercepts_bounds'>bounds</a>.
<a href='#SkPaint_getTextIntercepts_bounds'>bounds</a> <a href='#SkPaint_getTextIntercepts_bounds'>describes</a> <a href='#SkPaint_getTextIntercepts_bounds'>a</a> <a href='#SkPaint_getTextIntercepts_bounds'>pair</a> <a href='#SkPaint_getTextIntercepts_bounds'>of</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>parallel</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Line'>the</a> <a href='#SkPaint_getTextIntercepts_text'>text</a> <a href='#SkPaint_getTextIntercepts_text'>advance</a>.
The return count is zero or a multiple of two, and is at most twice the number of <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>in</a>
the <a href='undocumented#String'>string</a>.
Uses <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_TextEncoding'>decode</a> <a href='#SkPaint_getTextIntercepts_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>to</a> <a href='undocumented#SkTypeface'>get</a> <a href='undocumented#SkTypeface'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>,
and  <a href='#Text_Size'>text size</a>,  <a href='#Fake_Bold'>fake bold</a>, <a href='#SkPaint_getTextIntercepts_text'>and</a> <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>to</a> <a href='undocumented#SkPathEffect'>scale</a> <a href='undocumented#SkPathEffect'>and</a> <a href='undocumented#SkPathEffect'>modify</a> <a href='undocumented#SkPathEffect'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>.
Uses <a href='#SkPaint_getTextIntercepts_x'>x</a>, <a href='#SkPaint_getTextIntercepts_y'>y</a> <a href='#SkPaint_getTextIntercepts_y'>to</a> <a href='#SkPaint_getTextIntercepts_y'>position</a> <a href='#SkPaint_getTextIntercepts_intervals'>intervals</a>.

Pass nullptr for <a href='#SkPaint_getTextIntercepts_intervals'>intervals</a> <a href='#SkPaint_getTextIntercepts_intervals'>to</a> <a href='#SkPaint_getTextIntercepts_intervals'>determine</a> <a href='#SkPaint_getTextIntercepts_intervals'>the</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>interval</a> <a href='undocumented#Size'>array</a>.

<a href='#SkPaint_getTextIntercepts_intervals'>intervals</a> <a href='#SkPaint_getTextIntercepts_intervals'>are</a> <a href='#SkPaint_getTextIntercepts_intervals'>cached</a> <a href='#SkPaint_getTextIntercepts_intervals'>to</a> <a href='#SkPaint_getTextIntercepts_intervals'>improve</a> <a href='#SkPaint_getTextIntercepts_intervals'>performance</a> <a href='#SkPaint_getTextIntercepts_intervals'>for</a> <a href='#SkPaint_getTextIntercepts_intervals'>multiple</a> <a href='#SkPaint_getTextIntercepts_intervals'>calls</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextIntercepts_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getTextIntercepts_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_x'><code><strong>x</strong></code></a></td>
    <td>x-axis value of the origin of the <a href='#SkPaint_getTextIntercepts_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_y'><code><strong>y</strong></code></a></td>
    <td>y-axis value of the origin of the <a href='#SkPaint_getTextIntercepts_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_bounds'><code><strong>bounds</strong></code></a></td>
    <td>lower and upper <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>parallel</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>advance</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_intervals'><code><strong>intervals</strong></code></a></td>
    <td>returned intersections; may be nullptr</td>
  </tr>
</table>

### Return Value

number of intersections; may be zero

### Example

<div><fiddle-embed name="2a0b80ed20d193c688085b79deb5bdc9"><div>Underline uses intercepts to draw on either side of the <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>Descender</a>.
</div></fiddle-embed></div>

<a name='SkPaint_getPosTextIntercepts'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getPosTextIntercepts'>getPosTextIntercepts</a>(<a href='#SkPaint_getPosTextIntercepts'>const</a> <a href='#SkPaint_getPosTextIntercepts'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>, <a href='undocumented#Text'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pos</a>[],
                         <a href='SkPoint_Reference#SkPoint'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bounds</a>[2], <a href='undocumented#SkScalar'>SkScalar</a>* <a href='undocumented#SkScalar'>intervals</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns the number of <a href='#SkPaint_getPosTextIntercepts_intervals'>intervals</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>that</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>intersect</a> <a href='#SkPaint_getPosTextIntercepts_bounds'>bounds</a>.
<a href='#SkPaint_getPosTextIntercepts_bounds'>bounds</a> <a href='#SkPaint_getPosTextIntercepts_bounds'>describes</a> <a href='#SkPaint_getPosTextIntercepts_bounds'>a</a> <a href='#SkPaint_getPosTextIntercepts_bounds'>pair</a> <a href='#SkPaint_getPosTextIntercepts_bounds'>of</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>parallel</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Line'>the</a> <a href='#SkPaint_getPosTextIntercepts_text'>text</a> <a href='#SkPaint_getPosTextIntercepts_text'>advance</a>.
The return count is zero or a multiple of two, and is at most twice the number of <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>in</a>
the <a href='undocumented#String'>string</a>.
Uses <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_TextEncoding'>decode</a> <a href='#SkPaint_getPosTextIntercepts_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>to</a> <a href='undocumented#SkTypeface'>get</a> <a href='undocumented#SkTypeface'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>,
and  <a href='#Text_Size'>text size</a>,  <a href='#Fake_Bold'>fake bold</a>, <a href='#SkPaint_getPosTextIntercepts_text'>and</a> <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>to</a> <a href='undocumented#SkPathEffect'>scale</a> <a href='undocumented#SkPathEffect'>and</a> <a href='undocumented#SkPathEffect'>modify</a> <a href='undocumented#SkPathEffect'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>.
Uses <a href='#SkPaint_getPosTextIntercepts_pos'>pos</a> <a href='#SkPaint_getPosTextIntercepts_pos'>array</a> <a href='#SkPaint_getPosTextIntercepts_pos'>to</a> <a href='#SkPaint_getPosTextIntercepts_pos'>position</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>intervals</a>.

Pass nullptr for <a href='#SkPaint_getPosTextIntercepts_intervals'>intervals</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>to</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>determine</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>the</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>interval</a> <a href='undocumented#Size'>array</a>.

<a href='#SkPaint_getPosTextIntercepts_intervals'>intervals</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>are</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>cached</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>to</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>improve</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>performance</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>for</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>multiple</a> <a href='#SkPaint_getPosTextIntercepts_intervals'>calls</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getPosTextIntercepts_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextIntercepts_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getPosTextIntercepts_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextIntercepts_pos'><code><strong>pos</strong></code></a></td>
    <td>positions of each <a href='undocumented#Glyph'>glyph</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextIntercepts_bounds'><code><strong>bounds</strong></code></a></td>
    <td>lower and upper <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>parallel</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>advance</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextIntercepts_intervals'><code><strong>intervals</strong></code></a></td>
    <td>returned intersections; may be nullptr</td>
  </tr>
</table>

### Return Value

number of intersections; may be zero

### Example

<div><fiddle-embed name="5b5754fdb90b44c245d72567854eca04"><div><a href='undocumented#Text'>Text</a> <a href='undocumented#Text'>intercepts</a> <a href='undocumented#Text'>draw</a> <a href='undocumented#Text'>on</a> <a href='undocumented#Text'>either</a> <a href='undocumented#Text'>side</a> <a href='undocumented#Text'>of</a>, <a href='undocumented#Text'>but</a> <a href='undocumented#Text'>not</a> <a href='undocumented#Text'>inside</a>, <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>in</a> <a href='undocumented#Glyph'>a</a> <a href='undocumented#Glyph'>run</a>.
</div></fiddle-embed></div>

<a name='SkPaint_getPosTextHIntercepts'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getPosTextHIntercepts'>getPosTextHIntercepts</a>(<a href='#SkPaint_getPosTextHIntercepts'>const</a> <a href='#SkPaint_getPosTextHIntercepts'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>length</a>, <a href='undocumented#Text'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xpos</a>[], <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>constY</a>,
                          <a href='undocumented#SkScalar'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bounds</a>[2], <a href='undocumented#SkScalar'>SkScalar</a>* <a href='undocumented#SkScalar'>intervals</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns the number of <a href='#SkPaint_getPosTextHIntercepts_intervals'>intervals</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>that</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>intersect</a> <a href='#SkPaint_getPosTextHIntercepts_bounds'>bounds</a>.
<a href='#SkPaint_getPosTextHIntercepts_bounds'>bounds</a> <a href='#SkPaint_getPosTextHIntercepts_bounds'>describes</a> <a href='#SkPaint_getPosTextHIntercepts_bounds'>a</a> <a href='#SkPaint_getPosTextHIntercepts_bounds'>pair</a> <a href='#SkPaint_getPosTextHIntercepts_bounds'>of</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>parallel</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Line'>the</a> <a href='#SkPaint_getPosTextHIntercepts_text'>text</a> <a href='#SkPaint_getPosTextHIntercepts_text'>advance</a>.
The return count is zero or a multiple of two, and is at most twice the number of <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>in</a>
the <a href='undocumented#String'>string</a>.
Uses <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_TextEncoding'>decode</a> <a href='#SkPaint_getPosTextHIntercepts_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>to</a> <a href='undocumented#SkTypeface'>get</a> <a href='undocumented#SkTypeface'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>,
and  <a href='#Text_Size'>text size</a>,  <a href='#Fake_Bold'>fake bold</a>, <a href='#SkPaint_getPosTextHIntercepts_text'>and</a> <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>to</a> <a href='undocumented#SkPathEffect'>scale</a> <a href='undocumented#SkPathEffect'>and</a> <a href='undocumented#SkPathEffect'>modify</a> <a href='undocumented#SkPathEffect'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>.
Uses <a href='#SkPaint_getPosTextHIntercepts_xpos'>xpos</a> <a href='#SkPaint_getPosTextHIntercepts_xpos'>array</a>, <a href='#SkPaint_getPosTextHIntercepts_constY'>constY</a> <a href='#SkPaint_getPosTextHIntercepts_constY'>to</a> <a href='#SkPaint_getPosTextHIntercepts_constY'>position</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>intervals</a>.

Pass nullptr for <a href='#SkPaint_getPosTextHIntercepts_intervals'>intervals</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>to</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>determine</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>the</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>interval</a> <a href='undocumented#Size'>array</a>.

<a href='#SkPaint_getPosTextHIntercepts_intervals'>intervals</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>are</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>cached</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>to</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>improve</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>performance</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>for</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>multiple</a> <a href='#SkPaint_getPosTextHIntercepts_intervals'>calls</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getPosTextHIntercepts_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>indices</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextHIntercepts_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getPosTextHIntercepts_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextHIntercepts_xpos'><code><strong>xpos</strong></code></a></td>
    <td>positions of each <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>on</a> <a href='undocumented#Glyph'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextHIntercepts_constY'><code><strong>constY</strong></code></a></td>
    <td>position of each <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>on</a> <a href='undocumented#Glyph'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextHIntercepts_bounds'><code><strong>bounds</strong></code></a></td>
    <td>lower and upper <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>parallel</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>advance</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextHIntercepts_intervals'><code><strong>intervals</strong></code></a></td>
    <td>returned intersections; may be nullptr</td>
  </tr>
</table>

### Return Value

number of intersections; may be zero

### Example

<div><fiddle-embed name="dc9851c43acc3716aca8c9a4d40d452d"><div><a href='undocumented#Text'>Text</a> <a href='undocumented#Text'>intercepts</a> <a href='undocumented#Text'>do</a> <a href='undocumented#Text'>not</a> <a href='undocumented#Text'>take</a> <a href='undocumented#Text'>stroke</a> <a href='undocumented#Text'>thickness</a> <a href='undocumented#Text'>into</a> <a href='undocumented#Text'>consideration</a>.
</div></fiddle-embed></div>

<a name='SkPaint_getTextBlobIntercepts'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getTextBlobIntercepts'>getTextBlobIntercepts</a>(<a href='#SkPaint_getTextBlobIntercepts'>const</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>* <a href='SkTextBlob_Reference#SkTextBlob'>blob</a>, <a href='SkTextBlob_Reference#SkTextBlob'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bounds</a>[2], <a href='undocumented#SkScalar'>SkScalar</a>* <a href='undocumented#SkScalar'>intervals</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns the number of <a href='#SkPaint_getTextBlobIntercepts_intervals'>intervals</a> <a href='#SkPaint_getTextBlobIntercepts_intervals'>that</a> <a href='#SkPaint_getTextBlobIntercepts_intervals'>intersect</a> <a href='#SkPaint_getTextBlobIntercepts_bounds'>bounds</a>.
<a href='#SkPaint_getTextBlobIntercepts_bounds'>bounds</a> <a href='#SkPaint_getTextBlobIntercepts_bounds'>describes</a> <a href='#SkPaint_getTextBlobIntercepts_bounds'>a</a> <a href='#SkPaint_getTextBlobIntercepts_bounds'>pair</a> <a href='#SkPaint_getTextBlobIntercepts_bounds'>of</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>parallel</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>advance</a>.
The return count is zero or a multiple of two, and is at most twice the number of <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>in</a>
the <a href='undocumented#String'>string</a>.
Uses <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>to</a> <a href='undocumented#SkTypeface'>get</a> <a href='undocumented#SkTypeface'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>,
and  <a href='#Text_Size'>text size</a>,  <a href='#Fake_Bold'>fake bold</a>, <a href='undocumented#Text'>and</a> <a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>to</a> <a href='undocumented#SkPathEffect'>scale</a> <a href='undocumented#SkPathEffect'>and</a> <a href='undocumented#SkPathEffect'>modify</a> <a href='undocumented#SkPathEffect'>the</a> <a href='undocumented#Glyph'>glyph</a> <a href='SkPath_Reference#Path'>paths</a>.
Uses run array to position <a href='#SkPaint_getTextBlobIntercepts_intervals'>intervals</a>.

<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>must</a> <a href='#SkPaint_TextEncoding'>be</a> <a href='#SkPaint_TextEncoding'>set</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID_TextEncoding</a>.

Pass nullptr for <a href='#SkPaint_getTextBlobIntercepts_intervals'>intervals</a> <a href='#SkPaint_getTextBlobIntercepts_intervals'>to</a> <a href='#SkPaint_getTextBlobIntercepts_intervals'>determine</a> <a href='#SkPaint_getTextBlobIntercepts_intervals'>the</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>interval</a> <a href='undocumented#Size'>array</a>.

<a href='#SkPaint_getTextBlobIntercepts_intervals'>intervals</a> <a href='#SkPaint_getTextBlobIntercepts_intervals'>are</a> <a href='#SkPaint_getTextBlobIntercepts_intervals'>cached</a> <a href='#SkPaint_getTextBlobIntercepts_intervals'>to</a> <a href='#SkPaint_getTextBlobIntercepts_intervals'>improve</a> <a href='#SkPaint_getTextBlobIntercepts_intervals'>performance</a> <a href='#SkPaint_getTextBlobIntercepts_intervals'>for</a> <a href='#SkPaint_getTextBlobIntercepts_intervals'>multiple</a> <a href='#SkPaint_getTextBlobIntercepts_intervals'>calls</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextBlobIntercepts_blob'><code><strong>blob</strong></code></a></td>
    <td><a href='undocumented#Glyph'>glyphs</a>, <a href='undocumented#Glyph'>positions</a>, <a href='undocumented#Glyph'>and</a> <a href='undocumented#Text'>text</a> <a href='SkPaint_Reference#Paint'>paint</a> <a href='SkPaint_Reference#Paint'>attributes</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextBlobIntercepts_bounds'><code><strong>bounds</strong></code></a></td>
    <td>lower and upper <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>parallel</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>advance</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextBlobIntercepts_intervals'><code><strong>intervals</strong></code></a></td>
    <td>returned intersections; may be nullptr</td>
  </tr>
</table>

### Return Value

number of intersections; may be zero

### Example

<div><fiddle-embed name="f2229dd5c8e76f9e12fafe59b61353c8"></fiddle-embed></div>

<a name='SkPaint_getFontBounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPaint_getFontBounds'>getFontBounds</a>() <a href='#SkPaint_getFontBounds'>const</a>
</pre>

Returns the union of bounds of all <a href='undocumented#Glyph'>glyphs</a>.
Returned dimensions are computed by <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>manager</a> <a href='undocumented#Font'>from</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Data'>data</a>,
ignoring <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Hinting'>Hinting</a>. <a href='#SkPaint_Hinting'>Includes</a> <a href='undocumented#Font'>font</a> <a href='undocumented#Font'>metrics</a>, <a href='undocumented#Font'>but</a> <a href='undocumented#Font'>not</a> <a href='undocumented#Font'>fake</a> <a href='undocumented#Font'>bold</a> <a href='undocumented#Font'>or</a> <a href='undocumented#SkPathEffect'>SkPathEffect</a>.

If <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>large</a>, <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>scale</a> <a href='undocumented#Text'>is</a> <a href='undocumented#Text'>one</a>, <a href='undocumented#Text'>and</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>skew</a> <a href='undocumented#Text'>is</a> <a href='undocumented#Text'>zero</a>,
returns the bounds as:
{ <a href='undocumented#SkFontMetrics'>SkFontMetrics</a>::<a href='#SkFontMetrics_fXMin'>fXMin</a>, <a href='undocumented#SkFontMetrics'>SkFontMetrics</a>::<a href='#SkFontMetrics_fTop'>fTop</a>, <a href='undocumented#SkFontMetrics'>SkFontMetrics</a>::<a href='#SkFontMetrics_fXMax'>fXMax</a>, <a href='undocumented#SkFontMetrics'>SkFontMetrics</a>::<a href='#SkFontMetrics_fBottom'>fBottom</a> }.

### Return Value

union of bounds of all <a href='undocumented#Glyph'>glyphs</a>

### Example

<div><fiddle-embed name="facaddeec7943bc491988e345e27e65f">

#### Example Output

~~~~
metrics bounds = { -12.2461, -14.7891, 21.5215, 5.55469 }
font bounds    = { -12.2461, -14.7891, 21.5215, 5.55469 }
~~~~

</fiddle-embed></div>

<a name='SkPaint_nothingToDraw'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_nothingToDraw'>nothingToDraw</a>() <a href='#SkPaint_nothingToDraw'>const</a>
</pre>

Returns true if <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>prevents</a> <a href='SkPaint_Reference#SkPaint'>all</a> <a href='SkPaint_Reference#SkPaint'>drawing</a>;
otherwise, the <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>may</a> <a href='SkPaint_Reference#SkPaint'>or</a> <a href='SkPaint_Reference#SkPaint'>may</a> <a href='SkPaint_Reference#SkPaint'>not</a> <a href='SkPaint_Reference#SkPaint'>allow</a> <a href='SkPaint_Reference#SkPaint'>drawing</a>.

Returns true if, for example, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>combined</a> <a href='SkBlendMode_Reference#SkBlendMode'>with</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>computes</a> <a href='SkColor_Reference#Alpha'>a</a>
new <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkColor_Reference#Alpha'>zero</a>.

### Return Value

true if <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>prevents</a> <a href='SkPaint_Reference#SkPaint'>all</a> <a href='SkPaint_Reference#SkPaint'>drawing</a>

### Example

<div><fiddle-embed name="2973b05bfbb6b4c29332c8ac4fcf3995">

#### Example Output

~~~~
initial nothing to draw: false
blend dst nothing to draw: true
blend src over nothing to draw: false
alpha 0 nothing to draw: true
~~~~

</fiddle-embed></div>

<a name='Fast_Bounds'></a>

Private: To be made private.

<a href='#Paint_Fast_Bounds'>Fast_Bounds</a> <a href='#Paint_Fast_Bounds'>functions</a> <a href='#Paint_Fast_Bounds'>conservatively</a> <a href='#Paint_Fast_Bounds'>outset</a> <a href='#Paint_Fast_Bounds'>a</a> <a href='#Paint_Fast_Bounds'>drawing</a> <a href='#Paint_Fast_Bounds'>bounds</a> <a href='#Paint_Fast_Bounds'>by</a> <a href='#Paint_Fast_Bounds'>additional</a> <a href='#Paint_Fast_Bounds'>area</a>
<a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>may</a> <a href='SkPaint_Reference#Paint'>draw</a> <a href='SkPaint_Reference#Paint'>to</a>.

<a name='SkPaint_canComputeFastBounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_canComputeFastBounds'>canComputeFastBounds</a>() <a href='#SkPaint_canComputeFastBounds'>const</a>
</pre>

Private: (to be made private)

Returns true if <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>does</a> <a href='SkPaint_Reference#Paint'>not</a> <a href='SkPaint_Reference#Paint'>include</a> <a href='SkPaint_Reference#Paint'>elements</a> <a href='SkPaint_Reference#Paint'>requiring</a> <a href='SkPaint_Reference#Paint'>extensive</a> <a href='SkPaint_Reference#Paint'>computation</a>
<a href='SkPaint_Reference#Paint'>to</a> <a href='SkPaint_Reference#Paint'>compute</a> <a href='undocumented#Device'>Device</a> <a href='undocumented#Device'>bounds</a> <a href='undocumented#Device'>of</a> <a href='undocumented#Device'>drawn</a> <a href='undocumented#Device'>geometry</a>. <a href='undocumented#Device'>For</a> <a href='undocumented#Device'>instance</a>, <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>with</a> <a href='#Path_Effect'>Path_Effect</a>
<a href='#Path_Effect'>always</a> <a href='#Path_Effect'>returns</a> <a href='#Path_Effect'>false</a>.

### Return Value

true if <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>allows</a> <a href='SkPaint_Reference#Paint'>for</a> <a href='SkPaint_Reference#Paint'>fast</a> <a href='SkPaint_Reference#Paint'>computation</a> <a href='SkPaint_Reference#Paint'>of</a> <a href='SkPaint_Reference#Paint'>bounds</a>

<a name='SkPaint_computeFastBounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkPaint_computeFastBounds'>computeFastBounds</a>(<a href='#SkPaint_computeFastBounds'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>orig</a>, <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>storage</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Private: (to be made private)

Only call if <a href='#SkPaint_canComputeFastBounds'>canComputeFastBounds</a> <a href='#SkPaint_canComputeFastBounds'>returned</a> <a href='#SkPaint_canComputeFastBounds'>true</a>. <a href='#SkPaint_canComputeFastBounds'>This</a> <a href='#SkPaint_canComputeFastBounds'>takes</a> <a href='#SkPaint_canComputeFastBounds'>a</a>
<a href='#SkPaint_canComputeFastBounds'>raw</a> <a href='#SkPaint_canComputeFastBounds'>rectangle</a>, <a href='#SkPaint_canComputeFastBounds'>the</a> <a href='#SkPaint_canComputeFastBounds'>raw</a> <a href='#SkPaint_canComputeFastBounds'>bounds</a> <a href='#SkPaint_canComputeFastBounds'>of</a> <a href='#SkPaint_canComputeFastBounds'>a</a> <a href='#SkPaint_canComputeFastBounds'>shape</a>; <a href='#SkPaint_canComputeFastBounds'>and</a> <a href='#SkPaint_canComputeFastBounds'>adjusts</a> <a href='#SkPaint_canComputeFastBounds'>it</a> <a href='#SkPaint_canComputeFastBounds'>for</a> <a href='#SkPaint_canComputeFastBounds'>stylistic</a>
<a href='#SkPaint_canComputeFastBounds'>effects</a> <a href='#SkPaint_canComputeFastBounds'>in</a> <a href='#SkPaint_canComputeFastBounds'>the</a> <a href='SkPaint_Reference#Paint'>paint</a>, <a href='SkPaint_Reference#Paint'>such</a> <a href='SkPaint_Reference#Paint'>as</a> <a href='SkPaint_Reference#Paint'>stroking</a>. <a href='SkPaint_Reference#Paint'>If</a> <a href='SkPaint_Reference#Paint'>needed</a>, <a href='SkPaint_Reference#Paint'>it</a> <a href='SkPaint_Reference#Paint'>uses</a> <a href='SkPaint_Reference#Paint'>the</a> <a href='#SkPaint_computeFastBounds_storage'>storage</a>
<a href='#SkPaint_computeFastBounds_storage'>parameter</a>. <a href='#SkPaint_computeFastBounds_storage'>It</a> <a href='#SkPaint_computeFastBounds_storage'>returns</a> <a href='#SkPaint_computeFastBounds_storage'>the</a> <a href='#SkPaint_computeFastBounds_storage'>adjusted</a> <a href='#SkPaint_computeFastBounds_storage'>bounds</a> <a href='#SkPaint_computeFastBounds_storage'>that</a> <a href='#SkPaint_computeFastBounds_storage'>can</a> <a href='#SkPaint_computeFastBounds_storage'>then</a> <a href='#SkPaint_computeFastBounds_storage'>be</a> <a href='#SkPaint_computeFastBounds_storage'>used</a>
<a href='#SkPaint_computeFastBounds_storage'>for</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_quickReject'>quickReject</a> <a href='#SkCanvas_quickReject'>tests</a>.

<a href='#SkCanvas_quickReject'>The</a> <a href='#SkCanvas_quickReject'>returned</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>will</a> <a href='SkRect_Reference#Rect'>either</a> <a href='SkRect_Reference#Rect'>be</a> <a href='#SkPaint_computeFastBounds_orig'>orig</a> <a href='#SkPaint_computeFastBounds_orig'>or</a> <a href='#SkPaint_computeFastBounds_storage'>storage</a>, <a href='#SkPaint_computeFastBounds_storage'>thus</a> <a href='#SkPaint_computeFastBounds_storage'>the</a> <a href='#SkPaint_computeFastBounds_storage'>caller</a>
<a href='#SkPaint_computeFastBounds_storage'>should</a> <a href='#SkPaint_computeFastBounds_storage'>not</a> <a href='#SkPaint_computeFastBounds_storage'>rely</a> <a href='#SkPaint_computeFastBounds_storage'>on</a> <a href='#SkPaint_computeFastBounds_storage'>storage</a> <a href='#SkPaint_computeFastBounds_storage'>being</a> <a href='#SkPaint_computeFastBounds_storage'>set</a> <a href='#SkPaint_computeFastBounds_storage'>to</a> <a href='#SkPaint_computeFastBounds_storage'>the</a> <a href='#SkPaint_computeFastBounds_storage'>result</a>, <a href='#SkPaint_computeFastBounds_storage'>but</a> <a href='#SkPaint_computeFastBounds_storage'>should</a> <a href='#SkPaint_computeFastBounds_storage'>always</a>
<a href='#SkPaint_computeFastBounds_storage'>use</a> <a href='#SkPaint_computeFastBounds_storage'>the</a> <a href='#SkPaint_computeFastBounds_storage'>returned</a> <a href='#SkPaint_computeFastBounds_storage'>value</a>. <a href='#SkPaint_computeFastBounds_storage'>It</a> <a href='#SkPaint_computeFastBounds_storage'>is</a> <a href='#SkPaint_computeFastBounds_storage'>legal</a> <a href='#SkPaint_computeFastBounds_storage'>for</a> <a href='#SkPaint_computeFastBounds_orig'>orig</a> <a href='#SkPaint_computeFastBounds_orig'>and</a> <a href='#SkPaint_computeFastBounds_storage'>storage</a> <a href='#SkPaint_computeFastBounds_storage'>to</a> <a href='#SkPaint_computeFastBounds_storage'>be</a> <a href='#SkPaint_computeFastBounds_storage'>the</a> <a href='#SkPaint_computeFastBounds_storage'>same</a>
<a href='SkRect_Reference#Rect'>Rect</a>.

Private: For example:
    if (!path.isInverseFillType() && paint.canComputeFastBounds()) {
        SkRect storage;
        if (canvas->quickReject(paint.computeFastBounds(path.getBounds(), &storage))) {
            return; // do not draw the path
        }
    }
    // draw the path

### Parameters

<table>  <tr>    <td><a name='SkPaint_computeFastBounds_orig'><code><strong>orig</strong></code></a></td>
    <td>geometry modified by <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>when</a> <a href='SkPaint_Reference#Paint'>drawn</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_computeFastBounds_storage'><code><strong>storage</strong></code></a></td>
    <td>computed bounds of geometry; may not be nullptr</td>
  </tr>
</table>

### Return Value

fast computed bounds

<a name='SkPaint_computeFastStrokeBounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkPaint_computeFastStrokeBounds'>computeFastStrokeBounds</a>(<a href='#SkPaint_computeFastStrokeBounds'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>orig</a>, <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>storage</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Private: (to be made private)

### Parameters

<table>  <tr>    <td><a name='SkPaint_computeFastStrokeBounds_orig'><code><strong>orig</strong></code></a></td>
    <td>geometry modified by <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>when</a> <a href='SkPaint_Reference#Paint'>drawn</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_computeFastStrokeBounds_storage'><code><strong>storage</strong></code></a></td>
    <td>computed bounds of geometry</td>
  </tr>
</table>

### Return Value

fast computed bounds

<a name='SkPaint_doComputeFastBounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkPaint_doComputeFastBounds'>doComputeFastBounds</a>(<a href='#SkPaint_doComputeFastBounds'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>orig</a>, <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>storage</a>, <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>style</a>) <a href='#SkPaint_Style'>const</a>
</pre>

Private: (to be made private)

Computes the bounds, overriding the <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkPaint_Style'>Style</a>. <a href='#SkPaint_Style'>This</a> <a href='#SkPaint_Style'>can</a> <a href='#SkPaint_Style'>be</a> <a href='#SkPaint_Style'>used</a> <a href='#SkPaint_Style'>to</a>
<a href='#SkPaint_Style'>account</a> <a href='#SkPaint_Style'>for</a> <a href='#SkPaint_Style'>additional</a> <a href='#SkPaint_Style'>width</a> <a href='#SkPaint_Style'>required</a> <a href='#SkPaint_Style'>by</a> <a href='#SkPaint_Style'>stroking</a> <a href='#SkPaint_doComputeFastBounds_orig'>orig</a>, <a href='#SkPaint_doComputeFastBounds_orig'>without</a>
<a href='#SkPaint_doComputeFastBounds_orig'>altering</a> <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>set</a> <a href='#SkPaint_Style'>to</a> <a href='#SkPaint_Style'>fill</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_doComputeFastBounds_orig'><code><strong>orig</strong></code></a></td>
    <td>geometry modified by <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>when</a> <a href='SkPaint_Reference#Paint'>drawn</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_doComputeFastBounds_storage'><code><strong>storage</strong></code></a></td>
    <td>computed bounds of geometry</td>
  </tr>
  <tr>    <td><a name='SkPaint_doComputeFastBounds_style'><code><strong>style</strong></code></a></td>
    <td>overrides <a href='#SkPaint_Style'>Style</a></td>
  </tr>
</table>

### Return Value

fast computed bounds

<a name='Utility'></a>

