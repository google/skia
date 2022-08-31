
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 result = colorRed;
    do {
        vec4 _0_x = colorGreen;
        result = _0_x;
    } while (result != colorGreen);
    return result;
}
