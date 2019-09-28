/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/svg/SkSVGCanvas.h"

#include "include/c/sk_svg.h"

#include "src/c/sk_types_priv.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_canvas_t* sk_svgcanvas_create(const sk_rect_t* bounds, sk_xmlwriter_t* writer) {
    return ToCanvas(SkSVGCanvas::Make(*AsRect(bounds), AsXMLWriter(writer)).release());
}
