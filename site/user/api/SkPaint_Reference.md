SkPaint Reference
===

# <a name="Paint"></a> Paint

# <a name="SkPaint"></a> Class SkPaint
<a href="#Paint">Paint</a> controls options applied when drawing and measuring. <a href="#Paint">Paint</a> collects all
options outside of the <a href="#Clip">Canvas Clip</a> and <a href="#Matrix">Canvas Matrix</a>.

Various options apply to text, strokes and fills, and images. 

Some options may not be implemented on all platforms; in these cases, setting
the option has no effect. Some options are conveniences that duplicate <a href="SkCanvas_Reference#Canvas">Canvas</a>
functionality; for instance, text size is identical to matrix scale.

<a href="#Paint">Paint</a> options are rarely exclusive; each option modifies a stage of the drawing
pipeline and multiple pipeline stages may be affected by a single <a href="#Paint">Paint</a>.

<a href="#Paint">Paint</a> collects effects and filters that describe single-pass and multiple-pass 
algorithms that alter the drawing geometry, color, and transparency. For instance,
<a href="#Paint">Paint</a> does not directly implement dashing or blur, but contains the objects that do so. 

The objects contained by <a href="#Paint">Paint</a> are opaque, and cannot be edited outside of the <a href="#Paint">Paint</a>
to affect it. The implementation is free to defer computations associated with the
<a href="#Paint">Paint</a>, or ignore them altogether. For instance, some <a href="undocumented#GPU">GPU</a> implementations draw all
<a href="SkPath_Reference#Path">Path</a> geometries with Anti-aliasing, regardless of how <a href="#SkPaint_kAntiAlias_Flag">SkPaint::kAntiAlias Flag</a> 
is set in <a href="#Paint">Paint</a>.

<a href="#Paint">Paint</a> describes a single color, a single font, a single image quality, and so on.
Multiple colors are drawn either by using multiple paints or with objects like
<a href="undocumented#Shader">Shader</a> attached to <a href="#Paint">Paint</a>.

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |
| <a href="#Initializers">Initializers</a> | Constructors and initialization. |
| <a href="undocumented#Destructor">Destructor</a> | <a href="#Paint">Paint</a> termination. |
| <a href="#Management">Management</a> | <a href="#Paint">Paint</a> copying, moving, comparing. |
| <a href="#SkPaint_Hinting">Hinting</a> | <a href="undocumented#Glyph">Glyph</a> outline adjustment. |
| <a href="#SkPaint_Flags">Flags</a> | Attributes represented by single bits. |
| Anti-alias | Approximating coverage with transparency. |
| <a href="#Dither">Dither</a> | Distributing color error. |
| <a href="#Device_Text">Device Text</a> | Increase precision of glyph position. |
| <a href="SkPaint_Reference#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a> | Custom sized bitmap <a href="#Glyph">Glyphs</a>. |
| <a href="#Automatic_Hinting">Automatic Hinting</a> | Always adjust glyph paths. |
| <a href="#Vertical_Text">Vertical Text</a> | Orient text from top to bottom. |
| <a href="#Fake_Bold">Fake Bold</a> | Approximate font styles. |
| <a href="SkPaint_Reference#Full_Hinting_Spacing">Full Hinting Spacing</a> | <a href="undocumented#Glyph">Glyph</a> spacing affected by hinting. |
| <a href="#Filter_Quality_Methods">Filter Quality Methods</a> | Get and set <a href="undocumented#Filter_Quality">Filter Quality</a>. |
| <a href="#Color_Methods">Color Methods</a> | Get and set <a href="undocumented#Color">Color</a>. |
| <a href="#SkPaint_Style">Style</a> | Geometry filling, stroking. |
| <a href="#Stroke_Width">Stroke Width</a> | Thickness perpendicular to geometry. |
| <a href="#Miter_Limit">Miter Limit</a> | Maximum length of stroked corners. |
| <a href="#Stroke_Cap">Stroke Cap</a> | Decorations at ends of open strokes. |
| <a href="#Stroke_Join">Stroke Join</a> | Decoration at corners of strokes. |
| <a href="#Fill_Path">Fill Path</a> | Make <a href="SkPath_Reference#Path">Path</a> from <a href="undocumented#Path_Effect">Path Effect</a>, stroking. |
| <a href="#Shader_Methods">Shader Methods</a> | Get and set <a href="undocumented#Shader">Shader</a>. |
| <a href="#Color_Filter_Methods">Color Filter Methods</a> | Get and set <a href="undocumented#Color_Filter">Color Filter</a>. |
| <a href="#Blend_Mode_Methods">Blend Mode Methods</a> | Get and set <a href="undocumented#Blend_Mode">Blend Mode</a>. |
| <a href="#Path_Effect_Methods">Path Effect Methods</a> | Get and set <a href="undocumented#Path_Effect">Path Effect</a>. |
| <a href="#Mask_Filter_Methods">Mask Filter Methods</a> | Get and set <a href="undocumented#Mask_Filter">Mask Filter</a>. |
| <a href="#Typeface_Methods">Typeface Methods</a> | Get and set <a href="undocumented#Typeface">Typeface</a>. |
| <a href="#Rasterizer_Methods">Rasterizer Methods</a> | Get and set <a href="undocumented#Rasterizer">Rasterizer</a>. |
| <a href="#Image_Filter_Methods">Image Filter Methods</a> | Get and set <a href="undocumented#Image_Filter">Image Filter</a>. |
| <a href="#Draw_Looper_Methods">Draw Looper Methods</a> | Get and set <a href="undocumented#Draw_Looper">Draw Looper</a>. |
| <a href="#Text_Align">Text Align</a> | <a href="undocumented#Text">Text</a> placement relative to position. |
| <a href="#Text_Size">Text Size</a> | Overall height in points. |
| <a href="#Text_Scale_X">Text Scale X</a> | <a href="undocumented#Text">Text</a> horizontal scale. |
| <a href="#Text_Skew_X">Text Skew X</a> | <a href="undocumented#Text">Text</a> horizontal slant. |
| <a href="#Text_Encoding">Text Encoding</a> | <a href="undocumented#Text">Text</a> encoded as characters or <a href="#Glyph">Glyphs</a>. |
| <a href="#Font_Metrics">Font Metrics</a> | Common glyph dimensions. |
| <a href="#Measure_Text">Measure Text</a> | Width, height, bounds of text. |
| <a href="#Text_Path">Text Path</a> | Geometry of <a href="#Glyph">Glyphs</a>. |
| <a href="#Text_Intercepts">Text Intercepts</a> | Advanced underline, strike through. |
| <a href="#Fast_Bounds">Fast Bounds</a> | Approximate area required by <a href="#Paint">Paint</a>. |

## <a name="Constants"></a> Constants

| constants | description |
| --- | ---  |
| <a href="#SkPaint_Align">Align</a> | <a href="undocumented#Glyph">Glyph</a> locations relative to text position. |
| <a href="#SkPaint_Cap">Cap</a> | Start and end geometry on stroked shapes. |
| <a href="#SkPaint_Flags">Flags</a> | Values described by bits and masks. |
| <a href="#SkPaint_FontMetrics_FontMetricsFlags">FontMetrics::FontMetricsFlags</a> | Valid <a href="#Font_Metrics">Font Metrics</a>. |
| <a href="#SkPaint_Hinting">Hinting</a> | Level of glyph outline adjustment. |
| <a href="#SkPaint_Join">Join</a> | Corner geometry on stroked shapes. |
| <a href="#SkPaint_Style">Style</a> | Stroke, fill, or both. |
| <a href="#SkPaint_TextEncoding">TextEncoding</a> | Character or glyph encoded size. |

## <a name="Structs"></a> Structs

| struct | description |
| --- | ---  |
| <a href="#SkPaint_FontMetrics">FontMetrics</a> | <a href="undocumented#Typeface">Typeface</a> values. |

## <a name="Constructors"></a> Constructors

|  | description |
| --- | ---  |
| <a href="#SkPaint_empty_constructor">SkPaint()</a> | Constructs with default values. |
| <a href="#SkPaint_copy_const_SkPaint">SkPaint(const SkPaint& paint)</a> | Makes a shallow copy. |
| <a href="#SkPaint_move_SkPaint">SkPaint(SkPaint&& paint)</a> | Moves paint without copying it. |
|  | Decreases <a href="undocumented#Reference_Count">Reference Count</a> of owned objects. |

## <a name="Operators"></a> Operators

| operator | description |
| --- | ---  |
| <a href="#SkPaint_copy_operator">operator=(const SkPaint& paint)</a> | Makes a shallow copy. |
| <a href="#SkPaint_move_operator">operator=(SkPaint&& paint)</a> | Moves paint without copying it. |
| <a href="#SkPaint_equal_operator">operator==(const SkPaint& a, const SkPaint& b)</a> | Compares paints for equality. |
| <a href="#SkPaint_notequal_operator">operator!=(const SkPaint& a, const SkPaint& b)</a> | Compares paints for inequality. |

