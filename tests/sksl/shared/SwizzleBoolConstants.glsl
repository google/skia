
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bvec4 v = bvec4(bool(colorGreen.y));
    bvec4 result;
    result = bvec4(v.x, true, true, true);
    result = bvec4(v.xy, false, true);
    result = bvec4(v.x, true, true, false);
    result = bvec4(bvec2(false, v.y), true, true);
    result = bvec4(v.xyz, true);
    result = bvec4(v.xy, true, true);
    result = bvec4(bvec3(v.xz.x, false, v.xz.y), true);
    result = bvec4(v.x, true, false, false);
    result = bvec4(bvec3(true, v.yz.xy), false);
    result = bvec4(bvec3(false, v.y, true), false);
    result = bvec4(bvec3(true, true, v.z), false);
    result = v;
    result = bvec4(v.xyz, true);
    result = bvec4(v.xyw.xy, false, v.xyw.z);
    result = bvec4(v.xy, true, false);
    result = bvec4(v.xzw.x, true, v.xzw.yz);
    result = bvec4(v.xz.x, false, v.xz.y, true);
    result = bvec4(v.xw.x, true, true, v.xw.y);
    result = bvec4(v.x, true, false, true);
    result = bvec4(true, v.yzw.xyz);
    result = bvec4(false, v.yz.xy, true);
    result = bvec4(false, v.yw.x, true, v.yw.y);
    result = bvec4(true, v.y, true, true);
    result = bvec4(false, false, v.zw.xy);
    result = bvec4(false, false, v.z, true);
    result = bvec4(false, true, true, v.w);
    return any(result) ? colorGreen : colorRed;
}
