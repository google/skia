#version 400
out vec4 sk_FragColor;
uniform vec4 colorWhite;
vec4 main() {
    vec4 _0_MakeTempVar;
    {
        vec4 _1_d = colorWhite;
        _0_MakeTempVar = vec4(0.0, _1_d.y, 0.0, _1_d.w);
    }
    return _0_MakeTempVar;
}
