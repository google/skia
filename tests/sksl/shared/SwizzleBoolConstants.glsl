
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bvec4 v = bvec4(bool(colorGreen.y));
    bvec4 result;
    result = bvec4(v.x, true, true, true);
    result = bvec4(v.xy, false, true);
    result = bvec4(v.x, true, true, false);
    result = bvec4(false, v.y, true, true);
    result = bvec4(v.xyz, true);
    result = bvec4(v.xy, true, true);
    result = bvec4(v.x, false, v.z, true);
    result = bvec4(v.x, true, false, false);
    result = bvec4(true, v.yz, false);
    result = bvec4(false, v.y, true, false);
    result = bvec4(true, true, v.z, false);
    result = v;
    result = bvec4(v.xyz, true);
    result = bvec4(v.xy, false, v.w);
    result = bvec4(v.xy, true, false);
    result = bvec4(v.x, true, v.zw);
    result = bvec4(v.x, false, v.z, true);
    result = bvec4(v.x, true, true, v.w);
    result = bvec4(v.x, true, false, true);
    result = bvec4(true, v.yzw);
    result = bvec4(false, v.yz, true);
    result = bvec4(false, v.y, true, v.w);
    result = bvec4(true, v.y, true, true);
    result = bvec4(false, false, v.zw);
    result = bvec4(false, false, v.z, true);
    result = bvec4(false, true, true, v.w);
    return any(result) ? colorGreen : colorRed;
}
