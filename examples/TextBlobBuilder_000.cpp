// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(TextBlobBuilder_000, 256, 256, true, 0);
// HASH=d9dbabfe24aad92ee3c8144513e90d81
void draw(SkCanvas* canvas) {
    SkTextBlobBuilder builder;
    sk_sp<SkTextBlob> blob = builder.make();
    SkDebugf("blob " "%s" " nullptr", blob == nullptr ? "equals" : "does not equal");
}
}
