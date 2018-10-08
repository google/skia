SkPaint Reference
===

<a name='SkPaint'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='#SkPaint'>SkPaint</a> {
public:
    <a href='#SkPaint_empty_constructor'>SkPaint()</a>;
    <a href='#SkPaint_copy_const_SkPaint'>SkPaint(const SkPaint& paint)</a>;
    <a href='#SkPaint_move_SkPaint'>SkPaint(SkPaint&& paint)</a>;
    <a href='#SkPaint_destructor'>~SkPaint()</a>;
    <a href='#SkPaint'>SkPaint</a>& <a href='#SkPaint_copy_operator'>operator=(const SkPaint& paint)</a>;
    <a href='#SkPaint'>SkPaint</a>& <a href='#SkPaint_move_operator'>operator=(SkPaint&& paint)</a>;
    friend bool <a href='#SkPaint_equal_operator'>operator==(const SkPaint& a, const SkPaint& b)</a>;
    friend bool <a href='#SkPaint_notequal_operator'>operator!=(const SkPaint& a, const SkPaint& b)</a>;
    uint32_t <a href='#SkPaint_getHash'>getHash</a>() const;
    void <a href='#SkPaint_reset'>reset</a>();

    enum <a href='#SkPaint_Hinting'>Hinting</a> {
        <a href='#SkPaint_kNo_Hinting'>kNo_Hinting</a> = 0,
        <a href='#SkPaint_kSlight_Hinting'>kSlight_Hinting</a> = 1,
        <a href='#SkPaint_kNormal_Hinting'>kNormal_Hinting</a> = 2,
        <a href='#SkPaint_kFull_Hinting'>kFull_Hinting</a> = 3,
    };

    <a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_getHinting'>getHinting</a>() const;
    void <a href='#SkPaint_setHinting'>setHinting</a>(<a href='#SkPaint_Hinting'>Hinting</a> hintingLevel);

    enum <a href='#SkPaint_Flags'>Flags</a> {
        <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a> = 0x01,
        <a href='#SkPaint_kDither_Flag'>kDither_Flag</a> = 0x04,
        <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a> = 0x20,
        <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a> = 0x40,
        <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a> = 0x80,
        <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a> = 0x200,
        <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> = 0x400,
        <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a> = 0x800,
        <a href='#SkPaint_kVerticalText_Flag'>kVerticalText_Flag</a> = 0x1000,
        <a href='#SkPaint_kAllFlags'>kAllFlags</a> = 0xFFFF,
    };

    enum <a href='#SkPaint_ReserveFlags'>ReserveFlags</a> {
        <a href='#SkPaint_kUnderlineText_ReserveFlag'>kUnderlineText_ReserveFlag</a> = 0x08,
        <a href='#SkPaint_kStrikeThruText_ReserveFlag'>kStrikeThruText_ReserveFlag</a> = 0x10,
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
    bool <a href='#SkPaint_isVerticalText'>isVerticalText</a>() const;
    void <a href='#SkPaint_setVerticalText'>setVerticalText</a>(bool verticalText);
    bool <a href='#SkPaint_isFakeBoldText'>isFakeBoldText</a>() const;
    void <a href='#SkPaint_setFakeBoldText'>setFakeBoldText</a>(bool fakeBoldText);
    bool <a href='#SkPaint_isDevKernText'>isDevKernText</a>() const;
    void <a href='#SkPaint_setDevKernText'>setDevKernText</a>(bool);
    <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='#SkPaint_getFilterQuality'>getFilterQuality</a>() const;
    void <a href='#SkPaint_setFilterQuality'>setFilterQuality</a>(<a href='undocumented#SkFilterQuality'>SkFilterQuality</a> quality);

    enum <a href='#SkPaint_Style'>Style</a> {
        <a href='#SkPaint_kFill_Style'>kFill_Style</a>,
        <a href='#SkPaint_kStroke_Style'>kStroke_Style</a>,
        <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>,
    };

    static constexpr int <a href='#SkPaint_kStyleCount'>kStyleCount</a> = <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a> + 1;

    <a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_getStyle'>getStyle</a>() const;
    void <a href='#SkPaint_setStyle'>setStyle</a>(<a href='#SkPaint_Style'>Style</a> style);
    <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPaint_getColor'>getColor</a>() const;
    <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a> <a href='#SkPaint_getColor4f'>getColor4f</a>() const;
    void <a href='#SkPaint_setColor'>setColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> color);
    void <a href='#SkPaint_setColor4f'>setColor4f</a>(const <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>& color, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* colorSpace);
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

    static constexpr int <a href='#SkPaint_kCapCount'>kCapCount</a> = <a href='#SkPaint_kLast_Cap'>kLast_Cap</a> + 1;

    enum <a href='#SkPaint_Join'>Join</a> {
        <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>,
        <a href='#SkPaint_kRound_Join'>kRound_Join</a>,
        <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>,
        <a href='#SkPaint_kLast_Join'>kLast_Join</a> = <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>,
        <a href='#SkPaint_kDefault_Join'>kDefault_Join</a> = <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>,
    };

    static constexpr int <a href='#SkPaint_kJoinCount'>kJoinCount</a> = <a href='#SkPaint_kLast_Join'>kLast_Join</a> + 1;

    <a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_getStrokeCap'>getStrokeCap</a>() const;
    void <a href='#SkPaint_setStrokeCap'>setStrokeCap</a>(<a href='#SkPaint_Cap'>Cap</a> cap);
    <a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a>() const;
    void <a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a>(<a href='#SkPaint_Join'>Join</a> join);
    bool <a href='#SkPaint_getFillPath'>getFillPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& src, <a href='SkPath_Reference#SkPath'>SkPath</a>* dst, const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect,
                     <a href='undocumented#SkScalar'>SkScalar</a> resScale = 1) const;
    bool <a href='#SkPaint_getFillPath_2'>getFillPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& src, <a href='SkPath_Reference#SkPath'>SkPath</a>* dst) const;
    <a href='undocumented#SkShader'>SkShader</a>* <a href='#SkPaint_getShader'>getShader</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkShader'>SkShader</a>> <a href='#SkPaint_refShader'>refShader</a>() const;
    void <a href='#SkPaint_setShader'>setShader</a>(sk_sp<<a href='undocumented#SkShader'>SkShader</a>> shader);
    <a href='undocumented#SkColorFilter'>SkColorFilter</a>* <a href='#SkPaint_getColorFilter'>getColorFilter</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorFilter'>SkColorFilter</a>> <a href='#SkPaint_refColorFilter'>refColorFilter</a>() const;
    void <a href='#SkPaint_setColorFilter'>setColorFilter</a>(sk_sp<<a href='undocumented#SkColorFilter'>SkColorFilter</a>> colorFilter);
    <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkPaint_getBlendMode'>getBlendMode</a>() const;
    bool <a href='#SkPaint_isSrcOver'>isSrcOver</a>() const;
    void <a href='#SkPaint_setBlendMode'>setBlendMode</a>(<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode);
    <a href='undocumented#SkPathEffect'>SkPathEffect</a>* <a href='#SkPaint_getPathEffect'>getPathEffect</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkPathEffect'>SkPathEffect</a>> <a href='#SkPaint_refPathEffect'>refPathEffect</a>() const;
    void <a href='#SkPaint_setPathEffect'>setPathEffect</a>(sk_sp<<a href='undocumented#SkPathEffect'>SkPathEffect</a>> pathEffect);
    <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>* <a href='#SkPaint_getMaskFilter'>getMaskFilter</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>> <a href='#SkPaint_refMaskFilter'>refMaskFilter</a>() const;
    void <a href='#SkPaint_setMaskFilter'>setMaskFilter</a>(sk_sp<<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>> maskFilter);
    <a href='undocumented#SkTypeface'>SkTypeface</a>* <a href='#SkPaint_getTypeface'>getTypeface</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkTypeface'>SkTypeface</a>> <a href='#SkPaint_refTypeface'>refTypeface</a>() const;
    void <a href='#SkPaint_setTypeface'>setTypeface</a>(sk_sp<<a href='undocumented#SkTypeface'>SkTypeface</a>> typeface);
    <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='#SkPaint_getImageFilter'>getImageFilter</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkImageFilter'>SkImageFilter</a>> <a href='#SkPaint_refImageFilter'>refImageFilter</a>() const;
    void <a href='#SkPaint_setImageFilter'>setImageFilter</a>(sk_sp<<a href='undocumented#SkImageFilter'>SkImageFilter</a>> imageFilter);
    <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>* <a href='#SkPaint_getDrawLooper'>getDrawLooper</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>> <a href='#SkPaint_refDrawLooper'>refDrawLooper</a>() const;
    <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>* <a href='#SkPaint_getLooper'>getLooper</a>() const;
    void <a href='#SkPaint_setDrawLooper'>setDrawLooper</a>(sk_sp<<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>> drawLooper);
    void <a href='#SkPaint_setLooper'>setLooper</a>(sk_sp<<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>> drawLooper);

    enum <a href='#SkPaint_Align'>Align</a> {
        <a href='#SkPaint_kLeft_Align'>kLeft_Align</a>,
        <a href='#SkPaint_kCenter_Align'>kCenter_Align</a>,
        <a href='#SkPaint_kRight_Align'>kRight_Align</a>,
    };

    static constexpr int <a href='#SkPaint_kAlignCount'>kAlignCount</a> = 3;

    <a href='#SkPaint_Align'>Align</a> <a href='#SkPaint_getTextAlign'>getTextAlign</a>() const;
    void <a href='#SkPaint_setTextAlign'>setTextAlign</a>(<a href='#SkPaint_Align'>Align</a> align);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSize'>getTextSize</a>() const;
    void <a href='#SkPaint_setTextSize'>setTextSize</a>(<a href='undocumented#SkScalar'>SkScalar</a> textSize);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextScaleX'>getTextScaleX</a>() const;
    void <a href='#SkPaint_setTextScaleX'>setTextScaleX</a>(<a href='undocumented#SkScalar'>SkScalar</a> scaleX);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSkewX'>getTextSkewX</a>() const;
    void <a href='#SkPaint_setTextSkewX'>setTextSkewX</a>(<a href='undocumented#SkScalar'>SkScalar</a> skewX);

    enum <a href='#SkPaint_TextEncoding'>TextEncoding</a> {
        <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>,
        <a href='#SkPaint_kUTF16_TextEncoding'>kUTF16_TextEncoding</a>,
        <a href='#SkPaint_kUTF32_TextEncoding'>kUTF32_TextEncoding</a>,
        <a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID_TextEncoding</a>,
    };

    <a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_getTextEncoding'>getTextEncoding</a>() const;
    void <a href='#SkPaint_setTextEncoding'>setTextEncoding</a>(<a href='#SkPaint_TextEncoding'>TextEncoding</a> encoding);

    struct <a href='#SkPaint_FontMetrics'>FontMetrics</a> {

        enum <a href='#SkPaint_FontMetrics_FontMetricsFlags'>FontMetricsFlags</a> {
            <a href='#SkPaint_FontMetrics_kUnderlineThicknessIsValid_Flag'>kUnderlineThicknessIsValid_Flag</a> = 1 << 0,
            <a href='#SkPaint_FontMetrics_kUnderlinePositionIsValid_Flag'>kUnderlinePositionIsValid_Flag</a> = 1 << 1,
            <a href='#SkPaint_FontMetrics_kStrikeoutThicknessIsValid_Flag'>kStrikeoutThicknessIsValid_Flag</a> = 1 << 2,
            <a href='#SkPaint_FontMetrics_kStrikeoutPositionIsValid_Flag'>kStrikeoutPositionIsValid_Flag</a> = 1 << 3,
        };

        uint32_t <a href='#SkPaint_FontMetrics_fFlags'>fFlags</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fTop'>fTop</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fAscent'>fAscent</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fDescent'>fDescent</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fBottom'>fBottom</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fLeading'>fLeading</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fAvgCharWidth'>fAvgCharWidth</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fMaxCharWidth'>fMaxCharWidth</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fXMin'>fXMin</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fXMax'>fXMax</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fXHeight'>fXHeight</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fCapHeight'>fCapHeight</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fUnderlineThickness'>fUnderlineThickness</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fUnderlinePosition'>fUnderlinePosition</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fStrikeoutThickness'>fStrikeoutThickness</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_FontMetrics_fStrikeoutPosition'>fStrikeoutPosition</a>;
        bool <a href='#SkPaint_FontMetrics_hasUnderlineThickness'>hasUnderlineThickness</a>(<a href='undocumented#SkScalar'>SkScalar</a>* thickness) const;
        bool <a href='#SkPaint_FontMetrics_hasUnderlinePosition'>hasUnderlinePosition</a>(<a href='undocumented#SkScalar'>SkScalar</a>* position) const;
        bool <a href='#SkPaint_FontMetrics_hasStrikeoutThickness'>hasStrikeoutThickness</a>(<a href='undocumented#SkScalar'>SkScalar</a>* thickness) const;
        bool <a href='#SkPaint_FontMetrics_hasStrikeoutPosition'>hasStrikeoutPosition</a>(<a href='undocumented#SkScalar'>SkScalar</a>* position) const;
    };

    <a href='undocumented#SkScalar'>SkScalar</a> getFontMetrics(FontMetrics* metrics, SkScalar scale = 0) const;
    <a href='undocumented#SkScalar'>SkScalar</a> getFontSpacing() const;
    int textToGlyphs(const void* text, size_t byteLength,
                     SkGlyphID glyphs[]) const;
    bool containsText(const void* text, size_t byteLength) const;
    void glyphsToUnichars(const SkGlyphID glyphs[], int count, SkUnichar text[]) const;
    int countText(const void* text, size_t byteLength) const;
    <a href='undocumented#SkScalar'>SkScalar</a> measureText(const void* text, size_t length, SkRect* bounds) const;
    <a href='undocumented#SkScalar'>SkScalar</a> measureText(const void* text, size_t length) const;
    size_t breakText(const void* text, size_t length, SkScalar maxWidth,
                      SkScalar* measuredWidth = nullptr) const;
    int getTextWidths(const void* text, size_t byteLength, SkScalar widths[],
                      SkRect bounds[] = nullptr) const;
    void getTextPath(const void* text, size_t length, SkScalar x, SkScalar y,
                     SkPath* path) const;
    void getPosTextPath(const void* text, size_t length,
                        const SkPoint pos[], SkPath* path) const;
    int getTextIntercepts(const void* text, size_t length, SkScalar x, SkScalar y,
                          const SkScalar bounds[2], SkScalar* intervals) const;
    int getPosTextIntercepts(const void* text, size_t length, const SkPoint pos[],
                             const SkScalar bounds[2], SkScalar* intervals) const;
    int getPosTextHIntercepts(const void* text, size_t length, const SkScalar xpos[],
                              SkScalar constY, const SkScalar bounds[2], SkScalar* intervals) const;
    int getTextBlobIntercepts(const SkTextBlob* blob, const SkScalar bounds[2],
                              SkScalar* intervals) const;
    <a href='SkRect_Reference#SkRect'>SkRect</a> getFontBounds() const;
    bool nothingToDraw() const;
    bool canComputeFastBounds() const;
    const <a href='SkRect_Reference#SkRect'>SkRect</a>& computeFastBounds(const SkRect& orig, SkRect* storage) const;
    const <a href='SkRect_Reference#SkRect'>SkRect</a>& computeFastStrokeBounds(const SkRect& orig,
                                          SkRect* storage) const;
    const <a href='SkRect_Reference#SkRect'>SkRect</a>& doComputeFastBounds(const SkRect& orig, SkRect* storage,
                                      Style style) const;
};
</pre>

