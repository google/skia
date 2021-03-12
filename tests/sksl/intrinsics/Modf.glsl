
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 value = colorGreen.yyyy * 2.5;
    vec4 whole;
    vec4 fraction;
    bvec4 ok;
    fraction.x = modf(value.x, whole.x);
    ok.x = whole.x == 2.0 && fraction.x == 0.5;
    fraction.xy = modf(value.xy, whole.xy);
    ok.y = whole.y == 2.0 && fraction.y == 0.5;
    fraction.xyz = modf(value.xyz, whole.xyz);
    ok.z = whole.z == 2.0 && fraction.z == 0.5;
    fraction = modf(value, whole);
    ok.w = whole.w == 2.0 && fraction.w == 0.5;
    return all(ok) ? colorGreen : colorRed;
}
