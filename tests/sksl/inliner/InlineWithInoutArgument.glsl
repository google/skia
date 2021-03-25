
out vec4 sk_FragColor;
uniform vec4 colorGreen;
void outParameter_vh4(inout vec4 x) {
    x *= x;
}
vec4 main() {
    vec4 c = colorGreen;
    outParameter_vh4(c);
    return c;
}
