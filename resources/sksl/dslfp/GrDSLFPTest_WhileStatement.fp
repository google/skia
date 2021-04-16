// (This test code was largely borrowed from shared/WhileLoopControlFlow.sksl.)
half4 main() {
    half4 color = half4(1);

    // Basic while loop without a block.
    while (color.r > 0.5) color.r -= 0.25;

    // While loop with a block and a break statement.
    while (color.a == 1) {
        color.r -= 0.25;
        if (color.r <= 0) break;
    }

    // While loop with a block and a continue statement.
    while (color.b > 0) {
        color.b -= 0.25;
        if (color.a == 1) continue; // should always happen
        color.g = 0;
    }

    // color contains green.
    return color;
}
