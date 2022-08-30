#version 400
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 MakeTempVar_h4h4(vec4 c) {
    {
        vec4 d = colorGreen;
        c = d;
    }
    {
        return c;
    }
}
vec4 main() {
    return MakeTempVar_h4h4(colorRed);
}
