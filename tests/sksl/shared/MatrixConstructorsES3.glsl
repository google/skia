
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
vec4 main() {
    vec4 f4 = vec4(testMatrix2x2);
    bool ok = mat2x3(f4, f4.xy) == mat2x3(1.0, 2.0, 3.0, 4.0, 1.0, 2.0);
    ok = ok && mat2x4(f4.xyz, f4.wxyz, f4.w) == mat2x4(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0);
    ok = ok && mat3(f4.xy, f4.zw, f4, f4.x) == mat3(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0);
    ok = ok && mat4x2(f4.xyz, f4.wxyz, f4.w) == mat4x2(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0);
    ok = ok && mat4x3(f4.x, f4.yzwx, f4.yzwx, f4.yzw) == mat4x3(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0);
    return ok ? colorGreen : colorRed;
}
