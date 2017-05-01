function testFiddles() {
    var SkCanvas_readPixels_3_code =
    "void draw(SkCanvas* canvas) {\n" +
    "    canvas->clear(0x8055aaff);\n" +
    "    SkBitmap bitmap;\n" +
    "    bitmap.allocPixels(SkImageInfo::MakeN32Premul(1, 1));\n" +
    "    canvas->readPixels(bitmap, 0, 0);\n" +
    "    SkDebugf(\"pixel = %08x\\n\", bitmap.getAddr32(0, 0)[0]);\n" +
    "}\n";

    var SkCanvas_readPixels_3_json = {
        "code": SkCanvas_readPixels_3_code,
        "options": {
            "width": 256,
            "height": 256,
            "source": 0,
            "textOnly": true
        },
        "name": "SkCanvas_readPixels_3",
        "overwrite": true
    }

    runFiddle(SkCanvas_readPixels_3_json);
}
