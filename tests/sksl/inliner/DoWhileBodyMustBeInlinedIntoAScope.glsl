
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 get_h4() {
    vec4 x = colorGreen;
    return x;
}
vec4 main() {
    vec4 result = colorRed;
    do result = get_h4(); while (result != colorGreen);
    return result;
}
