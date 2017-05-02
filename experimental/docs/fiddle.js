function testFiddles() {

var SkPaint_empty_constructor_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    #ifndef SkUserConfig_DEFINED\n" +
"    #define SkUserConfig_DEFINED\n" +
"    #define SkPaintDefaults_Flags      0x01   // always enable antialiasing\n" +
"    #define SkPaintDefaults_TextSize   24.f   // double default font size\n" +
"    #define SkPaintDefaults_Hinting    3      // use full hinting\n" +
"    #define SkPaintDefaults_MiterLimit 10.f   // use HTML Canvas miter limit setting\n" +
"    #endif\n" +
"}\n";

var SkPaint_empty_constructor_json = {
    "code": SkPaint_empty_constructor_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_empty_constructor",
    "overwrite": true
}

runFiddle(SkPaint_empty_constructor_json);

var SkPaint_copy_constructor_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1;\n" +
"    paint1.setColor(SK_ColorRED);\n" +
"    SkPaint paint2(paint1);\n" +
"    paint2.setColor(SK_ColorBLUE);\n" +
"    SkDebugf(\"SK_ColorRED %c= paint1.getColor()\\n\", SK_ColorRED == paint1.getColor() ? '=' : '!');\n" +
"    SkDebugf(\"SK_ColorBLUE %c= paint2.getColor()\\n\", SK_ColorBLUE == paint2.getColor() ? '=' : '!');\n" +
"}\n";

var SkPaint_copy_constructor_json = {
    "code": SkPaint_copy_constructor_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_copy_constructor",
    "overwrite": true
}

runFiddle(SkPaint_copy_constructor_json);

var SkPaint_move_constructor_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    float intervals[] = { 5, 5 };\n" +
"    paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 2.5f));\n" +
"    SkPaint dashed(std::move(paint));\n" +
"    SkDebugf(\"path effect unique: %s\\n\", dashed.getPathEffect()->unique() ? \"true\" : \"false\");\n" +
"}\n";

var SkPaint_move_constructor_json = {
    "code": SkPaint_move_constructor_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_move_constructor",
    "overwrite": true
}

runFiddle(SkPaint_move_constructor_json);

var SkPaint_copy_assignment_operator_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setColor(SK_ColorRED);\n" +
"    paint2 = paint1;\n" +
"    SkDebugf(\"SK_ColorRED %c= paint1.getColor()\\n\", SK_ColorRED == paint1.getColor() ? '=' : '!');\n" +
"    SkDebugf(\"SK_ColorRED %c= paint2.getColor()\\n\", SK_ColorRED == paint2.getColor() ? '=' : '!');\n" +
"}\n";

var SkPaint_copy_assignment_operator_json = {
    "code": SkPaint_copy_assignment_operator_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_copy_assignment_operator",
    "overwrite": true
}

runFiddle(SkPaint_copy_assignment_operator_json);

var SkPaint_move_assignment_operator_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setColor(SK_ColorRED);\n" +
"    paint2 = std::move(paint1);\n" +
"    SkDebugf(\"SK_ColorRED == paint2.getColor()\\n\", SK_ColorRED == paint2.getColor() ? '=' : '!');\n" +
"}\n";

var SkPaint_move_assignment_operator_json = {
    "code": SkPaint_move_assignment_operator_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_move_assignment_operator",
    "overwrite": true
}

runFiddle(SkPaint_move_assignment_operator_json);

var SkPaint_equal_operator_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setColor(SK_ColorRED);\n" +
"    paint2.setColor(0xFFFF0000);\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 == paint2 ? '=' : '!');\n" +
"    float intervals[] = { 5, 5 };\n" +
"    paint1.setPathEffect(SkDashPathEffect::Make(intervals, 2, 2.5f));\n" +
"    paint2.setPathEffect(SkDashPathEffect::Make(intervals, 2, 2.5f));\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 == paint2 ? '=' : '!');\n" +
"}\n";

var SkPaint_equal_operator_json = {
    "code": SkPaint_equal_operator_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_equal_operator",
    "overwrite": true
}

runFiddle(SkPaint_equal_operator_json);

var SkPaint_not_equal_operator_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setColor(SK_ColorRED);\n" +
"    paint2.setColor(0xFFFF0000);\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 == paint2 ? '=' : '!');\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 != paint2 ? '!' : '=');\n" +
"}\n";

var SkPaint_not_equal_operator_json = {
    "code": SkPaint_not_equal_operator_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_not_equal_operator",
    "overwrite": true
}

runFiddle(SkPaint_not_equal_operator_json);

var SkPaint_getHash_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setColor(SK_ColorRED);\n" +
"    paint2.setColor(0xFFFF0000);\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 == paint2 ? '=' : '!');\n" +
"    SkDebugf(\"paint1.getHash() %c= paint2.getHash()\\n\",\n" +
"             paint1.getHash() == paint2.getHash() ? '=' : '!');\n" +
"}\n";

var SkPaint_getHash_json = {
    "code": SkPaint_getHash_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getHash",
    "overwrite": true
}

runFiddle(SkPaint_getHash_json);

var SkPaint_flatten_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    class PaintDumper : public SkWriteBuffer {\n" +
"    public:\n" +
"        bool isCrossProcess() const override { return false; };\n" +
"        void writeByteArray(const void* data, size_t size) override {}\n" +
"        void writeBool(bool value) override {}\n" +
"        void writeScalar(SkScalar value) override {}\n" +
"        void writeScalarArray(const SkScalar* value, uint32_t count) override {}\n" +
"        void writeInt(int32_t value) override {}\n" +
"        void writeIntArray(const int32_t* value, uint32_t count) override {}\n" +
"        void writeUInt(uint32_t value) override {}\n" +
"        void writeString(const char* value) override {}\n" +
"        void writeFlattenable(const SkFlattenable* flattenable) override {}\n" +
"        void writeColorArray(const SkColor* color, uint32_t count) override {}\n" +
"        void writeColor4f(const SkColor4f& color) override {}\n" +
"        void writeColor4fArray(const SkColor4f* color, uint32_t count) override {}\n" +
"        void writePoint(const SkPoint& point) override {}\n" +
"        void writePointArray(const SkPoint* point, uint32_t count) override {}\n" +
"        void writeMatrix(const SkMatrix& matrix) override {}\n" +
"        void writeIRect(const SkIRect& rect) override {}\n" +
"        void writeRect(const SkRect& rect) override {}\n" +
"        void writeRegion(const SkRegion& region) override {}\n" +
"        void writePath(const SkPath& path) override {}\n" +
"        size_t writeStream(SkStream* stream, size_t length) override { return 0; }\n" +
"        void writeBitmap(const SkBitmap& bitmap) override {}\n" +
"        void writeImage(const SkImage*) override {}\n" +
"        void writeTypeface(SkTypeface* typeface) override {}\n" +
"        void writePaint(const SkPaint& paint) override {}\n" +
"        void writeColor(SkColor color) override {\n" +
"            SkDebugf(\"color = 0x%08x\\n\", color);\n" +
"        }\n" +
"    } dumper;\n" +
"    SkPaint paint;\n" +
"    paint.setColor(SK_ColorRED);\n" +
"    paint.flatten(dumper);\n" +
"}\n";

var SkPaint_flatten_json = {
    "code": SkPaint_flatten_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_flatten",
    "overwrite": true
}

runFiddle(SkPaint_flatten_json);

var SkPaint_reset_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setColor(SK_ColorRED);\n" +
"    paint1.reset();\n" +
"    SkDebugf(\"paint1 %c= paint2\", paint1 == paint2 ? '=' : '!');\n" +
"}\n";

var SkPaint_reset_json = {
    "code": SkPaint_reset_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_reset",
    "overwrite": true
}

runFiddle(SkPaint_reset_json);

var SkPaint_getHinting_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"SkPaint::kNormal_Hinting %c= paint.getHinting()\\n\",\n" +
"            SkPaint::kNormal_Hinting == paint.getHinting() ? '=' : ':');\n" +
"}\n";

var SkPaint_getHinting_json = {
    "code": SkPaint_getHinting_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getHinting",
    "overwrite": true
}

runFiddle(SkPaint_getHinting_json);

var SkPaint_setHinting_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint2.setHinting(SkPaint::kNormal_Hinting);\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 == paint2 ? '=' : ':');\n" +
"}\n";

var SkPaint_setHinting_json = {
    "code": SkPaint_setHinting_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setHinting",
    "overwrite": true
}

runFiddle(SkPaint_setHinting_json);

var SkPaint_getFlags_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    SkDebugf(\"(SkPaint::kAntiAlias_Flag & paint.getFlags()) %c= 0\\n\",\n" +
"        SkPaint::kAntiAlias_Flag & paint.getFlags() ? '!' : '=');\n" +
"}\n";

var SkPaint_getFlags_json = {
    "code": SkPaint_getFlags_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getFlags",
    "overwrite": true
}

runFiddle(SkPaint_getFlags_json);

var SkPaint_setFlags_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setFlags((uint32_t) (SkPaint::kAntiAlias_Flag | SkPaint::kDither_Flag));\n" +
"    SkDebugf(\"paint.isAntiAlias()\\n\", paint.isAntiAlias() ? '!' : '=');\n" +
"    SkDebugf(\"paint.isDither()\\n\", paint.isDither() ? '!' : '=');\n" +
"}\n";

var SkPaint_setFlags_json = {
    "code": SkPaint_setFlags_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setFlags",
    "overwrite": true
}

runFiddle(SkPaint_setFlags_json);

var Anti_alias_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.allocN32Pixels(50, 50);\n" +
"    SkCanvas offscreen(bitmap);\n" +
"    SkPaint paint;\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(10);\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    for (bool antialias : { false, true }) {\n" +
"        paint.setColor(antialias ? SK_ColorRED : SK_ColorBLUE);\n" +
"        paint.setAntiAlias(antialias);\n" +
"        bitmap.eraseColor(0);\n" +
"        offscreen.drawLine(5, 5, 15, 30, paint);\n" +
"        canvas->drawLine(5, 5, 15, 30, paint);\n" +
"        canvas->save();\n" +
"        canvas->scale(10, 10);\n" +
"        canvas->drawBitmap(bitmap, antialias ? 12 : 0, 0);\n" +
"        canvas->restore();\n" +
"        canvas->translate(15, 0);\n" +
"    }\n" +
"}\n";

var Anti_alias_json = {
    "code": Anti_alias_code,
    "options": {
        "width": 512,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Anti_alias",
    "overwrite": true
}

runFiddle(Anti_alias_json);

var SkPaint_isAntiAlias_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"paint.isAntiAlias() %c= !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)\\n\",\n" +
"            paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag) ? '=' : '!');\n" +
"    paint.setAntiAlias(true);\n" +
"    SkDebugf(\"paint.isAntiAlias() %c= !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)\\n\",\n" +
"            paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag) ? '=' : '!');\n" +
"}\n";

var SkPaint_isAntiAlias_json = {
    "code": SkPaint_isAntiAlias_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_isAntiAlias",
    "overwrite": true
}

runFiddle(SkPaint_isAntiAlias_json);

var SkPaint_setAntiAlias_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setAntiAlias(true);\n" +
"    paint2.setFlags(paint2.getFlags() | SkPaint::kAntiAlias_Flag);\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 == paint2 ? '=' : '!');\n" +
"}\n";

var SkPaint_setAntiAlias_json = {
    "code": SkPaint_setAntiAlias_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setAntiAlias",
    "overwrite": true
}

runFiddle(SkPaint_setAntiAlias_json);

var Dither_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkBitmap bm16;\n" +
"    bm16.allocPixels(SkImageInfo::Make(20, 10, kRGB_565_SkColorType, kOpaque_SkAlphaType));\n" +
"    SkCanvas c16(bm16);\n" +
"    SkPoint points[] = {{0, 0}, {19, 0}, {20, 0}};\n" +
"    SkColor colors[] = {SK_ColorBLUE, SK_ColorRED, SK_ColorGREEN};\n" +
"    SkPaint paint;\n" +
"    paint.setShader(SkGradientShader::MakeLinear(\n" +
"                     points, colors, nullptr, SK_ARRAY_COUNT(colors),\n" +
"                     SkShader::kClamp_TileMode, 0, nullptr));\n" +
"    paint.setDither(true);\n" +
"    c16.drawPaint(paint);\n" +
"    canvas->scale(12, 12);\n" +
"    canvas->drawBitmap(bm16, 0, 0);\n" +
"    paint.setDither(false);\n" +
"    c16.drawPaint(paint);\n" +
"    canvas->drawBitmap(bm16, 0, 11);\n" +
"}\n";

var Dither_json = {
    "code": Dither_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Dither",
    "overwrite": true
}

runFiddle(Dither_json);

var SkPaint_isDither_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"paint.isDither() %c= !!(paint.getFlags() & SkPaint::kDither_Flag)\\n\",\n" +
"            paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag) ? '=' : '!');\n" +
"    paint.setDither(true);\n" +
"    SkDebugf(\"paint.isDither() %c= !!(paint.getFlags() & SkPaint::kDither_Flag)\\n\",\n" +
"            paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag) ? '=' : '!');\n" +
"}\n";

var SkPaint_isDither_json = {
    "code": SkPaint_isDither_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_isDither",
    "overwrite": true
}

runFiddle(SkPaint_isDither_json);

var SkPaint_setDither_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setDither(true);\n" +
"    paint2.setFlags(paint2.getFlags() | SkPaint::kDither_Flag);\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 == paint2 ? '=' : '!');\n" +
"}\n";

var SkPaint_setDither_json = {
    "code": SkPaint_setDither_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setDither",
    "overwrite": true
}

runFiddle(SkPaint_setDither_json);

var Paint_Device_Text_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkBitmap bitmap;\n" +
"    bitmap.allocN32Pixels(24, 33);\n" +
"    SkCanvas offscreen(bitmap);\n" +
"    offscreen.clear(SK_ColorWHITE);\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setTextSize(20);\n" +
"    for (bool lcd : { false, true }) {\n" +
"        paint.setLCDRenderText(lcd);\n" +
"        for (bool subpixel : { false, true }) {\n" +
"            paint.setSubpixelText(subpixel);\n" +
"            offscreen.drawText(\",,,,\", 4, 0, 4, paint);\n" +
"            offscreen.translate(0, 7);\n" +
"        }\n" +
"    }\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    canvas->drawBitmap(bitmap, 4, 12);\n" +
"    canvas->scale(9, 9);\n" +
"    canvas->drawBitmap(bitmap, 4, -1);\n" +
"}\n";

var Paint_Device_Text_json = {
    "code": Paint_Device_Text_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Device_Text",
    "overwrite": true
}

runFiddle(Paint_Device_Text_json);

var SkPaint_isSubpixelText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"paint.isSubpixelText() %c= !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)\\n\",\n" +
"            paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag) ? '=' : '!');\n" +
"    paint.setSubpixelText(true);\n" +
"    SkDebugf(\"paint.isSubpixelText() %c= !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)\\n\",\n" +
"            paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag) ? '=' : '!');\n" +
"}\n";

var SkPaint_isSubpixelText_json = {
    "code": SkPaint_isSubpixelText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_isSubpixelText",
    "overwrite": true
}

runFiddle(SkPaint_isSubpixelText_json);

var SkPaint_setSubpixelText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setSubpixelText(true);\n" +
"    paint2.setFlags(paint2.getFlags() | SkPaint::kSubpixelText_Flag);\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 == paint2 ? '=' : '!');\n" +
"}\n";

var SkPaint_setSubpixelText_json = {
    "code": SkPaint_setSubpixelText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setSubpixelText",
    "overwrite": true
}

runFiddle(SkPaint_setSubpixelText_json);

var SkPaint_isLCDRenderText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"paint.isLCDRenderText() %c= !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)\\n\",\n" +
"            paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag) ? '=' : '!');\n" +
"    paint.setLCDRenderText(true);\n" +
"    SkDebugf(\"paint.isLCDRenderText() %c= !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)\\n\",\n" +
"            paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag) ? '=' : '!');\n" +
"}\n";

var SkPaint_isLCDRenderText_json = {
    "code": SkPaint_isLCDRenderText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_isLCDRenderText",
    "overwrite": true
}

runFiddle(SkPaint_isLCDRenderText_json);

var SkPaint_setLCDRenderText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setLCDRenderText(true);\n" +
"    paint2.setFlags(paint2.getFlags() | SkPaint::kLCDRenderText_Flag);\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 == paint2 ? '=' : '!');\n" +
"}\n";

var SkPaint_setLCDRenderText_json = {
    "code": SkPaint_setLCDRenderText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setLCDRenderText",
    "overwrite": true
}

runFiddle(SkPaint_setLCDRenderText_json);


var SkPaint_isEmbeddedBitmapText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"paint.isEmbeddedBitmapText() %c=\"\n" +
"            \" !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)\\n\",\n" +
"            paint.isEmbeddedBitmapText() ==\n" +
"            !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag) ? '=' : '!');\n" +
"    paint.setEmbeddedBitmapText(true);\n" +
"    SkDebugf(\"paint.isEmbeddedBitmapText() %c=\"\n" +
"            \" !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)\\n\",\n" +
"            paint.isEmbeddedBitmapText() ==\n" +
"            !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag) ? '=' : '!');\n" +
"}\n";

var SkPaint_isEmbeddedBitmapText_json = {
    "code": SkPaint_isEmbeddedBitmapText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_isEmbeddedBitmapText",
    "overwrite": true
}

runFiddle(SkPaint_isEmbeddedBitmapText_json);

var SkPaint_setEmbeddedBitmapText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setEmbeddedBitmapText(true);\n" +
"    paint2.setFlags(paint2.getFlags() | SkPaint::kEmbeddedBitmapText_Flag);\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 == paint2 ? '=' : '!');\n" +
"}\n";

var SkPaint_setEmbeddedBitmapText_json = {
    "code": SkPaint_setEmbeddedBitmapText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setEmbeddedBitmapText",
    "overwrite": true
}

runFiddle(SkPaint_setEmbeddedBitmapText_json);

var Vertical_Text_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setTextSize(50);\n" +
"    for (bool vertical : { false, true } ) {\n" +
"        paint.setVerticalText(vertical);\n" +
"        canvas->drawText(\"aAlL\", 4, 25, 50, paint);\n" +
"    }\n" +
"}\n";

var Vertical_Text_json = {
    "code": Vertical_Text_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Vertical_Text",
    "overwrite": true
}

runFiddle(Vertical_Text_json);

var SkPaint_isVerticalText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"paint.isVerticalText() %c= !!(paint.getFlags() & SkPaint::kVerticalText_Flag)\\n\",\n" +
"            paint.isVerticalText() == !!(paint.getFlags() & SkPaint::kVerticalText_Flag) ? '=' : '!');\n" +
"    paint.setVerticalText(true);\n" +
"    SkDebugf(\"paint.isVerticalText() %c= !!(paint.getFlags() & SkPaint::kVerticalText_Flag)\\n\",\n" +
"            paint.isVerticalText() == !!(paint.getFlags() & SkPaint::kVerticalText_Flag) ? '=' : '!');\n" +
"}\n";

var SkPaint_isVerticalText_json = {
    "code": SkPaint_isVerticalText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_isVerticalText",
    "overwrite": true
}

runFiddle(SkPaint_isVerticalText_json);

var SkPaint_setVerticalText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setVerticalText(true);\n" +
"    paint2.setFlags(paint2.getFlags() | SkPaint::kVerticalText_Flag);\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 == paint2 ? '=' : '!');\n" +
"}\n";

var SkPaint_setVerticalText_json = {
    "code": SkPaint_setVerticalText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setVerticalText",
    "overwrite": true
}

runFiddle(SkPaint_setVerticalText_json);

var Fake_Bold_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setTextSize(40);\n" +
"    canvas->drawText(\"OjYy_-\", 6, 10, 50, paint);\n" +
"    paint.setFakeBoldText(true);\n" +
"    canvas->drawText(\"OjYy_-\", 6, 10, 90, paint);\n" +
"    // create a custom fake bold by varying the stroke width\n" +
"    paint.setFakeBoldText(false);\n" +
"    paint.setStyle(SkPaint::kStrokeAndFill_Style);\n" +
"    paint.setStrokeWidth(40.f / 48);\n" +
"    canvas->drawText(\"OjYy_-\", 6, 10, 130, paint);\n" +
"}\n";

var Fake_Bold_json = {
    "code": Fake_Bold_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Fake_Bold",
    "overwrite": true
}

runFiddle(Fake_Bold_json);

var SkPaint_isFakeBoldText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"paint.isFakeBoldText() %c= !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)\\n\",\n" +
"            paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag) ? '=' : '!');\n" +
"    paint.setFakeBoldText(true);\n" +
"    SkDebugf(\"paint.isFakeBoldText() %c= !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)\\n\",\n" +
"            paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag) ? '=' : '!');\n" +
"}\n";

var SkPaint_isFakeBoldText_json = {
    "code": SkPaint_isFakeBoldText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_isFakeBoldText",
    "overwrite": true
}

runFiddle(SkPaint_isFakeBoldText_json);

var SkPaint_setFakeBoldText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setFakeBoldText(true);\n" +
"    paint2.setFlags(paint2.getFlags() | SkPaint::kFakeBoldText_Flag);\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 == paint2 ? '=' : '!');\n" +
"}\n";

var SkPaint_setFakeBoldText_json = {
    "code": SkPaint_setFakeBoldText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setFakeBoldText",
    "overwrite": true
}

runFiddle(SkPaint_setFakeBoldText_json);

var SkPaint_isDevKernText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"paint.isDevKernText() %c= !!(paint.getFlags() & SkPaint::kDevKernText_Flag)\\n\",\n" +
"            paint.isDevKernText() == !!(paint.getFlags() & SkPaint::kDevKernText_Flag) ? '=' : '!');\n" +
"    paint.setDevKernText(true);\n" +
"    SkDebugf(\"paint.isDevKernText() %c= !!(paint.getFlags() & SkPaint::kDevKernText_Flag)\\n\",\n" +
"            paint.isDevKernText() == !!(paint.getFlags() & SkPaint::kDevKernText_Flag) ? '=' : '!');\n" +
"}\n";

var SkPaint_isDevKernText_json = {
    "code": SkPaint_isDevKernText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_isDevKernText",
    "overwrite": true
}

runFiddle(SkPaint_isDevKernText_json);

var SkPaint_setDevKernText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint1, paint2;\n" +
"    paint1.setDevKernText(true);\n" +
"    paint2.setFlags(paint2.getFlags() | SkPaint::kDevKernText_Flag);\n" +
"    SkDebugf(\"paint1 %c= paint2\\n\", paint1 == paint2 ? '=' : '!');\n" +
"}\n";

var SkPaint_setDevKernText_json = {
    "code": SkPaint_setDevKernText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setDevKernText",
    "overwrite": true
}

runFiddle(SkPaint_setDevKernText_json);

var Paint_Filter_Quality_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    canvas->scale(.2f, .2f);\n" +
"    for (SkFilterQuality q : { kNone_SkFilterQuality, kLow_SkFilterQuality, \n" +
"                               kMedium_SkFilterQuality, kHigh_SkFilterQuality } ) {\n" +
"        paint.setFilterQuality(q);\n" +
"        canvas->drawImage(image.get(), 0, 0, &paint);\n" +
"        canvas->translate(550, 0);\n" +
"        if (kLow_SkFilterQuality == q) canvas->translate(-1100, 550);\n" +
"    }\n" +
"}\n";

var Paint_Filter_Quality_json = {
    "code": Paint_Filter_Quality_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Filter_Quality",
    "overwrite": true
}

runFiddle(Paint_Filter_Quality_json);

var SkPaint_getFilterQuality_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"kNone_SkFilterQuality %c= paint.getFilterQuality()\\n\",\n" +
"            kNone_SkFilterQuality == paint.getFilterQuality() ? '=' : '!');\n" +
"}\n";

var SkPaint_getFilterQuality_json = {
    "code": SkPaint_getFilterQuality_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getFilterQuality",
    "overwrite": true
}

runFiddle(SkPaint_getFilterQuality_json);

var SkPaint_setFilterQuality_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setFilterQuality(kHigh_SkFilterQuality);\n" +
"    SkDebugf(\"kHigh_SkFilterQuality %c= paint.getFilterQuality()\\n\",\n" +
"            kHigh_SkFilterQuality == paint.getFilterQuality() ? '=' : '!');\n" +
"}\n";

var SkPaint_setFilterQuality_json = {
    "code": SkPaint_setFilterQuality_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setFilterQuality",
    "overwrite": true
}

runFiddle(SkPaint_setFilterQuality_json);

var Paint_Color_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    canvas->clear(SK_ColorWHITE);  // opaque white\n" +
"    paint.setColor(0x8000FF00);  // transparent green\n" +
"    canvas->drawCircle(50, 50, 40, paint);\n" +
"    paint.setARGB(128, 255, 0, 0); // transparent red\n" +
"    canvas->drawCircle(80, 50, 40, paint);\n" +
"    paint.setColor(SK_ColorBLUE);\n" +
"    paint.setAlpha(0x80);\n" +
"    canvas->drawCircle(65, 65, 40, paint);\n" +
"}\n";

var Paint_Color_json = {
    "code": Paint_Color_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Color",
    "overwrite": true
}

runFiddle(Paint_Color_json);

var SkPaint_getColor_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setColor(SK_ColorYELLOW);\n" +
"    SkColor y = paint.getColor();\n" +
"    SkDebugf(\"Yellow is %d%% red, %d%% green, and %d%% blue.\\n\", (int) (SkColorGetR(y) / 2.55f),\n" +
"            (int) (SkColorGetG(y) / 2.55f), (int) (SkColorGetB(y) / 2.55f));\n" +
"}\n";

var SkPaint_getColor_json = {
    "code": SkPaint_getColor_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getColor",
    "overwrite": true
}

runFiddle(SkPaint_getColor_json);

var SkPaint_setColor_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint green1, green2;\n" +
"    unsigned a = 255;\n" +
"    unsigned r = 0;\n" +
"    unsigned g = 255;\n" +
"    unsigned b = 0;\n" +
"    green1.setColor((a << 24) + (r << 16) + (g << 8) + (b << 0));\n" +
"    green2.setColor(0xFF00FF00);\n" +
"    SkDebugf(\"green1 %c= green2\\n\", green1 == green2 ? '=' : '!');\n" +
"}\n";

var SkPaint_setColor_json = {
    "code": SkPaint_setColor_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setColor",
    "overwrite": true
}

runFiddle(SkPaint_setColor_json);

var SkPaint_getAlpha_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"255 %c= paint.getAlpha()\\n\", 255 == paint.getAlpha() ? '=' : '!');\n" +
"}\n";

var SkPaint_getAlpha_json = {
    "code": SkPaint_getAlpha_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getAlpha",
    "overwrite": true
}

runFiddle(SkPaint_getAlpha_json);

var SkPaint_setAlpha_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setColor(0x00112233);\n" +
"    paint.setAlpha(0x44);\n" +
"    SkDebugf(\"0x44112233 %c= paint.getColor()\\n\", 0x44112233 == paint.getColor() ? '=' : '!');\n" +
"}\n";

var SkPaint_setAlpha_json = {
    "code": SkPaint_setAlpha_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setAlpha",
    "overwrite": true
}

runFiddle(SkPaint_setAlpha_json);

var SkPaint_setARGB_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint transRed1, transRed2;\n" +
"    transRed1.setARGB(255 / 2, 255, 0, 0);\n" +
"    transRed2.setColor(SkColorSetARGB(255 / 2, 255, 0, 0));\n" +
"    SkDebugf(\"transRed1 %c= transRed2\", transRed1 == transRed2 ? '=' : '!');\n" +
"}\n";

var SkPaint_setARGB_json = {
    "code": SkPaint_setARGB_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setARGB",
    "overwrite": true
}

runFiddle(SkPaint_setARGB_json);

var SkPaint_getStyle_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"SkPaint::kFill_Style %c= paint.getStyle()\\n\",\n" +
"            SkPaint::kFill_Style == paint.getStyle() ? '=' : '!');\n" +
"}\n";

var SkPaint_getStyle_json = {
    "code": SkPaint_getStyle_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getStyle",
    "overwrite": true
}

runFiddle(SkPaint_getStyle_json);

var SkPaint_setStyle_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setStrokeWidth(5);\n" +
"    SkRegion region;\n" +
"    region.op(140, 10, 160, 30, SkRegion::kUnion_Op);\n" +
"    region.op(170, 40, 190, 60, SkRegion::kUnion_Op);\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.setInfo(SkImageInfo::MakeA8(50, 50), 50);\n" +
"    uint8_t pixels[50][50];\n" +
"    for (int x = 0; x < 50; ++x) {\n" +
"        for (int y = 0; y < 50; ++y) {\n" +
"            pixels[y][x] = (x + y) % 5 ? 0xFF : 0x00;\n" +
"        }\n" +
"    }\n" +
"    bitmap.setPixels(pixels);\n" +
"    for (auto style : { SkPaint::kFill_Style,\n" +
"                        SkPaint::kStroke_Style,\n" +
"                        SkPaint::kStrokeAndFill_Style }) {\n" +
"        paint.setStyle(style);\n" +
"        canvas->drawLine(10, 10, 60, 60, paint);\n" +
"        canvas->drawRect({80, 10, 130, 60}, paint);\n" +
"        canvas->drawRegion(region, paint);\n" +
"        canvas->drawBitmap(bitmap, 200, 10, &paint);\n" +
"        canvas->translate(0, 80);\n" +
"    }\n" +
"}\n";

var SkPaint_setStyle_json = {
    "code": SkPaint_setStyle_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_setStyle",
    "overwrite": true
}

runFiddle(SkPaint_setStyle_json);

var Stroke_Width_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    for (bool antialias : { false, true }) { \n" +
"        paint.setAntiAlias(antialias);\n" +
"        for (int width = 0; width <= 4; ++width) {\n" +
"            SkScalar offset = antialias * 100 + width * 20;\n" +
"            paint.setStrokeWidth(width * 0.25f);\n" +
"            canvas->drawLine(10 + offset,  10, 20 + offset,  60, paint);\n" +
"            canvas->drawLine(10 + offset, 110, 60 + offset, 160, paint);\n" +
"        }\n" +
"    }\n" +
"}\n";

var Stroke_Width_json = {
    "code": Stroke_Width_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Stroke_Width",
    "overwrite": true
}

runFiddle(Stroke_Width_json);

var SkPaint_getStrokeWidth_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"0 %c= paint.getStrokeWidth()\\n\", 0 == paint.getStrokeWidth() ? '=' : '!');\n" +
"}\n";

var SkPaint_getStrokeWidth_json = {
    "code": SkPaint_getStrokeWidth_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getStrokeWidth",
    "overwrite": true
}

runFiddle(SkPaint_getStrokeWidth_json);

var SkPaint_setStrokeWidth_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setStrokeWidth(5);\n" +
"    paint.setStrokeWidth(-1);\n" +
"    SkDebugf(\"5 %c= paint.getStrokeWidth()\\n\", 5 == paint.getStrokeWidth() ? '=' : '!');\n" +
"}\n";

var SkPaint_setStrokeWidth_json = {
    "code": SkPaint_setStrokeWidth_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setStrokeWidth",
    "overwrite": true
}

runFiddle(SkPaint_setStrokeWidth_json);

var Miter_Limit_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPoint pts[] = {{ 10, 50 }, { 110, 80 }, { 10, 110 }};\n" +
"    SkVector v[] = { pts[0] - pts[1], pts[2] - pts[1] };\n" +
"    SkScalar angle1 = SkScalarATan2(v[0].fY, v[0].fX);\n" +
"    SkScalar angle2 = SkScalarATan2(v[1].fY, v[1].fX);\n" +
"    const SkScalar strokeWidth = 20;\n" +
"    SkScalar miterLimit = 1 / SkScalarSin((angle2 - angle1) / 2);\n" +
"    SkScalar miterLength = strokeWidth * miterLimit;\n" +
"    SkPath path;\n" +
"    path.moveTo(pts[0]);\n" +
"    path.lineTo(pts[1]);\n" +
"    path.lineTo(pts[2]);\n" +
"    SkPaint paint;  // set to default kMiter_Join\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeMiter(miterLimit);\n" +
"    paint.setStrokeWidth(strokeWidth);\n" +
"    canvas->drawPath(path, paint);\n" +
"    paint.setStrokeWidth(1);\n" +
"    canvas->drawLine(pts[1].fX - miterLength / 2, pts[1].fY + 50,\n" +
"                     pts[1].fX + miterLength / 2, pts[1].fY + 50, paint);\n" +
"    canvas->translate(200, 0);\n" +
"    miterLimit *= 0.99f;\n" +
"    paint.setStrokeMiter(miterLimit);\n" +
"    paint.setStrokeWidth(strokeWidth);\n" +
"    canvas->drawPath(path, paint);\n" +
"    paint.setStrokeWidth(1);\n" +
"    canvas->drawLine(pts[1].fX - miterLength / 2, pts[1].fY + 50,\n" +
"                     pts[1].fX + miterLength / 2, pts[1].fY + 50, paint);\n" +
"}\n";

var Miter_Limit_json = {
    "code": Miter_Limit_code,
    "options": {
        "width": 512,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Miter_Limit",
    "overwrite": true
}

runFiddle(Miter_Limit_json);

var SkPaint_getStrokeMiter_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"default miter limit == %g\\n\", paint.getStrokeMiter());\n" +
"}\n";

var SkPaint_getStrokeMiter_json = {
    "code": SkPaint_getStrokeMiter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getStrokeMiter",
    "overwrite": true
}

runFiddle(SkPaint_getStrokeMiter_json);

var SkPaint_setStrokeMiter_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setStrokeMiter(8);\n" +
"    paint.setStrokeMiter(-1);\n" +
"    SkDebugf(\"default miter limit == %g\\n\", paint.getStrokeMiter());\n" +
"}\n";

var SkPaint_setStrokeMiter_json = {
    "code": SkPaint_setStrokeMiter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setStrokeMiter",
    "overwrite": true
}

runFiddle(SkPaint_setStrokeMiter_json);

var Stroke_Cap_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(20);\n" +
"    SkPath path;\n" +
"    path.moveTo(30, 30);\n" +
"    path.lineTo(30, 30);\n" +
"    path.moveTo(70, 30);\n" +
"    path.lineTo(90, 40);\n" +
"    for (SkPaint::Cap c : { SkPaint::kButt_Cap, SkPaint::kRound_Cap, SkPaint::kSquare_Cap } ) {\n" +
"        paint.setStrokeCap(c);\n" +
"        canvas->drawPath(path, paint);\n" +
"        canvas->translate(0, 70);\n" +
"    }\n" +
"}\n";

var Stroke_Cap_json = {
    "code": Stroke_Cap_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Stroke_Cap",
    "overwrite": true
}

runFiddle(Stroke_Cap_json);

var SkPaint_getStrokeCap_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"kButt_Cap %c= default stroke cap\\n\",\n" +
"            SkPaint::kButt_Cap == paint.getStrokeCap() ? '=' : '!');\n" +
"}\n";

var SkPaint_getStrokeCap_json = {
    "code": SkPaint_getStrokeCap_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getStrokeCap",
    "overwrite": true
}

runFiddle(SkPaint_getStrokeCap_json);

var SkPaint_setStrokeCap_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setStrokeCap(SkPaint::kRound_Cap);\n" +
"    paint.setStrokeCap((SkPaint::Cap) SkPaint::kCapCount);\n" +
"    SkDebugf(\"kRound_Cap %c= paint.getStrokeCap()\\n\",\n" +
"            SkPaint::kRound_Cap == paint.getStrokeCap() ? '=' : '!');\n" +
"}\n";

var SkPaint_setStrokeCap_json = {
    "code": SkPaint_setStrokeCap_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setStrokeCap",
    "overwrite": true
}

runFiddle(SkPaint_setStrokeCap_json);

var Stroke_Join_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(20);\n" +
"    SkPath path;\n" +
"    path.moveTo(30, 30);\n" +
"    path.lineTo(40, 50);\n" +
"    path.conicTo(70, 30, 100, 30, .707f);\n" +
"    for (SkPaint::Join j : { SkPaint::kMiter_Join, SkPaint::kRound_Join, SkPaint::kBevel_Join } ) {\n" +
"        paint.setStrokeJoin(j);\n" +
"        canvas->drawPath(path, paint);\n" +
"        canvas->translate(0, 70);\n" +
"    }\n" +
"}\n";

var Stroke_Join_json = {
    "code": Stroke_Join_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Stroke_Join",
    "overwrite": true
}

runFiddle(Stroke_Join_json);

var SkPaint_Join_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPath path;\n" +
"    path.moveTo(10, 50);\n" +
"    path.quadTo(35, 110, 60, 210);\n" +
"    path.quadTo(105, 110, 130, 10);\n" +
"    SkPaint paint;  // set to default kMiter_Join\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(20);\n" +
"    canvas->drawPath(path, paint);\n" +
"    canvas->translate(150, 0);\n" +
"    paint.setStrokeJoin(SkPaint::kBevel_Join);\n" +
"    canvas->drawPath(path, paint);\n" +
"    canvas->translate(150, 0);\n" +
"    paint.setStrokeJoin(SkPaint::kRound_Join);\n" +
"    canvas->drawPath(path, paint);\n" +
"}\n";

var SkPaint_Join_json = {
    "code": SkPaint_Join_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_Join",
    "overwrite": true
}

runFiddle(SkPaint_Join_json);

var SkPaint_getStrokeJoin_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"kMiter_Join %c= default stroke join\\n\",\n" +
"            SkPaint::kMiter_Join == paint.getStrokeJoin() ? '=' : '!');\n" +
"}\n";

var SkPaint_getStrokeJoin_json = {
    "code": SkPaint_getStrokeJoin_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getStrokeJoin",
    "overwrite": true
}

runFiddle(SkPaint_getStrokeJoin_json);

var SkPaint_setStrokeJoin_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setStrokeJoin(SkPaint::kMiter_Join);\n" +
"    paint.setStrokeJoin((SkPaint::Join) SkPaint::kJoinCount);\n" +
"    SkDebugf(\"kMiter_Join %c= paint.getStrokeJoin()\\n\",\n" +
"            SkPaint::kMiter_Join == paint.getStrokeJoin() ? '=' : '!');\n" +
"}\n";

var SkPaint_setStrokeJoin_json = {
    "code": SkPaint_setStrokeJoin_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setStrokeJoin",
    "overwrite": true
}

runFiddle(SkPaint_setStrokeJoin_json);

var SkPaint_getFillPath_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint strokePaint;\n" +
"    strokePaint.setAntiAlias(true);\n" +
"    strokePaint.setStyle(SkPaint::kStroke_Style);\n" +
"    strokePaint.setStrokeWidth(.1f);\n" +
"    SkPath strokePath;\n" +
"    strokePath.moveTo(.08f, .08f);\n" +
"    strokePath.quadTo(.09f, .08f, .17f, .17f);\n" +
"    SkPath fillPath;\n" +
"    SkPaint outlinePaint(strokePaint);\n" +
"    outlinePaint.setStrokeWidth(2);\n" +
"    SkMatrix scale = SkMatrix::MakeScale(300, 300);\n" +
"    for (SkScalar precision : { 0.01f, .1f, 1.f, 10.f, 100.f } ) {\n" +
"        strokePaint.getFillPath(strokePath, &fillPath, nullptr, precision);\n" +
"        fillPath.transform(scale);\n" +
"        canvas->drawPath(fillPath, outlinePaint);\n" +
"        canvas->translate(60, 0);\n" +
"        if (1.f == precision) canvas->translate(-180, 100);\n" +
"    }\n" +
"    strokePath.transform(scale);\n" +
"    strokePaint.setStrokeWidth(30);\n" +
"    canvas->drawPath(strokePath, strokePaint);\n" +
"}\n";

var SkPaint_getFillPath_json = {
    "code": SkPaint_getFillPath_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_getFillPath",
    "overwrite": true
}

runFiddle(SkPaint_getFillPath_json);

var SkPaint_getFillPath_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(10);\n" +
"    SkPath strokePath;\n" +
"    strokePath.moveTo(20, 20);\n" +
"    strokePath.lineTo(100, 100);\n" +
"    canvas->drawPath(strokePath, paint);\n" +
"    SkPath fillPath;\n" +
"    paint.getFillPath(strokePath, &fillPath);\n" +
"    paint.setStrokeWidth(2);\n" +
"    canvas->translate(40, 0);\n" +
"    canvas->drawPath(fillPath, paint);\n" +
"}\n";

var SkPaint_getFillPath_2_json = {
    "code": SkPaint_getFillPath_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_getFillPath_2",
    "overwrite": true
}

runFiddle(SkPaint_getFillPath_2_json);

var Paint_Shader_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint;\n" +
"   SkPoint center = { 50, 50 };\n" +
"   SkScalar radius = 50;\n" +
"   const SkColor colors[] = { 0xFFFFFFFF, 0xFF000000 };\n" +
"   paint.setShader(SkGradientShader::MakeRadial(center, radius, colors,\n" +
"        nullptr, SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode));\n" +
"   for (SkScalar a : { 0.3f, 0.6f, 1.0f } ) {\n" +
"       paint.setAlpha((int) (a * 255));\n" +
"       canvas->drawCircle(center.fX, center.fY, radius, paint);\n" +
"       canvas->translate(70, 70);\n" +
"   }\n" +
"}\n";

var Paint_Shader_json = {
    "code": Paint_Shader_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Shader",
    "overwrite": true
}

runFiddle(Paint_Shader_json);

var Paint_Shader_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint;\n" +
"   SkBitmap bitmap;\n" +
"   bitmap.setInfo(SkImageInfo::MakeA8(5, 1), 5);  // bitmap only contains alpha\n" +
"   uint8_t pixels[5] = { 0x22, 0x55, 0x88, 0xBB, 0xFF };\n" +
"   bitmap.setPixels(pixels);\n" +
"   paint.setShader(SkShader::MakeBitmapShader(bitmap, \n" +
"            SkShader::kMirror_TileMode, SkShader::kMirror_TileMode));\n" +
"   canvas->clear(SK_ColorWHITE);\n" +
"   for (SkColor c : { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN } ) {\n" +
"       paint.setColor(c);  // all components in color affect shader\n" +
"       canvas->drawCircle(50, 50, 50, paint);\n" +
"       canvas->translate(70, 70);\n" +
"   }\n" +
"}\n";

var Paint_Shader_2_json = {
    "code": Paint_Shader_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Shader_2",
    "overwrite": true
}

runFiddle(Paint_Shader_2_json);

var SkPaint_getShader_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint;\n" +
"   SkDebugf(\"nullptr %c= shader\\n\", paint.getShader() ? '!' : '=');\n" +
"   paint.setShader(SkShader::MakeEmptyShader());\n" +
"   SkDebugf(\"nullptr %c= shader\\n\", paint.getShader() ? '!' : '=');\n" +
"}\n";

var SkPaint_getShader_json = {
    "code": SkPaint_getShader_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getShader",
    "overwrite": true
}

runFiddle(SkPaint_getShader_json);

var SkPaint_refShader_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint1, paint2;\n" +
"   paint1.setShader(SkShader::MakeEmptyShader());\n" +
"   SkDebugf(\"shader unique: %s\\n\", paint1.getShader()->unique() ? \"true\" : \"false\");\n" +
"   paint2.setShader(paint1.refShader());\n" +
"   SkDebugf(\"shader unique: %s\\n\", paint1.getShader()->unique() ? \"true\" : \"false\");\n" +
"}\n";

var SkPaint_refShader_json = {
    "code": SkPaint_refShader_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_refShader",
    "overwrite": true
}

runFiddle(SkPaint_refShader_json);

var SkPaint_setShader_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setColor(SK_ColorBLUE);\n" +
"    paint.setShader(SkShader::MakeColorShader(SK_ColorRED));\n" +
"    canvas->drawRect(SkRect::MakeWH(40, 40), paint);\n" +
"    paint.setShader(nullptr);\n" +
"    canvas->translate(50, 0);\n" +
"    canvas->drawRect(SkRect::MakeWH(40, 40), paint);\n" +
"}\n";

var SkPaint_setShader_json = {
    "code": SkPaint_setShader_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_setShader",
    "overwrite": true
}

runFiddle(SkPaint_setShader_json);

var Paint_Color_Filter_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setColorFilter(SkColorMatrixFilter::MakeLightingFilter(0xFFFFFF, 0xFF0000));\n" +
"    for (SkColor c : { SK_ColorBLACK, SK_ColorGREEN } ) {\n" +
"        paint.setColor(c);\n" +
"        canvas->drawRect(SkRect::MakeXYWH(50, 50, 50, 50), paint);\n" +
"        paint.setAlpha(0x80);\n" +
"        canvas->drawRect(SkRect::MakeXYWH(100, 100, 50, 50), paint);\n" +
"        canvas->translate(100, 0);\n" +
"    }\n" +
"}\n";

var Paint_Color_Filter_json = {
    "code": Paint_Color_Filter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Color_Filter",
    "overwrite": true
}

runFiddle(Paint_Color_Filter_json);

var SkPaint_getColorFilter_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint;\n" +
"   SkDebugf(\"nullptr %c= color filter\\n\", paint.getColorFilter() ? '!' : '=');\n" +
"   paint.setColorFilter(SkColorFilter::MakeModeFilter(SK_ColorLTGRAY, SkBlendMode::kSrcIn));\n" +
"   SkDebugf(\"nullptr %c= color filter\\n\", paint.getColorFilter() ? '!' : '=');\n" +
"}\n";

var SkPaint_getColorFilter_json = {
    "code": SkPaint_getColorFilter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getColorFilter",
    "overwrite": true
}

runFiddle(SkPaint_getColorFilter_json);

var SkPaint_refColorFilter_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint1, paint2;\n" +
"   paint1.setColorFilter(SkColorFilter::MakeModeFilter(0xFFFF0000, SkBlendMode::kSrcATop));\n" +
"   SkDebugf(\"color filter unique: %s\\n\", paint1.getColorFilter()->unique() ? \"true\" : \"false\");\n" +
"   paint2.setColorFilter(paint1.refColorFilter());\n" +
"   SkDebugf(\"color filter unique: %s\\n\", paint1.getColorFilter()->unique() ? \"true\" : \"false\");\n" +
"}\n";

var SkPaint_refColorFilter_json = {
    "code": SkPaint_refColorFilter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_refColorFilter",
    "overwrite": true
}

runFiddle(SkPaint_refColorFilter_json);

var SkPaint_setColorFilter_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint;\n" +
"   paint.setColorFilter(SkColorFilter::MakeModeFilter(SK_ColorLTGRAY, SkBlendMode::kSrcIn));\n" +
"   canvas->drawRect(SkRect::MakeWH(50, 50), paint);\n" +
"   paint.setColorFilter(nullptr);\n" +
"   canvas->translate(70, 0);\n" +
"   canvas->drawRect(SkRect::MakeWH(50, 50), paint);\n" +
"}\n";

var SkPaint_setColorFilter_json = {
    "code": SkPaint_setColorFilter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_setColorFilter",
    "overwrite": true
}

runFiddle(SkPaint_setColorFilter_json);

var Paint_Blend_Mode_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint normal, blender;\n" +
"    normal.setColor(0xFF58a889);\n" +
"    blender.setColor(0xFF8958a8);\n" +
"    canvas->clear(0);\n" +
"    for (SkBlendMode m : { SkBlendMode::kSrcOver, SkBlendMode::kSrcIn, SkBlendMode::kSrcOut } ) {\n" +
"        normal.setBlendMode(SkBlendMode::kSrcOver);\n" +
"        canvas->drawOval(SkRect::MakeXYWH(30, 30, 30, 80), normal);\n" +
"        blender.setBlendMode(m);\n" +
"        canvas->drawOval(SkRect::MakeXYWH(10, 50, 80, 30), blender);\n" +
"        canvas->translate(70, 70);\n" +
"    }\n" +
"}\n";

var Paint_Blend_Mode_json = {
    "code": Paint_Blend_Mode_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Blend_Mode",
    "overwrite": true
}

runFiddle(Paint_Blend_Mode_json);

var SkPaint_getBlendMode_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint;\n" +
"   SkDebugf(\"kSrcOver %c= getBlendMode\\n\", \n" +
"            SkBlendMode::kSrcOver == paint.getBlendMode() ? '=' : '!');\n" +
"   paint.setBlendMode(SkBlendMode::kSrc);\n" +
"   SkDebugf(\"kSrcOver %c= getBlendMode\\n\", \n" +
"            SkBlendMode::kSrcOver == paint.getBlendMode() ? '=' : '!');\n" +
"}\n";

var SkPaint_getBlendMode_json = {
    "code": SkPaint_getBlendMode_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getBlendMode",
    "overwrite": true
}

runFiddle(SkPaint_getBlendMode_json);

var SkPaint_isSrcOver_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint;\n" +
"   SkDebugf(\"isSrcOver %c= true\\n\", paint.isSrcOver() ? '=' : '!');\n" +
"   paint.setBlendMode(SkBlendMode::kSrc);\n" +
"   SkDebugf(\"isSrcOver %c= true\\n\", paint.isSrcOver() ? '=' : '!');\n" +
"}\n";

var SkPaint_isSrcOver_json = {
    "code": SkPaint_isSrcOver_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_isSrcOver",
    "overwrite": true
}

runFiddle(SkPaint_isSrcOver_json);

var SkPaint_setBlendMode_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint;\n" +
"   SkDebugf(\"isSrcOver %c= true\\n\", paint.isSrcOver() ? '=' : '!');\n" +
"   paint.setBlendMode(SkBlendMode::kSrc);\n" +
"   SkDebugf(\"isSrcOver %c= true\\n\", paint.isSrcOver() ? '=' : '!');\n" +
"}\n";

var SkPaint_setBlendMode_json = {
    "code": SkPaint_setBlendMode_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setBlendMode",
    "overwrite": true
}

runFiddle(SkPaint_setBlendMode_json);

var Paint_Path_Effect_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(8);\n" +
"    SkScalar intervals[] = {4, 6, 3, 1};\n" +
"    paint.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 1));\n" +
"    canvas->drawRect({20, 20, 120, 120}, paint);\n" +
"}\n";

var Paint_Path_Effect_json = {
    "code": Paint_Path_Effect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Path_Effect",
    "overwrite": true
}

runFiddle(Paint_Path_Effect_json);

var SkPaint_getPathEffect_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint;\n" +
"   SkDebugf(\"nullptr %c= path effect\\n\", paint.getPathEffect() ? '!' : '=');\n" +
"   paint.setPathEffect(SkCornerPathEffect::Make(10));\n" +
"   SkDebugf(\"nullptr %c= path effect\\n\", paint.getPathEffect() ? '!' : '=');\n" +
"}\n";

var SkPaint_getPathEffect_json = {
    "code": SkPaint_getPathEffect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getPathEffect",
    "overwrite": true
}

runFiddle(SkPaint_getPathEffect_json);

var SkPaint_refPathEffect_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint1, paint2;\n" +
"   paint1.setPathEffect(SkArcToPathEffect::Make(10));\n" +
"   SkDebugf(\"path effect unique: %s\\n\", paint1.getPathEffect()->unique() ? \"true\" : \"false\");\n" +
"   paint2.setPathEffect(paint1.refPathEffect());\n" +
"   SkDebugf(\"path effect unique: %s\\n\", paint1.getPathEffect()->unique() ? \"true\" : \"false\");\n" +
"}\n";

var SkPaint_refPathEffect_json = {
    "code": SkPaint_refPathEffect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_refPathEffect",
    "overwrite": true
}

runFiddle(SkPaint_refPathEffect_json);

var SkPaint_setPathEffect_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setPathEffect(SkDiscretePathEffect::Make(3, 5));\n" +
"    canvas->drawRect(SkRect::MakeXYWH(40, 40, 175, 175), paint);\n" +
"}\n";

var SkPaint_setPathEffect_json = {
    "code": SkPaint_setPathEffect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_setPathEffect",
    "overwrite": true
}

runFiddle(SkPaint_setPathEffect_json);

var Paint_Mask_Filter_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setMaskFilter(SkBlurMaskFilter::Make(kSolid_SkBlurStyle, 3));\n" +
"    canvas->drawRect(SkRect::MakeXYWH(40, 40, 175, 175), paint);\n" +
"}\n";

var Paint_Mask_Filter_json = {
    "code": Paint_Mask_Filter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Mask_Filter",
    "overwrite": true
}

runFiddle(Paint_Mask_Filter_json);

var SkPaint_getMaskFilter_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint;\n" +
"   SkDebugf(\"nullptr %c= mask filter\\n\", paint.getMaskFilter() ? '!' : '=');\n" +
"   paint.setMaskFilter(SkBlurMaskFilter::Make(kOuter_SkBlurStyle, 3));\n" +
"   SkDebugf(\"nullptr %c= mask filter\\n\", paint.getMaskFilter() ? '!' : '=');\n" +
"}\n";

var SkPaint_getMaskFilter_json = {
    "code": SkPaint_getMaskFilter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getMaskFilter",
    "overwrite": true
}

runFiddle(SkPaint_getMaskFilter_json);

var SkPaint_refMaskFilter_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint1, paint2;\n" +
"   paint1.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, 1));\n" +
"   SkDebugf(\"mask filter unique: %s\\n\", paint1.getMaskFilter()->unique() ? \"true\" : \"false\");\n" +
"   paint2.setMaskFilter(paint1.refMaskFilter());\n" +
"   SkDebugf(\"mask filter unique: %s\\n\", paint1.getMaskFilter()->unique() ? \"true\" : \"false\");\n" +
"}\n";

var SkPaint_refMaskFilter_json = {
    "code": SkPaint_refMaskFilter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_refMaskFilter",
    "overwrite": true
}

runFiddle(SkPaint_refMaskFilter_json);

var SkPaint_setMaskFilter_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(10);\n" +
"    paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, 10));\n" +
"    canvas->drawRect(SkRect::MakeXYWH(40, 40, 175, 175), paint);\n" +
"}\n";

var SkPaint_setMaskFilter_json = {
    "code": SkPaint_setMaskFilter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_setMaskFilter",
    "overwrite": true
}

runFiddle(SkPaint_setMaskFilter_json);

var Paint_Typeface_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setTypeface(SkTypeface::MakeDefault(SkTypeface::kBold));\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setTextSize(36);\n" +
"    const char aBigHello[] = \"A Big Hello!\";\n" +
"    canvas->drawText(aBigHello, sizeof(aBigHello) - 1, 10, 100, paint);\n" +
"    paint.setTypeface(nullptr);\n" +
"    paint.setFakeBoldText(true);\n" +
"    canvas->drawText(aBigHello, sizeof(aBigHello) - 1, 10, 140, paint);\n" +
"}\n";

var Paint_Typeface_json = {
    "code": Paint_Typeface_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Typeface",
    "overwrite": true
}

runFiddle(Paint_Typeface_json);

var SkPaint_getTypeface_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint;\n" +
"   SkDebugf(\"nullptr %c= typeface\\n\", paint.getTypeface() ? '!' : '=');\n" +
"   paint.setTypeface(SkTypeface::MakeFromName(\"Times New Roman\", SkFontStyle()));\n" +
"   SkDebugf(\"nullptr %c= typeface\\n\", paint.getTypeface() ? '!' : '=');\n" +
"}\n";

var SkPaint_getTypeface_json = {
    "code": SkPaint_getTypeface_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getTypeface",
    "overwrite": true
}

runFiddle(SkPaint_getTypeface_json);

var SkPaint_refTypeface_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint1, paint2;\n" +
"   paint1.setTypeface(SkTypeface::MakeFromName(\"Times New Roman\", \n" +
"            SkFontStyle(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width,\n" +
"            SkFontStyle::kItalic_Slant)));\n" +
"   SkDebugf(\"typeface1 %c= typeface2\\n\",\n" +
"            paint1.getTypeface() == paint2.getTypeface() ? '=' : '!');\n" +
"   paint2.setTypeface(paint1.refTypeface());\n" +
"   SkDebugf(\"typeface1 %c= typeface2\\n\",\n" +
"            paint1.getTypeface() == paint2.getTypeface() ? '=' : '!');\n" +
"}\n";

var SkPaint_refTypeface_json = {
    "code": SkPaint_refTypeface_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_refTypeface",
    "overwrite": true
}

runFiddle(SkPaint_refTypeface_json);

var SkPaint_setTypeface_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    const char courierNew[] = \"Courier New\";\n" +
"    paint.setTypeface(SkTypeface::MakeFromName(courierNew, SkFontStyle()));\n" +
"    canvas->drawText(courierNew, sizeof(courierNew) - 1, 10, 30, paint);\n" +
"    paint.setTypeface(nullptr);\n" +
"    canvas->drawText(\"default\", 7, 10, 50, paint);\n" +
"}\n";

var SkPaint_setTypeface_json = {
    "code": SkPaint_setTypeface_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_setTypeface",
    "overwrite": true
}

runFiddle(SkPaint_setTypeface_json);

var Paint_Rasterizer_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkLayerRasterizer::Builder layerBuilder;\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(1);\n" +
"    layerBuilder.addLayer(paint);\n" +
"    paint.setAlpha(0x10);\n" +
"    paint.setStyle(SkPaint::kFill_Style);\n" +
"    paint.setBlendMode(SkBlendMode::kSrc);\n" +
"    layerBuilder.addLayer(paint);\n" +
"    paint.reset();\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setTextSize(50);\n" +
"    paint.setRasterizer(layerBuilder.detach());\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    canvas->drawText(\"outline\", 7, 10, 50, paint);\n" +
"}\n";

var Paint_Rasterizer_json = {
    "code": Paint_Rasterizer_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Rasterizer",
    "overwrite": true
}

runFiddle(Paint_Rasterizer_json);

var SkPaint_getRasterizer_code = 
"class DummyRasterizer : public SkRasterizer {\n" +
"public:\n" +
"    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(DummyRasterizer)\n" +
"};\n" +
"sk_sp<SkFlattenable> DummyRasterizer::CreateProc(SkReadBuffer&) {\n" +
"    return sk_make_sp<DummyRasterizer>();\n" +
"}\n" +
"\n" +
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    DummyRasterizer dummy;\n" +
"    SkDebugf(\"nullptr %c= rasterizer\\n\", paint.getRasterizer() ? '!' : '=');\n" +
"    paint.setRasterizer(sk_make_sp<DummyRasterizer>());\n" +
"    SkDebugf(\"nullptr %c= rasterizer\\n\", paint.getRasterizer() ? '!' : '=');\n" +
"}\n";

var SkPaint_getRasterizer_json = {
    "code": SkPaint_getRasterizer_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getRasterizer",
    "overwrite": true
}

runFiddle(SkPaint_getRasterizer_json);

var SkPaint_refRasterizer_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkLayerRasterizer::Builder layerBuilder;\n" +
"   SkPaint paint1, paint2;\n" +
"   layerBuilder.addLayer(paint2);\n" +
"   paint1.setRasterizer(layerBuilder.detach());\n" +
"   SkDebugf(\"rasterizer unique: %s\\n\", paint1.getRasterizer()->unique() ? \"true\" : \"false\");\n" +
"   paint2.setRasterizer(paint1.refRasterizer());\n" +
"   SkDebugf(\"rasterizer unique: %s\\n\", paint1.getRasterizer()->unique() ? \"true\" : \"false\");\n" +
"}\n";

var SkPaint_refRasterizer_json = {
    "code": SkPaint_refRasterizer_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_refRasterizer",
    "overwrite": true
}

runFiddle(SkPaint_refRasterizer_json);

var SkPaint_setRasterizer_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkLayerRasterizer::Builder layerBuilder;\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(2);\n" +
"    layerBuilder.addLayer(paint);\n" +
"    paint.reset();\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setTextSize(50);\n" +
"    paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, 3));\n" +
"    paint.setRasterizer(layerBuilder.detach());\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    canvas->drawText(\"blurry out\", 10, 0, 50, paint);\n" +
"}\n";

var SkPaint_setRasterizer_json = {
    "code": SkPaint_setRasterizer_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_setRasterizer",
    "overwrite": true
}

runFiddle(SkPaint_setRasterizer_json);

var Paint_Image_Filter_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(2);\n" +
"    SkRegion region;\n" +
"    region.op( 10, 10, 50, 50, SkRegion::kUnion_Op);\n" +
"    region.op( 10, 50, 90, 90, SkRegion::kUnion_Op);\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    paint.setImageFilter(SkImageFilter::MakeBlur(5.0f, 5.0f, nullptr));\n" +
"    canvas->drawRegion(region, paint);\n" +
"    paint.setImageFilter(nullptr);\n" +
"    paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, 5));\n" +
"    canvas->translate(100, 100);\n" +
"    canvas->drawRegion(region, paint);\n" +
"}\n";

var Paint_Image_Filter_json = {
    "code": Paint_Image_Filter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Image_Filter",
    "overwrite": true
}

runFiddle(Paint_Image_Filter_json);

var SkPaint_getImageFilter_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint;\n" +
"   SkDebugf(\"nullptr %c= image filter\\n\", paint.getImageFilter() ? '!' : '=');\n" +
"   paint.setImageFilter(SkImageFilter::MakeBlur(kOuter_SkBlurStyle, 3, nullptr, nullptr));\n" +
"   SkDebugf(\"nullptr %c= image filter\\n\", paint.getImageFilter() ? '!' : '=');\n" +
"}\n";

var SkPaint_getImageFilter_json = {
    "code": SkPaint_getImageFilter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getImageFilter",
    "overwrite": true
}

runFiddle(SkPaint_getImageFilter_json);

var SkPaint_refImageFilter_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint1, paint2;\n" +
"   paint1.setImageFilter(SkOffsetImageFilter::Make(25, 25, nullptr));\n" +
"   SkDebugf(\"image filter unique: %s\\n\", paint1.getImageFilter()->unique() ? \"true\" : \"false\");\n" +
"   paint2.setImageFilter(paint1.refImageFilter());\n" +
"   SkDebugf(\"image filter unique: %s\\n\", paint1.getImageFilter()->unique() ? \"true\" : \"false\");\n" +
"}\n";

var SkPaint_refImageFilter_json = {
    "code": SkPaint_refImageFilter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_refImageFilter",
    "overwrite": true
}

runFiddle(SkPaint_refImageFilter_json);

var SkPaint_setImageFilter_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.allocN32Pixels(100, 100);\n" +
"    SkCanvas offscreen(bitmap);\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setColor(SK_ColorWHITE);\n" +
"    paint.setTextSize(96);\n" +
"    offscreen.clear(0);\n" +
"    offscreen.drawText(\"e\", 1, 20, 70, paint);\n" +
"    paint.setImageFilter(\n" +
"           SkLightingImageFilter::MakePointLitDiffuse(SkPoint3::Make(80, 100, 10),\n" +
"           SK_ColorWHITE, 1, 2, nullptr, nullptr));\n" +
"    canvas->drawBitmap(bitmap, 0, 0, &paint);\n" +
"}\n";

var SkPaint_setImageFilter_json = {
    "code": SkPaint_setImageFilter_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_setImageFilter",
    "overwrite": true
}

runFiddle(SkPaint_setImageFilter_json);

var Paint_Draw_Looper_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkLayerDrawLooper::LayerInfo info;\n" +
"    info.fPaintBits = (SkLayerDrawLooper::BitFlags) SkLayerDrawLooper::kColorFilter_Bit;\n" +
"    info.fColorMode = SkBlendMode::kSrc;\n" +
"    SkLayerDrawLooper::Builder looperBuilder;\n" +
"    SkPaint* loopPaint = looperBuilder.addLayer(info);\n" +
"    loopPaint->setColor(SK_ColorRED);\n" +
"    info.fOffset.set(20, 20);\n" +
"    loopPaint = looperBuilder.addLayer(info);\n" +
"    loopPaint->setColor(SK_ColorBLUE);\n" +
"    SkPaint paint;\n" +
"    paint.setDrawLooper(looperBuilder.detach());\n" +
"    canvas->drawCircle(50, 50, 50, paint);\n" +
"}\n";

var Paint_Draw_Looper_json = {
    "code": Paint_Draw_Looper_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Draw_Looper",
    "overwrite": true
}

runFiddle(Paint_Draw_Looper_json);

var SkPaint_getDrawLooper_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint;\n" +
"   SkDebugf(\"nullptr %c= draw looper\\n\", paint.getDrawLooper() ? '!' : '=');\n" +
"   SkLayerDrawLooper::Builder looperBuilder;\n" +
"   paint.setDrawLooper(looperBuilder.detach());\n" +
"   SkDebugf(\"nullptr %c= draw looper\\n\", paint.getDrawLooper() ? '!' : '=');\n" +
"}\n";

var SkPaint_getDrawLooper_json = {
    "code": SkPaint_getDrawLooper_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getDrawLooper",
    "overwrite": true
}

runFiddle(SkPaint_getDrawLooper_json);

var SkPaint_refDrawLooper_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkPaint paint1, paint2;\n" +
"   SkLayerDrawLooper::Builder looperBuilder;\n" +
"   paint1.setDrawLooper(looperBuilder.detach());\n" +
"   SkDebugf(\"draw looper unique: %s\\n\", paint1.getDrawLooper()->unique() ? \"true\" : \"false\");\n" +
"   paint2.setDrawLooper(paint1.refDrawLooper());\n" +
"   SkDebugf(\"draw looper unique: %s\\n\", paint1.getDrawLooper()->unique() ? \"true\" : \"false\");\n" +
"}\n";

var SkPaint_refDrawLooper_json = {
    "code": SkPaint_refDrawLooper_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_refDrawLooper",
    "overwrite": true
}

runFiddle(SkPaint_refDrawLooper_json);

var SkPaint_setDrawLooper_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setDrawLooper(SkBlurDrawLooper::Make(0x7FFF0000, 4, -5, -10));\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(10);\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setColor(0x7f0000ff);\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    canvas->drawCircle(100, 100, 50, paint);\n" +
"}\n";

var SkPaint_setDrawLooper_json = {
    "code": SkPaint_setDrawLooper_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_setDrawLooper",
    "overwrite": true
}

runFiddle(SkPaint_setDrawLooper_json);

var Paint_Text_Align_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(40);\n" +
"    SkPoint position[] = {{100, 50}, {150, 40}};\n" +
"    for (SkPaint::Align a : { SkPaint::kLeft_Align,\n" +
"                              SkPaint::kCenter_Align,\n" +
"                              SkPaint::kRight_Align}) {\n" +
"        paint.setTextAlign(a);\n" +
"        canvas->drawPosText(\"Aa\", 2, position, paint);\n" +
"        canvas->translate(0, 50);\n" +
"    }\n" +
"}\n";

var Paint_Text_Align_json = {
    "code": Paint_Text_Align_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Text_Align",
    "overwrite": true
}

runFiddle(Paint_Text_Align_json);

var Paint_Text_Align_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(40);\n" +
"    paint.setVerticalText(true);\n" +
"    for (SkPaint::Align a : { SkPaint::kLeft_Align,\n" +
"                                SkPaint::kCenter_Align,\n" +
"                                SkPaint::kRight_Align}) {\n" +
"        paint.setTextAlign(a);\n" +
"        canvas->drawText(\"Aa\", 2, 50, 100, paint);\n" +
"        canvas->translate(50, 0);\n" +
"    }\n" +
"}\n";

var Paint_Text_Align_2_json = {
    "code": Paint_Text_Align_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Text_Align_2",
    "overwrite": true
}

runFiddle(Paint_Text_Align_2_json);

var SkPaint_getTextAlign_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"kLeft_Align %c= default\\n\", SkPaint::kLeft_Align == paint.getTextAlign() ? '=' : '!');\n" +
"}\n";

var SkPaint_getTextAlign_json = {
    "code": SkPaint_getTextAlign_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getTextAlign",
    "overwrite": true
}

runFiddle(SkPaint_getTextAlign_json);

var SkPaint_setTextAlign_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(40);\n" +
"    canvas->drawText(\"Aa\", 2, 100, 50, paint);\n" +
"    paint.setTextAlign(SkPaint::kCenter_Align);\n" +
"    canvas->drawText(\"Aa\", 2, 100, 100, paint);\n" +
"    paint.setTextAlign((SkPaint::Align) SkPaint::kAlignCount);\n" +
"    canvas->drawText(\"Aa\", 2, 100, 150, paint);\n" +
"}\n";

var SkPaint_setTextAlign_json = {
    "code": SkPaint_setTextAlign_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_setTextAlign",
    "overwrite": true
}

runFiddle(SkPaint_setTextAlign_json);

var Paint_Text_Size_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    canvas->drawText(\"12 point\", 8, 10, 20, paint);\n" +
"    paint.setTextSize(24);\n" +
"    canvas->drawText(\"24 point\", 8, 10, 60, paint);\n" +
"    paint.setTextSize(48);\n" +
"    canvas->drawText(\"48 point\", 8, 10, 120, paint);\n" +
"}\n";

var Paint_Text_Size_json = {
    "code": Paint_Text_Size_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Text_Size",
    "overwrite": true
}

runFiddle(Paint_Text_Size_json);

var SkPaint_getTextSize_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"12 %c= default text size\\n\", 12 == paint.getTextSize() ? '=' : '!');\n" +
"}\n";

var SkPaint_getTextSize_json = {
    "code": SkPaint_getTextSize_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getTextSize",
    "overwrite": true
}

runFiddle(SkPaint_getTextSize_json);

var SkPaint_setTextSize_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"12 %c= text size\\n\", 12 == paint.getTextSize() ? '=' : '!');\n" +
"    paint.setTextSize(-20);\n" +
"    SkDebugf(\"12 %c= text size\\n\", 12 == paint.getTextSize() ? '=' : '!');\n" +
"}\n";

var SkPaint_setTextSize_json = {
    "code": SkPaint_setTextSize_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setTextSize",
    "overwrite": true
}

runFiddle(SkPaint_setTextSize_json);

var Paint_Text_Scale_X_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setTextSize(24);\n" +
"    paint.setTextScaleX(.8f);\n" +
"    canvas->drawText(\"narrow\", 6, 10, 20, paint);\n" +
"    paint.setTextScaleX(1);\n" +
"    canvas->drawText(\"normal\", 6, 10, 60, paint);\n" +
"    paint.setTextScaleX(1.2f);\n" +
"    canvas->drawText(\"wide\", 4, 10, 100, paint);\n" +
"}\n";

var Paint_Text_Scale_X_json = {
    "code": Paint_Text_Scale_X_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Text_Scale_X",
    "overwrite": true
}

runFiddle(Paint_Text_Scale_X_json);

var SkPaint_getTextScaleX_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"1 %c= default text scale x\\n\", 1 == paint.getTextScaleX() ? '=' : '!');\n" +
"}\n";

var SkPaint_getTextScaleX_json = {
    "code": SkPaint_getTextScaleX_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getTextScaleX",
    "overwrite": true
}

runFiddle(SkPaint_getTextScaleX_json);

var SkPaint_setTextScaleX_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setTextScaleX(0.f / 0.f);\n" +
"    SkDebugf(\"text scale %s-a-number\\n\", SkScalarIsNaN(paint.getTextScaleX()) ? \"not\" : \"is\");\n" +
"}\n";

var SkPaint_setTextScaleX_json = {
    "code": SkPaint_setTextScaleX_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setTextScaleX",
    "overwrite": true
}

runFiddle(SkPaint_setTextScaleX_json);

var Paint_Text_Skew_X_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setTextSize(24);\n" +
"    paint.setTextSkewX(-.25f);\n" +
"    canvas->drawText(\"right-leaning\", 13, 10, 100, paint);\n" +
"    paint.setTextSkewX(0);\n" +
"    canvas->drawText(\"normal\", 6, 10, 60, paint);\n" +
"    paint.setTextSkewX(.25f);\n" +
"    canvas->drawText(\"left-leaning\", 12, 10, 20, paint);\n" +
"}\n";

var Paint_Text_Skew_X_json = {
    "code": Paint_Text_Skew_X_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Text_Skew_X",
    "overwrite": true
}

runFiddle(Paint_Text_Skew_X_json);

var SkPaint_getTextSkewX_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"0 %c= default text skew x\\n\", 0 == paint.getTextSkewX() ? '=' : '!');\n" +
"}\n";

var SkPaint_getTextSkewX_json = {
    "code": SkPaint_getTextSkewX_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getTextSkewX",
    "overwrite": true
}

runFiddle(SkPaint_getTextSkewX_json);

var SkPaint_setTextSkewX_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setTextScaleX(1.f / 0.f);\n" +
"    SkDebugf(\"text scale %s-finite\\n\", SkScalarIsFinite(paint.getTextScaleX()) ? \"is\" : \"not\");\n" +
"}\n";

var SkPaint_setTextSkewX_json = {
    "code": SkPaint_setTextSkewX_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setTextSkewX",
    "overwrite": true
}

runFiddle(SkPaint_setTextSkewX_json);

var SkPaint_SetTextMatrix_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkMatrix matrix;\n" +
"    SkPaint::SetTextMatrix(&matrix, 12, 1, 0);\n" +
"    matrix.dump();\n" +
"}\n";

var SkPaint_SetTextMatrix_json = {
    "code": SkPaint_SetTextMatrix_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_SetTextMatrix",
    "overwrite": true
}

runFiddle(SkPaint_SetTextMatrix_json);

var SkPaint_setTextMatrix_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkMatrix matrix;\n" +
"    paint.setTextMatrix(&matrix);\n" +
"    matrix.dump();\n" +
"}\n";

var SkPaint_setTextMatrix_json = {
    "code": SkPaint_setTextMatrix_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setTextMatrix",
    "overwrite": true
}

runFiddle(SkPaint_setTextMatrix_json);

var Paint_Text_Encoding_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    const char hello8[] = \"Hello!\";\n" +
"    const uint16_t hello16[] = { 'H', 'e', 'l', 'l', 'o', '!' };\n" +
"    const uint32_t hello32[] = { 'H', 'e', 'l', 'l', 'o', '!' };\n" +
"    paint.setTextSize(24);\n" +
"    canvas->drawText(hello8, sizeof(hello8) - 1, 10, 30, paint);\n" +
"    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);\n" +
"    canvas->drawText(hello16, sizeof(hello16), 10, 60, paint);\n" +
"    paint.setTextEncoding(SkPaint::kUTF32_TextEncoding);\n" +
"    canvas->drawText(hello32, sizeof(hello32), 10, 90, paint);\n" +
"    uint16_t glyphs[SK_ARRAY_COUNT(hello32)];\n" +
"    paint.textToGlyphs(hello32, sizeof(hello32), glyphs);\n" +
"    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);\n" +
"    canvas->drawText(glyphs, sizeof(glyphs), 10, 120, paint);\n" +
"}\n";

var Paint_Text_Encoding_json = {
    "code": Paint_Text_Encoding_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Text_Encoding",
    "overwrite": true
}

runFiddle(Paint_Text_Encoding_json);

var SkPaint_getTextEncoding_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"kUTF8_TextEncoding %c= text encoding\\n\", \n" +
"            SkPaint::kUTF8_TextEncoding == paint.getTextEncoding() ? '=' : '!');\n" +
"    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);\n" +
"    SkDebugf(\"kGlyphID_TextEncoding %c= text encoding\\n\", \n" +
"            SkPaint::kGlyphID_TextEncoding == paint.getTextEncoding() ? '=' : '!');\n" +
"}\n";

var SkPaint_getTextEncoding_json = {
    "code": SkPaint_getTextEncoding_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getTextEncoding",
    "overwrite": true
}

runFiddle(SkPaint_getTextEncoding_json);

var SkPaint_setTextEncoding_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setTextEncoding((SkPaint::TextEncoding) 4);\n" +
"    SkDebugf(\"4 %c= text encoding\\n\", 4 == paint.getTextEncoding() ? '=' : '!');\n" +
"}\n";

var SkPaint_setTextEncoding_json = {
    "code": SkPaint_setTextEncoding_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_setTextEncoding",
    "overwrite": true
}

runFiddle(SkPaint_setTextEncoding_json);

var Paint_Font_Metrics_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setTextSize(120);\n" +
"    SkPaint::FontMetrics fm;\n" +
"    SkScalar lineHeight = paint.getFontMetrics(&fm);\n" +
"    SkPoint pt = { 70, 180 };\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    canvas->drawText(\"M\", 1, pt.fX, pt.fY, paint);\n" +
"    canvas->drawLine(pt.fX, pt.fY, pt.fX, pt.fY + fm.fTop, paint);\n" +
"    SkScalar ascent = pt.fY + fm.fAscent;\n" +
"    canvas->drawLine(pt.fX - 25, ascent, pt.fX - 25, ascent + lineHeight, paint);\n" +
"    canvas->drawLine(pt.fX - 50, pt.fY, pt.fX - 50, pt.fY + fm.fDescent, paint);\n" +
"    canvas->drawLine(pt.fX + 100, pt.fY, pt.fX + 100, pt.fY + fm.fAscent, paint);\n" +
"    canvas->drawLine(pt.fX + 125, pt.fY, pt.fX + 125, pt.fY - fm.fXHeight, paint);\n" +
"    canvas->drawLine(pt.fX + 150, pt.fY, pt.fX + 150, pt.fY - fm.fCapHeight, paint);\n" +
"    canvas->drawLine(pt.fX + 5, pt.fY, pt.fX + 5, pt.fY + fm.fBottom, paint);\n" +
"    SkScalar xmin = pt.fX + fm.fXMin;\n" +
"    canvas->drawLine(xmin, pt.fY + 60, xmin + fm.fMaxCharWidth, pt.fY + 60, paint);\n" +
"    canvas->drawLine(xmin, pt.fY - 145, pt.fX, pt.fY - 145, paint);\n" +
"    canvas->drawLine(pt.fX + fm.fXMax, pt.fY - 160, pt.fX, pt.fY - 160, paint);\n" +
"    SkScalar upos = pt.fY + fm.fUnderlinePosition;\n" +
"    canvas->drawLine(pt.fX + 25, upos, pt.fX + 130, upos, paint);\n" +
"    SkScalar urad = fm.fUnderlineThickness / 2;\n" +
"    canvas->drawLine(pt.fX + 130, upos - urad, pt.fX + 160, upos - urad, paint);\n" +
"    canvas->drawLine(pt.fX + 130, upos + urad, pt.fX + 160, upos + urad, paint);\n" +
"    paint.setTextSize(12);\n" +
"    canvas->drawText(\"x-min\", 5,         pt.fX - 50, pt.fY - 148, paint);\n" +
"    canvas->drawText(\"x-max\", 5,         pt.fX + 140, pt.fY - 150, paint);\n" +
"    canvas->drawText(\"max char width\", 14, pt.fX + 120, pt.fY + 57, paint);\n" +
"    canvas->drawText(\"underline position\", 18, pt.fX + 30, pt.fY + 22, paint);\n" +
"    canvas->drawText(\"underline thickness\", 19, pt.fX + 162, pt.fY + 13, paint);\n" +
"    canvas->rotate(-90);\n" +
"    canvas->drawText(\"descent\", 7,      -pt.fY - 30, pt.fX - 54,  paint);\n" +
"    canvas->drawText(\"line height\", 11, -pt.fY,      pt.fX - 29,  paint);\n" +
"    canvas->drawText(\"top\", 3,          -pt.fY + 30, pt.fX - 4,   paint);\n" +
"    canvas->drawText(\"ascent\", 6,       -pt.fY,      pt.fX + 110, paint);\n" +
"    canvas->drawText(\"x-height\", 8,     -pt.fY,      pt.fX + 135, paint);\n" +
"    canvas->drawText(\"cap-height\", 10,  -pt.fY,      pt.fX + 160, paint);\n" +
"    canvas->drawText(\"bottom\", 6,       -pt.fY - 50, pt.fX + 15,  paint);\n" +
"}\n";

var Paint_Font_Metrics_json = {
    "code": Paint_Font_Metrics_code,
    "options": {
        "width": 512,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Paint_Font_Metrics",
    "overwrite": true
}

runFiddle(Paint_Font_Metrics_json);

var SkPaint_getFontMetrics_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(32);\n" +
"    SkScalar lineHeight = paint.getFontMetrics(nullptr);\n" +
"    canvas->drawText(\"line 1\", 6, 10, 40, paint);\n" +
"    canvas->drawText(\"line 2\", 6, 10, 40 + lineHeight, paint);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(10);\n" +
"    lineHeight = paint.getFontMetrics(nullptr, 1.10f);  // account for stroke height\n" +
"    canvas->drawText(\"line 3\", 6, 120, 40, paint);\n" +
"    canvas->drawText(\"line 4\", 6, 120, 40 + lineHeight, paint);\n" +
"}\n";

var SkPaint_getFontMetrics_json = {
    "code": SkPaint_getFontMetrics_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_getFontMetrics",
    "overwrite": true
}

runFiddle(SkPaint_getFontMetrics_json);

var SkPaint_getFontSpacing_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    for (SkScalar textSize : { 12, 18, 24, 32 } ) {\n" +
"        paint.setTextSize(textSize);\n" +
"        SkDebugf(\"textSize: %g fontSpacing: %g\\n\", textSize, paint.getFontSpacing());\n" +
"    }\n" +
"}\n";

var SkPaint_getFontSpacing_json = {
    "code": SkPaint_getFontSpacing_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getFontSpacing",
    "overwrite": true
}

runFiddle(SkPaint_getFontSpacing_json);

var SkPaint_getFontBounds_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkPaint::FontMetrics fm;\n" +
"    paint.getFontMetrics(&fm);\n" +
"    SkRect fb = paint.getFontBounds();\n" +
"    SkDebugf(\"metrics bounds = { %g, %g, %g, %g }\\n\", fm.fXMin, fm.fTop, fm.fXMax, fm.fBottom );\n" +
"    SkDebugf(\"font bounds    = { %g, %g, %g, %g }\\n\", fb.fLeft, fb.fTop, fb.fRight, fm.fBottom );\n" +
"}\n";

var SkPaint_getFontBounds_json = {
    "code": SkPaint_getFontBounds_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_getFontBounds",
    "overwrite": true
}

runFiddle(SkPaint_getFontBounds_json);

var SkPaint_textToGlyphs_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    const uint8_t utf8[] = { 0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0xF0, 0x90, 0x8D, 0x88 };\n" +
"    std::vector<SkGlyphID> glyphs;\n" +
"    int count = paint.textToGlyphs(utf8, sizeof(utf8), nullptr);\n" +
"    glyphs.resize(count);\n" +
"    (void) paint.textToGlyphs(utf8, sizeof(utf8), &glyphs.front());\n" +
"    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);\n" +
"    paint.setTextSize(32);\n" +
"    canvas->drawText(&glyphs.front(), glyphs.size() * sizeof(SkGlyphID), 10, 40, paint);\n" +
"}\n";

var SkPaint_textToGlyphs_json = {
    "code": SkPaint_textToGlyphs_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_textToGlyphs",
    "overwrite": true
}

runFiddle(SkPaint_textToGlyphs_json);

var SkPaint_countText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    const uint8_t utf8[] = { 0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0xF0, 0x90, 0x8D, 0x88 };\n" +
"    SkDebugf(\"count = %d\\n\", paint.countText(utf8, sizeof(utf8)));\n" +
"}\n";

var SkPaint_countText_json = {
    "code": SkPaint_countText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_countText",
    "overwrite": true
}

runFiddle(SkPaint_countText_json);

var SkPaint_containsText_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    const uint16_t goodChar = 0x00B0;  // degree symbol\n" +
"    const uint16_t badChar = 0xD800;   // Unicode surrogate\n" +
"    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);\n" +
"    SkDebugf(\"0x%04x %c= has char\\n\", goodChar, \n" +
"            paint.containsText(&goodChar, 2) ? '=' : '!');\n" +
"    SkDebugf(\"0x%04x %c= has char\\n\", badChar,\n" +
"            paint.containsText(&badChar, 2) ? '=' : '!');\n" +
"}\n";

var SkPaint_containsText_json = {
    "code": SkPaint_containsText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_containsText",
    "overwrite": true
}

runFiddle(SkPaint_containsText_json);

var SkPaint_containsText_2_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    const uint16_t goodGlyph = 511;\n" +
"    const uint16_t zeroGlyph = 0;\n" +
"    const uint16_t badGlyph = 65535; // larger than glyph count in font\n" +
"    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);\n" +
"    SkDebugf(\"0x%04x %c= has glyph\\n\", goodGlyph, \n" +
"            paint.containsText(&goodGlyph, 2) ? '=' : '!');\n" +
"    SkDebugf(\"0x%04x %c= has glyph\\n\", zeroGlyph,\n" +
"            paint.containsText(&zeroGlyph, 2) ? '=' : '!');\n" +
"    SkDebugf(\"0x%04x %c= has glyph\\n\", badGlyph,\n" +
"            paint.containsText(&badGlyph, 2) ? '=' : '!');\n" +
"}\n";

var SkPaint_containsText_2_json = {
    "code": SkPaint_containsText_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_containsText_2",
    "overwrite": true
}

runFiddle(SkPaint_containsText_2_json);


var SkPaint_measureText_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setTextSize(50);\n" +
"    const char str[] = \"ay^jZ\";\n" +
"    const int count = sizeof(str) - 1;\n" +
"            canvas->drawText(str, count , 25, 50, paint);\n" +
"    SkRect bounds;\n" +
"    paint.measureText(str, count, &bounds);\n" +
"    canvas->translate(25, 50);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    canvas->drawRect(bounds, paint);\n" +
"}\n";

var SkPaint_measureText_json = {
    "code": SkPaint_measureText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_measureText",
    "overwrite": true
}

runFiddle(SkPaint_measureText_json);

var SkPaint_measureText_2_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkDebugf(\"default width = %g\\n\", paint.measureText(\"!\", 1));\n" +
"    paint.setTextSize(paint.getTextSize() * 2);\n" +
"    SkDebugf(\"double width = %g\\n\", paint.measureText(\"!\", 1));\n" +
"}\n";

var SkPaint_measureText_2_json = {
    "code": SkPaint_measureText_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_measureText_2",
    "overwrite": true
}

runFiddle(SkPaint_measureText_2_json);

var SkPaint_breakText_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setTextSize(50);\n" +
"    const char str[] = \"Breakfast\";\n" +
"    const int count = sizeof(str) - 1;\n" +
"    canvas->drawText(str, count, 25, 50, paint);\n" +
"    SkScalar measuredWidth;\n" +
"    int partialBytes = paint.breakText(str, count, 100, &measuredWidth);\n" +
"    canvas->drawText(str, partialBytes, 25, 100, paint);\n" +
"    canvas->drawLine(25, 60, 25 + 100, 60, paint);\n" +
"    canvas->drawLine(25, 110, 25 + measuredWidth, 110, paint);\n" +
"}\n";

var SkPaint_breakText_json = {
    "code": SkPaint_breakText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_breakText",
    "overwrite": true
}

runFiddle(SkPaint_breakText_json);

var SkPaint_getTextWidths_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setTextSize(50);\n" +
"    const char str[] = \"abc\";\n" +
"    const int bytes = sizeof(str) - 1;\n" +
"    int count = paint.getTextWidths(str, bytes, nullptr);\n" +
"    std::vector<SkScalar> widths;\n" +
"    std::vector<SkRect> bounds;\n" +
"    widths.resize(count);\n" +
"    bounds.resize(count);\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    for (int loop = 0; loop < 2; ++loop) {\n" +
"        (void) paint.getTextWidths(str, count, &widths.front(), &bounds.front());\n" +
"        SkPoint loc = { 25, 60 };\n" +
"        canvas->drawText(str, bytes, loc.fX, loc.fY, paint);\n" +
"        paint.setStyle(SkPaint::kStroke_Style);\n" +
"        paint.setStrokeWidth(0);\n" +
"        SkScalar advanceY = loc.fY + 10;\n" +
"        for (int index = 0; index < count; ++index) {\n" +
"            bounds[index].offset(loc.fX, loc.fY);\n" +
"            canvas->drawRect(bounds[index], paint);\n" +
"            canvas->drawLine(loc.fX, advanceY, loc.fX + widths[index], advanceY, paint);\n" +
"            loc.fX += widths[index];\n" +
"            advanceY += 5;\n" +
"        }\n" +
"        canvas->translate(0, 80);\n" +
"        paint.setStrokeWidth(3);\n" +
"    }\n" +
"}\n";

var SkPaint_getTextWidths_json = {
    "code": SkPaint_getTextWidths_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_getTextWidths",
    "overwrite": true
}

runFiddle(SkPaint_getTextWidths_json);

var SkPaint_getTextPath_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(80);\n" +
"    SkPath path, path2;\n" +
"    paint.getTextPath(\"ABC\", 3, 20, 80, &path);\n" +
"    path.offset(20, 20, &path2);\n" +
"    Op(path, path2, SkPathOp::kDifference_SkPathOp, &path);\n" +
"    path.addPath(path2);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    canvas->drawPath(path, paint);\n" +
"}\n";

var SkPaint_getTextPath_json = {
    "code": SkPaint_getTextPath_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_getTextPath",
    "overwrite": true
}

runFiddle(SkPaint_getTextPath_json);

var SkPaint_getPosTextPath_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(80);\n" +
"    SkPath path, path2;\n" +
"    SkPoint pos[] = {{20, 60}, {30, 70}, {40, 80}};\n" +
"    paint.getPosTextPath(\"ABC\", 3, pos, &path);\n" +
"    Simplify(path, &path);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    canvas->drawPath(path, paint);\n" +
"}\n";

var SkPaint_getPosTextPath_json = {
    "code": SkPaint_getPosTextPath_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_getPosTextPath",
    "overwrite": true
}

runFiddle(SkPaint_getPosTextPath_json);

var SkPaint_getTextIntercepts_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(120);\n" +
"    SkPoint textOrigin = { 20, 100 };\n" +
"    SkScalar bounds[] = { 100, 108 };\n" +
"    int count = paint.getTextIntercepts(\"y\", 1, textOrigin.fX, textOrigin.fY, bounds, nullptr);\n" +
"    std::vector<SkScalar> intervals;\n" +
"    intervals.resize(count);\n" +
"    (void) paint.getTextIntercepts(\"y\", 1, textOrigin.fX, textOrigin.fY, bounds, &intervals.front());\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    canvas->drawText(\"y\", 1, textOrigin.fX, textOrigin.fY, paint);\n" +
"    paint.setColor(SK_ColorRED);\n" +
"    SkScalar x = textOrigin.fX;\n" +
"    for (int i = 0; i < count; i += 2) {\n" +
"        canvas->drawRect({x, bounds[0], intervals[i], bounds[1]}, paint);\n" +
"        x = intervals[i + 1];\n" +
"    }\n" +
"    canvas->drawRect({intervals[count - 1], bounds[0],\n" +
"        textOrigin.fX + paint.measureText(\"y\", 1), bounds[1]}, paint);\n" +
"}\n";

var SkPaint_getTextIntercepts_json = {
    "code": SkPaint_getTextIntercepts_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_getTextIntercepts",
    "overwrite": true
}

runFiddle(SkPaint_getTextIntercepts_json);

var SkPaint_getPosTextIntercepts_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(120);\n" +
"    paint.setVerticalText(true);\n" +
"    SkPoint textPos[] = {{ 60, 40 }, { 60, 140 }};\n" +
"    SkScalar bounds[] = { 56, 64 };\n" +
"    const char str[] = \"A-\";\n" +
"    int len = sizeof(str) - 1;\n" +
"    int count = paint.getPosTextIntercepts(str, len, textPos, bounds, nullptr);\n" +
"    std::vector<SkScalar> intervals;\n" +
"    intervals.resize(count);\n" +
"    (void) paint.getPosTextIntercepts(str, len, textPos, bounds, &intervals.front());\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    canvas->drawPosText(str, len, textPos, paint);\n" +
"    paint.setColor(SK_ColorRED);\n" +
"    SkScalar y = textPos[0].fY;\n" +
"    for (int i = 0; i < count; i+= 2) {\n" +
"        canvas->drawRect({bounds[0], y, bounds[1], intervals[i]}, paint);\n" +
"        y = intervals[i + 1];\n" +
"    }\n" +
"    canvas->drawRect({bounds[0], intervals[count - 1], bounds[1], 240}, paint);\n" +
"}\n";

var SkPaint_getPosTextIntercepts_json = {
    "code": SkPaint_getPosTextIntercepts_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_getPosTextIntercepts",
    "overwrite": true
}

runFiddle(SkPaint_getPosTextIntercepts_json);

var SkPaint_getPosTextHIntercepts_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(120);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(4);\n" +
"    SkScalar textPosH[] = { 20, 80, 140 };\n" +
"    SkScalar y = 100;\n" +
"    SkScalar bounds[] = { 56, 78 };\n" +
"    const char str[] = \"\\\\-/\";\n" +
"    int len = sizeof(str) - 1;\n" +
"    int count = paint.getPosTextHIntercepts(str, len, textPosH, y, bounds, nullptr);\n" +
"    std::vector<SkScalar> intervals;\n" +
"    intervals.resize(count);\n" +
"    (void) paint.getPosTextHIntercepts(str, len, textPosH, y, bounds, &intervals.front());\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    canvas->drawPosTextH(str, len, textPosH, y, paint);\n" +
"    paint.setColor(0xFFFF7777);\n" +
"    paint.setStyle(SkPaint::kFill_Style);\n" +
"    SkScalar x = textPosH[0];\n" +
"    for (int i = 0; i < count; i+= 2) {\n" +
"        canvas->drawRect({x, bounds[0], intervals[i], bounds[1]}, paint);\n" +
"        x = intervals[i + 1];\n" +
"    }\n" +
"    canvas->drawRect({intervals[count - 1], bounds[0], 180, bounds[1]}, paint);\n" +
"}\n";

var SkPaint_getPosTextHIntercepts_json = {
    "code": SkPaint_getPosTextHIntercepts_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_getPosTextHIntercepts",
    "overwrite": true
}

runFiddle(SkPaint_getPosTextHIntercepts_json);

var SkPaint_getTextBlobIntercepts_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(120);\n" +
"    SkPoint textPos = { 20, 130 };\n" +
"    int len = 3;\n" +
"    SkTextBlobBuilder textBlobBuilder;\n" +
"    const SkTextBlobBuilder::RunBuffer& run = \n" +
"            textBlobBuilder.allocRun(paint, len, textPos.fX, textPos.fY);\n" +
"    run.glyphs[0] = 10;\n" +
"    run.glyphs[1] = 20;\n" +
"    run.glyphs[2] = 30;       \n" +
"    sk_sp<const SkTextBlob> blob = textBlobBuilder.make();\n" +
"    canvas->drawTextBlob(blob.get(), textPos.fX, textPos.fY, paint);\n" +
"    SkScalar bounds[] = { 116, 134 };\n" +
"    int count = paint.getTextBlobIntercepts(blob.get(), bounds, nullptr);\n" +
"    std::vector<SkScalar> intervals;\n" +
"    intervals.resize(count);\n" +
"    (void) paint.getTextBlobIntercepts(blob.get(), bounds, &intervals.front());\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    canvas->drawTextBlob(blob.get(), 0, 0, paint);\n" +
"    paint.setColor(0xFFFF7777);\n" +
"    SkScalar x = textPos.fX;\n" +
"    for (int i = 0; i < count; i+= 2) {\n" +
"        canvas->drawRect({x, bounds[0], intervals[i], bounds[1]}, paint);\n" +
"        x = intervals[i + 1];\n" +
"    }\n" +
"    canvas->drawRect({intervals[count - 1], bounds[0], 180, bounds[1]}, paint);\n" +
"}\n";

var SkPaint_getTextBlobIntercepts_json = {
    "code": SkPaint_getTextBlobIntercepts_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_getTextBlobIntercepts",
    "overwrite": true
}

runFiddle(SkPaint_getTextBlobIntercepts_json);

var SkPaint_nothingToDraw_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    SkDebugf(\"nothing to draw: %s\\n\", paint.nothingToDraw() ? \"true\" : \"false\");\n" +
"    paint.setBlendMode(SkBlendMode::kDst);\n" +
"    SkDebugf(\"nothing to draw: %s\\n\", paint.nothingToDraw() ? \"true\" : \"false\");\n" +
"    paint.setBlendMode(SkBlendMode::kSrcOver);\n" +
"    SkDebugf(\"nothing to draw: %s\\n\", paint.nothingToDraw() ? \"true\" : \"false\");\n" +
"    paint.setAlpha(0);\n" +
"    SkDebugf(\"nothing to draw: %s\\n\", paint.nothingToDraw() ? \"true\" : \"false\");\n" +
"}\n";

var SkPaint_nothingToDraw_json = {
    "code": SkPaint_nothingToDraw_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_nothingToDraw",
    "overwrite": true
}

runFiddle(SkPaint_nothingToDraw_json);

var SkPaint_canComputeFastBounds_code = 
"void draw(SkCanvas* canvas) {\n" + 
"}\n";

var SkPaint_canComputeFastBounds_json = {
    "code": SkPaint_canComputeFastBounds_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_canComputeFastBounds",
    "overwrite": true
}

runFiddle(SkPaint_canComputeFastBounds_json);

var SkPaint_computeFastBounds_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkPath path;\n" +
"    path.addCircle(50, 50, 50);\n" +
"    if (paint.canComputeFastBounds()) {\n" +
"        SkRect adjustedBounds;\n" +
"        const SkRect& rawBounds = path.getBounds();\n" +
"        if (canvas->quickReject(paint.computeFastBounds(rawBounds, &adjustedBounds))) {\n" +
"            return; // don't draw the path\n" +
"        }\n" +
"    }\n" +
"    canvas->drawPath(path, paint);\n" +
"}\n";

var SkPaint_computeFastBounds_json = {
    "code": SkPaint_computeFastBounds_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_computeFastBounds",
    "overwrite": true
}

runFiddle(SkPaint_computeFastBounds_json);

var SkPaint_computeFastStrokeBounds_code = 
"void draw(SkCanvas* canvas) {\n" + 
"}\n";

var SkPaint_computeFastStrokeBounds_json = {
    "code": SkPaint_computeFastStrokeBounds_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_computeFastStrokeBounds",
    "overwrite": true
}

runFiddle(SkPaint_computeFastStrokeBounds_json);

var SkPaint_doComputeFastBounds_code = 
"void draw(SkCanvas* canvas) {\n" + 
"}\n";

var SkPaint_doComputeFastBounds_json = {
    "code": SkPaint_doComputeFastBounds_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkPaint_doComputeFastBounds",
    "overwrite": true
}

runFiddle(SkPaint_doComputeFastBounds_json);

var SkPaint_toString_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    SkString str;\n" +
"    paint.toString(&str);\n" +
"    const char textSize[] = \"TextSize:\";\n" +
"    const int trailerSize = strlen(\"</dd><dt>\");\n" +
"    int textSizeLoc = str.find(textSize) + strlen(textSize) + trailerSize;\n" +
"    const char* sizeStart = &str.c_str()[textSizeLoc];\n" +
"    int textSizeEnd = SkStrFind(sizeStart, \"</dd>\");\n" +
"    SkDebugf(\"text size = %.*s\\n\", textSizeEnd, sizeStart);\n" +
"}\n";

var SkPaint_toString_json = {
    "code": SkPaint_toString_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkPaint_toString",
    "overwrite": true
}

runFiddle(SkPaint_toString_json);

var SkCanvas_MakeRasterDirect_code = 
"void draw(SkCanvas* ) {\n" +
"    SkImageInfo info = SkImageInfo::MakeN32Premul(3, 3);  // device aligned, 32 bpp, premultipled\n" +
"    const size_t minRowBytes = info.minRowBytes();  // bytes used by one bitmap row\n" +
"    const size_t size = info.getSafeSize(minRowBytes);  // bytes used by all rows\n" +
"    SkAutoTMalloc<SkPMColor> storage(size);  // allocate storage for pixels\n" +
"    SkPMColor* pixels = storage.get();  // get pointer to allocated storage\n" +
"    // create a SkCanvas backed by a raster device, and delete it when the\n" +
"    // function goes out of scope.\n" +
"    std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirect(info, pixels, minRowBytes);\n" +
"    canvas->clear(SK_ColorWHITE);  // white is unpremultiplied, in ARGB order\n" +
"    canvas->flush();  // ensure that pixels are cleared\n" +
"    SkPMColor pmWhite = pixels[0];  // the premultiplied format may vary\n" +
"    SkPaint paint;  // by default, draws black\n" +
"    canvas->drawPoint(1, 1, paint);  // draw in the center\n" +
"    canvas->flush();  // ensure that point was drawn\n" +
"    for (int y = 0; y < info.height(); ++y) {\n" +
"        for (int x = 0; x < info.width(); ++x) {\n" +
"            SkDebugf(\"%c\", *pixels++ == pmWhite ? '-' : 'x');\n" +
"        }\n" +
"        SkDebugf(\"\\n\");\n" +
"    }\n" +
"}\n";

var SkCanvas_MakeRasterDirect_json = {
    "code": SkCanvas_MakeRasterDirect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_MakeRasterDirect",
    "overwrite": true
}

runFiddle(SkCanvas_MakeRasterDirect_json);

var SkCanvas_MakeRasterDirectN32_code = 
"void draw(SkCanvas* ) {\n" +
"    const int width = 3;\n" +
"    const int height = 3;\n" +
"    SkPMColor pixels[height][width];  // allocate a 3x3 premultiplied bitmap on the stack\n" +
"    // create a SkCanvas backed by a raster device, and delete it when the\n" +
"    // function goes out of scope.\n" +
"    std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirectN32(\n" +
"            width,\n" +
"            height,\n" +
"            pixels[0],  // top left of the bitmap\n" +
"            sizeof(pixels[0]));  // byte width of the each row\n" +
"    // write a pre-multiplied value for white into all pixels in the bitmap\n" +
"    canvas->clear(SK_ColorWHITE);  // white is unpremultiplied, in ARGB order\n" +
"    canvas->flush();  // ensure that pixels is ready to be read\n" +
"    SkPMColor pmWhite = pixels[0][0];  // the premultiplied format may vary\n" +
"    SkPaint paint;  // by default, draws black\n" +
"    canvas->drawPoint(1, 1, paint);  // draw in the center\n" +
"    for (int y = 0; y < height; ++y) {\n" +
"        for (int x = 0; x < width; ++x) {\n" +
"            SkDebugf(\"%c\", pixels[y][x] == pmWhite ? '-' : 'x');\n" +
"        }\n" +
"        SkDebugf(\"\\n\");\n" +
"    }\n" +
"}\n";

var SkCanvas_MakeRasterDirectN32_json = {
    "code": SkCanvas_MakeRasterDirectN32_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_MakeRasterDirectN32",
    "overwrite": true
}

runFiddle(SkCanvas_MakeRasterDirectN32_json);

var SkCanvas_empty_constructor_code = 
"// Returns true if either the canvas rotates the text by 90 degrees, or the paint does.\n" +
"static void check_for_up_and_down_text(const SkCanvas* canvas, const SkPaint& paint) {\n" +
"    bool paintHasVertical = paint.isVerticalText();\n" +
"    const SkMatrix& matrix = canvas->getTotalMatrix();\n" +
"    bool matrixIsVertical = matrix.preservesRightAngles() && !matrix.isScaleTranslate();\n" +
"    SkDebugf(\"paint draws text %s\\n\", paintHasVertical != matrixIsVertical ?\n" +
"            \"top to bottom\" : \"left to right\");\n" +
"}\n" +
"static void check_for_up_and_down_text(const SkPaint& paint) {\n" +
"    SkCanvas canvas;  // placeholder only, does not have an associated device\n" +
"    check_for_up_and_down_text(&canvas, paint);\n" +
"}\n" +
"\n" +
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    check_for_up_and_down_text(paint);  // paint draws text left to right\n" +
"    paint.setVerticalText(true);\n" +
"    check_for_up_and_down_text(paint);  // paint draws text top to bottom\n" +
"    paint.setVerticalText(false);\n" +
"    canvas->rotate(90);\n" +
"    check_for_up_and_down_text(canvas, paint);  // paint draws text top to bottom\n" +
"}\n";

var SkCanvas_empty_constructor_json = {
    "code": SkCanvas_empty_constructor_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_empty_constructor",
    "overwrite": true
}

runFiddle(SkCanvas_empty_constructor_json);

var SkCanvas__int_int_const_SkSurfaceProps_star_code = 
"void draw(SkCanvas* ) {\n" + 
"    SkCanvas canvas(10, 20);  // 10 units wide, 20 units high\n" +
"    canvas.clipRect(SkRect::MakeXYWH(30, 40, 5, 10));  // clip is outside canvas' device\n" +
"    SkDebugf(\"canvas %s empty\\n\", canvas.getDeviceClipBounds().isEmpty() ? \"is\" : \"is not\");\n" +
"}\n";

var SkCanvas__int_int_const_SkSurfaceProps_star_json = {
    "code": SkCanvas__int_int_const_SkSurfaceProps_star_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas__int_int_const_SkSurfaceProps_star",
    "overwrite": true
}

runFiddle(SkCanvas__int_int_const_SkSurfaceProps_star_json);


var SkCanvas_copy_constructor_code = 
"void draw(SkCanvas* ) {\n" + 
"    SkBitmap bitmap;\n" +
"    // create a bitmap 5 wide and 11 high\n" +
"    bitmap.allocPixels(SkImageInfo::MakeN32Premul(5, 11));\n" +
"    SkCanvas canvas(bitmap);\n" +
"    canvas.clear(SK_ColorWHITE);  // white is unpremultiplied, in ARGB order\n" +
"    SkPixmap pixmap;  // provides guaranteed access to the drawn pixels\n" +
"	if (!canvas.peekPixels(&pixmap)) {\n" +
"        SkDebugf(\"peekPixels should never fail.\\n\");\n" +
"    }\n" +
"    const SkPMColor* pixels = pixmap.addr32();  // points to top left of bitmap\n" +
"    SkPMColor pmWhite = pixels[0];  // the premultiplied format may vary\n" +
"    SkPaint paint;  // by default, draws black, 12 point text\n" +
"    canvas.drawText(\"!\", 1, 1, 10, paint);  // 1 char at baseline (1, 10)\n" +
"    for (int y = 0; y < bitmap.height(); ++y) {\n" +
"        for (int x = 0; x < bitmap.width(); ++x) {\n" +
"            SkDebugf(\"%c\", *pixels++ == pmWhite ? '-' : 'x');\n" +
"        }\n" +
"        SkDebugf(\"\\n\");\n" +
"    }\n" +
"}\n";

var SkCanvas_copy_constructor_json = {
    "code": SkCanvas_copy_constructor_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_copy_constructor",
    "overwrite": true
}

runFiddle(SkCanvas_copy_constructor_json);

var SkCanvas__const_SkBitmap_const_SkSurfaceProps_code = 
"void draw(SkCanvas* ) {\n" + 
"    SkBitmap bitmap;\n" +
"    // create a bitmap 5 wide and 11 high\n" +
"    bitmap.allocPixels(SkImageInfo::MakeN32Premul(5, 11));\n" +
"    SkCanvas canvas(bitmap, SkSurfaceProps(0, kUnknown_SkPixelGeometry));\n" +
"    canvas.clear(SK_ColorWHITE);  // white is unpremultiplied, in ARGB order\n" +
"    SkPixmap pixmap;  // provides guaranteed access to the drawn pixels\n" +
"	if (!canvas.peekPixels(&pixmap)) {\n" +
"        SkDebugf(\"peekPixels should never fail.\\n\");\n" +
"    }\n" +
"    const SkPMColor* pixels = pixmap.addr32();  // points to top left of bitmap\n" +
"    SkPMColor pmWhite = pixels[0];  // the premultiplied format may vary\n" +
"    SkPaint paint;  // by default, draws black, 12 point text\n" +
"    canvas.drawText(\"!\", 1, 1, 10, paint);  // 1 char at baseline (1, 10)\n" +
"    for (int y = 0; y < bitmap.height(); ++y) {\n" +
"        for (int x = 0; x < bitmap.width(); ++x) {\n" +
"            SkDebugf(\"%c\", *pixels++ == pmWhite ? '-' : 'x');\n" +
"        }\n" +
"        SkDebugf(\"\\n\");\n" +
"    }\n" +
"}\n";

var SkCanvas__const_SkBitmap_const_SkSurfaceProps_json = {
    "code": SkCanvas__const_SkBitmap_const_SkSurfaceProps_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas__const_SkBitmap_const_SkSurfaceProps",
    "overwrite": true
}

runFiddle(SkCanvas__const_SkBitmap_const_SkSurfaceProps_json);


var SkCanvas_getMetaData_code = 
"void draw(SkCanvas* ) {\n" + 
"    const char* kHelloMetaData = \"HelloMetaData\";\n" +
"    SkCanvas canvas;\n" +
"    SkMetaData& metaData = canvas.getMetaData();\n" +
"    SkDebugf(\"before: %s\\n\", metaData.findString(kHelloMetaData));\n" +
"    metaData.setString(kHelloMetaData, \"Hello!\");\n" +
"    SkDebugf(\"during: %s\\n\", metaData.findString(kHelloMetaData));\n" +
"    metaData.removeString(kHelloMetaData);\n" +
"    SkDebugf(\"after: %s\\n\", metaData.findString(kHelloMetaData));\n" +
"}\n";

var SkCanvas_getMetaData_json = {
    "code": SkCanvas_getMetaData_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_getMetaData",
    "overwrite": true
}

runFiddle(SkCanvas_getMetaData_json);

var SkCanvas_imageInfo_code = 
"void draw(SkCanvas* ) {\n" + 
"    SkCanvas canvas;\n" +
"    SkImageInfo canvasInfo = canvas.imageInfo();\n" +
"    SkImageInfo emptyInfo;\n" +
"    SkDebugf(\"emptyInfo %c= canvasInfo\\n\", emptyInfo == canvasInfo ? '=' : '!');\n" +
"}\n";

var SkCanvas_imageInfo_json = {
    "code": SkCanvas_imageInfo_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_imageInfo",
    "overwrite": true
}

runFiddle(SkCanvas_imageInfo_json);

var SkCanvas_getProps_code = 
"void draw(SkCanvas* ) {\n" + 
"    SkBitmap bitmap;\n" +
"    SkCanvas canvas(bitmap, SkSurfaceProps(0, kRGB_V_SkPixelGeometry));\n" +
"    SkSurfaceProps surfaceProps(0, kUnknown_SkPixelGeometry);\n" +
"    SkDebugf(\"isRGB:%d\\n\", SkPixelGeometryIsRGB(surfaceProps.pixelGeometry()));\n" +
"    if (!canvas.getProps(&surfaceProps)) {\n" +
"        SkDebugf(\"getProps failed unexpectedly.\\n\");\n" +
"    }\n" +
"    SkDebugf(\"isRGB:%d\\n\", SkPixelGeometryIsRGB(surfaceProps.pixelGeometry()));\n" +
"}\n";

var SkCanvas_getProps_json = {
    "code": SkCanvas_getProps_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_getProps",
    "overwrite": true
}

runFiddle(SkCanvas_getProps_json);


var SkCanvas_getBaseLayerSize_code = 
"void draw(SkCanvas* ) {\n" + 
"    SkBitmap bitmap;\n" +
"    bitmap.allocPixels(SkImageInfo::MakeN32Premul(20, 30));\n" +
"    SkCanvas canvas(bitmap, SkSurfaceProps(0, kUnknown_SkPixelGeometry));\n" +
"    canvas.clipRect(SkRect::MakeWH(10, 40));\n" +
"    SkIRect clipDeviceBounds = canvas.getDeviceClipBounds();\n" +
"    if (clipDeviceBounds.isEmpty()) {\n" +
"        SkDebugf(\"Empty clip bounds is unexpected!\\n\");\n" +
"    }\n" +
"    SkDebugf(\"clip=%d,%d\\n\", clipDeviceBounds.width(), clipDeviceBounds.height());\n" +
"    SkISize baseLayerSize = canvas.getBaseLayerSize();\n" +
"    SkDebugf(\"size=%d,%d\\n\", baseLayerSize.width(), baseLayerSize.height());\n" +
"}\n";

var SkCanvas_getBaseLayerSize_json = {
    "code": SkCanvas_getBaseLayerSize_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_getBaseLayerSize",
    "overwrite": true
}

runFiddle(SkCanvas_getBaseLayerSize_json);

var SkCanvas_makeSurface_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(5, 6);\n" +
"        SkCanvas* smallCanvas = surface->getCanvas();\n" +
"        SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(3, 4);\n" +
"        sk_sp<SkSurface> compatible = smallCanvas->makeSurface(imageInfo);\n" +
"        SkDebugf(\"compatible %c= nullptr\\n\", compatible == nullptr ? '=' : '!');\n" +
"        SkDebugf(\"size = %d, %d\\n\", compatible->width(), compatible->height());\n" +
"}\n";

var SkCanvas_makeSurface_json = {
    "code": SkCanvas_makeSurface_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_makeSurface",
    "overwrite": true
}

runFiddle(SkCanvas_makeSurface_json);

var SkCanvas_getGrContext_code = 
"void draw(SkCanvas* canvas) {\n" +
"    if (canvas->getGrContext()) {\n" +
"         canvas->clear(SK_ColorRED);\n" +
"    } else {\n" +
"         canvas->clear(SK_ColorBLUE);\n" +
"    }\n" +
"}\n";

var SkCanvas_getGrContext_json = {
    "code": SkCanvas_getGrContext_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_getGrContext",
    "overwrite": true
}

runFiddle(SkCanvas_getGrContext_json);

var SkCanvas_accessTopLayerPixels_code = 
"void draw(SkCanvas* canvas) {\n" +
"    if (canvas->accessTopLayerPixels(nullptr, nullptr)) {\n" +
"         canvas->clear(SK_ColorRED);\n" +
"    } else {\n" +
"         canvas->clear(SK_ColorBLUE);\n" +
"    }\n" +
"}\n";

var SkCanvas_accessTopLayerPixels_json = {
    "code": SkCanvas_accessTopLayerPixels_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_accessTopLayerPixels",
    "overwrite": true
}

runFiddle(SkCanvas_accessTopLayerPixels_json);

var SkCanvas_accessTopLayerPixels_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"  SkPaint paint;\n" +
"  paint.setTextSize(100);\n" +
"  canvas->clear(SK_ColorWHITE);\n" +
"  canvas->drawText(\"ABC\", 3, 20, 160, paint);\n" +
"  SkRect layerBounds = SkRect::MakeXYWH(32, 32, 192, 192);\n" +
"  canvas->saveLayerAlpha(&layerBounds, 128);\n" +
"  canvas->clear(SK_ColorWHITE);\n" +
"  canvas->drawText(\"DEF\", 3, 20, 160, paint);\n" +
"  SkImageInfo imageInfo;\n" +
"  size_t rowBytes;\n" +
"  SkIPoint origin;\n" +
"  uint32_t* access = (uint32_t*) canvas->accessTopLayerPixels(&imageInfo, &rowBytes, &origin);\n" +
"  if (access) {\n" +
"    int h = imageInfo.height();\n" +
"    int v = imageInfo.width();\n" +
"    int rowWords = rowBytes / sizeof(uint32_t);\n" +
"    for (int y = 0; y < h; ++y) {\n" +
"        int newY = (y - h / 2) * 2 + h / 2;\n" +
"        if (newY < 0 || newY >= h) {\n" +
"            continue;\n" +
"        }\n" +
"        for (int x = 0; x < v; ++x) {\n" +
"            int newX = (x - v / 2) * 2 + v / 2;\n" +
"            if (newX < 0 || newX >= v) {\n" +
"                continue;\n" +
"            }\n" +
"            if (access[y * rowWords + x] == SK_ColorBLACK) {\n" +
"                access[newY * rowWords + newX] = SK_ColorGRAY;\n" +
"            }\n" +
"        }\n" +
"    }\n" +
"  }\n" +
"  canvas->restore();\n" +
"}\n";

var SkCanvas_accessTopLayerPixels_2_json = {
    "code": SkCanvas_accessTopLayerPixels_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_accessTopLayerPixels_2",
    "overwrite": true
}

runFiddle(SkCanvas_accessTopLayerPixels_2_json);

var SkCanvas_accessTopRasterHandle_code = 
"static void DeleteCallback(void*, void* context) {\n" +
"    delete (char*) context;\n" +
"}\n" +
"class CustomAllocator : public SkRasterHandleAllocator {\n" +
"public:\n" +
"    bool allocHandle(const SkImageInfo& info, Rec* rec) override {\n" +
"        char* context = new char[4]{'s', 'k', 'i', 'a'};\n" +
"        rec->fReleaseProc = DeleteCallback;\n" +
"        rec->fReleaseCtx = context;\n" +
"        rec->fHandle = context;\n" +
"        rec->fPixels = context;\n" +
"        rec->fRowBytes = 4;\n" +
"        return true;\n" +
"    }\n" +
"    void updateHandle(Handle handle, const SkMatrix& ctm, const SkIRect& clip_bounds) override {\n" +
"        // apply canvas matrix and clip to custom environment\n" +
"    }\n" +
"};\n" +
"\n" +
"void draw(SkCanvas* canvas) {\n" +
"    const SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);\n" +
"    std::unique_ptr<SkCanvas> c2 =\n" +
"            SkRasterHandleAllocator::MakeCanvas(std::unique_ptr<CustomAllocator>(\n" +
"            new CustomAllocator()), info);\n" +
"    char* context = (char*) c2->accessTopRasterHandle();\n" +
"    SkDebugf(\"context = %.4s\\n\", context);\n" +
"}\n";

var SkCanvas_accessTopRasterHandle_json = {
    "code": SkCanvas_accessTopRasterHandle_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_accessTopRasterHandle",
    "overwrite": true
}

runFiddle(SkCanvas_accessTopRasterHandle_json);

var SkCanvas_peekPixels_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPixmap pixmap;\n" +
"    if (canvas->peekPixels(&pixmap)) {\n" +
"        SkDebugf(\"width=%d height=%d\\n\", pixmap.bounds().width(), pixmap.bounds().height());\n" +
"    }\n" +
"}\n";

var SkCanvas_peekPixels_json = {
    "code": SkCanvas_peekPixels_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_peekPixels",
    "overwrite": true
}

runFiddle(SkCanvas_peekPixels_json);

var SkCanvas_readPixels_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    canvas->clear(0x8055aaff);\n" +
"    for (SkAlphaType alphaType : { kPremul_SkAlphaType, kUnpremul_SkAlphaType } ) {\n" +
"        uint32_t pixel = 0;\n" +
"        SkImageInfo info = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType, alphaType);\n" +
"        if (canvas->readPixels(info, &pixel, 4, 0, 0)) {\n" +
"            SkDebugf(\"pixel = %08x\\n\", pixel);\n" +
"        }\n" +
"    }\n" +
"}\n";

var SkCanvas_readPixels_json = {
    "code": SkCanvas_readPixels_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_readPixels",
    "overwrite": true
}

runFiddle(SkCanvas_readPixels_json);

var SkCanvas_readPixels_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(0x8055aaff);\n" +
"    uint32_t pixels[1] = { 0 };\n" +
"    SkPixmap pixmap(SkImageInfo::MakeN32Premul(1, 1), pixels, 4);\n" +
"    canvas->readPixels(pixmap, 0, 0);\n" +
"    SkDebugf(\"pixel = %08x\\n\", pixels[0]);\n" +
"}\n";

var SkCanvas_readPixels_2_json = {
    "code": SkCanvas_readPixels_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_readPixels_2",
    "overwrite": true
}

runFiddle(SkCanvas_readPixels_2_json);

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

var SkCanvas_writePixels_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkImageInfo imageInfo = SkImageInfo::MakeN32(256, 1, kPremul_SkAlphaType);\n" +
"    for (int y = 0; y < 256; ++y) {\n" +
"        uint32_t pixels[256];\n" +
"        for (int x = 0; x < 256; ++x) {\n" +
"            pixels[x] = SkColorSetARGB(x, x + y, x, x - y);\n" +
"        }\n" +
"        canvas->writePixels(imageInfo, &pixels, sizeof(pixels), 0, y);\n" +
"    }\n" +
"}\n";

var SkCanvas_writePixels_json = {
    "code": SkCanvas_writePixels_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_writePixels",
    "overwrite": true
}

runFiddle(SkCanvas_writePixels_json);

var SkCanvas_writePixels_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(2, 2);\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.setInfo(imageInfo);\n" +
"    uint32_t pixels[4];\n" +
"    bitmap.setPixels(pixels);\n" +
"    for (int y = 0; y < 256; y += 2) {\n" +
"        for (int x = 0; x < 256;  x += 2) {\n" +
"            pixels[0] = SkColorSetRGB(x, y, x | y);\n" +
"            pixels[1] = SkColorSetRGB(x ^ y, y, x);\n" +
"            pixels[2] = SkColorSetRGB(x, x & y, y);\n" +
"            pixels[3] = SkColorSetRGB(~x, ~y, x);\n" +
"            canvas->writePixels(bitmap, x, y);\n" +
"        }\n" +
"    }\n" +
"}\n";

var SkCanvas_writePixels_2_json = {
    "code": SkCanvas_writePixels_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_writePixels_2",
    "overwrite": true
}

runFiddle(SkCanvas_writePixels_2_json);

var Canvas_State_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    canvas->clear(SK_ColorWHITE);               // draws to entire destination\n" +
"    canvas->save();                             // records stack depth to restore  \n" +
"    canvas->clipRect(SkRect::MakeWH(100, 100)); // constrains drawing to clip\n" +
"    canvas->clear(SK_ColorRED);                 // draws to limit of clip\n" +
"    canvas->save();                             // records stack depth to restore \n" +
"    canvas->clipRect(SkRect::MakeWH(50, 150));  // Rect below 100 is ignored\n" +
"    canvas->clear(SK_ColorBLUE);                // draws to smaller clip\n" +
"    canvas->restore();                          // enlarges clip\n" +
"    canvas->drawLine(20, 20, 150, 150, paint);  // line below 100 is not drawn\n" +
"    canvas->restore();                          // enlarges clip\n" +
"    canvas->drawLine(150, 20, 50, 120, paint);  // line below 100 is drawn\n" +
"}\n";

var Canvas_State_json = {
    "code": Canvas_State_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Canvas_State",
    "overwrite": true
}

runFiddle(Canvas_State_json);

var Canvas_State_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clipRect(SkRect::MakeWH(100, 100));\n" +
"    canvas->clear(SK_ColorRED);\n" +
"    canvas->scale(.5, .5);\n" +
"    canvas->clipRect(SkRect::MakeWH(100, 100));\n" +
"    canvas->clear(SK_ColorBLUE);\n" +
"}\n";

var Canvas_State_2_json = {
    "code": Canvas_State_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Canvas_State_2",
    "overwrite": true
}

runFiddle(Canvas_State_2_json);

var SkCanvas_save_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    SkRect rect = { 0, 0, 25, 25 };\n" +
"    canvas->drawRect(rect, paint);\n" +
"    canvas->save();\n" +
"    canvas->translate(50, 50);\n" +
"    canvas->drawRect(rect, paint);\n" +
"    canvas->restore();\n" +
"    paint.setColor(SK_ColorRED);\n" +
"    canvas->drawRect(rect, paint);\n" +
"}\n";

var SkCanvas_save_json = {
    "code": SkCanvas_save_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_save",
    "overwrite": true
}

runFiddle(SkCanvas_save_json);

var SkCanvas_saveLayer_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint, blur;\n" +
"    blur.setImageFilter(SkImageFilter::MakeBlur(3, 3, nullptr));\n" +
"    canvas->saveLayer(nullptr, &blur);\n" +
"    SkRect rect = { 25, 25, 50, 50};\n" +
"    canvas->drawRect(rect, paint);\n" +
"    canvas->translate(50, 50);\n" +
"    paint.setColor(SK_ColorRED);\n" +
"    canvas->drawRect(rect, paint);\n" +
"    canvas->restore();\n" +
"}\n";

var SkCanvas_saveLayer_json = {
    "code": SkCanvas_saveLayer_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_saveLayer",
    "overwrite": true
}

runFiddle(SkCanvas_saveLayer_json);

var SkCanvas_saveLayer_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint, blur;\n" +
"    blur.setImageFilter(SkImageFilter::MakeBlur(3, 3, nullptr));\n" +
"    canvas->saveLayer(SkRect::MakeWH(90, 90), &blur);\n" +
"    SkRect rect = { 25, 25, 50, 50};\n" +
"    canvas->drawRect(rect, paint);\n" +
"    canvas->translate(50, 50);\n" +
"    paint.setColor(SK_ColorRED);\n" +
"    canvas->drawRect(rect, paint);\n" +
"    canvas->restore();\n" +
"}\n";

var SkCanvas_saveLayer_2_json = {
    "code": SkCanvas_saveLayer_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_saveLayer_2",
    "overwrite": true
}

runFiddle(SkCanvas_saveLayer_2_json);

var SkCanvas_saveLayerPreserveLCDTextRequests_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setLCDRenderText(true);\n" +
"    paint.setTextSize(20);\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    for (auto preserve : { false, true } ) {\n" +
"        preserve ? canvas->saveLayerPreserveLCDTextRequests(nullptr, nullptr)\n" +
"                 : canvas->saveLayer(nullptr, nullptr);\n" +
"        SkPaint p;\n" +
"        p.setColor(SK_ColorWHITE);\n" +
"        // Comment out the next line to draw on a non-opaque background.\n" +
"        canvas->drawRect(SkRect::MakeLTRB(25, 40, 200, 70), p);\n" +
"        canvas->drawText(\"Hamburgefons\", 12, 30, 60, paint);\n" +
"        p.setColor(0xFFCCCCCC);\n" +
"        canvas->drawRect(SkRect::MakeLTRB(25, 70, 200, 100), p);\n" +
"        canvas->drawText(\"Hamburgefons\", 12, 30, 90, paint);\n" +
"        canvas->restore();\n" +
"        canvas->translate(0, 80);\n" +
"    }\n" +
"}\n";

var SkCanvas_saveLayerPreserveLCDTextRequests_json = {
    "code": SkCanvas_saveLayerPreserveLCDTextRequests_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_saveLayerPreserveLCDTextRequests",
    "overwrite": true
}

runFiddle(SkCanvas_saveLayerPreserveLCDTextRequests_json);

var SkCanvas_saveLayerAlpha_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint paint;\n" +
"    paint.setColor(SK_ColorRED);\n" +
"    canvas->drawCircle(50, 50, 50, paint);\n" +
"    canvas->saveLayerAlpha(nullptr, 128);\n" +
"    paint.setColor(SK_ColorBLUE);\n" +
"    canvas->drawCircle(100, 50, 50, paint);\n" +
"    paint.setColor(SK_ColorGREEN);\n" +
"    paint.setAlpha(128);\n" +
"    canvas->drawCircle(75, 90, 50, paint);\n" +
"    canvas->restore();\n" +
"}\n";

var SkCanvas_saveLayerAlpha_json = {
    "code": SkCanvas_saveLayerAlpha_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_saveLayerAlpha",
    "overwrite": true
}

runFiddle(SkCanvas_saveLayerAlpha_json);

var SkCanvas_SaveLayerFlags_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint redPaint, bluePaint, scalePaint;\n" +
"    redPaint.setColor(SK_ColorRED);\n" +
"    canvas->drawCircle(21, 21, 8, redPaint);\n" +
"    bluePaint.setColor(SK_ColorBLUE);\n" +
"    canvas->drawCircle(31, 21, 8, bluePaint);\n" +
"    SkMatrix matrix;\n" +
"    matrix.setScale(4, 4);\n" +
"    scalePaint.setAlpha(0x40);\n" +
"    scalePaint.setImageFilter(SkImageFilter::MakeMatrixFilter(matrix, kNone_SkFilterQuality, nullptr));\n" +
"    SkCanvas::SaveLayerRec saveLayerRec(nullptr, &scalePaint, SkCanvas::kInitWithPrevious_SaveLayerFlag); \n" +
"    canvas->saveLayer(saveLayerRec);\n" +
"    canvas->restore();\n" +
"}\n";

var SkCanvas_SaveLayerFlags_json = {
    "code": SkCanvas_SaveLayerFlags_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_SaveLayerFlags",
    "overwrite": true
}

runFiddle(SkCanvas_SaveLayerFlags_json);

var SkCanvas_SaveLayerRec_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint redPaint, bluePaint;\n" +
"    redPaint.setAntiAlias(true);\n" +
"    redPaint.setColor(SK_ColorRED);\n" +
"    canvas->drawCircle(21, 21, 8, redPaint);\n" +
"    bluePaint.setColor(SK_ColorBLUE);\n" +
"    canvas->drawCircle(31, 21, 8, bluePaint);\n" +
"    SkMatrix matrix;\n" +
"    matrix.setScale(4, 4);\n" +
"    auto scaler = SkImageFilter::MakeMatrixFilter(matrix, kNone_SkFilterQuality, nullptr);\n" +
"    SkCanvas::SaveLayerRec saveLayerRec(nullptr, nullptr, scaler.get(), 0); \n" +
"    canvas->saveLayer(saveLayerRec);\n" +
"    canvas->drawCircle(125, 85, 8, redPaint);\n" +
"    canvas->restore();\n" +
"}\n";

var SkCanvas_SaveLayerRec_json = {
    "code": SkCanvas_SaveLayerRec_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_SaveLayerRec",
    "overwrite": true
}

runFiddle(SkCanvas_SaveLayerRec_json);

var SkCanvas_SaveLayerRec_SaveLayerRec_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkCanvas::SaveLayerRec rec1;\n" +
"    rec1.fSaveLayerFlags = SkCanvas::kIsOpaque_SaveLayerFlag;\n" +
"    SkCanvas::SaveLayerRec rec2(nullptr, nullptr, SkCanvas::kIsOpaque_SaveLayerFlag);\n" +
"    SkDebugf(\"rec1 %c= rec2\\n\", rec1.fBounds == rec2.fBounds\n" +
"            && rec1.fPaint == rec2.fPaint\n" +
"            && rec1.fBackdrop == rec2.fBackdrop\n" +
"            && rec1.fSaveLayerFlags == rec2.fSaveLayerFlags ? '=' : '!');\n" +
"}\n";

var SkCanvas_SaveLayerRec_SaveLayerRec_json = {
    "code": SkCanvas_SaveLayerRec_SaveLayerRec_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_SaveLayerRec_SaveLayerRec",
    "overwrite": true
}

runFiddle(SkCanvas_SaveLayerRec_SaveLayerRec_json);

var SkCanvas_SaveLayerRec_SaveLayerRec_2_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkCanvas::SaveLayerRec rec1;\n" +
"    SkCanvas::SaveLayerRec rec2(nullptr, nullptr);\n" +
"    SkDebugf(\"rec1 %c= rec2\\n\", rec1.fBounds == rec2.fBounds\n" +
"            && rec1.fPaint == rec2.fPaint\n" +
"            && rec1.fBackdrop == rec2.fBackdrop\n" +
"            && rec1.fSaveLayerFlags == rec2.fSaveLayerFlags ? '=' : '!');\n" +
"}\n";

var SkCanvas_SaveLayerRec_SaveLayerRec_2_json = {
    "code": SkCanvas_SaveLayerRec_SaveLayerRec_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_SaveLayerRec_SaveLayerRec_2",
    "overwrite": true
}

runFiddle(SkCanvas_SaveLayerRec_SaveLayerRec_2_json);

var SkCanvas_SaveLayerRec_SaveLayerRec_3_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkCanvas::SaveLayerRec rec1;\n" +
"    SkCanvas::SaveLayerRec rec2(nullptr, nullptr, nullptr, 0);\n" +
"    SkDebugf(\"rec1 %c= rec2\\n\", rec1.fBounds == rec2.fBounds\n" +
"            && rec1.fPaint == rec2.fPaint\n" +
"            && rec1.fBackdrop == rec2.fBackdrop\n" +
"            && rec1.fSaveLayerFlags == rec2.fSaveLayerFlags ? '=' : '!');\n" +
"}\n";

var SkCanvas_SaveLayerRec_SaveLayerRec_3_json = {
    "code": SkCanvas_SaveLayerRec_SaveLayerRec_3_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_SaveLayerRec_SaveLayerRec_3",
    "overwrite": true
}

runFiddle(SkCanvas_SaveLayerRec_SaveLayerRec_3_json);

var SkCanvas_saveLayer_3_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    // sk_sp<SkImage> image = GetResourceAsImage(\"mandrill_256.png\");\n" +
"    canvas->drawImage(image, 0, 0, nullptr);\n" +
"    SkCanvas::SaveLayerRec rec;\n" +
"    SkPaint paint;\n" +
"    paint.setBlendMode(SkBlendMode::kPlus);\n" +
"    rec.fSaveLayerFlags = SkCanvas::kInitWithPrevious_SaveLayerFlag;\n" +
"    rec.fPaint = &paint;\n" +
"    canvas->saveLayer(rec);\n" +
"    paint.setBlendMode(SkBlendMode::kClear);\n" +
"    canvas->drawCircle(128, 128, 96, paint);\n" +
"    canvas->restore();\n" +
"}\n";

var SkCanvas_saveLayer_3_json = {
    "code": SkCanvas_saveLayer_3_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 3,
        "textOnly": false
    },
    "name": "SkCanvas_saveLayer_3",
    "overwrite": true
}

runFiddle(SkCanvas_saveLayer_3_json);

var SkCanvas_restore_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkCanvas simple;\n" +
"    SkDebugf(\"depth = %d\\n\", simple.getSaveCount());\n" +
"    simple.restore();\n" +
"    SkDebugf(\"depth = %d\\n\", simple.getSaveCount());\n" +
"}\n";

var SkCanvas_restore_json = {
    "code": SkCanvas_restore_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_restore",
    "overwrite": true
}

runFiddle(SkCanvas_restore_json);

var SkCanvas_getSaveCount_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkCanvas simple;\n" +
"    SkDebugf(\"depth = %d\\n\", simple.getSaveCount());\n" +
"    simple.save();\n" +
"    SkDebugf(\"depth = %d\\n\", simple.getSaveCount());\n" +
"    simple.restore();\n" +
"    SkDebugf(\"depth = %d\\n\", simple.getSaveCount());\n" +
"}\n";

var SkCanvas_getSaveCount_json = {
    "code": SkCanvas_getSaveCount_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_getSaveCount",
    "overwrite": true
}

runFiddle(SkCanvas_getSaveCount_json);

var SkCanvas_restoreToCount_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkDebugf(\"depth = %d\\n\", canvas->getSaveCount());\n" +
"    canvas->save();\n" +
"    canvas->save();\n" +
"    SkDebugf(\"depth = %d\\n\", canvas->getSaveCount());\n" +
"    canvas->restoreToCount(0);\n" +
"    SkDebugf(\"depth = %d\\n\", canvas->getSaveCount());\n" +
"}\n";

var SkCanvas_restoreToCount_json = {
    "code": SkCanvas_restoreToCount_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_restoreToCount",
    "overwrite": true
}

runFiddle(SkCanvas_restoreToCount_json);

var SkCanvas_translate_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint filledPaint;\n" +
"    SkPaint outlinePaint;\n" +
"    outlinePaint.setStyle(SkPaint::kStroke_Style);\n" +
"    outlinePaint.setColor(SK_ColorBLUE);\n" +
"    canvas->save();\n" +
"    canvas->translate(50, 50);\n" +
"    canvas->drawCircle(28, 28, 15, outlinePaint);  // blue center: (50+28, 50+28)\n" +
"    canvas->scale(2, 1/2.f);\n" +
"    canvas->drawCircle(28, 28, 15, filledPaint);   // black center: (50+(28*2), 50+(28/2))\n" +
"    canvas->restore();\n" +
"    filledPaint.setColor(SK_ColorGRAY);\n" +
"    outlinePaint.setColor(SK_ColorRED);\n" +
"    canvas->scale(2, 1/2.f);\n" +
"    canvas->drawCircle(28, 28, 15, outlinePaint);  // red center: (28*2, 28/2)\n" +
"    canvas->translate(50, 50);\n" +
"    canvas->drawCircle(28, 28, 15, filledPaint);   // gray center: ((50+28)*2, (50+28)/2)\n" +
"}\n";

var SkCanvas_translate_json = {
    "code": SkCanvas_translate_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_translate",
    "overwrite": true
}

runFiddle(SkCanvas_translate_json);

var SkCanvas_scale_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    SkRect rect = { 10, 20, 60, 120 };\n" +
"    canvas->translate(20, 20);\n" +
"    canvas->drawRect(rect, paint);\n" +
"    canvas->scale(2, .5f);\n" +
"    paint.setColor(SK_ColorGRAY);\n" +
"    canvas->drawRect(rect, paint);\n" +
"}\n";

var SkCanvas_scale_json = {
    "code": SkCanvas_scale_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_scale",
    "overwrite": true
}

runFiddle(SkCanvas_scale_json);

var SkCanvas_rotate_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    canvas->translate(128, 128);\n" +
"    canvas->drawCircle(0, 0, 60, paint);\n" +
"    canvas->save();\n" +
"    canvas->rotate(10 * 360 / 60);   // 10 minutes of 60 scaled to 360 degrees\n" +
"    canvas->drawLine(0, 0, 0, -50, paint); \n" +
"    canvas->restore();\n" +
"    canvas->rotate(5 * 360 / 12);    // 5 hours of 12 scaled to 360 degrees\n" +
"    canvas->drawLine(0, 0, 0, -30, paint);\n" +
"}\n";

var SkCanvas_rotate_json = {
    "code": SkCanvas_rotate_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_rotate",
    "overwrite": true
}

runFiddle(SkCanvas_rotate_json);

var SkCanvas_rotate_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(96);\n" +
"    canvas->drawText(\"A1\", 2, 130, 100, paint);\n" +
"    canvas->rotate(180, 130, 100);\n" +
"    canvas->drawText(\"A1\", 2, 130, 100, paint);\n" +
"}\n";

var SkCanvas_rotate_2_json = {
    "code": SkCanvas_rotate_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_rotate_2",
    "overwrite": true
}

runFiddle(SkCanvas_rotate_2_json);

var SkCanvas_skew_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkPaint paint;\n" +
"    paint.setTextSize(128);\n" +
"    canvas->translate(30, 130);\n" +
"    canvas->save();\n" +
"    canvas->skew(-.5, 0);\n" +
"    canvas->drawText(\"A1\", 2, 0, 0, paint);\n" +
"    canvas->restore();\n" +
"    canvas->save();\n" +
"    canvas->skew(0, .5);\n" +
"    paint.setColor(SK_ColorRED);\n" +
"    canvas->drawText(\"A1\", 2, 0, 0, paint);\n" +
"    canvas->restore();\n" +
"    canvas->skew(-.5, .5);\n" +
"    paint.setColor(SK_ColorBLUE);\n" +
"    canvas->drawText(\"A1\", 2, 0, 0, paint);\n" +
"}\n";

var SkCanvas_skew_json = {
    "code": SkCanvas_skew_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_skew",
    "overwrite": true
}

runFiddle(SkCanvas_skew_json);

var SkCanvas_concat_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(80);\n" +
"    paint.setTextScaleX(.3);\n" +
"    SkMatrix matrix;\n" +
"    SkRect rect[2] = {{ 10, 20, 90, 110 }, { 40, 130, 140, 180 }};\n" +
"    matrix.setRectToRect(rect[0], rect[1], SkMatrix::kFill_ScaleToFit);\n" +
"    canvas->drawRect(rect[0], paint);\n" +
"    canvas->drawRect(rect[1], paint);\n" +
"    paint.setColor(SK_ColorWHITE);\n" +
"    canvas->drawText(\"Here\", 4, rect[0].fLeft + 10, rect[0].fBottom - 10, paint);\n" +
"    canvas->concat(matrix);\n" +
"    canvas->drawText(\"There\", 5, rect[0].fLeft + 10, rect[0].fBottom - 10, paint);\n" +
"}\n";

var SkCanvas_concat_json = {
    "code": SkCanvas_concat_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_concat",
    "overwrite": true
}

runFiddle(SkCanvas_concat_json);

var SkCanvas_setMatrix_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    canvas->scale(4, 6);\n" +
"    canvas->drawText(\"truth\", 5, 2, 10, paint);\n" +
"    SkMatrix matrix;\n" +
"    matrix.setScale(2.8f, 6);\n" +
"    canvas->setMatrix(matrix);\n" +
"    canvas->drawText(\"consequences\", 12, 2, 20, paint);\n" +
"}\n";

var SkCanvas_setMatrix_json = {
    "code": SkCanvas_setMatrix_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_setMatrix",
    "overwrite": true
}

runFiddle(SkCanvas_setMatrix_json);

var SkCanvas_resetMatrix_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    canvas->scale(4, 6);\n" +
"    canvas->drawText(\"truth\", 5, 2, 10, paint);\n" +
"    canvas->resetMatrix();\n" +
"    canvas->scale(2.8f, 6);\n" +
"    canvas->drawText(\"consequences\", 12, 2, 20, paint);\n" +
"}\n";

var SkCanvas_resetMatrix_json = {
    "code": SkCanvas_resetMatrix_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_resetMatrix",
    "overwrite": true
}

runFiddle(SkCanvas_resetMatrix_json);

var SkCanvas_getTotalMatrix_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkDebugf(\"isIdentity %s\\n\", canvas->getTotalMatrix().isIdentity() ? \"true\" : \"false\");\n" +
"}\n";

var SkCanvas_getTotalMatrix_json = {
    "code": SkCanvas_getTotalMatrix_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_getTotalMatrix",
    "overwrite": true
}

runFiddle(SkCanvas_getTotalMatrix_json);

var Canvas_Clip_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint redPaint, scalePaint;\n" +
"    redPaint.setAntiAlias(true);\n" +
"    redPaint.setColor(SK_ColorRED);\n" +
"    canvas->save();\n" +
"    for (bool antialias : { false, true } ) {\n" +
"        canvas->save();\n" +
"        canvas->clipRect(SkRect::MakeWH(19.5f, 21.5f), antialias);\n" +
"        canvas->drawCircle(17, 21, 8, redPaint);\n" +
"        canvas->restore();\n" +
"        canvas->translate(16, 0);\n" +
"    }\n" +
"    canvas->restore();\n" +
"    SkMatrix matrix;\n" +
"    matrix.setScale(6, 6);\n" +
"    scalePaint.setImageFilter(\n" +
"            SkImageFilter::MakeMatrixFilter(matrix, kNone_SkFilterQuality, nullptr));\n" +
"    SkCanvas::SaveLayerRec saveLayerRec(\n" +
"            nullptr, &scalePaint, SkCanvas::kInitWithPrevious_SaveLayerFlag); \n" +
"    canvas->saveLayer(saveLayerRec);\n" +
"    canvas->restore();\n" +
"}\n";

var Canvas_Clip_json = {
    "code": Canvas_Clip_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "Canvas_Clip",
    "overwrite": true
}

runFiddle(Canvas_Clip_json);

var SkCanvas_clipRect_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    canvas->rotate(10);\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    for (auto alias: { false, true } ) {\n" +
"        canvas->save();\n" +
"        canvas->clipRect(SkRect::MakeWH(90, 120), SkClipOp::kIntersect, alias);\n" +
"        canvas->drawCircle(100, 100, 60, paint);\n" +
"        canvas->restore();\n" +
"        canvas->translate(80, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_clipRect_json = {
    "code": SkCanvas_clipRect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_clipRect",
    "overwrite": true
}

runFiddle(SkCanvas_clipRect_json);

var SkCanvas_clipRect_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint paint;\n" +
"    for (SkClipOp op: { SkClipOp::kIntersect, SkClipOp::kDifference } ) {\n" +
"        canvas->save();\n" +
"        canvas->clipRect(SkRect::MakeWH(90, 120), op, false);\n" +
"        canvas->drawCircle(100, 100, 60, paint);\n" +
"        canvas->restore();\n" +
"        canvas->translate(80, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_clipRect_2_json = {
    "code": SkCanvas_clipRect_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_clipRect_2",
    "overwrite": true
}

runFiddle(SkCanvas_clipRect_2_json);

var SkCanvas_clipRect_3_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setColor(0x8055aaff);\n" +
"    SkRect clipRect = { 0, 0, 90.4f, 120.4f };\n" +
"    for (auto alias: { false, true } ) {\n" +
"        canvas->save();\n" +
"        canvas->clipRect(clipRect, SkClipOp::kIntersect, alias);\n" +
"        canvas->drawCircle(70, 100, 60, paint);\n" +
"        canvas->restore();\n" +
"        canvas->save();\n" +
"        canvas->clipRect(clipRect, SkClipOp::kDifference, alias);\n" +
"        canvas->drawCircle(70, 100, 60, paint);\n" +
"        canvas->restore();\n" +
"        canvas->translate(120, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_clipRect_3_json = {
    "code": SkCanvas_clipRect_3_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_clipRect_3",
    "overwrite": true
}

runFiddle(SkCanvas_clipRect_3_json);

var SkCanvas_clipRRect_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setColor(0x8055aaff);\n" +
"    SkRRect oval;\n" +
"    oval.setOval({10, 20, 90, 100});\n" +
"    canvas->clipRRect(oval, SkClipOp::kIntersect, true);\n" +
"    canvas->drawCircle(70, 100, 60, paint);\n" +
"}\n";

var SkCanvas_clipRRect_json = {
    "code": SkCanvas_clipRRect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_clipRRect",
    "overwrite": true
}

runFiddle(SkCanvas_clipRRect_json);

var SkCanvas_clipRRect_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint paint;\n" +
"    paint.setColor(0x8055aaff);\n" +
"    auto oval = SkRRect::MakeOval({10, 20, 90, 100});\n" +
"    canvas->clipRRect(oval, SkClipOp::kIntersect);\n" +
"    canvas->drawCircle(70, 100, 60, paint);\n" +
"}\n";

var SkCanvas_clipRRect_2_json = {
    "code": SkCanvas_clipRRect_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_clipRRect_2",
    "overwrite": true
}

runFiddle(SkCanvas_clipRRect_2_json);

var SkCanvas_clipRRect_3_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    auto oval = SkRRect::MakeRectXY({10, 20, 90, 100}, 9, 13);\n" +
"    canvas->clipRRect(oval, true);\n" +
"    canvas->drawCircle(70, 100, 60, paint);\n" +
"}\n";

var SkCanvas_clipRRect_3_json = {
    "code": SkCanvas_clipRRect_3_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_clipRRect_3",
    "overwrite": true
}

runFiddle(SkCanvas_clipRRect_3_json);

var SkCanvas_clipPath_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    SkPath path;\n" +
"    path.addRect({20, 30, 100, 110});\n" +
"    path.setFillType(SkPath::kInverseWinding_FillType);\n" +
"    canvas->save();\n" +
"    canvas->clipPath(path, SkClipOp::kDifference, false);\n" +
"    canvas->drawCircle(70, 100, 60, paint);\n" +
"    canvas->restore();\n" +
"    canvas->translate(100, 100);\n" +
"    path.setFillType(SkPath::kWinding_FillType);\n" +
"    canvas->clipPath(path, SkClipOp::kIntersect, false);\n" +
"    canvas->drawCircle(70, 100, 60, paint);\n" +
"}\n";

var SkCanvas_clipPath_json = {
    "code": SkCanvas_clipPath_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_clipPath",
    "overwrite": true
}

runFiddle(SkCanvas_clipPath_json);

var SkCanvas_clipPath_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    SkPath path;\n" +
"    path.addRect({20, 30, 100, 110});\n" +
"    path.addRect({50, 80, 130, 150});\n" +
"    path.setFillType(SkPath::kWinding_FillType);\n" +
"    canvas->save();\n" +
"    canvas->clipPath(path, SkClipOp::kIntersect);\n" +
"    canvas->drawCircle(70, 100, 60, paint);\n" +
"    canvas->restore();\n" +
"    canvas->translate(100, 100);\n" +
"    path.setFillType(SkPath::kEvenOdd_FillType);\n" +
"    canvas->clipPath(path, SkClipOp::kIntersect);\n" +
"    canvas->drawCircle(70, 100, 60, paint);\n" +
"}\n";

var SkCanvas_clipPath_2_json = {
    "code": SkCanvas_clipPath_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_clipPath_2",
    "overwrite": true
}

runFiddle(SkCanvas_clipPath_2_json);

var SkCanvas_clipPath_3_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    SkPath path;\n" +
"    SkPoint poly[] = {{20, 20}, {80, 20}, {80, 80}, {40, 80}, {40, 40}, {100, 40}, {100, 100}, {20, 100}};\n" +
"    path.addPoly(poly, SK_ARRAY_COUNT(poly), true);\n" +
"    path.setFillType(SkPath::kWinding_FillType);\n" +
"    canvas->save();\n" +
"    canvas->clipPath(path, SkClipOp::kIntersect);\n" +
"    canvas->drawCircle(50, 50, 45, paint);\n" +
"    canvas->restore();\n" +
"    canvas->translate(100, 100);\n" +
"    path.setFillType(SkPath::kEvenOdd_FillType);\n" +
"    canvas->clipPath(path, SkClipOp::kIntersect);\n" +
"    canvas->drawCircle(50, 50, 45, paint);\n" +
"}\n";

var SkCanvas_clipPath_3_json = {
    "code": SkCanvas_clipPath_3_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_clipPath_3",
    "overwrite": true
}

runFiddle(SkCanvas_clipPath_3_json);


var SkCanvas_clipRegion_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint paint;\n" +
"    SkIRect iRect = {30, 40, 120, 130 };\n" +
"    SkRegion region(iRect);\n" +
"    canvas->rotate(10);\n" +
"    canvas->save();\n" +
"    canvas->clipRegion(region, SkClipOp::kIntersect);\n" +
"    canvas->drawCircle(50, 50, 45, paint);\n" +
"    canvas->restore();\n" +
"    canvas->translate(100, 100);\n" +
"    canvas->clipRect(SkRect::Make(iRect), SkClipOp::kIntersect);\n" +
"    canvas->drawCircle(50, 50, 45, paint);\n" +
"}\n";

var SkCanvas_clipRegion_json = {
    "code": SkCanvas_clipRegion_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_clipRegion",
    "overwrite": true
}

runFiddle(SkCanvas_clipRegion_json);

var SkCanvas_quickReject_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkRect testRect = {30, 30, 120, 129 }; \n" +
"    SkRect clipRect = {30, 130, 120, 230 }; \n" +
"    canvas->save();\n" +
"    canvas->clipRect(clipRect);\n" +
"    SkDebugf(\"quickReject %s\\n\", canvas->quickReject(testRect) ? \"true\" : \"false\");\n" +
"    canvas->restore();\n" +
"    canvas->rotate(10);\n" +
"    canvas->clipRect(clipRect);\n" +
"    SkDebugf(\"quickReject %s\\n\", canvas->quickReject(testRect) ? \"true\" : \"false\");\n" +
"}\n";

var SkCanvas_quickReject_json = {
    "code": SkCanvas_quickReject_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_quickReject",
    "overwrite": true
}

runFiddle(SkCanvas_quickReject_json);

var SkCanvas_quickReject_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPoint testPoints[] = {{30,  30}, {120,  30}, {120, 129} }; \n" +
"    SkPoint clipPoints[] = {{30, 130}, {120, 130}, {120, 230} }; \n" +
"    SkPath testPath, clipPath;\n" +
"    testPath.addPoly(testPoints, SK_ARRAY_COUNT(testPoints), true);\n" +
"    clipPath.addPoly(clipPoints, SK_ARRAY_COUNT(clipPoints), true);\n" +
"    canvas->save();\n" +
"    canvas->clipPath(clipPath);\n" +
"    SkDebugf(\"quickReject %s\\n\", canvas->quickReject(testPath) ? \"true\" : \"false\");\n" +
"    canvas->restore();\n" +
"    canvas->rotate(10);\n" +
"    canvas->clipPath(clipPath);\n" +
"    SkDebugf(\"quickReject %s\\n\", canvas->quickReject(testPath) ? \"true\" : \"false\");\n" +
"}\n";

var SkCanvas_quickReject_2_json = {
    "code": SkCanvas_quickReject_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_quickReject_2",
    "overwrite": true
}

runFiddle(SkCanvas_quickReject_2_json);

var SkCanvas_getLocalClipBounds_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    SkCanvas local(256, 256);\n" +
"    canvas = &local;\n" +
"    SkRect bounds = canvas->getLocalClipBounds();\n" +
"    SkDebugf(\"left:%g  top:%g  right:%g  bottom:%g\\n\",\n" +
"            bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);\n" +
"    SkPoint clipPoints[]  = {{30, 130}, {120, 130}, {120, 230} }; \n" +
"    SkPath clipPath;\n" +
"    clipPath.addPoly(clipPoints, SK_ARRAY_COUNT(clipPoints), true);\n" +
"    canvas->clipPath(clipPath);\n" +
"    bounds = canvas->getLocalClipBounds();\n" +
"    SkDebugf(\"left:%g  top:%g  right:%g  bottom:%g\\n\",\n" +
"            bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);\n" +
"    canvas->scale(2, 2);\n" +
"    bounds = canvas->getLocalClipBounds();\n" +
"    SkDebugf(\"left:%g  top:%g  right:%g  bottom:%g\\n\",\n" +
"            bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);\n" +
"}\n";

var SkCanvas_getLocalClipBounds_json = {
    "code": SkCanvas_getLocalClipBounds_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_getLocalClipBounds",
    "overwrite": true
}

runFiddle(SkCanvas_getLocalClipBounds_json);

var SkCanvas_getLocalClipBounds_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkCanvas local(256, 256);\n" +
"    canvas = &local;\n" +
"    SkRect bounds;\n" +
"    SkDebugf(\"local bounds empty = %s\\n\", canvas->getLocalClipBounds(&bounds)\n" +
"             ? \"false\" : \"true\");\n" +
"    SkPath path;\n" +
"    canvas->clipPath(path);\n" +
"    SkDebugf(\"local bounds empty = %s\\n\", canvas->getLocalClipBounds(&bounds)\n" +
"             ? \"false\" : \"true\");\n" +
"}\n";

var SkCanvas_getLocalClipBounds_2_json = {
    "code": SkCanvas_getLocalClipBounds_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_getLocalClipBounds_2",
    "overwrite": true
}

runFiddle(SkCanvas_getLocalClipBounds_2_json);

var SkCanvas_getDeviceClipBounds_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkCanvas device(256, 256);\n" +
"    canvas = &device;\n" +
"    SkIRect bounds = canvas->getDeviceClipBounds();\n" +
"    SkDebugf(\"left:%d  top:%d  right:%d  bottom:%d\\n\",\n" +
"            bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);\n" +
"    SkPoint clipPoints[]  = {{30, 130}, {120, 130}, {120, 230} }; \n" +
"    SkPath clipPath;\n" +
"    clipPath.addPoly(clipPoints, SK_ARRAY_COUNT(clipPoints), true);\n" +
"    canvas->save();\n" +
"    canvas->clipPath(clipPath);\n" +
"    bounds = canvas->getDeviceClipBounds();\n" +
"    SkDebugf(\"left:%d  top:%d  right:%d  bottom:%d\\n\",\n" +
"            bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);\n" +
"    canvas->restore();\n" +
"    canvas->scale(1.f/2, 1.f/2);\n" +
"    canvas->clipPath(clipPath);\n" +
"    bounds = canvas->getDeviceClipBounds();\n" +
"    SkDebugf(\"left:%d  top:%d  right:%d  bottom:%d\\n\",\n" +
"            bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);\n" +
"}\n";

var SkCanvas_getDeviceClipBounds_json = {
    "code": SkCanvas_getDeviceClipBounds_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_getDeviceClipBounds",
    "overwrite": true
}

runFiddle(SkCanvas_getDeviceClipBounds_json);

var SkCanvas_getDeviceClipBounds_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkIRect bounds;\n" +
"    SkDebugf(\"device bounds empty = %s\\n\", canvas->getDeviceClipBounds(&bounds)\n" +
"             ? \"false\" : \"true\");\n" +
"    SkPath path;\n" +
"    canvas->clipPath(path);\n" +
"    SkDebugf(\"device bounds empty = %s\\n\", canvas->getDeviceClipBounds(&bounds)\n" +
"             ? \"false\" : \"true\");\n" +
"}\n";

var SkCanvas_getDeviceClipBounds_2_json = {
    "code": SkCanvas_getDeviceClipBounds_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_getDeviceClipBounds_2",
    "overwrite": true
}

runFiddle(SkCanvas_getDeviceClipBounds_2_json);

var SkCanvas_drawColor_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    canvas->drawColor(SK_ColorRED);\n" +
"    canvas->clipRect(SkRect::MakeWH(150, 150));\n" +
"    canvas->drawColor(SkColorSetARGB(0x80, 0x00, 0xFF, 0x00), SkBlendMode::kPlus);\n" +
"    canvas->clipRect(SkRect::MakeWH(75, 75));\n" +
"    canvas->drawColor(SkColorSetARGB(0x80, 0x00, 0x00, 0xFF), SkBlendMode::kPlus);\n" +
"}\n";

var SkCanvas_drawColor_json = {
    "code": SkCanvas_drawColor_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawColor",
    "overwrite": true
}

runFiddle(SkCanvas_drawColor_json);

var SkCanvas_clear_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->save();\n" +
"    canvas->clipRect(SkRect::MakeWH(256, 128));\n" +
"    canvas->clear(SkColorSetARGB(0x80, 0xFF, 0x00, 0x00)); \n" +
"    canvas->restore();\n" +
"    canvas->save();\n" +
"    canvas->clipRect(SkRect::MakeWH(150, 192));\n" +
"    canvas->clear(SkColorSetARGB(0x80, 0x00, 0xFF, 0x00));\n" +
"    canvas->restore();\n" +
"    canvas->clipRect(SkRect::MakeWH(75, 256));\n" +
"    canvas->clear(SkColorSetARGB(0x80, 0x00, 0x00, 0xFF));\n" +
"}\n";

var SkCanvas_clear_json = {
    "code": SkCanvas_clear_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_clear",
    "overwrite": true
}

runFiddle(SkCanvas_clear_json);

var SkCanvas_drawPaint_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkColor     colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };\n" +
"    SkScalar    pos[] = { 0, SK_Scalar1/2, SK_Scalar1 };\n" +
"    SkPaint     paint;\n" +
"    paint.setShader(SkGradientShader::MakeSweep(256, 256, colors, pos, SK_ARRAY_COUNT(colors)));\n" +
"    canvas->drawPaint(paint);\n" +
"}\n";

var SkCanvas_drawPaint_json = {
    "code": SkCanvas_drawPaint_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawPaint",
    "overwrite": true
}

runFiddle(SkCanvas_drawPaint_json);

var SkCanvas_PointMode_code = 
"void draw(SkCanvas* canvas) {\n" +
"  SkPaint paint;\n" +
"  paint.setStyle(SkPaint::kStroke_Style);\n" +
"  paint.setStrokeWidth(10);\n" +
"  SkPoint points[] = {{64, 32}, {96, 96}, {32, 96}};\n" +
"  canvas->drawPoints(SkCanvas::kPoints_PointMode, 3, points, paint);\n" +
"  canvas->translate(128, 0);\n" +
"  canvas->drawPoints(SkCanvas::kLines_PointMode, 3, points, paint);\n" +
"  canvas->translate(0, 128);\n" +
"  canvas->drawPoints(SkCanvas::kPolygon_PointMode, 3, points, paint);\n" +
"  SkPath path;\n" +
"  path.addPoly(points, 3, false);\n" +
"  canvas->translate(-128, 0);\n" +
"  canvas->drawPath(path, paint);\n" +
"}\n";

var SkCanvas_PointMode_json = {
    "code": SkCanvas_PointMode_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_PointMode",
    "overwrite": true
}

runFiddle(SkCanvas_PointMode_json);

var SkCanvas_drawPoints_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(10);\n" +
"    paint.setColor(0x80349a45);\n" +
"    const SkPoint points[] = {{32, 16}, {48, 48}, {16, 32}};\n" +
"    const SkPaint::Join join[] = { SkPaint::kRound_Join, SkPaint::kMiter_Join, SkPaint::kBevel_Join };\n" +
"    int joinIndex = 0;\n" +
"    SkPath path;\n" +
"    path.addPoly(points, 3, false);\n" +
"    for (const auto cap : { SkPaint::kRound_Cap, SkPaint::kSquare_Cap, SkPaint::kButt_Cap } ) {\n" +
"        paint.setStrokeCap(cap);\n" +
"        paint.setStrokeJoin(join[joinIndex++]);\n" +
"        for (const auto mode : { SkCanvas::kPoints_PointMode, SkCanvas::kLines_PointMode, SkCanvas::kPolygon_PointMode } ) {\n" +
"            canvas->drawPoints(mode, 3, points, paint);\n" +
"            canvas->translate(64, 0);\n" +
"        }\n" +
"        canvas->drawPath(path, paint);\n" +
"        canvas->translate(-192, 64);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawPoints_json = {
    "code": SkCanvas_drawPoints_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawPoints",
    "overwrite": true
}

runFiddle(SkCanvas_drawPoints_json);

var SkCanvas_drawPoint_code = 
"void draw(SkCanvas* canvas) {\n" +
"  canvas->clear(SK_ColorWHITE);\n" +
"  SkPaint paint;\n" +
"  paint.setAntiAlias(true);\n" +
"  paint.setColor(0x80349a45);\n" +
"  paint.setStyle(SkPaint::kStroke_Style);\n" +
"  paint.setStrokeWidth(100);\n" +
"  paint.setStrokeCap(SkPaint::kRound_Cap);\n" +
"  canvas->scale(1, 1.2f);\n" +
"  canvas->drawPoint(64, 96, paint);\n" +
"  canvas->scale(.6f, .8f);\n" +
"  paint.setColor(SK_ColorWHITE);\n" +
"  canvas->drawPoint(106, 120, paint);\n" +
"}\n";

var SkCanvas_drawPoint_json = {
    "code": SkCanvas_drawPoint_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawPoint",
    "overwrite": true
}

runFiddle(SkCanvas_drawPoint_json);

var SkCanvas_drawLine_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setColor(0xFF9a67be);\n" +
"    paint.setStrokeWidth(20);\n" +
"    canvas->skew(1, 0);\n" +
"    canvas->drawLine(32, 96, 32, 160, paint);\n" +
"    canvas->skew(-2, 0);\n" +
"    canvas->drawLine(288, 96, 288, 160, paint);\n" +
"}\n";

var SkCanvas_drawLine_json = {
    "code": SkCanvas_drawLine_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawLine",
    "overwrite": true
}

runFiddle(SkCanvas_drawLine_json);

var SkCanvas_drawRect_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkPoint rectPts[] = { {64, 48}, {192, 160} };\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(20);\n" +
"    paint.setStrokeJoin(SkPaint::kRound_Join);\n" +
"    SkMatrix rotator;\n" +
"    rotator.setRotate(30, 128, 128);\n" +
"    for (auto color : { SK_ColorRED, SK_ColorBLUE, SK_ColorYELLOW, SK_ColorMAGENTA } ) {\n" +
"        paint.setColor(color);\n" +
"        SkRect rect;\n" +
"        rect.set(rectPts[0], rectPts[1]);\n" +
"        canvas->drawRect(rect, paint);\n" +
"        rotator.mapPoints(rectPts, 2);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawRect_json = {
    "code": SkCanvas_drawRect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawRect",
    "overwrite": true
}

runFiddle(SkCanvas_drawRect_json);

var SkCanvas_drawIRect_code = 
"void draw(SkCanvas* canvas) {\n" + 
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkIRect rect = { 64, 48, 192, 160 };\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(20);\n" +
"    paint.setStrokeJoin(SkPaint::kRound_Join);\n" +
"    for (auto color : { SK_ColorRED, SK_ColorBLUE, SK_ColorYELLOW, SK_ColorMAGENTA } ) {\n" +
"        paint.setColor(color);\n" +
"        canvas->drawIRect(rect, paint);\n" +
"        canvas->rotate(30, 128, 128);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawIRect_json = {
    "code": SkCanvas_drawIRect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawIRect",
    "overwrite": true
}

runFiddle(SkCanvas_drawIRect_json);

var SkCanvas_drawRegion_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    SkRegion region;\n" +
"    region.op( 10, 10, 50, 50, SkRegion::kUnion_Op);\n" +
"    region.op( 10, 50, 90, 90, SkRegion::kUnion_Op);\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(20);\n" +
"    paint.setStrokeJoin(SkPaint::kRound_Join);\n" +
"    canvas->drawRegion(region, paint);\n" +
"}\n";

var SkCanvas_drawRegion_json = {
    "code": SkCanvas_drawRegion_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawRegion",
    "overwrite": true
}

runFiddle(SkCanvas_drawRegion_json);

var SkCanvas_drawOval_code = 
"void draw(SkCanvas* canvas) {\n" +
"    canvas->clear(0xFF3f5f9f);\n" +
"    SkColor  kColor1 = SkColorSetARGB(0xff, 0xff, 0x7f, 0);\n" +
"    SkColor  g1Colors[] = { kColor1, SkColorSetA(kColor1, 0x20) };\n" +
"    SkPoint  g1Points[] = { { 0, 0 }, { 0, 100 } };\n" +
"    SkScalar pos[] = { 0.2f, 1.0f };\n" +
"    SkRect bounds = SkRect::MakeWH(80, 70);\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setShader(SkGradientShader::MakeLinear(g1Points, g1Colors, pos, SK_ARRAY_COUNT(g1Colors),\n" +
"            SkShader::kClamp_TileMode));\n" +
"    canvas->drawOval(bounds , paint);\n" +
"}\n";

var SkCanvas_drawOval_json = {
    "code": SkCanvas_drawOval_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawOval",
    "overwrite": true
}

runFiddle(SkCanvas_drawOval_json);

var SkCanvas_drawRRect_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    SkRect outer = {30, 40, 210, 220};\n" +
"    SkRect radii = {30, 50, 70, 90 };\n" +
"    SkRRect rRect;\n" +
"    rRect.setNinePatch(outer, radii.fLeft, radii.fTop, radii.fRight, radii.fBottom);\n" +
"    canvas->drawRRect(rRect, paint);\n" +
"    paint.setColor(SK_ColorWHITE);\n" +
"    canvas->drawLine(outer.fLeft + radii.fLeft, outer.fTop, outer.fLeft + radii.fLeft, outer.fBottom, paint);\n" +
"    canvas->drawLine(outer.fRight - radii.fRight, outer.fTop, outer.fRight - radii.fRight, outer.fBottom, paint);\n" +
"    canvas->drawLine(outer.fLeft, outer.fTop + radii.fTop, outer.fRight, outer.fTop + radii.fTop, paint);\n" +
"    canvas->drawLine(outer.fLeft, outer.fBottom - radii.fBottom, outer.fRight, outer.fBottom - radii.fBottom, paint);\n" +
"}\n";

var SkCanvas_drawRRect_json = {
    "code": SkCanvas_drawRRect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawRRect",
    "overwrite": true
}

runFiddle(SkCanvas_drawRRect_json);

var SkCanvas_drawDRRect_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkRRect outer = SkRRect::MakeRect({20, 40, 210, 200});\n" +
"   SkRRect inner = SkRRect::MakeOval({60, 70, 170, 160});\n" +
"   SkPaint paint;\n" +
"   canvas->drawDRRect(outer, inner, paint);\n" +
"}\n";

var SkCanvas_drawDRRect_json = {
    "code": SkCanvas_drawDRRect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawDRRect",
    "overwrite": true
}

runFiddle(SkCanvas_drawDRRect_json);

var SkCanvas_drawDRRect_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"   SkRRect outer = SkRRect::MakeRect({20, 40, 210, 200});\n" +
"   SkRRect inner = SkRRect::MakeRectXY({60, 70, 170, 160}, 10, 10);\n" +
"   SkPaint paint;\n" +
"   paint.setAntiAlias(true);\n" +
"   paint.setStyle(SkPaint::kStroke_Style);\n" +
"   paint.setStrokeWidth(20);\n" +
"   paint.setStrokeJoin(SkPaint::kRound_Join);\n" +
"   canvas->drawDRRect(outer, inner, paint);\n" +
"   paint.setStrokeWidth(1);\n" +
"   paint.setColor(SK_ColorWHITE);\n" +
"   canvas->drawDRRect(outer, inner, paint);\n" +
"}\n";

var SkCanvas_drawDRRect_2_json = {
    "code": SkCanvas_drawDRRect_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawDRRect_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawDRRect_2_json);

var SkCanvas_drawCircle_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    canvas->drawCircle(128, 128, 90, paint);\n" +
"    paint.setColor(SK_ColorWHITE);\n" +
"    canvas->drawCircle(86, 86, 20, paint);\n" +
"    canvas->drawCircle(160, 76, 20, paint);\n" +
"    canvas->drawCircle(140, 150, 35, paint);\n" +
"}\n";

var SkCanvas_drawCircle_json = {
    "code": SkCanvas_drawCircle_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawCircle",
    "overwrite": true
}

runFiddle(SkCanvas_drawCircle_json);

var SkCanvas_drawArc_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    SkRect oval = { 4, 4, 60, 60};\n" +
"    for (auto useCenter : { false, true } ) {\n" +
"        for (auto style : { SkPaint::kFill_Style, SkPaint::kStroke_Style } ) {\n" +
"            paint.setStyle(style);\n" +
"            for (auto degrees : { 45, 90, 180, 360} ) {\n" +
"                canvas->drawArc(oval, 0, degrees , useCenter, paint);\n" +
"                canvas->translate(64, 0);\n" +
"            }\n" +
"            canvas->translate(-256, 64);\n" +
"        }\n" +
"    }\n" +
"}\n";

var SkCanvas_drawArc_json = {
    "code": SkCanvas_drawArc_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawArc",
    "overwrite": true
}

runFiddle(SkCanvas_drawArc_json);

var SkCanvas_drawArc_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    paint.setStrokeWidth(4);\n" +
"    SkRect oval = { 4, 4, 60, 60};\n" +
"    float intervals[] = { 5, 5 };\n" +
"    paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 2.5f));\n" +
"    for (auto degrees : { 270, 360, 540, 720 } ) {\n" +
"        canvas->drawArc(oval, 0, degrees , false, paint);\n" +
"        canvas->translate(64, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawArc_2_json = {
    "code": SkCanvas_drawArc_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawArc_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawArc_2_json);

var SkCanvas_drawRoundRect_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkVector radii[] = { {0, 20}, {10, 10}, {10, 20}, {10, 40} };\n" +
"    SkPaint paint;\n" +
"    paint.setStrokeWidth(15);\n" +
"    paint.setStrokeJoin(SkPaint::kRound_Join);\n" +
"    paint.setAntiAlias(true);\n" +
"    for (auto style : { SkPaint::kStroke_Style, SkPaint::kFill_Style  } ) {\n" +
"        paint.setStyle(style );\n" +
"        for (size_t i = 0; i < SK_ARRAY_COUNT(radii); ++i) {\n" +
"           canvas->drawRoundRect({10, 10, 60, 40}, radii[i].fX, radii[i].fY, paint);\n" +
"           canvas->translate(0, 60);\n" +
"        }\n" +
"        canvas->translate(80, -240);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawRoundRect_json = {
    "code": SkCanvas_drawRoundRect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawRoundRect",
    "overwrite": true
}

runFiddle(SkCanvas_drawRoundRect_json);

var SkCanvas_drawPath_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPath path;\n" +
"    path.moveTo(20, 20);\n" +
"    path.quadTo(60, 20, 60, 60);\n" +
"    path.close();\n" +
"    path.moveTo(60, 20);\n" +
"    path.quadTo(60, 60, 20, 60);\n" +
"    SkPaint paint;\n" +
"    paint.setStrokeWidth(10);\n" +
"    paint.setAntiAlias(true);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    for (auto join: { SkPaint::kBevel_Join, SkPaint::kRound_Join, SkPaint::kMiter_Join } ) {\n" +
"        paint.setStrokeJoin(join);\n" +
"        for (auto cap: { SkPaint::kButt_Cap, SkPaint::kSquare_Cap, SkPaint::kRound_Cap  } ) {\n" +
"            paint.setStrokeCap(cap);\n" +
"            canvas->drawPath(path, paint);\n" +
"            canvas->translate(80, 0);\n" +
"        }\n" +
"        canvas->translate(-240, 60);\n" +
"    }\n" +
"    paint.setStyle(SkPaint::kFill_Style);\n" +
"    for (auto fill : { SkPath::kWinding_FillType, SkPath::kEvenOdd_FillType, SkPath::kInverseWinding_FillType } ) {\n" +
"        path.setFillType(fill);\n" +
"        canvas->save();\n" +
"        canvas->clipRect({0, 10, 80, 70});\n" +
"        canvas->drawPath(path, paint);\n" +
"        canvas->restore();\n" +
"        canvas->translate(80, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawPath_json = {
    "code": SkCanvas_drawPath_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawPath",
    "overwrite": true
}

runFiddle(SkCanvas_drawPath_json);

var SkCanvas_drawImage_code = 
"void draw(SkCanvas* canvas) {\n" +
"   // sk_sp<SkImage> image;\n" +
"   SkImage* imagePtr = image.get();\n" +
"   canvas->drawImage(imagePtr, 0, 0);\n" +
"   SkPaint paint;\n" +
"   canvas->drawImage(imagePtr, 80, 0, &paint);\n" +
"   paint.setAlpha(0x80);\n" +
"   canvas->drawImage(imagePtr, 160, 0, &paint);\n" +
"}\n";

var SkCanvas_drawImage_json = {
    "code": SkCanvas_drawImage_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 4,
        "textOnly": false
    },
    "name": "SkCanvas_drawImage",
    "overwrite": true
}

runFiddle(SkCanvas_drawImage_json);

var SkCanvas_drawImage_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"   // sk_sp<SkImage> image;\n" +
"   canvas->drawImage(image, 0, 0);\n" +
"   SkPaint paint;\n" +
"   canvas->drawImage(image, 80, 0, &paint);\n" +
"   paint.setAlpha(0x80);\n" +
"   canvas->drawImage(image, 160, 0, &paint);\n" +
"}\n";

var SkCanvas_drawImage_2_json = {
    "code": SkCanvas_drawImage_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 4,
        "textOnly": false
    },
    "name": "SkCanvas_drawImage_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawImage_2_json);

var SkCanvas_SrcRectConstraint_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkBitmap redBorder;\n" +
"    redBorder.allocPixels(SkImageInfo::MakeN32Premul(4, 4));\n" +
"    SkCanvas checkRed(redBorder);\n" +
"    checkRed.clear(SK_ColorRED);\n" +
"    uint32_t checkers[][2] = { { SK_ColorBLACK, SK_ColorWHITE },\n" +
"                               { SK_ColorWHITE, SK_ColorBLACK } };\n" +
"    checkRed.writePixels(\n" +
"            SkImageInfo::MakeN32Premul(2, 2), (void*) checkers, sizeof(checkers[0]), 1, 1);\n" +
"    canvas->scale(16, 16);\n" +
"    canvas->drawBitmap(redBorder, 0, 0, nullptr);\n" +
"    canvas->resetMatrix();\n" +
"    sk_sp<SkImage> image = SkImage::MakeFromBitmap(redBorder);\n" +
"    SkPaint lowPaint;\n" +
"    lowPaint.setFilterQuality(kLow_SkFilterQuality);\n" +
"    for (auto constraint : { SkCanvas::kStrict_SrcRectConstraint,\n" +
"                             SkCanvas::kFast_SrcRectConstraint } ) {\n" +
"        canvas->translate(80, 0);\n" +
"        canvas->drawImageRect(image.get(), SkRect::MakeLTRB(1, 1, 3, 3),\n" +
"                SkRect::MakeLTRB(16, 16, 48, 48), &lowPaint, constraint);\n" +
"    }\n" +
"}\n";

var SkCanvas_SrcRectConstraint_json = {
    "code": SkCanvas_SrcRectConstraint_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_SrcRectConstraint",
    "overwrite": true
}

runFiddle(SkCanvas_SrcRectConstraint_json);

var SkCanvas_drawImageRect_code = 
"void draw(SkCanvas* canvas) {\n" +
"    uint32_t pixels[][4] = { \n" +
"            { 0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000 },\n" +
"            { 0xFFFF0000, 0xFF000000, 0xFFFFFFFF, 0xFFFF0000 },\n" +
"            { 0xFFFF0000, 0xFFFFFFFF, 0xFF000000, 0xFFFF0000 },\n" +
"            { 0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000 } };\n" +
"    SkBitmap redBorder;\n" +
"    redBorder.installPixels(SkImageInfo::MakeN32Premul(4, 4), \n" +
"            (void*) pixels, sizeof(pixels[0]));\n" +
"    sk_sp<SkImage> image = SkImage::MakeFromBitmap(redBorder);\n" +
"    SkPaint lowPaint;\n" +
"    for (auto constraint : {\n" +
"            SkCanvas::kFast_SrcRectConstraint,\n" +
"            SkCanvas::kStrict_SrcRectConstraint,\n" +
"            SkCanvas::kFast_SrcRectConstraint } ) {\n" +
"        canvas->drawImageRect(image.get(), SkRect::MakeLTRB(1, 1, 3, 3),\n" +
"                SkRect::MakeLTRB(16, 16, 48, 48), &lowPaint, constraint);\n" +
"        lowPaint.setFilterQuality(kLow_SkFilterQuality);\n" +
"        canvas->translate(80, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawImageRect_json = {
    "code": SkCanvas_drawImageRect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawImageRect",
    "overwrite": true
}

