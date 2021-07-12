
out vec4 sk_FragColor;
uniform float unknownInput;
vec4 main() {
    int i = int(unknownInput);
    ivec4 i4 = ivec4(i);
    i4 = ivec4(ivec2(i), 0, 1);
    i4 = ivec4(0, i, 1, 0);
    i4 = ivec4(0, i, 0, i);
    return vec4(i4);
}
