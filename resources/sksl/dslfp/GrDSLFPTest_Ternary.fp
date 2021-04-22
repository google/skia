half4 main() {
    half4 green = half4(0, 1, 0, 1);
    half4 red   = half4(1, 0, 0, 1);
    bool  t     = true;
    bool  f     = false;

    return half4(t ? green.r : red.r,                     // true  -> green.r
                 f ? red.g : green.g,                     // false -> green.g
                 (green.g == red.r) ? green.b : red.r,    // true  -> green.b
                 (green.a != red.a) ? red.g : green.a);   // false -> green.a
}
