
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 green_h4() {
    vec4 x = colorGreen;
    return x;
}
vec4 main() {
    vec4 result = colorRed;
    while (result != colorGreen) result = green_h4();
    return result;
}
