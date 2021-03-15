struct Inputs {
    int a;
    uint b;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = float(popcount(_in.a));
    _out.sk_FragColor.y = float(popcount(_in.b));
    return _out;
}
