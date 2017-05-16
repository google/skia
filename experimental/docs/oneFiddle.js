function testFiddles() {
    var SkCanvas_drawAnnotation_code =
    "void draw(SkCanvas* canvas) {\n" +
    "    const char text[] = \"Click this link!\";\n" +
    "    SkRect bounds;\n" +
    "    SkPaint paint;\n" +
    "    paint.setTextSize(40);\n" +
    "    (void)paint.measureText(text, strlen(text), &bounds);\n" +
    "    const char url[] = \"https://www.google.com/\";\n" +
    "    sk_sp<SkData> urlData(SkData::MakeWithCString(url));\n" +
    "    canvas->drawAnnotation(bounds, \"url_key\", urlData.get());\n" +
    "}\n";

    var SkCanvas_drawAnnotation_json = {
        "code": SkCanvas_drawAnnotation_code,
        "options": {
            "width": 256,
            "height": 1,
            "source": 0,
            "textOnly": false
        },
        "name": "SkCanvas_drawAnnotation",
        "overwrite": true
    }

    runFiddle(SkCanvas_drawAnnotation_json);

    var SkCanvas_drawAnnotation_2_code =
    "void draw(SkCanvas* canvas) {\n" +
    "    const char text[] = \"Click this link!\";\n" +
    "    SkRect bounds;\n" +
    "    SkPaint paint;\n" +
    "    paint.setTextSize(40);\n" +
    "    (void)paint.measureText(text, strlen(text), &bounds);\n" +
    "    const char url[] = \"https://www.google.com/\";\n" +
    "    sk_sp<SkData> urlData(SkData::MakeWithCString(url));\n" +
    "    canvas->drawAnnotation(bounds, \"url_key\", urlData.get());\n" +
    "}\n";

    var SkCanvas_drawAnnotation_2_json = {
        "code": SkCanvas_drawAnnotation_2_code,
        "options": {
            "width": 256,
            "height": 1,
            "source": 0,
            "textOnly": false
        },
        "name": "SkCanvas_drawAnnotation_2",
        "overwrite": true
    }

    runFiddle(SkCanvas_drawAnnotation_2_json);
}