runFiddle(SkCanvas_drawImageRect_json);

var SkCanvas_drawImageRect_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    // sk_sp<SkImage> image;\n" +
"    for (auto i : { 1, 2, 4, 8 } ) {\n" +
"        canvas->drawImageRect(image.get(), SkIRect::MakeLTRB(0, 0, 100, 100), \n" +
"            SkRect::MakeXYWH(i * 20, i * 20, i * 20, i * 20), nullptr);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawImageRect_2_json = {
    "code": SkCanvas_drawImageRect_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 4,
        "textOnly": false
    },
    "name": "SkCanvas_drawImageRect_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawImageRect_2_json);

var SkCanvas_drawImageRect_3_code = 
"void draw(SkCanvas* canvas) {\n" + 
"}\n";

var SkCanvas_drawImageRect_3_json = {
    "code": SkCanvas_drawImageRect_3_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawImageRect_3",
    "overwrite": true
}

runFiddle(SkCanvas_drawImageRect_3_json);

var SkCanvas_drawImageRect_4_code = 
"void draw(SkCanvas* canvas) {\n" +
"    uint32_t pixels[][2] = { { SK_ColorBLACK, SK_ColorWHITE },\n" +
"                             { SK_ColorWHITE, SK_ColorBLACK } };\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.installPixels(SkImageInfo::MakeN32Premul(2, 2), \n" +
"            (void*) pixels, sizeof(pixels[0]));\n" +
"    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);\n" +
"    SkPaint paint;\n" +
"    canvas->scale(4, 4);\n" +
"    for (auto alpha : { 50, 100, 150, 255 } ) {\n" +
"        paint.setAlpha(alpha);\n" +
"        canvas->drawImageRect(image, SkRect::MakeWH(2, 2), SkRect::MakeWH(8, 8), &paint);\n" +
"        canvas->translate(8, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawImageRect_4_json = {
    "code": SkCanvas_drawImageRect_4_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawImageRect_4",
    "overwrite": true
}