## <a name="Member_Functions"></a> Member Functions

| function | description |
| --- | ---  |
| <a href="#SkPaint_breakText">breakText</a> | Returns text that fits in a width. |
| <a href="#SkPaint_canComputeFastBounds">canComputeFastBounds</a> | Returns true if settings allow for fast bounds computation. |
| <a href="#SkPaint_computeFastBounds">computeFastBounds</a> | Returns fill bounds for quick reject tests. |
| <a href="#SkPaint_computeFastStrokeBounds">computeFastStrokeBounds</a> | Returns stroke bounds for quick reject tests. |
| <a href="#SkPaint_containsText">containsText</a> | Returns if all text corresponds to <a href="#Glyph">Glyphs</a>. |
| <a href="#SkPaint_countText">countText</a> | Returns number of <a href="#Glyph">Glyphs</a> in text. |
| <a href="#SkPaint_doComputeFastBounds">doComputeFastBounds</a> | Returns bounds for quick reject tests. |
| <a href="#SkPaint_flatten">flatten</a> | Serializes into a buffer. |
| <a href="#SkPaint_getAlpha">getAlpha</a> | Returns <a href="#Alpha">Color Alpha</a>, color opacity. |
| <a href="#SkPaint_getBlendMode">getBlendMode</a> | Returns <a href="undocumented#Blend_Mode">Blend Mode</a>, how colors combine with <a href="undocumented#Device">Device</a>. |
| <a href="#SkPaint_getColor">getColor</a> | Returns <a href="#Alpha">Color Alpha</a> and <a href="#RGB">Color RGB</a>, one drawing color. |
| <a href="#SkPaint_getColorFilter">getColorFilter</a> | Returns <a href="undocumented#Color_Filter">Color Filter</a>, how colors are altered. |
| <a href="#SkPaint_getDrawLooper">getDrawLooper</a> | Returns <a href="undocumented#Draw_Looper">Draw Looper</a>, multiple layers. |
| <a href="#SkPaint_getFillPath">getFillPath</a> | Returns fill path equivalent to stroke. |
| <a href="#SkPaint_getFilterQuality">getFilterQuality</a> | Returns <a href="undocumented#Filter_Quality">Filter Quality</a>, image filtering level. |
| <a href="#SkPaint_getFlags">getFlags</a> | Returns <a href="#SkPaint_Flags">Flags</a> stored in a bit field. |
| <a href="#SkPaint_getFontBounds">getFontBounds</a> | Returns union all glyph bounds. |
| <a href="#SkPaint_getFontMetrics">getFontMetrics</a> | Returns <a href="undocumented#Typeface">Typeface</a> metrics scaled by text size. |
| <a href="#SkPaint_getFontSpacing">getFontSpacing</a> | Returns recommended spacing between lines. |
| <a href="#SkPaint_getHash">getHash</a> | Returns a shallow hash for equality checks. |
| <a href="#SkPaint_getHinting">getHinting</a> | Returns <a href="#SkPaint_Hinting">Hinting</a>, glyph outline adjustment level. |
| <a href="#SkPaint_getImageFilter">getImageFilter</a> | Returns <a href="undocumented#Image_Filter">Image Filter</a>, alter pixels; blur. |
| <a href="#SkPaint_getMaskFilter">getMaskFilter</a> | Returns <a href="undocumented#Mask_Filter">Mask Filter</a>, alterations to <a href="undocumented#Mask_Alpha">Mask Alpha</a>. |
| <a href="#SkPaint_getPathEffect">getPathEffect</a> | Returns <a href="undocumented#Path_Effect">Path Effect</a>, modifications to path geometry; dashing. |
| <a href="#SkPaint_getPosTextPath">getPosTextPath</a> | Returns <a href="SkPath_Reference#Path">Path</a> equivalent to positioned text. |
| <a href="#SkPaint_getPosTextIntercepts">getPosTextIntercepts</a> | Returns where lines intersect positioned text; underlines. |
| <a href="#SkPaint_getPosTextHIntercepts">getPosTextHIntercepts</a> | Returns where lines intersect horizontally positioned text; underlines. |
| <a href="#SkPaint_getRasterizer">getRasterizer</a> | Returns <a href="undocumented#Rasterizer">Rasterizer</a>, <a href="undocumented#Mask_Alpha">Mask Alpha</a> generation from <a href="SkPath_Reference#Path">Path</a>. |
| <a href="#SkPaint_getShader">getShader</a> | Returns <a href="undocumented#Shader">Shader</a>, multiple drawing colors; gradients. |
| <a href="#SkPaint_getStrokeCap">getStrokeCap</a> | Returns <a href="#SkPaint_Cap">Cap</a>, the area drawn at path ends. |
| <a href="#SkPaint_getStrokeJoin">getStrokeJoin</a> | Returns <a href="#SkPaint_Join">Join</a>, geometry on path corners. |
| <a href="#SkPaint_getStrokeMiter">getStrokeMiter</a> | Returns <a href="#Miter_Limit">Miter Limit</a>, angles with sharp corners. |
| <a href="#SkPaint_getStrokeWidth">getStrokeWidth</a> | Returns thickness of the stroke. |
| <a href="#SkPaint_getStyle">getStyle</a> | Returns <a href="#SkPaint_Style">Style</a>: stroke, fill, or both. |
| <a href="#SkPaint_getTextAlign">getTextAlign</a> | Returns <a href="#SkPaint_Align">Align</a>: left, center, or right. |
| <a href="#SkPaint_getTextBlobIntercepts">getTextBlobIntercepts</a> | Returns where lines intersect <a href="undocumented#Text_Blob">Text Blob</a>; underlines. |
| <a href="#SkPaint_getTextEncoding">getTextEncoding</a> | Returns character or glyph encoded size.    b |
| <a href="#SkPaint_getTextIntercepts">getTextIntercepts</a> | Returns where lines intersect text; underlines. |
| <a href="#SkPaint_getTextPath">getTextPath</a> | Returns <a href="SkPath_Reference#Path">Path</a> equivalent to text. |
| <a href="#SkPaint_getTextScaleX">getTextScaleX</a> | Returns the text horizontal scale; condensed text. |
| <a href="#SkPaint_getTextSkewX">getTextSkewX</a> | Returns the text horizontal skew; oblique text. |
| <a href="#SkPaint_getTextSize">getTextSize</a> | Returns text size in points. |
| <a href="#SkPaint_getTextWidths">getTextWidths</a> | Returns advance and bounds for each glyph in text. |
| <a href="#SkPaint_getTypeface">getTypeface</a> | Returns <a href="undocumented#Typeface">Typeface</a>, font description. |
| <a href="#SkPaint_glyphsToUnichars">glyphsToUnichars</a> | Converts <a href="#Glyph">Glyphs</a> into text. |
| <a href="#SkPaint_isAntiAlias">isAntiAlias</a> | Returns true if Anti-alias is set. |
| <a href="#SkPaint_isAutohinted">isAutohinted</a> | Returns true if <a href="#Glyph">Glyphs</a> are always hinted. |
| <a href="#SkPaint_isDevKernText">isDevKernText</a> | Returns true if <a href="SkPaint_Reference#Full_Hinting_Spacing">Full Hinting Spacing</a> is set. |
| <a href="#SkPaint_isDither">isDither</a> | Returns true if <a href="#Dither">Dither</a> is set. |
| <a href="#SkPaint_isEmbeddedBitmapText">isEmbeddedBitmapText</a> | Returns true if <a href="SkPaint_Reference#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a> is set. |
| <a href="#SkPaint_isFakeBoldText">isFakeBoldText</a> | Returns true if <a href="#Fake_Bold">Fake Bold</a> is set. |
| <a href="#SkPaint_isLCDRenderText">isLCDRenderText</a> | Returns true if <a href="SkPaint_Reference#LCD_Text">LCD Text</a> is set. |
| <a href="#SkPaint_isSrcOver">isSrcOver</a> | Returns true if <a href="undocumented#Blend_Mode">Blend Mode</a> is <a href="#SkBlendMode_kSrcOver">SkBlendMode::kSrcOver</a>. |
| <a href="#SkPaint_isSubpixelText">isSubpixelText</a> | Returns true if <a href="SkPaint_Reference#Subpixel_Text">Subpixel Text</a> is set. |
| <a href="#SkPaint_isVerticalText">isVerticalText</a> | Returns true if <a href="#Vertical_Text">Vertical Text</a> is set. |
| <a href="#SkPaint_measureText">measureText</a> | Returns advance width and bounds of text. |
| <a href="#SkPaint_nothingToDraw">nothingToDraw</a> | Returns true if <a href="#Paint">Paint</a> prevents all drawing. |
| <a href="#SkPaint_refColorFilter">refColorFilter</a> | References <a href="undocumented#Color_Filter">Color Filter</a>, how colors are altered. |
| <a href="#SkPaint_refDrawLooper">refDrawLooper</a> | References <a href="undocumented#Draw_Looper">Draw Looper</a>, multiple layers. |
| <a href="#SkPaint_refImageFilter">refImageFilter</a> | References <a href="undocumented#Image_Filter">Image Filter</a>, alter pixels; blur. |
| <a href="#SkPaint_refMaskFilter">refMaskFilter</a> | References <a href="undocumented#Mask_Filter">Mask Filter</a>, alterations to <a href="undocumented#Mask_Alpha">Mask Alpha</a>. |
| <a href="#SkPaint_refPathEffect">refPathEffect</a> | References <a href="undocumented#Path_Effect">Path Effect</a>, modifications to path geometry; dashing. |
| <a href="#SkPaint_refRasterizer">refRasterizer</a> | References <a href="undocumented#Rasterizer">Rasterizer</a>, mask generation from path. |
| <a href="#SkPaint_refShader">refShader</a> | References <a href="undocumented#Shader">Shader</a>, multiple drawing colors; gradients. |
| <a href="#SkPaint_refTypeface">refTypeface</a> | References <a href="undocumented#Typeface">Typeface</a>, font description. |
| <a href="#SkPaint_reset">reset</a> | Sets to default values. |
| <a href="#SkPaint_setAlpha">setAlpha</a> | Sets <a href="#Alpha">Color Alpha</a>, color opacity. |
| <a href="#SkPaint_setAntiAlias">setAntiAlias</a> | Sets or clears Anti-alias. |
| <a href="#SkPaint_setARGB">setARGB</a> | Sets color by component. |
| <a href="#SkPaint_setAutohinted">setAutohinted</a> | Sets <a href="#Glyph">Glyphs</a> to always be hinted. |
| <a href="#SkPaint_setBlendMode">setBlendMode</a> | Sets <a href="undocumented#Blend_Mode">Blend Mode</a>, how colors combine with destination. |
| <a href="#SkPaint_setColor">setColor</a> | Sets <a href="#Alpha">Color Alpha</a> and <a href="#RGB">Color RGB</a>, one drawing color. |
| <a href="#SkPaint_setColorFilter">setColorFilter</a> | Sets <a href="undocumented#Color_Filter">Color Filter</a>, alters color. |
| <a href="#SkPaint_setDevKernText">setDevKernText</a> | Sets or clears <a href="SkPaint_Reference#Full_Hinting_Spacing">Full Hinting Spacing</a>. |
| <a href="#SkPaint_setDither">setDither</a> | Sets or clears <a href="#Dither">Dither</a>. |
| <a href="#SkPaint_setDrawLooper">setDrawLooper</a> | Sets <a href="undocumented#Draw_Looper">Draw Looper</a>, multiple layers. |
| <a href="#SkPaint_setEmbeddedBitmapText">setEmbeddedBitmapText</a> | Sets or clears <a href="SkPaint_Reference#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a>. |
| <a href="#SkPaint_setFakeBoldText">setFakeBoldText</a> | Sets or clears <a href="#Fake_Bold">Fake Bold</a>. |
| <a href="#SkPaint_setFilterQuality">setFilterQuality</a> | Sets <a href="undocumented#Filter_Quality">Filter Quality</a>, the image filtering level. |
| <a href="#SkPaint_setFlags">setFlags</a> | Sets multiple <a href="#SkPaint_Flags">Flags</a> in a bit field. |
| <a href="#SkPaint_setHinting">setHinting</a> | Sets <a href="#SkPaint_Hinting">Hinting</a>, glyph outline adjustment level. |
| <a href="#SkPaint_setLCDRenderText">setLCDRenderText</a> | Sets or clears <a href="SkPaint_Reference#LCD_Text">LCD Text</a>. |
| <a href="#SkPaint_setMaskFilter">setMaskFilter</a> | Sets <a href="undocumented#Mask_Filter">Mask Filter</a>, alterations to <a href="undocumented#Mask_Alpha">Mask Alpha</a>. |
| <a href="#SkPaint_setPathEffect">setPathEffect</a> | Sets <a href="undocumented#Path_Effect">Path Effect</a>, modifications to path geometry; dashing. |
| <a href="#SkPaint_setRasterizer">setRasterizer</a> | Sets <a href="undocumented#Rasterizer">Rasterizer</a>, <a href="undocumented#Mask_Alpha">Mask Alpha</a> generation from <a href="SkPath_Reference#Path">Path</a>. |
| <a href="#SkPaint_setImageFilter">setImageFilter</a> | Sets <a href="undocumented#Image_Filter">Image Filter</a>, alter pixels; blur. |
| <a href="#SkPaint_setShader">setShader</a> | Sets <a href="undocumented#Shader">Shader</a>, multiple drawing colors; gradients. |
| <a href="#SkPaint_setStrokeCap">setStrokeCap</a> | Sets <a href="#SkPaint_Cap">Cap</a>, the area drawn at path ends. |
| <a href="#SkPaint_setStrokeJoin">setStrokeJoin</a> | Sets <a href="#SkPaint_Join">Join</a>, geometry on path corners. |
| <a href="#SkPaint_setStrokeMiter">setStrokeMiter</a> | Sets <a href="#Miter_Limit">Miter Limit</a>, angles with sharp corners. |
| <a href="#SkPaint_setStrokeWidth">setStrokeWidth</a> | Sets thickness of the stroke. |
| <a href="#SkPaint_setStyle">setStyle</a> | Sets <a href="#SkPaint_Style">Style</a>: stroke, fill, or both. |
| <a href="#SkPaint_setSubpixelText">setSubpixelText</a> | Sets or clears <a href="SkPaint_Reference#Subpixel_Text">Subpixel Text</a>. |
| <a href="#SkPaint_setTextAlign">setTextAlign</a> | Sets <a href="#SkPaint_Align">Align</a>: left, center, or right. |
| <a href="#SkPaint_setTextEncoding">setTextEncoding</a> | Sets character or glyph encoded size. |
| <a href="#SkPaint_setTextScaleX">setTextScaleX</a> | Sets the text horizontal scale; condensed text. |
| <a href="#SkPaint_setTextSkewX">setTextSkewX</a> | Sets the text horizontal skew; oblique text. |
| <a href="#SkPaint_setTextSize">setTextSize</a> | Sets text size in points. |
| <a href="#SkPaint_setTypeface">setTypeface</a> | Sets <a href="undocumented#Typeface">Typeface</a>, font description. |
| <a href="#SkPaint_setVerticalText">setVerticalText</a> | Sets or clears <a href="#Vertical_Text">Vertical Text</a>. |
| <a href="#SkPaint_textToGlyphs">textToGlyphs</a> | Converts text into glyph indices. |
| <a href="#SkPaint_toString">toString</a> | Converts <a href="#Paint">Paint</a> to machine readable form. |
| <a href="#SkPaint_unflatten">unflatten</a> | Populates from a serialized stream. |

