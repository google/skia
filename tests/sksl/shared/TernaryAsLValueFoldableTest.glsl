
out vec4 sk_FragColor;
uniform float unknownInput;
vec4 main() {
    float r;
    float g;
    r = 1.0 - unknownInput;
    g = unknownInput;
    return vec4(r, g, 0.0, 1.0);
}