runFiddle(SkCanvas_drawImageRect_4_json);

var SkCanvas_drawImageRect_5_code = 
"void draw(SkCanvas* canvas) {\n" +
"    uint32_t pixels[][2] = { { 0x00000000, 0x55555555},\n" +
"                             { 0xAAAAAAAA, 0xFFFFFFFF} };\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.installPixels(SkImageInfo::MakeN32Premul(2, 2), \n" +
"            (void*) pixels, sizeof(pixels[0]));\n" +
"    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);\n" +
"    SkPaint paint;\n" +
"    canvas->scale(4, 4);\n" +
"    for (auto color : { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN } ) {\n" +
"        paint.setColorFilter(SkColorFilter::MakeModeFilter(color, SkBlendMode::kPlus));\n" +
"        canvas->drawImageRect(image, SkIRect::MakeWH(2, 2), SkRect::MakeWH(8, 8), &paint);\n" +
"        canvas->translate(8, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawImageRect_5_json = {
    "code": SkCanvas_drawImageRect_5_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawImageRect_5",
    "overwrite": true
}

runFiddle(SkCanvas_drawImageRect_5_json);

var SkCanvas_drawImageRect_6_code = 
"void draw(SkCanvas* canvas) {\n" +
"    uint32_t pixels[][2] = { { 0x00000000, 0x55550000},\n" +
"                             { 0xAAAA0000, 0xFFFF0000} };\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.installPixels(SkImageInfo::MakeN32Premul(2, 2), \n" +
"            (void*) pixels, sizeof(pixels[0]));\n" +
"    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);\n" +
"    SkPaint paint;\n" +
"    canvas->scale(4, 4);\n" +
"    for (auto color : { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN } ) {\n" +
"        paint.setColorFilter(SkColorFilter::MakeModeFilter(color, SkBlendMode::kPlus));\n" +
"        canvas->drawImageRect(image, SkRect::MakeWH(8, 8), &paint);\n" +
"        canvas->translate(8, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawImageRect_6_json = {
    "code": SkCanvas_drawImageRect_6_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawImageRect_6",
    "overwrite": true
}

