/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGCanvas_DEFINED
#define SkSVGCanvas_DEFINED

#include "SkCanvas.h"

class SkXMLWriter;

class SK_API SkSVGCanvas {
public:
    /**
     *  Returns a new canvas that will generate SVG commands from its draw calls, and send
     *  them to the provided xmlwriter. Ownership of the xmlwriter is not transfered to the canvas,
     *  but it must stay valid during the lifetime of the returned canvas.
     *
     *  The canvas may buffer some drawing calls, so the output is not guaranteed to be valid
     *  or complete until the canvas instance is deleted.
     *
     *  The 'bounds' parameter defines an initial SVG viewport (viewBox attribute on the root
     *  SVG element).
     */
    static SkCanvas* Create(const SkRect& bounds, SkXMLWriter*);
};

#endif
