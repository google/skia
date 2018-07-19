SkIRect Reference
===

# <a name='IRect'>IRect</a>

# <a name='SkIRect'>Struct SkIRect</a>
<a href='#SkIRect'>SkIRect</a> holds four 32-bit integer coordinates describing the upper and
lower bounds of a rectangle. <a href='#SkIRect'>SkIRect</a> may be created from outer bounds or
from position, width, and height. <a href='#SkIRect'>SkIRect</a> describes an area; if its right
is less than or equal to its left, or if its bottom is less than or equal to
its top, it is considered empty.

## Overview

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constructor'>Constructors</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>functions that construct <a href='#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member_Function'>Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>global and class member functions</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member'>Members</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>member values</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Operator'>Operators</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>operator overloading methods</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Related_Function'>Related Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>similar member functions grouped together</td>
  </tr>
</table>


## <a name='Related_Function'>Related Function</a>


SkIRect global, <code>struct</code>, and <code>class</code> related member functions share a topic.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Inset_Outset_Offset'>Inset Outset Offset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>moves sides</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Intersection'>Intersection</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to shared bounds</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Join'>Join</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to union of bounds</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Property'>Property</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>member values, center, validity</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Set'>Set</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>replaces all values</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Sorting'>Sorting</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>orders sides</td>
  </tr>
</table>

## <a name='Member_Function'>Member Function</a>


SkIRect member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_EmptyIRect'>EmptyIRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns immutable bounds of (0, 0, 0, 0)</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_Intersects'>Intersects</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if areas overlap</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_IntersectsNoEmptyCheck'>IntersectsNoEmptyCheck</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if areas overlap skips empty check</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_MakeEmpty'>MakeEmpty</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns bounds of (0, 0, 0, 0)</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_MakeLTRB'>MakeLTRB</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from int left, top, right, bottom</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_MakeSize'>MakeSize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from <a href='undocumented#ISize'>ISize</a> returning (0, 0, width, height)</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_MakeWH'>MakeWH</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from int input returning (0, 0, width, height)</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_MakeXYWH'>MakeXYWH</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from int input returning (x, y, width, height)</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_adjust'>adjust</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>moves the sides independently relative to their original locations</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_bottom'>bottom</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns larger bounds in y, if sorted</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_contains'>contains</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='SkIPoint_Reference#IPoint'>IPoint</a> (x, y) is equal or inside</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if contains unsorted <a href='#IRect'>IRect</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_height'>height</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns span in y</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_height64'>height64</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns span in y as int64_t</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_inset'>inset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>moves the sides symmetrically about the center</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_intersect'>intersect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to shared area; returns true if not empty</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to shared area; returns true if not empty skips empty check</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_isEmpty'>isEmpty</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if width or height are zero or negative or they exceed int32_t</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_isEmpty64'>isEmpty64</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if width or height are zero or negative</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_join'>join</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to union of bounds</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_left'>left</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns smaller bounds in x, if sorted</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_makeInset'>makeInset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from sides moved symmetrically about the center</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_makeOffset'>makeOffset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from translated sides</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_makeOutset'>makeOutset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from sides moved symmetrically about the center</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_makeSorted'>makeSorted</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs <a href='#IRect'>IRect</a>, ordering sides from smaller to larger</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_offset'>offset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>translates sides without changing width and height</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_offsetTo'>offsetTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>translates to (x, y) without changing width and height</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_outset'>outset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>moves the sides symmetrically about the center</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_right'>right</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns larger bounds in x, if sorted</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_set'>set</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to (left, top, right, bottom)</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_setEmpty'>setEmpty</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to (0, 0, 0, 0)</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_setLTRB'>setLTRB</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to <a href='undocumented#SkScalar'>SkScalar</a> input (left, top, right, bottom)</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_setXYWH'>setXYWH</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to (x, y, width, height)</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_size'>size</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#ISize'>ISize</a> (width, height)</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_sort'>sort</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>orders sides from smaller to larger</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_top'>top</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns smaller bounds in y, if sorted</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_width'>width</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns span in x</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_width64'>width64</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns span in y as int64_t</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_x'>x</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns bounds left</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_y'>y</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns bounds top</td>
  </tr>
</table>

## <a name='Member'>Member</a>



### Members

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Name</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#fLeft'><code>fLeft</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>smaller x-axis bounds</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#fTop'><code>fTop</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>smaller y-axis bounds</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#fRight'><code>fRight</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>larger x-axis bounds</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#fBottom'><code>fBottom</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>larger y-axis bounds</td>
  </tr>
</table>


SkIRect members may be read and written directly without using a member function.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_fBottom'>fBottom</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>larger y-axis bounds</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_fLeft'>fLeft</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>smaller x-axis bounds</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_fRight'>fRight</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>larger x-axis bounds</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_fTop'>fTop</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>smaller y-axis bounds</td>
  </tr>
</table>

### Members

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Name</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIRect_fLeft'><code>fLeft</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value. The smaller of the horizontal values when sorted.
When equal to or greater than <a href='#SkIRect_fRight'>fRight</a>, <a href='#IRect'>IRect</a> is empty.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIRect_fTop'><code>fTop</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value. The smaller of the horizontal values when sorted.
When equal to or greater than <a href='#SkIRect_fBottom'>fBottom</a>, <a href='#IRect'>IRect</a> is empty.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIRect_fRight'><code>fRight</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value. The larger of the vertical values when sorted.
When equal to or less than <a href='#SkIRect_fLeft'>fLeft</a>, <a href='#IRect'>IRect</a> is empty.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIRect_fBottom'><code>fBottom</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value. The larger of the vertical values when sorted.
When equal to or less than <a href='#SkIRect_fTop'>fTop</a>, <a href='#IRect'>IRect</a> is empty.
</td>
  </tr>


## <a name='Constructor'>Constructor</a>


SkIRect can be constructed or initialized by these functions, including C++ class constructors.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_EmptyIRect'>EmptyIRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns immutable bounds of (0, 0, 0, 0)</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_MakeEmpty'>MakeEmpty</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns bounds of (0, 0, 0, 0)</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_MakeLTRB'>MakeLTRB</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from int left, top, right, bottom</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_MakeSize'>MakeSize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from <a href='undocumented#ISize'>ISize</a> returning (0, 0, width, height)</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_MakeWH'>MakeWH</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from int input returning (0, 0, width, height)</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_MakeXYWH'>MakeXYWH</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from int input returning (x, y, width, height)</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_makeInset'>makeInset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from sides moved symmetrically about the center</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_makeOffset'>makeOffset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from translated sides</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_makeOutset'>makeOutset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from sides moved symmetrically about the center</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_makeSorted'>makeSorted</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs <a href='#IRect'>IRect</a>, ordering sides from smaller to larger</td>
  </tr>
</table>

<a name='SkIRect_MakeEmpty'></a>
## MakeEmpty

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='#SkIRect'>SkIRect</a> SK_WARN_UNUSED_RESULT <a href='#SkIRect_MakeEmpty'>MakeEmpty</a>()
</pre>

Returns constructed <a href='#IRect'>IRect</a> set to (0, 0, 0, 0).
Many other rectangles are empty; if left is equal to or greater than right,
or if top is equal to or greater than bottom. Setting all members to zero
is a convenience, but does not designate a special empty rectangle.

### Return Value

bounds (0, 0, 0, 0)

### Example

<div><fiddle-embed name="0ade3971c1d2616564992e286966ec8a">

#### Example Output

~~~~
MakeEmpty isEmpty: true
offset rect isEmpty: true
inset rect isEmpty: true
outset rect isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_EmptyIRect'>EmptyIRect</a> <a href='#SkIRect_isEmpty'>isEmpty</a> <a href='#SkIRect_setEmpty'>setEmpty</a> <a href='SkRect_Reference#SkRect_MakeEmpty'>SkRect::MakeEmpty</a>

---

<a name='SkIRect_MakeWH'></a>
## MakeWH

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='#SkIRect'>SkIRect</a> SK_WARN_UNUSED_RESULT <a href='#SkIRect_MakeWH'>MakeWH</a>(int32_t w, int32_t h)
</pre>

Returns constructed <a href='#IRect'>IRect</a> set to (0, 0, <a href='#SkIRect_MakeWH_w'>w</a>, <a href='#SkIRect_MakeWH_h'>h</a>). Does not validate input; <a href='#SkIRect_MakeWH_w'>w</a> or <a href='#SkIRect_MakeWH_h'>h</a>
may be negative.

### Parameters

<table>  <tr>    <td><a name='SkIRect_MakeWH_w'><code><strong>w</strong></code></a></td>
    <td>width of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeWH_h'><code><strong>h</strong></code></a></td>
    <td>height of constructed <a href='#IRect'>IRect</a></td>
  </tr>
</table>

### Return Value

bounds (0, 0, <a href='#SkIRect_MakeWH_w'>w</a>, <a href='#SkIRect_MakeWH_h'>h</a>)

### Example

<div><fiddle-embed name="e36827a1a6ae2b1c26e7a8a08f325a07">

#### Example Output

