
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 v = testInputs;
    v = vec4(v.x, 1.0, 1.0, 1.0);
    v = vec4(v.xy, 1.0, 1.0);
    v = vec4(v.x, 1.0, 1.0, 1.0);
    v = vec4(0.0, v.y, 1.0, 1.0);
    v = vec4(v.xyz, 1.0);
    v = vec4(v.xy, 1.0, 1.0);
    v = vec4(v.x, 0.0, v.z, 1.0);
    v = vec4(v.x, 1.0, 0.0, 1.0);
    v = vec4(1.0, v.yz, 1.0);
    v = vec4(0.0, v.y, 1.0, 1.0);
    v = vec4(1.0, 1.0, v.z, 1.0);
    v = vec4(v.xyz, 1.0);
    v = vec4(v.xy, 0.0, v.w);
    v = vec4(v.xy, 1.0, 0.0);
    v = vec4(v.x, 1.0, v.zw);
    v = vec4(v.x, 0.0, v.z, 1.0);
    v = vec4(v.x, 1.0, 1.0, v.w);
    v = vec4(v.x, 1.0, 0.0, 1.0);
    v = vec4(1.0, v.yzw);
    v = vec4(0.0, v.yz, 1.0);
    v = vec4(0.0, v.y, 1.0, v.w);
    v = vec4(1.0, v.y, 1.0, 1.0);
    v = vec4(0.0, 0.0, v.zw);
    v = vec4(0.0, 0.0, v.z, 1.0);
    v = vec4(0.0, 1.0, 1.0, v.w);
    return v == vec4(0.0, 1.0, 1.0, 1.0) ? colorGreen : colorRed;
}