runFiddle(SkCanvas_drawImageRect_6_json);

var SkCanvas_drawImageNine_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkIRect center = { 20, 10, 50, 40 };\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.allocPixels(SkImageInfo::MakeN32Premul(60, 60));\n" +
"    SkCanvas bitCanvas(bitmap);\n" +
"    SkPaint paint;\n" +
"    SkColor gray = 0xFF000000;\n" +
"    int left = 0;\n" +
"    for (auto right: { center.fLeft, center.fRight, bitmap.width() } ) {\n" +
"        int top = 0;\n" +
"        for (auto bottom: { center.fTop, center.fBottom, bitmap.height() } ) {\n" +
"            paint.setColor(gray);\n" +
"            bitCanvas.drawIRect(SkIRect::MakeLTRB(left, top, right, bottom), paint);\n" +
"            gray += 0x001f1f1f;\n" +
"            top = bottom;\n" +
"        }\n" +
"        left = right; \n" +
"    }\n" +
"    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);\n" +
"    SkImage* imagePtr = image.get();\n" +
"    for (auto dest: { 20, 30, 40, 60, 90 } ) {\n" +
"        canvas->drawImageNine(imagePtr, center, SkRect::MakeWH(dest, dest), nullptr);\n" +
"        canvas->translate(dest + 4, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawImageNine_json = {
    "code": SkCanvas_drawImageNine_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawImageNine",
    "overwrite": true
}

