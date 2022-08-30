
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform float unknownInput;
uniform mat2 testMatrix2x2;
vec4 func1_h4h(float h) {
    return vec4(h) * vec4(h);
}
vec4 funcA4_h4h(float a[4]) {
    return vec4(a[0], a[1], a[2], 1.0) * a[3];
}
vec4 funcA5_h4h(float a[5]) {
    return vec4(a[0], a[1], a[2], a[3]) * a[4];
}
vec4 main() {
    vec4 var;
    int i = int(unknownInput);
    var = funcA4_h4h(float[4](1.0, 2.0, 3.0, 4.0));
    var = func1_h4h(colorGreen[i]);
    var = funcA5_h4h(float[5](1.0, 2.0, 3.0, 4.0, 5.0));
    return colorGreen;
}
