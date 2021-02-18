
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 v = testInputs;
    v = vec4(v.x, 1.0, 1.0, 1.0);
    v = vec4(v.xy, 1.0, 1.0);
    v = vec4(v.x, 1.0, 1.0, 1.0);
    v = vec4(vec2(0.0, v.y), 1.0, 1.0);
    v = vec4(v.xyz, 1.0);
    v = vec4(v.xy, 1.0, 1.0);
    v = vec4(vec3(v.xz.x, 0.0, v.xz.y), 1.0);
    v = vec4(v.x, 1.0, 0.0, 1.0);
    v = vec4(vec3(1.0, v.yz.xy), 1.0);
    v = vec4(vec3(0.0, v.y, 1.0), 1.0);
    v = vec4(vec3(1.0, 1.0, v.z), 1.0);
    v = vec4(v.xyz, 1.0);
    v = vec4(v.xyw.xy, 0.0, v.xyw.z);
    v = vec4(v.xy, 1.0, 0.0);
    v = vec4(v.xzw.x, 1.0, v.xzw.yz);
    v = vec4(v.xz.x, 0.0, v.xz.y, 1.0);
    v = vec4(v.xw.x, 1.0, 1.0, v.xw.y);
    v = vec4(v.x, 1.0, 0.0, 1.0);
    v = vec4(1.0, v.yzw.xyz);
    v = vec4(0.0, v.yz.xy, 1.0);
    v = vec4(0.0, v.yw.x, 1.0, v.yw.y);
    v = vec4(1.0, v.y, 1.0, 1.0);
    v = vec4(0.0, 0.0, v.zw.xy);
    v = vec4(0.0, 0.0, v.z, 1.0);
    v = vec4(0.0, 1.0, 1.0, v.w);
    return v == vec4(0.0, 1.0, 1.0, 1.0) ? colorGreen : colorRed;
}
