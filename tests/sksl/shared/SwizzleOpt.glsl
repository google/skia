
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform vec4 testInputs;
float fn(vec4 v) {
    for (int x = 1;x <= 2; ++x) {
        return v.x;
    }
}
vec4 main() {
    vec4 v = testInputs;
    v = vec4(v.xyz, 0.0).wzyx;
    v = vec3(v.xw, 0.0).zzxy;
    v = vec3(vec3(v.xxxw.xw, 0.0).zzxy.wz, 1.0).zzxy;
    v = vec3(v.wzyw.yz, 1.0).xyzz;
    v = v.wzyx.wzyx;
    v = vec4(v.xxxx.zz, 1.0, 1.0);
    v = v.zw.yxyx;
    v = vec3(fn(v), 123.0, 456.0).yyzz;
    v = vec3(fn(v), vec2(123.0, 456.0)).yyzz;
    v = vec3(fn(v), 123.0, 456.0).yzzx;
    v = vec3(fn(v), vec2(123.0, 456.0)).yzzx;
    v = vec3(fn(v), 123.0, 456.0).yxxz;
    v = vec3(fn(v), vec2(123.0, 456.0)).yxxz;
    v = vec4(1.0, 2.0, 3.0, 4.0).xxyz;
    v = vec4(1.0, colorRed.xyz).yzwx;
    v = vec4(1.0, colorRed.xyz).yxzw;
    v.wzyx = v;
    v.xw = v.yz;
    v.wzyx.yzw = vec3(v.ww, 1.0);
    return v == vec4(1.0) ? colorGreen : colorRed;
}
