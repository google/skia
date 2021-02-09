
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 constant_swizzle() {
    vec4 v = testInputs;
    float x = v.x;
    float y = v.y;
    float z = v.z;
    float w = v.w;
    return vec4(x, y, z, w);
}
vec4 foldable() {
    vec4 v = vec4(0.0, 1.0, 2.0, 3.0);
    float x = v.x;
    float y = v.y;
    float z = v.z;
    float w = v.w;
    return vec4(x, y, z, w);
}
vec4 main() {
    vec4 a = constant_swizzle();
    vec4 b = foldable();
    return a == vec4(-1.25, 0.0, 0.75, 2.25) && b == vec4(0.0, 1.0, 2.0, 3.0) ? colorGreen : colorRed;
}
