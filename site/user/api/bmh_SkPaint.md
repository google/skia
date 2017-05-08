# <a name="Paint"></a> Paint
<a href="bmh_SkPaint?cl=9919#Paint">Paint</a> controls options applied when drawing and measuring. <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> collects all
options outside of the <a href="bmh_SkCanvas?cl=9919#Clip">Canvas_Clip</a> and <a href="bmh_SkCanvas?cl=9919#Matrix">Canvas_Matrix</a>.
Various options apply to text, strokes and fills, and images. 
Some options may not be implemented on all platforms; in these cases, setting
the option has no effect. Some options are conveniences that duplicate <a href="bmh_SkCanvas?cl=9919#Canvas">Canvas</a>
functionality; for instance, text size is identical to matrix scale.
<a href="bmh_SkPaint?cl=9919#Paint">Paint</a> options are rarely exclusive; each option modifies a stage of the drawing
pipeline and multiple pipeline stages may be effected by a single <a href="bmh_SkPaint?cl=9919#Paint">Paint</a>.
<a href="bmh_SkPaint?cl=9919#Paint">Paint</a> collects effect and filters that describe single-pass and multiple-pass 
algorithms that alter the drawing geometry, color, and transparency. For instance,
<a href="bmh_SkPaint?cl=9919#Paint">Paint</a> does not directly implement dashing or blur, but contains the objects that do so. 
The objects contained by <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> are opaque, and cannot be edited outside of the <a href="bmh_SkPaint?cl=9919#Paint">Paint</a>
and affect it. The implementation is free to defer computations associated with the
<a href="bmh_SkPaint?cl=9919#Paint">Paint</a>, or ignore them altogether. For instance, some <a href="bmh_undocumented?cl=9919#GPU">GPU</a> implementations draw all
<a href="bmh_undocumented?cl=9919#Path">Path</a> geometries with anti-aliasing, regardless of <a href="bmh_SkPaint?cl=9919#kAntiAlias_Flag">SkPaint::kAntiAlias_Flag</a> setting.
<a href="bmh_SkPaint?cl=9919#Paint">Paint</a> describes a single color, a single font, a single image quality, and so on.
Multiple colors are drawn either by using multiple paints or with objects like
<a href="bmh_undocumented?cl=9919#Shader">Shader</a> attached to <a href="bmh_SkPaint?cl=9919#Paint">Paint</a>.

# <a name="SkPaint"></a> Class SkPaint

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |
| <a href="bmh_SkPaint?cl=9919#Initializers">Initializers</a> | Constructors and initialization. |
| <a href="bmh_SkPaint?cl=9919#Destructor">Destructor</a> | <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> termination. |
| <a href="bmh_SkPaint?cl=9919#Management">Management</a> | <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> copying, moving, comparing. |
| <a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a> | Glyph outline adjustment. |
| <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> | Attributes represented by single bits. |
| <a href="bmh_SkPaint?cl=9919#Anti_alias">Anti-alias</a> | Approximating coverage with transparency. |
| <a href="bmh_SkPaint?cl=9919#Dither">Dither</a> | Distributing color error. |
| <a href="bmh_SkPaint?cl=9919#Device_Text">Device_Text</a> | Increase precision of glyph position. |
| <a href="bmh_SkPaint?cl=9919#Font_Embedded_Bitmaps">Font_Embedded_Bitmaps</a> | Custom-sized bitmap glyphs. |
| <a href="bmh_SkPaint?cl=9919#Forced_Auto_hinting">Forced_Auto-hinting</a> | Deprecated. |
| <a href="bmh_SkPaint?cl=9919#Vertical_Text">Vertical_Text</a> | Orient text from top to bottom. |
| <a href="bmh_SkPaint?cl=9919#Text_Decorations">Text_Decorations</a> | Approximate font styles like <a href="bmh_SkPaint?cl=9919#Fake_Bold">Fake_Bold</a>. |
| <a href="bmh_SkPaint?cl=9919#Full_Hinting_Spacing">Full_Hinting_Spacing</a> | Glyph spacing affected by hinting. |
| <a href="bmh_SkPaint?cl=9919#Filter_Quality_Methods">Filter_Quality_Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter_Quality</a>. |
| <a href="bmh_SkPaint?cl=9919#Color_Methods">Color_Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Color">Color</a>. |
| <a href="bmh_SkPaint?cl=9919#Style">Style</a> | Geometry filling, stroking. |
| <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> | Thickness perpendicular to geometry. |
| <a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a> | Maximum length of stroked corners. |
| <a href="bmh_SkPaint?cl=9919#Stroke_Cap">Stroke_Cap</a> | Decorations at ends of open strokes. |
| <a href="bmh_SkPaint?cl=9919#Stroke_Join">Stroke_Join</a> | Decoration at corners of strokes. |
| <a href="bmh_SkPaint?cl=9919#Fill_Path">Fill_Path</a> | Make <a href="bmh_undocumented?cl=9919#Path">Path</a> from <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, stroking. |
| <a href="bmh_SkPaint?cl=9919#Shader_Methods">Shader_Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Shader">Shader</a>. |
| <a href="bmh_SkPaint?cl=9919#Color_Filter_Methods">Color_Filter_Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>. |
| <a href="bmh_SkPaint?cl=9919#Blend_Mode_Methods">Blend_Mode_Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a>. |
| <a href="bmh_SkPaint?cl=9919#Path_Effect_Methods">Path_Effect_Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>. |
| <a href="bmh_SkPaint?cl=9919#Mask_Filter_Methods">Mask_Filter_Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>. |
| <a href="bmh_SkPaint?cl=9919#Typeface_Methods">Typeface_Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>. |
| <a href="bmh_SkPaint?cl=9919#Rasterizer_Methods">Rasterizer_Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>. |
| <a href="bmh_SkPaint?cl=9919#Image_Filter_Methods">Image_Filter_Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a>. |
| <a href="bmh_SkPaint?cl=9919#Draw_Looper_Methods">Draw_Looper_Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a>. |
| <a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> | <a href="bmh_undocumented?cl=9919#Text">Text</a> placement relative to position. |
| <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> | Overall height in points. |
| <a href="bmh_SkPaint?cl=9919#Text_Scale_X">Text_Scale_X</a> | <a href="bmh_undocumented?cl=9919#Text">Text</a> horizontal scale. |
| <a href="bmh_SkPaint?cl=9919#Text_Skew_X">Text_Skew_X</a> | <a href="bmh_undocumented?cl=9919#Text">Text</a> horizontal slant. |
| <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> | <a href="bmh_undocumented?cl=9919#Text">Text</a> encoded as characters or glyphs. |
| <a href="bmh_SkPaint?cl=9919#Font_Metrics">Font_Metrics</a> | Common glyph dimensions. |
| <a href="bmh_SkPaint?cl=9919#Measure_Text">Measure_Text</a> | Width, height, bounds of text. |
| <a href="bmh_SkPaint?cl=9919#Text_Path">Text_Path</a> | Geometry of glyphs. |
| <a href="bmh_SkPaint?cl=9919#Text_Intercepts">Text_Intercepts</a> | Advanced underline, strike through. |
| <a href="bmh_SkPaint?cl=9919#Fast_Bounds">Fast_Bounds</a> | Appproxiate area required by <a href="bmh_SkPaint?cl=9919#Paint">Paint</a>. |

## <a name="Constants"></a> Constants

| constants | description |
| --- | ---  |
| <a href="bmh_SkPaint?cl=9919#Align">Align</a> | Glyph locations relative to text position. |
| <a href="bmh_SkPaint?cl=9919#Cap">Cap</a> | Start and end geometry on stroked shapes. |
| <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> | Values described by bits and masks. |
| <a href="bmh_SkPaint?cl=9919#FontMetrics_FontMetricsFlags">FontMetrics::FontMetricsFlags</a> | Valid <a href="bmh_SkPaint?cl=9919#Font_Metrics">Font_Metrics</a>. |
| <a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a> | Level of glyph outline adjustment. |
| <a href="bmh_SkPaint?cl=9919#Join">Join</a> | Corner geometry on stroked shapes. |
| <a href="bmh_SkPaint?cl=9919#Style">Style</a> | Stroke, fill, or both. |
| <a href="bmh_SkPaint?cl=9919#TextEncoding">TextEncoding</a> | Character or glyph encoding size. |

## <a name="Structs"></a> Structs

| struct | description |
| --- | ---  |
| <a href="bmh_SkPaint?cl=9919#FontMetrics">FontMetrics</a> | <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> values. |

## <a name="Constructors"></a> Constructors

|  | description |
| --- | ---  |
| <a href="bmh_SkPaint?cl=9919#SkPaint">SkPaint</a>() | Constructs with default values. |
| <a href="bmh_SkPaint?cl=9919#SkPaint">SkPaint</a>(const <a href="bmh_SkPaint?cl=9919#SkPaint">SkPaint</a>& paint) | Makes a shallow copy. |
| <a href="bmh_SkPaint?cl=9919#SkPaint">SkPaint</a>(<a href="bmh_SkPaint?cl=9919#SkPaint">SkPaint</a>&& paint) | Moves paint without copying it. |
|  | Decreases <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> of owned objects. |

## <a name="Operators"></a> Operators

| operator | description |
| --- | ---  |
| <a href="bmh_SkPaint?cl=9919#copy_assignment_operator">operator=(const SkPaint&)</a> | Makes a shallow copy. |
| <a href="bmh_SkPaint?cl=9919#move_assignment_operator">operator=(SkPaint&&)</a> | Moves paint without copying it. |
| <a href="bmh_SkPaint?cl=9919#equal_operator">operator==(const SkPaint& a, const SkPaint& b)</a> | Compares paints for equality. |
| <a href="bmh_SkPaint?cl=9919#not_equal_operator">operator!=(const SkPaint& a, const SkPaint& b)</a> | Compares paints for inequality. |

## <a name="Member_Functions"></a> Member Functions

| function | description |
| --- | ---  |
| <a href="bmh_SkPaint?cl=9919#breakText">breakText</a> | Returns text that fits in a width. |
| <a href="bmh_SkPaint?cl=9919#canComputeFastBounds">canComputeFastBounds</a> | Returns true if settings allow for fast bounds computation. |
| <a href="bmh_SkPaint?cl=9919#computeFastBounds">computeFastBounds</a> | Returns fill bounds for quick reject tests. |
| <a href="bmh_SkPaint?cl=9919#computeFastStrokeBounds">computeFastStrokeBounds</a> | Returns stroke bounds for quick reject tests. |
| <a href="bmh_SkPaint?cl=9919#containsText">containsText</a> | Returns if all text corresponds to glyphs. |
| <a href="bmh_SkPaint?cl=9919#countText">countText</a> | Returns number of glyphs in text. |
| <a href="bmh_SkPaint?cl=9919#doComputeFastBounds">doComputeFastBounds</a> | Returns bounds for quick reject tests. |
| <a href="bmh_SkPaint?cl=9919#flatten">flatten</a> | Serializes into a buffer. |
| <a href="bmh_SkPaint?cl=9919#getAlpha">getAlpha</a> | Returns <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a>, color opacity. |
| <a href="bmh_SkPaint?cl=9919#getBlendMode">getBlendMode</a> | Returns <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a>, how colors combine with dest. |
| <a href="bmh_SkPaint?cl=9919#getColor">getColor</a> | Returns <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> and <a href="bmh_undocumented?cl=9919#RGB">Color_RGB</a>, one drawing color. |
| <a href="bmh_SkPaint?cl=9919#getColorFilter">getColorFilter</a> | Returns <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>, how colors are altered. |
| <a href="bmh_SkPaint?cl=9919#getDrawLooper">getDrawLooper</a> | Returns <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a>, multiple layers. |
| <a href="bmh_SkPaint?cl=9919#getFillPath">getFillPath</a> | Returns fill path equivalent to stroke. |
| <a href="bmh_SkPaint?cl=9919#getFilterQuality">getFilterQuality</a> | Returns <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter_Quality</a>, image filtering level. |
| <a href="bmh_SkPaint?cl=9919#getFlags">getFlags</a> | Returns <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> stored in a bit field. |
| <a href="bmh_SkPaint?cl=9919#getFontBounds">getFontBounds</a> | Returns union all glyph bounds. |
| <a href="bmh_SkPaint?cl=9919#getFontMetrics">getFontMetrics</a> | Returns <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> metrics scaled by text size. |
| <a href="bmh_SkPaint?cl=9919#getFontSpacing">getFontSpacing</a> | Returns recommended spacing between lines. |
| <a href="bmh_SkPaint?cl=9919#getHash">getHash</a> | Returns a shallow hash for equality checks. |
| <a href="bmh_SkPaint?cl=9919#getHinting">getHinting</a> | Returns <a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a>, glyph outline adjustment level. |
| <a href="bmh_SkPaint?cl=9919#getImageFilter">getImageFilter</a> | Returns <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a>, alter pixels; blur. |
| <a href="bmh_SkPaint?cl=9919#getMaskFilter">getMaskFilter</a> | Returns <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>, alterations to <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask_Alpha</a>. |
| <a href="bmh_SkPaint?cl=9919#getPathEffect">getPathEffect</a> | Returns <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, modifications to path geometry; dashing. |
| <a href="bmh_SkPaint?cl=9919#getPosTextPath">getPosTextPath</a> | Returns <a href="bmh_undocumented?cl=9919#Path">Path</a> equivalent to positioned text. |
| <a href="bmh_SkPaint?cl=9919#getPosTextIntercepts">getPosTextIntercepts</a> | Returns where lines intersect positioned text; underlines. |
| <a href="bmh_SkPaint?cl=9919#getPosTextHIntercepts">getPosTextHIntercepts</a> | Returns where lines intersect horizontally positioned text; underlines. |
| <a href="bmh_SkPaint?cl=9919#getRasterizer">getRasterizer</a> | Returns <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask_Alpha</a> generation from <a href="bmh_undocumented?cl=9919#Path">Path</a>. |
| <a href="bmh_SkPaint?cl=9919#getShader">getShader</a> | Returns <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, multiple drawing colors; gradients. |
| <a href="bmh_SkPaint?cl=9919#getStrokeCap">getStrokeCap</a> | Returns <a href="bmh_SkPaint?cl=9919#Cap">Cap</a>, the area drawn at path ends. |
| <a href="bmh_SkPaint?cl=9919#getStrokeJoin">getStrokeJoin</a> | Returns <a href="bmh_SkPaint?cl=9919#Join">Join</a>, geometry on path corners. |
| <a href="bmh_SkPaint?cl=9919#getStrokeMiter">getStrokeMiter</a> | Returns <a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a>, angles with sharp corners. |
| <a href="bmh_SkPaint?cl=9919#getStrokeWidth">getStrokeWidth</a> | Returns thickness of the stroke. |
| <a href="bmh_SkPaint?cl=9919#getStyle">getStyle</a> | Returns <a href="bmh_SkPaint?cl=9919#Style">Style</a>: stroke, fill, or both. |
| <a href="bmh_SkPaint?cl=9919#getTextAlign">getTextAlign</a> | Returns <a href="bmh_SkPaint?cl=9919#Align">Align</a>: left, center, or right. |
| <a href="bmh_SkPaint?cl=9919#getTextBlobIntercepts">getTextBlobIntercepts</a> | Returns where lines intersect <a href="bmh_undocumented?cl=9919#Text_Blob">Text_Blob</a>; underlines. |
| <a href="bmh_SkPaint?cl=9919#getTextEncoding">getTextEncoding</a> | Returns character or glyph encoding size. |
| <a href="bmh_SkPaint?cl=9919#getTextIntercepts">getTextIntercepts</a> | Returns where lines intersect text; underlines. |
| <a href="bmh_SkPaint?cl=9919#getTextPath">getTextPath</a> | Returns <a href="bmh_undocumented?cl=9919#Path">Path</a> equivalent to text. |
| <a href="bmh_SkPaint?cl=9919#getTextScaleX">getTextScaleX</a> | Returns the text horizontal scale; condensed text. |
| <a href="bmh_SkPaint?cl=9919#getTextSkewX">getTextSkewX</a> | Returns the text horizontal skew; oblique text. |
| <a href="bmh_SkPaint?cl=9919#getTextSize">getTextSize</a> | Returns text size in points. |
| <a href="bmh_SkPaint?cl=9919#getTextWidths">getTextWidths</a> | Returns advance and bounds for each glyph in text. |
| <a href="bmh_SkPaint?cl=9919#getTypeface">getTypeface</a> | Returns <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, font description. |
| <a href="bmh_SkPaint?cl=9919#glyphsToUnichars">glyphsToUnichars</a> | Converts glyphs into text. |
| <a href="bmh_SkPaint?cl=9919#isAntiAlias">isAntiAlias</a> | Returns true if <a href="bmh_SkPaint?cl=9919#Anti_alias">Anti-alias</a> is set. |
| <a href="bmh_SkPaint?cl=9919#isAutohinted">isAutohinted</a> | Deprecated. |
| <a href="bmh_SkPaint?cl=9919#isDevKernText">isDevKernText</a> | Returns true if <a href="bmh_SkPaint?cl=9919#Full_Hinting_Spacing">Full_Hinting_Spacing</a> is set. |
| <a href="bmh_SkPaint?cl=9919#isDither">isDither</a> | Returns true if <a href="bmh_SkPaint?cl=9919#Dither">Dither</a> is set. |
| <a href="bmh_SkPaint?cl=9919#isEmbeddedBitmapText">isEmbeddedBitmapText</a> | Returns true if <a href="bmh_SkPaint?cl=9919#Font_Embedded_Bitmaps">Font_Embedded_Bitmaps</a> is set. |
| <a href="bmh_SkPaint?cl=9919#isFakeBoldText">isFakeBoldText</a> | Returns true if <a href="bmh_SkPaint?cl=9919#Fake_Bold">Fake_Bold</a> is set. |
| <a href="bmh_SkPaint?cl=9919#isLCDRenderText">isLCDRenderText</a> | Returns true if <a href="bmh_SkPaint?cl=9919#LCD_Text">LCD_Text</a> is set. |
| <a href="bmh_SkPaint?cl=9919#isSrcOver">isSrcOver</a> | Returns true if <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a> is <a href="bmh_undocumented?cl=9919#kSrcOver">SkBlendMode::kSrcOver</a>. |
| <a href="bmh_SkPaint?cl=9919#isSubpixelText">isSubpixelText</a> | Returns true if <a href="bmh_SkPaint?cl=9919#Subpixel_Text">Subpixel_Text</a> is set. |
| <a href="bmh_SkPaint?cl=9919#isVerticalText">isVerticalText</a> | Returns true if <a href="bmh_SkPaint?cl=9919#Vertical_Text">Vertical_Text</a> is set. |
| <a href="bmh_SkPaint?cl=9919#measureText">measureText</a> | Returns advance width and bounds of text. |
| <a href="bmh_SkPaint?cl=9919#nothingToDraw">nothingToDraw</a> | Returns true if <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> prevents all drawing. |
| <a href="bmh_SkPaint?cl=9919#refColorFilter">refColorFilter</a> | References <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>, how colors are altered. |
| <a href="bmh_SkPaint?cl=9919#refDrawLooper">refDrawLooper</a> | References <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a>, multiple layers. |
| <a href="bmh_SkPaint?cl=9919#refImageFilter">refImageFilter</a> | References <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a>, alter pixels; blur. |
| <a href="bmh_SkPaint?cl=9919#refMaskFilter">refMaskFilter</a> | References <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>, alterations to <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask_Alpha</a>. |
| <a href="bmh_SkPaint?cl=9919#refPathEffect">refPathEffect</a> | References <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, modifications to path geometry; dashing. |
| <a href="bmh_SkPaint?cl=9919#refRasterizer">refRasterizer</a> | References <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, mask generation from path. |
| <a href="bmh_SkPaint?cl=9919#refShader">refShader</a> | References <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, multiple drawing colors; gradients. |
| <a href="bmh_SkPaint?cl=9919#refTypeface">refTypeface</a> | References <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, font description. |
| <a href="bmh_SkPaint?cl=9919#reset">reset</a> | Sets to default values. |
| <a href="bmh_SkPaint?cl=9919#setAlpha">setAlpha</a> | Sets <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a>, color opacity. |
| <a href="bmh_SkPaint?cl=9919#setAntiAlias">setAntiAlias</a> | Sets or clears <a href="bmh_SkPaint?cl=9919#Anti_alias">Anti-alias</a>. |
| <a href="bmh_SkPaint?cl=9919#setARGB">setARGB</a> | Sets color by component. |
| <a href="bmh_SkPaint?cl=9919#setAutohinted">setAutohinted</a> | Deprecated. |
| <a href="bmh_SkPaint?cl=9919#setBlendMode">setBlendMode</a> | Sets <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a>, how colors combine with destination. |
| <a href="bmh_SkPaint?cl=9919#setColor">setColor</a> | Sets <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> and <a href="bmh_undocumented?cl=9919#RGB">Color_RGB</a>, one drawing color. |
| <a href="bmh_SkPaint?cl=9919#setColorFilter">setColorFilter</a> | Sets <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>, alters color. |
| <a href="bmh_SkPaint?cl=9919#setDevKernText">setDevKernText</a> | Sets or clears <a href="bmh_SkPaint?cl=9919#Full_Hinting_Spacing">Full_Hinting_Spacing</a>. |
| <a href="bmh_SkPaint?cl=9919#setDither">setDither</a> | Sets or clears <a href="bmh_SkPaint?cl=9919#Dither">Dither</a>. |
| <a href="bmh_SkPaint?cl=9919#setDrawLooper">setDrawLooper</a> | Sets <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a>, multiple layers. |
| <a href="bmh_SkPaint?cl=9919#setEmbeddedBitmapText">setEmbeddedBitmapText</a> | Sets or clears <a href="bmh_SkPaint?cl=9919#Font_Embedded_Bitmaps">Font_Embedded_Bitmaps</a>. |
| <a href="bmh_SkPaint?cl=9919#setFakeBoldText">setFakeBoldText</a> | Sets or clears <a href="bmh_SkPaint?cl=9919#Fake_Bold">Fake_Bold</a>. |
| <a href="bmh_SkPaint?cl=9919#setFilterQuality">setFilterQuality</a> | Sets or clears <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter_Quality</a>, image filtering level. |
| <a href="bmh_SkPaint?cl=9919#setFlags">setFlags</a> | Sets multiple <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> in a bit field. |
| <a href="bmh_SkPaint?cl=9919#setHinting">setHinting</a> | Sets <a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a>, glyph outline adjustment level. |
| <a href="bmh_SkPaint?cl=9919#setLCDRenderText">setLCDRenderText</a> | Sets or clears <a href="bmh_SkPaint?cl=9919#LCD_Text">LCD_Text</a>. |
| <a href="bmh_SkPaint?cl=9919#setMaskFilter">setMaskFilter</a> | Sets <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>, alterations to <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask_Alpha</a>. |
| <a href="bmh_SkPaint?cl=9919#setPathEffect">setPathEffect</a> | Sets <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, modifications to path geometry; dashing. |
| <a href="bmh_SkPaint?cl=9919#setRasterizer">setRasterizer</a> | Sets <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask_Alpha</a> generation from <a href="bmh_undocumented?cl=9919#Path">Path</a>. |
| <a href="bmh_SkPaint?cl=9919#setImageFilter">setImageFilter</a> | Sets <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a>, alter pixels; blur. |
| <a href="bmh_SkPaint?cl=9919#setShader">setShader</a> | Sets <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, multiple drawing colors; gradients. |
| <a href="bmh_SkPaint?cl=9919#setStrokeCap">setStrokeCap</a> | Sets <a href="bmh_SkPaint?cl=9919#Cap">Cap</a>, the area drawn at path ends. |
| <a href="bmh_SkPaint?cl=9919#setStrokeJoin">setStrokeJoin</a> | Sets <a href="bmh_SkPaint?cl=9919#Join">Join</a>, geometry on path corners. |
| <a href="bmh_SkPaint?cl=9919#setStrokeMiter">setStrokeMiter</a> | Sets <a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a>, angles with sharp corners. |
| <a href="bmh_SkPaint?cl=9919#setStrokeWidth">setStrokeWidth</a> | Sets thickness of the stroke. |
| <a href="bmh_SkPaint?cl=9919#setStyle">setStyle</a> | Sets <a href="bmh_SkPaint?cl=9919#Style">Style</a>: stroke, fill, or both. |
| <a href="bmh_SkPaint?cl=9919#setSubpixelText">setSubpixelText</a> | Sets or clears <a href="bmh_SkPaint?cl=9919#Subpixel_Text">Subpixel_Text</a>. |
| <a href="bmh_SkPaint?cl=9919#setTextAlign">setTextAlign</a> | Sets <a href="bmh_SkPaint?cl=9919#Align">Align</a>: left, center, or right. |
| <a href="bmh_SkPaint?cl=9919#setTextEncoding">setTextEncoding</a> | Sets character or glyph encoding size. |
| <a href="bmh_SkPaint?cl=9919#setTextMatrix">setTextMatrix</a> | Returns <a href="bmh_undocumented?cl=9919#Matrix">Matrix</a> built from text size, scale, and skew. |
| <a href="bmh_SkPaint?cl=9919#setTextScaleX">setTextScaleX</a> | Sets the text horizontal scale; condensed text. |
| <a href="bmh_SkPaint?cl=9919#setTextSkewX">setTextSkewX</a> | Sets the text horizontal skew; oblique text. |
| <a href="bmh_SkPaint?cl=9919#setTextSize">setTextSize</a> | Sets text size in points. |
| <a href="bmh_SkPaint?cl=9919#setTypeface">setTypeface</a> | Sets <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, font description. |
| <a href="bmh_SkPaint?cl=9919#setVerticalText">setVerticalText</a> | Sets or clears <a href="bmh_SkPaint?cl=9919#Vertical_Text">Vertical_Text</a>. |
| <a href="bmh_SkPaint?cl=9919#textToGlyphs">textToGlyphs</a> | Converts text into glyph indices. |
| <a href="bmh_SkPaint?cl=9919#toString">toString</a> | Converts <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> to machine parsable form (<a href="bmh_undocumented?cl=9919#Developer_Mode">Developer_Mode</a>) |
| <a href="bmh_SkPaint?cl=9919#unflatten">unflatten</a> | Populates from a serialized stream. |

