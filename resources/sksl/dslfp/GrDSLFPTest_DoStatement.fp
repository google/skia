layout(key) in bool shouldLoop;  // always equals false

// (This test code was largely borrowed from shared/DoWhileControlFlow.sksl.)
half4 main() {
    half4 color = half4(1, 1, 1, 1);

    // Simple do-while loop, with no Block.
    do color.r -= 0.25; while (shouldLoop);

    // Do-while loop with a Block and Break in the middle.
    do {
        color.r -= 0.25;
        if (color.r <= 0) break;
    } while (color.a == 1);

    // Do-while loop with a Block and Continue in the middle.
    do {
        color.b -= 0.25;
        if (color.a == 1) continue; // should always happen
        color.g = 0;
    } while (color.b > 0);

    // color contains green.
    return color;
}
