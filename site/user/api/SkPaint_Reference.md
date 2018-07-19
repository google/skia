SkPaint Reference
===

# <a name='Paint'>Paint</a>

# <a name='SkPaint'>Class SkPaint</a>
<a href='#Paint'>Paint</a> controls options applied when drawing and measuring. <a href='#Paint'>Paint</a> collects all
options outside of the <a href='SkCanvas_Reference#Clip'>Canvas Clip</a> and <a href='SkCanvas_Reference#Matrix'>Canvas Matrix</a>.

Various options apply to text, strokes and fills, and images.

Some options may not be implemented on all platforms; in these cases, setting
the option has no effect. Some options are conveniences that duplicate <a href='SkCanvas_Reference#Canvas'>Canvas</a>
functionality; for instance, text size is identical to matrix scale.

<a href='#Paint'>Paint</a> options are rarely exclusive; each option modifies a stage of the drawing
pipeline and multiple pipeline stages may be affected by a single <a href='#Paint'>Paint</a>.

<a href='#Paint'>Paint</a> collects effects and filters that describe single-pass and multiple-pass
algorithms that alter the drawing geometry, color, and transparency. For instance,
<a href='#Paint'>Paint</a> does not directly implement dashing or blur, but contains the objects that do so.

The objects contained by <a href='#Paint'>Paint</a> are opaque, and cannot be edited outside of the <a href='#Paint'>Paint</a>
to affect it. The implementation is free to defer computations associated with the
<a href='#Paint'>Paint</a>, or ignore them altogether. For instance, some GPU implementations draw all
<a href='SkPath_Reference#Path'>Path</a> geometries with <a href='#Anti_Alias'>Anti Aliasing</a>, regardless of how <a href='#SkPaint_kAntiAlias_Flag'>SkPaint::kAntiAlias Flag</a>
is set in <a href='#Paint'>Paint</a>.

<a href='#Paint'>Paint</a> describes a single color, a single font, a single image quality, and so on.
Multiple colors are drawn either by using multiple paints or with objects like
<a href='undocumented#Shader'>Shader</a> attached to <a href='#Paint'>Paint</a>.

## Overview

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constant'>Constants</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>enum and enum class, and their const values</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constructor'>Constructors</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>functions that construct <a href='#SkPaint'>SkPaint</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member_Function'>Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>global and class member functions</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Operator'>Operators</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>operator overloading methods</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Related_Function'>Related Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>similar member functions grouped together</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Struct'>Struct Declarations</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>embedded struct members</td>
  </tr>
</table>


## <a name='Related_Function'>Related Function</a>


SkPaint global, <code>struct</code>, and <code>class</code> related member functions share a topic.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Alpha_Methods'>Alpha Methods</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>get and set <a href='SkColor_Reference#Alpha'>Alpha</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Anti_Alias'>Anti Alias</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>approximating coverage with transparency</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Automatic_Hinting'>Automatic Hinting</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>always adjust glyph paths</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Blend_Mode_Methods'>Blend Mode Methods</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>get and set <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Filter_Methods'>Color Filter Methods</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>get and set <a href='undocumented#Color_Filter'>Color Filter</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Methods'>Color Methods</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>get and set <a href='SkColor_Reference#Color'>Color</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Device_Text'>Device Text</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>increase precision of glyph position</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Dither'>Dither</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>distributing color error</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Draw_Looper_Methods'>Draw Looper Methods</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>get and set <a href='undocumented#Draw_Looper'>Draw Looper</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Fake_Bold'>Fake Bold</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>approximate font styles</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Fast_Bounds'>Fast Bounds</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>approximate area required by <a href='#Paint'>Paint</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Fill_Path'>Fill Path</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>make <a href='SkPath_Reference#Path'>Path</a> from <a href='undocumented#Path_Effect'>Path Effect</a>, stroking</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Filter_Quality_Methods'>Filter Quality Methods</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>get and set <a href='undocumented#Filter_Quality'>Filter Quality</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_Flags'>Flags</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>attributes represented by single bits</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>custom sized bitmap <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Font_Metrics'>Font Metrics</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>common glyph dimensions</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Full_Hinting_Spacing'>Full Hinting Spacing</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>glyph spacing affected by hinting</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_Hinting'>Hinting</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>glyph outline adjustment</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Image_Filter_Methods'>Image Filter Methods</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>get and set <a href='undocumented#Image_Filter'>Image Filter</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Initializers'>Initializers</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructors and initialization</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#LCD_Text'>LCD Text</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>text relying on the order of RGB stripes</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Linear_Text'>Linear Text</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>selects text rendering as <a href='undocumented#Glyph'>Glyph</a> or <a href='SkPath_Reference#Path'>Path</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Management'>Management</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>paint copying, moving, comparing</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Mask_Filter_Methods'>Mask Filter Methods</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>get and set <a href='undocumented#Mask_Filter'>Mask Filter</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Measure_Text'>Measure Text</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>width, height, bounds of text</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Miter_Limit'>Miter Limit</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>maximum length of stroked corners</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Path_Effect_Methods'>Path Effect Methods</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>get and set <a href='undocumented#Path_Effect'>Path Effect</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Shader_Methods'>Shader Methods</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>get and set <a href='undocumented#Shader'>Shader</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Stroke_Cap'>Stroke Cap</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>decorations at ends of open strokes</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Stroke_Join'>Stroke Join</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>decoration at corners of strokes</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Stroke_Width'>Stroke Width</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>thickness perpendicular to geometry</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_Style'>Style</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>geometry filling, stroking</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Style_Fill'>Style Fill</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>fill and stroke</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Style_Hairline'>Style Hairline</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>lines and curves with minimal width</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Style_Stroke'>Style Stroke</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>lines and curves with width</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Subpixel_Text'>Subpixel Text</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>uses pixel transparency to represent fractional offset</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Text_Align'>Text Align</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>text placement relative to position</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Text_Encoding'>Text Encoding</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>text encoded as characters or <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Text_Intercepts'>Text Intercepts</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>advanced underline, strike through</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Text_Path'>Text Path</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>geometry of <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Text_Scale_X'>Text Scale X</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>text horizontal scale</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Text_Size'>Text Size</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>overall height in points</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Text_Skew_X'>Text Skew X</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>text horizontal slant</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Typeface_Methods'>Typeface Methods</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>get and set <a href='undocumented#Typeface'>Typeface</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Utility'>Utility</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>rarely called management functions</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Vertical_Text'>Vertical Text</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>orient text from top to bottom</td>
  </tr>
</table>

## <a name='Constant'>Constant</a>


SkPaint related constants are defined by <code>enum</code>, <code>enum class</code>,  <code>#define</code>, <code>const</code>, and <code>constexpr</code>.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_Align'>Align</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>glyph locations relative to text position</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_Cap'>Cap</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>start and end geometry on stroked shapes</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_Flags'>Flags</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>values described by bits and masks</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_Hinting'>Hinting</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>level of glyph outline adjustment</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_Join'>Join</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>corner geometry on stroked shapes</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_Style'>Style</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>stroke, fill, or both</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_TextEncoding'>TextEncoding</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>character or glyph encoded size</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kAlignCount'>kAlignCount</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>number of different <a href='#Text_Align'>Text Align</a> values defined</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kAllFlags'>kAllFlags</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>mask of all <a href='#SkPaint_Flags'>Flags</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias Flag</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>mask for setting <a href='#Anti_Alias'>Anti Alias</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting Flag</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>mask for setting <a href='#Automatic_Hinting'>Automatic Hinting</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kBevel_Join'>kBevel Join</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>connects outside edges</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kButt_Cap'>kButt Cap</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>no stroke extension</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kCapCount'>kCapCount</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>number of different <a href='#Stroke_Cap'>Stroke Cap</a> values defined</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kCenter_Align'>kCenter Align</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>centers line of glyphs by its width or height</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kDefault_Cap'>kDefault Cap</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>equivalent to <a href='#SkPaint_kButt_Cap'>kButt Cap</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kDefault_Join'>kDefault Join</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>equivalent to <a href='#SkPaint_kMiter_Join'>kMiter Join</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kDither_Flag'>kDither Flag</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>mask for setting <a href='#Dither'>Dither</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText Flag</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>mask for setting <a href='#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText Flag</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>mask for setting <a href='#Fake_Bold'>Fake Bold</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kFill_Style'>kFill Style</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>set to fill geometry</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kFull_Hinting'>kFull Hinting</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>modifies glyph outlines for maximum constrast</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID TextEncoding</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>uses two byte words to represent glyph indices</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kJoinCount'>kJoinCount</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>number of different <a href='#Stroke_Join'>Stroke Join</a> values defined</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText Flag</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>mask for setting <a href='#LCD_Text'>LCD Text</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kLast_Cap'>kLast Cap</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>largest <a href='#Stroke_Cap'>Stroke Cap</a> value</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kLast_Join'>kLast Join</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>equivalent to the largest value for <a href='#Stroke_Join'>Stroke Join</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kLeft_Align'>kLeft Align</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>positions glyph by computed font offset</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kLinearText_Flag'>kLinearText Flag</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>mask for setting <a href='#Linear_Text'>Linear Text</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kMiter_Join'>kMiter Join</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>extends to <a href='#Miter_Limit'>Miter Limit</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kNo_Hinting'>kNo Hinting</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>glyph outlines unchanged</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>glyph outlines modified to improve constrast</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kRight_Align'>kRight Align</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>moves lines of glyphs by its width or height</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kRound_Cap'>kRound Cap</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds circle</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kRound_Join'>kRound Join</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds circle</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kSlight_Hinting'>kSlight Hinting</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>minimal modification to improve constrast</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kSquare_Cap'>kSquare Cap</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds square</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill Style</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to stroke and fill geometry</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kStroke_Style'>kStroke Style</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>set to stroke geometry</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kStyleCount'>kStyleCount</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>number of different <a href='#SkPaint_Style'>Style</a> values defined</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText Flag</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>mask for setting <a href='#Subpixel_Text'>Subpixel Text</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kUTF16_TextEncoding'>kUTF16 TextEncoding</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>uses two byte words to represent most of Unicode</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kUTF32_TextEncoding'>kUTF32 TextEncoding</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>uses four byte words to represent all of Unicode</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kUTF8_TextEncoding'>kUTF8 TextEncoding</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>uses bytes to represent UTF-8 or ASCII</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>mask for setting <a href='#Vertical_Text'>Vertical Text</a></td>
  </tr>
</table>

## <a name='Struct'>Struct</a>


SkPaint uses C++ structs to declare the public data structures and interfaces.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics'>FontMetrics</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>values computed by <a href='undocumented#Font_Manager'>Font Manager</a> using <a href='undocumented#Typeface'>Typeface</a></td>
  </tr>
</table>

## <a name='Constructor'>Constructor</a>


SkPaint can be constructed or initialized by these functions, including C++ class constructors.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_empty_constructor'>SkPaint()</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs with default values</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_move_SkPaint'>SkPaint(SkPaint&& paint)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>moves paint without copying it</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_copy_const_SkPaint'>SkPaint(const SkPaint& paint)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>makes a shallow copy</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_destructor'>~SkPaint()</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>decreases <a href='undocumented#Reference_Count'>Reference Count</a> of owned objects</td>
  </tr>
</table>

## <a name='Operator'>Operator</a>


SkPaint operators inline class member functions with arithmetic equivalents.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_notequal_operator'>operator!=(const SkPaint& a, const SkPaint& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>compares paints for inequality</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_move_operator'>operator=(SkPaint&& paint)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>moves paint without copying it</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_copy_operator'>operator=(const SkPaint& paint)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>makes a shallow copy</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_equal_operator'>operator==(const SkPaint& a, const SkPaint& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>compares paints for equality</td>
  </tr>
</table>

## <a name='Member_Function'>Member Function</a>


SkPaint member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_breakText'>breakText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns text that fits in a width</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_canComputeFastBounds'>canComputeFastBounds</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if settings allow for fast bounds computation</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_computeFastBounds'>computeFastBounds</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns fill bounds for quick reject tests</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_computeFastStrokeBounds'>computeFastStrokeBounds</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns stroke bounds for quick reject tests</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_containsText'>containsText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if all text corresponds to <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_countText'>countText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns number of <a href='undocumented#Glyph'>Glyphs</a> in text</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_doComputeFastBounds'>doComputeFastBounds</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns bounds for quick reject tests</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getAlpha'>getAlpha</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='SkColor_Reference#Alpha'>Color Alpha</a>, color opacity</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getBlendMode'>getBlendMode</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a>, how colors combine with <a href='undocumented#Device'>Device</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getColor'>getColor</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='SkColor_Reference#Alpha'>Color Alpha</a> and RGB, one drawing color</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getColorFilter'>getColorFilter</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#Color_Filter'>Color Filter</a>, how colors are altered</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getDrawLooper'>getDrawLooper</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#Draw_Looper'>Draw Looper</a>, multiple layers</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getFillPath'>getFillPath</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns fill path equivalent to stroke</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getFilterQuality'>getFilterQuality</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#Filter_Quality'>Filter Quality</a>, image filtering level</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getFlags'>getFlags</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkPaint_Flags'>Flags</a> stored in a bit field</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getFontBounds'>getFontBounds</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns union all glyph bounds</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getFontMetrics'>getFontMetrics</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#Typeface'>Typeface</a> metrics scaled by text size</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getFontSpacing'>getFontSpacing</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns recommended spacing between lines</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getHash'>getHash</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns a shallow hash for equality checks</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getHinting'>getHinting</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkPaint_Hinting'>Hinting</a>, glyph outline adjustment level</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getImageFilter'>getImageFilter</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#Image_Filter'>Image Filter</a>, alter pixels; blur</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getMaskFilter'>getMaskFilter</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#Mask_Filter'>Mask Filter</a>, alterations to <a href='undocumented#Mask_Alpha'>Mask Alpha</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getPathEffect'>getPathEffect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#Path_Effect'>Path Effect</a>, modifications to path geometry; dashing</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getPosTextHIntercepts'>getPosTextHIntercepts</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns where lines intersect horizontally positioned text; underlines</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getPosTextIntercepts'>getPosTextIntercepts</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns where lines intersect positioned text; underlines</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getPosTextPath'>getPosTextPath</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='SkPath_Reference#Path'>Path</a> equivalent to positioned text</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getShader'>getShader</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#Shader'>Shader</a>, multiple drawing colors; gradients</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getStrokeCap'>getStrokeCap</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkPaint_Cap'>Cap</a>, the area drawn at path ends</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkPaint_Join'>Join</a>, geometry on path corners</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getStrokeMiter'>getStrokeMiter</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Miter_Limit'>Miter Limit</a>, angles with sharp corners</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getStrokeWidth'>getStrokeWidth</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns thickness of the stroke</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getStyle'>getStyle</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkPaint_Style'>Style</a>: stroke, fill, or both</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getTextAlign'>getTextAlign</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkPaint_Align'>Align</a>: left, center, or right</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getTextBlobIntercepts'>getTextBlobIntercepts</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns where lines intersect <a href='undocumented#Text_Blob'>Text Blob</a>; underlines</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getTextEncoding'>getTextEncoding</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns character or glyph encoded size</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getTextIntercepts'>getTextIntercepts</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns where lines intersect text; underlines</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getTextPath'>getTextPath</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='SkPath_Reference#Path'>Path</a> equivalent to text</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getTextScaleX'>getTextScaleX</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns the text horizontal scale; condensed text</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getTextSize'>getTextSize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns text size in points</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getTextSkewX'>getTextSkewX</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns the text horizontal skew; oblique text</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getTextWidths'>getTextWidths</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns advance and bounds for each glyph in text</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_getTypeface'>getTypeface</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#Typeface'>Typeface</a>, font description</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_glyphsToUnichars'>glyphsToUnichars</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>converts <a href='undocumented#Glyph'>Glyphs</a> into text</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_isAntiAlias'>isAntiAlias</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#Anti_Alias'>Anti Alias</a> is set</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_isAutohinted'>isAutohinted</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='undocumented#Glyph'>Glyphs</a> are always hinted</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_isDither'>isDither</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#Dither'>Dither</a> is set</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_isEmbeddedBitmapText'>isEmbeddedBitmapText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a> is set</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_isFakeBoldText'>isFakeBoldText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#Fake_Bold'>Fake Bold</a> is set</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_isLCDRenderText'>isLCDRenderText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#LCD_Text'>LCD Text</a> is set</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_isLinearText'>isLinearText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if text is converted to <a href='SkPath_Reference#Path'>Path</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_isSrcOver'>isSrcOver</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> is <a href='SkBlendMode_Reference#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_isSubpixelText'>isSubpixelText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#Subpixel_Text'>Subpixel Text</a> is set</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_isVerticalText'>isVerticalText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#Vertical_Text'>Vertical Text</a> is set</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_measureText'>measureText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns advance width and bounds of text</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_nothingToDraw'>nothingToDraw</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#Paint'>Paint</a> prevents all drawing</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_refColorFilter'>refColorFilter</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>references <a href='undocumented#Color_Filter'>Color Filter</a>, how colors are altered</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_refDrawLooper'>refDrawLooper</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>references <a href='undocumented#Draw_Looper'>Draw Looper</a>, multiple layers</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_refImageFilter'>refImageFilter</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>references <a href='undocumented#Image_Filter'>Image Filter</a>, alter pixels; blur</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_refMaskFilter'>refMaskFilter</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>references <a href='undocumented#Mask_Filter'>Mask Filter</a>, alterations to <a href='undocumented#Mask_Alpha'>Mask Alpha</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_refPathEffect'>refPathEffect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>references <a href='undocumented#Path_Effect'>Path Effect</a>, modifications to path geometry; dashing</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_refShader'>refShader</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>references <a href='undocumented#Shader'>Shader</a>, multiple drawing colors; gradients</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_refTypeface'>refTypeface</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>references <a href='undocumented#Typeface'>Typeface</a>, font description</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_reset'>reset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to default values</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setARGB'>setARGB</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets color by component</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setAlpha'>setAlpha</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='SkColor_Reference#Alpha'>Color Alpha</a>, color opacity</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setAntiAlias'>setAntiAlias</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets or clears <a href='#Anti_Alias'>Anti Alias</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setAutohinted'>setAutohinted</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='undocumented#Glyph'>Glyphs</a> to always be hinted</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setBlendMode'>setBlendMode</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a>, how colors combine with destination</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setColor'>setColor</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='SkColor_Reference#Alpha'>Color Alpha</a> and RGB, one drawing color</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setColorFilter'>setColorFilter</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='undocumented#Color_Filter'>Color Filter</a>, alters color</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setDither'>setDither</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets or clears <a href='#Dither'>Dither</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setDrawLooper'>setDrawLooper</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='undocumented#Draw_Looper'>Draw Looper</a>, multiple layers</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setEmbeddedBitmapText'>setEmbeddedBitmapText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets or clears <a href='#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setFakeBoldText'>setFakeBoldText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets or clears <a href='#Fake_Bold'>Fake Bold</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setFilterQuality'>setFilterQuality</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='undocumented#Filter_Quality'>Filter Quality</a>, the image filtering level</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setFlags'>setFlags</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets multiple <a href='#SkPaint_Flags'>Flags</a> in a bit field</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setHinting'>setHinting</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='#SkPaint_Hinting'>Hinting</a>, glyph outline adjustment level</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setImageFilter'>setImageFilter</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='undocumented#Image_Filter'>Image Filter</a>, alter pixels; blur</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setLCDRenderText'>setLCDRenderText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets or clears <a href='#LCD_Text'>LCD Text</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setLinearText'>setLinearText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>converts to <a href='SkPath_Reference#Path'>Path</a> before draw or measure</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setMaskFilter'>setMaskFilter</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='undocumented#Mask_Filter'>Mask Filter</a>, alterations to <a href='undocumented#Mask_Alpha'>Mask Alpha</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setPathEffect'>setPathEffect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='undocumented#Path_Effect'>Path Effect</a>, modifications to path geometry; dashing</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setShader'>setShader</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='undocumented#Shader'>Shader</a>, multiple drawing colors; gradients</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setStrokeCap'>setStrokeCap</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='#SkPaint_Cap'>Cap</a>, the area drawn at path ends</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='#SkPaint_Join'>Join</a>, geometry on path corners</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setStrokeMiter'>setStrokeMiter</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='#Miter_Limit'>Miter Limit</a>, angles with sharp corners</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setStrokeWidth'>setStrokeWidth</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets thickness of the stroke</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setStyle'>setStyle</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='#SkPaint_Style'>Style</a>: stroke, fill, or both</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setSubpixelText'>setSubpixelText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets or clears <a href='#Subpixel_Text'>Subpixel Text</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setTextAlign'>setTextAlign</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='#SkPaint_Align'>Align</a>: left, center, or right</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setTextEncoding'>setTextEncoding</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets character or glyph encoded size</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setTextScaleX'>setTextScaleX</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets the text horizontal scale; condensed text</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setTextSize'>setTextSize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets text size in points</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setTextSkewX'>setTextSkewX</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets the text horizontal skew; oblique text</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setTypeface'>setTypeface</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='undocumented#Typeface'>Typeface</a>, font description</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_setVerticalText'>setVerticalText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets or clears <a href='#Vertical_Text'>Vertical Text</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_textToGlyphs'>textToGlyphs</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>converts text into glyph indices</td>
  </tr>
</table>

## <a name='Initializers'>Initializers</a>

<a name='SkPaint_empty_constructor'></a>
## SkPaint

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint'>SkPaint</a>()
</pre>

Constructs <a href='#Paint'>Paint</a> with default values.

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

The flags, text size, hinting, and miter limit may be overridden at compile time by defining
paint default values. The overrides may be included in "SkUserConfig.h" or predefined by the
build system.

### Return Value

default initialized <a href='#Paint'>Paint</a>

### Example

<div><fiddle-embed name="c4b2186d85c142a481298f7144295ffd"></fiddle-embed></div>

---

<a name='SkPaint_copy_const_SkPaint'></a>
## SkPaint

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint'>SkPaint</a>(const <a href='#SkPaint'>SkPaint</a>& paint)
</pre>

Makes a shallow copy of <a href='#Paint'>Paint</a>. <a href='undocumented#Typeface'>Typeface</a>, <a href='undocumented#Path_Effect'>Path Effect</a>, <a href='undocumented#Shader'>Shader</a>,
<a href='undocumented#Mask_Filter'>Mask Filter</a>, <a href='undocumented#Color_Filter'>Color Filter</a>, <a href='undocumented#Draw_Looper'>Draw Looper</a>, and <a href='undocumented#Image_Filter'>Image Filter</a> are shared
between the original <a href='#SkPaint_copy_const_SkPaint_paint'>paint</a> and the copy. Objects containing <a href='undocumented#Reference_Count'>Reference Count</a> increment
their references by one.

The referenced objects <a href='undocumented#Path_Effect'>Path Effect</a>, <a href='undocumented#Shader'>Shader</a>, <a href='undocumented#Mask_Filter'>Mask Filter</a>, <a href='undocumented#Color_Filter'>Color Filter</a>,
<a href='undocumented#Draw_Looper'>Draw Looper</a>, and <a href='undocumented#Image_Filter'>Image Filter</a> cannot be modified after they are created.
This prevents objects with <a href='undocumented#Reference_Count'>Reference Count</a> from being modified once <a href='#Paint'>Paint</a> refers to them.

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
<a href='#SkPaint'>SkPaint</a>(<a href='#SkPaint'>SkPaint</a>&& paint)
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
void <a href='#SkPaint_reset'>reset</a>()
</pre>

Sets all <a href='#Paint'>Paint</a> contents to their initial values. This is equivalent to replacing
<a href='#Paint'>Paint</a> with the result of <a href='#SkPaint_empty_constructor'>SkPaint()</a>.

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
<a href='#SkPaint_destructor'>~SkPaint</a>()
</pre>

Decreases <a href='#Paint'>Paint</a> <a href='undocumented#Reference_Count'>Reference Count</a> of owned objects: <a href='undocumented#Typeface'>Typeface</a>, <a href='undocumented#Path_Effect'>Path Effect</a>, <a href='undocumented#Shader'>Shader</a>,
<a href='undocumented#Mask_Filter'>Mask Filter</a>, <a href='undocumented#Color_Filter'>Color Filter</a>, <a href='undocumented#Draw_Looper'>Draw Looper</a>, and <a href='undocumented#Image_Filter'>Image Filter</a>. If the
objects containing <a href='undocumented#Reference_Count'>Reference Count</a> go to zero, they are deleted.

---

## <a name='Management'>Management</a>

<a name='SkPaint_copy_operator'></a>
## operator=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint'>SkPaint</a>& <a href='#SkPaint_copy_operator'>operator=(const SkPaint& paint)</a>
</pre>

Makes a shallow copy of <a href='#Paint'>Paint</a>. <a href='undocumented#Typeface'>Typeface</a>, <a href='undocumented#Path_Effect'>Path Effect</a>, <a href='undocumented#Shader'>Shader</a>,
<a href='undocumented#Mask_Filter'>Mask Filter</a>, <a href='undocumented#Color_Filter'>Color Filter</a>, <a href='undocumented#Draw_Looper'>Draw Looper</a>, and <a href='undocumented#Image_Filter'>Image Filter</a> are shared
between the original <a href='#SkPaint_copy_operator_paint'>paint</a> and the copy. Objects containing <a href='undocumented#Reference_Count'>Reference Count</a> in the
prior destination are decreased by one, and the referenced objects are deleted if the
resulting count is zero. Objects containing <a href='undocumented#Reference_Count'>Reference Count</a> in the parameter <a href='#SkPaint_copy_operator_paint'>paint</a>
are increased by one. <a href='#SkPaint_copy_operator_paint'>paint</a> is unmodified.

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
<a href='#SkPaint'>SkPaint</a>& <a href='#SkPaint_move_operator'>operator=(SkPaint&& paint)</a>
</pre>

Moves the <a href='#SkPaint_move_operator_paint'>paint</a> to avoid increasing the reference counts
of objects referenced by the <a href='#SkPaint_move_operator_paint'>paint</a> parameter. Objects containing <a href='undocumented#Reference_Count'>Reference Count</a> in the
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

Compares <a href='#SkPaint_equal_operator_a'>a</a> and <a href='#SkPaint_equal_operator_b'>b</a>, and returns true if <a href='#SkPaint_equal_operator_a'>a</a> and <a href='#SkPaint_equal_operator_b'>b</a> are equivalent. May return false
if <a href='undocumented#Typeface'>Typeface</a>, <a href='undocumented#Path_Effect'>Path Effect</a>, <a href='undocumented#Shader'>Shader</a>, <a href='undocumented#Mask_Filter'>Mask Filter</a>, <a href='undocumented#Color_Filter'>Color Filter</a>,
<a href='undocumented#Draw_Looper'>Draw Looper</a>, or <a href='undocumented#Image_Filter'>Image Filter</a> have identical contents but different pointers.

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

Compares <a href='#SkPaint_notequal_operator_a'>a</a> and <a href='#SkPaint_notequal_operator_b'>b</a>, and returns true if <a href='#SkPaint_notequal_operator_a'>a</a> and <a href='#SkPaint_notequal_operator_b'>b</a> are not equivalent. May return true
if <a href='undocumented#Typeface'>Typeface</a>, <a href='undocumented#Path_Effect'>Path Effect</a>, <a href='undocumented#Shader'>Shader</a>, <a href='undocumented#Mask_Filter'>Mask Filter</a>, <a href='undocumented#Color_Filter'>Color Filter</a>,
<a href='undocumented#Draw_Looper'>Draw Looper</a>, or <a href='undocumented#Image_Filter'>Image Filter</a> have identical contents but different pointers.

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
uint32_t <a href='#SkPaint_getHash'>getHash</a>() const
</pre>

Returns a hash generated from <a href='#Paint'>Paint</a> values and pointers.
Identical hashes guarantee that the paints are
equivalent, but differing hashes do not guarantee that the paints have differing
contents.

If <a href='#SkPaint_equal_operator'>operator==(const SkPaint& a, const SkPaint& b)</a> returns true for two paints,
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

## <a name='Hinting'>Hinting</a>

## <a name='SkPaint_Hinting'>Enum SkPaint::Hinting</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Hinting'>Hinting</a> {
        <a href='#SkPaint_kNo_Hinting'>kNo Hinting</a>            = 0,
        <a href='#SkPaint_kSlight_Hinting'>kSlight Hinting</a>        = 1,
        <a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a>        = 2,
        <a href='#SkPaint_kFull_Hinting'>kFull Hinting</a>          = 3,
    };
