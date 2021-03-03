
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 v = testInputs;
    v = vec4(v.x, 1.0, 1.0, 1.0);
    v = vec4(v.xy, 1.0, 1.0);
    v = vec4(vec2(v.x, 1.0), 1.0, 1.0);
    v = vec4(vec2(v.y, 0.0).yx, 1.0, 1.0);
    v = vec4(v.xyz, 1.0);
    v = vec4(vec3(v.xy, 1.0), 1.0);
    v = vec4(vec3(v.xz, 0.0).xzy, 1.0);
    v = vec4(vec3(v.x, 1.0, 0.0), 1.0);
    v = vec4(vec3(v.yz, 1.0).zxy, 1.0);
    v = vec4(vec3(v.y, 0.0, 1.0).yxz, 1.0);
    v = vec4(vec2(v.z, 1.0).yyx, 1.0);
    v = vec4(v.xyz, 1.0);
    v = vec4(v.xyw, 0.0).xywz;
    v = vec4(v.xy, 1.0, 0.0);
    v = vec4(v.xzw, 1.0).xwyz;
    v = vec4(v.xz, 0.0, 1.0).xzyw;
    v = vec3(v.xw, 1.0).xzzy;
    v = vec3(v.x, 1.0, 0.0).xyzy;
    v = vec4(v.yzw, 1.0).wxyz;
    v = vec4(v.yz, 0.0, 1.0).zxyw;
    v = vec4(v.yw, 0.0, 1.0).zxwy;
    v = vec2(v.y, 1.0).yxyy;
    v = vec3(v.zw, 0.0).zzxy;
    v = vec3(v.z, 0.0, 1.0).yyxz;
    v = vec3(v.w, 0.0, 1.0).yzzx;
    return v == vec4(0.0, 1.0, 1.0, 1.0) ? colorGreen : colorRed;
}