~~~~
all equal
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeSize'>MakeSize</a> <a href='#SkIRect_MakeXYWH'>MakeXYWH</a> <a href='SkRect_Reference#SkRect_MakeWH'>SkRect::MakeWH</a> <a href='SkRect_Reference#SkRect_MakeIWH'>SkRect::MakeIWH</a>

---

<a name='SkIRect_MakeSize'></a>
## MakeSize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='#SkIRect'>SkIRect</a> SK_WARN_UNUSED_RESULT <a href='#SkIRect_MakeSize'>MakeSize</a>(const <a href='undocumented#SkISize'>SkISize</a>& size)
</pre>

Returns constructed <a href='#IRect'>IRect</a> set to (0, 0, size.<a href='#SkIRect_width'>width</a>, size.<a href='#SkIRect_height'>height</a>).
Does not validate input; size.<a href='#SkIRect_width'>width</a> or size.<a href='#SkIRect_height'>height</a> may be negative.

### Parameters

<table>  <tr>    <td><a name='SkIRect_MakeSize_size'><code><strong>size</strong></code></a></td>
    <td>values for <a href='#IRect'>IRect</a> width and height</td>
  </tr>
</table>

### Return Value

bounds (0, 0, size.<a href='#SkIRect_width'>width</a>, size.<a href='#SkIRect_height'>height</a>)

### Example

<div><fiddle-embed name="c6586ff8d24869c780169b0d19c75df6">

#### Example Output

~~~~
round width: 26  height: 36
floor width: 25  height: 35
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeWH'>MakeWH</a> <a href='#SkIRect_MakeXYWH'>MakeXYWH</a> <a href='SkRect_Reference#SkRect_Make'>SkRect::Make</a><sup><a href='SkRect_Reference#SkRect_Make_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_MakeIWH'>SkRect::MakeIWH</a>

---

<a name='SkIRect_MakeLTRB'></a>
## MakeLTRB

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='#SkIRect'>SkIRect</a> SK_WARN_UNUSED_RESULT <a href='#SkIRect_MakeLTRB'>MakeLTRB</a>(int32_t l, int32_t t, int32_t r, int32_t b)
</pre>

Returns constructed <a href='#IRect'>IRect</a> set to (<a href='#SkIRect_MakeLTRB_l'>l</a>, <a href='#SkIRect_MakeLTRB_t'>t</a>, <a href='#SkIRect_MakeLTRB_r'>r</a>, <a href='#SkIRect_MakeLTRB_b'>b</a>). Does not sort input; <a href='#IRect'>IRect</a> may
result in <a href='#SkIRect_fLeft'>fLeft</a> greater than <a href='#SkIRect_fRight'>fRight</a>, or <a href='#SkIRect_fTop'>fTop</a> greater than <a href='#SkIRect_fBottom'>fBottom</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_MakeLTRB_l'><code><strong>l</strong></code></a></td>
    <td>integer stored in <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeLTRB_t'><code><strong>t</strong></code></a></td>
    <td>integer stored in <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeLTRB_r'><code><strong>r</strong></code></a></td>
    <td>integer stored in <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeLTRB_b'><code><strong>b</strong></code></a></td>
    <td>integer stored in <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

bounds (<a href='#SkIRect_MakeLTRB_l'>l</a>, <a href='#SkIRect_MakeLTRB_t'>t</a>, <a href='#SkIRect_MakeLTRB_r'>r</a>, <a href='#SkIRect_MakeLTRB_b'>b</a>)

### Example

<div><fiddle-embed name="ec1473b700c594f2df9749a12a06b89b">

#### Example Output

~~~~
rect: 5, 35, 15, 25  isEmpty: true
rect: 5, 25, 15, 35  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeXYWH'>MakeXYWH</a> <a href='SkRect_Reference#SkRect_MakeLTRB'>SkRect::MakeLTRB</a>

---

<a name='SkIRect_MakeXYWH'></a>
## MakeXYWH

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='#SkIRect'>SkIRect</a> SK_WARN_UNUSED_RESULT <a href='#SkIRect_MakeXYWH'>MakeXYWH</a>(int32_t x, int32_t y, int32_t w, int32_t h)
</pre>

Returns constructed <a href='#IRect'>IRect</a> set to:
(x, y, x + <a href='#SkIRect_MakeXYWH_w'>w</a>, y + <a href='#SkIRect_MakeXYWH_h'>h</a>)
.
Does not validate input;
<a href='#SkIRect_MakeXYWH_w'>w</a> or <a href='#SkIRect_MakeXYWH_h'>h</a> may be negative.

### Parameters

<table>  <tr>    <td><a name='SkIRect_MakeXYWH_x'><code><strong>x</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeXYWH_y'><code><strong>y</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeXYWH_w'><code><strong>w</strong></code></a></td>
    <td>added to x and stored in <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeXYWH_h'><code><strong>h</strong></code></a></td>
    <td>added to y and stored in <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

bounds at (x, y) with width <a href='#SkIRect_MakeXYWH_w'>w</a> and height <a href='#SkIRect_MakeXYWH_h'>h</a>

### Example

<div><fiddle-embed name="598ee14350bd1d961cae6b36fa3df17e">

#### Example Output

~~~~
rect: 5, 35, -10, 60  isEmpty: true
rect: -10, 35, 5, 60  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeLTRB'>MakeLTRB</a> <a href='SkRect_Reference#SkRect_MakeXYWH'>SkRect::MakeXYWH</a>

---

## <a name='Property'>Property</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_bottom'>bottom</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns larger bounds in y, if sorted</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_height'>height</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns span in y</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_height64'>height64</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns span in y as int64_t</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_isEmpty'>isEmpty</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if width or height are zero or negative or they exceed int32_t</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_isEmpty64'>isEmpty64</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if width or height are zero or negative</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_left'>left</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns smaller bounds in x, if sorted</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_right'>right</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns larger bounds in x, if sorted</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_size'>size</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#ISize'>ISize</a> (width, height)</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_top'>top</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns smaller bounds in y, if sorted</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_width'>width</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns span in x</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_width64'>width64</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns span in y as int64_t</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_x'>x</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns bounds left</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_y'>y</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns bounds top</td>
  </tr>
</table>

<a name='SkIRect_left'></a>
## left

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_left'>left</a>() const
</pre>

Returns left edge of <a href='#IRect'>IRect</a>, if sorted.
Call <a href='#SkIRect_sort'>sort</a> to reverse <a href='#SkIRect_fLeft'>fLeft</a> and <a href='#SkIRect_fRight'>fRight</a> if needed.

### Return Value

<a href='#SkIRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="caf38ea4431bc246ba198f6a8c2b0f01">

#### Example Output

~~~~
unsorted.fLeft: 15 unsorted.left(): 15
sorted.fLeft: 10 sorted.left(): 10
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_x'>x</a> <a href='SkRect_Reference#SkRect_left'>SkRect::left()</a>

---

<a name='SkIRect_top'></a>
## top

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_top'>top</a>() const
</pre>

Returns top edge of <a href='#IRect'>IRect</a>, if sorted. Call <a href='#SkIRect_isEmpty'>isEmpty</a> to see if <a href='#IRect'>IRect</a> may be invalid,
and <a href='#SkIRect_sort'>sort</a> to reverse <a href='#SkIRect_fTop'>fTop</a> and <a href='#SkIRect_fBottom'>fBottom</a> if needed.

### Return Value

<a href='#SkIRect_fTop'>fTop</a>

### Example

<div><fiddle-embed name="cbec1ae6530e95943775450b1d11f19e">

#### Example Output

~~~~
unsorted.fTop: 25 unsorted.top(): 25
sorted.fTop: 5 sorted.top(): 5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_y'>y</a> <a href='SkRect_Reference#SkRect_top'>SkRect::top()</a>

---

<a name='SkIRect_right'></a>
## right

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_right'>right</a>() const
</pre>

Returns right edge of <a href='#IRect'>IRect</a>, if sorted.
Call <a href='#SkIRect_sort'>sort</a> to reverse <a href='#SkIRect_fLeft'>fLeft</a> and <a href='#SkIRect_fRight'>fRight</a> if needed.

### Return Value

<a href='#SkIRect_fRight'>fRight</a>

### Example

<div><fiddle-embed name="97e210976f1ee0387b30c70635cf114f">

#### Example Output

~~~~
unsorted.fRight: 10 unsorted.right(): 10
sorted.fRight: 15 sorted.right(): 15
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_fRight'>fRight</a> <a href='SkRect_Reference#SkRect_right'>SkRect::right()</a>

---

<a name='SkIRect_bottom'></a>
## bottom

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_bottom'>bottom</a>() const
</pre>

Returns bottom edge of <a href='#IRect'>IRect</a>, if sorted. Call <a href='#SkIRect_isEmpty'>isEmpty</a> to see if <a href='#IRect'>IRect</a> may be invalid,
and <a href='#SkIRect_sort'>sort</a> to reverse <a href='#SkIRect_fTop'>fTop</a> and <a href='#SkIRect_fBottom'>fBottom</a> if needed.

### Return Value

<a href='#SkIRect_fBottom'>fBottom</a>

### Example

<div><fiddle-embed name="c32afebc296054a181621648a184b8e3">

#### Example Output

~~~~
unsorted.fBottom: 5 unsorted.bottom(): 5
sorted.fBottom: 25 sorted.bottom(): 25
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_fBottom'>fBottom</a> <a href='SkRect_Reference#SkRect_bottom'>SkRect::bottom()</a>