# <a name="Initializers"></a> Initializers

<a name="SkPaint_empty_constructor"></a>
## SkPaint

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPaint()
</pre>

Constructs <a href="#Paint">Paint</a> with default values.

| attribute | default value |
| --- | ---  |
| Anti-alias | false |
| <a href="undocumented#Blend_Mode">Blend Mode</a> | <a href="#SkBlendMode_kSrcOver">SkBlendMode::kSrcOver</a> |
| <a href="undocumented#Color">Color</a> | <a href="undocumented#SK_ColorBLACK">SK ColorBLACK</a> |
| <a href="#Alpha">Color Alpha</a> | 255 |
| <a href="undocumented#Color_Filter">Color Filter</a> | nullptr |
| <a href="#Dither">Dither</a> | false |
| <a href="undocumented#Draw_Looper">Draw Looper</a> | nullptr |
| <a href="#Fake_Bold">Fake Bold</a> | false |
| <a href="undocumented#Filter_Quality">Filter Quality</a> | <a href="undocumented#SkFilterQuality">kNone SkFilterQuality</a> |
| <a href="SkPaint_Reference#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a> | false |
| <a href="#Automatic_Hinting">Automatic Hinting</a> | false |
| <a href="SkPaint_Reference#Full_Hinting_Spacing">Full Hinting Spacing</a> | false |
| <a href="#SkPaint_Hinting">Hinting</a> | <a href="#SkPaint_kNormal_Hinting">kNormal Hinting</a> |
| <a href="undocumented#Image_Filter">Image Filter</a> | nullptr |
| <a href="SkPaint_Reference#LCD_Text">LCD Text</a> | false |
| <a href="SkPaint_Reference#Linear_Text">Linear Text</a> | false |
| <a href="#Miter_Limit">Miter Limit</a> | 4 |
| <a href="undocumented#Mask_Filter">Mask Filter</a> | nullptr |
| <a href="undocumented#Path_Effect">Path Effect</a> | nullptr |
| <a href="undocumented#Rasterizer">Rasterizer</a> | nullptr |
| <a href="undocumented#Shader">Shader</a> | nullptr |
| <a href="#SkPaint_Style">Style</a> | <a href="#SkPaint_kFill_Style">kFill Style</a> |
| <a href="#Text_Align">Text Align</a> | <a href="#SkPaint_kLeft_Align">kLeft Align</a> |
| <a href="#Text_Encoding">Text Encoding</a> | <a href="#SkPaint_kUTF8_TextEncoding">kUTF8 TextEncoding</a> |
| <a href="#Text_Scale_X">Text Scale X</a> | 1 |
| <a href="#Text_Size">Text Size</a> | 12 |
| <a href="#Text_Skew_X">Text Skew X</a> | 0 |
| <a href="undocumented#Typeface">Typeface</a> | nullptr |
| <a href="#Stroke_Cap">Stroke Cap</a> | <a href="#SkPaint_kButt_Cap">kButt Cap</a> |
| <a href="#Stroke_Join">Stroke Join</a> | <a href="#SkPaint_kMiter_Join">kMiter Join</a> |
| <a href="#Stroke_Width">Stroke Width</a> | 0 |
| <a href="SkPaint_Reference#Subpixel_Text">Subpixel Text</a> | false |
| <a href="#Vertical_Text">Vertical Text</a> | false |

