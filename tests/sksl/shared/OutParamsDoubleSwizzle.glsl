
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec2 swizzle_lvalue_h2hhh2h(float x, float y, inout vec2 color, float z) {
    color.yx = color;
    return vec2(x + y, z);
}
void func_vh4(inout vec4 color) {
    vec2 t = swizzle_lvalue_h2hhh2h(1.0, 2.0, color.xz, 5.0);
    color.yw = t;
}
vec4 main() {
    vec4 result = vec4(0.0, 1.0, 2.0, 3.0);
    func_vh4(result);
    return result == vec4(2.0, 3.0, 0.0, 5.0) ? colorGreen : colorRed;
}
