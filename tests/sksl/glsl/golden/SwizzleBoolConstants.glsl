
out vec4 sk_FragColor;
uniform bvec4 v;
void main() {
    bvec4 result;
    result = bvec4(v.x, true, true, true);
    result = bvec4(v.xy, false, true);
    result = bvec4(bvec2(v.x, bool(1)), true, false);
    result = bvec4(bvec2(v.y, bool(0)).yx, true, true);
    result = bvec4(v.xyz, true);
    result = bvec4(bvec3(v.xy, bool(1)), true);
    result = bvec4(bvec3(v.xz, bool(0)).xzy, true);
    result = bvec4(bvec3(v.x, bool(1), bool(0)), false);
    result = bvec4(bvec3(v.yz, bool(1)).zxy, false);
    result = bvec4(bvec3(v.y, bool(0), bool(1)).yxz, false);
    result = bvec4(bvec2(v.z, bool(1)).yyx, false);
    result = v;
    result = bvec4(v.xyz, bool(1));
    result = bvec4(v.xyw, bool(0)).xywz;
    result = bvec4(v.xy, bool(1), bool(0));
    result = bvec4(v.xzw, bool(1)).xwyz;
    result = bvec4(v.xz, bool(0), bool(1)).xzyw;
    result = bvec3(v.xw, bool(1)).xzzy;
    result = bvec3(v.x, bool(1), bool(0)).xyzy;
    result = bvec4(v.yzw, bool(1)).wxyz;
    result = bvec4(v.yz, bool(0), bool(1)).zxyw;
    result = bvec4(v.yw, bool(0), bool(1)).zxwy;
    result = bvec2(v.y, bool(1)).yxyy;
    result = bvec3(v.zw, bool(0)).zzxy;
    result = bvec3(v.z, bool(0), bool(1)).yyxz;
    result = bvec3(v.w, bool(0), bool(1)).yzzx;
    sk_FragColor = any(result) ? vec4(1.0) : vec4(0.0);
}
