struct Inputs {
    float4 src;
    float4 dst;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 _0_result = _in.src + (1.0 - _in.src.w) * _in.dst;
    _0_result.xyz = max(_0_result.xyz, (1.0 - _in.dst.w) * _in.src.xyz + _in.dst.xyz);
    _out.sk_FragColor = _0_result;
    return _out;
}
