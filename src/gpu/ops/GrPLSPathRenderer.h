/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPLSPathRenderer_DEFINED
#define GrPLSPathRenderer_DEFINED

#include "GrPathRenderer.h"

/*
 * Renders arbitrary antialiased paths using pixel local storage as a scratch buffer. The overall
 * technique is very similar to the approach presented in "Resolution independent rendering of
 * deformable vector objects using graphics hardware" by Kokojima et al.

 * We first render the straight-line portions of the path (essentially pretending as if all segments
 * were kLine_Verb) as a triangle fan, using a fragment shader which updates the winding counts
 * appropriately. We then render the curved portions of the path using a Loop-Blinn shader which
 * calculates which portion of the triangle is covered by the quad (conics and cubics are split down
 * to quads). Where we diverge from Kokojima is that, instead of rendering into the stencil buffer
 * and using built-in MSAA to handle straight-line antialiasing, we use the pixel local storage area
 * and calculate the MSAA ourselves in the fragment shader. Essentially, we manually evaluate the
 * coverage of each pixel four times, storing four winding counts into the pixel local storage area,
 * and compute the final coverage based on those winding counts.
 *
 * Our approach is complicated by the need to perform antialiasing on straight edges as well,
 * without relying on hardware MSAA. We instead bloat the triangles to ensure complete coverage,
 * pass the original (un-bloated) vertices in to the fragment shader, and then have the fragment
 * shader use these vertices to evaluate whether a given sample is located within the triangle or
 * not. This gives us MSAA4 edges on triangles which line up nicely with no seams. We similarly face
 * problems on the back (flat) edges of quads, where we have to ensure that the back edge is
 * antialiased in the same way. Similar to the triangle case, we pass in the two (unbloated)
 * vertices defining the back edge of the quad and the fragment shader uses these vertex coordinates
 * to discard samples falling on the other side of the quad's back edge.
 */
class GrPLSPathRenderer : public GrPathRenderer {
public:
    GrPLSPathRenderer();

    bool onCanDrawPath(const CanDrawPathArgs& args) const override;

protected:
    bool onDrawPath(const DrawPathArgs& args) override;
};

#endif