</pre>

<a href='#SkPaint_Hinting'>Hinting</a> adjusts the glyph outlines so that the shape provides a uniform
look at a given point size on font engines that support it. <a href='#SkPaint_Hinting'>Hinting</a> may have a
muted effect or no effect at all depending on the platform.

The four levels roughly control corresponding features on platforms that use FreeType
as the <a href='undocumented#Engine'>Font Engine</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kNo_Hinting'><code>SkPaint::kNo_Hinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Leaves glyph outlines unchanged from their native representation.
With FreeType, this is equivalent to the FT_LOAD_NO_HINTING
bit-field constant supplied to FT_Load_Glyph, which indicates that the vector
outline being loaded should not be fitted to the pixel grid but simply scaled
to 26.6 fractional pixels.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kSlight_Hinting'><code>SkPaint::kSlight_Hinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Modifies glyph outlines minimally to improve constrast.
With FreeType, this is equivalent in spirit to the
FT_LOAD_TARGET_LIGHT value supplied to FT_Load_Glyph. It chooses a
lighter hinting algorithm for non-monochrome modes.
Generated <a href='undocumented#Glyph'>Glyphs</a> may be fuzzy but better resemble their original shape.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kNormal_Hinting'><code>SkPaint::kNormal_Hinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Modifies glyph outlines to improve constrast. This is the default.
With FreeType, this supplies FT_LOAD_TARGET_NORMAL to FT_Load_Glyph,
choosing the default hinting algorithm, which is optimized for standard
gray-level rendering.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kFull_Hinting'><code>SkPaint::kFull_Hinting</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Modifies glyph outlines for maximum constrast. With FreeType, this selects
FT_LOAD_TARGET_LCD or FT_LOAD_TARGET_LCD_V if <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText Flag</a> is set.
FT_LOAD_TARGET_LCD is a variant of FT_LOAD_TARGET_NORMAL optimized for
horizontally decimated LCD displays; FT_LOAD_TARGET_LCD_V is a
variant of FT_LOAD_TARGET_NORMAL optimized for vertically decimated LCD displays.
</td>
  </tr>
</table>

On <a href='undocumented#OS_X'>OS X</a> and iOS, hinting controls whether <a href='undocumented#Core_Graphics'>Core Graphics</a> dilates the font outlines
to account for LCD text. No hinting uses <a href='undocumented#Core_Text'>Core Text</a> <a href='undocumented#Grayscale'>Grayscale</a> output.
Normal hinting uses <a href='undocumented#Core_Text'>Core Text</a> LCD output. If <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText Flag</a> is clear,
the LCD output is reduced to a single <a href='undocumented#Grayscale'>Grayscale</a> channel.

On Windows with DirectWrite, <a href='#SkPaint_Hinting'>Hinting</a> has no effect.

<a href='#SkPaint_Hinting'>Hinting</a> defaults to <a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a>.
Set <a href='undocumented#SkPaintDefaults_Hinting'>SkPaintDefaults Hinting</a> at compile time to change the default setting.

<a name='SkPaint_getHinting'></a>
## getHinting

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Hinting'>Hinting</a> <a href='#SkPaint_getHinting'>getHinting</a>() const
</pre>

Returns level of glyph outline adjustment.

### Return Value

one of: <a href='#SkPaint_kNo_Hinting'>kNo Hinting</a>, <a href='#SkPaint_kSlight_Hinting'>kSlight Hinting</a>, <a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a>, <a href='#SkPaint_kFull_Hinting'>kFull Hinting</a>

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
void <a href='#SkPaint_setHinting'>setHinting</a>(<a href='#SkPaint_Hinting'>Hinting</a> hintingLevel)
</pre>

Sets level of glyph outline adjustment.
Does not check for valid values of <a href='#SkPaint_setHinting_hintingLevel'>hintingLevel</a>.

| <a href='#SkPaint_Hinting'>Hinting</a> | value | effect on generated glyph outlines |
| --- | --- | ---  |
| <a href='#SkPaint_kNo_Hinting'>kNo Hinting</a> | 0 | leaves glyph outlines unchanged from their native representation |
| <a href='#SkPaint_kSlight_Hinting'>kSlight Hinting</a> | 1 | modifies glyph outlines minimally to improve contrast |
| <a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a> | 2 | modifies glyph outlines to improve contrast |
| <a href='#SkPaint_kFull_Hinting'>kFull Hinting</a> | 3 | modifies glyph outlines for maximum contrast |

### Parameters

<table>  <tr>    <td><a name='SkPaint_setHinting_hintingLevel'><code><strong>hintingLevel</strong></code></a></td>
    <td>one of: <a href='#SkPaint_kNo_Hinting'>kNo Hinting</a>, <a href='#SkPaint_kSlight_Hinting'>kSlight Hinting</a>, <a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a>, <a href='#SkPaint_kFull_Hinting'>kFull Hinting</a></td>
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
        <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias Flag</a>       = 0x01,
        <a href='#SkPaint_kDither_Flag'>kDither Flag</a>          = 0x04,
        <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText Flag</a>    = 0x20,
        <a href='#SkPaint_kLinearText_Flag'>kLinearText Flag</a>      = 0x40,
        <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText Flag</a>    = 0x80,
        <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText Flag</a>   = 0x200,
        <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText Flag</a> = 0x400,
        <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting Flag</a>     = 0x800,
        <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a>    = 0x1000,

        <a href='#SkPaint_kAllFlags'>kAllFlags</a> = 0xFFFF,
    };

</pre>

The bit values stored in <a href='#SkPaint_Flags'>Flags</a>.
The default value for <a href='#SkPaint_Flags'>Flags</a>, normally zero, can be changed at compile time
with a custom definition of <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults Flags</a>.
All flags can be read and written explicitly; <a href='#SkPaint_Flags'>Flags</a> allows manipulating
multiple settings at once.

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
mask of all <a href='#SkPaint_Flags'>Flags</a>, including private flags and flags reserved for future use</td>
  </tr>
<a href='#SkPaint_Flags'>Flags</a> default to all flags clear, disabling the associated feature.
</table>

## <a name='SkPaint_ReserveFlags'>Enum SkPaint::ReserveFlags</a>

Deprecated.

soonOnly valid for Android framework.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_ReserveFlags'>ReserveFlags</a> {
        <a href='#SkPaint_kUnderlineText_ReserveFlag'>kUnderlineText ReserveFlag</a>   = 0x08,
        <a href='#SkPaint_kStrikeThruText_ReserveFlag'>kStrikeThruText ReserveFlag</a>  = 0x10,
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
Deprecated.

soon</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kStrikeThruText_ReserveFlag'><code>SkPaint::kStrikeThruText_ReserveFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x0010</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Deprecated.

soon</td>
  </tr>
</table>

<a name='SkPaint_getFlags'></a>
## getFlags

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPaint_getFlags'>getFlags</a>() const
</pre>

Returns paint settings described by <a href='#SkPaint_Flags'>Flags</a>. Each setting uses one
bit, and can be tested with <a href='#SkPaint_Flags'>Flags</a> members.

### Return Value

zero, one, or more bits described by <a href='#SkPaint_Flags'>Flags</a>

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
void <a href='#SkPaint_setFlags'>setFlags</a>(uint32_t flags)
</pre>

Replaces <a href='#SkPaint_Flags'>Flags</a> with <a href='#SkPaint_setFlags_flags'>flags</a>, the union of the <a href='#SkPaint_Flags'>Flags</a> members.
All <a href='#SkPaint_Flags'>Flags</a> members may be cleared, or one or more may be set.

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

<a href='#Anti_Alias'>Anti Alias</a> drawing approximates partial pixel coverage with transparency.
If <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias Flag</a> is clear, pixel centers contained by the shape edge are drawn opaque.
If <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias Flag</a> is set, pixels are drawn with <a href='SkColor_Reference#Alpha'>Color Alpha</a> equal to their coverage.

The rule for <a href='undocumented#Alias'>Aliased</a> pixels is inconsistent across platforms. A shape edge
passing through the pixel center may, but is not required to, draw the pixel.

<a href='undocumented#Raster_Engine'>Raster Engine</a> draws <a href='undocumented#Alias'>Aliased</a> pixels whose centers are on or to the right of the start of an
active <a href='SkPath_Reference#Path'>Path</a> edge, and whose center is to the left of the end of the active <a href='SkPath_Reference#Path'>Path</a> edge.

A platform may only support <a href='#Anti_Alias'>Anti Aliased</a> drawing. Some GPU-backed platforms use
<a href='undocumented#Supersampling'>Supersampling</a> to <a href='#Anti_Alias'>Anti Alias</a> all drawing, and have no mechanism to selectively
<a href='undocumented#Alias'>Alias</a>.

The amount of coverage computed for <a href='#Anti_Alias'>Anti Aliased</a> pixels also varies across platforms.

<a href='#Anti_Alias'>Anti Alias</a> is disabled by default.
<a href='#Anti_Alias'>Anti Alias</a> can be enabled by default by setting <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults Flags</a> to <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias Flag</a>
at compile time.

### Example

<div><fiddle-embed name="a6575a49467ce8d28bb01cc7638fa04d"><div>A red line is drawn with transparency on the edges to make it look smoother.
A blue line draws only where the pixel centers are contained.
The lines are drawn into <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>, then drawn magnified to make the
<a href='undocumented#Alias'>Aliasing</a> easier to see.
</div></fiddle-embed></div>

<a name='SkPaint_isAntiAlias'></a>
## isAntiAlias

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isAntiAlias'>isAntiAlias</a>() const
</pre>

Returns true if pixels on the active edges of <a href='SkPath_Reference#Path'>Path</a> may be drawn with partial transparency.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a> masked with <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias Flag</a>.

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
void <a href='#SkPaint_setAntiAlias'>setAntiAlias</a>(bool aa)
</pre>

Requests, but does not require, that <a href='SkPath_Reference#Path'>Path</a> edge pixels draw opaque or with
partial transparency.

Sets <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias Flag</a> if <a href='#SkPaint_setAntiAlias_aa'>aa</a> is true.
Clears <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias Flag</a> if <a href='#SkPaint_setAntiAlias_aa'>aa</a> is false.

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

<a href='#Dither'>Dither</a> increases fidelity by adjusting the color of adjacent pixels.
This can help to smooth color transitions and reducing banding in gradients.
Dithering lessens visible banding from <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>
and <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> gradients,
and improves rendering into a <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

Dithering is always enabled for linear gradients drawing into
<a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a> and <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.
<a href='#Dither'>Dither</a> cannot be enabled for <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a> and
<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

<a href='#Dither'>Dither</a> is disabled by default.
<a href='#Dither'>Dither</a> can be enabled by default by setting <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults Flags</a> to <a href='#SkPaint_kDither_Flag'>kDither Flag</a>
at compile time.

Some platform implementations may ignore dithering. SetSK_IGNORE_GPU_DITHERto ignore <a href='#Dither'>Dither</a> on <a href='undocumented#GPU_Surface'>GPU Surface</a>.

### Example

<div><fiddle-embed name="8b26507690b71462f44642b911890bbf"><div>Dithering in the bottom half more closely approximates the requested color by
alternating nearby colors from pixel to pixel.
</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="76d4d4a7931a48495e4d5f54e073be53"><div>Dithering introduces subtle adjustments to color to smooth gradients.
Drawing the gradient repeatedly with <a href='SkBlendMode_Reference#SkBlendMode_kPlus'>SkBlendMode::kPlus</a> exaggerates the
dither, making it easier to see.
</div></fiddle-embed></div>

<a name='SkPaint_isDither'></a>
## isDither

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isDither'>isDither</a>() const
</pre>

Returns true if color error may be distributed to smooth color transition.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a> masked with <a href='#SkPaint_kDither_Flag'>kDither Flag</a>.

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
void <a href='#SkPaint_setDither'>setDither</a>(bool dither)
</pre>

Requests, but does not require, to distribute color error.

Sets <a href='#SkPaint_kDither_Flag'>kDither Flag</a> if <a href='#SkPaint_setDither_dither'>dither</a> is true.
Clears <a href='#SkPaint_kDither_Flag'>kDither Flag</a> if <a href='#SkPaint_setDither_dither'>dither</a> is false.

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

<a href='#LCD_Text'>LCD Text</a> and <a href='#Subpixel_Text'>Subpixel Text</a> increase the precision of glyph position.

When set, <a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText Flag</a> takes advantage of the organization of RGB stripes that
create a color, and relies
on the small size of the stripe and visual perception to make the color fringing imperceptible.
<a href='#LCD_Text'>LCD Text</a> can be enabled on devices that orient stripes horizontally or vertically, and that order
the color components as RGB or BGR.

<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText Flag</a> uses the pixel transparency to represent a fractional offset.
As the opaqueness
of the color increases, the edge of the glyph appears to move towards the outside of the pixel.

Either or both techniques can be enabled.
<a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText Flag</a> and <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText Flag</a> are clear by default.
<a href='#LCD_Text'>LCD Text</a> or <a href='#Subpixel_Text'>Subpixel Text</a> can be enabled by default by setting <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults Flags</a> to
<a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText Flag</a> or <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText Flag</a> (or both) at compile time.

### Example

<div><fiddle-embed name="4606ae1be792d6bc46d496432f050ee9"><div>Four commas are drawn normally and with combinations of <a href='#LCD_Text'>LCD Text</a> and <a href='#Subpixel_Text'>Subpixel Text</a>.
When <a href='#Subpixel_Text'>Subpixel Text</a> is disabled, the comma <a href='undocumented#Glyph'>Glyphs</a> are identical, but not evenly spaced.
When <a href='#Subpixel_Text'>Subpixel Text</a> is enabled, the comma <a href='undocumented#Glyph'>Glyphs</a> are unique, but appear evenly spaced.
</div></fiddle-embed></div>

