#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=68b6d0208eb0b4de67fc152381af7a58
REG_FIDDLE(TextBlob_Deserialize, 256, 24, false, 0) {
#include "SkSerialProcs.h"

void draw(SkCanvas* canvas) {
    SkFont blobFont;
    blobFont.setSize(24);
    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromText("Hello World!", 12, blobFont);
    sk_sp<SkData> data = blob->serialize(SkSerialProcs());
    uint16_t glyphs[6];
    SkPaint blobPaint;
    blobPaint.textToGlyphs("Hacker", 6, glyphs);
    memcpy((char*)data->writable_data() + 0x54, glyphs, sizeof(glyphs));
    sk_sp<SkTextBlob> copy = SkTextBlob::Deserialize(data->data(), data->size(), SkDeserialProcs());
    canvas->drawTextBlob(copy, 20, 20, SkPaint());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
