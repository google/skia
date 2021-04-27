layout(key) in half one;                       // always equals 1.0
layout(key, when=one != 1.0f) in half unused;  // never used
layout(key) float alsoUnused = one + one;      // also never used

half4 main() {
    half4 color = half4(0);

    // Basic if statement. (00 == 00: true --> color=0001)
    if (color.rg == color.ba) color.a = one;

    // Basic if statement with Block. (00 == 01: false)
    if (color.rg == color.ba) {
        color.r = color.a;
    }

    // TODO(skia:11872): Add test for If statement with comma-expression statement instead of Block.

    // Basic if-else statement. (0 == 0: true --> color=1011)
    if (color.r == color.g) color = color.araa; else color = color.rrra;

    // Chained if-else statements.
    if (color.r + color.g + color.b + color.a == one) {  // (3 == 1: false)
        color = half4(-1);
    } else if (color.r + color.g + color.b + color.a == 2) {  // (3 == 2: false)
        color = half4(-2);
    } else {
        color = color.ggaa; // (color=0011)
    }

    // Nested if-else statements.
    if (color.r == one) {  // (0 == 1: false)
        if (color.r == 2) {
            color = color.rrrr;
        } else {
            color = color.gggg;
        }
    } else {
        if (color.b * color.a == one) { // (1*1 == 1: true)
            color = color.rbga; // (color = 0101)
        } else {
            color = color.aaaa;
        }
    }

    return color;
}