## <a name='Linear_Text'>Linear Text</a>

<a href='#Linear_Text'>Linear Text</a> selects whether text is rendered as a <a href='undocumented#Glyph'>Glyph</a> or as a <a href='SkPath_Reference#Path'>Path</a>.
If <a href='#SkPaint_kLinearText_Flag'>kLinearText Flag</a> is set, it has the same effect as setting <a href='#SkPaint_Hinting'>Hinting</a> to <a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a>.
If <a href='#SkPaint_kLinearText_Flag'>kLinearText Flag</a> is clear, it is the same as setting <a href='#SkPaint_Hinting'>Hinting</a> to <a href='#SkPaint_kNo_Hinting'>kNo Hinting</a>.

<a name='SkPaint_isLinearText'></a>
## isLinearText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isLinearText'>isLinearText</a>() const
</pre>

Returns true if text is converted to <a href='SkPath_Reference#Path'>Path</a> before drawing and measuring.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a> masked with <a href='#SkPaint_kLinearText_Flag'>kLinearText Flag</a>.

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
void <a href='#SkPaint_setLinearText'>setLinearText</a>(bool linearText)
</pre>

Returns true if text is converted to <a href='SkPath_Reference#Path'>Path</a> before drawing and measuring.
By default, <a href='#SkPaint_kLinearText_Flag'>kLinearText Flag</a> is clear.

Sets <a href='#SkPaint_kLinearText_Flag'>kLinearText Flag</a> if <a href='#SkPaint_setLinearText_linearText'>linearText</a> is true.
Clears <a href='#SkPaint_kLinearText_Flag'>kLinearText Flag</a> if <a href='#SkPaint_setLinearText_linearText'>linearText</a> is false.

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

<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText Flag</a> uses the pixel transparency to represent a fractional offset.
As the opaqueness
of the color increases, the edge of the glyph appears to move towards the outside of the pixel.

<a name='SkPaint_isSubpixelText'></a>
## isSubpixelText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isSubpixelText'>isSubpixelText</a>() const
</pre>

Returns true if <a href='undocumented#Glyph'>Glyphs</a> at different sub-pixel positions may differ on pixel edge coverage.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a> masked with <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText Flag</a>.

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
void <a href='#SkPaint_setSubpixelText'>setSubpixelText</a>(bool subpixelText)
</pre>

Requests, but does not require, that <a href='undocumented#Glyph'>Glyphs</a> respect sub-pixel positioning.

Sets <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText Flag</a> if <a href='#SkPaint_setSubpixelText_subpixelText'>subpixelText</a> is true.
Clears <a href='#SkPaint_kSubpixelText_Flag'>kSubpixelText Flag</a> if <a href='#SkPaint_setSubpixelText_subpixelText'>subpixelText</a> is false.

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

When set, <a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText Flag</a> takes advantage of the organization of RGB stripes that
create a color, and relies
on the small size of the stripe and visual perception to make the color fringing imperceptible.
<a href='#LCD_Text'>LCD Text</a> can be enabled on devices that orient stripes horizontally or vertically, and that order
the color components as RGB or BGR.

<a name='SkPaint_isLCDRenderText'></a>
## isLCDRenderText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isLCDRenderText'>isLCDRenderText</a>() const
</pre>

Returns true if <a href='undocumented#Glyph'>Glyphs</a> may use LCD striping to improve glyph edges.

Returns true if <a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText Flag</a> is set.

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
void <a href='#SkPaint_setLCDRenderText'>setLCDRenderText</a>(bool lcdText)
</pre>

Requests, but does not require, that <a href='undocumented#Glyph'>Glyphs</a> use LCD striping for glyph edges.

Sets <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText Flag</a> if <a href='#SkPaint_setLCDRenderText_lcdText'>lcdText</a> is true.
Clears <a href='#SkPaint_kLCDRenderText_Flag'>kLCDRenderText Flag</a> if <a href='#SkPaint_setLCDRenderText_lcdText'>lcdText</a> is false.

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

<a href='#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a> allows selecting custom sized bitmap <a href='undocumented#Glyph'>Glyphs</a>.
<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText Flag</a> when set chooses an embedded bitmap glyph over an outline contained
in a font if the platform supports this option.

FreeType selects the bitmap glyph if available when <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText Flag</a> is set, and selects
the outline glyph if <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText Flag</a> is clear.
Windows may select the bitmap glyph but is not required to do so.
<a href='undocumented#OS_X'>OS X</a> and iOS do not support this option.

<a href='#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a> is disabled by default.
<a href='#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a> can be enabled by default by setting <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults Flags</a> to
<a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText Flag</a> at compile time.

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
bool <a href='#SkPaint_isEmbeddedBitmapText'>isEmbeddedBitmapText</a>() const
</pre>

Returns true if <a href='undocumented#Engine'>Font Engine</a> may return <a href='undocumented#Glyph'>Glyphs</a> from font bitmaps instead of from outlines.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a> masked with <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText Flag</a>.

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
void <a href='#SkPaint_setEmbeddedBitmapText'>setEmbeddedBitmapText</a>(bool useEmbeddedBitmapText)
</pre>

Requests, but does not require, to use bitmaps in fonts instead of outlines.

Sets <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText Flag</a> if <a href='#SkPaint_setEmbeddedBitmapText_useEmbeddedBitmapText'>useEmbeddedBitmapText</a> is true.
Clears <a href='#SkPaint_kEmbeddedBitmapText_Flag'>kEmbeddedBitmapText Flag</a> if <a href='#SkPaint_setEmbeddedBitmapText_useEmbeddedBitmapText'>useEmbeddedBitmapText</a> is false.

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

If <a href='#SkPaint_Hinting'>Hinting</a> is set to <a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a> or <a href='#SkPaint_kFull_Hinting'>kFull Hinting</a>, <a href='#Automatic_Hinting'>Automatic Hinting</a>
instructs the <a href='undocumented#Font_Manager'>Font Manager</a> to always hint <a href='undocumented#Glyph'>Glyphs</a>.
<a href='#Automatic_Hinting'>Automatic Hinting</a> has no effect if <a href='#SkPaint_Hinting'>Hinting</a> is set to <a href='#SkPaint_kNo_Hinting'>kNo Hinting</a> or
<a href='#SkPaint_kSlight_Hinting'>kSlight Hinting</a>.

<a href='#Automatic_Hinting'>Automatic Hinting</a> only affects platforms that use FreeType as the <a href='undocumented#Font_Manager'>Font Manager</a>.

<a name='SkPaint_isAutohinted'></a>
## isAutohinted

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isAutohinted'>isAutohinted</a>() const
</pre>

Returns true if <a href='#SkPaint_Hinting'>Hinting</a> is set to <a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a> or <a href='#SkPaint_kFull_Hinting'>kFull Hinting</a>, and if
platform uses FreeType as the <a href='undocumented#Font_Manager'>Font Manager</a>. If true, instructs
the <a href='undocumented#Font_Manager'>Font Manager</a> to always hint <a href='undocumented#Glyph'>Glyphs</a>.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a> masked with <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting Flag</a>.

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
void <a href='#SkPaint_setAutohinted'>setAutohinted</a>(bool useAutohinter)
</pre>

Sets whether to always hint <a href='undocumented#Glyph'>Glyphs</a>.
If <a href='#SkPaint_Hinting'>Hinting</a> is set to <a href='#SkPaint_kNormal_Hinting'>kNormal Hinting</a> or <a href='#SkPaint_kFull_Hinting'>kFull Hinting</a> and <a href='#SkPaint_setAutohinted_useAutohinter'>useAutohinter</a> is set,
instructs the <a href='undocumented#Font_Manager'>Font Manager</a> to always hint <a href='undocumented#Glyph'>Glyphs</a>.
<a href='#Automatic_Hinting'>Automatic Hinting</a> has no effect if <a href='#SkPaint_Hinting'>Hinting</a> is set to <a href='#SkPaint_kNo_Hinting'>kNo Hinting</a> or
<a href='#SkPaint_kSlight_Hinting'>kSlight Hinting</a>.

Only affects platforms that use FreeType as the <a href='undocumented#Font_Manager'>Font Manager</a>.

Sets <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting Flag</a> if <a href='#SkPaint_setAutohinted_useAutohinter'>useAutohinter</a> is true.
Clears <a href='#SkPaint_kAutoHinting_Flag'>kAutoHinting Flag</a> if <a href='#SkPaint_setAutohinted_useAutohinter'>useAutohinter</a> is false.

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

<a href='undocumented#Text'>Text</a> may be drawn by positioning each glyph, or by positioning the first glyph and
using <a href='undocumented#Advance'>Font Advance</a> to position subsequent <a href='undocumented#Glyph'>Glyphs</a>. By default, each successive glyph
is positioned to the right of the preceding glyph. <a href='#Vertical_Text'>Vertical Text</a> sets successive
<a href='undocumented#Glyph'>Glyphs</a> to position below the preceding glyph.

Skia can translate text character codes as a series of <a href='undocumented#Glyph'>Glyphs</a>, but does not implement
font substitution,
textual substitution, line layout, or contextual spacing like <a href='undocumented#Kerning'>Kerning</a> pairs. Use
a text shaping engine like <a href='https://harfbuzz.org/'>HarfBuzz</a></a> to translate text runs
into glyph series.

<a href='#Vertical_Text'>Vertical Text</a> is clear if text is drawn left to right or set if drawn from top to bottom.

<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> if clear draws text left to right.
<a href='#SkPaint_Flags'>Flags</a> <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> if set draws text top to bottom.

<a href='#Vertical_Text'>Vertical Text</a> is clear by default.
<a href='#Vertical_Text'>Vertical Text</a> can be set by default by setting <a href='undocumented#SkPaintDefaults_Flags'>SkPaintDefaults Flags</a> to
<a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> at compile time.

### Example

<div><fiddle-embed name="8df5800819311b71373d9abb669b49b8"></fiddle-embed></div>

<a name='SkPaint_isVerticalText'></a>
## isVerticalText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isVerticalText'>isVerticalText</a>() const
</pre>

Returns true if <a href='undocumented#Glyph'>Glyphs</a> are drawn top to bottom instead of left to right.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a> masked with <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a>.

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
void <a href='#SkPaint_setVerticalText'>setVerticalText</a>(bool verticalText)
</pre>

Returns true if text advance positions the next glyph below the previous glyph instead of to the
right of previous glyph.

Sets <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> if vertical is true.
Clears <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> if vertical is false.

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
is not available. Skia does not provide font substitution; it is up to the client to find the
bold font face using the platform <a href='undocumented#Font_Manager'>Font Manager</a>.

Use <a href='#Text_Skew_X'>Text Skew X</a> to approximate an italic font style when the italic font face
is not available.

A FreeType based port may define SK_USE_FREETYPE_EMBOLDEN at compile time to direct
the font engine to create the bold <a href='undocumented#Glyph'>Glyphs</a>. Otherwise, the extra bold is computed
by increasing the stroke width and setting the <a href='#SkPaint_Style'>Style</a> to <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill Style</a> as needed.

<a href='#Fake_Bold'>Fake Bold</a> is disabled by default.

### Example

<div><fiddle-embed name="e811f4829a2daaaeaad3795504a7e02a"></fiddle-embed></div>

<a name='SkPaint_isFakeBoldText'></a>
## isFakeBoldText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isFakeBoldText'>isFakeBoldText</a>() const
</pre>

Returns true if approximate bold by increasing the stroke width when creating glyph bitmaps
from outlines.

Equivalent to <a href='#SkPaint_getFlags'>getFlags</a> masked with <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText Flag</a>.

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
void <a href='#SkPaint_setFakeBoldText'>setFakeBoldText</a>(bool fakeBoldText)
</pre>

Increases stroke width when creating glyph bitmaps to approximate a bold typeface.

Sets <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText Flag</a> if <a href='#SkPaint_setFakeBoldText_fakeBoldText'>fakeBoldText</a> is true.
Clears <a href='#SkPaint_kFakeBoldText_Flag'>kFakeBoldText Flag</a> if <a href='#SkPaint_setFakeBoldText_fakeBoldText'>fakeBoldText</a> is false.

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

if <a href='#SkPaint_Hinting'>Hinting</a> is set to <a href='#SkPaint_kFull_Hinting'>kFull Hinting</a>, <a href='#Full_Hinting_Spacing'>Full Hinting Spacing</a> adjusts the character
spacing by the difference of the hinted and <a href='undocumented#Unhinted'>Unhinted</a> <a href='undocumented#Left_Side_Bearing'>Left Side Bearing</a> and
<a href='undocumented#Right_Side_Bearing'>Right Side Bearing</a>. <a href='#Full_Hinting_Spacing'>Full Hinting Spacing</a> only applies to platforms that use
FreeType as their <a href='undocumented#Engine'>Font Engine</a>.

<a href='#Full_Hinting_Spacing'>Full Hinting Spacing</a> is not related to text <a href='undocumented#Kerning'>Kerning</a>, where the space between
a specific pair of characters is adjusted using data in the font <a href='undocumented#Kerning'>Kerning</a> tables.

<a name='SkPaint_isDevKernText'></a>
## isDevKernText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_isDevKernText'>isDevKernText</a>() const
</pre>

Deprecated.

---

<a name='SkPaint_setDevKernText'></a>
## setDevKernText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setDevKernText'>setDevKernText</a>(bool)
</pre>

Deprecated.

---

## <a name='Filter_Quality_Methods'>Filter Quality Methods</a>

<a href='undocumented#Filter_Quality'>Filter Quality</a> trades speed for image filtering when the image is scaled.
A lower <a href='undocumented#Filter_Quality'>Filter Quality</a> draws faster, but has less fidelity.
A higher <a href='undocumented#Filter_Quality'>Filter Quality</a> draws slower, but looks better.
If the image is drawn without scaling, the <a href='undocumented#Filter_Quality'>Filter Quality</a> choice will not result
in a noticeable difference.

<a href='undocumented#Filter_Quality'>Filter Quality</a> is used in <a href='#Paint'>Paint</a> passed as a parameter to

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

and when <a href='#Paint'>Paint</a> has a <a href='undocumented#Shader'>Shader</a> specialization that uses <a href='SkImage_Reference#Image'>Image</a> or <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>.

<a href='undocumented#Filter_Quality'>Filter Quality</a> is <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a> by default.

### Example

<div><fiddle-embed name="69369cff2f5b145a6f616092513266a0"></fiddle-embed></div>

<a name='SkPaint_getFilterQuality'></a>
## getFilterQuality

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='#SkPaint_getFilterQuality'>getFilterQuality</a>() const
</pre>

Returns <a href='undocumented#Filter_Quality'>Filter Quality</a>, the image filtering level. A lower setting
draws faster; a higher setting looks better when the image is scaled.

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

---

<a name='SkPaint_setFilterQuality'></a>
## setFilterQuality

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setFilterQuality'>setFilterQuality</a>(<a href='undocumented#SkFilterQuality'>SkFilterQuality</a> quality)
</pre>

Sets <a href='undocumented#Filter_Quality'>Filter Quality</a>, the image filtering level. A lower setting
draws faster; a higher setting looks better when the image is scaled.
Does not check to see if <a href='#SkPaint_setFilterQuality_quality'>quality</a> is valid.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setFilterQuality_quality'><code><strong>quality</strong></code></a></td>
    <td>one of: <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a>,
<a href='undocumented#kMedium_SkFilterQuality'>kMedium_SkFilterQuality</a>, <a href='undocumented#kHigh_SkFilterQuality'>kHigh_SkFilterQuality</a></td>
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
| <a href='#SkPaint_getColor'>getColor</a> | returns <a href='SkColor_Reference#Alpha'>Color Alpha</a> and RGB, one drawing color |
| <a href='#SkPaint_setColor'>setColor</a> | sets <a href='SkColor_Reference#Alpha'>Color Alpha</a> and RGB, one drawing color |

<a href='SkColor_Reference#Color'>Color</a> specifies the red, blue, green, and <a href='SkColor_Reference#Alpha'>Color Alpha</a>
values used to draw a filled or stroked shape in a 32-bit value. Each component
occupies 8-bits, ranging from zero: no contribution; to 255: full intensity.
All values in any combination are valid.

<a href='SkColor_Reference#Color'>Color</a> is not <a href='undocumented#Premultiply'>Premultiplied</a>; <a href='SkColor_Reference#Alpha'>Color Alpha</a> sets the transparency independent of
RGB: red, blue, and green.

The bit positions of <a href='SkColor_Reference#Alpha'>Color Alpha</a> and RGB are independent of the bit
positions on the output device, which may have more or fewer bits, and may have
a different arrangement.

| bit positions | <a href='SkColor_Reference#Alpha'>Color Alpha</a> | red | blue | green |
| --- | --- | --- | --- | ---  |
|  | 31 - 24 | 23 - 16 | 15 - 8 | 7 - 0 |

### Example

<div><fiddle-embed name="214b559d75c65a7bef6ef4be1f860053"></fiddle-embed></div>

<a name='SkPaint_getColor'></a>
## getColor

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPaint_getColor'>getColor</a>() const
</pre>

Retrieves <a href='SkColor_Reference#Alpha'>Alpha</a> and RGB, <a href='undocumented#Unpremultiply'>Unpremultiplied</a>, packed into 32 bits.
Use helpers <a href='SkColor_Reference#SkColorGetA'>SkColorGetA</a>, <a href='SkColor_Reference#SkColorGetR'>SkColorGetR</a>, <a href='SkColor_Reference#SkColorGetG'>SkColorGetG</a>, and <a href='SkColor_Reference#SkColorGetB'>SkColorGetB</a> to extract
a color component.

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

<a href='SkColor_Reference#SkColor'>SkColor</a>

---

<a name='SkPaint_setColor'></a>
## setColor

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setColor'>setColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> color)
</pre>

Sets <a href='SkColor_Reference#Alpha'>Alpha</a> and RGB used when stroking and filling. The <a href='#SkPaint_setColor_color'>color</a> is a 32-bit value,
<a href='undocumented#Unpremultiply'>Unpremultiplied</a>, packing 8-bit components for <a href='SkColor_Reference#Alpha'>Alpha</a>, red, blue, and green.

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

<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPaint_setARGB'>setARGB</a> <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>

---

## <a name='Alpha_Methods'>Alpha Methods</a>

<a href='SkColor_Reference#Alpha'>Color Alpha</a> sets the transparency independent of RGB: red, blue, and green.

<a name='SkPaint_getAlpha'></a>
## getAlpha

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint8_t <a href='#SkPaint_getAlpha'>getAlpha</a>() const
</pre>

Retrieves <a href='SkColor_Reference#Alpha'>Alpha</a> from the <a href='SkColor_Reference#Color'>Color</a> used when stroking and filling.

### Return Value

<a href='SkColor_Reference#Alpha'>Alpha</a> ranging from zero, fully transparent, to 255, fully opaque

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
void <a href='#SkPaint_setAlpha'>setAlpha</a>(<a href='undocumented#U8CPU'>U8CPU</a> a)
</pre>

Replaces <a href='SkColor_Reference#Alpha'>Alpha</a>, leaving RGB
unchanged. An out of range value triggers an assert in the debug
build. <a href='#SkPaint_setAlpha_a'>a</a> is <a href='#SkPaint_setAlpha_a'>a</a> value from zero to 255.
<a href='#SkPaint_setAlpha_a'>a</a> set to zero makes <a href='SkColor_Reference#Color'>Color</a> fully transparent; <a href='#SkPaint_setAlpha_a'>a</a> set to 255 makes <a href='SkColor_Reference#Color'>Color</a>
fully opaque.

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
void <a href='#SkPaint_setARGB'>setARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> a, <a href='undocumented#U8CPU'>U8CPU</a> r, <a href='undocumented#U8CPU'>U8CPU</a> g, <a href='undocumented#U8CPU'>U8CPU</a> b)
</pre>

Sets <a href='SkColor_Reference#Color'>Color</a> used when drawing solid fills. The color components range from 0 to 255.
The color is <a href='undocumented#Unpremultiply'>Unpremultiplied</a>; <a href='SkColor_Reference#Alpha'>Alpha</a> sets the transparency independent of RGB.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setARGB_a'><code><strong>a</strong></code></a></td>
    <td>amount of <a href='SkColor_Reference#Alpha'>Color Alpha</a>, from fully transparent (0) to fully opaque (255)</td>
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

---

## <a name='Style'>Style</a>

<a href='#SkPaint_Style'>Style</a> specifies if the geometry is filled, stroked, or both filled and stroked.
Some shapes ignore <a href='#SkPaint_Style'>Style</a> and are always drawn filled or stroked.

Set <a href='#SkPaint_Style'>Style</a> to <a href='#SkPaint_kFill_Style'>kFill Style</a> to fill the shape.
The fill covers the area inside the geometry for most shapes.

Set <a href='#SkPaint_Style'>Style</a> to <a href='#SkPaint_kStroke_Style'>kStroke Style</a> to stroke the shape.

## <a name='Style_Fill'>Style Fill</a>

