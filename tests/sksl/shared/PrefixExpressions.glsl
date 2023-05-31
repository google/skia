
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool ok = true;
    int i = 5;
    ++i;
    ok = ok && i == 6;
    ok = ok && ++i == 7;
    ok = ok && --i == 6;
    --i;
    ok = ok && i == 5;
    float f = 0.5;
    ++f;
    ok = ok && f == 1.5;
    ok = ok && ++f == 2.5;
    ok = ok && --f == 1.5;
    --f;
    ok = ok && f == 0.5;
    ok = ok && !(colorGreen.x == 1.0);
    uint val = uint(colorGreen.x);
    uvec2 mask = uvec2(val, ~val);
    ivec2 imask = ivec2(~mask);
    mask = ~mask & uvec2(~imask);
    ok = ok && mask == uvec2(0u);
    float one = colorGreen.x;
    mat4 m = mat4(one);
    return ok ? -m * -colorGreen : colorRed;
}
