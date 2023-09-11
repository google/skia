// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=45bca8747b8f49b5be34b520897ef048
REG_FIDDLE(Image_CrossContextTextureFromPixmap, 256, 64, false, 4) {
    void draw(SkCanvas * canvas) {
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext) {
            return;
        }

        SkPixmap pixmap;
        if (source.peekPixels(&pixmap)) {
            sk_sp<SkImage> image = SkImages::CrossContextTextureFromPixmap(dContext, pixmap, false);
            canvas->drawImage(image, 0, 0);
        }
    }
}  // END FIDDLE
