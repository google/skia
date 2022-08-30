#version 400
out vec4 sk_FragColor;
uniform vec4 colorWhite;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 MakeTempVar_h4h4(vec4 c) {
    if (c.xy == c.zw) {
        return colorGreen;
    } else {
        return colorRed;
    }
}
vec4 main() {
    return MakeTempVar_h4h4(colorWhite);
}
