layout(ctype=SkPMColor4f) in uniform half4 colorGreen, colorRed;

half4 main() {
    bool t = true;
    bool f = false;

    return half4(t ? colorGreen.r : colorRed.r,                             // true  -> colorGreen.r
                 f ? colorRed.g : colorGreen.g,                             // false -> colorGreen.g
                 (colorGreen.g == colorRed.r) ? colorGreen.b : colorRed.r,  // true  -> colorGreen.b
                 (colorGreen.a != colorRed.a) ? colorRed.g : colorGreen.a); // false -> colorGreen.a
}