The flags, text size, hinting, and miter limit may be overridden at compile time by defining
paint default values. The overrides may be included in "<a href="undocumented#SkUserConfig">SkUserConfig</a>.h" or predefined by the 
build system.

### Return Value

default initialized <a href="#Paint">Paint</a>

### Example

<div><fiddle-embed name="c4b2186d85c142a481298f7144295ffd"></fiddle-embed></div>

---

<a name="SkPaint_copy_const_SkPaint"></a>
## SkPaint

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPaint(const SkPaint& paint)
</pre>

Makes a shallow copy of <a href="#Paint">Paint</a>. <a href="undocumented#Typeface">Typeface</a>, <a href="undocumented#Path_Effect">Path Effect</a>, <a href="undocumented#Shader">Shader</a>,
<a href="undocumented#Mask_Filter">Mask Filter</a>, <a href="undocumented#Color_Filter">Color Filter</a>, <a href="undocumented#Rasterizer">Rasterizer</a>, <a href="undocumented#Draw_Looper">Draw Looper</a>, and <a href="undocumented#Image_Filter">Image Filter</a> are shared
between the original <a href="#SkPaint_copy_const_SkPaint_paint">paint</a> and the copy. Objects containing <a href="undocumented#Reference_Count">Reference Count</a> increment
their references by one.

The referenced objects <a href="undocumented#Path_Effect">Path Effect</a>, <a href="undocumented#Shader">Shader</a>, <a href="undocumented#Mask_Filter">Mask Filter</a>, <a href="undocumented#Color_Filter">Color Filter</a>, <a href="undocumented#Rasterizer">Rasterizer</a>,
<a href="undocumented#Draw_Looper">Draw Looper</a>, and <a href="undocumented#Image_Filter">Image Filter</a> cannot be modified after they are created.
This prevents objects with <a href="undocumented#Reference_Count">Reference Count</a> from being modified once <a href="#Paint">Paint</a> refers to them.

### Parameters

<table>  <tr>    <td><a name="SkPaint_copy_const_SkPaint_paint"> <code><strong>paint </strong></code> </a></td> <td>
original to copy</td>
  </tr>
</table>

### Return Value

shallow copy of <a href="#SkPaint_copy_const_SkPaint_paint">paint</a>

### Example

<div><fiddle-embed name="b99971ad0ef243d617925289d963b62d">

#### Example Output

~~~~
SK_ColorRED == paint1.getColor()
SK_ColorBLUE == paint2.getColor()
~~~~

</fiddle-embed></div>

---

<a name="SkPaint_move_SkPaint"></a>
## SkPaint

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPaint(SkPaint&& paint)
</pre>

Implements a move constructor to avoid increasing the reference counts
of objects referenced by the <a href="#SkPaint_move_SkPaint_paint">paint</a>.

After the call, <a href="#SkPaint_move_SkPaint_paint">paint</a> is undefined, and can be safely destructed.

### Parameters

<table>  <tr>    <td><a name="SkPaint_move_SkPaint_paint"> <code><strong>paint </strong></code> </a></td> <td>
original to move</td>
  </tr>
</table>

### Return Value

content of <a href="#SkPaint_move_SkPaint_paint">paint</a>

### Example

<div><fiddle-embed name="8ed1488a503cd5282b86a51614aa90b1">

#### Example Output

~~~~
path effect unique: true
~~~~

</fiddle-embed></div>

---

<a name="SkPaint_reset"></a>
## reset

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void reset()
</pre>

Sets all <a href="#Paint">Paint</a> contents to their initial values. This is equivalent to replacing
<a href="#Paint">Paint</a> with the result of <a href="#SkPaint_empty_constructor">SkPaint()</a>.

### Example

<div><fiddle-embed name="ef269937ade7e7353635121d9a64f9f7">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

# <a name="Destructor"></a> Destructor

<a name="SkPaint_destructor"></a>
## ~SkPaint

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
~SkPaint()
</pre>

Decreases <a href="#Paint">Paint</a> <a href="undocumented#Reference_Count">Reference Count</a> of owned objects: <a href="undocumented#Typeface">Typeface</a>, <a href="undocumented#Path_Effect">Path Effect</a>, <a href="undocumented#Shader">Shader</a>,
<a href="undocumented#Mask_Filter">Mask Filter</a>, <a href="undocumented#Color_Filter">Color Filter</a>, <a href="undocumented#Rasterizer">Rasterizer</a>, <a href="undocumented#Draw_Looper">Draw Looper</a>, and <a href="undocumented#Image_Filter">Image Filter</a>. If the
objects containing <a href="undocumented#Reference_Count">Reference Count</a> go to zero, they are deleted.

---

# <a name="Management"></a> Management

<a name="SkPaint_copy_operator"></a>
## operator=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPaint& operator=(const SkPaint& paint)
</pre>

Makes a shallow copy of <a href="#Paint">Paint</a>. <a href="undocumented#Typeface">Typeface</a>, <a href="undocumented#Path_Effect">Path Effect</a>, <a href="undocumented#Shader">Shader</a>,
<a href="undocumented#Mask_Filter">Mask Filter</a>, <a href="undocumented#Color_Filter">Color Filter</a>, <a href="undocumented#Rasterizer">Rasterizer</a>, <a href="undocumented#Draw_Looper">Draw Looper</a>, and <a href="undocumented#Image_Filter">Image Filter</a> are shared
between the original <a href="#SkPaint_copy_operator_paint">paint</a> and the copy. Objects containing <a href="undocumented#Reference_Count">Reference Count</a> in the
prior destination are decreased by one, and the referenced objects are deleted if the
resulting count is zero. Objects containing <a href="undocumented#Reference_Count">Reference Count</a> in the parameter <a href="#SkPaint_copy_operator_paint">paint</a>
are increased by one. <a href="#SkPaint_copy_operator_paint">paint</a> is unmodified.

### Parameters

<table>  <tr>    <td><a name="SkPaint_copy_operator_paint"> <code><strong>paint </strong></code> </a></td> <td>
original to copy</td>
  </tr>
</table>

### Return Value

content of <a href="#SkPaint_copy_operator_paint">paint</a>

### Example

<div><fiddle-embed name="b476a9088f80dece176ed577807d3992">

#### Example Output

~~~~
SK_ColorRED == paint1.getColor()
SK_ColorRED == paint2.getColor()
~~~~

</fiddle-embed></div>

---

<a name="SkPaint_move_operator"></a>
## operator=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPaint& operator=(SkPaint&& paint)
</pre>

Moves the <a href="#SkPaint_move_operator_paint">paint</a> to avoid increasing the reference counts
of objects referenced by the <a href="#SkPaint_move_operator_paint">paint</a> parameter. Objects containing <a href="undocumented#Reference_Count">Reference Count</a> in the
prior destination are decreased by one; those objects are deleted if the resulting count
is zero.

After the call, <a href="#SkPaint_move_operator_paint">paint</a> is undefined, and can be safely destructed.

### Parameters

<table>  <tr>    <td><a name="SkPaint_move_operator_paint"> <code><strong>paint </strong></code> </a></td> <td>
original to move</td>
  </tr>
</table>

### Return Value

content of <a href="#SkPaint_move_operator_paint">paint</a>

### Example

<div><fiddle-embed name="9fb7459b097d713f5f1fe5675afe14f5">

#### Example Output

~~~~
SK_ColorRED == paint2.getColor()
~~~~

</fiddle-embed></div>

---

<a name="SkPaint_equal_operator"></a>
## operator==

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool operator==(const SkPaint& a, const SkPaint& b)
</pre>