---

<a name='SkIRect_x'></a>
## x

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_x'>x</a>() const
</pre>

Returns left edge of <a href='#IRect'>IRect</a>, if sorted. Call <a href='#SkIRect_isEmpty'>isEmpty</a> to see if <a href='#IRect'>IRect</a> may be invalid,
and <a href='#SkIRect_sort'>sort</a> to reverse <a href='#SkIRect_fLeft'>fLeft</a> and <a href='#SkIRect_fRight'>fRight</a> if needed.

### Return Value

<a href='#SkIRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="2a59cbfd1330a0db520d6ebb2b7c68c7">

#### Example Output

~~~~
unsorted.fLeft: 15 unsorted.x(): 15
sorted.fLeft: 10 sorted.x(): 10
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_left'>left</a> <a href='#SkIRect_y'>y</a> <a href='SkRect_Reference#SkRect_x'>SkRect::x()</a>

---

<a name='SkIRect_y'></a>
## y

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_y'>y</a>() const
</pre>

Returns top edge of <a href='#IRect'>IRect</a>, if sorted. Call <a href='#SkIRect_isEmpty'>isEmpty</a> to see if <a href='#IRect'>IRect</a> may be invalid,
and <a href='#SkIRect_sort'>sort</a> to reverse <a href='#SkIRect_fTop'>fTop</a> and <a href='#SkIRect_fBottom'>fBottom</a> if needed.

### Return Value

<a href='#SkIRect_fTop'>fTop</a>

### Example

<div><fiddle-embed name="6ea461e71f7fc80605818fbf493caa63">

#### Example Output

~~~~
unsorted.fTop: 25 unsorted.y(): 25
sorted.fTop: 5 sorted.y(): 5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_top'>top</a> <a href='#SkIRect_x'>x</a> <a href='SkRect_Reference#SkRect_y'>SkRect::y()</a>

---

<a name='SkIRect_width'></a>
## width

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_width'>width</a>() const
</pre>

Returns span on the x-axis. This does not check if <a href='#IRect'>IRect</a> is sorted, or if
result fits in 32-bit signed integer; result may be negative.

### Return Value

<a href='#SkIRect_fRight'>fRight</a> minus <a href='#SkIRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="4acfbe051805940210c8916a94794142">

#### Example Output

~~~~
unsorted width: -5
large width: -5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_height'>height</a> <a href='#SkIRect_width64'>width64</a> <a href='#SkIRect_height64'>height64</a> <a href='SkRect_Reference#SkRect_width'>SkRect::width()</a>

---

<a name='SkIRect_width64'></a>
## width64

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int64_t <a href='#SkIRect_width64'>width64</a>() const
</pre>

Returns span on the x-axis. This does not check if <a href='#IRect'>IRect</a> is sorted, so the
result may be negative. This is safer than calling <a href='#SkIRect_width'>width</a> since <a href='#SkIRect_width'>width</a> might
overflow in its calculation.

### Return Value

<a href='#SkIRect_fRight'>fRight</a> minus <a href='#SkIRect_fLeft'>fLeft</a> cast to int64_t

### Example

<div><fiddle-embed name="63977f97999bbd6eecfdcc7575d75492">

#### Example Output

~~~~
width: -5 width64: 4294967291
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_width'>width</a> <a href='#SkIRect_height'>height</a> <a href='#SkIRect_height64'>height64</a> <a href='SkRect_Reference#SkRect_width'>SkRect::width()</a>

---

<a name='SkIRect_height'></a>
## height

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_height'>height</a>() const
</pre>

Returns span on the y-axis. This does not check if <a href='#IRect'>IRect</a> is sorted, or if
result fits in 32-bit signed integer; result may be negative.

### Return Value

<a href='#SkIRect_fBottom'>fBottom</a> minus <a href='#SkIRect_fTop'>fTop</a>

### Example

<div><fiddle-embed name="0175bae87fafcd9433ae661574695586">

#### Example Output

~~~~
unsorted height: -5
large height: -5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_width'>width</a> <a href='SkRect_Reference#SkRect_height'>SkRect::height()</a>

---

<a name='SkIRect_height64'></a>
## height64

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int64_t <a href='#SkIRect_height64'>height64</a>() const
</pre>

Returns span on the y-axis. This does not check if <a href='#IRect'>IRect</a> is sorted, so the
result may be negative. This is safer than calling <a href='#SkIRect_height'>height</a> since <a href='#SkIRect_height'>height</a> might
overflow in its calculation.

### Return Value

<a href='#SkIRect_fBottom'>fBottom</a> minus <a href='#SkIRect_fTop'>fTop</a> cast to int64_t

### Example

<div><fiddle-embed name="02dd98716e54bbd8c2f0ff23b7ef98cf">

#### Example Output

~~~~
height: -5 height64: 4294967291
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_width'>width</a> <a href='#SkIRect_height'>height</a> <a href='#SkIRect_width64'>width64</a> <a href='SkRect_Reference#SkRect_height'>SkRect::height()</a>

---

<a name='SkIRect_size'></a>
## size

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkISize'>SkISize</a> <a href='#SkIRect_size'>size</a>() const
</pre>

Returns spans on the x-axis and y-axis. This does not check if <a href='#IRect'>IRect</a> is sorted,
or if result fits in 32-bit signed integer; result may be negative.

### Return Value

<a href='undocumented#ISize'>ISize</a> (width, height)

### Example

<div><fiddle-embed name="8b3224641cb3053a7b8a5798b6cd1cf6">

#### Example Output

~~~~
original rect: 20, 30, 40, 50  size: 20, 20
offset rect: 40, 50, 60, 70  size: 20, 20
outset rect: 20, 30, 80, 90  size: 60, 60
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_height'>height</a> <a href='#SkIRect_width'>width</a> <a href='#SkIRect_MakeSize'>MakeSize</a>

---

<a name='SkIRect_isEmpty'></a>
## isEmpty

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_isEmpty'>isEmpty</a>() const
</pre>

Returns true if <a href='#SkIRect_width'>width</a> or <a href='#SkIRect_height'>height</a> .

### Return Value

true if <a href='#SkIRect_width'>width</a> or <a href='#SkIRect_height'>height</a> are zero or negative

### Example

<div><fiddle-embed name="edaad064b6de249b7a7c768dfa000adc">

#### Example Output

~~~~
rect: {20, 40, 10, 50} is empty
sorted: {10, 40, 20, 50} is not empty
rect: {20, 40, 20, 50} is empty
sorted: {20, 40, 20, 50} is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_EmptyIRect'>EmptyIRect</a> <a href='#SkIRect_MakeEmpty'>MakeEmpty</a> <a href='#SkIRect_sort'>sort</a> <a href='SkRect_Reference#SkRect_isEmpty'>SkRect::isEmpty</a>

---

<a name='SkIRect_isEmpty64'></a>
## isEmpty64

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_isEmpty64'>isEmpty64</a>() const
</pre>

Returns true if <a href='#SkIRect_fLeft'>fLeft</a> is equal to or greater than <a href='#SkIRect_fRight'>fRight</a>, or if <a href='#SkIRect_fTop'>fTop</a> is equal
to or greater than <a href='#SkIRect_fBottom'>fBottom</a>. Call <a href='#SkIRect_sort'>sort</a> to reverse rectangles with negative
<a href='#SkIRect_width64'>width64</a> or <a href='#SkIRect_height64'>height64</a>.

### Return Value

true if <a href='#SkIRect_width64'>width64</a> or <a href='#SkIRect_height64'>height64</a> are zero or negative

### Example

<div><fiddle-embed name="eb905faa1084ccab3ad0605df4c27ea4">

#### Example Output

~~~~
rect: {20, 40, 10, 50} is empty
sorted: {10, 40, 20, 50} is not empty
rect: {20, 40, 20, 50} is empty
sorted: {20, 40, 20, 50} is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_EmptyIRect'>EmptyIRect</a> <a href='#SkIRect_MakeEmpty'>MakeEmpty</a> <a href='#SkIRect_sort'>sort</a> <a href='SkRect_Reference#SkRect_isEmpty'>SkRect::isEmpty</a>

---

## <a name='Operator'>Operator</a>


