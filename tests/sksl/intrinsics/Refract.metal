struct Inputs {
    float a;
    float b;
    float c;
    float4 d;
    float4 e;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = (refract(float2(_in.a, 0), float2(_in.b, 0), _in.c).x);
    _out.sk_FragColor = refract(_in.d, _in.e, _in.c);
    return _out;
}