Compares <a href="#SkPaint_equal_operator_a">a</a> and <a href="#SkPaint_equal_operator_b">b</a>, and returns true if <a href="#SkPaint_equal_operator_a">a</a> and <a href="#SkPaint_equal_operator_b">b</a> are equivalent. May return false
if <a href="undocumented#Typeface">Typeface</a>, <a href="undocumented#Path_Effect">Path Effect</a>, <a href="undocumented#Shader">Shader</a>, <a href="undocumented#Mask_Filter">Mask Filter</a>, <a href="undocumented#Color_Filter">Color Filter</a>, <a href="undocumented#Rasterizer">Rasterizer</a>,
<a href="undocumented#Draw_Looper">Draw Looper</a>, or <a href="undocumented#Image_Filter">Image Filter</a> have identical contents but different pointers.

### Parameters

<table>  <tr>    <td><a name="SkPaint_equal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#Paint">Paint</a> to compare</td>
  </tr>  <tr>    <td><a name="SkPaint_equal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#Paint">Paint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href="#Paint">Paint</a> pair are equivalent

### Example

<div><fiddle-embed name="7481a948e34672720337a631830586dd">

#### Example Output

~~~~
paint1 == paint2
paint1 != paint2
~~~~

</fiddle-embed></div>

---

<a name="SkPaint_notequal_operator"></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool operator!=(const SkPaint& a, const SkPaint& b)
</pre>

Compares <a href="#SkPaint_notequal_operator_a">a</a> and <a href="#SkPaint_notequal_operator_b">b</a>, and returns true if <a href="#SkPaint_notequal_operator_a">a</a> and <a href="#SkPaint_notequal_operator_b">b</a> are not equivalent. May return true
if <a href="undocumented#Typeface">Typeface</a>, <a href="undocumented#Path_Effect">Path Effect</a>, <a href="undocumented#Shader">Shader</a>, <a href="undocumented#Mask_Filter">Mask Filter</a>, <a href="undocumented#Color_Filter">Color Filter</a>, <a href="undocumented#Rasterizer">Rasterizer</a>,
<a href="undocumented#Draw_Looper">Draw Looper</a>, or <a href="undocumented#Image_Filter">Image Filter</a> have identical contents but different pointers.

### Parameters

<table>  <tr>    <td><a name="SkPaint_notequal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#Paint">Paint</a> to compare</td>
  </tr>  <tr>    <td><a name="SkPaint_notequal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#Paint">Paint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href="#Paint">Paint</a> pair are not equivalent

### Example

<div><fiddle-embed name="b6c8484b1187f555b435ad5369833be4">

#### Example Output

~~~~
paint1 == paint2
paint1 == paint2
~~~~

</fiddle-embed></div>

---

<a name="SkPaint_getHash"></a>
## getHash

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
uint32_t getHash() const
</pre>

Returns a hash generated from <a href="#Paint">Paint</a> values and pointers.
Identical hashes guarantee that the paints are
equivalent, but differing hashes do not guarantee that the paints have differing
contents.

If <a href="#SkPaint_equal_operator">operator==(const SkPaint& a, const SkPaint& b)</a> returns true for two paints,
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

---

<a name="SkPaint_flatten"></a>
## flatten

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void flatten(SkWriteBuffer& buffer) const
</pre>

Serializes <a href="#Paint">Paint</a> into a <a href="#SkPaint_flatten_buffer">buffer</a>. A companion <a href="#SkPaint_unflatten">unflatten</a> call
can reconstitute the paint at a later time.

### Parameters

<table>  <tr>    <td><a name="SkPaint_flatten_buffer"> <code><strong>buffer </strong></code> </a></td> <td>
<a href="undocumented#Write_Buffer">Write Buffer</a> receiving the flattened <a href="#Paint">Paint</a> data</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="670672b146b50eced4d3dd10c701e0a7">

#### Example Output

~~~~
color = 0xffff0000
~~~~

</fiddle-embed></div>

---

<a name="SkPaint_unflatten"></a>
## unflatten

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void unflatten(SkReadBuffer& buffer)
</pre>

Populates <a href="#Paint">Paint</a>, typically from a serialized stream, created by calling
<a href="#SkPaint_flatten">flatten</a> at an earlier time.

<a href="undocumented#SkReadBuffer">SkReadBuffer</a> class is not public, so <a href="#SkPaint_unflatten">unflatten</a> cannot be meaningfully called
by the client.

### Parameters

<table>  <tr>    <td><a name="SkPaint_unflatten_buffer"> <code><strong>buffer </strong></code> </a></td> <td>
serialized data describing <a href="#Paint">Paint</a> content</td>
  </tr>
</table>

### See Also

<a href="undocumented#SkReadBuffer">SkReadBuffer</a>

---

# <a name="Hinting"></a> Hinting

## <a name="SkPaint_Hinting"></a> Enum SkPaint::Hinting

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
enum <a href="#Hinting">Hinting</a> {
<a href="#SkPaint_kNo_Hinting">kNo Hinting</a>            = 0,
<a href="#SkPaint_kSlight_Hinting">kSlight Hinting</a>        = 1,
<a href="#SkPaint_kNormal_Hinting">kNormal Hinting</a>        = 2,
<a href="#SkPaint_kFull_Hinting">kFull Hinting</a>          = 3,
};</pre>

<a href="#Hinting">Hinting</a> adjusts the glyph outlines so that the shape provides a uniform
look at a given point size on font engines that support it. <a href="#Hinting">Hinting</a> may have a
muted effect or no effect at all depending on the platform.

The four levels roughly control corresponding features on platforms that use <a href="undocumented#FreeType">FreeType</a>
as the <a href="#Engine">Font Engine</a>.

### Constants

<table>
  <tr>
    <td><a name="SkPaint_kNo_Hinting"> <code><strong>SkPaint::kNo_Hinting </strong></code> </a></td><td>0</td><td>Leaves glyph outlines unchanged from their native representation.
With <a href="undocumented#FreeType">FreeType</a>, this is equivalent to the <a href="undocumented#FT_LOAD_NO_HINTING">FT LOAD NO HINTING</a>
bit-field constant supplied to <a href="undocumented#FT_Load_Glyph">FT Load Glyph</a>, which indicates that the vector
outline being loaded should not be fitted to the pixel grid but simply scaled
to 26.6 fractional pixels.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kSlight_Hinting"> <code><strong>SkPaint::kSlight_Hinting </strong></code> </a></td><td>1</td><td>Modifies glyph outlines minimally to improve constrast.
With <a href="undocumented#FreeType">FreeType</a>, this is equivalent in spirit to the
<a href="undocumented#FT_LOAD_TARGET_LIGHT">FT LOAD TARGET LIGHT</a> value supplied to <a href="undocumented#FT_Load_Glyph">FT Load Glyph</a>. It chooses a 
lighter hinting algorithm for non-monochrome modes.
Generated <a href="#Glyph">Glyphs</a> may be fuzzy but better resemble their original shape.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kNormal_Hinting"> <code><strong>SkPaint::kNormal_Hinting </strong></code> </a></td><td>2</td><td>Modifies glyph outlines to improve constrast. This is the default.
With <a href="undocumented#FreeType">FreeType</a>, this supplies <a href="undocumented#FT_LOAD_TARGET_NORMAL">FT LOAD TARGET NORMAL</a> to <a href="undocumented#FT_Load_Glyph">FT Load Glyph</a>,
choosing the default hinting algorithm, which is optimized for standard 
gray-level rendering.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kFull_Hinting"> <code><strong>SkPaint::kFull_Hinting </strong></code> </a></td><td>3</td><td>Modifies glyph outlines for maxiumum constrast. With <a href="undocumented#FreeType">FreeType</a>, this selects
<a href="undocumented#FT_LOAD_TARGET_LCD">FT LOAD TARGET LCD</a> or <a href="undocumented#FT_LOAD_TARGET_LCD_V">FT LOAD TARGET LCD V</a> if <a href="#SkPaint_kLCDRenderText_Flag">kLCDRenderText Flag</a> is set. 
<a href="undocumented#FT_LOAD_TARGET_LCD">FT LOAD TARGET LCD</a> is a variant of <a href="undocumented#FT_LOAD_TARGET_NORMAL">FT LOAD TARGET NORMAL</a> optimized for 
horizontally decimated <a href="undocumented#LCD">LCD</a> displays; <a href="undocumented#FT_LOAD_TARGET_LCD_V">FT LOAD TARGET LCD V</a> is a 
variant of <a href="undocumented#FT_LOAD_TARGET_NORMAL">FT LOAD TARGET NORMAL</a> optimized for vertically decimated <a href="undocumented#LCD">LCD</a> displays.</td>
  </tr>
</table>

On <a href="undocumented#Windows">Windows</a> with <a href="undocumented#DirectWrite">DirectWrite</a>, <a href="#Hinting">Hinting</a> has no effect.

