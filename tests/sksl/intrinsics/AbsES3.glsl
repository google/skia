
out vec4 sk_FragColor;
uniform vec4 minus1234;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((abs(int(minus1234.x)) == 1 && abs(ivec2(minus1234.xy)) == ivec2(1, 2)) && abs(ivec3(minus1234.xyz)) == ivec3(1, 2, 3)) && abs(ivec4(minus1234)) == ivec4(1, 2, 3, 4) ? colorGreen : colorRed;
}
