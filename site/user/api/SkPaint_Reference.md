SkPaint Reference
===

# <a name="Paint"></a> Paint
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
<a href="SkPath_Reference#Path">Path</a> geometries with <a href="undocumented#Anti_alias">Anti-aliasing</a>, regardless of how <a href="#SkPaint_kAntiAlias_Flag">SkPaint::kAntiAlias Flag</a> 
is set in <a href="#Paint">Paint</a>.

<a href="#Paint">Paint</a> describes a single color, a single font, a single image quality, and so on.
Multiple colors are drawn either by using multiple paints or with objects like
<a href="undocumented#Shader">Shader</a> attached to <a href="#Paint">Paint</a>.

# <a name="SkPaint"></a> Class SkPaint

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |
| <a href="#Initializers">Initializers</a> | Constructors and initialization. |
| <a href="undocumented#Destructor">Destructor</a> | <a href="#Paint">Paint</a> termination. |
| <a href="#Management">Management</a> | <a href="#Paint">Paint</a> copying, moving, comparing. |
| <a href="#SkPaint_Hinting">Hinting</a> | <a href="undocumented#Glyph">Glyph</a> outline adjustment. |
| <a href="#SkPaint_Flags">Flags</a> | Attributes represented by single bits. |
| <a href="SkPaint_Reference#Anti_alias">Anti-alias</a> | Approximating coverage with transparency. |
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
| <a href="#SkPaint_TextEncoding">TextEncoding</a> | Character or glyph encoding size. |

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
| <a href="#SkPaint_copy_assignment_operator">operator=(const SkPaint& paint)</a> | Makes a shallow copy. |
| <a href="#SkPaint_move_assignment_operator">operator=(SkPaint&& paint)</a> | Moves paint without copying it. |
| <a href="#SkPaint_equal_operator">operator==(const SkPaint& a, const SkPaint& b)</a> | Compares paints for equality. |
| <a href="#SkPaint_not_equal_operator">operator!=(const SkPaint& a, const SkPaint& b)</a> | Compares paints for inequality. |

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
| <a href="#SkPaint_getTextEncoding">getTextEncoding</a> | Returns character or glyph encoding size. |
| <a href="#SkPaint_getTextIntercepts">getTextIntercepts</a> | Returns where lines intersect text; underlines. |
| <a href="#SkPaint_getTextPath">getTextPath</a> | Returns <a href="SkPath_Reference#Path">Path</a> equivalent to text. |
| <a href="#SkPaint_getTextScaleX">getTextScaleX</a> | Returns the text horizontal scale; condensed text. |
| <a href="#SkPaint_getTextSkewX">getTextSkewX</a> | Returns the text horizontal skew; oblique text. |
| <a href="#SkPaint_getTextSize">getTextSize</a> | Returns text size in points. |
| <a href="#SkPaint_getTextWidths">getTextWidths</a> | Returns advance and bounds for each glyph in text. |
| <a href="#SkPaint_getTypeface">getTypeface</a> | Returns <a href="undocumented#Typeface">Typeface</a>, font description. |
| <a href="#SkPaint_glyphsToUnichars">glyphsToUnichars</a> | Converts <a href="#Glyph">Glyphs</a> into text. |
| <a href="#SkPaint_isAntiAlias">isAntiAlias</a> | Returns true if <a href="SkPaint_Reference#Anti_alias">Anti-alias</a> is set. |
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
| <a href="#SkPaint_setAntiAlias">setAntiAlias</a> | Sets or clears <a href="SkPaint_Reference#Anti_alias">Anti-alias</a>. |
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
| <a href="#SkPaint_setTextEncoding">setTextEncoding</a> | Sets character or glyph encoding size. |
| <a href="#SkPaint_setTextScaleX">setTextScaleX</a> | Sets the text horizontal scale; condensed text. |
| <a href=