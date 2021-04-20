half4 main() {
    half4 v = half4(1, 2, 3, 4);

    v = half4(v.x, 1, 1, 1);
    v = half4(v.xy, 1, 1);
    v = half4(v.x1, 1, 1);
    v = half4(v.0y, 1, 1);
    v = half4(v.xyz, 1);
    v = half4(v.xy1, 1);
    v = half4(v.x0z, 1);
    v = half4(v.x10, 1);
    v = half4(v.1yz, 1);
    v = half4(v.0y1, 1);
    v = half4(v.11z, 1);
    v = v.xyzw;
    v = v.xyz1;
    v = v.wwww;
    v = v.xy10;
    v = v.xzzx;
    v = v.x0z1;
    v = v.x11w;
    v = v.x101;
    v = v.1yzw;
    v = v.0yz1;
    v = v.0y1w;
    v = v.1y11;
    v = v.00zw;
    v = v.00z1;
    v = v.011w;

    v = v.rgba;
    v = v.rgb0.abgr;
    v = v.rgba.00ra;
    v = v.rgba.rrra.00ra.11ab;
    v = v.abga.gb11;
    v = v.abgr.abgr;
    v = half4(v.rrrr.bb, 1, 1);
    v = half4(v.ba.grgr);

    bool4 b = bool4(true, true, true, true);
    b = bool4(b.x, true, true, true);
    b = bool4(b.xy, false, true);
    b = bool4(b.x1, true, false);
    b = bool4(b.0y, true, true);
    b = bool4(b.xyz, true);
    b = bool4(b.xy1, true);
    b = bool4(b.x0z, true);
    b = bool4(b.x10, false);
    b = bool4(b.1yz, false);
    b = bool4(b.0y1, false);
    b = bool4(b.11z, false);
    b = b.xyzw;
    b = b.xyz1;
    b = b.wwww;
    b = b.xy10;
    b = b.xzzx;
    b = b.x0z1;
    b = b.x11w;
    b = b.x101;
    b = b.1yzw;
    b = b.0yz1;
    b = b.0y1w;
    b = b.1y11;
    b = b.00zw;
    b = b.00z1;
    b = b.011w;

    return half4(b.xy, 0, v.z);
}
