SkRect
======

*Rectangles*

<!--Updated Mar 4, 2011-->

SkRect is basic to many drawing and measuring operations. It can be
drawn using canvas.drawRect(), but it is also used to return the
bounds of objects like paths and text characters. It is specified
using SkScalar values.

SkIRect is the integer counter part to SkRect, but is specified using
32bit integers.

<!--?prettify lang=cc?-->

    struct SkRect {
       SkScalar fLeft;
       SkScalar fTop;
       SkScalar fRight;
       SkScalar fBottom;
       // methods
    };

    SkRect rect = SkRect::MakeLTRB(left, top, right, bottom);

SkRect has the usual getters, to return width(), height(), centerX(),
etc. It also has methods to compute unions and intersections between
rectangles.

Converting between SkRect and SkIRect is asymetric. Short of overflow
issues when SkScalar is an int, converting from SkIRect to SkRect is
straight forward:

<!--?prettify lang=cc?-->

    SkRect::set(const SkIRect&);

However, convert from SkRect to SkIRect needs to know how to go from
fractional values to integers.

<!--?prettify lang=cc?-->

    SkRect::round(SkIRect*) const;     // Round each coordinate.
    SkRect::roundOut(SkIRect*) const;  // Apply floor to left/top,
                                       // and ceil to right/bottom.

In Skia, rectangle coordinates describe the boundary of what is drawn,
such that an empty rectangle encloses zero pixels:

bool SkRect::isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }

<!--?prettify lang=cc?-->

    SkScalar SkRect::width() const { return fRight - fLeft; }

    SkScalar SkRect::height() const { return fBottom - fTop; }

    bool SkRect::contains(SkScalar x, SkScalar y) const {
        return fLeft <= x && x < fRight && fTop <= y && y < fBottom;
    }

Thus, to draw a single pixel (assuming no matrix on the canvas), the
rectangle should be initialized as follows:

<!--?prettify lang=cc?-->

    SkRect r = SkRect::MakeXYWH(x, y, SkIntToScalar(1), SkIntToScalar(1));

The same conventions hold for the integer counterpart: SkIRect. This
also dovetails with SkRegion, which has the same model for set
membership, and which uses SkIRect.