<a href="#Hinting">Hinting</a> defaults to <a href="#SkPaint_kNormal_Hinting">kNormal Hinting</a>.
Set <a href="undocumented#SkPaintDefaults_Hinting">SkPaintDefaults Hinting</a> at compile time to change the default setting.



<a name="SkPaint_getHinting"></a>
## getHinting

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
Hinting getHinting() const
</pre>

Returns level of glyph outline adjustment.

### Return Value

one of: <a href="#SkPaint_kNo_Hinting">kNo Hinting</a>, <a href="#SkPaint_kSlight_Hinting">kSlight Hinting</a>, <a href="#SkPaint_kNormal_Hinting">kNormal Hinting</a>, <a href="#SkPaint_kFull_Hinting">kFull Hinting</a>

### Example

<div><fiddle-embed name="329e2e5a5919ac431e1c58878a5b99e0">

#### Example Output

~~~~
SkPaint::kNormal_Hinting == paint.getHinting()
~~~~

</fiddle-embed></div>

---

<a name="SkPaint_setHinting"></a>
## setHinting

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setHinting(Hinting hintingLevel)
</pre>

Sets level of glyph outline adjustment.
Does not check for valid values of <a href="#SkPaint_setHinting_hintingLevel">hintingLevel</a>.

| <a href="#Hinting">Hinting</a> | value | effect on generated glyph outlines |
| --- | --- | ---  |
| <a href="#SkPaint_kNo_Hinting">kNo Hinting</a> | 0 | leaves glyph outlines unchanged from their native representation |
| <a href="#SkPaint_kSlight_Hinting">kSlight Hinting</a> | 1 | modifies glyph outlines minimally to improve contrast |
| <a href="#SkPaint_kNormal_Hinting">kNormal Hinting</a> | 2 | modifies glyph outlines to improve contrast |
| <a href="#SkPaint_kFull_Hinting">kFull Hinting</a> | 3 | modifies glyph outlines for maximum contrast |

### Parameters

<table>  <tr>    <td><a name="SkPaint_setHinting_hintingLevel"> <code><strong>hintingLevel </strong></code> </a></td> <td>
one of: <a href="#SkPaint_kNo_Hinting">kNo Hinting</a>, <a href="#SkPaint_kSlight_Hinting">kSlight Hinting</a>, <a href="#SkPaint_kNormal_Hinting">kNormal Hinting</a>, <a href="#SkPaint_kFull_Hinting">kFull Hinting</a></td>
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

# <a name="Flags"></a> Flags

## <a name="SkPaint_Flags"></a> Enum SkPaint::Flags

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
enum <a href="#Flags">Flags</a> {
<a href="#SkPaint_kAntiAlias_Flag">kAntiAlias Flag</a>       = 0x01,
<a href="#SkPaint_kDither_Flag">kDither Flag</a>          = 0x04,
<a href="#SkPaint_kFakeBoldText_Flag">kFakeBoldText Flag</a>    = 0x20,
<a href="#SkPaint_kLinearText_Flag">kLinearText Flag</a>      = 0x40,
<a href="#SkPaint_kSubpixelText_Flag">kSubpixelText Flag</a>    = 0x80,
<a href="#SkPaint_kDevKernText_Flag">kDevKernText Flag</a>     = 0x100,
<a href="#SkPaint_kLCDRenderText_Flag">kLCDRenderText Flag</a>   = 0x200,
<a href="#SkPaint_kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a> = 0x400,
<a href="#SkPaint_kAutoHinting_Flag">kAutoHinting Flag</a>     = 0x800,
<a href="#SkPaint_kVerticalText_Flag">kVerticalText Flag</a>    = 0x1000,
<a href="#SkPaint_kGenA8FromLCD_Flag">kGenA8FromLCD Flag</a>    = 0x2000,

<a href="#SkPaint_kAllFlags">kAllFlags</a> = 0xFFFF,
};
</pre>

The bit values stored in <a href="#Flags">Flags</a>.
The default value for <a href="#Flags">Flags</a>, normally zero, can be changed at compile time
with a custom definition of <a href="undocumented#SkPaintDefaults_Flags">SkPaintDefaults Flags</a>.
All flags can be read and written explicitly; <a href="#Flags">Flags</a> allows manipulating
multiple settings at once.

### Constants

<table>
  <tr>
    <td><a name="SkPaint_kAntiAlias_Flag"> <code><strong>SkPaint::kAntiAlias_Flag </strong></code> </a></td><td>0x0001 </td><td>mask for setting Anti-alias</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kDither_Flag"> <code><strong>SkPaint::kDither_Flag </strong></code> </a></td><td>0x0004</td><td>mask for setting <a href="#Dither">Dither</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint_kFakeBoldText_Flag"> <code><strong>SkPaint::kFakeBoldText_Flag </strong></code> </a></td><td>0x0020</td><td>mask for setting <a href="#Fake_Bold">Fake Bold</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint_kLinearText_Flag"> <code><strong>SkPaint::kLinearText_Flag </strong></code> </a></td><td>0x0040</td><td>mask for setting <a href="SkPaint_Reference#Linear_Text">Linear Text</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint_kSubpixelText_Flag"> <code><strong>SkPaint::kSubpixelText_Flag </strong></code> </a></td><td>0x0080</td><td>mask for setting <a href="SkPaint_Reference#Subpixel_Text">Subpixel Text</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint_kDevKernText_Flag"> <code><strong>SkPaint::kDevKernText_Flag </strong></code> </a></td><td>0x0100</td><td>mask for setting <a href="SkPaint_Reference#Full_Hinting_Spacing">Full Hinting Spacing</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint_kLCDRenderText_Flag"> <code><strong>SkPaint::kLCDRenderText_Flag </strong></code> </a></td><td>0x0200</td><td>mask for setting <a href="SkPaint_Reference#LCD_Text">LCD Text</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint_kEmbeddedBitmapText_Flag"> <code><strong>SkPaint::kEmbeddedBitmapText_Flag </strong></code> </a></td><td>0x0400</td><td>mask for setting <a href="SkPaint_Reference#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint_kAutoHinting_Flag"> <code><strong>SkPaint::kAutoHinting_Flag </strong></code> </a></td><td>0x0800</td><td>mask for setting <a href="#Automatic_Hinting">Automatic Hinting</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint_kVerticalText_Flag"> <code><strong>SkPaint::kVerticalText_Flag </strong></code> </a></td><td>0x1000</td><td>mask for setting <a href="#Vertical_Text">Vertical Text</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint_kGenA8FromLCD_Flag"> <code><strong>SkPaint::kGenA8FromLCD_Flag </strong></code> </a></td><td>0x2000</td><td>not intended for public use</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kAllFlags"> <code><strong>SkPaint::kAllFlags </strong></code> </a></td><td>0xFFFF</td><td>mask of all <a href="#Flags">Flags</a>, including private flags and flags reserved for future use</td>
  </tr>
<a href="#Flags">Flags</a> default to all flags clear, disabling the associated feature.

</table>