<a href='#Paint'>Paint</a> controls options applied when drawing and measuring

## <a name='Initializers'>Initializers</a>

<a name='SkPaint_empty_constructor'></a>
## SkPaint

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint'>SkPaint</a>(
</pre>

Constructs <a href='#Paint'>Paint</a> with default values

| attribute | default value |
| --- | ---  |
| <a href='#Anti_Alias'>Anti Alias</a> | false |
| <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> | <a href='SkBlendMode_Reference#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a> |
| <a href='SkColor_Reference#Color'>Color</a> | <a href='SkColor_Reference#SK_ColorBLACK'>SK ColorBLACK</a> |
| <a href='SkColor_Reference#Alpha'>Color Alpha</a> | 255 |
| <a href='undocumented#Color_Filter'>Color Filter</a> | nullptr |
| <a href='#Dither'>Dither</a> | false |
| <a href='undocumented#Draw_Looper'>Draw Looper</a> | nullptr |
| <a href='#Fake_Bold'>Fake Bold</a> | false |
| <a href='undocumented#Filter_Quality'>Filter Quality</a> | <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a> |
| <a href='#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a> | false |
| <a href='#Automatic_Hinting'>Automatic Hinting</a> | false |
| <a href='#Full_Hinting_Spacing'>Full Hinting Spacing</a> | false |
| <a href='#SkPaint_Hinting'>Hinting</a> | <a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a> |
| <a href='undocumented#Image_Filter'>Image Filter</a> | nullptr |
| <a href='#LCD_Text'>LCD Text</a> | false |
| <a href='#Linear_Text'>Linear Text</a> | false |
| <a href='#Miter_Limit'>Miter Limit</a> | 4 |
| <a href='undocumented#Mask_Filter'>Mask Filter</a> | nullptr |
| <a href='undocumented#Path_Effect'>Path Effect</a> | nullptr |
| <a href='undocumented#Shader'>Shader</a> | nullptr |
| <a href='#SkPaint_Style'>Style</a> | <a href='#SkPaint_kFill_Style'>kFill Style</a> |
| <a href='#Text_Align'>Text Align</a> | <a href='#SkPaint_kLeft_Align'>kLeft Align</a> |
| <a href='#Text_Encoding'>Text Encoding</a> | <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8 TextEncoding</a> |
| <a href='#Text_Scale_X'>Text Scale X</a> | 1 |
| <a href='#Text_Size'>Text Size</a> | 12 |
| <a href='#Text_Skew_X'>Text Skew X</a> | 0 |
| <a href='undocumented#Typeface'>Typeface</a> | nullptr |
| <a href='#Stroke_Cap'>Stroke Cap</a> | <a href='#SkPaint_kButt_Cap'>kButt Cap</a> |
| <a href='#Stroke_Join'>Stroke Join</a> | <a href='#SkPaint_kMiter_Join'>kMiter Join</a> |
| <a href='#Stroke_Width'>Stroke Width</a> | 0 |
| <a href='#Subpixel_Text'>Subpixel Text</a> | false |
| <a href='#Vertical_Text'>Vertical Text</a> | false |

The flags

### Return Value

default initialized <a href='#Paint'>Paint</a>

### Example

<div><fiddle-embed name="c4b2186d85c142a481298f7144295ffd"></fiddle-embed></div>

---

<a name='SkPaint_copy_const_SkPaint'></a>
## SkPaint

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint'>SkPaint</a>(const <a href='#SkPaint'>SkPaint</a>
</pre>

Makes a shallow copy of <a href='#Paint'>Paint</a>

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

---

<a name='SkPaint_move_SkPaint'></a>
## SkPaint

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint'>SkPaint</a>(<a href='#SkPaint'>SkPaint</a>
</pre>

Implements a move constructor to avoid increasing the reference counts
of objects referenced by the <a href='#SkPaint_move_SkPaint_paint'>paint</a>

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

---

<a name='SkPaint_reset'></a>
## reset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_reset'>reset</a>(
</pre>

Sets all <a href='#Paint'>Paint</a> contents to their initial values

### Example

<div><fiddle-embed name="ef269937ade7e7353635121d9a64f9f7">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_destructor'></a>
## ~SkPaint

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_destructor'>~SkPaint</a>(
</pre>

Decreases <a href='#Paint'>Paint</a> <a href='undocumented#Reference_Count'>Reference Count</a> of owned objects

---

## <a name='Management'>Management</a>

<a name='SkPaint_copy_operator'></a>
## operator=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint'>SkPaint</a>
</pre>

Makes a shallow copy of <a href='#Paint'>Paint</a>

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

---

<a name='SkPaint_move_operator'></a>
## operator=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint'>SkPaint</a>
</pre>

Moves the <a href='#SkPaint_move_operator_paint'>paint</a> to avoid increasing the reference counts
of objects referenced by the <a href='#SkPaint_move_operator_paint'>paint</a> parameter

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

---

<a name='SkPaint_equal_operator'></a>
## operator==

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_equal_operator'>operator==(const SkPaint& a, const SkPaint& b)</a>
</pre>

Compares <a href='#SkPaint_equal_operator_a'>a</a> and <a href='#SkPaint_equal_operator_b'>b</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#Paint'>Paint</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkPaint_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#Paint'>Paint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#Paint'>Paint</a> pair are equivalent

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

---

<a name='SkPaint_notequal_operator'></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_notequal_operator'>operator!=(const SkPaint& a, const SkPaint& b)</a>
</pre>

Compares <a href='#SkPaint_notequal_operator_a'>a</a> and <a href='#SkPaint_notequal_operator_b'>b</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#Paint'>Paint</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkPaint_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#Paint'>Paint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#Paint'>Paint</a> pair are not equivalent

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

---

<a name='SkPaint_getHash'></a>
## getHash

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPaint_getHash'>getHash</a>(
</pre>

Returns a hash generated from <a href='#Paint'>Paint</a> values and pointers

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

---

## <a name='Hinting'>Hinting</a>

## <a name='SkPaint_Hinting'>Enum SkPaint::Hinting</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Hinting'>Hinting</a> {
        <a href='#SkPaint_kNo_Hinting'>kNo_Hinting</a>            = 0,
        <a href='#SkPaint_kSlight_Hinting'>kSlight_Hinting</a>        = 1,
        <a href='#SkPaint_kNormal_Hinting'>kNormal_Hinting</a>        = 2,
        <a href='#SkPaint_kFull_Hinting'>kFull_Hinting</a>          = 3,
    };
</pre>

<a href='#SkPaint_Hinting'>Hinting</a> adjusts the glyph outlines so that the shape provides a uniform
look at a given point size on font engines that support it

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kNo_Hinting'><code>SkPaint::kNo_Hinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Leaves glyph outlines unchanged from their native representation</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kSlight_Hinting'><code>SkPaint::kSlight_Hinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Modifies glyph outlines minimally to improve constrast</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kNormal_Hinting'><code>SkPaint::kNormal_Hinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Modifies glyph outlines to improve constrast</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kFull_Hinting'><code>SkPaint::kFull_Hinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Modifies glyph outlines for maximum constrast</td>
  </tr>
</table>

On <a href='undocumented#OS_X'>OS X</a> and iOS

<a name='SkPaint_getHinting'></a>
## getHinting

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_getHinting'>getHinting</a>(
</pre>

Returns level of glyph outline adjustment

### Return Value

one of

### Example

<div><fiddle-embed name="329e2e5a5919ac431e1c58878a5b99e0">

#### Example Output

~~~~
SkPaint::kNormal_Hinting == paint.getHinting()
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setHinting'></a>
## setHinting

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setHinting'>setHinting</a>(<a href='#SkPaint_Hinting'>Hinting</a> hintingLevel
</pre>

Sets level of glyph outline adjustment

| <a href='#SkPaint_Hinting'>Hinting</a> | value | effect on generated glyph outlines |
| --- | --- | ---  |
| <a href='#SkPaint_kNo_Hinting'>kNo Hinting</a> | 0 | leaves glyph outlines unchanged from their native representation |
| <a href='#SkPaint_kSlight_Hinting'>kSlight Hinting</a> | 1 | modifies glyph outlines minimally to improve contrast |
| <a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a> | 2 | modifies glyph outlines to improve contrast |
| <a href='#SkPaint_kFull_Hinting'>kFull Hinting</a> | 3 | modifies glyph outlines for maximum contrast |

### Parameters

<table>  <tr>    <td><a name='SkPaint_setHinting_hintingLevel'><code><strong>hintingLevel</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="78153fbd3f1000cb33b97bbe831ed34e">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

## <a name='Flags'>Flags</a>

## <a name='SkPaint_Flags'>Enum SkPaint::Flags</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Flags'>Flags</a> {
        <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias_Flag</a>       = 0x01,
        <a href='#SkPaint_kDither_Flag'>kDither_Flag</a>          = 0x04,
        <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText_Flag</a>    = 0x20,
        <a href='#SkPaint_kLinearText_Flag'>kLinearText_Flag</a>      = 0x40,
        <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText_Flag</a>    = 0x80,
        <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText_Flag</a>   = 0x200,
        <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText_Flag</a> = 0x400,
        <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting_Flag</a>     = 0x800,
        <a href='#SkPaint_kVerticalText_Flag'>kVerticalText_Flag</a>    = 0x1000,

        <a href='#SkPaint_kAllFlags'>kAllFlags</a> = 0xFFFF,
    };

</pre>

The bit values stored in <a href='#SkPaint_Flags'>Flags</a>

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
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kVerticalText_Flag'><code>SkPaint::kVerticalText_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x1000</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask for setting Vertical_Text</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kAllFlags'><code>SkPaint::kAllFlags</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
mask of all <a href='#SkPaint_Flags'>Flags</a></td>
  </tr>
<a href='#SkPaint_Flags'>Flags</a> default to all flags clear</table>

## <a name='SkPaint_ReserveFlags'>Enum SkPaint::ReserveFlags</a>

To be deprecated soon.

Only valid for Android framework

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_ReserveFlags'>ReserveFlags</a> {
        <a href='#SkPaint_kUnderlineText_ReserveFlag'>kUnderlineText_ReserveFlag</a>   = 0x08,
        <a href='#SkPaint_kStrikeThruText_ReserveFlag'>kStrikeThruText_ReserveFlag</a>  = 0x10,
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
## getFlags

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPaint_getFlags'>getFlags</a>(
</pre>

Returns paint settings described by <a href='#SkPaint_Flags'>Flags</a>

### Return Value

zero

### Example

<div><fiddle-embed name="8a3f8c309533388b01aa66e1267f322d">

#### Example Output

~~~~
(SkPaint::kAntiAlias_Flag & paint.getFlags()) != 0
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setFlags'></a>
## setFlags

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setFlags'>setFlags</a>(uint32_t flags
</pre>

Replaces <a href='#SkPaint_Flags'>Flags</a> with <a href='#SkPaint_setFlags_flags'>flags</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setFlags_flags'><code><strong>flags</strong></code></a></td>
    <td>union of <a href='#SkPaint_Flags'>Flags</a> for <a href='#Paint'>Paint</a></td>
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

---

## <a name='Anti_Alias'>Anti Alias</a>

<a href='#Anti_Alias'>Anti Alias</a> drawing approximates partial pixel coverage with transparencyA platform may only support <a href='#Anti_Alias'>Anti Aliased</a> drawing

