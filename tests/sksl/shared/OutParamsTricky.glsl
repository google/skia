
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 result = vec4(0.0, 1.0, 2.0, 3.0);
    result.xz.xy = vec2(2.0, 0.0);

    result.yw = vec2(3.0, 5.0);

    return result == vec4(2.0, 3.0, 0.0, 5.0) ? colorGreen : colorRed;
}
