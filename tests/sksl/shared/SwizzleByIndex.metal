struct Uniforms {
    float4 testInputs;
    float4 colorBlack;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

float4 non_constant_swizzle(Uniforms _uniforms) {
    float4 v = _uniforms.testInputs;
    int4 i = int4(_uniforms.colorBlack);
    float x = v[i.x];
    float y = v[i.y];
    float z = v[i.z];
    float w = v[i.w];
    return float4(x, y, z, w);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = all(non_constant_swizzle(_uniforms) == float4(-1.25, -1.25, -1.25, 0.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