SkIRect operators inline class member functions with arithmetic equivalents.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_notequal_operator'>operator!=(const SkIRect& a, const SkIRect& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if members are unequal</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_equal_operator'>operator==(const SkIRect& a, const SkIRect& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if members are equal</td>
  </tr>
</table>

<a name='SkIRect_equal_operator'></a>
## operator==

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_equal_operator'>operator==(const SkIRect& a, const SkIRect& b)</a>
</pre>

Returns true if all members in <a href='#SkIRect_equal_operator_a'>a</a>: <a href='#SkIRect_fLeft'>fLeft</a>, <a href='#SkIRect_fTop'>fTop</a>, <a href='#SkIRect_fRight'>fRight</a>, and <a href='#SkIRect_fBottom'>fBottom</a>; are
identical to corresponding members in <a href='#SkIRect_equal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkIRect_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to compare</td>
  </tr>
</table>

### Return Value

true if members are equal

### Example

<div><fiddle-embed name="c79a196278c58b34cd5f551b0124ecc9">

#### Example Output

~~~~
test == sorted
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_notequal_operator'>operator!=(const SkIRect& a, const SkIRect& b)</a>

---

<a name='SkIRect_notequal_operator'></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_notequal_operator'>operator!=(const SkIRect& a, const SkIRect& b)</a>
</pre>

Returns true if any member in <a href='#SkIRect_notequal_operator_a'>a</a>: <a href='#SkIRect_fLeft'>fLeft</a>, <a href='#SkIRect_fTop'>fTop</a>, <a href='#SkIRect_fRight'>fRight</a>, and <a href='#SkIRect_fBottom'>fBottom</a>; is not
identical to the corresponding member in <a href='#SkIRect_notequal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkIRect_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to compare</td>
  </tr>
</table>

### Return Value

true if members are not equal

### Example

<div><fiddle-embed name="6c4acd8aa203f632b7d85cae672abf4d">

#### Example Output

~~~~
test != sorted
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_equal_operator'>operator==(const SkIRect& a, const SkIRect& b)</a>

---

## <a name='Set'>Set</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_set'>set</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to (left, top, right, bottom)</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_setEmpty'>setEmpty</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to (0, 0, 0, 0)</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_setLTRB'>setLTRB</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to <a href='undocumented#SkScalar'>SkScalar</a> input (left, top, right, bottom)</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_setXYWH'>setXYWH</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to (x, y, width, height)</td>
  </tr>
</table>

<a name='SkIRect_setEmpty'></a>
## setEmpty

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_setEmpty'>setEmpty</a>()
</pre>

Sets <a href='#IRect'>IRect</a> to (0, 0, 0, 0).

Many other rectangles are empty; if left is equal to or greater than right,
or if top is equal to or greater than bottom. Setting all members to zero
is a convenience, but does not designate a special empty rectangle.

### Example

<div><fiddle-embed name="94039c3cc9e911c8ab2993d56fd06210">

#### Example Output

~~~~
rect: {3, 4, 1, 2} is empty
rect: {0, 0, 0, 0} is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeEmpty'>MakeEmpty</a> <a href='SkRect_Reference#SkRect_setEmpty'>SkRect::setEmpty</a>

---

<a name='SkIRect_set'></a>
## set

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_set'>set</a>(int32_t left, int32_t top, int32_t right, int32_t bottom)
</pre>

Sets <a href='#IRect'>IRect</a> to (left, top, right, bottom).
left and right are not sorted; left is not necessarily less than right.
top and bottom are not sorted; top is not necessarily less than bottom.

### Parameters

<table>  <tr>    <td><a name='SkIRect_set_left'><code><strong>left</strong></code></a></td>
    <td>assigned to <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_set_top'><code><strong>top</strong></code></a></td>
    <td>assigned to <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_set_right'><code><strong>right</strong></code></a></td>
    <td>assigned to <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_set_bottom'><code><strong>bottom</strong></code></a></td>
    <td>assigned to <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1912c37076b7f3bf6aebfa167e971bec">

#### Example Output

~~~~
rect1: {3, 4, 1, 2}
rect2: {3, 4, 1, 2}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_setLTRB'>setLTRB</a> <a href='#SkIRect_setXYWH'>setXYWH</a> <a href='SkRect_Reference#SkRect_set'>SkRect::set</a><sup><a href='SkRect_Reference#SkRect_set_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_set_3'>[3]</a></sup><sup><a href='SkRect_Reference#SkRect_set_4'>[4]</a></sup>

---

<a name='SkIRect_setLTRB'></a>
## setLTRB

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_setLTRB'>setLTRB</a>(int32_t left, int32_t top, int32_t right, int32_t bottom)
</pre>

Sets <a href='#IRect'>IRect</a> to (left, top, right, bottom).
left and right are not sorted; left is not necessarily less than right.
top and bottom are not sorted; top is not necessarily less than bottom.

### Parameters

<table>  <tr>    <td><a name='SkIRect_setLTRB_left'><code><strong>left</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setLTRB_top'><code><strong>top</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setLTRB_right'><code><strong>right</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setLTRB_bottom'><code><strong>bottom</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ead6bdcf2ae77ec19a1c5a96f5b31af8">

#### Example Output

~~~~
rect1: {3, 4, 1, 2}
rect2: {3, 4, 1, 2}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_set'>set</a> <a href='#SkIRect_setXYWH'>setXYWH</a> <a href='SkRect_Reference#SkRect_setLTRB'>SkRect::setLTRB</a>

---

<a name='SkIRect_setXYWH'></a>
## setXYWH

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_setXYWH'>setXYWH</a>(int32_t x, int32_t y, int32_t width, int32_t height)
</pre>

Sets <a href='#IRect'>IRect</a> to:
(x, y, x + width, y + height)
.
Does not validate input;
width or height may be negative.

### Parameters

<table>  <tr>    <td><a name='SkIRect_setXYWH_x'><code><strong>x</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setXYWH_y'><code><strong>y</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setXYWH_width'><code><strong>width</strong></code></a></td>
    <td>added to x and stored in <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setXYWH_height'><code><strong>height</strong></code></a></td>
    <td>added to y and stored in <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0e1db8c86678c004e504f47641b44b17">

#### Example Output

~~~~
rect: 5, 35, -10, 60  isEmpty: true
rect: -10, 35, 5, 60  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeXYWH'>MakeXYWH</a> <a href='#SkIRect_setLTRB'>setLTRB</a> <a href='#SkIRect_set'>set</a> <a href='SkRect_Reference#SkRect_setXYWH'>SkRect::setXYWH</a>

---

## <a name='Inset_Outset_Offset'>Inset Outset Offset</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_adjust'>adjust</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>moves the sides independently relative to their original locations</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_inset'>inset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>moves the sides symmetrically about the center</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_offset'>offset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>translates sides without changing width and height</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_offset'>offset(int32 t dx, int32 t dy)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_offsetTo'>offsetTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>translates to (x, y) without changing width and height</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_outset'>outset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>moves the sides symmetrically about the center</td>
  </tr>
</table>

<a name='SkIRect_makeOffset'></a>
## makeOffset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_makeOffset'>makeOffset</a>(int32_t dx, int32_t dy) const
</pre>

Returns <a href='#IRect'>IRect</a> offset by (<a href='#SkIRect_makeOffset_dx'>dx</a>, <a href='#SkIRect_makeOffset_dy'>dy</a>).

If <a href='#SkIRect_makeOffset_dx'>dx</a> is negative, <a href='#IRect'>IRect</a> returned is moved to the left.
If <a href='#SkIRect_makeOffset_dx'>dx</a> is positive, <a href='#IRect'>IRect</a> returned is moved to the right.
If <a href='#SkIRect_makeOffset_dy'>dy</a> is negative, <a href='#IRect'>IRect</a> returned is moved upward.
If <a href='#SkIRect_makeOffset_dy'>dy</a> is positive, <a href='#IRect'>IRect</a> returned is moved downward.

### Parameters

<table>  <tr>    <td><a name='SkIRect_makeOffset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a> and <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_makeOffset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a> and <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='#IRect'>IRect</a> offset by <a href='#SkIRect_makeOffset_dx'>dx</a> and <a href='#SkIRect_makeOffset_dy'>dy</a>, with original width and height

### Example

<div><fiddle-embed name="737c747df07ddf392c05970440de0927">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: 25, 82, 35, 92  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_offset'>offset</a><sup><a href='#SkIRect_offset_2'>[2]</a></sup> <a href='#SkIRect_makeInset'>makeInset</a> <a href='#SkIRect_makeOutset'>makeOutset</a> <a href='SkRect_Reference#SkRect_makeOffset'>SkRect::makeOffset</a>

---

<a name='SkIRect_makeInset'></a>
## makeInset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_makeInset'>makeInset</a>(int32_t dx, int32_t dy) const
</pre>

Returns <a href='#IRect'>IRect</a>, inset by (<a href='#SkIRect_makeInset_dx'>dx</a>, <a href='#SkIRect_makeInset_dy'>dy</a>).

If <a href='#SkIRect_makeInset_dx'>dx</a> is negative, <a href='#IRect'>IRect</a> returned is wider.
If <a href='#SkIRect_makeInset_dx'>dx</a> is positive, <a href='#IRect'>IRect</a> returned is narrower.
If <a href='#SkIRect_makeInset_dy'>dy</a> is negative, <a href='#IRect'>IRect</a> returned is taller.
If <a href='#SkIRect_makeInset_dy'>dy</a> is positive, <a href='#IRect'>IRect</a> returned is shorter.

### Parameters

<table>  <tr>    <td><a name='SkIRect_makeInset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a> and subtracted from <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_makeInset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a> and subtracted from <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='#IRect'>IRect</a> inset symmetrically left and right, top and bottom

### Example

<div><fiddle-embed name="1db94b2c76e0a7a71856532335fa56b6">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: 25, 82, 5, 28  isEmpty: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_inset'>inset</a> <a href='#SkIRect_makeOffset'>makeOffset</a> <a href='#SkIRect_makeOutset'>makeOutset</a> <a href='SkRect_Reference#SkRect_makeInset'>SkRect::makeInset</a>

---

<a name='SkIRect_makeOutset'></a>
## makeOutset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_makeOutset'>makeOutset</a>(int32_t dx, int32_t dy) const
</pre>

Returns <a href='#IRect'>IRect</a>, outset by (<a href='#SkIRect_makeOutset_dx'>dx</a>, <a href='#SkIRect_makeOutset_dy'>dy</a>).

If <a href='#SkIRect_makeOutset_dx'>dx</a> is negative, <a href='#IRect'>IRect</a> returned is narrower.
If <a href='#SkIRect_makeOutset_dx'>dx</a> is positive, <a href='#IRect'>IRect</a> returned is wider.
If <a href='#SkIRect_makeOutset_dy'>dy</a> is negative, <a href='#IRect'>IRect</a> returned is shorter.
If <a href='#SkIRect_makeOutset_dy'>dy</a> is positive, <a href='#IRect'>IRect</a> returned is taller.

### Parameters

<table>  <tr>    <td><a name='SkIRect_makeOutset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset subtracted to <a href='#SkIRect_fLeft'>fLeft</a> and added from <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_makeOutset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset subtracted to <a href='#SkIRect_fTop'>fTop</a> and added from <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='#IRect'>IRect</a> outset symmetrically left and right, top and bottom

### Example

<div><fiddle-embed name="240e2953e3455c08f6d89255feff8416">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: -5, 18, 35, 92  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_outset'>outset</a> <a href='#SkIRect_makeOffset'>makeOffset</a> <a href='#SkIRect_makeInset'>makeInset</a> <a href='SkRect_Reference#SkRect_makeOutset'>SkRect::makeOutset</a>

---

<a name='SkIRect_offset'></a>
## offset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_offset'>offset</a>(int32_t dx, int32_t dy)
</pre>

Offsets <a href='#IRect'>IRect</a> by adding <a href='#SkIRect_offset_dx'>dx</a> to <a href='#SkIRect_fLeft'>fLeft</a>, <a href='#SkIRect_fRight'>fRight</a>; and by adding <a href='#SkIRect_offset_dy'>dy</a> to <a href='#SkIRect_fTop'>fTop</a>, <a href='#SkIRect_fBottom'>fBottom</a>.

If <a href='#SkIRect_offset_dx'>dx</a> is negative, moves <a href='#IRect'>IRect</a> returned to the left.
If <a href='#SkIRect_offset_dx'>dx</a> is positive, moves <a href='#IRect'>IRect</a> returned to the right.
If <a href='#SkIRect_offset_dy'>dy</a> is negative, moves <a href='#IRect'>IRect</a> returned upward.
If <a href='#SkIRect_offset_dy'>dy</a> is positive, moves <a href='#IRect'>IRect</a> returned downward.

### Parameters

<table>  <tr>    <td><a name='SkIRect_offset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a> and <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_offset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a> and <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="77e633b2174ffae923c038b303418b50">

#### Example Output

~~~~
rect: 15, 27, 55, 86
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_offsetTo'>offsetTo</a> <a href='#SkIRect_makeOffset'>makeOffset</a> <a href='SkRect_Reference#SkRect_offset'>SkRect::offset</a><sup><a href='SkRect_Reference#SkRect_offset_2'>[2]</a></sup>

---

<a name='SkIRect_offset_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_offset'>offset</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& delta)
</pre>

