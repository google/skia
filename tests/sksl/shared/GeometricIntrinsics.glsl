
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 main() {
    float _0_scalar;
    float _1_x = 1.0;
    _1_x = length(_1_x);
    _1_x = distance(_1_x, 2.0);
    _1_x = dot(_1_x, 2.0);
    _1_x = normalize(_1_x);
    float x = _1_x;

    vec2 _2_vector;
    vec2 _3_x = vec2(1.0, 2.0);
    _3_x = vec2(length(_3_x));
    _3_x = vec2(distance(_3_x, vec2(3.0, 4.0)));
    _3_x = vec2(dot(_3_x, vec2(3.0, 4.0)));
    _3_x = normalize(_3_x);
    vec2 y = _3_x;

    return colorGreen;
}
