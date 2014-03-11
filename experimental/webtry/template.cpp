#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkImageInfo.h"
#include "SkForceLinking.h"

int main() {
    SkForceLinking(false);
    SkGraphics::Init();

    SkImageInfo info = SkImageInfo::MakeN32(300, 300, kPremul_SkAlphaType);
    SkBitmap bitmap;
    bitmap.setConfig(info);
    bitmap.allocPixels();
    SkCanvas c(bitmap);
    c.drawColor(SK_ColorWHITE);

    $usercode

    if (!SkImageEncoder::EncodeFile("foo.png", bitmap, SkImageEncoder::kPNG_Type, 100)) {
        printf("Failed to encode\n");
    }
}
