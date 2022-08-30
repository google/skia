#version 400
out vec4 sk_FragColor;
uniform vec4 colorWhite;
vec4 MakeTempVar_h4h4(vec4 c) {
    {
        vec4 d = c;
        return vec4(0.0, d.y, 0.0, d.w);
    }
}
vec4 main() {
    return MakeTempVar_h4h4(colorWhite);
}