### Example

<div><fiddle-embed name="a6575a49467ce8d28bb01cc7638fa04d"><div>A red line is drawn with transparency on the edges to make it look smoother</div></fiddle-embed></div>

<a name='SkPaint_isAntiAlias'></a>
## isAntiAlias

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isAntiAlias'>isAntiAlias</a>(
</pre>

Returns true if pixels on the active edges of <a href='SkPath_Reference#Path'>Path</a> may be drawn with partial transparency

### Return Value

<a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias Flag</a> state

### Example

<div><fiddle-embed name="d7d5f4f7da7acd5104a652f490c6f7b8">

#### Example Output

~~~~
paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)
paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setAntiAlias'></a>
## setAntiAlias

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setAntiAlias'>setAntiAlias</a>(bool aa
</pre>

Requests

### Parameters

<table>  <tr>    <td><a name='SkPaint_setAntiAlias_aa'><code><strong>aa</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c2ff148374d01cbef845b223e725905c">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

## <a name='Dither'>Dither</a>

<a href='#Dither'>Dither</a> increases fidelity by adjusting the color of adjacent pixels <code>SK_IGNORE_GPU_DITHER</code>to ignore <a href='#Dither'>Dither</a> on <a href='undocumented#GPU_Surface'>GPU Surface</a>

### Example

<div><fiddle-embed name="8b26507690b71462f44642b911890bbf"><div>Dithering in the bottom half more closely approximates the requested color by
alternating nearby colors from pixel to pixel</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="76d4d4a7931a48495e4d5f54e073be53"><div>Dithering introduces subtle adjustments to color to smooth gradients</div></fiddle-embed></div>

<a name='SkPaint_isDither'></a>
## isDither

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isDither'>isDither</a>(
</pre>

Returns true if color error may be distributed to smooth color transition

### Return Value

<a href='#SkPaint_kDither_Flag'>kDither Flag</a> state

### Example

<div><fiddle-embed name="f4ce93f6c5e7335436a985377fd980c0">

#### Example Output

~~~~
paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag)
paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag)
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setDither'></a>
## setDither

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setDither'>setDither</a>(bool dither
</pre>

Requests

### Parameters

<table>  <tr>    <td><a name='SkPaint_setDither_dither'><code><strong>dither</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kDither_Flag'>kDither Flag</a></td>
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

---

### See Also

Gradient <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>

## <a name='Device_Text'>Device Text</a>

<a href='#LCD_Text'>LCD Text</a> and <a href='#Subpixel_Text'>Subpixel Text</a> increase the precision of glyph position

### Example

<div><fiddle-embed name="4606ae1be792d6bc46d496432f050ee9"><div>Four commas are drawn normally and with combinations of <a href='#LCD_Text'>LCD Text</a> and <a href='#Subpixel_Text'>Subpixel Text</a></div></fiddle-embed></div>

## <a name='Linear_Text'>Linear Text</a>

<a href='#Linear_Text'>Linear Text</a> selects whether text is rendered as a <a href='undocumented#Glyph'>Glyph</a> or as a <a href='SkPath_Reference#Path'>Path</a>

<a name='SkPaint_isLinearText'></a>
## isLinearText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isLinearText'>isLinearText</a>(
</pre>

Returns true if text is converted to <a href='SkPath_Reference#Path'>Path</a> before drawing and measuring

### Return Value

<a href='#SkPaint_kLinearText_Flag'>kLinearText Flag</a> state

### Example

<div><fiddle-embed name="2890ad644f980637837e6fcb386fb462"></fiddle-embed></div>

### See Also

<a href='#SkPaint_setLinearText'>setLinearText</a> <a href='#SkPaint_Hinting'>Hinting</a>

---

<a name='SkPaint_setLinearText'></a>
## setLinearText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setLinearText'>setLinearText</a>(bool linearText
</pre>

Returns true if text is converted to <a href='SkPath_Reference#Path'>Path</a> before drawing and measuring

### Parameters

<table>  <tr>    <td><a name='SkPaint_setLinearText_linearText'><code><strong>linearText</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kLinearText_Flag'>kLinearText Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c93bb912f3bddfb4d96d3ad70ada552b"></fiddle-embed></div>

### See Also

<a href='#SkPaint_isLinearText'>isLinearText</a> <a href='#SkPaint_Hinting'>Hinting</a>

---

## <a name='Subpixel_Text'>Subpixel Text</a>

<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText Flag</a> uses the pixel transparency to represent a fractional offset

<a name='SkPaint_isSubpixelText'></a>
## isSubpixelText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isSubpixelText'>isSubpixelText</a>(
</pre>

Returns true if <a href='undocumented#Glyph'>Glyphs</a> at different sub

### Return Value

<a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText Flag</a> state

### Example

<div><fiddle-embed name="abe9afc0932e2199324ae6cbb396e67c">

#### Example Output

~~~~
paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)
paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setSubpixelText'></a>
## setSubpixelText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setSubpixelText'>setSubpixelText</a>(bool subpixelText
</pre>

Requests

### Parameters

<table>  <tr>    <td><a name='SkPaint_setSubpixelText_subpixelText'><code><strong>subpixelText</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a77bbc1a4e3be9a8ab0f842f877c5ee4">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

## <a name='LCD_Text'>LCD Text</a>

When set

<a name='SkPaint_isLCDRenderText'></a>
## isLCDRenderText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isLCDRenderText'>isLCDRenderText</a>(
</pre>

Returns true if <a href='undocumented#Glyph'>Glyphs</a> may use LCD striping to improve glyph edges

### Return Value

<a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText Flag</a> state

### Example

<div><fiddle-embed name="68e1fd95dd2fd06a333899d2bd2396b9">

#### Example Output

~~~~
paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)
paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setLCDRenderText'></a>
## setLCDRenderText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setLCDRenderText'>setLCDRenderText</a>(bool lcdText
</pre>

Requests

### Parameters

<table>  <tr>    <td><a name='SkPaint_setLCDRenderText_lcdText'><code><strong>lcdText</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="50dedf8450159571a3edaf4f0050defe">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

## <a name='Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a>

<a href='#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a> allows selecting custom sized bitmap <a href='undocumented#Glyph'>Glyphs</a>

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
## isEmbeddedBitmapText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isEmbeddedBitmapText'>isEmbeddedBitmapText</a>(
</pre>

Returns true if <a href='undocumented#Engine'>Font Engine</a> may return <a href='undocumented#Glyph'>Glyphs</a> from font bitmaps instead of from outlines

### Return Value

<a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText Flag</a> state

### Example

<div><fiddle-embed name="eba10b27b790e87183ae451b3fc5c4b1">

#### Example Output

~~~~
paint.isEmbeddedBitmapText() == !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)
paint.isEmbeddedBitmapText() == !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setEmbeddedBitmapText'></a>
## setEmbeddedBitmapText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setEmbeddedBitmapText'>setEmbeddedBitmapText</a>(bool useEmbeddedBitmapText
</pre>

Requests

### Parameters

<table>  <tr>    <td><a name='SkPaint_setEmbeddedBitmapText_useEmbeddedBitmapText'><code><strong>useEmbeddedBitmapText</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="246dffdd93a484ba4ad7ecf71198a5d4">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

## <a name='Automatic_Hinting'>Automatic Hinting</a>

If <a href='#SkPaint_Hinting'>Hinting</a> is set to <a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a> or <a href='#SkPaint_kFull_Hinting'>kFull Hinting</a>

<a name='SkPaint_isAutohinted'></a>
## isAutohinted

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isAutohinted'>isAutohinted</a>(
</pre>

Returns true if <a href='#SkPaint_Hinting'>Hinting</a> is set to <a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a> or <a href='#SkPaint_kFull_Hinting'>kFull Hinting</a>

### Return Value

<a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting Flag</a> state

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

---

<a name='SkPaint_setAutohinted'></a>
## setAutohinted

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setAutohinted'>setAutohinted</a>(bool useAutohinter
</pre>

Sets whether to always hint <a href='undocumented#Glyph'>Glyphs</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setAutohinted_useAutohinter'><code><strong>useAutohinter</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4e185306d7de9390fe8445eed0139309"></fiddle-embed></div>

### See Also

<a href='#SkPaint_isAutohinted'>isAutohinted</a> <a href='#SkPaint_Hinting'>Hinting</a>

---

## <a name='Vertical_Text'>Vertical Text</a>

<a href='undocumented#Text'>Text</a> may be drawn by positioning each glyph <a href='https://harfbuzz.org/'>HarfBuzz</a></a> to translate text runs
into glyph series

### Example

<div><fiddle-embed name="8df5800819311b71373d9abb669b49b8"></fiddle-embed></div>

<a name='SkPaint_isVerticalText'></a>
## isVerticalText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isVerticalText'>isVerticalText</a>(
</pre>

Returns true if <a href='undocumented#Glyph'>Glyphs</a> are drawn top to bottom instead of left to right

### Return Value

<a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> state

### Example

<div><fiddle-embed name="4a269b16e644d473870ffa873396f139">

#### Example Output

~~~~
paint.isVerticalText() == !!(paint.getFlags() & SkPaint::kVerticalText_Flag)
paint.isVerticalText() == !!(paint.getFlags() & SkPaint::kVerticalText_Flag)
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setVerticalText'></a>
## setVerticalText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setVerticalText'>setVerticalText</a>(bool verticalText
</pre>

Returns true if text advance positions the next glyph below the previous glyph instead of to the
right of previous glyph

### Parameters

<table>  <tr>    <td><a name='SkPaint_setVerticalText_verticalText'><code><strong>verticalText</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6fbd7e9e1a346cb8d7f537786009c736">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

## <a name='Fake_Bold'>Fake Bold</a>

<a href='#Fake_Bold'>Fake Bold</a> approximates the bold font style accompanying a normal font when a bold font face
is not available

### Example

<div><fiddle-embed name="e811f4829a2daaaeaad3795504a7e02a"></fiddle-embed></div>

<a name='SkPaint_isFakeBoldText'></a>
## isFakeBoldText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isFakeBoldText'>isFakeBoldText</a>(
</pre>

Returns true if approximate bold by increasing the stroke width when creating glyph bitmaps
from outlines

### Return Value

<a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText Flag</a> state

### Example

<div><fiddle-embed name="f54d1f85b16073b80b9eef2e1a1d151d">

#### Example Output

~~~~
paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)
paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setFakeBoldText'></a>
## setFakeBoldText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setFakeBoldText'>setFakeBoldText</a>(bool fakeBoldText
</pre>

Increases stroke width when creating glyph bitmaps to approximate a bold typeface

### Parameters

<table>  <tr>    <td><a name='SkPaint_setFakeBoldText_fakeBoldText'><code><strong>fakeBoldText</strong></code></a></td>
    <td>setting for <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="594d47858eb11028cb626515a520910a">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

## <a name='Full_Hinting_Spacing'>Full Hinting Spacing</a>

if <a href='#SkPaint_Hinting'>Hinting</a> is set to <a href='#SkPaint_kFull_Hinting'>kFull Hinting</a>

<a name='SkPaint_isDevKernText'></a>
## isDevKernText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isDevKernText'>isDevKernText</a>(
</pre>

Deprecated.

---

<a name='SkPaint_setDevKernText'></a>
## setDevKernText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setDevKernText'>setDevKernText</a>(bool
</pre>

Deprecated.

---

## <a name='Filter_Quality_Methods'>Filter Quality Methods</a>

<a href='undocumented#Filter_Quality'>Filter Quality</a> trades speed for image filtering when the image is scaled

<table>  <tr>
    <td><a href='SkCanvas_Reference#SkCanvas_drawBitmap'>SkCanvas::drawBitmap</a></td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#SkCanvas_drawBitmapRect'>SkCanvas::drawBitmapRect</a></td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#SkCanvas_drawImage'>SkCanvas::drawImage</a></td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#SkCanvas_drawImageRect'>SkCanvas::drawImageRect</a></td>
  </tr>
</table>

and when <a href='#Paint'>Paint</a> has a <a href='undocumented#Shader'>Shader</a> specialization that uses <a href='SkImage_Reference#Image'>Image</a> or <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>

### Example

<div><fiddle-embed name="69369cff2f5b145a6f616092513266a0"></fiddle-embed></div>

