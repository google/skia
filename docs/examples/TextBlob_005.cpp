#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=90ce8c327d407b1faac73baa2ebd0378
REG_FIDDLE(TextBlob_serialize, 256, 64, false, 0) {
#include "SkSerialProcs.h"

void draw(SkCanvas* canvas) {
    SkFont blobFont;
    blobFont.setSize(24);
    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromText("Hello World", 11, blobFont);
    char storage[2048];
    size_t used = blob->serialize(SkSerialProcs(), storage, sizeof(storage));
    sk_sp<SkTextBlob> copy = SkTextBlob::Deserialize(storage, used, SkDeserialProcs());
    canvas->drawTextBlob(copy, 20, 20, SkPaint());
    std::string usage = "size=" + std::to_string(sizeof(storage)) + " used=" + std::to_string(used);
    canvas->drawString(usage.c_str(), 20, 40, SkPaint());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