runFiddle(SkCanvas_drawImageNine_json);

var SkCanvas_drawImageNine_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkIRect center = { 20, 10, 50, 40 };\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.allocPixels(SkImageInfo::MakeN32Premul(60, 60));\n" +
"    SkCanvas bitCanvas(bitmap);\n" +
"    SkPaint paint;\n" +
"    SkColor gray = 0xFF000000;\n" +
"    int left = 0;\n" +
"    for (auto right: { center.fLeft, center.fRight, bitmap.width() } ) {\n" +
"        int top = 0;\n" +
"        for (auto bottom: { center.fTop, center.fBottom, bitmap.height() } ) {\n" +
"            paint.setColor(gray);\n" +
"            bitCanvas.drawIRect(SkIRect::MakeLTRB(left, top, right, bottom), paint);\n" +
"            gray += 0x001f1f1f;\n" +
"            top = bottom;\n" +
"        }\n" +
"        left = right; \n" +
"    }\n" +
"    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);\n" +
"    for (auto dest: { 20, 30, 40, 60, 90 } ) {\n" +
"        canvas->drawImageNine(image, center, SkRect::MakeWH(dest, 110 - dest), nullptr);\n" +
"        canvas->translate(dest + 4, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawImageNine_2_json = {
    "code": SkCanvas_drawImageNine_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawImageNine_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawImageNine_2_json);

