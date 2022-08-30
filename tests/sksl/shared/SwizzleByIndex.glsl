
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorBlack;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 non_constant_swizzle_h4() {
    vec4 v = testInputs;
    ivec4 i = ivec4(colorBlack);
    float x = v[i.x];
    float y = v[i.y];
    float z = v[i.z];
    float w = v[i.w];
    return vec4(x, y, z, w);
}
vec4 main() {
    return non_constant_swizzle_h4() == vec4(-1.25, -1.25, -1.25, 0.0) ? colorGreen : colorRed;
}