# <a name="Initializers"></a> Initializers

<a name="empty_constructor"></a>

## SkPaint

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkPaint()
</pre>

Constructs <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> with default values.

| attribute | default value |
| --- | ---  |
| <a href="bmh_SkPaint?cl=9919#Anti_alias">Anti-alias</a> | false |
| <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a> | <a href="bmh_undocumented?cl=9919#kSrcOver">SkBlendMode::kSrcOver</a> |
| <a href="bmh_undocumented?cl=9919#Color">Color</a> | <a href="bmh_undocumented?cl=9919#SK_ColorBLACK">SK_ColorBLACK</a> |
| <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> | 255 |
| <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a> | nullptr |
| <a href="bmh_SkPaint?cl=9919#Dither">Dither</a> | false |
| <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> | nullptr |
| <a href="bmh_SkPaint?cl=9919#Fake_Bold">Fake_Bold</a> | false |
| <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter_Quality</a> | <a href="bmh_undocumented?cl=9919#SkFilterQuality">kNone_SkFilterQuality</a> |
| <a href="bmh_SkPaint?cl=9919#Font_Embedded_Bitmaps">Font_Embedded_Bitmaps</a> | false |
| <a href="bmh_SkPaint?cl=9919#Forced_Auto_hinting">Forced_Auto-hinting</a> | false |
| <a href="bmh_SkPaint?cl=9919#Full_Hinting_Spacing">Full_Hinting_Spacing</a> | false |
| <a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a> | <a href="bmh_SkPaint?cl=9919#kNormal_Hinting">kNormal_Hinting</a> |
| <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> | nullptr |
| <a href="bmh_SkPaint?cl=9919#LCD_Text">LCD_Text</a> | false |
| <a href="bmh_SkPaint?cl=9919#Linear_Text">Linear_Text</a> | false |
| <a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a> | 4 |
| <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> | nullptr |
| <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> | nullptr |
| <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> | nullptr |
| <a href="bmh_undocumented?cl=9919#Shader">Shader</a> | nullptr |
| <a href="bmh_SkPaint?cl=9919#Style">Style</a> | <a href="bmh_SkPaint?cl=9919#kFill_Style">kFill_Style</a> |
| <a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> | <a href="bmh_SkPaint?cl=9919#kLeft_Align">kLeft_Align</a> |
| <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> | <a href="bmh_SkPaint?cl=9919#kUTF8_TextEncoding">kUTF8_TextEncoding</a> |
| <a href="bmh_SkPaint?cl=9919#Text_Scale_X">Text_Scale_X</a> | 1 |
| <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> | 12 |
| <a href="bmh_SkPaint?cl=9919#Text_Skew_X">Text_Skew_X</a> | 0 |
| <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> | nullptr |
| <a href="bmh_SkPaint?cl=9919#Stroke_Cap">Stroke_Cap</a> | <a href="bmh_SkPaint?cl=9919#kButt_Cap">kButt_Cap</a> |
| <a href="bmh_SkPaint?cl=9919#Stroke_Join">Stroke_Join</a> | <a href="bmh_SkPaint?cl=9919#kMiter_Join">kMiter_Join</a> |
| <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> | 0 |
| <a href="bmh_SkPaint?cl=9919#Subpixel_Text">Subpixel_Text</a> | false |
| <a href="bmh_SkPaint?cl=9919#Vertical_Text">Vertical_Text</a> | false |

The flags, text size, hinting, and miter limit may be overridden at compile time by defining
paint default values. The overrides may be included in <a href="bmh_undocumented?cl=9919#SkUserConfig.h">SkUserConfig.h</a> or predefined by the 
build system.

### Example

<fiddle-embed name="9db1cabd02ad20f6e8654de6d697b1bc">

</fiddle-embed>

---

<a name="copy_constructor"></a>

## SkPaint

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkPaint(const SkPaint& paint)
</pre>

Makes a shallow copy of <a href="bmh_SkPaint?cl=9919#Paint">Paint</a>. <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, <a href="bmh_undocumented?cl=9919#Shader">Shader</a>,
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>, <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>, <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a>, and <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> are shared
between the original paint and the copy. These objects' <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> are increased.
The referenced objects <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>, <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>, <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>,
<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a>, and <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> cannot be modified after they are created.
This prevents objects with <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> from being modified once <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> refers to them.

### Parameters

<table>
  <tr>
    <td><code><strong>paint</strong></code></td> <td>Original identical to the copy.</td>
  </tr>
</table>

### Example

<fiddle-embed name="2de7fa7ce51a8f560b2951f8f5732cca">

#### Example Output

~~~~
SK_ColorRED == paint1.getColor()
SK_ColorBLUE == paint2.getColor()
~~~~

</fiddle-embed>

---

<a name="move_constructor"></a>

## SkPaint

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkPaint(SkPaint&& paint)
</pre>

Implements a move constructor to avoid incrementing the reference counts
of objects referenced by the paint.

### Parameters

<table>
  <tr>
    <td><code><strong>paint</strong></code></td> <td>After the call, undefined except that it can be safely destructed.</td>
  </tr>
</table>

### Example

<fiddle-embed name="e1134ec3127c6793ba0c5841e0e2a35e">

#### Example Output

~~~~
path effect unique: true
~~~~

</fiddle-embed>

---

<a name="reset"></a>

## reset

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void reset()
</pre>

Sets all paint's contents to their initial values. This is equivalent to replacing
the paint with the result of <a href="bmh_SkPaint?cl=9919#SkPaint">SkPaint</a>().

### Example

<fiddle-embed name="af301c5e19c8f5adab8c23e47413923f">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed>

---

# <a name="Destructor"></a> Destructor

<a name="destructor"></a>

## ~SkPaint

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
~SkPaint()
</pre>

Decreases <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> of owned objects: <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, <a href="bmh_undocumented?cl=9919#Shader">Shader</a>,
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>, <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>, <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a>, and <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a>. If the
objects' reference count goes to zero, delete them.
---

# <a name="Management"></a> Management

<a name="copy_assignment_operator"></a>

## operator=

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkPaint& operator=(const SkPaint&)
</pre>

Makes a shallow copy of <a href="bmh_SkPaint?cl=9919#Paint">Paint</a>. <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, <a href="bmh_undocumented?cl=9919#Shader">Shader</a>,
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>, <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>, <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a>, and <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> are shared
between the original paint and the copy. The objects' <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> are in the
prior destination are decreased by one, and the referenced objects are deleted if the
resulting count is zero. The objects' <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> in the parameter paint are increased
by one.

### Parameters

<table>
  <tr>
    <td><code><strong>paint</strong></code></td> <td>Original is unmodified.</td>
  </tr>
</table>

### Return Value

Copy identical to paint.

### Example

<fiddle-embed name="a33951d3aac20c13eab365dddb06e8e5">

#### Example Output

~~~~
SK_ColorRED == paint1.getColor()
SK_ColorRED == paint2.getColor()
~~~~

</fiddle-embed>

---

<a name="move_assignment_operator"></a>

## operator=

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkPaint& operator=(SkPaint&&)
</pre>

Moves the paint to avoid incrementing the reference counts
of objects referenced by the paint parameter. The objects' <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> are in the
prior destination are decreased by one, and the referenced objects are deleted if the
resulting count is zero.

### Parameters

<table>
  <tr>
    <td><code><strong>paint</strong></code></td> <td>After the call, undefined except that it can be safely destructed.</td>
  </tr>
</table>

### Return Value

Identical to paint with unmodfied <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Example

<fiddle-embed name="b178ef2aaf49b5da6ec78b87873ad0c2">

#### Example Output

~~~~
SK_ColorRED == paint2.getColor()
~~~~

</fiddle-embed>

---

<a name="equal_operator"></a>

## operator==

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool operator==(const SkPaint& a, const SkPaint& b)
</pre>

Compares a and b. If it returns true, a and b are equivalent. It may return false
if <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>, <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>, <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a>,
or <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> have identical contents but different pointers.

### Parameters

<table>
  <tr>
    <td><code><strong>a</strong></code></td> <td><a href="bmh_SkPaint?cl=9919#Paint">Paint</a> to compare.</td>
  </tr>
  <tr>
    <td><code><strong>b</strong></code></td> <td><a href="bmh_SkPaint?cl=9919#Paint">Paint</a> to compare.</td>
  </tr>
</table>

### Return Value

true if paints are equivalent.

### Example

<fiddle-embed name="2ebadcc4709652a1f39981ccfe4f85a6">

#### Example Output

~~~~
paint1 == paint2
paint1 != paint2
~~~~

</fiddle-embed>

---

<a name="not_equal_operator"></a>

## operator!=

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool operator!=(const SkPaint& a, const SkPaint& b)
</pre>

Returns true if the paints are not equivalent, but may return true
if <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>, <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>, <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a>,
or <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> have identical contents but different pointers.

### Parameters

<table>
  <tr>
    <td><code><strong>a</strong></code></td> <td><a href="bmh_SkPaint?cl=9919#Paint">Paint</a> to compare.</td>
  </tr>
  <tr>
    <td><code><strong>b</strong></code></td> <td><a href="bmh_SkPaint?cl=9919#Paint">Paint</a> to compare.</td>
  </tr>
</table>

### Return Value

true if paints are not equivalent.

### Example

<fiddle-embed name="96fb444c9a41da32105af262bfa4413d">

#### Example Output

~~~~
paint1 == paint2
paint1 == paint2
~~~~

</fiddle-embed>

---

<a name="getHash"></a>

## getHash

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
uint32_t getHash()  const
</pre>

Returns a hash generated from <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> values and pointers.
Identical hashes guarantee that the paints are
equivalent, but differing hashes do not guarantee that the paints have differing
contents.
If <a href="bmh_SkPaint?cl=9919#equal_operator">operator==(const SkPaint& a, const SkPaint& b)</a> returns true for two paints,
their hashes are also equal.
The hash returned is platform and implementation specific.

### Return Value

A shallow hash.

### Example

<fiddle-embed name="008d3bc3dfe20a442a77c0121e9a4c53">

#### Example Output

~~~~
paint1 == paint2
paint1.getHash() == paint2.getHash()
~~~~

</fiddle-embed>

---

<a name="flatten"></a>

## flatten

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void flatten(SkWriteBuffer&)  const
</pre>

Serializes <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> into a buffer. A companion <a href="bmh_SkPaint?cl=9919#unflatten">unflatten</a> call
can reconstitute the paint at a later time.

### Parameters

<table>
  <tr>
    <td><code><strong>buffer</strong></code></td> <td><a href="bmh_undocumented?cl=9919#Write_Buffer">Write_Buffer</a> receiving the flattened <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> data.</td>
  </tr>
</table>

### Example

<fiddle-embed name="c0840d7a4a2992f81082639f6c86f7bd">

#### Example Output

~~~~
color = 0xffff0000
~~~~

</fiddle-embed>

---

<a name="unflatten"></a>

## unflatten

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void unflatten(SkReadBuffer&)
</pre>

Populates <a href="bmh_SkPaint?cl=9919#Paint">Paint</a>, typically from a serialized stream, created by calling
<a href="bmh_SkPaint?cl=9919#flatten">flatten</a> at an earlier time.
<a href="bmh_undocumented?cl=9919#SkReadBuffer">SkReadBuffer</a> class is not public, so <a href="bmh_SkPaint?cl=9919#unflatten">unflatten</a> cannot be meaningfully called
by the client.
---

# <a name="Hinting"></a> Hinting

## <a name="SkPaint::Hinting"></a> Enum SkPaint::Hinting

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a> {
    <a href="bmh_SkPaint?cl=9919#kNo_Hinting">kNo_Hinting</a>            = 0,
    <a href="bmh_SkPaint?cl=9919#kSlight_Hinting">kSlight_Hinting</a>        = 1,
    <a href="bmh_SkPaint?cl=9919#kNormal_Hinting">kNormal_Hinting</a>        = 2,
    <a href="bmh_SkPaint?cl=9919#kFull_Hinting">kFull_Hinting</a>          = 3
};</pre>
<a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a> adjusts the glyph outlines so that the shape provides a uniform
look at a given point size on font engines that support it. <a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a> may have a
muted effect or no effect at all depending on the platform.
The four levels roughly control corresponding features on platforms that use <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a>
as their <a href="bmh_undocumented?cl=9919#Engine">Font_Engine</a>.

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kNo_Hinting"></a> <code><strong>SkPaint::kNo::Hinting</strong></code></td><td>0</td><td>Leaves glyph outlines unchanged from their native representation.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kSlight_Hinting"></a> <code><strong>SkPaint::kSlight::Hinting</strong></code></td><td>1</td><td>Modifies glyph outlines minimally to improve constrast.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kNormal_Hinting"></a> <code><strong>SkPaint::kNormal::Hinting</strong></code></td><td>2</td><td>Modifies glyph outlines to improve constrast.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kFull_Hinting"></a> <code><strong>SkPaint::kFull::Hinting</strong></code></td><td>3</td><td>Modifies glyph outlines for maxiumum constrast.</td>
  </tr>
</table>

On <a href="bmh_undocumented?cl=9919#Windows">Windows</a> with <a href="bmh_undocumented?cl=9919#DirectWrite">DirectWrite</a>, <a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a> has no effect.
<a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a> defaults to <a href="bmh_SkPaint?cl=9919#kNormal_Hinting">kNormal_Hinting</a>.
Set <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Hinting">SkPaintDefaults_Hinting</a> at compile time to change the default setting.

<a name="getHinting"></a>

## getHinting

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
Hinting getHinting()  const
</pre>

Returns level of glyph outline adjustment.

### Return Value

One of: <a href="bmh_SkPaint?cl=9919#kNo_Hinting">kNo_Hinting</a>, <a href="bmh_SkPaint?cl=9919#kSlight_Hinting">kSlight_Hinting</a>, <a href="bmh_SkPaint?cl=9919#kNormal_Hinting">kNormal_Hinting</a>, <a href="bmh_SkPaint?cl=9919#kFull_Hinting">kFull_Hinting</a>.

### Example

<fiddle-embed name="c91cfb5a3767889b802c577ebfd73221">

#### Example Output

~~~~
SkPaint::kNormal_Hinting == paint.getHinting()
~~~~

</fiddle-embed>

---

<a name="setHinting"></a>

## setHinting

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setHinting(Hinting hintingLevel)  const
</pre>

Sets level of glyph outline adjustment.
Does not check for valid values of hintingLevel.

### Parameters

<table>
  <tr>
    <td><code><strong>hintingLevel</strong></code></td> <td>One of: <a href="bmh_SkPaint?cl=9919#kNo_Hinting">kNo_Hinting</a>, <a href="bmh_SkPaint?cl=9919#kSlight_Hinting">kSlight_Hinting</a>, <a href="bmh_SkPaint?cl=9919#kNormal_Hinting">kNormal_Hinting</a>, <a href="bmh_SkPaint?cl=9919#kFull_Hinting">kFull_Hinting</a>.</td>
  </tr>
</table>

### Example

<fiddle-embed name="39a0706eaf1135f09657265ea8ad1d37">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed>

---

# <a name="Flags"></a> Flags

## <a name="SkPaint::Flags"></a> Enum SkPaint::Flags

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> {
    <a href="bmh_SkPaint?cl=9919#kAntiAlias_Flag">kAntiAlias_Flag</a>       = 0x01,
    <a href="bmh_SkPaint?cl=9919#kDither_Flag">kDither_Flag</a>          = 0x04,
    <a href="bmh_SkPaint?cl=9919#kFakeBoldText_Flag">kFakeBoldText_Flag</a>    = 0x20,
    <a href="bmh_SkPaint?cl=9919#kLinearText_Flag">kLinearText_Flag</a>      = 0x40,
    <a href="bmh_SkPaint?cl=9919#kSubpixelText_Flag">kSubpixelText_Flag</a>    = 0x80,
    <a href="bmh_SkPaint?cl=9919#kDevKernText_Flag">kDevKernText_Flag</a>     = 0x100,
    <a href="bmh_SkPaint?cl=9919#kLCDRenderText_Flag">kLCDRenderText_Flag</a>   = 0x200,
    <a href="bmh_SkPaint?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText_Flag</a> = 0x400,
    <a href="bmh_SkPaint?cl=9919#kAutoHinting_Flag">kAutoHinting_Flag</a>     = 0x800,
    <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a>    = 0x1000,
    <a href="bmh_SkPaint?cl=9919#kGenA8FromLCD_Flag">kGenA8FromLCD_Flag</a>    = 0x2000,
    <a href="bmh_SkPaint?cl=9919#kAllFlags">kAllFlags</a> = 0xFFFF,
#ifdef <a href="bmh_undocumented?cl=9919#SK_SUPPORT_LEGACY_PAINT_TEXTDECORATION">SK_SUPPORT_LEGACY_PAINT_TEXTDECORATION</a>
    <a href="bmh_SkPaint?cl=9919#kUnderlineText_Flag">kUnderlineText_Flag</a>   = 0x08,
    <a href="bmh_SkPaint?cl=9919#kStrikeThruText_Flag">kStrikeThruText_Flag</a>  = 0x10,
#endif
};
</pre>
The bit values stored in <a href="bmh_SkPaint?cl=9919#Flags">Flags</a>.
The default value for <a href="bmh_SkPaint?cl=9919#Flags">Flags</a>, normally zero, can be changed at compile time
with a custom definition of <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Flags">SkPaintDefaults_Flags</a>.
All flags can be read and written explicitly; <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> allows manipulating
multiple settings at once.

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kAntiAlias_Flag"></a> <code><strong>SkPaint::kAntiAlias::Flag</strong></code></td><td>0x0001 </td><td>Controls if <a href="bmh_SkPaint?cl=9919#Anti_alias">Anti-alias</a> is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kDither_Flag"></a> <code><strong>SkPaint::kDither::Flag</strong></code></td><td>0x0004</td><td>Controls if <a href="bmh_SkPaint?cl=9919#Dither">Dither</a> is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kUnderlineText_Flag"></a> <code><strong>SkPaint::kUnderlineText::Flag</strong></code></td><td>0x0008</td><td>Deprecated.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kStrikeThruText_Flag"></a> <code><strong>SkPaint::kStrikeThruText::Flag</strong></code></td><td>0x1000</td><td>Deprecated.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kFakeBoldText_Flag"></a> <code><strong>SkPaint::kFakeBoldText::Flag</strong></code></td><td>0x0020</td><td>Controls if <a href="bmh_SkPaint?cl=9919#Fake_Bold">Fake_Bold</a> is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kLinearText_Flag"></a> <code><strong>SkPaint::kLinearText::Flag</strong></code></td><td>0x0040</td><td>Controls if <a href="bmh_SkPaint?cl=9919#Linear_Text">Linear_Text</a> is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kSubpixelText_Flag"></a> <code><strong>SkPaint::kSubpixelText::Flag</strong></code></td><td>0x0080</td><td>Controls if <a href="bmh_SkPaint?cl=9919#Subpixel_Text">Subpixel_Text</a> is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kDevKernText_Flag"></a> <code><strong>SkPaint::kDevKernText::Flag</strong></code></td><td>0x0100</td><td>Controls if <a href="bmh_SkPaint?cl=9919#Full_Hinting_Spacing">Full_Hinting_Spacing</a> is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kLCDRenderText_Flag"></a> <code><strong>SkPaint::kLCDRenderText::Flag</strong></code></td><td>0x0200</td><td>Controls if <a href="bmh_SkPaint?cl=9919#LCD_Text">LCD_Text</a> is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kEmbeddedBitmapText_Flag"></a> <code><strong>SkPaint::kEmbeddedBitmapText::Flag</strong></code></td><td>0x0400</td><td>Controls if <a href="bmh_SkPaint?cl=9919#Font_Embedded_Bitmaps">Font_Embedded_Bitmaps</a> is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kAutoHinting_Flag"></a> <code><strong>SkPaint::kAutoHinting::Flag</strong></code></td><td>0x0800</td><td>Controls if <a href="bmh_SkPaint?cl=9919#Forced_Auto_hinting">Forced_Auto-hinting</a> is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kVerticalText_Flag"></a> <code><strong>SkPaint::kVerticalText::Flag</strong></code></td><td>0x1000</td><td>Controls if <a href="bmh_SkPaint?cl=9919#Vertical_Text">Vertical_Text</a> is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kGenA8FromLCD_Flag"></a> <code><strong>SkPaint::kGenA8FromLCD::Flag</strong></code></td><td>0x2000</td><td>Not intended for public use.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kAllFlags"></a> <code><strong>SkPaint::kAllFlags</strong></code></td><td>0xFFFF</td><td>Masks the range of <a href="bmh_SkPaint?cl=9919#Flags">Flags</a>, including private flags and flags
reserved for future use.</td>
  </tr>
<a href="bmh_SkPaint?cl=9919#Flags">Flags</a> default to all flags clear, disabling the associated feature.
</table>

## <a name="SkPaint::ReserveFlags"></a> Enum SkPaint::ReserveFlags

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
#ifdef <a href="bmh_undocumented?cl=9919#SK_BUILD_FOR_ANDROID_FRAMEWORK">SK_BUILD_FOR_ANDROID_FRAMEWORK</a>
    enum <a href="bmh_SkPaint?cl=9919#ReserveFlags">ReserveFlags</a> {
        <a href="bmh_SkPaint?cl=9919#kUnderlineText_ReserveFlag">kUnderlineText_ReserveFlag</a>   = 0x08,
        <a href="bmh_SkPaint?cl=9919#kStrikeThruText_ReserveFlag">kStrikeThruText_ReserveFlag</a>  = 0x10,
    };
#endif</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kUnderlineText_ReserveFlag"></a> <code><strong>SkPaint::kUnderlineText::ReserveFlag</strong></code></td><td>0x0008</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPaint::kStrikeThruText_ReserveFlag"></a> <code><strong>SkPaint::kStrikeThruText::ReserveFlag</strong></code></td><td>0x0010</td><td></td>
  </tr>
</table>

<a name="getFlags"></a>

## getFlags

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
uint32_t getFlags()  const
</pre>

Returns paint settings described by <a href="bmh_SkPaint?cl=9919#Flags">Flags</a>. Each setting uses one
bit, and can be tested with <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> members.

### Return Value

Zero, one, or more bits described by <a href="bmh_SkPaint?cl=9919#Flags">Flags</a>.

### Example

<fiddle-embed name="e95d09086fe8a862f78087e5fff070a4">

#### Example Output

~~~~
(SkPaint::kAntiAlias_Flag & paint.getFlags()) != 0
~~~~

</fiddle-embed>

---

<a name="setFlags"></a>

## setFlags

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setFlags(uint32_t flags)
</pre>

Sets <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> to the union of the <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> members.

### Parameters

<table>
  <tr>
    <td><code><strong>flags</strong></code></td> <td>Union of <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> for <a href="bmh_SkPaint?cl=9919#Paint">Paint</a>.</td>
  </tr>
</table>

### Example

<fiddle-embed name="3c209da57ee0acba480ba05ade14b6d7">

#### Example Output

~~~~
paint.isAntiAlias()
paint.isDither()
~~~~

</fiddle-embed>

---

