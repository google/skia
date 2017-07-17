void main() {
    // Generate a random number based on the fragment position. For this
    // random number generator, we use the "GLSL rand" function
    // that seems to be floating around on the internet. It works under
    // the assumption that sin(<big number>) oscillates with high frequency
    // and sampling it will generate "randomness". Since we're using this
    // for rendering and not cryptography it should be OK.

    // For each channel c, add the random offset to the pixel to either bump
    // it up or let it remain constant during quantization.
    float r = fract(sin(dot(sk_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453) - .5;
    sk_OutColor = clamp(1 / 255.0 * vec4(r) + sk_InColor, 0, 1);
}

@test(testData) {
    return GrDitherEffect::Make();
}