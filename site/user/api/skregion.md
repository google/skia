SkRegion
========

*Regions - set operations with rectangles*

<!-- Updated Mar 4, 2011 -->

Regions are a highly compressed way to represent (integer) areas. Skia
uses them to represent (internally) the current clip on the
canvas. Regions take their inspiration from the data type with the
same name on the original Macintosh (thank you Bill).

Regions are opaque structures, but they can be queried via
iterators. Best of all, they can be combined with other regions and
with rectangles (which can be thought of as "simple" regions. If you
remember Set operations from math class (intersection, union,
difference, etc.), then you're all ready to use regions.

<!--?prettify lang=cc?-->

    bool SkRegion::isEmpty();
    bool SkRegion::isRect();
    bool SkRegion::isComplex();

Regions can be classified into one of three types: empty, rectangular,
or complex.

Empty regions are just that, empty. All empty regions are equal (using
operator==). Compare this to rectangles (SkRect or SkIRect). Any
rectangle with fLeft >= fRight or fTop >= fBottom is consider empty,
but clearly there are different empty rectangles that are not equal.

<!--?prettify lang=cc?-->

    SkRect a = { 0, 0, 0, 0 };
    SkRect b = { 1, 1, 1, 1 };

Both a and b are empty, but they are definitely not equal to each
other. However, with regions, all empty regions are equal. If you
query its bounds, you will always get { 0, 0, 0, 0 }. Even if you
translate it, it will still be all zeros.

<!--?prettify lang=cc?-->

<!--?prettify lang=cc?-->

    SkRegion a, b;   // regions default to empty
    assert(a == b);
    a.offset(10, 20);
    assert(a == b);
    assert(a.getBounds() == { 0, 0, 0, 0 });   // not legal C++, but you get the point
    assert(b.getBounds() == { 0, 0, 0, 0 });

To initialize a region to something more interesting, use one of the
set() methods

<!--?prettify lang=cc?-->

    SkRegion a, b;
    a.setRect(10, 10, 50, 50);
    b.setRect(rect);    // see SkIRect
    c.setPath(path);   // see SkPath

This is the first step that SkCanvas performs when one of its
clip...() methods are called. The clip data is first transformed into
device coordinates (see SkMatrix), and then a region is build from the
data (either a rect or a path). The final step is to combine this new
region with the existing clip using the specified operator.

<!--?prettify lang=cc?-->

    enum Op {
        kUnion_Op,
        kIntersect_Op,
        kDifference_Op,
        kXor_Op,
        kReverseDifference_Op,
        kReplace_Op
    };

By default, intersect op is used when a clip call is made, but the
other operators are equally valid.

<!--?prettify lang=cc?-->

    // returns true if the resulting clip is non-empty (i.e. drawing can
    // still occur)
    bool SkCanvas::clipRect(const SkRect& rect, SkRegion::Op op) {
        SkRegion rgn;
    
        // peek at the CTM (current transformation matrix on the canvas)
        const SkMatrix& m = this->getTotalMatrix();
    
        if (m.rectStaysRect()) {    // check if a transformed rect can be
                                    // represented as another rect

            SkRect deviceRect;
            m.mapRect(&deviceRect, rect);
            SkIRect intRect;
            deviceRect.round(&intRect);
            rgn.setRect(intRect);
        } else {  // matrix rotates or skew (or is perspective)
            SkPath path;
            path.addRect(rect);
            path.transform(m);
            rgn.setPath(path);
        }
    
        // now combine the new region with the current one, using the specified *op*
        return fCurrentClip.op(rgn, op);
    }
