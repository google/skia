// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(TextBlobBuilder_empty_constructor, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkTextBlobBuilder builder;
    sk_sp<SkTextBlob> blob = builder.make();
    SkDebugf("blob " "%s" " nullptr", blob == nullptr ? "equals" : "does not equal");
}
}  // END FIDDLE