## <a name="SkPaint_ReserveFlags"></a> Enum SkPaint::ReserveFlags

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
enum <a href="#SkPaint_ReserveFlags">ReserveFlags</a> {
<a href="#SkPaint_kUnderlineText_ReserveFlag">kUnderlineText ReserveFlag</a>   = 0x08,
<a href="#SkPaint_kStrikeThruText_ReserveFlag">kStrikeThruText ReserveFlag</a>  = 0x10,
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPaint_kUnderlineText_ReserveFlag"> <code><strong>SkPaint::kUnderlineText_ReserveFlag </strong></code> </a></td><td>0x0008</td><td>mask for underline text</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kStrikeThruText_ReserveFlag"> <code><strong>SkPaint::kStrikeThruText_ReserveFlag </strong></code> </a></td><td>0x0010</td><td>mask for strike-thru text</td>
  </tr>
</table>

### See Also

<a href="#Flags">Flags</a> <a href="#SkPaint_getFlags">getFlags</a>



<a name="SkPaint_getFlags"></a>
## getFlags

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
uint32_t getFlags() const
</pre>

Returns paint settings described by <a href="#Flags">Flags</a>. Each setting uses one
bit, and can be tested with <a href="#Flags">Flags</a> members.

### Return Value

zero, one, or more bits described by <a href="#Flags">Flags</a>

### Example

<div><fiddle-embed name="8a3f8c309533388b01aa66e1267f322d">

#### Example Output

~~~~
(SkPaint::kAntiAlias_Flag & paint.getFlags()) != 0
~~~~

</fiddle-embed></div>

---

<a name="SkPaint_setFlags"></a>
## setFlags

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setFlags(uint32_t flags)
</pre>

Replaces <a href="#Flags">Flags</a> with <a href="#SkPaint_setFlags_flags">flags</a>, the union of the <a href="#Flags">Flags</a> members.
All <a href="#Flags">Flags</a> members may be cleared, or one or more may be set.

### Parameters

<table>  <tr>    <td><a name="SkPaint_setFlags_flags"> <code><strong>flags </strong></code> </a></td> <td>
union of <a href="#Flags">Flags</a> for <a href="#Paint">Paint</a></td>
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

# <a name="Anti-alias"></a> Anti-alias
Anti-alias drawing approximates partial pixel coverage with transparency.
If <a href="#SkPaint_kAntiAlias_Flag">kAntiAlias Flag</a> is clear, pixel centers contained by the shape edge are drawn opaque.
If <a href="#SkPaint_kAntiAlias_Flag">kAntiAlias Flag</a> is set, pixels are drawn with <a href="#Alpha">Color Alpha</a> equal to their coverage.

The rule for <a href="#Alias">Aliased</a> pixels is inconsistent across platforms. A shape edge 
passing through the pixel center may, but is not required to, draw the pixel.

<a href="undocumented#Raster_Engine">Raster Engine</a> draws <a href="#Alias">Aliased</a> pixels whose centers are on or to the right of the start of an
active <a href="SkPath_Reference#Path">Path</a> edge, and whose center is to the left of the end of the active <a href="SkPath_Reference#Path">Path</a> edge.

A platform may only support Anti-aliased drawing. Some <a href="undocumented#GPU">GPU</a>-backed platforms use
<a href="undocumented#Supersampling">Supersampling</a> to Anti-alias all drawing, and have no mechanism to selectively
<a href="undocumented#Alias">Alias</a>.

The amount of coverage computed for Anti-aliased pixels also varies across platforms.

Anti-alias is disabled by default.
Anti-alias can be enabled by default by setting <a href="undocumented#SkPaintDefaults_Flags">SkPaintDefaults Flags</a> to <a href="#SkPaint_kAntiAlias_Flag">kAntiAlias Flag</a>
at compile time.

### Example

<div><fiddle-embed name="a6575a49467ce8d28bb01cc7638fa04d"><div>A red line is drawn with transparency on the edges to make it look smoother.
A blue line draws only where the pixel centers are contained.
The lines are drawn into <a href="SkBitmap_Reference#Bitmap">Bitmap</a>, then drawn magnified to make the
<a href="#Alias">Aliasing</a> easier to see.</div></fiddle-embed></div>

<a name="SkPaint_isAntiAlias"></a>
## isAntiAlias

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isAntiAlias() const
</pre>

If true, pixels on the active edges of <a href="SkPath_Reference#Path">Path</a> may be drawn with partial transparency.

Equivalent to <a href="#SkPaint_getFlags">getFlags</a> masked with <a href="#SkPaint_kAntiAlias_Flag">kAntiAlias Flag</a>.

### Return Value

<a href="#SkPaint_kAntiAlias_Flag">kAntiAlias Flag</a> state

### Example

<div><fiddle-embed name="d7d5f4f7da7acd5104a652f490c6f7b8">

#### Example Output

~~~~
paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)
paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)
~~~~

</fiddle-embed></div>

---

<a name="SkPaint_setAntiAlias"></a>
## setAntiAlias

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setAntiAlias(bool aa)
</pre>

Requests, but does not require, that <a href="SkPath_Reference#Path">Path</a> edge pixels draw opaque or with
partial transparency.

Sets <a href="#SkPaint_kAntiAlias_Flag">kAntiAlias Flag</a> if <a href="#SkPaint_setAntiAlias_aa">aa</a> is true.
Clears <a href="#SkPaint_kAntiAlias_Flag">kAntiAlias Flag</a> if <a href="#SkPaint_setAntiAlias_aa">aa</a> is false.

### Parameters

<table>  <tr>    <td><a name="SkPaint_setAntiAlias_aa"> <code><strong>aa </strong></code> </a></td> <td>
setting for <a href="#SkPaint_kAntiAlias_Flag">kAntiAlias Flag</a></td>
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

# <a name="Dither"></a> Dither
<a href="#Dither">Dither</a> increases fidelity by adjusting the color of adjacent pixels. 
This can help to smooth color transitions and reducing banding in gradients.
Dithering lessens visible banding from <a href="undocumented#SkColorType">kRGB 565 SkColorType</a>
and <a href="undocumented#SkColorType">kRGBA 8888 SkColorType</a> gradients, 
and improves rendering into a <a href="undocumented#SkColorType">kRGB 565 SkColorType</a> <a href="SkSurface_Reference#Surface">Surface</a>.

Dithering is always enabled for linear gradients drawing into
<a href="undocumented#SkColorType">kRGB 565 SkColorType</a> <a href="SkSurface_Reference#Surface">Surface</a> and <a href="undocumented#SkColorType">kRGBA 8888 SkColorType</a> <a href="SkSurface_Reference#Surface">Surface</a>.
<a href="#Dither">Dither</a> cannot be enabled for <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a> <a href="SkSurface_Reference#Surface">Surface</a> and
<a href="undocumented#SkColorType">kRGBA F16 SkColorType</a> <a href="SkSurface_Reference#Surface">Surface</a>.

<a href="#Dither">Dither</a> is disabled by default.
<a href="#Dither">Dither</a> can be enabled by default by setting <a href="undocumented#SkPaintDefaults_Flags">SkPaintDefaults Flags</a> to <a href="#SkPaint_kDither_Flag">kDither Flag</a>
at compile time.

Some platform implementations may ignore dithering. Setto ignore <a href="#Dither">Dither</a> on <a href="undocumented#GPU_Surface">GPU Surface</a>.

### Example

<div><fiddle-embed name="8b26507690b71462f44642b911890bbf"><div>Dithering in the bottom half more closely approximates the requested color by
alternating nearby colors from pixel to pixel.</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="76d4d4a7931a48495e4d5f54e073be53"><div>Dithering introduces subtle adjustments to color to smooth gradients.
Drawing the gradient repeatedly with <a href="#SkBlendMode_kPlus">SkBlendMode::kPlus</a> exaggerates the
dither, making it easier to see.</div></fiddle-embed></div>

<a name="SkPaint_isDither"></a>
## isDither

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isDither() const
</pre>

If true, color error may be distributed to smooth color transition.
Equivalent to <a href="#SkPaint_getFlags">getFlags</a> masked with <a href="#SkPaint_kDither_Flag">kDither Flag</a>.

### Return Value

<a href="#SkPaint_kDither_Flag">kDither Flag</a> state

### Example

<div><fiddle-embed name="f4ce93f6c5e7335436a985377fd980c0">

#### Example Output

~~~~
paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag)
paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag)
~~~~

</fiddle-embed></div>

---

<a name="SkPaint_setDither"></a>
## setDither

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setDither(bool dither)
</pre>

Requests, but does not require, to distribute color error.

Sets <a href="#SkPaint_kDither_Flag">kDither Flag</a> if <a href="#SkPaint_setDither_dither">dither</a> is true.
Clears <a href="#SkPaint_kDither_Flag">kDither Flag</a> if <a href="#SkPaint_setDither_dither">dither</a> is false.

### Parameters

<table>  <tr>    <td><a name="SkPaint_setDither_dither"> <code><strong>dither </strong></code> </a></td> <td>
setting for <a href="#SkPaint_kDither_Flag">kDither Flag</a></td>
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

<a href="undocumented#SkColorType">kRGB 565 SkColorType</a>

---

### See Also

Gradient <a href="#RGB">Color RGB</a>-565

# <a name="Device_Text"></a> Device Text
<a href="SkPaint_Reference#LCD_Text">LCD Text</a> and <a href="SkPaint_Reference#Subpixel_Text">Subpixel Text</a> increase the precision of glyph position.

When set, <a href="#SkPaint_Flags">Flags</a> <a href="#SkPaint_kLCDRenderText_Flag">kLCDRenderText Flag</a> takes advantage of the organization of <a href="#RGB">Color RGB</a> stripes that 
create a color, and relies
on the small size of the stripe and visual perception to make the color fringing imperceptible.
<a href="SkPaint_Reference#LCD_Text">LCD Text</a> can be enabled on devices that orient stripes horizontally or vertically, and that order
the color components as <a href="#RGB">Color RGB</a> or <a href="#RBG">Color RBG</a>.

<a href="#SkPaint_Flags">Flags</a> <a href="#SkPaint_kSubpixelText_Flag">kSubpixelText Flag</a> uses the pixel transparency to represent a fractional offset. 
As the opaqueness
of the color increases, the edge of the glyph appears to move towards the outside of the pixel.