# <a name="Anti-alias"></a> Anti-alias
<a href="bmh_SkPaint?cl=9919#Anti_alias">Anti-alias</a> drawing approximates partial pixel coverage with transparency.
If <a href="bmh_SkPaint?cl=9919#kAntiAlias_Flag">kAntiAlias_Flag</a> is clear, pixel centers contained by the shape edge are drawn opaque.
If <a href="bmh_SkPaint?cl=9919#kAntiAlias_Flag">kAntiAlias_Flag</a> is set, pixels are drawn with <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> equal to their coverage.
The rule for aliased pixels is inconsistent across platforms. A shape edge 
passing through the pixel center may, but is not required to, draw the pixel.
<a href="bmh_undocumented?cl=9919#Raster_Engine">Raster_Engine</a> draws aliased pixels whose centers are on or to the right of the start of an
active <a href="bmh_undocumented?cl=9919#Path">Path</a> edge, and whose center is to the left of the end of the active <a href="bmh_undocumented?cl=9919#Path">Path</a> edge.
A platform may only support anti-aliased drawing. Some <a href="bmh_undocumented?cl=9919#GPU_backed">GPU-backed</a> platforms use
supersampling to anti-alias all drawing, and have no mechanism to selectively
alias.
The amount of coverage computed for anti-aliased pixels also varies across platforms.
<a href="bmh_SkPaint?cl=9919#Anti_alias">Anti-alias</a> is disabled by default.
<a href="bmh_SkPaint?cl=9919#Anti_alias">Anti-alias</a> can be enabled by default by setting <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Flags">SkPaintDefaults_Flags</a> to <a href="bmh_SkPaint?cl=9919#kAntiAlias_Flag">kAntiAlias_Flag</a>
at compile time.

### Example

<fiddle-embed name="1bb6f16fe6899fedbd155f862680aad1">

A red line is drawn with transparency on the edges to make it look smoother.
A blue line draws only where the pixel centers are contained.
The lines are drawn into an offscreen bitmap, then drawn magified to make the
aliasing easier to see.

</fiddle-embed>

<a name="isAntiAlias"></a>

## isAntiAlias

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool isAntiAlias()  const
</pre>

If true, pixels on the active edges of <a href="bmh_undocumented?cl=9919#Path">Path</a> may be drawn with partial transparency.
Equivalent to <a href="bmh_SkPaint?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint?cl=9919#kAntiAlias_Flag">kAntiAlias_Flag</a>.

### Return Value

<a href="bmh_SkPaint?cl=9919#kAntiAlias_Flag">kAntiAlias_Flag</a> state.

### Example

<fiddle-embed name="aecda6fe10e4a19b2574f1f2fb80d437">

#### Example Output

~~~~
paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)
paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)
~~~~

</fiddle-embed>

---

<a name="setAntiAlias"></a>

## setAntiAlias

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setAntiAlias(bool aa)
</pre>

Requests, but does not require, that <a href="bmh_undocumented?cl=9919#Path">Path</a> edge pixels draw opaque or with
partial transparency.
Sets <a href="bmh_SkPaint?cl=9919#kAntiAlias_Flag">kAntiAlias_Flag</a> if aa is true.
Clears <a href="bmh_SkPaint?cl=9919#kAntiAlias_Flag">kAntiAlias_Flag</a> if aa is false.

### Parameters

<table>
  <tr>
    <td><code><strong>aa</strong></code></td> <td>Sets or clears <a href="bmh_SkPaint?cl=9919#kAntiAlias_Flag">kAntiAlias_Flag</a>.</td>
  </tr>
</table>

### Example

<fiddle-embed name="231c9631061232096c3d7bd94ddd5c27">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed>

---

# <a name="Dither"></a> Dither
<a href="bmh_SkPaint?cl=9919#Dither">Dither</a> distributes color error to smooth color transitions.
Dithering removes visible banding from gradients and improves rendering into
a <a href="bmh_undocumented?cl=9919#SkColorType">kRGB_565_SkColorType</a> <a href="bmh_undocumented?cl=9919#Surface">Surface</a>.
<a href="bmh_SkPaint?cl=9919#Dither">Dither</a> is disabled by default.
<a href="bmh_SkPaint?cl=9919#Dither">Dither</a> can be enabled by default by setting <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Flags">SkPaintDefaults_Flags</a> to <a href="bmh_SkPaint?cl=9919#kDither_Flag">kDither_Flag</a>
at compile time.
Some platform implementations may ignore dithering.

### Example

<fiddle-embed name="b28987b65d3be14717e4faaacaf2faa1">

</fiddle-embed>

<a name="isDither"></a>

## isDither

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool isDither()  const
</pre>

If true, color error may be distributed to smooth color transition.
 Equivalent to <a href="bmh_SkPaint?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint?cl=9919#kDither_Flag">kDither_Flag</a>.

### Return Value

<a href="bmh_SkPaint?cl=9919#kDither_Flag">kDither_Flag</a> state.

### Example

<fiddle-embed name="87f9538060779de8a0284d9227bdf91a">

#### Example Output

~~~~
paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag)
paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag)
~~~~

</fiddle-embed>

---

<a name="setDither"></a>

## setDither

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setDither(bool dither)
</pre>

Suggests, but does not require, to distribute color error.
Sets <a href="bmh_SkPaint?cl=9919#kDither_Flag">kDither_Flag</a> if dither is true.
Clears <a href="bmh_SkPaint?cl=9919#kDither_Flag">kDither_Flag</a> if dither is false.

### Parameters

<table>
  <tr>
    <td><code><strong>dither</strong></code></td> <td>Sets or clears <a href="bmh_SkPaint?cl=9919#kDither_Flag">kDither_Flag</a>.</td>
  </tr>
</table>

### Example

<fiddle-embed name="4c7a25e4a7a314d84147a399763c9773">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed>

### See Also

<a href="bmh_undocumented?cl=9919#SkColorType">kRGB_565_SkColorType</a>

---

### See Also

Gradient <a href="bmh_undocumented?cl=9919#RGB_565">Color_RGB-565</a>

# <a name="Device_Text"></a> Device Text
<a href="bmh_SkPaint?cl=9919#LCD_Text">LCD_Text</a> and <a href="bmh_SkPaint?cl=9919#Subpixel_Text">Subpixel_Text</a> increase the precision of glyph position.
When set, <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint?cl=9919#kLCDRenderText_Flag">kLCDRenderText_Flag</a> takes advantage of the organization of <a href="bmh_undocumented?cl=9919#RGB">Color_RGB</a> stripes that 
create a color, and relies
on the small size of the stripe and visual perception to make the color fringing inperceptible.
<a href="bmh_SkPaint?cl=9919#LCD_Text">LCD_Text</a> can be enabled on devices that orient stripes horizontally or vertically, and that order
the color components as <a href="bmh_undocumented?cl=9919#RGB">Color_RGB</a> or <a href="bmh_undocumented?cl=9919#RBG">Color_RBG</a>.
<a href="bmh_SkPaint?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint?cl=9919#kSubpixelText_Flag">kSubpixelText_Flag</a> uses the pixel transparency to represent a fractional offset. 
As the opaqueness
of the color increases, the edge of the glyph appears to move towards the outside of the pixel.
Either or both techniques can be enabled.
<a href="bmh_SkPaint?cl=9919#kLCDRenderText_Flag">kLCDRenderText_Flag</a> and <a href="bmh_SkPaint?cl=9919#kSubpixelText_Flag">kSubpixelText_Flag</a> are clear by default.
<a href="bmh_SkPaint?cl=9919#LCD_Text">LCD_Text</a> or <a href="bmh_SkPaint?cl=9919#Subpixel_Text">Subpixel_Text</a> can be enabled by default by setting <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Flags">SkPaintDefaults_Flags</a> to 
<a href="bmh_SkPaint?cl=9919#kLCDRenderText_Flag">kLCDRenderText_Flag</a> or <a href="bmh_SkPaint?cl=9919#kSubpixelText_Flag">kSubpixelText_Flag</a> (or both) at compile time.

### Example

<fiddle-embed name="525062c7023c2c5f2f962a09baad75eb">

Four commas are drawn normally and with combinations of <a href="bmh_SkPaint?cl=9919#LCD_Text">LCD_Text</a> and <a href="bmh_SkPaint?cl=9919#Subpixel_Text">Subpixel_Text</a>.
When <a href="bmh_SkPaint?cl=9919#Subpixel_Text">Subpixel_Text</a> is disabled, the comma glyphs are indentical, but not evenly spaced.
When <a href="bmh_SkPaint?cl=9919#Subpixel_Text">Subpixel_Text</a> is enabled, the comma glyphs are unique, but appear evenly spaced.

</fiddle-embed>

## <a name="Linear_Text"></a> Linear Text

<a href="bmh_SkPaint?cl=9919#Linear_Text">Linear_Text</a> appears to have no effect, and has been superceded by setting <a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a> to <a href="bmh_SkPaint?cl=9919#kNo_Hinting">kNo_Hinting</a>.
If that's not the case, document here.

<a name="isLinearText"></a>

## isLinearText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool isLinearText()  const
</pre>

---

<a name="setLinearText"></a>

## setLinearText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setLinearText(bool linearText)
</pre>

---

## <a name="Subpixel_Text"></a> Subpixel Text

<a href="bmh_SkPaint?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint?cl=9919#kSubpixelText_Flag">kSubpixelText_Flag</a> uses the pixel transparency to represent a fractional offset. 
As the opaqueness
of the color increases, the edge of the glyph appears to move towards the outside of the pixel.

<a name="isSubpixelText"></a>

## isSubpixelText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool isSubpixelText()  const
</pre>

If true, glyphs at different sub-pixel positions may differ on pixel edge coverage.
Equivalent to <a href="bmh_SkPaint?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint?cl=9919#kSubpixelText_Flag">kSubpixelText_Flag</a>.

### Return Value

<a href="bmh_SkPaint?cl=9919#kSubpixelText_Flag">kSubpixelText_Flag</a> state.

### Example

<fiddle-embed name="fc6e1fa6b9a383e6529211dc773a464d">

#### Example Output

~~~~
paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)
paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)
~~~~

</fiddle-embed>

---

<a name="setSubpixelText"></a>

## setSubpixelText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setSubpixelText(bool subpixelText)
</pre>

Requests, but does not require, that glyphs respect sub-pixel positioning.
Sets <a href="bmh_SkPaint?cl=9919#kSubpixelText_Flag">kSubpixelText_Flag</a> if subpixelText is true.
Clears <a href="bmh_SkPaint?cl=9919#kSubpixelText_Flag">kSubpixelText_Flag</a> if subpixelText is false.

### Parameters

<table>
  <tr>
    <td><code><strong>subpixelText</strong></code></td> <td>Sets or clears <a href="bmh_SkPaint?cl=9919#kSubpixelText_Flag">kSubpixelText_Flag</a>.</td>
  </tr>
</table>

### Example

<fiddle-embed name="17270e681eb6931258acdbbbbefcc17c">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed>

---

## <a name="LCD_Text"></a> LCD Text

When set, <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint?cl=9919#kLCDRenderText_Flag">kLCDRenderText_Flag</a> takes advantage of the organization of <a href="bmh_undocumented?cl=9919#RGB">Color_RGB</a> stripes that 
create a color, and relies
on the small size of the stripe and visual perception to make the color fringing inperceptible.
<a href="bmh_SkPaint?cl=9919#LCD_Text">LCD_Text</a> can be enabled on devices that orient stripes horizontally or vertically, and that order
the color components as <a href="bmh_undocumented?cl=9919#RGB">Color_RGB</a> or <a href="bmh_undocumented?cl=9919#RBG">Color_RBG</a>.

<a name="isLCDRenderText"></a>

## isLCDRenderText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool isLCDRenderText()  const
</pre>

If true, glyphs may use <a href="bmh_undocumented?cl=9919#LCD">LCD</a> striping to improve glyph edges.
Returns true if <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint?cl=9919#kLCDRenderText_Flag">kLCDRenderText_Flag</a> is set.

### Return Value

<a href="bmh_SkPaint?cl=9919#kLCDRenderText_Flag">kLCDRenderText_Flag</a> state.

### Example

<fiddle-embed name="bf834391d2837b242c889c0591e828c8">

#### Example Output

~~~~
paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)
paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)
~~~~

</fiddle-embed>

---

<a name="setLCDRenderText"></a>

## setLCDRenderText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setLCDRenderText(bool lcdText)
</pre>

Requests, but does not require, that glyphs use <a href="bmh_undocumented?cl=9919#LCD">LCD</a> striping for glyph edges.
Sets <a href="bmh_SkPaint?cl=9919#kLCDRenderText_Flag">kLCDRenderText_Flag</a> if lcdText is true.
Clears <a href="bmh_SkPaint?cl=9919#kLCDRenderText_Flag">kLCDRenderText_Flag</a> if lcdText is false.

### Parameters

<table>
  <tr>
    <td><code><strong>lcdText</strong></code></td> <td>Sets or clears <a href="bmh_SkPaint?cl=9919#kLCDRenderText_Flag">kLCDRenderText_Flag</a>.</td>
  </tr>
</table>

### Example

<fiddle-embed name="50f5dc9c5337271fc088aab9f6e73bbf">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed>

---

# <a name="Font_Embedded_Bitmaps"></a> Font Embedded Bitmaps
<a href="bmh_SkPaint?cl=9919#Font_Embedded_Bitmaps">Font_Embedded_Bitmaps</a> allows selecting custom-sized bitmap glyphs.
<a href="bmh_SkPaint?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText_Flag</a> when set chooses an embedded bitmap glyph over an outline contained
in a font if the platform supports this option. 
<a href="bmh_undocumented?cl=9919#FreeType">FreeType</a> selects the bitmap glyph if available when <a href="bmh_SkPaint?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText_Flag</a> is set, and selects
the outline glyph if <a href="bmh_SkPaint?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText_Flag</a> is clear.
<a href="bmh_undocumented?cl=9919#Windows">Windows</a> may select the bitmap glyph but is not required to do so.
<a href="bmh_undocumented?cl=9919#OS_X">OS_X</a> and iOS do not support this option.
<a href="bmh_SkPaint?cl=9919#Font_Embedded_Bitmaps">Font_Embedded_Bitmaps</a> is disabled by default.
<a href="bmh_SkPaint?cl=9919#Font_Embedded_Bitmaps">Font_Embedded_Bitmaps</a> can be enabled by default by setting <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Flags">SkPaintDefaults_Flags</a> to
<a href="bmh_SkPaint?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText_Flag</a> at compile time.

### Example

<fiddle-embed name="">

The hintgasp <a href="bmh_undocumented?cl=9919#TrueType">TrueType</a> font in the <a href="bmh_undocumented?cl=9919#Skia">Skia</a> resources/fonts directory includes an embedded
bitmap glyph at odd font sizes. This example works on platforms that use <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a>
as their <a href="bmh_undocumented?cl=9919#Engine">Font_Engine</a>.
<a href="bmh_undocumented?cl=9919#Windows">Windows</a> may, but is not required to, return a bitmap glyph if <a href="bmh_SkPaint?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText_Flag</a> is set.

</fiddle-embed>

<a name="isEmbeddedBitmapText"></a>

## isEmbeddedBitmapText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool isEmbeddedBitmapText()  const
</pre>

If true, <a href="bmh_undocumented?cl=9919#Engine">Font_Engine</a> may return glyphs from font bitmaps instead of from outlines.
Equivalent to <a href="bmh_SkPaint?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText_Flag</a>.

### Return Value

<a href="bmh_SkPaint?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText_Flag</a> state.

### Example

<fiddle-embed name="3f4c8fd210a7d276871a11a1b330f401">

#### Example Output

~~~~
paint.isEmbeddedBitmapText() == !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)
paint.isEmbeddedBitmapText() == !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)
~~~~

</fiddle-embed>

---

<a name="setEmbeddedBitmapText"></a>

## setEmbeddedBitmapText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setEmbeddedBitmapText(bool useEmbeddedBitmapText)
</pre>

Requests, but does not require, to use bitmaps in fonts instead of outlines.
Sets <a href="bmh_SkPaint?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText_Flag</a> if useEmbeddedBitmapText is true.
Clears <a href="bmh_SkPaint?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText_Flag</a> if useEmbeddedBitmapText is false.

### Parameters

<table>
  <tr>
    <td><code><strong>aa</strong></code></td> <td>Sets or clears <a href="bmh_SkPaint?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText_Flag</a>.</td>
  </tr>
</table>

### Example

<fiddle-embed name="4d0c99d7d4a6f39d3c51a1be09060e48">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed>

---

# <a name="Forced_Auto-hinting"></a> Forced Auto-hinting
<a href="bmh_SkPaint?cl=9919#Forced_Auto_hinting">Forced_Auto-hinting</a> no longer has any effect.
Set <a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a> to <a href="bmh_SkPaint?cl=9919#kNo_Hinting">kNo_Hinting</a> to leave the font outlines unhinted instead.

<a name="isAutohinted"></a>

## isAutohinted

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool isAutohinted()  const
</pre>

Has no effect.
---

<a name="setAutohinted"></a>

## setAutohinted

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setAutohinted(bool useAutohinter)
</pre>

Has no effect.
---

# <a name="Vertical_Text"></a> Vertical Text
<a href="bmh_undocumented?cl=9919#Text">Text</a> may be drawn by positioning each glyph, or by positioning the first glyph and
using <a href="bmh_undocumented?cl=9919#Advance">Font_Advance</a> to position subsequent glyphs. By default, each successive glyph
is positioned to the right of the preceeding glyph. <a href="bmh_SkPaint?cl=9919#Vertical_Text">Vertical_Text</a> sets successive
glyphs to position below the preceeding glyph.
<a href="bmh_undocumented?cl=9919#Skia">Skia</a> can translate text character codes as a series of glyphs, but does not implement
font substitution, 
textual substitution, line layout, or contextual spacing like kerning pairs. Use
a text shaping engine likeHarfBuzz #http://harfbuzz.org/to translate text runs
into glyph series.
<a href="bmh_SkPaint?cl=9919#Vertical_Text">Vertical_Text</a> is clear if text is drawn left to right or set if drawn from top to bottom.
<a href="bmh_SkPaint?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> if clear draws text left to right.
<a href="bmh_SkPaint?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> if set draws text top to bottom.
<a href="bmh_SkPaint?cl=9919#Vertical_Text">Vertical_Text</a> is clear by default.
<a href="bmh_SkPaint?cl=9919#Vertical_Text">Vertical_Text</a> can be set by default by setting <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Flags">SkPaintDefaults_Flags</a> to
<a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> at compile time.

### Example

<fiddle-embed name="3c4995e97b7d0d0afeb718e44f9472c0">

</fiddle-embed>

<a name="isVerticalText"></a>

## isVerticalText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool isVerticalText()  const
</pre>

If true, glyphs are drawn top to bottom instead of left to right.
Equivalent to <a href="bmh_SkPaint?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a>.

### Return Value

<a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> state.

### Example

<fiddle-embed name="8de09d99eebade65c6446d8bfc980fe1">

#### Example Output

~~~~
paint.isVerticalText() == !!(paint.getFlags() & SkPaint::kVerticalText_Flag)
paint.isVerticalText() == !!(paint.getFlags() & SkPaint::kVerticalText_Flag)
~~~~

</fiddle-embed>

---

<a name="setVerticalText"></a>

## setVerticalText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setVerticalText(bool vertical)
</pre>

If true, text advance positions the next glyph below the previous glyph instead of to the
right of previous glyph.
Sets <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> if vertical is true.
Clears <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> if vertical is false.

### Parameters

<table>
  <tr>
    <td><code><strong>vertical</strong></code></td> <td>Sets or clears <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a>.</td>
  </tr>
</table>

### Example

<fiddle-embed name="4bc82860cbf0e9a9d2cc2627879ecc7f">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed>

---

# <a name="Text_Decorations"></a> Text Decorations

<a name="isUnderlineText"></a>

## isUnderlineText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool isUnderlineText()  const
</pre>

requires <a href="bmh_undocumented?cl=9919#SK_SUPPORT_LEGACY_PAINT_TEXTDECORATION">SK_SUPPORT_LEGACY_PAINT_TEXTDECORATION</a>
unsure if/how to document this---

<a name="isStrikeThruText"></a>

## isStrikeThruText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool isStrikeThruText()  const { return false; }
</pre>

requires <a href="bmh_undocumented?cl=9919#SK_SUPPORT_LEGACY_PAINT_TEXTDECORATION">SK_SUPPORT_LEGACY_PAINT_TEXTDECORATION</a>
unsure if/how to document this---

## <a name="Fake_Bold"></a> Fake Bold

<a href="bmh_SkPaint?cl=9919#Fake_Bold">Fake_Bold</a> approximates the bold font style accompanying a normal font when a bold font face
is not available. <a href="bmh_undocumented?cl=9919#Skia">Skia</a> does not provide font substitution; it is up to the client to find the
bold font face using the platform's <a href="bmh_undocumented?cl=9919#Font_Manager">Font_Manager</a>.
Use <a href="bmh_SkPaint?cl=9919#Text_Skew_X">Text_Skew_X</a> to approximate an italic font style when the italic font face 
is not available.
A <a href="bmh_undocumented?cl=9919#FreeType_based">FreeType-based</a> port may define <a href="bmh_undocumented?cl=9919#SK_USE_FREETYPE_EMBOLDEN">SK_USE_FREETYPE_EMBOLDEN</a> at compile time to direct
the font engine to create the bold glyphs. Otherwise, the extra bold is computed
by increasing the stroke width and setting the <a href="bmh_SkPaint?cl=9919#Style">Style</a> to <a href="bmh_SkPaint?cl=9919#kStrokeAndFill_Style">kStrokeAndFill_Style</a> as needed.  
<a href="bmh_SkPaint?cl=9919#Fake_Bold">Fake_Bold</a> is disabled by default.

### Example

<fiddle-embed name="c68a5108ac9e8945c30bf215549599b2">

</fiddle-embed>

<a name="isFakeBoldText"></a>

## isFakeBoldText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool isFakeBoldText()  const;
</pre>

If true, approximate bold by increasing the stroke width when creating glyph bitmaps
from outlines.
Equivalent to <a href="bmh_SkPaint?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint?cl=9919#kFakeBoldText_Flag">kFakeBoldText_Flag</a>.

### Return Value

<a href="bmh_SkPaint?cl=9919#kFakeBoldText_Flag">kFakeBoldText_Flag</a> state.

### Example

<fiddle-embed name="bc4ff096643bd3a132429f68d410492e">

#### Example Output

~~~~
paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)
paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)
~~~~

</fiddle-embed>

---

<a name="setFakeBoldText"></a>

## setFakeBoldText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setFakeBoldText(bool fakeBoldText)
</pre>

Use increased stroke width when creating glyph bitmaps to approximate bolding.
Sets <a href="bmh_SkPaint?cl=9919#kFakeBoldText_Flag">kFakeBoldText_Flag</a> if fakeBoldText is true.
Clears <a href="bmh_SkPaint?cl=9919#kFakeBoldText_Flag">kFakeBoldText_Flag</a> if fakeBoldText is false.

### Parameters

<table>
  <tr>
    <td><code><strong>fakeBoldText</strong></code></td> <td>Sets or clears <a href="bmh_SkPaint?cl=9919#kFakeBoldText_Flag">kFakeBoldText_Flag</a>.</td>
  </tr>
</table>

### Example

<fiddle-embed name="6e6982855b1a836762050e6b0bd5f618">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed>

---

# <a name="Full_Hinting_Spacing"></a> Full Hinting Spacing
<a href="bmh_SkPaint?cl=9919#Full_Hinting_Spacing">Full_Hinting_Spacing</a> adjusts the character spacing by the difference of the 
hinted and unhinted left and right side bearings, 
if <a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a> is set to <a href="bmh_SkPaint?cl=9919#kFull_Hinting">kFull_Hinting</a>. <a href="bmh_SkPaint?cl=9919#Full_Hinting_Spacing">Full_Hinting_Spacing</a> only
applies to platforms that use <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a> as their <a href="bmh_undocumented?cl=9919#Engine">Font_Engine</a>.
<a href="bmh_SkPaint?cl=9919#Full_Hinting_Spacing">Full_Hinting_Spacing</a> is not related to text kerning, where the space between
a specific pair of characters is adjusted using data in the font's kerning tables.

<a name="isDevKernText"></a>

## isDevKernText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool isDevKernText()  const
</pre>

Returns if character spacing may be adjusted by the hinting difference.
Equivalent to <a href="bmh_SkPaint?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint?cl=9919#kDevKernText_Flag">kDevKernText_Flag</a>.

### Return Value

<a href="bmh_SkPaint?cl=9919#kDevKernText_Flag">kDevKernText_Flag</a> state.

### Example

<fiddle-embed name="2a0016c5875bf070d18508457143fb16">

</fiddle-embed>

---

<a name="setDevKernText"></a>

## setDevKernText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setDevKernText(bool devKernText)
</pre>

Requests, but does not require, to use hinting to adjust glyph spacing.
Sets <a href="bmh_SkPaint?cl=9919#kDevKernText_Flag">kDevKernText_Flag</a> if devKernText is true.
Clears <a href="bmh_SkPaint?cl=9919#kDevKernText_Flag">kDevKernText_Flag</a> if devKernText is false.

### Parameters

<table>
  <tr>
    <td><code><strong>devKernText</strong></code></td> <td>Sets or clears devKernText.</td>
  </tr>
</table>

### Example

<fiddle-embed name="c00e36f5f3c49317c622e88e97d334a3">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed>

---

