
out vec4 sk_FragColor;
uniform float unknownInput;
vec4 main() {
    bool b = bool(unknownInput);
    bvec4 b4 = bvec4(b);
    b4 = bvec4(bvec2(b), false, true);
    b4 = bvec4(false, b, true, false);
    b4 = bvec4(false, b, false, b);
    return vec4(b4);
}
