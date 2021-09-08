
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 value = vec4(2.5, -2.5, 8.0, -0.125);
    const vec4 expectedWhole = vec4(2.0, -2.0, 8.0, 0.0);
    const vec4 expectedFraction = vec4(0.5, -0.5, 0.0, -0.125);
    bvec4 ok = bvec4(false);
    vec4 whole;
    vec4 fraction;
    fraction.x = modf(value.x, whole.x);
    ok.x = whole.x == 2.0 && fraction.x == 0.5;
    fraction.xy = modf(value.xy, whole.xy);
    ok.y = whole.xy == vec2(2.0, -2.0) && fraction.xy == vec2(0.5, -0.5);
    fraction.xyz = modf(value.xyz, whole.xyz);
    ok.z = whole.xyz == vec3(2.0, -2.0, 8.0) && fraction.xyz == vec3(0.5, -0.5, 0.0);
    fraction = modf(value, whole);
    ok.w = whole == expectedWhole && fraction == expectedFraction;
    return all(ok) ? colorGreen : colorRed;
}