### See Also

<a href='SkPath_Reference#Fill_Type'>Path Fill Type</a>

## <a name='Style_Stroke'>Style Stroke</a>

The stroke covers the area described by following the shape edge with a pen or brush of
<a href='#Stroke_Width'>Stroke Width</a>. The area covered where the shape starts and stops is described by <a href='#Stroke_Cap'>Stroke Cap</a>.
The area covered where the shape turns a corner is described by <a href='#Stroke_Join'>Stroke Join</a>.
The stroke is centered on the shape; it extends equally on either side of the shape edge.

As <a href='#Stroke_Width'>Stroke Width</a> gets smaller, the drawn path frame is thinner. <a href='#Stroke_Width'>Stroke Width</a> less than one
may have gaps, and if <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias Flag</a> is set, <a href='SkColor_Reference#Alpha'>Color Alpha</a> will increase to visually decrease coverage.

## <a name='Style_Hairline'>Style Hairline</a>

<a href='#Stroke_Width'>Stroke Width</a> of zero has a special meaning and switches drawing to use <a href='#Style_Hairline'>Hairline</a>.
<a href='#Style_Hairline'>Hairline</a> draws the thinnest continuous frame. If <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias Flag</a> is clear, adjacent pixels
flow horizontally, vertically,or diagonally.

<a href='SkPath_Reference#Path'>Path</a> drawing with <a href='#Style_Hairline'>Hairline</a> may hit the same pixel more than once. For instance, <a href='SkPath_Reference#Path'>Path</a> containing
two lines in one <a href='SkPath_Reference#Contour'>Path Contour</a> will draw the corner point once, but may both lines may draw the adjacent
pixel. If <a href='#SkPaint_kAntiAlias_Flag'>kAntiAlias Flag</a> is set, transparency is applied twice, resulting in a darker pixel. Some
GPU-backed implementations apply transparency at a later drawing stage, avoiding double hit pixels
while stroking.

## <a name='SkPaint_Style'>Enum SkPaint::Style</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Style'>Style</a> {
        <a href='#SkPaint_kFill_Style'>kFill Style</a>,
        <a href='#SkPaint_kStroke_Style'>kStroke Style</a>,
        <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill Style</a>,
    };

    static constexpr int <a href='#SkPaint_kStyleCount'>kStyleCount</a> = <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill Style</a> + 1;
</pre>

Set <a href='#SkPaint_Style'>Style</a> to fill, stroke, or both fill and stroke geometry.
The stroke and fill
share all paint attributes; for instance, they are drawn with the same color.

Use <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill Style</a> to avoid hitting the same pixels twice with a stroke draw and
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
Applies to <a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Region'>Region</a>, <a href='SkRRect_Reference#RRect'>Round Rect</a>, <a href='undocumented#Circle'>Circles</a>, <a href='undocumented#Oval'>Ovals</a>, <a href='SkPath_Reference#Path'>Path</a>, and <a href='undocumented#Text'>Text</a>.
<a href='SkBitmap_Reference#Bitmap'>Bitmap</a>, <a href='SkImage_Reference#Image'>Image</a>, <a href='undocumented#Patch'>Patches</a>, <a href='undocumented#Region'>Region</a>, <a href='undocumented#Sprite'>Sprites</a>, and <a href='undocumented#Vertices'>Vertices</a> are painted as if
<a href='#SkPaint_kFill_Style'>kFill Style</a> is set, and ignore the set <a href='#SkPaint_Style'>Style</a>.
The <a href='SkPath_Reference#Fill_Type'>Path Fill Type</a> specifies additional rules to fill the area outside the path edge,
and to create an unfilled hole inside the shape.
<a href='#SkPaint_Style'>Style</a> is set to <a href='#SkPaint_kFill_Style'>kFill Style</a> by default.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kStroke_Style'><code>SkPaint::kStroke_Style</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Applies to <a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Region'>Region</a>, <a href='SkRRect_Reference#RRect'>Round Rect</a>, <a href='undocumented#Arc'>Arcs</a>, <a href='undocumented#Circle'>Circles</a>, <a href='undocumented#Oval'>Ovals</a>, <a href='SkPath_Reference#Path'>Path</a>, and <a href='undocumented#Text'>Text</a>.
<a href='undocumented#Arc'>Arcs</a>, <a href='undocumented#Line'>Lines</a>, and points, are always drawn as if <a href='#SkPaint_kStroke_Style'>kStroke Style</a> is set,
and ignore the set <a href='#SkPaint_Style'>Style</a>.
The stroke construction is unaffected by the <a href='SkPath_Reference#Fill_Type'>Path Fill Type</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kStrokeAndFill_Style'><code>SkPaint::kStrokeAndFill_Style</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Applies to <a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Region'>Region</a>, <a href='SkRRect_Reference#RRect'>Round Rect</a>, <a href='undocumented#Circle'>Circles</a>, <a href='undocumented#Oval'>Ovals</a>, <a href='SkPath_Reference#Path'>Path</a>, and <a href='undocumented#Text'>Text</a>.
<a href='SkPath_Reference#Path'>Path</a> is treated as if it is set to <a href='SkPath_Reference#SkPath_kWinding_FillType'>SkPath::kWinding FillType</a>,
and the set <a href='SkPath_Reference#Fill_Type'>Path Fill Type</a> is ignored.
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
## getStyle

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_getStyle'>getStyle</a>() const
</pre>

Returns whether the geometry is filled, stroked, or filled and stroked.

### Return Value

one of:<a href='#SkPaint_kFill_Style'>kFill Style</a>, <a href='#SkPaint_kStroke_Style'>kStroke Style</a>, <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill Style</a>

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
void <a href='#SkPaint_setStyle'>setStyle</a>(<a href='#SkPaint_Style'>Style</a> style)
</pre>

Sets whether the geometry is filled, stroked, or filled and stroked.
Has no effect if <a href='#SkPaint_setStyle_style'>style</a> is not a legal <a href='#SkPaint_Style'>Style</a> value.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStyle_style'><code><strong>style</strong></code></a></td>
    <td>one of: <a href='#SkPaint_kFill_Style'>kFill Style</a>, <a href='#SkPaint_kStroke_Style'>kStroke Style</a>, <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill Style</a></td>
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

<a href='#Stroke_Width'>Stroke Width</a> sets the width for stroking. The width is the thickness
of the stroke perpendicular to the path direction when the paint style is
set to <a href='#SkPaint_kStroke_Style'>kStroke Style</a> or <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill Style</a>.

When width is greater than zero, the stroke encompasses as many pixels partially
or fully as needed. When the width equals zero, the paint enables hairlines;
the stroke is always one pixel wide.

The stroke dimensions are scaled by the canvas matrix, but <a href='#Style_Hairline'>Hairline</a> stroke
remains one pixel wide regardless of scaling.

The default width for the paint is zero.

### Example

<div><fiddle-embed name="5112c7209a19e035c61cef33a624a652" gpu="true"><div>The pixels hit to represent thin lines vary with the angle of the
line and the platform implementation.
</div></fiddle-embed></div>

<a name='SkPaint_getStrokeWidth'></a>
## getStrokeWidth

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getStrokeWidth'>getStrokeWidth</a>() const
</pre>

Returns the thickness of the pen used by <a href='#Paint'>Paint</a> to
outline the shape.

### Return Value

zero for <a href='#Style_Hairline'>Hairline</a>, greater than zero for pen thickness

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
void <a href='#SkPaint_setStrokeWidth'>setStrokeWidth</a>(<a href='undocumented#SkScalar'>SkScalar</a> width)
</pre>

Sets the thickness of the pen used by the paint to
outline the shape.
Has no effect if <a href='#SkPaint_setStrokeWidth_width'>width</a> is less than zero.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeWidth_width'><code><strong>width</strong></code></a></td>
    <td>zero thickness for <a href='#Style_Hairline'>Hairline</a>; greater than zero for pen thickness</td>
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

<a href='#Miter_Limit'>Miter Limit</a> specifies the maximum miter length,
relative to the stroke width.

<a href='#Miter_Limit'>Miter Limit</a> is used when the <a href='#Stroke_Join'>Stroke Join</a>
is set to <a href='#SkPaint_kMiter_Join'>kMiter Join</a>, and the <a href='#SkPaint_Style'>Style</a> is either <a href='#SkPaint_kStroke_Style'>kStroke Style</a>
or <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill Style</a>.

If the miter at a corner exceeds this limit, <a href='#SkPaint_kMiter_Join'>kMiter Join</a>
is replaced with <a href='#SkPaint_kBevel_Join'>kBevel Join</a>.

<a href='#Miter_Limit'>Miter Limit</a> can be computed from the corner angle:

miter limit = 1 / sin ( angle / 2 )<a href='#Miter_Limit'>Miter Limit</a> default value is 4.
The default may be changed at compile time by setting <a href='undocumented#SkPaintDefaults_MiterLimit'>SkPaintDefaults MiterLimit</a>
in "SkUserConfig.h" or as a define supplied by the build environment.

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

<div><fiddle-embed name="5de2de0f00354e59074a9bb1a42d5a63"><div>This example draws a stroked corner and the miter length beneath.
When the miter limit is decreased slightly, the miter join is replaced
by a bevel join.
</div></fiddle-embed></div>

<a name='SkPaint_getStrokeMiter'></a>
## getStrokeMiter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getStrokeMiter'>getStrokeMiter</a>() const
</pre>

Returns the limit at which a sharp corner is drawn beveled.

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
void <a href='#SkPaint_setStrokeMiter'>setStrokeMiter</a>(<a href='undocumented#SkScalar'>SkScalar</a> miter)
</pre>

Sets the limit at which a sharp corner is drawn beveled.
Valid values are zero and greater.
Has no effect if <a href='#SkPaint_setStrokeMiter_miter'>miter</a> is less than zero.

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
        <a href='#SkPaint_kButt_Cap'>kButt Cap</a>,
        <a href='#SkPaint_kRound_Cap'>kRound Cap</a>,
        <a href='#SkPaint_kSquare_Cap'>kSquare Cap</a>,

        <a href='#SkPaint_kLast_Cap'>kLast Cap</a> = <a href='#SkPaint_kSquare_Cap'>kSquare Cap</a>,
        <a href='#SkPaint_kDefault_Cap'>kDefault Cap</a> = <a href='#SkPaint_kButt_Cap'>kButt Cap</a>,
    };

    static constexpr int <a href='#SkPaint_kCapCount'>kCapCount</a> = <a href='#SkPaint_kLast_Cap'>kLast Cap</a> + 1;
</pre>

<a href='#Stroke_Cap'>Stroke Cap</a> draws at the beginning and end of an open <a href='SkPath_Reference#Contour'>Path Contour</a>.

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
Adds a circle with a diameter equal to <a href='#Stroke_Width'>Stroke Width</a> at the beginning
and end.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kSquare_Cap'><code>SkPaint::kSquare_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds a square with sides equal to <a href='#Stroke_Width'>Stroke Width</a> at the beginning
and end. The square sides are parallel to the initial and final direction
of the stroke.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kLast_Cap'><code>SkPaint::kLast_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Equivalent to the largest value for <a href='#Stroke_Cap'>Stroke Cap</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kDefault_Cap'><code>SkPaint::kDefault_Cap</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#Stroke_Cap'>Stroke Cap</a> is set to <a href='#SkPaint_kButt_Cap'>kButt Cap</a> by default.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kCapCount'><code>SkPaint::kCapCount</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#Stroke_Cap'>Stroke Cap</a> is a legal value.
</td>
  </tr>
</table>

Stroke describes the area covered by a pen of <a href='#Stroke_Width'>Stroke Width</a> as it
follows the <a href='SkPath_Reference#Contour'>Path Contour</a>, moving parallel to the contour direction.

If the <a href='SkPath_Reference#Contour'>Path Contour</a> is not terminated by <a href='SkPath_Reference#SkPath_kClose_Verb'>SkPath::kClose Verb</a>, the contour has a
visible beginning and end.

<a href='SkPath_Reference#Contour'>Path Contour</a> may start and end at the same point; defining <a href='SkPath_Reference#Contour_Zero_Length'>Zero Length Contour</a>.

<a href='#SkPaint_kButt_Cap'>kButt Cap</a> and <a href='SkPath_Reference#Contour_Zero_Length'>Zero Length Contour</a> is not drawn.
<a href='#SkPaint_kRound_Cap'>kRound Cap</a> and <a href='SkPath_Reference#Contour_Zero_Length'>Zero Length Contour</a> draws a circle of diameter <a href='#Stroke_Width'>Stroke Width</a>
at the contour point.
<a href='#SkPaint_kSquare_Cap'>kSquare Cap</a> and <a href='SkPath_Reference#Contour_Zero_Length'>Zero Length Contour</a> draws an upright square with a side of
<a href='#Stroke_Width'>Stroke Width</a> at the contour point.

<a href='#Stroke_Cap'>Stroke Cap</a> is <a href='#SkPaint_kButt_Cap'>kButt Cap</a> by default.

### Example

<div><fiddle-embed name="2bffb6384cc20077e632e7d01da045ca"></fiddle-embed></div>

<a name='SkPaint_getStrokeCap'></a>
## getStrokeCap

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_getStrokeCap'>getStrokeCap</a>() const
</pre>

Returns the geometry drawn at the beginning and end of strokes.

### Return Value

one of: <a href='#SkPaint_kButt_Cap'>kButt Cap</a>, <a href='#SkPaint_kRound_Cap'>kRound Cap</a>, <a href='#SkPaint_kSquare_Cap'>kSquare Cap</a>

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
void <a href='#SkPaint_setStrokeCap'>setStrokeCap</a>(<a href='#SkPaint_Cap'>Cap</a> cap)
</pre>

Sets the geometry drawn at the beginning and end of strokes.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeCap_cap'><code><strong>cap</strong></code></a></td>
    <td>one of: <a href='#SkPaint_kButt_Cap'>kButt Cap</a>, <a href='#SkPaint_kRound_Cap'>kRound Cap</a>, <a href='#SkPaint_kSquare_Cap'>kSquare Cap</a>;
has no effect if <a href='#SkPaint_setStrokeCap_cap'>cap</a> is not valid</td>
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

<a href='#Stroke_Join'>Stroke Join</a> draws at the sharp corners of an open or closed <a href='SkPath_Reference#Contour'>Path Contour</a>.

Stroke describes the area covered by a pen of <a href='#Stroke_Width'>Stroke Width</a> as it
follows the <a href='SkPath_Reference#Contour'>Path Contour</a>, moving parallel to the contour direction.

If the contour direction changes abruptly, because the tangent direction leading
to the end of a curve within the contour does not match the tangent direction of
the following curve, the pair of curves meet at <a href='#Stroke_Join'>Stroke Join</a>.

### Example

<div><fiddle-embed name="917c44b504d3f9308571fd3835d90a0d"></fiddle-embed></div>

## <a name='SkPaint_Join'>Enum SkPaint::Join</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Join'>Join</a> {
        <a href='#SkPaint_kMiter_Join'>kMiter Join</a>,
        <a href='#SkPaint_kRound_Join'>kRound Join</a>,
        <a href='#SkPaint_kBevel_Join'>kBevel Join</a>,

        <a href='#SkPaint_kLast_Join'>kLast Join</a> = <a href='#SkPaint_kBevel_Join'>kBevel Join</a>,
        <a href='#SkPaint_kDefault_Join'>kDefault Join</a> = <a href='#SkPaint_kMiter_Join'>kMiter Join</a>,
    };

    static constexpr int <a href='#SkPaint_kJoinCount'>kJoinCount</a> = <a href='#SkPaint_kLast_Join'>kLast Join</a> + 1;
</pre>

<a href='#SkPaint_Join'>Join</a> specifies how corners are drawn when a shape is stroked. <a href='#SkPaint_Join'>Join</a>
affects the four corners of a stroked rectangle, and the connected segments in a
stroked path.

Choose miter join to draw sharp corners. Choose round join to draw a circle with a
radius equal to the stroke width on top of the corner. Choose bevel join to minimally
connect the thick strokes.

The fill path constructed to describe the stroked path respects the join setting but may
not contain the actual join. For instance, a fill path constructed with round joins does
not necessarily include circles at each connected segment.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kMiter_Join'><code>SkPaint::kMiter_Join</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Extends the outside corner to the extent allowed by <a href='#Miter_Limit'>Miter Limit</a>.
If the extension exceeds <a href='#Miter_Limit'>Miter Limit</a>, <a href='#SkPaint_kBevel_Join'>kBevel Join</a> is used instead.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kRound_Join'><code>SkPaint::kRound_Join</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds a circle with a diameter of <a href='#Stroke_Width'>Stroke Width</a> at the sharp corner.
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
<a href='#Stroke_Join'>Stroke Join</a> is set to <a href='#SkPaint_kMiter_Join'>kMiter Join</a> by default.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kJoinCount'><code>SkPaint::kJoinCount</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#Stroke_Join'>Stroke Join</a> is a legal value.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3b1aebacc21c1836a52876b9b0b3905e"></fiddle-embed></div>

### See Also

<a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a> <a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a> <a href='#SkPaint_setStrokeMiter'>setStrokeMiter</a> <a href='#SkPaint_getStrokeMiter'>getStrokeMiter</a>

<a name='SkPaint_getStrokeJoin'></a>
## getStrokeJoin

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_getStrokeJoin'>getStrokeJoin</a>() const
</pre>

Returns the geometry drawn at the corners of strokes.

### Return Value

one of: <a href='#SkPaint_kMiter_Join'>kMiter Join</a>, <a href='#SkPaint_kRound_Join'>kRound Join</a>, <a href='#SkPaint_kBevel_Join'>kBevel Join</a>

### Example

<div><fiddle-embed name="af6dec8ef974aa67bf102f29915bcd6a">

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
void <a href='#SkPaint_setStrokeJoin'>setStrokeJoin</a>(<a href='#SkPaint_Join'>Join</a> join)
</pre>

Sets the geometry drawn at the corners of strokes.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setStrokeJoin_join'><code><strong>join</strong></code></a></td>
    <td>one of: <a href='#SkPaint_kMiter_Join'>kMiter Join</a>, <a href='#SkPaint_kRound_Join'>kRound Join</a>, <a href='#SkPaint_kBevel_Join'>kBevel Join</a>;
otherwise, has no effect</td>
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

<a href='#Fill_Path'>Fill Path</a> creates a <a href='SkPath_Reference#Path'>Path</a> by applying the <a href='undocumented#Path_Effect'>Path Effect</a>, followed by the <a href='#Style_Stroke'>Style Stroke</a>.

If <a href='#Paint'>Paint</a> contains <a href='undocumented#Path_Effect'>Path Effect</a>, <a href='undocumented#Path_Effect'>Path Effect</a> operates on the source <a href='SkPath_Reference#Path'>Path</a>; the result
replaces the destination <a href='SkPath_Reference#Path'>Path</a>. Otherwise, the source <a href='SkPath_Reference#Path'>Path</a> is replaces the
destination <a href='SkPath_Reference#Path'>Path</a>.

Fill <a href='SkPath_Reference#Path'>Path</a> can request the <a href='undocumented#Path_Effect'>Path Effect</a> to restrict to a culling rectangle, but
the <a href='undocumented#Path_Effect'>Path Effect</a> is not required to do so.

If <a href='#SkPaint_Style'>Style</a> is <a href='#SkPaint_kStroke_Style'>kStroke Style</a> or <a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill Style</a>,
and <a href='#Stroke_Width'>Stroke Width</a> is greater than zero, the <a href='#Stroke_Width'>Stroke Width</a>, <a href='#Stroke_Cap'>Stroke Cap</a>, <a href='#Stroke_Join'>Stroke Join</a>,
and <a href='#Miter_Limit'>Miter Limit</a> operate on the destination <a href='SkPath_Reference#Path'>Path</a>, replacing it.

Fill <a href='SkPath_Reference#Path'>Path</a> can specify the precision used by <a href='#Stroke_Width'>Stroke Width</a> to approximate the stroke geometry.

If the <a href='#SkPaint_Style'>Style</a> is <a href='#SkPaint_kStroke_Style'>kStroke Style</a> and the <a href='#Stroke_Width'>Stroke Width</a> is zero, <a href='#SkPaint_getFillPath'>getFillPath</a>
returns false since <a href='#Style_Hairline'>Hairline</a> has no filled equivalent.

<a name='SkPaint_getFillPath'></a>
## getFillPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_getFillPath'>getFillPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& src, <a href='SkPath_Reference#SkPath'>SkPath</a>* dst, const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect, <a href='undocumented#SkScalar'>SkScalar</a> resScale = 1) const
</pre>

Returns the filled equivalent of the stroked path.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getFillPath_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPath_Reference#Path'>Path</a> read to create a filled version</td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_dst'><code><strong>dst</strong></code></a></td>
    <td>resulting <a href='SkPath_Reference#Path'>Path</a>; may be the same as <a href='#SkPaint_getFillPath_src'>src</a>, but may not be nullptr</td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>optional limit passed to <a href='undocumented#Path_Effect'>Path Effect</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_resScale'><code><strong>resScale</strong></code></a></td>
    <td>if > 1, increase precision, else if (0 < res < 1) reduce precision