# <a name="Filter_Quality_Methods"></a> Filter Quality Methods
<a href="bmh_undocumented?cl=9919#Filter_Quality">Filter_Quality</a> trades speed for image filtering when the image is scaled.
A lower <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter_Quality</a> draws faster, but has less fidelity.
A higher <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter_Quality</a> draws slower, but looks better.
If the image is unscaled, the <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter_Quality</a> choice will not result in a noticable
difference.
<a href="bmh_undocumented?cl=9919#Filter_Quality">Filter_Quality</a> is used in <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> passed as a parameter to<a href="bmh_SkCanvas?cl=9919#drawBitmap">SkCanvas::drawBitmap</a>
<a href="bmh_SkCanvas?cl=9919#drawBitmapRect">SkCanvas::drawBitmapRect</a>
<a href="bmh_SkCanvas?cl=9919#drawImage">SkCanvas::drawImage</a>
<a href="bmh_SkCanvas?cl=9919#drawImageRect">SkCanvas::drawImageRect</a>and when <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> has a <a href="bmh_undocumented?cl=9919#Shader">Shader</a> specialization that uses <a href="bmh_undocumented?cl=9919#Image">Image</a> or <a href="bmh_undocumented?cl=9919#Bitmap">Bitmap</a>.
<a href="bmh_undocumented?cl=9919#Filter_Quality">Filter_Quality</a> is <a href="bmh_undocumented?cl=9919#SkFilterQuality">kNone_SkFilterQuality</a> by default.

### Example

<fiddle-embed name="97d214c30dcacf9f13a1487d5d2b377d">

</fiddle-embed>

<a name="getFilterQuality"></a>

## getFilterQuality

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkFilterQuality getFilterQuality()  const
</pre>

A lower setting
draws faster; a higher setting looks better when the image is scaled.

### Return Value

One of: <a href="bmh_undocumented?cl=9919#SkFilterQuality">kNone_SkFilterQuality</a>, <a href="bmh_undocumented?cl=9919#SkFilterQuality">kLow_SkFilterQuality</a>, 
                 <a href="bmh_undocumented?cl=9919#SkFilterQuality">kMedium_SkFilterQuality</a>, <a href="bmh_undocumented?cl=9919#SkFilterQuality">kHigh_SkFilterQuality</a>.

### Example

<fiddle-embed name="ba511d61d6149608616b8dc9af296b23">

#### Example Output

~~~~
kNone_SkFilterQuality == paint.getFilterQuality()
~~~~

</fiddle-embed>

---

<a name="setFilterQuality"></a>

## setFilterQuality

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setFilterQuality(SkFilterQuality quality)
</pre>

A lower setting
draws faster; a higher setting looks better when the image is scaled.

### Parameters

<table>
  <tr>
    <td><code><strong>quality</strong></code></td> <td>One of: <a href="bmh_undocumented?cl=9919#SkFilterQuality">kNone_SkFilterQuality</a>, <a href="bmh_undocumented?cl=9919#SkFilterQuality">kLow_SkFilterQuality</a>, 
                 <a href="bmh_undocumented?cl=9919#SkFilterQuality">kMedium_SkFilterQuality</a>, <a href="bmh_undocumented?cl=9919#SkFilterQuality">kHigh_SkFilterQuality</a>.
                 Not checked to see if quality is valid.</td>
  </tr>
</table>

### Example

<fiddle-embed name="9385d0ef925a17fd1eb5651b816e496e">

#### Example Output

~~~~
kHigh_SkFilterQuality == paint.getFilterQuality()
~~~~

</fiddle-embed>

### See Also

<a href="bmh_undocumented?cl=9919#SkFilterQuality">SkFilterQuality</a> <a href="bmh_undocumented?cl=9919#Image_Scaling">Image_Scaling</a>

---

# <a name="Color_Methods"></a> Color Methods
<a href="bmh_undocumented?cl=9919#Color">Color</a> specifies the <a href="bmh_undocumented?cl=9919#RGB_Red">Color_RGB_Red</a>, <a href="bmh_undocumented?cl=9919#RGB_Blue">Color_RGB_Blue</a>, <a href="bmh_undocumented?cl=9919#RGB_Green">Color_RGB_Green</a>, and <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> values used to draw a filled
or stroked shape in a
32-bit value. Each component occupies 8-bits, ranging from zero: no contribution;
to 255: full intensity. All values in any combination are valid.
<a href="bmh_undocumented?cl=9919#Color">Color</a> is not premultiplied;
<a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> sets the transparency independent of <a href="bmh_undocumented?cl=9919#RGB">Color_RGB</a>: <a href="bmh_undocumented?cl=9919#RGB_Red">Color_RGB_Red</a>, <a href="bmh_undocumented?cl=9919#RGB_Blue">Color_RGB_Blue</a>, and <a href="bmh_undocumented?cl=9919#RGB_Green">Color_RGB_Green</a>.
The bit positions of <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> and <a href="bmh_undocumented?cl=9919#RGB">Color_RGB</a> are independent of the bit positions
on the output device, which may have more or fewer bits, and may have a different arrangement.

| bit positions | <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> | <a href="bmh_undocumented?cl=9919#RGB_Red">Color_RGB_Red</a> | <a href="bmh_undocumented?cl=9919#RGB_Blue">Color_RGB_Blue</a> | <a href="bmh_undocumented?cl=9919#RGB_Green">Color_RGB_Green</a> |
| --- | --- | --- | --- | ---  |
|  | 31 - 24 | 23 - 16 | 15 - 8 | 7 - 0 |

### Example

<fiddle-embed name="f6eec81567acaca6638d696ad00cfb5d">

</fiddle-embed>

<a name="getColor"></a>

## getColor

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkColor getColor()  const
</pre>

Retrieves <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> and <a href="bmh_undocumented?cl=9919#RGB">Color_RGB</a>, unpremultiplied, packed into 32 bits.
Use helpers <a href="bmh_undocumented?cl=9919#SkColorGetA">SkColorGetA</a>, <a href="bmh_undocumented?cl=9919#SkColorGetR">SkColorGetR</a>, <a href="bmh_undocumented?cl=9919#SkColorGetG">SkColorGetG</a>, and <a href="bmh_undocumented?cl=9919#SkColorGetB">SkColorGetB</a> to extract
a color component.

### Return Value

Unpremultiplied <a href="bmh_undocumented?cl=9919#ARGB">Color_ARGB</a>.

### Example

<fiddle-embed name="896fd0df04c096398b5d5d7d47061cdf">

#### Example Output

~~~~
Yellow is 100% red, 100% green, and 0% blue.
~~~~

</fiddle-embed>

### See Also

<a href="bmh_undocumented?cl=9919#SkColor">SkColor</a>

---

<a name="setColor"></a>

## setColor

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setColor(SkColor color)
</pre>

Sets <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> and <a href="bmh_undocumented?cl=9919#RGB">Color_RGB</a> used when stroking and filling. The color is a 32-bit value,
unpremutiplied, packing 8-bit components for <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a>, <a href="bmh_undocumented?cl=9919#RGB_Red">Color_RGB_Red</a>, <a href="bmh_undocumented?cl=9919#RGB_Blue">Color_RGB_Blue</a>, and <a href="bmh_undocumented?cl=9919#RGB_Green">Color_RGB_Green</a>. 

### Parameters

<table>
  <tr>
    <td><code><strong>color</strong></code></td> <td>Unpremultiplied <a href="bmh_undocumented?cl=9919#ARGB">Color_ARGB</a>.</td>
  </tr>
</table>

### Example

<fiddle-embed name="be988544a8f36325c3480cf679ae021a">

#### Example Output

~~~~
green1 == green2
~~~~

</fiddle-embed>

### See Also

<a href="bmh_undocumented?cl=9919#SkColor">SkColor</a> <a href="bmh_SkPaint?cl=9919#setARGB">setARGB</a> <a href="bmh_undocumented?cl=9919#SkColorSetARGB">SkColorSetARGB</a>

---

## <a name="Alpha_Methods"></a> Alpha Methods

<a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> sets the transparency independent of <a href="bmh_undocumented?cl=9919#RGB">Color_RGB</a>: <a href="bmh_undocumented?cl=9919#RGB_Red">Color_RGB_Red</a>, <a href="bmh_undocumented?cl=9919#RGB_Blue">Color_RGB_Blue</a>, and <a href="bmh_undocumented?cl=9919#RGB_Green">Color_RGB_Green</a>.

<a name="getAlpha"></a>

## getAlpha

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
uint8_t getAlpha()  const
</pre>

Retrieves <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> from the <a href="bmh_undocumented?cl=9919#Color">Color</a> used when stroking and filling.

### Return Value

<a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> ranging from zero, fully transparent, to 255, fully opaque.

### Example

<fiddle-embed name="2b1f43ba1731db56cccd04172c62bb84">

#### Example Output

~~~~
255 == paint.getAlpha()
~~~~

</fiddle-embed>

---

<a name="setAlpha"></a>

## setAlpha

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setAlpha(U8CPU a)
</pre>

Replaces <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a>, leaving <a href="bmh_undocumented?cl=9919#RGB">Color_RGB</a> 
unchanged. An out of range value triggers an assert in the debug
build.

### Parameters

<table>
  <tr>
    <td><code><strong>a</strong></code></td> <td><a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> component (0..255) of <a href="bmh_undocumented?cl=9919#Color">Color</a>.</td>
  </tr>
</table>

### Example

<fiddle-embed name="4e783d6b70a95b2669f497ad7c4d5243">

#### Example Output

~~~~
0x44112233 == paint.getColor()
~~~~

</fiddle-embed>

---

<a name="setARGB"></a>

## setARGB

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b)
</pre>

Sets <a href="bmh_undocumented?cl=9919#Color">Color</a> used when drawing solid fills. The color components range from 0 to 255.
The color is unpremultiplied;
<a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> sets the transparency independent of <a href="bmh_undocumented?cl=9919#RGB">Color_RGB</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>a</strong></code></td> <td>Amount of <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a>, from fully transparent (0) to fully opaque (255).</td>
  </tr>
  <tr>
    <td><code><strong>r</strong></code></td> <td>Amount of <a href="bmh_undocumented?cl=9919#RGB_Red">Color_RGB_Red</a>, from no red (0) to full red (255).</td>
  </tr>
  <tr>
    <td><code><strong>g</strong></code></td> <td>Amount of <a href="bmh_undocumented?cl=9919#RGB_Green">Color_RGB_Green</a>, from no green (0) to full green (255).</td>
  </tr>
  <tr>
    <td><code><strong>b</strong></code></td> <td>Amount of <a href="bmh_undocumented?cl=9919#RGB_Blue">Color_RGB_Blue</a>, from no blue (0) to full blue (255).</td>
  </tr>
</table>

### Example

<fiddle-embed name="345a684de24090f4c469e15659bd101d">

#### Example Output

~~~~
transRed1 == transRed2
~~~~

</fiddle-embed>

### See Also

<a href="bmh_SkPaint?cl=9919#setColor">setColor</a> <a href="bmh_undocumented?cl=9919#SkColorSetARGB">SkColorSetARGB</a>

---

# <a name="Style"></a> Style
<a href="bmh_SkPaint?cl=9919#Style">Style</a> specifies if the geometry is filled, stroked, or both filled and stroked.
Some shapes ignore <a href="bmh_SkPaint?cl=9919#Style">Style</a> and are always drawn filled or stroked.
Set <a href="bmh_SkPaint?cl=9919#Style">Style</a> to <a href="bmh_SkPaint?cl=9919#kFill_Style">kFill_Style</a> to fill the shape.
The fill covers the area inside the geometry for most shapes.
Set <a href="bmh_SkPaint?cl=9919#Style">Style</a> to <a href="bmh_SkPaint?cl=9919#kStroke_Style">kStroke_Style</a> to stroke the shape.

## <a name="Fill"></a> Fill

### See Also

<a href="bmh_undocumented?cl=9919#Fill_Type">Path_Fill_Type</a>

## <a name="Stroke"></a> Stroke

The stroke covers the area described by following the shape's edge with a pen or brush of
<a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a>. The area covered where the shape starts and stops is described by <a href="bmh_SkPaint?cl=9919#Stroke_Cap">Stroke_Cap</a>.
The area covered where the shape turns a corner is described by <a href="bmh_SkPaint?cl=9919#Stroke_Join">Stroke_Join</a>.
The stroke is centered on the shape; it extends equally on either side of the shape's edge.
As <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> gets smaller, the drawn path frame is thinner. <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> less than one
may have gaps, and if <a href="bmh_SkPaint?cl=9919#kAntiAlias_Flag">kAntiAlias_Flag</a> is set, <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> will increase to visually decrease coverage.

## <a name="Hairline"></a> Hairline

<a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> of zero has a special meaning and switches drawing to use <a href="bmh_SkPaint?cl=9919#Hairline">Hairline</a>.
<a href="bmh_SkPaint?cl=9919#Hairline">Hairline</a> draws the thinnest continuous frame. If <a href="bmh_SkPaint?cl=9919#kAntiAlias_Flag">kAntiAlias_Flag</a> is clear, adjacent pixels 
flow horizontally, vertically,or diagonally. 
<a href="bmh_undocumented?cl=9919#Path">Path</a> drawing with <a href="bmh_SkPaint?cl=9919#Hairline">Hairline</a> may hit the same pixel more than once. For instance, <a href="bmh_undocumented?cl=9919#Path">Path</a> containing
two lines in one <a href="bmh_undocumented?cl=9919#Contour">Path_Contour</a> will draw the corner point once, but may both lines may draw the adjacent
pixel. If <a href="bmh_SkPaint?cl=9919#kAntiAlias_Flag">kAntiAlias_Flag</a> is set, transparency is applied twice, resulting in a darker pixel. Some
<a href="bmh_undocumented?cl=9919#GPU_backed">GPU-backed</a> implementations apply transparency at a later drawing stage, avoiding double hit pixels
while stroking.
Set <a href="bmh_SkPaint?cl=9919#Style">Style</a> to <a href="bmh_SkPaint?cl=9919#kStrokeAndFill_Style">kStrokeAndFill_Style</a> to draw both simultaneously. The stroke and fill
share all paint attributes; for instance, they are drawn with the same color.

## <a name="SkPaint::Style"></a> Enum SkPaint::Style

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint?cl=9919#Style">Style</a> {
    <a href="bmh_SkPaint?cl=9919#kFill_Style">kFill_Style</a>,
    <a href="bmh_SkPaint?cl=9919#kStroke_Style">kStroke_Style</a>,
    <a href="bmh_SkPaint?cl=9919#kStrokeAndFill_Style">kStrokeAndFill_Style</a>,
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kFill_Style"></a> <code><strong>SkPaint::kFill::Style</strong></code></td><td>0</td><td><a href="bmh_SkPaint?cl=9919#kFill_Style">kFill_Style</a> applies to <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Region">Region</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round_Rect</a>, <a href="bmh_undocumented?cl=9919#Circle">Circle</a>, <a href="bmh_undocumented?cl=9919#Oval">Oval</a>, <a href="bmh_undocumented?cl=9919#Path">Path</a>, and <a href="bmh_undocumented?cl=9919#Text">Text</a>. 
<a href="bmh_undocumented?cl=9919#Bitmap">Bitmap</a>, <a href="bmh_undocumented?cl=9919#Image">Image</a>, <a href="bmh_undocumented?cl=9919#Patch">Patch</a>, <a href="bmh_undocumented?cl=9919#Region">Region</a>, <a href="bmh_undocumented?cl=9919#Sprite">Sprite</a>, and <a href="bmh_undocumented?cl=9919#Vertices">Vertices</a> are painted as if <a href="bmh_SkPaint?cl=9919#kFill_Style">kFill_Style</a> is set,
and ignore the set <a href="bmh_SkPaint?cl=9919#Style">Style</a>.
The <a href="bmh_undocumented?cl=9919#Fill_Type">Path_Fill_Type</a> specifies additional rules to fill the area outside the path edge,
and to create an unfilled hole inside the shape.
<a href="bmh_SkPaint?cl=9919#Style">Style</a> is set to <a href="bmh_SkPaint?cl=9919#kFill_Style">kFill_Style</a> by default.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kStroke_Style"></a> <code><strong>SkPaint::kStroke::Style</strong></code></td><td>1</td><td><a href="bmh_SkPaint?cl=9919#kStroke_Style">kStroke_Style</a> applies to <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Region">Region</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round_Rect</a>, <a href="bmh_undocumented?cl=9919#Arc">Arc</a>, <a href="bmh_undocumented?cl=9919#Circle">Circle</a>, <a href="bmh_undocumented?cl=9919#Oval">Oval</a>,
<a href="bmh_undocumented?cl=9919#Path">Path</a>, and <a href="bmh_undocumented?cl=9919#Text">Text</a>. 
<a href="bmh_undocumented?cl=9919#Arc">Arc</a>, <a href="bmh_undocumented?cl=9919#Line">Line</a>, <a href="bmh_undocumented?cl=9919#Point">Point</a>, and <a href="bmh_undocumented?cl=9919#Array">Point_Array</a> are always drawn as if <a href="bmh_SkPaint?cl=9919#kStroke_Style">kStroke_Style</a> is set,
and ignore the set <a href="bmh_SkPaint?cl=9919#Style">Style</a>.
The stroke construction is unaffected by the <a href="bmh_undocumented?cl=9919#Fill_Type">Path_Fill_Type</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kStrokeAndFill_Style"></a> <code><strong>SkPaint::kStrokeAndFill::Style</strong></code></td><td>2</td><td><a href="bmh_SkPaint?cl=9919#kStrokeAndFill_Style">kStrokeAndFill_Style</a> applies to <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Region">Region</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round_Rect</a>, <a href="bmh_undocumented?cl=9919#Circle">Circle</a>, <a href="bmh_undocumented?cl=9919#Oval">Oval</a>, <a href="bmh_undocumented?cl=9919#Path">Path</a>, and <a href="bmh_undocumented?cl=9919#Text">Text</a>.
<a href="bmh_undocumented?cl=9919#Path">Path</a> is treated as if it is set to <a href="bmh_undocumented?cl=9919#kWinding_FillType">SkPath::kWinding_FillType</a>,
and the set <a href="bmh_undocumented?cl=9919#Fill_Type">Path_Fill_Type</a> is ignored.</td>
  </tr>
Use stroke and fill to avoid hitting the same pixels twice with a stroke draw and
a fill draw.
<a href="bmh_undocumented?cl=9919#Path">Path</a> with <a href="bmh_undocumented?cl=9919#kEvenOdd_FillType">SkPath::kEvenOdd_FillType</a> drawn with
<a href="bmh_SkPaint?cl=9919#kStroke_Style">kStroke_Style</a> and then drawn with <a href="bmh_SkPaint?cl=9919#kFill_Style">kFill_Style</a> may draw differently from one drawn
with <a href="bmh_SkPaint?cl=9919#kStrokeAndFill_Style">kStrokeAndFill_Style</a>.</table>

## <a name="SkPaint_anonymous"></a> Enum SkPaint::anonymous

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum {
    <a href="bmh_SkPaint?cl=9919#kStyleCount">kStyleCount</a> = <a href="bmh_SkPaint?cl=9919#kStrokeAndFill_Style">kStrokeAndFill_Style</a> + 1
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kStyleCount"></a> <code><strong>SkPaint::kStyleCount</strong></code></td><td>3</td><td><a href="bmh_SkPaint?cl=9919#kStyleCount">kStyleCount</a> is the number of different <a href="bmh_SkPaint?cl=9919#Style">Style</a> values defined.
<a href="bmh_SkPaint?cl=9919#kStyleCount">kStyleCount</a> may be used to verify that <a href="bmh_SkPaint?cl=9919#Style">Style</a> is a legal value.</td>
  </tr>
</table>

<a name="getStyle"></a>

## getStyle

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
Style getStyle()  const
</pre>

Whether the geometry is filled, stroked, or filled and stroked.

### Return Value

One of: <a href="bmh_SkPaint?cl=9919#kFill_Style">kFill_Style</a>, <a href="bmh_SkPaint?cl=9919#kStroke_Style">kStroke_Style</a>, <a href="bmh_SkPaint?cl=9919#kStrokeAndFill_Style">kStrokeAndFill_Style</a>.

### Example

<fiddle-embed name="3693fce7864fe4094cb3d3948db8cacf">

#### Example Output

~~~~
SkPaint::kFill_Style == paint.getStyle()
~~~~

</fiddle-embed>

### See Also

<a href="bmh_SkPaint?cl=9919#Style">Style</a> <a href="bmh_SkPaint?cl=9919#setStyle">setStyle</a>

---

<a name="setStyle"></a>

## setStyle

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setStyle(Style style)
</pre>

Sets whether the geometry is filled, stroked, or filled and stroked.

### Parameters

<table>
  <tr>
    <td><code><strong>style</strong></code></td> <td>One of: <a href="bmh_SkPaint?cl=9919#kFill_Style">kFill_Style</a>, <a href="bmh_SkPaint?cl=9919#kStroke_Style">kStroke_Style</a>, <a href="bmh_SkPaint?cl=9919#kStrokeAndFill_Style">kStrokeAndFill_Style</a>.
                  Has no effect if style is not a legal <a href="bmh_SkPaint?cl=9919#Style">Style</a> value.</td>
  </tr>
</table>

### Example

<fiddle-embed name="c7bb6248e4735b8d1a32d02fba40d344">

</fiddle-embed>

### See Also

<a href="bmh_SkPaint?cl=9919#Style">Style</a> <a href="bmh_SkPaint?cl=9919#getStyle">getStyle</a>

---

### See Also

<a href="bmh_undocumented?cl=9919#Fill_Type">Path_Fill_Type</a> <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> <a href="bmh_SkPaint?cl=9919#Style_Fill">Style_Fill</a> <a href="bmh_SkPaint?cl=9919#Style_Stroke">Style_Stroke</a>

# <a name="Stroke_Width"></a> Stroke Width
<a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> sets the width for stroking. The width is the thickness
of the stroke perpendicular to the path's direction when the paint's style is 
set to <a href="bmh_SkPaint?cl=9919#kStroke_Style">kStroke_Style</a> or <a href="bmh_SkPaint?cl=9919#kStrokeAndFill_Style">kStrokeAndFill_Style</a>.
When width is greater than zero, the stroke encompasses as many pixels partially
or fully as needed. When the width equals zero, the paint enables hairlines;
the stroke is always one pixel wide. 
The stroke's dimensions are scaled by the canvas matrix, but <a href="bmh_SkPaint?cl=9919#Hairline">Hairline</a> stroke
remains one pixel wide regardless of scaling.
The default width for the paint is zero.

### Example

<fiddle-embed name="3832b070eefd62afff5fe375baa86445">

raster gpuThe pixels hit to represent thin lines vary with the angle of the 
line and the platform's implementation.

</fiddle-embed>

<a name="getStrokeWidth"></a>

## getStrokeWidth

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkScalar getStrokeWidth()  const
</pre>

Returns the thickness of the pen used by <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> to
outline the shape.

### Return Value

Zero for <a href="bmh_SkPaint?cl=9919#Hairline">Hairline</a>, greater than zero for pen thickness.

### Example

<fiddle-embed name="983efec8708cec5d0e9bd12d8b950c55">

#### Example Output

~~~~
0 == paint.getStrokeWidth()
~~~~

</fiddle-embed>

---

<a name="setStrokeWidth"></a>

## setStrokeWidth

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setStrokeWidth(SkScalar width)
</pre>

Sets the thickness of the pen used by the paint to
outline the shape. 

### Parameters

<table>
  <tr>
    <td><code><strong>width</strong></code></td> <td>Zero thickness for <a href="bmh_SkPaint?cl=9919#Hairline">Hairline</a>; greater than zero for pen thickness. 
                  Has no effect if less than zero.</td>
  </tr>
</table>

### Example

<fiddle-embed name="4387d63c69fb469fb66668cc15719165">

#### Example Output

~~~~
5 == paint.getStrokeWidth()
~~~~

</fiddle-embed>

---

# <a name="Miter_Limit"></a> Miter Limit
<a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a> specifies the maximum miter length,
relative to the stroke width.
<a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a> is used when the <a href="bmh_SkPaint?cl=9919#Stroke_Join">Stroke_Join</a>
is set to <a href="bmh_SkPaint?cl=9919#kMiter_Join">kMiter_Join</a>, and the <a href="bmh_SkPaint?cl=9919#Style">Style</a> is either <a href="bmh_SkPaint?cl=9919#kStroke_Style">kStroke_Style</a>
or <a href="bmh_SkPaint?cl=9919#kStrokeAndFill_Style">kStrokeAndFill_Style</a>.
If the miter at a corner exceeds this limit, <a href="bmh_SkPaint?cl=9919#kMiter_Join">kMiter_Join</a>
is replaced with <a href="bmh_SkPaint?cl=9919#kBevel_Join">kBevel_Join</a>.
<a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a> can be computed from the corner angle:
miter limit = 1 / sin ( angle / 2 )<a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a> default value is 4.
The default may be changed at compile time by setting <a href="bmh_undocumented?cl=9919#SkPaintDefaults_MiterLimit">SkPaintDefaults_MiterLimit</a>
in <a href="bmh_undocumented?cl=9919#SkUserConfig.h">SkUserConfig.h</a> or as a define supplied by the build environment.
Here are some miter limits and the angles that triggers them.

| miter limit | angle in degrees |
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

<fiddle-embed name="00badb5eeabf35289325f0754c476f40">

This example draws a stroked corner and the miter length beneath.
When the miter limit is decreased slightly, the miter join is replaced
by a bevel join.

</fiddle-embed>

<a name="getStrokeMiter"></a>

## getStrokeMiter

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkScalar getStrokeMiter()  const
</pre>

The limit at which a sharp corner is drawn beveled.

### Return Value

<a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a>.

### Example

<fiddle-embed name="f5715648fe49fe6152ed7753ba41ec1a">

#### Example Output

~~~~
default miter limit == 4
~~~~

</fiddle-embed>

### See Also

<a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a> <a href="bmh_SkPaint?cl=9919#setStrokeMiter">setStrokeMiter</a> <a href="bmh_SkPaint?cl=9919#Join">Join</a>

---

<a name="setStrokeMiter"></a>

## setStrokeMiter

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setStrokeMiter(SkScalar miter)
</pre>

The limit at which a sharp corner is drawn beveled.

### Parameters

<table>
  <tr>
    <td><code><strong>miter</strong></code></td> <td>Valid values are zero and greater.
                   Has no effect if miter is less than zero.</td>
  </tr>
</table>

### Example

<fiddle-embed name="f8b9bc2c138950c080bed6ad1a820f44">

#### Example Output

~~~~
default miter limit == 8
~~~~

</fiddle-embed>

### See Also

<a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a> <a href="bmh_SkPaint?cl=9919#getStrokeMiter">getStrokeMiter</a> <a href="bmh_SkPaint?cl=9919#Join">Join</a>

---

# <a name="Stroke_Cap"></a> Stroke Cap
<a href="bmh_SkPaint?cl=9919#Stroke_Cap">Stroke_Cap</a> draws at the beginning and end of an open <a href="bmh_undocumented?cl=9919#Contour">Path_Contour</a>.

## <a name="SkPaint::Cap"></a> Enum SkPaint::Cap

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint?cl=9919#Cap">Cap</a> {
    <a href="bmh_SkPaint?cl=9919#kButt_Cap">kButt_Cap</a>,
    <a href="bmh_SkPaint?cl=9919#kRound_Cap">kRound_Cap</a>,
    <a href="bmh_SkPaint?cl=9919#kSquare_Cap">kSquare_Cap</a>,
    <a href="bmh_SkPaint?cl=9919#kLast_Cap">kLast_Cap</a> = <a href="bmh_SkPaint?cl=9919#kSquare_Cap">kSquare_Cap</a>,
    <a href="bmh_SkPaint?cl=9919#kDefault_Cap">kDefault_Cap</a> = <a href="bmh_SkPaint?cl=9919#kButt_Cap">kButt_Cap</a>
};
static constexpr int <a href="bmh_SkPaint?cl=9919#kCapCount">kCapCount</a> = <a href="bmh_SkPaint?cl=9919#kLast_Cap">kLast_Cap</a> + 1;</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kButt_Cap"></a> <code><strong>SkPaint::kButt::Cap</strong></code></td><td>0</td><td><a href="bmh_SkPaint?cl=9919#kButt_Cap">kButt_Cap</a> does not extend the stroke past the beginning or the end.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kRound_Cap"></a> <code><strong>SkPaint::kRound::Cap</strong></code></td><td>1</td><td><a href="bmh_SkPaint?cl=9919#kRound_Cap">kRound_Cap</a> adds a circle with a diameter equal to <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> at the beginning
and end.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kSquare_Cap"></a> <code><strong>SkPaint::kSquare::Cap</strong></code></td><td>2</td><td><a href="bmh_SkPaint?cl=9919#kSquare_Cap">kSquare_Cap</a> adds a square with sides equal to <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> at the beginning
and end. The square sides are parallel to the initial and final direction
of the stroke.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kLast_Cap"></a> <code><strong>SkPaint::kLast::Cap</strong></code></td><td>2</td><td><a href="bmh_SkPaint?cl=9919#kLast_Cap">kLast_Cap</a> is equivalent to the largest value for <a href="bmh_SkPaint?cl=9919#Stroke_Cap">Stroke_Cap</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kDefault_Cap"></a> <code><strong>SkPaint::kDefault::Cap</strong></code></td><td>0</td><td><a href="bmh_SkPaint?cl=9919#kDefault_Cap">kDefault_Cap</a> is equivalent to <a href="bmh_SkPaint?cl=9919#kButt_Cap">kButt_Cap</a>.
<a href="bmh_SkPaint?cl=9919#Stroke_Cap">Stroke_Cap</a> is set to <a href="bmh_SkPaint?cl=9919#kButt_Cap">kButt_Cap</a> by default.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kCapCount"></a> <code><strong>SkPaint::kCapCount</strong></code></td><td>3</td><td><a href="bmh_SkPaint?cl=9919#kCapCount">kCapCount</a> is the number of different <a href="bmh_SkPaint?cl=9919#Stroke_Cap">Stroke_Cap</a> values defined.
<a href="bmh_SkPaint?cl=9919#kCapCount">kCapCount</a> may be used to verify that <a href="bmh_SkPaint?cl=9919#Stroke_Cap">Stroke_Cap</a> is a legal value.</td>
  </tr>
