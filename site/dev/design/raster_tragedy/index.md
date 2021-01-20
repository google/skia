The Raster Tragedy in Skia
==========================

This is an extension of [The Raster Tragedy at Low-Resolution Revisited](http://rastertragedy.com)
as it applies to Skia. The Raster Tragedy describes a number of issues with typeface rasterization
with a particular emphasis on proper hinting to overcome these issues. Since not all fonts
are nicely hinted and sometimes hinting is not desired, there are additional hacks which may
be applied. Generally, one wants to hint purely informational text laid out for a particular
device, but not hint text which is art. Unless, of course, the hinting is part of the art like
Shift_JIS art.



The Gamma Hack
--------------

First, one should be aware of transfer functions (of which 'gamma' is an
example). A good introduction can be had at [What Every Coder Should Know About
Gamma](https://blog.johnnovak.net/2016/09/21/what-every-coder-should-know-about-gamma/).

In Skia, all color sources are converted into the destination color space and the blending is done
in the destination color space by applying the linear blend function. Skia does not convert into
a linear space, apply the linear blend, and convert back to the encoded space. If the destination
color space does not have a linear encoding this will lead to 'incorrect' blending. The idea is
that there are essentially two kinds of users of Skia. First there are existing systems which
are already using a non-linear encoding with a linear blend function. While the blend isn't
correct, these users generally don't want anything to change due to expectations. Second there
are those who want everything done correctly and they are willing to pay for a linearly encoded
destination in which the linear blend function is correct.

For bi-level glyph rendering a pixel is either covered or not, so there are no coverage blending
issues.

For regular full pixel partial coverage (anti-aliased) glyph rendering the user may or may not
want correct linear blending. In most non-linear encodings, using the linear blend function
tends to make black on white look slightly heavier, using the pixel grid as a kind of contrast
and optical sizing enhancement. It does the opposite for white on black, often making such
glyphs a bit under-covered. However, this fights the common issue of blooming where light on
dark on many displays tends to appear thicker than dark on light. (The black not being fully
black also contributes.) If the pixels are small enough and there is proper optical sizing and
perhaps anti-aliased drop out control (these latter two achieved either manually with proper
font selection or 'opsz', automatically, or through hinting) then correct linear blending tends
to look great. Otherwise black on white text tends to (correctly) get really anemic looking at
small sizes. So correct blending of glyph masks here should be left up to the user of Skia. If
they're really sophisticated and already tackled these issues then they may want linear blending
of the glyphs for best effect. Otherwise the glyphs should just keep looking like they used to
look due to expectations.

For subpixel partial coverage (subpixel anti-aliased) glyph masks linear blending in a
linear encoding is more or less required to avoid color fringing effects. The intensity of
the subpixels is being directly exploited so needs to be carefully controlled. The subpixels
tend to alleviate the issues with no full coverage (though still problematic if blitting text
in one of the display's primaries). One will still want optical sizing since the glyphs will
still look somewhat too light when scaled down linearly.

So, if subpixel anti-aliased glyph masks (and sometimes full pixel anti-aliased glyph masks)
need a correct blit how are they to be used with non-linearly encoded destinations?

One possible solution is to special case these blits. If blitting on the CPU it's often fast and
close enough to take the square root of the source and destination values, do the linear blend
function, then square the result (approximating the destination encoding as if its transfer
function is square). Many GPUs have a mode where they can blend in sRGB, though unfortunately
this generally applies to the whole framebuffer, not just individual draws. For various reasons,
Skia has avoided special casing these blends.

What Skia currently does is the gamma hack. When creating the glyph mask one usually knows
the approximate color which is going to be drawn, the transfer function, and that a linear
source-over blend is going to be used. The destination color is then guessed to be a contrasting
color (if there isn't any contrast the drawing won't be able to be seen anyway) so assume that the
destination color will be the perceptually opposite color in the destination color space. One can
now determine the desired value by converting the perceptual source and guessed destination into
a linear encoding, do the linear source-over blend, and convert to the destination encoding. The
coverage is then adjusted so that the result of a linear source-over blend on the non-linear
encoded values will be as close as possible to this desired value.

This works, but makes many assumptions. The first is the guess at the destination. A perceptual
middle gray could equally well (or poorly) contrast with black or white so the best guess
is drawing it on top of itself. Subpixel anti-aliased glyph masks drawn with this guess will
be drawn without any adjustment at all, leaving them color fringy. On macOS Skia tweaks the
destination guess to reduce the correction when using bright sources. This helps reducing
'blooming' issues with light on dark (aka dark mode) and better matches how CoreText seems
to do blending. The second is that a src-over blend is assumed, but users generally aren't as
discriminating of the results of other blends.

The gamma hack works best with subpixel anti-aliasing since the adjustment can be made per-channel
instead of full pixel. If this hack is applied to full pixel anti-aliased masks everything is
essentially being done in the nearest gray instead of the nearest color (since there is only
one channel through which to communicate), leading to poor results. Most users will not want
the gamma hack applied to full pixel anti-aliased glyphs.

Since the gamma hack is logically part of the blend, it must always be the very last adjustment
made to the coverage mask before being used by the blitter. The inputs are the destination
transfer function and the current color (in the destination color space). In Skia these come
from the color space on the SkSurface and the color on the SkPaint.


Optical Sizing Hack
-------------------

In metal type a type designer will draw out on paper the design for the faces of the final
sorts. Often there would be different drawings for different target sizes. Sometimes these
different sizes look quite different (like Display and Text faces of the same family). Then a
punch cutter takes these drawings and creates a piece of metal called a punch which can stamp
out these faces. The punch is used to create a negative in a softer piece of metal forming a
strike. This strike is made the correct width and called a matrix. This matrix is used as a mold
to cast individual sorts. The sorts are collected into a box called a type case. A typesetter
would take sorts from a type case and set them into a form. The printer would then ink the
sorts in the form and press the paper. (Note that the terms 'typeface' and 'font' aren't used
in this description, there is a lot of disagreement on what they apply to.)

Every step of this process is now automated in some way. Unfortunately, knowledge embedded in
the manual process has not always been replicated in the automation. This can be for a wide
variety of reasons, such as being overlooked or being difficult to emulate. One of these areas
is the art of optical sizing and managing thin features.

In general smaller type should be relatively heavier in weight than would be expected if taking
a larger size and linearly scaling it down. The type designer will draw out the faces with this
in mind, potentially with an eye toward how it would vary with size and with potential need
for ink traps. The punch cutter would then cut the punches at a given size with this in mind,
adjusting until the soot proofs looked good. There may even be slight adjustments to the matrix
itself if something seemed off. The typesetter would often know which specific cases contained
slightly heavier or lighter sorts. The printer would then adjust the ink and pressure to get
a good looking print.

Popular digital font formats didn't really support this until recently with the variable font
'opsz' axis. The presence of an 'opsz' axis is like the type designer giving really good
instructions about how the faces should look at various sizes. However, not all fonts have or
will have a 'opsz' axis. Since we don't always have an 'opsz' what can be done to prevent small
glyphs from looking all washed out?

One way the type designer could influence the optical size is through hinting. Any manual
hinting is done by someone looking at the result at small sizes. Tweaks are made until it
'looks good'. This often unconsciously bakes in optical sizing. Even autohinters eventually
end up emulating this, generally making the text more contrasty and somewhat heavier at small
pixel sizes (this depends on the target pixel size not the nominal requested size).

An alternate way the designer could provide optical sized fonts is using multiple font files
for different optical sizes and expect the user to select the right one (like Display and
Text variants). In theory the right one might also be selected automatically based on some
criteria. Switching font files because of nominal size changes may seem drastic, but this is
how the system font on macOS worked for a while.

One automatic way to do optical sizing is to artificially embolden the outline itself at small
nominal sizes. This is the approach taken by CoreText. This is a lot like the dreaded fake-bold,
but doesn't have the issue of the automatic fake bolding being heavier than ultra bold, since
this emboldening is applied uniformally based on requested nominal size.

Another automatic way to do optical sizing is to over-cover all the edges when doing
anti-aliasing. The pixels are a fixed physical size so affect small features more than large
features. Skia currently has rudimentary support for this in its 'contrast hack' which is
described more later.

As a note on optical sizing, this is one place where the nominal or optical size of the font is
treated differently from the final pixel size of the font. A SkCanvas may have a scale transform
active. Any hinting will be done at the final pixel size (the font size mapped through the
current transformation matrix), but the optical size is not affected by the transform since
it's actually a parameter for the SkTypeface (and maybe the SkFont).


The Contrast Hack
-----------------

Consider the example of [a pixel wide stroke at half-pixel
boundaries](http://rastertragedy.com/RTRCh3.htm#Sec1). As stated in section 3.1.3 "if we were
to choose the stroke positions to render the most faithful proportions and advance width, we
may have to compromise the rendering contrast." If the stroke lands on a pixel boundary all
is well. If the stoke lands at a half pixel boundary and correct linear blending is used then
the same number of photons are reaching the eye, but the reduction in photons is spread out
over twice the area. If the pixels were small enough then this wouldn't be an issue. Back of
the envelope suggests an 8K 20inch desktop monitor or ~400ppi being the minimum. Note that RGB
subpixel anti-aliasing brings a 100dpi display up to 300dpi, which is close, but still a bit
short. Exploiting the RGB subpixels also doesn't really increase the resolution as much when
getting close to the RGB primaries for the fill color.

One way to try to compensate for this is to cover some of these partially covered pixels more
until it visually looks better. Note that this depends on a lot of factors like the pixel density
of the display, the visual acuity of the user, the distance of the user from the display, and the
user's sensitivity to the potential variations in stem darkness which may result. Automatically
taking all these factors into account would be quite difficult. The correct function is generally
determined by having the user look at the result of applying various amounts of extra coverage
and having them to pick a setting that looks the least bad to them.

This specific form of over-covering is a form of drop out control for anti-aliasing and could
be implemented in a similar way, detecting when a stem comes on in one pixel and goes out in
the next and mark that for additional coverage. At raster time a cruder approximation could be
made by doing a pass in each of the horizontal and vertical directions and finding runs of more
than one non-fully-covered pixel and increasing their coverage.

If instead of doing these computationally expensive passes all coverage is boosted then in
addition to the smeared stems the entire outside edge of the glyph will also be bolded. Making
these outside edges heavier is a crude approximation of outsetting the initial path in a
rather complicated way and amounts to an optical sizing tweak. Just as hinting can be used to
approximate optical sizing if the user's perception of the pixel sizes is known in advance,
this is a pixel level tweak tied to a specific user and display combination.

Much like the gamma hack can be modified to reduce the correction for light on dark to
fight blooming, the contrast hack can be reduced when the color being drawn is known to be
light. Generally the contrast correction goes to zero as one approaches white.