to favor speed and size</td>
  </tr>
</table>

### Return Value

true if the path represents <a href='#Style_Fill'>Style Fill</a>, or false if it represents <a href='#Style_Hairline'>Hairline</a>

### Example

<div><fiddle-embed name="cedd6233848198e1fca4d1e14816baaf"><div>A very small <a href='SkPath_Reference#Quad'>Quad</a> stroke is turned into a filled path with increasing levels of precision.
At the lowest precision, the <a href='SkPath_Reference#Quad'>Quad</a> stroke is approximated by a rectangle.
At the highest precision, the filled path has high fidelity compared to the original stroke.
</div></fiddle-embed></div>

---

<a name='SkPaint_getFillPath_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_getFillPath'>getFillPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& src, <a href='SkPath_Reference#SkPath'>SkPath</a>* dst) const
</pre>

Returns the filled equivalent of the stroked path.

Replaces <a href='#SkPaint_getFillPath_2_dst'>dst</a> with the <a href='#SkPaint_getFillPath_2_src'>src</a> path modified by <a href='undocumented#Path_Effect'>Path Effect</a> and <a href='#Style_Stroke'>Style Stroke</a>.
<a href='undocumented#Path_Effect'>Path Effect</a>, if any, is not culled. <a href='#Stroke_Width'>Stroke Width</a> is created with default precision.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getFillPath_2_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPath_Reference#Path'>Path</a> read to create a filled version</td>
  </tr>
  <tr>    <td><a name='SkPaint_getFillPath_2_dst'><code><strong>dst</strong></code></a></td>
    <td>resulting <a href='SkPath_Reference#Path'>Path</a> <a href='#SkPaint_getFillPath_2_dst'>dst</a> may be the same as <a href='#SkPaint_getFillPath_2_src'>src</a>, but may not be nullptr</td>
  </tr>
</table>

### Return Value

true if the path represents <a href='#Style_Fill'>Style Fill</a>, or false if it represents <a href='#Style_Hairline'>Hairline</a>

### Example

<div><fiddle-embed name="e6d8ca0cc17e0b475bd54dd995825468"></fiddle-embed></div>

---

### See Also

<a href='#Style_Stroke'>Style Stroke</a> <a href='#Stroke_Width'>Stroke Width</a> <a href='undocumented#Path_Effect'>Path Effect</a>

## <a name='Shader_Methods'>Shader Methods</a>

<a href='undocumented#Shader'>Shader</a> defines the colors used when drawing a shape.
<a href='undocumented#Shader'>Shader</a> may be an image, a gradient, or a computed fill.
If <a href='#Paint'>Paint</a> has no <a href='undocumented#Shader'>Shader</a>, then <a href='SkColor_Reference#Color'>Color</a> fills the shape.

<a href='undocumented#Shader'>Shader</a> is modulated by <a href='SkColor_Reference#Alpha'>Color Alpha</a> component of <a href='SkColor_Reference#Color'>Color</a>.
If <a href='undocumented#Shader'>Shader</a> object defines only <a href='SkColor_Reference#Alpha'>Color Alpha</a>, then <a href='SkColor_Reference#Color'>Color</a> modulated by <a href='SkColor_Reference#Alpha'>Color Alpha</a> describes
the fill.

The drawn transparency can be modified without altering <a href='undocumented#Shader'>Shader</a>, by changing <a href='SkColor_Reference#Alpha'>Color Alpha</a>.

### Example

<div><fiddle-embed name="c015dc2010c15e1c00b4f7330232b0f7"></fiddle-embed></div>

If <a href='undocumented#Shader'>Shader</a> generates only <a href='SkColor_Reference#Alpha'>Color Alpha</a> then all components of <a href='SkColor_Reference#Color'>Color</a> modulate the output.

### Example

<div><fiddle-embed name="fe80fd80b98a20823db7fb9a077243c7"></fiddle-embed></div>

<a name='SkPaint_getShader'></a>
## getShader

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkShader'>SkShader</a>* <a href='#SkPaint_getShader'>getShader</a>() const
</pre>

Returns optional colors used when filling a path, such as a gradient.

Does not alter <a href='undocumented#Shader'>Shader</a> <a href='undocumented#Reference_Count'>Reference Count</a>.

### Return Value

<a href='undocumented#Shader'>Shader</a> if previously set, nullptr otherwise

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
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkShader'>SkShader</a>&gt; <a href='#SkPaint_refShader'>refShader</a>() const
</pre>

Returns optional colors used when filling a path, such as a gradient.

Increases <a href='undocumented#Shader'>Shader</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one.

### Return Value