Stroke describes the area covered by a pen of <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> as it 
follows the <a href="bmh_undocumented?cl=9919#Contour">Path_Contour</a>, moving parallel to the contours's direction.
If the <a href="bmh_undocumented?cl=9919#Contour">Path_Contour</a> is not terminated by <a href="bmh_undocumented?cl=9919#kClose_Verb">SkPath::kClose_Verb</a>, the contour has a
visible beginning and end.
<a href="bmh_undocumented?cl=9919#Contour">Path_Contour</a> may start and end at the same point; defining <a href="bmh_undocumented?cl=9919#Zero_Length">Zero_Length_Contour</a>.
<a href="bmh_SkPaint?cl=9919#kButt_Cap">kButt_Cap</a> and <a href="bmh_undocumented?cl=9919#Zero_Length">Zero_Length_Contour</a> is not drawn.
<a href="bmh_SkPaint?cl=9919#kRound_Cap">kRound_Cap</a> and <a href="bmh_undocumented?cl=9919#Zero_Length">Zero_Length_Contour</a> draws a circle of diameter <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> 
at the contour point.
<a href="bmh_SkPaint?cl=9919#kSquare_Cap">kSquare_Cap</a> and <a href="bmh_undocumented?cl=9919#Zero_Length">Zero_Length_Contour</a> draws an upright square with a side of
<a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> at the contour point.
<a href="bmh_SkPaint?cl=9919#Stroke_Cap">Stroke_Cap</a> is <a href="bmh_SkPaint?cl=9919#kButt_Cap">kButt_Cap</a> by default.
</table>

### Example

<fiddle-embed name="0cab4c079c48e3c277637647d46ce88a">

</fiddle-embed>

<a name="getStrokeCap"></a>

## getStrokeCap

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
Cap getStrokeCap()  const
</pre>

The geometry drawn at the beginning and end of strokes.

### Return Value

One of: <a href="bmh_SkPaint?cl=9919#kButt_Cap">kButt_Cap</a>, <a href="bmh_SkPaint?cl=9919#kRound_Cap">kRound_Cap</a>, <a href="bmh_SkPaint?cl=9919#kSquare_Cap">kSquare_Cap</a>.

### Example

<fiddle-embed name="753c12ecfc325c7e860ae13a0f045e60">

#### Example Output

~~~~
kButt_Cap == default stroke cap
~~~~

</fiddle-embed>

### See Also

<a href="bmh_SkPaint?cl=9919#Stroke_Cap">Stroke_Cap</a> <a href="bmh_SkPaint?cl=9919#setStrokeCap">setStrokeCap</a>

---

<a name="setStrokeCap"></a>

## setStrokeCap

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setStrokeCap(Cap cap)
</pre>

The geometry drawn at the beginning and end of strokes.

### Parameters

<table>
  <tr>
    <td><code><strong>cap</strong></code></td> <td>One of: <a href="bmh_SkPaint?cl=9919#kButt_Cap">kButt_Cap</a>, <a href="bmh_SkPaint?cl=9919#kRound_Cap">kRound_Cap</a>, <a href="bmh_SkPaint?cl=9919#kSquare_Cap">kSquare_Cap</a>.
                Has no effect if cap is not valid.</td>
  </tr>
</table>

### Example

<fiddle-embed name="88b79a48aaab2990093c057deffb05c6">

#### Example Output

~~~~
kRound_Cap == paint.getStrokeCap()
~~~~

</fiddle-embed>

### See Also

<a href="bmh_SkPaint?cl=9919#Stroke_Cap">Stroke_Cap</a> <a href="bmh_SkPaint?cl=9919#getStrokeCap">getStrokeCap</a>

---

# <a name="Stroke_Join"></a> Stroke Join
<a href="bmh_SkPaint?cl=9919#Stroke_Join">Stroke_Join</a> draws at the sharp corners of an open or closed <a href="bmh_undocumented?cl=9919#Contour">Path_Contour</a>.
Stroke describes the area covered by a pen of <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> as it 
follows the <a href="bmh_undocumented?cl=9919#Contour">Path_Contour</a>, moving parallel to the contours's direction.
If the contour direction changes abruptly, because the tangent direction leading
to the end of a curve within the contour does not match the tangent direction of
the following curve, the pair of curves meet at <a href="bmh_SkPaint?cl=9919#Stroke_Join">Stroke_Join</a>.

### Example

<fiddle-embed name="620b563d74734c49cc08e6fe07c01090">

</fiddle-embed>

## <a name="SkPaint::Join"></a> Enum SkPaint::Join

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint?cl=9919#Join">Join</a> {
    <a href="bmh_SkPaint?cl=9919#kMiter_Join">kMiter_Join</a>,
    <a href="bmh_SkPaint?cl=9919#kRound_Join">kRound_Join</a>,
    <a href="bmh_SkPaint?cl=9919#kBevel_Join">kBevel_Join</a>,
    <a href="bmh_SkPaint?cl=9919#kLast_Join">kLast_Join</a> = <a href="bmh_SkPaint?cl=9919#kBevel_Join">kBevel_Join</a>,
    <a href="bmh_SkPaint?cl=9919#kDefault_Join">kDefault_Join</a> = <a href="bmh_SkPaint?cl=9919#kMiter_Join">kMiter_Join</a>
};
static constexpr int <a href="bmh_SkPaint?cl=9919#kJoinCount">kJoinCount</a> = <a href="bmh_SkPaint?cl=9919#kLast_Join">kLast_Join</a> + 1;</pre>
<a href="bmh_SkPaint?cl=9919#Join">Join</a> specifies how corners are drawn when a shape is stroked. The paint's <a href="bmh_SkPaint?cl=9919#Join">Join</a> setting
affects the four corners of a stroked rectangle, and the connected segments in a stroked path.
Choose miter join to draw sharp corners. Choose round join to draw a circle with a
radius equal to the stroke width on top of the corner. Choose bevel join to minimally connect
the thick strokes.
The fill path constructed to describe the stroked path respects the join setting but may 
not contain the actual join. For instance, a fill path constructed with round joins does
not necessarily include circles at each connected segment.

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kMiter_Join"></a> <code><strong>SkPaint::kMiter::Join</strong></code></td><td>0</td><td><a href="bmh_SkPaint?cl=9919#kMiter_Join">kMiter_Join</a> extends the outside of the to the extent allowed by <a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a>.
If the extension exceeds <a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a>, <a href="bmh_SkPaint?cl=9919#kBevel_Join">kBevel_Join</a> is used instead.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kRound_Join"></a> <code><strong>SkPaint::kRound::Join</strong></code></td><td>1</td><td><a href="bmh_SkPaint?cl=9919#kRound_Join">kRound_Join</a> adds a circle with a diameter of <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> at the sharp corner.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kBevel_Join"></a> <code><strong>SkPaint::kBevel::Join</strong></code></td><td>2</td><td><a href="bmh_SkPaint?cl=9919#kBevel_Join">kBevel_Join</a> connects the outside edges of the sharp corner.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kLast_Join"></a> <code><strong>SkPaint::kLast::Join</strong></code></td><td>2</td><td><a href="bmh_SkPaint?cl=9919#kLast_Join">kLast_Join</a> is equivalent to the largest value for <a href="bmh_SkPaint?cl=9919#Stroke_Join">Stroke_Join</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kDefault_Join"></a> <code><strong>SkPaint::kDefault::Join</strong></code></td><td>1</td><td><a href="bmh_SkPaint?cl=9919#kDefault_Join">kDefault_Join</a> is equivalent to <a href="bmh_SkPaint?cl=9919#kMiter_Join">kMiter_Join</a>.
<a href="bmh_SkPaint?cl=9919#Stroke_Join">Stroke_Join</a> is set to <a href="bmh_SkPaint?cl=9919#kMiter_Join">kMiter_Join</a> by default.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kJoinCount"></a> <code><strong>SkPaint::kJoinCount</strong></code></td><td>3</td><td><a href="bmh_SkPaint?cl=9919#kJoinCount">kJoinCount</a> is the number of different <a href="bmh_SkPaint?cl=9919#Stroke_Join">Stroke_Join</a> values defined.
<a href="bmh_SkPaint?cl=9919#kJoinCount">kJoinCount</a> may be used to verify that <a href="bmh_SkPaint?cl=9919#Stroke_Join">Stroke_Join</a> is a legal value.</td>
  </tr>
</table>

### Example

<fiddle-embed name="93eb98f36614f1159645cf252fe9d3a5">

</fiddle-embed>

### See Also

<a href="bmh_SkPaint?cl=9919#setStrokeJoin">setStrokeJoin</a> <a href="bmh_SkPaint?cl=9919#getStrokeJoin">getStrokeJoin</a> <a href="bmh_SkPaint?cl=9919#setStrokeMiter">setStrokeMiter</a> <a href="bmh_SkPaint?cl=9919#getStrokeMiter">getStrokeMiter</a>

<a name="getStrokeJoin"></a>

## getStrokeJoin

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
Join getStrokeJoin()  const
</pre>

The geometry drawn at the corners of strokes. 

### Return Value

One of: <a href="bmh_SkPaint?cl=9919#kMiter_Join">kMiter_Join</a>, <a href="bmh_SkPaint?cl=9919#kRound_Join">kRound_Join</a>, <a href="bmh_SkPaint?cl=9919#kBevel_Join">kBevel_Join</a>.

### Example

<fiddle-embed name="841a73f24e10c6678630c179506c0595">

#### Example Output

~~~~
kMiter_Join == default stroke join
~~~~

</fiddle-embed>

### See Also

<a href="bmh_SkPaint?cl=9919#Stroke_Join">Stroke_Join</a> <a href="bmh_SkPaint?cl=9919#setStrokeJoin">setStrokeJoin</a>

---

<a name="setStrokeJoin"></a>

## setStrokeJoin

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setStrokeJoin(Join join)
</pre>

The geometry drawn at the corners of strokes. 

### Parameters

<table>
  <tr>
    <td><code><strong>join</strong></code></td> <td>One of: <a href="bmh_SkPaint?cl=9919#kMiter_Join">kMiter_Join</a>, <a href="bmh_SkPaint?cl=9919#kRound_Join">kRound_Join</a>, <a href="bmh_SkPaint?cl=9919#kBevel_Join">kBevel_Join</a>.
                 If join is not a valid, <a href="bmh_SkPaint?cl=9919#setStrokeJoin">setStrokeJoin</a> has no effect.</td>
  </tr>
</table>

### Example

<fiddle-embed name="693707e991e819a54baa7a3a0404ecee">

#### Example Output

~~~~
kMiter_Join == paint.getStrokeJoin()
~~~~

</fiddle-embed>

### See Also

<a href="bmh_SkPaint?cl=9919#Stroke_Join">Stroke_Join</a> <a href="bmh_SkPaint?cl=9919#getStrokeJoin">getStrokeJoin</a>

---

### See Also

<a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a>

# <a name="Fill_Path"></a> Fill Path
<a href="bmh_SkPaint?cl=9919#Fill_Path">Fill_Path</a> creates a <a href="bmh_undocumented?cl=9919#Path">Path</a> by applying the <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, followed by the <a href="bmh_SkPaint?cl=9919#Style_Stroke">Style_Stroke</a>.
If <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> contains <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> operates on the source <a href="bmh_undocumented?cl=9919#Path">Path</a>; the result
replaces the destination <a href="bmh_undocumented?cl=9919#Path">Path</a>. Otherwise, the source <a href="bmh_undocumented?cl=9919#Path">Path</a> is replaces the
destination <a href="bmh_undocumented?cl=9919#Path">Path</a>.
Fill <a href="bmh_undocumented?cl=9919#Path">Path</a> can request the <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> to restrict to a culling rectangle, but
the <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> is not required to do so.
If <a href="bmh_SkPaint?cl=9919#Style">Style</a> is <a href="bmh_SkPaint?cl=9919#kStroke_Style">kStroke_Style</a> or <a href="bmh_SkPaint?cl=9919#kStrokeAndFill_Style">kStrokeAndFill_Style</a>, 
and <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> is greater than zero, the <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a>, <a href="bmh_SkPaint?cl=9919#Stroke_Cap">Stroke_Cap</a>, <a href="bmh_SkPaint?cl=9919#Stroke_Join">Stroke_Join</a>,
and <a href="bmh_SkPaint?cl=9919#Miter_Limit">Miter_Limit</a> operate on the destination <a href="bmh_undocumented?cl=9919#Path">Path</a>, replacing it.
Fill <a href="bmh_undocumented?cl=9919#Path">Path</a> can specify the precision used by <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> to approximate the stroke geometry. 
If the <a href="bmh_SkPaint?cl=9919#Style">Style</a> is <a href="bmh_SkPaint?cl=9919#kStroke_Style">kStroke_Style</a> and the <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> is zero, <a href="bmh_SkPaint?cl=9919#getFillPath">getFillPath</a>
returns false since <a href="bmh_SkPaint?cl=9919#Hairline">Hairline</a> has no filled equivalent.

<a name="getFillPath"></a>

## getFillPath

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool getFillPath(const SkPath& src, SkPath* dst, const SkRect* cullRect,
                 SkScalar resScale = 1)
</pre>

The filled equivalent of the stroked path.

### Parameters

<table>
  <tr>
    <td><code><strong>src</strong></code></td> <td><a href="bmh_undocumented?cl=9919#Path">Path</a> read to create a filled version.</td>
  </tr>
  <tr>
    <td><code><strong>dst</strong></code></td> <td>The resulting <a href="bmh_undocumented?cl=9919#Path">Path</a> dst may be the same as src, but may not be nullptr.</td>
  </tr>
  <tr>
    <td><code><strong>cullRect</strong></code></td> <td>Optional limit passed to <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>.</td>
  </tr>
  <tr>
    <td><code><strong>resScale</strong></code></td> <td>If > 1, increase precision, else if (0 < res < 1) reduce precision
                    in favor of speed/size.</td>
  </tr>
</table>

### Return Value

true if the path represents <a href="bmh_SkPaint?cl=9919#Style_Fill">Style_Fill</a>, or false if it represents <a href="bmh_SkPaint?cl=9919#Hairline">Hairline</a>.

### Example

<fiddle-embed name="53da2db22c88f891a990f12de286bce3">

A very small quad stroke is turned into a filled path with increasing levels of precision.
At the lowest precision, the quad stroke is approximated by a rectangle. 
At the highest precision, the filled path has high fidelity compared to the original stroke.

</fiddle-embed>

---

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool getFillPath(const SkPath& src, SkPath* dst)  const
</pre>

The filled equivalent of the stroked path.
Replaces dst with the src path modified by <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> and <a href="bmh_SkPaint?cl=9919#Style_Stroke">Style_Stroke</a>.
<a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, if any, is not culled. <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> is created with default precision.

### Parameters

<table>
  <tr>
    <td><code><strong>src</strong></code></td> <td><a href="bmh_undocumented?cl=9919#Path">Path</a> read to create a filled version.</td>
  </tr>
  <tr>
    <td><code><strong>dst</strong></code></td> <td>The resulting <a href="bmh_undocumented?cl=9919#Path">Path</a> dst may be the same as src, but may not be nullptr.</td>
  </tr>
</table>

### Return Value

true if the path represents <a href="bmh_SkPaint?cl=9919#Style_Fill">Style_Fill</a>, or false if it represents <a href="bmh_SkPaint?cl=9919#Hairline">Hairline</a>.

### Example

<fiddle-embed name="095d99da76d587b015f7d3b675efc7b8">

</fiddle-embed>

---

### See Also

<a href="bmh_SkPaint?cl=9919#Style_Stroke">Style_Stroke</a> <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a> <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>

# <a name="Shader_Methods"></a> Shader Methods
<a href="bmh_undocumented?cl=9919#Shader">Shader</a> defines the colors used when drawing a shape.
<a href="bmh_undocumented?cl=9919#Shader">Shader</a> may be an image, a gradient, or a computed fill.
If <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> has no <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, then <a href="bmh_undocumented?cl=9919#Color">Color</a> fills the shape. 
<a href="bmh_undocumented?cl=9919#Shader">Shader</a> is modulated by <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> component of <a href="bmh_undocumented?cl=9919#Color">Color</a>.
If <a href="bmh_undocumented?cl=9919#Shader">Shader</a> object defines only <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a>, then <a href="bmh_undocumented?cl=9919#Color">Color</a> modulated by <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> describes
the fill.
The drawn transparency can be modified without altering <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, by changing <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a>.

### Example

<fiddle-embed name="c015dc2010c15e1c00b4f7330232b0f7">

</fiddle-embed>

If <a href="bmh_undocumented?cl=9919#Shader">Shader</a> generates only <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> then all components of <a href="bmh_undocumented?cl=9919#Color">Color</a> modulate the output.

### Example

<fiddle-embed name="f45b9765dd06d996921ffa1ef8e78d8a">

</fiddle-embed>

<a name="getShader"></a>

## getShader

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkShader* getShader()  const
</pre>

Optional colors used when filling a path, such as a gradient.
Does not alter <a href="bmh_undocumented?cl=9919#Shader">Shader</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Shader">Shader</a> if previously set, nullptr otherwise.

### Example

<fiddle-embed name="09f15b9fd88882850da2d235eb86292f">

#### Example Output

~~~~
nullptr == shader
nullptr != shader
~~~~

</fiddle-embed>

---

<a name="refShader"></a>

## refShader

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
sk_sp<SkShader> refShader()  const
</pre>

Optional colors used when filling a path, such as a gradient.
Increases <a href="bmh_undocumented?cl=9919#Shader">Shader</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> by one.

### Return Value

<a href="bmh_undocumented?cl=9919#Shader">Shader</a> if previously set, nullptr otherwise.

### Example

<fiddle-embed name="53da0295972a418cbc9607bbb17feaa8">

#### Example Output

~~~~
shader unique: true
shader unique: false
~~~~

</fiddle-embed>

---

<a name="setShader"></a>

## setShader

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setShader(sk_sp<SkShader> shader)
</pre>

Optional colors used when filling a path, such as a gradient.
Sets <a href="bmh_undocumented?cl=9919#Shader">Shader</a> to shader, decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> of the previous <a href="bmh_undocumented?cl=9919#Shader">Shader</a>.
Does not alter shader <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>shader</strong></code></td> <td>Pass nullptr to use <a href="bmh_undocumented?cl=9919#Color">Color</a> instead of <a href="bmh_undocumented?cl=9919#Shader">Shader</a> to fill.</td>
  </tr>
</table>

### Example

<fiddle-embed name="92996333200d7c0b2158927f7e8439c5">

</fiddle-embed>

---

# <a name="Color_Filter_Methods"></a> Color Filter Methods
<a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a> alters the color used when drawing a shape.
<a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a> may apply <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a>, transform the color through a matrix, or composite multiple filters.
If <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> has no <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>, the color is unaltered.
The drawn transparency can be modified without altering <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>, by changing <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a>.

### Example

<fiddle-embed name="3343ea21abaf9778d35e9953f0a6be10">

</fiddle-embed>

<a name="getColorFilter"></a>

## getColorFilter

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkColorFilter* getColorFilter()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getColorFilter">getColorFilter</a> returns <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#getColorFilter">getColorFilter</a> does not alter <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a> if previously set, nullptr otherwise.

### Example

<fiddle-embed name="093bdc627d6b59002670fd290931f6c9">

#### Example Output

~~~~
nullptr == color filter
nullptr != color filter
~~~~

</fiddle-embed>

---

<a name="refColorFilter"></a>

## refColorFilter

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
sk_sp<SkColorFilter> refColorFilter()  const
</pre>

<a href="bmh_SkPaint?cl=9919#refColorFilter">refColorFilter</a> returns <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#refColorFilter">refColorFilter</a> increases <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> by one.

