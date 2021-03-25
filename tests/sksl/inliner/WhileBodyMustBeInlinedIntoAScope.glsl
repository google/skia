
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 result = colorRed;
    while (result != colorGreen) {
        vec4 _0_x = colorGreen;
        result = _0_x;
    }
    return result;
}
