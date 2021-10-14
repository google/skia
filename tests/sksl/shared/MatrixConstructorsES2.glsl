
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
vec4 main() {
    vec4 f4 = vec4(testMatrix2x2);
    bool ok = mat2(f4.xyz, 4.0) == mat2(1.0, 2.0, 3.0, 4.0);
    ok = ok && mat3(f4.xy, f4.zw, f4, f4.x) == mat3(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0);
    ok = ok && mat4(f4.xyz, f4.wxy, f4.zwxy, f4.zw, f4) == mat4(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0);
    return ok ? colorGreen : colorRed;
}