### Example

<fiddle-embed name="be0095f54d59487ef2d2e87457910bc9">

#### Example Output

~~~~
color filter unique: true
color filter unique: false
~~~~

</fiddle-embed>

---

<a name="setColorFilter"></a>

## setColorFilter

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setColorFilter(sk_sp<SkColorFilter> filter)
</pre>

<a href="bmh_SkPaint?cl=9919#setColorFilter">setColorFilter</a> sets <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a> to filter, decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> of the previous <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>. 
Pass nullptr to clear <a href="bmh_undocumented?cl=9919#Color_Filter">Color_Filter</a>.
<a href="bmh_SkPaint?cl=9919#setColorFilter">setColorFilter</a> does not alter filter <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Example

<fiddle-embed name="01e7486180e7d3ecf9a6501e6afeb717">

</fiddle-embed>

---

# <a name="Blend_Mode_Methods"></a> Blend Mode Methods
<a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a> describes how <a href="bmh_undocumented?cl=9919#Color">Color</a> combines with the destination color.
The default setting, <a href="bmh_undocumented?cl=9919#kSrcOver">SkBlendMode::kSrcOver</a>, draws the source color
over the destination color.

### Example

<fiddle-embed name="73092d4d06faecea3c204d852a4dd8a8">

</fiddle-embed>

### See Also

<a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a>

<a name="getBlendMode"></a>

## getBlendMode

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkBlendMode getBlendMode()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getBlendMode">getBlendMode</a> returns <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a>.
By default, <a href="bmh_SkPaint?cl=9919#getBlendMode">getBlendMode</a> returns <a href="bmh_undocumented?cl=9919#kSrcOver">SkBlendMode::kSrcOver</a>.

### Example

<fiddle-embed name="4ec1864b8203d52c0810e8605092f45c">

#### Example Output

~~~~
kSrcOver == getBlendMode
kSrcOver != getBlendMode
~~~~

</fiddle-embed>

---

<a name="isSrcOver"></a>

## isSrcOver

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool isSrcOver()  const
</pre>

<a href="bmh_SkPaint?cl=9919#isSrcOver">isSrcOver</a> returns true if <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a> is <a href="bmh_undocumented?cl=9919#kSrcOver">SkBlendMode::kSrcOver</a>, the default.

### Example

<fiddle-embed name="257c9473db7a2b3a0fb2b9e2431e59a6">

#### Example Output

~~~~
isSrcOver == true
isSrcOver != true
~~~~

</fiddle-embed>

---

<a name="setBlendMode"></a>

## setBlendMode

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setBlendMode(SkBlendMode mode)
</pre>

<a href="bmh_SkPaint?cl=9919#setBlendMode">setBlendMode</a> sets <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a> to mode. 
<a href="bmh_SkPaint?cl=9919#setBlendMode">setBlendMode</a> does not check for valid input.

### Parameters

<table>
  <tr>
    <td><code><strong>mode</strong></code></td> <td><a href="bmh_undocumented?cl=9919#SkBlendMode">SkBlendMode</a> used to combine source color and destination.</td>
  </tr>
</table>

### Example

<fiddle-embed name="257c9473db7a2b3a0fb2b9e2431e59a6">

#### Example Output

~~~~
isSrcOver == true
isSrcOver != true
~~~~

</fiddle-embed>

---

# <a name="Path_Effect_Methods"></a> Path Effect Methods
<a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> modifies the path geometry before drawing it.
<a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> may implement dashing, custom fill effects and custom stroke effects.
If <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> has no <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>, the path geometry is unaltered when filled or stroked.

### Example

<fiddle-embed name="5daa7aebf14b7e224d0e1e98798e5e7d">

</fiddle-embed>

### See Also

<a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>

<a name="getPathEffect"></a>

## getPathEffect

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkPathEffect* getPathEffect()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getPathEffect">getPathEffect</a> returns <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#getPathEffect">getPathEffect</a> does not alter <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> if previously set, nullptr otherwise.

### Example

<fiddle-embed name="211a1b14bfa6c4332082c8eab4fbc5fd">

#### Example Output

~~~~
nullptr == path effect
nullptr != path effect
~~~~

</fiddle-embed>

---

<a name="refPathEffect"></a>

## refPathEffect

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
sk_sp<SkPathEffect> refPathEffect()  const
</pre>

<a href="bmh_SkPaint?cl=9919#refPathEffect">refPathEffect</a> returns <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#refPathEffect">refPathEffect</a> increases <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> by one.

### Example

<fiddle-embed name="c24ff92a980a5988697f6ae910560e21">

#### Example Output

~~~~
path effect unique: true
path effect unique: false
~~~~

</fiddle-embed>

---

<a name="setPathEffect"></a>

## setPathEffect

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setPathEffect(sk_sp<SkPathEffect> pathEffect)
</pre>

<a href="bmh_SkPaint?cl=9919#setPathEffect">setPathEffect</a> sets <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> to pathEffect, 
decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> of the previous <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>. 
Pass nullptr to leave the path geometry unaltered.
<a href="bmh_SkPaint?cl=9919#setPathEffect">setPathEffect</a> does not alter pathEffect <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Example

<fiddle-embed name="52dd55074ca0b7d520d04e750ca2a0d7">

</fiddle-embed>

---

# <a name="Mask_Filter_Methods"></a> Mask Filter Methods
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> uses <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> of the shape drawn to create <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask_Alpha</a>.
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> operates at a lower level than <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>; <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> takes a <a href="bmh_undocumented?cl=9919#Mask">Mask</a>,
and returns a <a href="bmh_undocumented?cl=9919#Mask">Mask</a>.
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> may change the geometry and transparency of the shape, such as creating a blur effect.
Set <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> to nullptr to prevent <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> from modifying the draw.

### Example

<fiddle-embed name="320b04ea1e1291d49f1e61994a0410fe">

</fiddle-embed>

<a name="getMaskFilter"></a>

## getMaskFilter

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkMaskFilter* getMaskFilter()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getMaskFilter">getMaskFilter</a> returns <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#getMaskFilter">getMaskFilter</a> does not alter <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> if previously set, nullptr otherwise.

### Example

<fiddle-embed name="8cd53ece8fc83e4560599ace094b0f16">

#### Example Output

~~~~
nullptr == mask filter
nullptr != mask filter
~~~~

</fiddle-embed>

---

<a name="refMaskFilter"></a>

## refMaskFilter

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
sk_sp<SkMaskFilter> refMaskFilter()  const
</pre>

<a href="bmh_SkPaint?cl=9919#refMaskFilter">refMaskFilter</a> returns <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#refMaskFilter">refMaskFilter</a> increases <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> by one.

### Example

<fiddle-embed name="ddb211ed3d5e226b0bb973de5076de26">

#### Example Output

~~~~
mask filter unique: true
mask filter unique: false
~~~~

</fiddle-embed>

---

<a name="setMaskFilter"></a>

## setMaskFilter

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setMaskFilter(sk_sp<SkMaskFilter> maskFilter)
</pre>

<a href="bmh_SkPaint?cl=9919#setMaskFilter">setMaskFilter</a> sets <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> to maskFilter,
decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> of the previous <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>. 
Pass nullptr to clear <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> and leave <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> effect on <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask_Alpha</a> unaltered.
<a href="bmh_SkPaint?cl=9919#setMaskFilter">setMaskFilter</a> does not affect <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>.
<a href="bmh_SkPaint?cl=9919#setMaskFilter">setMaskFilter</a> does not alter maskFilter <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Example

<fiddle-embed name="62c5a826692f85c3de3bab65e9e97aa9">

</fiddle-embed>

---

# <a name="Typeface_Methods"></a> Typeface Methods
<a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> identifies the font used when drawing and measuring text.
<a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> may be specified by name, from a file, or from a data stream.
The default <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> defers to the platform-specific default font
implementation.

### Example

<fiddle-embed name="bb32c82a23a147f528dfbc2e327a3fa4">

</fiddle-embed>

<a name="getTypeface"></a>

## getTypeface

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkTypeface* getTypeface()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getTypeface">getTypeface</a> returns <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#getTypeface">getTypeface</a> does not alter <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> if previously set, nullptr otherwise.

### Example

<fiddle-embed name="4d9ffb5761b62a9e3bc9b0bca8787bce">

#### Example Output

~~~~
nullptr == typeface
nullptr != typeface
~~~~

</fiddle-embed>

---

<a name="refTypeface"></a>

## refTypeface

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
sk_sp<SkTypeface> refTypeface()  const
</pre>

<a href="bmh_SkPaint?cl=9919#refTypeface">refTypeface</a> returns <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#refTypeface">refTypeface</a> increases <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> by one.

### Example

<fiddle-embed name="c8edce7b36a3ffda8af4fe89d7187dbc">

#### Example Output

~~~~
typeface1 != typeface2
typeface1 == typeface2
~~~~

</fiddle-embed>

---

<a name="setTypeface"></a>

## setTypeface

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setTypeface(sk_sp<SkTypeface> typeface)
</pre>

<a href="bmh_SkPaint?cl=9919#setTypeface">setTypeface</a> sets <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to typeface,
decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> of the previous <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>. 
Pass nullptr to clear <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> and use the default typeface.
<a href="bmh_SkPaint?cl=9919#setTypeface">setTypeface</a> does not alter typeface <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Example

<fiddle-embed name="a67efa7096c9b7ff7a4c59aa80310ce9">

</fiddle-embed>

---

# <a name="Rasterizer_Methods"></a> Rasterizer Methods
<a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> controls how shapes are converted to <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask_Alpha</a>. 
<a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> operates at a higher level than <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>; <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> takes a <a href="bmh_undocumented?cl=9919#Path">Path</a>,
and returns a <a href="bmh_undocumented?cl=9919#Mask">Mask</a>.
<a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> may change the geometry and transparency of the shape, such as
creating a shadow effect. <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> forms the base of <a href="bmh_undocumented?cl=9919#Layer">Rasterizer_Layer</a>, which
creates effects like embossing and outlining.
<a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> applies to <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Region">Region</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round_Rect</a>, <a href="bmh_undocumented?cl=9919#Arc">Arc</a>, <a href="bmh_undocumented?cl=9919#Circle">Circle</a>, <a href="bmh_undocumented?cl=9919#Oval">Oval</a>,
<a href="bmh_undocumented?cl=9919#Path">Path</a>, and <a href="bmh_undocumented?cl=9919#Text">Text</a>.

### Example

<fiddle-embed name="196bbd3000a2988ccc5460398cb32f5a">

</fiddle-embed>

<a name="getRasterizer"></a>

## getRasterizer

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkRasterizer* getRasterizer()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getRasterizer">getRasterizer</a> returns <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#getRasterizer">getRasterizer</a> does not alter <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> if previously set, nullptr otherwise.

### Example

<fiddle-embed name="0707d407c3a14388b107af8ae5873e55">

#### Example Output

~~~~
nullptr == rasterizer
nullptr != rasterizer
~~~~

</fiddle-embed>

---

<a name="refRasterizer"></a>

## refRasterizer

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
sk_sp<SkRasterizer> refRasterizer()  const
</pre>

<a href="bmh_SkPaint?cl=9919#refRasterizer">refRasterizer</a> returns <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#refRasterizer">refRasterizer</a> increases <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> by one.

### Example

<fiddle-embed name="c0855ce19a33cb7e5747750ef341b7b3">

#### Example Output

~~~~
rasterizer unique: true
rasterizer unique: false
~~~~

</fiddle-embed>

---

<a name="setRasterizer"></a>

## setRasterizer

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setRasterizer(sk_sp<SkRasterizer> rasterizer)
</pre>

<a href="bmh_SkPaint?cl=9919#setRasterizer">setRasterizer</a> sets <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> to rasterizer,
decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> of the previous <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>. 
Pass nullptr to clear <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> and leave <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> effect on <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask_Alpha</a> unaltered.
<a href="bmh_SkPaint?cl=9919#setRasterizer">setRasterizer</a> does not affect <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>.
<a href="bmh_SkPaint?cl=9919#setRasterizer">setRasterizer</a> does not alter rasterizer <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Example

<fiddle-embed name="181e7a5c63a8652edcdc64b7b957f8ec">

</fiddle-embed>

---

# <a name="Image_Filter_Methods"></a> Image Filter Methods
<a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> operates on the pixel representation of the shape, as modified by <a href="bmh_SkPaint?cl=9919#Paint">Paint</a>
with <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a> set to <a href="bmh_undocumented?cl=9919#kSrcOver">SkBlendMode::kSrcOver</a>. <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> creates a new bitmap,
which is drawn to the device using the set <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a>.
<a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> is higher level than <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>; for instance, an <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a>
can operate on all channels of <a href="bmh_undocumented?cl=9919#Color">Color</a>, while <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> generates <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> only.
<a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> operates independently of and can be used in combination with
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a> and <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>.

### Example

<fiddle-embed name="4fd19982b82bc27340a507183e39d7d3">

</fiddle-embed>

<a name="getImageFilter"></a>

## getImageFilter

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkImageFilter* getImageFilter()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getImageFilter">getImageFilter</a> returns <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#getImageFilter">getImageFilter</a> does not alter <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> if previously set, nullptr otherwise.

### Example

<fiddle-embed name="38788d42772641606e08c60d9dd418a2">

#### Example Output

~~~~
nullptr == image filter
nullptr != image filter
~~~~

</fiddle-embed>

---

<a name="refImageFilter"></a>

## refImageFilter

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
sk_sp<SkImageFilter> refImageFilter()  const
</pre>

<a href="bmh_SkPaint?cl=9919#refImageFilter">refImageFilter</a> returns <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#refImageFilter">refImageFilter</a> increases <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> by one.

### Example

<fiddle-embed name="0828822500bc17539a248ca16a75ddc7">

#### Example Output

~~~~
image filter unique: true
image filter unique: false
~~~~

</fiddle-embed>

---

<a name="setImageFilter"></a>

## setImageFilter

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setImageFilter(sk_sp<SkImageFilter> imageFilter)
</pre>

<a href="bmh_SkPaint?cl=9919#setImageFilter">setImageFilter</a> sets <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> to imageFilter,
decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> of the previous <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a>. 
Pass nullptr to clear <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a>, and remove <a href="bmh_undocumented?cl=9919#Image_Filter">Image_Filter</a> effect
on drawing.
<a href="bmh_SkPaint?cl=9919#setImageFilter">setImageFilter</a> does not affect <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> or <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask_Filter</a>.
<a href="bmh_SkPaint?cl=9919#setImageFilter">setImageFilter</a> does not alter imageFilter <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Example

<fiddle-embed name="7cac26cde809379c561c3af7af9c74db">

</fiddle-embed>

---

# <a name="Draw_Looper_Methods"></a> Draw Looper Methods
<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> sets a modifier that communicates state from one <a href="bmh_undocumented?cl=9919#Draw_Layer">Draw_Layer</a>
to another to construct the draw.
<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> draws one or more times, modifying the canvas and paint each time.
<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> may be used to draw multiple colors or create a colored shadow.
Set <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> to nullptr to prevent <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> from modifying the draw. 

### Example

<fiddle-embed name="8de8294eee43c40e41c4cf5f15bb2b40">

</fiddle-embed>

<a name="getDrawLooper"></a>

## getDrawLooper

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkDrawLooper* getDrawLooper()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getDrawLooper">getDrawLooper</a> returns <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#getDrawLooper">getDrawLooper</a> does not alter <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> if previously set, nullptr otherwise.

### Example

<fiddle-embed name="af4c5acc7a91e7f23c2af48018903ad4">

#### Example Output

~~~~
nullptr == draw looper
nullptr != draw looper
~~~~

</fiddle-embed>

---

<a name="refDrawLooper"></a>

## refDrawLooper

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
sk_sp<SkDrawLooper> refDrawLooper()  const
</pre>

<a href="bmh_SkPaint?cl=9919#refDrawLooper">refDrawLooper</a> returns <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> if set, or nullptr.
<a href="bmh_SkPaint?cl=9919#refDrawLooper">refDrawLooper</a> increases <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> by one.

### Example

<fiddle-embed name="bd18a131b759ef38033b0c8cf916d922">

#### Example Output

~~~~
draw looper unique: true
draw looper unique: false
~~~~

</fiddle-embed>

---

<a name="getLooper"></a>

## getLooper

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkDrawLooper* getLooper()  const
</pre>

---

<a name="setDrawLooper"></a>

## setDrawLooper

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setDrawLooper(sk_sp<SkDrawLooper> drawLooper)
</pre>

<a href="bmh_SkPaint?cl=9919#setDrawLooper">setDrawLooper</a> sets <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> to drawLooper,
decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a> of the previous drawLooper. 
Pass nullptr to clear <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> and leave <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw_Looper</a> effect on drawing unaltered.
<a href="bmh_SkPaint?cl=9919#setDrawLooper">setDrawLooper</a> does not alter drawLooper <a href="bmh_undocumented?cl=9919#Reference_Count">Reference_Count</a>.

### Example

<fiddle-embed name="b16bcd76b629922f476ce4a4e5aa0584">

</fiddle-embed>

---

<a name="setLooper"></a>

## setLooper

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setLooper(sk_sp<SkDrawLooper>)
</pre>

---

# <a name="Text_Align"></a> Text Align
<a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> adjusts the text relative to the text position.
<a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> affects glyphs drawn with <a href="bmh_SkCanvas?cl=9919#drawText">SkCanvas::drawText</a>, <a href="bmh_SkCanvas?cl=9919#drawPosText">SkCanvas::drawPosText</a>, and
<a href="bmh_SkCanvas?cl=9919#drawPosTextH">SkCanvas::drawPosTextH</a>, as well as calls that place text glyphs like getTextWidthds and <a href="bmh_SkPaint?cl=9919#getTextPath">getTextPath</a>.
The text position is set by the font for both horizontal and vertical text.
Typically, for horizontal text, the position is to the left side of the glyph on the base line;
and for vertical text, the position is the horizontal center at the glyph at the caps height.
<a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> adjusts the glyph position to center it or move it to abut the position 
using the metrics returned by the font.
<a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> defaults to <a href="bmh_SkPaint?cl=9919#kLeft_Align">kLeft_Align</a>.

## <a name="SkPaint::Align"></a> Enum SkPaint::Align

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint?cl=9919#Align">Align</a> {
    <a href="bmh_SkPaint?cl=9919#kLeft_Align">kLeft_Align</a>,
    <a href="bmh_SkPaint?cl=9919#kCenter_Align">kCenter_Align</a>,
    <a href="bmh_SkPaint?cl=9919#kRight_Align">kRight_Align</a>,
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kLeft_Align"></a> <code><strong>SkPaint::kLeft::Align</strong></code></td><td>0</td><td><a href="bmh_SkPaint?cl=9919#kLeft_Align">kLeft_Align</a> leaves the glyph at the position computed by the font offset by the text position.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kCenter_Align"></a> <code><strong>SkPaint::kCenter::Align</strong></code></td><td>1</td><td><a href="bmh_SkPaint?cl=9919#kCenter_Align">kCenter_Align</a> moves the glyph half its width if <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> has <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> clear, and
half its height if <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> has <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> set.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kRight_Align"></a> <code><strong>SkPaint::kRight::Align</strong></code></td><td>2</td><td><a href="bmh_SkPaint?cl=9919#kRight_Align">kRight_Align</a> moves the glyph by its width if <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> has <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> clear,
and by its height if <a href="bmh_SkPaint?cl=9919#Flags">Flags</a> has <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> set.</td>
  </tr>
</table>