<a name='SkPaint_getFilterQuality'></a>
## getFilterQuality

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='#SkPaint_getFilterQuality'>getFilterQuality</a>(
</pre>

Returns <a href='undocumented#Filter_Quality'>Filter Quality</a>

### Return Value

one of

### Example

<div><fiddle-embed name="d4ca1f23809b6835c4ba46ea98a86900">

#### Example Output

~~~~
kNone_SkFilterQuality == paint.getFilterQuality()
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setFilterQuality'></a>
## setFilterQuality

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setFilterQuality'>setFilterQuality</a>(<a href='undocumented#SkFilterQuality'>SkFilterQuality</a> quality
</pre>

Sets <a href='undocumented#Filter_Quality'>Filter Quality</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setFilterQuality_quality'><code><strong>quality</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e4288fabf24ee60b645e8bb6ea0afadf">

#### Example Output

~~~~
kHigh_SkFilterQuality == paint.getFilterQuality()
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#Image_Scaling'>Image Scaling</a>

---

## <a name='Color_Methods'>Color Methods</a>

| name | description |
| --- | ---  |
| <a href='#SkPaint_getColor'>getColor</a> | returns <a href='SkColor_Reference#Alpha'>Color Alpha</a> and RGB |
| <a href='#SkPaint_setColor'>setColor</a> | sets <a href='SkColor_Reference#Alpha'>Color Alpha</a> and RGB |

<a href='SkColor_Reference#Color'>Color</a> specifies the red

| bit positions | <a href='SkColor_Reference#Alpha'>Color Alpha</a> | red | blue | green |
| --- | --- | --- | --- | ---  |
|  | 31 | 23 | 15 | 7 |

### Example

<div><fiddle-embed name="214b559d75c65a7bef6ef4be1f860053"></fiddle-embed></div>

<a name='SkPaint_getColor'></a>
## getColor

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPaint_getColor'>getColor</a>(
</pre>

Retrieves <a href='SkColor_Reference#Alpha'>Alpha</a> and RGB

### Return Value

<a href='undocumented#Unpremultiply'>Unpremultiplied</a> ARGB

### Example

<div><fiddle-embed name="72d41f890203109a41f589a7403acae9">

#### Example Output

~~~~
Yellow is 100% red, 100% green, and 0% blue.
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_getColor4f'>getColor4f</a> <a href='SkColor_Reference#SkColor'>SkColor</a>

---

<a name='SkPaint_getColor4f'></a>
## getColor4f

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a> <a href='#SkPaint_getColor4f'>getColor4f</a>(
</pre>

Retrieves alpha and RGB

### Return Value

<a href='undocumented#Unpremultiply'>Unpremultiplied</a> RGBA

### Example

<div><fiddle-embed name="8512ea2176f36e8f1aeef311ff228790">

#### Example Output

~~~~
Yellow is 100% red, 100% green, and 0% blue.
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_getColor'>getColor</a> <a href='SkColor_Reference#SkColor'>SkColor</a>

---

<a name='SkPaint_setColor'></a>
## setColor

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setColor'>setColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> color
</pre>

Sets <a href='SkColor_Reference#Alpha'>Alpha</a> and RGB used when stroking and filling

### Parameters

<table>  <tr>    <td><a name='SkPaint_setColor_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>Unpremultiplied</a> ARGB</td>
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

---

<a name='SkPaint_setColor4f'></a>
## setColor4f

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setColor4f'>setColor4f</a>(const <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>
</pre>

Sets alpha and RGB used when stroking and filling

### Parameters

<table>  <tr>    <td><a name='SkPaint_setColor4f_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>Unpremultiplied</a> RGBA</td>
  </tr>
  <tr>    <td><a name='SkPaint_setColor4f_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td><a href='undocumented#Color_Space'>Color Space</a> describing the encoding of <a href='#SkPaint_setColor4f_color'>color</a></td>
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

---

## <a name='Alpha_Methods'>Alpha Methods</a>

<a href='SkColor_Reference#Alpha'>Color Alpha</a> sets the transparency independent of RGB

<a name='SkPaint_getAlpha'></a>
## getAlpha

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint8_t <a href='#SkPaint_getAlpha'>getAlpha</a>(
</pre>

Retrieves <a href='SkColor_Reference#Alpha'>Alpha</a> from the <a href='SkColor_Reference#Color'>Color</a> used when stroking and filling

### Return Value

<a href='SkColor_Reference#Alpha'>Alpha</a> ranging from zero

### Example

<div><fiddle-embed name="9a85bb62fe3d877b18fb7f952c4fa7f7">

#### Example Output

~~~~
255 == paint.getAlpha()
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setAlpha'></a>
## setAlpha

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setAlpha'>setAlpha</a>(<a href='undocumented#U8CPU'>U8CPU</a> a
</pre>

Replaces <a href='SkColor_Reference#Alpha'>Alpha</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setAlpha_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkColor_Reference#Alpha'>Alpha</a> component of <a href='SkColor_Reference#Color'>Color</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6ddc0360512dfb9947e75c17e6a8103d">

#### Example Output

~~~~
0x44112233 == paint.getColor()
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setARGB'></a>
## setARGB

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setARGB'>setARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> a
</pre>

Sets <a href='SkColor_Reference#Color'>Color</a> used when drawing solid fills

### Parameters

<table>  <tr>    <td><a name='SkPaint_setARGB_a'><code><strong>a</strong></code></a></td>
    <td>amount of <a href='SkColor_Reference#Alpha'>Color Alpha</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_setARGB_r'><code><strong>r</strong></code></a></td>
    <td>amount of red</td>
  </tr>
  <tr>    <td><a name='SkPaint_setARGB_g'><code><strong>g</strong></code></a></td>
    <td>amount of green</td>
  </tr>
  <tr>    <td><a name='SkPaint_setARGB_b'><code><strong>b</strong></code></a></td>
    <td>amount of blue</td>
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

---

## <a name='Style'>Style</a>

<a href='#SkPaint_Style'>Style</a> specifies if the geometry is filled

## <a name='Style_Fill'>Style Fill</a>

### See Also

<a href='SkPath_Reference#Fill_Type'>Path Fill Type</a>

## <a name='Style_Stroke'>Style Stroke</a>

The stroke covers the area described by following the shape edge with a pen or brush of
<a href='#Stroke_Width'>Stroke Width</a>

## <a name='Style_Hairline'>Style Hairline</a>

<a href='#Stroke_Width'>Stroke Width</a> of zero has a special meaning and switches drawing to use <a href='#Style_Hairline'>Hairline</a><a href='SkPath_Reference#Path'>Path</a> drawing with <a href='#Style_Hairline'>Hairline</a> may hit the same pixel more than once

## <a name='SkPaint_Style'>Enum SkPaint::Style</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Style'>Style</a> {
        <a href='#SkPaint_kFill_Style'>kFill_Style</a>,
        <a href='#SkPaint_kStroke_Style'>kStroke_Style</a>,
        <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a>,
    };

    static constexpr int <a href='#SkPaint_kStyleCount'>kStyleCount</a> = <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a> + 1;
</pre>

Set <a href='#SkPaint_Style'>Style</a> to fill

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kFill_Style'><code>SkPaint::kFill_Style</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Applies to <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kStroke_Style'><code>SkPaint::kStroke_Style</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Applies to <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kStrokeAndFill_Style'><code>SkPaint::kStrokeAndFill_Style</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Applies to <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kStyleCount'><code>SkPaint::kStyleCount</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#SkPaint_Style'>Style</a> is a legal value</td>
  </tr>
</table>

<a name='SkPaint_getStyle'></a>
## getStyle

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_getStyle'>getStyle</a>(
</pre>

Returns whether the geometry is filled

### Return Value

one of

### Example

<div><fiddle-embed name="1c5e18c3c0102d2dac86a78ba8c8ce01">

#### Example Output

~~~~
SkPaint::kFill_Style == paint.getStyle()
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_setStyle'>setStyle</a>

---

<a name='SkPaint_setStyle'></a>
## setStyle

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setStyle'>setStyle</a>(<a href='#SkPaint_Style'>Style</a> style
</pre>

Sets whether the geometry is filled

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStyle_style'><code><strong>style</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c7bb6248e4735b8d1a32d02fba40d344"></fiddle-embed></div>

### See Also

<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_getStyle'>getStyle</a>

---

### See Also

<a href='SkPath_Reference#Fill_Type'>Path Fill Type</a> <a href='undocumented#Path_Effect'>Path Effect</a> <a href='#Style_Fill'>Style Fill</a> <a href='#Style_Stroke'>Style Stroke</a>

## <a name='Stroke_Width'>Stroke Width</a>

<a href='#Stroke_Width'>Stroke Width</a> sets the width for stroking

### Example

<div><fiddle-embed name="5112c7209a19e035c61cef33a624a652" gpu="true"><div>The pixels hit to represent thin lines vary with the angle of the
line and the platform implementation</div></fiddle-embed></div>

<a name='SkPaint_getStrokeWidth'></a>
## getStrokeWidth

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getStrokeWidth'>getStrokeWidth</a>(
</pre>

Returns the thickness of the pen used by <a href='#Paint'>Paint</a> to
outline the shape

### Return Value

zero for <a href='#Style_Hairline'>Hairline</a>

### Example

<div><fiddle-embed name="99aa73f64df8bbf06e656cd891a81b9e">

#### Example Output

~~~~
0 == paint.getStrokeWidth()
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setStrokeWidth'></a>
## setStrokeWidth

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setStrokeWidth'>setStrokeWidth</a>(<a href='undocumented#SkScalar'>SkScalar</a> width
</pre>

Sets the thickness of the pen used by the paint to
outline the shape

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeWidth_width'><code><strong>width</strong></code></a></td>
    <td>zero thickness for <a href='#Style_Hairline'>Hairline</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0c4446c0870b5c7b5a2efe77ff92afb8">

#### Example Output

~~~~
5 == paint.getStrokeWidth()
~~~~

</fiddle-embed></div>

---

## <a name='Miter_Limit'>Miter Limit</a>

<a href='#Miter_Limit'>Miter Limit</a> specifies the maximum miter length <code>miter limit</code>

| miter limit | angle in degrees |
| --- | ---  |
| 10 | 11 |
| 9 | 12 |
| 8 | 14 |
| 7 | 16 |
| 6 | 19 |
| 5 | 23 |
| 4 | 28 |
| 3 | 38 |
| 2 | 60 |
| 1 | 180 |

### Example

<div><fiddle-embed name="5de2de0f00354e59074a9bb1a42d5a63"><div>This example draws a stroked corner and the miter length beneath</div></fiddle-embed></div>

<a name='SkPaint_getStrokeMiter'></a>
## getStrokeMiter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getStrokeMiter'>getStrokeMiter</a>(
</pre>

Returns the limit at which a sharp corner is drawn beveled

### Return Value

zero and greater <a href='#Miter_Limit'>Miter Limit</a>

### Example

<div><fiddle-embed name="50da74a43b725f07a914df588c867d36">

#### Example Output

~~~~
default miter limit == 4
~~~~

</fiddle-embed></div>

### See Also

<a href='#Miter_Limit'>Miter Limit</a> <a href='#SkPaint_setStrokeMiter'>setStrokeMiter</a> <a href='#SkPaint_Join'>Join</a>

---

<a name='SkPaint_setStrokeMiter'></a>
## setStrokeMiter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setStrokeMiter'>setStrokeMiter</a>(<a href='undocumented#SkScalar'>SkScalar</a> miter
</pre>

Sets the limit at which a sharp corner is drawn beveled

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeMiter_miter'><code><strong>miter</strong></code></a></td>
    <td>zero and greater <a href='#Miter_Limit'>Miter Limit</a></td>
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

<a href='#Miter_Limit'>Miter Limit</a> <a href='#SkPaint_getStrokeMiter'>getStrokeMiter</a> <a href='#SkPaint_Join'>Join</a>

---

## <a name='Stroke_Cap'>Stroke Cap</a>

## <a name='SkPaint_Cap'>Enum SkPaint::Cap</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Cap'>Cap</a> {
        <a href='#SkPaint_kButt_Cap'>kButt_Cap</a>,
        <a href='#SkPaint_kRound_Cap'>kRound_Cap</a>,
        <a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>,

        <a href='#SkPaint_kLast_Cap'>kLast_Cap</a> = <a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>,
        <a href='#SkPaint_kDefault_Cap'>kDefault_Cap</a> = <a href='#SkPaint_kButt_Cap'>kButt_Cap</a>,
    };

    static constexpr int <a href='#SkPaint_kCapCount'>kCapCount</a> = <a href='#SkPaint_kLast_Cap'>kLast_Cap</a> + 1;
</pre>

<a href='#Stroke_Cap'>Stroke Cap</a> draws at the beginning and end of an open <a href='SkPath_Overview#Contour'>Path Contour</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kButt_Cap'><code>SkPaint::kButt_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Does not extend the stroke past the beginning or the end</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kRound_Cap'><code>SkPaint::kRound_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds a circle with a diameter equal to <a href='#Stroke_Width'>Stroke Width</a> at the beginning
and end</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kSquare_Cap'><code>SkPaint::kSquare_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds a square with sides equal to <a href='#Stroke_Width'>Stroke Width</a> at the beginning
and end</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kLast_Cap'><code>SkPaint::kLast_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Equivalent to the largest value for <a href='#Stroke_Cap'>Stroke Cap</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kDefault_Cap'><code>SkPaint::kDefault_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#Stroke_Cap'>Stroke Cap</a> is set to <a href='#SkPaint_kButt_Cap'>kButt Cap</a> by default</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kCapCount'><code>SkPaint::kCapCount</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#Stroke_Cap'>Stroke Cap</a> is a legal value</td>
  </tr>
</table>

Stroke describes the area covered by a pen of <a href='#Stroke_Width'>Stroke Width</a> as it
follows the <a href='SkPath_Overview#Contour'>Path Contour</a>

### Example

<div><fiddle-embed name="2bffb6384cc20077e632e7d01da045ca"></fiddle-embed></div>

<a name='SkPaint_getStrokeCap'></a>
## getStrokeCap

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_getStrokeCap'>getStrokeCap</a>(
</pre>

Returns the geometry drawn at the beginning and end of strokes

### Return Value

one of

### Example

<div><fiddle-embed name="aabf9baee8e026fae36fca30e955512b">

#### Example Output

~~~~
kButt_Cap == default stroke cap
~~~~

</fiddle-embed></div>

### See Also

<a href='#Stroke_Cap'>Stroke Cap</a> <a href='#SkPaint_setStrokeCap'>setStrokeCap</a>

---

<a name='SkPaint_setStrokeCap'></a>
## setStrokeCap

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setStrokeCap'>setStrokeCap</a>(<a href='#SkPaint_Cap'>Cap</a> cap
</pre>

Sets the geometry drawn at the beginning and end of strokes

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeCap_cap'><code><strong>cap</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="de83fbd848a4625345b4b87a6e55d98a">

#### Example Output

~~~~
kRound_Cap == paint.getStrokeCap()
~~~~

</fiddle-embed></div>

### See Also

<a href='#Stroke_Cap'>Stroke Cap</a> <a href='#SkPaint_getStrokeCap'>getStrokeCap</a>

---

## <a name='Stroke_Join'>Stroke Join</a>

<a href='#Stroke_Join'>Stroke Join</a> draws at the sharp corners of an open or closed <a href='SkPath_Overview#Contour'>Path Contour</a>

### Example

<div><fiddle-embed name="917c44b504d3f9308571fd3835d90a0d"></fiddle-embed></div>

## <a name='SkPaint_Join'>Enum SkPaint::Join</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Join'>Join</a> {
        <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>,
        <a href='#SkPaint_kRound_Join'>kRound_Join</a>,
        <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>,

        <a href='#SkPaint_kLast_Join'>kLast_Join</a> = <a href='#SkPaint_kBevel_Join'>kBevel_Join</a>,
        <a href='#SkPaint_kDefault_Join'>kDefault_Join</a> = <a href='#SkPaint_kMiter_Join'>kMiter_Join</a>,
    };

    static constexpr int <a href='#SkPaint_kJoinCount'>kJoinCount</a> = <a href='#SkPaint_kLast_Join'>kLast_Join</a> + 1;
