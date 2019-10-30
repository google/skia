// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=b0d48a6e949db1cb545216ae9c3c3c70
REG_FIDDLE(Path_Iter_isClosedContour, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   for (bool forceClose : { false, true } ) {
       SkPath path;
       path.conicTo(1, 2, 3, 4, .5f);
       SkPath::Iter iter(path, forceClose);
       SkDebugf("without close(), forceClose is %s: isClosedContour returns %s\n",
           forceClose ? "true " : "false", iter.isClosedContour() ? "true" : "false");
       path.close();
       iter.setPath(path, forceClose);
       SkDebugf("with close(),    forceClose is %s: isClosedContour returns %s\n",
           forceClose ? "true " : "false", iter.isClosedContour() ? "true" : "false");
    }
}
}  // END FIDDLE