## <a name="SkPaint_anonymous_2"></a> Enum SkPaint::anonymous_2

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum {
    <a href="bmh_SkPaint?cl=9919#kAlignCount">kAlignCount</a> = 3
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kAlignCount"></a> <code><strong>SkPaint::kAlignCount</strong></code></td><td>3</td><td><a href="bmh_SkPaint?cl=9919#kAlignCount">kAlignCount</a> is the number of different <a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> values defined.</td>
  </tr>
</table>

### Example

<fiddle-embed name="d317346b0bb75ecef4e6d8d94ad08664">

Each position separately moves the glyph in drawPosText.

</fiddle-embed>

### Example

<fiddle-embed name="e003042766521a607dbbaa653a565709">

<a href="bmh_SkPaint?cl=9919#Vertical_Text">Vertical_Text</a> treats <a href="bmh_SkPaint?cl=9919#kLeft_Align">kLeft_Align</a> as top align, and <a href="bmh_SkPaint?cl=9919#kRight_Align">kRight_Align</a> as bottom align.

</fiddle-embed>

<a name="getTextAlign"></a>

## getTextAlign

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
Align getTextAlign()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getTextAlign">getTextAlign</a> returns <a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a>.
<a href="bmh_SkPaint?cl=9919#getTextAlign">getTextAlign</a> returns <a href="bmh_SkPaint?cl=9919#kLeft_Align">kLeft_Align</a> if <a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> has not been set.

### Example

<fiddle-embed name="70a061ac1c0ce260e1f8ec9f10e37285">

#### Example Output

~~~~
kLeft_Align == default
~~~~

</fiddle-embed>

---

<a name="setTextAlign"></a>

## setTextAlign

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void    setTextAlign(Align align)
</pre>

<a href="bmh_SkPaint?cl=9919#setTextAlign">setTextAlign</a> sets <a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> to align.
<a href="bmh_SkPaint?cl=9919#setTextAlign">setTextAlign</a> has no effect if align is an invalid value.

### Example

<fiddle-embed name="02967ca885ff7b481070253c86493244">

</fiddle-embed>

---

# <a name="Text_Size"></a> Text Size
<a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> adjusts the overall text size in points.
<a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> can be set to any positive value or zero.
<a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> defaults to 12.
Set <a href="bmh_undocumented?cl=9919#SkPaintDefaults_TextSize">SkPaintDefaults_TextSize</a> at compile time to change the default setting.

### Example

<fiddle-embed name="03cdf48119c978083f658fc8bf4385b7">

</fiddle-embed>

<a name="getTextSize"></a>

## getTextSize

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkScalar getTextSize()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getTextSize">getTextSize</a> returns <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> in points.

### Example

<fiddle-embed name="5d9555de745276df36e1087187fd7cf4">

</fiddle-embed>

---

<a name="setTextSize"></a>

## setTextSize

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setTextSize(SkScalar textSize)
</pre>

<a href="bmh_SkPaint?cl=9919#setTextSize">setTextSize</a> sets <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> in points.
<a href="bmh_SkPaint?cl=9919#setTextSize">setTextSize</a> has no effect if textSize is not greater than or equal to zero.
 
### Example

<fiddle-embed name="1cf7805047da9c02d21a447006c3fb09">

</fiddle-embed>

---

# <a name="Text_Scale_X"></a> Text Scale X
<a href="bmh_SkPaint?cl=9919#Text_Scale_X">Text_Scale_X</a> adjusts the text horizontal scale.
<a href="bmh_undocumented?cl=9919#Text">Text</a> scaling approximates condensed and expanded type faces when the actual face
is not available.
<a href="bmh_SkPaint?cl=9919#Text_Scale_X">Text_Scale_X</a> can be set to any value.
<a href="bmh_SkPaint?cl=9919#Text_Scale_X">Text_Scale_X</a> defaults to 1.

### Example

<fiddle-embed name="c7ed964e579009590293249ac9c53b89">

</fiddle-embed>

<a name="getTextScaleX"></a>

## getTextScaleX

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkScalar getTextScaleX()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getTextScaleX">getTextScaleX</a> returns <a href="bmh_SkPaint?cl=9919#Text_Scale_X">Text_Scale_X</a>.

### Example

<fiddle-embed name="0afc0838b3e1279b663b7b721b20dfad">

</fiddle-embed>

---

<a name="setTextScaleX"></a>

## setTextScaleX

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setTextScaleX(SkScalar scaleX)
</pre>

<a href="bmh_SkPaint?cl=9919#setTextScaleX">setTextScaleX</a> sets <a href="bmh_SkPaint?cl=9919#Text_Scale_X">Text_Scale_X</a>.
 
### Example

<fiddle-embed name="60b1221813c58876e7a6922832b371aa">

</fiddle-embed>

---

# <a name="Text_Skew_X"></a> Text Skew X
<a href="bmh_SkPaint?cl=9919#Text_Skew_X">Text_Skew_X</a> adjusts the text horizontal slant.
<a href="bmh_undocumented?cl=9919#Text">Text</a> skewing approximates italic and oblique type faces when the actual face
is not available.
<a href="bmh_SkPaint?cl=9919#Text_Skew_X">Text_Skew_X</a> can be set to any value.
<a href="bmh_SkPaint?cl=9919#Text_Skew_X">Text_Skew_X</a> defaults to 0.

### Example

<fiddle-embed name="b16bfa01960d8d9cd5fa7882fd046ba4">

</fiddle-embed>

<a name="getTextSkewX"></a>

## getTextSkewX

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkScalar getTextSkewX()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getTextSkewX">getTextSkewX</a> returns <a href="bmh_SkPaint?cl=9919#Text_Skew_X">Text_Skew_X</a>.

### Example

<fiddle-embed name="3425475634dab75c0e37bdae44d10cf4">

</fiddle-embed>

---

<a name="setTextSkewX"></a>

## setTextSkewX

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setTextSkewX(SkScalar skewX)
</pre>

<a href="bmh_SkPaint?cl=9919#setTextSkewX">setTextSkewX</a> sets <a href="bmh_SkPaint?cl=9919#Text_Skew_X">Text_Skew_X</a>.

### Example

<fiddle-embed name="e97458d2757bdfa72868f50d7c79ddcc">

</fiddle-embed>

---

<a name="SetTextMatrix"></a>

## SetTextMatrix

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
static SkMatrix* SetTextMatrix(SkMatrix* matrix, SkScalar size, SkScalar scaleX,
                               SkScalar skewX)
</pre>

Returns matrix, setting it to apply size, scaleX, and skewX.
<a href="bmh_SkPaint?cl=9919#SetTextMatrix">SetTextMatrix</a> sets matrix to scale in the direction of the x-axis by size times scaleX,
scale in the direction of the y-axis by size, and skew in the direction of the x-axis by skewX.
matrix is returned as a convenience.

### Example

<fiddle-embed name="46dcc8b7862e89040af77e07f698518e">

#### Example Output

~~~~
[ 12.0000   0.0000   0.0000][  0.0000  12.0000   0.0000][  0.0000   0.0000   1.0000]
~~~~

</fiddle-embed>

---

<a name="setTextMatrix"></a>

## setTextMatrix

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkMatrix* setTextMatrix(SkMatrix* matrix)  const
</pre>

Returns a <a href="bmh_undocumented?cl=9919#Matrix">Matrix</a> that applies <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a>, <a href="bmh_SkPaint?cl=9919#Text_Scale_X">Text_Scale_X</a>, and <a href="bmh_SkPaint?cl=9919#Text_Skew_X">Text_Skew_X</a>.
<a href="bmh_SkPaint?cl=9919#SetTextMatrix">SetTextMatrix</a> sets matrix to scale in the direction of the x-axis by size times scaleX,
scale in the direction of the y-axis by size, and skew in the direction of the x-axis by skewX.
matrix is returned as a convenience.

### Example

<fiddle-embed name="68d6097ab8927bd75d01a1251bf828f1">

#### Example Output

~~~~
[ 12.0000   0.0000   0.0000][  0.0000  12.0000   0.0000][  0.0000   0.0000   1.0000]
~~~~

</fiddle-embed>

---

# <a name="Text_Encoding"></a> Text Encoding
<a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> determines whether text specifies character codes and their encoded size,
or glyph indices. Character codes use the encoding specified by the<a href="bmh_undocumented?cl=9919#Unicode">Unicode</a> standard # http://unicode.org/standard/standard.html. Character codes encoded size
are specified by <a href="bmh_undocumented?cl=9919#UTF_8">UTF-8</a>, <a href="bmh_undocumented?cl=9919#UTF_16">UTF-16</a>, or <a href="bmh_undocumented?cl=9919#UTF_32">UTF-32</a>. All character encoding are able to represent all of
<a href="bmh_undocumented?cl=9919#Unicode">Unicode</a>, differing only in the total storage required.
<a href="bmh_undocumented?cl=9919#UTF_8">UTF-8</a> (<a href="bmh_undocumented?cl=9919#RFC">RFC</a> 3629) # https://tools.ietf.org/html/rfc3629is made up of 8-bit bytes, 
and is a superset of <a href="bmh_undocumented?cl=9919#ASCII">ASCII</a>.
<a href="bmh_undocumented?cl=9919#UTF_16">UTF-16</a> (<a href="bmh_undocumented?cl=9919#RFC">RFC</a> 2781) # https://tools.ietf.org/html/rfc2781is made up of 16-bit words, 
and is a superset of <a href="bmh_undocumented?cl=9919#Unicode">Unicode</a> ranges 0x0000 to 0xD7FF and 0xE000 to 0xFFFF.
<a href="bmh_undocumented?cl=9919#UTF_32">UTF-32</a> # http://www.unicode.org/versions/<a href="bmh_undocumented?cl=9919#Unicode5">Unicode5</a>.0.0/ch03.pdfis made up of 32-bit words,
and is a superset of <a href="bmh_undocumented?cl=9919#Unicode">Unicode</a>.
<a href="bmh_undocumented?cl=9919#Font_Manager">Font_Manager</a> uses font data to convert character codes into glyph indices. 
A glyph index is a 16-bit word.
<a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> is set to <a href="bmh_SkPaint?cl=9919#kUTF8_TextEncoding">kUTF8_TextEncoding</a> by default.

## <a name="SkPaint::TextEncoding"></a> Enum SkPaint::TextEncoding

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint?cl=9919#TextEncoding">TextEncoding</a> {
    <a href="bmh_SkPaint?cl=9919#kUTF8_TextEncoding">kUTF8_TextEncoding</a>,
    <a href="bmh_SkPaint?cl=9919#kUTF16_TextEncoding">kUTF16_TextEncoding</a>,
    <a href="bmh_SkPaint?cl=9919#kUTF32_TextEncoding">kUTF32_TextEncoding</a>,
    <a href="bmh_SkPaint?cl=9919#kGlyphID_TextEncoding">kGlyphID_TextEncoding</a>
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kUTF8_TextEncoding"></a> <code><strong>SkPaint::kUTF8::TextEncoding</strong></code></td><td>0</td><td><a href="bmh_SkPaint?cl=9919#kUTF8_TextEncoding">kUTF8_TextEncoding</a> uses a series of bytes to represent <a href="bmh_undocumented?cl=9919#UTF_8">UTF-8</a> or <a href="bmh_undocumented?cl=9919#ASCII">ASCII</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kUTF16_TextEncoding"></a> <code><strong>SkPaint::kUTF16::TextEncoding</strong></code></td><td>1</td><td><a href="bmh_SkPaint?cl=9919#kUTF16_TextEncoding">kUTF16_TextEncoding</a> uses a series of two byte words to represent most of <a href="bmh_undocumented?cl=9919#Unicode">Unicode</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kUTF32_TextEncoding"></a> <code><strong>SkPaint::kUTF32::TextEncoding</strong></code></td><td>2</td><td><a href="bmh_SkPaint?cl=9919#kUTF32_TextEncoding">kUTF32_TextEncoding</a> uses a series of four byte words to represent all of <a href="bmh_undocumented?cl=9919#Unicode">Unicode</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kGlyphID_TextEncoding"></a> <code><strong>SkPaint::kGlyphID::TextEncoding</strong></code></td><td>3</td><td><a href="bmh_SkPaint?cl=9919#kGlyphID_TextEncoding">kGlyphID_TextEncoding</a> uses a series of two byte words to represent glyph indices.</td>
  </tr>
</table>

### Example

<fiddle-embed name="8570b90dd0d10749c49cd8ff5f846be1">

</fiddle-embed>

<a name="getTextEncoding"></a>

## getTextEncoding

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
TextEncoding getTextEncoding()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getTextEncoding">getTextEncoding</a> returns <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a>.

### Example

<fiddle-embed name="f5277b17a3d17517dee0f54af727238c">

#### Example Output

~~~~
kUTF8_TextEncoding == text encoding
kGlyphID_TextEncoding == text encoding
~~~~

</fiddle-embed>

---

<a name="setTextEncoding"></a>

## setTextEncoding

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void setTextEncoding(TextEncoding encoding)
</pre>

<a href="bmh_SkPaint?cl=9919#setTextEncoding">setTextEncoding</a> sets <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> to encoding.
Invalid values for encoding are ignored.

### Example

<fiddle-embed name="9f34027d218dda8a970ab43484be9c92">

#### Example Output

~~~~
4 != text encoding
~~~~

</fiddle-embed>

---

# <a name="Font_Metrics"></a> Font Metrics
<a href="bmh_SkPaint?cl=9919#Font_Metrics">Font_Metrics</a> describe dimensions common to the glyphs in <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>.
The dimensions are computed by <a href="bmh_undocumented?cl=9919#Font_Manager">Font_Manager</a> from font data and do not take 
<a href="bmh_SkPaint?cl=9919#Paint">Paint</a> settings other than <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> into account.
<a href="bmh_undocumented?cl=9919#Font">Font</a> dimensions specify the anchor to the left of the glyph at baseline as the origin.
X-axis values to the left of the glyph are negative, and to the right of the left glyph edge
are positive.
Y-axis values above the baseline are negative, and below the baseline are positive.
 
### Example

<fiddle-embed name="e3e7f0a80becb227b2d4397c5a8771e9">

</fiddle-embed>

# <a name="SkPaint::FontMetrics"></a> Struct SkPaint::FontMetrics

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
struct <a href="bmh_SkPaint?cl=9919#FontMetrics">FontMetrics</a> {
    enum <a href="bmh_SkPaint?cl=9919#FontMetrics_FontMetricsFlags">FontMetricsFlags</a> {...
    uint32_t    <a href="bmh_SkPaint?cl=9919#FontMetrics_fFlags">fFlags</a>;
    <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint?cl=9919#FontMetrics_fTop">fTop</a>;
    <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint?cl=9919#FontMetrics_fAscent">fAscent</a>;
    <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint?cl=9919#FontMetrics_fDescent">fDescent</a>;
    <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint?cl=9919#FontMetrics_fBottom">fBottom</a>;
    <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint?cl=9919#FontMetrics_fLeading">fLeading</a>;
    <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint?cl=9919#FontMetrics_fAvgCharWidth">fAvgCharWidth</a>;
    <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint?cl=9919#FontMetrics_fMaxCharWidth">fMaxCharWidth</a>;
    <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint?cl=9919#FontMetrics_fXMin">fXMin</a>;
    <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint?cl=9919#FontMetrics_fXMax">fXMax</a>;
    <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint?cl=9919#FontMetrics_fXHeight">fXHeight</a>;
    <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint?cl=9919#FontMetrics_fCapHeight">fCapHeight</a>;
    <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint?cl=9919#FontMetrics_fUnderlineThickness">fUnderlineThickness</a>;
    <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint?cl=9919#FontMetrics_fUnderlinePosition">fUnderlinePosition</a>;
    bool <a href="bmh_SkPaint?cl=9919#FontMetrics_hasUnderlineThickness">hasUnderlineThickness</a>(...
    bool <a href="bmh_SkPaint?cl=9919#FontMetrics_hasUnderlinePosition">hasUnderlinePosition</a>(...
};</pre>
<a href="bmh_SkPaint?cl=9919#FontMetrics">FontMetrics</a> is filled out by <a href="bmh_SkPaint?cl=9919#getFontMetrics">getFontMetrics</a>. <a href="bmh_SkPaint?cl=9919#FontMetrics">FontMetrics</a> contents reflect the values
computed by <a href="bmh_undocumented?cl=9919#Font_Manager">Font_Manager</a> using <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>. Values are set to zero if they are
not availble.
<a href="bmh_SkPaint?cl=9919#FontMetrics_fUnderlineThickness">fUnderlineThickness</a> and <a href="bmh_SkPaint?cl=9919#FontMetrics_fUnderlinePosition">fUnderlinePosition</a> have a bit set in <a href="bmh_SkPaint?cl=9919#FontMetrics_fFlags">fFlags</a> if their values
are valid, since their value may be zero.

## <a name="SkPaint::FontMetrics::FontMetricsFlags"></a> Enum SkPaint::FontMetrics::FontMetricsFlags

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint?cl=9919#FontMetrics_FontMetricsFlags">FontMetricsFlags</a> {
    <a href="bmh_SkPaint?cl=9919#FontMetrics_kUnderlineThinknessIsValid_Flag">kUnderlineThinknessIsValid_Flag</a> = 1 << 0,
    <a href="bmh_SkPaint?cl=9919#FontMetrics_kUnderlinePositionIsValid_Flag">kUnderlinePositionIsValid_Flag</a> = 1 << 1,
};</pre>
<a href="bmh_SkPaint?cl=9919#FontMetrics_FontMetricsFlags">FontMetricsFlags</a> are set in <a href="bmh_SkPaint?cl=9919#FontMetrics_fFlags">fFlags</a> when underline metrics are valid;
the underline metric may be valid and zero.
Fonts with embedded bitmaps may not have valid underline metrics.

### Constants

<table>
  <tr>
    <td><a name="SkPaint::FontMetrics::kUnderlineThinknessIsValid_Flag"></a> <code><strong>SkPaint::FontMetrics::kUnderlineThinknessIsValid::Flag</strong></code></td><td>0x0001</td><td><a href="bmh_SkPaint?cl=9919#FontMetrics_kUnderlineThinknessIsValid_Flag">kUnderlineThinknessIsValid_Flag</a> set in <a href="bmh_SkPaint?cl=9919#FontMetrics_fFlags">fFlags</a> if <a href="bmh_SkPaint?cl=9919#FontMetrics_fUnderlineThickness">fUnderlineThickness</a> is valid.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::FontMetrics::kUnderlinePositionIsValid_Flag"></a> <code><strong>SkPaint::FontMetrics::kUnderlinePositionIsValid::Flag</strong></code></td><td>0x0002</td><td><a href="bmh_SkPaint?cl=9919#FontMetrics_kUnderlinePositionIsValid_Flag">kUnderlinePositionIsValid_Flag</a> set in <a href="bmh_SkPaint?cl=9919#FontMetrics_fFlags">fFlags</a> if <a href="bmh_SkPaint?cl=9919#FontMetrics_fUnderlinePosition">fUnderlinePosition</a> is valid.</td>
  </tr>
</table>

<a href="bmh_SkPaint?cl=9919#FontMetrics_fFlags">fFlags</a> is set when underline metrics are valid.<a href="bmh_SkPaint?cl=9919#FontMetrics_fTop">fTop</a> is the largest height for any glyph.
        <a href="bmh_SkPaint?cl=9919#FontMetrics_fTop">fTop</a> is a measure from the baseline, and is less than or equal to zero.<a href="bmh_SkPaint?cl=9919#FontMetrics_fAscent">fAscent</a> is the recommended distance above the baseline to reserve for a line of text.
        <a href="bmh_SkPaint?cl=9919#FontMetrics_fAscent">fAscent</a> is a measure from the baseline, and is less than or equal to zero.<a href="bmh_SkPaint?cl=9919#FontMetrics_fDescent">fDescent</a> is the recommended distance below the baseline to reserve for a line of text.
        <a href="bmh_SkPaint?cl=9919#FontMetrics_fDescent">fDescent</a> is a measure from the baseline, and is greater than or equal to zero.<a href="bmh_SkPaint?cl=9919#FontMetrics_fBottom">fBottom</a> is the greatest extent below the baseline for any glyph. 
        <a href="bmh_SkPaint?cl=9919#FontMetrics_fBottom">fBottom</a> is a measure from the baseline, and is greater than or equal to zero.<a href="bmh_SkPaint?cl=9919#FontMetrics_fLeading">fLeading</a> is the recommended distance to add between lines of text.
        <a href="bmh_SkPaint?cl=9919#FontMetrics_fLeading">fLeading</a> is greater than or equal to zero.<a href="bmh_SkPaint?cl=9919#FontMetrics_fAvgCharWidth">fAvgCharWidth</a> is the average character width, if it is available.
        <a href="bmh_SkPaint?cl=9919#FontMetrics_fAvgCharWidth">fAvgCharWidth</a> is zero if no average width is stored in the font.<a href="bmh_SkPaint?cl=9919#FontMetrics_fMaxCharWidth">fMaxCharWidth</a> is the max character width.<a href="bmh_SkPaint?cl=9919#FontMetrics_fXMin">fXMin</a> is the minimum bounding box x value for all glyphs. <a href="bmh_SkPaint?cl=9919#FontMetrics_fXMin">fXMin</a>
        is typically less than zero.<a href="bmh_SkPaint?cl=9919#FontMetrics_fXMax">fXMax</a> is the maximum bounding box x value for all glyphs. <a href="bmh_SkPaint?cl=9919#FontMetrics_fXMax">fXMax</a>
        is typically greater than zero.<a href="bmh_SkPaint?cl=9919#FontMetrics_fXHeight">fXHeight</a> is the height of a lower-case 'x'. <a href="bmh_SkPaint?cl=9919#FontMetrics_fXHeight">fXHeight</a> may be zero
        if no lower-case height is stored in the font.<a href="bmh_SkPaint?cl=9919#FontMetrics_fCapHeight">fCapHeight</a> is the height of an upper-case letter. <a href="bmh_SkPaint?cl=9919#FontMetrics_fCapHeight">fCapHeight</a> may be zero
        if no upper-case height is stored in the font.<a href="bmh_SkPaint?cl=9919#FontMetrics_fUnderlineThickness">fUnderlineThickness</a> is the underline thickness. If the metric
        is valid, the <a href="bmh_SkPaint?cl=9919#FontMetrics_kUnderlineThinknessIsValid_Flag">kUnderlineThinknessIsValid_Flag</a> is set in <a href="bmh_SkPaint?cl=9919#FontMetrics_fFlags">fFlags</a>.
        If <a href="bmh_SkPaint?cl=9919#FontMetrics_kUnderlineThinknessIsValid_Flag">kUnderlineThinknessIsValid_Flag</a> is clear, <a href="bmh_SkPaint?cl=9919#FontMetrics_fUnderlineThickness">fUnderlineThickness</a> is zero.<a href="bmh_SkPaint?cl=9919#FontMetrics_fUnderlinePosition">fUnderlinePosition</a> is the underline position relative to the baseline.
       If may be negative, to draw the underline above the baseline, zero
       to draw the underline on the baseline, or positive to draw the underline
       below the baseline. 
       If the metric is valid, the <a href="bmh_SkPaint?cl=9919#FontMetrics_kUnderlinePositionIsValid_Flag">kUnderlinePositionIsValid_Flag</a> is set in <a href="bmh_SkPaint?cl=9919#FontMetrics_fFlags">fFlags</a>.
       If <a href="bmh_SkPaint?cl=9919#FontMetrics_kUnderlinePositionIsValid_Flag">kUnderlinePositionIsValid_Flag</a> is clear, <a href="bmh_SkPaint?cl=9919#FontMetrics_fUnderlinePosition">fUnderlinePosition</a> is zero.

<a name="hasUnderlineThickness"></a>

## hasUnderlineThickness

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool hasUnderlineThickness(SkScalar* thickness)  const
</pre>

thickness receives the underline width.
        If <a href="bmh_SkPaint?cl=9919#Font_Metrics">Font_Metrics</a> has a valid underline thickness, return true, and set 
        thickness to that value. If it doesn't, return false, and ignore
        thickness.---

<a name="hasUnderlinePosition"></a>

## hasUnderlinePosition

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool hasUnderlinePosition(SkScalar* position)  const
</pre>

position receives the underline offset from the baseline.
        If <a href="bmh_SkPaint?cl=9919#Font_Metrics">Font_Metrics</a> has a valid underline position, return true, and set 
        position to that value. If it doesn't, return false, and ignore
        position.---

<a name="getFontMetrics"></a>

## getFontMetrics

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkScalar getFontMetrics(FontMetrics* metrics, SkScalar scale = 0)  const
</pre>

<a href="bmh_SkPaint?cl=9919#getFontMetrics">getFontMetrics</a> returns <a href="bmh_SkPaint?cl=9919#Font_Metrics">Font_Metrics</a> associated with <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>.
The return value is the recommended spacing between lines: the sum of metrics
descent, ascent, and leading.
If metrics is not nullptr, <a href="bmh_SkPaint?cl=9919#Font_Metrics">Font_Metrics</a> is copied to metrics.
<a href="bmh_SkPaint?cl=9919#getFontMetrics">getFontMetrics</a> scales its results by <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> but does not take into account
dimensions required by <a href="bmh_SkPaint?cl=9919#Text_Scale_X">Text_Scale_X</a>, <a href="bmh_SkPaint?cl=9919#Text_Skew_X">Text_Skew_X</a>, <a href="bmh_SkPaint?cl=9919#Fake_Bold">Fake_Bold</a>,
<a href="bmh_SkPaint?cl=9919#Style_Stroke">Style_Stroke</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>.
<a href="bmh_SkPaint?cl=9919#getFontMetrics">getFontMetrics</a> results can be additionally scaled by scale.

### Parameters

<table>
  <tr>
    <td><code><strong>metrics</strong></code></td> <td>If not null, <a href="bmh_SkPaint?cl=9919#Font_Metrics">Font_Metrics</a> for <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> are copied here.</td>
  </tr>
  <tr>
    <td><code><strong>scale</strong></code></td> <td>If not 0, an additional multiplier for returned values.</td>
  </tr>
</table>

### Return Value

The recommended spacing between lines.

### Example

<fiddle-embed name="68cd6aefd0012ef68e5986569d588343">

</fiddle-embed>

### See Also

<a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> <a href="bmh_SkPaint?cl=9919#Typeface_Methods">Typeface_Methods</a>

---

<a name="getFontSpacing"></a>

## getFontSpacing

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkScalar getFontSpacing()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getFontSpacing">getFontSpacing</a> returns the recommended spacing between lines: the sum of metrics
descent, ascent, and leading.
<a href="bmh_SkPaint?cl=9919#getFontSpacing">getFontSpacing</a> scales its results by <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> but does not take into account
dimensions required by stroking and <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>.
<a href="bmh_SkPaint?cl=9919#getFontSpacing">getFontSpacing</a> returns the same result as <a href="bmh_SkPaint?cl=9919#getFontMetrics">getFontMetrics</a>.

### Return Value

The recommended spacing between lines.

### Example

<fiddle-embed name="a67cc9ac54b30e741f1b6b9334ad2ecc">

#### Example Output

~~~~
textSize: 12 fontSpacing: 13.9688
textSize: 18 fontSpacing: 20.9531
textSize: 24 fontSpacing: 27.9375
textSize: 32 fontSpacing: 37.25
~~~~

</fiddle-embed>

---

<a name="getFontBounds"></a>

## getFontBounds

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkRect getFontBounds()  const
</pre>

<a href="bmh_SkPaint?cl=9919#getFontBounds">getFontBounds</a> returns the union of the bounds of all glyphs.
<a href="bmh_SkPaint?cl=9919#getFontBounds">getFontBounds</a> dimensions are computed by <a href="bmh_undocumented?cl=9919#Font_Manager">Font_Manager</a> from font data, 
ignoring <a href="bmh_SkPaint?cl=9919#Hinting">Hinting</a>. <a href="bmh_SkPaint?cl=9919#getFontBounds">getFontBounds</a> includes <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a>, <a href="bmh_SkPaint?cl=9919#Text_Scale_X">Text_Scale_X</a>,
and <a href="bmh_SkPaint?cl=9919#Text_Skew_X">Text_Skew_X</a>, but not <a href="bmh_SkPaint?cl=9919#Fake_Bold">Fake_Bold</a> or <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>.
If <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> is large, <a href="bmh_SkPaint?cl=9919#Text_Scale_X">Text_Scale_X</a> is one, and <a href="bmh_SkPaint?cl=9919#Text_Skew_X">Text_Skew_X</a> is zero,
<a href="bmh_SkPaint?cl=9919#getFontBounds">getFontBounds</a> returns the same bounds as <a href="bmh_SkPaint?cl=9919#Font_Metrics">Font_Metrics</a> { <a href="bmh_SkPaint?cl=9919#FontMetrics_fXMin">FontMetrics::fXMin</a>, 
<a href="bmh_SkPaint?cl=9919#FontMetrics_fTop">FontMetrics::fTop</a>, <a href="bmh_SkPaint?cl=9919#FontMetrics_fXMax">FontMetrics::fXMax</a>, <a href="bmh_SkPaint?cl=9919#FontMetrics_fBottom">FontMetrics::fBottom</a> }.

### Example

<fiddle-embed name="6a7b89373d7281a458c275ebb70e8d0e">

#### Example Output

~~~~
metrics bounds = { -12.2461, -14.7891, 21.5215, 5.55469 }
font bounds    = { -12.2461, -14.7891, 21.5215, 5.55469 }
~~~~

</fiddle-embed>

---

<a name="textToGlyphs"></a>

## textToGlyphs

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
int textToGlyphs(const void* text, size_t byteLength, SkGlyphID glyphs[])
</pre>

const
<a href="bmh_SkPaint?cl=9919#textToGlyphs">textToGlyphs</a> converts text into glyph indices.
<a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> specifies how text represents characters or glyphs.
<a href="bmh_SkPaint?cl=9919#textToGlyphs">textToGlyphs</a> returns the number of glyph indices represented by text.
glyphs may be nullptr, to compute the glyph count.
<a href="bmh_SkPaint?cl=9919#textToGlyphs">textToGlyphs</a> does not check text for valid character encoding or valid
glyph indices.
If byteLength equals zero, <a href="bmh_SkPaint?cl=9919#textToGlyphs">textToGlyphs</a> returns zero.
If byteLength includes a partial character, the partial character is ignored.
If <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> is <a href="bmh_SkPaint?cl=9919#kUTF8_TextEncoding">kUTF8_TextEncoding</a> and
text contains an invalid <a href="bmh_undocumented?cl=9919#UTF_8">UTF-8</a> sequence, zero is returned.

### Example

<fiddle-embed name="bfdd99089bdc3bdd2af67f3b6344b4ef">

</fiddle-embed>

---

<a name="countText"></a>

## countText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
int countText(const void* text, size_t byteLength)  const
</pre>

<a href="bmh_SkPaint?cl=9919#countText">countText</a> returns the number of glyphs in text.
<a href="bmh_SkPaint?cl=9919#countText">countText</a> uses <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> to count the glyphs.
<a href="bmh_SkPaint?cl=9919#countText">countText</a> returns the same result as <a href="bmh_SkPaint?cl=9919#textToGlyphs">textToGlyphs</a>.

### Example

<fiddle-embed name="6e9152c7aa85b69bb25c90122e0f75a7">

#### Example Output

~~~~
count = 4
~~~~

</fiddle-embed>

---

<a name="containsText"></a>

## containsText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool containsText(const void* text, size_t byteLength)  const
</pre>

<a href="bmh_SkPaint?cl=9919#containsText">containsText</a> returns true if all text corresponds to a non-zero glyph index. 
<a href="bmh_SkPaint?cl=9919#containsText">containsText</a> returns false if any characters in text are not supported in
<a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>.
If <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> is <a href="bmh_SkPaint?cl=9919#kGlyphID_TextEncoding">kGlyphID_TextEncoding</a>, <a href="bmh_SkPaint?cl=9919#containsText">containsText</a>
returns true if all glyph indices in text are non-zero; <a href="bmh_SkPaint?cl=9919#containsText">containsText</a>
does not check to see if text contains valid glyph indices for <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>.
<a href="bmh_SkPaint?cl=9919#containsText">containsText</a> returns true if bytelength is zero.

### Example

<fiddle-embed name="0e4177aaf9931893c6c80070a7bf44c2">

#### Example Output

~~~~
0x00b0 == has char
0xd800 != has char
~~~~

</fiddle-embed>

### Example

<fiddle-embed name="556776e9b7065168d6640042dde42ab8">

#### Example Output

~~~~
0x01ff == has glyph
0x0000 != has glyph
0xffff == has glyph
~~~~

</fiddle-embed>

### See Also

<a href="bmh_SkPaint?cl=9919#setTextEncoding">setTextEncoding</a> <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>

---

<a name="glyphsToUnichars"></a>

## glyphsToUnichars

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void glyphsToUnichars(const SkGlyphID glyphs[], int count, SkUnichar text[])  const
</pre>

<a href="bmh_SkPaint?cl=9919#glyphsToUnichars">glyphsToUnichars</a> converts glyphs into text if possible. 
<a href="bmh_SkPaint?cl=9919#glyphsToUnichars">glyphsToUnichars</a> is only supported on platforms that use <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a> as the <a href="bmh_undocumented?cl=9919#Engine">Font_Engine</a>.
Glyph values without direct <a href="bmh_undocumented?cl=9919#Unicode">Unicode</a> equivalents are mapped to zero. 
<a href="bmh_SkPaint?cl=9919#glyphsToUnichars">glyphsToUnichars</a> uses the <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, but is unaffected
by <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a>; the text values returned are equivalent to <a href="bmh_SkPaint?cl=9919#kUTF32_TextEncoding">kUTF32_TextEncoding</a>.

### Example

<fiddle-embed name="">

</fiddle-embed>

---

# <a name="Measure_Text"></a> Measure Text

<a name="measureText"></a>

## measureText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkScalar measureText(const void* text, size_t length, SkRect* bounds)  const
</pre>

Returns the advance width of text if <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> is clear,
and the height of text if <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> is set.
The advance is the normal distance to move before drawing additional text.
<a href="bmh_SkPaint?cl=9919#measureText">measureText</a> uses <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> to decode text, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the font metrics,
and <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a>, <a href="bmh_SkPaint?cl=9919#Text_Scale_X">Text_Scale_X</a>, <a href="bmh_SkPaint?cl=9919#Text_Skew_X">Text_Skew_X</a>, <a href="bmh_SkPaint?cl=9919#Stroke_Width">Stroke_Width</a>, and
<a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> to scale the metrics and bounds.
<a href="bmh_SkPaint?cl=9919#measureText">measureText</a> returns the bounding box of text if bounds is not nullptr.
The bounding box is computed as if the text was drawn at the origin.
 
### Parameters

<table>
  <tr>
    <td><code><strong>text</strong></code></td> <td>Character codes or glyph indices to be measured.</td>
  </tr>
  <tr>
    <td><code><strong>length</strong></code></td> <td>Number of bytes of text to measure.</td>
  </tr>
  <tr>
    <td><code><strong>bounds</strong></code></td> <td>Returns bounding box relative to (0, 0) if not nullptr.</td>
  </tr>
</table>

### Return Value

The advance width or height.

### Example

<fiddle-embed name="ed96dd912a1cbeeabb21b7ce386da003">

</fiddle-embed>

---

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkScalar measureText(const void* text, size_t length)  const
</pre>

Returns the adance width of text if <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> is clear,
and the height of text if <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> is set.
The advance is the normal distance to move before drawing additional text.
<a href="bmh_SkPaint?cl=9919#measureText">measureText</a> uses <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> to decode text, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the font metrics,
and <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> to scale the metrics.
<a href="bmh_SkPaint?cl=9919#measureText">measureText</a> does not scale the advance or bounds by <a href="bmh_SkPaint?cl=9919#Text_Decorations">Text_Decorations</a>
or <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>text</strong></code></td> <td>Character codes or glyph indices to be measured.</td>
  </tr>
  <tr>
    <td><code><strong>length</strong></code></td> <td>Number of bytes of text to measure.</td>
  </tr>
</table>

### Return Value

The advance width or height.

### Example

<fiddle-embed name="1db5db3866773f1fe788b8ef38b7dd71">

#### Example Output

~~~~
default width = 5
double width = 10
~~~~

</fiddle-embed>

---

<a name="breakText"></a>

## breakText

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
size_t  breakText(const void* text, size_t length, SkScalar maxWidth,
                  SkScalar* measuredWidth = NULL)
</pre>

const
    <a href="bmh_SkPaint?cl=9919#breakText">breakText</a> returns the bytes of text that fit within maxWidth.
    If <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> is clear, the text fragment fits if its advance width is less than or
    equal to maxWidth.
    If <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> is set, the text fragment fits if its advance height is less than or
    equal to maxWidth.
    <a href="bmh_SkPaint?cl=9919#breakText">breakText</a> measures only while the advance is less than or equal to maxWidth.
    <a href="bmh_SkPaint?cl=9919#breakText">breakText</a> returns the advance or the text fragment in measureWidth if it not nullptr.
    <a href="bmh_SkPaint?cl=9919#breakText">breakText</a> uses <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> to decode text, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the font metrics,
    and <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> to scale the metrics.
    <a href="bmh_SkPaint?cl=9919#breakText">breakText</a> does not scale the advance or bounds by <a href="bmh_SkPaint?cl=9919#Text_Decorations">Text_Decorations</a>
    or <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>text</strong></code></td> <td>Character codes or glyph indices to be measured.</td>
  </tr>
  <tr>
    <td><code><strong>length</strong></code></td> <td>Number of bytes of text to measure.</td>
  </tr>
  <tr>
    <td><code><strong>maxWidth</strong></code></td> <td>The advance limit; text is measured while advance is less than maxWidth.</td>
  </tr>
  <tr>
    <td><code><strong>measuredWidth</strong></code></td> <td>Returns the width of the text less than or equal to maxWidth.</td>
  </tr>
</table>

### Return Value

The bytes of text that fit, always less than or equal to length.

### Example

<fiddle-embed name="50698c82c7cc4510564cac15efaeef6e">

</fiddle-embed>

---

<a name="getTextWidths"></a>

## getTextWidths

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
int getTextWidths(const void* text, size_t byteLength, SkScalar widths[],
                  SkRect bounds[] = NULL)
</pre>

const
    <a href="bmh_SkPaint?cl=9919#getTextWidths">getTextWidths</a> retrieves the advance and bounds for each glyph in text, and returns
    the glyph count in text.
    Both widths and bounds may be nullptr.
    If widths is not nullptr, widths must be an array of glyph count entries.
    if bounds is not nullptr, bounds must be an array of glyph count entries. 
    If <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> is clear, widths returns the horizontal advance.
    If <a href="bmh_SkPaint?cl=9919#kVerticalText_Flag">kVerticalText_Flag</a> is set, widths returns the vertical advance.
    <a href="bmh_SkPaint?cl=9919#getTextWidths">getTextWidths</a> uses <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> to decode text, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the font metrics,
    and <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a> to scale the widths and bounds.
    <a href="bmh_SkPaint?cl=9919#getTextWidths">getTextWidths</a> does not scale the advance by <a href="bmh_SkPaint?cl=9919#Text_Decorations">Text_Decorations</a> or <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a>.
    <a href="bmh_SkPaint?cl=9919#getTextWidths">getTextWidths</a> does return include <a href="bmh_SkPaint?cl=9919#Text_Decorations">Text_Decorations</a> and <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> in the bounds.
   
### Parameters

<table>
  <tr>
    <td><code><strong>text</strong></code></td> <td>Character codes or glyph indices to be measured.</td>
  </tr>
  <tr>
    <td><code><strong>length</strong></code></td> <td>Number of bytes of text to measure.</td>
  </tr>
  <tr>
    <td><code><strong>widths</strong></code></td> <td>If not nullptr, returns the text advances for each glyph.</td>
  </tr>
  <tr>
    <td><code><strong>bounds</strong></code></td> <td>If not nullptr, returns the bounds for each glyph relative to the origin at (0, 0).</td>
  </tr>
</table>

### Return Value

The glyph count in text.

### Example

<fiddle-embed name="31f29360964e867e5569291d9b53a2aa">

</fiddle-embed>

---

# <a name="Text_Path"></a> Text Path
<a href="bmh_SkPaint?cl=9919#Text_Path">Text_Path</a> describes the geometry of glyphs used to draw text.

<a name="getTextPath"></a>

## getTextPath

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void getTextPath(const void* text, size_t length, SkScalar x, SkScalar y,
                 SkPath* path)
</pre>

const
<a href="bmh_SkPaint?cl=9919#getTextPath">getTextPath</a> returns the geometry as <a href="bmh_undocumented?cl=9919#Path">Path</a> equivalent to the drawn text.
<a href="bmh_SkPaint?cl=9919#getTextPath">getTextPath</a> uses <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> to decode text, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the glyph paths,
and <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a>, <a href="bmh_SkPaint?cl=9919#Text_Decorations">Text_Decorations</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> to scale and modify the glyph paths.
All of the glyph paths are stored in path.
<a href="bmh_SkPaint?cl=9919#getTextPath">getTextPath</a> uses x, y, and <a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> to position path.

### Parameters

<table>
  <tr>
    <td><code><strong>text</strong></code></td> <td>Character codes or glyph indices.</td>
  </tr>
  <tr>
    <td><code><strong>length</strong></code></td> <td>Number of bytes of text.</td>
  </tr>
  <tr>
    <td><code><strong>x</strong></code></td> <td>The x-coordinate of the origin of the text.</td>
  </tr>
  <tr>
    <td><code><strong>y</strong></code></td> <td>The y-coordinate of the origin of the text.</td>
  </tr>
  <tr>
    <td><code><strong>path</strong></code></td> <td>The geometry of the glyphs.</td>
  </tr>
</table>

### Example

<fiddle-embed name="4419b133092615ede9c32fbc3b567d1c">

</fiddle-embed>

---

<a name="getPosTextPath"></a>

## getPosTextPath

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void getPosTextPath(const void* text, size_t length, const SkPoint pos[],
                    SkPath* path)
</pre>

const
<a href="bmh_SkPaint?cl=9919#getPosTextPath">getPosTextPath</a> returns the geometry as <a href="bmh_undocumented?cl=9919#Path">Path</a> equivalent to the drawn text.
<a href="bmh_SkPaint?cl=9919#getPosTextPath">getPosTextPath</a> uses <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> to decode text, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the glyph paths,
and <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a>, <a href="bmh_SkPaint?cl=9919#Text_Decorations">Text_Decorations</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> to scale and modify the glyph paths.
All of the glyph paths are stored in path.
<a href="bmh_SkPaint?cl=9919#getPosTextPath">getPosTextPath</a> uses pos array and <a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> to position path.
pos contains a position for each glyph.

### Parameters

<table>
  <tr>
    <td><code><strong>text</strong></code></td> <td>Character codes or glyph indices.</td>
  </tr>
  <tr>
    <td><code><strong>length</strong></code></td> <td>Number of bytes of text.</td>
  </tr>
  <tr>
    <td><code><strong>pos</strong></code></td> <td>The positions of each glyph.</td>
  </tr>
  <tr>
    <td><code><strong>path</strong></code></td> <td>The geometry of the glyphs.</td>
  </tr>
</table>

### Example

<fiddle-embed name="f7dc4fad0676d7c6bf0771f406eff51e">

</fiddle-embed>

---

# <a name="Text_Intercepts"></a> Text Intercepts
<a href="bmh_SkPaint?cl=9919#Text_Intercepts">Text_Intercepts</a> describe the intersection of drawn text glyphs with a pair
of lines parallel to the text advance. <a href="bmh_SkPaint?cl=9919#Text_Intercepts">Text_Intercepts</a> permits creating a
underline that skips descenders.

<a name="getTextIntercepts"></a>

## getTextIntercepts

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
int getTextIntercepts(const void* text, size_t length, SkScalar x, SkScalar y,
                      const SkScalar bounds[2], SkScalar* intervals)
</pre>

const
    <a href="bmh_SkPaint?cl=9919#getTextIntercepts">getTextIntercepts</a> returns the number of intervals that intersect bounds.
    bounds describes a pair of lines parallel to the text advance.
    The return count is zero or a multiple of two, and is at most twice the number of glyphs in
    the string. 
    <a href="bmh_SkPaint?cl=9919#getTextIntercepts">getTextIntercepts</a> uses <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> to decode text, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the glyph paths,
    and <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a>, <a href="bmh_SkPaint?cl=9919#Text_Decorations">Text_Decorations</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> to scale and modify the glyph paths.
    <a href="bmh_SkPaint?cl=9919#getTextIntercepts">getTextIntercepts</a> uses x, y, and <a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> to position intervals.
        Pass nullptr for intervals to determine the size of the interval array.
        intervals are cached to improve performance for multiple calls.

### Parameters

<table>
  <tr>
    <td><code><strong>text</strong></code></td> <td>Character codes or glyph indices.</td>
  </tr>
  <tr>
    <td><code><strong>length</strong></code></td> <td>Number of bytes of text.</td>
  </tr>
  <tr>
    <td><code><strong>x</strong></code></td> <td>The x-coordinate of the origin of the text.</td>
  </tr>
  <tr>
    <td><code><strong>y</strong></code></td> <td>The y-coordinate of the origin of the text.</td>
  </tr>
  <tr>
    <td><code><strong>bounds</strong></code></td> <td>The lower and upper line parallel to the advance.</td>
  </tr>
  <tr>
    <td><code><strong>intervals</strong></code></td> <td>If not null, the returned intersections.</td>
  </tr>
</table>

### Return Value

The number of intersections, which may be zero.

### Example

<fiddle-embed name="31dd0970f3200545427390788636fbe4">

</fiddle-embed>

---

<a name="getPosTextIntercepts"></a>

## getPosTextIntercepts

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
int getPosTextIntercepts(const void* text, size_t length, const SkPoint pos[],
                         const SkScalar bounds[2], SkScalar* intervals)
</pre>

const
    <a href="bmh_SkPaint?cl=9919#getPosTextIntercepts">getPosTextIntercepts</a> returns the number of intervals that intersect bounds.
    bounds describes a pair of lines parallel to the text advance.
    The return count is zero or a multiple of two, and is at most twice the number of glyphs in
    the string. 
    <a href="bmh_SkPaint?cl=9919#getPosTextIntercepts">getPosTextIntercepts</a> uses <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> to decode text, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the glyph paths,
    and <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a>, <a href="bmh_SkPaint?cl=9919#Text_Decorations">Text_Decorations</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> to scale and modify the glyph paths.
    <a href="bmh_SkPaint?cl=9919#getPosTextIntercepts">getPosTextIntercepts</a> uses pos array and <a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> to position intervals.
        Pass nullptr for intervals to determine the size of the interval array.
        intervals are cached to improve performance for multiple calls.

### Parameters

<table>
  <tr>
    <td><code><strong>text</strong></code></td> <td>Character codes or glyph indices.</td>
  </tr>
  <tr>
    <td><code><strong>length</strong></code></td> <td>Number of bytes of text.</td>
  </tr>
  <tr>
    <td><code><strong>pos</strong></code></td> <td>The positions of each glyph.</td>
  </tr>
  <tr>
    <td><code><strong>bounds</strong></code></td> <td>The lower and upper line parallel to the advance.</td>
  </tr>
  <tr>
    <td><code><strong>intervals</strong></code></td> <td>If not null, the returned intersections.</td>
  </tr>
</table>

### Return Value

The number of intersections, which may be zero.

### Example

<fiddle-embed name="5c7ca6a2c8896e4717d1d7f92526f8aa">

</fiddle-embed>

---

<a name="getPosTextHIntercepts"></a>

## getPosTextHIntercepts

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
int getPosTextHIntercepts(const void* text, size_t length, const SkScalar xpos[],
                          SkScalar constY, const SkScalar bounds[2],
                          SkScalar* intervals)
</pre>

const
    <a href="bmh_SkPaint?cl=9919#getPosTextHIntercepts">getPosTextHIntercepts</a> returns the number of intervals that intersect bounds.
    bounds describes a pair of lines parallel to the text advance.
    The return count is zero or a multiple of two, and is at most twice the number of glyphs in
    the string. 
    <a href="bmh_SkPaint?cl=9919#getPosTextHIntercepts">getPosTextHIntercepts</a> uses <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> to decode text, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the glyph paths,
    and <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a>, <a href="bmh_SkPaint?cl=9919#Text_Decorations">Text_Decorations</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> to scale and modify the glyph paths.
    <a href="bmh_SkPaint?cl=9919#getPosTextHIntercepts">getPosTextHIntercepts</a> uses xpos array, constY, and <a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> to position intervals.
        Pass nullptr for intervals to determine the size of the interval array.
        intervals are cached to improve performance for multiple calls.

### Parameters

<table>
  <tr>
    <td><code><strong>text</strong></code></td> <td>Character codes or glyph indices.</td>
  </tr>
  <tr>
    <td><code><strong>length</strong></code></td> <td>Number of bytes of text.</td>
  </tr>
  <tr>
    <td><code><strong>xpos</strong></code></td> <td>The positions of each glyph in x.</td>
  </tr>
  <tr>
    <td><code><strong>constY</strong></code></td> <td>The position of each glyph in y.</td>
  </tr>
  <tr>
    <td><code><strong>bounds</strong></code></td> <td>The lower and upper line parallel to the advance.</td>
  </tr>
  <tr>
    <td><code><strong>intervals</strong></code></td> <td>If not null, the returned intersections.</td>
  </tr>
</table>

### Return Value

The number of intersections, which may be zero.

### Example

<fiddle-embed name="8f15e8305035afa24224bdc92a4cfb50">

</fiddle-embed>

---

<a name="getTextBlobIntercepts"></a>

## getTextBlobIntercepts

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
int getTextBlobIntercepts(const SkTextBlob* blob, const SkScalar bounds[2],
                          SkScalar* intervals)
</pre>

const
    <a href="bmh_SkPaint?cl=9919#getPosTextIntercepts">getPosTextIntercepts</a> returns the number of intervals that intersect bounds.
    bounds describes a pair of lines parallel to the text advance.
    The return count is zero or a multiple of two, and is at most twice the number of glyphs in
    the string. 
    <a href="bmh_SkPaint?cl=9919#getPosTextIntercepts">getPosTextIntercepts</a> uses <a href="bmh_SkPaint?cl=9919#Text_Encoding">Text_Encoding</a> to decode text, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the glyph paths,
    and <a href="bmh_SkPaint?cl=9919#Text_Size">Text_Size</a>, <a href="bmh_SkPaint?cl=9919#Text_Decorations">Text_Decorations</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path_Effect</a> to scale and modify the glyph paths.
    <a href="bmh_SkPaint?cl=9919#getPosTextIntercepts">getPosTextIntercepts</a> uses pos array and <a href="bmh_SkPaint?cl=9919#Text_Align">Text_Align</a> to position intervals.
        Pass nullptr for intervals to determine the size of the interval array.
        intervals are cached to improve performance for multiple calls.

### Parameters

<table>
  <tr>
    <td><code><strong>text</strong></code></td> <td>Character codes or glyph indices.</td>
  </tr>
  <tr>
    <td><code><strong>length</strong></code></td> <td>Number of bytes of text.</td>
  </tr>
  <tr>
    <td><code><strong>pos</strong></code></td> <td>The positions of each glyph.</td>
  </tr>
  <tr>
    <td><code><strong>bounds</strong></code></td> <td>The lower and upper line parallel to the advance.</td>
  </tr>
  <tr>
    <td><code><strong>array</strong></code></td> <td>If not null, the returned intersections.</td>
  </tr>
</table>

### Return Value

The number of intersections, which may be zero.

### Example

<fiddle-embed name="c4e51f1d95f7ab511c47a6ea6d4a3311">

</fiddle-embed>

---

<a name="nothingToDraw"></a>

## nothingToDraw

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool nothingToDraw()  const
</pre>

<a href="bmh_SkPaint?cl=9919#nothingToDraw">nothingToDraw</a> returns true if <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> prevents all drawing.
If <a href="bmh_SkPaint?cl=9919#nothingToDraw">nothingToDraw</a> returns false, the <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> is not guaranteed to allow drawing.
<a href="bmh_SkPaint?cl=9919#nothingToDraw">nothingToDraw</a> returns true if <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend_Mode</a> and <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> are enabled,
and set the computed <a href="bmh_undocumented?cl=9919#Alpha">Color_Alpha</a> to zero.

### Example

<fiddle-embed name="2fbbed1e035f836728a74673e6977c28">

#### Example Output

~~~~
nothing to draw: false
nothing to draw: true
nothing to draw: false
nothing to draw: true
~~~~

</fiddle-embed>

---

# <a name="Fast_Bounds"></a> Fast Bounds
<a href="bmh_SkPaint?cl=9919#Fast_Bounds">Fast_Bounds</a> methods conservatively outset a drawing bounds by additional area
<a href="bmh_SkPaint?cl=9919#Paint">Paint</a> may draw to.

<a name="canComputeFastBounds"></a>

## canComputeFastBounds

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
bool canComputeFastBounds()  const
</pre>

<a href="bmh_SkPaint?cl=9919#canComputeFastBounds">canComputeFastBounds</a> returns true if the current paint settings allow for fast computation of
bounds (i.e. there is nothing complex like a patheffect that would make
the bounds computation expensive.

### Example

<fiddle-embed name="04311b628fe9828d9ed921a36185db26">

</fiddle-embed>

---

<a name="computeFastBounds"></a>

## computeFastBounds

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
const SkRect& computeFastBounds(const SkRect& orig, SkRect* storage)  const
</pre>

Only call this if <a href="bmh_SkPaint?cl=9919#canComputeFastBounds">canComputeFastBounds</a>() returned true. This takes a
raw rectangle (the raw bounds of a shape), and adjusts it for stylistic
effects in the paint (e.g. stroking). If needed, it uses the storage
rect parameter. It returns the adjusted bounds that can then be used
for quickReject tests.
The returned rect will either be orig or storage, thus the caller
should not rely on storage being set to the result, but should always
use the retured value. It is legal for orig and storage to be the same
rect.

### Example

<fiddle-embed name="80634aec853885c120332095e25e1852">

</fiddle-embed>

---

<a name="computeFastStrokeBounds"></a>

## computeFastStrokeBounds

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
const SkRect& computeFastStrokeBounds(const SkRect& orig, SkRect* storage)
</pre>

const

### Example

<fiddle-embed name="04311b628fe9828d9ed921a36185db26">

</fiddle-embed>

---

<a name="doComputeFastBounds"></a>

## doComputeFastBounds

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
const SkRect& doComputeFastBounds(const SkRect& orig, SkRect* storage, Style)
</pre>

const
    Take the style explicitly, so the caller can force us to be stroked
    without having to make a copy of the paint just to change that field.

### Example

<fiddle-embed name="04311b628fe9828d9ed921a36185db26">

</fiddle-embed>

---

<a name="toString"></a>

## toString

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void toString(SkString* str)  const;
</pre>

Converts <a href="bmh_SkPaint?cl=9919#Paint">Paint</a> to machine parsable form in developer mode.

### Example

<fiddle-embed name="072356aee21483fbb454d01ebf31ef28">

#### Example Output

~~~~
text size = 12
~~~~

</fiddle-embed>

---

