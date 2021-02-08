
out vec4 sk_FragColor;
uniform vec4 colorWhite;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec2 x = colorWhite.xy;
    x = -x;
    return x == vec2(-1.0) ? colorGreen : colorRed;
}
