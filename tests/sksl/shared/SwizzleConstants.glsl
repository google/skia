
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 v = testInputs;
    v = vec4(v.x, 1.0, 1.0, 1.0);
    v = vec4(v.xy, 1.0, 1.0);
    v = vec4(vec2(v.x, float(1)), 1.0, 1.0);
    v = vec4(vec2(v.y, float(0)).yx, 1.0, 1.0);
    v = vec4(v.xyz, 1.0);
    v = vec4(vec3(v.xy, float(1)), 1.0);
    v = vec4(vec3(v.xz, float(0)).xzy, 1.0);
    v = vec4(vec3(v.x, float(1), float(0)), 1.0);
    v = vec4(vec3(v.yz, float(1)).zxy, 1.0);
    v = vec4(vec3(v.y, float(0), float(1)).yxz, 1.0);
    v = vec4(vec2(v.z, float(1)).yyx, 1.0);
    v = v.xyzw;
    v = vec4(v.xyz, float(1));
    v = vec4(v.xyw, float(0)).xywz;
    v = vec4(v.xy, float(1), float(0));
    v = vec4(v.xzw, float(1)).xwyz;
    v = vec4(v.xz, float(0), float(1)).xzyw;
    v = vec3(v.xw, float(1)).xzzy;
    v = vec3(v.x, float(1), float(0)).xyzy;
    v = vec4(v.yzw, float(1)).wxyz;
    v = vec4(v.yz, float(0), float(1)).zxyw;
    v = vec4(v.yw, float(0), float(1)).zxwy;
    v = vec2(v.y, float(1)).yxyy;
    v = vec3(v.zw, float(0)).zzxy;
    v = vec3(v.z, float(0), float(1)).yyxz;
    v = vec3(v.w, float(0), float(1)).yzzx;
    return v == vec4(0.0, 1.0, 1.0, 1.0) ? colorGreen : colorRed;
}
