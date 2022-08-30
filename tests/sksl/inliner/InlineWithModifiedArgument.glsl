
out vec4 sk_FragColor;
uniform vec4 colorGreen;
float parameterWrite_hh(float x) {
    x *= x;
    return x;
}
vec4 main() {
    vec4 c = colorGreen;
    c.y = parameterWrite_hh(c.y);
    return c;
}