</pre>

<a href='#SkPaint_Join'>Join</a> specifies how corners are drawn when a shape is stroked

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kMiter_Join'><code>SkPaint::kMiter_Join</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Extends the outside corner to the extent allowed by <a href='#Miter_Limit'>Miter Limit</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kRound_Join'><code>SkPaint::kRound_Join</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds a circle with a diameter of <a href='#Stroke_Width'>Stroke Width</a> at the sharp corner</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kBevel_Join'><code>SkPaint::kBevel_Join</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Connects the outside edges of the sharp corner</td>
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
<a href='#Stroke_Join'>Stroke Join</a> is set to <a href='#SkPaint_kMiter_Join'>kMiter Join</a> by default</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kJoinCount'><code>SkPaint::kJoinCount</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#Stroke_Join'>Stroke Join</a> is a legal value</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3b1aebacc21c1836a52876b9b0b3905e"></fiddle-embed></div>

### See Also

<a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a> <a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a> <a href='#SkPaint_setStrokeMiter'>setStrokeMiter</a> <a href='#SkPaint_getStrokeMiter'>getStrokeMiter</a>

<a name='SkPaint_getStrokeJoin'></a>
## getStrokeJoin

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a>(
</pre>

Returns the geometry drawn at the corners of strokes

### Return Value

one of

### Example

<div><fiddle-embed name="31bf751d0a8ddf176b871810820d8199">

#### Example Output

~~~~
kMiter_Join == default stroke join
~~~~

</fiddle-embed></div>

### See Also

<a href='#Stroke_Join'>Stroke Join</a> <a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a>

---

<a name='SkPaint_setStrokeJoin'></a>
## setStrokeJoin

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a>(<a href='#SkPaint_Join'>Join</a> join
</pre>

Sets the geometry drawn at the corners of strokes

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeJoin_join'><code><strong>join</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="48d963ad4286eddf680f9c511eb6da91">

#### Example Output

~~~~
kMiter_Join == paint.getStrokeJoin()
~~~~

</fiddle-embed></div>

### See Also

<a href='#Stroke_Join'>Stroke Join</a> <a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a>

---

### See Also

<a href='#Miter_Limit'>Miter Limit</a>

## <a name='Fill_Path'>Fill Path</a>

<a href='#Fill_Path'>Fill Path</a> creates a <a href='SkPath_Reference#Path'>Path</a> by applying the <a href='undocumented#Path_Effect'>Path Effect</a>

<a name='SkPaint_getFillPath'></a>
## getFillPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_getFillPath'>getFillPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>
</pre>

Returns the filled equivalent of the stroked path

### Parameters

<table>  <tr>    <td><a name='SkPaint_getFillPath_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPath_Reference#Path'>Path</a> read to create a filled version</td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_dst'><code><strong>dst</strong></code></a></td>
    <td>resulting <a href='SkPath_Reference#Path'>Path</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>optional limit passed to <a href='undocumented#Path_Effect'>Path Effect</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_resScale'><code><strong>resScale</strong></code></a></td>
    <td>if</td>
  </tr>
</table>

### Return Value

true if the path represents <a href='#Style_Fill'>Style Fill</a>

### Example

<div><fiddle-embed name="cedd6233848198e1fca4d1e14816baaf"><div>A very small <a href='SkPath_Reference#Quad'>Quad</a> stroke is turned into a filled path with increasing levels of precision</div></fiddle-embed></div>

---

<a name='SkPaint_getFillPath_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_getFillPath'>getFillPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>
</pre>

Returns the filled equivalent of the stroked path

### Parameters

<table>  <tr>    <td><a name='SkPaint_getFillPath_2_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPath_Reference#Path'>Path</a> read to create a filled version</td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_2_dst'><code><strong>dst</strong></code></a></td>
    <td>resulting <a href='SkPath_Reference#Path'>Path</a> <a href='#SkPaint_getFillPath_2_dst'>dst</a> may be the same as <a href='#SkPaint_getFillPath_2_src'>src</a></td>
  </tr>
</table>

### Return Value

true if the path represents <a href='#Style_Fill'>Style Fill</a>

### Example

<div><fiddle-embed name="e6d8ca0cc17e0b475bd54dd995825468"></fiddle-embed></div>

---

### See Also

<a href='#Style_Stroke'>Style Stroke</a> <a href='#Stroke_Width'>Stroke Width</a> <a href='undocumented#Path_Effect'>Path Effect</a>

## <a name='Shader_Methods'>Shader Methods</a>

<a href='undocumented#Shader'>Shader</a> defines the colors used when drawing a shape

### Example

<div><fiddle-embed name="c015dc2010c15e1c00b4f7330232b0f7"></fiddle-embed></div>

If <a href='undocumented#Shader'>Shader</a> generates only <a href='SkColor_Reference#Alpha'>Color Alpha</a> then all components of <a href='SkColor_Reference#Color'>Color</a> modulate the output

### Example

<div><fiddle-embed name="fe80fd80b98a20823db7fb9a077243c7"></fiddle-embed></div>

<a name='SkPaint_getShader'></a>
## getShader

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkShader'>SkShader</a>
</pre>

Returns optional colors used when filling a path

### Return Value

<a href='undocumented#Shader'>Shader</a> if previously set

### Example

<div><fiddle-embed name="09f15b9fd88882850da2d235eb86292f">

#### Example Output

~~~~
nullptr == shader
nullptr != shader
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_refShader'></a>
## refShader

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns optional colors used when filling a path

### Return Value

<a href='undocumented#Shader'>Shader</a> if previously set

### Example

<div><fiddle-embed name="53da0295972a418cbc9607bbb17feaa8">

#### Example Output

~~~~
shader unique: true
shader unique: false
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setShader'></a>
## setShader

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setShader'>setShader</a>(<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Sets optional colors used when filling a path

### Parameters

<table>  <tr>    <td><a name='SkPaint_setShader_shader'><code><strong>shader</strong></code></a></td>
    <td>how geometry is filled with color</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="77e64d5bae9b1ba037fd99252bb4aa58"></fiddle-embed></div>

---

## <a name='Color_Filter_Methods'>Color Filter Methods</a>

<a href='undocumented#Color_Filter'>Color Filter</a> alters the color used when drawing a shape

### Example

<div><fiddle-embed name="5abde56ca2f89a18b8e231abd1b57c56"></fiddle-embed></div>

<a name='SkPaint_getColorFilter'></a>
## getColorFilter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkColorFilter'>SkColorFilter</a>
</pre>

Returns <a href='undocumented#Color_Filter'>Color Filter</a> if set

### Return Value

<a href='undocumented#Color_Filter'>Color Filter</a> if previously set

### Example

<div><fiddle-embed name="093bdc627d6b59002670fd290931f6c9">

#### Example Output

~~~~
nullptr == color filter
nullptr != color filter
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_refColorFilter'></a>
## refColorFilter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='undocumented#Color_Filter'>Color Filter</a> if set

### Return Value

<a href='undocumented#Color_Filter'>Color Filter</a> if set

### Example

<div><fiddle-embed name="b588c95fa4c86ddbc4b0546762f08297">

#### Example Output

~~~~
color filter unique: true
color filter unique: false
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setColorFilter'></a>
## setColorFilter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setColorFilter'>setColorFilter</a>(<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Sets <a href='undocumented#Color_Filter'>Color Filter</a> to filter

### Parameters

<table>  <tr>    <td><a name='SkPaint_setColorFilter_colorFilter'><code><strong>colorFilter</strong></code></a></td>
    <td><a href='undocumented#Color_Filter'>Color Filter</a> to apply to subsequent draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c7b786dc9b3501cd0eaba47494b6fa31"></fiddle-embed></div>

---

## <a name='Blend_Mode_Methods'>Blend Mode Methods</a>

<a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> describes how <a href='SkColor_Reference#Color'>Color</a> combines with the destination color

### Example

<div><fiddle-embed name="73092d4d06faecea3c204d852a4dd8a8"></fiddle-embed></div>

### See Also

<a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a>

<a name='SkPaint_getBlendMode'></a>
## getBlendMode

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkPaint_getBlendMode'>getBlendMode</a>(
</pre>

Returns <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a>

### Return Value

mode used to combine source color with destination color

### Example

<div><fiddle-embed name="a1e059c8f6740fa2044cc64152b39dda">

#### Example Output

~~~~
kSrcOver == getBlendMode
kSrcOver != getBlendMode
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_isSrcOver'></a>
## isSrcOver

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isSrcOver'>isSrcOver</a>(
</pre>

Returns true if <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> is <a href='SkBlendMode_Reference#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a>

### Return Value

true if <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> is <a href='SkBlendMode_Reference#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a>

### Example

<div><fiddle-embed name="257c9473db7a2b3a0fb2b9e2431e59a6">

#### Example Output

~~~~
isSrcOver == true
isSrcOver != true
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setBlendMode'></a>
## setBlendMode

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setBlendMode'>setBlendMode</a>(<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode
</pre>

Sets <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> to <a href='#SkPaint_setBlendMode_mode'>mode</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setBlendMode_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> used to combine source color and destination</td>
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

---

## <a name='Path_Effect_Methods'>Path Effect Methods</a>

<a href='undocumented#Path_Effect'>Path Effect</a> modifies the path geometry before drawing it

### Example

<div><fiddle-embed name="8cf5684b187d60f09e11c4a48993ea39"></fiddle-embed></div>

### See Also

<a href='undocumented#Path_Effect'>Path Effect</a>

<a name='SkPaint_getPathEffect'></a>
## getPathEffect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkPathEffect'>SkPathEffect</a>
</pre>

Returns <a href='undocumented#Path_Effect'>Path Effect</a> if set

### Return Value

<a href='undocumented#Path_Effect'>Path Effect</a> if previously set

### Example

<div><fiddle-embed name="211a1b14bfa6c4332082c8eab4fbc5fd">

#### Example Output

~~~~
nullptr == path effect
nullptr != path effect
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_refPathEffect'></a>
## refPathEffect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='undocumented#Path_Effect'>Path Effect</a> if set

### Return Value

<a href='undocumented#Path_Effect'>Path Effect</a> if previously set

### Example

<div><fiddle-embed name="f56039b94c702c2704c8c5100e623aca">

#### Example Output

~~~~
path effect unique: true
path effect unique: false
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setPathEffect'></a>
## setPathEffect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setPathEffect'>setPathEffect</a>(<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Sets <a href='undocumented#Path_Effect'>Path Effect</a> to <a href='#SkPaint_setPathEffect_pathEffect'>pathEffect</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setPathEffect_pathEffect'><code><strong>pathEffect</strong></code></a></td>
    <td>replace <a href='SkPath_Reference#Path'>Path</a> with a modification when drawn</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="52dd55074ca0b7d520d04e750ca2a0d7"></fiddle-embed></div>

---

## <a name='Mask_Filter_Methods'>Mask Filter Methods</a>

<a href='undocumented#Mask_Filter'>Mask Filter</a> uses coverage of the shape drawn to create <a href='undocumented#Mask_Alpha'>Mask Alpha</a>

### Example

<div><fiddle-embed name="55d7b9d482ac8e17a6153f555a8adb8d"></fiddle-embed></div>

<a name='SkPaint_getMaskFilter'></a>
## getMaskFilter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>
</pre>

Returns <a href='undocumented#Mask_Filter'>Mask Filter</a> if set

### Return Value

<a href='undocumented#Mask_Filter'>Mask Filter</a> if previously set

### Example

<div><fiddle-embed name="5ac4b31371726da87bb7390b385e9fee">

#### Example Output

~~~~
nullptr == mask filter
nullptr != mask filter
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_refMaskFilter'></a>
## refMaskFilter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='undocumented#Mask_Filter'>Mask Filter</a> if set

### Return Value

<a href='undocumented#Mask_Filter'>Mask Filter</a> if previously set

### Example

<div><fiddle-embed name="084b0dc3cebd78718c651d58f257f799">

#### Example Output

~~~~
mask filter unique: true
mask filter unique: false
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setMaskFilter'></a>
## setMaskFilter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setMaskFilter'>setMaskFilter</a>(<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Sets <a href='undocumented#Mask_Filter'>Mask Filter</a> to <a href='#SkPaint_setMaskFilter_maskFilter'>maskFilter</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setMaskFilter_maskFilter'><code><strong>maskFilter</strong></code></a></td>
    <td>modifies clipping mask generated from drawn geometry</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a993831c40f3e134f809134e3b74e4a6"></fiddle-embed></div>

---

## <a name='Typeface_Methods'>Typeface Methods</a>

<a href='undocumented#Typeface'>Typeface</a> identifies the font used when drawing and measuring text

### Example

<div><fiddle-embed name="1a7a5062725139760962582f599f1b97"></fiddle-embed></div>

<a name='SkPaint_getTypeface'></a>
## getTypeface

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkTypeface'>SkTypeface</a>
</pre>

Returns <a href='undocumented#Typeface'>Typeface</a> if set

### Return Value

<a href='undocumented#Typeface'>Typeface</a> if previously set

### Example

<div><fiddle-embed name="5ce718e5a184baaac80e7098d7dad67b">

#### Example Output

~~~~
nullptr == typeface
nullptr != typeface
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_refTypeface'></a>
## refTypeface

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Increases <a href='undocumented#Typeface'>Typeface</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one

### Return Value

<a href='undocumented#Typeface'>Typeface</a> if previously set

### Example

<div><fiddle-embed name="8b5aa7e555a0dc31be69db7cadf471a1">

#### Example Output

~~~~
typeface1 != typeface2
typeface1 == typeface2
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setTypeface'></a>
## setTypeface

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTypeface'>setTypeface</a>(<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Sets <a href='undocumented#Typeface'>Typeface</a> to <a href='#SkPaint_setTypeface_typeface'>typeface</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTypeface_typeface'><code><strong>typeface</strong></code></a></td>
    <td>font and style used to draw text</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0e6fbb7773cd925b274552f4cd1abef2"></fiddle-embed></div>

---

## <a name='Image_Filter_Methods'>Image Filter Methods</a>

<a href='undocumented#Image_Filter'>Image Filter</a> operates on the pixel representation of the shape

### Example

<div><fiddle-embed name="ece04ee3d3761e3425f37c8f06f054c1"></fiddle-embed></div>

<a name='SkPaint_getImageFilter'></a>
## getImageFilter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkImageFilter'>SkImageFilter</a>
</pre>

Returns <a href='undocumented#Image_Filter'>Image Filter</a> if set

### Return Value

<a href='undocumented#Image_Filter'>Image Filter</a> if previously set

### Example

<div><fiddle-embed name="c11f8eaa1dd149bc18db21e23ce26904">

#### Example Output

~~~~
nullptr == image filter
nullptr != image filter
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_refImageFilter'></a>
## refImageFilter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='undocumented#Image_Filter'>Image Filter</a> if set

### Return Value

<a href='undocumented#Image_Filter'>Image Filter</a> if previously set

### Example

<div><fiddle-embed name="13f09088b569251547107d14ae989dc1">

#### Example Output

~~~~
image filter unique: true
image filter unique: false
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setImageFilter'></a>
## setImageFilter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setImageFilter'>setImageFilter</a>(<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Sets <a href='undocumented#Image_Filter'>Image Filter</a> to <a href='#SkPaint_setImageFilter_imageFilter'>imageFilter</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setImageFilter_imageFilter'><code><strong>imageFilter</strong></code></a></td>
    <td>how <a href='SkImage_Reference#Image'>Image</a> is sampled when transformed</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6679d6e4ec632715ee03e68391bd7f9a"></fiddle-embed></div>

---

## <a name='Draw_Looper_Methods'>Draw Looper Methods</a>

<a href='undocumented#Draw_Looper'>Draw Looper</a> sets a modifier that communicates state from one <a href='undocumented#Draw_Layer'>Draw Layer</a>
to another to construct the draw

### Example

<div><fiddle-embed name="84ec12a36e50df5ac565cc7a75ffbe9f"></fiddle-embed></div>

<a name='SkPaint_getDrawLooper'></a>
## getDrawLooper

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>
</pre>

Returns <a href='undocumented#Draw_Looper'>Draw Looper</a> if set

### Return Value

<a href='undocumented#Draw_Looper'>Draw Looper</a> if previously set

### Example

<div><fiddle-embed name="af4c5acc7a91e7f23c2af48018903ad4">

#### Example Output

~~~~
nullptr == draw looper
nullptr != draw looper
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_refDrawLooper'></a>
## refDrawLooper

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='undocumented#Draw_Looper'>Draw Looper</a> if set

### Return Value

<a href='undocumented#Draw_Looper'>Draw Looper</a> if previously set

### Example

<div><fiddle-embed name="2a3782c33f04ed17a725d0e449c6f7c3">

#### Example Output

~~~~
draw looper unique: true
draw looper unique: false
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_getLooper'></a>
## getLooper

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>
</pre>

Deprecated.

---

<a name='SkPaint_setDrawLooper'></a>
## setDrawLooper

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setDrawLooper'>setDrawLooper</a>(<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Sets <a href='undocumented#Draw_Looper'>Draw Looper</a> to <a href='#SkPaint_setDrawLooper_drawLooper'>drawLooper</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setDrawLooper_drawLooper'><code><strong>drawLooper</strong></code></a></td>
    <td>iterates through drawing one or more time</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bf10f838b330f0a3a3266d42ea68a638"></fiddle-embed></div>

---

<a name='SkPaint_setLooper'></a>
## setLooper

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setLooper'>setLooper</a>(<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Deprecated.

---

## <a name='Text_Align'>Text Align</a>

## <a name='SkPaint_Align'>Enum SkPaint::Align</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Align'>Align</a> {
        <a href='#SkPaint_kLeft_Align'>kLeft_Align</a>,
        <a href='#SkPaint_kCenter_Align'>kCenter_Align</a>,
        <a href='#SkPaint_kRight_Align'>kRight_Align</a>,
    };

    static constexpr int <a href='#SkPaint_kAlignCount'>kAlignCount</a> = 3;
</pre>

<a href='#SkPaint_Align'>Align</a> adjusts the text relative to the text position

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kLeft_Align'><code>SkPaint::kLeft_Align</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Leaves the glyph at the position computed by the font offset by the text position</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kCenter_Align'><code>SkPaint::kCenter_Align</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Moves the glyph half its width if <a href='#SkPaint_Flags'>Flags</a> has <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> clear</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kRight_Align'><code>SkPaint::kRight_Align</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Moves the glyph by its width if <a href='#SkPaint_Flags'>Flags</a> has <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> clear</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kAlignCount'><code>SkPaint::kAlignCount</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#SkPaint_Align'>Align</a> is a legal value</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="702617fd9ebc3f12e30081b5db93e8a8"><div>Each position separately moves the glyph in drawPosText</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="f1cbbbafe6b3c52b81309cccbf96a308"><div><a href='#Vertical_Text'>Vertical Text</a> treats <a href='#SkPaint_kLeft_Align'>kLeft Align</a> as top align</div></fiddle-embed></div>

<a name='SkPaint_getTextAlign'></a>
## getTextAlign

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Align'>Align</a> <a href='#SkPaint_getTextAlign'>getTextAlign</a>(
</pre>

Returns <a href='#Text_Align'>Text Align</a>

### Return Value

text placement relative to position

### Example

<div><fiddle-embed name="2df932f526e810f74c89d30ec3f4c947">

#### Example Output

~~~~
kLeft_Align == default
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setTextAlign'></a>
## setTextAlign

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void    <a href='#SkPaint_setTextAlign'>setTextAlign</a>(<a href='#SkPaint_Align'>Align</a> align
</pre>

Sets <a href='#Text_Align'>Text Align</a> to <a href='#SkPaint_setTextAlign_align'>align</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextAlign_align'><code><strong>align</strong></code></a></td>
    <td>text placement relative to position</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d37540afd918506ac2594665ca63979b"><div><a href='undocumented#Text'>Text</a> is left</div></fiddle-embed></div>

---

## <a name='Text_Size'>Text Size</a>

<a href='#Text_Size'>Text Size</a> adjusts the overall text size in points

### Example

<div><fiddle-embed name="91c9a3e498bb9412e4522a95d076ed5f"></fiddle-embed></div>

<a name='SkPaint_getTextSize'></a>
## getTextSize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSize'>getTextSize</a>(
</pre>

Returns <a href='#Text_Size'>Text Size</a> in points

### Return Value

typographic height of text

### Example

<div><fiddle-embed name="983e2a71ba72d4ba8c945420040b8f1c"></fiddle-embed></div>

---

<a name='SkPaint_setTextSize'></a>
## setTextSize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextSize'>setTextSize</a>(<a href='undocumented#SkScalar'>SkScalar</a> textSize
</pre>

Sets <a href='#Text_Size'>Text Size</a> in points

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextSize_textSize'><code><strong>textSize</strong></code></a></td>
    <td>typographic height of text</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6510c9e2f57b83c47e67829e7a68d493"></fiddle-embed></div>

---

## <a name='Text_Scale_X'>Text Scale X</a>

<a href='#Text_Scale_X'>Text Scale X</a> adjusts the text horizontal scale

### Example

<div><fiddle-embed name="d13d787c1e36f515319fc998411c1d91"></fiddle-embed></div>

<a name='SkPaint_getTextScaleX'></a>
## getTextScaleX

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextScaleX'>getTextScaleX</a>(
</pre>

Returns <a href='#Text_Scale_X'>Text Scale X</a>

### Return Value

text horizontal scale

### Example

<div><fiddle-embed name="5dc8e58f6910cb8e4de9ed60f888188b"></fiddle-embed></div>

---

<a name='SkPaint_setTextScaleX'></a>
## setTextScaleX

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextScaleX'>setTextScaleX</a>(<a href='undocumented#SkScalar'>SkScalar</a> scaleX
</pre>

Sets <a href='#Text_Scale_X'>Text Scale X</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextScaleX_scaleX'><code><strong>scaleX</strong></code></a></td>
    <td>text horizontal scale</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a75bbdb8bb866b125c4c1dd5e967d470"></fiddle-embed></div>

---

## <a name='Text_Skew_X'>Text Skew X</a>

<a href='#Text_Skew_X'>Text Skew X</a> adjusts the text horizontal slant

### Example

<div><fiddle-embed name="aff208b0aab265f273045b27e683c17c"></fiddle-embed></div>

<a name='SkPaint_getTextSkewX'></a>
## getTextSkewX

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSkewX'>getTextSkewX</a>(
</pre>

Returns <a href='#Text_Skew_X'>Text Skew X</a>

### Return Value

additional shear in x

### Example

<div><fiddle-embed name="11c10f466dae0d1639dbb9f6a0040218"></fiddle-embed></div>

---

<a name='SkPaint_setTextSkewX'></a>
## setTextSkewX

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextSkewX'>setTextSkewX</a>(<a href='undocumented#SkScalar'>SkScalar</a> skewX
</pre>

Sets <a href='#Text_Skew_X'>Text Skew X</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextSkewX_skewX'><code><strong>skewX</strong></code></a></td>
    <td>additional shear in x</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6bd705a6e0c5f8ee24f302fe531bfabc"></fiddle-embed></div>

---

## <a name='Text_Encoding'>Text Encoding</a>

## <a name='SkPaint_TextEncoding'>Enum SkPaint::TextEncoding</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_TextEncoding'>TextEncoding</a> {
        <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>,
        <a href='#SkPaint_kUTF16_TextEncoding'>kUTF16_TextEncoding</a>,
        <a href='#SkPaint_kUTF32_TextEncoding'>kUTF32_TextEncoding</a>,
        <a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID_TextEncoding</a>,
    };
</pre>

<a href='#SkPaint_TextEncoding'>TextEncoding</a> determines whether text specifies character codes and their encoded
size <a href='https://unicode.org/standard/standard.html'>Unicode standard</a></a> <a href='https://tools.ietf.org/html/rfc3629'>UTF-8 (RFC 3629)</a></a> encodes each character as one or more 8 <a href='https://tools.ietf.org/html/rfc2781'>UTF-16 (RFC 2781)</a></a> encodes each character as one or two 16 <a href='https://www.unicode.org/versions/Unicode5.0.0/ch03.pdf'>UTF-32</a></a> encodes each character as one 32

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

<div><fiddle-embed name="b29294e7f29d160a1b46abf2dcec9d2a"><div>First line is encoded in UTF</div></fiddle-embed></div>

<a name='SkPaint_getTextEncoding'></a>
## getTextEncoding

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_getTextEncoding'>getTextEncoding</a>(
</pre>

Returns <a href='#Text_Encoding'>Text Encoding</a>

### Return Value

one of

### Example

<div><fiddle-embed name="c6cc2780a9828b3af8c4621c12b29a1b">

#### Example Output

~~~~
kUTF8_TextEncoding == text encoding
kGlyphID_TextEncoding == text encoding
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_setTextEncoding'></a>
## setTextEncoding

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextEncoding'>setTextEncoding</a>(<a href='#SkPaint_TextEncoding'>TextEncoding</a> encoding
</pre>

Sets <a href='#Text_Encoding'>Text Encoding</a> to <a href='#SkPaint_setTextEncoding_encoding'>encoding</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextEncoding_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6d9ffdd3c5543e9f12972a06dd4a0ce5">

#### Example Output

~~~~
4 != text encoding
~~~~

</fiddle-embed></div>

---

## <a name='Font_Metrics'>Font Metrics</a>

<a href='#Font_Metrics'>Font Metrics</a> describe dimensions common to the <a href='undocumented#Glyph'>Glyphs</a> in <a href='undocumented#Typeface'>Typeface</a>

### Example

<div><fiddle-embed name="2bfa3783719fcd769af177a1b244e171"></fiddle-embed></div>

<a name='SkPaint_FontMetrics'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    struct <a href='#SkPaint_FontMetrics'>FontMetrics</a> {
        enum <a href='#SkPaint_FontMetrics_FontMetricsFlags'>FontMetricsFlags</a> {
            <a href='#SkPaint_FontMetrics_kUnderlineThicknessIsValid_Flag'>kUnderlineThicknessIsValid_Flag</a> = 1 << 0,
            <a href='#SkPaint_FontMetrics_kUnderlinePositionIsValid_Flag'>kUnderlinePositionIsValid_Flag</a> = 1 << 1,
            <a href='#SkPaint_FontMetrics_kStrikeoutThicknessIsValid_Flag'>kStrikeoutThicknessIsValid_Flag</a> = 1 << 2,
            <a href='#SkPaint_FontMetrics_kStrikeoutPositionIsValid_Flag'>kStrikeoutPositionIsValid_Flag</a> = 1 << 3,
        };

        uint32_t    <a href='#SkPaint_FontMetrics_fFlags'>fFlags</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fTop'>fTop</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fAscent'>fAscent</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fDescent'>fDescent</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fBottom'>fBottom</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fLeading'>fLeading</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fAvgCharWidth'>fAvgCharWidth</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fMaxCharWidth'>fMaxCharWidth</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fXMin'>fXMin</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fXMax'>fXMax</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fXHeight'>fXHeight</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fCapHeight'>fCapHeight</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fUnderlineThickness'>fUnderlineThickness</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fUnderlinePosition'>fUnderlinePosition</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fStrikeoutThickness'>fStrikeoutThickness</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkPaint_FontMetrics_fStrikeoutPosition'>fStrikeoutPosition</a>;

        bool <a href='#SkPaint_FontMetrics_hasUnderlineThickness'>hasUnderlineThickness</a>(<a href='undocumented#SkScalar'>SkScalar</a>* thickness) const;
        bool <a href='#SkPaint_FontMetrics_hasUnderlinePosition'>hasUnderlinePosition</a>(<a href='undocumented#SkScalar'>SkScalar</a>* position) const;
        bool <a href='#SkPaint_FontMetrics_hasStrikeoutThickness'>hasStrikeoutThickness</a>(<a href='undocumented#SkScalar'>SkScalar</a>* thickness) const;
        bool <a href='#SkPaint_FontMetrics_hasStrikeoutPosition'>hasStrikeoutPosition</a>(<a href='undocumented#SkScalar'>SkScalar</a>* position) const;
    };
</pre>

FontMetrics is filled out by

## <a name='SkPaint_FontMetrics_FontMetricsFlags'>Enum SkPaint::FontMetrics::FontMetricsFlags</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
        enum FontMetricsFlags {
            kUnderlineThicknessIsValid_Flag = 1 << 0,
            kUnderlinePositionIsValid_Flag = 1 << 1,
            kStrikeoutThicknessIsValid_Flag = 1 << 2,
            kStrikeoutPositionIsValid_Flag = 1 << 3,
        };
</pre>

FontMetricsFlags are set in fFlags when underline and strikeout metrics are valid

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_kUnderlineThicknessIsValid_Flag'><code>SkPaint::FontMetrics::kUnderlineThicknessIsValid_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0001</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if fUnderlineThickness is valid</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_kUnderlinePositionIsValid_Flag'><code>SkPaint::FontMetrics::kUnderlinePositionIsValid_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0002</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if fUnderlinePosition is valid</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_kStrikeoutThicknessIsValid_Flag'><code>SkPaint::FontMetrics::kStrikeoutThicknessIsValid_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0004</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if fStrikeoutThickness is valid</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_kStrikeoutPositionIsValid_Flag'><code>SkPaint::FontMetrics::kStrikeoutPositionIsValid_Flag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0008</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if fStrikeoutPosition is valid</td>
  </tr>
</table><table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>uint32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fFlags'><code>fFlags</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
is set to FontMetricsFlags when metrics are valid</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fTop'><code>fTop</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Greatest extent above the baseline for any glyph</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fAscent'><code>fAscent</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Recommended distance above the baseline to reserve for a line of text</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fDescent'><code>fDescent</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Recommended distance below the baseline to reserve for a line of text</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fBottom'><code>fBottom</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Greatest extent below the baseline for any glyph</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fLeading'><code>fLeading</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Recommended distance to add between lines of text</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fAvgCharWidth'><code>fAvgCharWidth</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Average character width</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fMaxCharWidth'><code>fMaxCharWidth</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
maximum character width</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fXMin'><code>fXMin</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Minimum bounding box x</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fXMax'><code>fXMax</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Maximum bounding box x</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fXHeight'><code>fXHeight</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be zero if no lower</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fCapHeight'><code>fCapHeight</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be zero if no upper</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fUnderlineThickness'><code>fUnderlineThickness</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
If the metric is valid</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fUnderlinePosition'><code>fUnderlinePosition</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Position of the top of the underline stroke relative to the baseline</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fStrikeoutThickness'><code>fStrikeoutThickness</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
If the metric is valid</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fStrikeoutPosition'><code>fStrikeoutPosition</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Position of the bottom of the strikeout stroke relative to the baseline</td>
  </tr>
</table>

<a name='SkPaint_FontMetrics_hasUnderlineThickness'></a>
## hasUnderlineThickness

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool hasUnderlineThickness(SkScalar* thickness) const
</pre>

Returns true if <a href='#Font_Metrics'>Font Metrics</a> has a valid underline <a href='#SkPaint_FontMetrics_hasUnderlineThickness_thickness'>thickness</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_FontMetrics_hasUnderlineThickness_thickness'><code><strong>thickness</strong></code></a></td>
    <td>storage for underline width</td>
  </tr>
</table>

### Return Value

true if font specifies underline width

---

<a name='SkPaint_FontMetrics_hasUnderlinePosition'></a>
## hasUnderlinePosition

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool hasUnderlinePosition(SkScalar* position) const
</pre>

Returns true if <a href='#Font_Metrics'>Font Metrics</a> has a valid underline <a href='#SkPaint_FontMetrics_hasUnderlinePosition_position'>position</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_FontMetrics_hasUnderlinePosition_position'><code><strong>position</strong></code></a></td>
    <td>storage for underline <a href='#SkPaint_FontMetrics_hasUnderlinePosition_position'>position</a></td>
  </tr>
</table>

### Return Value

true if font specifies underline <a href='#SkPaint_FontMetrics_hasUnderlinePosition_position'>position</a>

---

<a name='SkPaint_FontMetrics_hasStrikeoutThickness'></a>
## hasStrikeoutThickness

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool hasStrikeoutThickness(SkScalar* thickness) const
</pre>

Returns true if <a href='#Font_Metrics'>Font Metrics</a> has a valid strikeout <a href='#SkPaint_FontMetrics_hasStrikeoutThickness_thickness'>thickness</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_FontMetrics_hasStrikeoutThickness_thickness'><code><strong>thickness</strong></code></a></td>
    <td>storage for strikeout width</td>
  </tr>
</table>

### Return Value

true if font specifies strikeout width

---

<a name='SkPaint_FontMetrics_hasStrikeoutPosition'></a>
## hasStrikeoutPosition

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool hasStrikeoutPosition(SkScalar* position) const
</pre>

Returns true if <a href='#Font_Metrics'>Font Metrics</a> has a valid strikeout <a href='#SkPaint_FontMetrics_hasStrikeoutPosition_position'>position</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_FontMetrics_hasStrikeoutPosition_position'><code><strong>position</strong></code></a></td>
    <td>storage for strikeout <a href='#SkPaint_FontMetrics_hasStrikeoutPosition_position'>position</a></td>
  </tr>
</table>

### Return Value

true if font specifies strikeout <a href='#SkPaint_FontMetrics_hasStrikeoutPosition_position'>position</a>

---

<a name='SkPaint_getFontMetrics'></a>
## getFontMetrics

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getFontMetrics'>getFontMetrics</a>(<a href='#SkPaint_FontMetrics'>FontMetrics</a>
</pre>

Returns <a href='#Font_Metrics'>Font Metrics</a> associated with <a href='undocumented#Typeface'>Typeface</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_getFontMetrics_metrics'><code><strong>metrics</strong></code></a></td>
    <td>storage for <a href='#Font_Metrics'>Font Metrics</a> from <a href='undocumented#Typeface'>Typeface</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getFontMetrics_scale'><code><strong>scale</strong></code></a></td>
    <td>additional multiplier for returned values</td>
  </tr>
</table>

### Return Value

recommended spacing between lines

### Example

<div><fiddle-embed name="b899d84caba6607340322d317992d070"></fiddle-embed></div>

### See Also

<a href='#Text_Size'>Text Size</a> <a href='undocumented#Typeface'>Typeface</a> <a href='#Typeface_Methods'>Typeface Methods</a>

---

<a name='SkPaint_getFontSpacing'></a>
## getFontSpacing

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getFontSpacing'>getFontSpacing</a>(
</pre>

Returns the recommended spacing between lines

### Return Value

recommended spacing between lines

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

---

<a name='SkPaint_getFontBounds'></a>
## getFontBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPaint_getFontBounds'>getFontBounds</a>(
</pre>

Returns the union of bounds of all <a href='undocumented#Glyph'>Glyphs</a>

### Return Value

union of bounds of all <a href='undocumented#Glyph'>Glyphs</a>

### Example

<div><fiddle-embed name="facaddeec7943bc491988e345e27e65f">

#### Example Output

~~~~
metrics bounds = { -12.2461, -14.7891, 21.5215, 5.55469 }
font bounds    = { -12.2461, -14.7891, 21.5215, 5.55469 }
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_textToGlyphs'></a>
## textToGlyphs

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_textToGlyphs'>textToGlyphs</a>(const void
</pre>

Converts <a href='#SkPaint_textToGlyphs_text'>text</a> into glyph indices

### Parameters

<table>  <tr>    <td><a name='SkPaint_textToGlyphs_text'><code><strong>text</strong></code></a></td>
    <td>character storage encoded with <a href='#Text_Encoding'>Text Encoding</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_textToGlyphs_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>length of character storage in bytes</td>
  </tr>
  <tr>    <td><a name='SkPaint_textToGlyphs_glyphs'><code><strong>glyphs</strong></code></a></td>
    <td>storage for glyph indices</td>
  </tr>
</table>

### Return Value

number of <a href='#SkPaint_textToGlyphs_glyphs'>glyphs</a> represented by <a href='#SkPaint_textToGlyphs_text'>text</a> of length <a href='#SkPaint_textToGlyphs_byteLength'>byteLength</a>

### Example

<div><fiddle-embed name="343e9471a7f7b5f09abdc3b44983433b"></fiddle-embed></div>

---

<a name='SkPaint_countText'></a>
## countText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_countText'>countText</a>(const void
</pre>

Returns the number of <a href='undocumented#Glyph'>Glyphs</a> in <a href='#SkPaint_countText_text'>text</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_countText_text'><code><strong>text</strong></code></a></td>
    <td>character storage encoded with <a href='#Text_Encoding'>Text Encoding</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_countText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>length of character storage in bytes</td>
  </tr>
</table>

### Return Value

number of <a href='undocumented#Glyph'>Glyphs</a> represented by <a href='#SkPaint_countText_text'>text</a> of length <a href='#SkPaint_countText_byteLength'>byteLength</a>

### Example

<div><fiddle-embed name="85436c71aab5410767fc688ab0573e09">

#### Example Output

~~~~
count = 5
~~~~

</fiddle-embed></div>

---

<a name='SkPaint_containsText'></a>
## containsText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_containsText'>containsText</a>(const void
</pre>

Returns true if all <a href='#SkPaint_containsText_text'>text</a> corresponds to a non

### Parameters

<table>  <tr>    <td><a name='SkPaint_containsText_text'><code><strong>text</strong></code></a></td>
    <td>array of characters or <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_containsText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>number of bytes in <a href='#SkPaint_containsText_text'>text</a> array</td>
  </tr>
</table>

### Return Value

true if all <a href='#SkPaint_containsText_text'>text</a> corresponds to a non

<div><a href='#SkPaint_containsText'>containsText</a> succeeds for degree symbol</div>

#### Example Output

~~~~
0x00b0 == has char
0xd800 != has char
~~~~

### Example

<div><fiddle-embed name="083557b6f653d6fc00a34e01f87b74ff"><div><a href='#SkPaint_containsText'>containsText</a> returns true that glyph index is greater than zero</div>

#### Example Output

~~~~
0x01ff == has glyph
0x0000 != has glyph
0xffff == has glyph
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPaint_setTextEncoding'>setTextEncoding</a> <a href='undocumented#Typeface'>Typeface</a>

---

<a name='SkPaint_glyphsToUnichars'></a>
## glyphsToUnichars

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_glyphsToUnichars'>glyphsToUnichars</a>(const <a href='undocumented#SkGlyphID'>SkGlyphID</a> glyphs
</pre>

Converts <a href='#SkPaint_glyphsToUnichars_glyphs'>glyphs</a> into <a href='#SkPaint_glyphsToUnichars_text'>text</a> if possible

### Parameters

<table>  <tr>    <td><a name='SkPaint_glyphsToUnichars_glyphs'><code><strong>glyphs</strong></code></a></td>
    <td>array of indices into font</td>
  </tr>
  <tr>    <td><a name='SkPaint_glyphsToUnichars_count'><code><strong>count</strong></code></a></td>
    <td>length of glyph array</td>
  </tr>
  <tr>    <td><a name='SkPaint_glyphsToUnichars_text'><code><strong>text</strong></code></a></td>
    <td>storage for character codes</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c12686b0b3e0a87d0a248bbfc57e9492"><div>Convert UTF</div></fiddle-embed></div>

---

## <a name='Measure_Text'>Measure Text</a>

<a name='SkPaint_measureText'></a>
## measureText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_measureText'>measureText</a>(const void
</pre>

Returns the advance width of <a href='#SkPaint_measureText_text'>text</a> if <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> is clear

### Parameters

<table>  <tr>    <td><a name='SkPaint_measureText_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices to be measured</td>
  </tr>
  <tr>    <td><a name='SkPaint_measureText_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_measureText_text'>text</a> to measure</td>
  </tr>
  <tr>    <td><a name='SkPaint_measureText_bounds'><code><strong>bounds</strong></code></a></td>
    <td>returns bounding box relative to</td>
  </tr>
</table>

### Return Value

advance width or height

### Example

<div><fiddle-embed name="06084f609184470135a9cd9ebc5af149"></fiddle-embed></div>

---

<a name='SkPaint_measureText_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_measureText'>measureText</a>(const void
</pre>

Returns the advance width of <a href='#SkPaint_measureText_2_text'>text</a> if <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> is clear

### Parameters

<table>  <tr>    <td><a name='SkPaint_measureText_2_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices to be measured</td>
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

---

<a name='SkPaint_breakText'></a>
## breakText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkPaint_breakText'>breakText</a>(const void
</pre>

Returns the bytes of <a href='#SkPaint_breakText_text'>text</a> that fit within <a href='#SkPaint_breakText_maxWidth'>maxWidth</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_breakText_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices to be measured</td>
  </tr>
  <tr>    <td><a name='SkPaint_breakText_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_breakText_text'>text</a> to measure</td>
  </tr>
  <tr>    <td><a name='SkPaint_breakText_maxWidth'><code><strong>maxWidth</strong></code></a></td>
    <td>advance limit</td>
  </tr>
  <tr>    <td><a name='SkPaint_breakText_measuredWidth'><code><strong>measuredWidth</strong></code></a></td>
    <td>returns the width of the <a href='#SkPaint_breakText_text'>text</a> less than or equal to <a href='#SkPaint_breakText_maxWidth'>maxWidth</a></td>
  </tr>
</table>

### Return Value

bytes of <a href='#SkPaint_breakText_text'>text</a> that fit

### Example

<div><fiddle-embed name="fd0033470ccbd5c7059670fdbf96cffc"><div><a href='undocumented#Line'>Line</a> under</div></fiddle-embed></div>

---

<a name='SkPaint_getTextWidths'></a>
## getTextWidths

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getTextWidths'>getTextWidths</a>(const void
</pre>

Retrieves the advance and <a href='#SkPaint_getTextWidths_bounds'>bounds</a> for each glyph in <a href='#SkPaint_getTextWidths_text'>text</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextWidths_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices to be measured</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextWidths_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getTextWidths_text'>text</a> to measure</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextWidths_widths'><code><strong>widths</strong></code></a></td>
    <td>returns <a href='#SkPaint_getTextWidths_text'>text</a> advances for each glyph</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextWidths_bounds'><code><strong>bounds</strong></code></a></td>
    <td>returns <a href='#SkPaint_getTextWidths_bounds'>bounds</a> for each glyph relative to</td>
  </tr>
</table>

### Return Value

glyph count in <a href='#SkPaint_getTextWidths_text'>text</a>

### Example

<div><fiddle-embed name="6b9e101f49e9c2c28755c5bdcef64dfb"><div>Bounds of <a href='undocumented#Glyph'>Glyphs</a> increase for stroked <a href='#SkPaint_getTextWidths_text'>text</a></div></fiddle-embed></div>

---

## <a name='Text_Path'>Text Path</a>

<a href='#Text_Path'>Text Path</a> describes the geometry of <a href='undocumented#Glyph'>Glyphs</a> used to draw text

<a name='SkPaint_getTextPath'></a>
## getTextPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_getTextPath'>getTextPath</a>(const void
</pre>

Returns the geometry as <a href='SkPath_Reference#Path'>Path</a> equivalent to the drawn <a href='#SkPaint_getTextPath_text'>text</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextPath_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextPath_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getTextPath_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextPath_x'><code><strong>x</strong></code></a></td>
    <td><a href='#SkPaint_getTextPath_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextPath_y'><code><strong>y</strong></code></a></td>
    <td><a href='#SkPaint_getTextPath_y'>y</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextPath_path'><code><strong>path</strong></code></a></td>
    <td>geometry of the <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7c9e6a399f898d68026c1f0865e6f73e"><div><a href='undocumented#Text'>Text</a> is added to <a href='SkPath_Reference#Path'>Path</a></div></fiddle-embed></div>

---

<a name='SkPaint_getPosTextPath'></a>
## getPosTextPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_getPosTextPath'>getPosTextPath</a>(const void
</pre>

Returns the geometry as <a href='SkPath_Reference#Path'>Path</a> equivalent to the drawn <a href='#SkPaint_getPosTextPath_text'>text</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_getPosTextPath_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices</td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextPath_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getPosTextPath_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextPath_pos'><code><strong>pos</strong></code></a></td>
    <td>positions of each glyph</td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextPath_path'><code><strong>path</strong></code></a></td>
    <td>geometry of the <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7f27c93472aa99a7542fb3493076f072"><div>Simplifies three <a href='undocumented#Glyph'>Glyphs</a> to eliminate overlaps</div></fiddle-embed></div>

---

## <a name='Text_Intercepts'>Text Intercepts</a>

<a href='#Text_Intercepts'>Text Intercepts</a> describe the intersection of drawn text <a href='undocumented#Glyph'>Glyphs</a> with a pair
of lines parallel to the text advance

<a name='SkPaint_getTextIntercepts'></a>
## getTextIntercepts

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getTextIntercepts'>getTextIntercepts</a>(const void
</pre>

Returns the number of <a href='#SkPaint_getTextIntercepts_intervals'>intervals</a> that intersect <a href='#SkPaint_getTextIntercepts_bounds'>bounds</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextIntercepts_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getTextIntercepts_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_x'><code><strong>x</strong></code></a></td>
    <td><a href='#SkPaint_getTextIntercepts_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_y'><code><strong>y</strong></code></a></td>
    <td><a href='#SkPaint_getTextIntercepts_y'>y</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_bounds'><code><strong>bounds</strong></code></a></td>
    <td>lower and upper line parallel to the advance</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_intervals'><code><strong>intervals</strong></code></a></td>
    <td>returned intersections</td>
  </tr>
</table>

### Return Value

number of intersections

### Example

<div><fiddle-embed name="2a0b80ed20d193c688085b79deb5bdc9"><div>Underline uses intercepts to draw on either side of the glyph Descender</div></fiddle-embed></div>

---

<a name='SkPaint_getPosTextIntercepts'></a>
## getPosTextIntercepts

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getPosTextIntercepts'>getPosTextIntercepts</a>(const void
</pre>

Returns the number of <a href='#SkPaint_getPosTextIntercepts_intervals'>intervals</a> that intersect <a href='#SkPaint_getPosTextIntercepts_bounds'>bounds</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_getPosTextIntercepts_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices</td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextIntercepts_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getPosTextIntercepts_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextIntercepts_pos'><code><strong>pos</strong></code></a></td>
    <td>positions of each glyph</td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextIntercepts_bounds'><code><strong>bounds</strong></code></a></td>
    <td>lower and upper line parallel to the advance</td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextIntercepts_intervals'><code><strong>intervals</strong></code></a></td>
    <td>returned intersections</td>
  </tr>
</table>

### Return Value

number of intersections

### Example

<div><fiddle-embed name="98b2dfc552d0540a7c041fe7a2839bd7"><div><a href='undocumented#Text'>Text</a> intercepts draw on either side of</div></fiddle-embed></div>

---

<a name='SkPaint_getPosTextHIntercepts'></a>
## getPosTextHIntercepts

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getPosTextHIntercepts'>getPosTextHIntercepts</a>(const void
</pre>

Returns the number of <a href='#SkPaint_getPosTextHIntercepts_intervals'>intervals</a> that intersect <a href='#SkPaint_getPosTextHIntercepts_bounds'>bounds</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_getPosTextHIntercepts_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices</td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextHIntercepts_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getPosTextHIntercepts_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextHIntercepts_xpos'><code><strong>xpos</strong></code></a></td>
    <td>positions of each glyph in x</td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextHIntercepts_constY'><code><strong>constY</strong></code></a></td>
    <td>position of each glyph in y</td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextHIntercepts_bounds'><code><strong>bounds</strong></code></a></td>
    <td>lower and upper line parallel to the advance</td>
  </tr>
  <tr>    <td><a name='SkPaint_getPosTextHIntercepts_intervals'><code><strong>intervals</strong></code></a></td>
    <td>returned intersections</td>
  </tr>
</table>

### Return Value

number of intersections

### Example

<div><fiddle-embed name="dc9851c43acc3716aca8c9a4d40d452d"><div><a href='undocumented#Text'>Text</a> intercepts do not take stroke thickness into consideration</div></fiddle-embed></div>

---

<a name='SkPaint_getTextBlobIntercepts'></a>
## getTextBlobIntercepts

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getTextBlobIntercepts'>getTextBlobIntercepts</a>(const <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>
</pre>

Returns the number of <a href='#SkPaint_getTextBlobIntercepts_intervals'>intervals</a> that intersect <a href='#SkPaint_getTextBlobIntercepts_bounds'>bounds</a>

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextBlobIntercepts_blob'><code><strong>blob</strong></code></a></td>
    <td><a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextBlobIntercepts_bounds'><code><strong>bounds</strong></code></a></td>
    <td>lower and upper line parallel to the advance</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextBlobIntercepts_intervals'><code><strong>intervals</strong></code></a></td>
    <td>returned intersections</td>
  </tr>
</table>

### Return Value

number of intersections

### Example

<div><fiddle-embed name="f2229dd5c8e76f9e12fafe59b61353c8"></fiddle-embed></div>

---

<a name='SkPaint_nothingToDraw'></a>
## nothingToDraw

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_nothingToDraw'>nothingToDraw</a>(
</pre>

Returns true if <a href='#Paint'>Paint</a> prevents all drawing

### Return Value

true if <a href='#Paint'>Paint</a> prevents all drawing

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

---

## <a name='Fast_Bounds'>Fast Bounds</a>

Private: To be made private.

<a href='#Fast_Bounds'>Fast Bounds</a> functions conservatively outset a drawing bounds by additional area
<a href='#Paint'>Paint</a> may draw to

<a name='SkPaint_canComputeFastBounds'></a>
## canComputeFastBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_canComputeFastBounds'>canComputeFastBounds</a>(
</pre>

Private: (to be made private)

Returns true if <a href='#Paint'>Paint</a> does not include elements requiring extensive computation
to compute <a href='undocumented#Device'>Device</a> bounds of drawn geometry

### Return Value

true if <a href='#Paint'>Paint</a> allows for fast computation of bounds

---

<a name='SkPaint_computeFastBounds'></a>
## computeFastBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Private: (to be made private)

Only call this if <a href='#SkPaint_canComputeFastBounds'>canComputeFastBounds</a> returned truePrivate: For example:
    if (!path.isInverseFillType() && paint.canComputeFastBounds()) {
        SkRect storage;
        if (canvas->quickReject(paint.computeFastBounds(path.getBounds(), &storage))) {
            return; // do not draw the path
        }
    }
    // draw the path

### Parameters

<table>  <tr>    <td><a name='SkPaint_computeFastBounds_orig'><code><strong>orig</strong></code></a></td>
    <td>geometry modified by <a href='#Paint'>Paint</a> when drawn</td>
  </tr>
  <tr>    <td><a name='SkPaint_computeFastBounds_storage'><code><strong>storage</strong></code></a></td>
    <td>computed bounds of geometry</td>
  </tr>
</table>

### Return Value

fast computed bounds

---

<a name='SkPaint_computeFastStrokeBounds'></a>
## computeFastStrokeBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Private: (to be made private)

### Parameters

<table>  <tr>    <td><a name='SkPaint_computeFastStrokeBounds_orig'><code><strong>orig</strong></code></a></td>
    <td>geometry modified by <a href='#Paint'>Paint</a> when drawn</td>
  </tr>
  <tr>    <td><a name='SkPaint_computeFastStrokeBounds_storage'><code><strong>storage</strong></code></a></td>
    <td>computed bounds of geometry</td>
  </tr>
</table>

### Return Value

fast computed bounds

---

<a name='SkPaint_doComputeFastBounds'></a>
## doComputeFastBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Private: (to be made private)

Computes the bounds

### Parameters

<table>  <tr>    <td><a name='SkPaint_doComputeFastBounds_orig'><code><strong>orig</strong></code></a></td>
    <td>geometry modified by <a href='#Paint'>Paint</a> when drawn</td>
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

---

## <a name='Utility'>Utility</a>

