struct Inputs {
    float a;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    int b;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = ldexp(_in.a, _globals.b);
    return _out;
}