Either or both techniques can be enabled.
<a href="#SkPaint_kLCDRenderText_Flag">kLCDRenderText Flag</a> and <a href="#SkPaint_kSubpixelText_Flag">kSubpixelText Flag</a> are clear by default.
<a href="SkPaint_Reference#LCD_Text">LCD Text</a> or <a href="SkPaint_Reference#Subpixel_Text">Subpixel Text</a> can be enabled by default by setting <a href="undocumented#SkPaintDefaults_Flags">SkPaintDefaults Flags</a> to 
<a href="#SkPaint_kLCDRenderText_Flag">kLCDRenderText Flag</a> or <a href="#SkPaint_kSubpixelText_Flag">kSubpixelText Flag</a> (or both) at compile time.

### Example

<div><fiddle-embed name="4606ae1be792d6bc46d496432f050ee9"><div>Four commas are drawn normally and with combinations of <a href="SkPaint_Reference#LCD_Text">LCD Text</a> and <a href="SkPaint_Reference#Subpixel_Text">Subpixel Text</a>.
When <a href="SkPaint_Reference#Subpixel_Text">Subpixel Text</a> is disabled, the comma <a href="#Glyph">Glyphs</a> are identical, but not evenly spaced.
When <a href="SkPaint_Reference#Subpixel_Text">Subpixel Text</a> is enabled, the comma <a href="#Glyph">Glyphs</a> are unique, but appear evenly spaced.</div></fiddle-embed></div>

## <a name="Linear_Text"></a> Linear Text

<a href="SkPaint_Reference#Linear_Text">Linear Text</a> selects whether text is rendered as a <a href="undocumented#Glyph">Glyph</a> or as a <a href="SkPath_Reference#Path">Path</a>.
If <a href="#SkPaint_kLinearText_Flag">kLinearText Flag</a> is set, it has the same effect as setting <a href="#SkPaint_Hinting">Hinting</a> to <a href="#SkPaint_kNormal_Hinting">kNormal Hinting</a>.
If <a href="#SkPaint_kLinearText_Flag">kLinearText Flag</a> is clear, it is the same as setting <a href="#SkPaint_Hinting">Hinting</a> to <a href="#SkPaint_kNo_Hinting">kNo Hinting</a>.

<a name="SkPaint_isLinearText"></a>
## isLinearText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isLinearText() const
</pre>

If true, text is converted to <a href="SkPath_Reference#Path">Path</a> before drawing and measuring.

Equivalent to <a href="#SkPaint_getFlags">getFlags</a> masked with <a href="#SkPaint_kLinearText_Flag">kLinearText Flag</a>.

### Return Value

<a href="#SkPaint_kLinearText_Flag">kLinearText Flag</a> state

### Example

<div><fiddle-embed name="2890ad644f980637837e6fcb386fb462"></fiddle-embed></div>

### See Also

<a href="#SkPaint_setLinearText">setLinearText</a> <a href="#SkPaint_Hinting">Hinting</a>

---

<a name="SkPaint_setLinearText"></a>
## setLinearText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setLinearText(bool linearText)
</pre>

If true, text is converted to <a href="SkPath_Reference#Path">Path</a> before drawing and measuring.
By default, <a href="#SkPaint_kLinearText_Flag">kLinearText Flag</a> is clear.

Sets <a href="#SkPaint_kLinearText_Flag">kLinearText Flag</a> if <a href="#SkPaint_setLinearText_linearText">linearText</a> is true.
Clears <a href="#SkPaint_kLinearText_Flag">kLinearText Flag</a> if <a href="#SkPaint_setLinearText_linearText">linearText</a> is false.

### Parameters

<table>  <tr>    <td><a name="SkPaint_setLinearText_linearText"> <code><strong>linearText </strong></code> </a></td> <td>
setting for <a href="#SkPaint_kLinearText_Flag">kLinearText Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c93bb912f3bddfb4d96d3ad70ada552b"></fiddle-embed></div>

### See Also

<a href="#SkPaint_isLinearText">isLinearText</a> <a href="#SkPaint_Hinting">Hinting</a>

---

## <a name="Subpixel_Text"></a> Subpixel Text

<a href="#SkPaint_Flags">Flags</a> <a href="#SkPaint_kSubpixelText_Flag">kSubpixelText Flag</a> uses the pixel transparency to represent a fractional offset. 
As the opaqueness
of the color increases, the edge of the glyph appears to move towards the outside of the pixel.

<a name="SkPaint_isSubpixelText"></a>
## isSubpixelText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isSubpixelText() const
</pre>

If true, <a href="#Glyph">Glyphs</a> at different sub-pixel positions may differ on pixel edge coverage.

Equivalent to <a href="#SkPaint_getFlags">getFlags</a> masked with <a href="#SkPaint_kSubpixelText_Flag">kSubpixelText Flag</a>.

### Return Value

<a href="#SkPaint_kSubpixelText_Flag">kSubpixelText Flag</a> state

### Example

<div><fiddle-embed name="abe9afc0932e2199324ae6cbb396e67c">

#### Example Output

~~~~
paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)
paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)
~~~~

</fiddle-embed></div>

---

<a name="SkPaint_setSubpixelText"></a>
## setSubpixelText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setSubpixelText(bool subpixelText)
</pre>

Requests, but does not require, that <a href="#Glyph">Glyphs</a> respect sub-pixel positioning.

Sets <a href="#SkPaint_kSubpixelText_Flag">kSubpixelText Flag</a> if <a href="#SkPaint_setSubpixelText_subpixelText">subpixelText</a> is true.
Clears <a href="#SkPaint_kSubpixelText_Flag">kSubpixelText Flag</a> if <a href="#SkPaint_setSubpixelText_subpixelText">subpixelText</a> is false.

### Parameters

<table>  <tr>    <td><a name="SkPaint_setSubpixelText_subpixelText"> <code><strong>subpixelText </strong></code> </a></td> <td>
setting for <a href="#SkPaint_kSubpixelText_Flag">kSubpixelText Flag</a></td>
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

## <a name="LCD_Text"></a> LCD Text

When set, <a href="#SkPaint_Flags">Flags</a> <a href="#SkPaint_kLCDRenderText_Flag">kLCDRenderText Flag</a> takes advantage of the organization of <a href="#RGB">Color RGB</a> stripes that 
create a color, and relies
on the small size of the stripe and visual perception to make the color fringing imperceptible.
<a href="SkPaint_Reference#LCD_Text">LCD Text</a> can be enabled on devices that orient stripes horizontally or vertically, and that order
the color components as <a href="#RGB">Color RGB</a> or <a href="#RBG">Color RBG</a>.

<a name="SkPaint_isLCDRenderText"></a>
## isLCDRenderText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isLCDRenderText() const
</pre>

If true, <a href="#Glyph">Glyphs</a> may use <a href="undocumented#LCD">LCD</a> striping to improve glyph edges.

Returns true if <a href="#SkPaint_Flags">Flags</a> <a href="#SkPaint_kLCDRenderText_Flag">kLCDRenderText Flag</a> is set.

### Return Value

<a href="#SkPaint_kLCDRenderText_Flag">kLCDRenderText Flag</a> state

### Example

<div><fiddle-embed name="68e1fd95dd2fd06a333899d2bd2396b9">

#### Example Output

~~~~
paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)
paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)
~~~~

</fiddle-embed></div>

---

<a name="SkPaint_setLCDRenderText"></a>
## setLCDRenderText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setLCDRenderText(bool lcdText)
</pre>

Requests, but does not require, that <a href="#Glyph">Glyphs</a> use <a href="undocumented#LCD">LCD</a> striping for glyph edges.

Sets <a href="#SkPaint_kLCDRenderText_Flag">kLCDRenderText Flag</a> if <a href="#SkPaint_setLCDRenderText_lcdText">lcdText</a> is true.
Clears <a href="#SkPaint_kLCDRenderText_Flag">kLCDRenderText Flag</a> if <a href="#SkPaint_setLCDRenderText_lcdText">lcdText</a> is false.

### Parameters

<table>  <tr>    <td><a name="SkPaint_setLCDRenderText_lcdText"> <code><strong>lcdText </strong></code> </a></td> <td>
setting for <a href="#SkPaint_kLCDRenderText_Flag">kLCDRenderText Flag</a></td>
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

# <a name="Font_Embedded_Bitmaps"></a> Font Embedded Bitmaps
<a href="SkPaint_Reference#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a> allows selecting custom sized bitmap <a href="#Glyph">Glyphs</a>.
<a href="#SkPaint_Flags">Flags</a> <a href="#SkPaint_kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a> when set chooses an embedded bitmap glyph over an outline contained
in a font if the platform supports this option. 

<a href="undocumented#FreeType">FreeType</a> selects the bitmap glyph if available when <a href="#SkPaint_kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a> is set, and selects
the outline glyph if <a href="#SkPaint_kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a> is clear.
<a href="undocumented#Windows">Windows</a> may select the bitmap glyph but is not required to do so.
<a href="undocumented#OS_X">OS X</a> and <a href="undocumented#iOS">iOS</a> do not support this option.

<a href="SkPaint_Reference#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a> is disabled by default.
<a href="SkPaint_Reference#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a> can be enabled by def