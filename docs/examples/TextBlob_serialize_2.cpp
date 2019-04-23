// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=464201a828f7e94fc01cd57facfcd2f4
REG_FIDDLE(TextBlob_serialize_2, 256, 24, false, 0) {
#include "include/core/SkSerialProcs.h"

void draw(SkCanvas* canvas) {
    SkFont blobFont;
    blobFont.setSize(24);
    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromText("Hello World", 11, blobFont);
    sk_sp<SkData> data = blob->serialize(SkSerialProcs());
    sk_sp<SkTextBlob> copy = SkTextBlob::Deserialize(data->data(), data->size(), SkDeserialProcs());
    canvas->drawTextBlob(copy, 20, 20, SkPaint());
}
}  // END FIDDLE