var SkCanvas_drawBitmap_code = 
"void draw(SkCanvas* canvas) {\n" +
"    uint8_t pixels[][8] = { { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00},\n" +
"                            { 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00},\n" +
"                            { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00},\n" +
"                            { 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF},\n" +
"                            { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},\n" +
"                            { 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00},\n" +
"                            { 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00},\n" +
"                            { 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF} };\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.installPixels(SkImageInfo::MakeA8(8, 8), \n" +
"            (void*) pixels, sizeof(pixels[0]));\n" +
"    SkPaint paint;\n" +
"    canvas->scale(4, 4);\n" +
"    for (auto color : { SK_ColorRED, SK_ColorBLUE, 0xFF007F00} ) {\n" +
"        paint.setColor(color);\n" +
"        canvas->drawBitmap(bitmap, 0, 0, &paint);\n" +
"        canvas->translate(12, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawBitmap_json = {
    "code": SkCanvas_drawBitmap_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawBitmap",
    "overwrite": true
}

runFiddle(SkCanvas_drawBitmap_json);

var SkCanvas_drawBitmapRect_code = 
"void draw(SkCanvas* canvas) {\n" +
"    uint8_t pixels[][8] = { { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00},\n" +
"                            { 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00},\n" +
"                            { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00},\n" +
"                            { 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF},\n" +
"                            { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},\n" +
"                            { 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00},\n" +
"                            { 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00},\n" +
"                            { 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00} };\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.installPixels(SkImageInfo::MakeA8(8, 8), \n" +
"            (void*) pixels, sizeof(pixels[0]));\n" +
"    SkPaint paint;\n" +
"    paint.setMaskFilter(SkBlurMaskFilter::Make(kSolid_SkBlurStyle, 6));\n" +
"    for (auto color : { SK_ColorRED, SK_ColorBLUE, 0xFF007F00} ) {\n" +
"        paint.setColor(color);\n" +
"        canvas->drawBitmapRect(bitmap, SkRect::MakeWH(8, 8), SkRect::MakeWH(32, 32), &paint);\n" +
"        canvas->translate(48, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawBitmapRect_json = {
    "code": SkCanvas_drawBitmapRect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawBitmapRect",
    "overwrite": true
}

