
out vec4 sk_FragColor;
void main() {
    bvec4 v = bvec4(sqrt(1.0) == 1.0);
    bvec4 result;
    result = bvec4(v.x, true, true, true);
    result = bvec4(v.xy, false, true);
    result = bvec4(bvec2(v.x, true), true, false);
    result = bvec4(bvec2(v.y, false).yx, true, true);
    result = bvec4(v.xyz, true);
    result = bvec4(bvec3(v.xy, true), true);
    result = bvec4(bvec3(v.xz, false).xzy, true);
    result = bvec4(bvec3(v.x, true, false), false);
    result = bvec4(bvec3(v.yz, true).zxy, false);
    result = bvec4(bvec3(v.y, false, true).yxz, false);
    result = bvec4(bvec2(v.z, true).yyx, false);
    result = v;
    result = bvec4(v.xyz, true);
    result = bvec4(v.xyw, false).xywz;
    result = bvec4(v.xy, true, false);
    result = bvec4(v.xzw, true).xwyz;
    result = bvec4(v.xz, false, true).xzyw;
    result = bvec3(v.xw, true).xzzy;
    result = bvec3(v.x, true, false).xyzy;
    result = bvec4(v.yzw, true).wxyz;
    result = bvec4(v.yz, false, true).zxyw;
    result = bvec4(v.yw, false, true).zxwy;
    result = bvec2(v.y, true).yxyy;
    result = bvec3(v.zw, false).zzxy;
    result = bvec3(v.z, false, true).yyxz;
    result = bvec3(v.w, false, true).yzzx;
    sk_FragColor = any(result) ? vec4(1.0) : vec4(0.0);
}
