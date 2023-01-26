
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool ok = true;
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