<a href='undocumented#Shader'>Shader</a> if previously set, nullptr otherwise

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
void <a href='#SkPaint_setShader'>setShader</a>(<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkShader'>SkShader</a>&gt; shader)
</pre>

Sets optional colors used when filling a path, such as a gradient.

Sets <a href='undocumented#Shader'>Shader</a> to <a href='#SkPaint_setShader_shader'>shader</a>, decreasing <a href='undocumented#Reference_Count'>Reference Count</a> of the previous <a href='undocumented#Shader'>Shader</a>.
Increments <a href='#SkPaint_setShader_shader'>shader</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setShader_shader'><code><strong>shader</strong></code></a></td>
    <td>how geometry is filled with color; if nullptr, <a href='SkColor_Reference#Color'>Color</a> is used instead</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="77e64d5bae9b1ba037fd99252bb4aa58"></fiddle-embed></div>

---

## <a name='Color_Filter_Methods'>Color Filter Methods</a>

<a href='undocumented#Color_Filter'>Color Filter</a> alters the color used when drawing a shape.
<a href='undocumented#Color_Filter'>Color Filter</a> may apply <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a>, transform the color through a matrix, or composite multiple filters.
If <a href='#Paint'>Paint</a> has no <a href='undocumented#Color_Filter'>Color Filter</a>, the color is unaltered.

The drawn transparency can be modified without altering <a href='undocumented#Color_Filter'>Color Filter</a>, by changing <a href='SkColor_Reference#Alpha'>Color Alpha</a>.

### Example

<div><fiddle-embed name="5abde56ca2f89a18b8e231abd1b57c56"></fiddle-embed></div>

<a name='SkPaint_getColorFilter'></a>
## getColorFilter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkColorFilter'>SkColorFilter</a>* <a href='#SkPaint_getColorFilter'>getColorFilter</a>() const
</pre>

Returns <a href='undocumented#Color_Filter'>Color Filter</a> if set, or nullptr.
Does not alter <a href='undocumented#Color_Filter'>Color Filter</a> <a href='undocumented#Reference_Count'>Reference Count</a>.

### Return Value

<a href='undocumented#Color_Filter'>Color Filter</a> if previously set, nullptr otherwise

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
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkColorFilter'>SkColorFilter</a>&gt; <a href='#SkPaint_refColorFilter'>refColorFilter</a>() const
</pre>

Returns <a href='undocumented#Color_Filter'>Color Filter</a> if set, or nullptr.
Increases <a href='undocumented#Color_Filter'>Color Filter</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one.

### Return Value

<a href='undocumented#Color_Filter'>Color Filter</a> if set, or nullptr

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
void <a href='#SkPaint_setColorFilter'>setColorFilter</a>(<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkColorFilter'>SkColorFilter</a>&gt; colorFilter)
</pre>

Sets <a href='undocumented#Color_Filter'>Color Filter</a> to filter, decreasing <a href='undocumented#Reference_Count'>Reference Count</a> of the previous
<a href='undocumented#Color_Filter'>Color Filter</a>. Pass nullptr to clear <a href='undocumented#Color_Filter'>Color Filter</a>.

Increments filter <a href='undocumented#Reference_Count'>Reference Count</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setColorFilter_colorFilter'><code><strong>colorFilter</strong></code></a></td>
    <td><a href='undocumented#Color_Filter'>Color Filter</a> to apply to subsequent draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c7b786dc9b3501cd0eaba47494b6fa31"></fiddle-embed></div>

---

## <a name='Blend_Mode_Methods'>Blend Mode Methods</a>

<a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> describes how <a href='SkColor_Reference#Color'>Color</a> combines with the destination color.
The default setting, <a href='SkBlendMode_Reference#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a>, draws the source color
over the destination color.

### Example

<div><fiddle-embed name="73092d4d06faecea3c204d852a4dd8a8"></fiddle-embed></div>

### See Also

<a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a>

<a name='SkPaint_getBlendMode'></a>
## getBlendMode

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkPaint_getBlendMode'>getBlendMode</a>() const
</pre>

Returns <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a>.
By default, returns <a href='SkBlendMode_Reference#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a>.

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
bool <a href='#SkPaint_isSrcOver'>isSrcOver</a>() const
</pre>

Returns true if <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> is <a href='SkBlendMode_Reference#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a>, the default.

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
void <a href='#SkPaint_setBlendMode'>setBlendMode</a>(<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode)
</pre>

Sets <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> to <a href='#SkPaint_setBlendMode_mode'>mode</a>.
Does not check for valid input.

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

<a href='undocumented#Path_Effect'>Path Effect</a> modifies the path geometry before drawing it.
<a href='undocumented#Path_Effect'>Path Effect</a> may implement dashing, custom fill effects and custom stroke effects.
If <a href='#Paint'>Paint</a> has no <a href='undocumented#Path_Effect'>Path Effect</a>, the path geometry is unaltered when filled or stroked.

### Example

<div><fiddle-embed name="8cf5684b187d60f09e11c4a48993ea39"></fiddle-embed></div>

### See Also

<a href='undocumented#Path_Effect'>Path Effect</a>

<a name='SkPaint_getPathEffect'></a>
## getPathEffect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkPathEffect'>SkPathEffect</a>* <a href='#SkPaint_getPathEffect'>getPathEffect</a>() const
</pre>

Returns <a href='undocumented#Path_Effect'>Path Effect</a> if set, or nullptr.
Does not alter <a href='undocumented#Path_Effect'>Path Effect</a> <a href='undocumented#Reference_Count'>Reference Count</a>.

### Return Value

<a href='undocumented#Path_Effect'>Path Effect</a> if previously set, nullptr otherwise

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
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkPathEffect'>SkPathEffect</a>&gt; <a href='#SkPaint_refPathEffect'>refPathEffect</a>() const
</pre>

Returns <a href='undocumented#Path_Effect'>Path Effect</a> if set, or nullptr.
Increases <a href='undocumented#Path_Effect'>Path Effect</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one.

### Return Value

<a href='undocumented#Path_Effect'>Path Effect</a> if previously set, nullptr otherwise

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
void <a href='#SkPaint_setPathEffect'>setPathEffect</a>(<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkPathEffect'>SkPathEffect</a>&gt; pathEffect)
</pre>

Sets <a href='undocumented#Path_Effect'>Path Effect</a> to <a href='#SkPaint_setPathEffect_pathEffect'>pathEffect</a>, decreasing <a href='undocumented#Reference_Count'>Reference Count</a> of the previous
<a href='undocumented#Path_Effect'>Path Effect</a>. Pass nullptr to leave the path geometry unaltered.

Increments <a href='#SkPaint_setPathEffect_pathEffect'>pathEffect</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setPathEffect_pathEffect'><code><strong>pathEffect</strong></code></a></td>
    <td>replace <a href='SkPath_Reference#Path'>Path</a> with a modification when drawn</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="52dd55074ca0b7d520d04e750ca2a0d7"></fiddle-embed></div>

---

## <a name='Mask_Filter_Methods'>Mask Filter Methods</a>

<a href='undocumented#Mask_Filter'>Mask Filter</a> uses coverage of the shape drawn to create <a href='undocumented#Mask_Alpha'>Mask Alpha</a>.
<a href='undocumented#Mask_Filter'>Mask Filter</a> takes a Mask, and returns a Mask.

<a href='undocumented#Mask_Filter'>Mask Filter</a> may change the geometry and transparency of the shape, such as
creating a blur effect. Set <a href='undocumented#Mask_Filter'>Mask Filter</a> to nullptr to prevent <a href='undocumented#Mask_Filter'>Mask Filter</a> from
modifying the draw.

### Example

<div><fiddle-embed name="55d7b9d482ac8e17a6153f555a8adb8d"></fiddle-embed></div>

<a name='SkPaint_getMaskFilter'></a>
## getMaskFilter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>* <a href='#SkPaint_getMaskFilter'>getMaskFilter</a>() const
</pre>

Returns <a href='undocumented#Mask_Filter'>Mask Filter</a> if set, or nullptr.
Does not alter <a href='undocumented#Mask_Filter'>Mask Filter</a> <a href='undocumented#Reference_Count'>Reference Count</a>.

### Return Value

<a href='undocumented#Mask_Filter'>Mask Filter</a> if previously set, nullptr otherwise

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
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>&gt; <a href='#SkPaint_refMaskFilter'>refMaskFilter</a>() const
</pre>

Returns <a href='undocumented#Mask_Filter'>Mask Filter</a> if set, or nullptr.

Increases <a href='undocumented#Mask_Filter'>Mask Filter</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one.

### Return Value

<a href='undocumented#Mask_Filter'>Mask Filter</a> if previously set, nullptr otherwise

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
void <a href='#SkPaint_setMaskFilter'>setMaskFilter</a>(<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkMaskFilter'>SkMaskFilter</a>&gt; maskFilter)
</pre>

Sets <a href='undocumented#Mask_Filter'>Mask Filter</a> to <a href='#SkPaint_setMaskFilter_maskFilter'>maskFilter</a>, decreasing <a href='undocumented#Reference_Count'>Reference Count</a> of the previous
<a href='undocumented#Mask_Filter'>Mask Filter</a>. Pass nullptr to clear <a href='undocumented#Mask_Filter'>Mask Filter</a> and leave <a href='undocumented#Mask_Filter'>Mask Filter</a> effect on
<a href='undocumented#Mask_Alpha'>Mask Alpha</a> unaltered.

Increments <a href='#SkPaint_setMaskFilter_maskFilter'>maskFilter</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setMaskFilter_maskFilter'><code><strong>maskFilter</strong></code></a></td>
    <td>modifies clipping mask generated from drawn geometry</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a993831c40f3e134f809134e3b74e4a6"></fiddle-embed></div>

---

## <a name='Typeface_Methods'>Typeface Methods</a>

<a href='undocumented#Typeface'>Typeface</a> identifies the font used when drawing and measuring text.
<a href='undocumented#Typeface'>Typeface</a> may be specified by name, from a file, or from a data stream.
The default <a href='undocumented#Typeface'>Typeface</a> defers to the platform-specific default font
implementation.

### Example

<div><fiddle-embed name="1a7a5062725139760962582f599f1b97"></fiddle-embed></div>

<a name='SkPaint_getTypeface'></a>
## getTypeface

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkTypeface'>SkTypeface</a>* <a href='#SkPaint_getTypeface'>getTypeface</a>() const
</pre>

Returns <a href='undocumented#Typeface'>Typeface</a> if set, or nullptr.
Does not alter <a href='undocumented#Typeface'>Typeface</a> <a href='undocumented#Reference_Count'>Reference Count</a>.

### Return Value

<a href='undocumented#Typeface'>Typeface</a> if previously set, nullptr otherwise

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
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkTypeface'>SkTypeface</a>&gt; <a href='#SkPaint_refTypeface'>refTypeface</a>() const
</pre>

Increases <a href='undocumented#Typeface'>Typeface</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one.

### Return Value

<a href='undocumented#Typeface'>Typeface</a> if previously set, nullptr otherwise

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
void <a href='#SkPaint_setTypeface'>setTypeface</a>(<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkTypeface'>SkTypeface</a>&gt; typeface)
</pre>

Sets <a href='undocumented#Typeface'>Typeface</a> to <a href='#SkPaint_setTypeface_typeface'>typeface</a>, decreasing <a href='undocumented#Reference_Count'>Reference Count</a> of the previous <a href='undocumented#Typeface'>Typeface</a>.
Pass nullptr to clear <a href='undocumented#Typeface'>Typeface</a> and use the default <a href='#SkPaint_setTypeface_typeface'>typeface</a>. Increments
<a href='#SkPaint_setTypeface_typeface'>typeface</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTypeface_typeface'><code><strong>typeface</strong></code></a></td>
    <td>font and style used to draw text</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0e6fbb7773cd925b274552f4cd1abef2"></fiddle-embed></div>

---

## <a name='Image_Filter_Methods'>Image Filter Methods</a>

<a href='undocumented#Image_Filter'>Image Filter</a> operates on the pixel representation of the shape, as modified by <a href='#Paint'>Paint</a>
with <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> set to <a href='SkBlendMode_Reference#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a>. <a href='undocumented#Image_Filter'>Image Filter</a> creates a new bitmap,
which is drawn to the device using the set <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a>.

<a href='undocumented#Image_Filter'>Image Filter</a> is higher level than <a href='undocumented#Mask_Filter'>Mask Filter</a>; for instance, an <a href='undocumented#Image_Filter'>Image Filter</a>
can operate on all channels of <a href='SkColor_Reference#Color'>Color</a>, while <a href='undocumented#Mask_Filter'>Mask Filter</a> generates <a href='SkColor_Reference#Alpha'>Alpha</a> only.
<a href='undocumented#Image_Filter'>Image Filter</a> operates independently of and can be used in combination with
<a href='undocumented#Mask_Filter'>Mask Filter</a>.

### Example

<div><fiddle-embed name="ece04ee3d3761e3425f37c8f06f054c1"></fiddle-embed></div>

<a name='SkPaint_getImageFilter'></a>
## getImageFilter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='#SkPaint_getImageFilter'>getImageFilter</a>() const
</pre>

Returns <a href='undocumented#Image_Filter'>Image Filter</a> if set, or nullptr.
Does not alter <a href='undocumented#Image_Filter'>Image Filter</a> <a href='undocumented#Reference_Count'>Reference Count</a>.

### Return Value

<a href='undocumented#Image_Filter'>Image Filter</a> if previously set, nullptr otherwise

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
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkImageFilter'>SkImageFilter</a>&gt; <a href='#SkPaint_refImageFilter'>refImageFilter</a>() const
</pre>

Returns <a href='undocumented#Image_Filter'>Image Filter</a> if set, or nullptr.
Increases <a href='undocumented#Image_Filter'>Image Filter</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one.

### Return Value

<a href='undocumented#Image_Filter'>Image Filter</a> if previously set, nullptr otherwise

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
void <a href='#SkPaint_setImageFilter'>setImageFilter</a>(<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkImageFilter'>SkImageFilter</a>&gt; imageFilter)
</pre>

Sets <a href='undocumented#Image_Filter'>Image Filter</a> to <a href='#SkPaint_setImageFilter_imageFilter'>imageFilter</a>, decreasing <a href='undocumented#Reference_Count'>Reference Count</a> of the previous
<a href='undocumented#Image_Filter'>Image Filter</a>. Pass nullptr to clear <a href='undocumented#Image_Filter'>Image Filter</a>, and remove <a href='undocumented#Image_Filter'>Image Filter</a> effect
on drawing.

Increments <a href='#SkPaint_setImageFilter_imageFilter'>imageFilter</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one.

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
to another to construct the draw.

<a href='undocumented#Draw_Looper'>Draw Looper</a> draws one or more times, modifying the canvas and paint each time.
<a href='undocumented#Draw_Looper'>Draw Looper</a> may be used to draw multiple colors or create a colored shadow.
Set <a href='undocumented#Draw_Looper'>Draw Looper</a> to nullptr to prevent <a href='undocumented#Draw_Looper'>Draw Looper</a> from modifying the draw.

### Example

<div><fiddle-embed name="84ec12a36e50df5ac565cc7a75ffbe9f"></fiddle-embed></div>

<a name='SkPaint_getDrawLooper'></a>
## getDrawLooper

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>* <a href='#SkPaint_getDrawLooper'>getDrawLooper</a>() const
</pre>

Returns <a href='undocumented#Draw_Looper'>Draw Looper</a> if set, or nullptr.
Does not alter <a href='undocumented#Draw_Looper'>Draw Looper</a> <a href='undocumented#Reference_Count'>Reference Count</a>.

### Return Value

<a href='undocumented#Draw_Looper'>Draw Looper</a> if previously set, nullptr otherwise

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
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>&gt; <a href='#SkPaint_refDrawLooper'>refDrawLooper</a>() const
</pre>

Returns <a href='undocumented#Draw_Looper'>Draw Looper</a> if set, or nullptr.
Increases <a href='undocumented#Draw_Looper'>Draw Looper</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one.

### Return Value

<a href='undocumented#Draw_Looper'>Draw Looper</a> if previously set, nullptr otherwise

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
<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>* <a href='#SkPaint_getLooper'>getLooper</a>() const
</pre>

Deprecated.

---

<a name='SkPaint_setDrawLooper'></a>
## setDrawLooper

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setDrawLooper'>setDrawLooper</a>(<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>&gt; drawLooper)
</pre>

Sets <a href='undocumented#Draw_Looper'>Draw Looper</a> to <a href='#SkPaint_setDrawLooper_drawLooper'>drawLooper</a>, decreasing <a href='undocumented#Reference_Count'>Reference Count</a> of the previous
<a href='#SkPaint_setDrawLooper_drawLooper'>drawLooper</a>.  Pass nullptr to clear <a href='undocumented#Draw_Looper'>Draw Looper</a> and leave <a href='undocumented#Draw_Looper'>Draw Looper</a> effect on
drawing unaltered.

Increments <a href='#SkPaint_setDrawLooper_drawLooper'>drawLooper</a> <a href='undocumented#Reference_Count'>Reference Count</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setDrawLooper_drawLooper'><code><strong>drawLooper</strong></code></a></td>
    <td>iterates through drawing one or more time, altering <a href='#Paint'>Paint</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bf10f838b330f0a3a3266d42ea68a638"></fiddle-embed></div>

---

<a name='SkPaint_setLooper'></a>
## setLooper

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setLooper'>setLooper</a>(<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkDrawLooper'>SkDrawLooper</a>&gt; drawLooper)
</pre>

Deprecated.

---

## <a name='Text_Align'>Text Align</a>

## <a name='SkPaint_Align'>Enum SkPaint::Align</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_Align'>Align</a> {
        <a href='#SkPaint_kLeft_Align'>kLeft Align</a>,
        <a href='#SkPaint_kCenter_Align'>kCenter Align</a>,
        <a href='#SkPaint_kRight_Align'>kRight Align</a>,
    };

    static constexpr int <a href='#SkPaint_kAlignCount'>kAlignCount</a> = 3;
</pre>

<a href='#SkPaint_Align'>Align</a> adjusts the text relative to the text position.
<a href='#SkPaint_Align'>Align</a> affects <a href='undocumented#Glyph'>Glyphs</a> drawn with: <a href='SkCanvas_Reference#SkCanvas_drawText'>SkCanvas::drawText</a>, <a href='SkCanvas_Reference#SkCanvas_drawPosText'>SkCanvas::drawPosText</a>,
<a href='SkCanvas_Reference#SkCanvas_drawPosTextH'>SkCanvas::drawPosTextH</a>, <a href='SkCanvas_Reference#SkCanvas_drawTextOnPath'>SkCanvas::drawTextOnPath</a>,
<a href='SkCanvas_Reference#SkCanvas_drawTextOnPathHV'>SkCanvas::drawTextOnPathHV</a>, <a href='SkCanvas_Reference#SkCanvas_drawTextRSXform'>SkCanvas::drawTextRSXform</a>, <a href='SkCanvas_Reference#SkCanvas_drawTextBlob'>SkCanvas::drawTextBlob</a>,
and <a href='SkCanvas_Reference#SkCanvas_drawString'>SkCanvas::drawString</a>;
as well as calls that place text <a href='undocumented#Glyph'>Glyphs</a> like <a href='#SkPaint_getTextWidths'>getTextWidths</a> and <a href='#SkPaint_getTextPath'>getTextPath</a>.

The text position is set by the font for both horizontal and vertical text.
Typically, for horizontal text, the position is to the left side of the glyph on the
base line; and for vertical text, the position is the horizontal center of the glyph
at the caps height.

<a href='#SkPaint_Align'>Align</a> adjusts the glyph position to center it or move it to abut the position
using the metrics returned by the font.

<a href='#SkPaint_Align'>Align</a> defaults to <a href='#SkPaint_kLeft_Align'>kLeft Align</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kLeft_Align'><code>SkPaint::kLeft_Align</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Leaves the glyph at the position computed by the font offset by the text position.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kCenter_Align'><code>SkPaint::kCenter_Align</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Moves the glyph half its width if <a href='#SkPaint_Flags'>Flags</a> has <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> clear, and
half its height if <a href='#SkPaint_Flags'>Flags</a> has <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> set.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kRight_Align'><code>SkPaint::kRight_Align</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Moves the glyph by its width if <a href='#SkPaint_Flags'>Flags</a> has <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> clear,
and by its height if <a href='#SkPaint_Flags'>Flags</a> has <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> set.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_kAlignCount'><code>SkPaint::kAlignCount</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#SkPaint_Align'>Align</a> is a legal value.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="702617fd9ebc3f12e30081b5db93e8a8"><div>Each position separately moves the glyph in drawPosText.
</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="f1cbbbafe6b3c52b81309cccbf96a308"><div><a href='#Vertical_Text'>Vertical Text</a> treats <a href='#SkPaint_kLeft_Align'>kLeft Align</a> as top align, and <a href='#SkPaint_kRight_Align'>kRight Align</a> as bottom align.
</div></fiddle-embed></div>

<a name='SkPaint_getTextAlign'></a>
## getTextAlign

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_Align'>Align</a> <a href='#SkPaint_getTextAlign'>getTextAlign</a>() const
</pre>

Returns <a href='#Text_Align'>Text Align</a>.
Returns <a href='#SkPaint_kLeft_Align'>kLeft Align</a> if <a href='#Text_Align'>Text Align</a> has not been set.

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
void    <a href='#SkPaint_setTextAlign'>setTextAlign</a>(<a href='#SkPaint_Align'>Align</a> align)
</pre>

Sets <a href='#Text_Align'>Text Align</a> to <a href='#SkPaint_setTextAlign_align'>align</a>.
Has no effect if <a href='#SkPaint_setTextAlign_align'>align</a> is an invalid value.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextAlign_align'><code><strong>align</strong></code></a></td>
    <td>text placement relative to position</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d37540afd918506ac2594665ca63979b"><div><a href='undocumented#Text'>Text</a> is left-aligned by default, and then set to center. Setting the
alignment out of range has no effect.
</div></fiddle-embed></div>

---

## <a name='Text_Size'>Text Size</a>

<a href='#Text_Size'>Text Size</a> adjusts the overall text size in points.
<a href='#Text_Size'>Text Size</a> can be set to any positive value or zero.
<a href='#Text_Size'>Text Size</a> defaults to 12.
Set <a href='undocumented#SkPaintDefaults_TextSize'>SkPaintDefaults TextSize</a> at compile time to change the default setting.

### Example

<div><fiddle-embed name="91c9a3e498bb9412e4522a95d076ed5f"></fiddle-embed></div>

<a name='SkPaint_getTextSize'></a>
## getTextSize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSize'>getTextSize</a>() const
</pre>

Returns <a href='#Text_Size'>Text Size</a> in points.

### Return Value

typographic height of text

### Example

<div><fiddle-embed name="983e2a71ba72d4ba8c945420040b8f1c"></fiddle-embed></div>

---

<a name='SkPaint_setTextSize'></a>
## setTextSize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextSize'>setTextSize</a>(<a href='undocumented#SkScalar'>SkScalar</a> textSize)
</pre>

Sets <a href='#Text_Size'>Text Size</a> in points.
Has no effect if <a href='#SkPaint_setTextSize_textSize'>textSize</a> is not greater than or equal to zero.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextSize_textSize'><code><strong>textSize</strong></code></a></td>
    <td>typographic height of text</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6510c9e2f57b83c47e67829e7a68d493"></fiddle-embed></div>

---

## <a name='Text_Scale_X'>Text Scale X</a>

<a href='#Text_Scale_X'>Text Scale X</a> adjusts the text horizontal scale.
<a href='undocumented#Text'>Text</a> scaling approximates condensed and expanded type faces when the actual face
is not available.
<a href='#Text_Scale_X'>Text Scale X</a> can be set to any value.
<a href='#Text_Scale_X'>Text Scale X</a> defaults to 1.

### Example

<div><fiddle-embed name="d13d787c1e36f515319fc998411c1d91"></fiddle-embed></div>

<a name='SkPaint_getTextScaleX'></a>
## getTextScaleX

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextScaleX'>getTextScaleX</a>() const
</pre>

Returns <a href='#Text_Scale_X'>Text Scale X</a>.
Default value is 1.

### Return Value

text horizontal scale

### Example

<div><fiddle-embed name="5dc8e58f6910cb8e4de9ed60f888188b"></fiddle-embed></div>

---

<a name='SkPaint_setTextScaleX'></a>
## setTextScaleX

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextScaleX'>setTextScaleX</a>(<a href='undocumented#SkScalar'>SkScalar</a> scaleX)
</pre>

Sets <a href='#Text_Scale_X'>Text Scale X</a>.
Default value is 1.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextScaleX_scaleX'><code><strong>scaleX</strong></code></a></td>
    <td>text horizontal scale</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a75bbdb8bb866b125c4c1dd5e967d470"></fiddle-embed></div>

---

## <a name='Text_Skew_X'>Text Skew X</a>

<a href='#Text_Skew_X'>Text Skew X</a> adjusts the text horizontal slant.
<a href='undocumented#Text'>Text</a> skewing approximates italic and oblique type faces when the actual face
is not available.
<a href='#Text_Skew_X'>Text Skew X</a> can be set to any value.
<a href='#Text_Skew_X'>Text Skew X</a> defaults to 0.

### Example

<div><fiddle-embed name="aff208b0aab265f273045b27e683c17c"></fiddle-embed></div>

<a name='SkPaint_getTextSkewX'></a>
## getTextSkewX

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getTextSkewX'>getTextSkewX</a>() const
</pre>

Returns <a href='#Text_Skew_X'>Text Skew X</a>.
Default value is zero.

### Return Value

additional shear in x-axis relative to y-axis

### Example

<div><fiddle-embed name="c79a196278c58b34cd5f551b0124ecc9"></fiddle-embed></div>

---

<a name='SkPaint_setTextSkewX'></a>
## setTextSkewX

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_setTextSkewX'>setTextSkewX</a>(<a href='undocumented#SkScalar'>SkScalar</a> skewX)
</pre>

Sets <a href='#Text_Skew_X'>Text Skew X</a>.
Default value is zero.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextSkewX_skewX'><code><strong>skewX</strong></code></a></td>
    <td>additional shear in x-axis relative to y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6bd705a6e0c5f8ee24f302fe531bfabc"></fiddle-embed></div>

---

## <a name='Text_Encoding'>Text Encoding</a>

## <a name='SkPaint_TextEncoding'>Enum SkPaint::TextEncoding</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPaint_TextEncoding'>TextEncoding</a> {
        <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8 TextEncoding</a>,
        <a href='#SkPaint_kUTF16_TextEncoding'>kUTF16 TextEncoding</a>,
        <a href='#SkPaint_kUTF32_TextEncoding'>kUTF32 TextEncoding</a>,
        <a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID TextEncoding</a>,
    };
</pre>

<a href='#SkPaint_TextEncoding'>TextEncoding</a> determines whether text specifies character codes and their encoded
size, or glyph indices. Characters are encoded as specified by the <a href='https://unicode.org/standard/standard.html'>Unicode standard</a></a> .

Character codes encoded size are specified by UTF-8, UTF-16, or UTF-32.
All character code formats are able to represent all of Unicode, differing only
in the total storage required.

<a href='https://tools.ietf.org/html/rfc3629'>UTF-8 (RFC 3629)</a></a> encodes each character as one or more 8-bit bytes.

<a href='https://tools.ietf.org/html/rfc2781'>UTF-16 (RFC 2781)</a></a> encodes each character as one or two 16-bit words.

<a href='https://www.unicode.org/versions/Unicode5.0.0/ch03.pdf'>UTF-32</a></a> encodes each character as one 32-bit word.

<a href='undocumented#Font_Manager'>Font Manager</a> uses font data to convert character code points into glyph indices.
A glyph index is a 16-bit word.

<a href='#SkPaint_TextEncoding'>TextEncoding</a> is set to <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8 TextEncoding</a> by default.

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

<div><fiddle-embed name="b29294e7f29d160a1b46abf2dcec9d2a"><div>First line is encoded in UTF-8.
Second line is encoded in UTF-16.
Third line is encoded in UTF-32.
Fourth line has 16-bit glyph indices.
</div></fiddle-embed></div>

<a name='SkPaint_getTextEncoding'></a>
## getTextEncoding

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_getTextEncoding'>getTextEncoding</a>() const
</pre>

Returns <a href='#Text_Encoding'>Text Encoding</a>.
<a href='#Text_Encoding'>Text Encoding</a> determines how character code points are mapped to font glyph indices.

### Return Value

one of: <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8 TextEncoding</a>, <a href='#SkPaint_kUTF16_TextEncoding'>kUTF16 TextEncoding</a>, <a href='#SkPaint_kUTF32_TextEncoding'>kUTF32 TextEncoding</a>, or
<a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID TextEncoding</a>

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
void <a href='#SkPaint_setTextEncoding'>setTextEncoding</a>(<a href='#SkPaint_TextEncoding'>TextEncoding</a> encoding)
</pre>

Sets <a href='#Text_Encoding'>Text Encoding</a> to <a href='#SkPaint_setTextEncoding_encoding'>encoding</a>.
<a href='#Text_Encoding'>Text Encoding</a> determines how character code points are mapped to font glyph indices.
Invalid values for <a href='#SkPaint_setTextEncoding_encoding'>encoding</a> are ignored.

### Parameters

<table>  <tr>    <td><a name='SkPaint_setTextEncoding_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of: <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8 TextEncoding</a>, <a href='#SkPaint_kUTF16_TextEncoding'>kUTF16 TextEncoding</a>, <a href='#SkPaint_kUTF32_TextEncoding'>kUTF32 TextEncoding</a>, or
<a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID TextEncoding</a></td>
  </tr>
#

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

<a href='#Font_Metrics'>Font Metrics</a> describe dimensions common to the <a href='undocumented#Glyph'>Glyphs</a> in <a href='undocumented#Typeface'>Typeface</a>.
The dimensions are computed by <a href='undocumented#Font_Manager'>Font Manager</a> from font data and do not take
<a href='#Paint'>Paint</a> settings other than <a href='#Text_Size'>Text Size</a> into account.

<a href='undocumented#Font'>Font</a> dimensions specify the anchor to the left of the glyph at baseline as the origin.
X-axis values to the left of the glyph are negative, and to the right of the left glyph edge
are positive.
Y-axis values above the baseline are negative, and below the baseline are positive.

### Example

<div><fiddle-embed name="2bfa3783719fcd769af177a1b244e171"></fiddle-embed></div>

# <a name='SkPaint_FontMetrics'>Struct SkPaint::FontMetrics</a>

## <a name='Font_Metrics_Constant'>Constant</a>


SkPaint related constants are defined by <code>enum</code>, <code>enum class</code>,  <code>#define</code>, <code>const</code>, and <code>constexpr</code>.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_FontMetricsFlags'>FontMetricsFlags</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>valid <a href='#Font_Metrics'>Font Metrics</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_kStrikeoutPositionIsValid_Flag'>kStrikeoutPositionIsValid Flag</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>set if <a href='#SkPaint_FontMetrics_fStrikeoutPosition'>fStrikeoutPosition</a> is valid</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_kStrikeoutThicknessIsValid_Flag'>kStrikeoutThicknessIsValid Flag</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>set if <a href='#SkPaint_FontMetrics_fStrikeoutThickness'>fStrikeoutThickness</a> is valid</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_kUnderlinePositionIsValid_Flag'>kUnderlinePositionIsValid Flag</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>set if <a href='#SkPaint_FontMetrics_fUnderlinePosition'>fUnderlinePosition</a> is valid</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_kUnderlineThicknessIsValid_Flag'>kUnderlineThicknessIsValid Flag</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>set if <a href='#SkPaint_FontMetrics_fUnderlineThickness'>fUnderlineThickness</a> is valid</td>
  </tr>
</table>

## <a name='Font_Metrics_Member_Function'>Member_Function</a>


SkPaint member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_hasStrikeoutPosition'>hasStrikeoutPosition</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns strikeout position if set</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_hasStrikeoutThickness'>hasStrikeoutThickness</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns strikeout thickness if set</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_hasUnderlinePosition'>hasUnderlinePosition</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns underline position if set</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_hasUnderlineThickness'>hasUnderlineThickness</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns underline thickness if set</td>
  </tr>
</table>

## <a name='Font_Metrics_Member'>Member</a>


SkPaint members may be read and written directly without using a member function.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fAscent'>fAscent</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>distance to reserve above baseline</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fAvgCharWidth'>fAvgCharWidth</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>average character width</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fBottom'>fBottom</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>extent below baseline</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fCapHeight'>fCapHeight</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>height of an upper-case letter</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fDescent'>fDescent</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>distance to reserve below baseline</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fFlags'>fFlags</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>is set to <a href='#SkPaint_FontMetrics_FontMetricsFlags'>FontMetricsFlags</a> when metrics are valid</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fLeading'>fLeading</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>distance to add between lines</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fMaxCharWidth'>fMaxCharWidth</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>maximum character width</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fStrikeoutPosition'>fStrikeoutPosition</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>strikeout position relative to baseline</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fStrikeoutThickness'>fStrikeoutThickness</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>strikeout thickness</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fTop'>fTop</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>extent above baseline</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fUnderlinePosition'>fUnderlinePosition</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>underline position relative to baseline</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fUnderlineThickness'>fUnderlineThickness</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>underline thickness</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fXHeight'>fXHeight</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>height of lower-case 'x'</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fXMax'>fXMax</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>maximum x</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_FontMetrics_fXMin'>fXMin</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>minimum x</td>
  </tr>
</table>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    struct <a href='#SkPaint_FontMetrics'>FontMetrics</a> {
        enum <a href='#SkPaint_FontMetrics_FontMetricsFlags'>FontMetricsFlags</a> {
            <a href='#SkPaint_FontMetrics_kUnderlineThicknessIsValid_Flag'>kUnderlineThicknessIsValid Flag</a> = 1 << 0,
            <a href='#SkPaint_FontMetrics_kUnderlinePositionIsValid_Flag'>kUnderlinePositionIsValid Flag</a> = 1 << 1,
            <a href='#SkPaint_FontMetrics_kStrikeoutThicknessIsValid_Flag'>kStrikeoutThicknessIsValid Flag</a> = 1 << 2,
            <a href='#SkPaint_FontMetrics_kStrikeoutPositionIsValid_Flag'>kStrikeoutPositionIsValid Flag</a> = 1 << 3,
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

        bool <a href='#SkPaint_FontMetrics_hasUnderlineThickness'>hasUnderlineThickness(SkScalar* thickness)</a> const;
        bool <a href='#SkPaint_FontMetrics_hasUnderlinePosition'>hasUnderlinePosition(SkScalar* position)</a> const;
        bool <a href='#SkPaint_FontMetrics_hasStrikeoutThickness'>hasStrikeoutThickness(SkScalar* thickness)</a> const;
        bool <a href='#SkPaint_FontMetrics_hasStrikeoutPosition'>hasStrikeoutPosition(SkScalar* position)</a> const;
    };
</pre>

<a href='#SkPaint_FontMetrics'>FontMetrics</a> is filled out by <a href='#SkPaint_getFontMetrics'>getFontMetrics</a>. <a href='#SkPaint_FontMetrics'>FontMetrics</a> contents reflect the values
computed by <a href='undocumented#Font_Manager'>Font Manager</a> using <a href='undocumented#Typeface'>Typeface</a>. Values are set to zero if they are
not available.

All vertical values are relative to the baseline, on a y-axis pointing down.
Zero is on the baseline, negative values are above the baseline, and positive
values are below the baseline.

<a href='#SkPaint_FontMetrics_fUnderlineThickness'>fUnderlineThickness</a> and <a href='#SkPaint_FontMetrics_fUnderlinePosition'>fUnderlinePosition</a> have a bit set in <a href='#SkPaint_FontMetrics_fFlags'>fFlags</a> if their values
are valid, since their value may be zero.

<a href='#SkPaint_FontMetrics_fStrikeoutThickness'>fStrikeoutThickness</a> and <a href='#SkPaint_FontMetrics_fStrikeoutPosition'>fStrikeoutPosition</a> have a bit set in <a href='#SkPaint_FontMetrics_fFlags'>fFlags</a> if their values
are valid, since their value may be zero.

## <a name='SkPaint_FontMetrics_FontMetricsFlags'>Enum SkPaint::FontMetrics::FontMetricsFlags</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
        enum <a href='#SkPaint_FontMetrics_FontMetricsFlags'>FontMetricsFlags</a> {
            <a href='#SkPaint_FontMetrics_kUnderlineThicknessIsValid_Flag'>kUnderlineThicknessIsValid Flag</a> = 1 << 0,
            <a href='#SkPaint_FontMetrics_kUnderlinePositionIsValid_Flag'>kUnderlinePositionIsValid Flag</a> = 1 << 1,
            <a href='#SkPaint_FontMetrics_kStrikeoutThicknessIsValid_Flag'>kStrikeoutThicknessIsValid Flag</a> = 1 << 2,
            <a href='#SkPaint_FontMetrics_kStrikeoutPositionIsValid_Flag'>kStrikeoutPositionIsValid Flag</a> = 1 << 3,
        };
</pre>

<a href='#SkPaint_FontMetrics_FontMetricsFlags'>FontMetricsFlags</a> are set in <a href='#SkPaint_FontMetrics_fFlags'>fFlags</a> when underline and strikeout metrics are valid;
the underline or strikeout metric may be valid and zero.
Fonts with embedded bitmaps may not have valid underline or strikeout metrics.

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
</table>

### Members

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Name</th>
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
Greatest extent above the baseline for any glyph.
Typically less than zero.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fAscent'><code>fAscent</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Recommended distance above the baseline to reserve for a line of text.
Typically less than zero.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fDescent'><code>fDescent</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Recommended distance below the baseline to reserve for a line of text.
Typically greater than zero.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fBottom'><code>fBottom</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Greatest extent below the baseline for any glyph.
Typically greater than zero.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fLeading'><code>fLeading</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Recommended distance to add between lines of text.
Typically greater than or equal to zero.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fAvgCharWidth'><code>fAvgCharWidth</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Average character width, if it is available.
Zero if no average width is stored in the font.
</td>
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
Minimum bounding box x-axis value for all <a href='undocumented#Glyph'>Glyphs</a>.
Typically less than zero.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fXMax'><code>fXMax</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Maximum bounding box x-axis value for all <a href='undocumented#Glyph'>Glyphs</a>.
Typically greater than zero.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fXHeight'><code>fXHeight</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be zero if no lower-case height is stored in the font.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fCapHeight'><code>fCapHeight</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be zero if no upper-case height is stored in the font.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fUnderlineThickness'><code>fUnderlineThickness</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
If the metric is valid, the <a href='#SkPaint_FontMetrics_kUnderlineThicknessIsValid_Flag'>kUnderlineThicknessIsValid Flag</a> is set in <a href='#SkPaint_FontMetrics_fFlags'>fFlags</a>.
If <a href='#SkPaint_FontMetrics_kUnderlineThicknessIsValid_Flag'>kUnderlineThicknessIsValid Flag</a> is clear, <a href='#SkPaint_FontMetrics_fUnderlineThickness'>fUnderlineThickness</a> is zero.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fUnderlinePosition'><code>fUnderlinePosition</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Position of the top of the underline stroke relative to the baseline.
Typically positive when valid.

If the metric is valid, the <a href='#SkPaint_FontMetrics_kUnderlinePositionIsValid_Flag'>kUnderlinePositionIsValid Flag</a> is set in <a href='#SkPaint_FontMetrics_fFlags'>fFlags</a>.
If <a href='#SkPaint_FontMetrics_kUnderlinePositionIsValid_Flag'>kUnderlinePositionIsValid Flag</a> is clear, <a href='#SkPaint_FontMetrics_fUnderlinePosition'>fUnderlinePosition</a> is zero.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fStrikeoutThickness'><code>fStrikeoutThickness</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
If the metric is valid, the <a href='#SkPaint_FontMetrics_kStrikeoutThicknessIsValid_Flag'>kStrikeoutThicknessIsValid Flag</a> is set in <a href='#SkPaint_FontMetrics_fFlags'>fFlags</a>.
If <a href='#SkPaint_FontMetrics_kStrikeoutThicknessIsValid_Flag'>kStrikeoutThicknessIsValid Flag</a> is clear, <a href='#SkPaint_FontMetrics_fStrikeoutThickness'>fStrikeoutThickness</a> is zero.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPaint_FontMetrics_fStrikeoutPosition'><code>fStrikeoutPosition</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Position of the bottom of the strikeout stroke relative to the baseline.
Typically negative when valid.

If the metric is valid, the <a href='#SkPaint_FontMetrics_kStrikeoutPositionIsValid_Flag'>kStrikeoutPositionIsValid Flag</a> is set in <a href='#SkPaint_FontMetrics_fFlags'>fFlags</a>.
If <a href='#SkPaint_FontMetrics_kStrikeoutPositionIsValid_Flag'>kStrikeoutPositionIsValid Flag</a> is clear, <a href='#SkPaint_FontMetrics_fStrikeoutPosition'>fStrikeoutPosition</a> is zero.
</td>
  </tr>
</table>

<a name='SkPaint_FontMetrics_hasUnderlineThickness'></a>
## hasUnderlineThickness

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_FontMetrics_hasUnderlineThickness'>hasUnderlineThickness</a>(<a href='undocumented#SkScalar'>SkScalar</a>* thickness) const
</pre>

Returns true if <a href='#Font_Metrics'>Font Metrics</a> has a valid underline <a href='#SkPaint_FontMetrics_hasUnderlineThickness_thickness'>thickness</a>, and sets
<a href='#SkPaint_FontMetrics_hasUnderlineThickness_thickness'>thickness</a> to that value. If the underline <a href='#SkPaint_FontMetrics_hasUnderlineThickness_thickness'>thickness</a> is not valid,
return false, and ignore <a href='#SkPaint_FontMetrics_hasUnderlineThickness_thickness'>thickness</a>.

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
bool <a href='#SkPaint_FontMetrics_hasUnderlinePosition'>hasUnderlinePosition</a>(<a href='undocumented#SkScalar'>SkScalar</a>* position) const
</pre>

Returns true if <a href='#Font_Metrics'>Font Metrics</a> has a valid underline <a href='#SkPaint_FontMetrics_hasUnderlinePosition_position'>position</a>, and sets
<a href='#SkPaint_FontMetrics_hasUnderlinePosition_position'>position</a> to that value. If the underline <a href='#SkPaint_FontMetrics_hasUnderlinePosition_position'>position</a> is not valid,
return false, and ignore <a href='#SkPaint_FontMetrics_hasUnderlinePosition_position'>position</a>.

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
bool <a href='#SkPaint_FontMetrics_hasStrikeoutThickness'>hasStrikeoutThickness</a>(<a href='undocumented#SkScalar'>SkScalar</a>* thickness) const
</pre>

Returns true if <a href='#Font_Metrics'>Font Metrics</a> has a valid strikeout <a href='#SkPaint_FontMetrics_hasStrikeoutThickness_thickness'>thickness</a>, and sets
<a href='#SkPaint_FontMetrics_hasStrikeoutThickness_thickness'>thickness</a> to that value. If the underline <a href='#SkPaint_FontMetrics_hasStrikeoutThickness_thickness'>thickness</a> is not valid,
return false, and ignore <a href='#SkPaint_FontMetrics_hasStrikeoutThickness_thickness'>thickness</a>.

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
bool <a href='#SkPaint_FontMetrics_hasStrikeoutPosition'>hasStrikeoutPosition</a>(<a href='undocumented#SkScalar'>SkScalar</a>* position) const
</pre>

Returns true if <a href='#Font_Metrics'>Font Metrics</a> has a valid strikeout <a href='#SkPaint_FontMetrics_hasStrikeoutPosition_position'>position</a>, and sets
<a href='#SkPaint_FontMetrics_hasStrikeoutPosition_position'>position</a> to that value. If the underline <a href='#SkPaint_FontMetrics_hasStrikeoutPosition_position'>position</a> is not valid,
return false, and ignore <a href='#SkPaint_FontMetrics_hasStrikeoutPosition_position'>position</a>.

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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getFontMetrics'>getFontMetrics</a>(<a href='#SkPaint_FontMetrics'>FontMetrics</a>* metrics, <a href='undocumented#SkScalar'>SkScalar</a> scale = 0) const
</pre>

Returns <a href='#Font_Metrics'>Font Metrics</a> associated with <a href='undocumented#Typeface'>Typeface</a>.
The return value is the recommended spacing between lines: the sum of <a href='#SkPaint_getFontMetrics_metrics'>metrics</a>
descent, ascent, and leading.
If <a href='#SkPaint_getFontMetrics_metrics'>metrics</a> is not nullptr, <a href='#Font_Metrics'>Font Metrics</a> is copied to <a href='#SkPaint_getFontMetrics_metrics'>metrics</a>.
Results are scaled by <a href='#Text_Size'>Text Size</a> but does not take into account
dimensions required by <a href='#Text_Scale_X'>Text Scale X</a>, <a href='#Text_Skew_X'>Text Skew X</a>, <a href='#Fake_Bold'>Fake Bold</a>,
<a href='#Style_Stroke'>Style Stroke</a>, and <a href='undocumented#Path_Effect'>Path Effect</a>.
Results can be additionally scaled by <a href='#SkPaint_getFontMetrics_scale'>scale</a>; a <a href='#SkPaint_getFontMetrics_scale'>scale</a> of zero
is ignored.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getFontMetrics_metrics'><code><strong>metrics</strong></code></a></td>
    <td>storage for <a href='#Font_Metrics'>Font Metrics</a> from <a href='undocumented#Typeface'>Typeface</a>; may be nullptr</td>
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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_getFontSpacing'>getFontSpacing</a>() const
</pre>

Returns the recommended spacing between lines: the sum of metrics
descent, ascent, and leading.
Result is scaled by <a href='#Text_Size'>Text Size</a> but does not take into account
dimensions required by stroking and <a href='undocumented#Path_Effect'>Path Effect</a>.
Returns the same result as <a href='#SkPaint_getFontMetrics'>getFontMetrics</a>.

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
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPaint_getFontBounds'>getFontBounds</a>() const
</pre>

Returns the union of bounds of all <a href='undocumented#Glyph'>Glyphs</a>.
Returned dimensions are computed by <a href='undocumented#Font_Manager'>Font Manager</a> from font data,
ignoring <a href='#SkPaint_Hinting'>Hinting</a>. Includes <a href='#Text_Size'>Text Size</a>, <a href='#Text_Scale_X'>Text Scale X</a>,
and <a href='#Text_Skew_X'>Text Skew X</a>, but not <a href='#Fake_Bold'>Fake Bold</a> or <a href='undocumented#Path_Effect'>Path Effect</a>.

If <a href='#Text_Size'>Text Size</a> is large, <a href='#Text_Scale_X'>Text Scale X</a> is one, and <a href='#Text_Skew_X'>Text Skew X</a> is zero,
returns the same bounds as <a href='#Font_Metrics'>Font Metrics</a> { <a href='#SkPaint_FontMetrics_fXMin'>FontMetrics::fXMin</a>,
<a href='#SkPaint_FontMetrics_fTop'>FontMetrics::fTop</a>, <a href='#SkPaint_FontMetrics_fXMax'>FontMetrics::fXMax</a>, <a href='#SkPaint_FontMetrics_fBottom'>FontMetrics::fBottom</a> }.

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
int <a href='#SkPaint_textToGlyphs'>textToGlyphs</a>(const void* text, size_t byteLength, <a href='undocumented#SkGlyphID'>SkGlyphID</a> glyphs[]) const
</pre>

Converts <a href='#SkPaint_textToGlyphs_text'>text</a> into glyph indices.
Returns the number of glyph indices represented by <a href='#SkPaint_textToGlyphs_text'>text</a>.
<a href='#Text_Encoding'>Text Encoding</a> specifies how <a href='#SkPaint_textToGlyphs_text'>text</a> represents characters or <a href='#SkPaint_textToGlyphs_glyphs'>glyphs</a>.
<a href='#SkPaint_textToGlyphs_glyphs'>glyphs</a> may be nullptr, to compute the glyph count.

Does not check <a href='#SkPaint_textToGlyphs_text'>text</a> for valid character codes or valid glyph indices.

If <a href='#SkPaint_textToGlyphs_byteLength'>byteLength</a> equals zero, returns zero.
If <a href='#SkPaint_textToGlyphs_byteLength'>byteLength</a> includes a partial character, the partial character is ignored.

If <a href='#Text_Encoding'>Text Encoding</a> is <a href='#SkPaint_kUTF8_TextEncoding'>kUTF8 TextEncoding</a> and
<a href='#SkPaint_textToGlyphs_text'>text</a> contains an invalid UTF-8 sequence, zero is returned.

### Parameters

<table>  <tr>    <td><a name='SkPaint_textToGlyphs_text'><code><strong>text</strong></code></a></td>
    <td>character storage encoded with <a href='#Text_Encoding'>Text Encoding</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_textToGlyphs_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>length of character storage in bytes</td>
  </tr>
  <tr>    <td><a name='SkPaint_textToGlyphs_glyphs'><code><strong>glyphs</strong></code></a></td>
    <td>storage for glyph indices; may be nullptr</td>
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
int <a href='#SkPaint_countText'>countText</a>(const void* text, size_t byteLength) const
</pre>

Returns the number of <a href='undocumented#Glyph'>Glyphs</a> in <a href='#SkPaint_countText_text'>text</a>.
Uses <a href='#Text_Encoding'>Text Encoding</a> to count the <a href='undocumented#Glyph'>Glyphs</a>.
Returns the same result as <a href='#SkPaint_textToGlyphs'>textToGlyphs</a>.

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
bool <a href='#SkPaint_containsText'>containsText</a>(const void* text, size_t byteLength) const
</pre>

Returns true if all <a href='#SkPaint_containsText_text'>text</a> corresponds to a non-zero glyph index.
Returns false if any characters in <a href='#SkPaint_containsText_text'>text</a> are not supported in
<a href='undocumented#Typeface'>Typeface</a>.

If <a href='#Text_Encoding'>Text Encoding</a> is <a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID TextEncoding</a>,
returns true if all glyph indices in <a href='#SkPaint_containsText_text'>text</a> are non-zero;
does not check to see if <a href='#SkPaint_containsText_text'>text</a> contains valid glyph indices for <a href='undocumented#Typeface'>Typeface</a>.

Returns true if <a href='#SkPaint_containsText_byteLength'>byteLength</a> is zero.

### Parameters

<table>  <tr>    <td><a name='SkPaint_containsText_text'><code><strong>text</strong></code></a></td>
    <td>array of characters or <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_containsText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>number of bytes in <a href='#SkPaint_containsText_text'>text</a> array</td>
  </tr>
</table>

### Return Value

true if all <a href='#SkPaint_containsText_text'>text</a> corresponds to a non-zero glyph index

<div><a href='#SkPaint_containsText'>containsText</a> succeeds for degree symbol, but cannot find a glyph index
corresponding to the Unicode surrogate code point.
</div>

#### Example Output

~~~~
0x00b0 == has char
0xd800 != has char
~~~~

### Example

<div><fiddle-embed name="083557b6f653d6fc00a34e01f87b74ff"><div><a href='#SkPaint_containsText'>containsText</a> returns true that glyph index is greater than zero, not
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

---

<a name='SkPaint_glyphsToUnichars'></a>
## glyphsToUnichars

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_glyphsToUnichars'>glyphsToUnichars</a>(const <a href='undocumented#SkGlyphID'>SkGlyphID</a> glyphs[], int count, <a href='undocumented#SkUnichar'>SkUnichar</a> text[]) const
</pre>

Converts <a href='#SkPaint_glyphsToUnichars_glyphs'>glyphs</a> into <a href='#SkPaint_glyphsToUnichars_text'>text</a> if possible.
<a href='undocumented#Glyph'>Glyph</a> values without direct Unicode equivalents are mapped to zero.
Uses the <a href='undocumented#Typeface'>Typeface</a>, but is unaffected
by <a href='#Text_Encoding'>Text Encoding</a>; the <a href='#SkPaint_glyphsToUnichars_text'>text</a> values returned are equivalent to <a href='#SkPaint_kUTF32_TextEncoding'>kUTF32 TextEncoding</a>.

Only supported on platforms that use FreeType as the <a href='undocumented#Engine'>Font Engine</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_glyphsToUnichars_glyphs'><code><strong>glyphs</strong></code></a></td>
    <td>array of indices into font</td>
  </tr>
  <tr>    <td><a name='SkPaint_glyphsToUnichars_count'><code><strong>count</strong></code></a></td>
    <td>length of glyph array</td>
  </tr>
  <tr>    <td><a name='SkPaint_glyphsToUnichars_text'><code><strong>text</strong></code></a></td>
    <td>storage for character codes, one per glyph</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c12686b0b3e0a87d0a248bbfc57e9492"><div>Convert UTF-8 <a href='#SkPaint_glyphsToUnichars_text'>text</a> to <a href='#SkPaint_glyphsToUnichars_glyphs'>glyphs</a>; then convert <a href='#SkPaint_glyphsToUnichars_glyphs'>glyphs</a> to Unichar code points.
</div></fiddle-embed></div>

---

## <a name='Measure_Text'>Measure Text</a>

<a name='SkPaint_measureText'></a>
## measureText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_measureText'>measureText</a>(const void* text, size_t length, <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds) const
</pre>

Returns the advance width of <a href='#SkPaint_measureText_text'>text</a> if <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> is clear,
and the height of <a href='#SkPaint_measureText_text'>text</a> if <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> is set.
The advance is the normal distance to move before drawing additional <a href='#SkPaint_measureText_text'>text</a>.
Uses <a href='#Text_Encoding'>Text Encoding</a> to decode <a href='#SkPaint_measureText_text'>text</a>, <a href='undocumented#Typeface'>Typeface</a> to get the font metrics,
and <a href='#Text_Size'>Text Size</a>, <a href='#Text_Scale_X'>Text Scale X</a>, <a href='#Text_Skew_X'>Text Skew X</a>, <a href='#Stroke_Width'>Stroke Width</a>, and
<a href='undocumented#Path_Effect'>Path Effect</a> to scale the metrics and <a href='#SkPaint_measureText_bounds'>bounds</a>.
Returns the bounding box of <a href='#SkPaint_measureText_text'>text</a> if <a href='#SkPaint_measureText_bounds'>bounds</a> is not nullptr.
The bounding box is computed as if the <a href='#SkPaint_measureText_text'>text</a> was drawn at the origin.

### Parameters

<table>  <tr>    <td><a name='SkPaint_measureText_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices to be measured</td>
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

---

<a name='SkPaint_measureText_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPaint_measureText'>measureText</a>(const void* text, size_t length) const
</pre>

Returns the advance width of <a href='#SkPaint_measureText_2_text'>text</a> if <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> is clear,
and the height of <a href='#SkPaint_measureText_2_text'>text</a> if <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> is set.
The advance is the normal distance to move before drawing additional <a href='#SkPaint_measureText_2_text'>text</a>.
Uses <a href='#Text_Encoding'>Text Encoding</a> to decode <a href='#SkPaint_measureText_2_text'>text</a>, <a href='undocumented#Typeface'>Typeface</a> to get the font metrics,
and <a href='#Text_Size'>Text Size</a> to scale the metrics.
Does not scale the advance or bounds by <a href='#Fake_Bold'>Fake Bold</a> or <a href='undocumented#Path_Effect'>Path Effect</a>.

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
size_t <a href='#SkPaint_breakText'>breakText</a>(const void* text, size_t length, <a href='undocumented#SkScalar'>SkScalar</a> maxWidth,
                 <a href='undocumented#SkScalar'>SkScalar</a>* measuredWidth = nullptr) const
</pre>

Returns the bytes of <a href='#SkPaint_breakText_text'>text</a> that fit within <a href='#SkPaint_breakText_maxWidth'>maxWidth</a>.
If <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> is clear, the <a href='#SkPaint_breakText_text'>text</a> fragment fits if its advance width is less than or
equal to <a href='#SkPaint_breakText_maxWidth'>maxWidth</a>.
If <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> is set, the <a href='#SkPaint_breakText_text'>text</a> fragment fits if its advance height is less than or
equal to <a href='#SkPaint_breakText_maxWidth'>maxWidth</a>.
Measures only while the advance is less than or equal to <a href='#SkPaint_breakText_maxWidth'>maxWidth</a>.
Returns the advance or the <a href='#SkPaint_breakText_text'>text</a> fragment in <a href='#SkPaint_breakText_measuredWidth'>measuredWidth</a> if it not nullptr.
Uses <a href='#Text_Encoding'>Text Encoding</a> to decode <a href='#SkPaint_breakText_text'>text</a>, <a href='undocumented#Typeface'>Typeface</a> to get the font metrics,
and <a href='#Text_Size'>Text Size</a> to scale the metrics.
Does not scale the advance or bounds by <a href='#Fake_Bold'>Fake Bold</a> or <a href='undocumented#Path_Effect'>Path Effect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_breakText_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices to be measured</td>
  </tr>
  <tr>    <td><a name='SkPaint_breakText_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_breakText_text'>text</a> to measure</td>
  </tr>
  <tr>    <td><a name='SkPaint_breakText_maxWidth'><code><strong>maxWidth</strong></code></a></td>
    <td>advance limit; <a href='#SkPaint_breakText_text'>text</a> is measured while advance is less than <a href='#SkPaint_breakText_maxWidth'>maxWidth</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_breakText_measuredWidth'><code><strong>measuredWidth</strong></code></a></td>
    <td>returns the width of the <a href='#SkPaint_breakText_text'>text</a> less than or equal to <a href='#SkPaint_breakText_maxWidth'>maxWidth</a></td>
  </tr>
</table>

### Return Value

bytes of <a href='#SkPaint_breakText_text'>text</a> that fit, always less than or equal to <a href='#SkPaint_breakText_length'>length</a>

### Example

<div><fiddle-embed name="fd0033470ccbd5c7059670fdbf96cffc"><div><a href='undocumented#Line'>Line</a> under "" shows desired width, shorter than available characters.
<a href='undocumented#Line'>Line</a> under "" shows measured width after breaking <a href='#SkPaint_breakText_text'>text</a>.
</div></fiddle-embed></div>

---

<a name='SkPaint_getTextWidths'></a>
## getTextWidths

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getTextWidths'>getTextWidths</a>(const void* text, size_t byteLength, <a href='undocumented#SkScalar'>SkScalar</a> widths[], <a href='SkRect_Reference#SkRect'>SkRect</a> bounds[] = nullptr) const
</pre>

Retrieves the advance and <a href='#SkPaint_getTextWidths_bounds'>bounds</a> for each glyph in <a href='#SkPaint_getTextWidths_text'>text</a>, and returns
the glyph count in <a href='#SkPaint_getTextWidths_text'>text</a>.
Both <a href='#SkPaint_getTextWidths_widths'>widths</a> and <a href='#SkPaint_getTextWidths_bounds'>bounds</a> may be nullptr.
If <a href='#SkPaint_getTextWidths_widths'>widths</a> is not nullptr, <a href='#SkPaint_getTextWidths_widths'>widths</a> must be an array of glyph count entries.
if <a href='#SkPaint_getTextWidths_bounds'>bounds</a> is not nullptr, <a href='#SkPaint_getTextWidths_bounds'>bounds</a> must be an array of glyph count entries.
If <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> is clear, <a href='#SkPaint_getTextWidths_widths'>widths</a> returns the horizontal advance.
If <a href='#SkPaint_kVerticalText_Flag'>kVerticalText Flag</a> is set, <a href='#SkPaint_getTextWidths_widths'>widths</a> returns the vertical advance.
Uses <a href='#Text_Encoding'>Text Encoding</a> to decode <a href='#SkPaint_getTextWidths_text'>text</a>, <a href='undocumented#Typeface'>Typeface</a> to get the font metrics,
and <a href='#Text_Size'>Text Size</a> to scale the <a href='#SkPaint_getTextWidths_widths'>widths</a> and <a href='#SkPaint_getTextWidths_bounds'>bounds</a>.
Does not scale the advance by <a href='#Fake_Bold'>Fake Bold</a> or <a href='undocumented#Path_Effect'>Path Effect</a>.
Does include <a href='#Fake_Bold'>Fake Bold</a> and <a href='undocumented#Path_Effect'>Path Effect</a> in the <a href='#SkPaint_getTextWidths_bounds'>bounds</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextWidths_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices to be measured</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextWidths_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getTextWidths_text'>text</a> to measure</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextWidths_widths'><code><strong>widths</strong></code></a></td>
    <td>returns <a href='#SkPaint_getTextWidths_text'>text</a> advances for each glyph; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextWidths_bounds'><code><strong>bounds</strong></code></a></td>
    <td>returns <a href='#SkPaint_getTextWidths_bounds'>bounds</a> for each glyph relative to (0, 0); may be nullptr</td>
  </tr>
</table>

### Return Value

glyph count in <a href='#SkPaint_getTextWidths_text'>text</a>

### Example

<div><fiddle-embed name="6b9e101f49e9c2c28755c5bdcef64dfb"><div>Bounds of <a href='undocumented#Glyph'>Glyphs</a> increase for stroked <a href='#SkPaint_getTextWidths_text'>text</a>, but <a href='#SkPaint_getTextWidths_text'>text</a> advance remains the same.
The underlines show the <a href='#SkPaint_getTextWidths_text'>text</a> advance, spaced to keep them distinct.
</div></fiddle-embed></div>

---

## <a name='Text_Path'>Text Path</a>

<a href='#Text_Path'>Text Path</a> describes the geometry of <a href='undocumented#Glyph'>Glyphs</a> used to draw text.

<a name='SkPaint_getTextPath'></a>
## getTextPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_getTextPath'>getTextPath</a>(const void* text, size_t length, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, <a href='SkPath_Reference#SkPath'>SkPath</a>* path) const
</pre>

Returns the geometry as <a href='SkPath_Reference#Path'>Path</a> equivalent to the drawn <a href='#SkPaint_getTextPath_text'>text</a>.
Uses <a href='#Text_Encoding'>Text Encoding</a> to decode <a href='#SkPaint_getTextPath_text'>text</a>, <a href='undocumented#Typeface'>Typeface</a> to get the glyph paths,
and <a href='#Text_Size'>Text Size</a>, <a href='#Fake_Bold'>Fake Bold</a>, and <a href='undocumented#Path_Effect'>Path Effect</a> to scale and modify the glyph paths.
All of the glyph paths are stored in <a href='#SkPaint_getTextPath_path'>path</a>.
Uses <a href='#SkPaint_getTextPath_x'>x</a>, <a href='#SkPaint_getTextPath_y'>y</a>, and <a href='#Text_Align'>Text Align</a> to position <a href='#SkPaint_getTextPath_path'>path</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextPath_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextPath_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getTextPath_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextPath_x'><code><strong>x</strong></code></a></td>
    <td><a href='#SkPaint_getTextPath_x'>x</a>-axis value of the origin of the <a href='#SkPaint_getTextPath_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextPath_y'><code><strong>y</strong></code></a></td>
    <td><a href='#SkPaint_getTextPath_y'>y</a>-axis value of the origin of the <a href='#SkPaint_getTextPath_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextPath_path'><code><strong>path</strong></code></a></td>
    <td>geometry of the <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7c9e6a399f898d68026c1f0865e6f73e"><div><a href='undocumented#Text'>Text</a> is added to <a href='SkPath_Reference#Path'>Path</a>, offset, and subtracted from <a href='SkPath_Reference#Path'>Path</a>, then added at
the offset location. The result is rendered with one draw call.
</div></fiddle-embed></div>

---

<a name='SkPaint_getPosTextPath'></a>
## getPosTextPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPaint_getPosTextPath'>getPosTextPath</a>(const void* text, size_t length, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pos[], <a href='SkPath_Reference#SkPath'>SkPath</a>* path) const
</pre>

Returns the geometry as <a href='SkPath_Reference#Path'>Path</a> equivalent to the drawn <a href='#SkPaint_getPosTextPath_text'>text</a>.
Uses <a href='#Text_Encoding'>Text Encoding</a> to decode <a href='#SkPaint_getPosTextPath_text'>text</a>, <a href='undocumented#Typeface'>Typeface</a> to get the glyph paths,
and <a href='#Text_Size'>Text Size</a>, <a href='#Fake_Bold'>Fake Bold</a>, and <a href='undocumented#Path_Effect'>Path Effect</a> to scale and modify the glyph paths.
All of the glyph paths are stored in <a href='#SkPaint_getPosTextPath_path'>path</a>.
Uses <a href='#SkPaint_getPosTextPath_pos'>pos</a> array and <a href='#Text_Align'>Text Align</a> to position <a href='#SkPaint_getPosTextPath_path'>path</a>.
<a href='#SkPaint_getPosTextPath_pos'>pos</a> contains a position for each glyph.

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

<div><fiddle-embed name="7f27c93472aa99a7542fb3493076f072"><div>Simplifies three <a href='undocumented#Glyph'>Glyphs</a> to eliminate overlaps, and strokes the result.
</div></fiddle-embed></div>

---

## <a name='Text_Intercepts'>Text Intercepts</a>

<a href='#Text_Intercepts'>Text Intercepts</a> describe the intersection of drawn text <a href='undocumented#Glyph'>Glyphs</a> with a pair
of lines parallel to the text advance. <a href='#Text_Intercepts'>Text Intercepts</a> permits creating a
underline that skips <a href='undocumented#Descenders'>Descenders</a>.

<a name='SkPaint_getTextIntercepts'></a>
## getTextIntercepts

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getTextIntercepts'>getTextIntercepts</a>(const void* text, size_t length, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y,
                      const <a href='undocumented#SkScalar'>SkScalar</a> bounds[2], <a href='undocumented#SkScalar'>SkScalar</a>* intervals) const
</pre>

Returns the number of <a href='#SkPaint_getTextIntercepts_intervals'>intervals</a> that intersect <a href='#SkPaint_getTextIntercepts_bounds'>bounds</a>.
<a href='#SkPaint_getTextIntercepts_bounds'>bounds</a> describes a pair of lines parallel to the <a href='#SkPaint_getTextIntercepts_text'>text</a> advance.
The return count is zero or a multiple of two, and is at most twice the number of <a href='undocumented#Glyph'>Glyphs</a> in
the string.
Uses <a href='#Text_Encoding'>Text Encoding</a> to decode <a href='#SkPaint_getTextIntercepts_text'>text</a>, <a href='undocumented#Typeface'>Typeface</a> to get the glyph paths,
and <a href='#Text_Size'>Text Size</a>, <a href='#Fake_Bold'>Fake Bold</a>, and <a href='undocumented#Path_Effect'>Path Effect</a> to scale and modify the glyph paths.
Uses <a href='#SkPaint_getTextIntercepts_x'>x</a>, <a href='#SkPaint_getTextIntercepts_y'>y</a>, and <a href='#Text_Align'>Text Align</a> to position <a href='#SkPaint_getTextIntercepts_intervals'>intervals</a>.

Pass nullptr for <a href='#SkPaint_getTextIntercepts_intervals'>intervals</a> to determine the size of the interval array.

<a href='#SkPaint_getTextIntercepts_intervals'>intervals</a> are cached to improve performance for multiple calls.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextIntercepts_text'><code><strong>text</strong></code></a></td>
    <td>character codes or glyph indices</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkPaint_getTextIntercepts_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_x'><code><strong>x</strong></code></a></td>
    <td><a href='#SkPaint_getTextIntercepts_x'>x</a>-axis value of the origin of the <a href='#SkPaint_getTextIntercepts_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_y'><code><strong>y</strong></code></a></td>
    <td><a href='#SkPaint_getTextIntercepts_y'>y</a>-axis value of the origin of the <a href='#SkPaint_getTextIntercepts_text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_bounds'><code><strong>bounds</strong></code></a></td>
    <td>lower and upper line parallel to the advance</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextIntercepts_intervals'><code><strong>intervals</strong></code></a></td>
    <td>returned intersections; may be nullptr</td>
  </tr>
</table>

### Return Value

number of intersections; may be zero

### Example

<div><fiddle-embed name="2a0b80ed20d193c688085b79deb5bdc9"><div>Underline uses intercepts to draw on either side of the glyph Descender.
</div></fiddle-embed></div>

---

<a name='SkPaint_getPosTextIntercepts'></a>
## getPosTextIntercepts

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getPosTextIntercepts'>getPosTextIntercepts</a>(const void* text, size_t length, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pos[],
                         const <a href='undocumented#SkScalar'>SkScalar</a> bounds[2], <a href='undocumented#SkScalar'>SkScalar</a>* intervals) const
</pre>

Returns the number of <a href='#SkPaint_getPosTextIntercepts_intervals'>intervals</a> that intersect <a href='#SkPaint_getPosTextIntercepts_bounds'>bounds</a>.
<a href='#SkPaint_getPosTextIntercepts_bounds'>bounds</a> describes a pair of lines parallel to the <a href='#SkPaint_getPosTextIntercepts_text'>text</a> advance.
The return count is zero or a multiple of two, and is at most twice the number of <a href='undocumented#Glyph'>Glyphs</a> in
the string.
Uses <a href='#Text_Encoding'>Text Encoding</a> to decode <a href='#SkPaint_getPosTextIntercepts_text'>text</a>, <a href='undocumented#Typeface'>Typeface</a> to get the glyph paths,
and <a href='#Text_Size'>Text Size</a>, <a href='#Fake_Bold'>Fake Bold</a>, and <a href='undocumented#Path_Effect'>Path Effect</a> to scale and modify the glyph paths.
Uses <a href='#SkPaint_getPosTextIntercepts_pos'>pos</a> array and <a href='#Text_Align'>Text Align</a> to position <a href='#SkPaint_getPosTextIntercepts_intervals'>intervals</a>.

Pass nullptr for <a href='#SkPaint_getPosTextIntercepts_intervals'>intervals</a> to determine the size of the interval array.

<a href='#SkPaint_getPosTextIntercepts_intervals'>intervals</a> are cached to improve performance for multiple calls.

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
    <td>returned intersections; may be nullptr</td>
  </tr>
</table>

### Return Value

number of intersections; may be zero

### Example

<div><fiddle-embed name="98b2dfc552d0540a7c041fe7a2839bd7"><div><a href='undocumented#Text'>Text</a> intercepts draw on either side of, but not inside, <a href='undocumented#Glyph'>Glyphs</a> in a run.
</div></fiddle-embed></div>

---

<a name='SkPaint_getPosTextHIntercepts'></a>
## getPosTextHIntercepts

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getPosTextHIntercepts'>getPosTextHIntercepts</a>(const void* text, size_t length, const <a href='undocumented#SkScalar'>SkScalar</a> xpos[], <a href='undocumented#SkScalar'>SkScalar</a> constY,
                          const <a href='undocumented#SkScalar'>SkScalar</a> bounds[2], <a href='undocumented#SkScalar'>SkScalar</a>* intervals) const
</pre>

Returns the number of <a href='#SkPaint_getPosTextHIntercepts_intervals'>intervals</a> that intersect <a href='#SkPaint_getPosTextHIntercepts_bounds'>bounds</a>.
<a href='#SkPaint_getPosTextHIntercepts_bounds'>bounds</a> describes a pair of lines parallel to the <a href='#SkPaint_getPosTextHIntercepts_text'>text</a> advance.
The return count is zero or a multiple of two, and is at most twice the number of <a href='undocumented#Glyph'>Glyphs</a> in
the string.
Uses <a href='#Text_Encoding'>Text Encoding</a> to decode <a href='#SkPaint_getPosTextHIntercepts_text'>text</a>, <a href='undocumented#Typeface'>Typeface</a> to get the glyph paths,
and <a href='#Text_Size'>Text Size</a>, <a href='#Fake_Bold'>Fake Bold</a>, and <a href='undocumented#Path_Effect'>Path Effect</a> to scale and modify the glyph paths.
Uses <a href='#SkPaint_getPosTextHIntercepts_xpos'>xpos</a> array, <a href='#SkPaint_getPosTextHIntercepts_constY'>constY</a>, and <a href='#Text_Align'>Text Align</a> to position <a href='#SkPaint_getPosTextHIntercepts_intervals'>intervals</a>.

Pass nullptr for <a href='#SkPaint_getPosTextHIntercepts_intervals'>intervals</a> to determine the size of the interval array.

<a href='#SkPaint_getPosTextHIntercepts_intervals'>intervals</a> are cached to improve performance for multiple calls.

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
    <td>returned intersections; may be nullptr</td>
  </tr>
</table>

### Return Value

number of intersections; may be zero

### Example

<div><fiddle-embed name="dc9851c43acc3716aca8c9a4d40d452d"><div><a href='undocumented#Text'>Text</a> intercepts do not take stroke thickness into consideration.
</div></fiddle-embed></div>

---

<a name='SkPaint_getTextBlobIntercepts'></a>
## getTextBlobIntercepts

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPaint_getTextBlobIntercepts'>getTextBlobIntercepts</a>(const <a href='undocumented#SkTextBlob'>SkTextBlob</a>* blob, const <a href='undocumented#SkScalar'>SkScalar</a> bounds[2], <a href='undocumented#SkScalar'>SkScalar</a>* intervals) const
</pre>

Returns the number of <a href='#SkPaint_getTextBlobIntercepts_intervals'>intervals</a> that intersect <a href='#SkPaint_getTextBlobIntercepts_bounds'>bounds</a>.
<a href='#SkPaint_getTextBlobIntercepts_bounds'>bounds</a> describes a pair of lines parallel to the text advance.
The return count is zero or a multiple of two, and is at most twice the number of <a href='undocumented#Glyph'>Glyphs</a> in
the string.
Uses <a href='undocumented#Typeface'>Typeface</a> to get the glyph paths,
and <a href='#Text_Size'>Text Size</a>, <a href='#Fake_Bold'>Fake Bold</a>, and <a href='undocumented#Path_Effect'>Path Effect</a> to scale and modify the glyph paths.
Uses run array and <a href='#Text_Align'>Text Align</a> to position <a href='#SkPaint_getTextBlobIntercepts_intervals'>intervals</a>.

<a href='#Text_Encoding'>Text Encoding</a> must be set to <a href='#SkPaint_kGlyphID_TextEncoding'>SkPaint::kGlyphID TextEncoding</a>.

Pass nullptr for <a href='#SkPaint_getTextBlobIntercepts_intervals'>intervals</a> to determine the size of the interval array.

<a href='#SkPaint_getTextBlobIntercepts_intervals'>intervals</a> are cached to improve performance for multiple calls.

### Parameters

<table>  <tr>    <td><a name='SkPaint_getTextBlobIntercepts_blob'><code><strong>blob</strong></code></a></td>
    <td><a href='undocumented#Glyph'>Glyphs</a>, positions, and text paint attributes</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextBlobIntercepts_bounds'><code><strong>bounds</strong></code></a></td>
    <td>lower and upper line parallel to the advance</td>
  </tr>
  <tr>    <td><a name='SkPaint_getTextBlobIntercepts_intervals'><code><strong>intervals</strong></code></a></td>
    <td>returned intersections; may be nullptr</td>
  </tr>
</table>

### Return Value

number of intersections; may be zero

### Example

<div><fiddle-embed name="f2229dd5c8e76f9e12fafe59b61353c8"></fiddle-embed></div>

---

<a name='SkPaint_nothingToDraw'></a>
## nothingToDraw

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_nothingToDraw'>nothingToDraw</a>() const
</pre>

Returns true if <a href='#Paint'>Paint</a> prevents all drawing;
otherwise, the <a href='#Paint'>Paint</a> may or may not allow drawing.

Returns true if, for example, <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> combined with <a href='SkColor_Reference#Alpha'>Color Alpha</a> computes a
new <a href='SkColor_Reference#Alpha'>Alpha</a> of zero.

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

<a href='#Fast_Bounds'>Fast Bounds</a> functions conservatively outset a drawing bounds by additional area
<a href='#Paint'>Paint</a> may draw to.

<a name='SkPaint_canComputeFastBounds'></a>
## canComputeFastBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPaint_canComputeFastBounds'>canComputeFastBounds</a>() const
</pre>

Returns true if <a href='#Paint'>Paint</a> does not include elements requiring extensive computation
to compute <a href='undocumented#Device'>Device</a> bounds of drawn geometry. For instance, <a href='#Paint'>Paint</a> with <a href='undocumented#Path_Effect'>Path Effect</a>
always returns false.

### Return Value

true if <a href='#Paint'>Paint</a> allows for fast computation of bounds

---

<a name='SkPaint_computeFastBounds'></a>
## computeFastBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkPaint_computeFastBounds'>computeFastBounds</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& orig, <a href='SkRect_Reference#SkRect'>SkRect</a>* storage) const
</pre>

Only call this if <a href='#SkPaint_canComputeFastBounds'>canComputeFastBounds</a> returned true. This takes a
raw rectangle (the raw bounds of a shape), and adjusts it for stylistic
effects in the paint (e.g. stroking). If needed, it uses the <a href='#SkPaint_computeFastBounds_storage'>storage</a>
parameter. It returns the adjusted bounds that can then be used
for <a href='SkCanvas_Reference#SkCanvas_quickReject'>SkCanvas::quickReject</a> tests.

The returned <a href='SkRect_Reference#Rect'>Rect</a> will either be <a href='#SkPaint_computeFastBounds_orig'>orig</a> or <a href='#SkPaint_computeFastBounds_storage'>storage</a>, thus the caller
should not rely on <a href='#SkPaint_computeFastBounds_storage'>storage</a> being set to the result, but should always
use the returned value. It is legal for <a href='#SkPaint_computeFastBounds_orig'>orig</a> and <a href='#SkPaint_computeFastBounds_storage'>storage</a> to be the same
<a href='SkRect_Reference#Rect'>Rect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPaint_computeFastBounds_orig'><code><strong>orig</strong></code></a></td>
    <td>geometry modified by <a href='#Paint'>Paint</a> when drawn</td>
  </tr>
  <tr>    <td><a name='SkPaint_computeFastBounds_storage'><code><strong>storage</strong></code></a></td>
    <td>computed bounds of geometry; may not be nullptr</td>
  </tr>
</table>

### Return Value

fast computed bounds

---

<a name='SkPaint_computeFastStrokeBounds'></a>
## computeFastStrokeBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkPaint_computeFastStrokeBounds'>computeFastStrokeBounds</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& orig, <a href='SkRect_Reference#SkRect'>SkRect</a>* storage) const
</pre>

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
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkPaint_doComputeFastBounds'>doComputeFastBounds</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& orig, <a href='SkRect_Reference#SkRect'>SkRect</a>* storage, <a href='#SkPaint_Style'>Style</a> style) const
</pre>

Computes the bounds, overriding the <a href='#Paint'>Paint</a> <a href='#SkPaint_Style'>Style</a>. This can be used to
account for additional width required by stroking <a href='#SkPaint_doComputeFastBounds_orig'>orig</a>, without
altering <a href='#SkPaint_Style'>Style</a> set to fill.

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


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_containsText'>containsText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if all text corresponds to <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_countText'>countText</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns number of <a href='undocumented#Glyph'>Glyphs</a> in text</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_glyphsToUnichars'>glyphsToUnichars</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>converts <a href='undocumented#Glyph'>Glyphs</a> into text</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_nothingToDraw'>nothingToDraw</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#Paint'>Paint</a> prevents all drawing</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPaint_textToGlyphs'>textToGlyphs</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>converts text into glyph indices</td>
  </tr>
</table>

