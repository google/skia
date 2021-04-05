
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    if (vec4(0.0, 0.0, 1.0, 1.0) == vec4(ivec4(0, 0, 1, 1))) return colorGreen; else return colorRed;
}