Offsets <a href='#IRect'>IRect</a> by adding <a href='#SkIRect_offset_2_delta'>delta</a>.fX to <a href='#SkIRect_fLeft'>fLeft</a>, <a href='#SkIRect_fRight'>fRight</a>; and by adding <a href='#SkIRect_offset_2_delta'>delta</a>.fY to
<a href='#SkIRect_fTop'>fTop</a>, <a href='#SkIRect_fBottom'>fBottom</a>.

If <a href='#SkIRect_offset_2_delta'>delta</a>.fX is negative, moves <a href='#IRect'>IRect</a> returned to the left.
If <a href='#SkIRect_offset_2_delta'>delta</a>.fX is positive, moves <a href='#IRect'>IRect</a> returned to the right.
If <a href='#SkIRect_offset_2_delta'>delta</a>.fY is negative, moves <a href='#IRect'>IRect</a> returned upward.
If <a href='#SkIRect_offset_2_delta'>delta</a>.fY is positive, moves <a href='#IRect'>IRect</a> returned downward.

### Parameters

<table>  <tr>    <td><a name='SkIRect_offset_2_delta'><code><strong>delta</strong></code></a></td>
    <td>offset added to <a href='#IRect'>IRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="31a4c575499e76def651eb65994876f0">

#### Example Output

~~~~
rect: 15, 27, 55, 86
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_offsetTo'>offsetTo</a> <a href='#SkIRect_makeOffset'>makeOffset</a> <a href='SkRect_Reference#SkRect_offset'>SkRect::offset</a><sup><a href='SkRect_Reference#SkRect_offset_2'>[2]</a></sup>

---

<a name='SkIRect_offsetTo'></a>
## offsetTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_offsetTo'>offsetTo</a>(int32_t newX, int32_t newY)
</pre>

Offsets <a href='#IRect'>IRect</a> so that <a href='#SkIRect_fLeft'>fLeft</a> equals <a href='#SkIRect_offsetTo_newX'>newX</a>, and <a href='#SkIRect_fTop'>fTop</a> equals <a href='#SkIRect_offsetTo_newY'>newY</a>. width and height
are unchanged.

### Parameters

<table>  <tr>    <td><a name='SkIRect_offsetTo_newX'><code><strong>newX</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fLeft'>fLeft</a>, preserving <a href='#SkIRect_width'>width</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_offsetTo_newY'><code><strong>newY</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fTop'>fTop</a>, preserving <a href='#SkIRect_height'>height</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a2734ff23b35653956a3002e5c29ff91">

#### Example Output

~~~~
rect: 15, 27, 55, 86
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_offset'>offset</a><sup><a href='#SkIRect_offset_2'>[2]</a></sup> <a href='#SkIRect_makeOffset'>makeOffset</a> <a href='#SkIRect_setXYWH'>setXYWH</a> <a href='SkRect_Reference#SkRect_offsetTo'>SkRect::offsetTo</a>

---

<a name='SkIRect_inset'></a>
## inset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_inset'>inset</a>(int32_t dx, int32_t dy)
</pre>

Insets <a href='#IRect'>IRect</a> by (<a href='#SkIRect_inset_dx'>dx</a>,<a href='#SkIRect_inset_dy'>dy</a>).

If <a href='#SkIRect_inset_dx'>dx</a> is positive, makes <a href='#IRect'>IRect</a> narrower.
If <a href='#SkIRect_inset_dx'>dx</a> is negative, makes <a href='#IRect'>IRect</a> wider.
If <a href='#SkIRect_inset_dy'>dy</a> is positive, makes <a href='#IRect'>IRect</a> shorter.
If <a href='#SkIRect_inset_dy'>dy</a> is negative, makes <a href='#IRect'>IRect</a> taller.

### Parameters

<table>  <tr>    <td><a name='SkIRect_inset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a> and subtracted from <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_inset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a> and subtracted from <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9debaded1aa8bdf5077a4de0b3015b8f">

#### Example Output

~~~~
rect: 15, 27, 45, 60
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_outset'>outset</a> <a href='#SkIRect_makeInset'>makeInset</a> <a href='SkRect_Reference#SkRect_inset'>SkRect::inset</a>

---

<a name='SkIRect_outset'></a>
## outset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_outset'>outset</a>(int32_t dx, int32_t dy)
</pre>

Outsets <a href='#IRect'>IRect</a> by (<a href='#SkIRect_outset_dx'>dx</a>, <a href='#SkIRect_outset_dy'>dy</a>).

If <a href='#SkIRect_outset_dx'>dx</a> is positive, makes <a href='#IRect'>IRect</a> wider.
If <a href='#SkIRect_outset_dx'>dx</a> is negative, makes <a href='#IRect'>IRect</a> narrower.
If <a href='#SkIRect_outset_dy'>dy</a> is positive, makes <a href='#IRect'>IRect</a> taller.
If <a href='#SkIRect_outset_dy'>dy</a> is negative, makes <a href='#IRect'>IRect</a> shorter.

### Parameters

<table>  <tr>    <td><a name='SkIRect_outset_dx'><code><strong>dx</strong></code></a></td>
    <td>subtracted to <a href='#SkIRect_fLeft'>fLeft</a> and added from <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_outset_dy'><code><strong>dy</strong></code></a></td>
    <td>subtracted to <a href='#SkIRect_fTop'>fTop</a> and added from <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3fc62ca29428195f33a3a02b3eb74e4f">

#### Example Output

~~~~
rect: 5, 1, 55, 86
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_inset'>inset</a> <a href='#SkIRect_makeOutset'>makeOutset</a> <a href='SkRect_Reference#SkRect_outset'>SkRect::outset</a>

---

## <a name='Intersection'>Intersection</a>

<a href='#IRect'>IRects</a> intersect when they enclose a common area. To intersect, each of the pair
must describe area; <a href='#SkIRect_fLeft'>fLeft</a> is less than <a href='#SkIRect_fRight'>fRight</a>, and <a href='#SkIRect_fTop'>fTop</a> is less than <a href='#SkIRect_fBottom'>fBottom</a>;
empty() returns false. The intersection of <a href='#IRect'>IRect</a> pair can be described by:

(max(a.<a href='#SkIRect_fLeft'>fLeft</a>, b.<a href='#SkIRect_fLeft'>fLeft</a>), max(a.<a href='#SkIRect_fTop'>fTop</a>, b.<a href='#SkIRect_fTop'>fTop</a>),
min(a.<a href='#SkIRect_fRight'>fRight</a>, b.<a href='#SkIRect_fRight'>fRight</a>), min(a.<a href='#SkIRect_fBottom'>fBottom</a>, b.<a href='#SkIRect_fBottom'>fBottom</a>))
.

The intersection is only meaningful if the resulting <a href='#IRect'>IRect</a> is not empty and
describes an area: <a href='#SkIRect_fLeft'>fLeft</a> is less than <a href='#SkIRect_fRight'>fRight</a>, and <a href='#SkIRect_fTop'>fTop</a> is less than <a href='#SkIRect_fBottom'>fBottom</a>.


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_Intersects'>Intersects</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if areas overlap</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_IntersectsNoEmptyCheck'>IntersectsNoEmptyCheck</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if areas overlap skips empty check</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_contains'>contains</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='SkIPoint_Reference#IPoint'>IPoint</a> (x, y) is equal or inside</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_contains'>contains(int32 t x, int32 t y)</a> const</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if contains unsorted <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck(int32 t left, int32 t top, int32 t right, int32 t bottom)</a> const</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_intersect'>intersect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to shared area; returns true if not empty</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_intersect'>intersect(const SkIRect& r)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to shared area; returns true if not empty skips empty check</td>
  </tr>
</table>

<a name='SkIRect_adjust'></a>
## adjust

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_adjust'>adjust</a>(int32_t dL, int32_t dT, int32_t dR, int32_t dB)
</pre>

Adjusts <a href='#IRect'>IRect</a> by adding <a href='#SkIRect_adjust_dL'>dL</a> to <a href='#SkIRect_fLeft'>fLeft</a>, <a href='#SkIRect_adjust_dT'>dT</a> to <a href='#SkIRect_fTop'>fTop</a>, <a href='#SkIRect_adjust_dR'>dR</a> to <a href='#SkIRect_fRight'>fRight</a>, and <a href='#SkIRect_adjust_dB'>dB</a> to <a href='#SkIRect_fBottom'>fBottom</a>.

If <a href='#SkIRect_adjust_dL'>dL</a> is positive, narrows <a href='#IRect'>IRect</a> on the left. If negative, widens it on the left.
If <a href='#SkIRect_adjust_dT'>dT</a> is positive, shrinks <a href='#IRect'>IRect</a> on the top. If negative, lengthens it on the top.
If <a href='#SkIRect_adjust_dR'>dR</a> is positive, narrows <a href='#IRect'>IRect</a> on the right. If negative, widens it on the right.
If <a href='#SkIRect_adjust_dB'>dB</a> is positive, shrinks <a href='#IRect'>IRect</a> on the bottom. If negative, lengthens it on the bottom.

The resulting <a href='#IRect'>IRect</a> is not checked for validity. Thus, if the resulting <a href='#IRect'>IRect</a> left is
greater than right, the <a href='#IRect'>IRect</a> will be considered empty. Call <a href='#SkIRect_sort'>sort</a> after this call
if that is not the desired behavior.

### Parameters

<table>  <tr>    <td><a name='SkIRect_adjust_dL'><code><strong>dL</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_adjust_dT'><code><strong>dT</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_adjust_dR'><code><strong>dR</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_adjust_dB'><code><strong>dB</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8dc91284493dd012cca3d0ce4c66bda4">

#### Example Output

~~~~
rect: 10, 10, 20, 20
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_inset'>inset</a> <a href='#SkIRect_outset'>outset</a>

---

<a name='SkIRect_contains'></a>
## contains

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_contains'>contains</a>(int32_t x, int32_t y) const
</pre>

Returns true if:
<a href='#SkIRect_fLeft'>fLeft</a> <= x < <a href='#SkIRect_fRight'>fRight</a> && <a href='#SkIRect_fTop'>fTop</a> <= y < <a href='#SkIRect_fBottom'>fBottom</a>.

Returns false if <a href='#IRect'>IRect</a> is empty.

Considers input to describe constructed <a href='#IRect'>IRect</a>:
(x, y, x + 1, y + 1)
and
returns true if constructed area is completely enclosed by <a href='#IRect'>IRect</a> area.

### Parameters

<table>  <tr>    <td><a name='SkIRect_contains_x'><code><strong>x</strong></code></a></td>
    <td>test <a href='SkIPoint_Reference#IPoint'>IPoint</a> x-coordinate</td>
  </tr>
  <tr>    <td><a name='SkIRect_contains_y'><code><strong>y</strong></code></a></td>
    <td>test <a href='SkIPoint_Reference#IPoint'>IPoint</a> y-coordinate</td>
  </tr>
</table>

### Return Value

true if (x, y) is inside <a href='#IRect'>IRect</a>

### Example

