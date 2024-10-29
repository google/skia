// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/fiddle/examples.h"

#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
#include "include/ports/SkFontMgr_fontconfig.h"
#endif

#if defined(SK_FONTMGR_CORETEXT_AVAILABLE)
#include "include/ports/SkFontMgr_mac_ct.h"
#endif

#if defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE)
#include "include/ports/SkTypeface_win.h"
#endif

#if defined(SK_FONTMGR_FREETYPE_DIRECTORY_AVAILABLE)
#include "include/ports/SkFontMgr_directory.h"
#endif

template sk_tools::Registry<fiddle::Example>* sk_tools::Registry<fiddle::Example>::gHead;

// These globals are needed by fiddles:
GrBackendTexture backEndTexture;
GrBackendRenderTarget backEndRenderTarget;
GrBackendTexture backEndTextureRenderTarget;
SkBitmap source;
sk_sp<SkImage> image;
double duration = 1.0;
double frame = 1.0;
sk_sp<SkFontMgr> fontMgr;

int main() {
    constexpr int kImgCount = 7;
    sk_sp<SkImage> images[kImgCount];
    SkBitmap bitmaps[kImgCount];
    for (int i = 1; i < kImgCount; ++i) {
        SkString path = SkStringPrintf("resources/images/example_%d.png", i);
        images[i] = SkImages::DeferredFromEncodedData(SkData::MakeFromFileName(path.c_str()));
        SkAssertResult(images[i] && images[i]->asLegacyBitmap(&bitmaps[i]));
    }
#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
    fontMgr = SkFontMgr_New_FontConfig(nullptr);
#elif defined(SK_FONTMGR_CORETEXT_AVAILABLE)
    fontMgr = SkFontMgr_New_CoreText(nullptr);
#elif defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE)
    fontMgr = SkFontMgr_New_DirectWrite();
#elif defined(SK_FONTMGR_FREETYPE_DIRECTORY_AVAILABLE)
    fontMgr = SkFontMgr_New_Custom_Directory("/usr/share/fonts/");
#else
    #error "Unsupported OS"
#endif
    for (const fiddle::Example& example : sk_tools::Registry<fiddle::Example>::Range()) {
        SkASSERT((unsigned)example.fImageIndex < (unsigned)kImgCount);
        image = images[example.fImageIndex];
        source = bitmaps[example.fImageIndex];
        SkBitmap bmp;
        bmp.allocN32Pixels(example.fWidth, example.fHeight);
        bmp.eraseColor(SK_ColorWHITE);
        SkCanvas canvas(bmp);
        SkDebugf("==> %s\n", example.fName);
        example.fFunc(&canvas);
    }
}
