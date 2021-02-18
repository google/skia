
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
    v = vec4(0.0, v.xyz.zyx);
    v = vec4(0.0, 0.0, v.xw.xy);
    v = vec4(1.0, 1.0, v.xw.yx.xy);
    v = vec4(v.zy.xy, 1.0, 1.0);
    v = vec4(v.xx, 1.0, 1.0);
    v = v.wzwz;
    v = vec3(fn(v), 123.0, 456.0).yyzz;
    v = vec3(fn(v), 123.0, 456.0).yyzz;
    v = vec4(123.0, 456.0, 456.0, fn(v));
    v = vec4(123.0, 456.0, 456.0, fn(v));
    v = vec3(fn(v), 123.0, 456.0).yxxz;
    v = vec3(fn(v), 123.0, 456.0).yxxz;
    v = vec4(1.0, 1.0, 2.0, 3.0);
    v = vec4(colorRed.xyz.xyz, 1.0);
    v = vec4(colorRed.xyz.x, 1.0, colorRed.xyz.yz);
    v.wzyx = v;
    v.xw = v.yz;
    v.zyx = vec3(v.ww, 1.0);
    return v == vec4(1.0) ? colorGreen : colorRed;
}
