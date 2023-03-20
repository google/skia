// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skbug_633528, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    static const char imageData[] =
            "\211PNG\r\n\32\n\0\0\0\rIHDR\0\0\0\202\0\0\0\202\2\3\0\0\0\367m\370"
            "n\0\0\0\tPLTE\0\1\0\0\377\377\375\377\374\301\233\213\345\0\0\0\tp"
            "HYs\0\0\13\23\0\0\13\23\1\0\232\234\30\0\0\0\7tIME\7\340\10\2\22\2"
            " V\23\7<\0\0\0\341IDATX\303\205\3301a\3\1\20\3\301'\21\10)\34\24&\21"
            "h\202\240B\207\322\14<[\17\202}\36\366\363\377\275\367\363K\361\242"
            "\370\263\10E)Fq\26\241(\305(\316\"\24\245\30\305Y\204\242\24\2438\213"
            "P\224b\24g\21\212R\214\342,BQ\212Q\234E(J1\212\263\10E)Fq\26\241(\305"
            "(\316\"\24\245\30\305Y\204\242\24\2438\213P\224b\24g\21\212R\214\342"
            ",BQ\212Q\234E(J1\212\263\10E)Fq\26\241(\305(\316\"\24\245\30\305Y\204"
            "\242\24\2438\213P\224b\24g\21\212R\214\342,BQ\212Q\234E(J1\212\263"
            "\10E)Fq\26\241(\305(\316\"\24\245\30\305Y\204\242\24\2438\213P\224"
            "b\24g\21\212R\214\342,BQ\212Q\334\333\203\344\3v\211\352J5\271\206"
            "*\0\0\0\0IEND\256B`\202";
    auto i = SkImages::DeferredFromEncodedData(SkData::MakeWithoutCopy(imageData, 343));
    canvas->scale(0.99f, 1.01f);
    canvas->clipRect(SkRect::MakeXYWH(64, 64, 128, 128));
    canvas->drawImage(i, 63, 63);
}
}  // END FIDDLE
