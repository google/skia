
out vec4 sk_FragColor;
void main() {
    bvec4 v = bvec4(sqrt(1.0) == 1.0);
    bvec4 result;
    result = bvec4(v.x, true, true, true);
    result = bvec4(v.xy, false, true);
    result = bvec4(bvec2(v.x, true), true, false);
    result = bvec4(bvec2(false, v.y), true, true);
    result = bvec4(v.xyz, true);
    result = bvec4(bvec3(v.xy, true), true);
    result = bvec4(bvec3(v.xz, false).xzy, true);
    result = bvec4(bvec3(v.x, true, false), false);
    result = bvec4(bvec3(v.yz, true).zxy, false);
    result = bvec4(bvec3(false, v.y, true), false);
    result = bvec4(bvec3(true, true, v.z), false);
    result = v;
    result = bvec4(v.xyz, true);
    result = bvec4(v.xyw, false).xywz;
    result = bvec4(v.xy, true, false);
    result = bvec4(v.xzw, true).xwyz;
    result = bvec4(v.xz, false, true).xzyw;
    result = bvec3(v.xw, true).xzzy;
    result = bvec4(v.x, true, false, true);
    result = bvec4(v.yzw, true).wxyz;
    result = bvec4(v.yz, false, true).zxyw;
    result = bvec4(v.yw, false, true).zxwy;
    result = bvec4(true, v.y, true, true);
    result = bvec3(v.zw, false).zzxy;
    result = bvec4(false, false, v.z, true);
    result = bvec4(false, true, true, v.w);
    sk_FragColor = any(result) ? vec4(1.0) : vec4(0.0);
}
