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
| [isAntiAlias](#SkPaint_isAntiAlias) | Returns true if [Anti-alias](#Anti_alias) is set. |
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
| [setAntiAlias](#SkPaint_setAntiAlias) | Sets or clears [Anti-alias](#Anti_alias). |
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
| [Anti-alias](#Anti_alias) | false |
| [Dither](#Dither) | false |
| [Fake_Bold](#Fake_Bold) | false |
| [Filter_Quality](#Filter_Quality) | [kNone_SkFilterQuality](#SkFilterQuality) |
| [Font_Embedded_Bitmaps](#Font_Embedded_Bitmaps) | false |
| [Forced_Auto-hinting](#Forced_Auto_hinting) | false |
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

<fiddle-embed name="e1134ec3127c6793ba0c5841e0e2a35e"></fiddle-embed>

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
    <td><code><strong>a</strong></code></td> <td>[Paint](#Paint) to compare.</td>
  </tr>
  <tr>
    <td><code><strong>b</strong></code></td> <td>[Paint](#Paint) to compare.</td>
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
    <td><code><strong>a</strong></code></td> <td>[Paint](#Paint) to compare.</td>
  </tr>
  <tr>
    <td><code><strong>b</strong></code></td> <td>[Paint](#Paint) to compare.</td>
  </tr>
</table>

### Return Value

true if paints are not equivalent.

### Example

<fiddle-embed name="96fb444c9a41da32105af262bfa4413d"></fiddle-embed>

#### Example Output

~~~~
paint1 == paint2
paint1 == paint2
~~~~

---

<a name="SkPaint_getHash"></a>
<!--?prettify lang=cc?-->

    uint32_t getHash() const

Returns a hash generated from [Paint](#Paint) values and pointers.
Identical hashes guarantee that the paints are
equivalent, but differing hashes do not guarantee that the paints have differing
contents.

If [operator==(const SkPaint& a, const SkPaint& b)](#SkPaint_equal_operator) returns true for two paints,
their hashes are also equal.

The hash returned is platform and implementation specific.

### Return Value

A shallow hash.

### Example

<fiddle-embed name="008d3bc3dfe20a442a77c0121e9a4c53"></fiddle-embed>

#### Example Output

~~~~
paint1 == paint2
paint1.getHash() == paint2.getHash()
~~~~

---

<a name="SkPaint_flatten"></a>
<!--?prettify lang=cc?-->

    void flatten(SkWriteBuffer&) const

Serializes [Paint](#Paint) into a buffer. A companion [unflatten](#SkPaint_unflatten) call
can reconstitute the paint at a later time.

### Parameters

<table>
  <tr>
    <td><code><strong>buffer</strong></code></td> <td>[Write_Buffer](#Write_Buffer) receiving the flattened [Paint](#Paint) data.</td>
  </tr>
</table>

### Example

<fiddle-embed name="c0840d7a4a2992f81082639f6c86f7bd"></fiddle-embed>

#### Example Output

~~~~
color = 0xffff0000
~~~~

---

<a name="SkPaint_unflatten"></a>
<!--?prettify lang=cc?-->

    void unflatten(SkReadBuffer&)

Populates [Paint](#Paint), typically from a serialized stream, created by calling
[flatten](#SkPaint_flatten) at an earlier time.

[SkReadBuffer](#SkReadBuffer) class is not public, so [unflatten](#SkPaint_unflatten) cannot be meaningfully called
by the client.

6172---

<a name="SkPaint_reset"></a>
<!--?prettify lang=cc?-->

    void reset()

Sets all paint's contents to their initial values. This is equivalent to replacing
the paint with the result of [SkPaint()](#SkPaint_empty_constructor).

### Example

<fiddle-embed name="af301c5e19c8f5adab8c23e47413923f"></fiddle-embed>

#### Example Output

~~~~
paint1 == paint2
~~~~

---

## <a name="Paint_Hinting"></a> Paint Hinting
### <a name="SkPaint_Hinting"></a> Enum SkPaint::Hinting



[Paint_Hinting](#Paint_Hinting) adjusts the glyph outlines so that the shape provides a uniform
look at a given point size on font engines that support it. [Paint_Hinting](#Paint_Hinting) may have a
muted effect or no effect at all depending on the platform.

The four levels roughly control corresponding features on platforms that use [FreeType](#FreeType)
as their [Font_Engine](#Font_Engine).

#### Constants

<table>
  <tr>
    <td><a name="SkPaint_kNo_Hinting"></a> <code><strong>SkPaint::kNo_Hinting</strong></code></td><td>0</td><td>Leaves glyph outlines unchanged from their native representation.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kSlight_Hinting"></a> <code><strong>SkPaint::kSlight_Hinting</strong></code></td><td>1</td><td>Modifies glyph outlines minimally to improve constrast.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kNormal_Hinting"></a> <code><strong>SkPaint::kNormal_Hinting</strong></code></td><td>2</td><td>Modifies glyph outlines to improve constrast.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kFull_Hinting"></a> <code><strong>SkPaint::kFull_Hinting</strong></code></td><td>3</td><td>Modifies glyph outlines for maxiumum constrast.</td>
  </tr>
</table>

On [Windows](#Windows) with [DirectWrite](#DirectWrite), [Hinting](#SkPaint_Hinting) has no effect.

[Paint_Hinting](#Paint_Hinting) defaults to [kNormal_Hinting](#SkPaint_kNormal_Hinting).
Set [SkPaintDefaults_Hinting](#SkPaintDefaults_Hinting) at compile time to change the default setting.

<a name="SkPaint_getHinting"></a>
<!--?prettify lang=cc?-->

    Hinting getHinting() const

Returns level of glyph outline adjustment.

#### Return Value

One of: [kNo_Hinting](#SkPaint_kNo_Hinting), [kSlight_Hinting](#SkPaint_kSlight_Hinting), [kNormal_Hinting](#SkPaint_kNormal_Hinting), [kFull_Hinting](#SkPaint_kFull_Hinting).

#### Example

<fiddle-embed name="c91cfb5a3767889b802c577ebfd73221"></fiddle-embed>

##### Example Output

~~~~
SkPaint::kNormal_Hinting == paint.getHinting()
~~~~

---

<a name="SkPaint_setHinting"></a>
<!--?prettify lang=cc?-->

    void setHinting(Hinting hintingLevel) const

Sets level of glyph outline adjustment.
Does not check for valid values of hintingLevel.

#### Parameters

<table>
  <tr>
    <td><code><strong>hintingLevel</strong></code></td> <td>One of: [kNo_Hinting](#SkPaint_kNo_Hinting), [kSlight_Hinting](#SkPaint_kSlight_Hinting), [kNormal_Hinting](#SkPaint_kNormal_Hinting), [kFull_Hinting](#SkPaint_kFull_Hinting).</td>
  </tr>
</table>

#### Example

<fiddle-embed name="39a0706eaf1135f09657265ea8ad1d37"></fiddle-embed>

##### Example Output

~~~~
paint1 == paint2
~~~~

---

## <a name="Paint_Flags"></a> Paint Flags
### <a name="SkPaint_Flags"></a> Enum SkPaint::Flags



The bit values stored in [Paint_Flags](#Paint_Flags).
The default value for [Flags](#SkPaint_Flags), normally zero, can be changed at compile time
with a custom definition of [SkPaintDefaults_Flags](#SkPaintDefaults_Flags).
All flags can be read and written explicitly; [Flags](#SkPaint_Flags) allows manipulating
multiple settings at once.

#### Constants

<table>
  <tr>
    <td><a name="SkPaint_kAntiAlias_Flag"></a> <code><strong>SkPaint::kAntiAlias_Flag</strong></code></td><td>0x0001 </td><td>Controls if [Anti-alias](#Anti_alias) is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kDither_Flag"></a> <code><strong>SkPaint::kDither_Flag</strong></code></td><td>0x0004</td><td>Controls if [Dither](#Dither) is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kUnderlineText_Flag"></a> <code><strong>SkPaint::kUnderlineText_Flag</strong></code></td><td>0x0008</td><td>Deprecated.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kStrikeThruText_Flag"></a> <code><strong>SkPaint::kStrikeThruText_Flag</strong></code></td><td>0x1000</td><td>Deprecated.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kFakeBoldText_Flag"></a> <code><strong>SkPaint::kFakeBoldText_Flag</strong></code></td><td>0x0020</td><td>Controls if [Fake_Bold](#Fake_Bold) is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kLinearText_Flag"></a> <code><strong>SkPaint::kLinearText_Flag</strong></code></td><td>0x0040</td><td>Controls if [Linear_Text](#Linear_Text) is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kSubpixelText_Flag"></a> <code><strong>SkPaint::kSubpixelText_Flag</strong></code></td><td>0x0080</td><td>Controls if [Subpixel_Text](#Subpixel_Text) is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kDevKernText_Flag"></a> <code><strong>SkPaint::kDevKernText_Flag</strong></code></td><td>0x0100</td><td>Controls if [Full_Hinting_Spacing](#Full_Hinting_Spacing) is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kLCDRenderText_Flag"></a> <code><strong>SkPaint::kLCDRenderText_Flag</strong></code></td><td>0x0200</td><td>Controls if [LCD_Text](#LCD_Text) is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kEmbeddedBitmapText_Flag"></a> <code><strong>SkPaint::kEmbeddedBitmapText_Flag</strong></code></td><td>0x0400</td><td>Controls if [Font_Embedded_Bitmaps](#Font_Embedded_Bitmaps) is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kAutoHinting_Flag"></a> <code><strong>SkPaint::kAutoHinting_Flag</strong></code></td><td>0x0800</td><td>Controls if [Forced_Auto-hinting](#Forced_Auto_hinting) is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kVerticalText_Flag"></a> <code><strong>SkPaint::kVerticalText_Flag</strong></code></td><td>0x1000</td><td>Controls if [Vertical_Text](#Vertical_Text) is enabled or disabled.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kGenA8FromLCD_Flag"></a> <code><strong>SkPaint::kGenA8FromLCD_Flag</strong></code></td><td>0x2000</td><td>Not intended for public use.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kAllFlags"></a> <code><strong>SkPaint::kAllFlags</strong></code></td><td>0xFFFF</td><td>Masks the range of [Paint_Flags](#Paint_Flags), including private flags and flags
reserved for future use.</td>
  </tr>
[Paint_Flags](#Paint_Flags) default to all flags clear, disabling the associated feature.
</table>

### <a name="SkPaint_ReserveFlags"></a> Enum SkPaint::ReserveFlags



#### Constants

<table>
  <tr>
    <td><a name="SkPaint_kUnderlineText_ReserveFlag"></a> <code><strong>SkPaint::kUnderlineText_ReserveFlag</strong></code></td><td>0x0008</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPaint_kStrikeThruText_ReserveFlag"></a> <code><strong>SkPaint::kStrikeThruText_ReserveFlag</strong></code></td><td>0x0010</td><td></td>
  </tr>
</table>

<a name="SkPaint_getFlags"></a>
<!--?prettify lang=cc?-->

    uint32_t getFlags() const

Returns paint settings described by [Flags](#SkPaint_Flags). Each setting uses one
bit, and can be tested with [Flags](#SkPaint_Flags) members.

#### Return Value

Zero, one, or more bits described by [Flags](#SkPaint_Flags).

#### Example

<fiddle-embed name="e95d09086fe8a862f78087e5fff070a4"></fiddle-embed>

##### Example Output

~~~~
(SkPaint::kAntiAlias_Flag & paint.getFlags()) != 0
~~~~

---

<a name="SkPaint_setFlags"></a>
<!--?prettify lang=cc?-->

    void setFlags(uint32_t flags)

Sets [Paint_Flags](#Paint_Flags) to the union of the [Flags](#SkPaint_Flags) members.

#### Parameters

<table>
  <tr>
    <td><code><strong>flags</strong></code></td> <td>Union of [Flags](#SkPaint_Flags) for [Paint](#Paint).</td>
  </tr>
</table>

#### Example

<fiddle-embed name="3c209da57ee0acba480ba05ade14b6d7"></fiddle-embed>

##### Example Output

~~~~
paint.isAntiAlias()
paint.isDither()
~~~~

---

## <a name="Anti-alias"></a> Anti-alias
[Anti-alias](#Anti_alias) drawing approximates partial pixel coverage with transparency.
If [kAntiAlias_Flag](#SkPaint_kAntiAlias_Flag) is clear, pixel centers contained by the shape edge are drawn opaque.
If [kAntiAlias_Flag](#SkPaint_kAntiAlias_Flag) is set, pixels are drawn with [Color_Alpha](#Color_Alpha) equal to their coverage.

The rule for aliased pixels is inconsistent across platforms. A shape edge 
passing through the pixel center may, but is not required to, draw the pixel.

[Raster_Engine](#Raster_Engine) draws aliased pixels whose centers are on or to the right of the start of an
active [Path](#Path) edge, and whose center is to the left of the end of the active [Path](#Path) edge.

A platform may only support anti-aliased drawing. Some [GPU-backed](#GPU-backed) platforms use
supersampling to anti-alias all drawing, and have no mechanism to selectively
alias.

The amount of coverage computed for anti-aliased pixels also varies across platforms.

[Anti-alias](#Anti_alias) is disabled by default.
[Anti-alias](#Anti_alias) can be enabled by default by setting [SkPaintDefaults_Flags](#SkPaintDefaults_Flags) to [kAntiAlias_Flag](#SkPaint_kAntiAlias_Flag)
at compile time.

### Example

<fiddle-embed name="4f7972bc27854e447dacd9a7e404fe3c"></fiddle-embed>

512A red line is drawn with transparency on the edges to make it look smoother.
A blue line draws only where the pixel centers are contained.
The lines are drawn into an offscreen bitmap, then drawn magified to make the
aliasing easier to see.<a name="SkPaint_isAntiAlias"></a>
<!--?prettify lang=cc?-->

    bool isAntiAlias() const

If true, pixels on the active edges of [Path](#Path) may be drawn with partial transparency.

Equivalent to [getFlags](#SkPaint_getFlags) masked with [kAntiAlias_Flag](#SkPaint_kAntiAlias_Flag).

#### Return Value

[kAntiAlias_Flag](#SkPaint_kAntiAlias_Flag) state.

#### Example

<fiddle-embed name="aecda6fe10e4a19b2574f1f2fb80d437"></fiddle-embed>

##### Example Output

~~~~
paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)
paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)
~~~~

---

<a name="SkPaint_setAntiAlias"></a>
<!--?prettify lang=cc?-->

    void setAntiAlias(bool aa)

Requests, but does not require, that [Path](#Path) edge pixels draw opaque or with
partial transparency.

Sets [kAntiAlias_Flag](#SkPaint_kAntiAlias_Flag) if aa is true.
Clears [kAntiAlias_Flag](#SkPaint_kAntiAlias_Flag) if aa is false.

#### Parameters

<table>
  <tr>
    <td><code><strong>aa</strong></code></td> <td>Sets or clears [kAntiAlias_Flag](#SkPaint_kAntiAlias_Flag).</td>
  </tr>
</table>

#### Example

<fiddle-embed name="231c9631061232096c3d7bd94ddd5c27"></fiddle-embed>

##### Example Output

~~~~
paint1 == paint2
~~~~

---

## <a name="Dither"></a> Dither
[Dither](#Dither) distributes color error to smooth color transitions.
Dithering removes visible banding from gradients and improves rendering into
a [kRGB_565_SkColorType](#SkColorType) [Surface](#Surface).

6224
[Dither](#Dither) is ignored if the gradient has two colors.[Dither](#Dither) is disabled by default.
[Dither](#Dither) can be enabled by default by setting [SkPaintDefaults_Flags](#SkPaintDefaults_Flags) to [kDither_Flag](#SkPaint_kDither_Flag)
at compile time.

Some platform implementations may ignore dithering.

### Example

<fiddle-embed name="b28987b65d3be14717e4faaacaf2faa1"></fiddle-embed>

<a name="SkPaint_isDither"></a>
<!--?prettify lang=cc?-->

    bool isDither() const

If true, color error may be distributed to smooth color transition.
 
Equivalent to [getFlags](#SkPaint_getFlags) masked with [kDither_Flag](#SkPaint_kDither_Flag).

#### Return Value

[kDither_Flag](#SkPaint_kDither_Flag) state.

#### Example

<fiddle-embed name="87f9538060779de8a0284d9227bdf91a"></fiddle-embed>

##### Example Output

~~~~
paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag)
paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag)
~~~~

---

<a name="SkPaint_setDither"></a>
<!--?prettify lang=cc?-->

    void setDither(bool dither)

Suggests, but does not require, to distribute color error.

Sets [kDither_Flag](#SkPaint_kDither_Flag) if dither is true.
Clears [kDither_Flag](#SkPaint_kDither_Flag) if dither is false.

#### Parameters

<table>
  <tr>
    <td><code><strong>dither</strong></code></td> <td>Sets or clears [kDither_Flag](#SkPaint_kDither_Flag).</td>
  </tr>
</table>

#### Example

<fiddle-embed name="4c7a25e4a7a314d84147a399763c9773"></fiddle-embed>

##### Example Output

~~~~
paint1 == paint2
~~~~

#### See Also

[kRGB_565_SkColorType](#SkColorType)---

### See Also

[Gradient](#Gradient) [Color_RGB-565](#Color_RGB_565)## <a name="Paint_Device_Text"></a> Paint Device Text
[LCD_Text](#LCD_Text) and [Subpixel_Text](#Subpixel_Text) increase the precision of glyph position.

When set, [Flags](#SkPaint_Flags) [kLCDRenderText_Flag](#SkPaint_kLCDRenderText_Flag) takes advantage of the organization of [Color_RGB](#Color_RGB) stripes that 
create a color, and relies
on the small size of the stripe and visual perception to make the color fringing inperceptible.
[LCD_Text](#LCD_Text) can be enabled on devices that orient stripes horizontally or vertically, and that order
the color components as [Color_RGB](#Color_RGB) or [Color_RBG](#Color_RBG).

[Flags](#SkPaint_Flags) [kSubpixelText_Flag](#SkPaint_kSubpixelText_Flag) uses the pixel transparency to represent a fractional offset. 
As the opaqueness
of the color increases, the edge of the glyph appears to move towards the outside of the pixel.

Either or both techniques can be enabled.
[kLCDRenderText_Flag](#SkPaint_kLCDRenderText_Flag) and [kSubpixelText_Flag](#SkPaint_kSubpixelText_Flag) are clear by default.
[LCD_Text](#LCD_Text) or [Subpixel_Text](#Subpixel_Text) can be enabled by default by setting [SkPaintDefaults_Flags](#SkPaintDefaults_Flags) to 
[kLCDRenderText_Flag](#SkPaint_kLCDRenderText_Flag) or [kSubpixelText_Flag](#SkPaint_kSubpixelText_Flag) (or both) at compile time.

### Example

<fiddle-embed name="525062c7023c2c5f2f962a09baad75eb"></fiddle-embed>

Four commas are drawn normally and with combinations of [LCD_Text](#LCD_Text) and [Subpixel_Text](#Subpixel_Text).
When [Subpixel_Text](#Subpixel_Text) is disabled, the comma glyphs are indentical, but not evenly spaced.
When [Subpixel_Text](#Subpixel_Text) is enabled, the comma glyphs are unique, but appear evenly spaced.### <a name="Linear_Text"></a> Linear Text
[Linear_Text](#Linear_Text) appears to have no effect, and has been superceded by setting [Hinting](#SkPaint_Hinting) to [kNo_Hinting](#SkPaint_kNo_Hinting).
If that's not the case, document here.<a name="SkPaint_isLinearText"></a>
<!--?prettify lang=cc?-->

    bool isLinearText() const

---

<a name="SkPaint_setLinearText"></a>
<!--?prettify lang=cc?-->

    void setLinearText(bool linearText)

---

### <a name="Subpixel_Text"></a> Subpixel Text
[Flags](#SkPaint_Flags) [kSubpixelText_Flag](#SkPaint_kSubpixelText_Flag) uses the pixel transparency to represent a fractional offset. 
As the opaqueness
of the color increases, the edge of the glyph appears to move towards the outside of the pixel.

<a name="SkPaint_isSubpixelText"></a>
<!--?prettify lang=cc?-->

    bool isSubpixelText() const

If true, glyphs at different sub-pixel positions may differ on pixel edge coverage.

Equivalent to [getFlags](#SkPaint_getFlags) masked with [kSubpixelText_Flag](#SkPaint_kSubpixelText_Flag).

##### Return Value

[kSubpixelText_Flag](#SkPaint_kSubpixelText_Flag) state.

##### Example

<fiddle-embed name="fc6e1fa6b9a383e6529211dc773a464d"></fiddle-embed>

###### Example Output

~~~~
paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)
paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)
~~~~

---

<a name="SkPaint_setSubpixelText"></a>
<!--?prettify lang=cc?-->

    void setSubpixelText(bool subpixelText)

Requests, but does not require, that glyphs respect sub-pixel positioning.

Sets [kSubpixelText_Flag](#SkPaint_kSubpixelText_Flag) if subpixelText is true.
Clears [kSubpixelText_Flag](#SkPaint_kSubpixelText_Flag) if subpixelText is false.

##### Parameters

<table>
  <tr>
    <td><code><strong>subpixelText</strong></code></td> <td>Sets or clears [kSubpixelText_Flag](#SkPaint_kSubpixelText_Flag).</td>
  </tr>
</table>

##### Example

<fiddle-embed name="17270e681eb6931258acdbbbbefcc17c"></fiddle-embed>

###### Example Output

~~~~
paint1 == paint2
~~~~

---

### <a name="LCD_Text"></a> LCD Text
When set, [Flags](#SkPaint_Flags) [kLCDRenderText_Flag](#SkPaint_kLCDRenderText_Flag) takes advantage of the organization of [Color_RGB](#Color_RGB) stripes that 
create a color, and relies
on the small size of the stripe and visual perception to make the color fringing inperceptible.
[LCD_Text](#LCD_Text) can be enabled on devices that orient stripes horizontally or vertically, and that order
the color components as [Color_RGB](#Color_RGB) or [Color_RBG](#Color_RBG).

<a name="SkPaint_isLCDRenderText"></a>
<!--?prettify lang=cc?-->

    bool isLCDRenderText() const

If true, glyphs may use [LCD](#LCD) striping to improve glyph edges.

Returns true if [Flags](#SkPaint_Flags) [kLCDRenderText_Flag](#SkPaint_kLCDRenderText_Flag) is set.

##### Return Value

[kLCDRenderText_Flag](#SkPaint_kLCDRenderText_Flag) state.

##### Example

<fiddle-embed name="bf834391d2837b242c889c0591e828c8"></fiddle-embed>

###### Example Output

~~~~
paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)
paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)
~~~~

---

<a name="SkPaint_setLCDRenderText"></a>
<!--?prettify lang=cc?-->

    void setLCDRenderText(bool lcdText)

Requests, but does not require, that glyphs use [LCD](#LCD) striping for glyph edges.

Sets [kLCDRenderText_Flag](#SkPaint_kLCDRenderText_Flag) if lcdText is true.
Clears [kLCDRenderText_Flag](#SkPaint_kLCDRenderText_Flag) if lcdText is false.

##### Parameters

<table>
  <tr>
    <td><code><strong>lcdText</strong></code></td> <td>Sets or clears [kLCDRenderText_Flag](#SkPaint_kLCDRenderText_Flag).</td>
  </tr>
</table>

##### Example

<fiddle-embed name="50f5dc9c5337271fc088aab9f6e73bbf"></fiddle-embed>

###### Example Output

~~~~
paint1 == paint2
~~~~

---

## <a name="Font_Embedded_Bitmaps"></a> Font Embedded Bitmaps
[Font_Embedded_Bitmaps](#Font_Embedded_Bitmaps) allows selecting custom-sized bitmap glyphs.
[Flags](#SkPaint_Flags) [kEmbeddedBitmapText_Flag](#SkPaint_kEmbeddedBitmapText_Flag) when set chooses an embedded bitmap glyph over an outline contained
in a font if the platform supports this option. 

[FreeType](#FreeType) selects the bitmap glyph if available when [kEmbeddedBitmapText_Flag](#SkPaint_kEmbeddedBitmapText_Flag) is set, and selects
the outline glyph if [kEmbeddedBitmapText_Flag](#SkPaint_kEmbeddedBitmapText_Flag) is clear.
[Windows](#Windows) may select the bitmap glyph but is not required to do so.
[OS_X](#OS_X) and iOS do not support this option.

[Font_Embedded_Bitmaps](#Font_Embedded_Bitmaps) is disabled by default.
[Font_Embedded_Bitmaps](#Font_Embedded_Bitmaps) can be enabled by default by setting [SkPaintDefaults_Flags](#SkPaintDefaults_Flags) to
[kEmbeddedBitmapText_Flag](#SkPaint_kEmbeddedBitmapText_Flag) at compile time.

### Example

<fiddle-embed name=""></fiddle-embed>

The hintgasp [TrueType](#TrueType) font in the [Skia](#Skia) resources/fonts directory includes an embedded
bitmap glyph at odd font sizes. This example works on platforms that use [FreeType](#FreeType)
as their [Font_Engine](#Font_Engine).
[Windows](#Windows) may, but is not required to, return a bitmap glyph if [kEmbeddedBitmapText_Flag](#SkPaint_kEmbeddedBitmapText_Flag) is set.embeddedbitmap.png<a name="SkPaint_isEmbeddedBitmapText"></a>
<!--?prettify lang=cc?-->

    bool isEmbeddedBitmapText() const

If true, [Font_Engine](#Font_Engine) may return glyphs from font bitmaps instead of from outlines.

Equivalent to [getFlags](#SkPaint_getFlags) masked with [kEmbeddedBitmapText_Flag](#SkPaint_kEmbeddedBitmapText_Flag).

#### Return Value

[kEmbeddedBitmapText_Flag](#SkPaint_kEmbeddedBitmapText_Flag) state.

#### Example

<fiddle-embed name="3f4c8fd210a7d276871a11a1b330f401"></fiddle-embed>

##### Example Output

~~~~
paint.isEmbeddedBitmapText() == !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)
paint.isEmbeddedBitmapText() == !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)
~~~~

---

<a name="SkPaint_setEmbeddedBitmapText"></a>
<!--?prettify lang=cc?-->

    void setEmbeddedBitmapText(bool useEmbeddedBitmapText)

Requests, but does not require, to use bitmaps in fonts instead of outlines.

Sets [kEmbeddedBitmapText_Flag](#SkPaint_kEmbeddedBitmapText_Flag) if useEmbeddedBitmapText is true.
Clears [kEmbeddedBitmapText_Flag](#SkPaint_kEmbeddedBitmapText_Flag) if useEmbeddedBitmapText is false.

#### Parameters

<table>
  <tr>
    <td><code><strong>aa</strong></code></td> <td>Sets or clears [kEmbeddedBitmapText_Flag](#SkPaint_kEmbeddedBitmapText_Flag).</td>
  </tr>
</table>

#### Example

<fiddle-embed name="4d0c99d7d4a6f39d3c51a1be09060e48"></fiddle-embed>

##### Example Output

~~~~
paint1 == paint2
~~~~

---

## <a name="Forced_Auto-hinting"></a> Forced Auto-hinting
[Forced_Auto-hinting](#Forced_Auto_hinting) no longer has any effect.
Set [Hinting](#SkPaint_Hinting) to [kNo_Hinting](#SkPaint_kNo_Hinting) to leave the font outlines unhinted instead.<a name="SkPaint_isAutohinted"></a>
<!--?prettify lang=cc?-->

    bool isAutohinted() const

Has no effect.
---

<a name="SkPaint_setAutohinted"></a>
<!--?prettify lang=cc?-->

    void setAutohinted(bool useAutohinter)

Has no effect.
---

## <a name="Vertical_Text"></a> Vertical Text
[Text](#Text) may be drawn by positioning each glyph, or by positioning the first glyph and
using [Font_Advance](#Font_Advance) to position subsequent glyphs. By default, each successive glyph
is positioned to the right of the preceeding glyph. [Vertical_Text](#Vertical_Text) sets successive
glyphs to position below the preceeding glyph.

[Skia](#Skia) can translate text character codes as a series of glyphs, but does not implement
font substitution, 
textual substitution, line layout, or contextual spacing like kerning pairs. Use
a text shaping engine likeHarfBuzz #http://harfbuzz.org/to translate text runs
into glyph series.

[Vertical_Text](#Vertical_Text) is clear if text is drawn left to right or set if drawn from top to bottom.

[Flags](#SkPaint_Flags) [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) if clear draws text left to right.
[Flags](#SkPaint_Flags) [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) if set draws text top to bottom.

[Vertical_Text](#Vertical_Text) is clear by default.
[Vertical_Text](#Vertical_Text) can be set by default by setting [SkPaintDefaults_Flags](#SkPaintDefaults_Flags) to
[kVerticalText_Flag](#SkPaint_kVerticalText_Flag) at compile time.

### Example

<fiddle-embed name="3c4995e97b7d0d0afeb718e44f9472c0"></fiddle-embed>

<a name="SkPaint_isVerticalText"></a>
<!--?prettify lang=cc?-->

    bool isVerticalText() const

If true, glyphs are drawn top to bottom instead of left to right.

Equivalent to [getFlags](#SkPaint_getFlags) masked with [kVerticalText_Flag](#SkPaint_kVerticalText_Flag).

#### Return Value

[kVerticalText_Flag](#SkPaint_kVerticalText_Flag) state.

#### Example

<fiddle-embed name="8de09d99eebade65c6446d8bfc980fe1"></fiddle-embed>

##### Example Output

~~~~
paint.isVerticalText() == !!(paint.getFlags() & SkPaint::kVerticalText_Flag)
paint.isVerticalText() == !!(paint.getFlags() & SkPaint::kVerticalText_Flag)
~~~~

---

<a name="SkPaint_setVerticalText"></a>
<!--?prettify lang=cc?-->

    void setVerticalText(bool vertical)

If true, text advance positions the next glyph below the previous glyph instead of to the
right of previous glyph.

Sets [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) if vertical is true.
Clears [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) if vertical is false.

#### Parameters

<table>
  <tr>
    <td><code><strong>vertical</strong></code></td> <td>Sets or clears [kVerticalText_Flag](#SkPaint_kVerticalText_Flag).</td>
  </tr>
</table>

#### Example

<fiddle-embed name="4bc82860cbf0e9a9d2cc2627879ecc7f"></fiddle-embed>

##### Example Output

~~~~
paint1 == paint2
~~~~

---

## <a name="Paint_Text_Decorations"></a> Paint Text Decorations
<a name="SkPaint_isUnderlineText"></a>
<!--?prettify lang=cc?-->

    bool isUnderlineText() const

requires [SK_SUPPORT_LEGACY_PAINT_TEXTDECORATION](#SK_SUPPORT_LEGACY_PAINT_TEXTDECORATION)
unsure if/how to document this---

<a name="SkPaint_isStrikeThruText"></a>
<!--?prettify lang=cc?-->

    bool isStrikeThruText() const { return false; }

requires [SK_SUPPORT_LEGACY_PAINT_TEXTDECORATION](#SK_SUPPORT_LEGACY_PAINT_TEXTDECORATION)
unsure if/how to document this---

### <a name="Fake_Bold"></a> Fake Bold
[Fake_Bold](#Fake_Bold) approximates the bold font style accompanying a normal font when a bold font face
is not available. [Skia](#Skia) does not provide font substitution; it is up to the client to find the
bold font face using the platform's [Font_Manager](#Font_Manager).

Use [Paint_Text_Skew_X](#Paint_Text_Skew_X) to approximate an italic font style when the italic font face 
is not available.

A [FreeType-based](#FreeType-based) port may define [SK_USE_FREETYPE_EMBOLDEN](#SK_USE_FREETYPE_EMBOLDEN) at compile time to direct
the font engine to create the bold glyphs. Otherwise, the extra bold is computed
by increasing the stroke width and setting the [Style](#SkPaint_Style) to [kStrokeAndFill_Style](#SkPaint_kStrokeAndFill_Style) as needed.  

[Fake_Bold](#Fake_Bold) is disabled by default.

#### Example

<fiddle-embed name="c68a5108ac9e8945c30bf215549599b2"></fiddle-embed>

<a name="SkPaint_isFakeBoldText"></a>
<!--?prettify lang=cc?-->

    bool isFakeBoldText() const;

If true, approximate bold by increasing the stroke width when creating glyph bitmaps
from outlines.

Equivalent to [getFlags](#SkPaint_getFlags) masked with [kFakeBoldText_Flag](#SkPaint_kFakeBoldText_Flag).

##### Return Value

[kFakeBoldText_Flag](#SkPaint_kFakeBoldText_Flag) state.

##### Example

<fiddle-embed name="bc4ff096643bd3a132429f68d410492e"></fiddle-embed>

###### Example Output

~~~~
paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)
paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)
~~~~

---

<a name="SkPaint_setFakeBoldText"></a>
<!--?prettify lang=cc?-->

    void setFakeBoldText(bool fakeBoldText)

Use increased stroke width when creating glyph bitmaps to approximate bolding.

Sets [kFakeBoldText_Flag](#SkPaint_kFakeBoldText_Flag) if fakeBoldText is true.
Clears [kFakeBoldText_Flag](#SkPaint_kFakeBoldText_Flag) if fakeBoldText is false.

##### Parameters

<table>
  <tr>
    <td><code><strong>fakeBoldText</strong></code></td> <td>Sets or clears [kFakeBoldText_Flag](#SkPaint_kFakeBoldText_Flag).</td>
  </tr>
</table>

##### Example

<fiddle-embed name="6e6982855b1a836762050e6b0bd5f618"></fiddle-embed>

###### Example Output

~~~~
paint1 == paint2
~~~~

---

## <a name="Full_Hinting_Spacing"></a> Full Hinting Spacing
[Full_Hinting_Spacing](#Full_Hinting_Spacing) adjusts the character spacing by the difference of the 
hinted and unhinted left and right side bearings, 
if [Hinting](#SkPaint_Hinting) is set to [kFull_Hinting](#SkPaint_kFull_Hinting). [Full_Hinting_Spacing](#Full_Hinting_Spacing) only
applies to platforms that use [FreeType](#FreeType) as their [Font_Engine](#Font_Engine).

[Full_Hinting_Spacing](#Full_Hinting_Spacing) is not related to text kerning, where the space between
a specific pair of characters is adjusted using data in the font's kerning tables.

<a name="SkPaint_isDevKernText"></a>
<!--?prettify lang=cc?-->

    bool isDevKernText() const

Returns if character spacing may be adjusted by the hinting difference.

Equivalent to [getFlags](#SkPaint_getFlags) masked with [kDevKernText_Flag](#SkPaint_kDevKernText_Flag).

#### Return Value

[kDevKernText_Flag](#SkPaint_kDevKernText_Flag) state.

#### Example

<fiddle-embed name="2a0016c5875bf070d18508457143fb16"></fiddle-embed>

---

<a name="SkPaint_setDevKernText"></a>
<!--?prettify lang=cc?-->

    void setDevKernText(bool devKernText)

Requests, but does not require, to use hinting to adjust glyph spacing.

Sets [kDevKernText_Flag](#SkPaint_kDevKernText_Flag) if devKernText is true.
Clears [kDevKernText_Flag](#SkPaint_kDevKernText_Flag) if devKernText is false.

#### Parameters

<table>
  <tr>
    <td><code><strong>devKernText</strong></code></td> <td>Sets or clears devKernText.</td>
  </tr>
</table>

#### Example

<fiddle-embed name="c00e36f5f3c49317c622e88e97d334a3"></fiddle-embed>

##### Example Output

~~~~
paint1 == paint2
~~~~

---

## <a name="Paint_Filter_Quality"></a> Paint Filter Quality
[Paint_Filter_Quality](#Paint_Filter_Quality) trades speed for image filtering when the image is scaled.
A lower [Filter_Quality](#Filter_Quality) draws faster, but has less fidelity.
A higher [Filter_Quality](#Filter_Quality) draws slower, but looks better.
If the image is unscaled, the [Filter_Quality](#Filter_Quality) choice will not result in a noticable
difference.

[Filter_Quality](#Filter_Quality) is used in [Paint](#Paint) passed as a parameter to[SkCanvas](#SkCanvas)::drawBitmap
[SkCanvas](#SkCanvas)::drawBitmapRect
[SkCanvas](#SkCanvas)::drawImage
[SkCanvas](#SkCanvas)::drawImageRectand when [Paint](#Paint) has a [Shader](#Shader) specialization that uses [Image](#Image) or [Bitmap](#Bitmap).

[Filter_Quality](#Filter_Quality) is [kNone_SkFilterQuality](#SkFilterQuality) by default.

### Example

<fiddle-embed name="97d214c30dcacf9f13a1487d5d2b377d"></fiddle-embed>

<a name="SkPaint_getFilterQuality"></a>
<!--?prettify lang=cc?-->

    SkFilterQuality getFilterQuality() const

A lower setting
draws faster; a higher setting looks better when the image is scaled.

#### Return Value

One of: [kNone_SkFilterQuality](#SkFilterQuality), [kLow_SkFilterQuality](#SkFilterQuality), 
                 [kMedium_SkFilterQuality](#SkFilterQuality), [kHigh_SkFilterQuality](#SkFilterQuality).

#### Example

<fiddle-embed name="ba511d61d6149608616b8dc9af296b23"></fiddle-embed>

##### Example Output

~~~~
kNone_SkFilterQuality == paint.getFilterQuality()
~~~~

---

<a name="SkPaint_setFilterQuality"></a>
<!--?prettify lang=cc?-->

    void setFilterQuality(SkFilterQuality quality)

A lower setting
draws faster; a higher setting looks better when the image is scaled.

#### Parameters

<table>
  <tr>
    <td><code><strong>quality</strong></code></td> <td>One of: [kNone_SkFilterQuality](#SkFilterQuality), [kLow_SkFilterQuality](#SkFilterQuality), 
                 [kMedium_SkFilterQuality](#SkFilterQuality), [kHigh_SkFilterQuality](#SkFilterQuality).
                 Not checked to see if quality is valid.</td>
  </tr>
</table>

#### Example

<fiddle-embed name="9385d0ef925a17fd1eb5651b816e496e"></fiddle-embed>

##### Example Output

~~~~
kHigh_SkFilterQuality == paint.getFilterQuality()
~~~~

#### See Also

[SkFilterQuality](#SkFilterQuality) [Image_Scaling](#Image_Scaling)---

## <a name="Paint_Color"></a> Paint Color
[Paint_Color](#Paint_Color) specifies the [Color_Red](#Color_Red), [Color_Blue](#Color_Blue), [Color_Green](#Color_Green), and [Color_Alpha](#Color_Alpha) values used to draw a filled
or stroked shape in a
32-bit value. Each component occupies 8-bits, ranging from zero: no contribution;
to 255: full intensity. All values in any combination are valid.

[Paint_Color](#Paint_Color) is not premultiplied;
[Paint_Alpha](#Paint_Alpha) sets the transparency independent of [Color_RGB](#Color_RGB): [Color_Red](#Color_Red), [Color_Blue](#Color_Blue), and [Color_Green](#Color_Green).

The bit positions of [Color_Alpha](#Color_Alpha) and [Color_RGB](#Color_RGB) are independent of the bit positions
on the output device, which may have more or fewer bits, and may have a different arrangement.


| bit positions | [Color_Alpha](#Color_Alpha) | [Color_Red](#Color_Red) | [Color_Blue](#Color_Blue) | [Color_Green](#Color_Green) |
| --- | --- | --- | --- | ---  |
|  | 31 - 24 | 23 - 16 | 15 - 8 | 7 - 0 |

### Example

<fiddle-embed name="f6eec81567acaca6638d696ad00cfb5d"></fiddle-embed>

<a name="SkPaint_getColor"></a>
<!--?prettify lang=cc?-->

    SkColor getColor() const

Retrieves [Color_Alpha](#Color_Alpha) and [Color_RGB](#Color_RGB), unpremultiplied, packed into 32 bits.
Use helpers [SkColorGetA](#SkColorGetA), [SkColorGetR](#SkColorGetR), [SkColorGetG](#SkColorGetG), and [SkColorGetB](#SkColorGetB) to extract
a color component.

#### Return Value

Unpremultiplied [Color_ARGB](#Color_ARGB).

#### Example

<fiddle-embed name="896fd0df04c096398b5d5d7d47061cdf"></fiddle-embed>

##### Example Output

~~~~
Yellow is 100% red, 100% green, and 0% blue.
~~~~

#### See Also

[SkColor](#SkColor)---

<a name="SkPaint_setColor"></a>
<!--?prettify lang=cc?-->

    void setColor(SkColor color)

Sets [Color_Alpha](#Color_Alpha) and [Color_RGB](#Color_RGB) used when stroking and filling. The color is a 32-bit value,
unpremutiplied, packing 8-bit components for [Color_Alpha](#Color_Alpha), [Color_Red](#Color_Red), [Color_Blue](#Color_Blue), and [Color_Green](#Color_Green). 

#### Parameters

<table>
  <tr>
    <td><code><strong>color</strong></code></td> <td>Unpremultiplied [Color_ARGB](#Color_ARGB).</td>
  </tr>
</table>

#### Example

<fiddle-embed name="be988544a8f36325c3480cf679ae021a"></fiddle-embed>

##### Example Output

~~~~
green1 == green2
~~~~

#### See Also

[SkColor](#SkColor) [setARGB](#SkPaint_setARGB) [SkColorSetARGB](#SkColorSetARGB)---

### <a name="Paint_Alpha"></a> Paint Alpha
[Paint_Alpha](#Paint_Alpha) sets the transparency independent of [Color_RGB](#Color_RGB): [Color_Red](#Color_Red), [Color_Blue](#Color_Blue), and [Color_Green](#Color_Green).

<a name="SkPaint_getAlpha"></a>
<!--?prettify lang=cc?-->

    uint8_t getAlpha() const

Retrieves [Color_Alpha](#Color_Alpha) from the [Paint_Color](#Paint_Color) used when stroking and filling.

##### Return Value

[Color_Alpha](#Color_Alpha) ranging from zero, fully transparent, to 255, fully opaque.

##### Example

<fiddle-embed name="2b1f43ba1731db56cccd04172c62bb84"></fiddle-embed>

###### Example Output

~~~~
255 == paint.getAlpha()
~~~~

---

<a name="SkPaint_setAlpha"></a>
<!--?prettify lang=cc?-->

    void setAlpha(U8CPU a)

Replaces [Color_Alpha](#Color_Alpha), leaving [Color_RGB](#Color_RGB) 
unchanged. An out of range value triggers an assert in the debug
build.

##### Parameters

<table>
  <tr>
    <td><code><strong>a</strong></code></td> <td>[Color_Alpha](#Color_Alpha) component (0..255) of [Paint_Color](#Paint_Color).</td>
  </tr>
</table>

##### Example

<fiddle-embed name="4e783d6b70a95b2669f497ad7c4d5243"></fiddle-embed>

###### Example Output

~~~~
0x44112233 == paint.getColor()
~~~~

---

<a name="SkPaint_setARGB"></a>
<!--?prettify lang=cc?-->

    void setARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b)

Sets [Paint_Color](#Paint_Color) used when drawing solid fills. The color components range from 0 to 255.
The color is unpremultiplied;
[Color_Alpha](#Color_Alpha) sets the transparency independent of [Color_RGB](#Color_RGB).

#### Parameters

<table>
  <tr>
    <td><code><strong>a</strong></code></td> <td>Amount of [Color_Alpha](#Color_Alpha), from fully transparent (0) to fully opaque (255).</td>
  </tr>
  <tr>
    <td><code><strong>r</strong></code></td> <td>Amount of [Color_Red](#Color_Red), from no red (0) to full red (255).</td>
  </tr>
  <tr>
    <td><code><strong>g</strong></code></td> <td>Amount of [Color_Green](#Color_Green), from no green (0) to full green (255).</td>
  </tr>
  <tr>
    <td><code><strong>b</strong></code></td> <td>Amount of [Color_Blue](#Color_Blue), from no blue (0) to full blue (255).</td>
  </tr>
</table>

#### Example

<fiddle-embed name="345a684de24090f4c469e15659bd101d"></fiddle-embed>

##### Example Output

~~~~
transRed1 == transRed2
~~~~

#### See Also

[setColor](#SkPaint_setColor) [SkColorSetARGB](#SkColorSetARGB)---

## <a name="Paint_Style"></a> Paint Style
[Paint_Style](#Paint_Style) specifies if the geometry is filled, stroked, or both filled and stroked.
Some shapes ignore [Paint_Style](#Paint_Style) and are always drawn filled or stroked.

Set [Paint_Style](#Paint_Style) to [kFill_Style](#SkPaint_kFill_Style) to fill the shape.
The fill covers the area inside the geometry for most shapes.

Set [Paint_Style](#Paint_Style) to [kStroke_Style](#SkPaint_kStroke_Style) to stroke the shape.

### <a name="Paint_Stroke"></a> Paint Stroke
The stroke covers the area described by following the shape's edge with a pen or brush of
[Stroke_Width](#Stroke_Width). The area covered where the shape starts and stops is described by [Stroke_Cap](#Stroke_Cap).
The area covered where the shape turns a corner is described by [Stroke_Join](#Stroke_Join).
The stroke is centered on the shape; it extends equally on either side of the shape's edge.

As [Stroke_Width](#Stroke_Width) gets smaller, the drawn path frame is thinner. [Stroke_Width](#Stroke_Width) less than one
may have gaps, and if [kAntiAlias_Flag](#SkPaint_kAntiAlias_Flag) is set, [Paint_Alpha](#Paint_Alpha) will increase to visually decrease coverage.### <a name="Paint_Hairline"></a> Paint Hairline
[Stroke_Width](#Stroke_Width) of zero has a special meaning and switches drawing to use [Paint_Hairline](#Paint_Hairline).
[Paint_Hairline](#Paint_Hairline) draws the thinnest continuous frame. If [kAntiAlias_Flag](#SkPaint_kAntiAlias_Flag) is clear, adjacent pixels 
flow horizontally, vertically,or diagonally. 

[Path](#Path) drawing with [Paint_Hairline](#Paint_Hairline) may hit the same pixel more than once. For instance, [Path](#Path) containing
two lines in one [Path_Contour](#Path_Contour) will draw the corner point once, but may both lines may draw the adjacent
pixel. If [kAntiAlias_Flag](#SkPaint_kAntiAlias_Flag) is set, transparency is applied twice, resulting in a darker pixel. Some
[GPU-backed](#GPU-backed) implementations apply transparency at a later drawing stage, avoiding double hit pixels
while stroking.
Set [Paint_Style](#Paint_Style) to [kStrokeAndFill_Style](#SkPaint_kStrokeAndFill_Style) to draw both simultaneously. The stroke and fill
share all paint attributes; for instance, they are drawn with the same color.

### <a name="SkPaint_Style"></a> Enum SkPaint::Style



#### Constants

<table>
  <tr>
    <td><a name="SkPaint_kFill_Style"></a> <code><strong>SkPaint::kFill_Style</strong></code></td><td>0</td><td>[kFill_Style](#SkPaint_kFill_Style) applies to [Rect](#Rect), [Region](#Region), [Round_Rect](#Round_Rect), [Circle](#Circle), [Oval](#Oval), [Path](#Path), and [Text](#Text). 
[Bitmap](#Bitmap), [Image](#Image), [Patch](#Patch), [Region](#Region), [Sprite](#Sprite), and [Vertices](#Vertices) are painted as if [kFill_Style](#SkPaint_kFill_Style) is set,
and ignore the set [Paint_Style](#Paint_Style).
The [Path_Fill_Type](#Path_Fill_Type) specifies additional rules to fill the area outside the path edge,
and to create an unfilled hole inside the shape.
[Paint_Style](#Paint_Style) is set to [kFill_Style](#SkPaint_kFill_Style) by default.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kStroke_Style"></a> <code><strong>SkPaint::kStroke_Style</strong></code></td><td>1</td><td>[kStroke_Style](#SkPaint_kStroke_Style) applies to [Rect](#Rect), [Region](#Region), [Round_Rect](#Round_Rect), [Arc](#Arc), [Circle](#Circle), [Oval](#Oval),
[Path](#Path), and [Text](#Text). 
[Arc](#Arc), [Line](#Line), [Point](#Point), and [Points](#Points) are always drawn as if [kStroke_Style](#SkPaint_kStroke_Style) is set,
and ignore the set [Paint_Style](#Paint_Style).
The stroke construction is unaffected by the [Path_Fill_Type](#Path_Fill_Type).</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kStrokeAndFill_Style"></a> <code><strong>SkPaint::kStrokeAndFill_Style</strong></code></td><td>2</td><td>[kStrokeAndFill_Style](#SkPaint_kStrokeAndFill_Style) applies to [Rect](#Rect), [Region](#Region), [Round_Rect](#Round_Rect), [Circle](#Circle), [Oval](#Oval), [Path](#Path), and [Text](#Text).
[Path](#Path) is treated as if it is set to [SkPath_kWinding_FillType](#SkPath_kWinding_FillType),
and the set [Path_Fill_Type](#Path_Fill_Type) is ignored.</td>
  </tr>
Use stroke and fill to avoid hitting the same pixels twice with a stroke draw and
a fill draw.

[Path](#Path) with [SkPath_kEvenOdd_FillType](#SkPath_kEvenOdd_FillType) drawn with
[kStroke_Style](#SkPaint_kStroke_Style) and then drawn with [kFill_Style](#SkPaint_kFill_Style) may draw differently from one drawn
with [kStrokeAndFill_Style](#SkPaint_kStrokeAndFill_Style).</table>

### <a name="SkPaint_anonymous"></a> Enum SkPaint::anonymous



#### Constants

<table>
  <tr>
    <td><a name="SkPaint_kStyleCount"></a> <code><strong>SkPaint::kStyleCount</strong></code></td><td>3</td><td>[kStyleCount](#SkPaint_kStyleCount) is the number of different [Paint_Style](#Paint_Style) values defined.
[kStyleCount](#SkPaint_kStyleCount) may be used to verify that [Paint_Style](#Paint_Style) is a legal value.</td>
  </tr>
</table>

<a name="SkPaint_getStyle"></a>
<!--?prettify lang=cc?-->

    Style getStyle() const

Whether the geometry is filled, stroked, or filled and stroked.

#### Return Value

One of: [kFill_Style](#SkPaint_kFill_Style), [kStroke_Style](#SkPaint_kStroke_Style), [kStrokeAndFill_Style](#SkPaint_kStrokeAndFill_Style).

#### Example

<fiddle-embed name="3693fce7864fe4094cb3d3948db8cacf"></fiddle-embed>

##### Example Output

~~~~
SkPaint::kFill_Style == paint.getStyle()
~~~~

#### See Also

[Paint_Style](#Paint_Style) [setStyle](#SkPaint_setStyle)---

<a name="SkPaint_setStyle"></a>
<!--?prettify lang=cc?-->

    void setStyle(Style style)

Sets whether the geometry is filled, stroked, or filled and stroked.

#### Parameters

<table>
  <tr>
    <td><code><strong>style</strong></code></td> <td>One of: [kFill_Style](#SkPaint_kFill_Style), [kStroke_Style](#SkPaint_kStroke_Style), [kStrokeAndFill_Style](#SkPaint_kStrokeAndFill_Style).
                  Has no effect if style is not a legal [Style](#SkPaint_Style) value.</td>
  </tr>
</table>

#### Example

<fiddle-embed name="c7bb6248e4735b8d1a32d02fba40d344"></fiddle-embed>

#### See Also

[Paint_Style](#Paint_Style) [getStyle](#SkPaint_getStyle)---

### See Also

[Path_Fill_Type](#Path_Fill_Type) [Path_Effect](#Path_Effect) [Style_Fill](#Style_Fill) [Style_Stroke](#Style_Stroke)## <a name="Style_Fill"></a> Style Fill
### See Also

[Path_Fill_Type](#Path_Fill_Type)## <a name="Style_Stroke"></a> Style Stroke
## <a name="Stroke_Width"></a> Stroke Width
[Stroke_Width](#Stroke_Width) sets the width for stroking. The width is the thickness
of the stroke perpendicular to the path's direction when the paint's style is 
set to [kStroke_Style](#SkPaint_kStroke_Style) or [kStrokeAndFill_Style](#SkPaint_kStrokeAndFill_Style).

When width is greater than zero, the stroke encompasses as many pixels partially
or fully as needed. When the width equals zero, the paint enables hairlines;
the stroke is always one pixel wide. 

The stroke's dimensions are scaled by the canvas matrix, but [Paint_Hairline](#Paint_Hairline) stroke
remains one pixel wide regardless of scaling.

The default width for the paint is zero.

### Example

<fiddle-embed name="3832b070eefd62afff5fe375baa86445"></fiddle-embed>

raster gpuThe pixels hit to represent thin lines vary with the angle of the 
line and the platform's implementation.<a name="SkPaint_getStrokeWidth"></a>
<!--?prettify lang=cc?-->

    SkScalar getStrokeWidth() const

Returns the thickness of the pen used by [Paint](#Paint) to
outline the shape.

#### Return Value

Zero for [Paint_Hairline](#Paint_Hairline), greater than zero for pen thickness.

#### Example

<fiddle-embed name="983efec8708cec5d0e9bd12d8b950c55"></fiddle-embed>

##### Example Output

~~~~
0 == paint.getStrokeWidth()
~~~~

---

<a name="SkPaint_setStrokeWidth"></a>
<!--?prettify lang=cc?-->

    void setStrokeWidth(SkScalar width)

Sets the thickness of the pen used by the paint to
outline the shape. 

#### Parameters

<table>
  <tr>
    <td><code><strong>width</strong></code></td> <td>Zero thickness for [Paint_Hairline](#Paint_Hairline); greater than zero for pen thickness. 
                  Has no effect if less than zero.</td>
  </tr>
</table>

#### Example

<fiddle-embed name="4387d63c69fb469fb66668cc15719165"></fiddle-embed>

##### Example Output

~~~~
5 == paint.getStrokeWidth()
~~~~

---

## <a name="Miter_Limit"></a> Miter Limit
[Miter_Limit](#Miter_Limit) specifies the maximum miter length,
relative to the stroke width.

[Miter_Limit](#Miter_Limit) is used when the [Stroke_Join](#Stroke_Join)
is set to [kMiter_Join](#SkPaint_kMiter_Join), and the [Paint_Style](#Paint_Style) is either [kStroke_Style](#SkPaint_kStroke_Style)
or [kStrokeAndFill_Style](#SkPaint_kStrokeAndFill_Style).

If the miter at a corner exceeds this limit, [kMiter_Join](#SkPaint_kMiter_Join)
is replaced with [kBevel_Join](#SkPaint_kBevel_Join).

[Miter_Limit](#Miter_Limit) can be computed from the corner angle:

miter limit = 1 / sin ( angle / 2 )[Miter_Limit](#Miter_Limit) default value is 4.
The default may be changed at compile time by setting [SkPaintDefaults_MiterLimit](#SkPaintDefaults_MiterLimit)
in [SkUserConfig.h](#SkUserConfig.h) or as a define supplied by the build environment.

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

<fiddle-embed name="73930f59e9c0c788139b33d67d61a7e2"></fiddle-embed>

512This example draws a stroked corner and the miter length beneath.
When the miter limit is decreased slightly, the miter join is replaced
by a bevel join.<a name="SkPaint_getStrokeMiter"></a>
<!--?prettify lang=cc?-->

    SkScalar getStrokeMiter() const

The limit at which a sharp corner is drawn beveled.

#### Return Value

[Miter_Limit](#Miter_Limit).

#### Example

<fiddle-embed name="f5715648fe49fe6152ed7753ba41ec1a"></fiddle-embed>

##### Example Output

~~~~
default miter limit == 4
~~~~

#### See Also

[Miter_Limit](#Miter_Limit) [setStrokeMiter](#SkPaint_setStrokeMiter) [Join](#SkPaint_Join)---

<a name="SkPaint_setStrokeMiter"></a>
<!--?prettify lang=cc?-->

    void setStrokeMiter(SkScalar miter)

The limit at which a sharp corner is drawn beveled.

#### Parameters

<table>
  <tr>
    <td><code><strong>miter</strong></code></td> <td>Valid values are zero and greater.
                   Has no effect if miter is less than zero.</td>
  </tr>
</table>

#### Example

<fiddle-embed name="f8b9bc2c138950c080bed6ad1a820f44"></fiddle-embed>

##### Example Output

~~~~
default miter limit == 8
~~~~

#### See Also

[Miter_Limit](#Miter_Limit) [getStrokeMiter](#SkPaint_getStrokeMiter) [Join](#SkPaint_Join)---

## <a name="Stroke_Cap"></a> Stroke Cap
[Stroke_Cap](#Stroke_Cap) draws at the beginning and end of an open [Path_Contour](#Path_Contour).

### <a name="SkPaint_Cap"></a> Enum SkPaint::Cap



#### Constants

<table>
  <tr>
    <td><a name="SkPaint_kButt_Cap"></a> <code><strong>SkPaint::kButt_Cap</strong></code></td><td>0</td><td>[kButt_Cap](#SkPaint_kButt_Cap) does not extend the stroke past the beginning or the end.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kRound_Cap"></a> <code><strong>SkPaint::kRound_Cap</strong></code></td><td>1</td><td>[kRound_Cap](#SkPaint_kRound_Cap) adds a circle with a diameter equal to [Stroke_Width](#Stroke_Width) at the beginning
and end.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kSquare_Cap"></a> <code><strong>SkPaint::kSquare_Cap</strong></code></td><td>2</td><td>[kSquare_Cap](#SkPaint_kSquare_Cap) adds a square with sides equal to [Stroke_Width](#Stroke_Width) at the beginning
and end. The square sides are parallel to the initial and final direction
of the stroke.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kLast_Cap"></a> <code><strong>SkPaint::kLast_Cap</strong></code></td><td>2</td><td>[kLast_Cap](#SkPaint_kLast_Cap) is equivalent to the largest value for [Stroke_Cap](#Stroke_Cap).</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kDefault_Cap"></a> <code><strong>SkPaint::kDefault_Cap</strong></code></td><td>0</td><td>[kDefault_Cap](#SkPaint_kDefault_Cap) is equivalent to [kButt_Cap](#SkPaint_kButt_Cap).
[Stroke_Cap](#Stroke_Cap) is set to [kButt_Cap](#SkPaint_kButt_Cap) by default.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kCapCount"></a> <code><strong>SkPaint::kCapCount</strong></code></td><td>3</td><td>[kCapCount](#SkPaint_kCapCount) is the number of different [Stroke_Cap](#Stroke_Cap) values defined.
[kCapCount](#SkPaint_kCapCount) may be used to verify that [Stroke_Cap](#Stroke_Cap) is a legal value.</td>
  </tr>
[Paint_Stroke](#Paint_Stroke) describes the area covered by a pen of [Stroke_Width](#Stroke_Width) as it 
follows the [Path_Contour](#Path_Contour), moving parallel to the contours's direction.

If the [Path_Contour](#Path_Contour) is not terminated by [SkPath_kClose_Verb](#SkPath_kClose_Verb), the contour has a
visible beginning and end.

[Path_Contour](#Path_Contour) may start and end at the same point; defining [Zero_Length_Contour](#Zero_Length_Contour).

[kButt_Cap](#SkPaint_kButt_Cap) and [Zero_Length_Contour](#Zero_Length_Contour) is not drawn.
[kRound_Cap](#SkPaint_kRound_Cap) and [Zero_Length_Contour](#Zero_Length_Contour) draws a circle of diameter [Stroke_Width](#Stroke_Width) 
at the contour point.
[kSquare_Cap](#SkPaint_kSquare_Cap) and [Zero_Length_Contour](#Zero_Length_Contour) draws an upright square with a side of
[Stroke_Width](#Stroke_Width) at the contour point.

[Stroke_Cap](#Stroke_Cap) is [kButt_Cap](#SkPaint_kButt_Cap) by default.

</table>

### Example

<fiddle-embed name="0cab4c079c48e3c277637647d46ce88a"></fiddle-embed>

<a name="SkPaint_getStrokeCap"></a>
<!--?prettify lang=cc?-->

    Cap getStrokeCap() const

The geometry drawn at the beginning and end of strokes.

#### Return Value

One of: [kButt_Cap](#SkPaint_kButt_Cap), [kRound_Cap](#SkPaint_kRound_Cap), [kSquare_Cap](#SkPaint_kSquare_Cap).

#### Example

<fiddle-embed name="753c12ecfc325c7e860ae13a0f045e60"></fiddle-embed>

##### Example Output

~~~~
kButt_Cap == default stroke cap
~~~~

#### See Also

[Stroke_Cap](#Stroke_Cap) [setStrokeCap](#SkPaint_setStrokeCap)---

<a name="SkPaint_setStrokeCap"></a>
<!--?prettify lang=cc?-->

    void setStrokeCap(Cap cap)

The geometry drawn at the beginning and end of strokes.

#### Parameters

<table>
  <tr>
    <td><code><strong>cap</strong></code></td> <td>One of: [kButt_Cap](#SkPaint_kButt_Cap), [kRound_Cap](#SkPaint_kRound_Cap), [kSquare_Cap](#SkPaint_kSquare_Cap).
                Has no effect if cap is not valid.</td>
  </tr>
</table>

#### Example

<fiddle-embed name="88b79a48aaab2990093c057deffb05c6"></fiddle-embed>

##### Example Output

~~~~
kRound_Cap == paint.getStrokeCap()
~~~~

#### See Also

[Stroke_Cap](#Stroke_Cap) [getStrokeCap](#SkPaint_getStrokeCap)---

## <a name="Stroke_Join"></a> Stroke Join
[Stroke_Join](#Stroke_Join) draws at the sharp corners of an open or closed [Path_Contour](#Path_Contour).

[Paint_Stroke](#Paint_Stroke) describes the area covered by a pen of [Stroke_Width](#Stroke_Width) as it 
follows the [Path_Contour](#Path_Contour), moving parallel to the contours's direction.

If the contour direction changes abruptly, because the tangent direction leading
to the end of a curve within the contour does not match the tangent direction of
the following curve, the pair of curves meet at [Stroke_Join](#Stroke_Join).

### Example

<fiddle-embed name="620b563d74734c49cc08e6fe07c01090"></fiddle-embed>

### <a name="SkPaint_Join"></a> Enum SkPaint::Join



[Join](#SkPaint_Join) specifies how corners are drawn when a shape is stroked. The paint's [Join](#SkPaint_Join) setting
affects the four corners of a stroked rectangle, and the connected segments in a stroked path.

Choose miter join to draw sharp corners. Choose round join to draw a circle with a
radius equal to the stroke width on top of the corner. Choose bevel join to minimally connect
the thick strokes.

The fill path constructed to describe the stroked path respects the join setting but may 
not contain the actual join. For instance, a fill path constructed with round joins does
not necessarily include circles at each connected segment.

#### Constants

<table>
  <tr>
    <td><a name="SkPaint_kMiter_Join"></a> <code><strong>SkPaint::kMiter_Join</strong></code></td><td>0</td><td>[kMiter_Join](#SkPaint_kMiter_Join) extends the outside of the to the extent allowed by [Miter_Limit](#Miter_Limit).
If the extension exceeds [Miter_Limit](#Miter_Limit), [kBevel_Join](#SkPaint_kBevel_Join) is used instead.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kRound_Join"></a> <code><strong>SkPaint::kRound_Join</strong></code></td><td>1</td><td>[kRound_Join](#SkPaint_kRound_Join) adds a circle with a diameter of [Stroke_Width](#Stroke_Width) at the sharp corner.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kBevel_Join"></a> <code><strong>SkPaint::kBevel_Join</strong></code></td><td>2</td><td>[kBevel_Join](#SkPaint_kBevel_Join) connects the outside edges of the sharp corner.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kLast_Join"></a> <code><strong>SkPaint::kLast_Join</strong></code></td><td>2</td><td>[kLast_Join](#SkPaint_kLast_Join) is equivalent to the largest value for [Stroke_Join](#Stroke_Join).</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kDefault_Join"></a> <code><strong>SkPaint::kDefault_Join</strong></code></td><td>1</td><td>[kDefault_Join](#SkPaint_kDefault_Join) is equivalent to [kMiter_Join](#SkPaint_kMiter_Join).
[Stroke_Join](#Stroke_Join) is set to [kMiter_Join](#SkPaint_kMiter_Join) by default.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kJoinCount"></a> <code><strong>SkPaint::kJoinCount</strong></code></td><td>3</td><td>[kJoinCount](#SkPaint_kJoinCount) is the number of different [Stroke_Join](#Stroke_Join) values defined.
[kJoinCount](#SkPaint_kJoinCount) may be used to verify that [Stroke_Join](#Stroke_Join) is a legal value.</td>
  </tr>
</table>

#### Example

<fiddle-embed name="93eb98f36614f1159645cf252fe9d3a5"></fiddle-embed>

#### See Also

[setStrokeJoin](#SkPaint_setStrokeJoin) [getStrokeJoin](#SkPaint_getStrokeJoin) [setStrokeMiter](#SkPaint_setStrokeMiter) [getStrokeMiter](#SkPaint_getStrokeMiter)<a name="SkPaint_getStrokeJoin"></a>
<!--?prettify lang=cc?-->

    Join getStrokeJoin() const

The geometry drawn at the corners of strokes. 

#### Return Value

One of: [kMiter_Join](#SkPaint_kMiter_Join), [kRound_Join](#SkPaint_kRound_Join), [kBevel_Join](#SkPaint_kBevel_Join).

#### Example

<fiddle-embed name="841a73f24e10c6678630c179506c0595"></fiddle-embed>

##### Example Output

~~~~
kMiter_Join == default stroke join
~~~~

#### See Also

[Stroke_Join](#Stroke_Join) [setStrokeJoin](#SkPaint_setStrokeJoin)---

<a name="SkPaint_setStrokeJoin"></a>
<!--?prettify lang=cc?-->

    void setStrokeJoin(Join join)

The geometry drawn at the corners of strokes. 

#### Parameters

<table>
  <tr>
    <td><code><strong>join</strong></code></td> <td>One of: [kMiter_Join](#SkPaint_kMiter_Join), [kRound_Join](#SkPaint_kRound_Join), [kBevel_Join](#SkPaint_kBevel_Join).
                 If join is not a valid, [setStrokeJoin](#SkPaint_setStrokeJoin) has no effect.</td>
  </tr>
</table>

#### Example

<fiddle-embed name="693707e991e819a54baa7a3a0404ecee"></fiddle-embed>

##### Example Output

~~~~
kMiter_Join == paint.getStrokeJoin()
~~~~

#### See Also

[Stroke_Join](#Stroke_Join) [getStrokeJoin](#SkPaint_getStrokeJoin)---

### See Also

[Miter_Limit](#Miter_Limit)## <a name="Fill_Path"></a> Fill Path
[Fill_Path](#Fill_Path) creates a [Path](#Path) by applying the [Path_Effect](#Path_Effect), followed by the [Style_Stroke](#Style_Stroke).

If [Paint](#Paint) contains [Path_Effect](#Path_Effect), [Path_Effect](#Path_Effect) operates on the source [Path](#Path); the result
replaces the destination [Path](#Path). Otherwise, the source [Path](#Path) is replaces the
destination [Path](#Path).

Fill [Path](#Path) can request the [Path_Effect](#Path_Effect) to restrict to a culling rectangle, but
the [Path_Effect](#Path_Effect) is not required to do so.

If [Paint_Style](#Paint_Style) is [kStroke_Style](#SkPaint_kStroke_Style) or [kStrokeAndFill_Style](#SkPaint_kStrokeAndFill_Style), 
and [Stroke_Width](#Stroke_Width) is greater than zero, the [Stroke_Width](#Stroke_Width), [Stroke_Cap](#Stroke_Cap), [Stroke_Join](#Stroke_Join),
and [Miter_Limit](#Miter_Limit) operate on the destination [Path](#Path), replacing it.

Fill [Path](#Path) can specify the precision used by [Stroke_Width](#Stroke_Width) to approximate the stroke geometry. 

If the [Paint_Style](#Paint_Style) is [kStroke_Style](#SkPaint_kStroke_Style) and the [Stroke_Width](#Stroke_Width) is zero, [getFillPath](#SkPaint_getFillPath)
returns false since [Paint_Hairline](#Paint_Hairline) has no filled equivalent.

<a name="SkPaint_getFillPath"></a>
<!--?prettify lang=cc?-->

    bool getFillPath(const SkPath& src, SkPath* dst, const SkRect* cullRect,

[SkScalar](#SkScalar) resScale = 1)

    The filled equivalent of the stroked path.

#### Parameters

<table>
  <tr>
    <td><code><strong>src</strong></code></td> <td>[Path](#Path) read to create a filled version.</td>
  </tr>
  <tr>
    <td><code><strong>dst</strong></code></td> <td>The resulting [Path](#Path) dst may be the same as src, but may not be nullptr.</td>
  </tr>
  <tr>
    <td><code><strong>cullRect</strong></code></td> <td>Optional limit passed to [Path_Effect](#Path_Effect).</td>
  </tr>
  <tr>
    <td><code><strong>resScale</strong></code></td> <td>If > 1, increase precision, else if (0 < res < 1) reduce precision
                    in favor of speed/size.</td>
  </tr>
</table>

#### Return Value

true if the path represents [Style_Fill](#Style_Fill), or false if it represents [Paint_Hairline](#Paint_Hairline).

#### Example

<fiddle-embed name="53da2db22c88f891a990f12de286bce3"></fiddle-embed>

A very small quad stroke is turned into a filled path with increasing levels of precision.
At the lowest precision, the quad stroke is approximated by a rectangle. 
At the highest precision, the filled path has high fidelity compared to the original stroke.---

<a name="SkPaint_getFillPath_2"></a>
<!--?prettify lang=cc?-->

    bool getFillPath(const SkPath& src, SkPath* dst) const

The filled equivalent of the stroked path.

Replaces dst with the src path modified by [Path_Effect](#Path_Effect) and [Style_Stroke](#Style_Stroke).
[Path_Effect](#Path_Effect), if any, is not culled. [Stroke_Width](#Stroke_Width) is created with default precision.

#### Parameters

<table>
  <tr>
    <td><code><strong>src</strong></code></td> <td>[Path](#Path) read to create a filled version.</td>
  </tr>
  <tr>
    <td><code><strong>dst</strong></code></td> <td>The resulting [Path](#Path) dst may be the same as src, but may not be nullptr.</td>
  </tr>
</table>

#### Return Value

true if the path represents [Style_Fill](#Style_Fill), or false if it represents [Paint_Hairline](#Paint_Hairline).

#### Example

<fiddle-embed name="095d99da76d587b015f7d3b675efc7b8"></fiddle-embed>

---

### See Also

[Style_Stroke](#Style_Stroke) [Stroke_Width](#Stroke_Width) [Path_Effect](#Path_Effect)## <a name="Paint_Shader"></a> Paint Shader
[Paint_Shader](#Paint_Shader) references [Shader](#Shader) that defines the colors used when drawing a shape.
[Shader](#Shader) may be an image, a gradient, or a computed fill.
If [Paint](#Paint) has no [Shader](#Shader), then [Paint_Color](#Paint_Color) fills the shape. 

[Paint_Shader](#Paint_Shader) is modulated by [Color_Alpha](#Color_Alpha) component of [Paint_Color](#Paint_Color).
If [Shader](#Shader) object defines only [Color_Alpha](#Color_Alpha), then [Paint_Color](#Paint_Color) modulated by [Color_Alpha](#Color_Alpha) describes
the fill.

The drawn transparency can be modified without altering [Paint_Shader](#Paint_Shader), by changing [Color_Alpha](#Color_Alpha).

### Example

<fiddle-embed name="c015dc2010c15e1c00b4f7330232b0f7"></fiddle-embed>

If [Shader](#Shader) generates only [Color_Alpha](#Color_Alpha) then all components of [Paint_Color](#Paint_Color) modulate the output.

### Example

<fiddle-embed name="f45b9765dd06d996921ffa1ef8e78d8a"></fiddle-embed>

<a name="SkPaint_getShader"></a>
<!--?prettify lang=cc?-->

    SkShader* getShader() const

Optional colors used when filling a path, such as a gradient.

Does not alter [Shader](#Shader) [Reference_Count](#Reference_Count).

#### Return Value

[Shader](#Shader) if previously set, nullptr otherwise.

#### Example

<fiddle-embed name="09f15b9fd88882850da2d235eb86292f"></fiddle-embed>

##### Example Output

~~~~
nullptr == shader
nullptr != shader
~~~~

---

<a name="SkPaint_refShader"></a>
<!--?prettify lang=cc?-->

    sk_sp<SkShader> refShader() const

Optional colors used when filling a path, such as a gradient.

Increases [Shader](#Shader) [Reference_Count](#Reference_Count) by one.

#### Return Value

[Shader](#Shader) if previously set, nullptr otherwise.

#### Example

<fiddle-embed name="53da0295972a418cbc9607bbb17feaa8"></fiddle-embed>

##### Example Output

~~~~
shader unique: true
shader unique: false
~~~~

---

<a name="SkPaint_setShader"></a>
<!--?prettify lang=cc?-->

    void setShader(sk_sp<SkShader> shader)

Optional colors used when filling a path, such as a gradient.

Sets [Paint_Shader](#Paint_Shader) to shader, decrementing [Reference_Count](#Reference_Count) of the previous [Shader](#Shader).
Does not alter shader [Reference_Count](#Reference_Count).

#### Parameters

<table>
  <tr>
    <td><code><strong>shader</strong></code></td> <td>Pass nullptr to use [Paint_Color](#Paint_Color) instead of [Paint_Shader](#Paint_Shader) to fill.</td>
  </tr>
</table>

#### Example

<fiddle-embed name="92996333200d7c0b2158927f7e8439c5"></fiddle-embed>

---

## <a name="Paint_Color_Filter"></a> Paint Color Filter
[Paint_Color_Filter](#Paint_Color_Filter) alters the color used when drawing a shape.
[Color_Filter](#Color_Filter) may apply [Blend_Mode](#Blend_Mode), transform the color through a matrix, or composite multiple filters.
If [Paint](#Paint) has no [Color_Filter](#Color_Filter), the color is unaltered.

The drawn transparency can be modified without altering [Paint_Color_Filter](#Paint_Color_Filter), by changing [Color_Alpha](#Color_Alpha).

### Example

<fiddle-embed name="3343ea21abaf9778d35e9953f0a6be10"></fiddle-embed>

<a name="SkPaint_getColorFilter"></a>
<!--?prettify lang=cc?-->

    SkColorFilter* getColorFilter() const

[getColorFilter](#SkPaint_getColorFilter) returns [Paint_Color_Filter](#Paint_Color_Filter) if set, or nullptr.
[getColorFilter](#SkPaint_getColorFilter) does not alter [Color_Filter](#Color_Filter) [Reference_Count](#Reference_Count).

#### Return Value

[Color_Filter](#Color_Filter) if previously set, nullptr otherwise.

#### Example

<fiddle-embed name="093bdc627d6b59002670fd290931f6c9"></fiddle-embed>

##### Example Output

~~~~
nullptr == color filter
nullptr != color filter
~~~~

---

<a name="SkPaint_refColorFilter"></a>
<!--?prettify lang=cc?-->

    sk_sp<SkColorFilter> refColorFilter() const

[refColorFilter](#SkPaint_refColorFilter) returns [Paint_Color_Filter](#Paint_Color_Filter) if set, or nullptr.
[refColorFilter](#SkPaint_refColorFilter) increases [Color_Filter](#Color_Filter) [Reference_Count](#Reference_Count) by one.

#### Example

<fiddle-embed name="be0095f54d59487ef2d2e87457910bc9"></fiddle-embed>

##### Example Output

~~~~
color filter unique: true
color filter unique: false
~~~~

---

<a name="SkPaint_setColorFilter"></a>
<!--?prettify lang=cc?-->

    void setColorFilter(sk_sp<SkColorFilter> filter)

[setColorFilter](#SkPaint_setColorFilter) sets [Paint_Shader](#Paint_Shader) to filter, decrementing [Reference_Count](#Reference_Count) of the previous [Shader](#Shader). 
Pass nullptr to clear [Paint_Color_Filter](#Paint_Color_Filter).
[setColorFilter](#SkPaint_setColorFilter) does not alter filter [Reference_Count](#Reference_Count).

#### Example

<fiddle-embed name="01e7486180e7d3ecf9a6501e6afeb717"></fiddle-embed>

---

## <a name="Paint_Blend_Mode"></a> Paint Blend Mode
[Paint_Blend_Mode](#Paint_Blend_Mode) describes how [Paint_Color](#Paint_Color) combines with the destination color.
The default setting, [SkBlendMode_kSrcOver](#SkBlendMode_kSrcOver), draws the source color
over the destination color.

### Example

<fiddle-embed name="73092d4d06faecea3c204d852a4dd8a8"></fiddle-embed>

### See Also

[Blend_Mode](#Blend_Mode)<a name="SkPaint_getBlendMode"></a>
<!--?prettify lang=cc?-->

    SkBlendMode getBlendMode() const

[getBlendMode](#SkPaint_getBlendMode) returns [Paint_Blend_Mode](#Paint_Blend_Mode).
By default, [getBlendMode](#SkPaint_getBlendMode) returns [SkBlendMode_kSrcOver](#SkBlendMode_kSrcOver).

#### Example

<fiddle-embed name="4ec1864b8203d52c0810e8605092f45c"></fiddle-embed>

##### Example Output

~~~~
kSrcOver == getBlendMode
kSrcOver != getBlendMode
~~~~

---

<a name="SkPaint_isSrcOver"></a>
<!--?prettify lang=cc?-->

    bool isSrcOver() const

[isSrcOver](#SkPaint_isSrcOver) returns true if [Paint_Blend_Mode](#Paint_Blend_Mode) is [SkBlendMode_kSrcOver](#SkBlendMode_kSrcOver), the default.

#### Example

<fiddle-embed name="257c9473db7a2b3a0fb2b9e2431e59a6"></fiddle-embed>

##### Example Output

~~~~
isSrcOver == true
isSrcOver != true
~~~~

---

<a name="SkPaint_setBlendMode"></a>
<!--?prettify lang=cc?-->

    void setBlendMode(SkBlendMode mode)

[setBlendMode](#SkPaint_setBlendMode) sets [Paint_Blend_Mode](#Paint_Blend_Mode) to mode. 
[setBlendMode](#SkPaint_setBlendMode) does not check for valid input.

#### Parameters

<table>
  <tr>
    <td><code><strong>mode</strong></code></td> <td>[SkBlendMode](#SkBlendMode) used to combine source color and destination.</td>
  </tr>
</table>

#### Example

<fiddle-embed name="257c9473db7a2b3a0fb2b9e2431e59a6"></fiddle-embed>

##### Example Output

~~~~
isSrcOver == true
isSrcOver != true
~~~~

---

## <a name="Paint_Path_Effect"></a> Paint Path Effect
[Paint_Path_Effect](#Paint_Path_Effect) modifies the path geometry before drawing it.
[Path_Effect](#Path_Effect) may implement dashing, custom fill effects and custom stroke effects.
If [Paint](#Paint) has no [Path_Effect](#Path_Effect), the path geometry is unaltered when filled or stroked.

### Example

<fiddle-embed name="5daa7aebf14b7e224d0e1e98798e5e7d"></fiddle-embed>

### See Also

[Path_Effect](#Path_Effect)<a name="SkPaint_getPathEffect"></a>
<!--?prettify lang=cc?-->

    SkPathEffect* getPathEffect() const

[getPathEffect](#SkPaint_getPathEffect) returns [Paint_Path_Effect](#Paint_Path_Effect) if set, or nullptr.
[getPathEffect](#SkPaint_getPathEffect) does not alter [Path_Effect](#Path_Effect) [Reference_Count](#Reference_Count).

#### Return Value

[Path_Effect](#Path_Effect) if previously set, nullptr otherwise.

#### Example

<fiddle-embed name="211a1b14bfa6c4332082c8eab4fbc5fd"></fiddle-embed>

##### Example Output

~~~~
nullptr == path effect
nullptr != path effect
~~~~

---

<a name="SkPaint_refPathEffect"></a>
<!--?prettify lang=cc?-->

    sk_sp<SkPathEffect> refPathEffect() const

[refPathEffect](#SkPaint_refPathEffect) returns [Paint_Path_Effect](#Paint_Path_Effect) if set, or nullptr.
[refPathEffect](#SkPaint_refPathEffect) increases [Path_Effect](#Path_Effect) [Reference_Count](#Reference_Count) by one.

#### Example

<fiddle-embed name="c24ff92a980a5988697f6ae910560e21"></fiddle-embed>

##### Example Output

~~~~
path effect unique: true
path effect unique: false
~~~~

---

<a name="SkPaint_setPathEffect"></a>
<!--?prettify lang=cc?-->

    void setPathEffect(sk_sp<SkPathEffect> pathEffect)

[setPathEffect](#SkPaint_setPathEffect) sets [Paint_Path_Effect](#Paint_Path_Effect) to pathEffect, 
decrementing [Reference_Count](#Reference_Count) of the previous [Path_Effect](#Path_Effect). 
Pass nullptr to leave the path geometry unaltered.
[setPathEffect](#SkPaint_setPathEffect) does not alter pathEffect [Reference_Count](#Reference_Count).

#### Example

<fiddle-embed name="52dd55074ca0b7d520d04e750ca2a0d7"></fiddle-embed>

---

## <a name="Paint_Mask_Filter"></a> Paint Mask Filter
[Paint_Mask_Filter](#Paint_Mask_Filter) uses [Color_Alpha](#Color_Alpha) of the shape drawn to create [Mask_Alpha](#Mask_Alpha).
[Mask_Filter](#Mask_Filter) operates at a lower level than [Rasterizer](#Rasterizer); [Mask_Filter](#Mask_Filter) takes a [Mask](#Mask),
and returns a [Mask](#Mask).
[Mask_Filter](#Mask_Filter) may change the geometry and transparency of the shape, such as creating a blur effect.
Set [Paint_Mask_Filter](#Paint_Mask_Filter) to nullptr to prevent [Mask_Filter](#Mask_Filter) from modifying the draw.

### Example

<fiddle-embed name="320b04ea1e1291d49f1e61994a0410fe"></fiddle-embed>

<a name="SkPaint_getMaskFilter"></a>
<!--?prettify lang=cc?-->

    SkMaskFilter* getMaskFilter() const

[getMaskFilter](#SkPaint_getMaskFilter) returns [Paint_Mask_Filter](#Paint_Mask_Filter) if set, or nullptr.
[getMaskFilter](#SkPaint_getMaskFilter) does not alter [Mask_Filter](#Mask_Filter) [Reference_Count](#Reference_Count).

#### Return Value

[Mask_Filter](#Mask_Filter) if previously set, nullptr otherwise.

#### Example

<fiddle-embed name="8cd53ece8fc83e4560599ace094b0f16"></fiddle-embed>

##### Example Output

~~~~
nullptr == mask filter
nullptr != mask filter
~~~~

---

<a name="SkPaint_refMaskFilter"></a>
<!--?prettify lang=cc?-->

    sk_sp<SkMaskFilter> refMaskFilter() const

[refMaskFilter](#SkPaint_refMaskFilter) returns [Paint_Mask_Filter](#Paint_Mask_Filter) if set, or nullptr.
[refMaskFilter](#SkPaint_refMaskFilter) increases [Mask_Filter](#Mask_Filter) [Reference_Count](#Reference_Count) by one.

#### Example

<fiddle-embed name="ddb211ed3d5e226b0bb973de5076de26"></fiddle-embed>

##### Example Output

~~~~
mask filter unique: true
mask filter unique: false
~~~~

---

<a name="SkPaint_setMaskFilter"></a>
<!--?prettify lang=cc?-->

    void setMaskFilter(sk_sp<SkMaskFilter> maskFilter)

[setMaskFilter](#SkPaint_setMaskFilter) sets [Paint_Mask_Filter](#Paint_Mask_Filter) to maskFilter,
decrementing [Reference_Count](#Reference_Count) of the previous [Mask_Filter](#Mask_Filter). 
Pass nullptr to clear [Paint_Mask_Filter](#Paint_Mask_Filter) and leave [Mask_Filter](#Mask_Filter) effect on [Mask_Alpha](#Mask_Alpha) unaltered.
[setMaskFilter](#SkPaint_setMaskFilter) does not affect [Paint_Rasterizer](#Paint_Rasterizer).
[setMaskFilter](#SkPaint_setMaskFilter) does not alter maskFilter [Reference_Count](#Reference_Count).

#### Example

<fiddle-embed name="62c5a826692f85c3de3bab65e9e97aa9"></fiddle-embed>

---

## <a name="Paint_Typeface"></a> Paint Typeface
[Paint_Typeface](#Paint_Typeface) identifies the font used when drawing and measuring text.
[Typeface](#Typeface) may be specified by name, from a file, or from a data stream.
The default [Typeface](#Typeface) defers to the platform-specific default font
implementation.

### Example

<fiddle-embed name="bb32c82a23a147f528dfbc2e327a3fa4"></fiddle-embed>

<a name="SkPaint_getTypeface"></a>
<!--?prettify lang=cc?-->

    SkTypeface* getTypeface() const

[getTypeface](#SkPaint_getTypeface) returns [Paint_Typeface](#Paint_Typeface) if set, or nullptr.
[getTypeface](#SkPaint_getTypeface) does not alter [Typeface](#Typeface) [Reference_Count](#Reference_Count).

#### Return Value

[Typeface](#Typeface) if previously set, nullptr otherwise.

#### Example

<fiddle-embed name="4d9ffb5761b62a9e3bc9b0bca8787bce"></fiddle-embed>

##### Example Output

~~~~
nullptr == typeface
nullptr != typeface
~~~~

---

<a name="SkPaint_refTypeface"></a>
<!--?prettify lang=cc?-->

    sk_sp<SkTypeface> refTypeface() const

[refTypeface](#SkPaint_refTypeface) returns [Paint_Typeface](#Paint_Typeface) if set, or nullptr.
[refTypeface](#SkPaint_refTypeface) increases [Typeface](#Typeface) [Reference_Count](#Reference_Count) by one.

#### Example

<fiddle-embed name="c8edce7b36a3ffda8af4fe89d7187dbc"></fiddle-embed>

##### Example Output

~~~~
typeface1 != typeface2
typeface1 == typeface2
~~~~

---

<a name="SkPaint_setTypeface"></a>
<!--?prettify lang=cc?-->

    void setTypeface(sk_sp<SkTypeface> typeface)

[setTypeface](#SkPaint_setTypeface) sets [Paint_Typeface](#Paint_Typeface) to typeface,
decrementing [Reference_Count](#Reference_Count) of the previous [Typeface](#Typeface). 
Pass nullptr to clear [Paint_Typeface](#Paint_Typeface) and use the default typeface.
[setTypeface](#SkPaint_setTypeface) does not alter typeface [Reference_Count](#Reference_Count).

#### Example

<fiddle-embed name="a67efa7096c9b7ff7a4c59aa80310ce9"></fiddle-embed>

---

## <a name="Paint_Rasterizer"></a> Paint Rasterizer
[Paint_Rasterizer](#Paint_Rasterizer) controls how shapes are converted to [Mask_Alpha](#Mask_Alpha). 
[Rasterizer](#Rasterizer) operates at a higher level than [Mask_Filter](#Mask_Filter); [Rasterizer](#Rasterizer) takes a [Path](#Path),
and returns a [Mask](#Mask).
[Rasterizer](#Rasterizer) may change the geometry and transparency of the shape, such as
creating a shadow effect. [Rasterizer](#Rasterizer) forms the base of [Layer_Rasterizer](#Layer_Rasterizer), which
creates effects like embossing and outlining.
[Rasterizer](#Rasterizer) applies to [Rect](#Rect), [Region](#Region), [Round_Rect](#Round_Rect), [Arc](#Arc), [Circle](#Circle), [Oval](#Oval),
[Path](#Path), and [Text](#Text).

### Example

<fiddle-embed name="196bbd3000a2988ccc5460398cb32f5a"></fiddle-embed>

<a name="SkPaint_getRasterizer"></a>
<!--?prettify lang=cc?-->

    SkRasterizer* getRasterizer() const

[getRasterizer](#SkPaint_getRasterizer) returns [Paint_Rasterizer](#Paint_Rasterizer) if set, or nullptr.
[getRasterizer](#SkPaint_getRasterizer) does not alter [Rasterizer](#Rasterizer) [Reference_Count](#Reference_Count).

#### Return Value

[Rasterizer](#Rasterizer) if previously set, nullptr otherwise.

#### Example

<fiddle-embed name="0707d407c3a14388b107af8ae5873e55"></fiddle-embed>

##### Example Output

~~~~
nullptr == rasterizer
nullptr != rasterizer
~~~~

---

<a name="SkPaint_refRasterizer"></a>
<!--?prettify lang=cc?-->

    sk_sp<SkRasterizer> refRasterizer() const

[refRasterizer](#SkPaint_refRasterizer) returns [Paint_Rasterizer](#Paint_Rasterizer) if set, or nullptr.
[refRasterizer](#SkPaint_refRasterizer) increases [Rasterizer](#Rasterizer) [Reference_Count](#Reference_Count) by one.

#### Example

<fiddle-embed name="c0855ce19a33cb7e5747750ef341b7b3"></fiddle-embed>

##### Example Output

~~~~
rasterizer unique: true
rasterizer unique: false
~~~~

---

<a name="SkPaint_setRasterizer"></a>
<!--?prettify lang=cc?-->

    void setRasterizer(sk_sp<SkRasterizer> rasterizer)

[setRasterizer](#SkPaint_setRasterizer) sets [Paint_Rasterizer](#Paint_Rasterizer) to rasterizer,
decrementing [Reference_Count](#Reference_Count) of the previous [Rasterizer](#Rasterizer). 
Pass nullptr to clear [Paint_Rasterizer](#Paint_Rasterizer) and leave [Rasterizer](#Rasterizer) effect on [Mask_Alpha](#Mask_Alpha) unaltered.
[setRasterizer](#SkPaint_setRasterizer) does not affect [Paint_Mask_Filter](#Paint_Mask_Filter).
[setRasterizer](#SkPaint_setRasterizer) does not alter rasterizer [Reference_Count](#Reference_Count).

#### Example

<fiddle-embed name="181e7a5c63a8652edcdc64b7b957f8ec"></fiddle-embed>

---

## <a name="Paint_Image_Filter"></a> Paint Image Filter
[Paint_Image_Filter](#Paint_Image_Filter) operates on the pixel representation of the shape, as modified by [Paint](#Paint)
with [Blend_Mode](#Blend_Mode) set to [SkBlendMode_kSrcOver](#SkBlendMode_kSrcOver). [Image_Filter](#Image_Filter) creates a new bitmap,
which is drawn to the device using the set [Paint_Blend_Mode](#Paint_Blend_Mode).
[Paint_Image_Filter](#Paint_Image_Filter) is higher level than [Paint_Mask_Filter](#Paint_Mask_Filter); for instance, an [Image_Filter](#Image_Filter)
can operate on all channels of [Color](#Color), while [Mask_Filter](#Mask_Filter) generates [Color_Alpha](#Color_Alpha) only.
[Paint_Image_Filter](#Paint_Image_Filter) operates independently of and can be used in combination with
[Mask_Filter](#Mask_Filter) and [Rasterizer](#Rasterizer).

### Example

<fiddle-embed name="4fd19982b82bc27340a507183e39d7d3"></fiddle-embed>

<a name="SkPaint_getImageFilter"></a>
<!--?prettify lang=cc?-->

    SkImageFilter* getImageFilter() const

[getImageFilter](#SkPaint_getImageFilter) returns [Paint_Image_Filter](#Paint_Image_Filter) if set, or nullptr.
[getImageFilter](#SkPaint_getImageFilter) does not alter [Image_Filter](#Image_Filter) [Reference_Count](#Reference_Count).

#### Return Value

[Image_Filter](#Image_Filter) if previously set, nullptr otherwise.

#### Example

<fiddle-embed name="38788d42772641606e08c60d9dd418a2"></fiddle-embed>

##### Example Output

~~~~
nullptr == image filter
nullptr != image filter
~~~~

---

<a name="SkPaint_refImageFilter"></a>
<!--?prettify lang=cc?-->

    sk_sp<SkImageFilter> refImageFilter() const

[refImageFilter](#SkPaint_refImageFilter) returns [Paint_Image_Filter](#Paint_Image_Filter) if set, or nullptr.
[refImageFilter](#SkPaint_refImageFilter) increases [Image_Filter](#Image_Filter) [Reference_Count](#Reference_Count) by one.

#### Example

<fiddle-embed name="0828822500bc17539a248ca16a75ddc7"></fiddle-embed>

##### Example Output

~~~~
image filter unique: true
image filter unique: false
~~~~

---

<a name="SkPaint_setImageFilter"></a>
<!--?prettify lang=cc?-->

    void setImageFilter(sk_sp<SkImageFilter> imageFilter)

[setImageFilter](#SkPaint_setImageFilter) sets [Paint_Image_Filter](#Paint_Image_Filter) to imageFilter,
decrementing [Reference_Count](#Reference_Count) of the previous [Image_Filter](#Image_Filter). 
Pass nullptr to clear [Paint_Image_Filter](#Paint_Image_Filter), and remove [Image_Filter](#Image_Filter) effect
on drawing.
[setImageFilter](#SkPaint_setImageFilter) does not affect [Paint_Rasterizer](#Paint_Rasterizer) or [Paint_Mask_Filter](#Paint_Mask_Filter).
[setImageFilter](#SkPaint_setImageFilter) does not alter imageFilter [Reference_Count](#Reference_Count).

#### Example

<fiddle-embed name="7cac26cde809379c561c3af7af9c74db"></fiddle-embed>

---

## <a name="Paint_Draw_Looper"></a> Paint Draw Looper
[Paint_Draw_Looper](#Paint_Draw_Looper) sets a modifier that communicates state from one [Draw_Layer](#Draw_Layer)
to another to construct the draw.
[Draw_Looper](#Draw_Looper) draws one or more times, modifying the canvas and paint each time.
[Draw_Looper](#Draw_Looper) may be used to draw multiple colors or create a colored shadow.
Set [Paint_Draw_Looper](#Paint_Draw_Looper) to nullptr to prevent [Draw_Looper](#Draw_Looper) from modifying the draw. 

### Example

<fiddle-embed name="8de8294eee43c40e41c4cf5f15bb2b40"></fiddle-embed>

<a name="SkPaint_getDrawLooper"></a>
<!--?prettify lang=cc?-->

    SkDrawLooper* getDrawLooper() const

[getDrawLooper](#SkPaint_getDrawLooper) returns [Paint_Draw_Looper](#Paint_Draw_Looper) if set, or nullptr.
[getDrawLooper](#SkPaint_getDrawLooper) does not alter [Draw_Looper](#Draw_Looper) [Reference_Count](#Reference_Count).

#### Return Value

[Draw_Looper](#Draw_Looper) if previously set, nullptr otherwise.

#### Example

<fiddle-embed name="af4c5acc7a91e7f23c2af48018903ad4"></fiddle-embed>

##### Example Output

~~~~
nullptr == draw looper
nullptr != draw looper
~~~~

---

<a name="SkPaint_refDrawLooper"></a>
<!--?prettify lang=cc?-->

    sk_sp<SkDrawLooper> refDrawLooper() const

[refDrawLooper](#SkPaint_refDrawLooper) returns [Paint_Draw_Looper](#Paint_Draw_Looper) if set, or nullptr.
[refDrawLooper](#SkPaint_refDrawLooper) increases [Draw_Looper](#Draw_Looper) [Reference_Count](#Reference_Count) by one.

#### Example

<fiddle-embed name="bd18a131b759ef38033b0c8cf916d922"></fiddle-embed>

##### Example Output

~~~~
draw looper unique: true
draw looper unique: false
~~~~

---

<a name="SkPaint_getLooper"></a>
<!--?prettify lang=cc?-->

    SkDrawLooper* getLooper() const

6259---

<a name="SkPaint_setDrawLooper"></a>
<!--?prettify lang=cc?-->

    void setDrawLooper(sk_sp<SkDrawLooper> drawLooper)

[setDrawLooper](#SkPaint_setDrawLooper) sets [Paint_Draw_Looper](#Paint_Draw_Looper) to drawLooper,
decrementing [Reference_Count](#Reference_Count) of the previous drawLooper. 
Pass nullptr to clear [Paint_Draw_Looper](#Paint_Draw_Looper) and leave [Draw_Looper](#Draw_Looper) effect on drawing unaltered.
[setDrawLooper](#SkPaint_setDrawLooper) does not alter drawLooper [Reference_Count](#Reference_Count).

#### Example

<fiddle-embed name="b16bcd76b629922f476ce4a4e5aa0584"></fiddle-embed>

---

<a name="SkPaint_setLooper"></a>
<!--?prettify lang=cc?-->

    void setLooper(sk_sp<SkDrawLooper>)

6259---

## <a name="Paint_Text_Align"></a> Paint Text Align
[Paint_Text_Align](#Paint_Text_Align) adjusts the text relative to the text position.
[Paint_Text_Align](#Paint_Text_Align) affects glyphs drawn with [SkCanvas](#SkCanvas)::drawText, [SkCanvas](#SkCanvas)::drawPosText, and
[SkCanvas](#SkCanvas)::drawPosTextH, as well as calls that place text glyphs like getTextWidthds and [getTextPath](#SkPaint_getTextPath).

The text position is set by the font for both horizontal and vertical text.
Typically, for horizontal text, the position is to the left side of the glyph on the base line;
and for vertical text, the position is the horizontal center at the glyph at the caps height.

[Paint_Text_Align](#Paint_Text_Align) adjusts the glyph position to center it or move it to abut the position 
using the metrics returned by the font.

[Paint_Text_Align](#Paint_Text_Align) defaults to [kLeft_Align](#SkPaint_kLeft_Align).

### <a name="SkPaint_Align"></a> Enum SkPaint::Align



#### Constants

<table>
  <tr>
    <td><a name="SkPaint_kLeft_Align"></a> <code><strong>SkPaint::kLeft_Align</strong></code></td><td>0</td><td>[kLeft_Align](#SkPaint_kLeft_Align) leaves the glyph at the position computed by the font offset by the text position.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kCenter_Align"></a> <code><strong>SkPaint::kCenter_Align</strong></code></td><td>1</td><td>[kCenter_Align](#SkPaint_kCenter_Align) moves the glyph half its width if [Paint_Flags](#Paint_Flags) has [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) clear, and
half its height if [Paint_Flags](#Paint_Flags) has [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) set.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kRight_Align"></a> <code><strong>SkPaint::kRight_Align</strong></code></td><td>2</td><td>[kRight_Align](#SkPaint_kRight_Align) moves the glyph by its width if [Paint_Flags](#Paint_Flags) has [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) clear,
and by its height if [Paint_Flags](#Paint_Flags) has [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) set.</td>
  </tr>
</table>

### <a name="SkPaint_anonymous_2"></a> Enum SkPaint::anonymous_2



#### Constants

<table>
  <tr>
    <td><a name="SkPaint_kAlignCount"></a> <code><strong>SkPaint::kAlignCount</strong></code></td><td>3</td><td>[kAlignCount](#SkPaint_kAlignCount) is the number of different [Paint_Text_Align](#Paint_Text_Align) values defined.</td>
  </tr>
</table>

### Example

<fiddle-embed name="d317346b0bb75ecef4e6d8d94ad08664"></fiddle-embed>

Each position separately moves the glyph in drawPosText.### Example

<fiddle-embed name="e003042766521a607dbbaa653a565709"></fiddle-embed>

[Vertical_Text](#Vertical_Text) treats [kLeft_Align](#SkPaint_kLeft_Align) as top align, and [kRight_Align](#SkPaint_kRight_Align) as bottom align.<a name="SkPaint_getTextAlign"></a>
<!--?prettify lang=cc?-->

    Align getTextAlign() const

[getTextAlign](#SkPaint_getTextAlign) returns [Paint_Text_Align](#Paint_Text_Align).
[getTextAlign](#SkPaint_getTextAlign) returns [kLeft_Align](#SkPaint_kLeft_Align) if [Paint_Text_Align](#Paint_Text_Align) has not been set.

#### Example

<fiddle-embed name="70a061ac1c0ce260e1f8ec9f10e37285"></fiddle-embed>

##### Example Output

~~~~
kLeft_Align == default
~~~~

---

<a name="SkPaint_setTextAlign"></a>
<!--?prettify lang=cc?-->

    void    setTextAlign(Align align)

[setTextAlign](#SkPaint_setTextAlign) sets [Paint_Text_Align](#Paint_Text_Align) to align.
[setTextAlign](#SkPaint_setTextAlign) has no effect if align is an invalid value.

#### Example

<fiddle-embed name="02967ca885ff7b481070253c86493244"></fiddle-embed>

---

## <a name="Paint_Text_Size"></a> Paint Text Size
[Paint_Text_Size](#Paint_Text_Size) adjusts the overall text size in points.
[Paint_Text_Size](#Paint_Text_Size) can be set to any positive value or zero.
[Paint_Text_Size](#Paint_Text_Size) defaults to 12.
Set [SkPaintDefaults_TextSize](#SkPaintDefaults_TextSize) at compile time to change the default setting.

### Example

<fiddle-embed name="03cdf48119c978083f658fc8bf4385b7"></fiddle-embed>

<a name="SkPaint_getTextSize"></a>
<!--?prettify lang=cc?-->

    SkScalar getTextSize() const

[getTextSize](#SkPaint_getTextSize) returns [Paint_Text_Size](#Paint_Text_Size) in points.

#### Example

<fiddle-embed name="5d9555de745276df36e1087187fd7cf4"></fiddle-embed>

---

<a name="SkPaint_setTextSize"></a>
<!--?prettify lang=cc?-->

    void setTextSize(SkScalar textSize)

[setTextSize](#SkPaint_setTextSize) sets [Paint_Text_Size](#Paint_Text_Size) in points.
[setTextSize](#SkPaint_setTextSize) has no effect if textSize is not greater than or equal to zero.
 
#### Example

<fiddle-embed name="1cf7805047da9c02d21a447006c3fb09"></fiddle-embed>

---

## <a name="Paint_Text_Scale_X"></a> Paint Text Scale X
[Paint_Text_Scale_X](#Paint_Text_Scale_X) adjusts the text horizontal scale.
[Text](#Text) scaling approximates condensed and expanded type faces when the actual face
is not available.
[Paint_Text_Scale_X](#Paint_Text_Scale_X) can be set to any value.
[Paint_Text_Scale_X](#Paint_Text_Scale_X) defaults to 1.

### Example

<fiddle-embed name="c7ed964e579009590293249ac9c53b89"></fiddle-embed>

<a name="SkPaint_getTextScaleX"></a>
<!--?prettify lang=cc?-->

    SkScalar getTextScaleX() const

[getTextScaleX](#SkPaint_getTextScaleX) returns [Paint_Text_Scale_X](#Paint_Text_Scale_X).

#### Example

<fiddle-embed name="0afc0838b3e1279b663b7b721b20dfad"></fiddle-embed>

---

<a name="SkPaint_setTextScaleX"></a>
<!--?prettify lang=cc?-->

    void setTextScaleX(SkScalar scaleX)

[setTextScaleX](#SkPaint_setTextScaleX) sets [Paint_Text_Scale_X](#Paint_Text_Scale_X).
 
#### Example

<fiddle-embed name="60b1221813c58876e7a6922832b371aa"></fiddle-embed>

---

## <a name="Paint_Text_Skew_X"></a> Paint Text Skew X
[Paint_Text_Skew_X](#Paint_Text_Skew_X) adjusts the text horizontal skew.
[Text](#Text) skewing approximates italic and oblique type faces when the actual face
is not available.
[Paint_Text_Skew_X](#Paint_Text_Skew_X) can be set to any value.
[Paint_Text_Skew_X](#Paint_Text_Skew_X) defaults to 0.

### Example

<fiddle-embed name="b16bfa01960d8d9cd5fa7882fd046ba4"></fiddle-embed>

<a name="SkPaint_getTextSkewX"></a>
<!--?prettify lang=cc?-->

    SkScalar getTextSkewX() const

[getTextSkewX](#SkPaint_getTextSkewX) returns [Paint_Text_Skew_X](#Paint_Text_Skew_X).

#### Example

<fiddle-embed name="3425475634dab75c0e37bdae44d10cf4"></fiddle-embed>

---

<a name="SkPaint_setTextSkewX"></a>
<!--?prettify lang=cc?-->

    void setTextSkewX(SkScalar skewX)

[setTextSkewX](#SkPaint_setTextSkewX) sets [Paint_Text_Skew_X](#Paint_Text_Skew_X).

#### Example

<fiddle-embed name="e97458d2757bdfa72868f50d7c79ddcc"></fiddle-embed>

---

<a name="SkPaint_SetTextMatrix"></a>
<!--?prettify lang=cc?-->

    static SkMatrix* SetTextMatrix(SkMatrix* matrix, SkScalar size,

[SkScalar](#SkScalar) scaleX, [SkScalar](#SkScalar) skewX)

    Returns matrix, setting it to apply size, scaleX, and skewX.
    [SetTextMatrix](#SkPaint_SetTextMatrix) sets matrix to scale in the direction of the x-axis by size times scaleX,
    scale in the direction of the y-axis by size, and skew in the direction of the x-axis by skewX.
    matrix is returned as a convenience.

### Example

<fiddle-embed name="6617cfa316e85aef183e1661e7d95c5e"></fiddle-embed>

#### Example Output

~~~~
[ 12.0000   0.0000   0.0000][  0.0000  12.0000   0.0000][  0.0000   0.0000   1.0000]
~~~~

---

<a name="SkPaint_setTextMatrix"></a>
<!--?prettify lang=cc?-->

    SkMatrix* setTextMatrix(SkMatrix* matrix) const

Returns a [Matrix](#Matrix) that applies [Paint_Text_Size](#Paint_Text_Size), [Paint_Text_Scale_X](#Paint_Text_Scale_X), and [Paint_Text_Skew_X](#Paint_Text_Skew_X).
[SetTextMatrix](#SkPaint_SetTextMatrix) sets matrix to scale in the direction of the x-axis by size times scaleX,
scale in the direction of the y-axis by size, and skew in the direction of the x-axis by skewX.
matrix is returned as a convenience.

### Example

<fiddle-embed name="24d55d886bbd82fff7c8c65085697ca3"></fiddle-embed>

#### Example Output

~~~~
[ 12.0000   0.0000   0.0000][  0.0000  12.0000   0.0000][  0.0000   0.0000   1.0000]
~~~~

---

## <a name="Paint_Text_Encoding"></a> Paint Text Encoding
[Paint_Text_Encoding](#Paint_Text_Encoding) determines whether text specifies character codes and their encoded size,
or glyph indices. Character codes use the encoding specified by the[Unicode](#Unicode) standard # http://unicode.org/standard/standard.html. Character codes encoded size
are specified by [UTF-8](#UTF-8), [UTF-16](#UTF-16), or [UTF-32](#UTF-32). All character encoding are able to represent all of
[Unicode](#Unicode), differing only in the total storage required.

[UTF-8](#UTF-8) ([RFC](#RFC) 3629) # https://tools.ietf.org/html/rfc3629is made up of 8-bit bytes, 
and is a superset of [ASCII](#ASCII).
[UTF-16](#UTF-16) ([RFC](#RFC) 2781) # https://tools.ietf.org/html/rfc2781is made up of 16-bit words, 
and is a superset of [Unicode](#Unicode) ranges 0x0000 to 0xD7FF and 0xE000 to 0xFFFF.
[UTF-32](#UTF-32) # http://www.unicode.org/versions/[Unicode5](#Unicode5).0.0/ch03.pdfis made up of 32-bit words,
and is a superset of [Unicode](#Unicode).

[Font_Manager](#Font_Manager) uses font data to convert character codes into glyph indices. 
A glyph index is a 16-bit word.

[Paint_Text_Encoding](#Paint_Text_Encoding) is set to [kUTF8_TextEncoding](#SkPaint_kUTF8_TextEncoding) by default.

### <a name="SkPaint_TextEncoding"></a> Enum SkPaint::TextEncoding



#### Constants

<table>
  <tr>
    <td><a name="SkPaint_kUTF8_TextEncoding"></a> <code><strong>SkPaint::kUTF8_TextEncoding</strong></code></td><td>0</td><td>[kUTF8_TextEncoding](#SkPaint_kUTF8_TextEncoding) uses a series of bytes to represent [UTF-8](#UTF-8) or [ASCII](#ASCII).</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kUTF16_TextEncoding"></a> <code><strong>SkPaint::kUTF16_TextEncoding</strong></code></td><td>1</td><td>[kUTF16_TextEncoding](#SkPaint_kUTF16_TextEncoding) uses a series of two byte words to represent most of [Unicode](#Unicode).</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kUTF32_TextEncoding"></a> <code><strong>SkPaint::kUTF32_TextEncoding</strong></code></td><td>2</td><td>[kUTF32_TextEncoding](#SkPaint_kUTF32_TextEncoding) uses a series of four byte words to represent all of [Unicode](#Unicode).</td>
  </tr>
  <tr>
    <td><a name="SkPaint_kGlyphID_TextEncoding"></a> <code><strong>SkPaint::kGlyphID_TextEncoding</strong></code></td><td>3</td><td>[kGlyphID_TextEncoding](#SkPaint_kGlyphID_TextEncoding) uses a series of two byte words to represent glyph indices.</td>
  </tr>
</table>

### Example

<fiddle-embed name="8570b90dd0d10749c49cd8ff5f846be1"></fiddle-embed>

<a name="SkPaint_getTextEncoding"></a>
<!--?prettify lang=cc?-->

    TextEncoding getTextEncoding() const

[getTextEncoding](#SkPaint_getTextEncoding) returns [Paint_Text_Encoding](#Paint_Text_Encoding).

#### Example

<fiddle-embed name="f5277b17a3d17517dee0f54af727238c"></fiddle-embed>

##### Example Output

~~~~
kUTF8_TextEncoding == text encoding
kGlyphID_TextEncoding == text encoding
~~~~

---

<a name="SkPaint_setTextEncoding"></a>
<!--?prettify lang=cc?-->

    void setTextEncoding(TextEncoding encoding)

[setTextEncoding](#SkPaint_setTextEncoding) sets [Paint_Text_Encoding](#Paint_Text_Encoding) to encoding.
Invalid values for encoding are ignored.

#### Example

<fiddle-embed name="9f34027d218dda8a970ab43484be9c92"></fiddle-embed>

##### Example Output

~~~~
4 != text encoding
~~~~

---

## <a name="Paint_Font_Metrics"></a> Paint Font Metrics
[Paint_Font_Metrics](#Paint_Font_Metrics) describe dimensions common to the glyphs in [Paint_Typeface](#Paint_Typeface).
The dimensions are computed by [Font_Manager](#Font_Manager) from font data and do not take 
[Paint](#Paint) settings other than [Paint_Text_Size](#Paint_Text_Size) into account.

Font dimensions specify the anchor to the left of the glyph at baseline as the origin.
X-axis values to the left of the glyph are negative, and to the right of the left glyph edge
are positive.
Y-axis values above the baseline are negative, and below the baseline are positive.
 
### Example

<fiddle-embed name="6734e8817d75d95e2802b1d3f85a5fa6"></fiddle-embed>

512### <a name="SkPaint_FontMetrics"></a> Struct SkPaint::FontMetrics


[FontMetrics](#SkPaint_FontMetrics) is filled out by [getFontMetrics](#SkPaint_getFontMetrics). [FontMetrics](#SkPaint_FontMetrics) contents reflect the values
computed by [Font_Manager](#Font_Manager) using [Paint_Typeface](#Paint_Typeface). Values are set to zero if they are
not availble.

[fUnderlineThickness](#SkPaint_FontMetrics_fUnderlineThickness) and [fUnderlinePosition](#SkPaint_FontMetrics_fUnderlinePosition) have a bit set in [fFlags](#SkPaint_FontMetrics_fFlags) if their values
are valid, since their value may be zero.

#### <a name="SkPaint_FontMetrics_FontMetricsFlags"></a> Enum SkPaint::FontMetrics_FontMetricsFlags



[FontMetricsFlags](#SkPaint_FontMetrics_FontMetricsFlags) are set in [fFlags](#SkPaint_FontMetrics_fFlags) when underline metrics are valid;
the underline metric may be valid and zero.
Fonts with embedded bitmaps may not have valid underline metrics.

##### Constants

<table>
  <tr>
    <td><a name="SkPaint_FontMetrics_kUnderlineThinknessIsValid_Flag"></a> <code><strong>SkPaint::FontMetrics_kUnderlineThinknessIsValid_Flag</strong></code></td><td>0x0001</td><td>[kUnderlineThinknessIsValid_Flag](#SkPaint_FontMetrics_kUnderlineThinknessIsValid_Flag) set in [fFlags](#SkPaint_FontMetrics_fFlags) if [fUnderlineThickness](#SkPaint_FontMetrics_fUnderlineThickness) is valid.</td>
  </tr>
  <tr>
    <td><a name="SkPaint_FontMetrics_kUnderlinePositionIsValid_Flag"></a> <code><strong>SkPaint::FontMetrics_kUnderlinePositionIsValid_Flag</strong></code></td><td>0x0002</td><td>[kUnderlinePositionIsValid_Flag](#SkPaint_FontMetrics_kUnderlinePositionIsValid_Flag) set in [fFlags](#SkPaint_FontMetrics_fFlags) if [fUnderlinePosition](#SkPaint_FontMetrics_fUnderlinePosition) is valid.</td>
  </tr>
</table>

[fFlags](#SkPaint_FontMetrics_fFlags) is set when underline metrics are valid.[fTop](#SkPaint_FontMetrics_fTop) is the largest height for any glyph.
        [fTop](#SkPaint_FontMetrics_fTop) is a measure from the baseline, and is less than or equal to zero.[fAscent](#SkPaint_FontMetrics_fAscent) is the recommended distance above the baseline to reserve for a line of text.
        [fAscent](#SkPaint_FontMetrics_fAscent) is a measure from the baseline, and is less than or equal to zero.[fDescent](#SkPaint_FontMetrics_fDescent) is the recommended distance below the baseline to reserve for a line of text.
        [fDescent](#SkPaint_FontMetrics_fDescent) is a measure from the baseline, and is greater than or equal to zero.[fBottom](#SkPaint_FontMetrics_fBottom) is the greatest extent below the baseline for any glyph. 
        [fBottom](#SkPaint_FontMetrics_fBottom) is a measure from the baseline, and is greater than or equal to zero.[fLeading](#SkPaint_FontMetrics_fLeading) is the recommended distance to add between lines of text.
        [fLeading](#SkPaint_FontMetrics_fLeading) is greater than or equal to zero.[fAvgCharWidth](#SkPaint_FontMetrics_fAvgCharWidth) is the average character width, if it is available.
        [fAvgCharWidth](#SkPaint_FontMetrics_fAvgCharWidth) is zero if no average width is stored in the font.[fMaxCharWidth](#SkPaint_FontMetrics_fMaxCharWidth) is the max character width.[fXMin](#SkPaint_FontMetrics_fXMin) is the minimum bounding box x value for all glyphs. [fXMin](#SkPaint_FontMetrics_fXMin)
        is typically less than zero.[fXMax](#SkPaint_FontMetrics_fXMax) is the maximum bounding box x value for all glyphs. [fXMax](#SkPaint_FontMetrics_fXMax)
        is typically greater than zero.[fXHeight](#SkPaint_FontMetrics_fXHeight) is the height of a lower-case 'x'. [fXHeight](#SkPaint_FontMetrics_fXHeight) may be zero
        if no lower-case height is stored in the font.[fCapHeight](#SkPaint_FontMetrics_fCapHeight) is the height of an upper-case letter. [fCapHeight](#SkPaint_FontMetrics_fCapHeight) may be zero
        if no upper-case height is stored in the font.[fUnderlineThickness](#SkPaint_FontMetrics_fUnderlineThickness) is the underline thickness. If the metric
        is valid, the [kUnderlineThinknessIsValid_Flag](#SkPaint_FontMetrics_kUnderlineThinknessIsValid_Flag) is set in [fFlags](#SkPaint_FontMetrics_fFlags).
        If [kUnderlineThinknessIsValid_Flag](#SkPaint_FontMetrics_kUnderlineThinknessIsValid_Flag) is clear, [fUnderlineThickness](#SkPaint_FontMetrics_fUnderlineThickness) is zero.[fUnderlinePosition](#SkPaint_FontMetrics_fUnderlinePosition) is the underline position relative to the baseline.
       If may be negative, to draw the underline above the baseline, zero
       to draw the underline on the baseline, or positive to draw the underline
       below the baseline. 

       If the metric is valid, the [kUnderlinePositionIsValid_Flag](#SkPaint_FontMetrics_kUnderlinePositionIsValid_Flag) is set in [fFlags](#SkPaint_FontMetrics_fFlags).
       If [kUnderlinePositionIsValid_Flag](#SkPaint_FontMetrics_kUnderlinePositionIsValid_Flag) is clear, [fUnderlinePosition](#SkPaint_FontMetrics_fUnderlinePosition) is zero.<a name="SkPaint_FontMetrics_hasUnderlineThickness"></a>
<!--?prettify lang=cc?-->

    bool hasUnderlineThickness(SkScalar* thickness) const

thickness receives the underline width.

        If [Paint_Font_Metrics](#Paint_Font_Metrics) has a valid underline thickness, return true, and set 
        thickness to that value. If it doesn't, return false, and ignore
        thickness.---

<a name="SkPaint_FontMetrics_hasUnderlinePosition"></a>
<!--?prettify lang=cc?-->

    bool hasUnderlinePosition(SkScalar* position) const

position receives the underline offset from the baseline.

        If [Paint_Font_Metrics](#Paint_Font_Metrics) has a valid underline position, return true, and set 
        position to that value. If it doesn't, return false, and ignore
        position.---

<a name="SkPaint_getFontMetrics"></a>
<!--?prettify lang=cc?-->

    SkScalar getFontMetrics(FontMetrics* metrics, SkScalar scale = 0) const

[getFontMetrics](#SkPaint_getFontMetrics) returns [Paint_Font_Metrics](#Paint_Font_Metrics) associated with [Paint_Typeface](#Paint_Typeface).
The return value is the recommended spacing between lines: the sum of metrics
descent, ascent, and leading.
If metrics is not nullptr, [Paint_Font_Metrics](#Paint_Font_Metrics) is copied to metrics.
[getFontMetrics](#SkPaint_getFontMetrics) scales its results by [Paint_Text_Size](#Paint_Text_Size) but does not take into account
dimensions required by [Paint_Text_Scale_X](#Paint_Text_Scale_X), [Paint_Text_Skew_X](#Paint_Text_Skew_X), [Fake_Bold](#Fake_Bold),
[Style_Stroke](#Style_Stroke), and [Path_Effect](#Path_Effect).
[getFontMetrics](#SkPaint_getFontMetrics) results can be additionally scaled by scale.

#### Parameters

<table>
  <tr>
    <td><code><strong>metrics</strong></code></td> <td>If not null, [Paint_Font_Metrics](#Paint_Font_Metrics) for [Paint_Typeface](#Paint_Typeface) are copied here.</td>
  </tr>
  <tr>
    <td><code><strong>scale</strong></code></td> <td>If not 0, an additional multiplier for returned values.</td>
  </tr>
</table>

#### Return Value

The recommended spacing between lines.

#### Example

<fiddle-embed name="68cd6aefd0012ef68e5986569d588343"></fiddle-embed>

#### See Also

[Paint_Text_Size](#Paint_Text_Size) [Paint_Typeface](#Paint_Typeface) [Typeface](#Typeface)---

<a name="SkPaint_getFontSpacing"></a>
<!--?prettify lang=cc?-->

    SkScalar getFontSpacing() const

[getFontSpacing](#SkPaint_getFontSpacing) returns the recommended spacing between lines: the sum of metrics
descent, ascent, and leading.
[getFontSpacing](#SkPaint_getFontSpacing) scales its results by [Paint_Text_Size](#Paint_Text_Size) but does not take into account
dimensions required by stroking and [Path_Effect](#Path_Effect).
[getFontSpacing](#SkPaint_getFontSpacing) returns the same result as [getFontMetrics](#SkPaint_getFontMetrics).

#### Return Value

The recommended spacing between lines.

#### Example

<fiddle-embed name="a67cc9ac54b30e741f1b6b9334ad2ecc"></fiddle-embed>

##### Example Output

~~~~
textSize: 12 fontSpacing: 13.9688
textSize: 18 fontSpacing: 20.9531
textSize: 24 fontSpacing: 27.9375
textSize: 32 fontSpacing: 37.25
~~~~

---

<a name="SkPaint_getFontBounds"></a>
<!--?prettify lang=cc?-->

    SkRect getFontBounds() const

[getFontBounds](#SkPaint_getFontBounds) returns the union of the bounds of all glyphs.
[getFontBounds](#SkPaint_getFontBounds) dimensions are computed by [Font_Manager](#Font_Manager) from font data, 
ignoring [Paint_Hinting](#Paint_Hinting). [getFontBounds](#SkPaint_getFontBounds) includes [Paint_Text_Size](#Paint_Text_Size), [Paint_Text_Scale_X](#Paint_Text_Scale_X),
and [Paint_Text_Skew_X](#Paint_Text_Skew_X), but not [Fake_Bold](#Fake_Bold) or [Path_Effect](#Path_Effect).

If [Paint_Text_Size](#Paint_Text_Size) is large, [Paint_Text_Scale_X](#Paint_Text_Scale_X) is one, and [Paint_Text_Skew_X](#Paint_Text_Skew_X) is zero,
[getFontBounds](#SkPaint_getFontBounds) returns the same bounds as [Paint_Font_Metrics](#Paint_Font_Metrics) { [FontMetrics_fXMin](#), 
[FontMetrics_fTop](#), [FontMetrics_fXMax](#), [FontMetrics_fBottom](#) }.

#### Example

<fiddle-embed name="6a7b89373d7281a458c275ebb70e8d0e"></fiddle-embed>

##### Example Output

~~~~
metrics bounds = { -12.2461, -14.7891, 21.5215, 5.55469 }
font bounds    = { -12.2461, -14.7891, 21.5215, 5.55469 }
~~~~

---

<a name="SkPaint_textToGlyphs"></a>
<!--?prettify lang=cc?-->

    int textToGlyphs(const void* text, size_t byteLength,

[SkGlyphID](#SkGlyphID) glyphs[]) const

[textToGlyphs](#SkPaint_textToGlyphs) converts text into glyph indices.
[Paint_Text_Encoding](#Paint_Text_Encoding) specifies how text represents characters or glyphs.
[textToGlyphs](#SkPaint_textToGlyphs) returns the number of glyph indices represented by text.
glyphs may be nullptr, to compute the glyph count.

[textToGlyphs](#SkPaint_textToGlyphs) does not check text for valid character encoding or valid
glyph indices.

If byteLength equals zero, [textToGlyphs](#SkPaint_textToGlyphs) returns zero.
If byteLength includes a partial character, the partial character is ignored.

If [Paint_Text_Encoding](#Paint_Text_Encoding) is [kUTF8_TextEncoding](#SkPaint_kUTF8_TextEncoding) and
text contains an invalid [UTF-8](#UTF-8) sequence, zero is returned.

### Example

<fiddle-embed name="bfdd99089bdc3bdd2af67f3b6344b4ef"></fiddle-embed>

---

<a name="SkPaint_countText"></a>
<!--?prettify lang=cc?-->

    int countText(const void* text, size_t byteLength) const

[countText](#SkPaint_countText) returns the number of glyphs in text.
[countText](#SkPaint_countText) uses [Paint_Text_Encoding](#Paint_Text_Encoding) to count the glyphs.
[countText](#SkPaint_countText) returns the same result as [textToGlyphs](#SkPaint_textToGlyphs).

### Example

<fiddle-embed name="6e9152c7aa85b69bb25c90122e0f75a7"></fiddle-embed>

#### Example Output

~~~~
count = 4
~~~~

---

<a name="SkPaint_containsText"></a>
<!--?prettify lang=cc?-->

    bool containsText(const void* text, size_t byteLength) const

[containsText](#SkPaint_containsText) returns true if all text corresponds to a non-zero glyph index. 
[containsText](#SkPaint_containsText) returns false if any characters in text are not supported in
[Paint_Typeface](#Paint_Typeface).

If [Paint_Text_Encoding](#Paint_Text_Encoding) is [kGlyphID_TextEncoding](#SkPaint_kGlyphID_TextEncoding), [containsText](#SkPaint_containsText)
returns true if all glyph indices in text are non-zero; [containsText](#SkPaint_containsText)
does not check to see if text contains valid glyph indices for [Paint_Typeface](#Paint_Typeface).

[containsText](#SkPaint_containsText) returns true if bytelength is zero.

### Example

<fiddle-embed name="0e4177aaf9931893c6c80070a7bf44c2"></fiddle-embed>

#### Example Output

~~~~
0x00b0 == has char
0xd800 != has char
~~~~

### Example

<fiddle-embed name="556776e9b7065168d6640042dde42ab8"></fiddle-embed>

#### Example Output

~~~~
0x01ff == has glyph
0x0000 != has glyph
0xffff == has glyph
~~~~

### See Also

[setTextEncoding](#SkPaint_setTextEncoding) [Typeface](#Typeface)---

<a name="SkPaint_glyphsToUnichars"></a>
<!--?prettify lang=cc?-->

    void glyphsToUnichars(const SkGlyphID glyphs[], int count, SkUnichar text[]) const

[glyphsToUnichars](#SkPaint_glyphsToUnichars) converts glyphs into text if possible. 
[glyphsToUnichars](#SkPaint_glyphsToUnichars) is only supported on platforms that use [FreeType](#FreeType) as the [Font_Engine](#Font_Engine).
Glyph values without direct [Unicode](#Unicode) equivalents are mapped to zero. 
[glyphsToUnichars](#SkPaint_glyphsToUnichars) uses the [Paint_Typeface](#Paint_Typeface), but is unaffected
by [Paint_Text_Encoding](#Paint_Text_Encoding); the text values returned are equivalent to [kUTF32_TextEncoding](#SkPaint_kUTF32_TextEncoding).

### Example

<fiddle-embed name=""></fiddle-embed>

---

## <a name="Paint_Measure_Text"></a> Paint Measure Text
<a name="SkPaint_measureText"></a>
<!--?prettify lang=cc?-->

    SkScalar measureText(const void* text, size_t length, SkRect* bounds) const

Returns the advance width of text if [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) is clear,
and the height of text if [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) is set.
The advance is the normal distance to move before drawing additional text.
[measureText](#SkPaint_measureText) uses [Paint_Text_Encoding](#Paint_Text_Encoding) to decode text, [Paint_Typeface](#Paint_Typeface) to get the font metrics,
and [Paint_Text_Size](#Paint_Text_Size), [Paint_Text_Scale_X](#Paint_Text_Scale_X), [Paint_Text_Skew_X](#Paint_Text_Skew_X), [Stroke_Width](#Stroke_Width), and
[Path_Effect](#Path_Effect) to scale the metrics and bounds.
[measureText](#SkPaint_measureText) returns the bounding box of text if bounds is not nullptr.
The bounding box is computed as if the text was drawn at the origin.
 
#### Parameters

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

#### Return Value

The advance width or height.

#### Example

<fiddle-embed name="ed96dd912a1cbeeabb21b7ce386da003"></fiddle-embed>

---

<a name="SkPaint_measureText_2"></a>
<!--?prettify lang=cc?-->

    SkScalar measureText(const void* text, size_t length) const

Returns the adance width of text if [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) is clear,
and the height of text if [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) is set.
The advance is the normal distance to move before drawing additional text.
[measureText](#SkPaint_measureText) uses [Paint_Text_Encoding](#Paint_Text_Encoding) to decode text, [Paint_Typeface](#Paint_Typeface) to get the font metrics,
and [Paint_Text_Size](#Paint_Text_Size) to scale the metrics.
[measureText](#SkPaint_measureText) does not scale the advance or bounds by [Paint_Text_Decorations](#Paint_Text_Decorations)
or [Path_Effect](#Path_Effect).

#### Parameters

<table>
  <tr>
    <td><code><strong>text</strong></code></td> <td>Character codes or glyph indices to be measured.</td>
  </tr>
  <tr>
    <td><code><strong>length</strong></code></td> <td>Number of bytes of text to measure.</td>
  </tr>
</table>

#### Return Value

The advance width or height.

#### Example

<fiddle-embed name="1db5db3866773f1fe788b8ef38b7dd71"></fiddle-embed>

##### Example Output

~~~~
default width = 5
double width = 10
~~~~

---

<a name="SkPaint_breakText"></a>
<!--?prettify lang=cc?-->

    size_t  breakText(const void* text, size_t length, SkScalar maxWidth,

[SkScalar](#SkScalar)* measuredWidth = [NULL](#NULL)) const

    [breakText](#SkPaint_breakText) returns the bytes of text that fit within maxWidth.
    If [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) is clear, the text fragment fits if its advance width is less than or
    equal to maxWidth.
    If [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) is set, the text fragment fits if its advance height is less than or
    equal to maxWidth.
    [breakText](#SkPaint_breakText) measures only while the advance is less than or equal to maxWidth.
    [breakText](#SkPaint_breakText) returns the advance or the text fragment in measureWidth if it not nullptr.
    [breakText](#SkPaint_breakText) uses [Paint_Text_Encoding](#Paint_Text_Encoding) to decode text, [Paint_Typeface](#Paint_Typeface) to get the font metrics,
    and [Paint_Text_Size](#Paint_Text_Size) to scale the metrics.
    [breakText](#SkPaint_breakText) does not scale the advance or bounds by [Paint_Text_Decorations](#Paint_Text_Decorations)
    or [Path_Effect](#Path_Effect).

#### Parameters

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

#### Return Value

The bytes of text that fit, always less than or equal to length.

#### Example

<fiddle-embed name="50698c82c7cc4510564cac15efaeef6e"></fiddle-embed>

---

<a name="SkPaint_getTextWidths"></a>
<!--?prettify lang=cc?-->

    int getTextWidths(const void* text, size_t byteLength, SkScalar widths[],

[SkRect](#SkRect) bounds[] = [NULL](#NULL)) const

    [getTextWidths](#SkPaint_getTextWidths) retrieves the advance and bounds for each glyph in text, and returns
    the glyph count in text.
    Both widths and bounds may be nullptr.
    If widths is not nullptr, widths must be an array of glyph count entries.
    if bounds is not nullptr, bounds must be an array of glyph count entries. 
    If [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) is clear, widths returns the horizontal advance.
    If [kVerticalText_Flag](#SkPaint_kVerticalText_Flag) is set, widths returns the vertical advance.
    [getTextWidths](#SkPaint_getTextWidths) uses [Paint_Text_Encoding](#Paint_Text_Encoding) to decode text, [Paint_Typeface](#Paint_Typeface) to get the font metrics,
    and [Paint_Text_Size](#Paint_Text_Size) to scale the widths and bounds.
    [getTextWidths](#SkPaint_getTextWidths) does not scale the advance by [Paint_Text_Decorations](#Paint_Text_Decorations) or [Path_Effect](#Path_Effect).
    [getTextWidths](#SkPaint_getTextWidths) does return include [Paint_Text_Decorations](#Paint_Text_Decorations) and [Path_Effect](#Path_Effect) in the bounds.
   
#### Parameters

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

#### Return Value

The glyph count in text.

#### Example

<fiddle-embed name="31f29360964e867e5569291d9b53a2aa"></fiddle-embed>

---

## <a name="Paint_Text_Path"></a> Paint Text Path
[Paint_Text_Path](#Paint_Text_Path) describes the geometry of glyphs used to draw text.

<a name="SkPaint_getTextPath"></a>
<!--?prettify lang=cc?-->

    void getTextPath(const void* text, size_t length, SkScalar x, SkScalar y,

[SkPath](#SkPath)* path) const

[getTextPath](#SkPaint_getTextPath) returns the geometry as [Path](#Path) equivalent to the drawn text.
[getTextPath](#SkPaint_getTextPath) uses [Paint_Text_Encoding](#Paint_Text_Encoding) to decode text, [Paint_Typeface](#Paint_Typeface) to get the glyph paths,
and [Paint_Text_Size](#Paint_Text_Size), [Paint_Text_Decorations](#Paint_Text_Decorations), and [Path_Effect](#Path_Effect) to scale and modify the glyph paths.
All of the glyph paths are stored in path.
[getTextPath](#SkPaint_getTextPath) uses x, y, and [Paint_Text_Align](#Paint_Text_Align) to position path.

#### Parameters

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

#### Example

<fiddle-embed name="4419b133092615ede9c32fbc3b567d1c"></fiddle-embed>

---

<a name="SkPaint_getPosTextPath"></a>
<!--?prettify lang=cc?-->

    void getPosTextPath(const void* text, size_t length,

const [SkPoint](#SkPoint) pos[], [SkPath](#SkPath)* path) const

[getPosTextPath](#SkPaint_getPosTextPath) returns the geometry as [Path](#Path) equivalent to the drawn text.
[getPosTextPath](#SkPaint_getPosTextPath) uses [Paint_Text_Encoding](#Paint_Text_Encoding) to decode text, [Paint_Typeface](#Paint_Typeface) to get the glyph paths,
and [Paint_Text_Size](#Paint_Text_Size), [Paint_Text_Decorations](#Paint_Text_Decorations), and [Path_Effect](#Path_Effect) to scale and modify the glyph paths.
All of the glyph paths are stored in path.
[getPosTextPath](#SkPaint_getPosTextPath) uses pos array and [Paint_Text_Align](#Paint_Text_Align) to position path.
pos contains a position for each glyph.

#### Parameters

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

#### Example

<fiddle-embed name="f7dc4fad0676d7c6bf0771f406eff51e"></fiddle-embed>

---

## <a name="Paint_Text_Intercepts"></a> Paint Text Intercepts
[Paint_Text_Intercepts](#Paint_Text_Intercepts) describe the intersection of drawn text glyphs with a pair
of lines parallel to the text advance. [Paint_Text_Intercepts](#Paint_Text_Intercepts) permits creating a
underline that skips descenders.

<a name="SkPaint_getTextIntercepts"></a>
<!--?prettify lang=cc?-->

    int getTextIntercepts(const void* text, size_t length, SkScalar x, SkScalar y,

const [SkScalar](#SkScalar) bounds[2], [SkScalar](#SkScalar)* intervals) const

    [getTextIntercepts](#SkPaint_getTextIntercepts) returns the number of intervals that intersect bounds.
    bounds describes a pair of lines parallel to the text advance.
    The return count is zero or a multiple of two, and is at most twice the number of glyphs in
    the string. 
    [getTextIntercepts](#SkPaint_getTextIntercepts) uses [Paint_Text_Encoding](#Paint_Text_Encoding) to decode text, [Paint_Typeface](#Paint_Typeface) to get the glyph paths,
    and [Paint_Text_Size](#Paint_Text_Size), [Paint_Text_Decorations](#Paint_Text_Decorations), and [Path_Effect](#Path_Effect) to scale and modify the glyph paths.
    [getTextIntercepts](#SkPaint_getTextIntercepts) uses x, y, and [Paint_Text_Align](#Paint_Text_Align) to position intervals.
    
    Pass nullptr for intervals to determine the size of the interval array.
    
    intervals are cached to improve performance for multiple calls.

#### Parameters

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

#### Return Value

The number of intersections, which may be zero.

#### Example

<fiddle-embed name="31dd0970f3200545427390788636fbe4"></fiddle-embed>

---

<a name="SkPaint_getPosTextIntercepts"></a>
<!--?prettify lang=cc?-->

    int getPosTextIntercepts(const void* text, size_t length, const SkPoint pos[],

const [SkScalar](#SkScalar) bounds[2], [SkScalar](#SkScalar)* intervals) const

    [getPosTextIntercepts](#SkPaint_getPosTextIntercepts) returns the number of intervals that intersect bounds.
    bounds describes a pair of lines parallel to the text advance.
    The return count is zero or a multiple of two, and is at most twice the number of glyphs in
    the string. 
    [getPosTextIntercepts](#SkPaint_getPosTextIntercepts) uses [Paint_Text_Encoding](#Paint_Text_Encoding) to decode text, [Paint_Typeface](#Paint_Typeface) to get the glyph paths,
    and [Paint_Text_Size](#Paint_Text_Size), [Paint_Text_Decorations](#Paint_Text_Decorations), and [Path_Effect](#Path_Effect) to scale and modify the glyph paths.
    [getPosTextIntercepts](#SkPaint_getPosTextIntercepts) uses pos array and [Paint_Text_Align](#Paint_Text_Align) to position intervals.
    
    Pass nullptr for intervals to determine the size of the interval array.
    
    intervals are cached to improve performance for multiple calls.

#### Parameters

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

#### Return Value

The number of intersections, which may be zero.

#### Example

<fiddle-embed name="5c7ca6a2c8896e4717d1d7f92526f8aa"></fiddle-embed>

---

<a name="SkPaint_getPosTextHIntercepts"></a>
<!--?prettify lang=cc?-->

    int getPosTextHIntercepts(const void* text, size_t length, const SkScalar xpos[],

[SkScalar](#SkScalar) constY, const [SkScalar](#SkScalar) bounds[2], [SkScalar](#SkScalar)* intervals) const

    [getPosTextHIntercepts](#SkPaint_getPosTextHIntercepts) returns the number of intervals that intersect bounds.
    bounds describes a pair of lines parallel to the text advance.
    The return count is zero or a multiple of two, and is at most twice the number of glyphs in
    the string. 
    [getPosTextHIntercepts](#SkPaint_getPosTextHIntercepts) uses [Paint_Text_Encoding](#Paint_Text_Encoding) to decode text, [Paint_Typeface](#Paint_Typeface) to get the glyph paths,
    and [Paint_Text_Size](#Paint_Text_Size), [Paint_Text_Decorations](#Paint_Text_Decorations), and [Path_Effect](#Path_Effect) to scale and modify the glyph paths.
    [getPosTextHIntercepts](#SkPaint_getPosTextHIntercepts) uses xpos array, constY, and [Paint_Text_Align](#Paint_Text_Align) to position intervals.
    
    Pass nullptr for intervals to determine the size of the interval array.
    
    intervals are cached to improve performance for multiple calls.

#### Parameters

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

#### Return Value

The number of intersections, which may be zero.

#### Example

<fiddle-embed name="8f15e8305035afa24224bdc92a4cfb50"></fiddle-embed>

---

<a name="SkPaint_getTextBlobIntercepts"></a>
<!--?prettify lang=cc?-->

    int getTextBlobIntercepts(const SkTextBlob* blob, const SkScalar bounds[2],

[SkScalar](#SkScalar)* intervals) const

    [getPosTextIntercepts](#SkPaint_getPosTextIntercepts) returns the number of intervals that intersect bounds.
    bounds describes a pair of lines parallel to the text advance.
    The return count is zero or a multiple of two, and is at most twice the number of glyphs in
    the string. 
    [getPosTextIntercepts](#SkPaint_getPosTextIntercepts) uses [Paint_Text_Encoding](#Paint_Text_Encoding) to decode text, [Paint_Typeface](#Paint_Typeface) to get the glyph paths,
    and [Paint_Text_Size](#Paint_Text_Size), [Paint_Text_Decorations](#Paint_Text_Decorations), and [Path_Effect](#Path_Effect) to scale and modify the glyph paths.
    [getPosTextIntercepts](#SkPaint_getPosTextIntercepts) uses pos array and [Paint_Text_Align](#Paint_Text_Align) to position intervals.
    
    Pass nullptr for intervals to determine the size of the interval array.
    
    intervals are cached to improve performance for multiple calls.

#### Parameters

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

#### Return Value

The number of intersections, which may be zero.

#### Example

<fiddle-embed name="c4e51f1d95f7ab511c47a6ea6d4a3311"></fiddle-embed>

---

<a name="SkPaint_nothingToDraw"></a>
<!--?prettify lang=cc?-->

    bool nothingToDraw() const

[nothingToDraw](#SkPaint_nothingToDraw) returns true if [Paint](#Paint) prevents all drawing.
If [nothingToDraw](#SkPaint_nothingToDraw) returns false, the [Paint](#Paint) is not guaranteed to allow drawing.

[nothingToDraw](#SkPaint_nothingToDraw) returns true if [Paint_Blend_Mode](#Paint_Blend_Mode) and [Paint_Alpha](#Paint_Alpha) are enabled,
and set the computed [Color_Alpha](#Color_Alpha) to zero.

### Example

<fiddle-embed name="2fbbed1e035f836728a74673e6977c28"></fiddle-embed>

#### Example Output

~~~~
nothing to draw: false
nothing to draw: true
nothing to draw: false
nothing to draw: true
~~~~

---

## <a name="Paint_Fast_Bounds"></a> Paint Fast Bounds
[Paint_Fast_Bounds](#Paint_Fast_Bounds) methods conservatively outset a drawing bounds by additional area
[Paint](#Paint) may draw to.

<a name="SkPaint_canComputeFastBounds"></a>
<!--?prettify lang=cc?-->

    bool canComputeFastBounds() const

[canComputeFastBounds](#SkPaint_canComputeFastBounds) returns true if the current paint settings allow for fast computation of
bounds (i.e. there is nothing complex like a patheffect that would make
the bounds computation expensive.

#### Example

<fiddle-embed name="04311b628fe9828d9ed921a36185db26"></fiddle-embed>

---

<a name="SkPaint_computeFastBounds"></a>
<!--?prettify lang=cc?-->

    const SkRect& computeFastBounds(const SkRect& orig, SkRect* storage) const

Only call this if [canComputeFastBounds](#SkPaint_canComputeFastBounds)() returned true. This takes a
raw rectangle (the raw bounds of a shape), and adjusts it for stylistic
effects in the paint (e.g. stroking). If needed, it uses the storage
rect parameter. It returns the adjusted bounds that can then be used
for quickReject tests.
The returned rect will either be orig or storage, thus the caller
should not rely on storage being set to the result, but should always
use the retured value. It is legal for orig and storage to be the same
rect.

#### Example

<fiddle-embed name="80634aec853885c120332095e25e1852"></fiddle-embed>

---

<a name="SkPaint_computeFastStrokeBounds"></a>
<!--?prettify lang=cc?-->

    const SkRect& computeFastStrokeBounds(const SkRect& orig,

[SkRect](#SkRect)* storage) const#### Example

<fiddle-embed name="04311b628fe9828d9ed921a36185db26"></fiddle-embed>

---

<a name="SkPaint_doComputeFastBounds"></a>
<!--?prettify lang=cc?-->

    const SkRect& doComputeFastBounds(const SkRect& orig, SkRect* storage,

[Style](#SkPaint_Style)) const

    Take the style explicitly, so the caller can force us to be stroked
    without having to make a copy of the paint just to change that field.

#### Example

<fiddle-embed name="04311b628fe9828d9ed921a36185db26"></fiddle-embed>

---

<a name="SkPaint_toString"></a>
<!--?prettify lang=cc?-->

    void toString(SkString* str) const;

Converts [Paint](#Paint) to machine parsable form in developer mode.

### Example

<fiddle-embed name="072356aee21483fbb454d01ebf31ef28"></fiddle-embed>

#### Example Output

~~~~
text size = 12
~~~~

---

