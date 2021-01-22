Skia Color Management
=====================

What we mean by color management
--------------------------------

All the color spaces Skia works with describe themselves by how to transform
colors from that color space to a common "connection" color space called XYZ
D50.  And we can infer from that same description how to transform from that
XYZ D50 space back to the original color space.  XYZ D50 is a color space
represented in three dimensions like RGB, but the XYZ parts are not RGB-like at
all, rather a linear remix of those channels.  Y is closest to what you'd think
of as brightness, but X and Z are a little more abstract.  It's kind of like
YUV if you're familiar with that.  The "D50" part refers to the whitepoint of
this space, around 5000 Kelvin.

All color managed drawing is divided into six parts, three steps connecting the
source colors to that XYZ D50 space, then three symmetric steps connecting back
from XYZ D50 to the destination color space.  Some of these steps can
annihilate with each other into no-ops, sometimes all the way to the entire
process amounting to a no-op when the source space and destination space are
the same.  Here are the steps:

Color management steps
----------------------

1. unpremultiply if the source color is premultiplied  -- alpha is not involved
   in color management, and we need to divide it out if it's multiplied in
2. linearize the source color using the source color space's transfer function
3. convert those unpremultiplied, linear source colors to XYZ D50 gamut by
   multiplying by a 3x3 matrix
4. convert those XYZ D50 colors to the destination gamut by multiplying by a 3x3 matrix
5. encode that color using the inverse of the destination color space's transfer function
6. premultiply by alpha if the destination is premultiplied

If you poke around in our code the clearest place to see this logic is in a
type called SkColorSpaceXformSteps.  You'll see it as 5 steps there: we always
merge the innermost two operations into a single 3x3 matrix multiply.

Optimizations
-------------

Whenever we're about to do some drawing we look at which of those steps we
really need to do.  Any step that's a fundamental no-op we skip:

   * skip 1 if the source is already unpremultiplied
   * skip 2 if the source is already linearly encoded
   * skip 3 and 4 if that single concatenated matrix is identity (i.e. the
     source and destination color spaces have the same gamut)
   * skip 5 if the destination wants linear encoding
   * skip 6 if the destination wants to be unpremultiplied

We can reason from those basic skips into some more advanced optimizations:

  * if we've skipped 3 and 4 already, we can skip 2 and 5 any time the transfer
    functions are the same  -- sending colors through a given transfer function
    and its own inverse is a no-op
  * if we've skipped all of 2-5, we can skip 1 and 6 if we were going to do
    both --- no sense in unpremultiplying just to re-premultiply.
  * opaque colors can be treated as either unpremultiplied or premultiplied,
    whichever lets us skip more steps.

All this comes together to an impressive "nothing to do" most of the time.  If
you're drawing opaque colors in a given color space to a destination tagged
with that same color space, we'll notice we can skip all six steps.  Sometimes
fewer steps are needed, sometimes more.  In general if you need to do a gamut
conversion, you should generally expect all the middle steps to be active.
Steps 2 and 5 are by far the most expensive to compute.

nullptr SkColorSpace defaults
-----------------------------

Now how do nullptr SkColorSpace defaults work into all of this?  We preface all
that logic I've just mentioned above with this little snippet:

     if (srcCS == nullptr) { srcCS = sRGB; }
     if (dstCS == nullptr) { dstCS = srcCS; }

(Order matters there.)  The gist is, we assume any untagged sources are sRGB.
And if you leave your surface untagged, we act as if your destination fluidly
matches whatever source you're trying to draw into it, which skips at least
steps 2-5 as listed above, maintaining an unmanaged color mode of drawing
compatible with how retro Skia used to work before we introduce color
management.  It's not very principled, but it's handy in practice to keep
around.
