
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform float unknownInput;
uniform mat2 testMatrix2x2;
vec4 main() {
    vec4 var;
    int i = int(unknownInput);
    var = vec4(4.0, 8.0, 12.0, 4.0);
    float _0_h = colorGreen[i];
    var = vec4(_0_h) * vec4(_0_h);
    float _1_a[5] = float[5](1.0, 2.0, 3.0, 4.0, 5.0);
    var = vec4(_1_a[0], _1_a[1], _1_a[2], _1_a[3]) * _1_a[4];
    i *= int(var.x);
    return colorGreen;
}