<div><fiddle-embed name="a7958a4e0668f5cf805a8e78eb57f51d">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50)
rect: (30, 50, 40, 60) does not contain (40, 50)
rect: (30, 50, 40, 60) does not contain (30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a><sup><a href='#SkIRect_containsNoEmptyCheck_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_contains'>SkRect::contains</a><sup><a href='SkRect_Reference#SkRect_contains_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_contains_3'>[3]</a></sup>

---

<a name='SkIRect_contains_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_contains'>contains</a>(int32_t left, int32_t top, int32_t right, int32_t bottom) const
</pre>

Constructs <a href='#IRect'>IRect</a> to intersect from (left, top, right, bottom). Does not sort
construction.

Returns true if <a href='#IRect'>IRect</a> contains construction.
Returns false if <a href='#IRect'>IRect</a> is empty or construction is empty.

### Parameters

<table>  <tr>    <td><a name='SkIRect_contains_2_left'><code><strong>left</strong></code></a></td>
    <td>x-axis minimum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_contains_2_top'><code><strong>top</strong></code></a></td>
    <td>y-axis minimum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_contains_2_right'><code><strong>right</strong></code></a></td>
    <td>x-axis maximum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_contains_2_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y-axis maximum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
</table>

### Return Value

true if all sides of <a href='#IRect'>IRect</a> are outside construction

### Example

<div><fiddle-embed name="eae55f284818d9965ec5834747d14a48">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50, 31, 51)
rect: (30, 50, 40, 60) does not contain (39, 49, 40, 50)
rect: (30, 50, 40, 60) does not contain (29, 59, 30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a><sup><a href='#SkIRect_containsNoEmptyCheck_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_contains'>SkRect::contains</a><sup><a href='SkRect_Reference#SkRect_contains_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_contains_3'>[3]</a></sup>

---

<a name='SkIRect_contains_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_contains'>contains</a>(const <a href='#SkIRect'>SkIRect</a>& r) const
</pre>

Returns true if <a href='#IRect'>IRect</a> contains <a href='#SkIRect_contains_3_r'>r</a>.
Returns false if <a href='#IRect'>IRect</a> is empty or <a href='#SkIRect_contains_3_r'>r</a> is empty.

<a href='#IRect'>IRect</a> contains <a href='#SkIRect_contains_3_r'>r</a> when <a href='#IRect'>IRect</a> area completely includes <a href='#SkIRect_contains_3_r'>r</a> area.

### Parameters

<table>  <tr>    <td><a name='SkIRect_contains_3_r'><code><strong>r</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> contained</td>
  </tr>
</table>

### Return Value

true if all sides of <a href='#IRect'>IRect</a> are outside <a href='#SkIRect_contains_3_r'>r</a>

### Example

<div><fiddle-embed name="ee0185db622602b4eb19583c2f42c734">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50, 31, 51)
rect: (30, 50, 40, 60) does not contain (39, 49, 40, 50)
rect: (30, 50, 40, 60) does not contain (29, 59, 30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a><sup><a href='#SkIRect_containsNoEmptyCheck_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_contains'>SkRect::contains</a><sup><a href='SkRect_Reference#SkRect_contains_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_contains_3'>[3]</a></sup>

---

<a name='SkIRect_contains_4'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_contains'>contains</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r) const
</pre>

Returns true if <a href='#IRect'>IRect</a> contains <a href='#SkIRect_contains_4_r'>r</a>.
Returns false if <a href='#IRect'>IRect</a> is empty or <a href='#SkIRect_contains_4_r'>r</a> is empty.

<a href='#IRect'>IRect</a> contains <a href='#SkIRect_contains_4_r'>r</a> when <a href='#IRect'>IRect</a> area completely includes <a href='#SkIRect_contains_4_r'>r</a> area.

### Parameters

<table>  <tr>    <td><a name='SkIRect_contains_4_r'><code><strong>r</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> contained</td>
  </tr>
</table>

### Return Value

true if all sides of <a href='#IRect'>IRect</a> are outside <a href='#SkIRect_contains_4_r'>r</a>

### Example

<div><fiddle-embed name="acbd79ffb304f332e4b38ef18e19663e">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50, 31, 51)
rect: (30, 50, 40, 60) does not contain (39, 49, 40, 50)
rect: (30, 50, 40, 60) does not contain (29, 59, 30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a><sup><a href='#SkIRect_containsNoEmptyCheck_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_contains'>SkRect::contains</a><sup><a href='SkRect_Reference#SkRect_contains_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_contains_3'>[3]</a></sup>

---

<a name='SkIRect_containsNoEmptyCheck'></a>
## containsNoEmptyCheck

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a>(int32_t left, int32_t top, int32_t right, int32_t bottom) const
</pre>

Constructs <a href='#IRect'>IRect</a> from (left, top, right, bottom). Does not sort
construction.

Returns true if <a href='#IRect'>IRect</a> contains construction.
Asserts if <a href='#IRect'>IRect</a> is empty or construction is empty, and if SK_DEBUG is defined.

Return is undefined if <a href='#IRect'>IRect</a> is empty or construction is empty.

### Parameters

<table>  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_left'><code><strong>left</strong></code></a></td>
    <td>x-axis minimum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_top'><code><strong>top</strong></code></a></td>
    <td>y-axis minimum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_right'><code><strong>right</strong></code></a></td>
    <td>x-axis maximum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y-axis maximum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
</table>

### Return Value

true if all sides of <a href='#IRect'>IRect</a> are outside construction

### Example

<div><fiddle-embed name="fef2a36bee224e92500199fa9d3cbb8b">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50, 31, 51)
rect: (30, 50, 40, 60) does not contain (39, 49, 40, 50)
rect: (30, 50, 40, 60) does not contain (29, 59, 30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_contains'>contains</a><sup><a href='#SkIRect_contains_2'>[2]</a></sup><sup><a href='#SkIRect_contains_3'>[3]</a></sup><sup><a href='#SkIRect_contains_4'>[4]</a></sup> <a href='SkRect_Reference#SkRect_contains'>SkRect::contains</a><sup><a href='SkRect_Reference#SkRect_contains_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_contains_3'>[3]</a></sup>

---

<a name='SkIRect_containsNoEmptyCheck_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a>(const <a href='#SkIRect'>SkIRect</a>& r) const
</pre>

Returns true if <a href='#IRect'>IRect</a> contains construction.
Asserts if <a href='#IRect'>IRect</a> is empty or construction is empty, and if SK_DEBUG is defined.

Return is undefined if <a href='#IRect'>IRect</a> is empty or construction is empty.

### Parameters

<table>  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_2_r'><code><strong>r</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> contained</td>
  </tr>
</table>

### Return Value

true if all sides of <a href='#IRect'>IRect</a> are outside <a href='#SkIRect_containsNoEmptyCheck_2_r'>r</a>

### Example

<div><fiddle-embed name="8f91f58001d9c10420eb146fbc169af4">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50, 31, 51)
rect: (30, 50, 40, 60) does not contain (39, 49, 40, 50)
rect: (30, 50, 40, 60) does not contain (29, 59, 30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_contains'>contains</a><sup><a href='#SkIRect_contains_2'>[2]</a></sup><sup><a href='#SkIRect_contains_3'>[3]</a></sup><sup><a href='#SkIRect_contains_4'>[4]</a></sup> <a href='SkRect_Reference#SkRect_contains'>SkRect::contains</a><sup><a href='SkRect_Reference#SkRect_contains_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_contains_3'>[3]</a></sup>

---

<a name='SkIRect_intersect'></a>
## intersect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_intersect'>intersect</a>(const <a href='#SkIRect'>SkIRect</a>& r)
</pre>

Returns true if <a href='#IRect'>IRect</a> intersects <a href='#SkIRect_intersect_r'>r</a>, and sets <a href='#IRect'>IRect</a> to intersection.
Returns false if <a href='#IRect'>IRect</a> does not intersect <a href='#SkIRect_intersect_r'>r</a>, and leaves <a href='#IRect'>IRect</a> unchanged.

Returns false if either <a href='#SkIRect_intersect_r'>r</a> or <a href='#IRect'>IRect</a> is empty, leaving <a href='#IRect'>IRect</a> unchanged.

### Parameters

<table>  <tr>    <td><a name='SkIRect_intersect_r'><code><strong>r</strong></code></a></td>
    <td>limit of result</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_intersect_r'>r</a> and <a href='#IRect'>IRect</a> have area in common

### Example

<div><fiddle-embed name="ea233f5d5d1ae0e76fc6f2eb371c927a"><div>Two <a href='undocumented#SkDebugf'>SkDebugf</a> calls are required. If the calls are combined, their arguments
may not be evaluated in left to right order: the printed intersection may
be before or after the call to intersect.
</div>

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a> <a href='#SkIRect_join'>join</a><sup><a href='#SkIRect_join_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_intersect'>SkRect::intersect</a><sup><a href='SkRect_Reference#SkRect_intersect_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_intersect_3'>[3]</a></sup>

---

<a name='SkIRect_intersect_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT <a href='#SkIRect_intersect'>intersect</a>(const <a href='#SkIRect'>SkIRect</a>& a, const <a href='#SkIRect'>SkIRect</a>& b)
</pre>

Returns true if <a href='#SkIRect_intersect_2_a'>a</a> intersects <a href='#SkIRect_intersect_2_b'>b</a>, and sets <a href='#IRect'>IRect</a> to intersection.
Returns false if <a href='#SkIRect_intersect_2_a'>a</a> does not intersect <a href='#SkIRect_intersect_2_b'>b</a>, and leaves <a href='#IRect'>IRect</a> unchanged.

Returns false if either <a href='#SkIRect_intersect_2_a'>a</a> or <a href='#SkIRect_intersect_2_b'>b</a> is empty, leaving <a href='#IRect'>IRect</a> unchanged.

### Parameters

<table>  <tr>    <td><a name='SkIRect_intersect_2_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
  <tr>    <td><a name='SkIRect_intersect_2_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_intersect_2_a'>a</a> and <a href='#SkIRect_intersect_2_b'>b</a> have area in common

### Example

<div><fiddle-embed name="b2db0573aacf99ca52776c5522459d02">

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a> <a href='#SkIRect_join'>join</a><sup><a href='#SkIRect_join_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_intersect'>SkRect::intersect</a><sup><a href='SkRect_Reference#SkRect_intersect_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_intersect_3'>[3]</a></sup>

---

<a name='SkIRect_intersectNoEmptyCheck'></a>
## intersectNoEmptyCheck

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT <a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a>(const <a href='#SkIRect'>SkIRect</a>& a, const <a href='#SkIRect'>SkIRect</a>& b)
</pre>

Returns true if <a href='#SkIRect_intersectNoEmptyCheck_a'>a</a> intersects <a href='#SkIRect_intersectNoEmptyCheck_b'>b</a>, and sets <a href='#IRect'>IRect</a> to intersection.
Returns false if <a href='#SkIRect_intersectNoEmptyCheck_a'>a</a> does not intersect <a href='#SkIRect_intersectNoEmptyCheck_b'>b</a>, and leaves <a href='#IRect'>IRect</a> unchanged.

Asserts if either <a href='#SkIRect_intersectNoEmptyCheck_a'>a</a> or <a href='#SkIRect_intersectNoEmptyCheck_b'>b</a> is empty, and if SK_DEBUG is defined.

### Parameters

<table>  <tr>    <td><a name='SkIRect_intersectNoEmptyCheck_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
  <tr>    <td><a name='SkIRect_intersectNoEmptyCheck_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_intersectNoEmptyCheck_a'>a</a> and <a href='#SkIRect_intersectNoEmptyCheck_b'>b</a> have area in common

### Example

<div><fiddle-embed name="d35fbc9fdea71df8b8a12fd3da50d11c">

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_intersect'>intersect</a><sup><a href='#SkIRect_intersect_2'>[2]</a></sup><sup><a href='#SkIRect_intersect_3'>[3]</a></sup> <a href='#SkIRect_join'>join</a><sup><a href='#SkIRect_join_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_intersect'>SkRect::intersect</a><sup><a href='SkRect_Reference#SkRect_intersect_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_intersect_3'>[3]</a></sup>

---

<a name='SkIRect_intersect_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_intersect'>intersect</a>(int32_t left, int32_t top, int32_t right, int32_t bottom)
</pre>

Constructs <a href='#IRect'>IRect</a> to intersect from (left, top, right, bottom). Does not sort
construction.

Returns true if <a href='#IRect'>IRect</a> intersects construction, and sets <a href='#IRect'>IRect</a> to intersection.
Returns false if <a href='#IRect'>IRect</a> does not intersect construction, and leaves <a href='#IRect'>IRect</a> unchanged.

Returns false if either construction or <a href='#IRect'>IRect</a> is empty, leaving <a href='#IRect'>IRect</a> unchanged.

### Parameters

<table>  <tr>    <td><a name='SkIRect_intersect_3_left'><code><strong>left</strong></code></a></td>
    <td>x-axis minimum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_intersect_3_top'><code><strong>top</strong></code></a></td>
    <td>y-axis minimum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_intersect_3_right'><code><strong>right</strong></code></a></td>
    <td>x-axis maximum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_intersect_3_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y-axis maximum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
</table>

### Return Value

true if construction and <a href='#IRect'>IRect</a> have area in common

### Example

<div><fiddle-embed name="200422990eded2f754ab9893118f2645"><div>Two <a href='undocumented#SkDebugf'>SkDebugf</a> calls are required. If the calls are combined, their arguments
may not be evaluated in left to right order: the printed intersection may
be before or after the call to intersect.
</div>

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a> <a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_join'>join</a><sup><a href='#SkIRect_join_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_intersect'>SkRect::intersect</a><sup><a href='SkRect_Reference#SkRect_intersect_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_intersect_3'>[3]</a></sup>

---

<a name='SkIRect_Intersects'></a>
## Intersects

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkIRect_Intersects'>Intersects</a>(const <a href='#SkIRect'>SkIRect</a>& a, const <a href='#SkIRect'>SkIRect</a>& b)
</pre>

Returns true if <a href='#SkIRect_Intersects_a'>a</a> intersects <a href='#SkIRect_Intersects_b'>b</a>.
Returns false if either <a href='#SkIRect_Intersects_a'>a</a> or <a href='#SkIRect_Intersects_b'>b</a> is empty, or do not intersect.

### Parameters

<table>  <tr>    <td><a name='SkIRect_Intersects_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
  <tr>    <td><a name='SkIRect_Intersects_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_Intersects_a'>a</a> and <a href='#SkIRect_Intersects_b'>b</a> have area in common

### Example

<div><fiddle-embed name="0c67cf8981389efc7108369fb9b7976b">

#### Example Output

~~~~
intersection
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_IntersectsNoEmptyCheck'>IntersectsNoEmptyCheck</a> <a href='#SkIRect_intersect'>intersect</a><sup><a href='#SkIRect_intersect_2'>[2]</a></sup><sup><a href='#SkIRect_intersect_3'>[3]</a></sup> <a href='SkRect_Reference#SkRect_intersect'>SkRect::intersect</a><sup><a href='SkRect_Reference#SkRect_intersect_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_intersect_3'>[3]</a></sup>

---

<a name='SkIRect_IntersectsNoEmptyCheck'></a>
## IntersectsNoEmptyCheck

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkIRect_IntersectsNoEmptyCheck'>IntersectsNoEmptyCheck</a>(const <a href='#SkIRect'>SkIRect</a>& a, const <a href='#SkIRect'>SkIRect</a>& b)
</pre>

Returns true if <a href='#SkIRect_IntersectsNoEmptyCheck_a'>a</a> intersects <a href='#SkIRect_IntersectsNoEmptyCheck_b'>b</a>.
Asserts if either <a href='#SkIRect_IntersectsNoEmptyCheck_a'>a</a> or <a href='#SkIRect_IntersectsNoEmptyCheck_b'>b</a> is empty, and if SK_DEBUG is defined.

### Parameters

<table>  <tr>    <td><a name='SkIRect_IntersectsNoEmptyCheck_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
  <tr>    <td><a name='SkIRect_IntersectsNoEmptyCheck_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_IntersectsNoEmptyCheck_a'>a</a> and <a href='#SkIRect_IntersectsNoEmptyCheck_b'>b</a> have area in common

### Example

<div><fiddle-embed name="dba234d15162fb5b26e1a96529ca6a2a">

#### Example Output

~~~~
intersection
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_intersect'>intersect</a><sup><a href='#SkIRect_intersect_2'>[2]</a></sup><sup><a href='#SkIRect_intersect_3'>[3]</a></sup> <a href='SkRect_Reference#SkRect_intersect'>SkRect::intersect</a><sup><a href='SkRect_Reference#SkRect_intersect_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_intersect_3'>[3]</a></sup>

---

## <a name='Join'>Join</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_join'>join</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to union of bounds</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_join'>join(int32 t left, int32 t top, int32 t right, int32 t bottom)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_join_2'>join(const SkIRect& r)</a></td>
  </tr>
</table>

<a name='SkIRect_join'></a>
## join

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_join'>join</a>(int32_t left, int32_t top, int32_t right, int32_t bottom)
</pre>

Constructs <a href='#IRect'>IRect</a> to intersect from (left, top, right, bottom). Does not sort
construction.

Sets <a href='#IRect'>IRect</a> to the union of itself and the construction.

Has no effect if construction is empty. Otherwise, if <a href='#IRect'>IRect</a> is empty, sets
<a href='#IRect'>IRect</a> to construction.

### Parameters

<table>  <tr>    <td><a name='SkIRect_join_left'><code><strong>left</strong></code></a></td>
    <td>x-axis minimum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_join_top'><code><strong>top</strong></code></a></td>
    <td>y-axis minimum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_join_right'><code><strong>right</strong></code></a></td>
    <td>x-axis maximum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_join_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y-axis maximum of constructed <a href='#IRect'>IRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c00ef06289d21db70340e465690e0e08">

#### Example Output

~~~~
join: 10, 20, 55, 65
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_set'>set</a> <a href='SkRect_Reference#SkRect_join'>SkRect::join</a><sup><a href='SkRect_Reference#SkRect_join_2'>[2]</a></sup>

---

<a name='SkIRect_join_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_join'>join</a>(const <a href='#SkIRect'>SkIRect</a>& r)
</pre>

Sets <a href='#IRect'>IRect</a> to the union of itself and <a href='#SkIRect_join_2_r'>r</a>.

Has no effect if <a href='#SkIRect_join_2_r'>r</a> is empty. Otherwise, if <a href='#IRect'>IRect</a> is empty, sets <a href='#IRect'>IRect</a> to <a href='#SkIRect_join_2_r'>r</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_join_2_r'><code><strong>r</strong></code></a></td>
    <td>expansion <a href='#IRect'>IRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="75fd81c1d3512e63890d085593018876">

#### Example Output

~~~~
join: 10, 20, 55, 65
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_set'>set</a> <a href='SkRect_Reference#SkRect_join'>SkRect::join</a><sup><a href='SkRect_Reference#SkRect_join_2'>[2]</a></sup>

---

## <a name='Sorting'>Sorting</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIRect_sort'>sort</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>orders sides from smaller to larger</td>
  </tr>
</table>

<a name='SkIRect_sort'></a>
## sort

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_sort'>sort</a>()
</pre>

Swaps <a href='#SkIRect_fLeft'>fLeft</a> and <a href='#SkIRect_fRight'>fRight</a> if <a href='#SkIRect_fLeft'>fLeft</a> is greater than <a href='#SkIRect_fRight'>fRight</a>; and swaps
<a href='#SkIRect_fTop'>fTop</a> and <a href='#SkIRect_fBottom'>fBottom</a> if <a href='#SkIRect_fTop'>fTop</a> is greater than <a href='#SkIRect_fBottom'>fBottom</a>. Result may be empty,
and <a href='#SkIRect_width'>width</a> and <a href='#SkIRect_height'>height</a> will be zero or positive.

### Example

<div><fiddle-embed name="fa12547fcfd4c1aef3db1a1f6aae0fe4">

#### Example Output

~~~~
rect: 30, 50, 20, 10
sorted: 20, 10, 30, 50
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_makeSorted'>makeSorted</a> <a href='SkRect_Reference#SkRect_sort'>SkRect::sort</a>

---

<a name='SkIRect_makeSorted'></a>
## makeSorted

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_makeSorted'>makeSorted</a>() const
</pre>

Returns <a href='#IRect'>IRect</a> with <a href='#SkIRect_fLeft'>fLeft</a> and <a href='#SkIRect_fRight'>fRight</a> swapped if <a href='#SkIRect_fLeft'>fLeft</a> is greater than <a href='#SkIRect_fRight'>fRight</a>; and
with <a href='#SkIRect_fTop'>fTop</a> and <a href='#SkIRect_fBottom'>fBottom</a> swapped if <a href='#SkIRect_fTop'>fTop</a> is greater than <a href='#SkIRect_fBottom'>fBottom</a>. Result may be empty;
and <a href='#SkIRect_width'>width</a> and <a href='#SkIRect_height'>height</a> will be zero or positive.

### Return Value

sorted <a href='#IRect'>IRect</a>

### Example

<div><fiddle-embed name="de89926c374aa16427916900b89a3441">

#### Example Output

~~~~
rect: 30, 50, 20, 10
sorted: 20, 10, 30, 50
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_sort'>sort</a> <a href='SkRect_Reference#SkRect_makeSorted'>SkRect::makeSorted</a>

---

<a name='SkIRect_EmptyIRect'></a>
## EmptyIRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static const <a href='#SkIRect'>SkIRect</a>& SK_WARN_UNUSED_RESULT <a href='#SkIRect_EmptyIRect'>EmptyIRect</a>()
</pre>

Returns a reference to immutable empty <a href='#IRect'>IRect</a>, set to (0, 0, 0, 0).

### Return Value

global <a href='#IRect'>IRect</a> set to all zeroes

### Example

<div><fiddle-embed name="65e0b9b52e907902630577941fb3ed6d">

#### Example Output

~~~~
rect: 0, 0, 0, 0
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeEmpty'>MakeEmpty</a>

---

<a name='SkIRect_MakeLargest'></a>
## MakeLargest

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkIRect'>SkIRect</a> SK_WARN_UNUSED_RESULT <a href='#SkIRect_MakeLargest'>MakeLargest</a>()
</pre>

Deprecated.

---

