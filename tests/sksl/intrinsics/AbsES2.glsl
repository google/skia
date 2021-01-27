
out vec4 sk_FragColor;
uniform vec4 minus1234;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((abs(minus1234.x) == 1.0 && abs(minus1234.xy) == vec2(1.0, 2.0)) && abs(minus1234.xyz) == vec3(1.0, 2.0, 3.0)) && abs(minus1234) == vec4(1.0, 2.0, 3.0, 4.0) ? colorGreen : colorRed;
}