runFiddle(SkCanvas_drawBitmapRect_json);

var SkCanvas_drawBitmapRect_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    uint8_t pixels[][8] = { { 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00},\n" +
"                            { 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00},\n" +
"                            { 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF},\n" +
"                            { 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF},\n" +
"                            { 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF},\n" +
"                            { 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF},\n" +
"                            { 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00},\n" +
"                            { 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00} };\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.installPixels(SkImageInfo::MakeA8(8, 8), \n" +
"            (void*) pixels, sizeof(pixels[0]));\n" +
"    SkPaint paint;\n" +
"    paint.setFilterQuality(kHigh_SkFilterQuality);\n" +
"    for (auto color : { SK_ColorRED, SK_ColorBLUE, 0xFF007F00, 0xFF7f007f} ) {\n" +
"        paint.setColor(color);\n" +
"        canvas->drawBitmapRect(bitmap, SkIRect::MakeWH(8, 8), SkRect::MakeWH(32, 32), &paint);\n" +
"        canvas->translate(48.25f, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawBitmapRect_2_json = {
    "code": SkCanvas_drawBitmapRect_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawBitmapRect_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawBitmapRect_2_json);

var SkCanvas_drawBitmapRect_3_code = 
"void draw(SkCanvas* canvas) {\n" +
"    uint32_t pixels[][2] = { { 0x00000000, 0x55550000},\n" +
"                             { 0xAAAA0000, 0xFFFF0000} };\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.installPixels(SkImageInfo::MakeN32Premul(2, 2), \n" +
"            (void*) pixels, sizeof(pixels[0]));\n" +
"    SkPaint paint;\n" +
"    canvas->scale(4, 4);\n" +
"    for (auto color : { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN } ) {\n" +
"        paint.setColorFilter(SkColorFilter::MakeModeFilter(color, SkBlendMode::kPlus));\n" +
"        canvas->drawBitmapRect(bitmap, SkRect::MakeWH(8, 8), &paint);\n" +
"        canvas->translate(8, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawBitmapRect_3_json = {
    "code": SkCanvas_drawBitmapRect_3_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawBitmapRect_3",
    "overwrite": true
}

runFiddle(SkCanvas_drawBitmapRect_3_json);

var SkCanvas_drawBitmapNine_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkIRect center = { 20, 10, 50, 40 };\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.allocPixels(SkImageInfo::MakeN32Premul(60, 60));\n" +
"    SkCanvas bitCanvas(bitmap);\n" +
"    SkPaint paint;\n" +
"    SkColor gray = 0xFF000000;\n" +
"    int left = 0;\n" +
"    for (auto right: { center.fLeft, center.fRight, bitmap.width() } ) {\n" +
"        int top = 0;\n" +
"        for (auto bottom: { center.fTop, center.fBottom, bitmap.height() } ) {\n" +
"            paint.setColor(gray);\n" +
"            bitCanvas.drawIRect(SkIRect::MakeLTRB(left, top, right, bottom), paint);\n" +
"            gray += 0x001f1f1f;\n" +
"            top = bottom;\n" +
"        }\n" +
"        left = right; \n" +
"    }\n" +
"    for (auto dest: { 20, 30, 40, 60, 90 } ) {\n" +
"        canvas->drawBitmapNine(bitmap, center, SkRect::MakeWH(dest, 110 - dest), nullptr);\n" +
"        canvas->translate(dest + 4, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawBitmapNine_json = {
    "code": SkCanvas_drawBitmapNine_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawBitmapNine",
    "overwrite": true
}

runFiddle(SkCanvas_drawBitmapNine_json);

var SkCanvas_drawBitmapLattice_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkIRect center = { 20, 10, 50, 40 };\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.allocPixels(SkImageInfo::MakeN32Premul(60, 60));\n" +
"    SkCanvas bitCanvas(bitmap);\n" +
"    SkPaint paint;\n" +
"    SkColor gray = 0xFF000000;\n" +
"    int left = 0;\n" +
"    for (auto right: { center.fLeft, center.fRight, bitmap.width() } ) {\n" +
"        int top = 0;\n" +
"        for (auto bottom: { center.fTop, center.fBottom, bitmap.height() } ) {\n" +
"            paint.setColor(gray);\n" +
"            bitCanvas.drawIRect(SkIRect::MakeLTRB(left, top, right, bottom), paint);\n" +
"            gray += 0x001f1f1f;\n" +
"            top = bottom;\n" +
"        }\n" +
"        left = right; \n" +
"    }\n" +
"    const int xDivs[] = { center.fLeft, center.fRight };\n" +
"    const int yDivs[] = { center.fTop, center.fBottom };\n" +
"    SkCanvas::Lattice::Flags flags[3][3];\n" +
"    memset(flags, 0, sizeof(flags));  \n" +
"    flags[1][1] = SkCanvas::Lattice::kTransparent_Flags;\n" +
"    SkCanvas::Lattice lattice = { xDivs, yDivs, flags[0], SK_ARRAY_COUNT(xDivs),\n" +
"         SK_ARRAY_COUNT(yDivs), nullptr };\n" +
"    for (auto dest: { 20, 30, 40, 60, 90 } ) {\n" +
"        canvas->drawBitmapLattice(bitmap, lattice , SkRect::MakeWH(dest, 110 - dest), nullptr);\n" +
"        canvas->translate(dest + 4, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawBitmapLattice_json = {
    "code": SkCanvas_drawBitmapLattice_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawBitmapLattice",
    "overwrite": true
}

runFiddle(SkCanvas_drawBitmapLattice_json);

var SkCanvas_drawImageLattice_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkIRect center = { 20, 10, 50, 40 };\n" +
"    SkBitmap bitmap;\n" +
"    bitmap.allocPixels(SkImageInfo::MakeN32Premul(60, 60));\n" +
"    SkCanvas bitCanvas(bitmap);\n" +
"    SkPaint paint;\n" +
"    SkColor gray = 0xFF000000;\n" +
"    int left = 0;\n" +
"    for (auto right: { center.fLeft, center.fRight, bitmap.width() } ) {\n" +
"        int top = 0;\n" +
"        for (auto bottom: { center.fTop, center.fBottom, bitmap.height() } ) {\n" +
"            paint.setColor(gray);\n" +
"            bitCanvas.drawIRect(SkIRect::MakeLTRB(left, top, right, bottom), paint);\n" +
"            gray += 0x001f1f1f;\n" +
"            top = bottom;\n" +
"        }\n" +
"        left = right; \n" +
"    }\n" +
"    const int xDivs[] = { center.fLeft, center.fRight };\n" +
"    const int yDivs[] = { center.fTop, center.fBottom };\n" +
"    SkCanvas::Lattice::Flags flags[3][3];\n" +
"    memset(flags, 0, sizeof(flags));  \n" +
"    flags[1][1] = SkCanvas::Lattice::kTransparent_Flags;\n" +
"    SkCanvas::Lattice lattice = { xDivs, yDivs, flags[0], SK_ARRAY_COUNT(xDivs),\n" +
"         SK_ARRAY_COUNT(yDivs), nullptr };\n" +
"    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);\n" +
"    SkImage* imagePtr = image.get();\n" +
"    for (auto dest: { 20, 30, 40, 60, 90 } ) {\n" +
"        canvas->drawImageNine(imagePtr, center, SkRect::MakeWH(dest, dest), nullptr);\n" +
"        canvas->translate(dest + 4, 0);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawImageLattice_json = {
    "code": SkCanvas_drawImageLattice_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawImageLattice",
    "overwrite": true
}

