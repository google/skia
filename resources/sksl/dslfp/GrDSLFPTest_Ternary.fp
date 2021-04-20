layout(ctype=SkPMColor4f) in uniform half4 colorGreen, colorRed;

half4 main() {
    half4 green = colorGreen;
    half4 red   = colorRed;
    bool  t     = true;
    bool  f     = false;

    return half4(t ? green.r : red.r,                     // true  -> green.r
                 f ? red.g : green.g,                     // false -> green.g
                 (green.g == red.r) ? green.b : red.r,    // true  -> green.b
                 (green.a != red.a) ? red.g : green.a);   // false -> green.a
}
