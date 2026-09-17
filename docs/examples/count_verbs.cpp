// Copyright 2020 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(count_verbs, 256, 256, false, 0) {
#include "include/utils/SkTextUtils.h"

#include <array>

static SkPath make_path(const SkFont& font) {
    SkPath path;
    const char text[] = "SKIA";
    SkTextUtils::GetPath(text, strlen(text), SkTextEncoding::kUTF8, 0, 0, font, &path);
    return path;
}

static void count_verbs(const SkPath& path, int counts[6]) {
    SkPath::Iter it(path, false);
    for (int i = 0; i < 6; ++i) {
        counts[i] = 0;
    }
    while (auto rec = it.next()) {
        counts[(unsigned)rec->fVerb]++;
    }
}

void draw(SkCanvas* canvas) {
    SkFont font(fontMgr->matchFamilyStyle("DejaVu Sans Mono", SkFontStyle()), 30);
    SkPath path = make_path(font);
    std::array<int, 6> counts;
    count_verbs(path, counts.data());

    // output results:
    std::array<const char*, 6> verbs = {"Move", "Line", "Quad", "Conic", "Cubic", "Close"};
    SkPoint pt = SkPoint::Make(10.0f, 5.0f + font.getSpacing());
    SkPaint p;
    canvas->clear(SK_ColorWHITE);
    for (int i = 0; i < 6; ++i) {
        canvas->drawString(SkStringPrintf("%-5s %3d", verbs[i], counts[i]), pt.fX, pt.fY, font,
                           p);
        pt.fY += font.getSpacing();
    }
}
}  // END FIDDLE
