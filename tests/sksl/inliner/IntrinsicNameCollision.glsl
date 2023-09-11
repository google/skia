
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 outer_h4() {
    float _0_distance = colorGreen.w;
    vec4 color = vec4(distance(colorGreen.xw, colorGreen.xw), distance(colorGreen.xw, colorGreen.yw), distance(colorGreen.xw, colorGreen.zw), distance(colorGreen.xw, colorGreen.ww));
    return color * _0_distance;
}
vec4 main() {
    return outer_h4();
}