runFiddle(SkCanvas_drawImageLattice_json);

var SkCanvas_drawText_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    float textSizes[] = { 12, 18, 24, 36 };\n" +
"    for (auto size: textSizes ) {\n" +
"        paint.setTextSize(size);\n" +
"        canvas->drawText(\"Aa\", 2, 10, 20, paint);\n" +
"        canvas->translate(0, size * 2);\n" +
"    }\n" +
"    paint.reset();\n" +
"    paint.setAntiAlias(true);\n" +
"    float yPos = 20;\n" +
"    for (auto size: textSizes ) {\n" +
"        float scale = size / 12.f;\n" +
"        canvas->resetMatrix();\n" +
"        canvas->translate(100, 0);\n" +
"        canvas->scale(scale, scale);\n" +
"        canvas->drawText(\"Aa\", 2, 10 / scale, yPos / scale, paint);\n" +
"        yPos += size * 2; \n" +
"    }\n" +
"}\n";

var SkCanvas_drawText_json = {
    "code": SkCanvas_drawText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawText",
    "overwrite": true
}

runFiddle(SkCanvas_drawText_json);

var SkCanvas_drawPosText_code = 
"void draw(SkCanvas* canvas) {\n" +
"  const char hello[] = \"HeLLo!\";\n" +
"  const SkPoint pos[] = { {40, 100}, {82, 95}, {115, 110}, {130, 95}, {145, 85},\n" +
"    {172, 100} };\n" +
"  SkPaint paint;\n" +
"  paint.setTextSize(60);\n" +
"  canvas->drawPosText(hello, strlen(hello), pos, paint);\n" +
"}\n";

var SkCanvas_drawPosText_json = {
    "code": SkCanvas_drawPosText_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawPosText",
    "overwrite": true
}

runFiddle(SkCanvas_drawPosText_json);

var SkCanvas_drawPosTextH_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkScalar xpos[] = { 20, 40, 80, 160 };\n" +
"    SkPaint paint;\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    canvas->drawPosTextH(\"XXXX\", 4, xpos, 20, paint);\n" +
"}\n";

var SkCanvas_drawPosTextH_json = {
    "code": SkCanvas_drawPosTextH_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawPosTextH",
    "overwrite": true
}

runFiddle(SkCanvas_drawPosTextH_json);

var SkCanvas_drawTextOnPathHV_code = 
"void draw(SkCanvas* canvas) { \n" +
"    const char aero[] = \"correo a\" \"\\xC3\" \"\\xA9\" \"reo\";\n" +
"    const size_t len = sizeof(aero) - 1;\n" +
"    SkPath path;\n" +
"    path.addOval({43-26, 43-26, 43+26, 43+26}, SkPath::kCW_Direction, 3);\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(24);\n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    for (auto offset : { 0, 10, 20 } ) {\n" +
"        canvas->drawTextOnPathHV(aero, len, path, 0, -offset, paint);\n" +
"        canvas->translate(70 + offset, 70 + offset);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawTextOnPathHV_json = {
    "code": SkCanvas_drawTextOnPathHV_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawTextOnPathHV",
    "overwrite": true
}

runFiddle(SkCanvas_drawTextOnPathHV_json);

var SkCanvas_drawTextOnPath_code = 
"void draw(SkCanvas* canvas) { \n" +
"    canvas->clear(SK_ColorWHITE);\n" +
"    const char roller[] = \"rollercoaster\";\n" +
"    const size_t len = sizeof(roller) - 1;\n" +
"    SkPath path;\n" +
"    path.cubicTo(40, -80, 120, 80, 160, -40);\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(32);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    SkMatrix matrix;\n" +
"    matrix.setIdentity();\n" +
"    for (int i = 0; i < 3; ++i) {\n" +
"        canvas->translate(25, 60);\n" +
"        canvas->drawPath(path, paint);\n" +
"        canvas->drawTextOnPath(roller, len, path, &matrix, paint);\n" +
"        matrix.preTranslate(0, 10);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawTextOnPath_json = {
    "code": SkCanvas_drawTextOnPath_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawTextOnPath",
    "overwrite": true
}

runFiddle(SkCanvas_drawTextOnPath_json);

var SkCanvas_drawTextRSXform_code = 
"void draw(SkCanvas* canvas) {  \n" +
"    const int iterations = 26;\n" +
"    SkRSXform transforms[iterations];\n" +
"    char alphabet[iterations];\n" +
"    SkScalar angle = 0;\n" +
"    SkScalar scale = 1;\n" +
"    for (size_t i = 0; i < SK_ARRAY_COUNT(transforms); ++i) {\n" +
"        const SkScalar s = SkScalarSin(angle) * scale;\n" +
"        const SkScalar c = SkScalarCos(angle) * scale;\n" +
"        transforms[i] = SkRSXform::Make(-c, -s, -s * 16, c * 16);\n" +
"        angle += .45;\n" +
"        scale += .2;\n" +
"        alphabet[i] = 'A' + i;\n" +
"    }\n" +
"    SkPaint paint;\n" +
"    paint.setTextAlign(SkPaint::kCenter_Align);\n" +
"    canvas->translate(110, 138);\n" +
"    canvas->drawTextRSXform(alphabet, sizeof(alphabet), transforms, nullptr, paint);\n" +
"}\n";

var SkCanvas_drawTextRSXform_json = {
    "code": SkCanvas_drawTextRSXform_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawTextRSXform",
    "overwrite": true
}

runFiddle(SkCanvas_drawTextRSXform_json);

var SkCanvas_drawTextBlob_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkTextBlobBuilder textBlobBuilder;\n" +
"    const char bunny[] = \"/(^x^)\\\\\";\n" +
"    const int len = sizeof(bunny) - 1;\n" +
"    uint16_t glyphs[len];\n" +
"    SkPaint paint;\n" +
"    paint.textToGlyphs(bunny, len, glyphs);\n" +
"    int runs[] = { 3, 1, 3 };\n" +
"    SkPoint textPos = { 20, 100 };\n" +
"    int glyphIndex = 0;\n" +
"    for (auto runLen : runs) {\n" +
"        paint.setTextSize(1 == runLen ? 20 : 50);\n" +
"        const SkTextBlobBuilder::RunBuffer& run = \n" +
"                textBlobBuilder.allocRun(paint, runLen, textPos.fX, textPos.fY);\n" +
"        memcpy(run.glyphs, &glyphs[glyphIndex], sizeof(glyphs[0]) * runLen);\n" +
"        textPos.fX += paint.measureText(&bunny[glyphIndex], runLen, nullptr);\n" +
"        glyphIndex += runLen;\n" +
"    }\n" +
"    sk_sp<const SkTextBlob> blob = textBlobBuilder.make();\n" +
"    paint.reset();\n" +
"    canvas->drawTextBlob(blob.get(), 0, 0, paint);\n" +
"}\n";

var SkCanvas_drawTextBlob_json = {
    "code": SkCanvas_drawTextBlob_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawTextBlob",
    "overwrite": true
}

runFiddle(SkCanvas_drawTextBlob_json);

var SkCanvas_drawTextBlob_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkTextBlobBuilder textBlobBuilder;\n" +
"    SkPaint paint;\n" +
"    paint.setTextSize(50);\n" +
"    paint.setColor(SK_ColorRED);\n" +
"    const SkTextBlobBuilder::RunBuffer& run = \n" +
"            textBlobBuilder.allocRun(paint, 1, 20, 100);\n" +
"    run.glyphs[0] = 20;\n" +
"    sk_sp<const SkTextBlob> blob = textBlobBuilder.make();\n" +
"    paint.setTextSize(10);\n" +
"    paint.setColor(SK_ColorBLUE);\n" +
"    canvas->drawTextBlob(blob.get(), 0, 0, paint);\n" +
"}\n";

var SkCanvas_drawTextBlob_2_json = {
    "code": SkCanvas_drawTextBlob_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawTextBlob_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawTextBlob_2_json);

var SkCanvas_drawPicture_code = 
"void draw(SkCanvas* canvas) {  \n" +
"    SkPictureRecorder recorder;\n" +
"    SkCanvas* recordingCanvas = recorder.beginRecording(50, 50);\n" +
"    for (auto color : { SK_ColorRED, SK_ColorBLUE, 0xff007f00 } ) {\n" +
"        SkPaint paint;\n" +
"        paint.setColor(color);\n" +
"        recordingCanvas->drawRect({10, 10, 30, 40}, paint);\n" +
"        recordingCanvas->translate(10, 10);\n" +
"        recordingCanvas->scale(1.2f, 1.4f);\n" +
"    }\n" +
"    sk_sp<SkPicture> playback = recorder.finishRecordingAsPicture();\n" +
"    const SkPicture* playbackPtr = playback.get();\n" +
"    canvas->drawPicture(playback);\n" +
"    canvas->scale(2, 2);\n" +
"    canvas->translate(50, 0);\n" +
"    canvas->drawPicture(playback);\n" +
"}\n";

var SkCanvas_drawPicture_json = {
    "code": SkCanvas_drawPicture_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawPicture",
    "overwrite": true
}

runFiddle(SkCanvas_drawPicture_json);

var SkCanvas_drawPicture_2_code = 
"void draw(SkCanvas* canvas) {  \n" +
"    SkPictureRecorder recorder;\n" +
"    SkCanvas* recordingCanvas = recorder.beginRecording(50, 50);\n" +
"    for (auto color : { SK_ColorRED, SK_ColorBLUE, 0xff007f00 } ) {\n" +
"        SkPaint paint;\n" +
"        paint.setColor(color);\n" +
"        recordingCanvas->drawRect({10, 10, 30, 40}, paint);\n" +
"        recordingCanvas->translate(10, 10);\n" +
"        recordingCanvas->scale(1.2f, 1.4f);\n" +
"    }\n" +
"    sk_sp<SkPicture> playback = recorder.finishRecordingAsPicture();\n" +
"    canvas->drawPicture(playback);\n" +
"    canvas->scale(2, 2);\n" +
"    canvas->translate(50, 0);\n" +
"    canvas->drawPicture(playback);\n" +
"}\n";

var SkCanvas_drawPicture_2_json = {
    "code": SkCanvas_drawPicture_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawPicture_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawPicture_2_json);

var SkCanvas_drawPicture_3_code = 
"void draw(SkCanvas* canvas) {  \n" +
"    SkPaint paint;\n" +
"    SkPictureRecorder recorder;\n" +
"    SkCanvas* recordingCanvas = recorder.beginRecording(50, 50);\n" +
"    for (auto color : { SK_ColorRED, SK_ColorBLUE, 0xff007f00 } ) {\n" +
"        paint.setColor(color);\n" +
"        recordingCanvas->drawRect({10, 10, 30, 40}, paint);\n" +
"        recordingCanvas->translate(10, 10);\n" +
"        recordingCanvas->scale(1.2f, 1.4f);\n" +
"    }\n" +
"    sk_sp<SkPicture> playback = recorder.finishRecordingAsPicture();\n" +
"    const SkPicture* playbackPtr = playback.get();\n" +
"    SkMatrix matrix;\n" +
"    matrix.reset();\n" +
"    for (auto alpha : { 70, 140, 210 } ) {\n" +
"    paint.setAlpha(alpha);\n" +
"    canvas->drawPicture(playbackPtr, &matrix, &paint);\n" +
"    matrix.preTranslate(70, 70);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawPicture_3_json = {
    "code": SkCanvas_drawPicture_3_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawPicture_3",
    "overwrite": true
}

runFiddle(SkCanvas_drawPicture_3_json);

var SkCanvas_drawPicture_4_code = 
"void draw(SkCanvas* canvas) {  \n" +
"    SkPaint paint;\n" +
"    SkPictureRecorder recorder;\n" +
"    SkCanvas* recordingCanvas = recorder.beginRecording(50, 50);\n" +
"    for (auto color : { SK_ColorRED, SK_ColorBLUE, 0xff007f00 } ) {\n" +
"        paint.setColor(color);\n" +
"        recordingCanvas->drawRect({10, 10, 30, 40}, paint);\n" +
"        recordingCanvas->translate(10, 10);\n" +
"        recordingCanvas->scale(1.2f, 1.4f);\n" +
"    }\n" +
"    sk_sp<SkPicture> playback = recorder.finishRecordingAsPicture();\n" +
"    SkMatrix matrix;\n" +
"    matrix.reset();\n" +
"    for (auto alpha : { 70, 140, 210 } ) {\n" +
"    paint.setAlpha(alpha);\n" +
"    canvas->drawPicture(playback, &matrix, &paint);\n" +
"    matrix.preTranslate(70, 70);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawPicture_4_json = {
    "code": SkCanvas_drawPicture_4_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawPicture_4",
    "overwrite": true
}

runFiddle(SkCanvas_drawPicture_4_json);

var SkCanvas_drawVertices_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    SkPoint points[] = { { 0, 0 }, { 250, 0 }, { 100, 100 }, { 0, 250 } };\n" +
"    SkColor colors[] = { SK_ColorRED, SK_ColorBLUE, SK_ColorYELLOW, SK_ColorCYAN };\n" +
"    auto vertices = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode,\n" +
"            SK_ARRAY_COUNT(points), points, nullptr, colors);\n" +
"    canvas->drawVertices(vertices.get(), SkBlendMode::kSrc, paint);\n" +
"}\n";

var SkCanvas_drawVertices_json = {
    "code": SkCanvas_drawVertices_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawVertices",
    "overwrite": true
}

runFiddle(SkCanvas_drawVertices_json);

var SkCanvas_drawVertices_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    SkPoint points[] = { { 0, 0 }, { 250, 0 }, { 100, 100 }, { 0, 250 } };\n" +
"    SkPoint texs[] = { { 0, 0 }, { 0, 250 }, { 250, 250 }, { 250, 0 } };\n" +
"    SkColor colors[] = { SK_ColorRED, SK_ColorBLUE, SK_ColorYELLOW, SK_ColorCYAN };\n" +
"    paint.setShader(SkGradientShader::MakeLinear(points, colors, nullptr, 4,\n" +
"            SkShader::kClamp_TileMode));\n" +
"    auto vertices = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode,\n" +
"            SK_ARRAY_COUNT(points), points, texs, colors);\n" +
"    canvas->drawVertices(vertices.get(), SkBlendMode::kDarken, paint);\n" +
"}\n";

var SkCanvas_drawVertices_2_json = {
    "code": SkCanvas_drawVertices_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawVertices_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawVertices_2_json);

var SkCanvas_drawPatch_code = 
"void draw(SkCanvas* canvas) {\n" +
"    // SkBitmap source = cmbkygk;\n" +
"    SkPaint paint;\n" +
"    paint.setFilterQuality(kLow_SkFilterQuality);\n" +
"    paint.setAntiAlias(true);\n" +
"    SkPoint cubics[] = { { 3, 1 },    { 4, 2 }, { 5, 1 },    { 7, 3 },\n" +
"                      /* { 7, 3 }, */ { 6, 4 }, { 7, 5 },    { 5, 7 },\n" +
"                      /* { 5, 7 }, */ { 4, 6 }, { 3, 7 },    { 1, 5 },\n" +
"                      /* { 1, 5 }, */ { 2, 4 }, { 1, 3 }, /* { 3, 1 } */ };\n" +
"    SkColor colors[] = { 0xbfff0000, 0xbf0000ff, 0xbfff00ff, 0xbf00ffff };\n" +
"    SkPoint texCoords[] = { { -30, -30 }, { 162, -30}, { 162, 162}, { -30, 162} };\n" +
"    paint.setShader(SkShader::MakeBitmapShader(source, SkShader::kClamp_TileMode,\n" +
"                                                       SkShader::kClamp_TileMode, nullptr));\n" +
"    canvas->scale(15, 15);\n" +
"    for (auto blend : { SkBlendMode::kSrcOver, SkBlendMode::kModulate, SkBlendMode::kXor } ) {\n" +
"        canvas->drawPatch(cubics, colors, texCoords, blend, paint);\n" +
"        canvas->translate(4, 4);\n" +
"    }\n" +
"}\n";

var SkCanvas_drawPatch_json = {
    "code": SkCanvas_drawPatch_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 5,
        "textOnly": false
    },
    "name": "SkCanvas_drawPatch",
    "overwrite": true
}

runFiddle(SkCanvas_drawPatch_json);

var SkCanvas_drawPatch_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkPaint paint;\n" +
"    paint.setAntiAlias(true);\n" +
"    SkPoint cubics[] = { { 3, 1 },    { 4, 2 }, { 5, 1 },    { 7, 3 },\n" +
"                      /* { 7, 3 }, */ { 6, 4 }, { 7, 5 },    { 5, 7 },\n" +
"                      /* { 5, 7 }, */ { 4, 6 }, { 3, 7 },    { 1, 5 },\n" +
"                      /* { 1, 5 }, */ { 2, 4 }, { 1, 3 }, /* { 3, 1 } */ };\n" +
"    SkColor colors[] = { SK_ColorRED, SK_ColorBLUE, SK_ColorYELLOW, SK_ColorCYAN };\n" +
"    canvas->scale(30, 30);\n" +
"    canvas->drawPatch(cubics, colors, nullptr, paint);\n" +
"    SkPoint text[] = { {3,0.9f}, {4,2.5f}, {5,0.9f}, {7.5f,3.2f}, {5.5f,4.2f},\n" +
"            {7.5f,5.2f}, {5,7.5f}, {4,5.9f}, {3,7.5f}, {0.5f,5.2f}, {2.5f,4.2f},\n" +
"            {0.5f,3.2f} };\n" +
"    paint.setTextSize(18.f / 30);\n" +
"    paint.setTextAlign(SkPaint::kCenter_Align);\n" +
"    for (int i = 0; i< 10; ++i) {\n" +
"       char digit = '0' + i;\n" +
"       canvas->drawText(&digit, 1, text[i].fX, text[i].fY, paint);\n" +
"    }\n" +
"    canvas->drawText(\"10\", 2, text[10].fX, text[10].fY, paint);\n" +
"    canvas->drawText(\"11\", 2, text[11].fX, text[11].fY, paint);\n" +
"    paint.setStyle(SkPaint::kStroke_Style);\n" +
"    canvas->drawPoints(SkCanvas::kPolygon_PointMode, 12, cubics, paint);\n" +
"    canvas->drawLine(cubics[11].fX, cubics[11].fY, cubics[0].fX, cubics[0].fY, paint);\n" +
"}\n";

var SkCanvas_drawPatch_2_json = {
    "code": SkCanvas_drawPatch_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawPatch_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawPatch_2_json);

var SkCanvas_drawPatch_2_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"    // SkBitmap source = checkerboard;\n" +
"    SkPaint paint;\n" +
"    paint.setFilterQuality(kLow_SkFilterQuality);\n" +
"    paint.setAntiAlias(true);\n" +
"    SkPoint cubics[] = { { 3, 1 },    { 4, 2 }, { 5, 1 },    { 7, 3 },\n" +
"                      /* { 7, 3 }, */ { 6, 4 }, { 7, 5 },    { 5, 7 },\n" +
"                      /* { 5, 7 }, */ { 4, 6 }, { 3, 7 },    { 1, 5 },\n" +
"                      /* { 1, 5 }, */ { 2, 4 }, { 1, 3 }, /* { 3, 1 } */ };\n" +
"    SkPoint texCoords[] = { { 0, 0 }, { 0, 62}, { 62, 62}, { 62, 0 } };\n" +
"    paint.setShader(SkShader::MakeBitmapShader(source, SkShader::kClamp_TileMode,\n" +
"                                                       SkShader::kClamp_TileMode, nullptr));\n" +
"    canvas->scale(30, 30);\n" +
"    canvas->drawPatch(cubics, nullptr, texCoords, paint);\n" +
"}\n";

var SkCanvas_drawPatch_2_2_json = {
    "code": SkCanvas_drawPatch_2_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 6,
        "textOnly": false
    },
    "name": "SkCanvas_drawPatch_2_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawPatch_2_2_json);

var SkCanvas_drawAtlas_code = 
"void draw(SkCanvas* canvas) {\n" +
"  // SkBitmap source = mandrill;\n" +
"  SkRSXform xforms[] = { { .5f, 0, 0, 0 }, {0, .5f, 200, 100 } };\n" +
"  SkRect tex[] = { { 0, 0, 250, 250 }, { 0, 0, 250, 250 } };\n" +
"  SkColor colors[] = { 0x7f55aa00, 0x7f3333bf };\n" +
"  const SkImage* imagePtr = image.get();\n" +
"  canvas->drawAtlas(imagePtr, xforms, tex, colors, 2, SkBlendMode::kSrcOver, nullptr, nullptr);\n" +
"}\n";

var SkCanvas_drawAtlas_json = {
    "code": SkCanvas_drawAtlas_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 3,
        "textOnly": false
    },
    "name": "SkCanvas_drawAtlas",
    "overwrite": true
}

runFiddle(SkCanvas_drawAtlas_json);

var SkCanvas_drawAtlas_2_code = 
"void draw(SkCanvas* canvas) {\n" +
"  // SkBitmap source = mandrill;\n" +
"  SkRSXform xforms[] = { { .5f, 0, 0, 0 }, {0, .5f, 200, 100 } };\n" +
"  SkRect tex[] = { { 0, 0, 250, 250 }, { 0, 0, 250, 250 } };\n" +
"  SkColor colors[] = { 0x7f55aa00, 0x7f3333bf };\n" +
"  SkPaint paint;\n" +
"  paint.setAlpha(127);\n" +
"  canvas->drawAtlas(image, xforms, tex, colors, 2, SkBlendMode::kPlus, nullptr, &paint);\n" +
"}\n";

var SkCanvas_drawAtlas_2_json = {
    "code": SkCanvas_drawAtlas_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 3,
        "textOnly": false
    },
    "name": "SkCanvas_drawAtlas_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawAtlas_2_json);

var SkCanvas_drawAtlas_3_code = 
"void draw(SkCanvas* canvas) {\n" +
"  // sk_sp<SkImage> image = mandrill;\n" +
"  SkRSXform xforms[] = { { .5f, 0, 0, 0 }, {0, .5f, 200, 100 } };\n" +
"  SkRect tex[] = { { 0, 0, 250, 250 }, { 0, 0, 250, 250 } };\n" +
"  const SkImage* imagePtr = image.get();\n" +
"  canvas->drawAtlas(imagePtr, xforms, tex, 2, nullptr, nullptr);\n" +
"}\n";

var SkCanvas_drawAtlas_3_json = {
    "code": SkCanvas_drawAtlas_3_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 3,
        "textOnly": false
    },
    "name": "SkCanvas_drawAtlas_3",
    "overwrite": true
}

runFiddle(SkCanvas_drawAtlas_3_json);

var SkCanvas_drawAtlas_4_code = 
"void draw(SkCanvas* canvas) {\n" +
"  // sk_sp<SkImage> image = mandrill;\n" +
"  SkRSXform xforms[] = { { 1, 0, 0, 0 }, {0, 1, 300, 100 } };\n" +
"  SkRect tex[] = { { 0, 0, 200, 200 }, { 200, 0, 400, 200 } };\n" +
"  canvas->drawAtlas(image, xforms, tex, 2, nullptr, nullptr);\n" +
"}\n";

var SkCanvas_drawAtlas_4_json = {
    "code": SkCanvas_drawAtlas_4_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 3,
        "textOnly": false
    },
    "name": "SkCanvas_drawAtlas_4",
    "overwrite": true
}

runFiddle(SkCanvas_drawAtlas_4_json);

var SkCanvas_drawDrawable_code = 
"struct MyDrawable : public SkDrawable {\n" +
"    SkRect onGetBounds() override { return SkRect::MakeWH(50, 100);  }\n" +
"    void onDraw(SkCanvas* canvas) override {\n" +
"       SkPath path;\n" +
"       path.conicTo(10, 90, 50, 90, 0.9f);\n" +
"       SkPaint paint;\n" +
"       paint.setColor(SK_ColorBLUE);\n" +
"       canvas->drawRect(path.getBounds(), paint);\n" +
"       paint.setAntiAlias(true);\n" +
"       paint.setColor(SK_ColorWHITE);\n" +
"       canvas->drawPath(path, paint);\n" +
"    }\n" +
"};\n" +
"\n" +
"void draw(SkCanvas* canvas) {\n" +
"    sk_sp<SkDrawable> drawable(new MyDrawable);\n" +
"  SkMatrix matrix;\n" +
"  matrix.setTranslate(10, 10);\n" +
"  canvas->drawDrawable(drawable.get(), &matrix);\n" +
"}\n";

var SkCanvas_drawDrawable_json = {
    "code": SkCanvas_drawDrawable_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawDrawable",
    "overwrite": true
}

runFiddle(SkCanvas_drawDrawable_json);

var SkCanvas_drawDrawable_2_code = 
"struct MyDrawable : public SkDrawable {\n" +
"    SkRect onGetBounds() override { return SkRect::MakeWH(50, 100);  }\n" +
"    void onDraw(SkCanvas* canvas) override {\n" +
"       SkPath path;\n" +
"       path.conicTo(10, 90, 50, 90, 0.9f);\n" +
"       SkPaint paint;\n" +
"       paint.setColor(SK_ColorBLUE);\n" +
"       canvas->drawRect(path.getBounds(), paint);\n" +
"       paint.setAntiAlias(true);\n" +
"       paint.setColor(SK_ColorWHITE);\n" +
"       canvas->drawPath(path, paint);\n" +
"    }\n" +
"};\n" +
"\n" +
"void draw(SkCanvas* canvas) {\n" +
"    sk_sp<SkDrawable> drawable(new MyDrawable);\n" +
"  canvas->drawDrawable(drawable.get(), 10, 10);\n" +
"}\n";

var SkCanvas_drawDrawable_2_json = {
    "code": SkCanvas_drawDrawable_2_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawDrawable_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawDrawable_2_json);

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
        "height": 256,
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
        "height": 256,
        "source": 0,
        "textOnly": false
    },
    "name": "SkCanvas_drawAnnotation_2",
    "overwrite": true
}

runFiddle(SkCanvas_drawAnnotation_2_json);

var SkCanvas_isClipEmpty_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkDebugf(\"clip is%s empty\\n\", canvas->isClipEmpty() ? \"\" : \" not\");\n" +
"    SkPath path;\n" +
"    canvas->clipPath(path);\n" +
"    SkDebugf(\"clip is%s empty\\n\", canvas->isClipEmpty() ? \"\" : \" not\");\n" +
"}\n";

var SkCanvas_isClipEmpty_json = {
    "code": SkCanvas_isClipEmpty_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_isClipEmpty",
    "overwrite": true
}

runFiddle(SkCanvas_isClipEmpty_json);

var SkCanvas_isClipRect_code = 
"void draw(SkCanvas* canvas) {\n" +
"    SkDebugf(\"clip is%s rect\\n\", canvas->isClipRect() ? \"\" : \" not\");\n" +
"    canvas->clipRect({0, 0, 0, 0});\n" +
"    SkDebugf(\"clip is%s rect\\n\", canvas->isClipRect() ? \"\" : \" not\");\n" +
"}\n";

var SkCanvas_isClipRect_json = {
    "code": SkCanvas_isClipRect_code,
    "options": {
        "width": 256,
        "height": 256,
        "source": 0,
        "textOnly": true
    },
    "name": "SkCanvas_isClipRect",
    "overwrite": true
}

runFiddle(SkCanvas_isClipRect_json);

}

