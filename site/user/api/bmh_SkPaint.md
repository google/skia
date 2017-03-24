# <a name="SkPaint"></a> Class SkPaint
## <a name="Paint_Overview"></a> Paint Overview
### <a name="Paint_Subtopics"></a> Paint Subtopics
### <a name="Paint_Constants"></a> Paint Constants

| constants | description |
| --- | ---  |
| [Align](#SkPaint_Align) | Glyph locations relative to text position. |
| [Cap](#SkPaint_Cap) | Start and end geometry on stroked shapes. |
| [Flags](#SkPaint_Flags) | Values described by bits and masks. |
| [FontMetrics_FontMetricsFlags](#) | Valid [Paint_Font_Metrics](#Paint_Font_Metrics). |
| [Hinting](#SkPaint_Hinting) | Level of glyph outline adjustment. |
| [Join](#SkPaint_Join) | Corner geometry on stroked shapes. |
| [Style](#SkPaint_Style) | Stroke, fill, or both. |
| [TextEncoding](#SkPaint_TextEncoding) | Character or glyph encoding size. |

### <a name="Structures"></a> Structures

| structure | description |
| --- | ---  |
| [FontMetrics](#SkPaint_FontMetrics) | [Typeface](#Typeface) values. |

### <a name="Paint_Constructors"></a> Paint Constructors

|  | description |
| --- | ---  |
| [SkPaint()](#SkPaint_empty_constructor) | Constructs with default values. |
| [SkPaint(const SkPaint& paint)](#SkPaint_copy_constructor) | Makes a shallow copy. |
| [SkPaint(SkPaint&& paint)](#SkPaint_move_constructor) | Moves paint without copying it. |
| [~SkPaint()](#SkPaint_destructor) | Decreases [Reference_Count](#Reference_Count) of owned objects. |

### <a name="Paint_Operators"></a> Paint Operators

| operator | description |
| --- | ---  |
| [operator=(const SkPaint&)](#SkPaint_copy_assignment_operator) | Makes a shallow copy. |
| [operator=(SkPaint&&)](#SkPaint_move_assignment_operator) | Moves paint without copying it. |
| [operator==(const SkPaint& a, const SkPaint& b)](#SkPaint_equal_operator) | Compares paints for equality. |
| [operator!=(const SkPaint& a, const SkPaint& b)](#SkPaint_not_equal_operator) | Compares paints for inequality. |

### <a name="Paint_Member_Functions"></a> Paint Member Functions

| function | description |
| --- | ---  |
| [breakText](#SkPaint_breakText) | Returns text that fits in a width. |
| [canComputeFastBounds](#SkPaint_canComputeFastBounds) | Returns true if settings allow for fast bounds computation. |
| [computeFastBounds](#SkPaint_computeFastBounds) | Returns fill bounds for quick reject tests. |
| [computeFastStrokeBounds](#SkPaint_computeFastStrokeBounds) | Returns stroke bounds for quick reject tests. |
| [containsText](#SkPaint_containsText) | Returns if all text corresponds to glyphs. |
| [countText](#SkPaint_countText) | Returns number of glyphs in text. |
| [doComputeFastBounds](#SkPaint_doComputeFastBounds) | Returns bounds for quick reject tests. |
| [flatten](#SkPaint_flatten) | Serializes into a buffer. |
| [getAlpha](#SkPaint_getAlpha) | Returns [Color_Alpha](#Color_Alpha), color opacity. |
| [getBlendMode](#SkPaint_getBlendMode) | Returns [Blend_Mode](#Blend_Mode), how colors combine with dest. |
| [getColor](#SkPaint_getColor) | Returns [Color_Alpha](#Color_Alpha) and [Color_RGB](#Color_RGB), one drawing color. |
| [getColorFilter](#SkPaint_getColorFilter) | Returns [Color_Filter](#Color_Filter), how colors are altered. |
| [getDrawLooper](#SkPaint_getDrawLooper) | Returns [Draw_Looper](#Draw_Looper), multiple layers. |
| [getFillPath](#SkPaint_getFillPath) | Returns fill path equivalent to stroke. |
| [getFilterQuality](#SkPaint_getFilterQuality) | Returns [Filter_Quality](#Filter_Quality), image filtering level. |
| [getFlags](#SkPaint_getFlags) | Returns [Flags](#SkPaint_Flags) stored in a bit field. |
| [getFontBounds](#SkPaint_getFontBounds) | Returns union all glyph bounds. |
| [getFontMetrics](#SkPaint_getFontMetrics) | Returns [Typeface](#Typeface) metrics scaled by text size. |
| [getFontSpacing](#SkPaint_getFontSpacing) | Returns recommended spacing between lines. |
| [getHash](#SkPaint_getHash) | Returns a shallow hash for equality checks. |
| [getHinting](#SkPaint_getHinting) | Returns [Hinting](#SkPaint_Hinting), glyph outline adjustment level. |
| [getImageFilter](#SkPaint_getImageFilter) | Returns [Image_Filter](#Image_Filter), alter pixels; blur. |
| [getMaskFilter](#SkPaint_getMaskFilter) | Returns [Mask_Filter](#Mask_Filter), alterations to [Mask_Alpha](#Mask_Alpha). |
| [getPathEffect](#SkPaint_getPathEffect) | Returns [Path_Effect](#Path_Effect), modifications to path geometry; dashing. |
| [getPosTextPath](#SkPaint_getPosTextPath) | Returns [Path](#Path) equivalent to positioned text. |
| [getPosTextIntercepts](#SkPaint_getPosTextIntercepts) | Returns where lines intersect positioned text; underlines. |
| [getPosTextHIntercepts](#SkPaint_getPosTextHIntercepts) | Returns where lines intersect horizontally positioned text; underlines. |
| [getRasterizer](#SkPaint_getRasterizer) | Returns [Rasterizer](#Rasterizer), [Mask_Alpha](#Mask_Alpha) generation from [Path](#Path). |
| [getShader](#SkPaint_getShader) | Returns [Shader](#Shader), multiple drawing colors; gradients. |
| [getStrokeCap](#SkPaint_getStrokeCap) | Returns [Cap](#SkPaint_Cap), the area drawn at path ends. |
| [getStrokeJoin](#SkPaint_getStrokeJoin) | Returns [Join](#SkPaint_Join), geometry on path corners. |
| [getStrokeMiter](#SkPaint_getStrokeMiter) | Returns [Miter_Limit](#Miter_Limit), angles with sharp corners. |
| [getStrokeWidth](#SkPaint_getStrokeWidth) | Returns thickness of the stroke. |
| [getStyle](#SkPaint_getStyle) | Returns [Style](#SkPaint_Style): stroke, fill, or both. |
| [getTextAlign](#SkPaint_getTextAlign) | Returns [Align](#SkPaint_Align): left, center, or right. |
| [getTextBlobIntercepts](#SkPaint_getTextBlobIntercepts) | Returns where lines intersect [Text_Blob](#Text_Blob); underlines. |
| [getTextEncoding](#SkPaint_getTextEncoding) | Returns character or glyph encoding size. |
| [getTextIntercepts](#SkPaint_getTextIntercepts) | Returns where lines intersect text; underlines. |
| [getTextPath](#SkPaint_getTextPath) | Returns [Path](#Path) equivalent to text. |
| [getTextScaleX](#SkPaint_getTextScaleX) | Returns the text horizontal scale; condensed text. |
| [getTextSkewX](#SkPaint_getTextSkewX) | Returns the text horizontal skew; oblique text. |
| [getTextSize](#SkPaint_getTextSize) | Returns text size in points. |
| [getTextWidths](#SkPaint_getTextWidths) | Returns advance and bounds for each glyph in text. |
| [getTypeface](#SkPaint_getTypeface) | Returns [Typeface](#Typeface), font description. |
| [glyphsToUnichars](#SkPaint_glyphsToUnichars) | Converts glyphs into text. |
| [isAntiAlias](#SkPaint_isAntiAlias) | Returns true if [Anti-alias](#Anti-alias) is set. |
| [isAutohinted](#SkPaint_isAutohinted) | Deprecated. |
| [isDevKernText](#SkPaint_isDevKernText) | Returns true if [Full_Hinting_Spacing](#Full_Hinting_Spacing) is set. |
| [isDither](#SkPaint_isDither) | Returns true if [Dither](#Dither) is set. |
| [isEmbeddedBitmapText](#SkPaint_isEmbeddedBitmapText) | Returns true if [Font_Embedded_Bitmaps](#Font_Embedded_Bitmaps) is set. |
| [isFakeBoldText](#SkPaint_isFakeBoldText) | Returns true if [Fake_Bold](#Fake_Bold) is set. |
| [isLCDRenderText](#SkPaint_isLCDRenderText) | Returns true if [LCD_Text](#LCD_Text) is set. |
| [isSrcOver](#SkPaint_isSrcOver) | Returns true if [Blend_Mode](#Blend_Mode) is [SkBlendMode_kSrcOver](#SkBlendMode_kSrcOver). |
| [isSubpixelText](#SkPaint_isSubpixelText) | Returns true if [Subpixel_Text](#Subpixel_Text) is set. |
| [isVerticalText](#SkPaint_isVerticalText) | Returns true if [Vertical_Text](#Vertical_Text) is set. |
| [measureText](#SkPaint_measureText) | Returns advance width and bounds of text. |
| [nothingToDraw](#SkPaint_nothingToDraw) | Returns true if [Paint](#Paint) prevents all drawing. |
| [refColorFilter](#SkPaint_refColorFilter) | References [Color_Filter](#Color_Filter), how colors are altered. |
| [refDrawLooper](#SkPaint_refDrawLooper) | References [Draw_Looper](#Draw_Looper), multiple layers. |
| [refImageFilter](#SkPaint_refImageFilter) | References [Image_Filter](#Image_Filter), alter pixels; blur. |
| [refMaskFilter](#SkPaint_refMaskFilter) | References [Mask_Filter](#Mask_Filter), alterations to [Mask_Alpha](#Mask_Alpha). |
| [refPathEffect](#SkPaint_refPathEffect) | References [Path_Effect](#Path_Effect), modifications to path geometry; dashing. |
| [refRasterizer](#SkPaint_refRasterizer) | References [Rasterizer](#Rasterizer), mask generation from path. |
| [refShader](#SkPaint_refShader) | References [Shader](#Shader), multiple drawing colors; gradients. |
| [refTypeface](#SkPaint_refTypeface) | References [Typeface](#Typeface), font description. |
| [reset](#SkPaint_reset) | Sets to default values. |
| [setAlpha](#SkPaint_setAlpha) | Sets [Color_Alpha](#Color_Alpha), color opacity. |
| [setAntiAlias](#SkPaint_setAntiAlias) | Sets or clears [Anti-alias](#Anti-alias). |
| [setARGB](#SkPaint_setARGB) | Sets color by component. |
| [setAutohinted](#SkPaint_setAutohinted) | Deprecated. |
| [setBlendMode](#SkPaint_setBlendMode) | Sets [Blend_Mode](#Blend_Mode), how colors combine with destination. |
| [setColor](#SkPaint_setColor) | Sets [Color_Alpha](#Color_Alpha) and [Color_RGB](#Color_RGB), one drawing color. |
| [setColorFilter](#SkPaint_setColorFilter) | Sets [Color_Filter](#Color_Filter), alters color. |
| [setDevKernText](#SkPaint_setDevKernText) | Sets or clears [Full_Hinting_Spacing](#Full_Hinting_Spacing). |
| [setDither](#SkPaint_setDither) | Sets or clears [Dither](#Dither). |
| [setDrawLooper](#SkPaint_setDrawLooper) | Sets [Draw_Looper](#Draw_Looper), multiple layers. |
| [setEmbeddedBitmapText](#SkPaint_setEmbeddedBitmapText) | Sets or clears [Font_Embedded_Bitmaps](#Font_Embedded_Bitmaps). |
| [setFakeBoldText](#SkPaint_setFakeBoldText) | Sets or clears [Fake_Bold](#Fake_Bold). |
| [setFilterQuality](#SkPaint_setFilterQuality) | Sets or clears [Filter_Quality](#Filter_Quality), image filtering level. |
| [setFlags](#SkPaint_setFlags) | Sets multiple [Flags](#SkPaint_Flags) in a bit field. |
| [setHinting](#SkPaint_setHinting) | Sets [Hinting](#SkPaint_Hinting), glyph outline adjustment level. |
| [setLCDRenderText](#SkPaint_setLCDRenderText) | Sets or clears [LCD_Text](#LCD_Text). |
| [setMaskFilter](#SkPaint_setMaskFilter) | Sets [Mask_Filter](#Mask_Filter), alterations to [Mask_Alpha](#Mask_Alpha). |
| [setPathEffect](#SkPaint_setPathEffect) | Sets [Path_Effect](#Path_Effect), modifications to path geometry; dashing. |
| [setRasterizer](#SkPaint_setRasterizer) | Sets [Rasterizer](#Rasterizer), [Mask_Alpha](#Mask_Alpha) generation from [Path](#Path). |
| [setImageFilter](#SkPaint_setImageFilter) | Sets [Image_Filter](#Image_Filter), alter pixels; blur. |
| [setShader](#SkPaint_setShader) | Sets [Shader](#Shader), multiple drawing colors; gradients. |
| [setStrokeCap](#SkPaint_setStrokeCap) | Sets [Cap](#SkPaint_Cap), the area drawn at path ends. |
| [setStrokeJoin](#SkPaint_setStrokeJoin) | Sets [Join](#SkPaint_Join), geometry on path corners. |
| [setStrokeMiter](#SkPaint_setStrokeMiter) | Sets [Miter_Limit](#Miter_Limit), angles with sharp corners. |
| [setStrokeWidth](#SkPaint_setStrokeWidth) | Sets thickness of the stroke. |
| [setStyle](#SkPaint_setStyle) | Sets [Style](#SkPaint_Style): stroke, fill, or both. |
| [setSubpixelText](#SkPaint_setSubpixelText) | Sets or clears [Subpixel_Text](#Subpixel_Text). |
| [setTextAlign](#SkPaint_setTextAlign) | Sets [Align](#SkPaint_Align): left, center, or right. |
| [setTextEncoding](#SkPaint_setTextEncoding) | Sets character or glyph encoding size. |
| [setTextMatrix](#SkPaint_setTextMatrix) | Returns [Matrix](#Matrix) built from text size, scale, and skew. |
| [setTextScaleX](#SkPaint_setTextScaleX) | Sets the text horizontal scale; condensed text. |
| [setTextSkewX](#SkPaint_setTextSkewX) | Sets the text horizontal skew; oblique text. |
| [setTextSize](#SkPaint_setTextSize) | Sets text size in points. |
| [setTypeface](#SkPaint_setTypeface) | Sets [Typeface](#Typeface), font description. |
| [setVerticalText](#SkPaint_setVerticalText) | Sets or clears [Vertical_Text](#Vertical_Text). |
| [textToGlyphs](#SkPaint_textToGlyphs) | Converts text into glyph indices. |
| [toString](#SkPaint_toString) | Converts [Paint](#Paint) to machine parsable form ([Developer_Mode](#Developer_Mode)) |
| [unflatten](#SkPaint_unflatten) | Populates from a serialized stream. |

## <a name="Paint_Constructor"></a> Paint Constructor
<a name="SkPaint_empty_constructor"></a>
<!--?prettify lang=cc?-->

    SkPaint()

Constructs [Paint](#Paint) with default values.


| attribute | default value |
| --- | ---  |
| [Anti-alias](#Anti-alias) | false |
| [Dither](#Dither) | false |
| [Fake_Bold](#Fake_Bold) | false |
| [Filter_Quality](#Filter_Quality) | [kNone_SkFilterQuality](#SkFilterQuality) |
| [Font_Embedded_Bitmaps](#Font_Embedded_Bitmaps) | false |
| [Forced_Auto-hinting](#Forced_Auto-hinting) | false |
| [Full_Hinting_Spacing](#Full_Hinting_Spacing) | false |
| [LCD_Text](#LCD_Text) | false |
| [Linear_Text](#Linear_Text) | false |
| [Miter_Limit](#Miter_Limit) | 4 |
| [Paint_Alpha](#Paint_Alpha) | 255 |
| [Paint_Blend_Mode](#Paint_Blend_Mode) | [SkBlendMode_kSrcOver](#SkBlendMode_kSrcOver) |
| [Paint_Color](#Paint_Color) | [SK_ColorBLACK](#SK_ColorBLACK) |
| [Paint_Color_Filter](#Paint_Color_Filter) | nullptr |
| [Paint_Draw_Looper](#Paint_Draw_Looper) | nullptr |
| [Paint_Hinting](#Paint_Hinting) | [kNormal_Hinting](#SkPaint_kNormal_Hinting) |
| [Paint_Image_Filter](#Paint_Image_Filter) | nullptr |
| [Paint_Mask_Filter](#Paint_Mask_Filter) | nullptr |
| [Paint_Rasterizer](#Paint_Rasterizer) | nullptr |
| [Paint_Shader](#Paint_Shader) | nullptr |
| [Paint_Style](#Paint_Style) | [kFill_Style](#SkPaint_kFill_Style) |
| [Paint_Text_Align](#Paint_Text_Align) | [kLeft_Align](#SkPaint_kLeft_Align) |
| [Paint_Text_Encoding](#Paint_Text_Encoding) | [kUTF8_TextEncoding](#SkPaint_kUTF8_TextEncoding) |
| [Paint_Text_Scale_X](#Paint_Text_Scale_X) | 1 |
| [Paint_Text_Size](#Paint_Text_Size) | 12 |
| [Paint_Text_Skew_X](#Paint_Text_Skew_X) | 0 |
| [Paint_Typeface](#Paint_Typeface) | nullptr |
| [Path_Effect](#Path_Effect) | nullptr |
| [Stroke_Cap](#Stroke_Cap) | [kButt_Cap](#SkPaint_kButt_Cap) |
| [Stroke_Join](#Stroke_Join) | [kMiter_Join](#SkPaint_kMiter_Join) |
| [Stroke_Width](#Stroke_Width) | 0 |
| [Subpixel_Text](#Subpixel_Text) | false |
| [Vertical_Text](#Vertical_Text) | false |

The flags, text size, hinting, and miter limit may be overridden at compile time by defining
paint default values. The overrides may be included in [SkUserConfig.h](#SkUserConfig.h) or predefined by the 
build system.

#### Example

<fiddle-embed name="9db1cabd02ad20f6e8654de6d697b1bc"></fiddle-embed>

---

<a name="SkPaint_copy_constructor"></a>
<!--?prettify lang=cc?-->

    SkPaint(const SkPaint& paint)

Makes a shallow copy of [Paint](#Paint). [Typeface](#Typeface), [Path_Effect](#Path_Effect), [Shader](#Shader),
[Mask_Filter](#Mask_Filter), [Color_Filter](#Color_Filter), [Rasterizer](#Rasterizer), [Draw_Looper](#Draw_Looper), and [Image_Filter](#Image_Filter) are shared
between the original paint and the copy. These objects' [Reference_Count](#Reference_Count) are increased.

The referenced objects [Path_Effect](#Path_Effect), [Shader](#Shader), [Mask_Filter](#Mask_Filter), [Color_Filter](#Color_Filter), [Rasterizer](#Rasterizer),
[Draw_Looper](#Draw_Looper), and [Image_Filter](#Image_Filter) cannot be modified after they are created.
This prevents objects with [Reference_Count](#Reference_Count) from being modified once [Paint](#Paint) refers to them.

#### Parameters

<table>
  <tr>
    <td><code><strong>paint</strong></code></td> <td>Original identical to the copy.</td>
  </tr>
</table>

#### Example

<fiddle-embed name="2de7fa7ce51a8f560b2951f8f5732cca"></fiddle-embed>

##### Example Output

~~~~
SK_ColorRED == paint1.getColor()
SK_ColorBLUE == paint2.getColor()
~~~~

---

<a name="SkPaint_move_constructor"></a>
<!--?prettify lang=cc?-->

    SkPaint(SkPaint&& paint)

Implements a move constructor to avoid incrementing the reference counts
of objects referenced by the paint.

#### Parameters

<table>
  <tr>
    <td><code><strong>paint</strong></code></td> <td>After the call, undefined except that it can be safely destructed.</td>
  </tr>
</table>

#### Example

<fiddle-embed name="0f3d4d38fa43549c22c2cea35412a31b"></fiddle-embed>

##### Example Output

~~~~
path effect unique: true
~~~~

---

<a name="SkPaint_destructor"></a>
<!--?prettify lang=cc?-->

    ~SkPaint()

Decreases [Paint](#Paint) [Reference_Count](#Reference_Count) of owned objects: [Typeface](#Typeface), [Path_Effect](#Path_Effect), [Shader](#Shader),
[Mask_Filter](#Mask_Filter), [Color_Filter](#Color_Filter), [Rasterizer](#Rasterizer), [Draw_Looper](#Draw_Looper), and [Image_Filter](#Image_Filter). If the
objects' reference count goes to zero, delete them.
---

<a name="SkPaint_copy_assignment_operator"></a>
<!--?prettify lang=cc?-->

    SkPaint& operator=(const SkPaint&)

Makes a shallow copy of [Paint](#Paint). [Typeface](#Typeface), [Path_Effect](#Path_Effect), [Shader](#Shader),
[Mask_Filter](#Mask_Filter), [Color_Filter](#Color_Filter), [Rasterizer](#Rasterizer), [Draw_Looper](#Draw_Looper), and [Image_Filter](#Image_Filter) are shared
between the original paint and the copy. The objects' [Reference_Count](#Reference_Count) are in the
prior destination are decreased by one, and the referenced objects are deleted if the
resulting count is zero. The objects' [Reference_Count](#Reference_Count) in the parameter paint are increased
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

<fiddle-embed name="a33951d3aac20c13eab365dddb06e8e5"></fiddle-embed>

#### Example Output

~~~~
SK_ColorRED == paint1.getColor()
SK_ColorRED == paint2.getColor()
~~~~

---

<a name="SkPaint_move_assignment_operator"></a>
<!--?prettify lang=cc?-->

    SkPaint& operator=(SkPaint&&)

Moves the paint to avoid incrementing the reference counts
of objects referenced by the paint parameter. The objects' [Reference_Count](#Reference_Count) are in the
prior destination are decreased by one, and the referenced objects are deleted if the
resulting count is zero.

### Parameters

<table>
  <tr>
    <td><code><strong>paint</strong></code></td> <td>After the call, undefined except that it can be safely destructed.</td>
  </tr>
</table>

### Return Value

Identical to paint with unmodfied [Reference_Count](#Reference_Count).

### Example

<fiddle-embed name="b178ef2aaf49b5da6ec78b87873ad0c2"></fiddle-embed>

#### Example Output

~~~~
SK_ColorRED == paint2.getColor()
~~~~

---

<a name="SkPaint_equal_operator"></a>
<!--?prettify lang=cc?-->

    bool operator==(const SkPaint& a, const SkPaint& b)

Compares a and b. If it returns true, a and b are equivalent. It may return false
if [Typeface](#Typeface), [Path_Effect](#Path_Effect), [Shader](#Shader), [Mask_Filter](#Mask_Filter), [Color_Filter](#Color_Filter), [Rasterizer](#Rasterizer), [Draw_Looper](#Draw_Looper),
or [Image_Filter](#Image_Filter) have identical contents but different pointers.

### Parameters

<table>
  <tr>
    <td><code><strong>a</strong></code></td> <td><a href="#Paint">Paint</a> to compare.</td>
  </tr>
  <tr>
    <td><code><strong>b</strong></code></td> <td><a href="#Paint">Paint</a> to compare.</td>
  </tr>
</table>

### Return Value

true if paints are equivalent.

### Example

<fiddle-embed name="2ebadcc4709652a1f39981ccfe4f85a6"></fiddle-embed>

#### Example Output

~~~~
paint1 == paint2
paint1 != paint2
~~~~

---

<a name="SkPaint_not_equal_operator"></a>
<!--?prettify lang=cc?-->

    bool operator!=(const SkPaint& a, const SkPaint& b)

Returns true if the paints are not equivalent, but may return true
if [Typeface](#Typeface), [Path_Effect](#Path_Effect), [Shader](#Shader), [Mask_Filter](#Mask_Filter), [Color_Filter](#Color_Filter), [Rasterizer](#Rasterizer), [Draw_Looper](#Draw_Looper),
or [Image_Filter](#Image_Filter) have identical contents but different pointers.

### Parameters

<table>
  <tr>
    <td><code><strong>a</strong></code></td> <td><a href="#Paint">Paint</a> to compare.</td>
  </tr>
  <tr>
    <td><code><strong>b</strong></code></td> <td><a href="#Paint">Paint</a> to compare.</td>
  </tr>
</table>

### Return Value

true if paints are not equivalent.

### Example

<fiddle-embed name="96fb444c9a41da32105af262bfa4413d"></fiddle-embed>

##