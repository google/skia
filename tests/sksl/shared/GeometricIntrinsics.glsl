
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 main() {
    float _0_x = 1.0;
    _0_x = length(_0_x);
    _0_x = distance(_0_x, 2.0);
    _0_x = dot(_0_x, 2.0);
    _0_x = normalize(_0_x);
    vec2 _1_x = vec2(1.0, 2.0);
    _1_x = vec2(length(_1_x));
    _1_x = vec2(distance(_1_x, vec2(3.0, 4.0)));
    _1_x = vec2(dot(_1_x, vec2(3.0, 4.0)));
    _1_x = normalize(_1_x);
    return colorGreen;
}
