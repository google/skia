layout(key) in bool primaryColors;
layout(ctype=SkPMColor4f, when=primaryColors) in uniform half4 colorGreen, colorRed;
layout(ctype=SkPMColor4f, when=!primaryColors) in uniform half4 colorOrange, colorPurple;

half4 main() {
    half4 green = primaryColors ? colorGreen : colorOrange;
    half4 red = primaryColors ? colorRed : colorPurple;
    bool t = true;
    bool f = false;

    return half4(t ? green.r : red.r,                     // true  -> green.r
                 f ? red.g : green.g,                     // false -> green.g
                 (green.g == red.r) ? green.b : red.r,    // true  -> green.b
                 (green.a != red.a) ? red.g : green.a);   // false -> green.a
}
